/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"
#ifdef BOXEDWINE_WASM_JIT
#include "wasmJitBatchPolicy.h"

#include <limits>
#include <map>

namespace {
struct OpenEntry {
    WasmJitBatchEntryId id = 0;
    U64 bytes = 0;
    U32 pendingHits = 0;
};

struct OpenBatch {
    WasmJitBatchKey key;
    U64 sequence = 0;
    U64 bytes = 0;
    std::vector<OpenEntry> entries;
};
}

struct WasmJitBatchPolicy::State {
    explicit State(const WasmJitBatchLimits& limits) : limits(limits) {}

    WasmJitBatchLimits limits;
    U64 nextSequence = 1;
    std::vector<OpenBatch> openBatches;
    std::map<WasmJitBatchEntryId, WasmJitBatchKey> entryIndex;
    std::vector<WasmJitBatchEntryId> entryOrder;

    size_t findBatch(const WasmJitBatchKey& key) const {
        for (size_t i = 0; i < openBatches.size(); ++i) {
            if (openBatches[i].key == key) {
                return i;
            }
        }
        return openBatches.size();
    }

    void eraseEntryOrder(WasmJitBatchEntryId id) {
        auto found = std::find(entryOrder.begin(), entryOrder.end(), id);
        if (found != entryOrder.end()) {
            entryOrder.erase(found);
        }
    }

    WasmJitFlushRequest seal(size_t index, WasmJitFlushReason reason) {
        OpenBatch& batch = openBatches[index];
        WasmJitFlushRequest request;
        request.key = batch.key;
        request.reason = reason;
        request.entries.reserve(batch.entries.size());
        for (const OpenEntry& entry : batch.entries) {
            request.entries.push_back(entry.id);
            entryIndex.erase(entry.id);
            eraseEntryOrder(entry.id);
        }
        openBatches.erase(openBatches.begin() + index);
        return request;
    }
};

bool WasmJitBatchKey::operator==(const WasmJitBatchKey& other) const {
    return memory == other.memory && mappedFileKey == other.mappedFileKey;
}

WasmJitBatchPolicy::WasmJitBatchPolicy(const WasmJitBatchLimits& limits)
    : state(std::make_unique<State>(limits)) {
}

WasmJitBatchPolicy::~WasmJitBatchPolicy() = default;

std::vector<WasmJitFlushRequest> WasmJitBatchPolicy::enqueue(WasmJitBatchEntryId id, const WasmJitBatchKey& key, U64 byteCount) {
    std::vector<WasmJitFlushRequest> flushes;
    size_t index = state->findBatch(key);
    if (index != state->openBatches.size()) {
        U64 openBytes = state->openBatches[index].bytes;
        if (openBytes > state->limits.maxBatchBytes || byteCount > state->limits.maxBatchBytes - openBytes) {
            flushes.push_back(state->seal(index, WasmJitFlushReason::ByteCount));
            index = state->openBatches.size();
        }
    }

    if (index == state->openBatches.size()) {
        OpenBatch batch;
        batch.key = key;
        batch.sequence = state->nextSequence++;
        state->openBatches.push_back(batch);
        index = state->openBatches.size() - 1;
    }

    OpenBatch& batch = state->openBatches[index];
    batch.entries.push_back({id, byteCount, 0});
    batch.bytes += byteCount;
    state->entryIndex[id] = key;
    state->entryOrder.push_back(id);

    bool oversizedEntry = byteCount > state->limits.maxBatchBytes;
    if (oversizedEntry) {
        flushes.push_back(state->seal(index, WasmJitFlushReason::ByteCount));
    } else if (batch.entries.size() >= state->limits.maxBlocks) {
        flushes.push_back(state->seal(index, WasmJitFlushReason::BlockCount));
    } else if (batch.bytes >= state->limits.maxBatchBytes) {
        flushes.push_back(state->seal(index, WasmJitFlushReason::ByteCount));
    }

    while (openBytesForMemory(key.memory) > state->limits.maxProcessOpenBytes) {
        size_t oldest = state->openBatches.size();
        U64 oldestSequence = std::numeric_limits<U64>::max();
        for (size_t i = 0; i < state->openBatches.size(); ++i) {
            const OpenBatch& open = state->openBatches[i];
            if (open.key.memory == key.memory && !open.entries.empty() && open.sequence < oldestSequence) {
                oldest = i;
                oldestSequence = open.sequence;
            }
        }
        if (oldest == state->openBatches.size()) {
            break;
        }
        flushes.push_back(state->seal(oldest, WasmJitFlushReason::ProcessBytes));
    }
    return flushes;
}

bool WasmJitBatchPolicy::recordPendingHit(WasmJitBatchEntryId id, WasmJitFlushRequest& flush) {
    auto indexed = state->entryIndex.find(id);
    if (indexed == state->entryIndex.end()) {
        return false;
    }
    size_t batchIndex = state->findBatch(indexed->second);
    if (batchIndex == state->openBatches.size()) {
        return false;
    }
    OpenBatch& batch = state->openBatches[batchIndex];
    for (OpenEntry& entry : batch.entries) {
        if (entry.id == id) {
            if (entry.pendingHits != std::numeric_limits<U32>::max()) {
                ++entry.pendingHits;
            }
            if (entry.pendingHits >= state->limits.urgentPendingHits) {
                flush = state->seal(batchIndex, WasmJitFlushReason::PendingHits);
                return true;
            }
            return false;
        }
    }
    return false;
}

bool WasmJitBatchPolicy::cancel(WasmJitBatchEntryId id) {
    auto indexed = state->entryIndex.find(id);
    if (indexed == state->entryIndex.end()) {
        return false;
    }
    size_t batchIndex = state->findBatch(indexed->second);
    if (batchIndex == state->openBatches.size()) {
        return false;
    }
    OpenBatch& batch = state->openBatches[batchIndex];
    for (size_t entryIndex = 0; entryIndex < batch.entries.size(); ++entryIndex) {
        if (batch.entries[entryIndex].id == id) {
            batch.bytes -= batch.entries[entryIndex].bytes;
            batch.entries.erase(batch.entries.begin() + entryIndex);
            state->entryIndex.erase(indexed);
            state->eraseEntryOrder(id);
            if (batch.entries.empty()) {
                state->openBatches.erase(state->openBatches.begin() + batchIndex);
            }
            return true;
        }
    }
    return false;
}

std::vector<WasmJitBatchEntryId> WasmJitBatchPolicy::cancelMemory(KMemory* memory) {
    std::vector<WasmJitBatchEntryId> cancelled;
    for (WasmJitBatchEntryId id : state->entryOrder) {
        auto indexed = state->entryIndex.find(id);
        if (indexed != state->entryIndex.end() && indexed->second.memory == memory) {
            cancelled.push_back(id);
        }
    }
    for (WasmJitBatchEntryId id : cancelled) {
        cancel(id);
    }
    return cancelled;
}

U64 WasmJitBatchPolicy::openBytesForMemory(KMemory* memory) const {
    U64 result = 0;
    for (const OpenBatch& batch : state->openBatches) {
        if (batch.key.memory == memory) {
            result += batch.bytes;
        }
    }
    return result;
}

U32 WasmJitBatchPolicy::openEntryCount() const {
    return static_cast<U32>(state->entryIndex.size());
}

void WasmJitBatchPolicy::reset(const WasmJitBatchLimits& limits) {
    state->limits = limits;
    state->nextSequence = 1;
    state->openBatches.clear();
    state->entryIndex.clear();
    state->entryOrder.clear();
}
#endif
