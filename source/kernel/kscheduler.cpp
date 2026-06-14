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
#include "../x11/x11.h"
#include "knativeaudio.h"

#ifdef BOXEDWINE_MULTI_THREADED
static KList<KTimerCallback*> timers;
static BOXEDWINE_MUTEX timerMutex;
void runTimers() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(timerMutex);
    U32 millies = KSystem::getMilliesSinceStart();
    timers.for_each([millies](KListNode<KTimerCallback*>* node) {
        KTimerCallback* timer = node->data;

        if (timer->millies <= millies) {
            if (timer->run()) {
                timer->node.remove();
            }
        }
        });
}

U32 getNextTimer() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(timerMutex);
    U32 millies = KSystem::getMilliesSinceStart();
    U32 result = 0xFFFFFFFF;

    timers.for_each([millies, &result](KListNode<KTimerCallback*>* node) {
        KTimerCallback* timer = node->data;
        U32 next = 0;

        if (timer->millies <= millies) {
            next = 0;
        }
        else {
            next = timer->millies - millies;
        }
        if (next < result) {
            result = next;
        }
        });
    return result;
}

void addTimer(KTimerCallback* timer) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(timerMutex);
    timers.addToBack(&timer->node);
    timer->active = true;
}

void removeTimer(KTimerCallback* timer) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(timerMutex);
    timer->node.remove();
    timer->active = false;
}
#else
#include "kscheduler.h"
#include "knativesystem.h"

#include <stdio.h>

//#define LOG_SCHEDULER

KList<KThread*> scheduledThreads;
KList<KThread*> waitThreads;
KList<KTimerCallback*> timers;

void addTimer(KTimerCallback* timer) {
    timers.addToBack(&timer->node);
    timer->active = true;
}

void removeTimer(KTimerCallback* timer) {
    timer->node.remove();
    timer->active = false;
}

void scheduleThread(KThread* thread) {
#ifdef _DEBUG
    if (thread->waitingCond) {
        kpanic("can't schedule a thread that is waiting");
    }
    if (thread->condTimer.active) {
        kpanic("can't schedule a thread that is waiting on a timer");
    }
#endif
    thread->cpu->yield = false;
    scheduledThreads.addToFront(&thread->scheduledThreadNode);
}

void unscheduleThread(KThread* thread) {	    
    thread->scheduledThreadNode.remove();
    thread->cpu->yield = true;
}

void terminateOtherThread(const KProcessPtr& process, U32 threadId) {
    KThread* thread = process->getThreadById(threadId);
    if (thread) {
        unscheduleThread(thread);
        delete thread;
    }
}

void terminateCurrentThread(KThread* thread) {
	thread->terminating = true;
	unscheduleThread(thread);
}

#ifdef __EMSCRIPTEN__
// Single-threaded scheduler slice size (instructions/slice), self-tuned toward a
// ~10ms slice by runSlice() (grow when a slice took <9.5ms, shrink when >11ms).
// The master merge capped this at 10000, which throttled ST JIT/interp throughput
// (5-50x smaller slices than the pre-merge fixed 100000). The ceiling is raised
// back to 100000 so cheap slices reach pre-merge throughput, while the 2000 floor
// still lets it throttle down under load for UI responsiveness. STEP is widened
// (1000 -> 10000) to match the larger range so it ramps/throttles in a few steps.
static const S32 DEFAULT_CONTEXT_TIME = 10000;
static const S32 MIN_CONTEXT_TIME = 2000;
static const S32 MAX_CONTEXT_TIME = 100000;
static const S32 CONTEXT_TIME_STEP = 10000;
S32 contextTime = DEFAULT_CONTEXT_TIME;
S32 contextTimeRemaining = DEFAULT_CONTEXT_TIME;
#else
S32 contextTime = 100000;
S32 contextTimeRemaining = 100000;
#endif
int count;
extern struct Block emptyBlock;

#ifdef __EMSCRIPTEN__
static KThread* runSliceExceptionThread;

static S32 decreaseContextTime(S32 value) {
    if (value <= MIN_CONTEXT_TIME + CONTEXT_TIME_STEP) {
        return MIN_CONTEXT_TIME;
    }
    return value - CONTEXT_TIME_STEP;
}

static S32 increaseContextTime(S32 value) {
    if (value >= MAX_CONTEXT_TIME - CONTEXT_TIME_STEP) {
        return MAX_CONTEXT_TIME;
    }
    return value + CONTEXT_TIME_STEP;
}

bool recoverRunSliceException() {
    if (!runSliceExceptionThread) {
        return false;
    }
    runSliceExceptionThread->cpu->nextOp = nullptr;
    runSliceExceptionThread = nullptr;
    return true;
}
#endif

void runThreadSlice(KThread* thread) {
    CPU* cpu;

    cpu = thread->cpu;
    cpu->blockInstructionCount = 0;
    cpu->yield = false;
    cpu->nextOp = cpu->getNextOp(); // another thread that just ran could have modified this
#ifdef __EMSCRIPTEN__
    do {
        cpu->run();
    } while ((int)cpu->blockInstructionCount < contextTimeRemaining && !cpu->yield);
#else
    try {
        do {
            cpu->run();
        } while ((int)cpu->blockInstructionCount < contextTimeRemaining && !cpu->yield);
    } catch (...) {
        cpu->nextOp = nullptr;
    }
#endif

    cpu->instructionCount+=cpu->blockInstructionCount;
}

void runTimers() {
    U32 millies = KSystem::getMilliesSinceStart();
    timers.for_each([millies] (KListNode<KTimerCallback*>* node) {
        KTimerCallback* timer = node->data;

        if (timer->millies<=millies) {
            if (timer->run()) {                
                timer->node.remove();
            }
        }
    });
}

extern U64 sysCallTime;
U64 elapsedTimeMIPS;
U64 elapsedInstructionsMIPS;

bool runSlice() {    
    runTimers();

    if (scheduledThreads.isEmpty())
        return false;
    
    XServer* server = XServer::getServer(true);
    if (server) {
        server->isDisplayDirty = true; // a bit of a hack, sometimes popups in Basstour get missed and don't draw
        server->draw();
    }

    U64 elapsedTime = 0;

    contextTimeRemaining = contextTime;
    while (!scheduledThreads.isEmpty() && elapsedTime<9000) {
        U64 threadStartTime = KSystem::getMicroCounter();
        KListNode<KThread*>* node = scheduledThreads.front();
        KThread* currentThread = (KThread*)node->data;
        KNativeSystem::scheduledNewThread(currentThread);
        sysCallTime = 0;

#ifdef __EMSCRIPTEN__
        runSliceExceptionThread = currentThread;
#endif
        ChangeThread c(currentThread);
        static U64 rdtsc;
        currentThread->cpu->instructionCount = rdtsc;
        runThreadSlice(currentThread);
#ifdef __EMSCRIPTEN__
        runSliceExceptionThread = nullptr;
#endif
        rdtsc = currentThread->cpu->instructionCount;

        U64 threadEndTime = KSystem::getMicroCounter();
        U64 diff = threadEndTime - threadStartTime;

        elapsedTime+=diff;

        elapsedTimeMIPS+=diff;        
        elapsedInstructionsMIPS+=currentThread->cpu->blockInstructionCount;

        currentThread->userTime+=diff-sysCallTime;
        currentThread->kernelTime+=sysCallTime;

        if (currentThread->cpu->blockInstructionCount) {
            contextTimeRemaining = (U32)(contextTime * (10000-elapsedTime) / 10000);
        }
        // this is how we signal to delete the current thread, since we can't delete it in the syscall, maybe we should use smart_ptr for threads
        if (currentThread->terminating) {
            delete currentThread;
        } else if (!currentThread->waitingCond) {
            // make sure we are behind any threads that were recently scheduled
            node->remove();
            scheduledThreads.addToBack(node);
        }
    }
    if (!scheduledThreads.isEmpty()) {
        if (elapsedTime>11000) {
#ifdef __EMSCRIPTEN__
            contextTime = decreaseContextTime(contextTime);
#else
            if (contextTime>100000)
                contextTime-=20000;
#endif
        } else if (elapsedTime<9500) {
#ifdef __EMSCRIPTEN__
            contextTime = increaseContextTime(contextTime);
#else
            contextTime+=20000;
#endif
        }
    }
    //klog("ran slice in %dus %d", (U32)elapsedTime, contextTime);    
    
    return true;
}

U32 getMIPS() {
    U32 result = 0;
    if (elapsedTimeMIPS) {
        result = (U32)(elapsedInstructionsMIPS/elapsedTimeMIPS);
        elapsedTimeMIPS = 0;
        elapsedInstructionsMIPS = 0;
    }
    return result;
}

void waitForProcessToFinish(const KProcessPtr& process, KThread* thread) {
    while (!process->terminated) {
        runThreadSlice(thread);
    }
}
#endif
