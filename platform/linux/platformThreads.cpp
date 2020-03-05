#include "boxedwine.h"
#include <SDL.h>
#include "../../source/emulation/cpu/x64/x64CPU.h"
#include "../../source/emulation/cpu/x64/x64CodeChunk.h"
#include "../../source/emulation/hardmmu/hard_memory.h"
#include "../../source/emulation/cpu/normal/normalCPU.h"
#include "../../source/emulation/cpu/x64/x64Asm.h"
#include <string.h>

#ifdef __MACH__
#define __USE_GNU
#define _XOPEN_SOURCE

#define CONTEXT_RAX uc_mcontext->__ss.__rax
#define CONTEXT_RCX uc_mcontext->__ss.__rcx
#define CONTEXT_RDX uc_mcontext->__ss.__rdx
#define CONTEXT_RBX uc_mcontext->__ss.__rbx
#define CONTEXT_RSP uc_mcontext->__ss.__rsp
#define CONTEXT_RBP uc_mcontext->__ss.__rbp
#define CONTEXT_RSI uc_mcontext->__ss.__rsi
#define CONTEXT_RDI uc_mcontext->__ss.__rdi
#define CONTEXT_R8 uc_mcontext->__ss.__r8
#define CONTEXT_R9 uc_mcontext->__ss.__r9
#define CONTEXT_R10 uc_mcontext->__ss.__r10
#define CONTEXT_R11 uc_mcontext->__ss.__r11
#define CONTEXT_R13 uc_mcontext->__ss.__r13
#define CONTEXT_R14 uc_mcontext->__ss.__r14
#define CONTEXT_R15 uc_mcontext->__ss.__r15
#define CONTEXT_RIP uc_mcontext->__ss.__rip
#define CONTEXT_FLAGS uc_mcontext->__ss.__rflags
#define CONTEXT_ERR uc_mcontext->__es.__err
#define CONTEXT_XMM0 uc_mcontext->__fs.__fpu_xmm0
#define CONTEXT_XMM1 uc_mcontext->__fs.__fpu_xmm1
#define CONTEXT_XMM2 uc_mcontext->__fs.__fpu_xmm2
#define CONTEXT_XMM3 uc_mcontext->__fs.__fpu_xmm3
#define CONTEXT_XMM4 uc_mcontext->__fs.__fpu_xmm4
#define CONTEXT_XMM5 uc_mcontext->__fs.__fpu_xmm5
#define CONTEXT_XMM6 uc_mcontext->__fs.__fpu_xmm6
#define CONTEXT_XMM7 uc_mcontext->__fs.__fpu_xmm7
#define CONTEXT_FCW uc_mcontext->__fs.__fpu_fcw
#define CONTEXT_FSW uc_mcontext->__fs.__fpu_fsw
#define CONTEXT_FTW uc_mcontext->__fs.__fpu_ftw
#define CONTEXT_FPU_REG_0_LOW(context) ((U64*)&context->uc_mcontext->__fs.__fpu_stmm0)
#define CONTEXT_FPU_REG_0_HIGH(context) ((S16*)&context->uc_mcontext->__fs.__fpu_stmm0.__mmst_reg[8])
#define CONTEXT_FPU_REG_1_LOW(context) ((U64*)&context->uc_mcontext->__fs.__fpu_stmm1)
#define CONTEXT_FPU_REG_1_HIGH(context) ((S16*)&context->uc_mcontext->__fs.__fpu_stmm1.__mmst_reg[8])
#define CONTEXT_FPU_REG_2_LOW(context) ((U64*)&context->uc_mcontext->__fs.__fpu_stmm2)
#define CONTEXT_FPU_REG_2_HIGH(context) ((S16*)&context->uc_mcontext->__fs.__fpu_stmm2.__mmst_reg[8])
#define CONTEXT_FPU_REG_3_LOW(context) ((U64*)&context->uc_mcontext->__fs.__fpu_stmm3)
#define CONTEXT_FPU_REG_3_HIGH(context) ((S16*)&context->uc_mcontext->__fs.__fpu_stmm3.__mmst_reg[8])
#define CONTEXT_FPU_REG_4_LOW(context) ((U64*)&context->uc_mcontext->__fs.__fpu_stmm4)
#define CONTEXT_FPU_REG_4_HIGH(context) ((S16*)&context->uc_mcontext->__fs.__fpu_stmm4.__mmst_reg[8])
#define CONTEXT_FPU_REG_5_LOW(context) ((U64*)&context->uc_mcontext->__fs.__fpu_stmm5)
#define CONTEXT_FPU_REG_5_HIGH(context) ((S16*)&context->uc_mcontext->__fs.__fpu_stmm5.__mmst_reg[8])
#define CONTEXT_FPU_REG_6_LOW(context) ((U64*)&context->uc_mcontext->__fs.__fpu_stmm6)
#define CONTEXT_FPU_REG_6_HIGH(context) ((S16*)&context->uc_mcontext->__fs.__fpu_stmm6.__mmst_reg[8])
#define CONTEXT_FPU_REG_7_LOW(context) ((U64*)&context->uc_mcontext->__fs.__fpu_stmm7)
#define CONTEXT_FPU_REG_7_HIGH(context) ((S16*)&context->uc_mcontext->__fs.__fpu_stmm7.__mmst_reg[8])
#else
#define CONTEXT_RAX uc_mcontext.gregs[REG_RAX]
#define CONTEXT_RCX uc_mcontext.gregs[REG_RCX]
#define CONTEXT_RDX uc_mcontext.gregs[REG_RDX]
#define CONTEXT_RBX uc_mcontext.gregs[REG_RBX]
#define CONTEXT_RSP uc_mcontext.gregs[REG_RSP]
#define CONTEXT_RBP uc_mcontext.gregs[REG_RBP]
#define CONTEXT_RSI uc_mcontext.gregs[REG_RSI]
#define CONTEXT_RDI uc_mcontext.gregs[REG_RDI]
#define CONTEXT_R8 uc_mcontext.gregs[REG_R8]
#define CONTEXT_R9 uc_mcontext.gregs[REG_R9]
#define CONTEXT_R10 uc_mcontext.gregs[REG_R10]
#define CONTEXT_R11 uc_mcontext.gregs[REG_R11]
#define CONTEXT_R13 uc_mcontext.gregs[REG_R13]
#define CONTEXT_R14 uc_mcontext.gregs[REG_R14]
#define CONTEXT_R15 uc_mcontext.gregs[REG_R15]
#define CONTEXT_RIP uc_mcontext.gregs[REG_RIP]
#define CONTEXT_FLAGS uc_mcontext.gregs[REG_EFL]
#define CONTEXT_ERR uc_mcontext.gregs[REG_ERR]
#define CONTEXT_XMM0 uc_mcontext.fpregs->_xmm[0]
#define CONTEXT_XMM1 uc_mcontext.fpregs->_xmm[1]
#define CONTEXT_XMM2 uc_mcontext.fpregs->_xmm[2]
#define CONTEXT_XMM3 uc_mcontext.fpregs->_xmm[3]
#define CONTEXT_XMM4 uc_mcontext.fpregs->_xmm[4]
#define CONTEXT_XMM5 uc_mcontext.fpregs->_xmm[5]
#define CONTEXT_XMM6 uc_mcontext.fpregs->_xmm[6]
#define CONTEXT_XMM7 uc_mcontext.fpregs->_xmm[7]
#define CONTEXT_FCW uc_mcontext.fpregs->cwd
#define CONTEXT_FSW uc_mcontext.fpregs->swd
#define CONTEXT_FTW uc_mcontext.fpregs->ftw
union CAST_FPU {
    unsigned short int r16[4];
    U64 r64;
};

#define CONTEXT_FPU_REG_0_LOW(context) &((CAST_FPU*)&context->uc_mcontext.fpregs->_st[0].significand)->r64
#define CONTEXT_FPU_REG_0_HIGH(context) ((S16*)&context->uc_mcontext.fpregs->_st[0].exponent)
#define CONTEXT_FPU_REG_1_LOW(context) &((CAST_FPU*)&context->uc_mcontext.fpregs->_st[1].significand)->r64
#define CONTEXT_FPU_REG_1_HIGH(context) ((S16*)&context->uc_mcontext.fpregs->_st[1].exponent)
#define CONTEXT_FPU_REG_2_LOW(context) &((CAST_FPU*)&context->uc_mcontext.fpregs->_st[2].significand)->r64
#define CONTEXT_FPU_REG_2_HIGH(context) ((S16*)&context->uc_mcontext.fpregs->_st[2].exponent)
#define CONTEXT_FPU_REG_3_LOW(context) &((CAST_FPU*)&context->uc_mcontext.fpregs->_st[3].significand)->r64
#define CONTEXT_FPU_REG_3_HIGH(context) ((S16*)&context->uc_mcontext.fpregs->_st[3].exponent)
#define CONTEXT_FPU_REG_4_LOW(context) &((CAST_FPU*)&context->uc_mcontext.fpregs->_st[4].significand)->r64
#define CONTEXT_FPU_REG_4_HIGH(context) ((S16*)&context->uc_mcontext.fpregs->_st[4].exponent)
#define CONTEXT_FPU_REG_5_LOW(context) &((CAST_FPU*)&context->uc_mcontext.fpregs->_st[5].significand)->r64
#define CONTEXT_FPU_REG_5_HIGH(context) ((S16*)&context->uc_mcontext.fpregs->_st[5].exponent)
#define CONTEXT_FPU_REG_6_LOW(context) &((CAST_FPU*)&context->uc_mcontext.fpregs->_st[6].significand)->r64
#define CONTEXT_FPU_REG_6_HIGH(context) ((S16*)&context->uc_mcontext.fpregs->_st[6].exponent)
#define CONTEXT_FPU_REG_7_LOW(context) &((CAST_FPU*)&context->uc_mcontext.fpregs->_st[7].significand)->r64
#define CONTEXT_FPU_REG_7_HIGH(context) ((S16*)&context->uc_mcontext.fpregs->_st[7].exponent)
#endif
#include <ucontext.h>

#ifdef BOXEDWINE_MULTI_THREADED

void syncFromException(x64CPU* cpu, ucontext_t* context, bool includeFPU) {
    EAX = (U32)context->CONTEXT_RAX;
    ECX = (U32)context->CONTEXT_RCX;
    EDX = (U32)context->CONTEXT_RDX;
    EBX = (U32)context->CONTEXT_RBX;
    ESP = (U32)context->CONTEXT_R11;
    EBP = (U32)context->CONTEXT_RBP;
    ESI = (U32)context->CONTEXT_RSI;
    EDI = (U32)context->CONTEXT_RDI;
    cpu->flags = (U32)context->CONTEXT_FLAGS;
    cpu->lazyFlags = FLAGS_NONE;
    memcpy(&cpu->xmm[0], &context->CONTEXT_XMM0, 16);
    memcpy(&cpu->xmm[1], &context->CONTEXT_XMM1, 16);
    memcpy(&cpu->xmm[2], &context->CONTEXT_XMM2, 16);
    memcpy(&cpu->xmm[3], &context->CONTEXT_XMM3, 16);
    memcpy(&cpu->xmm[4], &context->CONTEXT_XMM4, 16);
    memcpy(&cpu->xmm[5], &context->CONTEXT_XMM5, 16);
    memcpy(&cpu->xmm[6], &context->CONTEXT_XMM6, 16);
    memcpy(&cpu->xmm[7], &context->CONTEXT_XMM7, 16);
    
    
    if (includeFPU && !cpu->thread->process->emulateFPU) {
        cpu->fpu.SetCW(*((U16*)&context->CONTEXT_FCW));
        cpu->fpu.SetSW(*((U16*)&context->CONTEXT_FSW));
        cpu->fpu.SetTagFromAbridged(context->CONTEXT_FTW);
        for (U32 i=0;i<8;i++) {
            if (!(context->CONTEXT_FTW & (1 << i))) {
                cpu->fpu.setReg(i, 0.0);
            } else {
                U32 index = (i-cpu->fpu.GetTop()) & 7;
                U64 low=0;
                S16 high=0;
                
                switch (index) {
                    case 0:
                        low = *CONTEXT_FPU_REG_0_LOW(context);
                        high = *CONTEXT_FPU_REG_0_HIGH(context);
                        break;
                    case 1:
                        low = *CONTEXT_FPU_REG_1_LOW(context);
                        high = *CONTEXT_FPU_REG_1_HIGH(context);
                        break;
                    case 2:
                        low = *CONTEXT_FPU_REG_2_LOW(context);
                        high = *CONTEXT_FPU_REG_2_HIGH(context);
                        break;
                    case 3:
                        low = *CONTEXT_FPU_REG_3_LOW(context);
                        high = *CONTEXT_FPU_REG_3_HIGH(context);
                        break;
                    case 4:
                        low = *CONTEXT_FPU_REG_4_LOW(context);
                        high = *CONTEXT_FPU_REG_4_HIGH(context);
                        break;
                    case 5:
                        low = *CONTEXT_FPU_REG_5_LOW(context);
                        high = *CONTEXT_FPU_REG_5_HIGH(context);
                        break;
                    case 6:
                        low = *CONTEXT_FPU_REG_6_LOW(context);
                        high = *CONTEXT_FPU_REG_6_HIGH(context);
                        break;
                    case 7:
                        low = *CONTEXT_FPU_REG_7_LOW(context);
                        high = *CONTEXT_FPU_REG_7_HIGH(context);
                        break;
                }
                double d = cpu->fpu.FLD80(low, high);
                cpu->fpu.setReg(i, d);
            }
        }
    }
}

void syncToException(x64CPU* cpu, ucontext_t* context, bool includeFPU) {
    context->CONTEXT_RAX = EAX;
    context->CONTEXT_RCX = ECX;
    context->CONTEXT_RDX = EDX;
    context->CONTEXT_RBX = EBX;
    context->CONTEXT_R11 = ESP;
    context->CONTEXT_RBP = EBP;
    context->CONTEXT_RSI = ESI;
    context->CONTEXT_RDI = EDI;
    context->CONTEXT_R14 = cpu->seg[SS].address;
    context->CONTEXT_R15 = cpu->seg[DS].address;
    cpu->fillFlags();
    context->CONTEXT_FLAGS &=~ CF | OF | PF | AF | SF | ZF;
    context->CONTEXT_FLAGS |= (cpu->flags & (CF | OF | PF | AF | SF | ZF));
    
    memcpy(&context->CONTEXT_XMM0, &cpu->xmm[0], 16);
    memcpy(&context->CONTEXT_XMM1, &cpu->xmm[1], 16);
    memcpy(&context->CONTEXT_XMM2, &cpu->xmm[2], 16);
    memcpy(&context->CONTEXT_XMM3, &cpu->xmm[3], 16);
    memcpy(&context->CONTEXT_XMM4, &cpu->xmm[4], 16);
    memcpy(&context->CONTEXT_XMM5, &cpu->xmm[5], 16);
    memcpy(&context->CONTEXT_XMM6, &cpu->xmm[6], 16);
    memcpy(&context->CONTEXT_XMM7, &cpu->xmm[7], 16);
    
    if (includeFPU && !cpu->thread->process->emulateFPU) {
#ifdef __MACH__
        *((U16*)&context->CONTEXT_FCW) = cpu->fpu.CW();
        *((U16*)&context->CONTEXT_FSW) = cpu->fpu.SW();
#else
        context->CONTEXT_FCW = cpu->fpu.CW();
        context->CONTEXT_FSW = cpu->fpu.SW();
#endif
        context->CONTEXT_FTW = cpu->fpu.GetAbridgedTag();
        for (U32 i=0;i<8;i++) {
            U32 index = (i-cpu->fpu.GetTop()) & 7;
            U64* low=NULL;
            S16* high=NULL;
            
            switch (index) {
                case 0:
                    low = CONTEXT_FPU_REG_0_LOW(context);
                    high = CONTEXT_FPU_REG_0_HIGH(context);
                    break;
                case 1:
                    low = CONTEXT_FPU_REG_1_LOW(context);
                    high = CONTEXT_FPU_REG_1_HIGH(context);
                    break;
                case 2:
                    low = CONTEXT_FPU_REG_2_LOW(context);
                    high = CONTEXT_FPU_REG_2_HIGH(context);
                    break;
                case 3:
                    low = CONTEXT_FPU_REG_3_LOW(context);
                    high = CONTEXT_FPU_REG_3_HIGH(context);
                    break;
                case 4:
                    low = CONTEXT_FPU_REG_4_LOW(context);
                    high = CONTEXT_FPU_REG_4_HIGH(context);
                    break;
                case 5:
                    low = CONTEXT_FPU_REG_5_LOW(context);
                    high = CONTEXT_FPU_REG_5_HIGH(context);
                    break;
                case 6:
                    low = CONTEXT_FPU_REG_6_LOW(context);
                    high = CONTEXT_FPU_REG_6_HIGH(context);
                    break;
                case 7:
                    low = CONTEXT_FPU_REG_7_LOW(context);
                    high = CONTEXT_FPU_REG_7_HIGH(context);
                    break;
            }
            
            if (!(context->CONTEXT_FTW & (1 << i))) {
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
    
    if (cpu!=(x64CPU*)context->CONTEXT_R13) {
        return;
    }
    
    std::function<void(DecodedOp*)> doSyncFrom = [cpu, context] (DecodedOp* op) {
        syncFromException(cpu, context, op?op->isFpuOp():true);
    };
    std::function<void(DecodedOp*)> doSyncTo = [cpu, context] (DecodedOp* op) {
        syncToException(cpu, context, op?op->isFpuOp():true);
    };
    
    bool readAccess = (((ucontext_t*)context)->CONTEXT_ERR & 1) == 0;

    U64 address = (U64)info->si_addr;
    U64 result = cpu->startException(address, readAccess, doSyncFrom, doSyncTo);
    if (result) {
        context->CONTEXT_RIP = result;
        return;
    }
    
    InException inException(cpu);
    if (info->si_signo == SIGILL) {
        if (context->CONTEXT_RSP & 0xf) {
            kpanic("seh_filter: bad stack alignment");
        }
        U64 rip = cpu->handleIllegalInstruction(context->CONTEXT_RIP);
        if (rip) {
            context->CONTEXT_RIP = rip;
            return;
        }
        return;
    } else if (info->si_signo == SIGBUS && info->si_code == BUS_ADRALN) {
        // :TODO: figure out how AC got set, I've only seen this while op logging
        context->CONTEXT_FLAGS &= ~AC;
        return;
    } else if ((info->si_signo == SIGBUS || info->si_signo == SIGSEGV) && ((context->CONTEXT_RIP & 0xFFFFFFFF00000000l)==(U64)cpu->thread->memory->executableMemoryId)) {
        U64 rip = cpu->handleAccessException(context->CONTEXT_RIP, address, readAccess, context->CONTEXT_RSI, context->CONTEXT_RDI, context->CONTEXT_R8, context->CONTEXT_R9, (U64*)&context->CONTEXT_R10, doSyncFrom, doSyncTo);
        if (rip) {
            context->CONTEXT_RIP = rip;
        }
        return;
    } else if (info->si_signo == SIGFPE && (info->si_code == FPE_INTDIV || info->si_code == FPE_FLTDIV)) {
        context->CONTEXT_RIP = cpu->handleDivByZero(doSyncFrom, doSyncTo);
        return;
    }
    kpanic("unhandled exception %d", info->si_signo);
}

U32 platformThreadCount = 0;

void* platformThreadProc(void* param) {
    static bool initializedHandler = false;
    if (!initializedHandler) {
        struct sigaction sa;
        sa.sa_sigaction=handler;
        sa.sa_flags=SA_SIGINFO;
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
