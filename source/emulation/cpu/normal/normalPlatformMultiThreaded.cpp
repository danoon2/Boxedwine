#include "boxedwine.h"
#include "knativesystem.h"

#if defined(BOXEDWINE_MULTI_THREADED) && !defined(BOXEDWINE_BINARY_TRANSLATOR)

std::atomic<int> platformThreadCount = 0;

static void platformThread(CPU* cpu) {
    KThread::setCurrentThread(cpu->thread);

    while (true) {
        try {
            cpu->nextBlock = cpu->getNextBlock();
            cpu->run();
        } catch (...) {
            int ii = 0;
        }
#ifdef __TEST
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

    std::shared_ptr<KProcess> process = cpu->thread->process;
    process->deleteThread(cpu->thread);

    platformThreadCount--;
    if (platformThreadCount == 0) {
        KSystem::shutingDown = true;
        KNativeSystem::postQuit();
    }
}

void scheduleThread(KThread* thread) {
    platformThreadCount++;
    CPU* cpu = thread->cpu;
    std::thread cppThread = std::thread(platformThread, cpu);
    cpu->nativeHandle = (U64)cppThread.native_handle();
    cppThread.detach();
    if (!thread->process->isSystemProcess() && KSystem::cpuAffinityCountForApp) {
        Platform::setCpuAffinityForThread(thread, KSystem::cpuAffinityCountForApp);
    }
}

void terminateOtherThread(const std::shared_ptr<KProcess>& process, U32 threadId) {
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