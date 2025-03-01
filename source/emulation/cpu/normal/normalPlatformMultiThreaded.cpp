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

#if defined(BOXEDWINE_MULTI_THREADED) && !defined(BOXEDWINE_BINARY_TRANSLATOR)

std::atomic<int> platformThreadCount = 0;

static void platformThread(CPU* cpu) {
    KThread::setCurrentThread(cpu->thread);
    KProcessPtr process = KSystem::getProcess(cpu->thread->process->id);

    while (true) {
        try {
            if (!cpu->nextBlock) {
                cpu->nextBlock = cpu->getNextBlock();
            }
            cpu->run();
        } catch (...) {
            cpu->nextBlock = nullptr;
        }
#ifdef __TEST
        if (cpu->nextBlock) {
            DecodedOp* o = cpu->nextBlock->op;
            bool found = false;

            while (o) {
                if (o->inst == IntIb && o->imm == 0x97) {
                    found = true;
                }
                o = o->next;
            }
            if (!found) {
                continue;
            }
        }
        return;
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

void scheduleThread(KThread* thread) {
    platformThreadCount++;
    CPU* cpu = thread->cpu;
#ifdef __TEST
    cpu->nativeHandle = (U64)new std::thread(platformThread, cpu);
#else
    std::thread cppThread = std::thread(platformThread, cpu);
    cpu->nativeHandle = (U64)cppThread.native_handle();
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