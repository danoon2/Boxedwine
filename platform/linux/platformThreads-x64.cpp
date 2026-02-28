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

#if defined(BOXEDWINE_JIT_X64) && defined(BOXEDWINE_HOST_EXCEPTIONS)

#include "../../source/emulation/softmmu/kmemory_soft.h"

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

extern U8 regCache[8];
extern U8 xmmCache[8];

void syncFromException(CPU* cpu, ucontext_t* context) {
#ifdef BOXEDWINE_64
    for (U32 i = 0; i < 8; i++) {
        if (regCache[i] != 0xFF) {
            switch (regCache[i]) {
            case 0: cpu->reg[i].u32 = (U32)context->CONTEXT_RAX; break;
            case 1: cpu->reg[i].u32 = (U32)context->CONTEXT_RCX; break;
            case 2: cpu->reg[i].u32 = (U32)context->CONTEXT_RDX; break;
            case 3: cpu->reg[i].u32 = (U32)context->CONTEXT_RBX; break;
            case 10: cpu->reg[i].u32 = (U32)context->CONTEXT_R10; break;
            case 5: cpu->reg[i].u32 = (U32)context->CONTEXT_RBP; break;
            case 6: cpu->reg[i].u32 = (U32)context->CONTEXT_RSI; break;
            case 7: cpu->reg[i].u32 = (U32)context->CONTEXT_RDI; break;
            default:
                kpanic("Linux x64 syncFromException");
            }
        }
    }
    memcpy(&cpu->xmm[0], &context->CONTEXT_XMM0, 16);
    memcpy(&cpu->xmm[1], &context->CONTEXT_XMM1, 16);
    memcpy(&cpu->xmm[2], &context->CONTEXT_XMM2, 16);
    memcpy(&cpu->xmm[3], &context->CONTEXT_XMM3, 16);
    memcpy(&cpu->xmm[4], &context->CONTEXT_XMM4, 16);
    memcpy(&cpu->xmm[5], &context->CONTEXT_XMM5, 16);
    memcpy(&cpu->xmm[6], &context->CONTEXT_XMM6, 16);
    memcpy(&cpu->xmm[7], &context->CONTEXT_XMM7, 16);

    for (U32 i = 0; i < 8; i++) {
        if (xmmCache[i] != 0xFF) {
            switch (xmmCache[i]) {
            case 0:
                memcpy(&cpu->xmm[i], &context->CONTEXT_XMM0, 16);
                break;
            case 1:
                memcpy(&cpu->xmm[i], &context->CONTEXT_XMM1, 16);
                break;
            case 2:
                memcpy(&cpu->xmm[i], &context->CONTEXT_XMM2, 16);
                break;
            case 3:
                memcpy(&cpu->xmm[i], &context->CONTEXT_XMM3, 16);
                break;
            case 4:
                memcpy(&cpu->xmm[i], &context->CONTEXT_XMM4, 16);
                break;
            case 5:
                memcpy(&cpu->xmm[i], &context->CONTEXT_XMM5, 16);
                break;
            case 6:
                memcpy(&cpu->xmm[i], &context->CONTEXT_XMM6, 16);
                break;
            case 7:
                memcpy(&cpu->xmm[i], &context->CONTEXT_XMM7, 16);
                break;
            default:
                kpanic("Linux x64 syncFromException xmm");
            }
        }
    }
#else
#error Linux x86 does not support BOXEDWINE_HOST_EXCEPTIONS
#endif    
}

class InException {
public:
    InException(CPU* cpu) : cpu(cpu) { this->cpu->inException = true; }
    ~InException() { this->cpu->inException = false; }
    CPU* cpu;
};

U32 exceptionCount;

#include <signal.h>
#include <pthread.h>

// this will quickly store the info then exit to signalHandler() to perform the logic there
void platformHandler(int sig, siginfo_t* info, void* vcontext) {
    exceptionCount++;
    ucontext_t* context = (ucontext_t*)vcontext;
    if (info->si_signo == SIGBUS) {
        // setup_openttd_14.1_(32bit)_(73101).exe, instruction 0x42bdd5 which is just a normal 32-bit read to a register can, but not always, trigger this.
        // I'm not sure why the alignment check flag gets enabled.  Windows platformThreads.cpp / seh_filter also implements this
        context->CONTEXT_FLAGS &= ~AC;
        return;
    }
    KThread* currentThread = KThread::currentThread();

    if (!currentThread) {
        return;
    }    
    CPU* cpu = (CPU*)currentThread->cpu;
    if (cpu != (CPU*)context->CONTEXT_R13) {
        return;
    }

    syncFromException(cpu, context);

    cpu->exceptionReadAddress = (((ucontext_t*)context)->CONTEXT_ERR & 2) == 0;
    cpu->exceptionAddress = (U64)info->si_addr;
    cpu->exceptionSigNo = info->si_signo;
    cpu->exceptionSigCode = info->si_code;
    cpu->exceptionIp = context->CONTEXT_RIP;

    context->CONTEXT_RIP = (U64)cpu->thread->process->signalHandler;
}

void signalHandler(CPU* cpu) {
    void* result = cpu->startException(cpu->exceptionAddress, cpu->exceptionReadAddress);
    if (result) {
        cpu->returnHostAddress = result;
        return;
    }
    InException e(cpu);

    if (cpu->exceptionSigNo == SIGSEGV || cpu->exceptionSigNo == SIGBUS) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->memory->mutex); // jitCache needs this
        U32 eip = 0;
        if (!getMemData(cpu->memory)->findOpFromJitAddress((U8*)cpu->exceptionIp, eip)) {
            klog("probably about to crash, could not find emulation instruction that caused exception");
            cpu->returnHostAddress = cpu->thread->process->blockExit;
        } else {            
            cpu->eip.u32 = eip;
            DecodedOp* op = cpu->getNextOp();
            if (op->flags2 & OP_FLAG2_SAVED_TMP_REG) {
                if (op->inst == MaskmovqEDIMmxMmx || op->inst == MaskmovdquE128XmmXmm) {
                    cpu->reg[7].u32 = cpu->tmpReg;
                }
            }
            void* result = cpu->handleAccessException(op);
            if (cpu->nextOp->pfnJitCode) {
                cpu->returnHostAddress = cpu->nextOp->pfnJitCode;
            } else {
                cpu->returnHostAddress = result;
            }
        }
        return;
    }
    kpanic_fmt("unhandled exception %d", cpu->exceptionSigNo);
}

#endif
