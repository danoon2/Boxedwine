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
#ifdef _M_ARM64
    for (U32 i = 0; i < 8; i++) {
        if (regCache[i] != 0xFF) {
            switch (regCache[i]) {
            case 0: ep->ContextRecord->X0 = cpu->reg[i].u32; break;
            case 1: ep->ContextRecord->X1 = cpu->reg[i].u32; break;
            case 2: ep->ContextRecord->X2 = cpu->reg[i].u32; break;
            case 3: ep->ContextRecord->X3 = cpu->reg[i].u32; break;
            case 4: ep->ContextRecord->X4 = cpu->reg[i].u32; break;
            case 5: ep->ContextRecord->X5 = cpu->reg[i].u32; break;
            case 6: ep->ContextRecord->X6 = cpu->reg[i].u32; break;
            case 7: ep->ContextRecord->X7 = cpu->reg[i].u32; break;
            default:
                kpanic("Windows ARM64 syncToException");
            }
        }
    }
    for (U32 i = 0; i < 8; i++) {
        if (xmmCache[i] != 0xFF) {
            ep->ContextRecord->V[xmmCache[i]].Low = cpu->xmm[i].pi.u64[0];
            ep->ContextRecord->V[xmmCache[i]].High = cpu->xmm[i].pi.u64[1];
        }
    }
    ep->ContextRecord->X8 = cpu->flags;
    ep->ContextRecord->X16 = cpu->src.u32;
    ep->ContextRecord->X10 = cpu->dst.u32;
    ep->ContextRecord->X15 = cpu->result.u32;
    ep->ContextRecord->X11 = cpu->lazyFlagType;

#elif defined(BOXEDWINE_64)
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
#ifdef _M_ARM64
    for (U32 i = 0; i < 8; i++) {
        if (regCache[i] != 0xFF) {
            switch (regCache[i]) {
            case 0: cpu->reg[i].u32 = (U32)ep->ContextRecord->X0; break;
            case 1: cpu->reg[i].u32 = (U32)ep->ContextRecord->X1; break;
            case 2: cpu->reg[i].u32 = (U32)ep->ContextRecord->X2; break;
            case 3: cpu->reg[i].u32 = (U32)ep->ContextRecord->X3; break;
            case 4: cpu->reg[i].u32 = (U32)ep->ContextRecord->X4; break;
            case 5: cpu->reg[i].u32 = (U32)ep->ContextRecord->X5; break;
            case 6: cpu->reg[i].u32 = (U32)ep->ContextRecord->X6; break;
            case 7: cpu->reg[i].u32 = (U32)ep->ContextRecord->X7; break;
            default:
                kpanic("Windows ARM64 syncFromException");
            }
        }
    }
    for (U32 i = 0; i < 8; i++) {
        if (xmmCache[i] != 0xFF) {
            cpu->xmm[i].pi.u64[0] = ep->ContextRecord->V[xmmCache[i]].Low;
            cpu->xmm[i].pi.u64[1] = ep->ContextRecord->V[xmmCache[i]].High;            
        }
    }
    cpu->flags = (U32)ep->ContextRecord->X8;
    cpu->src.u32 = (U32)ep->ContextRecord->X16;
    cpu->dst.u32 = (U32)ep->ContextRecord->X10;
    cpu->result.u32 = (U32)ep->ContextRecord->X15;
    cpu->lazyFlagType = (LazyFlagType)ep->ContextRecord->X11;

#elif defined(BOXEDWINE_64)
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

#ifdef _M_ARM64
#define CPU_REG X19
#define REG_IP Pc
#define SET_REG_IP(r) Pc = (U64)r
#elif defined(BOXEDWINE_64)
#define CPU_REG R13
#define REG_IP Rip
#define SET_REG_IP(r) Rip = (U64)r
#else
#define CPU_REG Edi
#define REG_IP Eip
#define SET_REG_IP(r) Eip = (U32)r
#endif

LONG WINAPI seh_filter(struct _EXCEPTION_POINTERS* ep) {
#ifndef _M_ARM64
    if (ep->ContextRecord->EFlags & AC) {
        // the comment on the next line was for the x64 binary translator, I'm not sure if it still applies to the JIT
        // :TODO: not sure what causes this, seen it in winroids
        ep->ContextRecord->EFlags &= ~AC;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
#endif
    KThread* currentThread = KThread::currentThread();
    if (!currentThread) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    CPU* cpu = currentThread->cpu;
    if (cpu != (CPU*)ep->ContextRecord->CPU_REG) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    bool inBinaryTranslator = cpu->memory->isCode((U8*)ep->ContextRecord->REG_IP);    
    if (!inBinaryTranslator) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    syncFromException(ep);
    //klog_fmt("exception at %llx(%d)", ep->ExceptionRecord->ExceptionInformation[1], (U32)ep->ExceptionRecord->ExceptionInformation[0]);
    bool readException = ep->ExceptionRecord->ExceptionInformation[0] == 0;
    void* result = cpu->startException((U32)ep->ExceptionRecord->ExceptionInformation[1], readException);
    if (result) {
        syncToException(ep);
        ep->ContextRecord->SET_REG_IP(result);
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    InException inException(cpu);

    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || ep->ExceptionRecord->ExceptionCode == EXCEPTION_DATATYPE_MISALIGNMENT || ep->ExceptionRecord->ExceptionCode == STATUS_ILLEGAL_INSTRUCTION) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->memory->mutex); // jitCache needs this
        U32 eip = 0;
        if (!getMemData(cpu->memory)->findOpFromJitAddress((U8*)ep->ContextRecord->REG_IP, eip)) {
            return EXCEPTION_CONTINUE_SEARCH;
        }        
#ifdef _DEBUG
        if (ep->ExceptionRecord->ExceptionCode != STATUS_ILLEGAL_INSTRUCTION && eip != cpu->eip.u32) {
            klog_fmt("%x/%x", cpu->eip.u32, eip);
        }        
#endif
        cpu->eip.u32 = eip;
        DecodedOp* op = cpu->getNextOp();
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