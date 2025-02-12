#include "boxedwine.h"

#ifdef BOXEDWINE_X64

#include "ksignal.h"
#include "../../source/emulation/cpu/x64/x64CPU.h"
#include "../../source/emulation/softmmu/kmemory_soft.h"
#include "../../source/emulation/cpu/normal/normalCPU.h"

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

void syncFromException(BtCPU* cpu, ucontext_t* context, bool includeFPU) {
    EAX = (U32)context->CONTEXT_RAX;
    ECX = (U32)context->CONTEXT_RCX;
    EDX = (U32)context->CONTEXT_RDX;
    EBX = (U32)context->CONTEXT_RBX;
    ESP = (U32)context->CONTEXT_R11;
    EBP = (U32)context->CONTEXT_RBP;
    ESI = (U32)context->CONTEXT_RSI;
    EDI = (U32)context->CONTEXT_RDI;
    //cpu->flags = (U32)context->CONTEXT_FLAGS;
    
    cpu->flags = (context->CONTEXT_FLAGS & (AF | CF | OF | SF | PF | ZF)) | (cpu->flags & DF); // DF is fully kept in sync, so don't override

    cpu->lazyFlags = FLAGS_NONE;
#if defined(BOXEDWINE_X64) && defined(BOXEDWINE_USE_SSE_FOR_FPU)
    if (cpu->fpu.isMMXInUse) {
        cpu->fpu.SetCW(context->CONTEXT_FCW);
        cpu->fpu.SetSW(context->CONTEXT_FSW);
        cpu->fpu.SetTagFromAbridged(context->CONTEXT_FTW);

        cpu->fpu.regs[0].signif = *CONTEXT_FPU_REG_0_LOW(context);
        cpu->fpu.regs[1].signif = *CONTEXT_FPU_REG_1_LOW(context);
        cpu->fpu.regs[2].signif = *CONTEXT_FPU_REG_2_LOW(context);
        cpu->fpu.regs[3].signif = *CONTEXT_FPU_REG_3_LOW(context);
        cpu->fpu.regs[4].signif = *CONTEXT_FPU_REG_4_LOW(context);
        cpu->fpu.regs[5].signif = *CONTEXT_FPU_REG_5_LOW(context);
        cpu->fpu.regs[6].signif = *CONTEXT_FPU_REG_6_LOW(context);
        cpu->fpu.regs[7].signif = *CONTEXT_FPU_REG_7_LOW(context);
    }

    memcpy(&cpu->xmm[0], &context->CONTEXT_XMM0, 16);
    memcpy(&cpu->xmm[1], &context->CONTEXT_XMM1, 16);
    memcpy(&cpu->xmm[2], &context->CONTEXT_XMM2, 16);
    memcpy(&cpu->xmm[3], &context->CONTEXT_XMM3, 16);
    memcpy(&cpu->xmm[4], &context->CONTEXT_XMM4, 16);
    memcpy(&cpu->xmm[5], &context->CONTEXT_XMM5, 16);
    memcpy(&cpu->xmm[6], &context->CONTEXT_XMM6, 16);
    memcpy(&cpu->xmm[7], &context->CONTEXT_XMM7, 16);
#endif
}

class InException {
public:
    InException(BtCPU* cpu) : cpu(cpu) { this->cpu->inException = true; }
    ~InException() { this->cpu->inException = false; }
    BtCPU* cpu;
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
    KMemoryData* mem = getMemData(currentThread->memory);

    if (!currentThread) {
        return;
    }    
    BtCPU* cpu = (BtCPU*)currentThread->cpu;
    if (cpu != (BtCPU*)context->CONTEXT_R13) {
        return;
    }
    x64CPU* x64Cpu = (x64CPU*)cpu;

    syncFromException(cpu, context, true);

    cpu->exceptionReadAddress = (((ucontext_t*)context)->CONTEXT_ERR & 2) == 0;
    cpu->exceptionAddress = (U64)info->si_addr;
    cpu->exceptionSigNo = info->si_signo;
    cpu->exceptionSigCode = info->si_code;
    x64Cpu->exceptionIp = context->CONTEXT_RIP;
    cpu->eip.u32 = getMemData(cpu->memory)->codeCache.getEipFromHost((U8*)context->CONTEXT_RIP) - cpu->seg[CS].address;

    context->CONTEXT_RIP = (U64)cpu->thread->process->runSignalAddress;
}

int getFPUCode(int code) {
    switch (code) {
    case FPE_INTDIV: return K_FPE_INTDIV;
    case FPE_INTOVF: return K_FPE_INTOVF;
    case FPE_FLTDIV: return K_FPE_FLTDIV;
    case FPE_FLTOVF: return K_FPE_FLTOVF;
    case FPE_FLTUND: return K_FPE_FLTUND;
    case FPE_FLTRES: return K_FPE_FLTRES;
    case FPE_FLTINV: return K_FPE_FLTINV;
    default: klog("getFPUCode unhandled code %d", code); return 0;
    }
}

extern bool writesFlags[InstructionCount];

void signalHandler() {
    BOXEDWINE_CRITICAL_SECTION;
    KThread* currentThread = KThread::currentThread();
    x64CPU* cpu = (x64CPU*)currentThread->cpu;
    KMemoryData* mem = getMemData(currentThread->memory);
    U64 fpu[8];

    U64 result = cpu->startException(cpu->exceptionAddress, cpu->exceptionReadAddress);
    if (result) {
        cpu->returnHostAddress = result;
        return;
    }
    InException e(cpu);
    if (cpu->exceptionSigNo == SIGFPE) {
        int code = getFPUCode(cpu->exceptionSigCode);
        cpu->returnHostAddress = cpu->handleFpuException(code);
        return;
    }
    if (cpu->exceptionSigNo == SIGSEGV || cpu->exceptionSigNo == SIGBUS) {
        DecodedOp* op = NormalCPU::decodeSingleOp(cpu, cpu->getEipAddress());
        if (writesFlags[op->inst]) {
            cpu->flags = ((cpu->instructionStoredFlags >> 8) & 0xFF) | (cpu->flags & DF) | ((cpu->instructionStoredFlags & 0xFF) ? OF : 0);
        }
#ifndef BOXEDWINE_USE_SSE_FOR_FPU
        cpu->loadFxState(op->inst);        
#endif
        bool saveFxState = true;
        cpu->returnHostAddress = cpu->handleAccessException(op);
        cpu->fillFlags();        
        cpu->updateX64Flags();
#ifndef BOXEDWINE_USE_SSE_FOR_FPU
        if (saveFxState) {
            cpu->saveToFxState(op->inst);
        }
#endif
        op->dealloc(true);
        return;
    }
    kpanic("unhandled exception %d", cpu->exceptionSigNo);
}

#endif
