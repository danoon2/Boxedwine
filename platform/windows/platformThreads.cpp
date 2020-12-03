#include "boxedwine.h"
#include <windows.h>
#include "../source/emulation/hardmmu/hard_memory.h"
#include "../source/emulation/cpu/normal/normalCPU.h"
#include "ksignal.h"
#include "../source/emulation/cpu/x64/x64CPU.h"

#ifdef BOXEDWINE_MULTI_THREADED

void syncFromException(struct _EXCEPTION_POINTERS *ep, bool includeFPU) {
    BtCPU* cpu = (BtCPU*)KThread::currentThread()->cpu;
    EAX = (U32)ep->ContextRecord->Rax;
    ECX = (U32)ep->ContextRecord->Rcx;
    EDX = (U32)ep->ContextRecord->Rdx;
    EBX = (U32)ep->ContextRecord->Rbx;
    ESP = (U32)ep->ContextRecord->R11;
    EBP = (U32)ep->ContextRecord->Rbp;
    ESI = (U32)ep->ContextRecord->Rsi;
    EDI = (U32)ep->ContextRecord->Rdi;
    cpu->flags = ep->ContextRecord->EFlags;
    cpu->lazyFlags = FLAGS_NONE;
    for (int i=0;i<8;i++) {
        cpu->xmm[i].pi.u64[0] = ep->ContextRecord->FltSave.XmmRegisters[i].Low;
        cpu->xmm[i].pi.u64[1] = ep->ContextRecord->FltSave.XmmRegisters[i].High;
    }

    if (includeFPU && !cpu->thread->process->emulateFPU) {
        cpu->fpu.SetCW(ep->ContextRecord->FltSave.ControlWord);
        cpu->fpu.SetSW(ep->ContextRecord->FltSave.StatusWord);
        cpu->fpu.SetTagFromAbridged(ep->ContextRecord->FltSave.TagWord); 
        for (U32 i=0;i<8;i++) {
            if (!(ep->ContextRecord->FltSave.TagWord & (1 << i))) {
                cpu->fpu.setReg(i, 0.0);
            } else {
                U32 index = (i-cpu->fpu.GetTop()) & 7;
                double d = cpu->fpu.FLD80(ep->ContextRecord->FltSave.FloatRegisters[index].Low, (S16)ep->ContextRecord->FltSave.FloatRegisters[index].High);
                cpu->fpu.setReg(i, d);
            }
        }
    }
}

void syncToException(struct _EXCEPTION_POINTERS *ep, bool includeFPU) {
    BtCPU* cpu = (BtCPU*)KThread::currentThread()->cpu;
    ep->ContextRecord->Rax = EAX;
    ep->ContextRecord->Rcx = ECX;
    ep->ContextRecord->Rdx = EDX;
    ep->ContextRecord->Rbx = EBX;
    ep->ContextRecord->R11 = ESP;
    ep->ContextRecord->Rbp = EBP;
    ep->ContextRecord->Rsi = ESI;
    ep->ContextRecord->Rdi = EDI;
    if (KSystem::useLargeAddressSpace) {
        ep->ContextRecord->R14 = (U64)cpu->eipToHostInstructionAddressSpaceMapping;
    } else {
        ep->ContextRecord->R14 = cpu->seg[SS].address;
    }
    ep->ContextRecord->R15 = cpu->seg[DS].address;
    cpu->fillFlags();
    ep->ContextRecord->EFlags = cpu->flags;
    for (int i=0;i<8;i++) {
        ep->ContextRecord->FltSave.XmmRegisters[i].Low = cpu->xmm[i].pi.u64[0];
        ep->ContextRecord->FltSave.XmmRegisters[i].High = cpu->xmm[i].pi.u64[1];
    }
    if (includeFPU && !cpu->thread->process->emulateFPU) {
        ep->ContextRecord->FltSave.ControlWord = cpu->fpu.CW();
        ep->ContextRecord->FltSave.StatusWord = cpu->fpu.SW();
        ep->ContextRecord->FltSave.TagWord = cpu->fpu.GetAbridgedTag();
        for (U32 i=0;i<8;i++) {
            U32 index = (i-cpu->fpu.GetTop()) & 7;
            if (!(ep->ContextRecord->FltSave.TagWord & (1 << i))) {
                ep->ContextRecord->FltSave.FloatRegisters[index].Low = 0;
                ep->ContextRecord->FltSave.FloatRegisters[index].High = 0;
            } else {                
                cpu->fpu.ST80(i, &ep->ContextRecord->FltSave.FloatRegisters[index].Low, (ULONGLONG*)&ep->ContextRecord->FltSave.FloatRegisters[index].High);
            }
        }
    }
}

class InException {
public:
    InException(BtCPU* cpu) : cpu(cpu) {this->cpu->inException = true;}
    ~InException() {this->cpu->inException = false;}
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

LONG WINAPI seh_filter(struct _EXCEPTION_POINTERS *ep) {
    BOXEDWINE_CRITICAL_SECTION;
    exceptionCount++;
    KThread* currentThread = KThread::currentThread();
    if (!currentThread) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    x64CPU* cpu = (x64CPU*)currentThread->cpu;
    if (ep->ContextRecord->EFlags & AC) {
        // :TODO: is there a way to clear in now
        ep->ContextRecord->EFlags&=~AC;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (cpu!=(BtCPU*)ep->ContextRecord->R13) {
        return EXCEPTION_CONTINUE_SEARCH;
    }	

    std::function<void(DecodedOp*)> doSyncFrom = [ep] (DecodedOp* op) {
            syncFromException(ep, op?op->isFpuOp():true);
        };
    std::function<void(DecodedOp*)> doSyncTo = [ep] (DecodedOp* op) {
            syncToException(ep, op?op->isFpuOp():true);
        };

    U64 result = cpu->startException(ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[0]==0, doSyncFrom, doSyncTo);
    if (result) {
        ep->ContextRecord->Rip = result;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    
    InException inException(cpu);
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION) {
        if (ep->ContextRecord->Rsp & 0xf) {
            kpanic("seh_filter: bad stack alignment");
        }
        U64 rip = cpu->handleIllegalInstruction(ep->ContextRecord->Rip);
        if (rip) {
            ep->ContextRecord->Rip = rip;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && (ep->ContextRecord->Rip & 0xFFFFFFFF00000000l)==(U64)cpu->thread->memory->executableMemoryId) {      
        std::function<U64(U32 reg)> getReg = [ep] (U32 reg) {
            if (reg == 8)
                return ep->ContextRecord->R8;
            if (reg == 9)
                return ep->ContextRecord->R9;
            if (reg == 6)
                return ep->ContextRecord->Rsi;
            if (reg == 7)
                return ep->ContextRecord->Rdi;
            kpanic("Unhandled reg: getReg: %d", reg);
            return (U64)0;
        };
        std::function<void(U32 reg, U64 value)> setReg = [ep](U32 reg, U64 value) {
            if (reg == 10) {
                ep->ContextRecord->R10 = value;
            } else {
                kpanic("Unhandled reg: setReg: %d", reg);
            }
        };

        U64 rip = cpu->handleAccessException(ep->ContextRecord->Rip, ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[0]==0, getReg, setReg, doSyncFrom, doSyncTo);
        if (rip) {
            ep->ContextRecord->Rip = rip;
        }
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_STACK_CHECK) {
        kpanic("EXCEPTION_FLT_STACK_CHECK");
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_DIVIDE_BY_ZERO) {
        int code = getFpuException(ep->ContextRecord->FltSave.ControlWord, ep->ContextRecord->FltSave.StatusWord);

        ep->ContextRecord->Rip = cpu->handleFpuException(code, doSyncFrom, doSyncTo);
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_DATATYPE_MISALIGNMENT) {
		// :TODO: figure out how AC got set, I've only seen this while op logging
        ep->ContextRecord->EFlags&=~AC;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

static PVOID pHandler;
U32 platformThreadCount = 0;

DWORD WINAPI platformThreadProc(LPVOID lpThreadParameter) {
    KThread* thread = (KThread*)lpThreadParameter;
    BtCPU* cpu = (BtCPU*)thread->cpu;
    
    if (!pHandler) {
        pHandler = AddVectoredExceptionHandler(1,seh_filter);
    }       
    cpu->startThread();
    return 0;
}

void scheduleThread(KThread* thread) {
    platformThreadCount++;
    BtCPU* cpu = (BtCPU*)thread->cpu;
    cpu->nativeHandle = (U64)CreateThread(NULL, 0, platformThreadProc, thread, CREATE_SUSPENDED, 0);
#ifdef BOXEDWINE_MULTI_THREADED
    if (!thread->process->isSystemProcess() && KSystem::cpuAffinityCountForApp) {
        Platform::setCpuAffinityForThread(KThread::currentThread(), KSystem::cpuAffinityCountForApp);
    }
#endif
#ifdef _DEBUG
    std::string s = std::to_string(thread->id) + " " + thread->process->name;
    std::wstring w(s.begin(), s.end());
    SetThreadDescription((HANDLE)cpu->nativeHandle, w.c_str());
#endif
    ResumeThread((HANDLE)cpu->nativeHandle);
}

#ifdef BOXEDWINE_MULTI_THREADED
void ATOMIC_WRITE64(U64* pTarget, U64 value) {
    InterlockedExchange64((volatile LONGLONG *)pTarget, (LONGLONG)value);
}
#endif

#endif