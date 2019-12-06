#include "boxedwine.h"
#include <windows.h>
#include <SDL.h>
#include "../source/emulation/cpu/x64/x64cpu.h"
#include "../source/emulation/cpu/x64/x64CodeChunk.h"
#include "../source/emulation/hardmmu/hard_memory.h"
#include "../source/emulation/cpu/normal/normalCPU.h"
#include "../source/emulation/cpu/x64/x64Asm.h"

#ifdef BOXEDWINE_MULTI_THREADED

void syncFromException(struct _EXCEPTION_POINTERS *ep, bool includeFPU) {
    x64CPU* cpu = (x64CPU*)KThread::currentThread()->cpu;
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
    x64CPU* cpu = (x64CPU*)KThread::currentThread()->cpu;
    ep->ContextRecord->Rax = EAX;
    ep->ContextRecord->Rcx = ECX;
    ep->ContextRecord->Rdx = EDX;
    ep->ContextRecord->Rbx = EBX;
    ep->ContextRecord->R11 = ESP;
    ep->ContextRecord->Rbp = EBP;
    ep->ContextRecord->Rsi = ESI;
    ep->ContextRecord->Rdi = EDI;
    ep->ContextRecord->R14 = cpu->seg[SS].address;
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
    InException(x64CPU* cpu) : cpu(cpu) {this->cpu->inException = true;}
    ~InException() {this->cpu->inException = false;}
    x64CPU* cpu;
};

U32 exceptionCount;

LONG WINAPI seh_filter(struct _EXCEPTION_POINTERS *ep) {
    BOXEDWINE_CRITICAL_SECTION;
    exceptionCount++;
    KThread* currentThread = KThread::currentThread();
    if (!currentThread) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    x64CPU* cpu = (x64CPU*)currentThread->cpu;
	if (cpu->restarting) {
		ep->ContextRecord->Rip = (U64)cpu->init();
		cpu->restarting = false;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
    if (ep->ContextRecord->EFlags & AC) {
        // :TODO: is there a way to clear in now
        ep->ContextRecord->EFlags&=~AC;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (cpu!=(x64CPU*)ep->ContextRecord->R13) {
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
        U64 rip = cpu->handleAccessException(ep->ContextRecord->Rip, ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[0]==0, ep->ContextRecord->Rsi, ep->ContextRecord->Rdi, ep->ContextRecord->R8, ep->ContextRecord->R9, &ep->ContextRecord->R10, doSyncFrom, doSyncTo); 
        if (rip) {
            ep->ContextRecord->Rip = rip;
        }
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_STACK_CHECK) {
        kpanic("EXCEPTION_FLT_STACK_CHECK");
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_DIVIDE_BY_ZERO) {
        ep->ContextRecord->Rip = cpu->handleDivByZero(doSyncFrom, doSyncTo);
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
    x64CPU* cpu = (x64CPU*)thread->cpu;
    
    if (!pHandler) {
        pHandler = AddVectoredExceptionHandler(1,seh_filter);
    }       
    cpu->startThread();
    return 0;
}

void scheduleThread(KThread* thread) {
    platformThreadCount++;
    x64CPU* cpu = (x64CPU*)thread->cpu;
    cpu->nativeHandle = (U64)CreateThread(NULL, 0, platformThreadProc, thread, 0, 0);        
}

#ifdef BOXEDWINE_MULTI_THREADED
void ATOMIC_WRITE64(U64* pTarget, U64 value) {
    InterlockedExchange64((volatile LONGLONG *)pTarget, (LONGLONG)value);
}
#endif

#endif