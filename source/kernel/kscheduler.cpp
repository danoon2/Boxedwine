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

#include "devfb.h"
#include "kscheduler.h"

#include <stdio.h>

//#define LOG_SCHEDULER

KList<KThread*> scheduledThreads;
KList<KThread*> waitThreads;
KList<KTimer*> timers;

void addTimer(KTimer* timer) {
    timers.addToBack(&timer->node);
    timer->active = true;
}

void removeTimer(KTimer* timer) {
    timer->node.remove();
    timer->active = false;
}

void wakeThread(KThread* thread) {
    if (!thread->waiting) {
        kwarn("wakeThread: tried to wake a thread that is not asleep");
        return;
    }
    thread->waiting = false;
    if (thread->timer.active) {
        removeTimer(&thread->timer);
    }
    thread->clearWaitNofifyNodes();
    thread->waitThreadNode.remove();

    scheduleThread(thread);
}

void wakeThreads(U32 wakeType) {
    waitThreads.for_each([wakeType](KListNode<KThread*>* node) {
        if (node->data->waitType == wakeType) {
            wakeThread(node->data);
        }
    });
}

void scheduleThread(KThread* thread) {
    scheduledThreads.addToFront(&thread->scheduledThreadNode);
}

void unscheduleThread(KThread* thread) {	
    if (thread->waiting) {
        // will wake up all threads waiting on this this
        wakeThread(thread);
    }
    thread->scheduledThreadNode.remove();
    thread->cpu->yield = true;
}

void waitThread(KThread* thread) {
    unscheduleThread(thread);
    thread->waiting = true;
    waitThreads.addToBack(&thread->waitThreadNode);
}

U32 contextTime = 100000;
#ifdef BOXEDWINE_HAS_SETJMP
jmp_buf runBlockJump;
#endif
int count;
extern struct Block emptyBlock;

void runThreadSlice(KThread* thread) {
    CPU* cpu;

    cpu = thread->cpu;
    cpu->blockInstructionCount = 0;
    cpu->yield = false;
    cpu->nextBlock = cpu->getNextBlock(); // another thread that just ran could have modified this
#ifdef BOXEDWINE_HAS_SETJMP
    if (setjmp(runBlockJump)==0) {
#endif
        do {
            cpu->run();
        } while (cpu->blockInstructionCount < contextTime && !cpu->yield);	

#ifdef BOXEDWINE_HAS_SETJMP
    } else {
        cpu->nextBlock = NULL;
    }
#endif

    cpu->instructionCount+=cpu->blockInstructionCount;
}

void runTimers() {
    U32 millies = getMilliesSinceStart();
    timers.for_each([millies] (KListNode<KTimer*>* node) {
        KTimer* timer = node->data;

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

void sdlUpdateContextForThread(KThread* thread);

bool runSlice() {    
    runTimers();

    if (scheduledThreads.isEmpty())
        return false;

    KListNode<KThread*>* node = scheduledThreads.front();
    KThread* currentThread = (KThread*)node->data;
    node->remove();
    scheduledThreads.addToBack(node);

    //flipFB();
	
    U64 startTime = Platform::getMicroCounter();
    U64 endTime;
    U64 diff;

    sdlUpdateContextForThread(currentThread);    
    sysCallTime = 0;    

    ChangeThread c(currentThread);
    static U64 rdtsc;
    currentThread->cpu->instructionCount = rdtsc;
    platformRunThreadSlice(currentThread);
    rdtsc = currentThread->cpu->instructionCount;

    endTime = Platform::getMicroCounter();
    diff = endTime-startTime;
        
    elapsedTimeMIPS+=diff-sysCallTime;

    if (currentThread->cpu->blockInstructionCount) {
        if (diff>20000) {
            contextTime-=2000;
        } else if (diff<10000) {
            contextTime+=2000;
        }
    }

    elapsedInstructionsMIPS+=currentThread->cpu->blockInstructionCount;

    currentThread->userTime+=diff;
    currentThread->kernelTime+=sysCallTime;

    // this is how we signal to delete the current thread, since we can't delete it in the syscall, maybe we should use smart_ptr for threads
    if (!currentThread->process) {
        delete currentThread;
    }
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