#include "boxedwine.h"
#include <SDL.h>
#include "../../source/emulation/cpu/x64/x64CPU.h"
#include "../../source/emulation/cpu/x64/x64CodeChunk.h"
#include "../../source/emulation/hardmmu/hard_memory.h"
#include "../../source/emulation/cpu/normal/normalCPU.h"
#include "../../source/emulation/cpu/x64/x64Asm.h"
#include <string.h>

#define __USE_GNU
#define _XOPEN_SOURCE
#include <ucontext.h>

#ifdef BOXEDWINE_MULTI_THREADED

void syncFromException(x64CPU* cpu, ucontext_t* context, bool includeFPU) {
    EAX = (U32)context->uc_mcontext->__ss.__rax;
    ECX = (U32)context->uc_mcontext->__ss.__rcx;
    EDX = (U32)context->uc_mcontext->__ss.__rdx;
    EBX = (U32)context->uc_mcontext->__ss.__rbx;
    ESP = (U32)context->uc_mcontext->__ss.__r11;
    EBP = (U32)context->uc_mcontext->__ss.__rbp;
    ESI = (U32)context->uc_mcontext->__ss.__rsi;
    EDI = (U32)context->uc_mcontext->__ss.__rdi;
    cpu->flags = (U32)context->uc_mcontext->__ss.__rflags;
    cpu->lazyFlags = FLAGS_NONE;
    memcpy(&cpu->xmm[0], &context->uc_mcontext->__fs.__fpu_xmm0, 16);
    memcpy(&cpu->xmm[1], &context->uc_mcontext->__fs.__fpu_xmm1, 16);
    memcpy(&cpu->xmm[2], &context->uc_mcontext->__fs.__fpu_xmm2, 16);
    memcpy(&cpu->xmm[3], &context->uc_mcontext->__fs.__fpu_xmm3, 16);
    memcpy(&cpu->xmm[4], &context->uc_mcontext->__fs.__fpu_xmm4, 16);
    memcpy(&cpu->xmm[5], &context->uc_mcontext->__fs.__fpu_xmm5, 16);
    memcpy(&cpu->xmm[6], &context->uc_mcontext->__fs.__fpu_xmm6, 16);
    memcpy(&cpu->xmm[7], &context->uc_mcontext->__fs.__fpu_xmm7, 16);
    
    
    if (includeFPU && !cpu->thread->process->emulateFPU) {
        cpu->fpu.SetCW(*((U16*)&context->uc_mcontext->__fs.__fpu_fcw));
        cpu->fpu.SetSW(*((U16*)&context->uc_mcontext->__fs.__fpu_fsw));
        cpu->fpu.SetTagFromAbridged(context->uc_mcontext->__fs.__fpu_ftw);
        for (U32 i=0;i<8;i++) {
            if (!(context->uc_mcontext->__fs.__fpu_ftw & (1 << i))) {
                cpu->fpu.setReg(i, 0.0);
            } else {
                U32 index = (i-cpu->fpu.GetTop()) & 7;
                U64 low;
                S16 high;
                
                switch (index) {
                    case 0:
                        low = *((U64*)&context->uc_mcontext->__fs.__fpu_stmm0);
                        high = *((S16*)&context->uc_mcontext->__fs.__fpu_stmm0.__mmst_reg[8]);
                        break;
                    case 1:
                        low = *((U64*)&context->uc_mcontext->__fs.__fpu_stmm1);
                        high = *((S16*)&context->uc_mcontext->__fs.__fpu_stmm1.__mmst_reg[8]);
                        break;
                    case 2:
                        low = *((U64*)&context->uc_mcontext->__fs.__fpu_stmm2);
                        high = *((S16*)&context->uc_mcontext->__fs.__fpu_stmm2.__mmst_reg[8]);
                        break;
                    case 3:
                        low = *((U64*)&context->uc_mcontext->__fs.__fpu_stmm3);
                        high = *((S16*)&context->uc_mcontext->__fs.__fpu_stmm3.__mmst_reg[8]);
                        break;
                    case 4:
                        low = *((U64*)&context->uc_mcontext->__fs.__fpu_stmm4);
                        high = *((S16*)&context->uc_mcontext->__fs.__fpu_stmm4.__mmst_reg[8]);
                        break;
                    case 5:
                        low = *((U64*)&context->uc_mcontext->__fs.__fpu_stmm5);
                        high = *((S16*)&context->uc_mcontext->__fs.__fpu_stmm5.__mmst_reg[8]);
                        break;
                    case 6:
                        low = *((U64*)&context->uc_mcontext->__fs.__fpu_stmm6);
                        high = *((S16*)&context->uc_mcontext->__fs.__fpu_stmm6.__mmst_reg[8]);
                        break;
                    case 7:
                        low = *((U64*)&context->uc_mcontext->__fs.__fpu_stmm7);
                        high = *((S16*)&context->uc_mcontext->__fs.__fpu_stmm7.__mmst_reg[8]);
                        break;
                }
                double d = cpu->fpu.FLD80(low, high);
                cpu->fpu.setReg(i, d);
            }
        }
    }
}

void syncToException(x64CPU* cpu, ucontext_t* context, bool includeFPU) {
    context->uc_mcontext->__ss.__rax = EAX;
    context->uc_mcontext->__ss.__rcx = ECX;
    context->uc_mcontext->__ss.__rdx = EDX;
    context->uc_mcontext->__ss.__rbx = EBX;
    context->uc_mcontext->__ss.__r11 = ESP;
    context->uc_mcontext->__ss.__rbp = EBP;
    context->uc_mcontext->__ss.__rsi = ESI;
    context->uc_mcontext->__ss.__rdi = EDI;
    context->uc_mcontext->__ss.__r14 = cpu->seg[SS].address;
    context->uc_mcontext->__ss.__r15 = cpu->seg[DS].address;
    cpu->fillFlags();
    context->uc_mcontext->__ss.__rflags &=~ CF | OF | PF | AF | SF | ZF;
    context->uc_mcontext->__ss.__rflags |= (cpu->flags & (CF | OF | PF | AF | SF | ZF));
    
    memcpy(&context->uc_mcontext->__fs.__fpu_xmm0, &cpu->xmm[0], 16);
    memcpy(&context->uc_mcontext->__fs.__fpu_xmm1, &cpu->xmm[1], 16);
    memcpy(&context->uc_mcontext->__fs.__fpu_xmm2, &cpu->xmm[2], 16);
    memcpy(&context->uc_mcontext->__fs.__fpu_xmm3, &cpu->xmm[3], 16);
    memcpy(&context->uc_mcontext->__fs.__fpu_xmm4, &cpu->xmm[4], 16);
    memcpy(&context->uc_mcontext->__fs.__fpu_xmm5, &cpu->xmm[5], 16);
    memcpy(&context->uc_mcontext->__fs.__fpu_xmm6, &cpu->xmm[6], 16);
    memcpy(&context->uc_mcontext->__fs.__fpu_xmm7, &cpu->xmm[7], 16);
    
    if (includeFPU && !cpu->thread->process->emulateFPU) {
        *((U16*)&context->uc_mcontext->__fs.__fpu_fcw) = cpu->fpu.CW();
        *((U16*)&context->uc_mcontext->__fs.__fpu_fsw) = cpu->fpu.SW();
        context->uc_mcontext->__fs.__fpu_ftw = cpu->fpu.GetAbridgedTag();
        for (U32 i=0;i<8;i++) {
            U32 index = (i-cpu->fpu.GetTop()) & 7;
            U64* low;
            S16* high;
            
            switch (index) {
                case 0:
                    low = (U64*)&context->uc_mcontext->__fs.__fpu_stmm0;
                    high = (S16*)&context->uc_mcontext->__fs.__fpu_stmm0.__mmst_reg[8];
                    break;
                case 1:
                    low = (U64*)&context->uc_mcontext->__fs.__fpu_stmm1;
                    high = (S16*)&context->uc_mcontext->__fs.__fpu_stmm1.__mmst_reg[8];
                    break;
                case 2:
                    low = (U64*)&context->uc_mcontext->__fs.__fpu_stmm2;
                    high = (S16*)&context->uc_mcontext->__fs.__fpu_stmm2.__mmst_reg[8];
                    break;
                case 3:
                    low = (U64*)&context->uc_mcontext->__fs.__fpu_stmm3;
                    high = (S16*)&context->uc_mcontext->__fs.__fpu_stmm3.__mmst_reg[8];
                    break;
                case 4:
                    low = (U64*)&context->uc_mcontext->__fs.__fpu_stmm4;
                    high = (S16*)&context->uc_mcontext->__fs.__fpu_stmm4.__mmst_reg[8];
                    break;
                case 5:
                    low = (U64*)&context->uc_mcontext->__fs.__fpu_stmm5;
                    high = (S16*)&context->uc_mcontext->__fs.__fpu_stmm5.__mmst_reg[8];
                    break;
                case 6:
                    low = (U64*)&context->uc_mcontext->__fs.__fpu_stmm6;
                    high = (S16*)&context->uc_mcontext->__fs.__fpu_stmm6.__mmst_reg[8];
                    break;
                case 7:
                    low = (U64*)&context->uc_mcontext->__fs.__fpu_stmm7;
                    high = (S16*)&context->uc_mcontext->__fs.__fpu_stmm7.__mmst_reg[8];
                    break;
            }
            
            if (!(context->uc_mcontext->__fs.__fpu_ftw& (1 << i))) {
                *low= 0;
                *high = 0;
            } else {
                U64 h=0;
                cpu->fpu.ST80(i, low, &h);
                *high = h;
            }
        }
    }
}

class InException {
public:
    InException(x64CPU* cpu) : cpu(cpu) {this->cpu->inException = true;}
    ~InException() {this->cpu->inException = false;}
    x64CPU* cpu;
};

U32 exceptionCount;

#include <signal.h>
#include <pthread.h>

static void handler(int sig, siginfo_t* info, void* vcontext)
{
    BOXEDWINE_CRITICAL_SECTION;
    exceptionCount++;
    
    KThread* currentThread = KThread::currentThread();
    if (!currentThread) {
        return;
    }
    ucontext_t *context = (ucontext_t*)vcontext;
    x64CPU* cpu = (x64CPU*)currentThread->cpu;
    if (cpu->restarting) {
        context->uc_mcontext->__ss.__rip = (U64)cpu->init();
        cpu->restarting = false;
        return;
    }
    if (cpu!=(x64CPU*)context->uc_mcontext->__ss.__r13) {
        return;
    }
    
    std::function<void(DecodedOp*)> doSyncFrom = [cpu, context] (DecodedOp* op) {
        syncFromException(cpu, context, op?op->isFpuOp():true);
    };
    std::function<void(DecodedOp*)> doSyncTo = [cpu, context] (DecodedOp* op) {
        syncToException(cpu, context, op?op->isFpuOp():true);
    };
    
#ifdef __MACH__
    bool readAccess = ((ucontext_t*)context)->uc_mcontext->__es.__err;
#else
    bool readAccess = ((ucontext_t*)context)->uc_mcontext.gregs[REG_ERR];
#endif
    
    U32 address = getHostAddress(currentThread, (void*)info->si_addr);
    U64 result = cpu->startException(address, readAccess, doSyncFrom, doSyncTo);
    if (result) {
        context->uc_mcontext->__ss.__rip = result;
        return;
    }
    
    InException inException(cpu);
    if (info->si_signo == SIGILL) {
        if (context->uc_mcontext->__ss.__rsp & 0xf) {
            kpanic("seh_filter: bad stack alignment");
        }
        U64 rip = cpu->handleIllegalInstruction(context->uc_mcontext->__ss.__rip);
        if (rip) {
            context->uc_mcontext->__ss.__rip = rip;
            return;
        }
    } else if (info->si_signo == SIGBUS && info->si_code == BUS_ADRALN) {
        // :TODO: figure out how AC got set, I've only seen this while op logging
        context->uc_mcontext->__ss.__rflags &= ~AC;
        return;
    } else if ((info->si_signo == SIGBUS || info->si_signo == SIGSEGV) && (context->uc_mcontext->__ss.__rip & 0xFFFFFFFF00000000l)==(U64)cpu->thread->memory->executableMemoryId) {
        U64 rip = cpu->handleAccessException(context->uc_mcontext->__ss.__rip, address, readAccess, context->uc_mcontext->__ss.__rsi, context->uc_mcontext->__ss.__rdi, context->uc_mcontext->__ss.__r8, context->uc_mcontext->__ss.__r9, &context->uc_mcontext->__ss.__r10, doSyncFrom, doSyncTo);
        if (rip) {
            context->uc_mcontext->__ss.__rip = rip;
        }
        return;
    } else if (info->si_signo == SIGFPE && (info->si_code == FPE_INTDIV || info->si_code == FPE_FLTDIV)) {
        context->uc_mcontext->__ss.__rip = cpu->handleDivByZero(doSyncFrom, doSyncTo);
        return;
    }
}

U32 platformThreadCount = 0;

void* platformThreadProc(void* param) {
    static bool initializedHandler = false;
    if (!initializedHandler) {
        struct sigaction sa = { .sa_sigaction=handler, .sa_flags=SA_SIGINFO };
        struct sigaction oldsa;
        sigaction(SIGBUS, &sa, &oldsa);
        sigaction(SIGSEGV, &sa, &oldsa);
        sigaction(SIGILL, &sa, &oldsa);
        sigaction(SIGFPE, &sa, &oldsa);
        initializedHandler = true;
    }
    platformThreadCount++;
    KThread* thread = (KThread*)param;
    x64CPU* cpu = (x64CPU*)thread->cpu;
    cpu->startThread();
    return 0;
}

void scheduleThread(KThread* thread) {
    platformThreadCount++;
    x64CPU* cpu = (x64CPU*)thread->cpu;
    pthread_t threadId;
    
    pthread_create(&threadId, NULL, platformThreadProc, thread);
    cpu->nativeHandle = (U64)(size_t)threadId;
}

#ifdef BOXEDWINE_MULTI_THREADED
void ATOMIC_WRITE64(U64* pTarget, U64 value) {
    __sync_synchronize();
    __sync_lock_test_and_set((volatile S64 *)pTarget, (S64)value);
}
#endif

#endif
