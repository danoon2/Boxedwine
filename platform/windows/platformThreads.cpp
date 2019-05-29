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
    ep->ContextRecord->R14 = cpu->seg[SS].address;
    ep->ContextRecord->R15 = cpu->seg[DS].address;
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
    if (!chunk) {
        /* some debug code
        U32 toEip = *(((U32*)hostAddress)+2);
        KThread* thread = KThread::currentThread();
        Memory* memory = thread->memory;

        for (U32 i=0;i<K_NUMBER_OF_PAGES;i++) {
            X64CodeChunk* chunk = memory->hostCodeChunks[i];
            while (chunk) {
                if (chunk->hasLinkTo(hostAddress)) {
                    int ii=0;
                }
                if (chunk->hasLinkToEip(toEip)) {
                    int ii=0;
                }
                chunk = chunk->getNext();
            }
        }
        */
        kpanic("handleChangedUnpatchedCode: could not find chunk");
    }
    U32 startOfEip = chunk->getEipThatContainsHostAddress(hostAddress, NULL);
    chunk->deallocAndRetranslate();   
    ep->ContextRecord->Rip = (U64)cpu->thread->memory->getExistingHostAddress(startOfEip);
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
    cpu->eip.u32 = chunk->getEipThatContainsHostAddress((void*)ep->ContextRecord->Rip, NULL)-cpu->seg[CS].address;

    // get the emulated op that caused the write
    DecodedOp* op = cpu->getExistingOp(cpu->eip.u32);
    if (op) {             
        // change permission of the page so that we can write to it
        U32 memWidth = instructionInfo[op->inst].writeMemWidth/8;
        U32 page1 = address >> K_PAGE_SHIFT;
        U32 page2 = (address+memWidth-1) >> K_PAGE_SHIFT;

        if (op->repNotZero || op->repZero) {
            // :TODO: might need to support more than just two pages
            if (ep->ContextRecord->EFlags & DF) {
                page2 = page1-1; // fixes rebal assault gog installer
            } else {
                page2 = page1+1;
            }
        }        

        bool clearedPage1 = false;
        bool clearedPage2 = false;

        if (cpu->thread->memory->nativeFlags[page1] & NATIVE_FLAG_CODEPAGE_READONLY) {
            ::clearCodePageReadOnly(cpu->thread->memory, page1);
            clearedPage1 = true;
        }
        // did the write span two pages?
        if (page1!=page2) {
            if (cpu->thread->memory->nativeFlags[page2] & NATIVE_FLAG_CODEPAGE_READONLY) {
                ::clearCodePageReadOnly(cpu->thread->memory, page2);
                clearedPage2 = true;
            }
        }

        static DecodedBlock b;
        DecodedBlock::currentBlock = &b;
        b.next1 = &b;
        b.next2 = &b;
        // do the write
        op->pfn = NormalCPU::getFunctionForOp(op);
        op->next = DecodedOp::alloc();
        op->next->inst = Done;
        op->next->pfn = NormalCPU::getFunctionForOp(op->next);
        syncFromException(cpu, ep); 

        // for string instruction, we modify (add memory offset and segment) rdi and rsi so that the native string instruction can be used, this code will revert it back to the original values
        // uses si
        if (op->inst==Lodsb || op->inst==Lodsw || op->inst==Lodsd) {
            ESI=(U32)(ep->ContextRecord->Rsi - cpu->memOffset - cpu->seg[op->base].address);
        }
        // uses di (Examples: diablo 1 will trigger this in the middle of the Stosd when creating a new game)
        if (op->inst==Stosb || op->inst==Stosw || op->inst==Stosd ||
            op->inst==Scasb || op->inst==Scasw || op->inst==Scasd) {
            EDI=(U32)(ep->ContextRecord->Rdi - cpu->memOffset - cpu->seg[ES].address);
        }
        // uses si and di
        if (op->inst==Movsb || op->inst==Movsw || op->inst==Movsd ||
            op->inst==Cmpsb || op->inst==Cmpsw || op->inst==Cmpsd) {
            ESI=(U32)(ep->ContextRecord->Rsi - cpu->memOffset - cpu->seg[op->base].address);
            EDI=(U32)(ep->ContextRecord->Rdi - cpu->memOffset - cpu->seg[ES].address);
        }
        if (cpu->flags & DF) {
            cpu->df = -1;
        } else {
            cpu->df = 1;
        }
        op->pfn(cpu, op);        
        syncToException(cpu, ep);
        op->dealloc(true);                        

        // change the page(s) we wrote to back to read-only
        if (clearedPage1) {
            ::makeCodePageReadOnly(cpu->thread->memory, page1);  
        }
        if (clearedPage2) {
            ::makeCodePageReadOnly(cpu->thread->memory, page2);
        }

        // if the instruction has not been translated into x64 then we can ignore it
        X64CodeChunk* dstChunk = cpu->thread->memory->getCodeChunkContainingEip(address - cpu->seg[CS].address);
        if (dstChunk) {
            dstChunk->patch(address, memWidth);
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

void OPCALL unscheduleCallback(CPU* cpu, DecodedOp* op) {
    jmp_buf* jmpBuf = ((x64CPU*)KThread::currentThread()->cpu)->jmpBuf;
    longjmp(*jmpBuf,1);
}

LONG WINAPI seh_filter(struct _EXCEPTION_POINTERS *ep) {
    BOXEDWINE_CRITICAL_SECTION;
    KThread* currentThread = KThread::currentThread();
    if (!currentThread) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    x64CPU* cpu = (x64CPU*)currentThread->cpu;
    if (cpu!=(x64CPU*)ep->ContextRecord->R13) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    if (currentThread->exiting) {
        X64Asm data(cpu);
        data.callCallback(unscheduleCallback);
        ep->ContextRecord->Rip = (U64)data.commit(true)->getHostAddress();        
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (cpu->inException) {
        syncFromException(cpu, ep);
        cpu->thread->seg_mapper((U32)ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[0]==0, ep->ExceptionRecord->ExceptionInformation[0]==1);
        syncToException(cpu, ep);
        ep->ContextRecord->Rip = (U64)cpu->translateEip(cpu->eip.u32); 
        return EXCEPTION_CONTINUE_EXECUTION;
    } 
    InException inException(cpu);
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION) {
        if (*((U8*)ep->ContextRecord->Rip)==0xce) {            
            return handleChangedUnpatchedCode(ep, cpu);
        } if (*((U8*)ep->ContextRecord->Rip)==0xcd) { 
            // free'd chunks are filled in with 0xcd, if this one is free'd, it is possible another thread replaced the chunk
            // while this thread jumped to it and this thread waited in the critical section at the top of this function.
            void* host = cpu->thread->memory->getExistingHostAddress(cpu->eip.u32+cpu->seg[CS].address);
            if (host) {
                ep->ContextRecord->Rip = (U64)host;
                return EXCEPTION_CONTINUE_EXECUTION;
            } else {
                kpanic("x64 seh_filter tried to run code in a free'd chunk");
            }
        } else {
            int ii=0;
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
#ifdef _DEBUG
            void* fromHost = cpu->thread->memory->getExistingHostAddress(cpu->fromEip);
#endif
            syncFromException(cpu, ep);
            cpu->thread->seg_mapper((U32)ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[0]==0, ep->ExceptionRecord->ExceptionInformation[0]==1);
            syncToException(cpu, ep);
            ep->ContextRecord->Rip = (U64)cpu->translateEip(cpu->eip.u32); 
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    } else if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_STACK_CHECK) {
        kpanic("EXCEPTION_FLT_STACK_CHECK");
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

U32 platformThreadCount = 0;
static PVOID pHandler;

DWORD WINAPI platformThreadProc(LPVOID lpThreadParameter) {
    KThread* thread = (KThread*)lpThreadParameter;
    x64CPU* cpu = (x64CPU*)thread->cpu;
    jmp_buf jmpBuf;
    U32 threadId = thread->id;
    U32 processId = thread->process->id;

    KThread::setCurrentThread(thread);
    // :TODO: hopefully this will eventually go away.  For now this prevents a signal from being generated which isn't handled yet
    SDL_Delay(50);     

    if (!pHandler)
        pHandler = AddVectoredExceptionHandler(1,seh_filter);

    if (!setjmp(jmpBuf)) {
        cpu->jmpBuf = &jmpBuf;
        cpu->run();
    }
    KProcess* process = KSystem::getProcess(processId);
    if (process) {
        thread = process->getThreadById(threadId);
        if (thread) {
            delete thread;
        }
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
    // get these before the thread exits
    KProcess* process = thread->process; 
#ifdef _DEBUG
    DWORD nativeThreadId = GetThreadId((HANDLE)((x64CPU*)thread->cpu)->nativeHandle);    
#endif
    U32 threadId = thread->id;    

    thread->exiting = true;
    BoxedWineCondition* cond = thread->waitingCond;
    if (cond) {
        cond->signal();
    }    
    while (process->getThreadById(threadId)) {
        SDL_Delay(10);
    }
}

void unscheduleCurrentThread() {
    jmp_buf* jmpBuf = ((x64CPU*)KThread::currentThread()->cpu)->jmpBuf;
    longjmp(*jmpBuf,1);
}

void addTimer(KTimer* timer) {
    kpanic("addTimer not implemented yet");
}

void removeTimer(KTimer* timer) {
    if (timer->active)
        kpanic("removeTimer not implemented yet");
}

#ifdef BOXEDWINE_MULTI_THREADED
void ATOMIC_WRITE64(U64* pTarget, U64 value) {
    InterlockedExchange64((volatile LONGLONG *)pTarget, (LONGLONG)value);
}
#endif

#endif