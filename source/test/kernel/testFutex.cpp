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

#include "../cpu/testCPU.h"

#include <atomic>
#include <chrono>
#include <thread>

namespace {

constexpr U32 TEST_FUTEX_ADDRESS = TEST_HEAP_ADDRESS + 0x980;
constexpr U32 TEST_FUTEX_WAIT = 0;
constexpr U32 TEST_FUTEX_WAKE = 1;
constexpr U32 TEST_FUTEX_PRIVATE_FLAG = 128;
constexpr U32 TEST_FUTEX_VALUE = 0x12345678;

bool waitForFutexWorker(const std::atomic<bool>& done, U32 timeoutMs) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (!done.load(std::memory_order_acquire) && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return done.load(std::memory_order_acquire);
}

} // namespace

void testSharedFutexUnmappedAddressReturnsEFAULT() {
    TestContext& context = testContext();
    constexpr U32 address = 0x61000000;
    if (context.memory->mmap(context.thread, address, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0) != address) {
        testFail("map shared-futex test page");
        return;
    }
    context.memory->writed(address, TEST_FUTEX_VALUE);
    if (context.thread->futex(address, TEST_FUTEX_WAKE, 1, 0, 0, 0, false) != 0) {
        testFail("mapped shared futex wake without waiters must return zero");
    }
    if (context.memory->unmap(address, K_PAGE_SIZE) != 0) {
        testFail("unmap shared-futex test page");
        return;
    }
    for (bool time64 : {false, true}) {
        for (U32 command : {TEST_FUTEX_WAIT, TEST_FUTEX_WAKE}) {
            U32 result = context.thread->futex(address, command, TEST_FUTEX_VALUE,
                0, 0, 0, time64);
            if (result != (U32)-K_EFAULT) {
                testFail("unmapped shared futex command %u time64 %d expected EFAULT, got %d",
                    command, time64, (S32)result);
            }
        }
    }
}

void testTerminatingThreadDoesNotEnterFutexWait() {
#ifdef BOXEDWINE_MULTI_THREADED
    TestContext& context = testContext();
    KThread* mainThread = context.thread;
    KThread* waiter = context.process->createThread();
    std::atomic<bool> done(false);
    std::atomic<U32> result(0);

    context.memory->writed(TEST_FUTEX_ADDRESS, TEST_FUTEX_VALUE);
    waiter->terminating = true;

    std::thread worker([waiter, &done, &result]() {
        KThread::setCurrentThread(waiter);
        result.store(waiter->futex(TEST_FUTEX_ADDRESS, TEST_FUTEX_WAIT | TEST_FUTEX_PRIVATE_FLAG, TEST_FUTEX_VALUE, 0, 0, 0, false), std::memory_order_release);
        done.store(true, std::memory_order_release);
        KThread::setCurrentThread(nullptr);
    });

    if (!waitForFutexWorker(done, 100)) {
        KThread::setCurrentThread(mainThread);
        mainThread->futex(TEST_FUTEX_ADDRESS, TEST_FUTEX_WAKE | TEST_FUTEX_PRIVATE_FLAG, 1, 0, 0, 0, false);
        waitForFutexWorker(done, 1000);
        testFail("terminating thread entered futex wait before returning EINTR");
    }

    worker.join();
    KThread::setCurrentThread(mainThread);

    const U32 expected = -K_EINTR;
    if (result.load(std::memory_order_acquire) != expected) {
        testFail("terminating futex wait expected %d (0x%X), got %d (0x%X)", (S32)expected, expected, (S32)result.load(std::memory_order_acquire), result.load(std::memory_order_acquire));
    }

    context.process->deleteThread(waiter);
#endif
}

#endif
