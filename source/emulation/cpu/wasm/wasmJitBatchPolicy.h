/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __WASM_JIT_BATCH_POLICY_H__
#define __WASM_JIT_BATCH_POLICY_H__

#ifdef BOXEDWINE_WASM_JIT
#include "boxedwine.h"

using WasmJitBatchEntryId = U64;

struct WasmJitBatchKey {
    KMemory* memory = nullptr;
    U32 mappedFileKey = 0;
    bool operator==(const WasmJitBatchKey& other) const;
};

enum class WasmJitFlushReason : U8 {
    BlockCount,
    ByteCount,
    PendingHits,
    ProcessBytes,
};

struct WasmJitBatchLimits {
    U32 maxBlocks = 64;
    U64 maxBatchBytes = 512 * 1024;
    U32 urgentPendingHits = 8;
    U64 maxProcessOpenBytes = 4 * 1024 * 1024;
};

struct WasmJitFlushRequest {
    WasmJitBatchKey key;
    WasmJitFlushReason reason = WasmJitFlushReason::BlockCount;
    std::vector<WasmJitBatchEntryId> entries;
};

class WasmJitBatchPolicy {
public:
    explicit WasmJitBatchPolicy(const WasmJitBatchLimits& limits = {});
    ~WasmJitBatchPolicy();
    std::vector<WasmJitFlushRequest> enqueue(WasmJitBatchEntryId id, const WasmJitBatchKey& key, U64 byteCount);
    bool recordPendingHit(WasmJitBatchEntryId id, WasmJitFlushRequest& flush);
    bool cancel(WasmJitBatchEntryId id);
    std::vector<WasmJitBatchEntryId> cancelMemory(KMemory* memory);
    U64 openBytesForMemory(KMemory* memory) const;
    U32 openEntryCount() const;
    void reset(const WasmJitBatchLimits& limits = {});

private:
    struct State;
    std::unique_ptr<State> state;
};

#endif
#endif
