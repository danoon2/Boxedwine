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

std::mutex testContextCreateMutex;
std::unique_ptr<TestContext> serialContext;
std::vector<std::unique_ptr<TestContext>> parallelContexts;
thread_local TestContext* currentContext = nullptr;
thread_local bool runningParallelTest = false;

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
    cpu->mxcsr = 0x1F80;
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
}

void testPushCode8(int value) {
    TestContext& context = testContext();
    context.memory->writeb(context.codeIp, value);
    context.codeIp++;
}

void testPushCode16(int value) {
    TestContext& context = testContext();
    context.memory->writew(context.codeIp, value);
    context.codeIp += 2;
}

void testPushCode32(int value) {
    TestContext& context = testContext();
    context.memory->writed(context.codeIp, value);
    context.codeIp += 4;
}

void testRunCPU() {
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

#endif
