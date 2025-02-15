#include "boxedwine.h"
#include <windows.h>
#include "../source/emulation/softmmu/kmemory_soft.h"
#include "../source/emulation/cpu/normal/normalCPU.h"
#include "ksignal.h"
#include "../source/emulation/cpu/binaryTranslation/btCpu.h"

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
    cpu->lazyFlags = FLAGS_NONE;    

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
    klog("fpu/mmx should not get here");
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
    KMemoryData* mem = getMemData(cpu->memory);
    bool inBinaryTranslator = mem->isAddressExecutable((U8*)ep->ContextRecord->Rip);

    if (!inBinaryTranslator) {
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
        // should only be triggered when a read/write crosses a page boundry or the page has a custom read/write handler        
        DecodedOp* op = NormalCPU::decodeSingleOp(cpu, cpu->getEipAddress());
        if (writesFlags[op->inst]) {
            cpu->flags = ((cpu->instructionStoredFlags >> 8) & 0xFF) | (cpu->flags & DF) | ((cpu->instructionStoredFlags & 0xFF) ? OF : 0);
        }
        ep->ContextRecord->Rip = cpu->handleAccessException(op);
        // X64Asm::checkMemory disables the code path for fpu and mmx to throw exceptions
        //
        // I'm not sure why, but setting the fpu state causes some stability issues so for now this is the band-aid
        syncToException(ep, op->isFpuOp() || op->isMmxOp());
        op->dealloc(true);
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

    cpu->flags = (ep->ContextRecord->X8 & (AF | CF | OF | SF | PF | ZF)) | (cpu->flags & DF); // DF is fully kept in sync, so don't override
    cpu->lazyFlags = FLAGS_NONE;
    cpu->eip.u32 = getMemData(cpu->memory)->codeCache.getEipFromHost((U8*)ep->ContextRecord->Pc);

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
    ep->ContextRecord->X8 &= ~(AF | CF | OF | SF | PF | ZF);
    ep->ContextRecord->X8 |= (cpu->flags & (AF | CF | OF | SF | PF | ZF));

    for (int i = 0; i < 8; i++) {
        ep->ContextRecord->V[i].Low = cpu->xmm[i].pi.u64[0];
        ep->ContextRecord->V[i].High = cpu->xmm[i].pi.u64[1];
        if (cpu->fpu.isMMXInUse) {
            ep->ContextRecord->V[i + 8].Low = cpu->fpu.getMMX(i)->q;
        }
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
    Armv8btCPU* cpu = (Armv8btCPU*)currentThread->cpu;
    if (cpu != (BtCPU*)ep->ContextRecord->X19) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    KMemoryData* mem = getMemData(cpu->memory);
    bool inBinaryTranslator = mem->isAddressExecutable((U8*)ep->ContextRecord->Pc);

    if (!inBinaryTranslator) {
        // might be in a c++ exception
        //
        // motorhead installer will trigger this a few time
        // caesar 3 installer will trigger this when it exits
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
        DecodedOp* op = NormalCPU::decodeSingleOp(cpu, cpu->getEipAddress());
        ep->ContextRecord->Pc = cpu->handleAccessException(op);
        op->dealloc(true);
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