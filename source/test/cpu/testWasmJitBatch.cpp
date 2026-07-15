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
#if defined(BOXEDWINE_WASM_JIT) && !defined(BOXEDWINE_MULTI_THREADED)
#include "../../emulation/cpu/jit/jitCodeLifecycle.h"
#include "../../emulation/cpu/wasm/jitWasmCodeGen.h"
#include "../../emulation/cpu/wasm/wasmEmitter.h"
#include "../../emulation/cpu/wasm/wasmJitBatchPolicy.h"
#include "../../emulation/cpu/wasm/wasmModuleMerger.h"
#include <emscripten.h>

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
    if (!third->pfnJitCode || (third->flags2 & OP_FLAG2_WASM_JIT_PENDING) || wasmJitTestPendingCount() != 0 || wasmJitTestRuntimeModuleCount() != 1 || wasmJitTestRuntimeGroupCount() != 1) {
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
