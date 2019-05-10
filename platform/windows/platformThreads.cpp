#include "boxedwine.h"
#include <windows.h>
#include <SDL.h>
#include "../source/emulation/cpu/x64/x64cpu.h"
#include "../source/emulation/cpu/x64/x64CodeChunk.h"
#include "../source/emulation/hardmmu/hard_memory.h"
#include "../source/emulation/cpu/normal/normalCPU.h"
#include "../source/emulation/cpu/x64/x64Asm.h"

#ifdef BOXEDWINE_MULTI_THREADED

void syncFromException(x64CPU* cpu, struct _EXCEPTION_POINTERS *ep) {
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
}

void syncToException(x64CPU* cpu, struct _EXCEPTION_POINTERS *ep) {
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
}

LONG handleChangedUnpatchedCode(struct _EXCEPTION_POINTERS *ep, x64CPU* cpu) {
#ifndef __TEST
    // only one thread at a time can update the host code pages and related date like opToAddressPages
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->thread->memory->executableMemoryMutex);
#endif
                        
    unsigned char* hostAddress = (unsigned char*)ep->ContextRecord->Rip;
    X64CodeChunk* chunk = cpu->thread->memory->getCodeChunkContainingHostAddress(hostAddress);
    chunk->updateStartingAtHostAddress(hostAddress);   
    return EXCEPTION_CONTINUE_EXECUTION;
}

LONG handleMissingCode(struct _EXCEPTION_POINTERS *ep, x64CPU* cpu, U32 inst) {
    U32 page = (U32)ep->ContextRecord->R8;
    U32 offset = (U32)ep->ContextRecord->R9;

    cpu->translateEip(((page << K_PAGE_SHIFT) | offset) - cpu->seg[CS].address);
    if (inst==0xC8048B4A) {
        ep->ContextRecord->Rax = (U64)(cpu->eipToHostInstruction[page]);
    } else {
        ep->ContextRecord->Rax = (U64)(cpu->eipToHostInstruction[page][offset]);
    }
    return EXCEPTION_CONTINUE_EXECUTION;
}

LONG handleCodePatch(struct _EXCEPTION_POINTERS *ep, x64CPU* cpu, U32 address) {
#ifndef __TEST
    // only one thread at a time can update the host code pages and related date like opToAddressPages
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->thread->memory->executableMemoryMutex);
#endif
    // get the emulated eip of the op that corresponds to the host address where the exception happened
    X64CodeChunk* chunk = cpu->thread->memory->getCodeChunkContainingHostAddress((void*)ep->ContextRecord->Rip);
    cpu->eip.u32 = chunk->getEipThatContainsHostAddress((void*)ep->ContextRecord->Rip, NULL) - cpu->seg[CS].address;

    // get the emulated op that caused the write
    DecodedOp* op = cpu->getExistingOp(cpu->eip.u32);
    if (op) {             
        // change permission of the page so that we can write to it
        // :TODO: what if memWidth is a string
        U32 memWidth = instructionInfo[op->inst].writeMemWidth/8;
        U32 page1 = address >> K_PAGE_SHIFT;
        U32 page2 = (address+memWidth-1) >> K_PAGE_SHIFT;

        ::clearCodePageReadOnly(cpu->thread->memory, page1);
        // did the write span two pages?
        if (page1!=page2) {
            ::clearCodePageReadOnly(cpu->thread->memory, page2);
        }

        // do the write
        op->pfn = NormalCPU::getFunctionForOp(op);
        op->next = DecodedOp::alloc();
        op->next->inst = Done;
        op->next->pfn = NormalCPU::getFunctionForOp(op->next);
        syncFromException(cpu, ep);        
        op->pfn(cpu, op);        
        syncToException(cpu, ep);
        op->dealloc(true);                        

        // change the page(s) we wrote to back to read-only
        ::makeCodePageReadOnly(cpu->thread->memory, page1);  
        if (page1!=page2) {
            ::makeCodePageReadOnly(cpu->thread->memory, page2);
        }

        // if the instruction has not been translated into x64 then we can ignore it
        X64CodeChunk* chunk = cpu->thread->memory->getCodeChunkContainingEip(address - cpu->seg[CS].address);
        if (chunk) {
            chunk->patch(address, memWidth);
        }                                          

        // eip was ajusted after running this instruction                        
        U32 a = cpu->getEipAddress();
        ep->ContextRecord->Rip = (U64)cpu->eipToHostInstruction[a >> K_PAGE_SHIFT][a & K_PAGE_MASK];
        return EXCEPTION_CONTINUE_EXECUTION;
    } else {                        
        kpanic("Threw an exception from a host location that doesn't map to an emulated instruction");
    }
    return EXCEPTION_CONTINUE_EXECUTION;
}

class InException {
public:
    InException(x64CPU* cpu) : cpu(cpu) {this->cpu->inException = true;}
    ~InException() {this->cpu->inException = false;}
    x64CPU* cpu;
};

LONG WINAPI seh_filter(struct _EXCEPTION_POINTERS *ep) {
    KThread* currentThread = KThread::currentThread();
    if (!currentThread) {
        return EXCEPTION_CONTINUE_SEARCH;;
    }
    x64CPU* cpu = (x64CPU*)currentThread->cpu;
    if (cpu!=(x64CPU*)ep->ContextRecord->R13) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    if (cpu->inException) {
        EAX = (U32)ep->ContextRecord->Rax;
        ECX = (U32)ep->ContextRecord->Rcx;
        EDX = (U32)ep->ContextRecord->Rdx;
        EBX = (U32)ep->ContextRecord->Rbx;
        ESP = (U32)ep->ContextRecord->R11;
        EBP = (U32)ep->ContextRecord->Rbp;
        ESI = (U32)ep->ContextRecord->Rsi;
        EDI = (U32)ep->ContextRecord->Rdi;
        cpu->thread->seg_mapper((U32)ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[0]==0, ep->ExceptionRecord->ExceptionInformation[0]==1);
        return EXCEPTION_CONTINUE_SEARCH;
    } 
    InException inException(cpu);
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION) {
        if (*((U8*)ep->ContextRecord->Rip)==0xce) {            
            return handleChangedUnpatchedCode(ep, cpu);
        } else {
        }
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && (ep->ContextRecord->Rip & 0xFFFFFFFF00000000l)==(U64)cpu->thread->memory->executableMemoryId) {      
        U32 inst = *((U32*)ep->ContextRecord->Rip);

        if (inst==0x088B4466 || inst==0xC8048B4A) {            
            return handleMissingCode(ep, cpu, inst);
        } else {                
            // check if the emulated memory caused the exception
            if ((ep->ExceptionRecord->ExceptionInformation[1] & 0xFFFFFFFF00000000l) == cpu->thread->memory->id) {                
                U32 address = (U32)ep->ExceptionRecord->ExceptionInformation[1];
            
                // check if emulated memory that caused the exception is a page that has code
                if (cpu->thread->memory->nativeFlags[address>>K_PAGE_SHIFT] & NATIVE_FLAG_CODEPAGE_READONLY) {
                    return handleCodePatch(ep, cpu, address);
                }
            }             

            EAX = (U32)ep->ContextRecord->Rax;
            ECX = (U32)ep->ContextRecord->Rcx;
            EDX = (U32)ep->ContextRecord->Rdx;
            EBX = (U32)ep->ContextRecord->Rbx;
            ESP = (U32)ep->ContextRecord->R11;
            EBP = (U32)ep->ContextRecord->Rbp;
            ESI = (U32)ep->ContextRecord->Rsi;
            EDI = (U32)ep->ContextRecord->Rdi;
            cpu->thread->seg_mapper((U32)ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[0]==0, ep->ExceptionRecord->ExceptionInformation[0]==1);
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

U32 platformThreadCount = 0;
static PVOID pHandler;

DWORD WINAPI platformThreadProc(LPVOID lpThreadParameter) {
    KThread* thread = (KThread*)lpThreadParameter;
    x64CPU* cpu = (x64CPU*)thread->cpu;
    jmp_buf jmpBuf;

    KThread::setCurrentThread(thread);
    // :TODO: hopefully this will eventually go away.  For now this prevents a signal from being generated which isn't handled yet
    SDL_Delay(50);     

    if (!pHandler)
        pHandler = AddVectoredExceptionHandler(1,seh_filter);

    if (!setjmp(jmpBuf)) {
        cpu->jmpBuf = &jmpBuf;
        cpu->run();
    }
    
    BOXEDWINE_CONDITION_LOCK(cpu->endCond);
    BOXEDWINE_CONDITION_SIGNAL(cpu->endCond);
    BOXEDWINE_CONDITION_UNLOCK(cpu->endCond);
    platformThreadCount--;
    if (platformThreadCount==0) {
        SDL_Event sdlevent;
        sdlevent.type = SDL_QUIT;        
        SDL_PushEvent(&sdlevent);
    }
    return 0;
}

void scheduleThread(KThread* thread) {
    platformThreadCount++;
    x64CPU* cpu = (x64CPU*)thread->cpu;
    cpu->nativeHandle = (U64)CreateThread(NULL, 0, platformThreadProc, thread, 0, 0);        
}

void unscheduleThread(KThread* thread) {
    if (thread==KThread::currentThread())
        return;
    kpanic("unscheduleThread not implemented yet");
}

void unscheduleCurrentThread() {
    jmp_buf* jmpBuf = ((x64CPU*)KThread::currentThread()->cpu)->jmpBuf;
    delete KThread::currentThread();
    longjmp(*jmpBuf,1);
}

void addTimer(KTimer* timer) {
    kpanic("addTimer not implemented yet");
}

void removeTimer(KTimer* timer) {
    if (timer->active)
        kpanic("removeTimer not implemented yet");
}

#endif