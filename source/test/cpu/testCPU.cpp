/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#ifdef __TEST

#include "ksignal.h"
#include "testCPU.h"
#if defined(BOXEDWINE_JIT_ARMV8)
#include "../../emulation/cpu/armv8/jitArmV8CodeGen.h"
#endif
#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)
#include "../../emulation/cpu/x32/jitX86CodeGen.h"
#endif
#if defined(BOXEDWINE_WASM_JIT) && !defined(BOXEDWINE_MULTI_THREADED)
#include "../../emulation/cpu/wasm/jitWasmCodeGen.h"
#endif
#ifdef BOXEDWINE_WASM_JIT
#include "../../emulation/cpu/jit/jitCodeLifecycle.h"
#endif
#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

extern void failed(const char* msg, ...);

namespace {

constexpr U32 TEST_PAGES_PER_SEG = 32;

std::atomic<bool> fastMode(false);
std::mutex testContextCreateMutex;
std::unique_ptr<TestContext> serialContext;
std::vector<std::unique_ptr<TestContext>> parallelContexts;
thread_local TestContext* currentContext = nullptr;
thread_local bool runningParallelTest = false;

#if defined(BOXEDWINE_JIT) && !defined(BOXEDWINE_WASM_JIT)
void OPCALL testJitRunCountCallback(CPU* cpu, DecodedOp* op) {
    (void)cpu;
    (void)op;
}
#endif

void setupSegments(TestContext& context) {
    CPU* cpu = context.cpu;
    KProcess* process = context.process.get();

    for (int i = 0; i < 6; i++) {
        cpu->seg[i].address = 0;
        cpu->seg[i].value = 0;
        process->hasSetSeg[i] = false;
    }
    process->hasSetStackMask = false;

    struct user_desc* ldt = process->getLDT(TEST_HEAP_SEG >> 3);
    ldt->entry_number = TEST_HEAP_SEG >> 3;
    ldt->base_addr = TEST_HEAP_ADDRESS;
    ldt->seg_32bit = 1;
    ldt->contents = 0;
    ldt->read_exec_only = 0;
    ldt->seg_not_present = 0;

    ldt = process->getLDT(TEST_STACK_SEG >> 3);
    ldt->entry_number = TEST_STACK_SEG >> 3;
    ldt->base_addr = TEST_STACK_ADDRESS - K_PAGE_SIZE * TEST_PAGES_PER_SEG;
    ldt->seg_32bit = 1;
    ldt->contents = 0;
    ldt->read_exec_only = 0;
    ldt->seg_not_present = 0;

    ldt = process->getLDT(TEST_CODE_SEG >> 3);
    ldt->entry_number = TEST_CODE_SEG >> 3;
    ldt->base_addr = TEST_CODE_ADDRESS;
    ldt->seg_32bit = 1;
    ldt->contents = 2;
    ldt->read_exec_only = 0;
    ldt->seg_not_present = 0;

    ldt = process->getLDT(TEST_CODE_SEG_16 >> 3);
    ldt->entry_number = TEST_CODE_SEG_16 >> 3;
    ldt->base_addr = TEST_CODE_ADDRESS;
    ldt->seg_32bit = 0;
    ldt->contents = 2;
    ldt->read_exec_only = 0;
    ldt->seg_not_present = 0;

    cpu->seg[CS].address = TEST_CODE_ADDRESS;
    cpu->seg[CS].value = TEST_CODE_SEG;
    cpu->seg[DS].address = TEST_HEAP_ADDRESS;
    cpu->seg[DS].value = TEST_HEAP_SEG;
    cpu->seg[SS].address = TEST_STACK_ADDRESS - K_PAGE_SIZE * TEST_PAGES_PER_SEG;
    cpu->seg[SS].value = TEST_STACK_SEG;
    cpu->stackMask = 0xffffffff;
    cpu->stackNotMask = 0;
    cpu->seg[ES].address = 0;
    cpu->seg[ES].value = 0;
    cpu->seg[GS].address = 0;
    cpu->seg[GS].value = 0;
    cpu->seg[FS].address = 0;
    cpu->seg[FS].value = 0;
    process->hasSetSeg[CS] = true;
    process->hasSetSeg[DS] = true;
    process->hasSetSeg[SS] = true;
}

void resetMemory(TestContext& context) {
    context.memory->memset(TEST_CODE_ADDRESS, 0, K_PAGE_SIZE * TEST_PAGES_PER_SEG);
    context.memory->memset(TEST_STACK_ADDRESS - K_PAGE_SIZE * TEST_PAGES_PER_SEG, 0, K_PAGE_SIZE * TEST_PAGES_PER_SEG);
    context.memory->memset(TEST_HEAP_ADDRESS, 0, K_PAGE_SIZE * TEST_PAGES_PER_SEG);
}

void createContext(TestContext& context) {
    context.process = KProcess::create();
    context.memory = KMemory::create(context.process.get());
    context.process->memory = context.memory;
    context.thread = context.process->createThread();
    context.cpu = context.thread->cpu;
    KThread::setCurrentThread(context.thread);

    context.memory->mmap(context.thread, ((TEST_STACK_ADDRESS >> K_PAGE_SHIFT) - TEST_PAGES_PER_SEG) << K_PAGE_SHIFT, TEST_PAGES_PER_SEG << K_PAGE_SHIFT, K_PROT_WRITE | K_PROT_READ | K_PROT_READ, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    context.memory->mmap(context.thread, TEST_CODE_ADDRESS, TEST_PAGES_PER_SEG << K_PAGE_SHIFT, K_PROT_WRITE | K_PROT_READ | K_PROT_READ | K_PROT_EXEC, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    context.memory->mmap(context.thread, TEST_HEAP_ADDRESS, TEST_PAGES_PER_SEG << K_PAGE_SHIFT, K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    setupSegments(context);
    resetMemory(context);
}

bool intInList(int value, const int* values, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (values[i] == value) {
            return true;
        }
    }
    return false;
}

} // namespace

TestContext& testContext() {
    if (!currentContext) {
        std::lock_guard<std::mutex> lock(testContextCreateMutex);
        if (!serialContext) {
            serialContext = std::make_unique<TestContext>();
            createContext(*serialContext);
        }
        currentContext = serialContext.get();
    }
    KThread::setCurrentThread(currentContext->thread);
    return *currentContext;
}

void ensureParallelContexts(U32 workerCount) {
    std::lock_guard<std::mutex> lock(testContextCreateMutex);
    while (parallelContexts.size() < workerCount) {
        std::unique_ptr<TestContext> context = std::make_unique<TestContext>();
        createContext(*context);
        parallelContexts.push_back(std::move(context));
    }
}

void bindParallelContext(U32 index) {
    currentContext = parallelContexts[index].get();
    KThread::setCurrentThread(currentContext->thread);
}

void resetEntryContext(TestContext& context) {
    KThread::setCurrentThread(context.thread);
    setupSegments(context);
    context.thread->inSignal = 0;
    context.thread->inSigMask = 0;
    context.thread->pendingSignals = 0;
#ifdef BOXEDWINE_MULTI_THREADED
    context.thread->startSignal = false;
#endif
    context.thread->interrupted = false;
    context.cpu->debugTrapOnNextInstruction = false;
    context.cpu->pendingDebugTrap = false;
    context.cpu->pendingDebugTrapCode = 0;
    context.cpu->pendingDebugTrapDr6 = 0;
    for (U32& debugReg : context.thread->debugRegs) {
        debugReg = 0;
    }
    context.thread->updateDebugTrapActive();
}

void testNewInstruction(int flags) {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;

    setupSegments(context);
    resetMemory(context);
    context.codeIp = TEST_CODE_ADDRESS;
    cpu->lazyFlagType = FLAGS_NONE;
    cpu->setFlags(flags, FMASK_ALL);
    cpu->big = 1;
    cpu->reg[0].u32 = 0;
    cpu->reg[1].u32 = 0;
    cpu->reg[2].u32 = 0;
    cpu->reg[3].u32 = 0;
    cpu->reg[4].u32 = 4096;
    cpu->reg[5].u32 = 0;
    cpu->reg[6].u32 = 0;
    cpu->reg[7].u32 = 0;
    cpu->eip.u32 = 0;
    cpu->nextOp = nullptr;
    context.memory->clearOpCache();
    cpu->setMxcsr(0x1F80);
    cpu->debugTrapOnNextInstruction = false;
    cpu->pendingDebugTrap = false;
    cpu->pendingDebugTrapCode = 0;
    cpu->pendingDebugTrapDr6 = 0;
    context.thread->inSignal = 0;
    context.thread->inSigMask = 0;
    context.thread->pendingSignals = 0;
#ifdef BOXEDWINE_MULTI_THREADED
    context.thread->startSignal = false;
#endif
    context.thread->interrupted = false;
    for (U32& debugReg : context.thread->debugRegs) {
        debugReg = 0;
    }
    context.thread->updateDebugTrapActive();
}

void testPushCode8(int value) {
    TestContext& context = testContext();
    context.memory->writeb(context.codeIp, value);
    context.memory->clearPageWriteCounts(context.codeIp >> K_PAGE_SHIFT);
    context.codeIp++;
}

void testPushCode16(int value) {
    TestContext& context = testContext();
    U32 startPage = context.codeIp >> K_PAGE_SHIFT;
    context.memory->writew(context.codeIp, value);
    context.codeIp += 2;
    context.memory->clearPageWriteCounts(startPage);
    context.memory->clearPageWriteCounts((context.codeIp - 1) >> K_PAGE_SHIFT);
}

void testPushCode32(int value) {
    TestContext& context = testContext();
    U32 startPage = context.codeIp >> K_PAGE_SHIFT;
    context.memory->writed(context.codeIp, value);
    context.codeIp += 4;
    context.memory->clearPageWriteCounts(startPage);
    context.memory->clearPageWriteCounts((context.codeIp - 1) >> K_PAGE_SHIFT);
}

void testRunCPU() {
#if defined(BOXEDWINE_JIT_ARMV8)
    ensureArmV8HardwareTSOForThread();
#endif
    TestContext& context = testContext();
    CPU* cpu = context.cpu;

    testPushCode8(0xcd);
    testPushCode8(0x97);

    cpu->nextOp = cpu->getNextOp();
    do {
        try {
            cpu->run();
        } catch (int) {
            // Guest memory faults install a signal handler and throw to abort
            // the current instruction, just as runThreadSlice expects.
            cpu->nextOp = nullptr;
        }
    } while (!cpu->nextOp || cpu->nextOp->inst != TestEnd);
}

void testFail(const char* msg, ...) {
    TestContext& context = testContext();
    context.failed = true;

    char text[1024];
    va_list args;
    va_start(args, msg);
    vsnprintf(text, sizeof(text), msg, args);
    va_end(args);

    context.failures.push_back(text);
    if (!runningParallelTest) {
        failed("%s", text);
    }
}

bool testIsFastMode() {
    return fastMode.load();
}

void testSetFastMode(bool fast) {
    fastMode.store(fast);
}

void testWasmJitOnlyBlockEntryIsCallable() {
#ifdef BOXEDWINE_WASM_JIT
    TestContext& context = testContext();
    CPU* cpu = context.cpu;

    testNewInstruction(0);
    testPushCode8(0x40); // inc eax
    testPushCode8(0x41); // inc ecx
    testPushCode8(0x42); // inc edx
    testRunCPU();

    if (jitUsesCodeMemory()) {
        testFail("wasm jit entries are not backed by native code memory");
    }

    DecodedOp* first = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
    DecodedOp* second = first ? first->next : nullptr;
    DecodedOp* third = second ? second->next : nullptr;

    if (!first || !second || !third) {
        testFail("wasm jit block metadata decode");
        return;
    }
    if (first->pfn != cpu->thread->process->startJITOp || !first->pfnJitCode) {
        testFail("wasm jit first op callable entry");
    }
    if (second->pfn == cpu->thread->process->startJITOp || second->pfnJitCode) {
        testFail("wasm jit second op interior is not callable entry");
    }
    if (third->pfn == cpu->thread->process->startJITOp || third->pfnJitCode) {
        testFail("wasm jit third op interior is not callable entry");
    }
    if (second->blockStart != first || third->blockStart != first) {
        testFail("wasm jit interior ops keep owner block");
    }

    cpu->eip.u32 = first->len;
    cpu->nextOp = second;
    cpu->run();

    if (second->pfn != cpu->thread->process->startJITOp || !second->pfnJitCode) {
        testFail("wasm jit fallthrough interior op can compile as subblock entry");
    }
    if (second->blockStart != first) {
        testFail("wasm jit fallthrough interior keeps longer owner block");
    }
    if (third->pfn == cpu->thread->process->startJITOp || third->pfnJitCode) {
        testFail("wasm jit fallthrough subblock interior is not callable entry");
    }

    if (first->pfnJitCode == second->pfnJitCode) {
        testFail("wasm jit parent and fallthrough subblock have distinct entries");
    }

    cpu->wasmJitActiveBlock = first;
    cpu->wasmJitBailout = 0;
    context.memory->removeCodeBlock(TEST_CODE_ADDRESS, first, false);

    if (cpu->wasmJitBailout != 1) {
        testFail("wasm jit active invalidation requests bailout");
    }
    if (first->pfnJitCode || second->pfnJitCode || third->pfnJitCode) {
        testFail("wasm jit parent invalidation clears subblock entries");
    }
    if (first->blockStart || second->blockStart || third->blockStart) {
        testFail("wasm jit parent invalidation clears owner metadata");
    }

    testNewInstruction(0);
    cpu->reg[0].u32 = 0x100; // eax
    cpu->reg[2].u32 = 0x200; // edx
    cpu->reg[6].u32 = 0x103FEB4C; // stale esi value should be overwritten
    cpu->reg[7].u32 = 0x11223344; // stale edi value should be overwritten
    context.memory->writed(TEST_HEAP_ADDRESS + 0x100, 0x12345678);
    context.memory->writed(TEST_HEAP_ADDRESS + 0x104, 3);

    testPushCode8(0x8b); // mov esi,[eax+4]
    testPushCode8(0x70);
    testPushCode8(0x04);
    testPushCode8(0x8b); // mov edi,[eax]
    testPushCode8(0x38);
    testPushCode8(0x66); // mov [edx+esi*8+6],di
    testPushCode8(0x89);
    testPushCode8(0x7c);
    testPushCode8(0xf2);
    testPushCode8(0x06);
    testRunCPU();

    if (context.memory->readw(TEST_HEAP_ADDRESS + 0x200 + 3 * 8 + 6) != 0x5678) {
        testFail("wasm jit scaled-index word store uses loaded esi/edi");
    }

    testNewInstruction(0);
    testPushCode8(0x75); // jnz to the ret, keeping ret inside a forward-branch range
    testPushCode8(0x00);
    testPushCode8(0xc3); // ret
    testPushCode8(0x43); // inc ebx, a decoded next function/op that must not join the ret block
    testPushCode8(0xcd);
    testPushCode8(0x97);

    context.memory->writed(cpu->seg[SS].address + cpu->reg[4].u32, 4);
    cpu->getOp(TEST_CODE_ADDRESS + 3, 0);
    cpu->nextOp = cpu->getNextOp();
    do {
        cpu->run();
    } while (!cpu->nextOp || cpu->nextOp->inst != TestEnd);

    DecodedOp* branch = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
    DecodedOp* ret = branch && branch->next ? branch->next : nullptr;
    DecodedOp* afterRet = ret && ret->next ? ret->next : context.memory->getDecodedOp(TEST_CODE_ADDRESS + 3);

    if (!branch || !ret || !afterRet) {
        testFail("wasm jit ret boundary metadata decode");
        return;
    }
    if (branch->pfn != cpu->thread->process->startJITOp || !branch->pfnJitCode) {
        testFail("wasm jit ret boundary block compiled");
    }
    if (branch->blockLen != 3 || branch->blockOpCount != 2) {
        testFail("wasm jit ret boundary stops at computed exit");
    }
    if (ret->blockStart != branch) {
        testFail("wasm jit ret stays in owner block");
    }
    if (afterRet->blockStart == branch) {
        testFail("wasm jit ret boundary does not absorb following decoded op");
    }

    testNewInstruction(0);
    testPushCode8(0xbe); // mov esi,3
    testPushCode32(3);
    testPushCode8(0xe8); // call helper
    testPushCode32(17);
    testPushCode8(0xbf); // mov edi,0x12345678
    testPushCode32(0x12345678);
    testPushCode8(0xba); // mov edx,0x200
    testPushCode32(0x200);
    testPushCode8(0x66); // mov [edx+esi*8+6],di
    testPushCode8(0x89);
    testPushCode8(0x7c);
    testPushCode8(0xf2);
    testPushCode8(0x06);
    testPushCode8(0xcd);
    testPushCode8(0x97);
    testPushCode8(0x56); // helper: push esi
    testPushCode8(0xbe); // mov esi,7
    testPushCode32(7);
    testPushCode8(0x5e); // pop esi
    testPushCode8(0xc3); // ret
    testRunCPU();

    if (context.memory->readw(TEST_HEAP_ADDRESS + 0x200 + 3 * 8 + 6) != 0x5678) {
        testFail("wasm jit call preserves restored esi for caller");
    }
    if (context.memory->readw(TEST_HEAP_ADDRESS + 0x200 + 7 * 8 + 6) == 0x5678) {
        testFail("wasm jit call must not use callee-clobbered esi");
    }
#endif
}

void testWasmJitOomRetryAfterRelease() {
#if defined(BOXEDWINE_WASM_JIT) && !defined(BOXEDWINE_MULTI_THREADED)
    TestContext& context = testContext();
    CPU* cpu = context.cpu;

    testNewInstruction(0);
    wasmJitTestResetRuntimeBatching();
    boxedwine_wasm_test_reset_oom_state();
    struct ResetOomState {
        ~ResetOomState() {
            wasmJitTestResetRuntimeBatching();
            boxedwine_wasm_test_reset_oom_state();
        }
    } resetOomState;

    U32 addresses[5];
    for (U32& address : addresses) {
        address = context.codeIp;
        testPushCode8(0x40); // inc eax
        testPushCode8(0xcd);
        testPushCode8(0x97); // TestEnd
    }

    auto compileBlock = [&](U32 address) -> DecodedOp* {
        DecodedOp* op = cpu->getOp(address, 0);
        if (!op) {
            testFail("wasm jit OOM recovery decode at %x", address);
            return nullptr;
        }
        startNewJIT(cpu, address, op);
        return op;
    };
    auto isJitBlock = [&](DecodedOp* op) {
        return op && op->pfn == cpu->thread->process->startJITOp && op->pfnJitCode && (op->flags & OP_FLAG_JIT);
    };

    DecodedOp* releasable = compileBlock(addresses[0]);
    if (!isJitBlock(releasable)) {
        testFail("wasm jit OOM recovery setup block compiled");
        return;
    }
    WasmJitRuntimeStatsSnapshot stats = wasmJitTestGetRuntimeStats();
    U64 rawBytesPerBlock = stats.rawInputBytes;
    if (stats.translatedAnonymous != 1 || stats.translatedFileBacked != 0 || stats.standaloneModules != 1 || rawBytesPerBlock == 0) {
        testFail("wasm jit standalone OOM statistics setup");
    }

    boxedwine_wasm_test_force_next_module_oom();
    DecodedOp* oomBlock = compileBlock(addresses[1]);
    if (isJitBlock(oomBlock)) {
        testFail("wasm jit forced OOM falls back to interpreter");
    }
    stats = wasmJitTestGetRuntimeStats();
    if (stats.translatedAnonymous != 2 || stats.rawInputBytes != rawBytesPerBlock * 2 || stats.oomRetries != 0 || stats.oomResumptions != 0) {
        testFail("wasm jit standalone OOM translation statistics");
    }

    DecodedOp* blockedBlock = compileBlock(addresses[2]);
    if (isJitBlock(blockedBlock)) {
        testFail("wasm jit OOM blocks compilation until release");
    }
    stats = wasmJitTestGetRuntimeStats();
    if (stats.translatedAnonymous != 3 || stats.rawInputBytes != rawBytesPerBlock * 3 || stats.oomRetries != 0 || stats.oomResumptions != 0) {
        testFail("wasm jit blocked standalone translation statistics");
    }

    context.memory->removeCodeBlock(addresses[0], releasable, false);

    DecodedOp* retryBlock = compileBlock(addresses[3]);
    if (!isJitBlock(retryBlock)) {
        testFail("wasm jit release permits one recovery probe");
    }
    stats = wasmJitTestGetRuntimeStats();
    if (stats.translatedAnonymous != 4 || stats.rawInputBytes != rawBytesPerBlock * 4 || stats.oomRetries != 1 || stats.oomResumptions != 1) {
        testFail("wasm jit standalone OOM retry statistics");
    }

    DecodedOp* continuedBlock = compileBlock(addresses[4]);
    if (!isJitBlock(continuedBlock)) {
        testFail("wasm jit successful OOM recovery resumes compilation");
    }
    stats = wasmJitTestGetRuntimeStats();
    if (stats.translatedAnonymous != 5 || stats.rawInputBytes != rawBytesPerBlock * 5 || stats.oomRetries != 1 || stats.oomResumptions != 1) {
        testFail("wasm jit resumed standalone statistics");
    }
#endif
}

namespace {

constexpr U32 CROSS_BLOCK_FLAGS_TARGET_OFFSET = 0x100;
constexpr U32 CROSS_BLOCK_FLAGS_MASK = CF | PF | AF | ZF | SF | OF;

void padCrossBlockFlagsCodeToTarget() {
    TestContext& context = testContext();
    while (context.codeIp < TEST_CODE_ADDRESS + CROSS_BLOCK_FLAGS_TARGET_OFFSET) {
        testPushCode8(0x90); // nop
    }
}

void verifyCrossBlockJitEntries(const char* name) {
#ifdef BOXEDWINE_JIT
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    DecodedOp* producer = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
    DecodedOp* consumer = context.memory->getDecodedOp(TEST_CODE_ADDRESS + CROSS_BLOCK_FLAGS_TARGET_OFFSET);

    if (!producer || !consumer) {
        testFail("%s metadata decode", name);
        return;
    }
    if (producer->pfn != cpu->thread->process->startJITOp || !producer->pfnJitCode) {
        testFail("%s producer compiled entry", name);
    }
    if (consumer->pfn != cpu->thread->process->startJITOp || !consumer->pfnJitCode) {
        testFail("%s consumer compiled entry", name);
    }
    if (producer->blockStart != producer) {
        testFail("%s producer owns block", name);
    }
    if (consumer->blockStart != consumer) {
        testFail("%s consumer owns block", name);
    }
    if (producer->pfnJitCode && producer->pfnJitCode == consumer->pfnJitCode) {
        testFail("%s producer and consumer use distinct compiled entries", name);
    }
#else
    (void)name;
#endif
}

void runCrossBlockFlagsCase(const U8* producerCode, U32 producerCodeLen, U32 initialFlags,
                            U32 esi, U32 edi, U32 expectedEsi, U32 expectedFlags,
                            const char* name) {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;

    testNewInstruction((int)initialFlags);
    cpu->reg[0].u32 = CROSS_BLOCK_FLAGS_TARGET_OFFSET; // eax: indirect jump target
    cpu->reg[6].u32 = esi;
    cpu->reg[7].u32 = edi;

    for (U32 i = 0; i < producerCodeLen; ++i) {
        testPushCode8(producerCode[i]);
    }
    testPushCode8(0xff); // jmp eax
    testPushCode8(0xe0);
    padCrossBlockFlagsCodeToTarget();
    testPushCode8(0x9c); // pushfd
    testPushCode8(0x5b); // pop ebx

    testRunCPU();

    if (cpu->reg[6].u32 != expectedEsi) {
        testFail("%s result", name);
    }
    if (((cpu->reg[3].u32 ^ expectedFlags) & CROSS_BLOCK_FLAGS_MASK) != 0) {
        testFail("%s flags", name);
    }
    verifyCrossBlockJitEntries(name);
}

}

void testFlagsAcrossIndirectJitBlockBoundary() {
    static const U8 incEsi[] = {0x46};
    runCrossBlockFlagsCase(incEsi, sizeof(incEsi), CROSS_BLOCK_FLAGS_MASK,
                           0, 0, 1, CF, "cross-block inc/pushfd");

    static const U8 subEsiEdi[] = {0x29, 0xfe};
    runCrossBlockFlagsCase(subEsiEdi, sizeof(subEsiEdi), ZF | OF,
                           0, 1, 0xffffffff, CF | PF | AF | SF,
                           "cross-block sub/pushfd");
}

void testJitOverlappingDirectJumpTarget() {
#ifdef BOXEDWINE_JIT
    CPU* cpu = testContext().cpu;

    testNewInstruction(0);
    cpu->reg[0].u32 = 0x12340000;

    testPushCode8(0x39); // cmp eax,eax
    testPushCode8(0xc0);
    testPushCode8(0x74); // jz +1 into the immediate bytes of the fallthrough mov
    testPushCode8(0x01);
    testPushCode8(0xb8); // fallthrough: mov eax,0x02eb07b0
    testPushCode8(0xb0); // target: mov al,7
    testPushCode8(0x07);
    testPushCode8(0xeb); // jmp over mov al,3
    testPushCode8(0x02);
    testPushCode8(0xb0); // fallthrough stream: mov al,3
    testPushCode8(0x03);
    testRunCPU();

    if ((cpu->reg[0].u32 & 0xff) != 7) {
        testFail("jit overlapping direct jump target");
    }
#endif
}

void testNativeJitRunCountWraps() {
#if defined(BOXEDWINE_JIT) && !defined(BOXEDWINE_WASM_JIT)
    DecodedOp op;
    op.runCount = 0xff;
    op.pfn = testJitRunCountCallback;

    firstDynamicOp(testContext().cpu, &op);

    if (op.runCount != 0) {
        testFail("native JIT runCount wraps after a failed compile threshold");
    }
#endif
}

namespace {

enum class DirectArithmeticTestOp {
    Add,
    Sub,
    And,
    Or,
    Xor
};

enum class DirectArithmeticTestShape {
    Reg,
    MemSource,
    Imm,
    MemReg,
    MemImm
};

struct DirectArithmeticOpInfo {
    DirectArithmeticTestOp op;
    const char* name;
    U8 regRmOpcode;
    U8 rmRegOpcode;
    U8 immediateGroup;
};

constexpr DirectArithmeticOpInfo DIRECT_ARITHMETIC_OPS[] = {
    {DirectArithmeticTestOp::Add, "add", 0x03, 0x01, 0},
    {DirectArithmeticTestOp::Sub, "sub", 0x2b, 0x29, 5},
    {DirectArithmeticTestOp::And, "and", 0x23, 0x21, 4},
    {DirectArithmeticTestOp::Or,  "or",  0x0b, 0x09, 1},
    {DirectArithmeticTestOp::Xor, "xor", 0x33, 0x31, 6},
};

constexpr U32 DIRECT_ARITHMETIC_MEM_SRC = 0x300;

constexpr U32 DIRECT_ARITHMETIC_CASES[][2] = {
    {0, 0},
    {1, 0},
    {0, 1},
    {0xffffffff, 1},
    {0x7fffffff, 1},
    {0x80000000, 1},
    {0x80000000, 0xffffffff},
    {0xaaaaaaaa, 0x55555555},
};

U32 directArithmeticParityFlag(U32 value) {
    U8 low = (U8)value;
    low ^= low >> 4;
    low &= 0x0f;
    return ((0x6996 >> low) & 1) == 0 ? PF : 0;
}

U32 directArithmeticFlags(DirectArithmeticTestOp op, U32 lhs, U32 rhs, U32& result) {
    U32 flags = 0;
    switch (op) {
    case DirectArithmeticTestOp::Add:
        result = lhs + rhs;
        if ((U64)lhs + rhs > 0xffffffffULL) flags |= CF;
        if ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000) != 0) flags |= OF;
        if (((lhs ^ rhs ^ result) & 0x10) != 0) flags |= AF;
        break;
    case DirectArithmeticTestOp::Sub:
        result = lhs - rhs;
        if (lhs < rhs) flags |= CF;
        if (((lhs ^ rhs) & (lhs ^ result) & 0x80000000) != 0) flags |= OF;
        if (((lhs ^ rhs ^ result) & 0x10) != 0) flags |= AF;
        break;
    case DirectArithmeticTestOp::And:
        result = lhs & rhs;
        break;
    case DirectArithmeticTestOp::Or:
        result = lhs | rhs;
        break;
    case DirectArithmeticTestOp::Xor:
        result = lhs ^ rhs;
        break;
    }
    if (result == 0) flags |= ZF;
    if (result & 0x80000000) flags |= SF;
    flags |= directArithmeticParityFlag(result);
    return flags;
}

bool directArithmeticCondition(U8 condition, U32 flags) {
    bool cf = (flags & CF) != 0;
    bool pf = (flags & PF) != 0;
    bool zf = (flags & ZF) != 0;
    bool sf = (flags & SF) != 0;
    bool of = (flags & OF) != 0;
    switch (condition) {
    case 0x0: return of;
    case 0x1: return !of;
    case 0x2: return cf;
    case 0x3: return !cf;
    case 0x4: return zf;
    case 0x5: return !zf;
    case 0x6: return cf || zf;
    case 0x7: return !cf && !zf;
    case 0x8: return sf;
    case 0x9: return !sf;
    case 0xa: return pf;
    case 0xb: return !pf;
    case 0xc: return sf != of;
    case 0xd: return sf == of;
    case 0xe: return zf || (sf != of);
    default: return !zf && (sf == of);
    }
}

U32 emitDirectArithmetic(const DirectArithmeticOpInfo& op, DirectArithmeticTestShape shape, U32 rhs) {
    if (shape == DirectArithmeticTestShape::Reg) {
        testPushCode8(op.regRmOpcode);
        testPushCode8(0xc2); // op eax,edx
        return 2;
    }
    if (shape == DirectArithmeticTestShape::MemSource) {
        testPushCode8(op.regRmOpcode);
        testPushCode8(0x05); // op eax,[disp32]
        testPushCode32(DIRECT_ARITHMETIC_MEM_SRC);
        return 6;
    }
    if (shape == DirectArithmeticTestShape::Imm) {
        testPushCode8(0x81);
        testPushCode8(0xc0 | (op.immediateGroup << 3)); // op eax,imm32
        testPushCode32(rhs);
        return 6;
    }
    if (shape == DirectArithmeticTestShape::MemReg) {
        testPushCode8(op.rmRegOpcode);
        testPushCode8(0x15); // op [disp32],edx
        testPushCode32(DIRECT_ARITHMETIC_MEM_SRC);
        return 6;
    }
    testPushCode8(0x81);
    testPushCode8(0x05 | (op.immediateGroup << 3)); // op [disp32],imm32
    testPushCode32(DIRECT_ARITHMETIC_MEM_SRC);
    testPushCode32(rhs);
    return 10;
}

const char* directArithmeticShapeName(DirectArithmeticTestShape shape) {
    switch (shape) {
    case DirectArithmeticTestShape::Reg: return "reg";
    case DirectArithmeticTestShape::MemSource: return "reg,mem";
    case DirectArithmeticTestShape::Imm: return "reg,imm";
    case DirectArithmeticTestShape::MemReg: return "mem,reg";
    default: return "mem,imm";
    }
}

void runDirectArithmeticFlagsCase(const DirectArithmeticOpInfo& op, DirectArithmeticTestShape shape,
                                  U32 lhs, U32 rhs, U8 condition) {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    U32 expectedResult = 0;
    U32 expectedFlags = directArithmeticFlags(op.op, lhs, rhs, expectedResult);
    U32 expectedCondition = directArithmeticCondition(condition, expectedFlags) ? 1 : 0;

    testNewInstruction(0);
    cpu->big = true;
    bool memoryDestination = shape == DirectArithmeticTestShape::MemReg || shape == DirectArithmeticTestShape::MemImm;
    cpu->reg[0].u32 = memoryDestination ? 0x89abcdef : lhs; // eax: register arithmetic destination
    cpu->reg[2].u32 = rhs;        // edx: register source
    cpu->reg[3].u32 = 0x12345678; // ebx: SETcc destination
    cpu->reg[6].u32 = 1;          // esi
    cpu->reg[7].u32 = 2;          // edi
    U32 initialMemory = memoryDestination ? lhs : rhs;
    context.memory->writed(TEST_HEAP_ADDRESS + DIRECT_ARITHMETIC_MEM_SRC, initialMemory);

    U32 producerLen = emitDirectArithmetic(op, shape, rhs);
    testPushCode8(0x0f); // setcc bl
    testPushCode8(0x90 + condition);
    testPushCode8(0xc3);
    testPushCode8(0x39); // cmp esi,edi; makes the arithmetic flags dead
    testPushCode8(0xfe);
    testRunCPU();

    const char* shapeName = directArithmeticShapeName(shape);
    U32 expectedEax = memoryDestination ? 0x89abcdef : expectedResult;
    if (cpu->reg[0].u32 != expectedEax) {
        testFail("direct %s %s condition %x result", op.name, shapeName, condition);
    }
    U32 expectedMemory = memoryDestination ? expectedResult : initialMemory;
    if (context.memory->readd(TEST_HEAP_ADDRESS + DIRECT_ARITHMETIC_MEM_SRC) != expectedMemory) {
        testFail("direct %s %s condition %x memory result", op.name, shapeName, condition);
    }
    if ((cpu->reg[3].u32 & 0xff) != expectedCondition) {
        testFail("direct %s %s condition %x flags", op.name, shapeName, condition);
    }

#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)
    DecodedOp* producer = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
    DecodedOp* set = context.memory->getDecodedOp(TEST_CODE_ADDRESS + producerLen);
    if (!producer || !set) {
        testFail("direct %s %s condition %x metadata decode", op.name, shapeName, condition);
        return;
    }
    if (producer->pfn != cpu->thread->process->startJITOp || !producer->pfnJitCode) {
        testFail("direct %s %s condition %x producer entry", op.name, shapeName, condition);
    }
    if (!(set->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) || set->pfnJitCode) {
        testFail("direct %s %s condition %x skipped set", op.name, shapeName, condition);
    }
#endif
}

void runDirectArithmeticWriteFaultCase() {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    constexpr U32 initialValue = 0x12345678;

    testNewInstruction((int)(CF | ZF));
    cpu->big = true;
    cpu->reg[0].u32 = 0;
    cpu->reg[2].u32 = 1;          // edx: add source
    cpu->reg[3].u32 = 0x89abcdef; // ebx: SETcc destination
    cpu->reg[6].u32 = 1;          // esi
    cpu->reg[7].u32 = 2;          // edi

    U32 targetAddress = cpu->seg[DS].address + DIRECT_ARITHMETIC_MEM_SRC;
    context.memory->writed(targetAddress, initialValue);

    testPushCode8(0x01);
    testPushCode8(0x15); // add [disp32],edx
    testPushCode32(DIRECT_ARITHMETIC_MEM_SRC);
    testPushCode8(0x0f);
    testPushCode8(0x92); // setb bl; fuses with add
    testPushCode8(0xc3);
    testPushCode8(0x39); // cmp esi,edi; makes the add flags dead
    testPushCode8(0xfe);
    constexpr U32 faultHandlerOffset = 11;

    KSigAction& action = context.process->sigActions[K_SIGSEGV];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + faultHandlerOffset;
    action.flags = 0;

    U32 targetPage = targetAddress & ~K_PAGE_MASK;
    if (context.memory->mprotect(context.thread, targetPage, K_PAGE_SIZE, K_PROT_READ)) {
        testFail("direct memory arithmetic could not protect destination page");
        action.reset();
        return;
    }

    testRunCPU();
    context.memory->mprotect(context.thread, targetPage, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);

    if (action.sigInfo[0] != K_SIGSEGV) {
        testFail("direct memory arithmetic write did not raise SIGSEGV");
    }
    if (context.memory->readd(targetAddress) != initialValue) {
        testFail("direct memory arithmetic write fault changed memory");
    }
    if (cpu->reg[3].u32 != 0x89abcdef) {
        testFail("direct memory arithmetic write fault executed fused consumer");
    }
    action.reset();
}

} // namespace

void testJitDirectArithmeticFlags() {
#ifdef BOXEDWINE_JIT
    constexpr DirectArithmeticTestShape shapes[] = {
        DirectArithmeticTestShape::Reg,
        DirectArithmeticTestShape::MemSource,
        DirectArithmeticTestShape::Imm,
        DirectArithmeticTestShape::MemReg,
        DirectArithmeticTestShape::MemImm,
    };
    for (const DirectArithmeticOpInfo& op : DIRECT_ARITHMETIC_OPS) {
        for (DirectArithmeticTestShape shape : shapes) {
            for (const auto& values : DIRECT_ARITHMETIC_CASES) {
                for (U8 condition = 0; condition < 16; ++condition) {
                    runDirectArithmeticFlagsCase(op, shape, values[0], values[1], condition);
                }
            }
        }
    }
    runDirectArithmeticWriteFaultCase();
#endif
}

namespace {

enum class DirectIncDecTestOp {
    Inc,
    Dec
};

enum class DirectIncDecCFObserver {
    Lahf,
    PushF,
    Salc
};

constexpr U32 DIRECT_INC_DEC_CASES[] = {
    0,
    1,
    0x0f,
    0xffffffff,
    0x7fffffff,
    0x80000000,
    0xaaaaaaaa,
    0x55555555,
};

bool directIncDecConditionUsesCF(U8 condition) {
    return condition == 0x2 || condition == 0x3 || condition == 0x6 || condition == 0x7;
}

void runDirectIncDecFlagsCase(DirectIncDecTestOp op, U32 value, bool oldCF, U8 condition) {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    U32 expectedResult = 0;
    U32 expectedFlags = directArithmeticFlags(
        op == DirectIncDecTestOp::Inc ? DirectArithmeticTestOp::Add : DirectArithmeticTestOp::Sub,
        value, 1, expectedResult);
    expectedFlags = (expectedFlags & ~CF) | (oldCF ? CF : 0);
    U32 expectedCondition = directArithmeticCondition(condition, expectedFlags) ? 1 : 0;

    testNewInstruction(0);
    cpu->big = true;
    cpu->reg[0].u32 = value;      // eax: INC/DEC destination
    cpu->reg[3].u32 = 0x12345678; // ebx: SETcc destination
    cpu->reg[6].u32 = 1;          // esi
    cpu->reg[7].u32 = 2;          // edi

    testPushCode8(oldCF ? 0xf9 : 0xf8); // stc/clc
    testPushCode8(0xff);
    testPushCode8(op == DirectIncDecTestOp::Inc ? 0xc0 : 0xc8); // inc/dec eax
    testPushCode8(0x0f);
    testPushCode8(0x90 + condition); // setcc bl
    testPushCode8(0xc3);
    testPushCode8(0x39); // cmp esi,edi; makes the INC/DEC flags dead
    testPushCode8(0xfe);
    testRunCPU();

    const char* opName = op == DirectIncDecTestOp::Inc ? "inc" : "dec";
    if (cpu->reg[0].u32 != expectedResult) {
        testFail("direct %s condition %x old CF %d result", opName, condition, oldCF);
    }
    if ((cpu->reg[3].u32 & 0xff) != expectedCondition) {
        testFail("direct %s condition %x old CF %d flags", opName, condition, oldCF);
    }

#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)
    DecodedOp* producer = context.memory->getDecodedOp(TEST_CODE_ADDRESS + 1);
    DecodedOp* set = context.memory->getDecodedOp(TEST_CODE_ADDRESS + 3);
    if (!producer || !set) {
        testFail("direct %s condition %x old CF %d metadata decode", opName, condition, oldCF);
        return;
    }
    bool expectedDirect = !directIncDecConditionUsesCF(condition);
    bool wasDirect = (set->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) != 0;
    if (wasDirect != expectedDirect) {
        testFail("direct %s condition %x old CF %d selection", opName, condition, oldCF);
    }
    if (expectedDirect && (producer->pfn != cpu->thread->process->startJITOp ||
        !producer->pfnJitCode || set->pfnJitCode)) {
        testFail("direct %s condition %x old CF %d metadata", opName, condition, oldCF);
    }
#endif
}

const char* directIncDecCFObserverName(DirectIncDecCFObserver observer) {
    switch (observer) {
    case DirectIncDecCFObserver::Lahf: return "lahf";
    case DirectIncDecCFObserver::PushF: return "pushf";
    case DirectIncDecCFObserver::Salc: return "salc";
    }
    return "unknown";
}

void runDirectIncDecInterveningCFReaderCase(DirectIncDecTestOp op, bool oldCF,
                                            DirectIncDecCFObserver observer) {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    U32 value = op == DirectIncDecTestOp::Inc ? 0xffffffff : 1;

    testNewInstruction(0);
    cpu->big = true;
    cpu->reg[0].u32 = 0x12340000; // EAX receives LAHF/SALC results
    cpu->reg[1].u32 = value;      // ECX is the INC/DEC destination
    cpu->reg[2].u32 = 0;          // EDX receives PUSHFD/POP
    cpu->reg[3].u32 = 0x12345678; // EBX is the SETcc destination
    cpu->reg[6].u32 = 1;          // ESI
    cpu->reg[7].u32 = 2;          // EDI

    testPushCode8(oldCF ? 0xf9 : 0xf8); // stc/clc
    testPushCode8(0xff);
    testPushCode8(op == DirectIncDecTestOp::Inc ? 0xc1 : 0xc9); // inc/dec ecx
    switch (observer) {
    case DirectIncDecCFObserver::Lahf:
        testPushCode8(0x9f);
        break;
    case DirectIncDecCFObserver::PushF:
        testPushCode8(0x9c); // pushfd
        testPushCode8(0x5a); // pop edx
        break;
    case DirectIncDecCFObserver::Salc:
        testPushCode8(0xd6);
        break;
    }
    U32 setOffset = observer == DirectIncDecCFObserver::PushF ? 5 : 4;
    testPushCode8(0x0f);
    testPushCode8(0x94); // setz bl
    testPushCode8(0xc3);
    testPushCode8(0x39); // cmp esi,edi; makes the INC/DEC flags dead
    testPushCode8(0xfe);
    testRunCPU();

    const char* opName = op == DirectIncDecTestOp::Inc ? "inc" : "dec";
    const char* observerName = directIncDecCFObserverName(observer);
    if (cpu->reg[1].u32 != 0) {
        testFail("direct %s intervening %s old CF %d result", opName, observerName, oldCF);
    }
    bool observedCF;
    if (observer == DirectIncDecCFObserver::Lahf) {
        observedCF = (cpu->reg[0].u32 & 0x100) != 0;
    } else if (observer == DirectIncDecCFObserver::PushF) {
        observedCF = (cpu->reg[2].u32 & CF) != 0;
    } else {
        observedCF = (cpu->reg[0].u32 & 0xff) != 0;
    }
    if (observedCF != oldCF) {
        testFail("direct %s intervening %s old CF %d preserved CF", opName, observerName, oldCF);
    }
    if ((cpu->reg[3].u32 & 0xff) != 1) {
        testFail("direct %s intervening %s old CF %d final condition", opName, observerName, oldCF);
    }

#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)
    DecodedOp* set = context.memory->getDecodedOp(TEST_CODE_ADDRESS + setOffset);
    if (!set || (set->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE)) {
        testFail("direct %s intervening %s old CF %d selection", opName, observerName, oldCF);
    }
#endif
}

} // namespace

void testJitDirectIncDecFlags() {
#ifdef BOXEDWINE_JIT
    for (DirectIncDecTestOp op : {DirectIncDecTestOp::Inc, DirectIncDecTestOp::Dec}) {
        for (U32 value : DIRECT_INC_DEC_CASES) {
            for (bool oldCF : {false, true}) {
                for (U8 condition = 0; condition < 16; ++condition) {
                    runDirectIncDecFlagsCase(op, value, oldCF, condition);
                }
            }
        }
    }
    for (DirectIncDecTestOp op : {DirectIncDecTestOp::Inc, DirectIncDecTestOp::Dec}) {
        for (bool oldCF : {false, true}) {
            for (DirectIncDecCFObserver observer : {DirectIncDecCFObserver::Lahf,
                                                    DirectIncDecCFObserver::PushF,
                                                    DirectIncDecCFObserver::Salc}) {
                runDirectIncDecInterveningCFReaderCase(op, oldCF, observer);
            }
        }
    }
#endif
}

void testJitDirectNegFlags() {
#ifdef BOXEDWINE_JIT
    for (U32 value : DIRECT_INC_DEC_CASES) {
        for (U8 condition = 0; condition < 16; ++condition) {
            TestContext& context = testContext();
            CPU* cpu = context.cpu;
            U32 expectedResult = 0;
            U32 expectedFlags = directArithmeticFlags(
                DirectArithmeticTestOp::Sub, 0, value, expectedResult);
            U32 expectedCondition = directArithmeticCondition(condition, expectedFlags) ? 1 : 0;

            testNewInstruction(0);
            cpu->big = true;
            cpu->reg[0].u32 = value;      // eax: NEG destination
            cpu->reg[3].u32 = 0x12345678; // ebx: SETcc destination
            cpu->reg[6].u32 = 1;          // esi
            cpu->reg[7].u32 = 2;          // edi

            testPushCode8(0xf7);
            testPushCode8(0xd8); // neg eax
            testPushCode8(0x0f);
            testPushCode8(0x90 + condition); // setcc bl
            testPushCode8(0xc3);
            testPushCode8(0x39); // cmp esi,edi; makes the NEG flags dead
            testPushCode8(0xfe);
            testRunCPU();

            if (cpu->reg[0].u32 != expectedResult) {
                testFail("direct neg condition %x result", condition);
            }
            if ((cpu->reg[3].u32 & 0xff) != expectedCondition) {
                testFail("direct neg condition %x flags", condition);
            }

#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)
            DecodedOp* producer = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
            DecodedOp* set = context.memory->getDecodedOp(TEST_CODE_ADDRESS + 2);
            if (!producer || !set) {
                testFail("direct neg condition %x metadata decode", condition);
                continue;
            }
            if (producer->pfn != cpu->thread->process->startJITOp || !producer->pfnJitCode ||
                !(set->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) || set->pfnJitCode) {
                testFail("direct neg condition %x metadata", condition);
            }
#endif
        }
    }
#endif
}

namespace {

enum class DirectCarryTestOp {
    Adc,
    Sbb
};

enum class DirectCarryTestShape {
    Reg,
    RegSelf,
    Mem,
    Imm
};

struct DirectCarryOpInfo {
    DirectCarryTestOp op;
    const char* name;
    U8 rmOpcode;
    U8 immediateGroup;
};

constexpr DirectCarryOpInfo DIRECT_CARRY_OPS[] = {
    {DirectCarryTestOp::Adc, "adc", 0x13, 2},
    {DirectCarryTestOp::Sbb, "sbb", 0x1b, 3},
};

U32 directCarryFlags(DirectCarryTestOp op, U32 lhs, U32 rhs, U32 carry, U32& result) {
    U32 flags = 0;
    if (op == DirectCarryTestOp::Adc) {
        U64 fullResult = (U64)lhs + rhs + carry;
        result = (U32)fullResult;
        if (fullResult > 0xffffffffULL) flags |= CF;
        if ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000) != 0) flags |= OF;
    } else {
        U64 fullSubtrahend = (U64)rhs + carry;
        result = lhs - rhs - carry;
        if ((U64)lhs < fullSubtrahend) flags |= CF;
        if (((lhs ^ rhs) & (lhs ^ result) & 0x80000000) != 0) flags |= OF;
    }
    if (((lhs ^ rhs ^ result) & 0x10) != 0) flags |= AF;
    if (result == 0) flags |= ZF;
    if (result & 0x80000000) flags |= SF;
    flags |= directArithmeticParityFlag(result);
    return flags;
}

U32 emitDirectCarry(const DirectCarryOpInfo& op, DirectCarryTestShape shape, U32 rhs) {
    if (shape == DirectCarryTestShape::Reg || shape == DirectCarryTestShape::RegSelf) {
        testPushCode8(op.rmOpcode);
        testPushCode8(shape == DirectCarryTestShape::Reg ? 0xc2 : 0xc0); // op eax,edx/eax
        return 2;
    }
    if (shape == DirectCarryTestShape::Mem) {
        testPushCode8(op.rmOpcode);
        testPushCode8(0x05); // op eax,[disp32]
        testPushCode32(DIRECT_ARITHMETIC_MEM_SRC);
        return 6;
    }
    testPushCode8(0x81);
    testPushCode8(0xc0 | (op.immediateGroup << 3)); // op eax,imm32
    testPushCode32(rhs);
    return 6;
}

const char* directCarryShapeName(DirectCarryTestShape shape) {
    switch (shape) {
    case DirectCarryTestShape::Reg: return "reg";
    case DirectCarryTestShape::RegSelf: return "self";
    case DirectCarryTestShape::Mem: return "mem";
    default: return "imm";
    }
}

void runDirectCarryFlagsCase(const DirectCarryOpInfo& op, DirectCarryTestShape shape,
                             U32 lhs, U32 rhs, bool carry, U8 condition) {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    U32 effectiveRhs = shape == DirectCarryTestShape::RegSelf ? lhs : rhs;
    U32 expectedResult = 0;
    U32 expectedFlags = directCarryFlags(op.op, lhs, effectiveRhs, carry ? 1 : 0, expectedResult);
    U32 expectedCondition = directArithmeticCondition(condition, expectedFlags) ? 1 : 0;

    testNewInstruction(0);
    cpu->big = true;
    cpu->reg[0].u32 = lhs;        // eax: arithmetic destination
    cpu->reg[2].u32 = rhs;        // edx: register source
    cpu->reg[3].u32 = 0x12345678; // ebx: SETcc destination
    cpu->reg[6].u32 = 1;          // esi
    cpu->reg[7].u32 = 2;          // edi
    context.memory->writed(TEST_HEAP_ADDRESS + DIRECT_ARITHMETIC_MEM_SRC, rhs);

    testPushCode8(carry ? 0xf9 : 0xf8); // stc/clc
    U32 producerLen = emitDirectCarry(op, shape, rhs);
    testPushCode8(0x0f);
    testPushCode8(0x90 + condition); // setcc bl
    testPushCode8(0xc3);
    testPushCode8(0x39); // cmp esi,edi; makes the ADC/SBB flags dead
    testPushCode8(0xfe);
    testRunCPU();

    const char* shapeName = directCarryShapeName(shape);
    if (cpu->reg[0].u32 != expectedResult) {
        testFail("direct %s %s condition %x carry %d result", op.name, shapeName, condition, carry);
    }
    if ((cpu->reg[3].u32 & 0xff) != expectedCondition) {
        testFail("direct %s %s condition %x carry %d flags", op.name, shapeName, condition, carry);
    }

#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)
    DecodedOp* producer = context.memory->getDecodedOp(TEST_CODE_ADDRESS + 1);
    DecodedOp* set = context.memory->getDecodedOp(TEST_CODE_ADDRESS + 1 + producerLen);
    if (!producer || !set) {
        testFail("direct %s %s condition %x carry %d metadata decode", op.name, shapeName, condition, carry);
        return;
    }
    if (producer->pfn != cpu->thread->process->startJITOp || !producer->pfnJitCode ||
        !(set->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) || set->pfnJitCode) {
        testFail("direct %s %s condition %x carry %d metadata", op.name, shapeName, condition, carry);
    }
#endif
}

} // namespace

void testJitDirectAdcSbbFlags() {
#ifdef BOXEDWINE_JIT
    constexpr DirectCarryTestShape shapes[] = {
        DirectCarryTestShape::Reg,
        DirectCarryTestShape::RegSelf,
        DirectCarryTestShape::Mem,
        DirectCarryTestShape::Imm,
    };
    for (const DirectCarryOpInfo& op : DIRECT_CARRY_OPS) {
        for (DirectCarryTestShape shape : shapes) {
            for (const auto& values : DIRECT_ARITHMETIC_CASES) {
                for (bool carry : {false, true}) {
                    for (U8 condition = 0; condition < 16; ++condition) {
                        runDirectCarryFlagsCase(op, shape, values[0], values[1], carry, condition);
                    }
                }
            }
        }
    }
#endif
}

namespace {

enum class DirectShiftTestOp {
    Shl,
    Shr,
    Sar
};

struct DirectShiftOpInfo {
    DirectShiftTestOp op;
    const char* name;
    U8 group;
};

constexpr DirectShiftOpInfo DIRECT_SHIFT_OPS[] = {
    {DirectShiftTestOp::Shl, "shl", 4},
    {DirectShiftTestOp::Shr, "shr", 5},
    {DirectShiftTestOp::Sar, "sar", 7},
};

constexpr U8 DIRECT_SHIFT_COUNTS[] = {1, 2, 7, 16, 31};

bool directShiftConditionUsesOF(U8 condition) {
    return condition < 2 || condition >= 0xc;
}

U32 directShiftFlags(DirectShiftTestOp op, U32 value, U8 count, U32& result) {
    U32 flags = 0;
    if (op == DirectShiftTestOp::Shl) {
        result = value << count;
        if ((value >> (32 - count)) & 1) flags |= CF;
        if (count == 1 && ((value ^ result) & 0x80000000)) flags |= OF;
    } else if (op == DirectShiftTestOp::Shr) {
        result = value >> count;
        if ((value >> (count - 1)) & 1) flags |= CF;
        if (count == 1 && (value & 0x80000000)) flags |= OF;
    } else {
        result = value >> count;
        if (value & 0x80000000) {
            result |= 0xffffffffU << (32 - count);
        }
        if ((value >> (count - 1)) & 1) flags |= CF;
    }
    if (result == 0) flags |= ZF;
    if (result & 0x80000000) flags |= SF;
    flags |= directArithmeticParityFlag(result);
    return flags;
}

void runDirectShiftFlagsCase(const DirectShiftOpInfo& op, U32 value, U8 count, U8 condition) {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    U32 expectedResult = 0;
    U32 expectedFlags = directShiftFlags(op.op, value, count, expectedResult);
    U32 expectedCondition = directArithmeticCondition(condition, expectedFlags) ? 1 : 0;

    testNewInstruction(0);
    cpu->big = true;
    cpu->reg[0].u32 = value;      // eax: shift destination
    cpu->reg[3].u32 = 0x12345678; // ebx: SETcc destination
    cpu->reg[6].u32 = 1;          // esi
    cpu->reg[7].u32 = 2;          // edi

    testPushCode8(0xc1);
    testPushCode8(0xc0 | (op.group << 3)); // shift eax,imm8
    testPushCode8(count);
    testPushCode8(0x0f);
    testPushCode8(0x90 + condition); // setcc bl
    testPushCode8(0xc3);
    testPushCode8(0x39); // cmp esi,edi; makes the shift flags dead
    testPushCode8(0xfe);
    testRunCPU();

    if (cpu->reg[0].u32 != expectedResult) {
        testFail("direct %s count %u condition %x value %x result %x expected %x",
            op.name, count, condition, value, cpu->reg[0].u32, expectedResult);
    }
    if ((count == 1 || !directShiftConditionUsesOF(condition)) &&
        (cpu->reg[3].u32 & 0xff) != expectedCondition) {
        testFail("direct %s count %u condition %x flags", op.name, count, condition);
    }

#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)
    DecodedOp* producer = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
    DecodedOp* set = context.memory->getDecodedOp(TEST_CODE_ADDRESS + 3);
    if (!producer || !set) {
        testFail("direct %s count %u condition %x metadata decode", op.name, count, condition);
        return;
    }
    if (producer->pfn != cpu->thread->process->startJITOp || !producer->pfnJitCode ||
        !(set->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) || set->pfnJitCode) {
        testFail("direct %s count %u condition %x metadata", op.name, count, condition);
    }
#endif
}

} // namespace

void testJitDirectShiftFlags() {
#ifdef BOXEDWINE_JIT
    for (const DirectShiftOpInfo& op : DIRECT_SHIFT_OPS) {
        for (U32 value : DIRECT_INC_DEC_CASES) {
            for (U8 count : DIRECT_SHIFT_COUNTS) {
                for (U8 condition = 0; condition < 16; ++condition) {
                    runDirectShiftFlagsCase(op, value, count, condition);
                }
            }
        }
    }
#endif
}

namespace {

enum class DirectDoubleShiftTestOp {
    Shld,
    Shrd
};

struct DirectDoubleShiftOpInfo {
    DirectDoubleShiftTestOp op;
    const char* name;
    U8 opcode;
};

constexpr DirectDoubleShiftOpInfo DIRECT_DOUBLE_SHIFT_OPS[] = {
    {DirectDoubleShiftTestOp::Shld, "shld", 0xa4},
    {DirectDoubleShiftTestOp::Shrd, "shrd", 0xac},
};

U32 directDoubleShiftFlags(DirectDoubleShiftTestOp op, U32 dest, U32 src, U8 count, U32& result) {
    U32 flags = 0;
    if (op == DirectDoubleShiftTestOp::Shld) {
        result = (dest << count) | (src >> (32 - count));
        if ((dest >> (32 - count)) & 1) flags |= CF;
    } else {
        result = (dest >> count) | (src << (32 - count));
        if ((dest >> (count - 1)) & 1) flags |= CF;
    }
    if (count == 1 && ((dest ^ result) & 0x80000000)) flags |= OF;
    if (result == 0) flags |= ZF;
    if (result & 0x80000000) flags |= SF;
    flags |= directArithmeticParityFlag(result);
    return flags;
}

void runDirectDoubleShiftFlagsCase(const DirectDoubleShiftOpInfo& op,
                                   U32 dest, U32 src, U8 count, U8 condition) {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    U32 expectedResult = 0;
    U32 expectedFlags = directDoubleShiftFlags(op.op, dest, src, count, expectedResult);
    U32 expectedCondition = directArithmeticCondition(condition, expectedFlags) ? 1 : 0;

    testNewInstruction(0);
    cpu->big = true;
    cpu->reg[0].u32 = dest;       // eax: double-shift destination
    cpu->reg[2].u32 = src;        // edx: double-shift source
    cpu->reg[3].u32 = 0x12345678; // ebx: SETcc destination
    cpu->reg[6].u32 = 1;          // esi
    cpu->reg[7].u32 = 2;          // edi

    testPushCode8(0x0f);
    testPushCode8(op.opcode);
    testPushCode8(0xd0); // shld/shrd eax,edx,imm8
    testPushCode8(count);
    testPushCode8(0x0f);
    testPushCode8(0x90 + condition); // setcc bl
    testPushCode8(0xc3);
    testPushCode8(0x39); // cmp esi,edi; makes the double-shift flags dead
    testPushCode8(0xfe);
    testRunCPU();

    if (cpu->reg[0].u32 != expectedResult) {
        testFail("direct %s count %u condition %x dest %x src %x result %x expected %x",
            op.name, count, condition, dest, src, cpu->reg[0].u32, expectedResult);
    }
    if ((count == 1 || !directShiftConditionUsesOF(condition)) &&
        (cpu->reg[3].u32 & 0xff) != expectedCondition) {
        testFail("direct %s count %u condition %x dest %x src %x flags",
            op.name, count, condition, dest, src);
    }

#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)
    DecodedOp* producer = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
    DecodedOp* set = context.memory->getDecodedOp(TEST_CODE_ADDRESS + 4);
    if (!producer || !set) {
        testFail("direct %s count %u condition %x metadata decode", op.name, count, condition);
        return;
    }
    if (producer->pfn != cpu->thread->process->startJITOp || !producer->pfnJitCode ||
        !(set->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) || set->pfnJitCode) {
        testFail("direct %s count %u condition %x metadata", op.name, count, condition);
    }
#endif
}

} // namespace

void testJitDirectDoubleShiftFlags() {
#ifdef BOXEDWINE_JIT
    for (const DirectDoubleShiftOpInfo& op : DIRECT_DOUBLE_SHIFT_OPS) {
        for (const auto& values : DIRECT_ARITHMETIC_CASES) {
            for (U8 count : DIRECT_SHIFT_COUNTS) {
                for (U8 condition = 0; condition < 16; ++condition) {
                    runDirectDoubleShiftFlagsCase(op, values[0], values[1], count, condition);
                }
            }
        }
    }
#endif
}

namespace {

enum class DirectBitTestShape {
    RegReg,
    RegImm,
    MemReg,
    MemImm
};

constexpr U32 DIRECT_BIT_TEST_MEM = 0x380;

struct DirectBitTestCase {
    U32 value;
    U32 bit;
};

constexpr DirectBitTestCase DIRECT_BIT_TEST_CASES[] = {
    {0x00000000, 0},
    {0x00000001, 0},
    {0x00000002, 1},
    {0x00008000, 15},
    {0x7fffffff, 31},
    {0x80000000, 31},
    {0x00000001, 32},
    {0x00000002, 33},
};

void runDirectBitTestCarryCase(DirectBitTestShape shape, U32 value, U32 bit, U8 condition) {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    bool memoryValue = shape == DirectBitTestShape::MemReg || shape == DirectBitTestShape::MemImm;
    bool registerBit = shape == DirectBitTestShape::RegReg || shape == DirectBitTestShape::MemReg;
    U32 expectedCondition = (value >> (bit & 31)) & 1;
    if (condition == 3) {
        expectedCondition ^= 1; // SETNB is the inverse of SETB.
    }

    testNewInstruction(0);
    cpu->big = true;
    cpu->reg[0].u32 = memoryValue ? 0x89abcdef : value; // eax: register bit-test value
    cpu->reg[2].u32 = bit;        // edx: register bit index
    cpu->reg[3].u32 = 0x12345678; // ebx: SETcc destination
    cpu->reg[6].u32 = 1;          // esi
    cpu->reg[7].u32 = 2;          // edi

    U32 producerLen = 0;
    if (!memoryValue && registerBit) {
        testPushCode8(0x0f);
        testPushCode8(0xa3);
        testPushCode8(0xd0); // bt eax,edx
        producerLen = 3;
    } else if (!memoryValue) {
        testPushCode8(0x0f);
        testPushCode8(0xba);
        testPushCode8(0xe0); // bt eax,imm8
        testPushCode8((U8)bit);
        producerLen = 4;
    } else if (registerBit) {
        testPushCode8(0x0f);
        testPushCode8(0xa3);
        testPushCode8(0x15); // bt [disp32],edx
        testPushCode32(DIRECT_BIT_TEST_MEM);
        producerLen = 7;
    } else {
        testPushCode8(0x0f);
        testPushCode8(0xba);
        testPushCode8(0x25); // bt [disp32],imm8
        testPushCode32(DIRECT_BIT_TEST_MEM);
        testPushCode8((U8)bit);
        producerLen = 8;
    }
    testPushCode8(0x0f);
    testPushCode8(0x90 + condition); // setb/setnb bl
    testPushCode8(0xc3);
    testPushCode8(0x39); // cmp esi,edi; makes CF dead
    testPushCode8(0xfe);

    U32 valueAddress = TEST_HEAP_ADDRESS + DIRECT_BIT_TEST_MEM;
    if (memoryValue && registerBit) {
        valueAddress += (bit >> 5) * sizeof(U32);
    }
    context.memory->writed(valueAddress, value);
    testRunCPU();

    const char* shapeName = shape == DirectBitTestShape::RegReg ? "reg,reg" :
        shape == DirectBitTestShape::RegImm ? "reg,imm" :
        shape == DirectBitTestShape::MemReg ? "mem,reg" : "mem,imm";
    U32 expectedEax = memoryValue ? 0x89abcdef : value;
    if (cpu->reg[0].u32 != expectedEax) {
        testFail("direct bt %s condition %x value %x bit %x changed value", shapeName, condition, value, bit);
    }
    if (memoryValue && context.memory->readd(valueAddress) != value) {
        testFail("direct bt %s condition %x value %x bit %x changed memory", shapeName, condition, value, bit);
    }
    if ((cpu->reg[3].u32 & 0xff) != expectedCondition) {
        testFail("direct bt %s condition %x value %x bit %x flags", shapeName, condition, value, bit);
    }

#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)
    DecodedOp* producer = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
    DecodedOp* set = context.memory->getDecodedOp(TEST_CODE_ADDRESS + producerLen);
    if (!producer || !set) {
        testFail("direct bt %s condition %x metadata decode", shapeName, condition);
        return;
    }
    if (producer->pfn != cpu->thread->process->startJITOp || !producer->pfnJitCode ||
        !(set->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) || set->pfnJitCode) {
        testFail("direct bt %s condition %x metadata", shapeName, condition);
    }
#endif
}

} // namespace

void testJitDirectBitTestCarry() {
#ifdef BOXEDWINE_JIT
    for (DirectBitTestShape shape : {DirectBitTestShape::RegReg, DirectBitTestShape::RegImm,
                                    DirectBitTestShape::MemReg, DirectBitTestShape::MemImm}) {
        for (const DirectBitTestCase& test : DIRECT_BIT_TEST_CASES) {
            for (U8 condition : {2, 3}) {
                runDirectBitTestCarryCase(shape, test.value, test.bit, condition);
            }
        }
    }
#endif
}

namespace {

enum class DirectRotateOneTestOp {
    Rol,
    Ror
};

struct DirectRotateOneOpInfo {
    DirectRotateOneTestOp op;
    const char* name;
    U8 group;
};

constexpr DirectRotateOneOpInfo DIRECT_ROTATE_ONE_OPS[] = {
    {DirectRotateOneTestOp::Rol, "rol", 0},
    {DirectRotateOneTestOp::Ror, "ror", 1},
};

U32 directRotateOneFlags(DirectRotateOneTestOp op, U32 value, U32& result) {
    U32 flags = 0;
    if (op == DirectRotateOneTestOp::Rol) {
        result = (value << 1) | (value >> 31);
        if (result & 1) flags |= CF;
        if (((result >> 31) ^ result) & 1) flags |= OF;
    } else {
        result = (value >> 1) | (value << 31);
        if (result & 0x80000000) flags |= CF;
        if (((result >> 31) ^ (result >> 30)) & 1) flags |= OF;
    }
    return flags;
}

void runDirectRotateOneFlagsCase(const DirectRotateOneOpInfo& op, U32 value, U8 condition) {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    U32 expectedResult = 0;
    U32 expectedFlags = directRotateOneFlags(op.op, value, expectedResult);
    U32 expectedCondition = directArithmeticCondition(condition, expectedFlags) ? 1 : 0;

    testNewInstruction(0);
    cpu->big = true;
    cpu->reg[0].u32 = value;      // eax: rotate destination
    cpu->reg[3].u32 = 0x12345678; // ebx: SETcc destination
    cpu->reg[6].u32 = 1;          // esi
    cpu->reg[7].u32 = 2;          // edi

    testPushCode8(0xd1);
    testPushCode8(0xc0 | (op.group << 3)); // rol/ror eax,1
    testPushCode8(0x0f);
    testPushCode8(0x90 + condition); // seto/setno/setb/setnb bl
    testPushCode8(0xc3);
    testPushCode8(0x39); // cmp esi,edi; makes the rotate flags dead
    testPushCode8(0xfe);
    testRunCPU();

    if (cpu->reg[0].u32 != expectedResult) {
        testFail("direct %s one condition %x value %x result %x expected %x",
            op.name, condition, value, cpu->reg[0].u32, expectedResult);
    }
    if ((cpu->reg[3].u32 & 0xff) != expectedCondition) {
        testFail("direct %s one condition %x value %x flags", op.name, condition, value);
    }

#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)
    DecodedOp* producer = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
    DecodedOp* set = context.memory->getDecodedOp(TEST_CODE_ADDRESS + 2);
    if (!producer || !set) {
        testFail("direct %s one condition %x metadata decode", op.name, condition);
        return;
    }
    if (producer->pfn != cpu->thread->process->startJITOp || !producer->pfnJitCode ||
        !(set->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) || set->pfnJitCode) {
        testFail("direct %s one condition %x metadata", op.name, condition);
    }
#endif
}

} // namespace

void testJitDirectRotateOneFlags() {
#ifdef BOXEDWINE_JIT
    constexpr U8 conditions[] = {0, 1, 2, 3};
    for (const DirectRotateOneOpInfo& op : DIRECT_ROTATE_ONE_OPS) {
        for (U32 value : DIRECT_INC_DEC_CASES) {
            for (U8 condition : conditions) {
                runDirectRotateOneFlagsCase(op, value, condition);
            }
        }
    }
#endif
}

bool testShouldRunRegister(bool fast, int reg) {
    if (!fast) {
        return true;
    }
    static const int fastRegs[] = {0, 1, 3, 4, 5, 7};
    return intInList(reg, fastRegs, sizeof(fastRegs) / sizeof(fastRegs[0]));
}

bool testShouldRunRegisterPair(bool fast, int dst, int src) {
    if (!fast) {
        return true;
    }
    static const int fastPairs[][2] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {2, 3},
        {3, 2},
        {4, 5},
        {5, 4},
        {6, 7},
        {7, 6},
        {4, 0},
        {0, 4}
    };
    for (size_t i = 0; i < sizeof(fastPairs) / sizeof(fastPairs[0]); ++i) {
        if (fastPairs[i][0] == dst && fastPairs[i][1] == src) {
            return true;
        }
    }
    return false;
}

bool testShouldRunMemoryBase(bool fast, int base) {
    if (!fast) {
        return true;
    }
    static const int fastBases[] = {0, 3, 4, 5, 6};
    return intInList(base, fastBases, sizeof(fastBases) / sizeof(fastBases[0]));
}

bool testShouldRunMemoryBaseDisplacement(bool fast, int base, int displacementIndex) {
    if (!fast) {
        return true;
    }
    if (!testShouldRunMemoryBase(true, base)) {
        return false;
    }
    if (displacementIndex == 0) {
        return base == 0 || base == 4;
    }
    if (displacementIndex == 1) {
        return base == 3 || base == 5;
    }
    return base == 6;
}

bool testShouldRunMemorySib(bool fast, int base, int index, int shift) {
    if (!fast) {
        return true;
    }
    static const int fastSibCases[][3] = {
        {0, 1, 0},
        {0, 6, 2},
        {1, 3, 2},
        {2, 7, 0},
        {3, 2, 1},
        {4, 1, 3},
        {4, 7, 2},
        {5, 0, 3},
        {5, 4, 0},
        {5, 6, 1},
        {6, 0, 3},
        {6, 5, 0},
        {7, 3, 1}
    };
    for (size_t i = 0; i < sizeof(fastSibCases) / sizeof(fastSibCases[0]); ++i) {
        if (fastSibCases[i][0] == base && fastSibCases[i][1] == index && fastSibCases[i][2] == shift) {
            return true;
        }
    }
    return false;
}

bool testRunRegister(int reg) {
    return testShouldRunRegister(testIsFastMode(), reg);
}

bool testRunRegisterPair(int dst, int src) {
    return testShouldRunRegisterPair(testIsFastMode(), dst, src);
}

bool testRunMemoryBase(int base) {
    return testShouldRunMemoryBase(testIsFastMode(), base);
}

bool testRunMemoryBaseDisplacement(int base, int displacementIndex) {
    return testShouldRunMemoryBaseDisplacement(testIsFastMode(), base, displacementIndex);
}

bool testRunMemorySib(int base, int index, int shift) {
    return testShouldRunMemorySib(testIsFastMode(), base, index, shift);
}

void testFastModeSelectionHelpers() {
    if (!testShouldRunRegisterPair(false, 2, 6) ||
            !testShouldRunMemorySib(false, 2, 4, 3) ||
            !testShouldRunMemoryBaseDisplacement(false, 7, 2)) {
        testFail("full mode should keep exhaustive combinations");
    }
    if (!testShouldRunRegisterPair(true, 4, 0) ||
            !testShouldRunRegisterPair(true, 6, 7) ||
            testShouldRunRegisterPair(true, 2, 6)) {
        testFail("fast mode register pair sampling");
    }
    if (!testShouldRunMemoryBaseDisplacement(true, 0, 0) ||
            !testShouldRunMemoryBaseDisplacement(true, 5, 1) ||
            !testShouldRunMemoryBaseDisplacement(true, 6, 2) ||
            testShouldRunMemoryBaseDisplacement(true, 7, 2)) {
        testFail("fast mode base/displacement sampling");
    }
    if (!testShouldRunMemorySib(true, 4, 7, 2) ||
            !testShouldRunMemorySib(true, 5, 0, 3) ||
            testShouldRunMemorySib(true, 2, 4, 3)) {
        testFail("fast mode sib sampling");
    }
}

#ifdef BOXEDWINE_HOST_EXCEPTIONS
void platformInitExceptionHandling();
#endif

void testRunParallel(const TestEntry* entries, size_t entryCount, U32 workerCount) {
    if (!entryCount) {
        return;
    }

#ifdef BOXEDWINE_HOST_EXCEPTIONS
    platformInitExceptionHandling();
#endif

    ensureParallelContexts(workerCount);

    std::atomic<size_t> nextEntry(0);
    std::mutex printMutex;
    std::shared_mutex serialTestMutex;

    auto runEntry = [&](U32 contextIndex, size_t entryIndex) {
        bindParallelContext(contextIndex);
        TestContext& context = *currentContext;
        runningParallelTest = true;

        resetEntryContext(context);
        context.failed = false;
        context.failures.clear();
        if (entries[entryIndex].flags & TEST_ENTRY_SERIAL) {
            std::unique_lock<std::shared_mutex> lock(serialTestMutex);
            entries[entryIndex].function();
        } else {
            std::shared_lock<std::shared_mutex> lock(serialTestMutex);
            entries[entryIndex].function();
        }
        {
            std::lock_guard<std::mutex> lock(printMutex);
            printf("%s", entries[entryIndex].name);
            printf(" ... ");
            if (context.failed) {
                printf("FAILED\n");
                for (const std::string& failure : context.failures) {
                    printf("  %s\n", failure.c_str());
                }
                failed("%s", entries[entryIndex].name);
            } else {
                printf("OK\n");
            }
            fflush(stdout);
        }
    };

    if (workerCount == 1) {
#if defined(BOXEDWINE_JIT_ARMV8)
        ensureArmV8HardwareTSOForThread();
#endif
        for (size_t i = 0; i < entryCount; ++i) {
            runEntry(0, i);
        }
        runningParallelTest = false;
        currentContext = nullptr;
        return;
    }

    std::vector<std::thread> workers;

    for (U32 i = 0; i < workerCount; ++i) {
        workers.push_back(std::thread([&, i]() {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
            platformInitExceptionHandling();
#endif
#if defined(BOXEDWINE_JIT_ARMV8)
            ensureArmV8HardwareTSOForThread();
#endif
            while (true) {
                size_t entryIndex = nextEntry.fetch_add(1);
                if (entryIndex >= entryCount) {
                    break;
                }
                runEntry(i, entryIndex);
            }
            runningParallelTest = false;
            currentContext = nullptr;
        }));
    }

    for (auto& worker : workers) {
        worker.join();
    }
}

void testDefaultUserSegmentsUseGdtSelectors() {
    KProcessPtr process = KProcess::create();
    std::unique_ptr<KMemory> memory(KMemory::create(process.get()));
    process->memory = memory.get();
    KThread* thread = process->createThread();
    CPU* cpu = thread->cpu;

    if (cpu->seg[CS].value != BOXEDWINE_INTERNAL_USER_CODE_SELECTOR) {
        testFail("internal default CS selector");
    }
    if (cpu->seg[SS].value != BOXEDWINE_INTERNAL_USER_DATA_SELECTOR ||
        cpu->seg[DS].value != BOXEDWINE_INTERNAL_USER_DATA_SELECTOR ||
        cpu->seg[ES].value != BOXEDWINE_INTERNAL_USER_DATA_SELECTOR) {
        testFail("internal default data selectors");
    }
    if (cpu->getSegValue(CS) != BOXEDWINE_VISIBLE_USER_CODE_SELECTOR) {
        testFail("visible default CS selector");
    }
    if (cpu->getSegValue(SS) != BOXEDWINE_VISIBLE_USER_DATA_SELECTOR ||
        cpu->getSegValue(DS) != BOXEDWINE_VISIBLE_USER_DATA_SELECTOR ||
        cpu->getSegValue(ES) != BOXEDWINE_VISIBLE_USER_DATA_SELECTOR) {
        testFail("visible default data selectors");
    }
    if (!cpu->setSegment(DS, BOXEDWINE_VISIBLE_USER_DATA_SELECTOR)) {
        testFail("set visible default data selector");
    }
    if (cpu->seg[DS].value != BOXEDWINE_INTERNAL_USER_DATA_SELECTOR) {
        testFail("visible default data selector should map to internal selector");
    }
}

void testSignalHandlerSegmentsUseGdtSelectors() {
    constexpr U32 SIGNAL_STACK_TOP = 0x70000000;
    constexpr U32 SIGNAL_HANDLER = 0x12345000;

    KProcessPtr process = KProcess::create();
    std::unique_ptr<KMemory> memory(KMemory::create(process.get()));
    process->memory = memory.get();
    KThread* thread = process->createThread();
    CPU* cpu = thread->cpu;
    KThread::setCurrentThread(thread);

    memory->mmap(thread, SIGNAL_STACK_TOP - K_PAGE_SIZE, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    cpu->reg[4].u32 = SIGNAL_STACK_TOP;
    process->sigActions[K_SIGUSR1].handlerAndSigAction = SIGNAL_HANDLER;

    thread->runSignal(K_SIGUSR1, 0, 0);

    if (cpu->eip.u32 != SIGNAL_HANDLER) {
        testFail("signal handler eip");
    }
    if (cpu->seg[CS].value != BOXEDWINE_INTERNAL_USER_CODE_SELECTOR) {
        testFail("signal handler internal CS selector");
    }
    if (cpu->seg[SS].value != BOXEDWINE_INTERNAL_USER_DATA_SELECTOR ||
        cpu->seg[DS].value != BOXEDWINE_INTERNAL_USER_DATA_SELECTOR ||
        cpu->seg[ES].value != BOXEDWINE_INTERNAL_USER_DATA_SELECTOR) {
        testFail("signal handler internal data selectors");
    }
    if (cpu->getSegValue(CS) != BOXEDWINE_VISIBLE_USER_CODE_SELECTOR) {
        testFail("signal handler visible CS selector");
    }
    if (cpu->getSegValue(SS) != BOXEDWINE_VISIBLE_USER_DATA_SELECTOR ||
        cpu->getSegValue(DS) != BOXEDWINE_VISIBLE_USER_DATA_SELECTOR ||
        cpu->getSegValue(ES) != BOXEDWINE_VISIBLE_USER_DATA_SELECTOR) {
        testFail("signal handler visible data selectors");
    }
}

void testSignalAlternateStackDeliverySemantics() {
    constexpr U32 ALT_STACK_BASE = 0x70000000;
    constexpr U32 ALT_STACK_SIZE = 0x10000;
    constexpr U32 ALT_STACK_TOP = ALT_STACK_BASE + ALT_STACK_SIZE;
    constexpr U32 SIGNAL_CONTEXT_SIZE = 768;
    constexpr U32 SIGNAL_HANDLER = 0x12345000;

    KProcessPtr process = KProcess::create();
    std::unique_ptr<KMemory> memory(KMemory::create(process.get()));
    process->memory = memory.get();
    KThread* thread = process->createThread();
    CPU* cpu = thread->cpu;
    KThread::setCurrentThread(thread);

    memory->mmap(thread, ALT_STACK_BASE, ALT_STACK_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    thread->alternateStack = ALT_STACK_BASE;
    thread->alternateStackSize = ALT_STACK_SIZE;
    cpu->reg[4].u32 = TEST_STACK_ADDRESS;
    process->sigActions[K_SIGUSR1].handlerAndSigAction = SIGNAL_HANDLER;
    process->sigActions[K_SIGUSR1].flags = K_SA_ONSTACK;

    thread->runSignal(K_SIGUSR1, 0, 0);

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (context != ALT_STACK_TOP - SIGNAL_CONTEXT_SIZE) {
        testFail("first SA_ONSTACK signal must start at the alternate stack top");
    }
    if (memory->readd(context + 0x8) != ALT_STACK_BASE ||
        memory->readd(context + 0xC) != 0 ||
        memory->readd(context + 0x10) != ALT_STACK_SIZE) {
        testFail("first SA_ONSTACK signal must save the enabled, inactive alternate stack");
    }

    U32 nestedStack = ALT_STACK_BASE + ALT_STACK_SIZE / 2;
    cpu->reg[4].u32 = nestedStack;
    thread->runSignal(K_SIGUSR1, 0, 0);

    context = memory->readd(cpu->reg[4].u32 + 12);
    if (context != nestedStack - SIGNAL_CONTEXT_SIZE) {
        testFail("nested SA_ONSTACK signal must continue below the current alternate stack pointer");
    }
    if (memory->readd(context + 0x8) != ALT_STACK_BASE ||
        memory->readd(context + 0xC) != K_SS_ONSTACK ||
        memory->readd(context + 0x10) != ALT_STACK_SIZE) {
        testFail("nested SA_ONSTACK signal must save the active alternate stack");
    }
}

void testSignalOnStackWithoutConfiguredAlternateStack() {
    constexpr U32 SIGNAL_STACK_TOP = 0x70000000;
    constexpr U32 SIGNAL_CONTEXT_SIZE = 768;
    constexpr U32 SIGNAL_HANDLER = 0x12345000;

    KProcessPtr process = KProcess::create();
    std::unique_ptr<KMemory> memory(KMemory::create(process.get()));
    process->memory = memory.get();
    KThread* thread = process->createThread();
    CPU* cpu = thread->cpu;
    KThread::setCurrentThread(thread);

    memory->mmap(thread, SIGNAL_STACK_TOP - K_PAGE_SIZE, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    cpu->reg[4].u32 = SIGNAL_STACK_TOP;
    process->sigActions[K_SIGUSR1].handlerAndSigAction = SIGNAL_HANDLER;
    process->sigActions[K_SIGUSR1].flags = K_SA_ONSTACK;

    thread->runSignal(K_SIGUSR1, 0, 0);

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (context != SIGNAL_STACK_TOP - SIGNAL_CONTEXT_SIZE) {
        testFail("SA_ONSTACK without a configured alternate stack must use the current stack");
    }
    if (memory->readd(context + 0x8) != 0 ||
        memory->readd(context + 0xC) != K_SS_DISABLE ||
        memory->readd(context + 0x10) != 0) {
        testFail("SA_ONSTACK without a configured alternate stack must save SS_DISABLE");
    }
}

void testSigaltstackReportsActualStackState() {
    constexpr U32 ALT_STACK_BASE = 0x70000000;
    constexpr U32 ALT_STACK_SIZE = 0x10000;
    constexpr U32 STRUCT_PAGE = 0x60000000;
    constexpr U32 NEW_STACK = STRUCT_PAGE;
    constexpr U32 OLD_STACK = STRUCT_PAGE + 16;

    KProcessPtr process = KProcess::create();
    std::unique_ptr<KMemory> memory(KMemory::create(process.get()));
    process->memory = memory.get();
    KThread* thread = process->createThread();
    CPU* cpu = thread->cpu;
    KThread::setCurrentThread(thread);

    memory->mmap(thread, ALT_STACK_BASE, ALT_STACK_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    memory->mmap(thread, STRUCT_PAGE, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    thread->alternateStack = ALT_STACK_BASE;
    thread->alternateStackSize = ALT_STACK_SIZE;
    cpu->seg[SS].address = 0;
    cpu->stackMask = 0xffffffff;

    cpu->reg[4].u32 = TEST_STACK_ADDRESS;
    thread->inSignal = 1;
    if (thread->signalstack(0, OLD_STACK) != 0 ||
        memory->readd(OLD_STACK) != ALT_STACK_BASE ||
        memory->readd(OLD_STACK + 4) != 0 ||
        memory->readd(OLD_STACK + 8) != ALT_STACK_SIZE) {
        testFail("sigaltstack must report an enabled stack as inactive while ESP is outside it");
    }

    memory->writed(NEW_STACK, ALT_STACK_BASE);
    memory->writed(NEW_STACK + 4, 0);
    memory->writed(NEW_STACK + 8, ALT_STACK_SIZE);
    if (thread->signalstack(NEW_STACK, 0) != 0) {
        testFail("sigaltstack must allow updates from a signal handler running on the normal stack");
    }

    cpu->reg[4].u32 = ALT_STACK_BASE + ALT_STACK_SIZE / 2;
    thread->inSignal = 0;
    if (thread->signalstack(0, OLD_STACK) != 0 || memory->readd(OLD_STACK + 4) != K_SS_ONSTACK) {
        testFail("sigaltstack must report SS_ONSTACK whenever ESP is inside the alternate stack");
    }

    memory->writed(NEW_STACK + 4, K_SS_DISABLE);
    if (thread->signalstack(NEW_STACK, 0) != (U32)-K_EPERM) {
        testFail("sigaltstack must reject changes while ESP is inside the alternate stack");
    }
}

void testSignalReturnPreservesLoadedInvalidTlsSelector() {
    constexpr U32 SIGNAL_STACK_TOP = 0x70000000;
    constexpr U32 SIGNAL_HANDLER = 0x12345000;
    constexpr U32 GS_SELECTOR = (TLS_ENTRY_START_INDEX << 3) | 3;

    KProcessPtr process = KProcess::create();
    std::unique_ptr<KMemory> memory(KMemory::create(process.get()));
    process->memory = memory.get();
    KThread* thread = process->createThread();
    CPU* cpu = thread->cpu;
    KThread::setCurrentThread(thread);

    memory->mmap(thread, SIGNAL_STACK_TOP - K_PAGE_SIZE, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    memory->mmap(thread, TEST_CODE_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    memory->writeb(TEST_CODE_ADDRESS, 0x90);

    struct user_desc tls = {};
    tls.entry_number = TLS_ENTRY_START_INDEX;
    tls.base_addr = TEST_HEAP_ADDRESS;
    tls.limit = 0xFFFFF;
    tls.seg_32bit = 1;
    tls.limit_in_pages = 1;
    tls.seg_not_present = 0;
    tls.useable = 1;
    thread->setTLS(&tls);

    if (!cpu->setSegment(GS, GS_SELECTOR)) {
        testFail("set GS to TLS selector");
    }
    cpu->reg[4].u32 = SIGNAL_STACK_TOP;
    cpu->eip.u32 = TEST_CODE_ADDRESS;
    process->sigActions[K_SIGUSR1].handlerAndSigAction = SIGNAL_HANDLER;

    thread->runSignal(K_SIGUSR1, 0, 0);
    if (cpu->eip.u32 != SIGNAL_HANDLER) {
        testFail("signal handler eip before invalid TLS return");
    }

    struct user_desc emptyTls = {};
    emptyTls.entry_number = TLS_ENTRY_START_INDEX;
    emptyTls.read_exec_only = 1;
    emptyTls.seg_not_present = 1;
    thread->setTLS(&emptyTls);

    U32 returnAddress = cpu->pop32();
    if (returnAddress != SIG_RETURN_ADDRESS) {
        testFail("signal return callback address");
    }
    onExitSignal(cpu, nullptr);

    if (cpu->seg[GS].value != GS_SELECTOR || cpu->seg[GS].address != TEST_HEAP_ADDRESS) {
        testFail("already-loaded invalid GS selector should be preserved on signal return");
    }
    if (cpu->eip.u32 != TEST_CODE_ADDRESS) {
        testFail("signal return eip after invalid TLS selector");
    }
}

void testSignalReturnDiscardsHandlerLazyFlags() {
    constexpr U32 SIGNAL_STACK_TOP = 0x70000000;
    constexpr U32 SIGNAL_HANDLER = 0x12345000;

    KProcessPtr process = KProcess::create();
    std::unique_ptr<KMemory> memory(KMemory::create(process.get()));
    process->memory = memory.get();
    KThread* thread = process->createThread();
    CPU* cpu = thread->cpu;
    KThread::setCurrentThread(thread);

    memory->mmap(thread, SIGNAL_STACK_TOP - K_PAGE_SIZE, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    cpu->reg[4].u32 = SIGNAL_STACK_TOP;
    cpu->eip.u32 = TEST_CODE_ADDRESS;
    cpu->flags = 2 | ZF;
    cpu->lazyFlagType = FLAGS_NONE;
    process->sigActions[K_SIGUSR1].handlerAndSigAction = SIGNAL_HANDLER;

    thread->runSignal(K_SIGUSR1, 0, 0);

    // Model a handler whose last arithmetic operation contradicts the saved
    // signal-context flags: SUB 0,1 has CF set and ZF clear.
    cpu->dst.u32 = 0;
    cpu->src.u32 = 1;
    cpu->result.u32 = 0xffffffff;
    cpu->lazyFlagType = FLAGS_SUB32;

    U32 returnAddress = cpu->pop32();
    if (returnAddress != SIG_RETURN_ADDRESS) {
        testFail("signal return callback address for lazy flags");
        return;
    }
    onExitSignal(cpu, nullptr);

    if (cpu->lazyFlagType != FLAGS_NONE) {
        testFail("signal return must discard handler lazy flags");
    }
    if (!cpu->getZF()) {
        testFail("signal return must preserve saved ZF");
    }
    if (cpu->getCF()) {
        testFail("signal return must preserve saved CF");
    }
}

void testSignalHandlerClearsTraceFlagUntilReturn() {
    constexpr U32 SIGNAL_STACK_TOP = 0x70000000;
    constexpr U32 SIGNAL_HANDLER = 0x12345000;

    KProcessPtr process = KProcess::create();
    std::unique_ptr<KMemory> memory(KMemory::create(process.get()));
    process->memory = memory.get();
    KThread* thread = process->createThread();
    CPU* cpu = thread->cpu;
    KThread::setCurrentThread(thread);

    memory->mmap(thread, SIGNAL_STACK_TOP - K_PAGE_SIZE, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    cpu->reg[4].u32 = SIGNAL_STACK_TOP;
    cpu->eip.u32 = TEST_CODE_ADDRESS;
    cpu->flags = 2 | TF;
    cpu->lazyFlagType = FLAGS_NONE;
    process->sigActions[K_SIGUSR1].handlerAndSigAction = SIGNAL_HANDLER;

    thread->runSignal(K_SIGUSR1, 0, 0);
    if (cpu->flags & TF) {
        testFail("signal handler inherited trace flag");
    }
    if (cpu->debugTrapActive) {
        testFail("signal handler retained active trace trap");
    }

    U32 returnAddress = cpu->pop32();
    if (returnAddress != SIG_RETURN_ADDRESS) {
        testFail("signal return callback address for trace flag");
        return;
    }
    onExitSignal(cpu, nullptr);

    if (!(cpu->flags & TF)) {
        testFail("signal return did not restore trace flag");
    }
    if (!cpu->debugTrapActive) {
        testFail("signal return did not restore active trace trap");
    }
}

void testJitSignalPendingReset() {
#ifdef BOXEDWINE_JIT
    TestContext& context = testContext();
    context.cpu->jitSignalPending.store(1, std::memory_order_release);

    context.cpu->reset();

    if (context.cpu->jitSignalPending.load(std::memory_order_acquire) != 0) {
        testFail("CPU reset must clear the JIT signal-pending latch");
    }
#endif
}

void testJitSignalPendingQueuedSignal() {
#ifdef BOXEDWINE_JIT
    TestContext& context = testContext();
    KThread* thread = context.thread;
    const U64 signalBit = 1ULL << (K_SIGUSR1 - 1);
    const U64 oldSigMask = thread->sigMask;
    const U64 oldSigWaitMask = thread->sigWaitMask;
    const U64 oldPendingSignals = thread->pendingSignals;
    const U32 oldJitSignalPending = context.cpu->jitSignalPending.load(std::memory_order_acquire);

    thread->sigMask |= signalBit;
    thread->sigWaitMask &= ~signalBit;
    thread->pendingSignals &= ~signalBit;
    context.cpu->jitSignalPending.store(0, std::memory_order_release);

    thread->signal(K_SIGUSR1, false);

    if (!(thread->pendingSignals & signalBit)) {
        testFail("queueing a masked signal must set the thread pending-signal mask");
    }
    if (context.cpu->jitSignalPending.load(std::memory_order_acquire) != 1) {
        testFail("queueing a masked signal must set the JIT signal-pending latch");
    }

    thread->sigMask = oldSigMask;
    thread->sigWaitMask = oldSigWaitMask;
    thread->pendingSignals = oldPendingSignals;
    context.cpu->jitSignalPending.store(oldJitSignalPending, std::memory_order_release);
#endif
}

#endif
