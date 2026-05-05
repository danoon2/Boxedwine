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

namespace {

constexpr U32 K_WNOHANG = 1;
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

} // namespace

void testWaitPid() {
    TestContext& context = testContext();
    KThread* thread = context.thread;
    KProcessPtr parent = context.process;
    U32 oldGroupId = parent->groupId;

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

    parent->groupId = oldGroupId;
}

#endif
