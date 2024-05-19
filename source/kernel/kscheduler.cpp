/*
 *  Copyright (C) 2016  The BoxedWine Team
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
#include "knativewindow.h"

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

void terminateOtherThread(const std::shared_ptr<KProcess>& process, U32 threadId) {
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

S32 contextTime = 100000;
S32 contextTimeRemaining = 100000;
int count;
extern struct Block emptyBlock;

void runThreadSlice(KThread* thread) {
    CPU* cpu;

    cpu = thread->cpu;
    cpu->blockInstructionCount = 0;
    cpu->yield = false;
    cpu->nextBlock = cpu->getNextBlock(); // another thread that just ran could have modified this
    try {
        do {
            cpu->run();
        } while ((int)cpu->blockInstructionCount < contextTimeRemaining && !cpu->yield);
    } catch (...) {
        cpu->nextBlock = nullptr;
    }

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
    
    std::shared_ptr<KNativeWindow> window = KNativeWindow::getNativeWindow();
    if (window) {
        window->flipFB();
    }
    U64 elapsedTime = 0;

    contextTimeRemaining = contextTime;
    while (!scheduledThreads.isEmpty() && elapsedTime<9000) {
        U64 threadStartTime = KSystem::getMicroCounter();
        KListNode<KThread*>* node = scheduledThreads.front();
        KThread* currentThread = (KThread*)node->data;
        KNativeWindow::getNativeWindow()->glUpdateContextForThread(currentThread);
        sysCallTime = 0;    

        ChangeThread c(currentThread);
        static U64 rdtsc;
        currentThread->cpu->instructionCount = rdtsc;
        runThreadSlice(currentThread);
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
            if (contextTime>100000)
                contextTime-=20000;
        } else if (elapsedTime<9500) {
            contextTime+=20000;
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

void waitForProcessToFinish(const std::shared_ptr<KProcess>& process, KThread* thread) {
    while (!process->terminated) {
        runThreadSlice(thread);
    }
}
#endif