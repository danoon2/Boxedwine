/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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

#if defined(BOXEDWINE_MULTI_THREADED)

std::atomic<int> platformThreadCount = 0;
void platformInitExceptionHandling();

// Helper path for the emscripten builds, which compile with global
// -fwasm-exceptions (project/emscripten/makefile), where try{}/catch(...) is a
// near-zero-cost WASM catch_all. The hot worker loop runs in a force-noinline
// platformThreadRun(); platformThread() keeps the catch hoisted above it, and the
// page-fault throw (KThread::seg_*: throw 1/2) unwinds out of platformThreadRun()
// into that catch. This noinline-split structure measured ~4 PERF_W95 points faster
// for multiThreadedJit than an inline per-block try{ cpu->run(); } (75 -> 79), and
// ~neutral for the normal-core multiThreaded.
#ifdef __EMSCRIPTEN__
#if defined(_MSC_VER)
#define BOXEDWINE_NOINLINE __declspec(noinline)
#else
#define BOXEDWINE_NOINLINE __attribute__((noinline))
#endif

// inline (not noinline): runs after every cpu->run() in the hot loop, so an
// out-of-line call here would cost a call per dispatch (measurably slower on the
// fast JIT). Only platformThreadRun is force-noinline (see above).
static inline bool platformThreadShouldStop(CPU* cpu) {
    if (cpu->thread->process->terminated) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->memory->mutex);
        cpu->memory->cleanup();
    }
    return cpu->thread->terminating;
}

static BOXEDWINE_NOINLINE void platformThreadRun(CPU* cpu) {
    do {
        cpu->run();
    } while (!platformThreadShouldStop(cpu));
}
#endif

static void platformThread(CPU* cpu) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
    platformInitExceptionHandling();
#endif
    KThread::setCurrentThread(cpu->thread);
    KProcessPtr process = KSystem::getProcess(cpu->thread->process->id);

    cpu->nextOp = cpu->getNextOp();
    if (!cpu->nextOp) {
        cpu->thread->seg_mapper(cpu->getEipAddress(), true, false, false);
        cpu->nextOp = cpu->getNextOp();
        if (!cpu->nextOp) {
			kpanic_fmt("Failed to get first op for thread %d of process %d at address %x", cpu->thread->id, process->id, cpu->getEipAddress());
		}
    }
#ifdef __EMSCRIPTEN__
    // emscripten: hot loop runs in platformThreadRun() (force-noinline); only a
    // thrown page fault returns control here, where we recover nextOp and re-run
    // the stop check exactly as the per-block loop below would.
    while (true) {
        try {
            platformThreadRun(cpu);
            break;
        } catch (...) {
            if (!cpu->thread->terminating) {
                cpu->nextOp = cpu->getNextOp();
            }
            if (platformThreadShouldStop(cpu)) {
                break;
            }
        }
    }
#else
    while (true) {
        try {
            cpu->run();
        } catch (...) {
            if (!cpu->thread->terminating) {
                cpu->nextOp = cpu->getNextOp();
            }
        }
#ifdef __TEST
        if (cpu->nextOp->inst == TestEnd) {
            return;
        }
        continue;
#else
        if (cpu->thread->process->terminated) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->memory->mutex);
            cpu->memory->cleanup();
        }
        if (cpu->thread->terminating) {
            break;
        }
#endif
    }
#endif

    cpu->thread->cleanup();

    platformThreadCount--;
    process->deleteThread(cpu->thread);
    if (platformThreadCount == 0) {
        KSystem::shutingDown = true;
        KNativeSystem::postQuit();
    }
}

#ifdef __TEST
void initThreadForTesting() {
}

void joinThread(KThread* thread) {
    std::thread* cppThread = (std::thread*)thread->cpu->nativeHandle;
    cppThread->join();
    delete cppThread;
}
#endif

void platformSetThreadDescription(KThread* thread);

void scheduleThread(KThread* thread) {
    platformThreadCount++;
    CPU* cpu = thread->cpu;
#ifdef __TEST
    cpu->nativeHandle = (U64)new std::thread(platformThread, cpu);
#else
    std::thread cppThread = std::thread(platformThread, cpu);
    cpu->nativeHandle = (U64)cppThread.native_handle();
#if defined(_DEBUG) && defined(BOXEDWINE_MSVC)
    platformSetThreadDescription(thread);
#endif
    cppThread.detach();
#endif
    if (!thread->process->isSystemProcess() && KSystem::cpuAffinityCountForApp) {
        Platform::setCpuAffinityForThread(thread, KSystem::cpuAffinityCountForApp);
    }
}

void terminateOtherThread(const KProcessPtr& process, U32 threadId) {
    KThread* thread = process->getThreadById(threadId);
    if (thread) {
        thread->terminating = true;

        const std::shared_ptr<BoxedWineCondition> cond = thread->waitingCond;

        // wake up the thread if it is waiting
        if (cond) {
            cond->lock();
            cond->signal();
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
