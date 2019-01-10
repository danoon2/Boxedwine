#include "boxedwine.h"
#include <windows.h>
#include <SDL.h>
#include "../source/emulation/cpu/x64/x64cpu.h"
#include "../source/emulation/hardmmu/hard_memory.h"

#ifdef BOXEDWINE_MULTI_THREADED

// for self modifying code this might be a bit slow 0x528f608
U32 getEmulatedEipFromHostRip(KThread* thread, U64 rip) {
    while (!thread->memory->hostToEip[((U32)rip)>>K_PAGE_SHIFT] || !thread->memory->hostToEip[((U32)rip)>>K_PAGE_SHIFT][rip & K_PAGE_MASK]) {
        rip--;
    }
    return thread->memory->hostToEip[((U32)rip)>>K_PAGE_SHIFT][rip & K_PAGE_MASK];
}

LONG WINAPI seh_filter(struct _EXCEPTION_POINTERS *ep) {
    x64CPU* cpu = (x64CPU*)ep->ContextRecord->R13;

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
        cpu->inException = false;
        return EXCEPTION_CONTINUE_SEARCH;
    } 
    cpu->inException = true;
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {      
        U32 inst = *((U32*)ep->ContextRecord->Rip);

        if (inst==0x088B4466 || inst==0xC8048B4A) {            
            U32 page = (U32)ep->ContextRecord->R8;
            U32 offset = (U32)ep->ContextRecord->R9;

            cpu->translateEip(((page << K_PAGE_SHIFT) | offset) - cpu->seg[CS].address);
            if (inst==0xC8048B4A) {
                ep->ContextRecord->Rax = (U64)(cpu->opToAddressPages[page]);
            } else {
                ep->ContextRecord->Rax = (U64)(cpu->opToAddressPages[page][offset]);
            }
            cpu->inException = false;
            return EXCEPTION_CONTINUE_EXECUTION;
        } else {
            cpu->eip.u32 = getEmulatedEipFromHostRip(cpu->thread, ep->ContextRecord->Rip) - cpu->seg[CS].address;
            if ((ep->ExceptionRecord->ExceptionInformation[1] & 0xFFFFFFFF00000000l) == cpu->thread->memory->id) {
                U32 address = (U32)ep->ExceptionRecord->ExceptionInformation[1];

#ifndef __TEST
                BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->thread->memory->executableMemoryMutex);
#endif
                if (cpu->thread->memory->nativeFlags[address>>K_PAGE_SHIFT] & NATIVE_FLAG_READONLY) {
                    /*
                    struct Block* block = decodeBlock(cpu, cpu->eip.u32);
                    struct Op* op = block->ops->next;                                        
                    U64 currentInstructionRip = (U64)(cpu->opToAddressPages[(cpu->eip.u32 + cpu->segAddress[CS])>>PAGE_SHIFT][(cpu->eip.u32 + cpu->segAddress[CS]) & PAGE_MASK]);
                    U8* currentInstruction = (U8*)currentInstructionRip;
                    U32 nextInstructionEip = (cpu->eip.u32+op->eipCount) & (cpu->big?0xFFFFFFFF:0xFFFF);
                    U64 nextInstructionRip = (U64)(cpu->opToAddressPages[(nextInstructionEip + cpu->segAddress[CS])>>PAGE_SHIFT][(nextInstructionEip + cpu->segAddress[CS]) & PAGE_MASK]);
                    S32 currentInstructionLen = (U32)(nextInstructionRip-currentInstructionRip);
                    struct x64_Data data;
                    U32 offset;
                    S32 i;

                    if (currentInstructionLen<5) {
                        kpanic("seh_filter: current instruction modifies code, but it can't be patched because it is less than 5 bytes");
                    }
                    
                    x64_initData(&data, cpu, cpu->eip.u32);                    
                    x64_writeCmd(&data, CMD_SELF_MODIFYING, cpu->eip.u32, op->eipCount);
                    // writeCmd will jmp to the next instruction

                    cpu->memory->x64MemPos+=data.memPos;
                    cpu->memory->x64AvailableMem-=data.memPos;
                    x64_commitMappedAddresses(&data);

                    // jmp instead of call so that stack isn't affected and because writeCmd now always jmp's back
                    currentInstruction[0] = 0xe9; // jmp jd
                    offset = (U32)(data.memStart-currentInstructionRip-5);
                    currentInstruction[1] = (U8)offset;
                    currentInstruction[2] = (U8)(offset >> 8);
                    currentInstruction[3] = (U8)(offset >> 16);
                    currentInstruction[4] = (U8)(offset >> 24);
                    for (i=5;i<currentInstructionLen;i++) {
                        currentInstruction[i]=0x90;
                    }                    
                    ep->ContextRecord->Rip = currentInstructionRip;

                    if (ep->ContextRecord->Rsi >> 32) {
                        ep->ContextRecord->Rdi+=cpu->negSegAddress[op->base];
                        ep->ContextRecord->Rdi+=cpu->negMemOffset;
                    }
                    if (ep->ContextRecord->Rdi >> 32) {
                        ep->ContextRecord->Rdi+=cpu->negSegAddress[ES];
                        ep->ContextRecord->Rdi+=cpu->negMemOffset;
                    }
                    */
                    cpu->inException = false;
                    return EXCEPTION_CONTINUE_EXECUTION;
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
    cpu->inException = false;
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