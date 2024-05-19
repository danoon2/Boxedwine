#include "boxedwine.h"
#include <windows.h>
#include "../source/emulation/softmmu/kmemory_soft.h"
#include "../source/emulation/cpu/normal/normalCPU.h"
#include "ksignal.h"
#include "../source/emulation/cpu/x64/x64CPU.h"

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
        // :TODO: not sure what causes this, seen it in winroids
        ep->ContextRecord->EFlags&=~AC;
        return EXCEPTION_CONTINUE_EXECUTION;
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
    if (cpu!=(BtCPU*)ep->ContextRecord->R13) {
        return EXCEPTION_CONTINUE_SEARCH;
    }	
    
    syncFromException(ep, true);
    U64 result = cpu->startException(ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[0]==0);
    if (result) {
        syncToException(ep, true);
        ep->ContextRecord->Rip = result;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    
    InException inException(cpu);

    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_STACK_CHECK) {
        kpanic("EXCEPTION_FLT_STACK_CHECK");
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_DIVIDE_BY_ZERO) {
        int code = getFpuException(ep->ContextRecord->FltSave.ControlWord, ep->ContextRecord->FltSave.StatusWord);

        ep->ContextRecord->Rip = cpu->handleFpuException(code);
        syncToException(ep, true);
        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (ep->ExceptionRecord->ExceptionCode == STATUS_INTEGER_DIVIDE_BY_ZERO) {
        ep->ContextRecord->Rip = cpu->handleFpuException(K_FPE_INTDIV);
        syncToException(ep, true);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

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
        pHandler = AddVectoredExceptionHandler(1,seh_filter);
    }
    cpu->startThread();
    return 0;
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
    InterlockedExchange64((volatile LONGLONG *)pTarget, (LONGLONG)value);
}
#endif

#endif