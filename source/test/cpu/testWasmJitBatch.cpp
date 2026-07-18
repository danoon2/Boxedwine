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
#if defined(BOXEDWINE_WASM_JIT)
#include "../../emulation/cpu/jit/jitCodeLifecycle.h"
#include "../../emulation/cpu/wasm/jitWasmCodeGen.h"
#include "../../emulation/cpu/wasm/wasmEmitter.h"
#include "../../emulation/cpu/wasm/wasmJitBatchPolicy.h"
#include "../../emulation/cpu/wasm/wasmModuleMerger.h"
#include <climits>
#include <emscripten.h>
#ifdef BOXEDWINE_MULTI_THREADED
#include <emscripten/threading.h>
#include <memory>
#include <pthread.h>
#endif

#ifdef BOXEDWINE_MULTI_THREADED
void testWasmJitMtCpuHazardStateIsCold() {
    CPU* cpu = testContext().cpu;
    uintptr_t existingStateEnd = (uintptr_t)&cpu->big + sizeof(cpu->big) - 1;
    uintptr_t hazardStateStart = (uintptr_t)&cpu->wasmJitActiveTableIndex;
    if (hazardStateStart <= existingStateEnd) {
        testFail("MT WASM JIT owner-hazard bookkeeping must not shift existing CPU/JIT state");
    }
}

void testWasmJitMtExecDetachPreservesSharedDecodedOps() {
    testNewInstruction(0);
    U32 address = testContext().codeIp;
    testPushCode8(0x90); // nop
    testPushCode8(0xcd);
    testPushCode8(0x97);
    DecodedOp* sharedOp = testContext().cpu->getOp(address, 0);
    void* sentinelJitCode = reinterpret_cast<void*>(0x1234);
    sharedOp->pfnJitCode = sentinelJitCode;

    KProcessPtr cloneProcess = KProcess::create();
    KMemory* cloneMemory = KMemory::create(cloneProcess.get());
    cloneProcess->memory = cloneMemory;
    cloneMemory->clone(testContext().memory, true);
    cloneMemory->execvReset(true);

    if (sharedOp->pfnJitCode != sentinelJitCode) {
        testFail("MT WASM JIT CLONE_VM exec detach must not sweep the shared decoded-op cache");
    }
    sharedOp->pfnJitCode = nullptr;

    U32 cloneProcessId = cloneProcess->id;
    KProcessWeakPtr weakCloneProcess = cloneProcess;
    if (KSystem::getProcess(cloneProcessId)) {
        KSystem::eraseProcess(cloneProcessId);
    }
    cloneProcess.reset();
    if (!weakCloneProcess.expired()) {
        testFail("MT WASM JIT exec detach test leaked its clone process");
    }
}
#endif

static std::vector<U8> makeMergeInput(S32 value, const char* memoryName = "memory") {
    WasmEmitter emitter;
    U32 type = emitter.addFuncType({WasmType::I32, WasmType::I32}, {});
    emitter.addMemoryImport("env", memoryName);
    U32 function = emitter.addFunction(type);
    emitter.addExport("execute", function);
    emitter.beginFunction({});
    emitter.emitLocalGet(0);
    emitter.emitI32Const(value);
    emitter.emitI32Store(0);
    emitter.endFunction();
    return emitter.finalize();
}

EM_JS(int, test_run_merged_wasm, (const void* bytes, int size, int count, U32 output), {
    try {
        var module = new WebAssembly.Module(new Uint8Array(HEAPU8.buffer, bytes, size));
        var instance = new WebAssembly.Instance(module, {env: {memory: wasmMemory}});
        for (var i = 0; i < count; ++i) {
            var fn = instance.exports['b' + i];
            if (!fn) {
                return 0;
            }
            fn(output + i * 4, 0);
        }
        return 1;
    } catch (e) {
        console.error('test_run_merged_wasm failed:', e);
        return 0;
    }
});

void testWasmJitModuleMerger() {
    std::vector<U8> first = makeMergeInput(0x11223344);
    std::vector<U8> second = makeMergeInput(0x55667788);
    std::vector<WasmJitMergeInput> inputs = {{&first}, {&second}};
    std::vector<U8> merged;
    BString error;
    if (!wasmJitMergeModules(inputs, merged, error)) {
        testFail("wasm runtime merger failed: %s", error.c_str());
        return;
    }
    U32 output[2] = {};
    if (!test_run_merged_wasm(merged.data(), (int)merged.size(), 2, (U32)(uintptr_t)output)) {
        testFail("wasm runtime merger browser validation");
    }
    if (output[0] != 0x11223344 || output[1] != 0x55667788) {
        testFail("wasm runtime merger exports execute distinct bodies");
    }

    std::vector<U8> incompatible = makeMergeInput(3, "otherMemory");
    inputs = {{&first}, {&incompatible}};
    if (wasmJitMergeModules(inputs, merged, error)) {
        testFail("wasm runtime merger rejects ABI mismatch");
    }
    std::vector<U8> truncated(first.begin(), first.end() - 1);
    inputs = {{&truncated}};
    if (wasmJitMergeModules(inputs, merged, error)) {
        testFail("wasm runtime merger rejects truncated code section");
    }
    inputs.clear();
    if (wasmJitMergeModules(inputs, merged, error)) {
        testFail("wasm runtime merger rejects empty input");
    }
    std::vector<U8> headerOnly = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00};
    inputs = {{&headerOnly}};
    if (wasmJitMergeModules(inputs, merged, error)) {
        testFail("wasm runtime merger rejects missing sections");
    }
    std::vector<U8> malformedUleb = headerOnly;
    malformedUleb.insert(malformedUleb.end(), {1, 0x80, 0x80, 0x80, 0x80, 0x10});
    inputs = {{&malformedUleb}};
    if (wasmJitMergeModules(inputs, merged, error)) {
        testFail("wasm runtime merger rejects overflowing section size");
    }
    std::vector<U8> alreadyMerged;
    inputs = {{&first}, {&second}};
    if (!wasmJitMergeModules(inputs, alreadyMerged, error)) {
        testFail("wasm runtime merger setup for multi-function rejection");
    } else {
        inputs = {{&alreadyMerged}};
        if (wasmJitMergeModules(inputs, merged, error)) {
            testFail("wasm runtime merger rejects multi-function input");
        }
    }
}

void testWasmJitBatchPolicy() {
    WasmJitBatchLimits limits;
    limits.maxBlocks = 2;
    limits.maxBatchBytes = 10;
    limits.urgentPendingHits = 2;
    limits.maxProcessOpenBytes = 12;
    KMemory* mem1 = reinterpret_cast<KMemory*>(0x1000);
    KMemory* mem2 = reinterpret_cast<KMemory*>(0x2000);
    WasmJitBatchKey a{mem1, 7};
    WasmJitBatchKey b{mem1, 8};

    WasmJitBatchPolicy countPolicy(limits);
    if (!countPolicy.enqueue(1, a, 4).empty()) {
        testFail("batch count early flush");
    }
    auto countFlush = countPolicy.enqueue(2, a, 4);
    if (countFlush.size() != 1 || countFlush[0].reason != WasmJitFlushReason::BlockCount || countFlush[0].entries != std::vector<U64>({1, 2})) {
        testFail("batch count flush contents");
    }

    WasmJitBatchPolicy bytePolicy(limits);
    bytePolicy.enqueue(3, a, 8);
    auto byteFlush = bytePolicy.enqueue(4, a, 4);
    if (byteFlush.size() != 1 || byteFlush[0].reason != WasmJitFlushReason::ByteCount || byteFlush[0].entries != std::vector<U64>({3})) {
        testFail("batch byte flush before overflow entry");
    }

    WasmJitBatchPolicy hitPolicy(limits);
    hitPolicy.enqueue(5, a, 4);
    WasmJitFlushRequest hitFlush;
    if (hitPolicy.recordPendingHit(5, hitFlush)) {
        testFail("batch urgent early flush");
    }
    if (!hitPolicy.recordPendingHit(5, hitFlush) || hitFlush.reason != WasmJitFlushReason::PendingHits || hitFlush.entries != std::vector<U64>({5})) {
        testFail("batch urgent hit flush");
    }

    WasmJitBatchPolicy capPolicy(limits);
    capPolicy.enqueue(6, a, 8);
    auto capFlush = capPolicy.enqueue(7, b, 8);
    if (capFlush.size() != 1 || capFlush[0].reason != WasmJitFlushReason::ProcessBytes || capFlush[0].entries != std::vector<U64>({6}) || capPolicy.openBytesForMemory(mem1) != 8) {
        testFail("batch process cap flushes oldest mapping");
    }
    capPolicy.enqueue(8, {mem2, 7}, 4);
    if (!capPolicy.cancel(7) || capPolicy.cancel(999)) {
        testFail("batch cancel result");
    }
    auto cancelled = capPolicy.cancelMemory(mem2);
    if (cancelled != std::vector<U64>({8})) {
        testFail("batch cancel memory entries");
    }

    WasmJitBatchPolicy processPolicy(limits);
    if (!processPolicy.enqueue(9, {mem1, 55}, 4).empty() || !processPolicy.enqueue(10, {mem2, 55}, 4).empty()) {
        testFail("batch same mapping key stays isolated by process memory");
    }
    if (processPolicy.openEntryCount() != 2) {
        testFail("batch process isolation keeps two open entries");
    }
}

void testWasmJitMappedFileRange() {
    auto oldMap = std::make_shared<MappedFile>();
    oldMap->address = 0x1000;
    oldMap->len = 0x1000;
    oldMap->key = 10;
    auto newMap = std::make_shared<MappedFile>();
    newMap->address = 0x1000;
    newMap->len = 0x1000;
    newMap->key = 20;
    auto other = std::make_shared<MappedFile>();
    other->address = 0x3000;
    other->len = 0x1000;
    other->key = 30;
    std::vector<MappedFilePtr> maps = {oldMap, other, newMap};

    if (KProcess::selectMappedFileForRangeForTest(maps, 0x1100, 0x20) != newMap) {
        testFail("mapped range chooses newest overlapping mapping");
    }
    if (KProcess::selectMappedFileForRangeForTest(maps, 0x1ff0, 0x20)) {
        testFail("mapped range rejects crossing mapping end");
    }
    if (KProcess::selectMappedFileForRangeForTest(maps, 0x1100, 0)) {
        testFail("mapped range rejects empty block");
    }
    if (KProcess::selectMappedFileForRangeForTest(maps, 0xfffffff0, 0x40)) {
        testFail("mapped range rejects address overflow");
    }
}

#ifdef BOXEDWINE_MULTI_THREADED
EM_JS(S32, testWasmJitMtRuntimeGroupSlotIsClear, (S32 slot), {
    return slot >= 0 && slot < wasmTable.length && wasmTable.get(slot) === null;
});

EM_JS(S32, testWasmJitMtCallRuntimeGroupSlot, (S32 slot, S32* destination), {
    HEAP32[destination >> 2] = 0;
    if (slot < 0 || slot >= wasmTable.length) {
        return 0;
    }
    var fn = wasmTable.get(slot);
    if (!fn) {
        return 0;
    }
    fn(destination, 0);
    return 1;
});

EM_JS(U32, testWasmJitMtOomClassifierExclusions, (), {
    if (typeof globalThis.bwWasmJitBrokerIsOom !== 'function') {
        return 0;
    }
    var result = 0;
    if (!globalThis.bwWasmJitBrokerIsOom(
            new WebAssembly.CompileError('WebAssembly.Module(): out of memory'))) {
        result |= 1;
    }
    if (!globalThis.bwWasmJitBrokerIsOom(
            new Error('missing MT runtime group export b0'))) {
        result |= 2;
    }
    var table = new WebAssembly.Table({
        initial: 0,
        maximum: 0,
        element: 'anyfunc'
    });
    try {
        table.grow(1);
    } catch (error) {
        if (!globalThis.bwWasmJitBrokerIsOom(error)) {
            result |= 4;
        }
    }
    return result;
});

EM_JS(U32, testWasmJitMtRuntimeBatchStatsJs, (), {
    if (typeof Module.getWasmJitMtRuntimeBatchStats !== 'function' ||
            typeof Module._wasm_jit_mt_copy_runtime_batch_stats !== 'function') {
        return 0;
    }
    var expectedKeys = [
        'translatedFileBacked',
        'translatedAnonymous',
        'pendingEntries',
        'openBytes',
        'sealedGroups',
        'groupedModules',
        'groupedFunctions',
        'constructionAttempts',
        'constructionSuccesses',
        'maxFunctionsPerGroup',
        'countFlushes',
        'byteFlushes',
        'urgentFlushes',
        'processCapFlushes',
        'cancelledEntries',
        'permanentFailures',
        'groupedOomBlocks',
        'skippedConstructionAttempts',
        'groupInstanceCreations',
        'groupInstanceReuses',
        'groupedBrokerHits',
        'groupedLocalCompiles',
        'maxBlocks',
        'maxBatchBytes',
        'urgentPendingHits',
        'maxProcessOpenBytes',
        'averageFunctionsPerGroup'
    ];
    var result = 0;
    var nativeStats = Module.getWasmJitMtRuntimeBatchStats();
    if (nativeStats && Object.getPrototypeOf(nativeStats) === Object.prototype &&
            Object.keys(nativeStats).join('|') === expectedKeys.join('|') &&
            nativeStats.translatedFileBacked === 2 &&
            nativeStats.groupedModules === 1 &&
            nativeStats.groupedFunctions === 2 &&
            nativeStats.groupInstanceCreations === 1 &&
            nativeStats.groupInstanceReuses === 0 &&
            nativeStats.groupedBrokerHits === 0 &&
            nativeStats.groupedLocalCompiles === 1 &&
            nativeStats.averageFunctionsPerGroup === 2) {
        result |= 1;
    }

    var originalCopy = Module._wasm_jit_mt_copy_runtime_batch_stats;
    Module._wasm_jit_mt_copy_runtime_batch_stats = function(snapshot) {
        HEAPU32.fill(0, snapshot >>> 2, (snapshot + 192) >>> 2);
        HEAPU32[snapshot >>> 2] = 1;
        HEAPU32[(snapshot >>> 2) + 1] = 1;
        HEAPU32[(snapshot >>> 2) + 10] = 2;
        HEAPU32[(snapshot >>> 2) + 12] = 5;
    };
    try {
        var syntheticStats = Module.getWasmJitMtRuntimeBatchStats();
        if (syntheticStats.translatedFileBacked === 0x100000001 &&
                syntheticStats.groupedModules === 2 &&
                syntheticStats.groupedFunctions === 5 &&
                syntheticStats.averageFunctionsPerGroup === 2.5) {
            result |= 2;
        }
    } finally {
        Module._wasm_jit_mt_copy_runtime_batch_stats = originalCopy;
    }

    var originalMalloc = Module._malloc;
    var originalFree = Module._free;
    var allocated = 0;
    var freed = 0;
    Module._malloc = function(size) {
        allocated = originalMalloc(size);
        return allocated;
    };
    Module._free = function(address) {
        if (address === allocated) {
            freed += 1;
        }
        return originalFree(address);
    };
    Module._wasm_jit_mt_copy_runtime_batch_stats = function() {
        throw new Error('forced runtime batch snapshot copy failure');
    };
    try {
        Module.getWasmJitMtRuntimeBatchStats();
    } catch (error) {
        if (allocated && freed === 1 &&
                String(error).indexOf('forced runtime batch snapshot copy failure') >= 0) {
            result |= 4;
        }
    } finally {
        Module._wasm_jit_mt_copy_runtime_batch_stats = originalCopy;
        Module._malloc = originalMalloc;
        Module._free = originalFree;
    }
    var highPointer = 0x80000000;
    var runtimeStatsSource = String(Module.getWasmJitMtRuntimeBatchStats);
    if ((highPointer >>> 2) === 0x20000000 &&
            (highPointer >> 2) < 0 &&
            (runtimeStatsSource.indexOf('snapshot >>> 2') >= 0 ||
                runtimeStatsSource.indexOf('snapshot>>>2') >= 0)) {
        result |= 8;
    }
    return result;
});

struct WasmJitMtGroupedOomWorkerArgs {
    std::vector<U8> bytes;
    KMemory* memory = nullptr;
    U32 expectedIncarnation = 0;
    U32 actualIncarnation = 0;
    bool canConstructBefore = false;
    bool installed = false;
    bool canConstructAfter = false;
    WasmJitMtRuntimeGroupConstructorStatsSnapshot constructors;
    S32 completed = 0;
};

static void* testWasmJitMtGroupedOomOtherWorker(void* opaque) {
    WasmJitMtGroupedOomWorkerArgs* args = (WasmJitMtGroupedOomWorkerArgs*)opaque;
    args->actualIncarnation = wasmJitTestGetMtMemoryIncarnation(args->memory);
    args->canConstructBefore = wasmJitTestCanCurrentWorkerConstructFreshMtGroup(args->memory);
    wasmJitTestResetMtRuntimeGroupConstructorStats();
    std::vector<S32> slots;
    args->installed = wasmJitTestInstallMtRuntimeGroup(args->bytes, 2, args->memory, slots) &&
        slots.size() == 2 && slots[0] >= 0 && slots[1] >= 0;
    args->constructors = wasmJitTestGetMtRuntimeGroupConstructorStats();
    args->canConstructAfter = wasmJitTestCanCurrentWorkerConstructFreshMtGroup(args->memory);
    __atomic_store_n(&args->completed, 1, __ATOMIC_RELEASE);
    emscripten_futex_wake(&args->completed, INT_MAX);
    return nullptr;
}

static bool waitForWasmJitMtGroupedOomOtherWorker(WasmJitMtGroupedOomWorkerArgs* args) {
    double deadline = emscripten_get_now() + 5000.0;
    while (emscripten_get_now() < deadline) {
        S32 completed = __atomic_load_n(&args->completed, __ATOMIC_ACQUIRE);
        if (completed) {
            return true;
        }
        emscripten_futex_wait(&args->completed, completed, 50.0);
    }
    return false;
}

static bool runWasmJitMtGroupedOomOtherWorker(WasmJitMtGroupedOomWorkerArgs& args) {
    auto state = std::make_unique<WasmJitMtGroupedOomWorkerArgs>(args);
    WasmJitMtGroupedOomWorkerArgs* rawState = state.get();
    wasmJitTestPrepareMtThreadStart(rawState, rawState->memory);
    pthread_t thread;
    if (pthread_create(&thread, nullptr, testWasmJitMtGroupedOomOtherWorker, rawState)) {
        wasmJitTestCancelMtThreadStart(rawState);
        return false;
    }
    state.release();
    if (!waitForWasmJitMtGroupedOomOtherWorker(rawState)) {
        wasmJitTestCancelMtThreadStart(rawState);
        // A delayed worker can still start, so keep all heap-owned inputs alive.
        pthread_detach(thread);
        return false;
    }
    if (pthread_join(thread, nullptr)) {
        // Completion precedes the worker epilogue, so retain state unless joined.
        pthread_detach(thread);
        return false;
    }
    args = *rawState;
    delete rawState;
    return args.completed != 0;
}

struct WasmJitMtGroupWorkerStatsRaceArgs {
    S32 completed = 0;
};

struct WasmJitMtGroupWorkerStatsSnapshotArgs {
    WasmJitMtRuntimeBatchStatsSnapshot snapshot = {};
    S32 started = 0;
    S32 completed = 0;
};

static void* testWasmJitMtRecordGroupWorkerEvent(void* opaque) {
    WasmJitMtGroupWorkerStatsRaceArgs* args =
        (WasmJitMtGroupWorkerStatsRaceArgs*)opaque;
    wasm_jit_mt_record_group_worker_event(1);
    __atomic_store_n(&args->completed, 1, __ATOMIC_RELEASE);
    emscripten_futex_wake(&args->completed, INT_MAX);
    return nullptr;
}

static void* testWasmJitMtCopyGroupWorkerStats(void* opaque) {
    WasmJitMtGroupWorkerStatsSnapshotArgs* args =
        (WasmJitMtGroupWorkerStatsSnapshotArgs*)opaque;
    __atomic_store_n(&args->started, 1, __ATOMIC_RELEASE);
    emscripten_futex_wake(&args->started, INT_MAX);
    wasm_jit_mt_copy_runtime_batch_stats(&args->snapshot);
    __atomic_store_n(&args->completed, 1, __ATOMIC_RELEASE);
    emscripten_futex_wake(&args->completed, INT_MAX);
    return nullptr;
}

static bool waitForWasmJitMtGroupWorkerStatsValue(S32* value, double timeout) {
    double deadline = emscripten_get_now() + timeout;
    while (emscripten_get_now() < deadline) {
        S32 current = __atomic_load_n(value, __ATOMIC_ACQUIRE);
        if (current) {
            return true;
        }
        emscripten_futex_wait(value, current, 25.0);
    }
    return false;
}

static bool testWasmJitMtGroupWorkerStatsSnapshotCoherent(KMemory* memory) {
    WasmJitMtGroupWorkerStatsRaceArgs eventArgs;
    WasmJitMtGroupWorkerStatsSnapshotArgs snapshotArgs;
    wasmJitTestPauseNextMtGroupWorkerEvent();
    wasmJitTestPrepareMtThreadStart(&eventArgs, memory);
    pthread_t eventThread;
    if (pthread_create(&eventThread, nullptr,
            testWasmJitMtRecordGroupWorkerEvent, &eventArgs)) {
        wasmJitTestCancelMtThreadStart(&eventArgs);
        wasmJitTestReleaseMtGroupWorkerEvent();
        return false;
    }
    bool paused = wasmJitTestWaitForMtGroupWorkerEventPause();
    if (paused) {
        wasmJitTestPrepareMtThreadStart(&snapshotArgs, memory);
    }
    pthread_t snapshotThread;
    bool snapshotCreated = paused && !pthread_create(
        &snapshotThread, nullptr, testWasmJitMtCopyGroupWorkerStats, &snapshotArgs);
    if (paused && !snapshotCreated) {
        wasmJitTestCancelMtThreadStart(&snapshotArgs);
    }
    bool snapshotStarted = snapshotCreated &&
        waitForWasmJitMtGroupWorkerStatsValue(&snapshotArgs.started, 2000.0);
    if (snapshotStarted) {
        waitForWasmJitMtGroupWorkerStatsValue(&snapshotArgs.completed, 250.0);
    }
    wasmJitTestReleaseMtGroupWorkerEvent();
    bool eventJoined = pthread_join(eventThread, nullptr) == 0;
    bool snapshotJoined = snapshotCreated &&
        pthread_join(snapshotThread, nullptr) == 0;
    return paused && snapshotStarted && eventJoined && snapshotJoined &&
        eventArgs.completed && snapshotArgs.completed &&
        snapshotArgs.snapshot.groupInstanceCreations ==
            snapshotArgs.snapshot.groupedLocalCompiles +
                snapshotArgs.snapshot.groupedBrokerHits;
}

void testWasmJitMtRuntimeGrouping() {
    S32 oldEnabled = wasmJitTestSetMtModuleBrokerEnabled(1);
    struct RestoreMtRuntimeGroupTest {
        S32 oldEnabled;
        ~RestoreMtRuntimeGroupTest() {
            wasmJitTestFailMtRuntimeGroupAfterSlots(-1);
            wasmJitTestResetMtRuntimeBatching();
            wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        }
    } restore{oldEnabled};

    testNewInstruction(0);
    wasmJitTestResetMtRuntimeBatching();
    if (!wasmJitTestAddMtSealedRequestForStats(testContext().memory, 76, 2) ||
            !wasmJitTestAddMtSealedRequestForStats(testContext().memory, 76, 3)) {
        testFail("MT WASM runtime batch creates two synthetic sealed requests");
        return;
    }
    WasmJitMtRuntimeBatchStatsSnapshot sealedStats = {};
    wasm_jit_mt_copy_runtime_batch_stats(&sealedStats);
    if (sealedStats.pendingEntries != 5 || sealedStats.openBytes != 0 ||
            sealedStats.sealedGroups != 2) {
        testFail("MT WASM runtime batch counts distinct same-key sealed requests");
        return;
    }
    wasmJitTestResetMtRuntimeBatching();
    WasmJitBatchLimits limits;
    limits.maxBlocks = 2;
    limits.maxBatchBytes = 1024 * 1024;
    limits.urgentPendingHits = 8;
    limits.maxProcessOpenBytes = 4 * 1024 * 1024;
    wasmJitTestSetMtBatchLimits(limits);
    wasmJitTestSetMtMappedFileKeyOverride(77);
    wasmJitTestEnableMtRuntimeBatching(true);

    U32 addresses[2];
    for (U32 i = 0; i < 2; ++i) {
        addresses[i] = testContext().codeIp;
        testPushCode8(i == 0 ? 0x40 : 0x41); // inc eax / inc ecx
        testPushCode8(0xcd);
        testPushCode8(0x97);
    }
    auto compile = [&](U32 address) {
        DecodedOp* op = testContext().cpu->getOp(address, 0);
        startNewJIT(testContext().cpu, address, op);
        return op;
    };
    U32 constructionCount = wasmJitTestMtRuntimeGroupConstructionCount();
    DecodedOp* first = compile(addresses[0]);
    if (!(first->flags2 & OP_FLAG2_WASM_JIT_PENDING) || first->pfnJitCode ||
            wasmJitTestMtPendingCount() != 1 || wasmJitTestMtRuntimeModuleCount() != 0 ||
            wasmJitTestMtRuntimeGroupConstructionCount() != constructionCount) {
        WasmJitMtRuntimeStatsSnapshot failedStats = wasmJitTestGetMtRuntimeStats();
        WasmJitMtBrokerModuleRef failedRef = wasmJitTestGetMtBrokerSlotRef(
            (int)(uintptr_t)first->pfnJitCode);
        testFail("MT WASM runtime batch first mapped block remains pending: pendingFlag=%u jit=%u pending=%u modules=%u constructions=%u/%u translated=%llu/%llu moduleId=%u owner=%u/%u",
            (first->flags2 & OP_FLAG2_WASM_JIT_PENDING) != 0,
            first->pfnJitCode != nullptr, wasmJitTestMtPendingCount(),
            wasmJitTestMtRuntimeModuleCount(),
            wasmJitTestMtRuntimeGroupConstructionCount(), constructionCount,
            failedStats.translatedFileBacked, failedStats.translatedAnonymous,
            failedRef.moduleId, failedRef.memoryId, failedRef.memoryIncarnation);
        return;
    }

    DecodedOp* second = compile(addresses[1]);
    if (!first->pfnJitCode || !second->pfnJitCode || first->pfnJitCode == second->pfnJitCode ||
            (first->flags2 & OP_FLAG2_WASM_JIT_PENDING) || (second->flags2 & OP_FLAG2_WASM_JIT_PENDING)) {
        testFail("MT WASM runtime batch publishes distinct callable slots");
        return;
    }
    WasmJitMtRuntimeStatsSnapshot stats = wasmJitTestGetMtRuntimeStats();
    if (wasmJitTestMtPendingCount() != 0 || wasmJitTestMtRuntimeGroupCount() != 1 ||
            wasmJitTestMtRuntimeModuleCount() != 1 ||
            wasmJitTestMtRuntimeGroupConstructionCount() != constructionCount + 1 ||
            stats.translatedFileBacked != 2 || stats.groupedModules != 1 ||
            stats.groupedFunctions != 2 || stats.countFlushes != 1 ||
            stats.constructionAttempts != 1 || stats.constructionSuccesses != 1 ||
            stats.maxFunctionsPerGroup != 2) {
        testFail("MT WASM runtime batch creates one two-function group");
        return;
    }
    WasmJitMtRuntimeBatchStatsSnapshot batchStats = {};
    wasm_jit_mt_copy_runtime_batch_stats(&batchStats);
    if (batchStats.translatedFileBacked != 2 || batchStats.groupedModules != 1 ||
            batchStats.groupedFunctions != 2 || batchStats.constructionAttempts != 1 ||
            batchStats.constructionSuccesses != 1 || batchStats.maxFunctionsPerGroup != 2 ||
            batchStats.countFlushes != 1 || batchStats.pendingEntries != 0 ||
            batchStats.openBytes != 0 || batchStats.sealedGroups != 0 ||
            batchStats.groupInstanceCreations != 1 || batchStats.groupInstanceReuses != 0 ||
            batchStats.groupedBrokerHits != 0 || batchStats.groupedLocalCompiles != 1) {
        testFail("MT WASM runtime batch exported snapshot matches grouped publication");
        return;
    }
    if (testWasmJitMtRuntimeBatchStatsJs() != 15) {
        testFail("MT WASM runtime batch JavaScript diagnostics shape and U64 decoding");
        return;
    }
    if (!wasmJitTestBrokerStatsSnapshotFailureCompletes()) {
        testFail("MT WASM broker stats rejects failed asynchronous runtime snapshot");
        return;
    }
    if (!wasmJitTestBrokerIncompleteStats(1, 0, 0, 1)) {
        testFail("MT WASM broker stats retain exact grouped totals with missing workers");
        return;
    }
    if (!testWasmJitMtGroupWorkerStatsSnapshotCoherent(testContext().memory)) {
        testFail("MT WASM broker grouped totals use one coherent event boundary");
        return;
    }

    testContext().cpu->reg[0].u32 = 0;
    wasmStartJITOp(testContext().cpu, first);
    if (testContext().cpu->reg[0].u32 != 1) {
        testFail("MT WASM runtime batch first block executes");
    }
    testContext().cpu->reg[1].u32 = 0;
    wasmStartJITOp(testContext().cpu, second);
    if (testContext().cpu->reg[1].u32 != 1) {
        testFail("MT WASM runtime batch second block executes");
    }

    wasmJitTestResetMtRuntimeBatching();
    testNewInstruction(0);
    wasmJitTestSetMtBatchLimits(limits);
    wasmJitTestEnableMtRuntimeBatching(true);
    for (U32 i = 0; i < 2; ++i) {
        addresses[i] = testContext().codeIp;
        testPushCode8(0x40 + i);
        testPushCode8(0xcd);
        testPushCode8(0x97);
    }
    wasmJitTestSetMtMappedFileKeyOverride(77);
    first = compile(addresses[0]);
    wasmJitTestSetMtMappedFileKeyOverride(78);
    second = compile(addresses[1]);
    if (!(first->flags2 & OP_FLAG2_WASM_JIT_PENDING) || !(second->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            wasmJitTestMtPendingCount() != 2 || wasmJitTestMtPendingRawBytes() == 0 ||
            wasmJitTestMtRuntimeModuleCount() != 0) {
        testFail("MT WASM runtime batches isolate mapped-file keys");
    }

    wasmJitTestResetMtRuntimeBatching();
    testNewInstruction(0);
    wasmJitTestSetMtBatchLimits(limits);
    wasmJitTestSetMtMappedFileKeyOverride(0);
    wasmJitTestEnableMtRuntimeBatching(true);
    U32 anonymousAddress = testContext().codeIp;
    testPushCode8(0x40);
    testPushCode8(0x41);
    testPushCode8(0x42);
    testPushCode8(0x43);
    testPushCode8(0xcd);
    testPushCode8(0x97);
    DecodedOp* anonymous = compile(anonymousAddress);
    stats = wasmJitTestGetMtRuntimeStats();
    if (!anonymous->pfnJitCode || (anonymous->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            wasmJitTestMtPendingCount() != 0 || wasmJitTestMtRuntimeGroupCount() != 0 ||
            stats.translatedAnonymous != 1 || stats.standaloneModules != 1) {
        testFail("MT WASM fresh anonymous four-op block stays standalone");
    }

    std::vector<U8> directFirst = makeMergeInput(0x11223344);
    std::vector<U8> directSecond = makeMergeInput(0x55667788);
    std::vector<WasmJitMergeInput> inputs = {{&directFirst}, {&directSecond}};
    std::vector<U8> merged;
    BString error;
    if (!wasmJitMergeModules(inputs, merged, error)) {
        testFail("MT WASM runtime group rollback merge setup: %s", error.c_str());
        return;
    }
    S32 failedFirstSlot = (S32)(uintptr_t)anonymous->pfnJitCode + 1;
    wasmJitTestFailMtRuntimeGroupAfterSlots(1);
    std::vector<S32> failedSlots;
    if (wasmJitTestInstallMtRuntimeGroup(merged, 2, testContext().memory, failedSlots)) {
        testFail("MT WASM runtime group reports partial install failure");
    }
    if (failedSlots.size() != 2 || failedSlots[0] != -1 || failedSlots[1] != -1) {
        testFail("MT WASM runtime group resets failed output slots");
    }
    if (wasmJitTestLazyInstallMtSlot(failedFirstSlot)) {
        testFail("MT WASM runtime group does not publish failed slot metadata");
    }
    if (!testWasmJitMtRuntimeGroupSlotIsClear(failedFirstSlot)) {
        testFail("MT WASM runtime group clears partial table installation");
    }
    wasmJitTestFailMtRuntimeGroupAfterSlots(-1);
    wasmJitTestResetMtRuntimeBatching();
    batchStats = {};
    wasm_jit_mt_copy_runtime_batch_stats(&batchStats);
    if (batchStats.maxBlocks != 64 || batchStats.maxBatchBytes != 512 * 1024 ||
            batchStats.urgentPendingHits != 8 ||
            batchStats.maxProcessOpenBytes != 4 * 1024 * 1024) {
        testFail("MT WASM runtime batch exported snapshot restores production limits");
    }
}

void testWasmJitMtGroupedOomBlock() {
    S32 oldEnabled = wasmJitTestSetMtModuleBrokerEnabled(1);
    struct RestoreMtGroupedOomTest {
        S32 oldEnabled;
        KMemory* memory;

        ~RestoreMtGroupedOomTest() {
            wasmJitTestSetMtBrokerCompileOom(0, false);
            wasmJitTestForceNextMtRuntimeGroupInstanceOom(0);
            wasmJitTestFailMtRuntimeGroupAfterSlots(-1);
            wasmJitTestResetMtRuntimeGroupConstructorStats();
            wasmJitTestResetMtRuntimeBatching();
            wasmJitTestSetMtModuleBrokerEnabled(1);
            wasmJitTestInvalidateMtBrokerMemory(memory);
            wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        }
    } restore{oldEnabled, testContext().memory};

    if (testWasmJitMtOomClassifierExclusions() != 7) {
        testFail("MT WASM grouped OOM classifier excludes non-constructor failures");
    }

    testNewInstruction(0);
    wasmJitTestResetMtRuntimeBatching();
    WasmJitBatchLimits limits;
    limits.maxBlocks = 2;
    limits.maxBatchBytes = 1024 * 1024;
    limits.urgentPendingHits = 8;
    limits.maxProcessOpenBytes = 4 * 1024 * 1024;
    wasmJitTestSetMtBatchLimits(limits);
    wasmJitTestEnableMtRuntimeBatching(true);

    std::vector<U8> reusableFirst = makeMergeInput(0x11223344);
    std::vector<U8> reusableSecond = makeMergeInput(0x55667788);
    std::vector<WasmJitMergeInput> reusableInputs = {{&reusableFirst}, {&reusableSecond}};
    std::vector<U8> reusableGroup;
    BString mergeError;
    if (!wasmJitMergeModules(reusableInputs, reusableGroup, mergeError)) {
        testFail("MT WASM grouped OOM reusable group merge setup: %s", mergeError.c_str());
        return;
    }

    std::vector<S32> reusableSlots;
    if (!wasmJitTestInstallMtRuntimeGroup(
            reusableGroup, 2, testContext().memory, reusableSlots) ||
            reusableSlots.size() != 2 || reusableSlots[0] < 0 || reusableSlots[1] < 0) {
        testFail("MT WASM grouped OOM publishes a reusable grouped module");
        return;
    }
    WasmJitMtBrokerModuleRef reusableRef =
        wasmJitTestGetMtBrokerSlotRef(reusableSlots[0]);
    if (!reusableRef.moduleId ||
            !wasmJitTestDropCurrentWorkerMtRuntimeGroupInstance(reusableSlots[0]) ||
            !wasmJitTestClearCurrentWorkerMtSlot(reusableSlots[0])) {
        testFail("MT WASM grouped OOM prepares a lazy broker-cache install");
        return;
    }

    auto addBlock = [&](U32 registerIndex, U32 opCount) {
        U32 address = testContext().codeIp;
        for (U32 i = 0; i < opCount; ++i) {
            testPushCode8(0x40 + ((registerIndex + i) & 3));
        }
        testPushCode8(0xcd);
        testPushCode8(0x97);
        return address;
    };
    auto addPair = [&](U32 registerIndex) {
        std::array<U32, 2> result;
        result[0] = addBlock(registerIndex, 1);
        result[1] = addBlock(registerIndex + 1, 1);
        return result;
    };
    auto compile = [&](U32 address) {
        DecodedOp* op = testContext().cpu->getOp(address, 0);
        startNewJIT(testContext().cpu, address, op);
        return op;
    };
    auto compilePair = [&](const std::array<U32, 2>& addresses, S32 mappedFileKey) {
        wasmJitTestSetMtMappedFileKeyOverride(mappedFileKey);
        std::array<DecodedOp*, 2> result;
        result[0] = compile(addresses[0]);
        result[1] = compile(addresses[1]);
        return result;
    };
    auto isInterpreted = [&](DecodedOp* op) {
        return op && !op->pfnJitCode && !(op->flags & OP_FLAG_JIT) &&
            !(op->flags2 & OP_FLAG2_WASM_JIT_PENDING) && op->pfn;
    };

    std::array<U32, 2> compileOomAddresses = addPair(0);
    std::array<U32, 2> blockedAddresses = addPair(2);
    U32 anonymousAddress = addBlock(0, 4);
    std::array<U32, 2> permanentFailureAddresses = addPair(0);
    std::array<U32, 2> afterPermanentFailureAddresses = addPair(2);
    std::array<U32, 2> instanceOomAddresses = addPair(0);
    std::array<U32, 2> instanceBlockedAddresses = addPair(2);

    U32 blockedMemoryId = (U32)(uintptr_t)testContext().memory;
    U32 blockedIncarnation = wasmJitTestGetMtMemoryIncarnation(testContext().memory);
    U32 syntheticIncarnation = blockedIncarnation ^ 0x80000000u;
    if (!syntheticIncarnation || syntheticIncarnation == blockedIncarnation) {
        syntheticIncarnation = blockedIncarnation == 1 ? 2 : 1;
    }
    wasmJitTestResetMtRuntimeGroupConstructorStats();
    wasmJitTestSetMtModuleBrokerEnabled(0);
    wasmJitTestSetMtBrokerCompileOom(0, true);
    std::array<DecodedOp*, 2> compileOomOps = compilePair(compileOomAddresses, 201);
    wasmJitTestSetMtBrokerCompileOom(0, false);
    wasmJitTestSetMtModuleBrokerEnabled(1);

    WasmJitMtRuntimeStatsSnapshot stats = wasmJitTestGetMtRuntimeStats();
    WasmJitMtRuntimeGroupConstructorStatsSnapshot constructors =
        wasmJitTestGetMtRuntimeGroupConstructorStats();
    if (!isInterpreted(compileOomOps[0]) || !isInterpreted(compileOomOps[1]) ||
            wasmJitTestMtPendingCount() != 0 || wasmJitTestMtOpenCount() != 0 ||
            wasmJitTestMtSealedCount() != 0 ||
            stats.constructionAttempts != 1 || stats.groupedOomBlocks != 1 ||
            stats.skippedConstructionAttempts != 0 || stats.permanentFailures != 0 ||
            constructors.moduleAttempts != 1 || constructors.instanceAttempts != 0 ||
            !wasmJitTestIsCurrentWorkerFreshMtGroupBlocked(
                blockedMemoryId, blockedIncarnation) ||
            wasmJitTestIsCurrentWorkerFreshMtGroupBlocked(
                blockedMemoryId, syntheticIncarnation) ||
            wasmJitTestCanCurrentWorkerConstructFreshMtGroup(testContext().memory)) {
        testFail("MT WASM grouped compile OOM blocks one exact Worker owner incarnation");
    }

    std::array<DecodedOp*, 2> blockedOps = compilePair(blockedAddresses, 202);
    stats = wasmJitTestGetMtRuntimeStats();
    constructors = wasmJitTestGetMtRuntimeGroupConstructorStats();
    if (!isInterpreted(blockedOps[0]) || !isInterpreted(blockedOps[1]) ||
            wasmJitTestMtPendingCount() != 0 || wasmJitTestMtOpenCount() != 0 ||
            wasmJitTestMtSealedCount() != 0 ||
            stats.constructionAttempts != 1 || stats.groupedOomBlocks != 1 ||
            stats.skippedConstructionAttempts != 1 || stats.permanentFailures != 0 ||
            constructors.moduleAttempts != 1 || constructors.instanceAttempts != 0) {
        testFail("MT WASM grouped OOM skips the next fresh group without constructors");
    }

    WasmJitMtBrokerStatsSnapshot beforeCacheHit =
        wasmJitTestGetMtBrokerStats(testContext().memory);
    S32 cacheValue = 0;
    bool cacheInstalled = wasmJitTestLazyInstallMtSlot(reusableSlots[0]);
    bool cacheExecuted = cacheInstalled &&
        testWasmJitMtCallRuntimeGroupSlot(reusableSlots[0], &cacheValue) != 0;
    WasmJitMtBrokerStatsSnapshot afterCacheHit =
        wasmJitTestGetMtBrokerStats(testContext().memory);
    constructors = wasmJitTestGetMtRuntimeGroupConstructorStats();
    if (!cacheExecuted || cacheValue != 0x11223344 ||
            afterCacheHit.brokerHits != beforeCacheHit.brokerHits + 1 ||
            constructors.moduleAttempts != 1 || constructors.instanceAttempts != 1) {
        testFail("MT WASM grouped OOM preserves lazy grouped broker-cache hits");
    }

    WasmJitMtGroupedOomWorkerArgs workerArgs;
    workerArgs.bytes = reusableGroup;
    workerArgs.memory = testContext().memory;
    workerArgs.expectedIncarnation = blockedIncarnation;
    if (!runWasmJitMtGroupedOomOtherWorker(workerArgs) ||
            workerArgs.actualIncarnation != workerArgs.expectedIncarnation ||
            !workerArgs.canConstructBefore || !workerArgs.installed ||
            !workerArgs.canConstructAfter ||
            workerArgs.constructors.moduleAttempts != 1 ||
            workerArgs.constructors.instanceAttempts != 1 ||
            wasmJitTestCanCurrentWorkerConstructFreshMtGroup(testContext().memory)) {
        testFail("MT WASM grouped OOM block stays on one physical Worker");
    }

    wasmJitTestSetMtMappedFileKeyOverride(0);
    DecodedOp* anonymous = compile(anonymousAddress);
    stats = wasmJitTestGetMtRuntimeStats();
    if (!anonymous->pfnJitCode || (anonymous->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            stats.translatedAnonymous != 1 || stats.standaloneModules != 1) {
        testFail("MT WASM grouped OOM leaves fresh anonymous standalone JIT usable");
    }

    wasmJitTestInvalidateMtBrokerMemory(testContext().memory);
    U32 replacementIncarnation = wasmJitTestGetMtMemoryIncarnation(testContext().memory);
    if (!blockedIncarnation || !replacementIncarnation ||
            replacementIncarnation == blockedIncarnation ||
            wasmJitTestIsCurrentWorkerFreshMtGroupBlocked(
                blockedMemoryId, blockedIncarnation) ||
            wasmJitTestIsCurrentWorkerFreshMtGroupBlocked(
                blockedMemoryId, replacementIncarnation) ||
            !wasmJitTestCanCurrentWorkerConstructFreshMtGroup(testContext().memory)) {
        testFail("MT WASM grouped OOM exact purge removes only the retired incarnation block");
    }

    wasmJitTestResetMtRuntimeBatching();
    wasmJitTestSetMtBatchLimits(limits);
    wasmJitTestEnableMtRuntimeBatching(true);
    wasmJitTestResetMtRuntimeGroupConstructorStats();
    wasmJitTestFailMtRuntimeGroupAfterSlots(1);
    std::array<DecodedOp*, 2> permanentFailureOps =
        compilePair(permanentFailureAddresses, 210);
    wasmJitTestFailMtRuntimeGroupAfterSlots(-1);
    stats = wasmJitTestGetMtRuntimeStats();
    if (!isInterpreted(permanentFailureOps[0]) || !isInterpreted(permanentFailureOps[1]) ||
            stats.permanentFailures == 0 || stats.groupedOomBlocks != 0 ||
            stats.skippedConstructionAttempts != 0 ||
            !wasmJitTestCanCurrentWorkerConstructFreshMtGroup(testContext().memory)) {
        testFail("MT WASM grouped permanent failure does not classify or block as OOM");
    }

    std::array<DecodedOp*, 2> afterPermanentFailureOps =
        compilePair(afterPermanentFailureAddresses, 211);
    stats = wasmJitTestGetMtRuntimeStats();
    if (!afterPermanentFailureOps[0]->pfnJitCode ||
            !afterPermanentFailureOps[1]->pfnJitCode ||
            stats.constructionAttempts != 2 || stats.constructionSuccesses != 1 ||
            stats.groupedModules != 1 || stats.groupedOomBlocks != 0 ||
            stats.skippedConstructionAttempts != 0) {
        testFail("MT WASM grouped permanent failure permits a later unrelated group");
    }

    wasmJitTestResetMtRuntimeBatching();
    wasmJitTestSetMtBatchLimits(limits);
    wasmJitTestEnableMtRuntimeBatching(true);
    wasmJitTestResetMtRuntimeGroupConstructorStats();
    wasmJitTestForceNextMtRuntimeGroupInstanceOom(1);
    std::array<DecodedOp*, 2> instanceOomOps = compilePair(instanceOomAddresses, 220);
    stats = wasmJitTestGetMtRuntimeStats();
    constructors = wasmJitTestGetMtRuntimeGroupConstructorStats();
    if (!isInterpreted(instanceOomOps[0]) || !isInterpreted(instanceOomOps[1]) ||
            wasmJitTestMtPendingCount() != 0 || wasmJitTestMtOpenCount() != 0 ||
            wasmJitTestMtSealedCount() != 0 ||
            stats.constructionAttempts != 1 || stats.groupedOomBlocks != 1 ||
            stats.skippedConstructionAttempts != 0 || stats.permanentFailures != 0 ||
            constructors.moduleAttempts != 1 || constructors.instanceAttempts != 1 ||
            wasmJitTestCanCurrentWorkerConstructFreshMtGroup(testContext().memory)) {
        testFail("MT WASM grouped instance OOM blocks after module acquisition");
    }

    std::array<DecodedOp*, 2> instanceBlockedOps =
        compilePair(instanceBlockedAddresses, 221);
    stats = wasmJitTestGetMtRuntimeStats();
    constructors = wasmJitTestGetMtRuntimeGroupConstructorStats();
    if (!isInterpreted(instanceBlockedOps[0]) || !isInterpreted(instanceBlockedOps[1]) ||
            wasmJitTestMtPendingCount() != 0 || wasmJitTestMtOpenCount() != 0 ||
            wasmJitTestMtSealedCount() != 0 ||
            stats.constructionAttempts != 1 || stats.groupedOomBlocks != 1 ||
            stats.skippedConstructionAttempts != 1 || stats.permanentFailures != 0 ||
            constructors.moduleAttempts != 1 || constructors.instanceAttempts != 1) {
        testFail("MT WASM grouped instance OOM blocks another fresh constructor attempt");
    }
    WasmJitMtRuntimeBatchStatsSnapshot batchStats = {};
    wasm_jit_mt_copy_runtime_batch_stats(&batchStats);
    if (batchStats.groupedOomBlocks != 1 ||
            batchStats.skippedConstructionAttempts != 1) {
        testFail("MT WASM grouped OOM exported snapshot matches final blocked sequence");
    }
}

void testWasmJitMtPendingLifecycle() {
    S32 oldEnabled = wasmJitTestSetMtModuleBrokerEnabled(1);
    struct RestoreMtPendingTest {
        S32 oldEnabled;
        ~RestoreMtPendingTest() {
            wasmJitTestCancelMtSealedEntryBeforePublish(-1);
            wasmJitTestReplaceMtMemoryBeforePublish(0, nullptr);
            wasmJitTestSetMtPendingByteChargeOverride(0);
            wasmJitTestResetMtRuntimeBatching();
            wasmJitTestSetMtModuleBrokerEnabled(oldEnabled);
        }
    } restore{oldEnabled};

    WasmJitBatchLimits productionLimits;
    auto beginPhase = [&](const WasmJitBatchLimits& limits, S32 mappedFileKey) {
        wasmJitTestCancelMtSealedEntryBeforePublish(-1);
        wasmJitTestReplaceMtMemoryBeforePublish(0, nullptr);
        wasmJitTestSetMtPendingByteChargeOverride(0);
        wasmJitTestResetMtRuntimeBatching();
        testNewInstruction(0);
        wasmJitTestSetMtBatchLimits(limits);
        wasmJitTestSetMtMappedFileKeyOverride(mappedFileKey);
        wasmJitTestEnableMtRuntimeBatching(true);
    };
    auto addBlocks = [&](U32 count) {
        std::vector<U32> addresses;
        addresses.reserve(count);
        for (U32 i = 0; i < count; ++i) {
            addresses.push_back(testContext().codeIp);
            testPushCode8(0x40 + (i & 3));
            testPushCode8(0xcd);
            testPushCode8(0x97);
        }
        return addresses;
    };
    auto compile = [&](CPU* cpu, U32 address) {
        DecodedOp* op = cpu->getOp(address, 0);
        startNewJIT(cpu, address, op);
        return op;
    };
    auto runPending = [&](CPU* cpu, U32 address, DecodedOp* op) {
        cpu->eip.u32 = address - cpu->seg[CS].address;
        cpu->nextOp = op;
        cpu->run();
    };
    auto restoreMappings = [&]() {
        constexpr U32 pagesPerSegment = 32;
        U32 segmentBytes = pagesPerSegment << K_PAGE_SHIFT;
        U32 stackBase = TEST_STACK_ADDRESS - segmentBytes;
        if (testContext().memory->mmap(testContext().thread, stackBase, segmentBytes,
                    K_PROT_WRITE | K_PROT_READ, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0) != stackBase ||
                testContext().memory->mmap(testContext().thread, TEST_CODE_ADDRESS, segmentBytes,
                    K_PROT_WRITE | K_PROT_READ | K_PROT_EXEC, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0) != TEST_CODE_ADDRESS ||
                testContext().memory->mmap(testContext().thread, TEST_HEAP_ADDRESS, segmentBytes,
                    K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0) != TEST_HEAP_ADDRESS) {
            testFail("MT WASM pending lifecycle restores mappings after memory reset");
        }
    };

    beginPhase(productionLimits, 101);
    std::vector<U32> addresses = addBlocks(64);
    std::vector<DecodedOp*> countOps;
    countOps.reserve(addresses.size());
    for (U32 address : addresses) {
        countOps.push_back(compile(testContext().cpu, address));
    }
    WasmJitMtRuntimeStatsSnapshot stats = wasmJitTestGetMtRuntimeStats();
    if (wasmJitTestMtPendingCount() != 0 || wasmJitTestMtRuntimeModuleCount() != 1 ||
            stats.countFlushes != 1 || stats.groupedFunctions != 64 ||
            !countOps.front()->pfnJitCode || !countOps.back()->pfnJitCode) {
        testFail("MT WASM pending lifecycle flushes the 64th same-mapping block");
    }

    beginPhase(productionLimits, 102);
    addresses = addBlocks(2);
    wasmJitTestSetMtPendingByteChargeOverride(300 * 1024);
    DecodedOp* byteFirst = compile(testContext().cpu, addresses[0]);
    DecodedOp* byteSecond = compile(testContext().cpu, addresses[1]);
    wasmJitTestSetMtPendingByteChargeOverride(0);
    stats = wasmJitTestGetMtRuntimeStats();
    if (!byteFirst->pfnJitCode || !(byteSecond->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            wasmJitTestMtPendingCount() != 1 || stats.byteFlushes != 1 ||
            stats.groupedModules != 1 || stats.groupedFunctions != 1) {
        testFail("MT WASM pending lifecycle flushes before a 512 KiB overflow entry");
    }

    WasmJitBatchLimits orderedFlushLimits = productionLimits;
    orderedFlushLimits.maxProcessOpenBytes = 450 * 1024;
    beginPhase(orderedFlushLimits, 110);
    addresses = addBlocks(3);
    wasmJitTestSetMtPendingByteChargeOverride(100 * 1024);
    wasmJitTestSetMtMappedFileKeyOverride(111);
    DecodedOp* oldest = compile(testContext().cpu, addresses[0]);
    wasmJitTestSetMtPendingByteChargeOverride(300 * 1024);
    wasmJitTestSetMtMappedFileKeyOverride(110);
    DecodedOp* byteBatch = compile(testContext().cpu, addresses[1]);
    wasmJitTestSetMtPendingByteChargeOverride(400 * 1024);
    DecodedOp* incoming = compile(testContext().cpu, addresses[2]);
    wasmJitTestSetMtPendingByteChargeOverride(0);
    stats = wasmJitTestGetMtRuntimeStats();
    if (!oldest->pfnJitCode || !byteBatch->pfnJitCode ||
            !(incoming->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            wasmJitTestMtPendingCount() != 1 || stats.byteFlushes != 1 ||
            stats.processCapFlushes != 1 || stats.groupedModules != 2) {
        testFail("MT WASM pending lifecycle processes ordered byte and process flush requests");
    }

    WasmJitBatchLimits processLimits = productionLimits;
    processLimits.maxBatchBytes = 8 * 1024 * 1024;
    beginPhase(processLimits, 120);
    addresses = addBlocks(2);
    wasmJitTestSetMtPendingByteChargeOverride(2 * 1024 * 1024 + 1);
    DecodedOp* processOldest = compile(testContext().cpu, addresses[0]);
    wasmJitTestSetMtMappedFileKeyOverride(121);
    DecodedOp* processNewest = compile(testContext().cpu, addresses[1]);
    wasmJitTestSetMtPendingByteChargeOverride(0);
    stats = wasmJitTestGetMtRuntimeStats();
    if (!processOldest->pfnJitCode || !(processNewest->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            wasmJitTestMtPendingCount() != 1 || stats.processCapFlushes != 1 ||
            stats.byteFlushes != 0 || stats.groupedFunctions != 1) {
        testFail("MT WASM pending lifecycle flushes the oldest mapping above the 4 MiB process cap");
    }

    beginPhase(productionLimits, 122);
    addresses = addBlocks(9);
    wasmJitTestSetMtPendingByteChargeOverride(511 * 1024);
    std::vector<DecodedOp*> productionCapOps;
    productionCapOps.reserve(addresses.size());
    for (U32 i = 0; i < addresses.size(); ++i) {
        wasmJitTestSetMtMappedFileKeyOverride(122 + i);
        productionCapOps.push_back(compile(testContext().cpu, addresses[i]));
    }
    wasmJitTestSetMtPendingByteChargeOverride(0);
    stats = wasmJitTestGetMtRuntimeStats();
    if (!productionCapOps.front()->pfnJitCode ||
            !(productionCapOps.back()->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            wasmJitTestMtPendingCount() != 8 || stats.processCapFlushes != 1 ||
            stats.byteFlushes != 0 || stats.groupedFunctions != 1) {
        testFail("MT WASM pending lifecycle enforces the 4 MiB cap with production batch limits");
    }

    beginPhase(productionLimits, 130);
    addresses = addBlocks(1);
    DecodedOp* urgent = compile(testContext().cpu, addresses[0]);
    U32 translations = wasmJitTestMtTranslationCount();
    startNewJIT(testContext().cpu, addresses[0], urgent);
    if (wasmJitTestMtTranslationCount() != translations || wasmJitTestMtPendingCount() != 1) {
        testFail("MT WASM pending lifecycle does not translate or enqueue a pending block twice");
    }
    urgent->runCount = 0xff;
    for (U32 i = 0; i < 7; ++i) {
        runPending(testContext().cpu, addresses[0], urgent);
    }
    if (!(urgent->flags2 & OP_FLAG2_WASM_JIT_PENDING) || urgent->pfnJitCode ||
            urgent->runCount != 0xff || wasmJitTestMtTranslationCount() != translations ||
            wasmJitTestMtPendingCount() != 1) {
        testFail("MT WASM pending lifecycle saturates runCount without retranslating before hit eight");
    }
    runPending(testContext().cpu, addresses[0], urgent);
    stats = wasmJitTestGetMtRuntimeStats();
    if (!urgent->pfnJitCode || (urgent->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            urgent->runCount != 0xff || wasmJitTestMtTranslationCount() != translations ||
            wasmJitTestMtPendingCount() != 0 || stats.urgentFlushes != 1 ||
            stats.groupedModules != 1 || stats.groupedFunctions != 1) {
        testFail("MT WASM pending lifecycle flushes one group on the eighth pending hit: jit=%d pendingFlag=%d runCount=%u translations=%u/%u pending=%u urgent=%llu groups=%llu functions=%llu",
            urgent->pfnJitCode != nullptr, (urgent->flags2 & OP_FLAG2_WASM_JIT_PENDING) != 0,
            urgent->runCount, wasmJitTestMtTranslationCount(), translations,
            wasmJitTestMtPendingCount(), stats.urgentFlushes, stats.groupedModules, stats.groupedFunctions);
    }

    beginPhase(productionLimits, 140);
    addresses = addBlocks(2);
    DecodedOp* generic = compile(testContext().cpu, addresses[0]);
    {
        std::unique_lock<std::recursive_mutex> lock(testContext().memory->mutex);
        jitCodeInvalidated(testContext().memory, {generic}, {});
    }
    if ((generic->flags2 & OP_FLAG2_WASM_JIT_PENDING) || wasmJitTestMtPendingCount() != 0) {
        testFail("MT WASM pending lifecycle generic invalidation cancels an open entry");
    }
    DecodedOp* removed = compile(testContext().cpu, addresses[1]);
    testContext().memory->removeCodeBlock(addresses[1], removed, false);
    if (wasmJitTestMtPendingCount() != 0) {
        testFail("MT WASM pending lifecycle removeCodeBlock cancels an open entry");
    }

    WasmJitBatchLimits pairLimits = productionLimits;
    pairLimits.maxBlocks = 2;
    beginPhase(pairLimits, 150);
    addresses = addBlocks(2);
    DecodedOp* cancelledSealed = compile(testContext().cpu, addresses[0]);
    wasmJitTestCancelMtSealedEntryBeforePublish(0);
    DecodedOp* publishedPeer = compile(testContext().cpu, addresses[1]);
    if (cancelledSealed->pfnJitCode || (cancelledSealed->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            !publishedPeer->pfnJitCode || wasmJitTestMtPendingCount() != 0) {
        testFail("MT WASM pending lifecycle cancels a sealed entry before publication");
    }

    beginPhase(pairLimits, 151);
    addresses = addBlocks(2);
    U32 lateOldIncarnation = wasmJitTestGetMtMemoryIncarnation(testContext().memory);
    compile(testContext().cpu, addresses[0]);
    DecodedOp* replacement = nullptr;
    wasmJitTestReplaceMtMemoryBeforePublish(addresses[0], &replacement);
    compile(testContext().cpu, addresses[1]);
    U32 lateReplacementIncarnation = wasmJitTestGetMtMemoryIncarnation(testContext().memory);
    stats = wasmJitTestGetMtRuntimeStats();
    if (!replacement || testContext().cpu->getOp(addresses[0], 0) != replacement ||
            replacement->pfnJitCode || (replacement->flags & OP_FLAG_JIT) ||
            (replacement->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            wasmJitTestMtPendingCount() != 0 ||
            !lateOldIncarnation || !lateReplacementIncarnation ||
            lateOldIncarnation == lateReplacementIncarnation ||
            stats.groupedModules != 0 || stats.groupedFunctions != 0 ||
            stats.constructionSuccesses != 0) {
        testFail("MT WASM pending lifecycle rejects old-incarnation state after late replacement: replacement=%d same=%d jit=%d pendingFlag=%d pending=%u incarnations=%u/%u groups=%llu functions=%llu successes=%llu",
            replacement != nullptr,
            replacement && testContext().cpu->getOp(addresses[0], 0) == replacement,
            replacement && replacement->pfnJitCode != nullptr,
            replacement && (replacement->flags2 & OP_FLAG2_WASM_JIT_PENDING) != 0,
            wasmJitTestMtPendingCount(), lateOldIncarnation, lateReplacementIncarnation,
            stats.groupedModules, stats.groupedFunctions, stats.constructionSuccesses);
    }

    beginPhase(productionLimits, 160);
    addresses = addBlocks(2);
    wasmJitTestSetMtMappedFileKeyOverride(160);
    compile(testContext().cpu, addresses[0]);
    wasmJitTestSetMtMappedFileKeyOverride(161);
    compile(testContext().cpu, addresses[1]);
    testContext().memory->clearOpCache();
    if (wasmJitTestMtPendingCount() != 0) {
        testFail("MT WASM pending lifecycle clearOpCache cancels every pending entry");
    }

    beginPhase(productionLimits, 170);
    addresses = addBlocks(2);
    U32 oldIncarnation = wasmJitTestGetMtMemoryIncarnation(testContext().memory);
    wasmJitTestSetMtMappedFileKeyOverride(170);
    compile(testContext().cpu, addresses[0]);
    wasmJitTestSetMtMappedFileKeyOverride(171);
    compile(testContext().cpu, addresses[1]);
    testContext().memory->execvReset(false);
    bool execCancelled = wasmJitTestMtPendingCount() == 0;
    restoreMappings();
    U32 replacementIncarnation = wasmJitTestGetMtMemoryIncarnation(testContext().memory);
    replacement = testContext().cpu->getOp(addresses[0], 0);
    if (!execCancelled || !oldIncarnation || !replacementIncarnation ||
            replacementIncarnation == oldIncarnation || replacement->pfnJitCode ||
            (replacement->flags2 & OP_FLAG2_WASM_JIT_PENDING)) {
        testFail("MT WASM pending lifecycle exec retires old pending work before replacement");
    }

    beginPhase(productionLimits, 180);
    addresses = addBlocks(2);
    wasmJitTestSetMtMappedFileKeyOverride(180);
    compile(testContext().cpu, addresses[0]);
    wasmJitTestSetMtMappedFileKeyOverride(181);
    compile(testContext().cpu, addresses[1]);
    testContext().memory->cleanup();
    bool cleanupCancelled = wasmJitTestMtPendingCount() == 0;
    testContext().memory->execvReset(true);
    restoreMappings();
    if (!cleanupCancelled) {
        testFail("MT WASM pending lifecycle cleanup cancels every pending entry");
    }

    beginPhase(productionLimits, 190);
    addresses = addBlocks(2);
    DecodedOp* parentPending = compile(testContext().cpu, addresses[0]);
    KProcessPtr cloneProcess = KProcess::create();
    KMemory* cloneMemory = KMemory::create(cloneProcess.get());
    cloneProcess->memory = cloneMemory;
    cloneMemory->clone(testContext().memory, true);
    KThread* cloneThread = cloneProcess->createThread();
    cloneThread->cpu->clone(testContext().cpu);
    KThread::setCurrentThread(cloneThread);
    DecodedOp* clonePending = compile(cloneThread->cpu, addresses[1]);
    if (!(parentPending->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            !(clonePending->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            wasmJitTestMtPendingCount() != 2) {
        testFail("MT WASM pending lifecycle keeps CLONE_VM wrappers in separate batches");
    }
    clonePending->runCount = 0xff;
    for (U32 i = 0; i < 8; ++i) {
        runPending(cloneThread->cpu, addresses[1], clonePending);
    }
    if (!clonePending->pfnJitCode || clonePending->pfn != cloneProcess->startJITOp ||
            !(parentPending->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            parentPending->pfnJitCode || wasmJitTestMtPendingCount() != 1) {
        testFail("MT WASM pending lifecycle preserves wrapper-local fallback and publication: cloneJit=%d clonePfn=%d parentPending=%d parentJit=%d pending=%u",
            clonePending->pfnJitCode != nullptr, clonePending->pfn == cloneProcess->startJITOp,
            (parentPending->flags2 & OP_FLAG2_WASM_JIT_PENDING) != 0,
            parentPending->pfnJitCode != nullptr, wasmJitTestMtPendingCount());
    }
    cloneMemory->removeCodeBlock(addresses[1], clonePending, false);
    cloneMemory->execvReset(true);
    KThread::setCurrentThread(testContext().thread);
    testContext().memory->removeCodeBlock(addresses[0], parentPending, false);
}
#else
void testWasmJitRuntimeGrouping() {
    if (!wasmJitTestRelocAllocationTransfer()) {
        testFail("cached persistence adopts lookup relocation allocation");
    }

    testNewInstruction(0);
    wasmJitTestResetRuntimeBatching();
    struct ResetBatching {
        ~ResetBatching() { wasmJitTestResetRuntimeBatching(); }
    } reset;
    WasmJitBatchLimits limits;
    limits.maxBlocks = 2;
    limits.maxBatchBytes = 1024 * 1024;
    limits.urgentPendingHits = 8;
    limits.maxProcessOpenBytes = 4 * 1024 * 1024;
    wasmJitTestSetBatchLimits(limits);
    wasmJitTestSetMappedFileKeyOverride(77);
    wasmJitTestEnableRuntimeBatching(true);

    U32 addresses[2];
    for (U32 i = 0; i < 2; ++i) {
        addresses[i] = testContext().codeIp;
        testPushCode8(i == 0 ? 0x40 : 0x41); // inc eax / inc ecx
        testPushCode8(0xcd);
        testPushCode8(0x97);
    }
    auto compile = [&](U32 address) {
        DecodedOp* op = testContext().cpu->getOp(address, 0);
        startNewJIT(testContext().cpu, address, op);
        return op;
    };
    DecodedOp* first = compile(addresses[0]);
    if (!(first->flags2 & OP_FLAG2_WASM_JIT_PENDING) || first->pfnJitCode) {
        testFail("runtime batch first block remains pending");
    }
    DecodedOp* second = compile(addresses[1]);
    if (!first->pfnJitCode || !second->pfnJitCode || first->pfnJitCode == second->pfnJitCode) {
        testFail("runtime batch publishes distinct table slots");
    }
    if (wasmJitTestRuntimeModuleCount() != 1 || wasmJitTestRuntimeGroupCount() != 1 || wasmJitTestPendingCount() != 0) {
        testFail("runtime batch instantiates one two-function module");
    }

    testContext().memory->removeCodeBlock(addresses[0], first, false);
    if (wasmJitTestRuntimeGroupCount() != 1) {
        testFail("runtime group survives first export release");
    }
    testContext().memory->removeCodeBlock(addresses[1], second, false);
    if (wasmJitTestRuntimeGroupCount() != 0 || wasmJitTestRuntimeGroupReleaseCount() != 1) {
        testFail("runtime group releases after final export");
    }

    WasmJitRuntimeStatsSnapshot stats = wasmJitTestGetRuntimeStats();
    if (stats.translatedFileBacked != 2 || stats.translatedAnonymous != 0 || stats.groupedModules != 1 || stats.groupedFunctions != 2 || stats.countFlushes != 1 || stats.standaloneModules != 0) {
        testFail("runtime batch aggregate statistics");
    }
    if (wasmJitTestRuntimeGroupReleaseCount() != 1) {
        testFail("runtime batch release statistics");
    }

    wasmJitTestResetRuntimeBatching();
    testNewInstruction(0);
    wasmJitTestSetBatchLimits(limits);
    wasmJitTestSetMappedFileKeyOverride(78);
    wasmJitTestEnableRuntimeBatching(true);
    wasmJitTestFailBatchAfterSlots(1);
    for (U32 i = 0; i < 2; ++i) {
        addresses[i] = testContext().codeIp;
        testPushCode8(0x40 + i);
        testPushCode8(0xcd);
        testPushCode8(0x97);
    }
    first = compile(addresses[0]);
    second = compile(addresses[1]);
    if (first->pfnJitCode || second->pfnJitCode || wasmJitTestPendingCount() != 0 || wasmJitTestRuntimeGroupCount() != 0) {
        testFail("runtime batch partial install rolls back every slot");
    }
    wasmJitTestFailBatchAfterSlots(-1);
}

void testWasmJitPendingLifecycle() {
    testNewInstruction(0);
    wasmJitTestResetRuntimeBatching();
    struct ResetBatching {
        ~ResetBatching() { wasmJitTestResetRuntimeBatching(); }
    } reset;

    WasmJitBatchLimits limits;
    limits.maxBlocks = 64;
    limits.maxBatchBytes = 1024 * 1024;
    limits.urgentPendingHits = 8;
    limits.maxProcessOpenBytes = 4 * 1024 * 1024;
    wasmJitTestSetBatchLimits(limits);
    wasmJitTestSetMappedFileKeyOverride(91);
    wasmJitTestEnableRuntimeBatching(true);

    U32 addresses[5];
    for (U32 i = 0; i < 5; ++i) {
        addresses[i] = testContext().codeIp;
        testPushCode8(0x40 + (i & 3)); // inc eax / inc ecx / inc edx / inc ebx
        testPushCode8(0xcd);
        testPushCode8(0x97);
    }
    U32 nestedAddress = testContext().codeIp;
    testPushCode8(0x40); // inc eax
    testPushCode8(0x41); // inc ecx
    testPushCode8(0x42); // inc edx
    testPushCode8(0xcd);
    testPushCode8(0x97);
    auto compile = [&](U32 address) {
        DecodedOp* op = testContext().cpu->getOp(address, 0);
        startNewJIT(testContext().cpu, address, op);
        return op;
    };

    DecodedOp* lifecycleHook = compile(addresses[4]);
    if (jitUsesCodeMemory()) {
        testFail("WASM JIT lifecycle owns installed entries outside code memory");
    }
    if (!(lifecycleHook->flags2 & OP_FLAG2_WASM_JIT_PENDING) || wasmJitTestPendingCount() != 1) {
        testFail("generic JIT lifecycle hook setup remains pending");
    }
    jitCodeInvalidated(testContext().memory, {lifecycleHook}, {});
    if ((lifecycleHook->flags2 & OP_FLAG2_WASM_JIT_PENDING) || wasmJitTestPendingCount() != 0) {
        testFail("generic JIT lifecycle hook cancels pending block");
    }
    testContext().memory->removeCodeBlock(addresses[4], lifecycleHook, false);

    DecodedOp* first = compile(addresses[0]);
    if (!(first->flags2 & OP_FLAG2_WASM_JIT_PENDING) || wasmJitTestPendingCount() != 1) {
        testFail("pending lifecycle range setup remains pending");
    }
    testContext().memory->removeCodeBlock(addresses[0], first, false);
    if (wasmJitTestPendingCount() != 0 || wasmJitTestRuntimeModuleCount() != 0) {
        testFail("pending lifecycle range invalidation cancels pending block");
    }

    DecodedOp* second = compile(addresses[1]);
    if (!(second->flags2 & OP_FLAG2_WASM_JIT_PENDING) || wasmJitTestPendingCount() != 1) {
        testFail("pending lifecycle full-cache setup remains pending");
    }
    testContext().memory->clearOpCache();
    if (wasmJitTestPendingCount() != 0 || wasmJitTestRuntimeModuleCount() != 0) {
        testFail("pending lifecycle full-cache invalidation cancels pending block");
    }

    DecodedOp* third = compile(addresses[2]);
    if (!(third->flags2 & OP_FLAG2_WASM_JIT_PENDING) || wasmJitTestPendingCount() != 1) {
        testFail("pending lifecycle urgent-hit setup remains pending");
    }
    for (U32 i = 0; i < 8; ++i) {
        testContext().cpu->eip.u32 = addresses[2] - testContext().cpu->seg[CS].address;
        testContext().cpu->nextOp = third;
        testContext().cpu->run();
    }
    bool missingMtRelocHazard = false;
#ifdef BOXEDWINE_MULTI_THREADED
    missingMtRelocHazard = !(third->flags2 & OP_FLAG2_WASM_JIT_RELOC_HAZARD);
#endif
    if (!third->pfnJitCode || (third->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            missingMtRelocHazard ||
            wasmJitTestPendingCount() != 0 ||
            wasmJitTestRuntimeModuleCount() != 1 ||
            wasmJitTestRuntimeGroupCount() != 1) {
        testFail("pending lifecycle urgent hit installs one-entry runtime group");
    }
    testContext().memory->removeCodeBlock(addresses[2], third, false);

    DecodedOp* nestedFirst = compile(nestedAddress);
    DecodedOp* nestedSecond = nestedFirst->next;
    startNewJIT(testContext().cpu, nestedAddress + nestedFirst->len, nestedSecond);
    if (!(nestedFirst->flags2 & OP_FLAG2_WASM_JIT_PENDING) || !(nestedSecond->flags2 & OP_FLAG2_WASM_JIT_PENDING) || wasmJitTestPendingCount() != 2) {
        testFail("pending lifecycle nested entries remain pending");
    }
    testContext().memory->removeCodeBlock(nestedAddress, nestedFirst, false);
    if (wasmJitTestPendingCount() != 0) {
        testFail("pending lifecycle parent invalidation cancels nested entries");
    }

    auto restoreMappings = [&]() {
        constexpr U32 pagesPerSegment = 32;
        U32 segmentBytes = pagesPerSegment << K_PAGE_SHIFT;
        U32 stackBase = TEST_STACK_ADDRESS - segmentBytes;
        if (testContext().memory->mmap(testContext().thread, stackBase, segmentBytes, K_PROT_WRITE | K_PROT_READ, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0) != stackBase || testContext().memory->mmap(testContext().thread, TEST_CODE_ADDRESS, segmentBytes, K_PROT_WRITE | K_PROT_READ | K_PROT_EXEC, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0) != TEST_CODE_ADDRESS || testContext().memory->mmap(testContext().thread, TEST_HEAP_ADDRESS, segmentBytes, K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0) != TEST_HEAP_ADDRESS) {
            testFail("pending lifecycle restores mappings after memory reset");
        }
    };

    DecodedOp* fourth = compile(addresses[3]);
    if (!(fourth->flags2 & OP_FLAG2_WASM_JIT_PENDING) || wasmJitTestPendingCount() != 1) {
        testFail("pending lifecycle exec-reset setup remains pending");
    }
    testContext().memory->execvReset(false);
    if (wasmJitTestPendingCount() != 0) {
        testFail("pending lifecycle exec reset cancels pending block");
    }
    restoreMappings();

    U32 teardownAddress = testContext().codeIp;
    testPushCode8(0x43); // inc ebx
    testPushCode8(0xcd);
    testPushCode8(0x97);
    DecodedOp* teardownOp = compile(teardownAddress);
    if (!(teardownOp->flags2 & OP_FLAG2_WASM_JIT_PENDING) || wasmJitTestPendingCount() != 1) {
        testFail("pending lifecycle teardown setup remains pending");
    }
    testContext().memory->cleanup();
    bool teardownCancelledPending = wasmJitTestPendingCount() == 0;
    testContext().memory->execvReset(true);
    restoreMappings();
    if (!teardownCancelledPending) {
        testFail("pending lifecycle teardown cancels pending block");
    }
}

void testWasmJitTinyAnonymousPromotion() {
    testNewInstruction(0);
    wasmJitTestResetRuntimeBatching();
    struct ResetBatching {
        ~ResetBatching() { wasmJitTestResetRuntimeBatching(); }
    } reset;

    WasmJitBatchLimits limits;
    limits.maxBlocks = 64;
    limits.maxBatchBytes = 1024 * 1024;
    limits.urgentPendingHits = 2;
    limits.maxProcessOpenBytes = 4 * 1024 * 1024;
    wasmJitTestSetBatchLimits(limits);
    wasmJitTestSetMappedFileKeyOverride(0);
    wasmJitTestSetTinyAdditionalHits(2);
    wasmJitTestEnableRuntimeBatching(true);

    U32 tinyAddress = testContext().codeIp;
    testPushCode8(0x40); // inc eax
    testPushCode8(0xcd);
    testPushCode8(0x97);

    U32 normalAddress = testContext().codeIp;
    testPushCode8(0x40); // inc eax
    testPushCode8(0x41); // inc ecx
    testPushCode8(0x42); // inc edx
    testPushCode8(0x43); // inc ebx
    testPushCode8(0xcd);
    testPushCode8(0x97);

    U32 invalidatedAddress = testContext().codeIp;
    testPushCode8(0x43); // inc ebx
    testPushCode8(0xcd);
    testPushCode8(0x97);

    U32 oomTinyAddress = testContext().codeIp;
    testPushCode8(0x42); // inc edx
    testPushCode8(0xcd);
    testPushCode8(0x97);

    auto compile = [&](U32 address) {
        DecodedOp* op = testContext().cpu->getOp(address, 0);
        startNewJIT(testContext().cpu, address, op);
        return op;
    };
    auto isStandaloneJit = [&](DecodedOp* op) {
        return op && op->pfn == testContext().cpu->thread->process->startJITOp && op->pfnJitCode && (op->flags & OP_FLAG_JIT) && !(op->flags2 & OP_FLAG2_WASM_JIT_PENDING);
    };

    DecodedOp* tiny = compile(tinyAddress);
    if (!tiny || !(tiny->flags2 & OP_FLAG2_WASM_JIT_PENDING) || tiny->pfnJitCode || wasmJitTestPendingCount() != 1 || wasmJitTestPendingRawBytes() == 0) {
        testFail("tiny anonymous block retains standalone bytes while pending");
        return;
    }
    wasmJitHandlePendingHit(testContext().cpu, tiny);
    if (!(tiny->flags2 & OP_FLAG2_WASM_JIT_PENDING) || tiny->pfnJitCode || wasmJitTestPendingCount() != 1) {
        testFail("tiny anonymous block remains pending before promotion target");
    }
    wasmJitHandlePendingHit(testContext().cpu, tiny);
    if (!isStandaloneJit(tiny) || wasmJitTestPendingCount() != 0 || wasmJitTestRuntimeGroupCount() != 0) {
        testFail("tiny anonymous block promotes through standalone instantiation");
    }

    DecodedOp* normal = compile(normalAddress);
    if (!isStandaloneJit(normal) || wasmJitTestPendingCount() != 0 || wasmJitTestRuntimeGroupCount() != 0) {
        testFail("four-op anonymous block installs immediately");
    }

    DecodedOp* invalidated = compile(invalidatedAddress);
    if (!invalidated || !(invalidated->flags2 & OP_FLAG2_WASM_JIT_PENDING) || wasmJitTestPendingCount() != 1 || wasmJitTestPendingRawBytes() == 0) {
        testFail("tiny anonymous invalidation setup retains pending bytes");
    }
    testContext().memory->removeCodeBlock(invalidatedAddress, invalidated, false);
    if (wasmJitTestPendingCount() != 0 || wasmJitTestPendingRawBytes() != 0) {
        testFail("tiny anonymous invalidation releases retained bytes");
    }

    DecodedOp* oomTiny = compile(oomTinyAddress);
    if (!oomTiny || !(oomTiny->flags2 & OP_FLAG2_WASM_JIT_PENDING) || oomTiny->pfnJitCode || wasmJitTestPendingCount() != 1) {
        testFail("tiny anonymous OOM setup remains pending");
        return;
    }
    auto runPending = [&](U32 address, DecodedOp* op) {
        testContext().cpu->eip.u32 = address - testContext().cpu->seg[CS].address;
        testContext().cpu->nextOp = op;
        testContext().cpu->run();
    };
    U32 attemptsBeforeOom = wasmJitTestStandaloneModuleAttemptCount();
    boxedwine_wasm_test_force_next_module_oom();
    runPending(oomTinyAddress, oomTiny);
    runPending(oomTinyAddress, oomTiny);
    if (!(oomTiny->flags2 & OP_FLAG2_WASM_JIT_PENDING) || oomTiny->pfnJitCode || wasmJitTestPendingCount() != 1 || wasmJitTestSealedCount() != 1 || !wasmJitCompilationPaused() || wasmJitTestStandaloneModuleAttemptCount() != attemptsBeforeOom + 1) {
        testFail("tiny anonymous OOM retains one globally ordered sealed request");
    }

    U32 pausedAttempts = wasmJitTestStandaloneModuleAttemptCount();
    runPending(oomTinyAddress, oomTiny);
    if (wasmJitTestStandaloneModuleAttemptCount() != pausedAttempts || !(oomTiny->flags2 & OP_FLAG2_WASM_JIT_PENDING) || !wasmJitCompilationPaused()) {
        testFail("tiny anonymous OOM blocks firstDynamicOp retry before release");
    }

    boxedwine_wasm_test_force_next_module_oom();
    testContext().memory->removeCodeBlock(normalAddress, normal, false);
    if (isStandaloneJit(oomTiny) || wasmJitTestPendingCount() != 1 || wasmJitTestSealedCount() != 1 || !wasmJitCompilationPaused() || wasmJitTestStandaloneModuleAttemptCount() != pausedAttempts + 1) {
        testFail("tiny anonymous retry OOM reseals until another real release");
    }

    U32 retryOomAttempts = wasmJitTestStandaloneModuleAttemptCount();
    runPending(oomTinyAddress, oomTiny);
    if (wasmJitTestStandaloneModuleAttemptCount() != retryOomAttempts) {
        testFail("tiny anonymous repeated OOM remains paused before another release");
    }

    testContext().memory->removeCodeBlock(tinyAddress, tiny, false);
    if (!isStandaloneJit(oomTiny) || wasmJitTestPendingCount() != 0 || wasmJitTestSealedCount() != 0 || wasmJitCompilationPaused() || wasmJitTestStandaloneModuleAttemptCount() != retryOomAttempts + 1) {
        testFail("tiny anonymous OOM retries and publishes after a real slot release");
    }

    testContext().memory->removeCodeBlock(oomTinyAddress, oomTiny, false);
}

void testWasmJitGroupedOomRecovery() {
    testNewInstruction(0);
    wasmJitTestResetRuntimeBatching();
    boxedwine_wasm_test_reset_oom_state();
    struct ResetBatching {
        ~ResetBatching() {
            wasmJitTestResetRuntimeBatching();
            boxedwine_wasm_test_reset_oom_state();
        }
    } reset;

    WasmJitBatchLimits limits;
    limits.maxBlocks = 2;
    limits.maxBatchBytes = 1024 * 1024;
    limits.urgentPendingHits = 8;
    limits.maxProcessOpenBytes = 4 * 1024 * 1024;
    wasmJitTestSetBatchLimits(limits);
    wasmJitTestSetMappedFileKeyOverride(0);
    wasmJitTestEnableRuntimeBatching(true);

    U32 releasableAddress = testContext().codeIp;
    testPushCode8(0x40); // inc eax
    testPushCode8(0x41); // inc ecx
    testPushCode8(0x42); // inc edx
    testPushCode8(0x43); // inc ebx
    testPushCode8(0xcd);
    testPushCode8(0x97);

    U32 addresses[2];
    for (U32 i = 0; i < 2; ++i) {
        addresses[i] = testContext().codeIp;
        testPushCode8(0x40 + i);
        testPushCode8(0xcd);
        testPushCode8(0x97);
    }

    auto compile = [&](U32 address) {
        DecodedOp* op = testContext().cpu->getOp(address, 0);
        startNewJIT(testContext().cpu, address, op);
        return op;
    };
    auto isInstalled = [&](DecodedOp* op) {
        return op && op->pfn == testContext().cpu->thread->process->startJITOp && op->pfnJitCode && (op->flags & OP_FLAG_JIT) && !(op->flags2 & OP_FLAG2_WASM_JIT_PENDING);
    };

    DecodedOp* releasable = compile(releasableAddress);
    if (!isInstalled(releasable)) {
        testFail("grouped OOM setup creates a releasable standalone slot");
        return;
    }

    wasmJitTestSetMappedFileKeyOverride(101);
    boxedwine_wasm_test_force_next_module_oom();
    DecodedOp* first = compile(addresses[0]);
    DecodedOp* second = compile(addresses[1]);
    if (!(first->flags2 & OP_FLAG2_WASM_JIT_PENDING) || first->pfnJitCode || !(second->flags2 & OP_FLAG2_WASM_JIT_PENDING) || second->pfnJitCode || wasmJitTestPendingCount() != 2 || wasmJitTestSealedCount() != 1) {
        testFail("grouped OOM retains one sealed two-entry runtime batch");
    }

    U32 attempts = wasmJitTestRuntimeModuleAttemptCount();
    wasmJitHandlePendingHit(testContext().cpu, first);
    if (wasmJitTestRuntimeModuleAttemptCount() != attempts) {
        testFail("grouped OOM blocks repeated module attempts before release");
    }

    testContext().memory->removeCodeBlock(addresses[0], first, false);
    if (wasmJitTestPendingCount() != 1 || wasmJitTestSealedCount() != 1) {
        testFail("grouped OOM cancellation rebuilds the retained sealed batch");
    }

    testContext().memory->removeCodeBlock(releasableAddress, releasable, false);
    if (!isInstalled(second) || wasmJitTestPendingCount() != 0 || wasmJitTestSealedCount() != 0 || wasmJitTestRuntimeModuleAttemptCount() != attempts + 1 || wasmJitTestRuntimeModuleCount() != 1 || wasmJitTestRuntimeGroupCount() != 1) {
        testFail("grouped OOM release retries the surviving entry as one runtime group");
    }
    testContext().memory->removeCodeBlock(addresses[1], second, false);
}
#endif
#endif
