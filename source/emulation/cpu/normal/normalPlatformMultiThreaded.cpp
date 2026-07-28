/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "knativesystem.h"
#if defined(BOXEDWINE_JIT_ARMV8)
#include "../armv8/jitArmV8CodeGen.h"
#endif
#ifdef BOXEDWINE_JIT
#include "../jit/jitCodeLifecycle.h"
#endif

#if defined(BOXEDWINE_MULTI_THREADED)

std::atomic<int> platformThreadCount = 0;
void platformInitExceptionHandling();

// inline (not noinline): runs after every cpu->run() in the hot loop, so an
// out-of-line call here would cost a call per dispatch (measurably slower on the
// fast JIT). Keeping the try/catch outside the noinline platformThreadRun measured
// ~4 PERF_W95 points faster for multiThreadedJit and is also valid on native hosts.
static inline bool platformThreadShouldStop(CPU* cpu) {
#ifdef __TEST
    if (cpu->nextOp && cpu->nextOp->inst == TestEnd) {
        return true;
    }
#endif
    if (cpu->thread->process->terminated) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->memory->mutex);
        cpu->memory->cleanup();
    }
    return cpu->thread->terminating;
}

static NO_INLINE void platformThreadRun(CPU* cpu) {
    do {
        cpu->run();
        cpu->thread->waitForPtraceResume();
    } while (!platformThreadShouldStop(cpu));
}

static void platformThread(CPU* cpu) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
    platformInitExceptionHandling();
#endif
#if defined(BOXEDWINE_JIT_ARMV8)
    ensureArmV8HardwareTSOForThread();
#endif
    KThread::setCurrentThread(cpu->thread);
    KProcessPtr process = KSystem::getProcess(cpu->thread->process->id);

    cpu->nextOp = cpu->getNextOp();
    if (!cpu->nextOp) {
        cpu->thread->seg_instructionFetch(cpu->getEipAddress(), false);
        cpu->nextOp = cpu->getNextOp();
        if (!cpu->nextOp) {
			kpanic_fmt("Failed to get first op for thread %d of process %d at address %x", cpu->thread->id, process->id, cpu->getEipAddress());
		}
    }
    while (true) {
        try {
            platformThreadRun(cpu);
#ifdef __TEST
            if (cpu->nextOp && cpu->nextOp->inst == TestEnd) {
                return;
            }
#endif
            break;
        } catch (...) {
            if (!cpu->thread->terminating) {
                cpu->nextOp = cpu->getNextOp();
            }
            cpu->thread->waitForPtraceResume();
            if (platformThreadShouldStop(cpu)) {
                break;
            }
        }
    }

    cpu->thread->cleanup();

    platformThreadCount--;
    process->deleteThread(cpu->thread);
    if (platformThreadCount == 0) {
        KSystem::shutingDown = true;
        KNativeSystem::postQuit();
    }
}

static void* platformThreadStart(void* arg) {
    platformThread((CPU*)arg);
    return nullptr;
}

#ifdef __TEST
void initThreadForTesting() {
}

void joinThread(KThread* thread) {
    platformJoinThread(thread);
}
#endif

void scheduleThread(KThread* thread) {
    platformThreadCount++;
    CPU* cpu = thread->cpu;
#ifdef BOXEDWINE_JIT
    jitThreadStartPreparing(cpu);
#endif
    S32 result = platformStartThread(thread, platformThreadStart);
    if (result) {
#ifdef BOXEDWINE_JIT
        jitThreadStartCancelled(cpu);
#endif
        platformThreadCount--;
        kpanic_fmt("platformStartThread failed: %d", result);
        return;
    }
    if (!thread->process->isSystemProcess() && KSystem::cpuAffinityCountForApp) {
        Platform::setCpuAffinityForThread(thread, KSystem::cpuAffinityCountForApp);
    }
}

void terminateOtherThread(const KProcessPtr& process, U32 threadId) {
    KThread* thread = process->getThreadById(threadId);
    if (thread) {
        BOXEDWINE_CONDITION cond;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->waitingCondSync);
            thread->terminating = true;
            cond = thread->waitingCond;
        }

        if (cond) {
            cond->lock();
            cond->signalAll();
            cond->unlock();
        }
    }

    while (true) {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(process->threadRemovedCondition);
        if (!process->getThreadById(threadId)) {
            break;
        }
        BOXEDWINE_CONDITION_WAIT_TIMEOUT(process->threadRemovedCondition, 1000);
    }
}

void terminateCurrentThread(KThread* thread) {
    thread->terminating = true;
}

void unscheduleThread(KThread* thread) {
}

#endif
