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
#ifndef BOXEDWINE_MULTI_THREADED
#include "kscheduler.h"
#include "../../emulation/cpu/normal/normalCPU.h"
#endif
#ifdef BOXEDWINE_VULKAN
#include "../../vulkan/vk_host.h"
#include "../../vulkan/vk_host_marshal.h"
#endif
#ifdef BOXEDWINE_JIT
#include "../../emulation/cpu/jit/jitCodeGen.h"
#include "../../emulation/softmmu/kmemory_soft.h"
#endif
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

#ifndef BOXEDWINE_MULTI_THREADED
void testSingleThreadSchedulerTimers() {
    extern S32 contextTime;
    extern S32 contextTimeRemaining;
    static U64 hostTime;
    TestContext& context = testContext();
    U32 dispatches = 0;
    class Timer : public KTimerCallback {
    public:
        Timer(U32& dispatches) : dispatches(dispatches) {}
        bool run() override {
            firedAt = dispatches;
            return true;
        }
        U32& dispatches;
        U32 firedAt = 0;
    } timer(dispatches), dueTimer(dispatches);

    if (getNextTimer() != 0xffffffff || context.thread->scheduledThreadNode.isInList()) {
        testFail("scheduler timer test requires an idle scheduler");
    }
    timer.millies = KSystem::getMilliesSinceStart() + 60000;
    addTimer(&timer);
    U32 remaining = getNextTimer();
    if (!remaining || remaining > 60000) {
        testFail("idle wait must be bounded by the next guest timer");
    }
    dueTimer.millies = 0;
    addTimer(&dueTimer);
    if (getNextTimer() != 0) {
        testFail("an overdue timer must prevent an idle wait");
    }
    removeTimer(&dueTimer);

    class YieldingCPU : public NormalCPU {
    public:
        YieldingCPU(KMemory* memory, Timer& timer, U32& dispatches)
            : NormalCPU(memory), timer(timer), dispatches(dispatches) { op.inst = Nop; }
        DecodedOp* getOp(U32, U32) override { return &op; }
        void run() override {
            ++dispatches;
            ++blockInstructionCount;
            hostTime += 1000;
            yield = true;
            if (dispatches == 1) {
                // Make a worker's timer due after runSlice's initial timer pass.
                timer.millies = 0;
            }
        }
        DecodedOp op;
        Timer& timer;
        U32& dispatches;
    } cpu(context.memory, timer, dispatches);
    struct RestoreScheduler {
        KThread* thread;
        CPU* cpu;
        S32 budget = contextTime;
        S32 remaining = contextTimeRemaining;
        ~RestoreScheduler() {
            unscheduleThread(thread);
            thread->cpu = cpu;
            contextTime = budget;
            contextTimeRemaining = remaining;
            setSchedulerTestClock(nullptr);
        }
    } restore{context.thread, context.thread->cpu};
    cpu.thread = context.thread;
    context.thread->cpu = &cpu;
    hostTime = 0;
    setSchedulerTestClock([]() -> U64 { return hostTime; });
    scheduleThread(context.thread);
    runSlice();
    if (timer.firedAt != 1 || dispatches != 9) {
        testFail("a runnable yielding thread must not delay timers for an entire slice");
    }
    if (getNextTimer() != 0xffffffff) {
        testFail("completed timers must not keep the scheduler awake");
    }
}

void testSingleThreadSchedulerTimeSlice() {
    extern S32 contextTime;
    extern S32 contextTimeRemaining;
    static U64 hostTime;
    static U32 clockReads;
    TestContext& context = testContext();
    struct RestoreScheduler {
        KThread* thread;
        CPU* cpu;
        S32 budget = contextTime;
        S32 remaining = contextTimeRemaining;
        ~RestoreScheduler() {
            thread->cpu = cpu;
            contextTime = budget;
            contextTimeRemaining = remaining;
            setSchedulerTestClock(nullptr);
        }
    } restore{context.thread, context.thread->cpu};

    // Give dispatches a deterministic host cost but very different guest
    // instruction counts. A fixed instruction quota would favor the slow CPU.
    class TimedCPU : public NormalCPU {
    public:
        TimedCPU(KMemory* memory) : NormalCPU(memory) { op.inst = Nop; }
        DecodedOp* getOp(U32, U32) override { return &op; }
        void run() override {
            ++dispatches;
            blockInstructionCount += instructionsPerDispatch;
            hostTime += 250;
            yield = dispatches == yieldAfter || dispatches == 4096;
#ifndef __EMSCRIPTEN__
            if (raiseFault) {
                throw 1;
            }
#endif
        }
        DecodedOp op;
        U32 instructionsPerDispatch = 1;
        U32 dispatches = 0;
        U32 yieldAfter = 0;
        bool raiseFault = false;
    } cpu(context.memory);
    cpu.thread = context.thread;
    context.thread->cpu = &cpu;
    setSchedulerTestClock([]() -> U64 { ++clockReads; return hostTime; });
    contextTime = 10000;

    for (U32 instructions : {1u, 32u, 513u, 1024u, 100000u}) {
        hostTime = 0;
        clockReads = 0;
        cpu.dispatches = 0;
        cpu.instructionsPerDispatch = instructions;
        cpu.instructionCount = 100;
        contextTimeRemaining = 1; // Each turn must start with a fresh budget.
#ifdef __EMSCRIPTEN__
        // Browser clocks are sampled only after at least 1024 instructions.
        // Even a slow dispatch must finish its batch before checking time.
        const U32 dispatchesPerClockCheck = (1024 + instructions - 1) / instructions;
#else
        const U32 dispatchesPerClockCheck = 1;
#endif
        const U32 expectedDispatches = ((4 + dispatchesPerClockCheck - 1) /
            dispatchesPerClockCheck) * dispatchesPerClockCheck;
        const U32 expectedClockReads = 1 + expectedDispatches / dispatchesPerClockCheck;
        runThreadSlice(context.thread);
        if (cpu.dispatches != expectedDispatches || hostTime != 250 * expectedDispatches ||
            cpu.instructionCount != 100 + expectedDispatches * instructions ||
            clockReads != expectedClockReads) {
            testFail("scheduler must check the timed deadline at each clock batch boundary");
        }
        runThreadSlice(context.thread);
        if (cpu.dispatches != 2 * expectedDispatches || hostTime != 500 * expectedDispatches ||
            cpu.instructionCount != 100 + 2 * expectedDispatches * instructions ||
            clockReads != 2 * expectedClockReads) {
            testFail("scheduler must resume with a fresh timed turn and preserve instruction accounting");
        }
    }
    hostTime = 0;
    clockReads = 0;
    cpu.dispatches = 0;
    cpu.instructionsPerDispatch = 1;
    cpu.yieldAfter = 1;
    runThreadSlice(context.thread);
    if (cpu.dispatches != 1 || hostTime != 250 || !cpu.yield || clockReads != 1) {
        testFail("a blocking or yielding thread must end its turn immediately");
    }
    cpu.yieldAfter = 0;
    cpu.op.inst = TestEnd;
    cpu.dispatches = 0;
    runThreadSlice(context.thread);
    if (cpu.dispatches != 1) {
        testFail("scheduler must preserve the test-end sentinel");
    }
#ifndef __EMSCRIPTEN__
    cpu.op.inst = Nop;
    cpu.raiseFault = true;
    cpu.dispatches = 0;
    runThreadSlice(context.thread);
    if (cpu.dispatches != 1 || cpu.nextOp != nullptr) {
        testFail("native scheduler must return and discard the decoded op after a guest fault");
    }
#endif
}
#endif

void testCloneVmCompiledEntryHelpers() {
#if defined(BOXEDWINE_JIT_ARMV8) || defined(BOXEDWINE_JIT_X64)
    auto& context = testContext();
    testNewInstruction(0);
#ifdef BOXEDWINE_JIT_ARMV8
    ensureArmV8HardwareTSOForThread();
#endif
    testPushCode8(0xb8); testPushCode32(0x12345678);
    testPushCode8(0xcd); testPushCode8(0x97);
    const U32 faultCode = TEST_CODE_ADDRESS + 0x100;
    context.codeIp = faultCode;
    testPushCode8(0xa1); testPushCode32(0x24000000 - TEST_HEAP_ADDRESS);
    testPushCode8(0xcd); testPushCode8(0x97);
    auto* head = context.cpu->getOp(TEST_CODE_ADDRESS, 0);
    auto* fault = context.cpu->getOp(faultCode, 0);
    startNewJIT(context.cpu, TEST_CODE_ADDRESS, head);
    startNewJIT(context.cpu, faultCode, fault);
    if (!head->pfnJitCode || !fault->pfnJitCode) {
        testFail("shared JIT helper test did not compile parent blocks");
        return;
    }
    KProcessPtr child = KProcess::create();
    child->memory = KMemory::create(child.get());
    // Match CLONE_VM's shared memory and CPU state without requiring the
    // filesystem fixture used by process-publication tests.
    child->memory->clone(context.memory, true);
    for (U32 segment = 0; segment < 6; ++segment) {
        child->hasSetSeg[segment] = context.process->hasSetSeg[segment];
        const U32 index = context.cpu->seg[segment].value >> 3;
        *child->getLDT(index) = *context.process->getLDT(index);
    }
    child->hasSetStackMask = context.process->hasSetStackMask;
    KThread* thread = child->createThread();
    thread->clone(context.thread);
    CPU* childCpu = thread->cpu;
    auto run = [childCpu](U32 address) {
        childCpu->eip.u32 = address - childCpu->seg[CS].address;
        childCpu->nextOp = childCpu->getNextOp();
        for (U32 dispatch = 0; dispatch < 20; ++dispatch) {
            if (childCpu->nextOp && childCpu->nextOp->inst == TestEnd) return;
            try { childCpu->run(); }
            catch (int) { childCpu->nextOp = nullptr; }
        }
        testFail("shared JIT helper test failed to reach its end");
    };
    {
        ChangeThread current(thread);
        run(TEST_CODE_ADDRESS);
        if (childCpu->reg[0].u32 != 0x12345678 || !child->startJITOp ||
            !child->syncToHost || !child->syncFromHost || !child->blockExit ||
            !child->calculateCF[0] || childCpu->calculateCF[0] != child->calculateCF[0]
#ifdef BOXEDWINE_POSIX
            || !child->signalHandler
#endif
            ) {
            // Check before provoking a host fault: the old behavior otherwise
            // loops at a null signal-handler address instead of failing cleanly.
            testFail("shared compiled entry did not initialize child process helpers");
        } else {
            const U32 length = ramPageLinearMemoryPageCount() * K_PAGE_SIZE;
            child->memory->mmap(thread, 0x24000000, length, K_PROT_READ | K_PROT_WRITE,
                K_MAP_FIXED | K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0);
            childCpu->reg[0].u32 = 0xffffffff;
            run(faultCode);
            if (childCpu->reg[0].u32 || childCpu->eip.u32 != faultCode + 5 - TEST_CODE_ADDRESS)
                testFail("shared compiled entry did not recover its first memory fault");
            child->memory->unmap(0x24000000, length);
        }
    }
    child->memory->execvReset(true);
    KSystem::eraseProcess(child->id);
    child.reset();
    if (context.memory->getDecodedOp(TEST_CODE_ADDRESS) != head || !head->pfnJitCode)
        testFail("child helper cleanup damaged parent compiled code");
#endif
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
    cpu->wasmJitBailout = WASM_JIT_BAILOUT_NONE;
    context.memory->removeCodeBlock(TEST_CODE_ADDRESS, first, false);

    if (cpu->wasmJitBailout != WASM_JIT_BAILOUT_SMC) {
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
    cpu->reg[3].u32 = 0; // ebx: stale fallthrough marker
    U32 iretStack = cpu->reg[4].u32;
    context.memory->writed(cpu->seg[SS].address + iretStack, 6); // resume at TestEnd
    context.memory->writed(cpu->seg[SS].address + iretStack + 4, cpu->seg[CS].value);
    context.memory->writed(cpu->seg[SS].address + iretStack + 8, cpu->flags | 2);

    testPushCode8(0x31); // xor eax,eax: set ZF so jnz is not taken
    testPushCode8(0xc0);
    testPushCode8(0x75); // jnz to inc ebx, keeping it after iret in this block
    testPushCode8(0x01);
    testPushCode8(0xcf); // iret: interpreter-backed control transfer
    testPushCode8(0x43); // must not run after iret returns from its WASM helper
    testPushCode8(0xcd);
    testPushCode8(0x97);
    testRunCPU();

    DecodedOp* iretBranch = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
    DecodedOp* iretOp = iretBranch && iretBranch->next ? iretBranch->next->next : nullptr;
    DecodedOp* staleFallthrough = iretOp ? iretOp->next : nullptr;
    if (!iretBranch || !iretOp || !staleFallthrough ||
        iretBranch->pfn != cpu->thread->process->startJITOp ||
        iretOp->blockStart != iretBranch || staleFallthrough->blockStart != iretBranch) {
        testFail("wasm jit interpreter-backed branch regression setup");
    }
    if (cpu->reg[3].u32 != 0) {
        testFail("wasm jit interpreter-backed branch must not execute stale fallthrough");
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

void testJitIndirectTargetProfile() {
#ifdef BOXEDWINE_JIT_X64
    DecodedOp op;
    for (U32 i = 0; i < 32; ++i) op.observeIndirectTarget(0);
    if (op.indirectTargetHits != 12 || op.indirectTargetEip != 0) testFail("zero monomorphic indirect target");
    op.observeIndirectTarget(17);
    for (U32 i = 0; i < 32; ++i) op.observeIndirectTarget(0);
    if (op.indirectTargetHits != 0xff || op.indirectTargetEip != 0) testFail("sticky polymorphic indirect target");
    op.reset();
    // Multiple interpreter workers may reach the same decoded call site.
    std::vector<std::thread> workers;
    for (U32 i = 0; i < 8; ++i) workers.emplace_back([&op]() {
        for (U32 j = 0; j < 1000; ++j) op.observeIndirectTarget(0x12345678);
    });
    for (auto& worker : workers) worker.join();
    if (op.indirectTargetHits != 12 || op.indirectTargetEip != 0x12345678) testFail("concurrent monomorphic indirect target");
    workers.clear();
    for (U32 i = 0; i < 8; ++i) workers.emplace_back([&op,i]() {
        for (U32 j = 0; j < 1000; ++j) op.observeIndirectTarget(i & 1 ? 0x87654321 : 0x12345678);
    });
    for (auto& worker : workers) worker.join();
    if (op.indirectTargetHits != 0xff || op.indirectTargetEip != 0x12345678) testFail("concurrent sticky polymorphic indirect target");
#endif
}

void testJitIndirectTargetFlags() {
#ifdef BOXEDWINE_JIT_X64
    auto& context = testContext();
    CPU* cpu = context.cpu;
    for (U32 csBase : {TEST_CODE_ADDRESS,0u}) for (bool memoryCall : {false,true}) {
        testNewInstruction(0);
        cpu->seg[CS].address = csBase;
        U32 returnOffset = memoryCall ? 8 : 4;
        auto emitCaller = [&](U32 offset) {
            context.codeIp = TEST_CODE_ADDRESS + offset;
            testPushCode8(0x39); testPushCode8(0xfe); // cmp esi,edi
            testPushCode8(0xff);
            if (memoryCall) { testPushCode8(0x15); testPushCode32(0); } // call [ds:0]
            else testPushCode8(0xd1); // call ecx
            testPushCode8(0x9c); testPushCode8(0x5b); // pushfd; pop ebx
            testPushCode8(0xcd); testPushCode8(0x97);
        };
        emitCaller(0);
        emitCaller(0x300); // A different return address exercises the return guard miss.
        for (U32 offset : {0x100u,0x200u}) {
            context.codeIp = TEST_CODE_ADDRESS + offset;
            testPushCode8(0xc3);
        }
        DecodedOp* producer = cpu->getOp(TEST_CODE_ADDRESS, 0);
        DecodedOp* call = producer->next;
        DecodedOp* callee = cpu->getOp(TEST_CODE_ADDRESS + 0x100, 0);
        for (U32 i = 0; i < 12; ++i) {
            call->observeIndirectTarget(TEST_CODE_ADDRESS + 0x100 - csBase);
            callee->observeIndirectTarget(TEST_CODE_ADDRESS + returnOffset - csBase);
        }
        startNewJIT(cpu, TEST_CODE_ADDRESS + 0x100, callee);
        startNewJIT(cpu, TEST_CODE_ADDRESS, producer);
        if (!producer->pfnJitCode || !callee->pfnJitCode) {
            testFail("indirect target flags setup did not compile");
            return;
        }
        constexpr U32 expected[] = {CF | PF | AF | SF, PF | ZF, 0};
        for (U32 scenario = 0; scenario < 3; ++scenario) {
            U32 caller = TEST_CODE_ADDRESS + (scenario == 2 ? 0x300 : 0) - csBase;
            U32 target = TEST_CODE_ADDRESS + (scenario == 1 ? 0x200 : 0x100) - csBase;
            for (U32 left = 0; left < 3; ++left) {
                cpu->reg[1].u32 = target;
                context.memory->writed(TEST_HEAP_ADDRESS, target);
                cpu->reg[6].u32 = left;
                cpu->reg[7].u32 = 1;
                cpu->reg[4].u32 = 4096;
                cpu->eip.u32 = caller;
                cpu->nextOp = cpu->getNextOp();
                do { cpu->run(); } while (!cpu->nextOp || cpu->nextOp->inst != TestEnd);
                if ((cpu->reg[3].u32 & FMASK_TEST) != expected[left] ||
                    cpu->reg[4].u32 != 4096 || cpu->eip.u32 != caller + returnOffset + 2) {
                    testFail("indirect target flags call %u scenario %u input %u", memoryCall, scenario, left);
                }
            }
        }
    }
    // A compiled RET can be shared at the same linear address with another
    // CS base. Its learned target offset must use the current segment base.
    testNewInstruction(0);
    context.codeIp = TEST_CODE_ADDRESS + 0x300;
    testPushCode8(0xc3);
    for (U32 offset : {0x100u,0x1100u}) {
        context.codeIp = TEST_CODE_ADDRESS + offset;
        testPushCode8(0xb8); testPushCode32(offset); // mov eax, distinct target
        testPushCode8(0xcd); testPushCode8(0x97);
    }
    DecodedOp* ret = cpu->getOp(TEST_CODE_ADDRESS + 0x300, 0);
    for (U32 i = 0; i < 12; ++i) ret->observeIndirectTarget(0x100);
    startNewJIT(cpu, TEST_CODE_ADDRESS + 0x300, ret);
    for (U32 shift : {0u,0x1000u}) {
        cpu->seg[CS].address = TEST_CODE_ADDRESS + shift;
        cpu->reg[4].u32 = 4096;
        context.memory->writed(cpu->seg[SS].address + 4096, 0x100);
        cpu->eip.u32 = 0x300 - shift;
        cpu->nextOp = ret;
        do { cpu->run(); } while (!cpu->nextOp || cpu->nextOp->inst != TestEnd);
        if (cpu->reg[0].u32 != 0x100 + shift || cpu->reg[4].u32 != 4100)
            testFail("indirect return guard CS base shift %u", shift);
    }
    cpu->seg[CS].address = TEST_CODE_ADDRESS;
#endif
}

void testWasmJitIndirectCacheState() {
#ifdef BOXEDWINE_WASM_JIT
    auto& context = testContext();
    CPU* cpu = context.cpu;
    // Cover an absent op, absent page, absent cache group, and an initial hit.
    // The producer must publish dirty GP and x87 state on every exit arm.
    for (U32 scenario = 0; scenario < 5; ++scenario) {
        testNewInstruction(0);
        cpu->fpu.reset();
        U32 offset = scenario == 1 ? 0x4000 : scenario == 2 ? 0x400000 : 0x100;
        U32 targetAddress = TEST_CODE_ADDRESS + offset;
        if (scenario == 2) {
            context.memory->mmap(context.thread, targetAddress, K_PAGE_SIZE,
                K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
        }
        testPushCode8(0xd9); testPushCode8(0xe8); // fld1
        testPushCode8(0xb8); testPushCode32(0x12345678); // mov eax, immediate
        testPushCode8(0xff); testPushCode8(0xe2); // jmp edx
        context.codeIp = targetAddress;
        testPushCode8(0xdd); testPushCode8(0x1d); testPushCode32(0); // fstp qword [ds:0]
        testPushCode8(0x89); testPushCode8(0xc3); // mov ebx,eax
        testPushCode8(0xcd); testPushCode8(0x97);
        DecodedOp* producer = cpu->getOp(TEST_CODE_ADDRESS, 0);
        startNewJIT(cpu, TEST_CODE_ADDRESS, producer);
        if (!producer->pfnJitCode) {
            testFail("indirect cache producer did not compile");
            return;
        }
        if (scenario >= 3) {
            if (scenario == 4) cpu->setIsBig(0);
            DecodedOp* target = cpu->getOp(targetAddress, 0);
            if (scenario == 4) cpu->setIsBig(1);
            else startNewJIT(cpu, targetAddress, target);
        } else if (context.memory->getDecodedOp(targetAddress)) {
            testFail("indirect cache miss unexpectedly decoded");
        }
        void* producerCode = producer->pfnJitCode;
        for (U32 pass = 0; pass < 3; ++pass) {
            if (pass == 2) {
                DecodedOp* target = context.memory->getDecodedOp(targetAddress);
                context.memory->removeCodeBlock(targetAddress, target, false);
            }
            cpu->fpu.reset();
            cpu->eip.u32 = 0;
            cpu->reg[2].u32 = offset;
            cpu->reg[3].u32 = 0;
            cpu->nextOp = producer;
            do { cpu->run(); } while (!cpu->nextOp || cpu->nextOp->inst != TestEnd);
            if (cpu->reg[3].u32 != 0x12345678 || cpu->fpu.top != 0 ||
                context.memory->readq(TEST_HEAP_ADDRESS) != 0x3ff0000000000000ULL ||
                cpu->eip.u32 != offset + 8 || producer->pfnJitCode != producerCode) {
                testFail("indirect cache state scenario %u pass %u", scenario, pass);
            }
        }
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

void testJitBranchTargetBoundaries() {
#ifdef BOXEDWINE_JIT
    for (bool taken : {false, true}) {
        testNewInstruction(CF);
        auto& context = testContext();
        CPU* cpu = context.cpu;
        cpu->reg[0].u32 = taken ? 0 : 1;
        cpu->reg[3].u32 = 0;
        testPushCode8(0x68); // push return address
        U32 returnAddress = context.codeIp;
        testPushCode32(0);
        testPushCode8(0x83); testPushCode8(0xf8); testPushCode8(0); // cmp eax,0
        testPushCode8(0x74); testPushCode8(1); // jz final ret
        testPushCode8(0x43); // inc ebx on fall-through
        U32 targetAddress = context.codeIp;
        testPushCode8(0xc3); // terminal RET is included in the compiled block
        context.memory->writed(returnAddress, context.codeIp - TEST_CODE_ADDRESS);
        testPushCode8(0xcd); testPushCode8(0x97);

        DecodedOp* entry = cpu->getNextOp();
        std::unique_ptr<JitCodeGen> jit(startNewJIT(cpu));
        jit->doJIT(TEST_CODE_ADDRESS, entry);
        DecodedOp* target = context.memory->getDecodedOp(targetAddress);
        if (!entry->pfnJitCode || jit->lastOpEip != targetAddress || !target ||
                !(target->flags2 & OP_FLAG2_JUMP_TARGET)) {
            testFail("JIT must mark an exact target at the final block instruction");
            return;
        }
        testRunCPU();
        if (cpu->reg[3].u32 != (taken ? 0u : 1u) || cpu->reg[4].u32 != 4096) {
            testFail("JIT final-instruction branch lost its path or stack state");
        }
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

void testNativeJitCooperativeScheduling() {
#if defined(BOXEDWINE_JIT) && !defined(BOXEDWINE_WASM_JIT) && !defined(BOXEDWINE_MULTI_THREADED)
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    const U32 iterations = 10000;
    // Finite loops make a missing scheduler check fail without hanging the suite.
    for (U32 kind = 0; kind < 7; ++kind) {
        testNewInstruction(CF);
        cpu->yield = false;
        cpu->reg[1].u32 = iterations;
        std::vector<U32> entries{0};
        if (kind < 3 || kind == 6) {
            testPushCode8(0x46); // inc esi
            if (kind == 2) {
                testPushCode8(0xe2); testPushCode8(0xfd); // loop 0
            } else if (kind == 6) {
                testPushCode8(0x49); // dec ecx
                testPushCode8(0x74); testPushCode8(0x02); // jz done
                testPushCode8(0xeb); testPushCode8(0xfa); // jmp 0
            } else {
                testPushCode8(0x49); // dec ecx
                testPushCode8(0x75); testPushCode8(0xfc); // jnz 0
            }
        } else {
            // Force a separate block with a direct or indirect jump, or a call.
            if (kind == 3) {
                testPushCode8(0xe9); testPushCode32(0x100 - 5);
            } else if (kind == 4) {
                cpu->reg[0].u32 = 0x100;
                testPushCode8(0xff); testPushCode8(0xe0); // jmp eax
            } else {
                testPushCode8(0xe8); testPushCode32(0x100 - 5);
            }
            const U32 continuation = context.codeIp - TEST_CODE_ADDRESS;
            if (kind == 5) {
                testPushCode8(0x49); // dec ecx
                testPushCode8(0x75); testPushCode8(0xf8); // jnz 0
            }
            testPushCode8(0x9c); testPushCode8(0x5b); // pushfd; pop ebx
            testPushCode8(0xcd); testPushCode8(0x97);
            context.codeIp = TEST_CODE_ADDRESS + 0x100;
            testPushCode8(0x46); // inc esi
            if (kind == 5) {
                testPushCode8(0xc3); // ret
            } else {
                testPushCode8(0x49); // dec ecx
                testPushCode8(0x0f); testPushCode8(0x85); // jnz 0
                testPushCode32(0 - 0x108);
                testPushCode8(0xe9); // jmp continuation
                testPushCode32(continuation - 0x10d);
            }
            entries.push_back(0x100);
            entries.push_back(continuation);
        }
        if (kind == 0) {
            // Overwrite flags after the loop, allowing the fused dec/jnz path.
            testPushCode8(0x31); testPushCode8(0xd2); // xor edx,edx
        }
        testPushCode8(0x9c); testPushCode8(0x5b); // pushfd; pop ebx
        testPushCode8(0xcd); testPushCode8(0x97);
        for (U32 offset : entries) {
            DecodedOp* op = cpu->getOp(TEST_CODE_ADDRESS + offset, 0);
            startNewJIT(cpu, TEST_CODE_ADDRESS + offset, op);
            if (!op->pfnJitCode) {
                testFail("cooperative JIT case %u did not compile", kind);
                return;
            }
        }
        cpu->nextOp = cpu->getNextOp();
        cpu->run();
        if (!cpu->reg[6].u32 || cpu->reg[6].u32 >= iterations) {
            testFail("cooperative JIT case %u must return before finishing its loop", kind);
        }
        U32 dispatches = 1;
        while ((!cpu->nextOp || cpu->nextOp->inst != TestEnd) && dispatches++ < iterations) {
            cpu->run();
        }
        if (cpu->reg[6].u32 != iterations || cpu->reg[1].u32 || cpu->reg[4].u32 != 4096 ||
            !cpu->nextOp || cpu->nextOp->inst != TestEnd) {
            testFail("cooperative JIT case %u must resume with intact registers, stack and EIP", kind);
        }
        if (kind != 0 && (!(cpu->reg[3].u32 & CF) || (kind != 2 && !(cpu->reg[3].u32 & ZF)))) {
            testFail("cooperative JIT case %u must preserve guest flags", kind);
        }
    }
    // The fallback helper can also chain directly to compiled code. It must
    // honor the same budget after completing the emulated instruction.
    extern DYN_PTR_SIZE jitRunSingleOp(CPU* cpu);
    testNewInstruction(CF);
    cpu->yield = false;
    testPushCode8(0x46); // inc esi (run by the fallback helper)
    testPushCode8(0x47); // inc edi (compiled continuation)
    testPushCode8(0xcd); testPushCode8(0x97);
    DecodedOp* continuation = cpu->getOp(TEST_CODE_ADDRESS + 1, 0);
    startNewJIT(cpu, TEST_CODE_ADDRESS + 1, continuation);
    cpu->jitBranchBudget = 1;
    if (jitRunSingleOp(cpu) || cpu->reg[6].u32 != 1 || cpu->eip.u32 != 1 ||
        cpu->nextOp != continuation) {
        testFail("native JIT fallback must return at an exhausted budget with its instruction complete");
    }
    cpu->eip.u32 = 0;
    cpu->jitBranchBudget = 2;
    if (!continuation->pfnJitCode || jitRunSingleOp(cpu) != (DYN_PTR_SIZE)continuation->pfnJitCode) {
        testFail("native JIT fallback must retain chaining while budget remains");
    }
#endif
}

#if defined(BOXEDWINE_JIT) && defined(BOXEDWINE_OPENGL) && defined(BOXEDWINE_MULTI_THREADED) && !defined(BOXEDWINE_WASM_JIT)
extern DYN_PTR_SIZE jitRunOpenGL(CPU* cpu);
extern thread_local U32 jitOpenGLFastCallCount;
namespace {
U32 glTestCalls;
U32 glTestMutation;
U32 glTestBoundary;
bool glTestSawCompiled;

void glTestCallback(CPU* cpu) {
    ++glTestCalls;
    if (cpu->peek32(1) != 0x11223344 || cpu->thread->marshalIndex != 0) {
        testFail("OpenGL callback stack arguments or marshal reset");
    }
    cpu->reg[0].u32 = 0xaabbccdd;
    cpu->reg[2].u32 = 0x76543210;
    cpu->tmpReg = 0xffffffff; // callbacks must not alter the saved instruction length
    if (glTestMutation && glTestCalls == JIT_RUN_COUNT + 5) {
        DecodedOp* op = cpu->memory->getDecodedOp(cpu->getEipAddress());
        glTestSawCompiled = op && op->pfnJitCode;
        cpu->memory->writeb(glTestMutation, 0x46); // replace inc edi with inc esi
    }
    switch (glTestBoundary) {
    case 1: cpu->yield = true; break;
    case 2: cpu->thread->terminating = true; break;
    case 3: cpu->jitSignalPending.store(1, std::memory_order_release); break;
    case 4: cpu->thread->pendingSignals = 1; break;
    case 5: cpu->eip.u32 = 7; throw 99; // simulate redirection to a fault handler
    }
}

struct ScopedGlTestCallbacks {
    Int99Callback* saved = int99Callback;
    U32 size = int99CallbackSize;
    Int99Callback callbacks[4] = { nullptr, nullptr, nullptr, glTestCallback };
    ScopedGlTestCallbacks() { int99Callback = callbacks; int99CallbackSize = 4; }
    ~ScopedGlTestCallbacks() { int99Callback = saved; int99CallbackSize = size; }
};

void prepareGlTest() {
    testNewInstruction(0);
    CPU* cpu = testContext().cpu;
    cpu->yield = false;
    cpu->thread->terminating = false;
    cpu->thread->pendingSignals = 0;
    cpu->jitSignalPending.store(0, std::memory_order_release);
    cpu->memory->writed(cpu->seg[SS].address + cpu->reg[4].u32 + 4, 0x11223344);
    glTestCalls = glTestMutation = glTestBoundary = 0;
    glTestSawCompiled = false;
    jitOpenGLFastCallCount = 0;
}

void emitGlTestCall() {
    testPushCode8(0xcd);
    testPushCode8(0x99);
    testPushCode32(3);
}
}
#endif

void testJitDirectTargetInvalidation() {
#if defined(BOXEDWINE_JIT) && !defined(BOXEDWINE_WASM_JIT)
    testNewInstruction(0);
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    const U32 targetOffset = K_PAGE_SIZE * 1024;
    const U32 targetAddress = TEST_CODE_ADDRESS + targetOffset;
    context.memory->mmap(context.thread, targetAddress, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    testPushCode8(0xe8); // call a separately compiled page in another cache group
    testPushCode32(targetOffset - 5);
    testPushCode8(0xcd); testPushCode8(0x97);
    context.codeIp = targetAddress;
    testPushCode8(0x46); // inc esi
    testPushCode8(0xc3); // ret

    DecodedOp* target = cpu->getOp(targetAddress, 0);
    startNewJIT(cpu, targetAddress, target);
    DecodedOp* caller = cpu->getOp(TEST_CODE_ADDRESS, 0);
    startNewJIT(cpu, TEST_CODE_ADDRESS, caller);
    if (!caller->pfnJitCode || !target->pfnJitCode || !caller->data.nextJump || *caller->data.nextJump != target) {
        testFail("Direct JIT target invalidation setup");
        return;
    }
    void* callerCode = caller->pfnJitCode;
    DecodedOp** targetSlot = caller->data.nextJump;
    auto runCaller = [&]() {
        cpu->eip.u32 = 0;
        cpu->reg[4].u32 = 4096;
        cpu->nextOp = caller;
        do {
            cpu->run();
        } while (!cpu->nextOp || cpu->nextOp->inst != TestEnd);
    };
    runCaller();
    if (cpu->reg[6].u32 != 1 || cpu->reg[7].u32 != 0) testFail("Direct JIT initial target");
#ifdef BOXEDWINE_MULTI_THREADED
    cpu->eip.u32 = 0;
    cpu->reg[4].u32 = 4096;
    cpu->nextOp = caller;
    cpu->jitSignalPending.store(1, std::memory_order_release);
    caller->pfn(cpu, caller);
    cpu->jitSignalPending.store(0, std::memory_order_release);
    if (cpu->eip.u32 != targetOffset || cpu->reg[4].u32 != 4092 || cpu->reg[6].u32 != 1 || cpu->nextOp) {
        testFail("Direct JIT pending signal must exit after the call push and before the target");
    }
#endif

    context.memory->writeb(targetAddress, 0x47); // replace with inc edi
    if (*targetSlot || caller->pfnJitCode != callerCode) {
        testFail("Direct JIT target invalidation must clear the slot and retain the caller");
        return;
    }
    runCaller(); // slot miss must reach the replacement through the run loop
    if (cpu->reg[6].u32 != 1 || cpu->reg[7].u32 != 1) testFail("Direct JIT target miss after invalidation");
    target = cpu->getOp(targetAddress, 0);
    startNewJIT(cpu, targetAddress, target);
    if (*targetSlot != target || !target->pfnJitCode || caller->pfnJitCode != callerCode) {
        testFail("Direct JIT replacement publication");
        return;
    }
    runCaller();
    if (cpu->reg[6].u32 != 1 || cpu->reg[7].u32 != 2) testFail("Direct JIT replaced compiled target");
    context.memory->unmap(targetAddress, K_PAGE_SIZE);
#endif
}

void testArmJitEntryInvalidatedBeforeDispatch() {
#ifdef BOXEDWINE_JIT_ARMV8
    testNewInstruction(0);
    CPU* cpu = testContext().cpu;
    testPushCode8(0x43); // inc ebx: the interpreter-chain head must not run again
    testPushCode8(0xb8); testPushCode32(0x12345678); // mov eax,0x12345678
    testPushCode8(0xcd); testPushCode8(0x97); // TestEnd
    DecodedOp* head = cpu->getOp(TEST_CODE_ADDRESS, 0);
    DecodedOp* target = cpu->getOp(TEST_CODE_ADDRESS + 1, 0);
    startNewJIT(cpu, TEST_CODE_ADDRESS + 1, target);
    if (!target->pfnJitCode) {
        testFail("ARM JIT entry invalidation setup did not compile");
        return;
    }
    OpCallback enterJit = cpu->thread->process->startJITOp;
    // Model invalidation after an interpreter tail call selected startJITOp,
    // but before that wrapper loads the target's compiled entry.
    cpu->memory->removeCodeBlock(TEST_CODE_ADDRESS + 1, target, false);
    cpu->eip.u32 = 1;
    cpu->reg[0].u32 = 0;
    cpu->reg[3].u32 = 0;
    cpu->nextOp = head;
    enterJit(cpu, target);
    if (cpu->eip.u32 != 1 || cpu->nextOp || cpu->reg[0].u32 || cpu->reg[3].u32) {
        testFail("Invalidated ARM JIT entry must discard the old chain head and preserve guest state");
        return;
    }
    do {
        cpu->run();
    } while (!cpu->nextOp || cpu->nextOp->inst != TestEnd);
    if (cpu->reg[0].u32 != 0x12345678 || cpu->reg[3].u32 != 0 || cpu->eip.u32 != 6) {
        testFail("Invalidated ARM JIT entry did not resume at the current instruction exactly once");
    }
#endif
}

void testJitEntryCacheInvalidation() {
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
    testNewInstruction(0);
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    auto groups = getMemData(cpu->memory)->opCache.getJitPageGroups();
#ifdef BOXEDWINE_JIT_ARMV8
    {
        extern void clearJitBlock(const std::vector<void*>&);
        U8* code = static_cast<U8*>(context.memory->allocCodeMemory(128));
        Platform::writeCodeToMemory(code, 128, [code]() { memset(code, 0xa5, 128); });
        // The native order deliberately differs from the supplied entry order.
        // The old range calculation wrapped to nearly 4 GiB and repeatedly
        // faulted inside macOS instruction-cache invalidation.
        clearJitBlock({code + 96, code + 32, code + 64, code});
        clearJitBlock({});
        for (U32 i = 0; i < 128; ++i) {
            if (code[i] != (i % 32 < 4 ? 0 : 0xa5))
                testFail("ARM unordered entry invalidation changed a neighboring byte");
        }
        getMemData(cpu->memory)->codeMemory.free(code);
    }
#endif
#ifdef BOXEDWINE_JIT_ARMV8
    auto flat = getMemData(cpu->memory)->opCache.getFlatJitEntries();
    if (flat != cpu->jitFlatEntries || (std::getenv("BOXEDWINE_ARM_FLAT_CACHE_SLOW") && flat))
        testFail("Flat JIT entry cache CPU binding or opt-out");
#endif
    if (std::getenv("BOXEDWINE_JIT_ENTRY_CACHE_SLOW")) {
        if (groups || cpu->jitEntryPageGroups) testFail("JIT entry cache opt-out was ignored");
#ifdef BOXEDWINE_JIT_ARMV8
        if (flat) testFail("Flat cache ignored compiled-entry cache opt-out");
#endif
        return;
    }
    if (!groups || groups != cpu->jitEntryPageGroups) {
        testFail("Compiled-entry JIT cache was not selected");
        return;
    }
    for (U32 pass = 0; pass < 2; ++pass) {
        for (U32 i = 0; i < 3; ++i) {
            U32 address = TEST_CODE_ADDRESS + i * K_PAGE_SIZE;
            context.codeIp = address;
            testPushCode8(0x40); // inc eax
            testPushCode8(0xcd); testPushCode8(0x97);
            DecodedOp* op = cpu->getOp(address, 0);
            U32 pageIndex = address >> K_PAGE_SHIFT;
            if (groups[pageIndex >> 10][pageIndex & 0x3ff] || op->getJitEntrySlot()) {
                testFail("Decoding alone allocated a compiled-entry page");
                return;
            }
#ifdef BOXEDWINE_JIT_ARMV8
            auto& cache = getMemData(cpu->memory)->opCache;
            void** direct = cache.getDirectJitEntryLocation(address);
            // Force several rehashes after taking the address. Never publish
            // fake code pointers: every uncompiled target must stay null.
            for (U32 n = 0; flat && !std::getenv("BOXEDWINE_ARM_COMPACT_DIRECT_SLOW") && n < 4096; ++n) {
                void** unused = cache.getDirectJitEntryLocation(0xb0000000u + n * 4096u);
                if (!unused || *unused) testFail("Compact direct cache uncompiled target");
            }
            if (!direct || *direct) testFail("Compact direct cache premature publication");
#endif
            startNewJIT(cpu, address, op);
#ifdef BOXEDWINE_JIT_ARMV8
            if (*direct != op->pfnJitCode || !*direct) testFail("Compact direct cache publication after rehash");
#endif
            auto page = groups[pageIndex >> 10][pageIndex & 0x3ff];
            U32 entryIndex = address & K_PAGE_MASK;
#ifdef BOXEDWINE_JIT_ARMV8
            U32 flatIndex = getMemData(cpu->memory)->opCache.getFlatJitEntryIndex(address);
            if (flat) entryIndex = flatIndex & K_PAGE_MASK;
#endif
            if (!page || page[entryIndex] != op->pfnJitCode || !op->pfnJitCode) {
                testFail("JIT entry cache missed a published compiled entry");
                return;
            }
#ifdef BOXEDWINE_JIT_ARMV8
            if (flat && (op->getJitEntrySlot() != flat + flatIndex || flat[flatIndex] != op->pfnJitCode))
                testFail("Flat JIT entry cache publication");
#endif
            context.memory->removeCodeBlock(address, op, false);
#ifdef BOXEDWINE_JIT_ARMV8
            if (*direct) testFail("Compact direct cache JIT-only invalidation");
#endif
            if (page[entryIndex] || op->pfnJitCode || context.memory->getDecodedOp(address) != op) {
                testFail("JIT entry cache retained code after JIT-only removal");
            }
            startNewJIT(cpu, address, op);
            if (page[entryIndex] != op->pfnJitCode || !op->pfnJitCode) {
                testFail("JIT entry cache missed republication");
            }
            context.memory->writeb(address, 0x43); // invalidate and replace with inc ebx
            if (page[entryIndex]) testFail("JIT entry cache retained invalidated code");
#ifdef BOXEDWINE_JIT_ARMV8
            if (*direct) testFail("Compact direct cache guest-write invalidation");
#endif
            DecodedOp* replacement = cpu->getOp(address, 0);
            if (!replacement || replacement->getJitEntrySlot() != &page[entryIndex] || page[entryIndex]) {
                testFail("JIT entry cache replacement binding");
            }
#ifdef BOXEDWINE_JIT_ARMV8
            if (flat && !std::getenv("BOXEDWINE_ARM_COMPACT_DIRECT_SLOW") &&
                replacement->getDirectJitEntrySlot() != direct) testFail("Compact direct cache replacement binding");
            startNewJIT(cpu, address, replacement);
            if (*direct != replacement->pfnJitCode || !*direct) testFail("Compact direct cache replacement publication");
#endif
        }
        context.memory->clearOpCache();
        for (U32 i = 0; i < K_NUMBER_OF_PAGES; ++i) {
            if (groups[i >> 10][i & 0x3ff]) testFail("JIT entry cache retained a page after clear");
        }
        if (groups != getMemData(cpu->memory)->opCache.getJitPageGroups()) {
            testFail("JIT entry cache view changed during clear");
        }
#ifdef BOXEDWINE_JIT_ARMV8
        if (flat) {
            if (flat != getMemData(cpu->memory)->opCache.getFlatJitEntries())
                testFail("Flat JIT cache reservation moved during clear");
            for (U32 i = 0; i < 3; ++i)
                if (flat[getMemData(cpu->memory)->opCache.getFlatJitEntryIndex(TEST_CODE_ADDRESS + i * K_PAGE_SIZE)])
                    testFail("Flat JIT cache retained cleared code");
            for (U32 address : {0u, 0xfffu, 0x400000u, 0x80000000u, 0xfff000ffu, 0xf0ff000fu, 0xfffff000u, 0xffffffffu}) {
                void** slot = getMemData(cpu->memory)->opCache.getJitEntryLocation(address);
                if (slot != flat + getMemData(cpu->memory)->opCache.getFlatJitEntryIndex(address) || *slot)
                    testFail("Flat JIT cache boundary lookup");
            }
            context.memory->clearOpCache();
        }
#endif
    }
#endif
}

void testJitOpenGLCallStateAndInvalidation() {
#if defined(BOXEDWINE_JIT) && defined(BOXEDWINE_OPENGL) && defined(BOXEDWINE_MULTI_THREADED) && !defined(BOXEDWINE_WASM_JIT)
    ScopedGlTestCallbacks callbacks;
    const bool fastEnabled = !std::getenv("BOXEDWINE_GL_INT99_SLOW");
    for (bool mutate : { false, true }) {
        prepareGlTest();
        CPU* cpu = testContext().cpu;
        constexpr U32 iterations = JIT_RUN_COUNT * 3;
        cpu->reg[1].u32 = iterations;
        testPushCode8(0x39); testPushCode8(0xf6); // cmp esi,esi
        emitGlTestCall();
        testPushCode8(0x0f); testPushCode8(0x94); testPushCode8(0xc3); // setz bl
        if (mutate) glTestMutation = testContext().codeIp;
        testPushCode8(0x47); // inc edi, changed by the callback after JIT compilation
        testPushCode8(0x49); // dec ecx
        testPushCode8(0x75); testPushCode8(0xf1); // jnz to cmp esi,esi
        testRunCPU();
        U32 expectedEdi = mutate ? JIT_RUN_COUNT + 4 : iterations;
        if (glTestCalls != iterations || cpu->reg[0].u32 != 0xaabbccdd || cpu->reg[2].u32 != 0x76543210 ||
            cpu->reg[3].u8 != 1 || cpu->reg[7].u32 != expectedEdi || cpu->reg[6].u32 != iterations - expectedEdi) {
            testFail("OpenGL JIT callback result, flags, exact execution or modified code (mutate=%d)", mutate);
        }
        if (fastEnabled && !jitOpenGLFastCallCount) testFail("OpenGL dedicated JIT path was not exercised");
        if (mutate && !glTestSawCompiled) testFail("OpenGL callback did not invalidate compiled code");
    }
#endif
}

#ifdef BOXEDWINE_VULKAN
extern Int99Callback int9ACallback[];
extern U32 int9ACallbackSize;
namespace {
U32 vkTestCalls;
void vkTestCallback(CPU* cpu) {
    ++vkTestCalls;
    const U32 expected[] = {0xfeedcafe, 0x11223344, 0x89abcdef, 0x76543210, 0x3fc00000, 0xcafebabe};
    for (U32 i = 0; i < 6; ++i) {
        if (cpu->peek32(i) != expected[i]) testFail("Vulkan original stack word %u", i);
    }
    cpu->reg[0].u32 = 0x55667788;
    cpu->reg[2].u32 = 0x12345678;
}
void VKAPI_PTR vkTestMultiDraw(VkCommandBuffer commandBuffer, U32 drawCount, const VkMultiDrawInfoEXT* draws,
    U32 instanceCount, U32 firstInstance, U32 stride) {
    ++vkTestCalls;
    if ((U64)commandBuffer != 0x123000 || drawCount != 2 || instanceCount != 2 || firstInstance != 7 ||
        stride != sizeof(VkMultiDrawInfoEXT) || draws[0].firstVertex != 5 || draws[0].vertexCount != 3 ||
        draws[1].firstVertex != 17 || draws[1].vertexCount != 6)
        testFail("Vulkan repacked multi-draw array must use the host stride");
}
bool vkTestAllocationFails;
VkResult VKAPI_PTR vkTestAllocateCommands(VkDevice device, const VkCommandBufferAllocateInfo* allocate, VkCommandBuffer* commands) {
    if (allocate->commandBufferCount != 2 || (U64)allocate->commandPool != 0x1234567800000001ULL)
        testFail("Vulkan command allocation inputs");
    for (U32 i = 0; i < 2; ++i) commands[i] = vkTestAllocationFails ? VK_NULL_HANDLE : (VkCommandBuffer)(uintptr_t)(0x1234a000 + i * 4096);
    return vkTestAllocationFails ? VK_ERROR_OUT_OF_HOST_MEMORY : VK_SUCCESS;
}
void VKAPI_PTR vkTestFreeCommands(VkDevice device, VkCommandPool pool, U32 count, const VkCommandBuffer* commands) {
    if ((U64)pool != 0x1234567800000001ULL || count != 2 || (U64)commands[0] != 0x1234a000 || commands[1] != VK_NULL_HANDLE)
        testFail("Vulkan explicit command buffer free");
}
void VKAPI_PTR vkTestDestroyPool(VkDevice device, VkCommandPool pool, const VkAllocationCallbacks* allocator) {
    if ((U64)pool != 0x1234567800000001ULL || allocator) testFail("Vulkan command pool destroy");
}
std::vector<VkObjectType> vkTestDestroyedTypes;
VkResult VKAPI_PTR vkTestPartialPipelines(VkDevice device, VkPipelineCache cache, U32 count,
    const VkGraphicsPipelineCreateInfo* create, const VkAllocationCallbacks* allocator, VkPipeline* pipelines) {
    if (count != 2 || pipelines[0] || pipelines[1]) testFail("Vulkan partial pipeline outputs must start empty");
    pipelines[0] = (VkPipeline)0x123456789ULL;
    return VK_ERROR_OUT_OF_DEVICE_MEMORY;
}
void VKAPI_PTR vkTestDestroyPipeline(VkDevice, VkPipeline pipeline, const VkAllocationCallbacks*) {
    if ((U64)pipeline != 0x123456789ULL) testFail("Vulkan partial pipeline cleanup handle");
    vkTestDestroyedTypes.push_back(VK_OBJECT_TYPE_PIPELINE);
}
void VKAPI_PTR vkTestDestroyBuffer(VkDevice, VkBuffer, const VkAllocationCallbacks*) {
    vkTestDestroyedTypes.push_back(VK_OBJECT_TYPE_BUFFER);
}
void VKAPI_PTR vkTestDestroyImageView(VkDevice, VkImageView, const VkAllocationCallbacks*) {
    vkTestDestroyedTypes.push_back(VK_OBJECT_TYPE_IMAGE_VIEW);
}
void VKAPI_PTR vkTestDestroySwapchain(VkDevice, VkSwapchainKHR, const VkAllocationCallbacks*) {
    vkTestDestroyedTypes.push_back(VK_OBJECT_TYPE_SWAPCHAIN_KHR);
}
void VKAPI_PTR vkTestFreeMemory(VkDevice, VkDeviceMemory, const VkAllocationCallbacks*) {
    vkTestDestroyedTypes.push_back(VK_OBJECT_TYPE_DEVICE_MEMORY);
}
}
#endif

void testVulkanDirectStackABI() {
#ifdef BOXEDWINE_VULKAN
    Int99Callback saved = int9ACallback[3];
    U32 savedSize = int9ACallbackSize;
    int9ACallback[3] = vkTestCallback;
    int9ACallbackSize = 4;
    for (U32 compiled = 0; compiled < 2; ++compiled) {
        testNewInstruction(0);
        CPU* cpu = testContext().cpu;
        U32 stack = cpu->reg[4].u32;
        const U32 words[] = {0xfeedcafe, 0x11223344, 0x89abcdef, 0x76543210, 0x3fc00000, 0xcafebabe};
        for (U32 i = 0; i < 6; ++i)
            cpu->memory->writed(cpu->seg[SS].address + stack + i * 4, words[i]);
        vkTestCalls = 0;
        testPushCode8(0xcd); testPushCode8(0x9a); testPushCode32(3);
        testPushCode8(0x43); // inc ebx: execute the continuation exactly once
        testPushCode8(0xcd); testPushCode8(0x97);
        DecodedOp* op = cpu->getOp(TEST_CODE_ADDRESS, 0);
        if (op->inst != Int9A || op->imm != 3 || op->len != 6)
            testFail("Vulkan immediate command decode");
#ifdef BOXEDWINE_JIT
        if (compiled) {
            startNewJIT(cpu, TEST_CODE_ADDRESS, op);
            if (!op->pfnJitCode) testFail("Vulkan ABI test did not compile");
        } else {
            // Test builds normally compile a block on its very first run.
            op->flags |= OP_FLAG_NO_JIT;
        }
#endif
        testRunCPU();
        if (vkTestCalls != 1 || cpu->reg[4].u32 != stack || cpu->reg[3].u32 != 1 ||
            cpu->reg[0].u32 != 0x55667788 || cpu->reg[2].u32 != 0x12345678)
            testFail("Vulkan ABI result, stack, or continuation (compiled=%u)", compiled);
    }
    int9ACallback[3] = saved;
    int9ACallbackSize = savedSize;

    // Wine's output conversion buffer can contain uninitialized handle slots.
    // Only sType/pNext are input; poisoned handle bytes must never be followed.
    testNewInstruction(0);
    KMemory* memory = testContext().memory;
    U32 address = TEST_HEAP_ADDRESS + 128;
    for (U32 i = 0; i < 144; i += 4) memory->writed(address + i, 0xcdcdcdcd);
    memory->writed(address, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES);
    memory->writed(address + 4, 0);
    VkPhysicalDeviceGroupProperties properties{};
    MarshalVkPhysicalDeviceGroupProperties::read(nullptr, memory, address, &properties);
    for (auto handle : properties.physicalDevices)
        if (handle != VK_NULL_HANDLE) testFail("Vulkan output handles read from uninitialized guest data");
    properties.physicalDeviceCount = 0;
    properties.subsetAllocation = VK_TRUE;
    MarshalVkPhysicalDeviceGroupProperties::write(nullptr, memory, address, &properties);
    for (U32 i = 0; i < VK_MAX_DEVICE_GROUP_SIZE; ++i)
        if (memory->readd(address + 12 + i * 4)) testFail("Vulkan inline handle array write");
    if (memory->readd(address + 140) != VK_TRUE) testFail("Vulkan field following inline handle array");
    memory->writeq(address, 0x1234567887654321ULL);
    if (translateVulkanObjectHandle(memory, VK_OBJECT_TYPE_DEVICE, address) != 0x1234567887654321ULL ||
        translateVulkanObjectHandle(memory, VK_OBJECT_TYPE_BUFFER, 0xabcdef0187654321ULL) != 0xabcdef0187654321ULL)
        testFail("Vulkan generic object handle translation");

    // A guest VkDescriptorImageInfo is 20 bytes, versus 24 on a 64-bit host.
    // Template offsets and strides describe guest bytes and cannot be forwarded.
    BoxedVulkanInfo info{};
    auto original = std::make_shared<MarshalVkDescriptorUpdateTemplateCreateInfo>();
    original->s.descriptorUpdateEntryCount = 4;
    auto entries = new VkDescriptorUpdateTemplateEntry[4]{};
    entries[0] = {0, 0, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 12, 20};
    entries[1] = {1, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 72, 24};
    entries[2] = {2, 0, 4, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, 100, 0};
    entries[3] = {3, 0, 2, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 108, 12};
    original->s.pDescriptorUpdateEntries = entries;
    info.descriptorUpdateTemplateCreateInfo[5] = original;
    VkDescriptorUpdateTemplateCreateInfo packed = original->s;
    std::vector<VkDescriptorUpdateTemplateEntry> packedEntries;
    if (!prepareDescriptorTemplate(packed, packedEntries) || entries[0].offset != 12 || entries[0].stride != 20 ||
        packedEntries[0].stride != sizeof(VkDescriptorImageInfo) || packedEntries[1].offset != 48)
        testFail("Vulkan descriptor template host layout");
    U32 dataAddress = TEST_HEAP_ADDRESS + 512;
    for (U32 i = 0; i < 2; ++i) {
        memory->writeq(dataAddress + 12 + i * 20, 0x1234567800000000ULL + i);
        memory->writeq(dataAddress + 20 + i * 20, 0x8765432100000000ULL + i);
        memory->writed(dataAddress + 28 + i * 20, VK_IMAGE_LAYOUT_GENERAL);
    }
    memory->writeq(dataAddress + 72, 0xabcdef0123456789ULL);
    memory->writeq(dataAddress + 80, 0x100000008ULL);
    memory->writeq(dataAddress + 88, VK_WHOLE_SIZE);
    memory->writed(dataAddress + 100, 0xdeadbeef);
    memory->writeq(dataAddress + 108, 0xabcdef0100000001ULL);
    memory->writeq(dataAddress + 120, 0xabcdef0100000002ULL);
    std::vector<U8> packedData;
    marshalDescriptorTemplateData(&info, memory, (VkDescriptorUpdateTemplate)5, dataAddress, packedData);
    VkDescriptorImageInfo images[2];
    memcpy(images, packedData.data(), sizeof(images));
    VkDescriptorBufferInfo buffer;
    memcpy(&buffer, packedData.data() + packedEntries[1].offset, sizeof(buffer));
    U32 inlineData;
    memcpy(&inlineData, packedData.data() + packedEntries[2].offset, sizeof(inlineData));
    U64 accelerationStructures[2];
    memcpy(accelerationStructures, packedData.data() + packedEntries[3].offset, sizeof(accelerationStructures));
    if ((U64)images[1].sampler != 0x1234567800000001ULL || (U64)images[1].imageView != 0x8765432100000001ULL ||
        images[1].imageLayout != VK_IMAGE_LAYOUT_GENERAL || (U64)buffer.buffer != 0xabcdef0123456789ULL ||
        buffer.offset != 0x100000008ULL || buffer.range != VK_WHOLE_SIZE || inlineData != 0xdeadbeef)
        testFail("Vulkan descriptor template guest data conversion");
    if (packedEntries[3].stride != sizeof(U64) || accelerationStructures[0] != 0xabcdef0100000001ULL ||
        accelerationStructures[1] != 0xabcdef0100000002ULL)
        testFail("Vulkan NV acceleration structure descriptor template conversion");

    BoxedVulkanInfo otherDevice{};
    VkDeviceMemory allocation = (VkDeviceMemory)7;
    std::vector<U8> nativeBytes(8192);
    registerVkMemoryAllocation(&info, allocation, 0x100001000ULL);
    registerVkMemoryAllocation(&otherDevice, allocation, 128);
    if (mapVkMemory(&info, allocation, nativeBytes.data(), 0, VK_WHOLE_SIZE) ||
        mapVkMemory(&otherDevice, allocation, nativeBytes.data(), 127, 2))
        testFail("Vulkan mapping must reject overflow and out-of-bounds ranges");
    U32 mapped = mapVkMemory(&info, allocation, nativeBytes.data(), 0x100000000ULL, VK_WHOLE_SIZE);
    if (!mapped || info.allocations[7].mappedLen != 4096 || otherDevice.allocations[7].size != 128)
        testFail("Vulkan allocation ownership and partial 64-bit mapping");
    if (mapped) {
        memory->writed(mapped + 100, 0x13579bdf);
        U32 value;
        memcpy(&value, nativeBytes.data() + 100, sizeof(value));
        if (value != 0x13579bdf) testFail("Vulkan mapped guest writes must reach host memory");
        unregisterVkMemoryAllocation(&info, allocation);
        if (memory->canRead(mapped, 1)) testFail("Vulkan free must retire guest memory mapping");
    }
    if (otherDevice.allocations.size() != 1) testFail("Vulkan free affected a different logical device");
    unregisterVkMemoryAllocation(&otherDevice, allocation);

    CPU* cpu = testContext().cpu;
    info.pvkCmdDrawMultiEXT = vkTestMultiDraw;
    U32 wrapper = TEST_HEAP_ADDRESS + 800, draws = TEST_HEAP_ADDRESS + 840;
    memory->writeq(wrapper, 0x123000);
    memory->writeq(wrapper + 8, (U64)&info);
    memory->writed(draws, 5); memory->writed(draws + 4, 3);
    memory->writed(draws + 16, 17); memory->writed(draws + 20, 6);
    const U32 drawArgs[] = {0, wrapper, 2, draws, 2, 7, 16};
    for (U32 i = 0; i < 7; ++i) memory->writed(cpu->seg[SS].address + cpu->reg[4].u32 + i * 4, drawArgs[i]);
    vkTestCalls = 0;
    vk_CmdDrawMultiEXT(cpu);
    if (vkTestCalls != 1) testFail("Vulkan multi-draw callback");

    info.pvkAllocateCommandBuffers = vkTestAllocateCommands;
    info.pvkFreeCommandBuffers = vkTestFreeCommands;
    info.pvkDestroyCommandPool = vkTestDestroyPool;
    U32 allocateAddress = TEST_HEAP_ADDRESS + 900, output = TEST_HEAP_ADDRESS + 944;
    VkCommandPool pool = (VkCommandPool)0x1234567800000001ULL;
    memory->writed(allocateAddress, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
    memory->writed(allocateAddress + 4, 0);
    memory->writeq(allocateAddress + 8, (U64)pool);
    memory->writed(allocateAddress + 16, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    memory->writed(allocateAddress + 20, 2);
    auto setArgs = [&](std::initializer_list<U32> args) {
        U32 i = 0;
        for (U32 arg : args) memory->writed(cpu->seg[SS].address + cpu->reg[4].u32 + i++ * 4, arg);
    };
    for (U32 iteration = 0; iteration < 8; ++iteration) {
        vkTestAllocationFails = iteration == 0;
        memory->writed(output, 0xcdcdcdcd); memory->writed(output + 4, 0xcdcdcdcd);
        memory->writed(output + 8, 0xdeadbeef);
        setArgs({0, wrapper, allocateAddress, output});
        vk_AllocateCommandBuffers(cpu);
        if (vkTestAllocationFails) {
            if (memory->readd(output) || memory->readd(output + 4) || !info.commandBuffersByPool.empty())
                testFail("Vulkan failed allocation must clear outputs without creating wrappers");
        } else {
            U32 first = memory->readd(output), second = memory->readd(output + 4);
            if (!first || !second || first == second || info.commandBuffersByPool[(U64)pool].size() != 2)
                testFail("Vulkan successful allocation must register distinct wrappers");
            memory->writed(output + 4, 0); // vkFreeCommandBuffers permits null entries.
            setArgs({0, wrapper, 1, 0x12345678, 2, output});
            vk_FreeCommandBuffers(cpu);
            if (info.commandBuffersByPool[(U64)pool].size() != 1)
                testFail("Vulkan explicit free must retire only the named wrapper");
            setArgs({0, wrapper, 1, 0x12345678, 0});
            vk_DestroyCommandPool(cpu);
            if (!info.commandBuffersByPool.empty()) testFail("Vulkan pool destruction must retire remaining wrappers");
            U32 found = 0;
            if (cpu->thread->process->vulkanPtrMap.get((void*)0x1234a000, found) ||
                cpu->thread->process->vulkanPtrMap.get((void*)0x1234b000, found))
                testFail("Vulkan command buffer wrappers survived pool destruction");
        }
        if (memory->readd(output + 8) != 0xdeadbeef) testFail("Vulkan command output array overrun");
    }

    info.device = (VkDevice)0x123000;
    info.pvkCreateGraphicsPipelines = vkTestPartialPipelines;
    info.pvkDestroyPipeline = vkTestDestroyPipeline;
    memory->memset(allocateAddress, 0, 176); // Two guest graphics-pipeline create infos.
    memory->writed(allocateAddress, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
    memory->writed(allocateAddress + 88, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
    output = TEST_HEAP_ADDRESS + 1200;
    memory->memset(output, (char)0xcd, 16);
    setArgs({0, wrapper, 0, 0, 2, allocateAddress, 0, output});
    vk_CreateGraphicsPipelines(cpu);
    if ((VkResult)cpu->reg[0].u32 != VK_ERROR_OUT_OF_DEVICE_MEMORY || memory->readq(output) != 0x123456789ULL ||
        memory->readq(output + 8) || info.liveObjects.count({VK_OBJECT_TYPE_PIPELINE, 0x123456789ULL}) != 1)
        testFail("Vulkan partial pipeline failure lost a live handle");
    vkTestDestroyedTypes.clear();
    setArgs({0, wrapper, 0x23456789, 1, 0});
    vk_DestroyPipeline(cpu);
    if (!info.liveObjects.empty() || vkTestDestroyedTypes != std::vector<VkObjectType>{VK_OBJECT_TYPE_PIPELINE})
        testFail("Vulkan explicit destruction must retire tracked handles");

    info.pvkDestroyBuffer = vkTestDestroyBuffer;
    info.pvkDestroyImageView = vkTestDestroyImageView;
    info.pvkDestroySwapchainKHR = vkTestDestroySwapchain;
    info.pvkFreeMemory = vkTestFreeMemory;
    // The same numeric handle can name objects of different types. Cleanup must
    // preserve each one and release views/resources before swapchains/memory.
    trackVulkanObject(&info, VK_OBJECT_TYPE_BUFFER, 7);
    trackVulkanObject(&info, VK_OBJECT_TYPE_BUFFER, 7);
    trackVulkanObject(&info, VK_OBJECT_TYPE_IMAGE_VIEW, 7);
    trackVulkanObject(&info, VK_OBJECT_TYPE_SWAPCHAIN_KHR, 7);
    registerVkMemoryAllocation(&info, (VkDeviceMemory)7, 4096);
    vkTestDestroyedTypes.clear();
    cleanupVulkanObjects(&info);
    cleanupVulkanObjects(&info);
    if (!info.liveObjects.empty() || !info.allocations.empty() || vkTestDestroyedTypes !=
        std::vector<VkObjectType>{VK_OBJECT_TYPE_IMAGE_VIEW, VK_OBJECT_TYPE_BUFFER, VK_OBJECT_TYPE_SWAPCHAIN_KHR, VK_OBJECT_TYPE_DEVICE_MEMORY})
        testFail("Vulkan abandoned resources cleanup order or duplicate destruction");
#endif
}

void testJitOpenGLCallBoundaries() {
#if defined(BOXEDWINE_JIT) && defined(BOXEDWINE_OPENGL) && defined(BOXEDWINE_MULTI_THREADED) && !defined(BOXEDWINE_WASM_JIT)
    ScopedGlTestCallbacks callbacks;
    for (U32 boundary = 0; boundary <= 6; ++boundary) {
        prepareGlTest();
        CPU* cpu = testContext().cpu;
        emitGlTestCall();
        testPushCode8(0x43); // compiled continuation: inc ebx
        testPushCode8(0xcd); testPushCode8(0x97); // test end / simulated fault handler
        DecodedOp* next = cpu->getOp(TEST_CODE_ADDRESS + 6, 0);
        startNewJIT(cpu, TEST_CODE_ADDRESS + 6, next);
        DYN_PTR_SIZE expected = (DYN_PTR_SIZE)next->pfnJitCode;
        if (!expected) testFail("OpenGL boundary continuation did not compile");
        cpu->eip.u32 = 0;
        cpu->tmpReg = (3 << 8) | 6; // nonzero index, six-byte direct ABI interrupt
        cpu->thread->marshalIndex = 9;
        glTestBoundary = boundary;
        if (boundary == 6) cpu->debugTrapActive = true; // force the generic debug-aware path
        DYN_PTR_SIZE result = jitRunOpenGL(cpu);
        if (glTestCalls != 1 || cpu->eip.u32 != (boundary == 5 ? 7u : 6u)) {
            testFail("OpenGL boundary %u execution or fault EIP", boundary);
        }
        if (boundary == 0 && result != expected) testFail("OpenGL boundary failed to chain to current compiled continuation");
        if (boundary >= 1 && boundary <= 5 && result) testFail("OpenGL boundary %u failed to return to run loop", boundary);
        if (boundary == 6 && jitOpenGLFastCallCount) testFail("OpenGL debug path bypassed generic instruction handling");
        cpu->yield = false;
        cpu->thread->terminating = false;
        cpu->thread->pendingSignals = 0;
        cpu->jitSignalPending.store(0, std::memory_order_release);
        cpu->debugTrapActive = false;
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

    [[maybe_unused]] U32 producerLen = emitDirectArithmetic(op, shape, rhs);
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

void runDirectArithmeticByteMoveCases() {
    const struct {
        const char* name;
        U8 code[4];
        U32 length;
        U32 eax;
        U32 ecx;
        U32 esi;
        bool canFuse;
    } cases[] = {
        {"mov ah,imm",   {0xb4, 0x12},             2, 0x00001200, 0x123456a5, 0x13572468, false},
        {"mov ah,cl",    {0x88, 0xcc},             2, 0x0000a500, 0x123456a5, 0x13572468, false},
        {"mov cl,ah",    {0x88, 0xe1},             2, 0x00008000, 0x12345680, 0x13572468, false},
        {"mov ch,ah",    {0x88, 0xe5},             2, 0x00008000, 0x123480a5, 0x13572468, false},
        {"movzx si,ah",  {0x66, 0x0f, 0xb6, 0xf4}, 4, 0x00008000, 0x123456a5, 0x13570080, false},
        {"movsx si,ah",  {0x66, 0x0f, 0xbe, 0xf4}, 4, 0x00008000, 0x123456a5, 0x1357ff80, false},
        {"movzx esi,ah", {0x0f, 0xb6, 0xf4},       3, 0x00008000, 0x123456a5, 0x00000080, false},
        {"movsx esi,ah", {0x0f, 0xbe, 0xf4},       3, 0x00008000, 0x123456a5, 0xffffff80, false},
        {"mov cl,imm",   {0xb1, 0x12},             2, 0x00008000, 0x12345612, 0x13572468, true},
        {"mov cl,al",    {0x88, 0xc1},             2, 0x00008000, 0x12345600, 0x13572468, true},
        {"movzx esi,al", {0x0f, 0xb6, 0xf0},       3, 0x00008000, 0x123456a5, 0x00000000, true},
        {"movsx si,al",  {0x66, 0x0f, 0xbe, 0xf0}, 4, 0x00008000, 0x123456a5, 0x13570000, true},
    };
    TestContext& context = testContext();
    CPU* cpu = context.cpu;

    for (const auto& move : cases) {
        testNewInstruction(0);
        cpu->big = true;
        cpu->reg[0].u32 = 0xffff0000;
        cpu->reg[1].u32 = 0x123456a5;
        cpu->reg[2].u32 = 0x00018000;
        cpu->reg[3].u32 = 0x12345678;
        cpu->reg[6].u32 = 0x13572468;
        cpu->reg[7].u32 = 2;

        testPushCode8(0x03); // add eax,edx: result 0x8000, CF set
        testPushCode8(0xc2);
        for (U32 i = 0; i < move.length; ++i) {
            testPushCode8(move.code[i]);
        }
        testPushCode8(0x0f); // setb bl: must observe ADD's carry across the move
        testPushCode8(0x92);
        testPushCode8(0xc3);
        testPushCode8(0x39); // cmp esi,edi: makes the ADD flags dead
        testPushCode8(0xfe);
        testRunCPU();

        if (cpu->reg[0].u32 != move.eax || cpu->reg[1].u32 != move.ecx || cpu->reg[6].u32 != move.esi) {
            testFail("direct arithmetic across %s register result", move.name);
        }
        if (cpu->reg[3].u32 != 0x12345601) {
            testFail("direct arithmetic across %s lost carry", move.name);
        }
#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)
        DecodedOp* producer = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
        DecodedOp* set = context.memory->getDecodedOp(TEST_CODE_ADDRESS + 2 + move.length);
        if (!producer || producer->pfn != cpu->thread->process->startJITOp || !producer->pfnJitCode || !set) {
            testFail("direct arithmetic across %s JIT entry", move.name);
        } else if (((set->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) != 0) != move.canFuse ||
                   (set->pfnJitCode == nullptr) != move.canFuse) {
            testFail("direct arithmetic across %s fusion boundary", move.name);
        }
#endif
    }
}

void runDirectArithmeticInterveningMemoryCase(bool store, bool indirect) {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    constexpr U32 targetAddress = 0x20000000;
    const char* name = store ? "mov [moffs],al" : indirect ? "mov al,[esi]" : "mov al,[moffs]";

    testNewInstruction(0);
    cpu->big = true;
    // Leave this page untouched so the generated memory access must resolve it.
    if (context.memory->mmap(context.thread, targetAddress, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
            K_MAP_FIXED | K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0) != targetAddress) {
        testFail("direct arithmetic across %s mmap", name);
        return;
    }
    cpu->reg[0].u32 = 0x12345678;
    cpu->reg[1].u32 = 0xffffffff;
    cpu->reg[2].u32 = 1;
    cpu->reg[3].u32 = 0x12345678;
    cpu->reg[5].u32 = 2;
    cpu->reg[6].u32 = targetAddress - cpu->seg[DS].address;
    cpu->reg[7].u32 = 1;

    testPushCode8(0x03); // add ecx,edx: must execute exactly once, producing zero
    testPushCode8(0xca);
    if (indirect) {
        testPushCode8(0x8a); // mov al,[esi]
        testPushCode8(0x06);
    } else {
        testPushCode8(store ? 0xa2 : 0xa0); // mov [moffs],al / mov al,[moffs]
        testPushCode32(targetAddress - cpu->seg[DS].address);
    }
    testPushCode8(0x0f); // setz bl
    testPushCode8(0x94);
    testPushCode8(0xc3);
    testPushCode8(0x39); // cmp edi,ebp: makes the ADD flags dead
    testPushCode8(0xef);
    testRunCPU();

    if (cpu->reg[1].u32 != 0 || cpu->reg[3].u32 != 0x12345601) {
        testFail("direct arithmetic across %s first-touch fault replayed ADD or lost flags", name);
    }
    if (cpu->reg[0].u32 != (store ? 0x12345678 : 0x12345600) ||
            context.memory->readb(targetAddress) != (store ? 0x78 : 0)) {
        testFail("direct arithmetic across %s memory result", name);
    }
#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
    DecodedOp* producer = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
    DecodedOp* move = context.memory->getDecodedOp(TEST_CODE_ADDRESS + 2);
    if (!producer || producer->pfn != cpu->thread->process->startJITOp || !producer->pfnJitCode) {
        testFail("direct arithmetic across %s producer JIT entry", name);
    }
    if (!move || !move->pfnJitCode || (move->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE)) {
        testFail("direct arithmetic across %s lost faulting instruction boundary", name);
    }
#endif
    context.memory->unmap(targetAddress, K_PAGE_SIZE);
}

} // namespace

#if defined(BOXEDWINE_JIT_ARMV8)
static void testArmFlagWindowEntries() {
    auto& ctx = testContext(); CPU* cpu = ctx.cpu;
    auto bytes = [](std::initializer_list<U8> code) { for (U8 b : code) testPushCode8(b); };
    for (U32 gap : {8u, 15u, 30u, 31u, 32u}) {
        testNewInstruction(0);
        bytes({0x01,0xd0}); // ADD EAX,EDX
        U32 middle = ctx.codeIp + (gap / 2) * 2;
        for (U32 n=0; n<gap; ++n) bytes({0x89,0xf5}); // MOV EBP,ESI
        U32 consumer=ctx.codeIp;
        bytes({0x0f,0x92,0xc3,0x0f,0x94,0xc1,0x39,0xf6});
        cpu->reg[0].u32=0xffffffff;cpu->reg[2].u32=1;cpu->reg[6].u32=0x31415926;
        testRunCPU();
        if(cpu->reg[0].u32!=0||cpu->reg[3].u32!=1||cpu->reg[1].u32!=1||cpu->reg[5].u32!=0x31415926)
            testFail("ARM flag window initial gap=%u",gap);
        auto execute = [&](U32 address) {
            cpu->eip.u32=address-TEST_CODE_ADDRESS;cpu->nextOp=cpu->getNextOp();cpu->yield=false;
            U32 dispatches=0;
            do { cpu->run(); if(++dispatches>1024){testFail("ARM flag window dispatch limit");break;} }
                while(!cpu->nextOp||cpu->nextOp->inst!=TestEnd);
        };
        for(U32 target : {middle,consumer})for(U32 flags : {0u,(U32)CF,(U32)ZF,(U32)(CF|ZF)}) {
            cpu->reg[0].u32=0x12345678;cpu->reg[2].u32=7;cpu->reg[3].u32=0x87654321;
            cpu->reg[1].u32=0xabcdef12;cpu->reg[5].u32=0x55555555;
            cpu->setFlags(flags,FMASK_ALL);cpu->lazyFlagType=FLAGS_NONE;
            execute(target);
            if(cpu->reg[0].u32!=0x12345678||cpu->reg[2].u32!=7||
               cpu->reg[3].u32!=(0x87654300|((flags&CF)?1u:0u))||
               cpu->reg[1].u32!=(0xabcdef00|((flags&ZF)?1u:0u))||
               cpu->reg[5].u32!=(target==middle?0x31415926u:0x55555555u))
                testFail("ARM flag window independent entry gap=%u target=%x flags=%x",gap,target,flags);
        }
        ctx.memory->writeb(TEST_CODE_ADDRESS,0x31); // XOR EAX,EDX replaces ADD
        cpu->reg[0].u32=2;cpu->reg[2].u32=2;cpu->reg[3].u32=0;cpu->reg[1].u32=0;
        execute(TEST_CODE_ADDRESS);
        if(cpu->reg[0].u32!=0||cpu->reg[3].u32!=0||cpu->reg[1].u32!=1)
            testFail("ARM flag window code replacement gap=%u",gap);
    }
}

static void testArmDirectFlagConsumers() {
    auto& ctx = testContext();
    CPU* cpu = ctx.cpu;
    auto bytes = [](std::initializer_list<U8> code) { for (U8 b : code) testPushCode8(b); };
    for (const auto& arithmetic : DIRECT_ARITHMETIC_OPS) for (const auto& values : DIRECT_ARITHMETIC_CASES)
    for (U8 first = 0; first < 16; ++first) for (U8 second = 0; second < 16; ++second)
    for (bool cmov : {false, true}) {
        testNewInstruction(0);
        cpu->reg[0].u32 = values[0]; cpu->reg[2].u32 = values[1];
        cpu->reg[3].u32 = 0x12345678; cpu->reg[1].u32 = 0x87654321;
        cpu->reg[6].u32 = 0x01234567; cpu->reg[7].u32 = 0x89abcdef;
        bytes({arithmetic.regRmOpcode, 0xc2}); // arithmetic eax,edx
        U32 firstEip = ctx.codeIp;
        if (cmov) bytes({0x0f, (U8)(0x40 + first), 0xde}); // cmovcc ebx,esi
        else bytes({0x0f, (U8)(0x90 + first), 0xc3}); // setcc bl
        bytes({0x89, 0xfd}); // mov ebp,edi: preserves flags between consumers
        U32 secondEip = ctx.codeIp;
        if (cmov) bytes({0x66, 0x0f, (U8)(0x40 + second), 0xcf}); // cmovcc cx,di
        else bytes({0x0f, (U8)(0x90 + second), 0xc5}); // setcc ch
        bytes({0x39, 0xf6}); // kill all arithmetic flags
        testRunCPU();
        U32 result;
        U32 flags = directArithmeticFlags(arithmetic.op, values[0], values[1], result);
        bool takeFirst = directArithmeticCondition(first, flags), takeSecond = directArithmeticCondition(second, flags);
        U32 ebx = cmov ? (takeFirst ? 0x01234567 : 0x12345678) : 0x12345600 | (takeFirst ? 1 : 0);
        U32 ecx = cmov ? (takeSecond ? 0x8765cdef : 0x87654321) : 0x87650021 | (takeSecond ? 0x100 : 0);
        if (cpu->reg[0].u32 != result || cpu->reg[2].u32 != values[1] || cpu->reg[3].u32 != ebx ||
                cpu->reg[1].u32 != ecx || cpu->reg[5].u32 != 0x89abcdef || cpu->reg[4].u32 != 4096)
            testFail("ARM flag consumers op=%s a=%x b=%x first=%u second=%u cmov=%u ebx=%x ecx=%x",
                arithmetic.name, values[0], values[1], first, second, cmov, cpu->reg[3].u32, cpu->reg[1].u32);
        bool hasParity = first == 10 || first == 11 || second == 10 || second == 11;
        bool hasOverflow = first < 2 || first >= 12 || second < 2 || second >= 12;
        bool fuse = (!hasParity || (!hasOverflow && armJitHasFlagM() && !std::getenv("BOXEDWINE_ARM_INTEGER_PARITY_SLOW"))) &&
            !std::getenv("BOXEDWINE_JIT_FLAG_CHAIN_SLOW");
        for (U32 address : {firstEip, secondEip}) {
            auto* op = ctx.memory->getDecodedOp(address);
            if (!op || ((op->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) != 0) != fuse)
                testFail("ARM flag-consumer fusion selection first=%u second=%u cmov=%u", first, second, cmov);
        }
    }
    for (const auto& arithmetic : DIRECT_ARITHMETIC_OPS) for (const auto& values : DIRECT_ARITHMETIC_CASES)
    for (U8 first = 0; first < 16; ++first) for (U8 second = 0; second < 16; ++second) {
        testNewInstruction(0); cpu->reg[0].u32 = values[0]; cpu->reg[2].u32 = values[1];
        cpu->reg[3].u32 = 0x12345678;
        constexpr U32 target = TEST_CODE_ADDRESS + 0x4000;
        bytes({arithmetic.regRmOpcode, 0xc2});
        U32 firstEip = ctx.codeIp;
        bytes({0x0f, (U8)(0x90 + first), 0xc3});
        U32 branchEip = ctx.codeIp;
        bytes({0x0f, (U8)(0x80 + second)}); testPushCode32(target - ctx.codeIp - 4);
        bytes({0x39, 0xf6, 0xb9}); testPushCode32(1); bytes({0xcd, 0x97});
        ctx.codeIp = target;
        bytes({0x39, 0xf6, 0xb9}); testPushCode32(2); bytes({0xcd, 0x97});
        cpu->getOp(target, 0); // expose both successor flag kills before compilation
        testRunCPU();
        U32 result, flags = directArithmeticFlags(arithmetic.op, values[0], values[1], result);
        if (cpu->reg[0].u32 != result || cpu->reg[1].u32 != (directArithmeticCondition(second, flags) ? 2u : 1u) ||
                cpu->reg[3].u32 != (0x12345600 | (directArithmeticCondition(first, flags) ? 1u : 0u)))
            testFail("ARM flag chain side exit op=%s first=%u branch=%u a=%x b=%x", arithmetic.name, first, second, values[0], values[1]);
        bool hasParity = first == 10 || first == 11 || second == 10 || second == 11;
        bool hasOverflow = first < 2 || first >= 12 || second < 2 || second >= 12;
        bool fuse = (!hasParity || (!hasOverflow && armJitHasFlagM() && !std::getenv("BOXEDWINE_ARM_INTEGER_PARITY_SLOW"))) &&
            !std::getenv("BOXEDWINE_JIT_FLAG_CHAIN_SLOW");
        for (U32 address : {firstEip, branchEip}) {
            auto* op = ctx.memory->getDecodedOp(address);
            if (!op || ((op->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) != 0) != fuse)
                testFail("ARM flag chain side-exit selection first=%u branch=%u", first, second);
        }
    }
    // Enter a previously fused interior with fresh flags, then invalidate the
    // producer and execute the changed code. Neither can reuse hidden NZCV.
    testNewInstruction(0);
    bytes({0x39, 0xd0, 0x0f, 0x92, 0xc3, 0x0f, 0x94, 0xc1, 0x39, 0xf6});
    testRunCPU();
    for (U32 flags : {0u, (U32)CF, (U32)ZF, (U32)(CF | ZF)}) {
        cpu->reg[0].u32 = 0xabcdef01; cpu->reg[3].u32 = 0x98765432; cpu->reg[1].u32 = 0x12345678;
        cpu->flags = flags; cpu->lazyFlagType = FLAGS_NONE; cpu->eip.u32 = 5; cpu->nextOp = nullptr;
        testRunCPU();
        if (cpu->reg[0].u32 != 0xabcdef01 || cpu->reg[3].u32 != 0x98765432 || cpu->reg[1].u32 != (0x12345600 | ((flags & ZF) ? 1u : 0u)))
            testFail("ARM independent entry reused flags from a fused producer");
    }
    ctx.memory->writeb(TEST_CODE_ADDRESS, 0x01); // change cmp eax,edx to add eax,edx
    cpu->reg[0].u32 = 0xffffffff; cpu->reg[2].u32 = 1; cpu->reg[3].u32 = cpu->reg[1].u32 = 0;
    cpu->eip.u32 = 0; cpu->nextOp = nullptr;
    testRunCPU();
    if (cpu->reg[0].u32 || cpu->reg[3].u32 != 1 || cpu->reg[1].u32 != 1)
        testFail("ARM flag chain invalidation reused an old producer");

    // A live flag observer after the consumers must retain complete lazy flags.
    for (const auto& arithmetic : DIRECT_ARITHMETIC_OPS) for (const auto& values : DIRECT_ARITHMETIC_CASES) {
        testNewInstruction(0); cpu->reg[0].u32 = values[0]; cpu->reg[2].u32 = values[1];
        bytes({arithmetic.regRmOpcode, 0xc2, 0x0f, 0x92, 0xc3, 0x0f, 0x94, 0xc1, 0x9c, 0x5f});
        testRunCPU();
        U32 result, flags = directArithmeticFlags(arithmetic.op, values[0], values[1], result);
        U32 mask = CF | PF | ZF | SF | OF;
        if (((cpu->reg[7].u32 ^ flags) & mask) || cpu->reg[0].u32 != result ||
                cpu->reg[3].u32 != ((flags & CF) ? 1u : 0u) || cpu->reg[1].u32 != ((flags & ZF) ? 1u : 0u))
            testFail("ARM flag-consumer chain lost live outgoing flags");
    }
}
#endif

#ifdef BOXEDWINE_JIT_ARMV8
static void testArmNarrowArithmetic() {
    auto& ctx=testContext();CPU* cpu=ctx.cpu;
    constexpr U32 edge[]={0,1,0x7f,0x80,0xff,0x7fff,0x8000,0xffff};
    auto bytes=[](std::initializer_list<U8> v){for(U8 b:v)testPushCode8(b);};
    for(const auto& arithmetic:DIRECT_ARITHMETIC_OPS) for(U32 bits:{8u,16u})
            for(bool immediate:{false,true}) for(U32 sample=0;sample<320;++sample) for(U8 condition=0;condition<16;++condition){
        testNewInstruction(CF|OF);
        U32 mask=(1u<<bits)-1,sign=1u<<(bits-1),dst=(sample/8)%8,src=(sample/32)%8;
        U32 lhs=(sample<256?sample:edge[(sample-256)/8])&mask;
        U32 rhs=(sample<256?sample*17+3:edge[sample%8])&mask;
        U32 dstFull=bits==8?(dst&3):dst,srcFull=bits==8?(src&3):src;
        U32 dstShift=bits==8&&dst>=4?8:0,srcShift=bits==8&&src>=4?8:0;
        for(U32 r=0;r<8;++r)cpu->reg[r].u32=0xa5b60000+r*0x101;
        cpu->reg[dstFull].u32=(cpu->reg[dstFull].u32&~(mask<<dstShift))|(lhs<<dstShift);
        if(!immediate)cpu->reg[srcFull].u32=(cpu->reg[srcFull].u32&~(mask<<srcShift))|(rhs<<srcShift);
        lhs=(cpu->reg[dstFull].u32>>dstShift)&mask;
        if(!immediate)rhs=(cpu->reg[srcFull].u32>>srcShift)&mask;
        U32 expected[8];for(U32 r=0;r<8;++r)expected[r]=cpu->reg[r].u32;
        U32 result=0,flags=0,group=0;
        switch(arithmetic.op){
        case DirectArithmeticTestOp::Add:result=(lhs+rhs)&mask;if(lhs+rhs>mask)flags|=CF;if(~(lhs^rhs)&(lhs^result)&sign)flags|=OF;break;
        case DirectArithmeticTestOp::Sub:group=5;result=(lhs-rhs)&mask;if(lhs<rhs)flags|=CF;if((lhs^rhs)&(lhs^result)&sign)flags|=OF;break;
        case DirectArithmeticTestOp::And:group=4;result=lhs&rhs;break;
        case DirectArithmeticTestOp::Or:group=1;result=lhs|rhs;break;
        case DirectArithmeticTestOp::Xor:group=6;result=lhs^rhs;break;
        }
        if(group==0||group==5)if((lhs^rhs^result)&16)flags|=AF;
        if(!result)flags|=ZF;if(result&sign)flags|=SF;flags|=directArithmeticParityFlag(result);
        expected[dstFull]=(expected[dstFull]&~(mask<<dstShift))|(result<<dstShift);
        expected[0]=(expected[0]&0xffffff00)|(directArithmeticCondition(condition,flags)?1:0);
        if(bits==16)bytes({0x66});
        if(immediate){bytes({(U8)(bits==8?0x80:0x81),(U8)(0xc0+group*8+dst)});if(bits==8)testPushCode8(rhs);else testPushCode16(rhs);}
        else bytes({(U8)(group*8+(bits==16?1:0)),(U8)(0xc0+src*8+dst)});
        U32 consumer=ctx.codeIp;
        bytes({0x0f,(U8)(0x90+condition),0xc0,0x39,0xf6,0xcd,0x97});
        testRunCPU();
        for(U32 r=0;r<8;++r)if(cpu->reg[r].u32!=expected[r])
            testFail("ARM narrow %s bits=%u imm=%u sample=%u condition=%u r%u=%x expected=%x",arithmetic.name,bits,immediate,sample,condition,r,cpu->reg[r].u32,expected[r]);
        auto* set=ctx.memory->getDecodedOp(consumer);
        bool fused=arithmetic.op==DirectArithmeticTestOp::Add&&condition>=12&&
            (bits!=8||dst<4)&&!std::getenv("BOXEDWINE_ARM_NARROW_ARITH_SLOW");
        fused |= (condition==10||condition==11)&&armJitHasFlagM()&&
            !std::getenv("BOXEDWINE_ARM_NARROW_PARITY_SLOW")&&
            (bits!=8||dst<4||std::getenv("BOXEDWINE_ARM_NARROW_PARITY_HIGH_SET"));
        if(!set||((set->flags2&OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE)!=0)!=fused)
            testFail("ARM narrow arithmetic fusion selection");
    }
}
#endif

#ifdef BOXEDWINE_JIT
static void testLogicalConditionsWithLiveFlags() {
    auto& ctx = testContext(); CPU* cpu = ctx.cpu;
    constexpr U32 values[] = {0, 1, 0x7f, 0x80, 0xff, 0x7fff, 0x80000000, 0xffffffff};
    auto bytes = [](std::initializer_list<U8> v) { for (U8 b : v) testPushCode8(b); };
    for (U32 operation = 0; operation < 4; ++operation) for (U32 bits : {8u, 16u, 32u})
            for (U32 lhs : values) for (U32 rhs : values) for (U8 condition = 0; condition < 16; ++condition)
            for (bool immediate : {false, true}) {
        testNewInstruction(CF | OF);
        U32 mask = bits == 32 ? 0xffffffff : (1u << bits) - 1;
        U32 original = (0xa5a50000 & ~mask) | (lhs & mask);
        cpu->reg[0].u32 = original; cpu->reg[2].u32 = rhs;
        cpu->reg[3].u32 = 0x12345678; cpu->reg[6].u32 = 42;
        U32 result = (operation == 1 ? lhs | rhs : operation == 2 ? lhs ^ rhs : lhs & rhs) & mask;
        U32 flags = directArithmeticParityFlag(result);
        if (!result) flags |= ZF;
        if (result & (1u << (bits - 1))) flags |= SF;
        U32 group = operation == 0 ? 4 : operation == 1 ? 1 : 6;
        if (bits == 16) bytes({0x66});
        if (immediate) {
            if (operation == 3) bytes({(U8)(bits == 8 ? 0xf6 : 0xf7), 0xc0});
            else bytes({(U8)(bits == 8 ? 0x80 : 0x81), (U8)(0xc0 + group * 8)});
            if (bits == 8) testPushCode8(rhs);
            else if (bits == 16) testPushCode16(rhs);
            else testPushCode32(rhs);
        } else if (operation == 3) bytes({(U8)(bits == 8 ? 0x84 : 0x85), 0xd0});
        else bytes({(U8)(group * 8 + (bits == 8 ? 0 : 1)), 0xd0});
        bytes({0x0f, (U8)(0x90 + condition), 0xc3, 0x9c, 0x5f, 0x39, 0xf6, 0xcd, 0x97});
        testRunCPU();
        U32 expected = operation == 3 ? original : (original & ~mask) | result;
        U32 defined = CF | PF | ZF | SF | OF;
        if (cpu->reg[0].u32 != expected || cpu->reg[3].u32 != (0x12345600 | (directArithmeticCondition(condition, flags) ? 1u : 0u)) ||
                (cpu->reg[7].u32 & defined) != flags || cpu->reg[4].u32 != 4096)
            testFail("logical condition op=%u bits=%u lhs=%x rhs=%x cc=%u immediate=%u", operation, bits, lhs, rhs, condition, immediate);
    }
}
#endif

#ifdef BOXEDWINE_JIT
static void testParityConsumers() {
    auto& ctx = testContext(); CPU* cpu = ctx.cpu;
    auto bytes = [](std::initializer_list<U8> v) { for (U8 b : v) testPushCode8(b); };
    for (U32 form = 0; form < 4; ++form) for (U32 low = 0; low < 256; ++low)
            for (U32 kind = 0; kind < 3; ++kind) for (U32 shape = 0; shape < 3; ++shape) {
        testNewInstruction(CF | OF | PF);
        U32 bits = form < 2 ? 8 : form == 2 ? 16 : 32, shift = form == 1 ? 8 : 0;
        U32 mask = bits == 32 ? 0xffffffffu : (1u << bits) - 1;
        U32 seed = (0xa5c39e00u & ~(255u << shift)) | (low << shift);
        cpu->reg[0].u32 = seed; cpu->reg[1].u32 = 0x12345678;
        cpu->reg[2].u32 = 0x31415926; cpu->reg[3].u32 = 0x98765432;
        cpu->reg[5].u32 = 0;
        U32 input = (seed >> shift) & mask, operand = kind == 1 ? 0x83 : 3;
        U32 result = (kind == 0 ? input + operand : kind == 1 ? input ^ operand : input >> operand) & mask;
        U32 expectedEax = (seed & ~(mask << shift)) | (result << shift);
        bool parity = directArithmeticParityFlag(result) != 0;
        if (bits == 16) bytes({0x66});
        bytes({(U8)(kind == 2 ? (bits == 8 ? 0xc0 : 0xc1) : (bits == 8 ? 0x80 : 0x81)),
            (U8)(0xc0 + (kind == 0 ? 0 : kind == 1 ? 48 : 40) + (shift ? 4 : 0))});
        if (bits == 8 || kind == 2) testPushCode8(operand);
        else if (bits == 16) testPushCode16(operand); else testPushCode32(operand);
        U32 expectedEbx = 0x98765432, expectedEcx = 0x12345678;
        if (shape == 0) {
            bytes({0x0f, 0x9a, 0xc0, 0x0f, 0x9b, 0xc4}); // setp al; setnp ah, both alias the producer
            expectedEax = (expectedEax & 0xffff0000) | (parity ? 1 : 256);
        } else if (shape == 1) {
            bytes({0x0f, 0x9b, 0xc7, 0x66, 0x0f, 0x4a, 0xca}); // setnp bh; cmovp cx,dx
            expectedEbx = 0x98760032 | (parity ? 0 : 256);
            if (parity) expectedEcx = 0x12345926;
        } else {
            bytes({0x0f, 0x9a, 0xc3, 0x9c, 0x5d, 0x0f, 0x4b, 0xca}); // live flags through pushfd
            expectedEbx = 0x98765400 | (parity ? 1 : 0);
            if (!parity) expectedEcx = 0x31415926;
        }
        bytes({0x39, 0xf6, 0xcd, 0x97});
        testRunCPU();
        if (cpu->reg[0].u32 != expectedEax || cpu->reg[1].u32 != expectedEcx ||
                cpu->reg[2].u32 != 0x31415926 || cpu->reg[3].u32 != expectedEbx ||
                cpu->reg[4].u32 != 4096 || (shape == 2 && ((cpu->reg[5].u32 & PF) != 0) != parity))
            testFail("parity consumers form=%u low=%u kind=%u shape=%u", form, low, kind, shape);
    }
}
#endif

#ifdef BOXEDWINE_JIT_ARMV8
static void testArmSseMemoryFaultConsumers() {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
    auto& ctx=testContext();CPU* cpu=ctx.cpu;
    const U32 groupBytes=ramPageLinearMemoryPageCount()*K_PAGE_SIZE;
    constexpr U32 target=0x22000000;
    for(bool wide:{false,true})for(bool unordered:{false,true})for(U32 shape=0;shape<5;++shape)
    for(U32 state=0;state<(shape==4?1u:3u);++state)
    for(U32 gap : {0u,9u,31u})
    for(U32 offset:{0u,K_PAGE_SIZE-1u,groupBytes-1u})
    for(U32 tier:{0u,(U32)LINEAR_MEMORY_RECOMPILE_FAULTS,(U32)MAX_OP_EXCEPTION_COUNT}) {
        if (shape != 4 && gap) continue;
        testNewInstruction(0);
        auto bytes=[](std::initializer_list<U8> v){for(U8 b:v)testPushCode8(b);};
        bytes({0xd9,0xe8,0xd9,0xe8,0xde,0xc1}); // dirty ST0=2
        U32 loop=ctx.codeIp;
        bytes({0x47}); // count executions of the producer prefix
        if(shape==4){testPushCode8(wide?0xf2:0xf3);bytes({0x0f,0x58,0xc1});}
        U32 producer=ctx.codeIp;
        if(wide)testPushCode8(0x66);
        bytes({0x0f,(U8)(unordered?0x2e:0x2f),0x06}); // [ESI]
        for (U32 n=0;n<gap;++n) bytes({0x89,0xed}); // MOV EBP,EBP preserves flags
        U32 consumer=ctx.codeIp;
        if(shape==0)bytes({0x0f,0x9a,0xc3,0x0f,0x96,0xc7});
        else if(shape==1)bytes({0x0f,0x46,0xc2,0x0f,0x9b,0xc3});
        else if(shape==2){bytes({0x72,5,0xb8});testPushCode32(43);}
        else if(shape==3)bytes({0x0f,0x9a,0xc3,0x9c,0x5d});
        else {bytes({0x0f,0x82});testPushCode32(loop-ctx.codeIp-4);}
        bytes({0x39,0xc9,0xdd,0x1d});testPushCode32(0x180);
        auto prepare=[&](U32 source){
            cpu->fpu.FINIT();
            U32 values[]={42,0x12345678,0x31415926,0x98765432,4096,0x55555555,source,0};
            for(U32 r=0;r<8;++r)cpu->reg[r].u32=values[r];
            for(U32 r=0;r<8;++r)for(U32 lane=0;lane<4;++lane)cpu->xmm[r].pi.u32[lane]=0x12340000+r*16+lane;
            if(wide){cpu->xmm[0].pi.u64[0]=shape==4?0xc000000000000000ull:state==0?0:state==1?0x3ff0000000000000ull:0x7ff8123456789abcull;cpu->xmm[1].pi.u64[0]=0x3ff0000000000000ull;}
            else {cpu->xmm[0].pi.u32[0]=shape==4?0xc0000000:state==0?0:state==1?0x3f800000:0x7fc12345;cpu->xmm[1].pi.u32[0]=0x3f800000;}
            cpu->setFlags(0,FMASK_ALL);ctx.memory->writeq(TEST_HEAP_ADDRESS+0x180,0);
        };
        ctx.memory->writeq(TEST_HEAP_ADDRESS+0x100,0);
        bytes({0xcd,0x97}); // finish writing before decoding or setting fault tiers
        prepare(0x100);
        auto* first = cpu->getOp(TEST_CODE_ADDRESS,0);
        auto* memoryOp = ctx.memory->getDecodedOp(producer);
        if (!memoryOp) { testFail("SSE memory fault producer not decoded"); return; }
        memoryOp->exceptionCount=tier;
        cpu->nextOp=first;cpu->yield=false;
        U32 warmupDispatches=0;
        do { cpu->run(); if (++warmupDispatches>1024) {testFail("SSE memory warmup dispatch limit"); break;} }
            while(!cpu->nextOp||cpu->nextOp->inst!=TestEnd);
        // Interior instructions need not expose an independently callable entry.
        if(!first->pfnJitCode)testFail("SSE memory fault warmup not compiled");
        if(ctx.memory->mmap(ctx.thread,target,groupBytes*2,K_PROT_READ|K_PROT_WRITE,K_MAP_FIXED|K_MAP_PRIVATE|K_MAP_ANONYMOUS,-1,0)!=target){testFail("SSE memory fault mapping failed");return;}
        prepare(target+offset-cpu->seg[DS].address);
        U64 saved[8][2];for(U32 r=0;r<8;++r)for(U32 lane=0;lane<2;++lane)saved[r][lane]=cpu->xmm[r].pi.u64[lane];
        cpu->eip.u32=0;cpu->nextOp=cpu->getNextOp();cpu->yield=false;
        U32 dispatches=0;do{cpu->run();if(++dispatches>1024){testFail("SSE memory fault dispatch limit");break;}}while(!cpu->nextOp||cpu->nextOp->inst!=TestEnd);
        U32 flags=state==0?ZF:state==1?0:CF|ZF|PF;
        U32 eax=shape==1?(state==1?42:0x31415926):shape==2?(state==2?42:43):42;
        U32 ebx=shape==0?0x98760000|(state==2?1:0)|(state==1?0:256):shape==1?0x98765400|(state==2?0:1):shape==3?0x98765400|(state==2?1:0):0x98765432;
        if(cpu->reg[0].u32!=eax||cpu->reg[1].u32!=0x12345678||cpu->reg[2].u32!=0x31415926||cpu->reg[3].u32!=ebx||cpu->reg[4].u32!=4096||cpu->reg[6].u32!=target+offset-cpu->seg[DS].address||cpu->reg[7].u32!=(shape==4?2u:1u)||
                (shape==3&&(cpu->reg[5].u32&FMASK_TEST)!=flags)||ctx.memory->readq(TEST_HEAP_ADDRESS+0x180)!=0x4000000000000000ull)
            testFail("SSE memory fault wide=%u unordered=%u shape=%u state=%u offset=%u tier=%u gap=%u",wide,unordered,shape,state,offset,tier,gap);
        if(shape==4){if(wide)saved[0][0]=0;else saved[0][0]&=0xffffffff00000000ull;}
        for(U32 r=0;r<8;++r)for(U32 lane=0;lane<2;++lane)if(cpu->xmm[r].pi.u64[lane]!=saved[r][lane])testFail("SSE memory fault clobbered xmm%u",r);
        if(shape==4){auto* op=ctx.memory->getDecodedOp(consumer);if(op&&(op->flags2&OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE))testFail("SSE memory backward branch fused after load");}
        ctx.memory->unmap(target,groupBytes*2);
    }
#endif
}

static void testArmMinMaxSelection() {
    constexpr U32 singles[] = {0,0x80000000,0x3f800000,0xbf800000,1,0x807fffff,
        0x7f800000,0xff800000,0x7fc12345,0x7f812345,0xffc23456,0xff823456};
    constexpr U64 doubles[] = {0,0x8000000000000000ull,0x3ff0000000000000ull,0xbff0000000000000ull,
        1,0x800fffffffffffffull,0x7ff0000000000000ull,0xfff0000000000000ull,
        0x7ff8123456789abcull,0x7ff0123456789abcull,0xfff823456789abcdull,0xfff023456789abcdull};
    auto selected = [](U64 a,U64 b,bool wide,bool maximum) {
        U64 magnitudeMask=wide?0x7fffffffffffffffull:0x7fffffffull;
        U64 infinity=wide?0x7ff0000000000000ull:0x7f800000ull;
        if((a&magnitudeMask)>infinity||(b&magnitudeMask)>infinity)return b;
        double lhs,rhs;
        if(wide){memcpy(&lhs,&a,8);memcpy(&rhs,&b,8);}
        else {float x,y;U32 aa=(U32)a,bb=(U32)b;memcpy(&x,&aa,4);memcpy(&y,&bb,4);lhs=x;rhs=y;}
        return (maximum?lhs>rhs:lhs<rhs)?a:b;
    };
    auto& ctx=testContext();CPU* cpu=ctx.cpu;
    for(bool wide:{false,true})for(bool scalar:{false,true})for(bool maximum:{false,true})
    for(U32 lhs=0;lhs<12;++lhs)for(U32 rhs=0;rhs<12;++rhs)for(U32 shape=0;shape<5;++shape){
        testNewInstruction(CF|PF|AF|ZF|SF|OF);
        const bool memory=shape>=2, alias=shape==1;
        const U32 offset=shape==3?0xffd:shape==4?0x3ffd:0x100;
        for(U32 reg=0;reg<8;++reg)for(U32 lane=0;lane<4;++lane)cpu->xmm[reg].pi.u32[lane]=0x12340000+reg*16+lane;
        U64 expected[2]={};
        if(wide)for(U32 lane=0;lane<2;++lane){
            U64 a=doubles[(lhs+lane)%12],b=doubles[(rhs+lane*2)%12];
            cpu->xmm[0].pi.u64[lane]=a;cpu->xmm[1].pi.u64[lane]=b;
            expected[lane]=scalar&&lane?a:selected(a,alias?a:b,true,maximum);
            if(memory)ctx.memory->writeq(TEST_HEAP_ADDRESS+offset+lane*8,b);
        }else for(U32 lane=0;lane<4;++lane){
            U32 a=singles[(lhs+lane)%12],b=singles[(rhs+lane*2)%12];
            cpu->xmm[0].pi.u32[lane]=a;cpu->xmm[1].pi.u32[lane]=b;
            U32 value=scalar&&lane?a:(U32)selected(a,alias?a:b,false,maximum);
            expected[lane/2]|=(U64)value<<((lane&1)*32);
            if(memory)ctx.memory->writed(TEST_HEAP_ADDRESS+offset+lane*4,b);
        }
        U64 preserved[7][2];for(U32 reg=1;reg<8;++reg)for(U32 lane=0;lane<2;++lane)preserved[reg-1][lane]=cpu->xmm[reg].pi.u64[lane];
        auto bytes=[](std::initializer_list<U8> v){for(U8 b:v)testPushCode8(b);};
        bytes({0xd9,0xe8,0xd9,0xe8,0xde,0xc1}); // dirty x87 ST0 = 2
        if(wide)testPushCode8(scalar?0xf2:0x66);else if(scalar)testPushCode8(0xf3);
        bytes({0x0f,(U8)(maximum?0x5f:0x5d),(U8)(memory?0x05:alias?0xc0:0xc1)});
        if(memory)testPushCode32(offset);
        bytes({0x9c,0x5b,0xdd,0x1d});testPushCode32(0x180);
        testRunCPU();
        if(cpu->xmm[0].pi.u64[0]!=expected[0]||cpu->xmm[0].pi.u64[1]!=expected[1]||
                (cpu->reg[3].u32&FMASK_TEST)!=(CF|PF|AF|ZF|SF|OF)||cpu->reg[4].u32!=4096||
                ctx.memory->readq(TEST_HEAP_ADDRESS+0x180)!=0x4000000000000000ull)
            testFail("ARM min/max wide=%u scalar=%u maximum=%u lhs=%u rhs=%u shape=%u",wide,scalar,maximum,lhs,rhs,shape);
        for(U32 reg=1;reg<8;++reg)for(U32 lane=0;lane<2;++lane)if(cpu->xmm[reg].pi.u64[lane]!=preserved[reg-1][lane])testFail("ARM min/max clobbered xmm%u",reg);
    }
}

static void testArmSseCompareConsumers() {
    struct Value { U32 single; U64 dbl; };
    constexpr Value values[] = {
        {0, 0}, {0x80000000, 0x8000000000000000ull},
        {0x3f800000, 0x3ff0000000000000ull}, {0xbf800000, 0xbff0000000000000ull},
        {1, 1}, {0x80000001, 0x8000000000000001ull},
        {0x7f800000, 0x7ff0000000000000ull}, {0xff800000, 0xfff0000000000000ull},
        {0x7fc12345, 0x7ff8123456789abcull}, {0x7f812345, 0x7ff0123456789abcull}
    };
    auto& ctx = testContext(); CPU* cpu = ctx.cpu;
    auto bytes = [](std::initializer_list<U8> v) { for (U8 b : v) testPushCode8(b); };
    for (bool wide : {false, true}) for (bool unordered : {false, true})
            for (const auto& lhs : values) for (const auto& rhs : values)
            for (U32 kind = 0; kind < 10; ++kind) for (U8 condition = 0; condition < 16; ++condition) {
        const bool memory = kind >= 5;
        const U32 shape = memory ? kind - 5 : kind;
        testNewInstruction(CF | PF | ZF | SF | OF);
        cpu->reg[0].u32 = 42; cpu->reg[1].u32 = 0x12345678;
        cpu->reg[2].u32 = 0x31415926; cpu->reg[3].u32 = 0x98765432;
        cpu->reg[6].u32 = 0; cpu->reg[7].u32 = 0;
        for (U32 lane = 0; lane < 4; ++lane) {
            cpu->xmm[0].pi.u32[lane] = 21 + lane; cpu->xmm[1].pi.u32[lane] = 31 + lane;
        }
        double a, b;
        if (wide) {
            cpu->xmm[0].pi.u64[0] = lhs.dbl; cpu->xmm[1].pi.u64[0] = rhs.dbl;
            memcpy(&a, &lhs.dbl, 8); memcpy(&b, &rhs.dbl, 8);
        } else {
            cpu->xmm[0].pi.u32[0] = lhs.single; cpu->xmm[1].pi.u32[0] = rhs.single;
            float af, bf; memcpy(&af, &lhs.single, 4); memcpy(&bf, &rhs.single, 4); a = af; b = bf;
        }
        U64 xmm[4] = {cpu->xmm[0].pi.u64[0], cpu->xmm[0].pi.u64[1], cpu->xmm[1].pi.u64[0], cpu->xmm[1].pi.u64[1]};
        U32 flags = a != a || b != b ? CF | ZF | PF : a == b ? ZF : a < b ? CF : 0;
        bool expected = directArithmeticCondition(condition, flags);
        bool second = directArithmeticCondition(condition ^ 4, flags);
        bytes({0xd9, 0xe8, 0xd9, 0xe8, 0xde, 0xc1}); // dirty cached ST0 = 2
        if (wide) bytes({0x66});
        bytes({0x0f, (U8)(unordered ? 0x2e : 0x2f), (U8)(memory ? 0x05 : 0xc1)});
        if (memory) {
            testPushCode32(0x100);
            ctx.memory->writeq(TEST_HEAP_ADDRESS + 0x100, wide ? rhs.dbl : rhs.single);
        }
        U32 consumer = ctx.codeIp;
        if (shape == 2) bytes({0x0f, (U8)(0x40 + condition), 0xc2});
        else if (shape == 3) {
            bytes({0x0f, (U8)(0x80 + condition)}); testPushCode32(5);
            bytes({0xb8}); testPushCode32(43);
        } else {
            bytes({0x0f, (U8)(0x90 + condition), 0xc3});
            if (shape == 1) bytes({0x0f, (U8)(0x90 + (condition ^ 4)), 0xc1});
            if (shape == 4) bytes({0x9c, 0x5f});
        }
        bytes({0x39, 0xf6, 0xdd, 0x1d}); testPushCode32(0x180); bytes({0xcd, 0x97});
        if (shape == 3) cpu->getOp(consumer + 11, 0);
        testRunCPU();
        U32 eax = shape == 2 ? (expected ? 0x31415926u : 42u) : shape == 3 ? (expected ? 42u : 43u) : 42u;
        U32 ebx = shape == 2 || shape == 3 ? 0x98765432u : 0x98765400u | (expected ? 1u : 0u);
        U32 ecx = shape == 1 ? 0x12345600u | (second ? 1u : 0u) : 0x12345678u;
        if (cpu->reg[0].u32 != eax || cpu->reg[3].u32 != ebx || cpu->reg[1].u32 != ecx ||
                cpu->reg[2].u32 != 0x31415926 || cpu->reg[4].u32 != 4096 ||
                (shape == 4 && (cpu->reg[7].u32 & FMASK_TEST) != flags) ||
                ctx.memory->readq(TEST_HEAP_ADDRESS + 0x180) != 0x4000000000000000ull ||
                cpu->xmm[0].pi.u64[0] != xmm[0] || cpu->xmm[0].pi.u64[1] != xmm[1] ||
                cpu->xmm[1].pi.u64[0] != xmm[2] || cpu->xmm[1].pi.u64[1] != xmm[3])
            testFail("ARM SSE compare wide=%u unordered=%u kind=%u cc=%u lhs=%llx rhs=%llx", wide, unordered, kind, condition, lhs.dbl, rhs.dbl);
        auto* op = ctx.memory->getDecodedOp(consumer);
        auto nativeCondition = [](U8 cc) { return cc == 2 || cc == 3 || cc == 6 || cc == 7 || cc == 10 || cc == 11; };
        bool native = !std::getenv("BOXEDWINE_ARM_SSE_NATIVE_FLAGS_SLOW") && nativeCondition(condition) &&
            (shape != 1 || nativeCondition(condition ^ 4));
        bool converted = armJitHasFlagM2() && condition != 10 && condition != 11 &&
            (shape != 1 || ((condition ^ 4) != 10 && (condition ^ 4) != 11));
        bool fused = (native || converted) && shape < 4 &&
            (!memory || !std::getenv("BOXEDWINE_ARM_SSE_MEMORY_SLOW")) &&
            !std::getenv("BOXEDWINE_ARM_SSE_COMPARE_SLOW") && (shape != 1 || !std::getenv("BOXEDWINE_JIT_FLAG_CHAIN_SLOW"));
        if (!op || ((op->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) != 0) != fused)
            testFail("ARM SSE compare fusion selection kind=%u cc=%u", kind, condition);
    }
}
#endif

static void testJitMultiplyOverflowState() {
#ifdef BOXEDWINE_JIT
    auto& ctx=testContext();CPU* cpu=ctx.cpu;
    constexpr U32 edge[]={0,1,2,0xffffffff,0x80000000,0x7fffffff,65535,65536,0xffff0000};
    const U8 aliases[][2]={{0,0},{0,2},{2,0},{3,6},{6,3}};
    auto bytes=[](std::initializer_list<U8> v){for(U8 b:v)testPushCode8(b);};
    for(U32 form=0;form<8;++form)for(U32 a:edge)for(U32 b:edge)
    for(const auto& alias:aliases)for(U32 shape=0;shape<5;++shape)for(bool lazy:{false,true}){
        testNewInstruction(CF|OF);
        U32 expected[8]={0x13579bdf,0x12345678,0x87654321,0x10203040,4096,0x55667788,0xaabbccdd,0xfedcba98};
        bool one=form<2||form>=6,immediate=form==3||form==5,memory=form>=4,isSigned=form!=0&&form!=6;
        U8 dst=one?0:alias[0],src=alias[1];
        if(immediate)expected[src]=a;
        else{expected[dst]=a;if(!memory)expected[src]=b;}
        U32 left=immediate?(memory?a:expected[src]):expected[dst];
        U32 right=immediate?b:(memory?b:expected[src]);
        for(U32 r=0;r<8;++r)cpu->reg[r].u32=expected[r];
        for(U32 r=0;r<8;++r)for(U32 lane=0;lane<4;++lane)cpu->xmm[r].pi.u32[lane]=r*16+lane+1;
        if(memory)ctx.memory->writed(TEST_HEAP_ADDRESS+0x100,immediate?a:b);
        if(one){bytes({0xf7,(U8)((memory?5:0xc0)+(isSigned?0x28:0x20)+(memory?0:src))});if(memory)testPushCode32(0x100);}
        else if(immediate){bytes({0x69,(U8)((memory?5:0xc0)+dst*8+(memory?0:src))});if(memory)testPushCode32(0x100);testPushCode32(b);}
        else{bytes({0x0f,0xaf,(U8)((memory?5:0xc0)+dst*8+(memory?0:src))});if(memory)testPushCode32(0x100);}
        U64 product=isSigned?(U64)((S64)(S32)left*(S64)(S32)right):(U64)left*right;
        bool overflow=isSigned?(S64)product!=(S64)(S32)(U32)product:(product>>32)!=0;
        expected[dst]=(U32)product;if(one)expected[2]=(U32)(product>>32);
        U8 cc=(U8)((a^b)&3);bool take=(cc&1)?!overflow:overflow;
        if(shape==0){bytes({0x0f,(U8)(0x90+cc),0xc1});expected[1]=(expected[1]&0xffffff00)|(take?1:0);}
        else if(shape==1){bytes({0x66,0x0f,(U8)(0x40+cc),0xd6});if(take)expected[2]=(expected[2]&0xffff0000)|(expected[6]&0xffff);}
        else if(shape==2){bytes({0x0f,0x92,0xc1,0x89,0xfd,0x0f,0x90,0xc5});expected[1]=(expected[1]&0xffff0000)|(overflow?0x101:0);expected[5]=expected[7];}
        else if(shape==3){bytes({(U8)(0x70+cc),5,0xb9});testPushCode32(0x31415926);if(!take)expected[1]=0x31415926;}
        else bytes({0x9c,0x59}); // PUSHFD; POP ECX: retain live architectural CF/OF
        bytes({0x39,0xff});
        if(lazy){
            // Contradict the multiply result with flags from a previous block.
            cpu->dst.u32=overflow?0:0xffffffff;cpu->src.u32=overflow?0:1;
            cpu->result.u32=0;cpu->lazyFlagType=FLAGS_ADD32;
        }
        testRunCPU();
        for(U32 r=0;r<8;++r){
            if(shape==4&&r==1){if((cpu->reg[r].u32&(CF|OF))!=(overflow?(CF|OF):0))testFail("multiply live flags form=%u a=%x b=%x lazy=%u got=%x",form,a,b,lazy,cpu->reg[r].u32);}
            else if(cpu->reg[r].u32!=expected[r])testFail("multiply state form=%u a=%x b=%x dst=%u src=%u shape=%u lazy=%u r%u=%x expected=%x",form,a,b,dst,src,shape,lazy,r,cpu->reg[r].u32,expected[r]);
            for(U32 lane=0;lane<4;++lane)if(cpu->xmm[r].pi.u32[lane]!=r*16+lane+1)testFail("multiply SIMD state");
        }
    }
#endif
}

static void testJitFlagResultRegions() {
#if defined(BOXEDWINE_WASM_JIT) || defined(BOXEDWINE_JIT_ARMV8)
    extern void startNewJIT(CPU*, U32, DecodedOp*);
    auto& ctx=testContext();CPU* cpu=ctx.cpu;
    struct Snapshot { U32 regs[8],flags;U64 xmm[16]; };
    auto capture=[&](){Snapshot state{};for(U32 r=0;r<8;++r){state.regs[r]=cpu->reg[r].u32;state.xmm[r*2]=cpu->xmm[r].pi.u64[0];state.xmm[r*2+1]=cpu->xmm[r].pi.u64[1];}cpu->fillFlags();state.flags=cpu->flags&FMASK_TEST;return state;};
    auto execute=[&](bool jit){
        U32 steps=0;
        if(jit){do{cpu->run();if(++steps>1024){testFail("JIT flag region JIT dispatch limit");return;}}while(!cpu->nextOp||cpu->nextOp->inst!=TestEnd);}
        else while(cpu->getNextOp()->inst!=TestEnd){if(++steps>1024||!cpu->runNextSingleOp()){testFail("JIT flag region interpreter limit");return;}}
    };
    const U32 values[]={0,1,0x7f,0xff,0x7fffffff,0xffffffff};
    for(U32 producer=0;producer<10;++producer)for(U32 form=0;form<4;++form)for(U32 seed:values)for(U32 gap:{0u,1u,15u,31u,32u})for(U32 shape=0;shape<3;++shape){
        std::vector<U8> code;
        auto bytes=[&](std::initializer_list<U8> v){for(U8 b:v)code.push_back(b);};
        auto dword=[&](U32 value){for(U32 n=0;n<4;++n)code.push_back((U8)(value>>(n*8)));};
        U32 consumer=0;
        for(U32 repeat=0;repeat<2;++repeat){
            if(form==2)bytes({0x66});
            U8 operand=form==1?4:0;
            if(producer==5) { // TEST AL/AH/AX/EAX,13
                bytes({(U8)(form<2?0xf6:0xf7),(U8)(0xc0+operand),13});
                if(form==2)bytes({0});else if(form==3)bytes({0,0,0});
            } else if(producer>=7) { // INC, DEC, NEG
                U8 opcode=producer==9?(form<2?0xf6:0xf7):(form<2?0xfe:0xff);
                U8 group=producer==7?0:producer==8?1:3;
                bytes({opcode,(U8)(0xc0+group*8+operand)});
            } else {
                const U8 groups[]={0,5,6,4,1,0,7}; // ADD/SUB/XOR/AND/OR/TEST/CMP
                bytes({(U8)(form<2?0x80:0x83),(U8)(0xc0+groups[producer]*8+operand),13});
            }
            bytes({0xb8});dword(0xdeadbeef); // the saved result must not alias EAX
            for(U32 n=0;n<gap;++n)if(n&1)bytes({0x8d,0x6d,1});else bytes({0x89,0xf5});
            if(shape==1){bytes({0x8b,0x1d});dword(0x100);}
            if(shape==2)bytes({0x75,2,0x89,0xf5}); // forward join preserving arithmetic flags
            if(!repeat)consumer=(U32)code.size();
            bytes({0x0f,0x9a,0xc2,0xb8});dword(0x11223344);
            if(repeat)bytes({0x0f,0x48,0xdf}); // CMOVS EBX,EDI
            else bytes({0x0f,0x98,0xc3}); // SETS BL keeps the first span straight-line
            bytes({0x0f,0x94,0xc6});
            if(repeat) {
                bytes({0x9c,0x59}); // observe the final published lazy state
                // Logical operations leave AF undefined; compare only defined bits.
                if(producer>=2&&producer<=5){bytes({0x81,0xe1});dword(~(U32)AF);}
            }
            bytes({0x8d,0x34,0x16}); // consume earlier flag results in later data flow
        }
        bytes({0x39,0xff,0xcd,0x97});
        auto registers=[&](U32 flags,U32 eip){
            U32 initial[]={seed,0x11223344,0x44556600,0x12345678,4096,0x13579bdf,0x2468ace0,0xabcdef01};
            for(U32 r=0;r<8;++r){cpu->reg[r].u32=initial[r];cpu->xmm[r].pi.u64[0]=0x1234567800000000ull+r;cpu->xmm[r].pi.u64[1]=0xabcdef1200000000ull+r;}
            cpu->setFlags(flags,FMASK_ALL);cpu->lazyFlagType=FLAGS_NONE;cpu->eip.u32=eip;cpu->nextOp=nullptr;cpu->yield=false;
        };
        auto setup=[&](){testNewInstruction(0);for(U8 b:code)testPushCode8(b);ctx.memory->writed(TEST_HEAP_ADDRESS+0x100,0x98765432);registers(CF|OF,0);};
        setup();execute(false);Snapshot expected=capture();
        setup();auto* entry=cpu->getOp(TEST_CODE_ADDRESS,0);startNewJIT(cpu,TEST_CODE_ADDRESS,entry);cpu->nextOp=entry;execute(true);Snapshot actual=capture();
        if(expected.flags!=actual.flags || memcmp(expected.regs,actual.regs,sizeof(expected.regs)) || memcmp(expected.xmm,actual.xmm,sizeof(expected.xmm)))testFail("JIT flag region producer=%u form=%u seed=%x gap=%u shape=%u",producer,form,seed,gap,shape);
        if(!entry->pfnJitCode)testFail("JIT flag region did not compile");
        if(gap==15&&shape==0&&seed==0){
            // A prior root compilation must not expose its private result local
            // to an independent entry at the former consumer.
            for(U32 flags:{0u,(U32)PF,(U32)SF,(U32)ZF,(U32)(PF|SF|ZF)}){
                registers(flags,consumer);execute(false);expected=capture();
                registers(flags,consumer);U32 address=TEST_CODE_ADDRESS+consumer;
                auto* middle=cpu->getOp(address,0);startNewJIT(cpu,address,middle);cpu->nextOp=middle;execute(true);actual=capture();
                if(expected.flags!=actual.flags || memcmp(expected.regs,actual.regs,sizeof(expected.regs)) || memcmp(expected.xmm,actual.xmm,sizeof(expected.xmm)))testFail("JIT flag region independent entry producer=%u form=%u flags=%x",producer,form,flags);
            }
        }
    }
#endif
}

static void testArmConditionalCompare() {
#ifdef BOXEDWINE_JIT_ARMV8
    extern void startNewJIT(CPU*, U32, DecodedOp*);
    auto& ctx = testContext(); CPU* cpu = ctx.cpu;
    constexpr U32 edge[] = {0,1,2,0xffffffff,0x80000000,0x7fffffff,0xaaaaaaaa,0x55555555,0x10000,31,32,4095,4096};
    constexpr U32 edgeCount = sizeof(edge) / sizeof(edge[0]);
    auto bytes = [](std::initializer_list<U8> code) { for (U8 b : code) testPushCode8(b); };
    auto execute = [&](U32 address) {
        cpu->eip.u32=address-TEST_CODE_ADDRESS;cpu->nextOp=nullptr;cpu->yield=false;
        U32 dispatches=0;
        do {cpu->run();if(++dispatches>1024){testFail("CCMP dispatch limit");break;}}
            while(!cpu->nextOp||cpu->nextOp->inst!=TestEnd);
    };
    for(U8 first=0;first<16;++first)for(U8 second=0;second<16;++second)
    for(U32 seed=0;seed<edgeCount;++seed)for(U32 form=0;form<4;++form)for(U32 live=0;live<3;++live) {
        testNewInstruction(CF|OF);
        const U32 a=edge[seed],b=edge[(seed+3)%edgeCount],c=edge[(seed+5)%edgeCount],d=edge[(seed+1)%edgeCount];
        U32 expected[]={a,0x12345678,d,0,4096,0x31415926,b,c};
        for(U32 r=0;r<8;++r){cpu->reg[r].u32=expected[r];for(U32 lane=0;lane<4;++lane)cpu->xmm[r].pi.u32[lane]=r*16+lane+3;}
        const U32 lhs1=form==3?b:a,rhs1=form==2?a:form==3?a:b;
        const U32 lhs2=form>=2?a:d,rhs2=form>=2?b:c;
        if(form==1){bytes({0x81,0xf8});testPushCode32(rhs1);}
        else bytes({0x39,(U8)(form==2?0xc0:form==3?0xc6:0xf0)});
        const U32 branch1=ctx.codeIp;bytes({0x0f,(U8)(0x80+first)});U32 disp1=ctx.codeIp;testPushCode32(0);
        const U32 compare2=ctx.codeIp;
        if(form==1){bytes({0x81,0xfa});testPushCode32(rhs2);}
        else bytes({0x39,(U8)(form>=2?0xf0:0xfa)});
        const U32 branch2=ctx.codeIp;bytes({0x0f,(U8)(0x80+second)});U32 disp2=ctx.codeIp;testPushCode32(0);
        bytes({0xbb});testPushCode32(1);
        if(live==2)bytes({0x9c,0x5d});
        const U32 target=ctx.codeIp;
        if(live==1)bytes({0x9c,0x5d});
        bytes({0x39,0xff,0xcd,0x97});
        ctx.memory->writed(disp1,target-disp1-4);ctx.memory->writed(disp2,target-disp2-4);
        cpu->getOp(target,0); // expose the flag kill at the shared successor
        auto* entry=cpu->getOp(TEST_CODE_ADDRESS,0);startNewJIT(cpu,TEST_CODE_ADDRESS,entry);
        U32 ignored;U32 flags1=directArithmeticFlags(DirectArithmeticTestOp::Sub,lhs1,rhs1,ignored);
        U32 flags2=directArithmeticFlags(DirectArithmeticTestOp::Sub,lhs2,rhs2,ignored);
        bool take1=directArithmeticCondition(first,flags1),take2=directArithmeticCondition(second,flags2);
        expected[3]=!take1&&!take2;
        execute(TEST_CODE_ADDRESS);
        for(U32 r=0;r<8;++r) {
            if(r==5&&live&&(live==1||expected[3])) {
                U32 wanted=take1?flags1:flags2;
                if((cpu->reg[r].u32&FMASK_TEST)!=(wanted&FMASK_TEST))testFail("CCMP live flags first=%u second=%u form=%u live=%u",first,second,form,live);
            } else if(cpu->reg[r].u32!=expected[r])testFail("CCMP result first=%u second=%u seed=%u form=%u live=%u r%u",first,second,seed,form,live,r);
            for(U32 lane=0;lane<4;++lane)if(cpu->xmm[r].pi.u32[lane]!=r*16+lane+3)testFail("CCMP SIMD state");
        }
        cpu->fillFlags();
        if((cpu->flags&FMASK_TEST)!=(ZF|PF))testFail("CCMP final flags");
        if(!live&&first!=10&&first!=11&&second!=10&&second!=11&&std::getenv("BOXEDWINE_ARM_CCMP")) {
            auto* op=ctx.memory->getDecodedOp(compare2);
            if(!op||!(op->flags2&OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE))testFail("CCMP candidate was not fused");
        }
        if(!live&&form==0&&seed==0) {
            for(U32 address:{branch1,compare2,branch2})for(U32 fresh:{0u,(U32)FMASK_TEST}) {
                for(U32 r=0;r<8;++r)cpu->reg[r].u32=expected[r];cpu->reg[3].u32=0;
                cpu->setFlags(fresh,FMASK_ALL);cpu->lazyFlagType=FLAGS_NONE;
                bool fail=address==branch1?directArithmeticCondition(first,fresh)||take2:
                    address==compare2?take2:directArithmeticCondition(second,fresh);
                execute(address);
                if(cpu->reg[3].u32!=(fail?0u:1u))testFail("CCMP independent entry first=%u second=%u address=%x",first,second,address);
            }
            // Replace the second comparison with ADD. The newly written guest
            // instruction must execute even when the former region was fused.
            ctx.memory->writeb(compare2,0x01);
            for(U32 r=0;r<8;++r)cpu->reg[r].u32=expected[r];cpu->reg[3].u32=0;
            U32 result;U32 addFlags=directArithmeticFlags(DirectArithmeticTestOp::Add,d,c,result);
            execute(TEST_CODE_ADDRESS);
            if(cpu->reg[2].u32!=(take1?d:result)||cpu->reg[3].u32!=(!take1&&!directArithmeticCondition(second,addFlags)?1u:0u))
                testFail("CCMP code replacement first=%u second=%u",first,second);
        }
    }
#endif
}

void testArmKnownLazyFlagType() {
#ifdef BOXEDWINE_JIT_ARMV8
    extern void startNewJIT(CPU*, U32, DecodedOp*);
    auto& ctx = testContext(); CPU* cpu = ctx.cpu;
    struct State { U32 regs[8], flags; U64 xmm[16]; };
    auto capture = [&]() {
        State state{};
        for (U32 r = 0; r < 8; ++r) {
            state.regs[r] = cpu->reg[r].u32;
            state.xmm[2*r] = cpu->xmm[r].pi.u64[0]; state.xmm[2*r+1] = cpu->xmm[r].pi.u64[1];
        }
        cpu->fillFlags(); state.flags = cpu->flags & FMASK_TEST; return state;
    };
    auto same = [](const State& a, const State& b) {
        return a.flags == b.flags && !memcmp(a.regs, b.regs, sizeof(a.regs)) && !memcmp(a.xmm, b.xmm, sizeof(a.xmm));
    };
    auto execute = [&](bool jit) {
        U32 steps = 0;
        if (jit) do {
            cpu->run(); if (++steps > 1024) { testFail("known flags JIT dispatch limit"); return; }
        } while (!cpu->nextOp || cpu->nextOp->inst != TestEnd);
        else while (cpu->getNextOp()->inst != TestEnd) {
            if (++steps > 1024 || !cpu->runNextSingleOp()) { testFail("known flags interpreter limit"); return; }
        }
    };
    for (U32 form = 0; form < 9; ++form) for (const auto& input : DIRECT_ARITHMETIC_CASES)
    for (U8 cc = 0; cc < 16; ++cc) for (U32 gap : {1u, 4u, 16u, 17u})
    for (U32 shape = 0; shape < 3; ++shape) for (bool live : {false, true}) {
        std::vector<U8> code;
        auto bytes = [&](std::initializer_list<U8> values) { for (U8 b : values) code.push_back(b); };
        auto dword = [&](U32 value) { for (U32 i = 0; i < 4; ++i) code.push_back((U8)(value >> (8*i))); };
        switch (form) {
        case 0: bytes({0x3b, 0xc2}); break; // CMP EAX,EDX
        case 1: bytes({0x3b, 0x05}); dword(0x104); break;
        case 2: bytes({0x39, 0x15}); dword(0x100); break;
        case 3: bytes({0x81, 0xf8}); dword(input[1]); break;
        case 4: bytes({0x81, 0x3d}); dword(0x100); dword(input[1]); break;
        case 5: bytes({0x85, 0xd0}); break; // TEST EAX,EDX
        case 6: bytes({0x85, 0x15}); dword(0x100); break;
        case 7: bytes({0xf7, 0xc0}); dword(input[1]); break;
        default: bytes({0xf7, 0x05}); dword(0x100); dword(input[1]); break;
        }
        std::vector<U32> entries;
        for (U32 move = 0; move < gap; ++move) {
            entries.push_back((U32)code.size());
            switch (move & 7) {
            case 0: bytes({0x89, 0xf0}); break; // Overwrite the original comparison operands.
            case 1: bytes({0x89, 0xfa}); break;
            case 2: bytes({0x8b, 0x2d}); dword(0x110); break;
            case 3: bytes({0x89, 0x2d}); dword(0x108); break;
            case 4: bytes({0x66, 0x8b, 0x05}); dword(0x110); break;
            case 5: bytes({0x88, 0x25}); dword(0x114); break;
            case 6: bytes({0x8d, 0x6e, 1}); break;
            case 7: bytes({0x0f, 0x28, 0xc1}); break;
            }
        }
        entries.push_back((U32)code.size());
        if (shape == 0) { bytes({(U8)(0x70 + cc), 5, 0xb9}); dword(0x12345678); }
        else if (shape == 1) bytes({0x0f, (U8)(0x90 + cc), 0xc1});
        else bytes({0x0f, (U8)(0x40 + cc), 0xce}); // CMOVcc ECX,ESI
        if (live) {
            bytes({0x9c, 0x5b});
            if (form >= 5) { bytes({0x81, 0xe3}); dword(~(U32)AF); } // TEST leaves AF undefined.
        }
        bytes({0x39, 0xff, 0xcd, 0x97});
        auto reset = [&](U32 eip, U32 incoming, bool lazy = false) {
            U32 values[] = {input[0], 0xaaaaaa00, input[1], 0x76543210, 4096, 0x12345678, 0x87654321, 0x13579bdf};
            for (U32 r = 0; r < 8; ++r) {
                cpu->reg[r].u32 = values[r];
                cpu->xmm[r].pi.u64[0] = 0x123456789abcdef0ull + r;
                cpu->xmm[r].pi.u64[1] = 0xabcdef0123456789ull + r;
            }
            ctx.memory->writed(TEST_HEAP_ADDRESS + 0x100, input[0]);
            ctx.memory->writed(TEST_HEAP_ADDRESS + 0x104, input[1]);
            ctx.memory->writed(TEST_HEAP_ADDRESS + 0x108, 0);
            ctx.memory->writed(TEST_HEAP_ADDRESS + 0x110, 0xabcd1234);
            ctx.memory->writed(TEST_HEAP_ADDRESS + 0x114, 0);
            cpu->setFlags(incoming, FMASK_ALL); cpu->lazyFlagType = FLAGS_NONE;
            if (lazy) {
                cpu->dst.u32 = 0xffffffff; cpu->src.u32 = 1; cpu->result.u32 = 0; cpu->lazyFlagType = FLAGS_ADD32;
            }
            cpu->eip.u32 = eip; cpu->nextOp = nullptr; cpu->yield = false;
        };
        auto setup = [&]() { testNewInstruction(0); for (U8 b : code) testPushCode8(b); reset(0, CF | OF); };
        setup(); execute(false); State expected = capture();
        U32 ignored; U32 flags = directArithmeticFlags(form < 5 ? DirectArithmeticTestOp::Sub : DirectArithmeticTestOp::And,
            input[0], input[1], ignored);
        bool take = directArithmeticCondition(cc, flags);
        U32 output = shape == 0 ? (take ? 0xaaaaaa00 : 0x12345678) : shape == 1 ? (0xaaaaaa00 | (take ? 1 : 0)) : take ? 0x87654321 : 0xaaaaaa00;
        if (expected.regs[1] != output) testFail("known flags reference condition form=%u cc=%u", form, cc);
        setup(); auto* head = cpu->getOp(TEST_CODE_ADDRESS, 0); startNewJIT(cpu, TEST_CODE_ADDRESS, head);
        bool probe = form == 0 && input[0] == 0 && input[1] == 1 && cc == 4 && gap == 4 && shape == 1 && live;
        if (probe && !std::getenv("BOXEDWINE_ARM_KNOWN_LAZY_TYPE_SLOW")) {
            for (U32 eip : entries) {
                auto* op = ctx.memory->getDecodedOp(TEST_CODE_ADDRESS + eip);
                if (!op || op->pfnJitCode) testFail("known flags interior was published before execution");
            }
        }
        cpu->nextOp = head; execute(true); State actual = capture();
        if (!same(expected, actual)) testFail("known flags form=%u lhs=%x rhs=%x cc=%u gap=%u shape=%u live=%u", form, input[0], input[1], cc, gap, shape, live);
        if (probe) {
            // Enter every proof-carrying interior with unrelated incoming flags.
            for (U32 entry : entries) for (U32 incoming : {0u, (U32)ZF, (U32)(CF | SF | OF)}) for (bool lazy : {false, true}) {
                reset(entry, incoming, lazy); execute(false); expected = capture();
                reset(entry, incoming, lazy); execute(true); actual = capture();
                if (!same(expected, actual)) testFail("known flags independent entry=%u incoming=%x lazy=%u", entry, incoming, lazy);
            }
            ctx.memory->writeb(TEST_CODE_ADDRESS, 0x03); // Replace CMP with ADD and invalidate the proof.
            reset(0, CF | OF); execute(false); expected = capture();
            reset(0, CF | OF); execute(true); actual = capture();
            if (!same(expected, actual)) testFail("known flags producer invalidation");
        }
    }
    // A forward edge can bypass the comparison but enter its following MOV.
    for (U8 cc = 0; cc < 16; ++cc) {
        testNewInstruction(0);
        auto bytes = [](std::initializer_list<U8> values) { for (U8 b : values) testPushCode8(b); };
        bytes({0x85, 0xff, 0x74, 2, 0x39, 0xd0, 0x89, 0xf3, 0x8b, 0x2d}); testPushCode32(0x110);
        bytes({0x0f, (U8)(0x90 + cc), 0xc1, 0x9c, 0x5b, 0x39, 0xff, 0xcd, 0x97});
        ctx.memory->writed(TEST_HEAP_ADDRESS + 0x110, 0x12345678);
        auto reset = [&](U32 selector) {
            for (U32 r = 0; r < 8; ++r) cpu->reg[r].u32 = r + 1;
            cpu->reg[0].u32 = 0x80000000; cpu->reg[2].u32 = 1;
            cpu->reg[4].u32 = 4096; cpu->reg[7].u32 = selector;
            cpu->setFlags(0, FMASK_ALL); cpu->lazyFlagType = FLAGS_NONE;
            cpu->eip.u32 = 0; cpu->nextOp = nullptr; cpu->yield = false;
        };
        auto* head = cpu->getOp(TEST_CODE_ADDRESS, 0); startNewJIT(cpu, TEST_CODE_ADDRESS, head);
        for (U32 selector : {1u, 0u, 1u}) {
            reset(selector); execute(false); State expected = capture();
            reset(selector); execute(true); State actual = capture();
            if (!same(expected, actual)) testFail("known flags branch bypass cc=%u selector=%u", cc, selector);
        }
    }
#endif
}

void testArmKnownLazyFlagFaults() {
#if defined(BOXEDWINE_JIT_ARMV8) && defined(BOXEDWINE_HOST_EXCEPTIONS)
    extern void startNewJIT(CPU*, U32, DecodedOp*);
    auto& ctx = testContext(); CPU* cpu = ctx.cpu;
    const U32 groupBytes = ramPageLinearMemoryPageCount() * K_PAGE_SIZE;
    constexpr U32 target = 0x24000000;
    auto bytes = [](std::initializer_list<U8> values) { for (U8 b : values) testPushCode8(b); };
    for (bool logical : {false, true}) for (U8 cc = 0; cc < 16; ++cc)
    for (bool store : {false, true}) for (U32 width : {1u, 2u, 4u})
    for (U32 offset : {0u, 1u, K_PAGE_SIZE - 1u, groupBytes - 1u})
    for (U32 tier : {0u, (U32)LINEAR_MEMORY_RECOMPILE_FAULTS, (U32)MAX_OP_EXCEPTION_COUNT})
    for (U32 incoming : {0xffffffffu, 0u, (U32)FMASK_TEST, (U32)SF}) {
        const bool signal = incoming != 0xffffffffu;
        testNewInstruction(CF | OF);
        ctx.memory->mmap(ctx.thread, target, groupBytes * 2, K_PROT_READ | K_PROT_WRITE,
            K_MAP_FIXED | K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0);
        if (signal) ctx.memory->writed(target + offset, 0xaabbccdd);
        ctx.memory->writed(TEST_HEAP_ADDRESS + 0x108, 0);
        cpu->reg[0].u32 = 0x80000000; cpu->reg[2].u32 = 1;
        cpu->reg[1].u32 = 0x12345600; cpu->reg[5].u32 = 0x98765432;
        cpu->reg[6].u32 = target + offset - TEST_HEAP_ADDRESS;
        bytes({0xff, 0x05}); testPushCode32(0x108); // Count the completed prefix.
        bytes({(U8)(logical ? 0x85 : 0x39), 0xd0}); // TEST/CMP EAX,EDX.
        bytes({0xb8}); testPushCode32(0x12345678);
        const U32 access = ctx.codeIp;
        if (width == 2) bytes({0x66});
        if (store) bytes({(U8)(width == 1 ? 0x88 : 0x89), 0x06});
        else bytes({(U8)(width == 1 ? 0x8a : 0x8b), (U8)(width == 1 ? 0x1e : 0x2e)}); // BL/BP/EBP.
        bytes({0x89, 0xfa, 0x0f, (U8)(0x90 + cc), 0xc1, 0xcd, 0x97});
        const U32 handler = TEST_CODE_ADDRESS + 0x6000;
        ctx.codeIp = handler; bytes({0xcd, 0x97});
        auto savedAction = ctx.process->sigActions[K_SIGSEGV];
        auto& action = ctx.process->sigActions[K_SIGSEGV];
        action.reset(); action.flags = K_SA_SIGINFO; action.handlerAndSigAction = handler;
        auto* head = cpu->getOp(TEST_CODE_ADDRESS, 0);
        auto* accessOp = ctx.memory->getDecodedOp(access);
        if (!accessOp) { testFail("known flags fault operand not decoded"); return; }
        accessOp->exceptionCount = tier;
        startNewJIT(cpu, TEST_CODE_ADDRESS, head);
        const U32 protectedPage = (target + offset + width - 1) & ~K_PAGE_MASK;
        if (signal) ctx.memory->mprotect(ctx.thread, protectedPage, K_PAGE_SIZE, store ? K_PROT_READ : 0);
        U32 ignored;
        U32 flags = directArithmeticFlags(logical ? DirectArithmeticTestOp::And : DirectArithmeticTestOp::Sub,
            0x80000000, 1, ignored);
        const U32 mask = logical ? FMASK_TEST & ~AF : FMASK_TEST;
        testRunCPU();
        if (signal) {
            const U32 uc = action.sigInfo[0] == K_SIGSEGV ? cpu->reg[2].u32 : 0;
            if (!uc || ctx.memory->readd(uc + 0x4c) != access - TEST_CODE_ADDRESS ||
                (ctx.memory->readd(uc + 0x54) & mask) != (flags & mask) ||
                ctx.memory->readd(uc + 0x40) != 0x12345678) {
                testFail("known flags fault state logical=%u cc=%u store=%u width=%u offset=%x tier=%u", logical, cc, store, width, offset, tier);
            } else {
                U32 savedFlags = ctx.memory->readd(uc + 0x54);
                ctx.memory->writed(uc + 0x54, (savedFlags & ~FMASK_TEST) | incoming);
                ctx.memory->mprotect(ctx.thread, protectedPage, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
                if (cpu->pop32() != SIG_RETURN_ADDRESS) testFail("known flags signal return address");
                onExitSignal(cpu, nullptr); cpu->nextOp = nullptr;
                testRunCPU(); flags = incoming;
            }
        } else if (action.sigInfo[0]) testFail("known flags first-touch raised a signal");
        ctx.memory->mprotect(ctx.thread, protectedPage, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
        cpu->fillFlags();
        if (cpu->reg[1].u32 != (0x12345600 | (directArithmeticCondition(cc, flags) ? 1 : 0)) ||
            (cpu->flags & mask) != (flags & mask))
            testFail("known flags recovery result logical=%u cc=%u store=%u width=%u offset=%x tier=%u incoming=%x", logical, cc, store, width, offset, tier, incoming);
        if (ctx.memory->readd(TEST_HEAP_ADDRESS + 0x108) != 1) testFail("known flags replayed the completed prefix");
        if (store) {
            const U32 value = width == 1 ? ctx.memory->readb(target + offset) : width == 2 ? ctx.memory->readw(target + offset) : ctx.memory->readd(target + offset);
            const U32 expected = width == 1 ? 0x78 : width == 2 ? 0x5678 : 0x12345678;
            if (value != expected) testFail("known flags recovered store value");
        }
        ctx.process->sigActions[K_SIGSEGV] = savedAction;
        ctx.memory->unmap(target, groupBytes * 2);
    }
#endif
}

void testJitDirectArithmeticFlags() {
    testArmConditionalCompare();
    testJitFlagResultRegions();
    testJitMultiplyOverflowState();
#ifdef BOXEDWINE_JIT_ARMV8
    testArmSseCompareConsumers();
    testArmMinMaxSelection();
    testArmSseMemoryFaultConsumers();
#endif
#ifdef BOXEDWINE_JIT
    testLogicalConditionsWithLiveFlags();
    testParityConsumers();
#endif
#ifdef BOXEDWINE_JIT_ARMV8
    testArmNarrowArithmetic();
#endif
#if defined(BOXEDWINE_JIT_ARMV8)
    testArmFlagWindowEntries();
    testArmDirectFlagConsumers();
#endif
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
    runDirectArithmeticByteMoveCases();
    runDirectArithmeticInterveningMemoryCase(false, true);
    runDirectArithmeticInterveningMemoryCase(false, false);
    runDirectArithmeticInterveningMemoryCase(true, false);
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

// Used by the x86 JIT metadata checks.
[[maybe_unused]] bool directIncDecConditionUsesCF(U8 condition) {
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
    [[maybe_unused]] U32 setOffset = observer == DirectIncDecCFObserver::PushF ? 5 : 4;
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

#if defined(BOXEDWINE_JIT_ARMV8)
static void testArmDirectIncDecPreservedCarry() {
    auto& ctx = testContext();
    CPU* cpu = ctx.cpu;
    auto bytes = [](std::initializer_list<U8> code) { for (U8 b : code) testPushCode8(b); };
    for (bool decrement : {false, true}) for (U32 value : DIRECT_INC_DEC_CASES)
    for (U32 source = 0; source < 3; ++source) for (bool carry : {false, true})
    for (U8 condition = 0; condition < 16; ++condition) for (bool liveOverflow : {false, true}) {
        testNewInstruction(carry ? CF : 0);
        cpu->reg[0].u32 = value;
        cpu->reg[1].u32 = 0x12345678;
        cpu->reg[2].u32 = 0x13572468;
        cpu->reg[3].u32 = 0x87654321;
        cpu->reg[6].u32 = source == 1 ? 1 : 0;
        cpu->reg[7].u32 = source == 1 ? (carry ? 0xffffffff : 0) : (carry ? 1 : 0);
        if (source) bytes({(U8)(source == 1 ? 0x01 : 0x39), 0xfe}); // add/cmp esi,edi
        U32 producer = ctx.codeIp;
        bytes({(U8)(decrement ? 0x48 : 0x40)});
        U32 consumer = ctx.codeIp;
        bytes({0x0f, (U8)(0x90 + condition), 0xc3}); // setcc bl
        bytes({0x0f, 0x92, 0xc1}); // setc cl: carry remains live after the first consumer
        if (liveOverflow) bytes({0x0f, 0x90, 0xc2}); // seto dl: requires the new INC/DEC flags
        bytes({0x39, 0xf6}); // cmp esi,esi: all preceding flags are now dead
        testRunCPU();
        U32 expectedResult;
        U32 flags = directArithmeticFlags(decrement ? DirectArithmeticTestOp::Sub : DirectArithmeticTestOp::Add,
            value, 1, expectedResult);
        flags = (flags & ~CF) | (carry ? CF : 0);
        if (cpu->reg[0].u32 != expectedResult || cpu->reg[1].u32 != (0x12345600 | (carry ? 1 : 0)) ||
                cpu->reg[3].u32 != (0x87654300 | (directArithmeticCondition(condition, flags) ? 1 : 0)) ||
                cpu->reg[2].u32 != (liveOverflow ? (0x13572400 | ((flags & OF) ? 1 : 0)) : 0x13572468))
            testFail("ARM direct INC/DEC preserved carry dec=%u value=%x source=%u carry=%u cc=%u liveOF=%u",
                decrement, value, source, carry, condition, liveOverflow);
        auto* p = ctx.memory->getDecodedOp(producer);
        auto* c = ctx.memory->getDecodedOp(consumer);
        bool expectedFusion = !liveOverflow && !directIncDecConditionUsesCF(condition) && condition != 10 && condition != 11 &&
            !std::getenv("BOXEDWINE_JIT_PRESERVED_CF_SLOW");
        if (!p || !c || !p->pfnJitCode || ((c->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) != 0) != expectedFusion)
            testFail("ARM preserved carry fusion selection cc=%u liveOF=%u", condition, liveOverflow);
    }
    for (U32 count : {1u, 2u, 15u, 16u, 17u, 65u, 1024u})
    for (bool carry : {false, true}) for (bool lazy : {false, true}) {
        testNewInstruction(carry ? CF : 0);
        cpu->reg[1].u32 = count;
        cpu->reg[6].u32 = 0;
        cpu->reg[7].u32 = carry ? 1 : 0;
        if (lazy) bytes({0x39, 0xfe}); // cmp esi,edi
        U32 header = ctx.codeIp;
        bytes({0x8d, 0x40, 1, 0x49, 0x75}); // lea eax,[eax+1]; dec ecx; jnz
        testPushCode8((U8)(header - ctx.codeIp - 1));
        bytes({0x0f, 0x92, 0xc3, 0x39, 0xf6}); // setc bl; cmp esi,esi
        testRunCPU();
        if (cpu->reg[0].u32 != count || cpu->reg[1].u32 || cpu->reg[3].u32 != (carry ? 1 : 0))
            testFail("ARM preserved carry loop count=%u carry=%u lazy=%u", count, carry, lazy);
    }
}
#endif

#if defined(BOXEDWINE_JIT_ARMV8)
static void testArmNarrowIncDecFlags() {
    auto& ctx = testContext(); CPU* cpu = ctx.cpu;
    auto bytes = [](std::initializer_list<U8> code) { for (U8 b : code) testPushCode8(b); };
    for (U32 bits : {8u, 16u}) for (U32 reg = 0; reg < 8; ++reg)
    for (bool decrement : {false, true}) for (bool carry : {false, true}) {
        std::vector<U32> values;
        if (bits == 8 && (reg == 0 || reg == 4)) for (U32 i = 0; i < 256; ++i) values.push_back(i);
        else values = {0, 1, 0xf, 0x7f, 0x80, 0xff, 0x7fff, 0x8000, 0xfffe, 0xffff};
        for (U32 input : values) for (U8 condition = 0; condition < 16; ++condition) {
            testNewInstruction(carry ? CF : 0);
            U32 expected[8];
            for (U32 i = 0; i < 8; ++i) expected[i] = cpu->reg[i].u32 = i == 4 ? 4096 : 0x12345678 + i * 0x01010101;
            U32 mask = (1u << bits) - 1, sign = 1u << (bits - 1);
            U32 index = bits == 8 ? reg & 3 : reg, shift = bits == 8 && reg >= 4 ? 8 : 0;
            input &= mask;
            expected[index] = cpu->reg[index].u32 = (expected[index] & ~(mask << shift)) | (input << shift);
            U32 result = (decrement ? input - 1 : input + 1) & mask;
            expected[index] = (expected[index] & ~(mask << shift)) | (result << shift);
            U32 flags = (carry ? CF : 0) | (result ? 0 : ZF) | ((result & sign) ? SF : 0) |
                directArithmeticParityFlag(result) | (((input ^ 1 ^ result) & 0x10) ? AF : 0) |
                ((result == (decrement ? sign - 1 : sign)) ? OF : 0);
            U32 expectedCondition = directArithmeticCondition(condition, flags) ? 1 : 0;
            expected[3] = (expected[3] & ~0xff) | expectedCondition;
            expected[1] = (expected[1] & ~0xff) | (carry ? 1 : 0);
            if (bits == 16) bytes({0x66});
            bytes({(U8)(bits == 16 ? 0xff : 0xfe), (U8)(0xc0 + (decrement ? 8 : 0) + reg)});
            U32 consumer = ctx.codeIp;
            bytes({0x0f, (U8)(0x90 + condition), 0xc3, 0x0f, 0x92, 0xc1, 0x39, 0xf6});
            testRunCPU();
            for (U32 i = 0; i < 8; ++i) if (cpu->reg[i].u32 != expected[i])
                testFail("ARM narrow flags bits=%u reg=%u dec=%u input=%x cf=%u cc=%u changed r%u=%x expected=%x",
                    bits, reg, decrement, input, carry, condition, i, cpu->reg[i].u32, expected[i]);
            bool fuse = armJitHasFlagM() && !directIncDecConditionUsesCF(condition) && condition != 10 && condition != 11 &&
                !std::getenv("BOXEDWINE_ARM_NARROW_FLAGS_SLOW") && !std::getenv("BOXEDWINE_JIT_PRESERVED_CF_SLOW");
            auto* op = ctx.memory->getDecodedOp(consumer);
            if (!op || ((op->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE) != 0) != fuse)
                testFail("ARM narrow fusion selection bits=%u cc=%u", bits, condition);
        }
    }
    for (U32 bits : {8u, 16u}) for (bool high : {false, true}) for (bool increment : {false, true})
    for (bool carry : {false, true}) for (bool lazy : {false, true}) for (U32 count : {1u, 2u, 127u, 128u, 255u}) {
        if (bits == 16 && high) continue;
        testNewInstruction(carry ? CF : 0);
        U32 mask = (1u << bits) - 1, shift = high ? 8 : 0;
        U32 initial = increment ? ((mask + 1 - count) & mask) : count;
        cpu->reg[0].u32 = (0x12348080u & ~(mask << shift)) | (initial << shift);
        cpu->reg[6].u32 = 0; cpu->reg[7].u32 = carry ? 1 : 0;
        if (lazy) bytes({0x39, 0xfe});
        U32 header = ctx.codeIp;
        if (bits == 16) bytes({0x66});
        bytes({(U8)(bits == 16 ? 0xff : 0xfe), (U8)(0xc0 + (increment ? 0 : 8) + (high ? 4 : 0)), 0x75});
        testPushCode8((U8)(header - ctx.codeIp - 1));
        bytes({0x0f, 0x92, 0xc3, 0x39, 0xf6});
        testRunCPU();
        if (cpu->reg[0].u32 != (0x12348080u & ~(mask << shift)) || cpu->reg[3].u32 != (carry ? 1u : 0u))
            testFail("ARM narrow loop bits=%u high=%u inc=%u carry=%u lazy=%u count=%u", bits, high, increment, carry, lazy, count);
    }
}
#endif

void testJitDirectIncDecFlags() {
#if defined(BOXEDWINE_JIT_ARMV8)
    testArmNarrowIncDecFlags();
#endif
#if defined(BOXEDWINE_JIT_ARMV8)
    testArmDirectIncDecPreservedCarry();
#endif
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
    [[maybe_unused]] U32 producerLen = emitDirectCarry(op, shape, rhs);
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

#ifdef BOXEDWINE_JIT_ARMV8
static void testArmCarryChains() {
    auto& ctx = testContext(); CPU* cpu = ctx.cpu;
    constexpr U32 values[] = {0,1,0x7fffffff,0x80000000,0xffffffff,0xfffffffe,0x12345678,0xaaaaaaaa};
    constexpr U8 regs[] = {0,3,6,7,2,5,1};
    auto bytes = [](std::initializer_list<U8> v) { for (U8 b : v) testPushCode8(b); };
    for (bool subtract : {false, true}) for (U32 shape = 0; shape < 3; ++shape)
            for (U32 length : {2u,4u,8u,9u}) for (U32 seed = 0; seed < 64; ++seed)
            for (bool interior : {false,true}) for (bool observe : {false,true}) {
        testNewInstruction((seed & 1) ? CF : 0);
        U32 expected[8];
        for (U32 r=0;r<8;++r) expected[r]=cpu->reg[r].u32=values[(seed+r)%8] ^ (r==3?values[seed/8]:0);
        expected[4]=cpu->reg[4].u32=4096;
        U32 flags=(seed&1)?CF:0, interiorEip=0;
        for (U32 i=0;i<length;++i) {
            U32 dst=regs[i%7], src=shape==1?dst:regs[(i+2)%7];
            U32 rhs=shape==2?values[(seed+i*3)%8]:expected[src];
            if (i==1) interiorEip=ctx.codeIp-TEST_CODE_ADDRESS;
            if (shape==2) { bytes({0x81,(U8)(0xc0+(subtract?(i?0x18:0x28):(i?0x10:0))+dst)}); testPushCode32(rhs); }
            else bytes({(U8)(subtract?(i?0x19:0x29):(i?0x11:0x01)),(U8)(0xc0+src*8+dst)});
            if (!interior || i) flags=directCarryFlags(subtract?DirectCarryTestOp::Sbb:DirectCarryTestOp::Adc,
                expected[dst],rhs,i?(flags&CF?1:0):0,expected[dst]);
        }
        if (observe) bytes({0x9c});
        bytes({0x39,0xc0,0xcd,0x97});
        auto* entry=cpu->getOp(TEST_CODE_ADDRESS,0);startNewJIT(cpu,TEST_CODE_ADDRESS,entry);
        if (interior) cpu->eip.u32=interiorEip;
        testRunCPU();
        if (observe) expected[4]-=4;
        for (U32 r=0;r<8;++r) if (cpu->reg[r].u32!=expected[r])
            testFail("ARM carry chain sub=%u shape=%u length=%u seed=%u entry=%u live=%u r%u=%x expected=%x",
                subtract,shape,length,seed,interior,observe,r,cpu->reg[r].u32,expected[r]);
        if (observe && (ctx.memory->readd(cpu->seg[SS].address+expected[4])&FMASK_TEST)!=flags)
            testFail("ARM carry chain lost outgoing flags sub=%u shape=%u length=%u seed=%u entry=%u",subtract,shape,length,seed,interior);
    }
    // Replacing the producer must invalidate all assumptions about carry.
    testNewInstruction(0); bytes({0x01,0xd0,0x11,0xeb,0x39,0xc0,0xcd,0x97});
    cpu->reg[0].u32=0;cpu->reg[2].u32=1;cpu->reg[3].u32=10;testRunCPU();
    ctx.memory->writeb(TEST_CODE_ADDRESS,0x29); // sub eax,edx followed by adc ebx,ebp
    cpu->reg[0].u32=0;cpu->reg[2].u32=1;cpu->reg[3].u32=10;cpu->eip.u32=0;cpu->nextOp=nullptr;
    testRunCPU();
    if(cpu->reg[0].u32!=0xffffffff||cpu->reg[3].u32!=11)testFail("ARM carry chain invalidation");
}
#endif

#ifdef BOXEDWINE_JIT_ARMV8
static void testArmCarryConsumers() {
    auto& ctx=testContext();CPU* cpu=ctx.cpu;
    constexpr U32 values[]={0,1,0x7fffffff,0x80000000,0xffffffff,0xfffffffe,0x12345678,0xaaaaaaaa};
    auto bytes=[](std::initializer_list<U8> v){for(U8 b:v)testPushCode8(b);};
    for(bool subtract:{false,true})for(U32 seed=0;seed<512;++seed)for(U32 mode=0;mode<4;++mode)for(U8 condition=0;condition<16;++condition){
        testNewInstruction(CF|OF);cpu->fpu.FINIT();cpu->fpu.top=7;
        cpu->reg[0].u32=values[seed%8];cpu->reg[2].u32=values[(seed/8)%8];cpu->reg[3].u32=values[seed/64];cpu->reg[5].u32=values[(seed+3)%8];
        cpu->reg[1].u32=0xabcd0011;cpu->reg[6].u32=42;cpu->reg[7].u32=84;
        U32 low,high,flags=directCarryFlags(subtract?DirectCarryTestOp::Sbb:DirectCarryTestOp::Adc,cpu->reg[0].u32,cpu->reg[2].u32,0,low);
        flags=directCarryFlags(subtract?DirectCarryTestOp::Sbb:DirectCarryTestOp::Adc,cpu->reg[3].u32,cpu->reg[5].u32,(flags&CF)?1:0,high);
        U32 yes=directArithmeticCondition(condition,flags)?1:0, expected=0;
        bytes({0xd9,0xe8,0xd9,0xe8,0xde,0xc1,(U8)(subtract?0x29:0x01),0xd0,(U8)(subtract?0x19:0x11),0xeb});
        if(mode==0){bytes({0x0f,(U8)(0x90+condition),0xc1});expected=0xabcd0000|yes;}
        else if(mode==1){bytes({0x0f,(U8)(0x90+condition),0xc1,0x0f,(U8)(0x90+(condition^4)),0xc5});expected=0xabcd0000|yes|(directArithmeticCondition(condition^4,flags)?0x100:0);}
        else if(mode==2){if(seed&1)bytes({0x66});bytes({0x0f,(U8)(0x40+condition),0xcf});expected=yes?((seed&1)?0xabcd0054:84):0xabcd0011;}
        else {bytes({0x0f,(U8)(0x80+condition)});testPushCode32(TEST_CODE_ADDRESS+0x400-ctx.codeIp-4);bytes({0xb9});testPushCode32(1);expected=yes?2:1;}
        auto finish=[&](){bytes({0x39,0xf6,0xdd,0x1d});testPushCode32(0x100);bytes({0xcd,0x97});};
        finish();
        if(mode==3){ctx.codeIp=TEST_CODE_ADDRESS+0x400;bytes({0xb9});testPushCode32(2);finish();cpu->getOp(TEST_CODE_ADDRESS+0x400,0);}
        testRunCPU();
        if(cpu->reg[0].u32!=low||cpu->reg[3].u32!=high||cpu->reg[1].u32!=expected||cpu->reg[4].u32!=4096||cpu->fpu.top!=7||ctx.memory->readq(TEST_HEAP_ADDRESS+0x100)!=0x4000000000000000ULL)
            testFail("ARM carry consumer sub=%u seed=%u mode=%u condition=%u low=%x/%x high=%x/%x result=%x/%x",subtract,seed,mode,condition,cpu->reg[0].u32,low,cpu->reg[3].u32,high,cpu->reg[1].u32,expected);
    }
}
#endif

void testJitDirectAdcSbbFlags() {
#ifdef BOXEDWINE_JIT_ARMV8
    testArmCarryConsumers();
#endif
#ifdef BOXEDWINE_JIT_ARMV8
    testArmCarryChains();
#endif
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

#ifdef BOXEDWINE_JIT_ARMV8
static void testArmShiftConsumers() {
    auto& ctx = testContext(); CPU* cpu = ctx.cpu;
    auto bytes = [](std::initializer_list<U8> v) { for (U8 b : v) testPushCode8(b); };
    constexpr U32 values[] = {0, 1, 0x7fffffff, 0x80000000, 0x80000001, 0xffffffff, 0x13579bdf, 0xabcdef01};
    for (const auto& operation : DIRECT_SHIFT_OPS) for (U8 count : {0, 1, 2, 7, 16, 31, 32, 33, 255})
    for (U32 value : values) for (U8 first = 0; first < 16; ++first) for (U32 kind = 0; kind < 5; ++kind) {
        const U32 incoming = 2 | CF | PF | OF;
        testNewInstruction(incoming); cpu->fpu.FINIT(); cpu->fpu.top = 7;
        cpu->reg[0].u32 = value; cpu->reg[1].u32 = 0x12345678; cpu->reg[2].u32 = 0x31415926;
        cpu->reg[3].u32 = 0x98765432; cpu->reg[6].u32 = 42; cpu->reg[7].u32 = 0;
        U32 result = value;
        U32 flags = count & 31 ? directShiftFlags(operation.op, value, count & 31, result) : incoming;
        // OF is architecturally undefined for counts greater than one. The
        // legacy lowering remains responsible for any consumers that use it.
        if ((count & 31) > 1 && directShiftConditionUsesOF(first)) continue;
        U8 second = first ^ 1;
        bool a = directArithmeticCondition(first, flags), b = directArithmeticCondition(second, flags);
        bytes({0xd9, 0xe8, 0xd9, 0xe8, 0xde, 0xc1}); // leave cached ST0 dirty
        U32 producer = ctx.codeIp;
        bytes({0xc1, (U8)(0xc0 | operation.group << 3), count});
        U32 consumer = ctx.codeIp;
        if (kind == 0) bytes({0x0f, (U8)(0x90 + first), 0xc3, 0x0f, (U8)(0x90 + second), 0xc7}); // BL/BH alias
        else if (kind == 1) bytes({0x0f, (U8)(0x90 + first), 0xc3, 0x0f, (U8)(0x40 + second), 0xce});
        else if (kind == 2) {
            bytes({0x0f, (U8)(0x90 + first), 0xc3, (U8)(0x70 + second), 5, 0xb9}); testPushCode32(43);
        } else {
            bytes({0x0f, (U8)(0x90 + first), 0xc3, 0x9c, 0x5f});
            if (kind == 4) bytes({0x0f, (U8)(0x90 + second), 0xc1});
        }
        U32 join = ctx.codeIp;
        bytes({0x39, 0xf6, 0xdd, 0x1d}); testPushCode32(0x100); bytes({0xcd, 0x97});
        if (kind == 2) cpu->getOp(join, 0);
        testRunCPU();
        U32 expectedB = kind == 0 ? (0x98760000 | (b ? 0x100 : 0) | (a ? 1 : 0)) : (0x98765400 | (a ? 1 : 0));
        U32 expectedC = kind == 1 ? (b ? 42u : 0x12345678u) : kind == 2 ? (b ? 0x12345678u : 43u) : kind == 4 ? (0x12345600 | (b ? 1 : 0)) : 0x12345678u;
        U32 defined = CF | PF | ZF | SF | ((count & 31) <= 1 ? OF : 0);
        if (cpu->reg[0].u32 != result || cpu->reg[1].u32 != expectedC || cpu->reg[3].u32 != expectedB ||
                cpu->reg[2].u32 != 0x31415926 || cpu->reg[4].u32 != 4096 ||
                (kind >= 3 && ((cpu->reg[7].u32 ^ flags) & defined)) ||
                ctx.memory->readq(TEST_HEAP_ADDRESS + 0x100) != 0x4000000000000000ull)
            testFail("ARM shift consumers op=%s count=%u value=%x first=%u kind=%u", operation.name, count, value, first, kind);
        if (kind == 0 && (count & 31) &&
                ((first != 10 && first != 11) || !std::getenv("BOXEDWINE_ARM_SHIFT_PARITY_SLOW")) &&
                armJitHasFlagM() && !std::getenv("BOXEDWINE_ARM_SHIFT_FLAGS_SLOW") &&
                !std::getenv("BOXEDWINE_JIT_FLAG_CHAIN_SLOW")) {
            auto* op = ctx.memory->getDecodedOp(consumer);
            if (!op || !(op->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE))
                testFail("ARM shift consumer fusion was not selected");
            // Enter the former consumer independently, with different flags.
            cpu->reg[3].u32 = 0x98765432; cpu->flags = 2 | ZF; cpu->lazyFlagType = FLAGS_NONE;
            cpu->eip.u32 = consumer - TEST_CODE_ADDRESS; cpu->nextOp = nullptr;
            testRunCPU();
            U32 firstNew = directArithmeticCondition(first, ZF) ? 1 : 0;
            U32 secondNew = directArithmeticCondition(second, ZF) ? 1 : 0;
            if (cpu->reg[3].u32 != (0x98760000 | (secondNew << 8) | firstNew))
                testFail("ARM shift independent entry reused native flags");
            // Invalidating a producer must not leave its fused interior cached.
            ctx.memory->writeb(producer + 1, 0xc0 | (operation.group == 4 ? 5 : 4) << 3);
            cpu->eip.u32 = producer - TEST_CODE_ADDRESS; cpu->nextOp = nullptr;
            cpu->reg[0].u32 = value; cpu->reg[3].u32 = 0x98765432;
            U32 newResult;
            U32 newFlags = directShiftFlags(operation.group == 4 ? DirectShiftTestOp::Shr : DirectShiftTestOp::Shl, value, count & 31, newResult);
            testRunCPU();
            if (cpu->reg[0].u32 != newResult || (cpu->reg[3].u32 & 255) != (directArithmeticCondition(first, newFlags) ? 1u : 0u))
                testFail("ARM shift invalidation reused old producer");
        }
    }
}
#endif

void testJitDirectShiftFlags() {
#ifdef BOXEDWINE_JIT_ARMV8
    testArmShiftConsumers();
#endif
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

void testSignalReturnRefreshesCodeSegmentSize() {
    constexpr U32 SIGNAL_STACK_TOP = 0x70000000;
    constexpr U32 SIGNAL_HANDLER = 0x12345000;
    constexpr U32 CODE_SELECTOR = 0x557;

    KProcessPtr process = KProcess::create();
    std::unique_ptr<KMemory> memory(KMemory::create(process.get()));
    process->memory = memory.get();
    KThread* thread = process->createThread();
    CPU* cpu = thread->cpu;
    KThread::setCurrentThread(thread);

    struct user_desc* code = thread->getLDT(CODE_SELECTOR >> 3);
    code->entry_number = CODE_SELECTOR >> 3;
    code->base_addr = TEST_CODE_ADDRESS;
    code->limit = 0xffff;
    code->seg_32bit = 0;
    code->contents = 2;
    code->read_exec_only = 0;
    code->seg_not_present = 0;

    if (!cpu->setSegment(CS, CODE_SELECTOR) || cpu->isBig()) {
        testFail("loading 16-bit CS must select 16-bit decoding");
    }

    memory->mmap(thread, TEST_CODE_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    for (U32 address = 0x102; address < 0x10c; address++) {
        memory->writeb(TEST_CODE_ADDRESS + address, 0x90);
    }
    memory->writeb(TEST_CODE_ADDRESS + 0x10c, 0xc3);
    memory->writeb(TEST_CODE_ADDRESS + 0x120, 0xeb);
    memory->writeb(TEST_CODE_ADDRESS + 0x121, 0x0a);
    memory->writeb(TEST_CODE_ADDRESS + 0x12c, 0xc3);
    DecodedOp* cachedOp = cpu->getOp(TEST_CODE_ADDRESS + 0x10c, 0);
    if (!cachedOp || cachedOp->inst != Retn16) {
        testFail("cache 16-bit instruction before changed CS descriptor");
    }
    cachedOp = cpu->getOp(TEST_CODE_ADDRESS + 0x12c, 0);
    if (!cachedOp || cachedOp->inst != Retn16) {
        testFail("cache 16-bit branch target before changed CS descriptor");
    }

    memory->mmap(thread, SIGNAL_STACK_TOP - K_PAGE_SIZE, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    cpu->reg[4].u32 = SIGNAL_STACK_TOP;
    cpu->eip.u32 = 0x102;
    process->sigActions[K_SIGUSR1].handlerAndSigAction = SIGNAL_HANDLER;
    thread->runSignal(K_SIGUSR1, 0, 0);

    // Wine's Win16 bridge changes the saved CS descriptor while handling
    // an exception, then resumes through the signal context with the same
    // selector. The hidden code-segment size must be reloaded as well.
    code->seg_32bit = 1;

    U32 returnAddress = cpu->pop32();
    if (returnAddress != SIG_RETURN_ADDRESS) {
        testFail("signal return callback address for changed CS descriptor");
        return;
    }
    onExitSignal(cpu, nullptr);

    if (cpu->getSegValue(CS) != CODE_SELECTOR || !cpu->isBig()) {
        testFail("signal return must refresh code-segment size");
    }
    cachedOp = memory->getDecodedOp(TEST_CODE_ADDRESS + 0x10c);
    if (!cachedOp || cachedOp->inst != Retn32) {
        testFail("signal return must re-decode code with the new segment size");
    }

    DecodedOp* jumpOp = cpu->getOp(TEST_CODE_ADDRESS + 0x120, 0);
    if (!jumpOp || jumpOp->inst != JmpJb || !jumpOp->data.nextJump ||
        memory->getDecodedOp(TEST_CODE_ADDRESS + 0x12c)) {
        testFail("32-bit branch must discard a cached 16-bit target");
        return;
    }
    DecodedOp* targetOp = cpu->getOp(TEST_CODE_ADDRESS + 0x12c, 0);
    if (!targetOp || targetOp->inst != Retn32 || *jumpOp->data.nextJump != targetOp) {
        testFail("32-bit branch target must use the replacement decoded op");
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

void testArmJitPollExitState() {
#if defined(BOXEDWINE_JIT_ARMV8) && defined(BOXEDWINE_MULTI_THREADED)
    auto& ctx = testContext(); CPU* cpu = ctx.cpu;
    auto bytes = [](std::initializer_list<U8> code) { for (U8 b : code) testPushCode8(b); };
    constexpr U32 target = 0x4000;
    // direct/register/memory calls and jumps, both near returns, and CALL ESP.
    for (U32 kind = 0; kind < 9; ++kind) {
        testNewInstruction(CF); cpu->fpu.FINIT(); cpu->fpu.top = 7;
        cpu->reg[0].u32 = target;
        if (kind == 6) cpu->reg[4].u32 = target;
        if (kind == 4 || kind == 5) cpu->push32(target);
        U32 beforeSp = cpu->reg[4].u32;
        for (U32 lane = 0; lane < 4; ++lane) {
            cpu->xmm[0].pi.u32[lane] = 10 + lane; cpu->xmm[1].pi.u32[lane] = 20 + lane;
        }
        ctx.memory->writed(TEST_HEAP_ADDRESS + 0x100, target);
        bytes({0x46, 0x66, 0x0f, 0xfe, 0xc1, 0xd9, 0xe8, 0xd9, 0xe8, 0xde, 0xc1});
        bool call = kind == 0 || kind == 1 || kind == 6 || kind == 7;
        if (kind == 0 || kind == 2) {
            bytes({(U8)(call ? 0xe8 : 0xe9)}); testPushCode32(TEST_CODE_ADDRESS + target - ctx.codeIp - 4);
        } else if (kind == 1 || kind == 3 || kind == 6) bytes({0xff, (U8)(kind == 6 ? 0xd4 : call ? 0xd0 : 0xe0)});
        else if (kind == 4) bytes({0xc3});
        else if (kind == 5) bytes({0xc2, 12, 0});
        else { bytes({0xff, (U8)(call ? 0x15 : 0x25)}); testPushCode32(0x100); }
        U32 continuation = ctx.codeIp;
        bytes({0xcd, 0x97});
        ctx.codeIp = TEST_CODE_ADDRESS + target;
        bytes({0x47}); // target must not run before the polling exit
        if (call) bytes({0xc3});
        bytes({0xcd, 0x97});
        for (U32 address : {TEST_CODE_ADDRESS + target, continuation, TEST_CODE_ADDRESS}) {
            auto* op = cpu->getOp(address, 0); if (!op->pfnJitCode) startNewJIT(cpu, address, op);
        }
        auto* entry = ctx.memory->getDecodedOp(TEST_CODE_ADDRESS);
        if (!entry || !entry->pfnJitCode) { testFail("ARM poll state setup"); continue; }
        cpu->jitSignalPending.store(1, std::memory_order_release);
        entry->pfn(cpu, entry);
        cpu->jitSignalPending.store(0, std::memory_order_release);
        U32 expectedSp = call ? beforeSp - 4 : beforeSp + (kind == 4 ? 4 : kind == 5 ? 16 : 0);
        cpu->fillFlags();
        if (cpu->eip.u32 != target || cpu->reg[0].u32 != target || cpu->reg[6].u32 != 1 || cpu->reg[7].u32 ||
                cpu->reg[4].u32 != expectedSp || cpu->nextOp || (cpu->flags & FMASK_TEST) != CF ||
                cpu->fpu.top != 6 || cpu->fpu.getF64(cpu->fpu.STV(0)) != 2.0)
            testFail("ARM poll exit state kind=%u eip=%x esi=%u edi=%u esp=%x expected=%x flags=%x top=%u", kind,
                cpu->eip.u32, cpu->reg[6].u32, cpu->reg[7].u32, cpu->reg[4].u32, expectedSp, cpu->flags, cpu->fpu.top);
        for (U32 lane = 0; lane < 4; ++lane) if (cpu->xmm[0].pi.u32[lane] != 30 + 2 * lane)
            testFail("ARM poll exit lost SIMD state");
        testRunCPU();
        if (cpu->reg[6].u32 != 1 || cpu->reg[7].u32 != 1 || cpu->reg[4].u32 != (call ? beforeSp : expectedSp) ||
                cpu->fpu.top != 6 || cpu->fpu.getF64(cpu->fpu.STV(0)) != 2.0)
            testFail("ARM poll restart replayed or lost state kind=%u", kind);
    }
    testNewInstruction(0);
    constexpr U32 loops = 2048;
    for (U32 i = 0; i < loops; ++i) {
        bytes({0xb9}); testPushCode32(1); // mov ecx,1
        bytes({0x49, 0x75, 0xfd}); // dec ecx; jnz dec
    }
    bytes({0x39, 0xc0, 0xcd, 0x97});
    auto* large = cpu->getOp(TEST_CODE_ADDRESS, 0);
    U32 beforeFallbacks = armJitLargePollFallbackCount();
    startNewJIT(cpu, TEST_CODE_ADDRESS, large);
    if (!large->pfnJitCode) testFail("ARM large poll block did not compile");
    if (!std::getenv("BOXEDWINE_ARM_INLINE_POLL") && armJitLargePollFallbackCount() <= beforeFallbacks)
        testFail("ARM large poll block did not exercise the size fallback");
    testRunCPU();
    if (cpu->reg[1].u32 || cpu->reg[4].u32 != 4096) testFail("ARM large poll block lost restart state");

#ifdef BOXEDWINE_HOST_EXCEPTIONS
    if (std::getenv("BOXEDWINE_ARM_MEMORY_EXIT_COLD")) {
        testNewInstruction(0);
        cpu->reg[6].u32=0x100;
        ctx.memory->writed(TEST_HEAP_ADDRESS+0x100,0x12345678);
        // Keep each decoded chain below its 1,000-op cap, then let the JIT
        // join the chains. Otherwise a long straight-line test stops before
        // its cold section can exceed 32 KiB. JNZ +0 has identical successors.
        std::vector<U32> segments;
        for(U32 group=0;group<32;++group) {
            segments.push_back(ctx.codeIp);
            for(U32 i=0;i<128;++i)bytes({0x8b,0x06}); // MOV EAX,[ESI]
            bytes({0x75,0x00});
        }
        bytes({0x39,0xff,0xcd,0x97});
        for(U32 address:segments) {
            for(auto* op=cpu->getOp(address,0);op&&op->inst!=TestEnd&&op->inst!=Done;op=op->next)
                op->exceptionCount=255;
        }
        auto* entry=cpu->getOp(TEST_CODE_ADDRESS,0);
        beforeFallbacks=armJitLargePollFallbackCount();
        startNewJIT(cpu,TEST_CODE_ADDRESS,entry);
        if(!entry->pfnJitCode||armJitLargePollFallbackCount()<=beforeFallbacks)
            testFail("ARM large memory-exit block did not exercise size fallback");
        cpu->nextOp=entry;U32 dispatches=0;
        do {cpu->run();if(++dispatches>4096){testFail("ARM large memory-exit dispatch limit");break;}}
            while(!cpu->nextOp||cpu->nextOp->inst!=TestEnd);
        if(cpu->reg[0].u32!=0x12345678||cpu->reg[6].u32!=0x100||cpu->reg[4].u32!=4096)
            testFail("ARM large memory-exit block lost state");
    }
#endif
#endif
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

#if defined(BOXEDWINE_JIT) && defined(BOXEDWINE_MULTI_THREADED)
static void OPCALL testQueueSignalDuringJitDispatch(CPU* cpu, DecodedOp*) {
    std::thread producer([cpu]() { cpu->thread->queuePendingSignal(K_SIGUSR1); });
    producer.join();
}
#endif

void testJitSignalPendingDispatch() {
#if defined(BOXEDWINE_JIT) && defined(BOXEDWINE_MULTI_THREADED)
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    KThread* thread = context.thread;
    const U64 signalBit = 1ULL << (K_SIGUSR1 - 1);
    const U64 oldSigMask = thread->sigMask;
    const auto oldAction = context.process->sigActions[K_SIGUSR1];

    testNewInstruction(0);
    testPushCode8(0x43); // inc ebx
    testPushCode8(0xcd); testPushCode8(0x97); // TestEnd
    while (context.codeIp < TEST_CODE_ADDRESS + 0x100) {
        testPushCode8(0x90);
    }
    testPushCode8(0x47); // handler: inc edi
    testRunCPU();
    DecodedOp* entry = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
    if (!entry || !entry->pfnJitCode) {
        testFail("signal dispatch test requires a compiled guest block");
        return;
    }
    context.process->sigActions[K_SIGUSR1].handlerAndSigAction = TEST_CODE_ADDRESS + 0x100;
    context.process->sigActions[K_SIGUSR1].flags = 0;
    thread->sigMask = signalBit;
    cpu->jitSignalPending.store(0, std::memory_order_release);

    auto dispatchGuest = [&]() {
        cpu->eip.u32 = 0;
        cpu->nextOp = entry;
        cpu->run();
    };
    dispatchGuest();
    if (cpu->reg[3].u32 != 2 || thread->inSignal) {
        testFail("dispatch without a signal must execute guest code");
    }

    std::thread producer([thread]() { thread->queuePendingSignal(K_SIGUSR1); });
    producer.join();
    dispatchGuest();
    dispatchGuest(); // The pending masked signal must survive a zero-latch dispatch.
    if (cpu->reg[3].u32 != 4 || !(thread->pendingSignals & signalBit) ||
            cpu->jitSignalPending.load(std::memory_order_acquire) || thread->inSignal) {
        testFail("masked signal must remain queued while guest code runs");
    }

    auto checkHandler = [&]() {
        if (cpu->getEipAddress() != TEST_CODE_ADDRESS + 0x100 ||
                thread->inSignal != 1 || (thread->pendingSignals & signalBit)) {
            testFail("pending signal must enter its handler before guest dispatch");
            return;
        }
        cpu->reg[7].u32 = 0;
        cpu->run();
        if (cpu->reg[7].u32 != 1) {
            testFail("pending signal handler must execute");
        }
        if (cpu->pop32() != SIG_RETURN_ADDRESS) {
            testFail("pending signal handler return address");
            return;
        }
        onExitSignal(cpu, nullptr);
    };
    thread->sigMask = 0;
    dispatchGuest(); // Deliver even though the latch was cleared while masked.
    checkHandler();
    if (cpu->reg[3].u32 != 4) {
        testFail("signal delivery must precede the interrupted guest instruction");
    }

    // Publish from another worker after run() has already checked signals.
    // This fixes the interleaving without relying on a timing-sensitive race.
    DecodedOp lateArrival;
    lateArrival.runCount = JIT_RUN_COUNT + 1;
    lateArrival.pfn = testQueueSignalDuringJitDispatch;
    cpu->eip.u32 = 0;
    cpu->nextOp = &lateArrival;
    cpu->run();
    if (!cpu->jitSignalPending.load(std::memory_order_acquire) ||
            !(thread->pendingSignals & signalBit)) {
        testFail("signal arriving during dispatch must stay pending");
    }
    cpu->nextOp = entry;
    cpu->run();
    checkHandler();

    thread->sigMask = oldSigMask;
    context.process->sigActions[K_SIGUSR1] = oldAction;
    cpu->nextOp = nullptr;
#endif
}


void testJitDecodedChainBoundary() {
#ifdef BOXEDWINE_JIT
    auto& ctx = testContext();
    CPU* cpu = ctx.cpu;
    auto bytes = [](std::initializer_list<U8> code) { for (U8 b : code) testPushCode8(b); };
    for (bool fused : {true, false}) for (bool taken : {false, true}) {
        testNewInstruction(taken ? ZF : 0);
        cpu->reg[0].u32 = taken ? 0 : 1;
        cpu->reg[2].u32 = 0;
        cpu->reg[3].u32 = 42;
        cpu->reg[6].u32 = 0;
        if (fused) bytes({0x39, 0xd0}); // cmp eax,edx
        U32 branchAddress = ctx.codeIp;
        bytes({0x0f, 0x84}); testPushCode32(5); // jz to the decoded chain's end
        U32 fallthrough = ctx.codeIp;
        bytes({0x31, 0xdb}); // xor ebx,ebx: flags dead on fall-through
        U32 tailAddress = ctx.codeIp;
        bytes({0x8d, 0x76, 1}); // lea esi,[esi+1]
        U32 target = ctx.codeIp;
        bytes({0x31, 0xd2}); // xor edx,edx: flags dead on taken path
        bytes({0xbf}); testPushCode32(0x12345678);
        bytes({0xcd, 0x97});
        DecodedOp* entry = cpu->getOp(TEST_CODE_ADDRESS, 0);
        cpu->getOp(fallthrough, 0);
        DecodedOp* targetOp = cpu->getOp(target, 0);
        DecodedOp* tail = ctx.memory->getDecodedOp(tailAddress);
        DecodedOp* savedNext = tail->next;
        // The decoder uses this sentinel to cap long normal-core chains.
        // The real guest instruction at this address is independently cached.
        tail->next = DecodedOp::allocDone();
        startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
        if (!entry->pfnJitCode) testFail("JIT did not compile a chain ending at Done");
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
        if (fused && !(ctx.memory->getDecodedOp(branchAddress)->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE))
            testFail("Done boundary test did not exercise a fused branch");
#endif
        testRunCPU();
        if (cpu->reg[3].u32 != (taken ? 42u : 0u) || cpu->reg[6].u32 != (taken ? 0u : 1u) ||
                cpu->reg[7].u32 != 0x12345678 || cpu->reg[4].u32 != 4096)
            testFail("JIT chain boundary lost branch state fused=%u taken=%u", fused, taken);
        if (targetOp == tail->next) testFail("test lost its decoded chain sentinel");
        tail->next = savedNext;
    }
#endif
}

void testNativeJitConditionalSideExits() {
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
    auto& ctx = testContext();
    CPU* cpu = ctx.cpu;
    auto bytes = [](std::initializer_list<U8> code) { for (U8 b : code) testPushCode8(b); };
    auto compileAt = [&](U32 address) {
        DecodedOp* op = cpu->getOp(address, 0);
        if (!op->pfnJitCode) startNewJIT(cpu, address, op);
        if (!op->pfnJitCode) testFail("native side exit failed to compile");
    };
    constexpr U32 target = TEST_CODE_ADDRESS + 0x4000;
    auto branch = [&](U8 condition) {
        bytes({0x0f, (U8)(0x80 + condition)});
        testPushCode32(target - ctx.codeIp - 4);
    };
    auto finish = [&](U32 value, bool liveFlags) {
        if (liveFlags) bytes({0x9c, 0x5f}); // pushfd; pop edi
        else bytes({0x39, 0xc0}); // replace flags on both successors, permitting fusion
        bytes({0xbb}); testPushCode32(value);
        bytes({0xdd, 0x1d}); testPushCode32(0x100); // fstp qword [100h]
        bytes({0xcd, 0x97});
    };
    // All materialized flag combinations, plus arithmetic producers with dead
    // and live outgoing flags. Both paths must preserve dirty GP/SIMD/x87 state.
    for (U32 mode = 0; mode < 3; ++mode) for (U8 condition = 0; condition < 16; ++condition)
    for (U32 sample = 0; sample < (mode ? 8u : 64u); ++sample) {
        U32 flags = 0, ignored;
        const U32 bits[] = {CF, PF, AF, ZF, SF, OF};
        for (U32 i = 0; i < 6; ++i) if (sample & (1u << i)) flags |= bits[i];
        if (mode) flags = directArithmeticFlags(DirectArithmeticTestOp::Sub,
            DIRECT_ARITHMETIC_CASES[sample][0], DIRECT_ARITHMETIC_CASES[sample][1], ignored);
        testNewInstruction(flags);
        cpu->fpu.FINIT();
        cpu->reg[0].u32 = mode ? DIRECT_ARITHMETIC_CASES[sample][0] : 0x12345678;
        cpu->reg[2].u32 = mode ? DIRECT_ARITHMETIC_CASES[sample][1] : 0x87654321;
        for (U32 lane = 0; lane < 4; ++lane) {
            cpu->xmm[0].pi.u32[lane] = 10 + lane;
            cpu->xmm[1].pi.u32[lane] = 20 + lane;
        }
        bytes({0x8d, 0x76, 0x01}); // lea esi,[esi+1]
        bytes({0x66, 0x0f, 0xfe, 0xc1}); // paddd xmm0,xmm1
        bytes({0xd9, 0xe8, 0xd9, 0xe8, 0xde, 0xc1}); // fld1; fld1; faddp
        if (mode) bytes({0x39, 0xd0}); // cmp eax,edx
        U32 branchAddress = ctx.codeIp;
        branch(condition);
        U32 fallthrough = ctx.codeIp;
        finish(10, mode != 1);
        ctx.codeIp = target;
        finish(20, mode != 1);
        if (mode == 1) compileAt(target); // expose both successors to flag liveness
        compileAt(TEST_CODE_ADDRESS);
        DecodedOp* entry = ctx.memory->getDecodedOp(TEST_CODE_ADDRESS);
        DecodedOp* interior = ctx.memory->getDecodedOp(fallthrough);
        if (!interior || interior->blockStart != entry)
            testFail("native side exit did not retain fall-through");
        if (mode == 1 && condition != 10 && condition != 11) {
            DecodedOp* branchOp = ctx.memory->getDecodedOp(branchAddress);
            if (!branchOp || !(branchOp->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE))
                testFail("native side exit did not exercise fused compare/branch");
        }
        testRunCPU(); // modes 0 and 2 exercise an initially uncompiled target
        bool taken = directArithmeticCondition(condition, flags);
        if (cpu->reg[3].u32 != (taken ? 20u : 10u) || cpu->reg[6].u32 != 1 || cpu->reg[4].u32 != 4096 ||
                ctx.memory->readq(TEST_HEAP_ADDRESS + 0x100) != 0x4000000000000000ull || cpu->fpu.top != 0)
            testFail("native side exit state mode=%u condition=%u sample=%u", mode, condition, sample);
        if (mode != 1 && ((cpu->reg[7].u32 ^ flags) & FMASK_TEST))
            testFail("native side exit live flags mode=%u condition=%u sample=%u", mode, condition, sample);
        for (U32 lane = 0; lane < 4; ++lane) if (cpu->xmm[0].pi.u32[lane] != 30 + lane * 2)
            testFail("native side exit lost SIMD state");
    }

    // Cold/warm targets, target-only invalidation, owner invalidation, and an
    // independent entry into the retained fall-through must all use fresh code.
    testNewInstruction(0);
    bytes({0x40, 0x83, 0xfa, 0x00}); // inc eax; cmp edx,0
    branch(4);
    U32 interior = ctx.codeIp;
    bytes({0xbb}); testPushCode32(10); bytes({0xcd, 0x97});
    ctx.codeIp = target;
    bytes({0xbb}); testPushCode32(20); bytes({0xcd, 0x97});
    for (U32 pass = 0; pass < 6; ++pass) {
        if (pass == 3) ctx.memory->writed(target + 1, 30);
        if (pass == 4) ctx.memory->writeb(TEST_CODE_ADDRESS, 0x46); // inc esi
        U32 address = pass == 5 ? interior : TEST_CODE_ADDRESS;
        compileAt(address);
        cpu->reg[0].u32 = cpu->reg[6].u32 = 0;
        cpu->reg[2].u32 = pass == 0 ? 1 : 0;
        cpu->eip.u32 = address - TEST_CODE_ADDRESS;
        cpu->nextOp = nullptr;
        testRunCPU();
        U32 expected = pass == 0 || pass == 5 ? 10 : pass >= 3 ? 30 : 20;
        if (cpu->reg[3].u32 != expected || cpu->reg[0].u32 != (pass < 4 ? 1u : 0u) ||
                cpu->reg[6].u32 != (pass == 4 ? 1u : 0u))
            testFail("native side exit invalidation/interior entry pass=%u", pass);
    }

    // A fault on either successor must report that instruction, retain the
    // completed prefix, and leave the faulting destination untouched.
    for (bool taken : {false, true}) for (bool store : {false, true}) for (bool cross : {false, true}) {
        testNewInstruction(CF | OF);
        cpu->reg[0].u32 = 101;
        cpu->reg[2].u32 = taken ? 0 : 1;
        cpu->reg[6].u32 = cross ? 0x1ffe : 0x2000;
        ctx.memory->writed(TEST_HEAP_ADDRESS + cpu->reg[6].u32, 0x12345678);
        bytes({0x66, 0x0f, 0x6e, 0xc0}); // movd xmm0,eax
        bytes({0x83, 0xfa, 0x00}); // cmp edx,0
        branch(4);
        U32 fault = 0;
        auto emitFault = [&](bool side) {
            bytes({0xbf}); testPushCode32(side ? 20 : 10);
            if (side == taken) fault = ctx.codeIp;
            bytes({(U8)(store ? 0x89 : 0x8b), 0x06}); // mov [esi],eax / mov eax,[esi]
            bytes({0xcd, 0x97});
        };
        emitFault(false); ctx.codeIp = target; emitFault(true);
        U32 handler = TEST_CODE_ADDRESS + 0x6000;
        ctx.codeIp = handler;
        bytes({0xcd, 0x97});
        auto old = ctx.process->sigActions[K_SIGSEGV];
        auto& action = ctx.process->sigActions[K_SIGSEGV];
        action.reset(); action.flags = K_SA_SIGINFO; action.handlerAndSigAction = handler;
        compileAt(TEST_CODE_ADDRESS); compileAt(target);
        ctx.memory->mprotect(ctx.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, store ? K_PROT_READ : 0);
        testRunCPU();
        ctx.memory->mprotect(ctx.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
        U32 uc = action.sigInfo[0] == K_SIGSEGV ? cpu->reg[2].u32 : 0;
        if (!uc || ctx.memory->readd(uc + 0x4c) != fault - TEST_CODE_ADDRESS ||
                ctx.memory->readd(uc + 0x40) != 101 || ctx.memory->readd(uc + 0x24) != (taken ? 20u : 10u) ||
                cpu->xmm[0].pi.u32[0] != 101 || ctx.memory->readd(TEST_HEAP_ADDRESS + (cross ? 0x1ffe : 0x2000)) != 0x12345678)
            testFail("native side exit fault state taken=%u store=%u cross=%u", taken, store, cross);
        if (uc && (ctx.memory->readd(uc + 0x54) & FMASK_TEST) != (taken ? ZF | PF : 0))
            testFail("native side exit fault flags");
        ctx.process->sigActions[K_SIGSEGV] = old;
    }
#endif
}

#if defined(BOXEDWINE_JIT) && !defined(BOXEDWINE_WASM_JIT)
namespace {
    struct NativeJitTestEnvironment {
        const char* name;
        bool present;
        std::string value;
        NativeJitTestEnvironment(const char* n, const char* v): name(n), present(std::getenv(n)!=nullptr), value(present?std::getenv(n):"") { set(v);
        }
        void set(const char* v) {
#ifdef _WIN32
            _putenv_s(name,v?v:"");
#else
            if(v) setenv(name,v,1);
            else unsetenv(name);
#endif
        }
        ~NativeJitTestEnvironment() { set(present?value.c_str():nullptr);
        }
    };
#ifdef BOXEDWINE_JIT_X64
    // The pinned-base policy also changes process-wide JIT entry helpers.
    // Use a fresh guest process, not just fresh blocks, for each setting.
    struct NativeJitTestContext {
        TestContext* previous = &testContext();
        KThread* previousThread = KThread::currentThread();
        TestContext context;
        NativeJitTestContext() {
            createContext(context);
            currentContext = &context;
        }
        ~NativeJitTestContext() {
            previous->failed |= context.failed;
            previous->failures.insert(previous->failures.end(), context.failures.begin(), context.failures.end());
            delete context.thread;
            KThread::setCurrentThread(previousThread);
            currentContext = previous;
            KSystem::eraseProcess(context.process->id);
            context.process.reset();
        }
    };
#endif
}
#endif

void testArmLinearReadAddress() {
#if defined(BOXEDWINE_JIT_ARMV8) && defined(BOXEDWINE_HOST_EXCEPTIONS)
    auto& ctx = testContext();
    CPU* cpu = ctx.cpu;
    const U32 groupBytes = ramPageLinearMemoryPageCount() * K_PAGE_SIZE;
    constexpr U32 counter = TEST_HEAP_ADDRESS + 0x100;
    constexpr U32 freshBase = 0x22004000;
    constexpr U32 input = 0x8976ff80;
    constexpr U32 contextOffsets[] = {0x40, 0x3c, 0x38, 0x34, 0x30, 0x2c, 0x28, 0x24};
    auto bytes = [](std::initializer_list<U8> code) { for (U8 b : code) testPushCode8(b); };
    for (U32 form = 0; form < 7; ++form)
    for (U32 base = 0; base < 8; ++base) for (bool alias : {false, true})
    for (U32 offset : {0u, 1u, K_PAGE_SIZE - 1u, groupBytes - 1u})
    for (U32 tier : {0u, (U32)LINEAR_MEMORY_RECOMPILE_FAULTS, (U32)MAX_OP_EXCEPTION_COUNT})
    for (U32 state = 0; state < 3; ++state) {
        const bool fault = state == 1, fresh = state == 2;
        if (fresh && tier) continue;
        // An inaccessible ESP page cannot also hold a signal frame. The
        // first two offsets leave the frame in the preceding writable page.
        if (fault && base == 4 && offset > 1) continue;
        testNewInstruction(CF | OF | AF);
        U32 savedEsp = cpu->seg[SS].address + cpu->reg[4].u32;
        cpu->seg[DS].address = cpu->seg[SS].address = 0;
        ctx.process->hasSetSeg[DS] = ctx.process->hasSetSeg[SS] = false;
        const U32 address = (fresh ? freshBase : TEST_HEAP_ADDRESS + 0x4000) + offset;
        if (fresh && ctx.memory->mmap(ctx.thread, freshBase, groupBytes * 2, K_PROT_READ | K_PROT_WRITE,
                K_MAP_FIXED | K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0) != freshBase) {
            testFail("linear read fresh mapping failed"); return;
        }
        if (!fresh) ctx.memory->writed(address, input);
        ctx.memory->writed(counter, 0);
        U32 initial[8];
        for (U32 r = 0; r < 8; ++r) initial[r] = 0x12340000 + r * 0x101;
        initial[4] = savedEsp;
        initial[base] = address;
        for (U32 r = 0; r < 8; ++r) cpu->reg[r].u32 = initial[r];
        const U32 dst = alias ? base : (base + 1) % 8;
        bytes({0xff, 0x05}); testPushCode32(counter); // INC: completed prefix, lazy flags, must not repeat.
        const U32 access = ctx.codeIp;
        if (form >= 5) bytes({0x66});
        if (!form) bytes({0x8b});
        else {
            const U8 opcodes[] = {0, 0xb6, 0xbe, 0xb7, 0xbf, 0xb6, 0xbe};
            bytes({0x0f, opcodes[form]});
        }
        bytes({(U8)((base == 5 ? 0x40 : 0) | (dst << 3) | base)});
        if (base == 4) bytes({0x24});
        if (base == 5) bytes({0});
        bytes({0xcd, 0x97});
        const U32 handler = TEST_CODE_ADDRESS + 0x6000;
        ctx.codeIp = handler; bytes({0xcd, 0x97});
        auto savedAction = ctx.process->sigActions[K_SIGSEGV];
        auto& action = ctx.process->sigActions[K_SIGSEGV];
        action.reset(); action.flags = K_SA_SIGINFO; action.handlerAndSigAction = handler;
        auto* entry = cpu->getOp(TEST_CODE_ADDRESS, 0);
        auto* readOp = ctx.memory->getDecodedOp(access);
        if (!readOp) { testFail("linear read not decoded"); return; }
        readOp->exceptionCount = tier;
        startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
        if (!entry->pfnJitCode) testFail("linear read did not compile");
        const U32 width = !form ? 4 : (form == 3 || form == 4) ? 2 : 1;
        const U32 protectedPage = (address + width - 1) & ~K_PAGE_MASK;
        if (fault && ctx.memory->mprotect(ctx.thread, protectedPage, K_PAGE_SIZE, 0)) {
            testFail("linear read protection setup failed"); return;
        }
        testRunCPU();
        if (fault) ctx.memory->mprotect(ctx.thread, protectedPage, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
        if (ctx.memory->readd(counter) != 1) testFail("linear read repeated completed prefix");
        if (fault) {
            U32 uc = action.sigInfo[0] == K_SIGSEGV ? cpu->reg[2].u32 : 0;
            if (!uc || ctx.memory->readd(uc + 0x4c) != access - TEST_CODE_ADDRESS ||
                    (ctx.memory->readd(uc + 0x54) & FMASK_TEST) != CF) {
                testFail("linear read fault eip/flags form=%u base=%u alias=%u offset=%x tier=%u", form, base, alias, offset, tier);
            } else for (U32 r = 0; r < 8; ++r) {
                if (ctx.memory->readd(uc + contextOffsets[r]) != initial[r])
                    testFail("linear read fault changed register %u form=%u base=%u alias=%u offset=%x tier=%u", r, form, base, alias, offset, tier);
            }
        } else {
            U32 value = fresh ? 0 : input;
            if (width == 1) value = (form == 2 || form == 6) ? (U32)(S32)(S8)value : (U8)value;
            if (width == 2) value = form == 4 ? (U32)(S32)(S16)value : (U16)value;
            if (form >= 5) value = (initial[dst] & 0xffff0000) | (value & 0xffff);
            initial[dst] = value;
            cpu->fillFlags();
            if (action.sigInfo[0] || (cpu->flags & FMASK_TEST) != CF)
                testFail("linear read unexpected signal/flags");
            for (U32 r = 0; r < 8; ++r) if (cpu->reg[r].u32 != initial[r])
                testFail("linear read result register %u form=%u base=%u alias=%u offset=%x tier=%u state=%u", r, form, base, alias, offset, tier, state);
        }
        if (!fresh && ctx.memory->readd(address) != input) testFail("linear read modified source memory");
        ctx.process->sigActions[K_SIGSEGV] = savedAction;
        if (fresh) ctx.memory->unmap(freshBase, groupBytes * 2);
    }
#endif
}

void testNativeX64MemoryPolicies() {
#ifdef BOXEDWINE_JIT_X64
    NativeJitTestEnvironment baseline("BOXEDWINE_TEST_JIT_BASELINE", "1");
    NativeJitTestEnvironment fused("BOXEDWINE_TEST_FUSE_MEMORY_ARITH", "0");
    NativeJitTestEnvironment borrowed("BOXEDWINE_TEST_BORROW_READ_ADDRESS", "0");
    NativeJitTestEnvironment pinned("BOXEDWINE_TEST_PIN_LINEAR_BASE", "0");
    auto bytes = [](std::initializer_list<U8> code) { for (U8 b : code) testPushCode8(b); };
    constexpr U32 input = 0x89765432;
    const U8 opcodes[] = {0x03, 0x2b, 0x23, 0x0b, 0x33, 0x8b}; // ADD/SUB/AND/OR/XOR/MOV
    for (U32 mode = 0; mode < 8; ++mode) {
        fused.set(mode & 1 ? "1" : "0");
        borrowed.set(mode & 2 ? "1" : "0");
        pinned.set(mode & 4 ? "1" : "0");
        NativeJitTestContext fresh;
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        auto before = nativeJitPolicyTestCounts();
        for (U32 operation = 0; operation < 6; ++operation)
        for (bool alias : {false, true}) for (U32 offset : {0x2000u, 0x2001u, 0x1ffeu})
        for (bool fault : {false, true}) {
            testNewInstruction(CF | OF);
            cpu->seg[DS].address = 0;
            ctx.process->hasSetSeg[DS] = false;
            U32 address = TEST_HEAP_ADDRESS + offset;
            U32 initial = alias ? address : 0x7fffffff;
            cpu->reg[0].u32 = initial;
            cpu->reg[1].u32 = 0x11223344;
            cpu->reg[6].u32 = address;
            ctx.memory->writed(address, input);
            bytes({0xbf}); testPushCode32(0x10203040); // completed prefix: MOV EDI,imm
            U32 accessEip = ctx.codeIp - TEST_CODE_ADDRESS;
            bytes({opcodes[operation], (U8)(alias ? 0x00 : 0x06)});
            bytes({0x31, 0xc9, 0xcd, 0x97}); // XOR ECX,ECX makes arithmetic flags dead
            U32 handler = TEST_CODE_ADDRESS + 0x6000;
            ctx.codeIp = handler;
            bytes({0xcd, 0x97});
            auto savedAction = ctx.process->sigActions[K_SIGSEGV];
            auto& action = ctx.process->sigActions[K_SIGSEGV];
            action.reset(); action.flags = K_SA_SIGINFO; action.handlerAndSigAction = handler;
            auto* entry = cpu->getOp(TEST_CODE_ADDRESS, 0);
            startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
            if (!entry->pfnJitCode) testFail("x64 memory policy did not compile");
            if (fault && ctx.memory->mprotect(ctx.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, 0))
                testFail("x64 memory policy could not protect operand");
            testRunCPU();
            if (fault) ctx.memory->mprotect(ctx.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
            if (fault) {
                U32 uc = action.sigInfo[0] == K_SIGSEGV ? cpu->reg[2].u32 : 0;
                if (!uc || ctx.memory->readd(uc + 0x4c) != accessEip ||
                    ctx.memory->readd(uc + 0x40) != initial || ctx.memory->readd(uc + 0x3c) != 0x11223344 ||
                    ctx.memory->readd(uc + 0x28) != address || ctx.memory->readd(uc + 0x24) != 0x10203040 ||
                    (ctx.memory->readd(uc + 0x54) & FMASK_TEST) != (CF | OF))
                    testFail("x64 memory policy fault mode=%u op=%u alias=%u offset=%x", mode, operation, alias, offset);
            } else {
                U32 expected[] = {initial + input, initial - input, initial & input, initial | input, initial ^ input, input};
                cpu->fillFlags();
                if (action.sigInfo[0] || cpu->reg[0].u32 != expected[operation] || cpu->reg[1].u32 ||
                    cpu->reg[6].u32 != address || cpu->reg[7].u32 != 0x10203040 ||
                    (cpu->flags & FMASK_TEST) != (ZF | PF))
                    testFail("x64 memory policy result mode=%u op=%u alias=%u offset=%x", mode, operation, alias, offset);
            }
            if (ctx.memory->readd(address) != input) testFail("x64 memory read modified its operand");
            ctx.process->sigActions[K_SIGSEGV] = savedAction;
        }
        auto after = nativeJitPolicyTestCounts();
        if ((after.fusedMemoryReads != before.fusedMemoryReads) != ((mode & 1) != 0))
            testFail("x64 memory policy did not exercise requested fused path mode=%u", mode);
        bool canBorrow = getMemData(ctx.memory)->useLinearMemoryJit() && !ramPageUseLinearMemoryAdjacent();
        if ((after.borrowedReadAddresses != before.borrowedReadAddresses) != ((mode & 2) != 0 && canBorrow))
            testFail("x64 memory policy did not exercise requested address path mode=%u", mode);
    }
#endif
}

void testNativeX64AddressPolicies() {
#ifdef BOXEDWINE_JIT_X64
    NativeJitTestEnvironment baseline("BOXEDWINE_TEST_JIT_BASELINE", "1");
    NativeJitTestEnvironment pinned("BOXEDWINE_TEST_PIN_LINEAR_BASE", "0");
    NativeJitTestEnvironment folded("BOXEDWINE_TEST_FOLD_LINEAR_DISPLACEMENT", "0");
    constexpr U32 input = 0x89765432, displacement = 64, savedFlags = CF | OF | AF;
    auto bytes = [](std::initializer_list<U8> code) { for (U8 b : code) testPushCode8(b); };
    for (U32 mode = 0; mode < 4; ++mode) {
        pinned.set(mode & 1 ? "1" : "0");
        folded.set(mode & 2 ? "1" : "0");
        NativeJitTestContext fresh;
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        const U32 groupBytes = ramPageLinearMemoryPageCount() * K_PAGE_SIZE;
        if (ctx.memory->mmap(ctx.thread, 0, groupBytes, K_PROT_READ | K_PROT_WRITE,
                K_MAP_FIXED | K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0) != 0) {
            testFail("x64 address policy could not map wrapped operand");
            return;
        }
        bool expectPinned = getMemData(ctx.memory)->useLinearMemoryJit();
#ifndef _WIN32
        expectPinned &= (mode & 1) && !ramPageUseLinearMemoryAdjacent();
#endif
#ifdef __linux__
        bool expectFolded = expectPinned && (mode & 2);
#else
        bool expectFolded = false;
#endif
        auto beforeMode = nativeJitPolicyTestCounts();
        for (U32 address : {TEST_HEAP_ADDRESS + 0x2000u, TEST_HEAP_ADDRESS + 0x2001u,
                TEST_HEAP_ADDRESS + 0x1ffeu, 0x20u})
        for (bool alias : {false, true}) for (bool fault : {false, true}) {
            bool wrapped = address < displacement;
            if (wrapped && fault) continue;
            testNewInstruction(savedFlags);
            cpu->seg[DS].address = 0;
            ctx.process->hasSetSeg[DS] = false;
            U32 base = address - displacement;
            U32 initialEax = alias ? base : 0x11223344;
            ctx.memory->writed(address, input);
            bytes({0xbf}); testPushCode32(0x10203040); // completed prefix: MOV EDI,imm
            U32 loadAddress = ctx.codeIp;
            bytes({0x8b, (U8)(alias ? 0x40 : 0x46), displacement}); // MOV EAX,[EAX/ESI+64]
            bytes({0xcd, 0x97});
            U32 handler = TEST_CODE_ADDRESS + 0x6000;
            ctx.codeIp = handler;
            bytes({0xcd, 0x97});
            auto savedAction = ctx.process->sigActions[K_SIGSEGV];
            auto& action = ctx.process->sigActions[K_SIGSEGV];
            action.reset(); action.flags = K_SA_SIGINFO; action.handlerAndSigAction = handler;
            auto before = nativeJitPolicyTestCounts();
            auto* first = cpu->getOp(TEST_CODE_ADDRESS, 0);
            startNewJIT(cpu, TEST_CODE_ADDRESS, first);
            if (!first->pfnJitCode) testFail("x64 address policy did not compile mode=%u", mode);
            if ((nativeJitPolicyTestCounts().foldedLinearReads != before.foldedLinearReads) != expectFolded)
                testFail("x64 address policy selected wrong folded path mode=%u", mode);
            if (fault && ctx.memory->mprotect(ctx.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, 0))
                testFail("x64 address policy could not protect operand");
            // Reuse the block: a folded wrap must retire to a safe wrapped load.
            for (U32 repeat = 0; repeat < (fault ? 1u : 2u); ++repeat) {
                cpu->reg[0].u32 = initialEax;
                cpu->reg[6].u32 = base;
                cpu->lazyFlagType = FLAGS_NONE;
                cpu->setFlags(savedFlags, FMASK_TEST);
                cpu->eip.u32 = 0;
                testRunCPU();
                cpu->fillFlags();
                if (fault) {
                    U32 uc = action.sigInfo[0] == K_SIGSEGV ? cpu->reg[2].u32 : 0;
                    if (!uc || ctx.memory->readd(uc + 0x4c) != loadAddress - TEST_CODE_ADDRESS ||
                        ctx.memory->readd(uc + 0x40) != initialEax || ctx.memory->readd(uc + 0x28) != base ||
                        ctx.memory->readd(uc + 0x24) != 0x10203040 ||
                        (ctx.memory->readd(uc + 0x54) & FMASK_TEST) != savedFlags)
                        testFail("x64 address policy fault mode=%u alias=%u address=%x", mode, alias, address);
                } else if (action.sigInfo[0] || cpu->reg[0].u32 != input || cpu->reg[6].u32 != base ||
                        cpu->reg[7].u32 != 0x10203040 || (cpu->flags & FMASK_TEST) != savedFlags) {
                    testFail("x64 address policy result mode=%u alias=%u address=%x repeat=%u", mode, alias, address, repeat);
                }
            }
            if (fault) ctx.memory->mprotect(ctx.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
            if (ctx.memory->readd(address) != input) testFail("x64 address policy changed its operand");
            if (wrapped && expectFolded) {
                auto* load = ctx.memory->getDecodedOp(loadAddress);
                if (!load || load->foldedLinearReadState() != DecodedOp::LINEAR_READ_WRAPPED)
                    testFail("x64 folded wrap did not select the persistent fallback");
            }
            ctx.process->sigActions[K_SIGSEGV] = savedAction;
        }
        if ((nativeJitPolicyTestCounts().pinnedLinearEntries != beforeMode.pinnedLinearEntries) != expectPinned)
            testFail("x64 address policy selected wrong entry helper mode=%u", mode);
    }
#endif
}

void testNativeX64FpuPolicies() {
#ifdef BOXEDWINE_JIT_X64
    extern void testNativeJitFpuCachePressure();
    NativeJitTestEnvironment stackPair("BOXEDWINE_TEST_STACK_PAIR_TOP", "0");
    NativeJitTestEnvironment forceSpill("BOXEDWINE_TEST_SPILL_FPU_TOP", "0");
    for (bool enabled : {false, true}) {
        stackPair.set(enabled ? "1" : "0");
        NativeJitTestContext fresh;
        auto before = nativeJitPolicyTestCounts();
        // Exercise TOP wrapping, register pressure, independent entry, and
        // guest fault state with both strategies using the existing oracles.
        testNativeJitFpuCachePressure();
        testWasmJitFpuFaultState();
        if ((nativeJitPolicyTestCounts().stackPairTopSpills != before.stackPairTopSpills) != enabled)
            testFail("x64 x87 policy did not exercise requested stack-pair path enabled=%u", enabled);
    }
#endif
}

void testNativeX64CarryPolicies() {
#ifdef BOXEDWINE_JIT_X64
    NativeJitTestEnvironment baseline("BOXEDWINE_TEST_JIT_BASELINE", nullptr);
    NativeJitTestEnvironment policies[] = {
        {"BOXEDWINE_TEST_TRUST_ADJACENT_CARRY", nullptr},
        {"BOXEDWINE_TEST_TRUST_HOST_CARRY", nullptr},
        {"BOXEDWINE_TEST_ZERO_HOST_CARRY", nullptr},
        {"BOXEDWINE_TEST_TRUST_CARRY_MOVES", nullptr},
        {"BOXEDWINE_TEST_NATIVE_CARRY_ARITH", nullptr},
        {"BOXEDWINE_TEST_NATIVE_CARRY_RR_ONLY", nullptr},
        {"BOXEDWINE_TEST_FLAGS_MOVZX", nullptr},
        {"BOXEDWINE_TEST_MOVZX_ADC_ONLY", nullptr},
        {"BOXEDWINE_TEST_MOVZX_CARRY_ONLY", nullptr},
    };
#if defined(__linux__) || defined(_WIN32)
    constexpr bool enabledByDefault = true;
#else
    constexpr bool enabledByDefault = false;
#endif
    auto& ctx = testContext();
    CPU* cpu = ctx.cpu;
    auto bytes = [](std::initializer_list<U8> code) { for (U8 b : code) testPushCode8(b); };
    const U32 inputs[] = {0, 1, 0xf, 0x7fffffff, 0x80000000, 0xffffffff};
    // Exercise host defaults, the fallback, and explicit enablement overriding
    // the fallback. Verify emitted paths as well as guest results and flags.
    for (U32 mode = 0; mode < 3; ++mode) {
        baseline.set(mode ? "1" : nullptr);
        for (auto& policy : policies) policy.set(mode == 2 ? "1" : nullptr);
        bool optimized = mode == 2 || (mode == 0 && enabledByDefault);
        auto before = nativeJitPolicyTestCounts();
        for (U32 a : inputs) for (U32 b : inputs) for (U32 carry = 0; carry < 2; ++carry)
        for (U32 gap = 0; gap < 3; ++gap) {
            testNewInstruction(0);
            cpu->reg[0].u32 = carry ? 0xffffffff : 0;
            cpu->reg[1].u32 = a;
            cpu->reg[2].u32 = b;
            cpu->reg[3].u32 = 1;
            cpu->reg[6].u32 = 0x12345678;
            bytes({0x03, 0xc3}); // ADD EAX,EBX produces the incoming carry
            if (gap == 1) bytes({0x8b, 0xfe}); // MOV EDI,ESI preserves host flags
            if (gap == 2) bytes({0x46}); // INC ESI preserves guest CF but replaces other flags
            bytes({0x13, 0xca}); // ADC ECX,EDX
            testRunCPU();
            U64 sum = (U64)a + b + carry;
            U32 result = (U32)sum;
            U32 flags = (sum >> 32 ? CF : 0) | (!result ? ZF : 0) | (result & 0x80000000 ? SF : 0) |
                (((a ^ b ^ result) & 0x10) ? AF : 0) | ((~(a ^ b) & (a ^ result) & 0x80000000) ? OF : 0);
            U8 parity = (U8)result;
            parity ^= parity >> 4; parity ^= parity >> 2; parity ^= parity >> 1;
            if (!(parity & 1)) flags |= PF;
            cpu->fillFlags();
            if (cpu->reg[1].u32 != result || cpu->reg[0].u32 != (carry ? 0u : 1u) ||
                cpu->reg[2].u32 != b || cpu->reg[6].u32 != 0x12345678u + (gap == 2) ||
                (gap == 1 && cpu->reg[7].u32 != 0x12345678) || (cpu->flags & FMASK_TEST) != flags)
                testFail("x64 carry policy mode=%u a=%x b=%x carry=%u gap=%u result=%x flags=%x expected=%x", mode, a, b, carry, gap,
                    cpu->reg[1].u32, cpu->flags & FMASK_TEST, flags);
        }
        // An entry at ADC or an intervening MOV must consume this entry's guest
        // CF, not host flags left over from an ADD it did not execute.
        for (U32 moves = 0; moves <= 2; ++moves) {
            testNewInstruction(0);
            bytes({0x03, 0xc3}); // ADD EAX,EBX
            std::vector<U32> entries;
            for (U32 move = 0; move < moves; ++move) {
                entries.push_back(ctx.codeIp);
                bytes({0x8b, 0xfe}); // MOV EDI,ESI
            }
            entries.push_back(ctx.codeIp);
            bytes({0x13, 0xca, 0xcd, 0x97}); // ADC ECX,EDX; TestEnd
            DecodedOp* first = cpu->getOp(TEST_CODE_ADDRESS, 0);
            startNewJIT(cpu, TEST_CODE_ADDRESS, first);
            if (!first->pfnJitCode) testFail("x64 carry entry test did not compile mode=%u", mode);
            for (U32 entry : entries) {
                auto* op = ctx.memory->getDecodedOp(entry);
                if (!op) {
                    testFail("x64 carry entry was not decoded mode=%u", mode);
                    continue;
                }
                if (optimized && op->pfnJitCode)
                    testFail("x64 trusted carry interior published as an external entry mode=%u moves=%u entry=%x", mode, moves, entry);
            }
            // Entering an interior may compile a separate, safe block there.
            // Check the original block's entry protection before executing any.
            for (U32 entry : entries) {
                for (U32 carry = 0; carry < 2; ++carry) {
                    cpu->reg[0].u32 = 0x12345678;
                    cpu->reg[1].u32 = 0xffffffff;
                    cpu->reg[2].u32 = 0;
                    cpu->reg[3].u32 = 1;
                    cpu->lazyFlagType = FLAGS_NONE;
                    cpu->setFlags(carry ? CF : 0, FMASK_TEST);
                    cpu->eip.u32 = entry - TEST_CODE_ADDRESS;
                    cpu->nextOp = cpu->getNextOp();
                    do { cpu->run(); } while (!cpu->nextOp || cpu->nextOp->inst != TestEnd);
                    cpu->fillFlags();
                    if (cpu->reg[0].u32 != 0x12345678 || cpu->reg[1].u32 != (carry ? 0u : 0xffffffff) ||
                        (cpu->flags & CF) != (carry ? CF : 0))
                        testFail("x64 carry external entry mode=%u moves=%u entry=%x carry=%u", mode, moves, entry, carry);
                }
            }
        }
        auto after = nativeJitPolicyTestCounts();
        if ((after.nativeCarryArithmetic != before.nativeCarryArithmetic) != optimized ||
            (after.trustedCarry != before.trustedCarry) != optimized)
            testFail("x64 carry policy tests did not select the expected paths mode=%u", mode);
    }
#endif
}

void testNativeJitPerformancePolicy() {
#if defined(BOXEDWINE_JIT) && !defined(BOXEDWINE_WASM_JIT)
    NativeJitTestEnvironment baseline("BOXEDWINE_TEST_JIT_BASELINE", nullptr);
    NativeJitTestEnvironment policy("BOXEDWINE_TEST_POLICY_UNIT", nullptr);
#if defined(BOXEDWINE_JIT_X64) && defined(__linux__)
    constexpr bool enabledByDefault = true;
#else
    constexpr bool enabledByDefault = false;
#endif
    if (nativeJitPerformanceEnabled(policy.name, true) != enabledByDefault ||
            nativeJitPerformanceEnabled(policy.name, false))
        testFail("native JIT tests must use this host's production policy defaults");
    baseline.set("1");
    if (nativeJitPerformanceEnabled(policy.name, true))
        testFail("native JIT baseline must disable production defaults");
    policy.set("1");
    if (!nativeJitPerformanceEnabled(policy.name, false))
        testFail("explicit native JIT policy must override the baseline");
    policy.set("0");
    baseline.set("0");
    if (nativeJitPerformanceEnabled(policy.name, true))
        testFail("explicit zero must disable a native JIT policy");
#endif
}

static void testNativeReturnFirstCsChange(bool adaptiveEnabled, bool earlyProfile, bool targetFirst) {
#if defined(BOXEDWINE_JIT) && !defined(BOXEDWINE_WASM_JIT)
    NativeJitTestEnvironment warmup("BOXEDWINE_TEST_PROFILE_WARMUP","1");
    NativeJitTestEnvironment adaptive("BOXEDWINE_TEST_ADAPTIVE_RETURN",adaptiveEnabled?"1":"0");
    NativeJitTestEnvironment early("BOXEDWINE_TEST_EARLY_RETURN_PROFILE",earlyProfile?"1":"0");
    NativeJitTestEnvironment target("BOXEDWINE_TEST_TRACE_TARGET_FIRST",targetFirst?"1":"0");
    NativeJitTestEnvironment indirect("BOXEDWINE_TEST_INDIRECT_TARGET_SLOW","0");
    NativeJitTestEnvironment returns("BOXEDWINE_TEST_RETURN_TARGET_SLOW","0");
    constexpr U32 CODE=0x500000, STACK=TEST_STACK_ADDRESS-256;
    auto fail=[](const char* message){testFail("native cached entry / first CS transition: %s",message);
        throw 1;
    };
    testNewInstruction(0);
    auto& t=testContext();
    auto* c=t.cpu;
    std::unique_ptr<KThread> earlierThread(t.process->createThread());
    earlierThread->cpu->reset();
    std::unique_ptr<KThread> prefixThreads[2];
    for (auto& thread : prefixThreads) {
        thread.reset(t.process->createThread());
        thread->cpu->reset();
    }
    try {
        for(int i=0;i<6;i++){c->seg[i].address=0;
            t.process->hasSetSeg[i]=false;
        }
        // Compile with no nonzero CS history; transition through the actual CPU setter below.
        if(t.memory->mmap(t.thread,CODE,4096,K_PROT_READ|K_PROT_WRITE|K_PROT_EXEC,K_MAP_FIXED|K_MAP_PRIVATE,-1,0)!=CODE)fail("map");
        t.memory->writeb(CODE,0xc3);
        t.memory->writeb(CODE+16,0xc3);
        const U32 targets[]={CODE+128,CODE+192,CODE+256};
        for(unsigned i=0;i<3;i++){t.memory->writeb(targets[i],0x40+i);
            t.memory->writeb(targets[i]+1,0xcd);
            t.memory->writeb(targets[i]+2,0x97);
        }
        // Leave this prefix cold, adjacent to an independently warmed target.
        t.memory->writeb(targets[0] - 5, 0xba); // MOV EDX,13
        t.memory->writed(targets[0] - 4, 13);
        auto invoke=[&](U32 pc,unsigned which,bool ret=true,U32 carry=CF){c->reg[0].u32=7;
            c->reg[1].u32=11;
            c->reg[3].u32=9;
            c->reg[2].u32=13;
            c->reg[4].u32=STACK;
            c->flags=2|carry;
            c->lazyFlagType=FLAGS_NONE;
            c->eip.u32=pc;
            c->nextOp=nullptr;
            t.memory->writed(STACK,targets[which]);
            for(unsigned step=0;step<4096;step++){c->run();
                if(c->nextOp&&c->nextOp->inst==TestEnd)break;
            }
            if(!c->nextOp||c->nextOp->inst!=TestEnd)fail("bounded execution");
            if(c->reg[4].u32!=STACK+(ret?4:0)||c->eip.u32!=targets[which]+1)fail("single stack pop and target EIP");
            if(c->reg[0].u32!=7+(which==0)||c->reg[1].u32!=11+(which==1)||c->reg[2].u32!=13+(which==2)||c->reg[3].u32!=9)fail("target registers");
            c->fillFlags();
            if((c->flags&CF)!=carry)fail("incoming carry");
        };
        for(unsigned i=0;i<3;i++)for(unsigned n=0;n<128;n++)invoke(targets[i],i,false);
        // A CPU reset before helper creation can later enter already compiled
        // code without compiling a block of its own. INC needs its carry helper.
        earlierThread->cpu->clone(c);
        {
            struct CurrentThread {
                KThread* previous = KThread::currentThread();
                ~CurrentThread() { KThread::setCurrentThread(previous); }
            } restoreThread;
            KThread::setCurrentThread(earlierThread.get());
            c = earlierThread->cpu;
            invoke(targets[0], 0, false);
            if (!c->calculateCF[0]) fail("cached entry must initialize this CPU's carry helpers");
#ifdef BOXEDWINE_HOST_EXCEPTIONS
            if (c->exceptionSigNo) fail("cached entry on earlier CPU must bind carry helpers without a host fault");
#endif
            c = t.cpu;
        }
        for (unsigned which = 0; which < 2; ++which) {
            struct CurrentThread {
                KThread* previous = KThread::currentThread();
                ~CurrentThread() { KThread::setCurrentThread(previous); }
            } restoreThread;
            prefixThreads[which]->cpu->clone(c);
            c = prefixThreads[which]->cpu;
            if (c->calculateCF[0]) fail("reset CPU must begin without carry helpers");
            KThread::setCurrentThread(prefixThreads[which].get());
            c->eip.u32 = targets[0] - 5;
            c->nextOp = nullptr;
            DecodedOp* prefix = c->getNextOp();
            if (!prefix || prefix->pfnJitCode || prefix->next != t.memory->getDecodedOp(targets[0]))
                fail("cold interpreter prefix must link to a cached compiled target");
            // Native test builds can compile on the first visit. Keep this
            // prefix interpreted so helper binding must happen at the cached
            // target entry, rather than as a side effect of compiling it.
            prefix->flags |= OP_FLAG_NO_JIT;
            prefix->runCount = JIT_RUN_COUNT + 1;
            invoke(targets[0] - 5, 0, false, which ? CF : 0);
            if (!c->calculateCF[0]) fail("interpreter fallthrough must initialize this CPU's carry helpers");
#ifdef BOXEDWINE_HOST_EXCEPTIONS
            if (c->exceptionSigNo) fail("interpreter fallthrough must bind carry helpers without a host fault");
#endif
            c = t.cpu;
        }
        invoke(CODE,0);
        auto*r=t.memory->getDecodedOp(CODE);
#ifdef BOXEDWINE_JIT_X64
        if(r->indirectTargetHits!=1||r->pfnJitCode)fail("production warmup first hit");
        if (adaptiveEnabled && earlyProfile) {
            invoke(CODE,1);
            if(r->indirectTargetHits!=13||r->imm!=targets[1]||r->indirectTargetEip!=targets[0]||r->pfnJitCode)fail("early second target");
        } else {
            // Without early profiling, first compile the one-target return;
            // later misses must still adapt correctly when that policy is on.
            for (unsigned n = 0; n < 128; ++n) invoke(CODE,0);
        }
        for(unsigned n=0;n<400;n++)invoke(CODE,adaptiveEnabled?n%2:0);
        const U8 expectedHits = adaptiveEnabled ? 76 : 12;
        if(r->indirectTargetHits!=expectedHits||!r->blockStart||!r->blockStart->pfnJitCode)fail("source compilation");
        void*stable=r->blockStart->pfnJitCode;
        for(unsigned n=0;n<256;n++)invoke(CODE,adaptiveEnabled?n%2:0);
        if(r->indirectTargetHits!=expectedHits||r->blockStart->pfnJitCode!=stable)fail("profile churn");
#else
        // Other native backends share cached-entry and CS-base correctness,
        // but do not implement x64's adaptive return profile.
        if (!r->pfnJitCode) startNewJIT(c, CODE, r);
        if (!r->pfnJitCode) fail("source compilation");
        void* stable = r->pfnJitCode;
#endif
        const U32 shift=0x10000;
        if(t.memory->mmap(t.thread,CODE+shift,4096,K_PROT_READ|K_PROT_WRITE|K_PROT_EXEC,K_MAP_FIXED|K_MAP_PRIVATE,-1,0)!=CODE+shift)fail("shifted target map");
        for(unsigned which=0;which<2;which++){
            U32 linear=targets[which]+shift;
            t.memory->writeb(linear,0xb8);
            t.memory->writed(linear+1,0xdead1234+which);
            t.memory->writeb(linear+5,0xcd);
            t.memory->writeb(linear+6,0x97);
        }
        for(unsigned which=0;which<2;which++){
            c->setSeg(CS,shift,c->seg[CS].value);
            c->reg[0].u32=7;
            c->reg[1].u32=11;
            c->reg[2].u32=13;
            c->reg[3].u32=9;
            c->reg[4].u32=STACK;
            c->flags=2|CF;
            c->lazyFlagType=FLAGS_NONE;
            c->eip.u32=CODE-shift;
            c->nextOp=nullptr;
            t.memory->writed(STACK,targets[which]);
            for(unsigned step=0;step<4096;step++){c->run();
                if(c->nextOp&&c->nextOp->inst==TestEnd)break;
            }
            if(!c->nextOp||c->nextOp->inst!=TestEnd)fail("bounded execution");
            c->fillFlags();
            if(c->reg[0].u32!=0xdead1234+which||c->reg[1].u32!=11||c->reg[2].u32!=13||c->reg[3].u32!=9||c->reg[4].u32!=STACK+4||c->eip.u32!=targets[which]+5||!(c->flags&CF)||r->pfnJitCode!=stable)fail("live CS guard / one pop / state / stable compiled entry");
#ifdef BOXEDWINE_JIT_X64
            if (r->indirectTargetHits != expectedHits) fail("stable return profile after CS change");
#endif
        }
        c->setSeg(CS,0,c->seg[CS].value);
#ifdef BOXEDWINE_JIT_X64
        if (adaptiveEnabled) {
            for (unsigned n = 0; n < 400; ++n) invoke(CODE,n%3);
            if (r->indirectTargetHits != 0xff) fail("third return target must select the generic fallback");
        }
#endif
    } catch(int) {
        c->setSeg(CS,0,c->seg[CS].value);
    }
#endif
}


void testNativeAdaptiveReturnFirstCsChange() {
#ifdef BOXEDWINE_JIT_X64
    for (bool early : {false, true}) for (bool targetFirst : {false, true})
        testNativeReturnFirstCsChange(true, early, targetFirst);
#endif
    testNativeReturnFirstCsChange(false, false, false);
}

#endif
