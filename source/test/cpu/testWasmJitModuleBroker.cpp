/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"
#include "testCPU.h"
#include "testWasmJitModuleBroker.h"

#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED) && defined(__EMSCRIPTEN__)

#include "../../emulation/cpu/wasm/jitWasmCodeGen.h"
#include "../../emulation/cpu/wasm/wasmEmitter.h"
#include "../../emulation/cpu/wasm/wasmJitBatchPolicy.h"
#include "../../emulation/cpu/wasm/wasmModuleMerger.h"
#include <climits>
#include <emscripten.h>
#include <emscripten/threading.h>

namespace {

constexpr S32 BROKER_LOOKUP_LOCAL_COMPILE = 1;
constexpr S32 BROKER_LOOKUP_HIT = 2;
constexpr U32 BROKER_MODULE_CLASS_STANDALONE = 1;
constexpr U32 BROKER_MODULE_CLASS_GROUPED = 2;
constexpr U32 BROKER_PRELOAD_LIMIT = 64;

std::vector<U8> makeBrokerTestModule() {
    WasmEmitter emitter;
    U32 type = emitter.addFuncType({}, {WasmType::I32});
    U32 function = emitter.addFunction(type);
    emitter.addExport("execute", function);
    emitter.beginFunction({});
    emitter.emitI32Const(42);
    emitter.endFunction();
    return emitter.finalize();
}

std::vector<U8> makeBrokerStoreModule(S32 value) {
    WasmEmitter emitter;
    U32 type = emitter.addFuncType({WasmType::I32, WasmType::I32}, {});
    emitter.addMemoryImport("env", "memory");
    U32 function = emitter.addFunction(type);
    emitter.addExport("execute", function);
    emitter.beginFunction({});
    emitter.emitLocalGet(0);
    emitter.emitI32Const(value);
    emitter.emitI32Store(0);
    emitter.endFunction();
    return emitter.finalize();
}

EM_JS(S32, testWasmJitBrokerLookup, (U32 moduleId, U32 memoryId, U32 memoryIncarnation,
        const U8* bytes, U32 size, U32 moduleClass, U32 representedBlockCount, S32* source), {
    if (typeof globalThis.bwWasmJitBrokerGetOrCompile !== 'function') {
        return -1;
    }
    try {
        var wasmBytes = new Uint8Array(HEAPU8.buffer, bytes, size);
        var result = globalThis.bwWasmJitBrokerGetOrCompile(moduleId, memoryId, memoryIncarnation,
            wasmBytes, true, moduleClass, representedBlockCount);
        HEAP32[source >> 2] = result.source;
        var instance = new WebAssembly.Instance(result.module, {});
        return instance.exports.execute();
    } catch (error) {
        if (!error || error.bwWasmJitBrokerOom !== true) {
            console.error('[WASM JIT broker test] lookup failed:', error);
        }
        return -1;
    }
});

EM_JS(void, testWasmJitBrokerSetPublicationDelayed, (U32 moduleId, S32 delayed), {
    globalThis.bwWasmJitBrokerTestSetPublicationDelayed(moduleId, delayed !== 0);
});

EM_JS(void, testWasmJitBrokerDeliveryBarrier, (U32 moduleId, S32* expected, S32* received), {
    Atomics.store(HEAP32, expected >> 2, -1);
    Atomics.store(HEAP32, received >> 2, 0);
    var pthread = typeof _pthread_self === 'function' ? _pthread_self() : 0;
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerDeliveryBarrier',
        args: [moduleId, pthread, expected, received]
    });
});

EM_JS(void, testWasmJitBrokerPublishRaw, (U32 moduleId, U32 memoryId, U32 memoryIncarnation,
        const U8* bytes, U32 size, U32 moduleClass, U32 representedBlockCount), {
    var module = new WebAssembly.Module(new Uint8Array(HEAPU8.buffer, bytes, size));
    if (typeof ENVIRONMENT_IS_PTHREAD !== 'undefined' && ENVIRONMENT_IS_PTHREAD) {
        postMessage({
            cmd: 'callHandler',
            handler: 'bwWasmJitBrokerPublish',
            args: [moduleId, memoryId, memoryIncarnation, module,
                typeof _pthread_self === 'function' ? _pthread_self() : 0,
                moduleClass, representedBlockCount]
        });
    } else {
        Module.bwWasmJitBrokerPublish(moduleId, memoryId, memoryIncarnation, module, 0,
            moduleClass, representedBlockCount);
    }
});

EM_JS(void, testWasmJitBrokerPublishRawWithoutSource,
        (U32 moduleId, U32 memoryId, U32 memoryIncarnation,
         const U8* bytes, U32 size, U32 moduleClass, U32 representedBlockCount), {
    var module = new WebAssembly.Module(new Uint8Array(HEAPU8.buffer, bytes, size));
    if (typeof ENVIRONMENT_IS_PTHREAD !== 'undefined' && ENVIRONMENT_IS_PTHREAD) {
        postMessage({
            cmd: 'callHandler',
            handler: 'bwWasmJitBrokerPublish',
            args: [moduleId, memoryId, memoryIncarnation, module, 0,
                moduleClass, representedBlockCount]
        });
    } else {
        Module.bwWasmJitBrokerPublish(moduleId, memoryId, memoryIncarnation, module, 0,
            moduleClass, representedBlockCount);
    }
});

EM_JS(void, testWasmJitBrokerPublicationBarrier,
        (U32 moduleId, S32* preloadSent, S32* receivedCacheEntries, S32* done), {
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerPublicationBarrier',
        args: [moduleId, preloadSent, receivedCacheEntries, done]
    });
});

EM_JS(void, testWasmJitBrokerSelectPreload,
        (U32 memoryId, U32 memoryIncarnation, const U32* heldIds, U32 heldCount,
         U32* sentIds, U32 sentCapacity, S32* sentCount, S32* done), {
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerSelectPreload',
        args: [memoryId, memoryIncarnation,
            Array.from(new Uint32Array(HEAPU8.buffer, heldIds, heldCount)),
            sentIds, sentCapacity, sentCount, done]
    });
});

EM_JS(void, testWasmJitBrokerSetTestPreloadCandidate,
        (U32 templateModuleId, U32 moduleId, U32 memoryId, U32 memoryIncarnation,
         U32 moduleClass, U32 representedBlockCount, S32 duplicatePublications,
         S32 enabled, S32* status), {
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerSetTestPreloadCandidate',
        args: [templateModuleId, moduleId, memoryId, memoryIncarnation,
            moduleClass, representedBlockCount, duplicatePublications, enabled, status]
    });
});

EM_JS(void, testWasmJitBrokerReleaseRaw, (U32 moduleId, U32 memoryId, U32 memoryIncarnation), {
    globalThis.bwWasmJitBrokerReleaseMemory(memoryId, memoryIncarnation, [moduleId]);
});

EM_JS(void, testWasmJitBrokerCopyMainStats, (WasmJitMtBrokerMainStatsSnapshot* snapshot, S32* done), {
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerCopyMainStats',
        args: [snapshot, done]
    });
});

EM_JS(void, testWasmJitBrokerPurgeBarrier, (U32 moduleId, U32 memoryId, U32 memoryIncarnation,
        S32* expected, S32* received, S32* cached), {
    Atomics.store(HEAP32, expected >> 2, -1);
    Atomics.store(HEAP32, received >> 2, 0);
    Atomics.store(HEAP32, cached >> 2, 0);
    var pthread = typeof _pthread_self === 'function' ? _pthread_self() : 0;
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerPurgeBarrier',
        args: [moduleId, memoryId, memoryIncarnation, pthread, expected, received, cached]
    });
});

EM_JS(S32, testWasmJitBrokerHasLocalModule, (U32 moduleId, U32 memoryId, U32 memoryIncarnation), {
    var entry = globalThis.bwWasmJitBrokerTestGetLocalModule &&
        globalThis.bwWasmJitBrokerTestGetLocalModule(moduleId, memoryId, memoryIncarnation);
    return entry ? 1 : 0;
});

EM_JS(U32, testWasmJitBrokerPartialPurgePublicationBlock,
        (U32 memoryId, U32 memoryIncarnation, U32 firstModuleId, U32 secondModuleId), {
    if (typeof globalThis.bwWasmJitBrokerTestPartialPurgePublicationBlock !== 'function') {
        return 0;
    }
    return globalThis.bwWasmJitBrokerTestPartialPurgePublicationBlock(
        memoryId, memoryIncarnation, firstModuleId, secondModuleId);
});

using BrokerExecMainStatsSnapshot = WasmJitMtBrokerMainStatsSnapshot;

EM_JS(void, testWasmJitBrokerCopyHolderCounts,
        (U32 moduleId, S32* holders, S32* workers, S32* done), {
    var pthread = typeof _pthread_self === 'function' ? _pthread_self() : 0;
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerCopyHolderCounts',
        args: [moduleId, pthread, holders, workers, done]
    });
});

EM_JS(void, testWasmJitBrokerTrackPurges, (U32 memoryId, U32 memoryIncarnation, S32* done), {
    var pthread = typeof _pthread_self === 'function' ? _pthread_self() : 0;
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerTrackPurges',
        args: [memoryId, memoryIncarnation, pthread, done]
    });
});

EM_JS(void, testWasmJitBrokerPurgeCountBarrier,
        (U32 memoryId, U32 memoryIncarnation, S32* expected, S32* received,
         S32* purgeMessages, S32* cachedModules), {
    Atomics.store(HEAP32, expected >> 2, -1);
    Atomics.store(HEAP32, received >> 2, 0);
    Atomics.store(HEAP32, purgeMessages >> 2, 0);
    Atomics.store(HEAP32, cachedModules >> 2, 0);
    var pthread = typeof _pthread_self === 'function' ? _pthread_self() : 0;
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerPurgeCountBarrier',
        args: [memoryId, memoryIncarnation, pthread,
            expected, received, purgeMessages, cachedModules]
    });
});

EM_JS(void, testWasmJitBrokerCopyQueryDisabled, (S32* queryDisabled, S32* done), {
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerCopyQueryDisabled',
        args: [queryDisabled, done]
    });
});

EM_JS(void, testWasmJitBrokerSetDeliveryFailure, (U32 moduleId, S32 enabled, S32* done), {
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerSetDeliveryFailure',
        args: [moduleId, enabled, done]
    });
});

EM_JS(void, testWasmJitBrokerSetDeliveryOom, (U32 moduleId, S32 enabled, S32* done), {
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerSetDeliveryOom',
        args: [moduleId, enabled, done]
    });
});

EM_JS(void, testWasmJitBrokerSetCompileOom, (U32 moduleId, S32 enabled), {
    globalThis.bwWasmJitBrokerTestSetCompileOom(moduleId, enabled !== 0);
});

EM_JS(void, testWasmJitBrokerSetDropNextRun, (S32 enabled, S32* done), {
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerSetDropNextRun',
        args: [enabled, done]
    });
});

EM_JS(void, testWasmJitBrokerBootstrapOwnerMiss, (S32* done), {
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerTestBootstrapOwnerMiss',
        args: [done]
    });
});

EM_JS(S32, testWasmJitBrokerSupportsDroppedRunTimeout, (), {
    return typeof ENVIRONMENT_IS_NODE !== 'undefined' && ENVIRONMENT_IS_NODE ? 0 : 1;
});

EM_JS(void, testWasmJitBrokerCopyPreloadOomScope,
        (U32 firstModuleId, U32 firstMemoryId, U32 firstMemoryIncarnation,
         U32 secondModuleId, U32 secondMemoryId, U32 secondMemoryIncarnation,
         U32* snapshot, S32* done), {
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerCopyPreloadOomScope',
        args: [firstModuleId, firstMemoryId, firstMemoryIncarnation,
            secondModuleId, secondMemoryId, secondMemoryIncarnation,
            snapshot, done]
    });
});

EM_JS(void, testWasmJitBrokerCopyPublicPreloadLimit, (S32* preloadModuleLimit, S32* done), {
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerCopyPublicPreloadLimit',
        args: [preloadModuleLimit, done]
    });
});

struct BrokerWorkerDiagnosticStats {
    U32 localCompiles = 0;
    U32 brokerHits = 0;
    U32 groupInstanceCreations = 0;
    U32 groupInstanceReuses = 0;
    U32 groupedBrokerHits = 0;
    U32 groupedLocalCompiles = 0;
};

EM_JS(void, testWasmJitBrokerCopyOwnerRowCounts,
        (U32* mainRows, U32* workerRows, BrokerWorkerDiagnosticStats* workerStats,
         U32 workerCapacity,
         U32* workerCount, U32* expectedWorkers, U32* repliedWorkers,
         U32* missingWorkers, U32* runMessages, U32* preloadedRunMessages,
         U32* preloadModulesBeforeRun, S32* done), {
    var pthread = typeof _pthread_self === 'function' ? _pthread_self() : 0;
    var localSnapshot = typeof globalThis.bwWasmJitBrokerTestLocalSnapshot === 'function' ?
        globalThis.bwWasmJitBrokerTestLocalSnapshot() : {
            localCompiles: 0,
            brokerHits: 0,
            groupInstanceCreations: 0,
            groupInstanceReuses: 0,
            groupedBrokerHits: 0,
            groupedLocalCompiles: 0,
            byOwner: []
        };
    postMessage({
        cmd: 'callHandler',
        handler: 'bwWasmJitBrokerCopyOwnerRowCounts',
        args: [mainRows, workerRows, workerStats, workerCapacity, workerCount,
            expectedWorkers, repliedWorkers, missingWorkers, runMessages,
            preloadedRunMessages, preloadModulesBeforeRun,
            pthread, localSnapshot, done]
    });
});

struct BrokerGroupWorkerStats {
    U32 groupInstanceCreations = 0;
    U32 groupInstanceReuses = 0;
    U32 groupedBrokerHits = 0;
    U32 groupedLocalCompiles = 0;
};

EM_JS(void, testWasmJitBrokerCopyGroupWorkerStats, (BrokerGroupWorkerStats* snapshot), {
    var stats = globalThis.bwWasmJitMtGroupWorkerStats || {};
    HEAPU32[(snapshot >> 2) + 0] = stats.groupInstanceCreations || 0;
    HEAPU32[(snapshot >> 2) + 1] = stats.groupInstanceReuses || 0;
    HEAPU32[(snapshot >> 2) + 2] = stats.groupedBrokerHits || 0;
    HEAPU32[(snapshot >> 2) + 3] = stats.groupedLocalCompiles || 0;
});

EM_JS(S32, testWasmJitBrokerCallGroupSlot, (S32 tableIndex, S32* destination), {
    HEAP32[destination >> 2] = 0;
    var fn = wasmTable.get(tableIndex);
    if (!fn) {
        return 0;
    }
    fn(destination, 0);
    return 1;
});

EM_JS(S32, testWasmJitBrokerHasGroupInstance,
        (U32 moduleId, U32 groupKind, U32 groupIdentity, U32 memoryId, U32 memoryIncarnation), {
    if (typeof globalThis.bwWasmJitMtGroupInstanceKey !== 'function' ||
            !globalThis.bwJitMtGroupInstances) {
        return 0;
    }
    var key = globalThis.bwWasmJitMtGroupInstanceKey(
        moduleId, groupKind, groupIdentity, memoryId, memoryIncarnation);
    return globalThis.bwJitMtGroupInstances.has(key) ? 1 : 0;
});

EM_JS(S32, testWasmJitBrokerSlotIsClear, (S32 tableIndex), {
    return tableIndex >= 0 && tableIndex < wasmTable.length &&
        wasmTable.get(tableIndex) === null ? 1 : 0;
});

bool waitForBrokerDelivery(S32& expected, S32& received) {
    double deadline = emscripten_get_now() + 5000.0;
    while (emscripten_get_now() < deadline) {
        S32 expectedValue = __atomic_load_n(&expected, __ATOMIC_ACQUIRE);
        S32 receivedValue = __atomic_load_n(&received, __ATOMIC_ACQUIRE);
        if (expectedValue > 0 && receivedValue >= expectedValue) {
            return true;
        }
        emscripten_futex_wait(&received, receivedValue, 50.0);
    }
    return false;
}

bool waitForAtomicDone(S32& done) {
    double deadline = emscripten_get_now() + 5000.0;
    while (emscripten_get_now() < deadline) {
        S32 value = __atomic_load_n(&done, __ATOMIC_ACQUIRE);
        if (value) {
            return true;
        }
        emscripten_futex_wait(&done, value, 50.0);
    }
    return false;
}

BrokerGroupWorkerStats getBrokerGroupWorkerStats() {
    BrokerGroupWorkerStats result;
    testWasmJitBrokerCopyGroupWorkerStats(&result);
    return result;
}

BrokerExecMainStatsSnapshot getBrokerExecMainStats() {
    return wasmJitTestGetMtBrokerMainStats();
}

bool getBrokerHolderCounts(U32 moduleId, S32& holders, S32& workers) {
    S32 done = 0;
    holders = -1;
    workers = -1;
    testWasmJitBrokerCopyHolderCounts(moduleId, &holders, &workers, &done);
    return waitForAtomicDone(done);
}

bool trackBrokerPurges(U32 memoryId, U32 memoryIncarnation) {
    S32 done = 0;
    testWasmJitBrokerTrackPurges(memoryId, memoryIncarnation, &done);
    return waitForAtomicDone(done);
}

bool waitForBrokerPurgeCount(U32 memoryId, U32 memoryIncarnation,
        S32& purgeMessages, S32& cachedModules) {
    S32 expected = -1;
    S32 received = 0;
    purgeMessages = 0;
    cachedModules = 0;
    testWasmJitBrokerPurgeCountBarrier(memoryId, memoryIncarnation,
        &expected, &received, &purgeMessages, &cachedModules);
    return waitForBrokerDelivery(expected, received);
}

bool setTestPreloadCandidate(U32 templateModuleId, U32 moduleId, U32 memoryId,
        U32 memoryIncarnation, U32 moduleClass, U32 representedBlockCount,
        S32 duplicatePublications, bool enabled) {
    S32 status = 0;
    testWasmJitBrokerSetTestPreloadCandidate(templateModuleId, moduleId, memoryId,
        memoryIncarnation, moduleClass, representedBlockCount, duplicatePublications,
        enabled ? 1 : 0, &status);
    return waitForAtomicDone(status) && status == 1;
}

bool waitForBrokerPublication(U32 moduleId, S32* preloadSent = nullptr, S32* receivedCacheEntries = nullptr) {
    S32 sent = -1;
    S32 received = -1;
    S32 done = 0;
    testWasmJitBrokerPublicationBarrier(moduleId, &sent, &received, &done);
    if (!waitForAtomicDone(done)) {
        return false;
    }
    if (preloadSent) {
        *preloadSent = sent;
    }
    if (receivedCacheEntries) {
        *receivedCacheEntries = received;
    }
    return true;
}

struct BrokerThreadArgs {
    U32 moduleId = 0;
    U32 memoryId = 0;
    U32 memoryIncarnation = 0;
    const U8* bytes = nullptr;
    U32 size = 0;
    S32 value = 0;
    S32 source = 0;
};

enum class BrokerBoundedThreadOperation : U32 {
    Run,
    CompileOom
};

enum class BrokerThreadRegistration : U32 {
    Missing,
    Current,
    Stale
};

struct BrokerBoundedThreadState {
    BrokerBoundedThreadOperation operation = BrokerBoundedThreadOperation::Run;
    S32 started = 0;
    S32 completed = 0;
    U32 cachedModuleId = 0;
    U32 compileModuleId = 0;
    U32 memoryId = 0;
    U32 memoryIncarnation = 0;
    U32 otherModuleId = 0;
    U32 otherMemoryId = 0;
    U32 otherMemoryIncarnation = 0;
    std::vector<U8> bytes;
    S32 oomValue = 0;
    S32 cachedValue = 0;
    S32 cachedSource = 0;
    S32 firstCompileValue = 0;
    S32 firstCompileSource = 0;
    S32 cachedAfterCompile = 0;
    S32 secondCompileValue = 0;
    S32 secondCompileSource = 0;
    S32 otherValue = 0;
    S32 otherSource = 0;
    S32 otherCached = 0;
};

void* brokerBoundedThread(void* arg) {
    BrokerBoundedThreadState* state = (BrokerBoundedThreadState*)arg;
    __atomic_store_n(&state->started, 1, __ATOMIC_RELEASE);
    if (state->operation == BrokerBoundedThreadOperation::CompileOom) {
        S32 oomSource = 0;
        wasmJitTestSetMtBrokerCompileOom(state->compileModuleId, true);
        state->oomValue = testWasmJitBrokerLookup(state->compileModuleId,
            state->memoryId, state->memoryIncarnation,
            state->bytes.data(), (U32)state->bytes.size(),
            BROKER_MODULE_CLASS_STANDALONE, 1, &oomSource);
        wasmJitTestSetMtBrokerCompileOom(state->compileModuleId, false);
        state->cachedValue = testWasmJitBrokerLookup(state->cachedModuleId,
            state->memoryId, state->memoryIncarnation,
            state->bytes.data(), (U32)state->bytes.size(),
            BROKER_MODULE_CLASS_STANDALONE, 1, &state->cachedSource);
        state->firstCompileValue = testWasmJitBrokerLookup(state->compileModuleId,
            state->memoryId, state->memoryIncarnation,
            state->bytes.data(), (U32)state->bytes.size(),
            BROKER_MODULE_CLASS_STANDALONE, 1, &state->firstCompileSource);
        state->cachedAfterCompile = testWasmJitBrokerHasLocalModule(state->compileModuleId,
            state->memoryId, state->memoryIncarnation);
        state->secondCompileValue = testWasmJitBrokerLookup(state->compileModuleId,
            state->memoryId, state->memoryIncarnation,
            state->bytes.data(), (U32)state->bytes.size(),
            BROKER_MODULE_CLASS_STANDALONE, 1, &state->secondCompileSource);
        state->otherValue = testWasmJitBrokerLookup(state->otherModuleId,
            state->otherMemoryId, state->otherMemoryIncarnation,
            state->bytes.data(), (U32)state->bytes.size(),
            BROKER_MODULE_CLASS_STANDALONE, 1, &state->otherSource);
        state->otherCached = testWasmJitBrokerHasLocalModule(state->otherModuleId,
            state->otherMemoryId, state->otherMemoryIncarnation);
    }
    __atomic_store_n(&state->completed, 1, __ATOMIC_RELEASE);
    emscripten_futex_wake(&state->completed, INT_MAX);
    return nullptr;
}

struct BrokerPreloadOomScopeSnapshot {
    U32 distinctWorkers = 0;
    U32 firstWorkerRuns = 0;
    U32 secondWorkerRuns = 0;
    U32 firstWorkerFirstOwnerModules = 0;
    U32 secondWorkerFirstOwnerModules = 0;
    U32 firstWorkerSecondOwnerModules = 0;
    U32 preloadAttempts = 0;
    U32 preloadSent = 0;
    U32 preloadFailures = 0;
    U32 preloadOomBlocks = 0;
};

void* brokerPreloadThread(void* arg) {
    BrokerThreadArgs* args = (BrokerThreadArgs*)arg;
    args->value = testWasmJitBrokerLookup(args->moduleId, args->memoryId, args->memoryIncarnation,
        args->bytes, args->size, BROKER_MODULE_CLASS_STANDALONE, 1, &args->source);
    return nullptr;
}

struct BrokerLatePublisherArgs : BrokerThreadArgs {
    BrokerExecMainStatsSnapshot stats;
};

void* brokerLatePublisherThread(void* arg) {
    BrokerLatePublisherArgs* args = (BrokerLatePublisherArgs*)arg;
    args->value = testWasmJitBrokerLookup(args->moduleId, args->memoryId, args->memoryIncarnation,
        args->bytes, args->size, BROKER_MODULE_CLASS_STANDALONE, 1, &args->source);
    args->stats = getBrokerExecMainStats();
    return nullptr;
}

struct BrokerLazyInstallArgs {
    int firstSlot = -1;
    int secondSlot = -1;
    bool firstInstalled = false;
    bool secondInstalled = false;
    bool callSlots = false;
    S32 firstValue = 0;
    S32 secondValue = 0;
    bool inspectFallbackKeys = false;
    U32 groupKind = 0;
    U32 groupIdentity = 0;
    U32 memoryId = 0;
    U32 firstMemoryIncarnation = 0;
    U32 secondMemoryIncarnation = 0;
    bool oldKeyAfterFirst = false;
    bool newKeyAfterFirst = false;
    bool oldKeyAfterSecond = false;
    bool newKeyAfterSecond = false;
    U32 instanceCount = 0;
    BrokerGroupWorkerStats stats;
};

void* brokerLazyInstallThread(void* arg) {
    BrokerLazyInstallArgs* args = (BrokerLazyInstallArgs*)arg;
    U32 beforeInstances = wasmJitTestCurrentWorkerRuntimeGroupInstanceCount();
    BrokerGroupWorkerStats beforeStats = getBrokerGroupWorkerStats();
    args->firstInstalled = wasmJitTestLazyInstallMtSlot(args->firstSlot);
    if (args->callSlots && args->firstInstalled) {
        args->firstInstalled =
            testWasmJitBrokerCallGroupSlot(args->firstSlot, &args->firstValue) != 0;
    }
    if (args->inspectFallbackKeys) {
        args->oldKeyAfterFirst = testWasmJitBrokerHasGroupInstance(
            0, args->groupKind, args->groupIdentity, args->memoryId,
            args->firstMemoryIncarnation) != 0;
        args->newKeyAfterFirst = testWasmJitBrokerHasGroupInstance(
            0, args->groupKind, args->groupIdentity, args->memoryId,
            args->secondMemoryIncarnation) != 0;
    }
    if (args->secondSlot >= 0) {
        args->secondInstalled = wasmJitTestLazyInstallMtSlot(args->secondSlot);
        if (args->callSlots && args->secondInstalled) {
            args->secondInstalled =
                testWasmJitBrokerCallGroupSlot(args->secondSlot, &args->secondValue) != 0;
        }
    }
    if (args->inspectFallbackKeys) {
        args->oldKeyAfterSecond = testWasmJitBrokerHasGroupInstance(
            0, args->groupKind, args->groupIdentity, args->memoryId,
            args->firstMemoryIncarnation) != 0;
        args->newKeyAfterSecond = testWasmJitBrokerHasGroupInstance(
            0, args->groupKind, args->groupIdentity, args->memoryId,
            args->secondMemoryIncarnation) != 0;
    }
    args->instanceCount = wasmJitTestCurrentWorkerRuntimeGroupInstanceCount() - beforeInstances;
    BrokerGroupWorkerStats afterStats = getBrokerGroupWorkerStats();
    args->stats.groupInstanceCreations =
        afterStats.groupInstanceCreations - beforeStats.groupInstanceCreations;
    args->stats.groupInstanceReuses =
        afterStats.groupInstanceReuses - beforeStats.groupInstanceReuses;
    args->stats.groupedBrokerHits =
        afterStats.groupedBrokerHits - beforeStats.groupedBrokerHits;
    args->stats.groupedLocalCompiles =
        afterStats.groupedLocalCompiles - beforeStats.groupedLocalCompiles;
    return nullptr;
}

struct MtNextModuleIdRestorer {
    U32 nextModuleId;

    ~MtNextModuleIdRestorer() {
        wasmJitTestSetMtNextModuleId(nextModuleId);
    }
};

struct BrokerTestProcessOwner {
    KProcessPtr process = KProcess::create();
    KMemory* memory = nullptr;
    U32 processId = 0;

    BrokerTestProcessOwner() {
        processId = process->id;
        memory = KMemory::create(process.get());
        process->memory = memory;
    }

    ~BrokerTestProcessOwner() {
        cleanup();
    }

    BrokerTestProcessOwner(const BrokerTestProcessOwner&) = delete;
    BrokerTestProcessOwner& operator=(const BrokerTestProcessOwner&) = delete;

    void cleanup() {
        if (!process) {
            return;
        }
        KProcessWeakPtr weakProcess = process;
        // Dropping the final process owner runs KMemory::cleanup(), which owns
        // the single broker lifecycle invalidation for this test process.
        if (KSystem::getProcess(processId)) {
            KSystem::eraseProcess(processId);
        }
        process.reset();
        memory = nullptr;
        if (!weakProcess.expired()) {
            testFail("MT WASM broker preload secondary process has an unexpected strong owner");
        }
    }
};

bool runPreparedBrokerThread(void* (*entry)(void*), void* arg, KMemory* owner) {
    wasmJitTestPrepareMtThreadStart(arg, owner);
    pthread_t thread;
    if (pthread_create(&thread, nullptr, entry, arg)) {
        wasmJitTestCancelMtThreadStart(arg);
        return false;
    }
    return pthread_join(thread, nullptr) == 0;
}

bool waitForBoundedBrokerThread(BrokerBoundedThreadState* state) {
    double deadline = emscripten_get_now() + 5000.0;
    while (emscripten_get_now() < deadline) {
        S32 completed = __atomic_load_n(&state->completed, __ATOMIC_ACQUIRE);
        if (completed) {
            return __atomic_load_n(&state->started, __ATOMIC_ACQUIRE) != 0;
        }
        emscripten_futex_wait(&state->completed, completed, 50.0);
    }
    return false;
}

bool runBoundedBrokerThread(BrokerBoundedThreadState& result, KMemory* owner,
        BrokerThreadRegistration registration) {
    auto state = std::make_unique<BrokerBoundedThreadState>(result);
    BrokerBoundedThreadState* rawState = state.get();
    if (registration == BrokerThreadRegistration::Current) {
        wasmJitTestPrepareMtThreadStart(rawState, owner);
    } else if (registration == BrokerThreadRegistration::Stale) {
        wasmJitTestPrepareMtStaleThreadStart(rawState, owner);
    }
    pthread_t thread;
    if (pthread_create(&thread, nullptr, brokerBoundedThread, rawState)) {
        wasmJitTestCancelMtThreadStart(rawState);
        return false;
    }
    state.release();
    if (!waitForBoundedBrokerThread(rawState)) {
        wasmJitTestCancelMtThreadStart(rawState);
        // A delayed worker can still start, so keep its heap-owned inputs alive.
        pthread_detach(thread);
        return false;
    }
    if (pthread_join(thread, nullptr)) {
        // Completion precedes the worker epilogue, so retain state unless joined.
        pthread_detach(thread);
        return false;
    }
    result = *rawState;
    delete rawState;
    return result.started && result.completed;
}

bool setDropNextBrokerRun(bool enabled) {
    S32 done = 0;
    testWasmJitBrokerSetDropNextRun(enabled ? 1 : 0, &done);
    return waitForAtomicDone(done);
}

bool testBootstrapOwnerMiss() {
    S32 done = 0;
    testWasmJitBrokerBootstrapOwnerMiss(&done);
    return waitForAtomicDone(done) && done == 1;
}

__attribute__((noinline)) void runBrokerDroppedRunDiagnostic() {
    if (!testWasmJitBrokerSupportsDroppedRunTimeout()) {
        return;
    }
    KMemory* timeoutOwner = reinterpret_cast<KMemory*>(0x7fff9f00);
    BrokerBoundedThreadState timeoutState;
    if (!setDropNextBrokerRun(true)) {
        testFail("MT WASM broker dropped-run test control timeout");
    }
    double timeoutStarted = emscripten_get_now();
    bool droppedRunForwarded = runBoundedBrokerThread(timeoutState,
        timeoutOwner, BrokerThreadRegistration::Current);
    double timeoutElapsed = emscripten_get_now() - timeoutStarted;
    if (!setDropNextBrokerRun(false)) {
        testFail("MT WASM broker dropped-run test control cleanup timeout");
    }
    if (droppedRunForwarded || timeoutState.started || timeoutState.completed ||
            timeoutElapsed < 4500.0 || timeoutElapsed > 6500.0) {
        testFail("MT WASM broker dropped run returns through bounded timeout path");
    }
    wasmJitTestInvalidateMtBrokerMemoryId((U32)(uintptr_t)timeoutOwner);
}

bool getPreloadOomScope(U32 firstModuleId, U32 firstMemoryId,
        U32 firstMemoryIncarnation, U32 secondModuleId, U32 secondMemoryId,
        U32 secondMemoryIncarnation, BrokerPreloadOomScopeSnapshot& snapshot) {
    S32 done = 0;
    testWasmJitBrokerCopyPreloadOomScope(firstModuleId, firstMemoryId,
        firstMemoryIncarnation, secondModuleId, secondMemoryId,
        secondMemoryIncarnation, (U32*)&snapshot, &done);
    return waitForAtomicDone(done);
}

bool sameMainStatsExceptOwnerMisses(const WasmJitMtBrokerMainStatsSnapshot& left,
        const WasmJitMtBrokerMainStatsSnapshot& right) {
    return left.firstPublications == right.firstPublications &&
        left.duplicatePublications == right.duplicatePublications &&
        left.ownershipErrors == right.ownershipErrors &&
        left.unknownModuleIds == right.unknownModuleIds &&
        left.scheduledDeliveries == right.scheduledDeliveries &&
        left.deliveryFailures == right.deliveryFailures &&
        left.purgedModules == right.purgedModules &&
        left.liveRegistryModules == right.liveRegistryModules &&
        left.preloadAttempts == right.preloadAttempts &&
        left.preloadCandidates == right.preloadCandidates &&
        left.preloadSent == right.preloadSent &&
        left.preloadSkippedHeld == right.preloadSkippedHeld &&
        left.preloadFailures == right.preloadFailures &&
        left.preloadOomBlocks == right.preloadOomBlocks &&
        left.staleIncarnationDrops == right.staleIncarnationDrops &&
        left.preloadModuleLimit == right.preloadModuleLimit;
}

struct BrokerOwnerRowCounts {
    U32 mainRows = 0;
    U32 expectedWorkers = 0;
    U32 repliedWorkers = 0;
    U32 missingWorkers = 0;
    U32 runMessages = 0;
    U32 preloadedRunMessages = 0;
    U32 preloadModulesBeforeRun = 0;
    std::vector<U32> workerRows;
    std::vector<BrokerWorkerDiagnosticStats> workerStats;
};

bool getBrokerOwnerRowCounts(BrokerOwnerRowCounts& result) {
    constexpr U32 MAX_WORKERS = 64;
    std::vector<U32> workerRows(MAX_WORKERS, 0);
    std::vector<BrokerWorkerDiagnosticStats> workerStats(MAX_WORKERS);
    U32 mainRows = 0;
    U32 workerCount = 0;
    U32 expectedWorkers = 0;
    U32 repliedWorkers = 0;
    U32 missingWorkers = 0;
    U32 runMessages = 0;
    U32 preloadedRunMessages = 0;
    U32 preloadModulesBeforeRun = 0;
    S32 done = 0;
    testWasmJitBrokerCopyOwnerRowCounts(&mainRows, workerRows.data(), workerStats.data(),
        (U32)workerRows.size(), &workerCount, &expectedWorkers,
        &repliedWorkers, &missingWorkers, &runMessages, &preloadedRunMessages,
        &preloadModulesBeforeRun, &done);
    if (!waitForAtomicDone(done) || done != 1 || workerCount > workerRows.size()) {
        std::cout << "MT WASM broker owner-row reply failure: status=" << done
            << " expected=" << expectedWorkers
            << " replied=" << repliedWorkers
            << " missing=" << missingWorkers << std::endl;
        return false;
    }
    workerRows.resize(workerCount);
    workerStats.resize(workerCount);
    result.mainRows = mainRows;
    result.expectedWorkers = expectedWorkers;
    result.repliedWorkers = repliedWorkers;
    result.missingWorkers = missingWorkers;
    result.runMessages = runMessages;
    result.preloadedRunMessages = preloadedRunMessages;
    result.preloadModulesBeforeRun = preloadModulesBeforeRun;
    result.workerRows = std::move(workerRows);
    result.workerStats = std::move(workerStats);
    return true;
}

bool brokerLifetimeTotalsMonotonic(const WasmJitMtBrokerMainStatsSnapshot& before,
        const WasmJitMtBrokerMainStatsSnapshot& after) {
    return after.firstPublications >= before.firstPublications &&
        after.duplicatePublications >= before.duplicatePublications &&
        after.ownershipErrors >= before.ownershipErrors &&
        after.unknownModuleIds >= before.unknownModuleIds &&
        after.scheduledDeliveries >= before.scheduledDeliveries &&
        after.deliveryFailures >= before.deliveryFailures &&
        after.purgedModules >= before.purgedModules &&
        after.preloadAttempts >= before.preloadAttempts &&
        after.preloadCandidates >= before.preloadCandidates &&
        after.preloadSent >= before.preloadSent &&
        after.preloadSkippedHeld >= before.preloadSkippedHeld &&
        after.preloadFailures >= before.preloadFailures &&
        after.preloadOwnerMisses >= before.preloadOwnerMisses &&
        after.preloadOomBlocks >= before.preloadOomBlocks &&
        after.staleIncarnationDrops >= before.staleIncarnationDrops &&
        after.preloadModuleLimit == before.preloadModuleLimit;
}

__attribute__((noinline)) void runBrokerOwnerRowStress(const std::vector<U8>& bytes) {
    constexpr U32 STRESS_OWNER_COUNT = 31;
    constexpr U32 STRESS_ROUNDS = 67;
    static_assert(STRESS_OWNER_COUNT * STRESS_ROUNDS >= 2048,
        "diagnostic owner stress must retire at least 2048 incarnations");

    if (testWasmJitBrokerPartialPurgePublicationBlock(
            0x7fff9e00, 1, 0xffffffe1, 0xffffffe2) != 31) {
        testFail("MT WASM broker partial purge preserves publication block until final exact purge");
    }

    BrokerOwnerRowCounts ownerRowBaseline;
    if (!getBrokerOwnerRowCounts(ownerRowBaseline)) {
        testFail("MT WASM broker owner-row baseline waits for every Worker reply");
        return;
    }
    WasmJitMtBrokerMainStatsSnapshot lifetimeBaseline = getBrokerExecMainStats();

    std::vector<std::unique_ptr<BrokerTestProcessOwner>> stressProcesses;
    stressProcesses.reserve(STRESS_OWNER_COUNT);
    std::vector<KMemory*> stressOwners;
    stressOwners.reserve(STRESS_OWNER_COUNT);
    for (U32 i = 0; i < STRESS_OWNER_COUNT; ++i) {
        stressProcesses.push_back(std::make_unique<BrokerTestProcessOwner>());
        stressOwners.push_back(stressProcesses.back()->memory);
    }
    std::vector<BrokerThreadArgs> stressArgs(STRESS_OWNER_COUNT);
    std::vector<pthread_t> stressThreads(STRESS_OWNER_COUNT);
    std::vector<U8> stressCreated(STRESS_OWNER_COUNT);
    std::vector<BrokerThreadArgs> retiredPublications;

    for (U32 round = 0; round < STRESS_ROUNDS; ++round) {
        for (U32 i = 0; i < STRESS_OWNER_COUNT; ++i) {
            KMemory* owner = stressOwners[i];
            BrokerThreadArgs& args = stressArgs[i];
            args = {};
            args.moduleId = wasmJitTestReserveMtBrokerModule(owner);
            args.memoryId = (U32)(uintptr_t)owner;
            args.memoryIncarnation = wasmJitTestGetMtMemoryIncarnation(owner);
            args.bytes = bytes.data();
            args.size = (U32)bytes.size();
            testWasmJitBrokerPublishRawWithoutSource(args.moduleId,
                args.memoryId, args.memoryIncarnation, args.bytes, args.size,
                BROKER_MODULE_CLASS_STANDALONE, 1);
        }
        if (!waitForBrokerPublication(stressArgs.back().moduleId)) {
            testFail("MT WASM broker diagnostic stress publication barrier");
            break;
        }

        std::fill(stressCreated.begin(), stressCreated.end(), 0);
        for (U32 i = 0; i < STRESS_OWNER_COUNT; ++i) {
            wasmJitTestPrepareMtThreadStart(&stressArgs[i], stressOwners[i]);
            if (pthread_create(&stressThreads[i], nullptr,
                    brokerPreloadThread, &stressArgs[i])) {
                wasmJitTestCancelMtThreadStart(&stressArgs[i]);
                testFail("MT WASM broker diagnostic stress pthread start");
                break;
            }
            stressCreated[i] = 1;
        }
        for (U32 i = 0; i < STRESS_OWNER_COUNT; ++i) {
            if (stressCreated[i] && pthread_join(stressThreads[i], nullptr)) {
                testFail("MT WASM broker diagnostic stress pthread join");
            }
            if (stressCreated[i] &&
                    (stressArgs[i].value != 42 ||
                     stressArgs[i].source != BROKER_LOOKUP_HIT)) {
                testFail("MT WASM broker diagnostic stress first lookup is preloaded");
            }
        }
        for (U32 i = 0; i < STRESS_OWNER_COUNT; ++i) {
            if (round + 1 == STRESS_ROUNDS && i < 4) {
                retiredPublications.push_back(stressArgs[i]);
            }
            wasmJitTestInvalidateMtBrokerMemoryId(stressArgs[i].memoryId);
        }
    }

    WasmJitMtBrokerMainStatsSnapshot beforeLatePublications =
        getBrokerExecMainStats();
    for (const BrokerThreadArgs& retired : retiredPublications) {
        testWasmJitBrokerPublishRaw(retired.moduleId,
            retired.memoryId, retired.memoryIncarnation,
            retired.bytes, retired.size, BROKER_MODULE_CLASS_STANDALONE, 1);
    }

    BrokerOwnerRowCounts ownerRowsBeforeRetiredRun;
    if (!getBrokerOwnerRowCounts(ownerRowsBeforeRetiredRun)) {
        testFail("MT WASM broker pre-stale-run row snapshot waits for every Worker reply");
    } else if (ownerRowsBeforeRetiredRun.mainRows != ownerRowBaseline.mainRows ||
            ownerRowsBeforeRetiredRun.workerRows != ownerRowBaseline.workerRows) {
        testFail("MT WASM broker late retired publications leave owner rows absent");
    }
    WasmJitMtBrokerMainStatsSnapshot beforeRetiredRun =
        getBrokerExecMainStats();
    BrokerBoundedThreadState retiredRun;
    bool retiredStarted = runBoundedBrokerThread(retiredRun,
        stressOwners[0], BrokerThreadRegistration::Stale);
    WasmJitMtBrokerMainStatsSnapshot afterRetiredRun =
        getBrokerExecMainStats();
    if (!retiredStarted || !retiredRun.started || !retiredRun.completed ||
            afterRetiredRun.preloadOwnerMisses !=
                beforeRetiredRun.preloadOwnerMisses + 1 ||
            afterRetiredRun.preloadAttempts != beforeRetiredRun.preloadAttempts) {
        testFail("MT WASM broker retired stale thread-start forwards with aggregate owner miss");
    }

    BrokerOwnerRowCounts ownerRowsAfterStress;
    if (!getBrokerOwnerRowCounts(ownerRowsAfterStress)) {
        testFail("MT WASM broker owner-row stress waits for every Worker reply");
    } else {
        if (ownerRowsAfterStress.mainRows != ownerRowBaseline.mainRows) {
            testFail("MT WASM broker retired stale thread-start does not recreate main owner row");
        }
        if (ownerRowsAfterStress.workerRows != ownerRowBaseline.workerRows) {
            testFail("MT WASM broker retired Worker diagnostic rows return to each live-owner baseline");
        }
    }
    WasmJitMtBrokerMainStatsSnapshot lifetimeAfterStress =
        getBrokerExecMainStats();
    if (lifetimeAfterStress.staleIncarnationDrops !=
            beforeLatePublications.staleIncarnationDrops +
                retiredPublications.size()) {
        testFail("MT WASM broker late retired publications increase aggregate stale drops");
    }
    if (!brokerLifetimeTotalsMonotonic(lifetimeBaseline, lifetimeAfterStress) ||
            lifetimeAfterStress.firstPublications <
                lifetimeBaseline.firstPublications +
                    STRESS_OWNER_COUNT * STRESS_ROUNDS ||
            lifetimeAfterStress.purgedModules <
                lifetimeBaseline.purgedModules +
                    STRESS_OWNER_COUNT * STRESS_ROUNDS) {
        testFail("MT WASM broker aggregate lifetime totals remain monotonic after row retirement");
    }
}

}

bool wasmJitTestWaitForMtBrokerDelivery(U32 moduleId) {
    S32 expected = -1;
    S32 received = 0;
    testWasmJitBrokerDeliveryBarrier(moduleId, &expected, &received);
    return waitForBrokerDelivery(expected, received);
}

WasmJitMtBrokerMainStatsSnapshot wasmJitTestGetMtBrokerMainStats() {
    WasmJitMtBrokerMainStatsSnapshot snapshot;
    S32 done = 0;
    testWasmJitBrokerCopyMainStats(&snapshot, &done);
    if (!waitForAtomicDone(done)) {
        testFail("MT WASM broker main stats timeout");
    }
    return snapshot;
}

bool wasmJitTestWaitForMtBrokerPurge(U32 moduleId, U32 memoryId, U32 memoryIncarnation, S32* cachedModules) {
    S32 expected = -1;
    S32 received = 0;
    S32 cached = 0;
    testWasmJitBrokerPurgeBarrier(moduleId, memoryId, memoryIncarnation, &expected, &received, &cached);
    bool result = waitForBrokerDelivery(expected, received);
    if (cachedModules) {
        *cachedModules = __atomic_load_n(&cached, __ATOMIC_ACQUIRE);
    }
    return result;
}

void wasmJitTestSetMtBrokerDeliveryFailure(U32 moduleId, bool enabled) {
    S32 done = 0;
    testWasmJitBrokerSetDeliveryFailure(moduleId, enabled ? 1 : 0, &done);
    if (!waitForAtomicDone(done)) {
        testFail("MT WASM broker delivery failure control timeout");
    }
}

void wasmJitTestSetMtBrokerDeliveryOom(U32 moduleId, bool enabled) {
    S32 done = 0;
    testWasmJitBrokerSetDeliveryOom(moduleId, enabled ? 1 : 0, &done);
    if (!waitForAtomicDone(done)) {
        testFail("MT WASM broker delivery OOM control timeout");
    }
}

void wasmJitTestSetMtBrokerCompileOom(U32 moduleId, bool enabled) {
    testWasmJitBrokerSetCompileOom(moduleId, enabled ? 1 : 0);
}

void testWasmJitMtModuleBrokerTransport() {
    S32 oldEnabled = wasmJitTestSetMtModuleBrokerEnabled(1);
    std::vector<U8> bytes = makeBrokerTestModule();
    KMemory* owner = testContext().memory;
    U32 memoryId = (U32)(uintptr_t)owner;
    U32 memoryIncarnation = wasmJitTestGetMtMemoryIncarnation(owner);
    U32 publishedModuleId = wasmJitTestReserveMtBrokerModule(owner);
    S32 firstSource = 0;
    if (testWasmJitBrokerLookup(publishedModuleId, memoryId, memoryIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1, &firstSource) != 42 ||
            firstSource != BROKER_LOOKUP_LOCAL_COMPILE) {
        testFail("MT WASM broker publisher local compile");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    S32 preloadSent = -1;
    S32 receivedCacheEntries = -1;
    if (!waitForBrokerPublication(publishedModuleId, &preloadSent, &receivedCacheEntries)) {
        testFail("MT WASM broker publication barrier");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    if (preloadSent != 0 || receivedCacheEntries != 0) {
        testFail("MT WASM broker publication records without worker delivery");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    BrokerThreadArgs args = {publishedModuleId, memoryId, memoryIncarnation,
        bytes.data(), (U32)bytes.size()};
    wasmJitTestPrepareMtThreadStart(&args, owner);
    pthread_t thread;
    int createResult = pthread_create(&thread, nullptr, brokerPreloadThread, &args);
    if (createResult) {
        wasmJitTestCancelMtThreadStart(&args);
    }
    if (createResult || pthread_join(thread, nullptr)) {
        testFail("MT WASM broker selected pthread start");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    if (args.value != 42 || args.source != BROKER_LOOKUP_HIT) {
        testFail("MT WASM broker selected pthread first lookup uses preloaded module");
    }

    BrokerThreadArgs unrelatedArgs = {wasmJitTestReserveMtBrokerModule(owner), memoryId, memoryIncarnation,
        bytes.data(), (U32)bytes.size()};
    std::thread unrelated(brokerPreloadThread, &unrelatedArgs);
    unrelated.join();
    if (unrelatedArgs.value != 42 || unrelatedArgs.source != BROKER_LOOKUP_LOCAL_COMPILE) {
        testFail("MT WASM broker unrelated std::thread compiles locally");
    }
    wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
}

void testWasmJitMtStandaloneModuleBroker() {
    S32 oldEnabled = wasmJitTestSetMtModuleBrokerEnabled(1);
    wasmJitTestResetMtBrokerStats();

    testNewInstruction(0);
    U32 address = testContext().codeIp;
    testPushCode8(0x40);
    testPushCode8(0xcd);
    testPushCode8(0x97);
    DecodedOp* op = testContext().cpu->getOp(address, 0);
    startNewJIT(testContext().cpu, address, op);

    int tableIndex = (int)(uintptr_t)op->pfnJitCode;
    if (op->flags2 & OP_FLAG2_WASM_JIT_RELOC_HAZARD) {
        testFail("MT WASM relocation-free standalone block keeps the fast call path");
    }
    WasmJitMtBrokerModuleRef ref = wasmJitTestGetMtBrokerSlotRef(tableIndex);
    WasmJitMtBrokerStatsSnapshot before = wasmJitTestGetMtBrokerStats(testContext().memory);
    if (!op->pfnJitCode || !ref.moduleId || ref.memoryId != (U32)(uintptr_t)testContext().memory ||
            ref.memoryIncarnation != before.memoryIncarnation || before.localCompiles != 1 || before.brokerHits != 0) {
        testFail("MT WASM standalone broker records initial module");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    if (!waitForBrokerPublication(ref.moduleId)) {
        testFail("MT WASM standalone broker publication barrier");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    BrokerLazyInstallArgs args;
    args.firstSlot = tableIndex;
    if (!runPreparedBrokerThread(brokerLazyInstallThread, &args, testContext().memory)) {
        testFail("MT WASM standalone broker selected pthread start");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    WasmJitMtBrokerStatsSnapshot after = wasmJitTestGetMtBrokerStats(testContext().memory);
    if (!args.firstInstalled || after.localCompiles != 1 || after.brokerHits != 1) {
        testFail("MT WASM standalone lazy install reuses broker module");
    }

    wasmJitTestSetMtActiveSlot(testContext().cpu, tableIndex);
    testContext().memory->removeCodeBlock(address, op, false);
    bool activeMetadataRetained = wasmJitTestHasMtSlotMetadata(tableIndex);
    bool activeSlotClear = testWasmJitBrokerSlotIsClear(tableIndex) != 0;
    S32 activeValidation = wasm_jit_mt_broker_validate_module(
        ref.moduleId, ref.memoryId, ref.memoryIncarnation);
    if (op->pfnJitCode || !activeMetadataRetained || activeSlotClear ||
            activeValidation != 1) {
        testFail("MT WASM standalone eviction unpublishes but defers active module reclamation: pfn=%p metadata=%d slotClear=%d validation=%d",
            op->pfnJitCode, activeMetadataRetained, activeSlotClear,
            activeValidation);
    }
    wasmJitTestSetMtActiveSlot(testContext().cpu, -1);
    wasmJitTestReapMtRetiredSlots(testContext().memory);
    bool retiredMetadataRetained = wasmJitTestHasMtSlotMetadata(tableIndex);
    bool retiredSlotClear = testWasmJitBrokerSlotIsClear(tableIndex) != 0;
    S32 retiredValidation = wasm_jit_mt_broker_validate_module(
        ref.moduleId, ref.memoryId, ref.memoryIncarnation);
    if (retiredMetadataRetained || !retiredSlotClear || retiredValidation != 0) {
        testFail("MT WASM standalone quiescence reclaims slot, bytes, relocations, and broker module: metadata=%d slotClear=%d validation=%d",
            retiredMetadataRetained, retiredSlotClear, retiredValidation);
    }

    CPU* resetCpu = CPU::allocCPU(testContext().memory);
    resetCpu->wasmJitActiveTableIndex = (U32)tableIndex;
    resetCpu->wasmJitActiveTableIndexLocal = (U32)tableIndex;
    resetCpu->wasmJitCallsUntilQuiescence = 17;
    resetCpu->wasmJitInCompiledCall = 1;
    resetCpu->wasmJitReapRetiredOnExit = 1;
    resetCpu->reset();
    if (resetCpu->wasmJitActiveTableIndex != (U32)tableIndex ||
            resetCpu->wasmJitActiveTableIndexLocal != (U32)tableIndex ||
            resetCpu->wasmJitCallsUntilQuiescence != 17 ||
            resetCpu->wasmJitInCompiledCall != 1 ||
            resetCpu->wasmJitReapRetiredOnExit != 1) {
        testFail("MT WASM exec reset preserves the unwinding compiled-call hazard");
    }
    resetCpu->wasmJitInCompiledCall = 0;
    resetCpu->wasmJitReapRetiredOnExit = 0;
    resetCpu->reset();
    if (resetCpu->wasmJitActiveTableIndex ||
            resetCpu->wasmJitActiveTableIndexLocal ||
            resetCpu->wasmJitCallsUntilQuiescence) {
        testFail("MT WASM quiescent reset clears a carried owner hazard");
    }
    delete resetCpu;
    wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
}

void testWasmJitMtGroupedModuleBroker() {
    S32 oldEnabled = wasmJitTestSetMtModuleBrokerEnabled(1);
    struct RestoreMtPersistenceTest {
        bool oldActive;

        ~RestoreMtPersistenceTest() {
            wasmJitTestSetMtPersistenceActive(oldActive);
        }
    } restorePersistence{wasmJitTestSetMtPersistenceActive(false)};
    wasmJitTestResetMtBrokerStats();

    std::vector<U8> first = makeBrokerStoreModule(0x11223344);
    std::vector<U8> second = makeBrokerStoreModule(0x55667788);
    std::vector<WasmJitMergeInput> inputs = {{&first}, {&second}};
    std::vector<U8> merged;
    BString error;
    if (!wasmJitMergeModules(inputs, merged, error)) {
        testFail("MT WASM broker group merge setup: %s", error.c_str());
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    int groupIdx = wasm_jit_mt_register_group(merged.data(), (int)merged.size());
    wasm_jit_mt_register_group_entry(groupIdx, 0x1000, 0x2000, "b0", 0);
    wasm_jit_mt_register_group_entry(groupIdx, 0x1004, 0x2001, "b1", 0);
    int firstSlot = wasmJitTestInstantiateMtGroupEntry((U32)groupIdx, 0, testContext().memory);
    int secondSlot = wasmJitTestInstantiateMtGroupEntry((U32)groupIdx, 1, testContext().memory);
    U32 moduleId = wasmJitTestGetMtGroupModuleId((U32)groupIdx, testContext().memory);
    WasmJitMtBrokerStatsSnapshot initial = wasmJitTestGetMtBrokerStats(testContext().memory);
    if (firstSlot < 0 || secondSlot < 0 || !moduleId || initial.localCompiles != 1 || initial.brokerHits != 0) {
        testFail("MT WASM broker group creates one module and one instance");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    if (!waitForBrokerPublication(moduleId)) {
        testFail("MT WASM broker group publication barrier");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    BrokerLazyInstallArgs args;
    args.firstSlot = firstSlot;
    args.secondSlot = secondSlot;
    if (!runPreparedBrokerThread(brokerLazyInstallThread, &args, testContext().memory)) {
        testFail("MT WASM broker group selected pthread start");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    WasmJitMtBrokerStatsSnapshot finalStats = wasmJitTestGetMtBrokerStats(testContext().memory);
    if (!args.firstInstalled || !args.secondInstalled || args.instanceCount != 1 ||
            finalStats.localCompiles != 1 || finalStats.brokerHits != 1) {
        testFail("MT WASM broker group consumer reuses one module and instance");
    }

    U32 constructionCount = wasmJitTestMtRuntimeGroupConstructionCount();
    U64 retainedRuntimeBytesBefore = wasmJitTestMtRuntimeGroupRetainedBytes();
    BrokerGroupWorkerStats beforeRuntimeStats = getBrokerGroupWorkerStats();
    std::vector<S32> runtimeSlots;
    if (!wasmJitTestInstallMtRuntimeGroup(merged, 2, testContext().memory, runtimeSlots) ||
            runtimeSlots.size() != 2 || runtimeSlots[0] < 0 || runtimeSlots[1] < 0) {
        testFail("MT WASM online broker group production install");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    WasmJitMtBrokerModuleRef runtimeRef = wasmJitTestGetMtBrokerSlotRef(runtimeSlots[0]);
    WasmJitMtBrokerModuleRef runtimeSecondRef = wasmJitTestGetMtBrokerSlotRef(runtimeSlots[1]);
    BrokerGroupWorkerStats afterRuntimeStats = getBrokerGroupWorkerStats();
    if (!runtimeRef.moduleId || runtimeRef.moduleId != runtimeSecondRef.moduleId ||
            runtimeRef.memoryId != (U32)(uintptr_t)testContext().memory ||
            runtimeRef.memoryIncarnation != runtimeSecondRef.memoryIncarnation ||
            wasmJitTestMtRuntimeGroupConstructionCount() != constructionCount + 1) {
        testFail("MT WASM online broker group records one exact grouped module construction");
    }
    if (afterRuntimeStats.groupInstanceCreations != beforeRuntimeStats.groupInstanceCreations + 1 ||
            afterRuntimeStats.groupInstanceReuses != beforeRuntimeStats.groupInstanceReuses ||
            afterRuntimeStats.groupedLocalCompiles != beforeRuntimeStats.groupedLocalCompiles + 1 ||
            afterRuntimeStats.groupedBrokerHits != beforeRuntimeStats.groupedBrokerHits) {
        testFail("MT WASM online broker group records one local construction in worker counters");
    }
    if (!waitForBrokerPublication(runtimeRef.moduleId)) {
        testFail("MT WASM online broker group publication barrier");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    BrokerLazyInstallArgs runtimeArgs;
    runtimeArgs.firstSlot = runtimeSlots[0];
    runtimeArgs.secondSlot = runtimeSlots[1];
    runtimeArgs.callSlots = true;
    WasmJitMtBrokerStatsSnapshot beforeRuntimeWorker =
        wasmJitTestGetMtBrokerStats(testContext().memory);
    if (!runPreparedBrokerThread(brokerLazyInstallThread, &runtimeArgs, testContext().memory)) {
        testFail("MT WASM online broker group selected pthread start");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    WasmJitMtBrokerStatsSnapshot afterRuntimeWorker =
        wasmJitTestGetMtBrokerStats(testContext().memory);
    if (!runtimeArgs.firstInstalled || !runtimeArgs.secondInstalled ||
            runtimeArgs.instanceCount != 1 ||
            runtimeArgs.firstValue != 0x11223344 ||
            runtimeArgs.secondValue != 0x55667788 ||
            afterRuntimeWorker.localCompiles != beforeRuntimeWorker.localCompiles ||
            afterRuntimeWorker.brokerHits != beforeRuntimeWorker.brokerHits + 1) {
        testFail("MT WASM online broker group consumer reuses one preloaded instance with distinct exports");
    }
    if (runtimeArgs.stats.groupInstanceCreations != 1 ||
            runtimeArgs.stats.groupInstanceReuses != 1 ||
            runtimeArgs.stats.groupedBrokerHits != 1 ||
            runtimeArgs.stats.groupedLocalCompiles != 0) {
        testFail("MT WASM online broker group records one broker hit and export reuse");
    }

    bool runtimeInstancePresent = testWasmJitBrokerHasGroupInstance(
        runtimeRef.moduleId, 0, 0, runtimeRef.memoryId,
        runtimeRef.memoryIncarnation) != 0;
    // CLONE_VM can expose the same decoded entries through a CPU which is not
    // in the KProcess that originally owned the compiled slot. Quiescence
    // must observe that CPU too, rather than scanning only the owner process.
    CPU* detachedActiveCpu = CPU::allocCPU(testContext().memory);
    wasmJitTestSetMtActiveSlot(detachedActiveCpu, runtimeSlots[0]);
    wasmJitTestRetireMtSlot(runtimeSlots[1]);
    bool activeRuntimeMetadataRetained =
        wasmJitTestHasMtSlotMetadata(runtimeSlots[1]);
    bool activeRuntimeSlotClear =
        testWasmJitBrokerSlotIsClear(runtimeSlots[1]) != 0;
    if (!runtimeInstancePresent || !activeRuntimeMetadataRetained ||
            activeRuntimeSlotClear) {
        testFail("MT WASM grouped block eviction defers sibling slots owned by the active incarnation: instance=%d metadata=%d slotClear=%d",
            runtimeInstancePresent, activeRuntimeMetadataRetained,
            activeRuntimeSlotClear);
    }
    wasmJitTestSetMtActiveSlot(detachedActiveCpu, -1);
    delete detachedActiveCpu;
    wasmJitTestReapMtRetiredSlots(testContext().memory);
    bool firstRuntimeMetadataRetained =
        wasmJitTestHasMtSlotMetadata(runtimeSlots[1]);
    bool firstRuntimeSlotClear =
        testWasmJitBrokerSlotIsClear(runtimeSlots[1]) != 0;
    runtimeInstancePresent = testWasmJitBrokerHasGroupInstance(
        runtimeRef.moduleId, 0, 0, runtimeRef.memoryId,
        runtimeRef.memoryIncarnation) != 0;
    if (firstRuntimeMetadataRetained || !firstRuntimeSlotClear ||
            !runtimeInstancePresent) {
        testFail("MT WASM grouped block quiescence reclaims one slot but retains the shared instance: instance=%d metadata=%d slotClear=%d",
            runtimeInstancePresent, firstRuntimeMetadataRetained,
            firstRuntimeSlotClear);
    }
    wasmJitTestRetireMtSlot(runtimeSlots[0]);
    wasmJitTestReapMtRetiredSlots(testContext().memory);
    bool secondRuntimeMetadataRetained =
        wasmJitTestHasMtSlotMetadata(runtimeSlots[0]);
    bool secondRuntimeSlotClear =
        testWasmJitBrokerSlotIsClear(runtimeSlots[0]) != 0;
    runtimeInstancePresent = testWasmJitBrokerHasGroupInstance(
        runtimeRef.moduleId, 0, 0, runtimeRef.memoryId,
        runtimeRef.memoryIncarnation) != 0;
    if (secondRuntimeMetadataRetained || !secondRuntimeSlotClear ||
            runtimeInstancePresent) {
        testFail("MT WASM grouped final block quiescence reclaims the group instance: instance=%d metadata=%d slotClear=%d",
            runtimeInstancePresent, secondRuntimeMetadataRetained,
            secondRuntimeSlotClear);
    }
    U64 retainedRuntimeBytesAfter = wasmJitTestMtRuntimeGroupRetainedBytes();
    if (retainedRuntimeBytesAfter != retainedRuntimeBytesBefore) {
        testFail("MT WASM grouped final block quiescence releases retained runtime module bytes: before=%llu after=%llu",
            retainedRuntimeBytesBefore, retainedRuntimeBytesAfter);
    }

    int fallbackGroupIdx = wasm_jit_mt_register_group(merged.data(), (int)merged.size());
    wasm_jit_mt_register_group_entry(fallbackGroupIdx, 0x2000, 0x3000, "b0", 1);
    wasm_jit_mt_register_group_entry(fallbackGroupIdx, 0x2004, 0x3001, "b1", 1);
    MtNextModuleIdRestorer nextModuleIdRestorer{wasmJitTestSetMtNextModuleId(0)};
    U32 beforeFallbackInstances = wasmJitTestCurrentWorkerRuntimeGroupInstanceCount();
    BrokerGroupWorkerStats beforeFallbackStats = getBrokerGroupWorkerStats();
    U32 firstFallbackIncarnation = wasmJitTestGetMtMemoryIncarnation(testContext().memory);
    int firstFallbackSlot =
        wasmJitTestInstantiateMtGroupEntry((U32)fallbackGroupIdx, 0, testContext().memory);
    constexpr U32 staleRelocSentinel = 0x7f4a39c1;
    if (firstFallbackSlot < 0 ||
            !wasmJitTestSetMtGroupRelocValue(firstFallbackSlot, 0, staleRelocSentinel)) {
        testFail("MT WASM grouped fallback relocation setup");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    WasmJitMtBrokerModuleRef firstFallbackRef =
        wasmJitTestGetMtBrokerSlotRef(firstFallbackSlot);
    U32 afterFirstFallbackInstances = wasmJitTestCurrentWorkerRuntimeGroupInstanceCount();
    wasmJitTestSetMtActiveSlot(testContext().cpu, firstFallbackSlot);
    wasmJitTestInvalidateMtBrokerMemory(testContext().memory);
    bool oldFallbackMetadataRetained = wasmJitTestHasMtSlotMetadata(firstFallbackSlot);
    bool oldFallbackSlotClear = testWasmJitBrokerSlotIsClear(firstFallbackSlot) != 0;
    bool oldFallbackInstanceRetained = testWasmJitBrokerHasGroupInstance(
        0, 0, wasmJitTestGetMtGroupInstanceIdentity((U32)fallbackGroupIdx),
        (U32)(uintptr_t)testContext().memory, firstFallbackIncarnation) != 0;
    if (!oldFallbackMetadataRetained || oldFallbackSlotClear || !oldFallbackInstanceRetained) {
        testFail("MT WASM grouped invalidation defers active owner state: metadata=%d slotClear=%d instance=%d",
            oldFallbackMetadataRetained, oldFallbackSlotClear, oldFallbackInstanceRetained);
    }
    U32 secondFallbackIncarnation = wasmJitTestGetMtMemoryIncarnation(testContext().memory);
    int replacementFallbackSlot =
        wasmJitTestInstantiateMtGroupEntry((U32)fallbackGroupIdx, 0, testContext().memory);
    U32 replacementRelocValue =
        wasmJitTestGetMtGroupRelocValue(replacementFallbackSlot, 0);
    int secondFallbackSlot =
        wasmJitTestInstantiateMtGroupEntry((U32)fallbackGroupIdx, 1, testContext().memory);
    WasmJitMtBrokerModuleRef secondFallbackRef =
        wasmJitTestGetMtBrokerSlotRef(secondFallbackSlot);
    bool replacementFallbackInstancePresent = testWasmJitBrokerHasGroupInstance(
        0, 0, wasmJitTestGetMtGroupInstanceIdentity((U32)fallbackGroupIdx),
        (U32)(uintptr_t)testContext().memory, secondFallbackIncarnation) != 0;
    BrokerGroupWorkerStats afterFallbackStats = getBrokerGroupWorkerStats();
    if (firstFallbackSlot < 0 || replacementFallbackSlot < 0 || secondFallbackSlot < 0 ||
            firstFallbackRef.moduleId || secondFallbackRef.moduleId ||
            firstFallbackRef.memoryIncarnation != firstFallbackIncarnation ||
            secondFallbackRef.memoryIncarnation != secondFallbackIncarnation ||
            firstFallbackIncarnation == secondFallbackIncarnation ||
            afterFirstFallbackInstances != beforeFallbackInstances + 1 ||
            !replacementFallbackInstancePresent) {
        testFail("MT WASM grouped fallback identity separates owner incarnations");
    }
    if (replacementRelocValue != 0) {
        testFail("MT WASM grouped fallback replacement incarnation starts with zero relocations: value=%x expected=0",
            replacementRelocValue);
    }
    wasmJitTestSetMtActiveSlot(testContext().cpu, -1);
    wasmJitTestReapMtRetiredSlots(testContext().memory);
    oldFallbackMetadataRetained = wasmJitTestHasMtSlotMetadata(firstFallbackSlot);
    oldFallbackSlotClear = testWasmJitBrokerSlotIsClear(firstFallbackSlot) != 0;
    oldFallbackInstanceRetained = testWasmJitBrokerHasGroupInstance(
        0, 0, wasmJitTestGetMtGroupInstanceIdentity((U32)fallbackGroupIdx),
        (U32)(uintptr_t)testContext().memory, firstFallbackIncarnation) != 0;
    if (oldFallbackMetadataRetained || !oldFallbackSlotClear || oldFallbackInstanceRetained) {
        testFail("MT WASM grouped quiescence reclaims deferred owner state: metadata=%d slotClear=%d instance=%d",
            oldFallbackMetadataRetained, oldFallbackSlotClear, oldFallbackInstanceRetained);
    }
    if (afterFallbackStats.groupInstanceCreations !=
            beforeFallbackStats.groupInstanceCreations + 2 ||
            afterFallbackStats.groupInstanceReuses != beforeFallbackStats.groupInstanceReuses + 1 ||
            afterFallbackStats.groupedLocalCompiles !=
                beforeFallbackStats.groupedLocalCompiles + 2 ||
            afterFallbackStats.groupedBrokerHits != beforeFallbackStats.groupedBrokerHits) {
        testFail("MT WASM grouped fallback records two incarnation-local constructions");
    }

    BrokerLazyInstallArgs fallbackArgs;
    fallbackArgs.firstSlot = firstFallbackSlot;
    fallbackArgs.secondSlot = secondFallbackSlot;
    fallbackArgs.callSlots = true;
    fallbackArgs.inspectFallbackKeys = true;
    fallbackArgs.groupIdentity =
        wasmJitTestGetMtGroupInstanceIdentity((U32)fallbackGroupIdx);
    fallbackArgs.memoryId = (U32)(uintptr_t)testContext().memory;
    fallbackArgs.firstMemoryIncarnation = firstFallbackIncarnation;
    fallbackArgs.secondMemoryIncarnation = secondFallbackIncarnation;
    if (!runPreparedBrokerThread(brokerLazyInstallThread, &fallbackArgs, testContext().memory)) {
        testFail("MT WASM grouped fallback exact-slot selected pthread start");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    if (fallbackArgs.firstInstalled || !fallbackArgs.secondInstalled ||
            fallbackArgs.oldKeyAfterFirst || fallbackArgs.newKeyAfterFirst ||
            fallbackArgs.oldKeyAfterSecond || !fallbackArgs.newKeyAfterSecond ||
            fallbackArgs.instanceCount != 1 ||
            fallbackArgs.firstValue != 0 ||
            fallbackArgs.secondValue != 0x55667788) {
        testFail("MT WASM grouped fallback lazy install rejects the retired owner and installs the replacement");
    }
    if (fallbackArgs.stats.groupInstanceCreations != 1 ||
            fallbackArgs.stats.groupInstanceReuses != 0 ||
            fallbackArgs.stats.groupedBrokerHits != 0 ||
            fallbackArgs.stats.groupedLocalCompiles != 1) {
        testFail("MT WASM grouped fallback lazy install records only the replacement cache identity");
    }
    wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
}

void testWasmJitMtModuleBrokerLifecycle() {
    S32 queryDisabled = 0;
    S32 queryDone = 0;
    testWasmJitBrokerCopyQueryDisabled(&queryDisabled, &queryDone);
    S32 configuredEnabled = wasmJitTestSetMtModuleBrokerEnabled(0);
    if (!waitForAtomicDone(queryDone)) {
        testFail("MT WASM broker query state timeout");
    } else if (configuredEnabled != (queryDisabled ? 0 : 1)) {
        testFail("MT WASM broker query disable latched before guest execution");
    }

    wasmJitTestResetMtBrokerStats();
    testNewInstruction(0);
    U32 disabledAddress = testContext().codeIp;
    testPushCode8(0x40);
    testPushCode8(0xcd);
    testPushCode8(0x97);
    DecodedOp* disabledOp = testContext().cpu->getOp(disabledAddress, 0);
    startNewJIT(testContext().cpu, disabledAddress, disabledOp);
    int disabledTableIndex = (int)(uintptr_t)disabledOp->pfnJitCode;
    WasmJitMtBrokerModuleRef disabledRef = wasmJitTestGetMtBrokerSlotRef(disabledTableIndex);
    WasmJitMtBrokerStatsSnapshot disabledStats = wasmJitTestGetMtBrokerStats(testContext().memory);
    if (!disabledOp->pfnJitCode || disabledRef.moduleId != 0 ||
            disabledRef.memoryId != (U32)(uintptr_t)testContext().memory ||
            disabledStats.localCompiles != 1 || disabledStats.brokerHits != 0) {
        testFail("MT WASM broker production instantiate honors shared disable flag");
    }

    wasmJitTestSetMtModuleBrokerEnabled(1);
    WasmJitMtBrokerMainStatsSnapshot baseline = wasmJitTestGetMtBrokerMainStats();

    std::vector<U8> bytes = makeBrokerTestModule();
    KMemory* owner = reinterpret_cast<KMemory*>(0x7fff2000);
    U32 moduleId = wasmJitTestReserveMtBrokerModule(owner);
    U32 memoryId = (U32)(uintptr_t)owner;
    U32 memoryIncarnation = wasmJitTestGetMtMemoryIncarnation(owner);
    S32 source = 0;
    if (testWasmJitBrokerLookup(moduleId, memoryId, memoryIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1, &source) != 42) {
        testFail("MT WASM broker lifecycle publication setup");
        wasmJitTestSetMtModuleBrokerEnabled(configuredEnabled);
        return;
    }
    if (!wasmJitTestWaitForMtBrokerDelivery(moduleId)) {
        testFail("MT WASM broker lifecycle delivery setup");
        wasmJitTestSetMtModuleBrokerEnabled(configuredEnabled);
        return;
    }

    testWasmJitBrokerPublishRaw(moduleId, memoryId, memoryIncarnation,
        bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1);
    testWasmJitBrokerPublishRaw(moduleId, memoryId + 4, memoryIncarnation,
        bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1);
    testWasmJitBrokerPublishRaw(0xfffffff0, memoryId, memoryIncarnation,
        bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1);
    WasmJitMtBrokerMainStatsSnapshot raced = wasmJitTestGetMtBrokerMainStats();
    if (raced.firstPublications != baseline.firstPublications + 1 || raced.duplicatePublications != baseline.duplicatePublications + 1 || raced.ownershipErrors != baseline.ownershipErrors + 1 || raced.unknownModuleIds != baseline.unknownModuleIds + 1 || raced.liveRegistryModules != baseline.liveRegistryModules + 1) {
        testFail("MT WASM broker duplicate and ownership accounting");
    }

    wasmJitTestInvalidateMtBrokerMemoryId(memoryId);
    S32 cachedAfterPurge = -1;
    if (!wasmJitTestWaitForMtBrokerPurge(moduleId, memoryId, memoryIncarnation, &cachedAfterPurge)) {
        testFail("MT WASM broker memory purge barrier timeout");
    } else if (cachedAfterPurge != 0) {
        testFail("MT WASM broker memory purge removes exact worker cache");
    }
    WasmJitMtBrokerMainStatsSnapshot purged = wasmJitTestGetMtBrokerMainStats();
    if (purged.purgedModules != raced.purgedModules + 1 || purged.liveRegistryModules != baseline.liveRegistryModules) {
        testFail("MT WASM broker memory purge accounting");
    }

    U32 failureId = wasmJitTestReserveMtBrokerModule(owner);
    U32 failureIncarnation = wasmJitTestGetMtMemoryIncarnation(owner);
    wasmJitTestSetMtBrokerDeliveryFailure(failureId, true);
    if (testWasmJitBrokerLookup(failureId, memoryId, failureIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1, &source) != 42 ||
            source != BROKER_LOOKUP_LOCAL_COMPILE) {
        testFail("MT WASM broker delivery failure publisher continues");
    }
    if (!waitForBrokerPublication(failureId)) {
        testFail("MT WASM broker delivery failure publication setup");
    }
    BrokerThreadArgs fallbackArgs = {failureId, memoryId, failureIncarnation,
        bytes.data(), (U32)bytes.size()};
    if (!runPreparedBrokerThread(brokerPreloadThread, &fallbackArgs, owner)) {
        testFail("MT WASM broker delivery failure selected pthread start");
    }
    WasmJitMtBrokerMainStatsSnapshot failedDelivery = wasmJitTestGetMtBrokerMainStats();
    wasmJitTestSetMtBrokerDeliveryFailure(failureId, false);
    if (fallbackArgs.value != 42 || fallbackArgs.source != BROKER_LOOKUP_LOCAL_COMPILE ||
            failedDelivery.deliveryFailures == purged.deliveryFailures) {
        testFail("MT WASM broker delivery failure falls back to local compile");
    }

    KMemory* raceOwner = reinterpret_cast<KMemory*>(0x7fff3000);
    U32 raceMemoryId = (U32)(uintptr_t)raceOwner;
    U32 raceModuleId = wasmJitTestReserveMtBrokerModule(raceOwner);
    U32 raceMemoryIncarnation = wasmJitTestGetMtMemoryIncarnation(raceOwner);
    S32 raceValue = 0;
    S32 raceSource = 0;
    std::thread delayedPublisher([&]() {
        testWasmJitBrokerSetPublicationDelayed(raceModuleId, 1);
        raceValue = testWasmJitBrokerLookup(raceModuleId, raceMemoryId, raceMemoryIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1, &raceSource);
        testWasmJitBrokerSetPublicationDelayed(raceModuleId, 0);
    });
    delayedPublisher.join();
    if (raceValue != 42 || raceSource != BROKER_LOOKUP_LOCAL_COMPILE) {
        testFail("MT WASM broker delayed publication setup");
    }
    WasmJitMtBrokerMainStatsSnapshot beforeLatePublication = wasmJitTestGetMtBrokerMainStats();
    wasmJitTestInvalidateMtBrokerMemoryId(raceMemoryId);
    testWasmJitBrokerPublishRaw(raceModuleId, raceMemoryId, raceMemoryIncarnation,
        bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1);
    WasmJitMtBrokerMainStatsSnapshot afterLatePublication = wasmJitTestGetMtBrokerMainStats();
    if (testWasmJitBrokerHasLocalModule(raceModuleId, raceMemoryId, raceMemoryIncarnation) ||
            afterLatePublication.purgedModules != beforeLatePublication.purgedModules ||
            afterLatePublication.liveRegistryModules != beforeLatePublication.liveRegistryModules ||
            afterLatePublication.unknownModuleIds != beforeLatePublication.unknownModuleIds) {
        testFail("MT WASM broker publication race rejects retired-incarnation module");
    }

    KMemory* reusedOwner = reinterpret_cast<KMemory*>(0x7fff4000);
    U32 reusedMemoryId = (U32)(uintptr_t)reusedOwner;
    U32 oldGenerationId = wasmJitTestReserveMtBrokerModule(reusedOwner);
    U32 oldGenerationIncarnation = wasmJitTestGetMtMemoryIncarnation(reusedOwner);
    if (testWasmJitBrokerLookup(oldGenerationId, reusedMemoryId, oldGenerationIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1, &source) != 42 ||
            !wasmJitTestWaitForMtBrokerDelivery(oldGenerationId)) {
        testFail("MT WASM broker pointer reuse old generation setup");
    }
    wasmJitTestInvalidateMtBrokerMemoryId(reusedMemoryId);
    S32 cachedOldGeneration = -1;
    if (!wasmJitTestWaitForMtBrokerPurge(oldGenerationId, reusedMemoryId, oldGenerationIncarnation,
            &cachedOldGeneration) || cachedOldGeneration != 0) {
        testFail("MT WASM broker pointer reuse old generation purge");
    }

    U32 newGenerationId = wasmJitTestReserveMtBrokerModule(reusedOwner);
    U32 newGenerationIncarnation = wasmJitTestGetMtMemoryIncarnation(reusedOwner);
    if (testWasmJitBrokerLookup(newGenerationId, reusedMemoryId, newGenerationIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1, &source) != 42 ||
            !wasmJitTestWaitForMtBrokerDelivery(newGenerationId)) {
        testFail("MT WASM broker pointer reuse new generation setup");
    }
    WasmJitMtBrokerMainStatsSnapshot beforeDelayedOldPurge = wasmJitTestGetMtBrokerMainStats();
    testWasmJitBrokerReleaseRaw(oldGenerationId, reusedMemoryId, oldGenerationIncarnation);
    S32 cachedOldAfterDelay = -1;
    if (!wasmJitTestWaitForMtBrokerPurge(oldGenerationId, reusedMemoryId, oldGenerationIncarnation,
            &cachedOldAfterDelay) || cachedOldAfterDelay != 0) {
        testFail("MT WASM broker delayed old-generation purge barrier");
    }
    if (!testWasmJitBrokerHasLocalModule(newGenerationId, reusedMemoryId, newGenerationIncarnation)) {
        testFail("MT WASM broker exact ID preserves pointer-reused worker cache");
    }
    WasmJitMtBrokerMainStatsSnapshot afterDelayedOldPurge = wasmJitTestGetMtBrokerMainStats();
    if (afterDelayedOldPurge.purgedModules != beforeDelayedOldPurge.purgedModules ||
            afterDelayedOldPurge.liveRegistryModules != beforeDelayedOldPurge.liveRegistryModules) {
        testFail("MT WASM broker delayed old-generation purge preserves new main module");
    }

    KMemory* protectedOwner = reinterpret_cast<KMemory*>(0x7fff5000);
    U32 protectedMemoryId = (U32)(uintptr_t)protectedOwner;
    U32 protectedModuleId = wasmJitTestReserveMtBrokerModule(protectedOwner);
    U32 protectedMemoryIncarnation = wasmJitTestGetMtMemoryIncarnation(protectedOwner);
    if (testWasmJitBrokerLookup(protectedModuleId, protectedMemoryId, protectedMemoryIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1, &source) != 42 ||
            !wasmJitTestWaitForMtBrokerDelivery(protectedModuleId)) {
        testFail("MT WASM broker owner-aware purge setup");
    }
    WasmJitMtBrokerMainStatsSnapshot beforeWrongOwnerPurge = wasmJitTestGetMtBrokerMainStats();
    testWasmJitBrokerReleaseRaw(protectedModuleId, reusedMemoryId, newGenerationIncarnation);
    if (!testWasmJitBrokerHasLocalModule(protectedModuleId, protectedMemoryId, protectedMemoryIncarnation)) {
        testFail("MT WASM broker wrong-owner purge preserves worker cache");
    }
    WasmJitMtBrokerMainStatsSnapshot afterWrongOwnerPurge = wasmJitTestGetMtBrokerMainStats();
    if (afterWrongOwnerPurge.purgedModules != beforeWrongOwnerPurge.purgedModules ||
            afterWrongOwnerPurge.liveRegistryModules != beforeWrongOwnerPurge.liveRegistryModules) {
        testFail("MT WASM broker wrong-owner purge preserves main module");
    }

    wasmJitTestInvalidateMtBrokerMemoryId(reusedMemoryId);
    S32 cachedNewAfterCleanup = -1;
    if (!wasmJitTestWaitForMtBrokerPurge(newGenerationId, reusedMemoryId, newGenerationIncarnation,
            &cachedNewAfterCleanup) || cachedNewAfterCleanup != 0) {
        testFail("MT WASM broker pointer reuse cleanup");
    }
    wasmJitTestInvalidateMtBrokerMemoryId(protectedMemoryId);
    S32 cachedAfterProtectedCleanup = -1;
    if (!wasmJitTestWaitForMtBrokerPurge(protectedModuleId, protectedMemoryId, protectedMemoryIncarnation,
            &cachedAfterProtectedCleanup) ||
            cachedAfterProtectedCleanup != 0) {
        testFail("MT WASM broker owner-aware purge cleanup");
    }

    wasmJitTestInvalidateMtBrokerMemoryId(memoryId);
    S32 cachedAfterFailureCleanup = -1;
    if (!wasmJitTestWaitForMtBrokerPurge(failureId, memoryId, failureIncarnation,
            &cachedAfterFailureCleanup) || cachedAfterFailureCleanup != 0) {
        testFail("MT WASM broker failure-path purge cleanup");
    }
    wasmJitTestSetMtModuleBrokerEnabled(configuredEnabled);
}

void testWasmJitMtModuleBrokerPreloadSelection() {
    S32 oldEnabled = wasmJitTestSetMtModuleBrokerEnabled(1);
    std::vector<U8> bytes = makeBrokerTestModule();
    KMemory* owner = testContext().memory;
    U32 memoryId = (U32)(uintptr_t)owner;

    U32 oldIncarnationModuleId = wasmJitTestReserveMtBrokerModule(owner);
    U32 oldIncarnation = wasmJitTestGetMtMemoryIncarnation(owner);
    wasmJitTestInvalidateMtBrokerMemoryId(memoryId);

    U32 memoryIncarnation = wasmJitTestGetMtMemoryIncarnation(owner);
    U32 grouped16Older = wasmJitTestReserveMtBrokerModule(owner);
    U32 grouped16Newer = wasmJitTestReserveMtBrokerModule(owner);
    U32 grouped8 = wasmJitTestReserveMtBrokerModule(owner);
    U32 grouped3Older = wasmJitTestReserveMtBrokerModule(owner);
    U32 grouped1Newer = wasmJitTestReserveMtBrokerModule(owner);
    U32 grouped32Held = wasmJitTestReserveMtBrokerModule(owner);
    U32 duplicate3 = wasmJitTestReserveMtBrokerModule(owner);
    U32 duplicate2Older = wasmJitTestReserveMtBrokerModule(owner);
    U32 duplicate2Newer = wasmJitTestReserveMtBrokerModule(owner);
    std::vector<U8> firstRuntime = makeBrokerStoreModule(0x11223344);
    std::vector<U8> secondRuntime = makeBrokerStoreModule(0x55667788);
    S32 grouped3Source = 0;
    if (testWasmJitBrokerLookup(grouped3Older, memoryId, memoryIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_GROUPED, 3,
            &grouped3Source) != 42 ||
            grouped3Source != BROKER_LOOKUP_LOCAL_COMPILE ||
            !waitForBrokerPublication(grouped3Older)) {
        testFail("MT WASM broker preload exact online group lower-bound setup");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    std::vector<WasmJitMergeInput> runtimeInputs = {{&firstRuntime}, {&secondRuntime}};
    std::vector<U8> runtimeBytes;
    BString runtimeError;
    if (!wasmJitMergeModules(runtimeInputs, runtimeBytes, runtimeError)) {
        testFail("MT WASM broker preload online group merge setup: %s", runtimeError.c_str());
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    std::vector<S32> runtimeSlots;
    if (!wasmJitTestInstallMtRuntimeGroup(runtimeBytes, 2, owner, runtimeSlots) ||
            runtimeSlots.size() != 2) {
        testFail("MT WASM broker preload online group install setup");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    U32 onlineGrouped2 = wasmJitTestGetMtBrokerSlotRef(runtimeSlots[0]).moduleId;
    if (!onlineGrouped2 || !waitForBrokerPublication(onlineGrouped2)) {
        testFail("MT WASM broker preload online group publication setup");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    std::vector<U32> ordinaryIds;
    ordinaryIds.reserve(63);
    for (U32 i = 0; i < 63; i++) {
        ordinaryIds.push_back(wasmJitTestReserveMtBrokerModule(owner));
    }

    auto publishModule = [&](U32 moduleId, U32 moduleClass, U32 representedBlockCount) {
        S32 lookupSource = 0;
        return testWasmJitBrokerLookup(moduleId, memoryId, memoryIncarnation,
            bytes.data(), (U32)bytes.size(), moduleClass, representedBlockCount, &lookupSource) == 42 &&
            lookupSource == BROKER_LOOKUP_LOCAL_COMPILE;
    };
    if (!publishModule(grouped16Older, BROKER_MODULE_CLASS_GROUPED, 16) ||
            !publishModule(grouped16Newer, BROKER_MODULE_CLASS_GROUPED, 16) ||
            !publishModule(grouped8, BROKER_MODULE_CLASS_GROUPED, 8) ||
            !publishModule(grouped1Newer, BROKER_MODULE_CLASS_GROUPED, 1) ||
            !publishModule(grouped32Held, BROKER_MODULE_CLASS_GROUPED, 32) ||
            !publishModule(duplicate3, BROKER_MODULE_CLASS_STANDALONE, 1) ||
            !publishModule(duplicate2Older, BROKER_MODULE_CLASS_STANDALONE, 1) ||
            !publishModule(duplicate2Newer, BROKER_MODULE_CLASS_STANDALONE, 1)) {
        testFail("MT WASM broker preload priority publication setup");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    for (U32 moduleId : ordinaryIds) {
        if (!publishModule(moduleId, BROKER_MODULE_CLASS_STANDALONE, 1)) {
            testFail("MT WASM broker preload ordinary publication setup");
            wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
            return;
        }
    }
    for (U32 i = 0; i < 3; i++) {
        testWasmJitBrokerPublishRaw(duplicate3, memoryId, memoryIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1);
    }
    for (U32 i = 0; i < 2; i++) {
        testWasmJitBrokerPublishRaw(duplicate2Older, memoryId, memoryIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1);
    }
    for (U32 i = 0; i < 2; i++) {
        testWasmJitBrokerPublishRaw(duplicate2Newer, memoryId, memoryIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1);
    }

    BrokerTestProcessOwner otherProcess;
    U32 otherProcessId = otherProcess.processId;
    KMemory* otherOwner = otherProcess.memory;
    U32 otherModuleId = wasmJitTestReserveMtBrokerModule(otherOwner);
    U32 otherMemoryId = (U32)(uintptr_t)otherOwner;
    U32 otherIncarnation = wasmJitTestGetMtMemoryIncarnation(otherOwner);
    S32 otherSource = 0;
    if (testWasmJitBrokerLookup(otherModuleId, otherMemoryId, otherIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_GROUPED, 32, &otherSource) != 42 ||
            !waitForBrokerPublication(otherModuleId)) {
        testFail("MT WASM broker preload other-owner setup");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    if (!setTestPreloadCandidate(grouped16Older, oldIncarnationModuleId, memoryId,
            oldIncarnation, BROKER_MODULE_CLASS_GROUPED, 64, 0, true)) {
        testFail("MT WASM broker preload stale-incarnation injection");
        wasmJitTestInvalidateMtBrokerMemoryId(memoryId);
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    std::vector<U32> heldIds = {grouped32Held};
    std::vector<U32> sentIds(BROKER_PRELOAD_LIMIT, 0);
    S32 sentCount = -1;
    S32 done = 0;
    testWasmJitBrokerSelectPreload(memoryId, memoryIncarnation,
        heldIds.data(), (U32)heldIds.size(), sentIds.data(), (U32)sentIds.size(), &sentCount, &done);
    if (!waitForAtomicDone(done)) {
        testFail("MT WASM broker preload selection timeout");
        setTestPreloadCandidate(0, oldIncarnationModuleId, memoryId, oldIncarnation,
            0, 0, 0, false);
        wasmJitTestInvalidateMtBrokerMemoryId(memoryId);
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    if (sentCount != BROKER_PRELOAD_LIMIT) {
        testFail("MT WASM broker preload selection sends exact bounded count");
        setTestPreloadCandidate(0, oldIncarnationModuleId, memoryId, oldIncarnation,
            0, 0, 0, false);
        wasmJitTestInvalidateMtBrokerMemoryId(memoryId);
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    if (sentIds[0] != grouped16Newer || sentIds[1] != grouped16Older ||
            sentIds[2] != grouped8 || sentIds[3] != grouped3Older ||
            sentIds[4] != onlineGrouped2 || sentIds[5] != grouped1Newer) {
        testFail("MT WASM broker preload grouped density brackets online group at exactly two blocks");
    }
    if (sentIds[6] != duplicate3 || sentIds[7] != duplicate2Newer ||
            sentIds[8] != duplicate2Older) {
        testFail("MT WASM broker preload duplicate count and recency order");
    }
    if (sentIds[9] != ordinaryIds.back()) {
        testFail("MT WASM broker preload remaining standalone recency order");
    }
    if (std::find(sentIds.begin(), sentIds.end(), grouped32Held) != sentIds.end()) {
        testFail("MT WASM broker preload held grouped module is skipped");
    }
    if (std::find(sentIds.begin(), sentIds.end(), otherModuleId) != sentIds.end() ||
            std::find(sentIds.begin(), sentIds.end(), oldIncarnationModuleId) != sentIds.end()) {
        testFail("MT WASM broker preload selection excludes other owners and stale incarnations");
    }

    if (!setTestPreloadCandidate(0, oldIncarnationModuleId, memoryId, oldIncarnation,
            0, 0, 0, false)) {
        testFail("MT WASM broker preload stale-incarnation cleanup");
    }
    wasmJitTestInvalidateMtBrokerMemoryId(memoryId);
    otherProcess.cleanup();
    if (KSystem::getProcess(otherProcessId)) {
        testFail("MT WASM broker preload secondary process is released from KSystem");
    }
    wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
}

void testWasmJitMtModuleBrokerThreadStartOwner() {
    S32 oldEnabled = wasmJitTestSetMtModuleBrokerEnabled(1);
    KMemory* owner = reinterpret_cast<KMemory*>(0x7fff6000);
    void* firstStartArg = reinterpret_cast<void*>(0x12345000);
    U32 firstIncarnation = wasmJitTestGetMtMemoryIncarnation(owner);
    if (!firstIncarnation) {
        testFail("MT WASM thread-start owner has nonzero incarnation");
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    wasmJitTestPrepareMtThreadStart(firstStartArg, owner);
    WasmJitMtThreadStartOwnerSnapshot taken;
    if (!wasmJitTestTakeMtThreadStart(firstStartArg, &taken) ||
            taken.memoryId != (U32)(uintptr_t)owner ||
            taken.memoryIncarnation != firstIncarnation) {
        testFail("MT WASM thread-start owner consumes exact registration");
    }
    if (wasmJitTestTakeMtThreadStart(firstStartArg, &taken)) {
        testFail("MT WASM thread-start owner registration is single-use");
    }

    void* cancelledStartArg = reinterpret_cast<void*>(0x12346000);
    wasmJitTestPrepareMtThreadStart(cancelledStartArg, owner);
    wasmJitTestCancelMtThreadStart(cancelledStartArg);
    if (wasmJitTestTakeMtThreadStart(cancelledStartArg, &taken)) {
        testFail("MT WASM thread-start owner cancellation removes registration");
    }

    U32 oldModuleId = wasmJitTestReserveMtBrokerModule(owner);
    wasmJitTestInvalidateMtBrokerMemoryId((U32)(uintptr_t)owner);
    U32 secondIncarnation = wasmJitTestGetMtMemoryIncarnation(owner);
    if (!secondIncarnation || secondIncarnation == firstIncarnation) {
        testFail("MT WASM memory invalidation advances incarnation");
    }
    if (wasm_jit_mt_broker_validate_module(oldModuleId, (U32)(uintptr_t)owner, firstIncarnation) != -2) {
        testFail("MT WASM retired module validates as stale incarnation");
    }
    wasmJitTestInvalidateMtBrokerMemoryId((U32)(uintptr_t)owner);
    wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
}

void testWasmJitMtModuleBrokerExecIncarnation() {
    S32 oldEnabled = wasmJitTestSetMtModuleBrokerEnabled(1);
    std::vector<U8> bytes = makeBrokerTestModule();
    KMemory* memory = testContext().memory;
    U32 memoryId = (U32)(uintptr_t)memory;
    U32 oldModuleId = wasmJitTestReserveMtBrokerModule(memory);
    U32 oldIncarnation = wasmJitTestGetMtMemoryIncarnation(memory);
    void* pendingStartArg = reinterpret_cast<void*>(0x12347000);
    S32 source = 0;

    wasmJitTestPrepareMtThreadStart(pendingStartArg, memory);
    if (!oldModuleId || !oldIncarnation ||
            testWasmJitBrokerLookup(oldModuleId, memoryId, oldIncarnation,
                bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1, &source) != 42 ||
            source != BROKER_LOOKUP_LOCAL_COMPILE ||
            !waitForBrokerPublication(oldModuleId) ||
            !testWasmJitBrokerHasLocalModule(oldModuleId, memoryId, oldIncarnation)) {
        testFail("MT WASM broker exec old-incarnation publication setup");
        wasmJitTestCancelMtThreadStart(pendingStartArg);
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    BrokerExecMainStatsSnapshot beforeInvalidation = getBrokerExecMainStats();
    wasmJitTestInvalidateMtBrokerMemory(memory);
    if (testWasmJitBrokerHasLocalModule(oldModuleId, memoryId, oldIncarnation)) {
        testFail("MT WASM broker exec invalidation purges local cache before return");
    }

    U32 newIncarnation = wasmJitTestGetMtMemoryIncarnation(memory);
    if (!newIncarnation || newIncarnation == oldIncarnation) {
        testFail("MT WASM broker exec same pointer receives new incarnation");
    }
    WasmJitMtThreadStartOwnerSnapshot pendingOwner;
    if (wasmJitTestTakeMtThreadStart(pendingStartArg, &pendingOwner)) {
        testFail("MT WASM broker exec retires pending thread-start owner");
    }

    source = 0;
    if (testWasmJitBrokerLookup(oldModuleId, memoryId, newIncarnation,
            bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1, &source) != 42 ||
            source != BROKER_LOOKUP_LOCAL_COMPILE ||
            !testWasmJitBrokerHasLocalModule(oldModuleId, memoryId, newIncarnation)) {
        testFail("MT WASM broker exec exact new owner cannot hit old row");
    }
    testWasmJitBrokerReleaseRaw(oldModuleId, memoryId, oldIncarnation);
    getBrokerExecMainStats();
    if (!testWasmJitBrokerHasLocalModule(oldModuleId, memoryId, newIncarnation)) {
        testFail("MT WASM broker delayed old purge preserves new incarnation row");
    }
    if (!trackBrokerPurges(memoryId, oldIncarnation)) {
        testFail("MT WASM broker late publication purge tracking timeout");
        S32 cleanupPurgeMessages = 0;
        S32 cleanupCachedModules = 0;
        if (!waitForBrokerPurgeCount(memoryId, oldIncarnation,
                cleanupPurgeMessages, cleanupCachedModules)) {
            testFail("MT WASM broker late publication tracking cleanup timeout");
        }
        testWasmJitBrokerReleaseRaw(oldModuleId, memoryId, newIncarnation);
        wasmJitTestInvalidateMtBrokerMemory(memory);
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    BrokerLatePublisherArgs lateArgs;
    lateArgs.moduleId = oldModuleId;
    lateArgs.memoryId = memoryId;
    lateArgs.memoryIncarnation = oldIncarnation;
    lateArgs.bytes = bytes.data();
    lateArgs.size = (U32)bytes.size();
    std::thread latePublisher(brokerLatePublisherThread, &lateArgs);
    latePublisher.join();
    if (lateArgs.value != 42 || lateArgs.source != BROKER_LOOKUP_LOCAL_COMPILE ||
            lateArgs.stats.staleIncarnationDrops != beforeInvalidation.staleIncarnationDrops + 1) {
        testFail("MT WASM broker late old publication is rejected as stale");
    }
    S32 latePurgeMessages = 0;
    S32 lateCachedModules = 0;
    if (!waitForBrokerPurgeCount(memoryId, oldIncarnation,
            latePurgeMessages, lateCachedModules)) {
        testFail("MT WASM broker late publication source purge barrier timeout");
    } else if (latePurgeMessages != 1 || lateCachedModules != 0) {
        testFail("MT WASM broker late publication purge targets only source");
    }
    testWasmJitBrokerReleaseRaw(oldModuleId, memoryId, newIncarnation);

    KMemory* holderOwner = reinterpret_cast<KMemory*>(0x7fff9000);
    U32 holderMemoryId = (U32)(uintptr_t)holderOwner;
    U32 holderModuleId = wasmJitTestReserveMtBrokerModule(holderOwner);
    U32 holderIncarnation = wasmJitTestGetMtMemoryIncarnation(holderOwner);
    testWasmJitBrokerPublishRawWithoutSource(holderModuleId, holderMemoryId, holderIncarnation,
        bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1);
    if (!waitForBrokerPublication(holderModuleId)) {
        testFail("MT WASM broker known-holder publication setup");
        wasmJitTestInvalidateMtBrokerMemoryId(holderMemoryId);
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }

    BrokerThreadArgs holderArgs = {holderModuleId, holderMemoryId, holderIncarnation,
        bytes.data(), (U32)bytes.size()};
    if (!runPreparedBrokerThread(brokerPreloadThread, &holderArgs, holderOwner) ||
            holderArgs.value != 42 || holderArgs.source != BROKER_LOOKUP_HIT) {
        testFail("MT WASM broker selected holder receives preload");
    }
    S32 holderCount = 0;
    S32 workerCount = 0;
    if (!getBrokerHolderCounts(holderModuleId, holderCount, workerCount)) {
        testFail("MT WASM broker known-holder count timeout");
        wasmJitTestInvalidateMtBrokerMemoryId(holderMemoryId);
        S32 cleanupPurgeMessages = 0;
        S32 cleanupCachedModules = 0;
        if (!waitForBrokerPurgeCount(holderMemoryId, holderIncarnation,
                cleanupPurgeMessages, cleanupCachedModules)) {
            testFail("MT WASM broker known-holder tracking cleanup timeout");
        }
        wasmJitTestInvalidateMtBrokerMemory(memory);
        wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        return;
    }
    if (holderCount != 1 || workerCount <= holderCount + 1) {
        testFail("MT WASM broker known holder is a strict worker subset");
    }

    BrokerExecMainStatsSnapshot beforeWrongRelease = getBrokerExecMainStats();
    testWasmJitBrokerReleaseRaw(holderModuleId, memoryId, newIncarnation);
    testWasmJitBrokerReleaseRaw(holderModuleId, holderMemoryId, holderIncarnation + 1);
    BrokerExecMainStatsSnapshot afterWrongRelease = getBrokerExecMainStats();
    S32 cachedAfterWrongRelease = -1;
    if (!wasmJitTestWaitForMtBrokerPurge(holderModuleId, holderMemoryId,
            holderIncarnation, &cachedAfterWrongRelease) || cachedAfterWrongRelease != 1 ||
            afterWrongRelease.purgedModules != beforeWrongRelease.purgedModules ||
            afterWrongRelease.liveRegistryModules != beforeWrongRelease.liveRegistryModules) {
        testFail("MT WASM broker wrong-owner release preserves main and local row");
    }

    wasmJitTestInvalidateMtBrokerMemoryId(holderMemoryId);
    S32 holderPurgeMessages = 0;
    S32 holderCachedModules = 0;
    if (!waitForBrokerPurgeCount(holderMemoryId, holderIncarnation,
            holderPurgeMessages, holderCachedModules)) {
        testFail("MT WASM broker known-holder purge barrier timeout");
    } else if (holderPurgeMessages != workerCount - 1 || holderCachedModules != 0) {
        testFail("MT WASM broker retirement broadcasts slot-safe purge to every observed allocated worker: messages=%d workers=%d cached=%d",
            holderPurgeMessages, workerCount, holderCachedModules);
    }

    U32 reusedModuleId = wasmJitTestReserveMtBrokerModule(holderOwner);
    U32 reusedIncarnation = wasmJitTestGetMtMemoryIncarnation(holderOwner);
    source = 0;
    if (!reusedModuleId || !reusedIncarnation || reusedIncarnation == holderIncarnation ||
            testWasmJitBrokerLookup(reusedModuleId, holderMemoryId, reusedIncarnation,
                bytes.data(), (U32)bytes.size(), BROKER_MODULE_CLASS_STANDALONE, 1, &source) != 42 ||
            source != BROKER_LOOKUP_LOCAL_COMPILE || !waitForBrokerPublication(reusedModuleId)) {
        testFail("MT WASM broker reused-pointer publication setup");
    }
    BrokerExecMainStatsSnapshot beforeDelayedOldRelease = getBrokerExecMainStats();
    testWasmJitBrokerReleaseRaw(reusedModuleId, holderMemoryId, holderIncarnation);
    BrokerExecMainStatsSnapshot afterDelayedOldRelease = getBrokerExecMainStats();
    if (!testWasmJitBrokerHasLocalModule(reusedModuleId, holderMemoryId, reusedIncarnation) ||
            afterDelayedOldRelease.purgedModules != beforeDelayedOldRelease.purgedModules ||
            afterDelayedOldRelease.liveRegistryModules != beforeDelayedOldRelease.liveRegistryModules) {
        testFail("MT WASM broker delayed old release preserves reused-pointer incarnation");
    }

    wasmJitTestInvalidateMtBrokerMemoryId(holderMemoryId);
    wasmJitTestInvalidateMtBrokerMemory(memory);
    wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
}

__attribute__((noinline)) void runBrokerPreloadDiagnosticsFull() {
    S32 oldEnabled = wasmJitTestSetMtModuleBrokerEnabled(1);
    std::vector<U8> bytes = makeBrokerTestModule();

    runBrokerDroppedRunDiagnostic();

    WasmJitMtBrokerMainStatsSnapshot beforeMissing = getBrokerExecMainStats();
    BrokerBoundedThreadState missingArgs;
    bool missingStarted = runBoundedBrokerThread(missingArgs, nullptr,
        BrokerThreadRegistration::Missing);
    WasmJitMtBrokerMainStatsSnapshot afterMissing = getBrokerExecMainStats();
    if (!missingStarted || !missingArgs.started || !missingArgs.completed ||
            afterMissing.preloadOwnerMisses != beforeMissing.preloadOwnerMisses + 1 ||
            !sameMainStatsExceptOwnerMisses(beforeMissing, afterMissing)) {
        testFail("MT WASM broker missing registration forwards with only owner miss");
    }
    KMemory* disabledOwner = reinterpret_cast<KMemory*>(0x7fffa000);
    BrokerBoundedThreadState disabledArgs;
    wasmJitTestSetMtModuleBrokerEnabled(0);
    WasmJitMtBrokerMainStatsSnapshot beforeDisabled = getBrokerExecMainStats();
    bool disabledStarted = runBoundedBrokerThread(disabledArgs,
        disabledOwner, BrokerThreadRegistration::Current);
    WasmJitMtBrokerMainStatsSnapshot afterDisabled = getBrokerExecMainStats();
    wasmJitTestSetMtModuleBrokerEnabled(1);
    if (!disabledStarted || !disabledArgs.started ||
            afterDisabled.preloadAttempts != beforeDisabled.preloadAttempts ||
            afterDisabled.preloadOwnerMisses != beforeDisabled.preloadOwnerMisses) {
        testFail("MT WASM broker disabled start forwards without preload diagnostics");
    }

    KMemory* staleOwner = reinterpret_cast<KMemory*>(0x7fffa100);
    BrokerBoundedThreadState staleArgs;
    WasmJitMtBrokerMainStatsSnapshot beforeStale = getBrokerExecMainStats();
    bool staleStarted = runBoundedBrokerThread(staleArgs,
        staleOwner, BrokerThreadRegistration::Stale);
    WasmJitMtBrokerMainStatsSnapshot afterStale = getBrokerExecMainStats();
    if (!staleStarted || !staleArgs.started ||
            afterStale.preloadOwnerMisses != beforeStale.preloadOwnerMisses + 1 ||
            afterStale.preloadAttempts != beforeStale.preloadAttempts) {
        testFail("MT WASM broker stale registration forwards with owner miss");
    }

    KMemory* emptyOwner = reinterpret_cast<KMemory*>(0x7fffa200);
    wasmJitTestGetMtMemoryIncarnation(emptyOwner);
    BrokerBoundedThreadState emptyArgs;
    WasmJitMtBrokerMainStatsSnapshot beforeEmpty = getBrokerExecMainStats();
    bool emptyStarted = runBoundedBrokerThread(emptyArgs,
        emptyOwner, BrokerThreadRegistration::Current);
    WasmJitMtBrokerMainStatsSnapshot afterEmpty = getBrokerExecMainStats();
    if (!emptyStarted || !emptyArgs.started ||
            afterEmpty.preloadAttempts != beforeEmpty.preloadAttempts + 1 ||
            afterEmpty.preloadCandidates != beforeEmpty.preloadCandidates ||
            afterEmpty.preloadSent != beforeEmpty.preloadSent) {
        testFail("MT WASM broker empty owner attempts preload and forwards start");
    }

    KMemory* failureOwner = reinterpret_cast<KMemory*>(0x7fffa300);
    U32 failureMemoryId = (U32)(uintptr_t)failureOwner;
    U32 failureModuleId = wasmJitTestReserveMtBrokerModule(failureOwner);
    U32 failureIncarnation = wasmJitTestGetMtMemoryIncarnation(failureOwner);
    testWasmJitBrokerPublishRawWithoutSource(failureModuleId, failureMemoryId,
        failureIncarnation, bytes.data(), (U32)bytes.size(),
        BROKER_MODULE_CLASS_STANDALONE, 1);
    if (!waitForBrokerPublication(failureModuleId)) {
        testFail("MT WASM broker non-OOM preload failure publication setup");
    }
    wasmJitTestSetMtBrokerDeliveryFailure(failureModuleId, true);
    BrokerBoundedThreadState failureArgs;
    WasmJitMtBrokerMainStatsSnapshot beforeFailure = getBrokerExecMainStats();
    bool failureStarted = runBoundedBrokerThread(failureArgs,
        failureOwner, BrokerThreadRegistration::Current);
    WasmJitMtBrokerMainStatsSnapshot afterFailure = getBrokerExecMainStats();
    wasmJitTestSetMtBrokerDeliveryFailure(failureModuleId, false);
    if (!failureStarted || !failureArgs.started ||
            afterFailure.preloadAttempts != beforeFailure.preloadAttempts + 1 ||
            afterFailure.preloadCandidates != beforeFailure.preloadCandidates + 1 ||
            afterFailure.preloadSent != beforeFailure.preloadSent ||
            afterFailure.preloadFailures != beforeFailure.preloadFailures + 1 ||
            afterFailure.deliveryFailures != beforeFailure.deliveryFailures + 1 ||
            afterFailure.preloadOomBlocks != beforeFailure.preloadOomBlocks) {
        testFail("MT WASM broker non-OOM preload failure stops batch and forwards start");
    }
    BrokerBoundedThreadState retryArgs;
    bool retryStarted = runBoundedBrokerThread(retryArgs,
        failureOwner, BrokerThreadRegistration::Current);
    WasmJitMtBrokerMainStatsSnapshot afterRetry = getBrokerExecMainStats();
    if (!retryStarted || !retryArgs.started ||
            afterRetry.preloadAttempts != afterFailure.preloadAttempts + 1 ||
            afterRetry.preloadSent != afterFailure.preloadSent + 1) {
        testFail("MT WASM broker non-OOM preload failure permits next start attempt");
    }

    KMemory* oomOwner = reinterpret_cast<KMemory*>(0x7fffa400);
    U32 oomMemoryId = (U32)(uintptr_t)oomOwner;
    U32 oomModuleId = wasmJitTestReserveMtBrokerModule(oomOwner);
    U32 oomIncarnation = wasmJitTestGetMtMemoryIncarnation(oomOwner);
    testWasmJitBrokerPublishRawWithoutSource(oomModuleId, oomMemoryId,
        oomIncarnation, bytes.data(), (U32)bytes.size(),
        BROKER_MODULE_CLASS_STANDALONE, 1);
    if (!waitForBrokerPublication(oomModuleId)) {
        testFail("MT WASM broker preload OOM publication setup");
    }
    wasmJitTestSetMtBrokerDeliveryOom(oomModuleId, true);
    BrokerBoundedThreadState oomArgs;
    WasmJitMtBrokerMainStatsSnapshot beforeOom = getBrokerExecMainStats();
    bool oomStarted = runBoundedBrokerThread(oomArgs,
        oomOwner, BrokerThreadRegistration::Current);
    WasmJitMtBrokerMainStatsSnapshot afterOom = getBrokerExecMainStats();
    wasmJitTestSetMtBrokerDeliveryOom(oomModuleId, false);
    if (!oomStarted || !oomArgs.started ||
            afterOom.preloadAttempts != beforeOom.preloadAttempts + 1 ||
            afterOom.preloadCandidates != beforeOom.preloadCandidates + 1 ||
            afterOom.preloadSent != beforeOom.preloadSent ||
            afterOom.preloadOomBlocks != beforeOom.preloadOomBlocks + 1 ||
            afterOom.preloadFailures != beforeOom.preloadFailures + 1 ||
            afterOom.deliveryFailures != beforeOom.deliveryFailures + 1) {
        testFail("MT WASM broker preload OOM blocks exact worker owner and forwards start");
    }
    BrokerBoundedThreadState blockedArgs;
    bool blockedStarted = runBoundedBrokerThread(blockedArgs,
        oomOwner, BrokerThreadRegistration::Current);
    WasmJitMtBrokerMainStatsSnapshot afterBlocked = getBrokerExecMainStats();
    if (!blockedStarted || !blockedArgs.started ||
            afterBlocked.preloadAttempts != afterOom.preloadAttempts ||
            afterBlocked.preloadSent != afterOom.preloadSent ||
            afterBlocked.preloadOomBlocks != afterOom.preloadOomBlocks) {
        testFail("MT WASM broker blocked exact worker owner skips preload without retry");
    }

    KMemory* scopeOwner = reinterpret_cast<KMemory*>(0x7fffa700);
    U32 scopeMemoryId = (U32)(uintptr_t)scopeOwner;
    U32 scopeModuleId = wasmJitTestReserveMtBrokerModule(scopeOwner);
    U32 scopeIncarnation = wasmJitTestGetMtMemoryIncarnation(scopeOwner);
    testWasmJitBrokerPublishRawWithoutSource(scopeModuleId, scopeMemoryId,
        scopeIncarnation, bytes.data(), (U32)bytes.size(),
        BROKER_MODULE_CLASS_STANDALONE, 1);
    KMemory* scopeOtherOwner = reinterpret_cast<KMemory*>(0x7fffa800);
    U32 scopeOtherMemoryId = (U32)(uintptr_t)scopeOtherOwner;
    U32 scopeOtherModuleId = wasmJitTestReserveMtBrokerModule(scopeOtherOwner);
    U32 scopeOtherIncarnation = wasmJitTestGetMtMemoryIncarnation(scopeOtherOwner);
    testWasmJitBrokerPublishRawWithoutSource(scopeOtherModuleId, scopeOtherMemoryId,
        scopeOtherIncarnation, bytes.data(), (U32)bytes.size(),
        BROKER_MODULE_CLASS_STANDALONE, 1);
    if (!waitForBrokerPublication(scopeModuleId) ||
            !waitForBrokerPublication(scopeOtherModuleId)) {
        testFail("MT WASM broker preload OOM scope publication setup");
    }
    BrokerPreloadOomScopeSnapshot scopeSnapshot;
    bool copiedScope = getPreloadOomScope(scopeModuleId, scopeMemoryId,
        scopeIncarnation, scopeOtherModuleId, scopeOtherMemoryId,
        scopeOtherIncarnation, scopeSnapshot);
    if (!copiedScope || !scopeSnapshot.distinctWorkers ||
            scopeSnapshot.firstWorkerRuns != 3 ||
            scopeSnapshot.secondWorkerRuns != 1 ||
            scopeSnapshot.firstWorkerFirstOwnerModules != 0 ||
            scopeSnapshot.secondWorkerFirstOwnerModules != 1 ||
            scopeSnapshot.firstWorkerSecondOwnerModules != 1 ||
            scopeSnapshot.preloadAttempts != 3 ||
            scopeSnapshot.preloadSent != 2 ||
            scopeSnapshot.preloadFailures != 1 ||
            scopeSnapshot.preloadOomBlocks != 1) {
        testFail("MT WASM broker preload OOM block has exact Worker and owner scope");
    }

    wasmJitTestInvalidateMtBrokerMemoryId(oomMemoryId);
    U32 newOomModuleId = wasmJitTestReserveMtBrokerModule(oomOwner);
    U32 newOomIncarnation = wasmJitTestGetMtMemoryIncarnation(oomOwner);
    testWasmJitBrokerPublishRawWithoutSource(newOomModuleId, oomMemoryId,
        newOomIncarnation, bytes.data(), (U32)bytes.size(),
        BROKER_MODULE_CLASS_STANDALONE, 1);
    if (!waitForBrokerPublication(newOomModuleId)) {
        testFail("MT WASM broker post-invalidation preload publication setup");
    }
    BrokerBoundedThreadState invalidatedArgs;
    WasmJitMtBrokerMainStatsSnapshot beforeInvalidated = getBrokerExecMainStats();
    bool invalidatedStarted = runBoundedBrokerThread(invalidatedArgs,
        oomOwner, BrokerThreadRegistration::Current);
    WasmJitMtBrokerMainStatsSnapshot afterInvalidated = getBrokerExecMainStats();
    if (!invalidatedStarted || !invalidatedArgs.started || !invalidatedArgs.completed ||
            newOomIncarnation == oomIncarnation ||
            afterInvalidated.preloadAttempts != beforeInvalidated.preloadAttempts + 1 ||
            afterInvalidated.preloadSent != beforeInvalidated.preloadSent + 1) {
        testFail("MT WASM broker invalidation permits preload for new incarnation");
    }

    KMemory* compileOwner = reinterpret_cast<KMemory*>(0x7fffa500);
    U32 compileMemoryId = (U32)(uintptr_t)compileOwner;
    U32 cachedModuleId = wasmJitTestReserveMtBrokerModule(compileOwner);
    U32 compileModuleId = wasmJitTestReserveMtBrokerModule(compileOwner);
    U32 compileIncarnation = wasmJitTestGetMtMemoryIncarnation(compileOwner);
    testWasmJitBrokerPublishRawWithoutSource(cachedModuleId, compileMemoryId,
        compileIncarnation, bytes.data(), (U32)bytes.size(),
        BROKER_MODULE_CLASS_STANDALONE, 1);
    if (!waitForBrokerPublication(cachedModuleId)) {
        testFail("MT WASM broker local compile OOM cache-hit setup");
    }
    KMemory* compileOtherOwner = reinterpret_cast<KMemory*>(0x7fffa600);
    U32 compileOtherMemoryId = (U32)(uintptr_t)compileOtherOwner;
    U32 compileOtherModuleId = wasmJitTestReserveMtBrokerModule(compileOtherOwner);
    U32 compileOtherIncarnation = wasmJitTestGetMtMemoryIncarnation(compileOtherOwner);
    BrokerBoundedThreadState compileArgs;
    compileArgs.operation = BrokerBoundedThreadOperation::CompileOom;
    compileArgs.cachedModuleId = cachedModuleId;
    compileArgs.compileModuleId = compileModuleId;
    compileArgs.memoryId = compileMemoryId;
    compileArgs.memoryIncarnation = compileIncarnation;
    compileArgs.otherModuleId = compileOtherModuleId;
    compileArgs.otherMemoryId = compileOtherMemoryId;
    compileArgs.otherMemoryIncarnation = compileOtherIncarnation;
    compileArgs.bytes = bytes;
    WasmJitMtBrokerMainStatsSnapshot beforeCompileOom = getBrokerExecMainStats();
    bool compileStarted = runBoundedBrokerThread(compileArgs,
        compileOwner, BrokerThreadRegistration::Current);
    bool otherPublished = waitForBrokerPublication(compileOtherModuleId);
    WasmJitMtBrokerMainStatsSnapshot afterCompileOom = getBrokerExecMainStats();
    if (!compileStarted || !compileArgs.started || !compileArgs.completed ||
            compileArgs.oomValue != -1 ||
            compileArgs.cachedValue != 42 || compileArgs.cachedSource != BROKER_LOOKUP_HIT ||
            compileArgs.firstCompileValue != 42 ||
            compileArgs.firstCompileSource != BROKER_LOOKUP_LOCAL_COMPILE ||
            compileArgs.cachedAfterCompile || compileArgs.secondCompileValue != 42 ||
            compileArgs.secondCompileSource != BROKER_LOOKUP_LOCAL_COMPILE ||
            compileArgs.otherValue != 42 ||
            compileArgs.otherSource != BROKER_LOOKUP_LOCAL_COMPILE ||
            !compileArgs.otherCached || !otherPublished ||
            afterCompileOom.firstPublications != beforeCompileOom.firstPublications + 1) {
        testFail("MT WASM broker local OOM blocks only the exact owner publication scope");
    }

    S32 publicPreloadLimit = 0;
    S32 publicStatsDone = 0;
    testWasmJitBrokerCopyPublicPreloadLimit(&publicPreloadLimit, &publicStatsDone);
    if (!waitForAtomicDone(publicStatsDone) ||
            afterCompileOom.preloadModuleLimit != BROKER_PRELOAD_LIMIT ||
            publicPreloadLimit != BROKER_PRELOAD_LIMIT) {
        testFail("MT WASM broker diagnostics expose constant preload module limit");
    }

    wasmJitTestInvalidateMtBrokerMemoryId(failureMemoryId);
    wasmJitTestInvalidateMtBrokerMemoryId(oomMemoryId);
    wasmJitTestInvalidateMtBrokerMemoryId(compileMemoryId);
    wasmJitTestInvalidateMtBrokerMemoryId(compileOtherMemoryId);
    wasmJitTestInvalidateMtBrokerMemoryId(scopeMemoryId);
    wasmJitTestInvalidateMtBrokerMemoryId(scopeOtherMemoryId);
    wasmJitTestInvalidateMtBrokerMemoryId((U32)(uintptr_t)emptyOwner);
    wasmJitTestInvalidateMtBrokerMemoryId((U32)(uintptr_t)disabledOwner);
    wasmJitTestInvalidateMtBrokerMemoryId((U32)(uintptr_t)staleOwner);
    runBrokerOwnerRowStress(bytes);

    wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
}

__attribute__((noinline)) void runBrokerOwnerRowDiagnosticsOnly() {
    S32 oldEnabled = wasmJitTestSetMtModuleBrokerEnabled(1);
    std::vector<U8> bytes = makeBrokerTestModule();
    runBrokerOwnerRowStress(bytes);
    wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
}

void testWasmJitMtModuleBrokerPreloadDiagnostics() {
    if (!testBootstrapOwnerMiss()) {
        testFail("MT WASM broker ignores only the unowned bootstrap run");
        return;
    }
    if (testWasmJitBrokerSupportsDroppedRunTimeout()) {
        runBrokerPreloadDiagnosticsFull();
    } else {
        runBrokerOwnerRowDiagnosticsOnly();
    }
}

void testWasmJitMtScheduleThreadPreload() {
    S32 oldEnabled = wasmJitTestSetMtModuleBrokerEnabled(1);
    KMemory* memory = testContext().memory;
    KProcessPtr process = testContext().process;
    U32 initialThreadCount = process->getThreadCount();
    struct RestoreScheduleThreadTest {
        S32 oldEnabled;
        KMemory* memory;
        KProcessPtr process;
        KThread* child = nullptr;
        bool scheduled = false;
        bool joined = false;

        ~RestoreScheduleThreadTest() {
            if (child && scheduled && !joined) {
                joinThread(child);
            }
            KThread::setCurrentThread(testContext().thread);
            if (child) {
                process->deleteThread(child);
            }
            wasmJitTestInvalidateMtBrokerMemory(memory);
            wasmJitTestResetMtRuntimeBatching();
            wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        }
    } restore{oldEnabled, memory, process};

    testNewInstruction(0);
    wasmJitTestResetMtBrokerStats();
    wasmJitTestResetMtRuntimeBatching();
    WasmJitBatchLimits limits;
    limits.maxBlocks = 2;
    limits.maxBatchBytes = 1024 * 1024;
    limits.urgentPendingHits = 8;
    limits.maxProcessOpenBytes = 4 * 1024 * 1024;
    wasmJitTestSetMtBatchLimits(limits);
    wasmJitTestSetMtMappedFileKeyOverride(277);
    wasmJitTestEnableMtRuntimeBatching(true);

    U32 firstAddress = testContext().codeIp;
    testPushCode8(0x40); // inc eax
    testPushCode8(0xe9); // jmp to the next block
    testPushCode32(0);
    U32 secondAddress = testContext().codeIp;
    testPushCode8(0x41); // inc ecx
    testPushCode8(0xcd);
    testPushCode8(0x97);

    auto compile = [](U32 address) {
        DecodedOp* op = testContext().cpu->getOp(address, 0);
        startNewJIT(testContext().cpu, address, op);
        return op;
    };
    DecodedOp* first = compile(firstAddress);
    if (!(first->flags2 & OP_FLAG2_WASM_JIT_PENDING) || first->pfnJitCode ||
            wasmJitTestMtPendingCount() != 1) {
        testFail("MT WASM scheduleThread first mapped block remains pending");
        return;
    }
    DecodedOp* second = compile(secondAddress);
    if (!first->pfnJitCode || !second->pfnJitCode ||
            first->pfnJitCode == second->pfnJitCode ||
            (first->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            (second->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            wasmJitTestMtRuntimeGroupCount() != 1 ||
            wasmJitTestMtRuntimeModuleCount() != 1) {
        testFail("MT WASM scheduleThread creates one two-slot online group");
        return;
    }

    int firstSlot = (int)(uintptr_t)first->pfnJitCode;
    int secondSlot = (int)(uintptr_t)second->pfnJitCode;
    WasmJitMtBrokerModuleRef firstRef = wasmJitTestGetMtBrokerSlotRef(firstSlot);
    WasmJitMtBrokerModuleRef secondRef = wasmJitTestGetMtBrokerSlotRef(secondSlot);
    if (!firstRef.moduleId || firstRef.moduleId != secondRef.moduleId ||
            firstRef.memoryId != (U32)(uintptr_t)memory ||
            firstRef.memoryIncarnation != secondRef.memoryIncarnation ||
            !waitForBrokerPublication(firstRef.moduleId)) {
        testFail("MT WASM scheduleThread publishes one exact grouped broker module");
        return;
    }
    if (!wasmJitTestResetMtScheduleThreadPreloadStats()) {
        testFail("MT WASM scheduleThread resets preload-before-run diagnostics");
        return;
    }

    BrokerOwnerRowCounts workersBefore;
    if (!getBrokerOwnerRowCounts(workersBefore)) {
        testFail("MT WASM scheduleThread captures every Worker baseline");
        return;
    }
    WasmJitMtBrokerStatsSnapshot brokerBefore =
        wasmJitTestGetMtBrokerStats(memory);
    WasmJitMtBrokerMainStatsSnapshot mainBefore =
        getBrokerExecMainStats();
    WasmJitMtRuntimeStatsSnapshot runtimeBefore =
        wasmJitTestGetMtRuntimeStats();

    restore.child = process->createThread();
    restore.child->cpu->clone(testContext().cpu);
    restore.child->cpu->reg[0].u32 = 0;
    restore.child->cpu->reg[1].u32 = 0;
    restore.child->cpu->eip.u32 =
        firstAddress - restore.child->cpu->seg[CS].address;
    restore.child->cpu->nextOp = nullptr;
    scheduleThread(restore.child);
    restore.scheduled = true;
    joinThread(restore.child);
    restore.joined = true;
    KThread::setCurrentThread(testContext().thread);

    if (restore.child->cpu->reg[0].u32 != 1 ||
            restore.child->cpu->reg[1].u32 != 1 ||
            !restore.child->cpu->nextOp ||
            restore.child->cpu->nextOp->inst != TestEnd) {
        testFail("MT WASM scheduleThread guest executes both grouped blocks");
    }

    BrokerOwnerRowCounts workersAfter;
    if (!getBrokerOwnerRowCounts(workersAfter)) {
        testFail("MT WASM scheduleThread captures every Worker result");
        return;
    }
    if (workersAfter.runMessages != 1 ||
            workersAfter.preloadedRunMessages != 1 ||
            workersAfter.preloadModulesBeforeRun != 1) {
        testFail("MT WASM scheduleThread preloads the grouped module before the run message");
    }
    WasmJitMtBrokerStatsSnapshot brokerAfter =
        wasmJitTestGetMtBrokerStats(memory);
    WasmJitMtBrokerMainStatsSnapshot mainAfter =
        getBrokerExecMainStats();
    WasmJitMtRuntimeStatsSnapshot runtimeAfter =
        wasmJitTestGetMtRuntimeStats();
    if (brokerAfter.localCompiles != brokerBefore.localCompiles ||
            brokerAfter.brokerHits != brokerBefore.brokerHits + 1 ||
            mainAfter.preloadAttempts != mainBefore.preloadAttempts + 1 ||
            mainAfter.preloadSent != mainBefore.preloadSent + 1 ||
            mainAfter.preloadOwnerMisses != mainBefore.preloadOwnerMisses ||
            mainAfter.ownershipErrors != mainBefore.ownershipErrors ||
            mainAfter.staleIncarnationDrops != mainBefore.staleIncarnationDrops ||
            runtimeAfter.permanentFailures != runtimeBefore.permanentFailures) {
        testFail("MT WASM scheduleThread consumes one preloaded broker module without recompiling");
    }

    if (workersAfter.workerStats.size() != workersBefore.workerStats.size()) {
        testFail("MT WASM scheduleThread preserves the allocated Worker set");
    } else {
        U32 selectedWorkers = 0;
        for (U32 i = 0; i < workersBefore.workerStats.size(); ++i) {
            const BrokerWorkerDiagnosticStats& before = workersBefore.workerStats[i];
            const BrokerWorkerDiagnosticStats& after = workersAfter.workerStats[i];
            bool selected =
                after.localCompiles == before.localCompiles &&
                after.brokerHits == before.brokerHits + 1 &&
                after.groupInstanceCreations == before.groupInstanceCreations + 1 &&
                after.groupInstanceReuses == before.groupInstanceReuses + 1 &&
                after.groupedBrokerHits == before.groupedBrokerHits + 1 &&
                after.groupedLocalCompiles == before.groupedLocalCompiles;
            bool unchanged =
                after.localCompiles == before.localCompiles &&
                after.brokerHits == before.brokerHits &&
                after.groupInstanceCreations == before.groupInstanceCreations &&
                after.groupInstanceReuses == before.groupInstanceReuses &&
                after.groupedBrokerHits == before.groupedBrokerHits &&
                after.groupedLocalCompiles == before.groupedLocalCompiles;
            if (selected) {
                selectedWorkers += 1;
            } else if (!unchanged) {
                testFail("MT WASM scheduleThread changes only the selected physical Worker counters");
                break;
            }
        }
        if (selectedWorkers != 1) {
            testFail("MT WASM scheduleThread records one grouped broker hit and same-instance reuse");
        }
    }

    process->deleteThread(restore.child);
    restore.child = nullptr;
    if (process->getThreadCount() != initialThreadCount) {
        testFail("MT WASM scheduleThread cleans guest thread state");
    }
}

#else

void testWasmJitMtModuleBrokerTransport() {
}

void testWasmJitMtStandaloneModuleBroker() {
}

void testWasmJitMtModuleBrokerLifecycle() {
}

void testWasmJitMtGroupedModuleBroker() {
}

void testWasmJitMtModuleBrokerThreadStartOwner() {
}

void testWasmJitMtModuleBrokerPreloadSelection() {
}

void testWasmJitMtModuleBrokerExecIncarnation() {
}

void testWasmJitMtModuleBrokerPreloadDiagnostics() {
}

void testWasmJitMtScheduleThreadPreload() {
}

bool wasmJitTestWaitForMtBrokerDelivery(U32 moduleId) {
    return false;
}

#endif
