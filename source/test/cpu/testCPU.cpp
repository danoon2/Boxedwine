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

#endif
