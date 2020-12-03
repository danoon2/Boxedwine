#include "boxedwine.h"

#ifdef BOXEDWINE_MULTI_THREADED

#include "../../source/emulation/cpu/binaryTranslation/btCpu.h"
#include <signal.h>
#include <pthread.h>

U32 platformThreadCount = 0;

void platformHandler(int sig, siginfo_t* info, void* vcontext);

void* platformThreadProc(void* param) {
    static bool initializedHandler = false;
    if (!initializedHandler) {
        struct sigaction sa;
        sa.sa_sigaction= platformHandler;
        sa.sa_flags=SA_SIGINFO;
        struct sigaction oldsa;
        sigaction(SIGBUS, &sa, &oldsa);
        sigaction(SIGSEGV, &sa, &oldsa);
        sigaction(SIGILL, &sa, &oldsa);
        sigaction(SIGFPE, &sa, &oldsa);
        for (int i = 0x91; i <= 0x96; i++) {
            sigaction(i, &sa, &oldsa);
        }
        sigaction(SIGTRAP, &sa, &oldsa);
        initializedHandler = true;
    }
    KThread* thread = (KThread*)param;
    BtCPU* cpu = (BtCPU*)thread->cpu;
    cpu->startThread();
    return 0;
}

void scheduleThread(KThread* thread) {
    BtCPU* cpu = (BtCPU*)thread->cpu;
    pthread_t threadId;
    platformThreadCount++; // need to increment before returning, otherwise if this is 0 the code will assume Wine exited
    pthread_create(&threadId, NULL, platformThreadProc, thread);
    cpu->nativeHandle = (U64)(size_t)threadId;
}

void ATOMIC_WRITE64(U64* pTarget, U64 value) {
    __sync_synchronize();
    __sync_lock_test_and_set((volatile S64 *)pTarget, (S64)value);
}

#endif
