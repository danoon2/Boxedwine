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
#include <windows.h>
#include "../source/emulation/softmmu/kmemory_soft.h"
#include "../source/emulation/cpu/normal/normalCPU.h"
#include "ksignal.h"
#include "../source/emulation/cpu/binaryTranslation/btCpu.h"

#ifdef BOXEDWINE_HOST_EXCEPTIONS
static PVOID pHandler;

class InException {
public:
    InException(CPU* cpu) : cpu(cpu) { this->cpu->inException = true; }
    ~InException() { this->cpu->inException = false; }
    CPU* cpu;
};

extern U8 regCache[8];
extern U8 xmmCache[8];

void syncToException(struct _EXCEPTION_POINTERS* ep) {
    CPU* cpu = KThread::currentThread()->cpu;
#ifdef BOXEDWINE_64
    for (U32 i = 0; i < 8; i++) {
        if (regCache[i] != 0xFF) {
            switch (regCache[i]) {
            case 0: ep->ContextRecord->Rax = cpu->reg[i].u32; break;
            case 1: ep->ContextRecord->Rcx = cpu->reg[i].u32; break;
            case 2: ep->ContextRecord->Rdx = cpu->reg[i].u32; break;
            case 3: ep->ContextRecord->Rbx = cpu->reg[i].u32; break;
            case 10: ep->ContextRecord->R10 = cpu->reg[i].u32; break;
            case 5: ep->ContextRecord->Rbp = cpu->reg[i].u32; break;
            case 6: ep->ContextRecord->Rsi = cpu->reg[i].u32; break;
            case 7: ep->ContextRecord->Rdi = cpu->reg[i].u32; break;
            default:
                kpanic("Windows x86 syncToException");
            }
        }
    }
    for (U32 i = 0; i < 8; i++) {
        if (xmmCache[i] != 0xFF) {
            switch (xmmCache[i]) {
            case 0: 
                ep->ContextRecord->Xmm0.Low = cpu->xmm[i].pi.u64[0];
                ep->ContextRecord->Xmm0.High = cpu->xmm[i].pi.u64[1];
                break;
            case 1:
                ep->ContextRecord->Xmm1.Low = cpu->xmm[i].pi.u64[0];
                ep->ContextRecord->Xmm1.High = cpu->xmm[i].pi.u64[1];
                break;
            case 2:
                ep->ContextRecord->Xmm2.Low = cpu->xmm[i].pi.u64[0];
                ep->ContextRecord->Xmm2.High = cpu->xmm[i].pi.u64[1];
                break;
            case 3:
                ep->ContextRecord->Xmm3.Low = cpu->xmm[i].pi.u64[0];
                ep->ContextRecord->Xmm3.High = cpu->xmm[i].pi.u64[1];
                break;
            case 4:
                ep->ContextRecord->Xmm4.Low = cpu->xmm[i].pi.u64[0];
                ep->ContextRecord->Xmm4.High = cpu->xmm[i].pi.u64[1];
                break;
            case 5:
                ep->ContextRecord->Xmm5.Low = cpu->xmm[i].pi.u64[0];
                ep->ContextRecord->Xmm5.High = cpu->xmm[i].pi.u64[1];
                break;
            case 6:
                ep->ContextRecord->Xmm6.Low = cpu->xmm[i].pi.u64[0];
                ep->ContextRecord->Xmm6.High = cpu->xmm[i].pi.u64[1];
                break;
            case 7:
                ep->ContextRecord->Xmm7.Low = cpu->xmm[i].pi.u64[0];
                ep->ContextRecord->Xmm7.High = cpu->xmm[i].pi.u64[1];
                break;
            default:
                kpanic("Windows x86 syncToException xmm");
            }
        }
    }
#else
    for (U32 i = 0; i < 8; i++) {
        if (regCache[i] != 0xFF) {
            switch (regCache[i]) {
            case 0: ep->ContextRecord->Eax = cpu->reg[i].u32; break;
            case 1: ep->ContextRecord->Ecx = cpu->reg[i].u32; break;
            case 2: ep->ContextRecord->Edx = cpu->reg[i].u32; break;
            case 3: ep->ContextRecord->Ebx = cpu->reg[i].u32; break;
            case 4: ep->ContextRecord->Esp = cpu->reg[i].u32; break;
            case 5: ep->ContextRecord->Ebp = cpu->reg[i].u32; break;
            case 6: ep->ContextRecord->Esi = cpu->reg[i].u32; break;
            case 7: ep->ContextRecord->Edi = cpu->reg[i].u32; break;
            }
        }
    }
    XSAVE_FORMAT* p = (XSAVE_FORMAT*)ep->ContextRecord->ExtendedRegisters;

    for (int i = 0; i < 8; i++) {
        if (xmmCache[i] != 0xFF) {
            p->XmmRegisters[xmmCache[i]].Low = cpu->xmm[i].pi.u64[0];
            p->XmmRegisters[xmmCache[i]].High = cpu->xmm[i].pi.u64[1];
        }
    }
#endif    
}

void syncFromException(struct _EXCEPTION_POINTERS* ep) {
    CPU* cpu = KThread::currentThread()->cpu;
#ifdef BOXEDWINE_64
    for (U32 i = 0; i < 8; i++) {
        if (regCache[i] != 0xFF) {
            switch (regCache[i]) {
            case 0: cpu->reg[i].u32 = (U32)ep->ContextRecord->Rax; break;
            case 1: cpu->reg[i].u32 = (U32)ep->ContextRecord->Rcx; break;
            case 2: cpu->reg[i].u32 = (U32)ep->ContextRecord->Rdx; break;
            case 3: cpu->reg[i].u32 = (U32)ep->ContextRecord->Rbx; break;
            case 10: cpu->reg[i].u32 = (U32)ep->ContextRecord->R10; break;
            case 5: cpu->reg[i].u32 = (U32)ep->ContextRecord->Rbp; break;
            case 6: cpu->reg[i].u32 = (U32)ep->ContextRecord->Rsi; break;
            case 7: cpu->reg[i].u32 = (U32)ep->ContextRecord->Rdi; break;
            default:
                kpanic("Windows x86 syncFromException");
            }
        }
    }
    for (U32 i = 0; i < 8; i++) {
        if (xmmCache[i] != 0xFF) {
            switch (xmmCache[i]) {
            case 0:
                cpu->xmm[i].pi.u64[0] = ep->ContextRecord->Xmm0.Low;
                cpu->xmm[i].pi.u64[1] = ep->ContextRecord->Xmm0.High;
                break;
            case 1:
                cpu->xmm[i].pi.u64[0] = ep->ContextRecord->Xmm1.Low;
                cpu->xmm[i].pi.u64[1] = ep->ContextRecord->Xmm1.High;
                break;
            case 2:
                cpu->xmm[i].pi.u64[0] = ep->ContextRecord->Xmm2.Low;
                cpu->xmm[i].pi.u64[1] = ep->ContextRecord->Xmm2.High;
                break;
            case 3:
                cpu->xmm[i].pi.u64[0] = ep->ContextRecord->Xmm3.Low;
                cpu->xmm[i].pi.u64[1] = ep->ContextRecord->Xmm3.High;
                break;
            case 4:
                cpu->xmm[i].pi.u64[0] = ep->ContextRecord->Xmm4.Low;
                cpu->xmm[i].pi.u64[1] = ep->ContextRecord->Xmm4.High;
                break;
            case 5:
                cpu->xmm[i].pi.u64[0] = ep->ContextRecord->Xmm5.Low;
                cpu->xmm[i].pi.u64[1] = ep->ContextRecord->Xmm5.High;
                break;
            case 6:
                cpu->xmm[i].pi.u64[0] = ep->ContextRecord->Xmm6.Low;
                cpu->xmm[i].pi.u64[1] = ep->ContextRecord->Xmm6.High;
                break;
            case 7:
                cpu->xmm[i].pi.u64[0] = ep->ContextRecord->Xmm7.Low;
                cpu->xmm[i].pi.u64[1] = ep->ContextRecord->Xmm7.High;
                break;
            default:
                kpanic("Windows x86 syncFromException xmm");
            }
        }
    }
#else
    for (U32 i = 0; i < 8; i++) {
        if (regCache[i] != 0xFF) {
            switch (regCache[i]) {
            case 0: cpu->reg[i].u32 = ep->ContextRecord->Eax; break;
            case 1: cpu->reg[i].u32 = ep->ContextRecord->Ecx; break;
            case 2: cpu->reg[i].u32 = ep->ContextRecord->Edx; break;
            case 3: cpu->reg[i].u32 = ep->ContextRecord->Ebx; break;
            case 4: cpu->reg[i].u32 = ep->ContextRecord->Esp; break;
            case 5: cpu->reg[i].u32 = ep->ContextRecord->Ebp; break;
            case 6: cpu->reg[i].u32 = ep->ContextRecord->Esi; break;
            case 7: cpu->reg[i].u32 = ep->ContextRecord->Edi; break;
            }
        }
    }
    XSAVE_FORMAT* p = (XSAVE_FORMAT*)ep->ContextRecord->ExtendedRegisters;

    for (int i = 0; i < 8; i++) {
        if (xmmCache[i] != 0xFF) {
            cpu->xmm[i].pi.u64[0] = p->XmmRegisters[xmmCache[i]].Low;
            cpu->xmm[i].pi.u64[1] = p->XmmRegisters[xmmCache[i]].High;
        }
    }
#endif    
}
LONG WINAPI seh_filter(struct _EXCEPTION_POINTERS* ep) {
    //BOXEDWINE_CRITICAL_SECTION;
    if (ep->ContextRecord->EFlags & AC) {
        // the comment on the next line was for the x64 binary translator, I'm not sure if it still applies to the JIT
        // :TODO: not sure what causes this, seen it in winroids
        ep->ContextRecord->EFlags &= ~AC;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    KThread* currentThread = KThread::currentThread();
    if (!currentThread) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    CPU* cpu = currentThread->cpu;
#ifdef _M_ARM64
#define CPU_REG X19
#define REG_IP Pc
#elif defined(BOXEDWINE_64)
#define CPU_REG R13
#define REG_IP Rip
#define SET_REG_IP(r) Rip = (U64)r
#else
#define CPU_REG Edi
#define REG_IP Eip
#define SET_REG_IP(r) Eip = (U32)r
#endif
    if (cpu != (CPU*)ep->ContextRecord->CPU_REG) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    bool inBinaryTranslator = cpu->memory->isCode((U8*)ep->ContextRecord->REG_IP);    
    if (!inBinaryTranslator) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    syncFromException(ep);
    bool readException = ep->ExceptionRecord->ExceptionInformation[0] == 0;
    void* result = cpu->startException((U32)ep->ExceptionRecord->ExceptionInformation[1], readException);
    if (result) {
        syncToException(ep);
        ep->ContextRecord->SET_REG_IP(result);
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    InException inException(cpu);

    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || ep->ExceptionRecord->ExceptionCode == EXCEPTION_DATATYPE_MISALIGNMENT) {
        U32 eip = 0;
        DecodedOp* op = getMemData(cpu->memory)->findOpFromJitAddress((void*)ep->ContextRecord->REG_IP, eip);
        if (!op) {
            return EXCEPTION_CONTINUE_SEARCH;
        }        
#ifdef _DEBUG
        if (eip != cpu->eip.u32) {
            kpanic_fmt("%x/%x %s", cpu->eip.u32, eip, op->name());
        }
        DecodedOp* op1 = cpu->getNextOp();
        if (op1 != op) {
            kpanic_fmt("%x/%x %s/%s", cpu->eip.u32, eip, op1->name(), op->name());
        }
#endif
        cpu->eip.u32 = eip;
#ifndef BOXEDWINE_64
        if (op->inst == Movsd || op->inst == Movsw || op->inst == Movsb) {
            cpu->reg[6].u32 = ep->ContextRecord->Esi;
            if (!readException && (op->flags2 & OP_FLAG2_WRITE_SAVED_ADDRESS)) {
                cpu->reg[7].u32 = cpu->tmpReg;
            } else {
                cpu->reg[7].u32 = ep->ContextRecord->Edx;
            }
            if (op->repNotZero || op->repZero) {
                cpu->reg[1].u32 = ep->ContextRecord->Ecx;
            }
        } if (op->inst == Cmpsb || op->inst == Cmpsw || op->inst == Cmpsd) {
            cpu->reg[6].u32 = ep->ContextRecord->Esi;
            cpu->reg[7].u32 = ep->ContextRecord->Edx;
            // cmps doesn't cache ecx
        } else if (op->inst == Stosb || op->inst == Stosw || op->inst == Stosd || op->inst == Scasb || op->inst == Scasw || op->inst == Scasd) {
            if (!readException && (op->flags2 & OP_FLAG2_WRITE_SAVED_ADDRESS)) {
                cpu->reg[7].u32 = cpu->tmpReg;
            } else {
                cpu->reg[7].u32 = ep->ContextRecord->Edx;
            }
            if (op->repNotZero || op->repZero) {
                cpu->reg[1].u32 = ep->ContextRecord->Ecx;
            }
        } else if (op->inst == Lodsb || op->inst == Lodsw || op->inst == Lodsd) {
            cpu->reg[6].u32 = ep->ContextRecord->Esi;
            if (op->repNotZero || op->repZero) {
                cpu->reg[1].u32 = ep->ContextRecord->Ecx;
            }
        }
#endif
        void* result = cpu->handleAccessException(op);
        if (cpu->nextOp->pfnJitCode) {
            ep->ContextRecord->SET_REG_IP(cpu->nextOp->pfnJitCode);
        } else {
            ep->ContextRecord->SET_REG_IP(result);
        }
        syncToException(ep);
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (ep->ExceptionRecord->ExceptionCode == STATUS_ILLEGAL_INSTRUCTION) {
        // this is weird, so far I have only seen it on the first instruction of a block, which is an instruction that shouldn't fail, maybe the clearCache wasn't done in time and another thread starting using it?
        klog("Instruction cache miss?");
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif
void platformInitExceptionHandling() {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
    if (!pHandler) {
        pHandler = AddVectoredExceptionHandler(1, seh_filter);
    }
#endif
}

#if defined(BOXEDWINE_BINARY_TRANSLATOR)
#include "../source/emulation/cpu/x64/x64CPU.h"
#include "../source/emulation/cpu/armv8bt/armv8btCPU.h"
#ifdef BOXEDWINE_X64
void syncFromException(struct _EXCEPTION_POINTERS* ep) {
    x64CPU* cpu = (x64CPU*)KThread::currentThread()->cpu;
    EAX = (U32)ep->ContextRecord->Rax;
    ECX = (U32)ep->ContextRecord->Rcx;
    EDX = (U32)ep->ContextRecord->Rdx;
    EBX = (U32)ep->ContextRecord->Rbx;
    ESP = (U32)ep->ContextRecord->R11;
    EBP = (U32)ep->ContextRecord->Rbp;
    ESI = (U32)ep->ContextRecord->Rsi;
    EDI = (U32)ep->ContextRecord->Rdi;

    cpu->flags = (ep->ContextRecord->EFlags & (AF | CF | OF | SF | PF | ZF)) | (cpu->flags & DF); // DF is fully kept in sync, so don't override
    cpu->lazyFlagType = FLAGS_NONE;    

#ifdef BOXEDWINE_USE_SSE_FOR_FPU
    if (cpu->fpu.isMMXInUse) {
#else
    if (!cpu->thread->process->emulateFPU || cpu->fpu.isMMXInUse) {
#endif
        cpu->fpu.SetCW(ep->ContextRecord->FltSave.ControlWord);
        cpu->fpu.SetSW(ep->ContextRecord->FltSave.StatusWord);
        cpu->fpu.SetTagFromAbridged(ep->ContextRecord->FltSave.TagWord);
    }
    for (int i = 0; i < 8; i++) {
        cpu->xmm[i].pi.u64[0] = ep->ContextRecord->FltSave.XmmRegisters[i].Low;
        cpu->xmm[i].pi.u64[1] = ep->ContextRecord->FltSave.XmmRegisters[i].High;
        if (cpu->fpu.isMMXInUse) {
            cpu->fpu.regs[i].signif = ep->ContextRecord->FltSave.FloatRegisters[i].Low;
            cpu->fpu.regs[i].signExp = (U16)ep->ContextRecord->FltSave.FloatRegisters[i].High;
        } 
#ifndef BOXEDWINE_USE_SSE_FOR_FPU
        else if (!cpu->thread->process->emulateFPU) {
            U32 index = (i - cpu->fpu.GetTop()) & 7;
            cpu->fpu.regs[i].signif = ep->ContextRecord->FltSave.FloatRegisters[index].Low;
            cpu->fpu.regs[i].signExp = (U16)ep->ContextRecord->FltSave.FloatRegisters[index].High;
            cpu->fpu.isRegCached[i] = false;
        }
#endif        
    }        
}

void syncToException(struct _EXCEPTION_POINTERS* ep, bool includeFPU = true) {
    BtCPU* cpu = (BtCPU*)KThread::currentThread()->cpu;
    ep->ContextRecord->Rax = EAX;
    ep->ContextRecord->Rcx = ECX;
    ep->ContextRecord->Rdx = EDX;
    ep->ContextRecord->Rbx = EBX;
    ep->ContextRecord->R11 = ESP;
    ep->ContextRecord->Rbp = EBP;
    ep->ContextRecord->Rsi = ESI;
    ep->ContextRecord->Rdi = EDI;
    cpu->fillFlags();
    ep->ContextRecord->EFlags &= ~(AF | CF | OF | SF | PF | ZF);
    ep->ContextRecord->EFlags |= (cpu->flags & (AF | CF | OF | SF | PF | ZF));

    if (!includeFPU) {
        for (int i = 0; i < 8; i++) {
            ep->ContextRecord->FltSave.XmmRegisters[i].Low = cpu->xmm[i].pi.u64[0];
            ep->ContextRecord->FltSave.XmmRegisters[i].High = cpu->xmm[i].pi.u64[1];
        }
        return;
    }

#ifdef BOXEDWINE_USE_SSE_FOR_FPU
    if (cpu->fpu.isMMXInUse) {
#else
    if (!cpu->thread->process->emulateFPU || cpu->fpu.isMMXInUse) {
#endif
        ep->ContextRecord->FltSave.ControlWord = cpu->fpu.CW();
        ep->ContextRecord->FltSave.StatusWord = cpu->fpu.SW();
        ep->ContextRecord->FltSave.TagWord = cpu->fpu.GetAbridgedTag(cpu);
    }

    for (int i = 0; i < 8; i++) {
        ep->ContextRecord->FltSave.XmmRegisters[i].Low = cpu->xmm[i].pi.u64[0];
        ep->ContextRecord->FltSave.XmmRegisters[i].High = cpu->xmm[i].pi.u64[1];

        if (cpu->fpu.isMMXInUse) {
            ep->ContextRecord->FltSave.FloatRegisters[i].Low = cpu->fpu.regs[i].signif;
            ep->ContextRecord->FltSave.FloatRegisters[i].High = 0xffff;
        }
#ifndef BOXEDWINE_USE_SSE_FOR_FPU
        else if (!cpu->thread->process->emulateFPU) {
            U32 index = (i - cpu->fpu.GetTop()) & 7;
            // don't access fpu.regs directly incase something was cached
            cpu->fpu.ST80(i, (U64*)&ep->ContextRecord->FltSave.FloatRegisters[index].Low, (U64*)&ep->ContextRecord->FltSave.FloatRegisters[index].High);
        }
#endif
    }
}

class InException {
public:
    InException(BtCPU* cpu) : cpu(cpu) { this->cpu->inException = true; }
    ~InException() { this->cpu->inException = false; }
    BtCPU* cpu;
};

U32 exceptionCount;

int getFpuException(int control, int status) {
    if ((status & 0x02) && (control & 0x02)) return EXCEPTION_FLT_DENORMAL_OPERAND;  /* DE flag */
    if ((status & 0x04) && (control & 0x04)) return K_FPE_FLTDIV;    /* ZE flag */
    if ((status & 0x08) && (control & 0x08)) return K_FPE_FLTOVF;          /* OE flag */
    if ((status & 0x10) && (control & 0x10)) return K_FPE_FLTUND;         /* UE flag */
    if ((status & 0x20) && (control & 0x20)) return K_FPE_FLTRES;    /* PE flag */
    return K_FPE_FLTINV;
}

extern bool writesFlags[InstructionCount];

LONG WINAPI seh_filter(struct _EXCEPTION_POINTERS* ep) {
    BOXEDWINE_CRITICAL_SECTION;
    exceptionCount++;
    KThread* currentThread = KThread::currentThread();
    if (!currentThread) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    x64CPU* cpu = (x64CPU*)currentThread->cpu;
    if (ep->ContextRecord->EFlags & AC) {
        // :TODO: not sure what causes this, seen it in winroids
        ep->ContextRecord->EFlags &= ~AC;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (cpu != (BtCPU*)ep->ContextRecord->R13) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    bool inBinaryTranslator = cpu->memory->isCode((U8*)ep->ContextRecord->Rip);

    if (!inBinaryTranslator) {
        if (cpu->exitToStartThreadLoop) {
            ep->ContextRecord->Rip = cpu->exitToStartThreadLoop;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        // might be in a c++ exception
        //
        // motorhead installer will trigger this a few time
        // caesar 3 installer will trigger this when it exits
        return EXCEPTION_CONTINUE_SEARCH;
    }
    syncFromException(ep);
    U64 result = cpu->startException(ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[0] == 0);
    if (result) {
        syncToException(ep, false);
        ep->ContextRecord->Rip = result;
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    InException inException(cpu);

    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_STACK_CHECK) {
        kpanic("EXCEPTION_FLT_STACK_CHECK");
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_DIVIDE_BY_ZERO) {
        int code = getFpuException(ep->ContextRecord->FltSave.ControlWord, ep->ContextRecord->FltSave.StatusWord);

        ep->ContextRecord->Rip = cpu->handleFpuException(code);
        syncToException(ep);
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_INEXACT_RESULT) {
        ep->ContextRecord->Rip = cpu->handleFpuException(K_FPE_FLTRES);
        syncToException(ep);
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (ep->ExceptionRecord->ExceptionCode == STATUS_INTEGER_DIVIDE_BY_ZERO) {
        ep->ContextRecord->Rip = cpu->handleFpuException(K_FPE_INTDIV);
        syncToException(ep);
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        // should only be triggered when a read/write crosses a page boundry, the page has a custom read/write handler or we are jumping to a new eip that hasn't been decoded
        DecodedOp* op;
        try {
            op = cpu->getNextOp(); // can throw an exception if the eip is invalid
            if (writesFlags[op->inst]) {
                cpu->flags = ((cpu->instructionStoredFlags >> 8) & 0xFF) | (cpu->flags & DF) | ((cpu->instructionStoredFlags & 0xFF) ? OF : 0);
            }
        } catch (...) {
            op = cpu->getNextOp();
        }
        ep->ContextRecord->Rip = cpu->handleAccessException(op);
        // X64Asm::checkMemory disables the code path for fpu and mmx to throw exceptions
        //
        // I'm not sure why, but setting the fpu state causes some stability issues so for now this is the band-aid
        syncToException(ep, op->isFpuOp() || op->isMmxOp());
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
#else
void syncFromException(struct _EXCEPTION_POINTERS* ep, bool includeFPU) {
    Armv8btCPU* cpu = (Armv8btCPU*)KThread::currentThread()->cpu;
    EAX = (U32)ep->ContextRecord->X0;
    ECX = (U32)ep->ContextRecord->X1;
    EDX = (U32)ep->ContextRecord->X2;
    EBX = (U32)ep->ContextRecord->X3;
    ESP = (U32)ep->ContextRecord->X4;
    EBP = (U32)ep->ContextRecord->X5;
    ESI = (U32)ep->ContextRecord->X6;
    EDI = (U32)ep->ContextRecord->X7;

    cpu->flags = (U32)ep->ContextRecord->X8;
    cpu->lazyFlagType = FLAGS_NONE;

    for (int i = 0; i < 8; i++) {
        cpu->xmm[i].pi.u64[0] = ep->ContextRecord->V[i].Low;
        cpu->xmm[i].pi.u64[1] = ep->ContextRecord->V[i].High;
        if (cpu->fpu.isMMXInUse) {
            cpu->fpu.getMMX(i)->q = ep->ContextRecord->V[i + 8].Low;
        }
    }
}

void syncToException(struct _EXCEPTION_POINTERS* ep, bool includeFPU) {
    BtCPU* cpu = (BtCPU*)KThread::currentThread()->cpu;
    ep->ContextRecord->X0 = EAX;
    ep->ContextRecord->X1 = ECX;
    ep->ContextRecord->X2 = EDX;
    ep->ContextRecord->X3 = EBX;
    ep->ContextRecord->X4 = ESP;
    ep->ContextRecord->X5 = EBP;
    ep->ContextRecord->X6 = ESI;
    ep->ContextRecord->X7 = EDI;
    cpu->fillFlags();
    ep->ContextRecord->X8 = cpu->flags;

    for (int i = 0; i < 8; i++) {
        ep->ContextRecord->V[i].Low = cpu->xmm[i].pi.u64[0];
        ep->ContextRecord->V[i].High = cpu->xmm[i].pi.u64[1];
        if (cpu->fpu.isMMXInUse) {
            ep->ContextRecord->V[i + 8].Low = cpu->fpu.getMMX(i)->q;
        }
    }

    ep->ContextRecord->X22 = cpu->seg[ES].address;
    ep->ContextRecord->X23 = cpu->seg[CS].address;
    ep->ContextRecord->X24 = cpu->seg[SS].address;
    ep->ContextRecord->X25 = cpu->seg[DS].address;
    ep->ContextRecord->X26 = cpu->seg[FS].address;
    ep->ContextRecord->X27 = cpu->seg[GS].address;
    ep->ContextRecord->X28 = cpu->stackMask;
}

class InException {
public:
    InException(BtCPU* cpu) : cpu(cpu) { this->cpu->inException = true; }
    ~InException() { this->cpu->inException = false; }
    BtCPU* cpu;
};

U32 exceptionCount;

int getFpuException(int control, int status) {
    if ((status & 0x02) && (control & 0x02)) return EXCEPTION_FLT_DENORMAL_OPERAND;  /* DE flag */
    if ((status & 0x04) && (control & 0x04)) return K_FPE_FLTDIV;    /* ZE flag */
    if ((status & 0x08) && (control & 0x08)) return K_FPE_FLTOVF;          /* OE flag */
    if ((status & 0x10) && (control & 0x10)) return K_FPE_FLTUND;         /* UE flag */
    if ((status & 0x20) && (control & 0x20)) return K_FPE_FLTRES;    /* PE flag */
    return K_FPE_FLTINV;
}

extern bool writesFlags[InstructionCount];

LONG WINAPI seh_filter(struct _EXCEPTION_POINTERS* ep) {
    BOXEDWINE_CRITICAL_SECTION;
    exceptionCount++;
    KThread* currentThread = KThread::currentThread();
    if (!currentThread) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    Armv8btCPU* cpu = (Armv8btCPU*)currentThread->cpu;
    if (cpu != (BtCPU*)ep->ContextRecord->X19) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    bool inBinaryTranslator = cpu->memory->isCode((U8*)ep->ContextRecord->Pc);

    if (!inBinaryTranslator) {
        // might be in a c++ exception
        //
        // motorhead installer will trigger this a few time
        // caesar 3 installer will trigger this when it exits
        if (cpu->exitToStartThreadLoop) {
            ep->ContextRecord->Pc = cpu->exitToStartThreadLoop;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }
    syncFromException(ep, true);
    U64 result = cpu->startException(ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[0] == 0);
    if (result) {
        syncToException(ep, true);
        ep->ContextRecord->Pc = result;
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    InException inException(cpu);

    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_STACK_CHECK) {
        kpanic("EXCEPTION_FLT_STACK_CHECK");
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_DIVIDE_BY_ZERO) {
        ep->ContextRecord->Pc = cpu->handleFpuException(K_FPE_FLTDIV);
        syncToException(ep, true);
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_INEXACT_RESULT) {
        ep->ContextRecord->Pc = cpu->handleFpuException(K_FPE_FLTRES);
        syncToException(ep, true);
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (ep->ExceptionRecord->ExceptionCode == STATUS_INTEGER_DIVIDE_BY_ZERO) {
        ep->ContextRecord->Pc = cpu->handleFpuException(K_FPE_INTDIV);
        syncToException(ep, true);
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || ep->ExceptionRecord->ExceptionCode == EXCEPTION_DATATYPE_MISALIGNMENT) {
        DecodedOp* op = cpu->getNextOp();
        if (!op->pfnJitCode) {
            cpu->translateEip(cpu->eip.u32);
            if (op->pfnJitCode) {
                ep->ContextRecord->Pc = (U64)op->pfnJitCode;
            }
        } 
        ep->ContextRecord->Pc = cpu->handleAccessException(op);
        syncToException(ep, true);
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (ep->ExceptionRecord->ExceptionCode == STATUS_ILLEGAL_INSTRUCTION) {
        // this is weird, so far I have only seen it on the first instruction of a block, which is an instruction that shouldn't fail, maybe the clearCache wasn't done in time and another thread starting using it?
        klog("Instruction cache miss?");
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif
std::atomic<int> platformThreadCount = 0;

static PVOID pHandler;

#ifdef __TEST
void initThreadForTesting() {
    if (!pHandler) {
        pHandler = AddVectoredExceptionHandler(1, seh_filter);
    }
}
#endif
DWORD WINAPI platformThreadProc(LPVOID lpThreadParameter) {
    KThread* thread = (KThread*)lpThreadParameter;
    BtCPU* cpu = (BtCPU*)thread->cpu;

    if (!pHandler) {
        pHandler = AddVectoredExceptionHandler(1, seh_filter);
    }
    cpu->startThread();
    return 0;
}

void joinThread(KThread* thread) {
    WaitForSingleObject((HANDLE)thread->cpu->nativeHandle, INFINITE);
}

void scheduleThread(KThread* thread) {
    platformThreadCount++;
    BtCPU* cpu = (BtCPU*)thread->cpu;
    cpu->nativeHandle = (U64)CreateThread(nullptr, 0, platformThreadProc, thread, CREATE_SUSPENDED, nullptr);
    if (!cpu->nativeHandle) {
        kpanic("scheduleThread failed to create thread");
        return;
    }
#ifdef BOXEDWINE_MULTI_THREADED
    if (!thread->process->isSystemProcess() && KSystem::cpuAffinityCountForApp) {
        Platform::setCpuAffinityForThread(KThread::currentThread(), KSystem::cpuAffinityCountForApp);
    }
#endif
#ifdef _DEBUG
    BString s = BString::valueOf(thread->id) + " " + thread->process->name;
    wchar_t tmp[256];
    s.w_str(tmp, 256);
    SetThreadDescription((HANDLE)cpu->nativeHandle, tmp);
#endif
    ResumeThread((HANDLE)cpu->nativeHandle);
}

#ifdef BOXEDWINE_MULTI_THREADED
void ATOMIC_WRITE64(U64* pTarget, U64 value) {
    InterlockedExchange64((volatile LONGLONG*)pTarget, (LONGLONG)value);
}
#endif

#endif