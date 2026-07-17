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
    ldt->read_exec_only = 0;
    ldt->seg_not_present = 0;

    ldt = process->getLDT(TEST_STACK_SEG >> 3);
    ldt->entry_number = TEST_STACK_SEG >> 3;
    ldt->base_addr = TEST_STACK_ADDRESS - K_PAGE_SIZE * TEST_PAGES_PER_SEG;
    ldt->seg_32bit = 1;
    ldt->read_exec_only = 0;
    ldt->seg_not_present = 0;

    ldt = process->getLDT(TEST_CODE_SEG >> 3);
    ldt->entry_number = TEST_CODE_SEG >> 3;
    ldt->base_addr = TEST_CODE_ADDRESS;
    ldt->seg_32bit = 1;
    ldt->read_exec_only = 0;
    ldt->seg_not_present = 0;

    ldt = process->getLDT(TEST_CODE_SEG_16 >> 3);
    ldt->entry_number = TEST_CODE_SEG_16 >> 3;
    ldt->base_addr = TEST_CODE_ADDRESS;
    ldt->seg_32bit = 0;
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
    context.memory->clearOpCache();
    cpu->mxcsr = 0x1F80;
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
        cpu->run();
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

    ensureParallelContexts(workerCount);

    std::atomic<size_t> nextEntry(0);
    std::mutex printMutex;

    auto runEntry = [&](U32 contextIndex, size_t entryIndex) {
        bindParallelContext(contextIndex);
        TestContext& context = *currentContext;
        runningParallelTest = true;

        context.failed = false;
        context.failures.clear();
        entries[entryIndex].function();
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

#endif
