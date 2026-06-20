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
#include "ksignal.h"

namespace {

constexpr U32 K_WNOHANG = 1;
constexpr U32 K_SYSCALL_PTRACE = 26;
constexpr U32 K_PTRACE_POKEUSER = 6;
constexpr U32 K_PTRACE_CONT = 7;
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

void expectWaitResult(const char* label, U32 actual, U32 expected) {
    if (actual != expected) {
        testFail("%s expected %d (0x%X), got %d (0x%X)", label, expected, expected, actual, actual);
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

    parent->sigActions[K_SIGTRAP] = oldSigTrap;
    parent->groupId = oldGroupId;
}

#endif
