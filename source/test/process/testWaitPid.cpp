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
#include "kscheduler.h"
#include "ksignal.h"
#include <atomic>
#include <chrono>
#include <thread>

namespace {

constexpr U32 K_WNOHANG = 1;
constexpr U32 K_SYSCALL_PTRACE = 26;
constexpr U32 K_PTRACE_POKEUSER = 6;
constexpr U32 K_PTRACE_CONT = 7;
constexpr U32 K_PTRACE_DETACH = 17;
constexpr U32 K_PTRACE_SINGLESTEP = 9;
constexpr U32 K_PTRACE_ATTACH = 16;
constexpr U32 I386_DEBUGREG_OFFSET = 252;
constexpr U32 STATUS_ADDRESS = TEST_HEAP_ADDRESS;

KProcessPtr createWaitChild(const KProcessPtr& parent, U32 groupId, bool terminated, U32 exitCode = 0) {
    KProcessPtr child = KProcess::create();
    child->parentId = parent->id;
    child->groupId = groupId;
    child->terminated = terminated;
    child->exitCode = exitCode;
    return child;
}

KProcessPtr createClonedThreadProcess(const KProcessPtr& parent, KThread* templateThread, U32 parentId, KThread** outThread) {
    KProcessPtr process = KProcess::create();
    process->memory = KMemory::create(process.get());
    process->memory->clone(parent->memory, false);

    process->groupId = parent->groupId;
    process->userId = parent->userId;
    process->effectiveUserId = parent->effectiveUserId;
    process->effectiveGroupId = parent->effectiveGroupId;
    process->currentDirectory = parent->currentDirectory;
    process->brkEnd = parent->brkEnd;
    std::copy(parent->sigActions, parent->sigActions + MAX_SIG_ACTIONS, process->sigActions);
    process->commandLine = parent->commandLine;
    process->exe = parent->exe;
    process->name = parent->name;
    process->path = parent->path;
    process->umaskValue = parent->umaskValue;
    process->loaderBaseAddress = parent->loaderBaseAddress;
    process->phdr = parent->phdr;
    process->phnum = parent->phnum;
    process->phentsize = parent->phentsize;
    process->entry = parent->entry;
    for (U32 i = 0; i < 8; i++) {
        process->hasSetSeg[i] = parent->hasSetSeg[i];
    }
    process->hasSetStackMask = parent->hasSetStackMask;
    process->parentId = parentId;

    *outThread = process->createThread();
    (*outThread)->clone(templateThread);
    return process;
}

void expectWaitResult(const char* label, U32 actual, U32 expected) {
    if (actual != expected) {
        testFail("%s expected %d (0x%X), got %d (0x%X)", label, expected, expected, actual, actual);
    }
}

void expectSignalBitClear(const char* label, U64 value, U32 signal) {
    U64 bit = 1ULL << (signal - 1);

    if (value & bit) {
        testFail("%s left signal %u pending", label, signal);
    }
}

void expectSignalBitSet(const char* label, U64 value, U32 signal) {
    U64 bit = 1ULL << (signal - 1);

    if (!(value & bit)) {
        testFail("%s did not leave signal %u pending", label, signal);
    }
}

void eraseIfPresent(U32 pid) {
    if (KSystem::getProcess(pid)) {
        KSystem::eraseProcess(pid);
    }
}

U32 runPtrace(TestContext& context, U32 request, U32 pid, U32 addr, U32 data) {
    context.cpu->reg[0].u32 = K_SYSCALL_PTRACE; // eax
    context.cpu->reg[3].u32 = request; // ebx
    context.cpu->reg[1].u32 = pid; // ecx
    context.cpu->reg[2].u32 = addr; // edx
    context.cpu->reg[6].u32 = data; // esi
    context.cpu->eip.u32 = 0;
    ksyscall(context.cpu, 2);
    return context.cpu->reg[0].u32;
}

} // namespace

void testWaitPid() {
    TestContext& context = testContext();
    KThread* thread = context.thread;
    KProcessPtr parent = context.process;
    U32 oldGroupId = parent->groupId;
    KSigAction oldSigTrap = parent->sigActions[K_SIGTRAP];

    parent->groupId = parent->id;

    KProcessPtr nonChild = KProcess::create();
    nonChild->parentId = parent->id + 0x1000;
    nonChild->terminated = true;
    expectWaitResult("waitpid(pid) rejects non-child", KSystem::waitpid(thread, nonChild->id, 0, K_WNOHANG), (U32)-K_ECHILD);
    if (!KSystem::getProcess(nonChild->id)) {
        testFail("waitpid(pid) reaped a non-child process");
    }
    eraseIfPresent(nonChild->id);

    KProcessPtr runningChild = createWaitChild(parent, parent->groupId, false);
    expectWaitResult("waitpid(pid, K_WNOHANG) reports running child", KSystem::waitpid(thread, runningChild->id, 0, K_WNOHANG), 0);
    eraseIfPresent(runningChild->id);

    KProcessPtr exitedChild = createWaitChild(parent, parent->groupId, true, 7);
    context.memory->writed(STATUS_ADDRESS, 0);
    expectWaitResult("waitpid(pid) reaps own child", KSystem::waitpid(thread, exitedChild->id, STATUS_ADDRESS, 0), exitedChild->id);
    expectWaitResult("waitpid(pid) writes exit status", context.memory->readd(STATUS_ADDRESS), 7 << 8);
    if (KSystem::getProcess(exitedChild->id)) {
        testFail("waitpid(pid) left reaped child in process table");
        eraseIfPresent(exitedChild->id);
    }

    KProcessPtr otherGroupChild = createWaitChild(parent, parent->groupId + 1, true, 3);
    expectWaitResult("waitpid(0) ignores child in another process group", KSystem::waitpid(thread, 0, 0, K_WNOHANG), (U32)-K_ECHILD);
    eraseIfPresent(otherGroupChild->id);

    U32 targetGroupId = parent->groupId + 2;
    KProcessPtr groupChild = createWaitChild(parent, targetGroupId, true, 4);
    expectWaitResult("waitpid(-pgid) reaps matching child group", KSystem::waitpid(thread, -(S32)targetGroupId, 0, K_WNOHANG), groupChild->id);

    KProcessPtr anyChild = createWaitChild(parent, parent->groupId + 3, true, 5);
    expectWaitResult("waitpid(-1) reaps any child", KSystem::waitpid(thread, -1, 0, K_WNOHANG), anyChild->id);

    KThread* tracedThread = parent->createThread();
    tracedThread->ptraceStopPending = true;
    context.memory->writed(STATUS_ADDRESS, 0);
    expectWaitResult("waitpid(tid) reports ptrace stop", KSystem::waitpid(thread, tracedThread->id, STATUS_ADDRESS, 0), tracedThread->id);
    expectWaitResult("waitpid(tid) writes ptrace SIGSTOP status", context.memory->readd(STATUS_ADDRESS), (K_SIGSTOP << 8) | 0x7f);
    if (tracedThread->ptraceStopPending) {
        testFail("waitpid(tid) did not consume ptrace stop");
    }
    if (!parent->getThreadById(tracedThread->id)) {
        testFail("waitpid(tid) reaped ptrace-stopped thread");
    }
    parent->deleteThread(tracedThread);

    KThread* stepThread = parent->createThread();
    stepThread->clone(thread);
    stepThread->cpu->reg[4].u32 = 4096;
    stepThread->ptraceStopPending = true;
    context.memory->writed(STATUS_ADDRESS, 0);
    expectWaitResult("waitpid(tid) reports ptrace stop before singlestep", KSystem::waitpid(thread, stepThread->id, STATUS_ADDRESS, 0), stepThread->id);
    expectWaitResult("waitpid(tid) writes initial ptrace SIGSTOP status", context.memory->readd(STATUS_ADDRESS), (K_SIGSTOP << 8) | 0x7f);

    expectWaitResult("ptrace(PTRACE_SINGLESTEP) returns success", runPtrace(context, K_PTRACE_SINGLESTEP, stepThread->id, 0, 0), 0);
    if (!(stepThread->cpu->flags & TF)) {
        testFail("ptrace(PTRACE_SINGLESTEP) did not arm trace flag");
    }

    stepThread->process->sigActions[K_SIGTRAP].handlerAndSigAction = TEST_CODE_ADDRESS + 1;
    stepThread->signalDebugTrap(2, 0x4000);
    context.memory->writed(STATUS_ADDRESS, 0);
    expectWaitResult("waitpid(tid) reports ptrace singlestep trap", KSystem::waitpid(thread, stepThread->id, STATUS_ADDRESS, K_WNOHANG), stepThread->id);
    expectWaitResult("waitpid(tid) writes ptrace SIGTRAP status", context.memory->readd(STATUS_ADDRESS), (K_SIGTRAP << 8) | 0x7f);
    parent->deleteThread(stepThread);

    KThread* hardwareThread = parent->createThread();
    hardwareThread->clone(thread);
    hardwareThread->cpu->reg[4].u32 = 4096;
    expectWaitResult("ptrace(PTRACE_ATTACH) returns success", runPtrace(context, K_PTRACE_ATTACH, hardwareThread->id, 0, 0), 0);
    context.memory->writed(STATUS_ADDRESS, 0);
    expectWaitResult("waitpid(tid) reports ptrace attach stop", KSystem::waitpid(thread, hardwareThread->id, STATUS_ADDRESS, 0), hardwareThread->id);
    expectWaitResult("waitpid(tid) writes attach SIGSTOP status", context.memory->readd(STATUS_ADDRESS), (K_SIGSTOP << 8) | 0x7f);
    expectWaitResult("ptrace(PTRACE_CONT) returns success", runPtrace(context, K_PTRACE_CONT, hardwareThread->id, 0, 0), 0);
    expectWaitResult("ptrace(PTRACE_POKEUSER DR0) returns success", runPtrace(context, K_PTRACE_POKEUSER, hardwareThread->id, I386_DEBUGREG_OFFSET, TEST_CODE_ADDRESS), 0);
    expectWaitResult("ptrace(PTRACE_POKEUSER DR7) returns success", runPtrace(context, K_PTRACE_POKEUSER, hardwareThread->id, I386_DEBUGREG_OFFSET + 7 * 4, 1), 0);

    parent->sigActions[K_SIGTRAP].handlerAndSigAction = TEST_CODE_ADDRESS + 1;
    memset(parent->sigActions[K_SIGTRAP].sigInfo, 0, sizeof(parent->sigActions[K_SIGTRAP].sigInfo));
    hardwareThread->signalDebugTrap(4, 1);
    context.memory->writed(STATUS_ADDRESS, 0);
    expectWaitResult("waitpid(tid) reports ptrace hardware breakpoint trap", KSystem::waitpid(thread, hardwareThread->id, STATUS_ADDRESS, K_WNOHANG), hardwareThread->id);
    expectWaitResult("waitpid(tid) writes hardware breakpoint SIGTRAP status", context.memory->readd(STATUS_ADDRESS), (K_SIGTRAP << 8) | 0x7f);
    if (parent->sigActions[K_SIGTRAP].sigInfo[0] == K_SIGTRAP) {
        testFail("ptrace hardware breakpoint trap used the in-process SIGTRAP handler");
    }
    parent->deleteThread(hardwareThread);

    KThread* softwareBreakpointThread = parent->createThread();
    softwareBreakpointThread->clone(thread);
    softwareBreakpointThread->cpu->reg[4].u32 = 4096;
    softwareBreakpointThread->cpu->eip.u32 = TEST_CODE_ADDRESS;
    expectWaitResult("ptrace(PTRACE_ATTACH) for software breakpoint returns success", runPtrace(context, K_PTRACE_ATTACH, softwareBreakpointThread->id, 0, 0), 0);
    expectWaitResult("waitpid(tid) reports software breakpoint attach stop", KSystem::waitpid(thread, softwareBreakpointThread->id, STATUS_ADDRESS, 0), softwareBreakpointThread->id);
    expectWaitResult("ptrace(PTRACE_CONT) before software breakpoint returns success", runPtrace(context, K_PTRACE_CONT, softwareBreakpointThread->id, 0, 0), 0);
    parent->sigActions[K_SIGTRAP].handlerAndSigAction = TEST_CODE_ADDRESS + 0x100;
    memset(parent->sigActions[K_SIGTRAP].sigInfo, 0, sizeof(parent->sigActions[K_SIGTRAP].sigInfo));
    softwareBreakpointThread->signalTrap(1);
    context.memory->writed(STATUS_ADDRESS, 0);
    expectWaitResult("waitpid(tid) reports ptrace software breakpoint trap", KSystem::waitpid(thread, softwareBreakpointThread->id, STATUS_ADDRESS, K_WNOHANG), softwareBreakpointThread->id);
    expectWaitResult("waitpid(tid) writes software breakpoint SIGTRAP status", context.memory->readd(STATUS_ADDRESS), (K_SIGTRAP << 8) | 0x7f);
    if (parent->sigActions[K_SIGTRAP].sigInfo[0] == K_SIGTRAP) {
        testFail("ptrace software breakpoint trap used the in-process SIGTRAP handler");
    }
    if (softwareBreakpointThread->cpu->eip.u32 != TEST_CODE_ADDRESS + 1) {
        testFail("ptrace software breakpoint did not leave eip after int3");
    }
    parent->deleteThread(softwareBreakpointThread);

    KThread* attachedTraceeThread = nullptr;
    KProcessPtr attachedTracee = createClonedThreadProcess(parent, thread, parent->id + 0x2000, &attachedTraceeThread);
    expectWaitResult("ptrace(PTRACE_ATTACH) for non-child tracee returns success", runPtrace(context, K_PTRACE_ATTACH, attachedTraceeThread->id, 0, 0), 0);
    KThread* wrongWaiterThread = nullptr;
    KProcessPtr wrongWaiterProcess = createClonedThreadProcess(parent, thread, parent->id + 0x3000, &wrongWaiterThread);
    expectWaitResult("non-tracer waitpid(tid) rejects ptrace stop", KSystem::waitpid(wrongWaiterThread, attachedTraceeThread->id, 0, K_WNOHANG), (U32)-K_ECHILD);
    if (!attachedTraceeThread->ptraceStopPending) {
        testFail("non-tracer waitpid(tid) consumed ptrace stop");
    }
    context.memory->writed(STATUS_ADDRESS, 0);
    expectWaitResult("waitpid(-1) reports attached non-child ptrace stop", KSystem::waitpid(thread, -1, STATUS_ADDRESS, K_WNOHANG), attachedTraceeThread->id);
    expectWaitResult("waitpid(-1) writes attached tracee SIGSTOP status", context.memory->readd(STATUS_ADDRESS), (K_SIGSTOP << 8) | 0x7f);
    expectWaitResult("ptrace(PTRACE_DETACH) for non-child tracee returns success", runPtrace(context, K_PTRACE_DETACH, attachedTraceeThread->id, 0, 0), 0);
    wrongWaiterProcess->deleteThread(wrongWaiterThread);
    attachedTracee->deleteThread(attachedTraceeThread);

    KThread* exitingTraceeThread = nullptr;
    KProcessPtr exitingTracee = createClonedThreadProcess(parent, thread, parent->id + 0x4000, &exitingTraceeThread);
    U32 exitingTraceeThreadId = exitingTraceeThread->id;
    expectWaitResult("ptrace(PTRACE_ATTACH) for exiting non-child tracee returns success", runPtrace(context, K_PTRACE_ATTACH, exitingTraceeThreadId, 0, 0), 0);
    expectWaitResult("waitpid(tid) reports exiting tracee attach stop", KSystem::waitpid(thread, exitingTraceeThreadId, STATUS_ADDRESS, 0), exitingTraceeThreadId);
    expectWaitResult("ptrace(PTRACE_CONT) for exiting non-child tracee returns success", runPtrace(context, K_PTRACE_CONT, exitingTraceeThreadId, 0, 0), 0);
    exitingTracee->terminated = true;
    exitingTracee->exitCode = 9;
    context.memory->writed(STATUS_ADDRESS, 0);
    expectWaitResult("waitpid(tid) reports terminated non-child ptrace tracee", KSystem::waitpid(thread, exitingTraceeThreadId, STATUS_ADDRESS, K_WNOHANG), exitingTraceeThreadId);
    expectWaitResult("waitpid(tid) writes terminated ptrace tracee exit status", context.memory->readd(STATUS_ADDRESS), 9 << 8);
    if (KSystem::getProcess(exitingTracee->id)) {
        testFail("waitpid(tid) left terminated ptrace tracee in process table");
        eraseIfPresent(exitingTracee->id);
    }
    exitingTracee->deleteThread(exitingTraceeThread);

#ifndef BOXEDWINE_MULTI_THREADED
    KThread* stoppedThread = parent->createThread();
    stoppedThread->clone(thread);
    stoppedThread->cpu->eip.u32 = 0;
    context.memory->writeb(TEST_CODE_ADDRESS, 0x90);
    context.memory->writeb(TEST_CODE_ADDRESS + 1, 0xcd);
    context.memory->writeb(TEST_CODE_ADDRESS + 2, 0x97);
    context.memory->clearOpCache();
    stoppedThread->setPtraceStop(K_SIGSTOP);
    runThreadSlice(stoppedThread);
    if (stoppedThread->cpu->eip.u32 != 0) {
        testFail("ptrace-stopped thread executed in single-thread scheduler");
    }
    stoppedThread->resumeFromPtraceStop();
    runThreadSlice(stoppedThread);
    if (stoppedThread->cpu->eip.u32 == 0) {
        testFail("ptrace-resumed thread did not execute in single-thread scheduler");
    }
    parent->deleteThread(stoppedThread);
#endif

    parent->sigActions[K_SIGTRAP] = oldSigTrap;
    parent->groupId = oldGroupId;
}

void testProcessSignalWakesSigwaitMask() {
    TestContext& context = testContext();
    KThread* thread = context.thread;
    KProcessPtr process = context.process;
    const U64 signalBit = 1ULL << (K_SIGUSR1 - 1);
    U64 oldSigMask = thread->sigMask;
    U64 oldSigWaitMask = thread->sigWaitMask;
    U64 oldFoundWaitSignal = thread->foundWaitSignal;
    U64 oldProcessPendingSignals = process->pendingSignals;

    thread->sigMask = signalBit;
    thread->sigWaitMask = signalBit;
    thread->foundWaitSignal = 0;
    process->pendingSignals &= ~signalBit;

    expectWaitResult("process signal to sigwait mask", process->signal(K_SIGUSR1), 0);
    expectWaitResult("process signal found sigwait signal", (U32)thread->foundWaitSignal, K_SIGUSR1);
    expectSignalBitClear("process signal to sigwait mask", process->pendingSignals, K_SIGUSR1);

    thread->sigMask = oldSigMask;
    thread->sigWaitMask = oldSigWaitMask;
    thread->foundWaitSignal = oldFoundWaitSignal;
    process->pendingSignals = oldProcessPendingSignals;
}

void testBlockedThreadSignalStartsHandler() {
#ifdef BOXEDWINE_MULTI_THREADED
    TestContext& context = testContext();
    KProcessPtr process = context.process;
    KThread* thread = context.thread;
    KThread* target = process->createThread();
    KSigAction oldSigInt = process->sigActions[K_SIGINT];
    const U32 signalHandler = TEST_CODE_ADDRESS + 0x300;
    const U64 signalBit = 1ULL << (K_SIGINT - 1);

    target->clone(thread);
    target->cpu->reg[4].u32 = 32 * K_PAGE_SIZE;
    target->pendingSignals &= ~signalBit;
    target->sigMask &= ~signalBit;
    target->startSignal = false;
    target->cpu->eip.u32 = TEST_CODE_ADDRESS;
    process->sigActions[K_SIGINT].handlerAndSigAction = signalHandler;
    process->sigActions[K_SIGINT].flags = 0;

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(target->waitingCondSync);
        target->waitingCond = std::make_shared<BoxedWineCondition>(B("testBlockedThreadSignalStartsHandler"));
    }

    expectWaitResult("signal blocked thread", target->signal(K_SIGINT, false), 0);
    if (!target->startSignal) {
        testFail("signal blocked thread did not request wait interruption");
    }
    expectSignalBitSet("signal blocked thread", target->pendingSignals, K_SIGINT);
    expectWaitResult("signal blocked thread eip before resume", target->cpu->eip.u32, TEST_CODE_ADDRESS);

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(target->waitingCondSync);
        target->waitingCond = nullptr;
    }
    target->startSignal = false;
    if (!target->runSignals()) {
        testFail("resumed blocked thread did not run pending signal");
    }
    expectSignalBitClear("resumed blocked thread", target->pendingSignals, K_SIGINT);
    expectWaitResult("resumed blocked thread handler eip", target->cpu->eip.u32, signalHandler);

    process->sigActions[K_SIGINT] = oldSigInt;
    process->deleteThread(target);
#endif
}

void testMemoryThreadCleanupUsesMemoryMutex() {
#ifdef BOXEDWINE_MULTI_THREADED
    TestContext& context = testContext();
    std::atomic<bool> started(false);
    std::atomic<bool> finished(false);

    BOXEDWINE_MUTEX_LOCK(context.memory->mutex);
    std::thread worker([&]() {
        started.store(true, std::memory_order_release);
        context.memory->threadCleanup(context.thread->id);
        finished.store(true, std::memory_order_release);
    });

    for (U32 i = 0; i < 100 && !started.load(std::memory_order_acquire); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (finished.load(std::memory_order_acquire)) {
        testFail("KMemory::threadCleanup ran while KMemory::mutex was held");
    }
    BOXEDWINE_MUTEX_UNLOCK(context.memory->mutex);
    worker.join();
#endif
}

#endif
