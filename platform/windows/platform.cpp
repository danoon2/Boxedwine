/*
 *  Copyright (C) 2016  The BoxedWine Team
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
#include <Windows.h>
#include "pixelformat.h"

LONGLONG PCFreq;
LONGLONG CounterStart;

void Platform::startMicroCounter()
{
    LARGE_INTEGER li;

    QueryPerformanceFrequency(&li);

    PCFreq = li.QuadPart;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

ULONGLONG Platform::getMicroCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (li.QuadPart-CounterStart)*1000000/PCFreq;
}

ULONGLONG Platform::getSystemTimeAsMicroSeconds() {
    FILETIME tm;
    ULONGLONG t;

    GetSystemTimeAsFileTime( &tm );
    t = ((ULONGLONG)tm.dwHighDateTime << 32) | (ULONGLONG)tm.dwLowDateTime;
    t-=116444736000000000l;
    t/=10;
    /*
    if (!startTime) {
        startTime = t;
    } else {
        ULONGLONG diff = t - startTime;
        t = startTime+diff/20;
    }
    */
    return t;
}

void Platform::listNodes(const std::string& nativePath, std::vector<ListNodeResult>& results) {
    std::string path;
    WIN32_FIND_DATA findData;
    HANDLE hFind;

    path = nativePath+"\\*.*";;
    hFind = FindFirstFile(path.c_str(), &findData); 
    if(hFind != INVALID_HANDLE_VALUE)  { 		
        do  { 
            if (strcmp(findData.cFileName, ".") && strcmp(findData.cFileName, ".."))  {
                results.push_back(ListNodeResult(findData.cFileName, (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0));
            }
        } while(FindNextFile(hFind, &findData)); 
        FindClose(hFind); 
    }
}

int getPixelFormats(PixelFormat* pfd, int maxPfs) {
    PIXELFORMATDESCRIPTOR p;
    HDC hdc = GetDC(GetDesktopWindow());
    int count = DescribePixelFormat(hdc, 0, 0, NULL);
    int result = 1;
    int i;

    for (i=1;i<=count && result<maxPfs;i++) {
        DescribePixelFormat(hdc, i, sizeof(p), &p);
        if ((p.dwFlags & PFD_SUPPORT_OPENGL) && p.cColorBits<=32 && !(p.dwFlags & PFD_GENERIC_FORMAT)) {
            pfd[result].nSize = 40;
            pfd[result].nVersion = 1;
            pfd[result].dwFlags = p.dwFlags;
            pfd[result].iPixelType = p.iPixelType;
            pfd[result].cColorBits = p.cColorBits;
            pfd[result].cRedBits = p.cRedBits;
            pfd[result].cRedShift = p.cRedShift;
            pfd[result].cGreenBits = p.cGreenBits;
            pfd[result].cGreenShift = p.cGreenShift;
            pfd[result].cBlueBits = p.cBlueBits;
            pfd[result].cBlueShift = p.cBlueShift;
            pfd[result].cAlphaBits = p.cAlphaBits;
            pfd[result].cAlphaShift = p.cAlphaShift;
            pfd[result].cAccumBits = p.cAccumBits;
            pfd[result].cAccumRedBits = p.cAccumRedBits;
            pfd[result].cAccumGreenBits = p.cAccumGreenBits;
            pfd[result].cAccumBlueBits = p.cAccumBlueBits;
            pfd[result].cAccumAlphaBits = p.cAccumAlphaBits;
            pfd[result].cDepthBits = p.cDepthBits;
            pfd[result].cStencilBits = p.cStencilBits;
            pfd[result].cAuxBuffers = p.cAuxBuffers;
            pfd[result].iLayerType = p.iLayerType;
            pfd[result].bReserved = p.bReserved;
            pfd[result].dwLayerMask = p.dwLayerMask;
            pfd[result].dwVisibleMask = p.dwVisibleMask;
            pfd[result].dwDamageMask = p.dwDamageMask;
            result++;
            fprintf(stderr, "Pixel Format: %d bit (%d%d%d%d) %s:%s depth=%d stencil=%d accum=%d\n", p.cColorBits, p.cRedBits, p.cBlueBits, p.cGreenBits, p.cAlphaBits, (p.dwFlags & K_PFD_GENERIC_FORMAT)?"not accelerated":"accelerated", (p.dwFlags & K_PFD_DOUBLEBUFFER)?"double buffered":"single buffered", p.cDepthBits, p.cStencilBits, p.cAccumBits);
        }
    }
    if (result==1) {
        for (i=1;i<=count && result<maxPfs;i++) {
            DescribePixelFormat(hdc, i, sizeof(p), &p);
            if ((p.dwFlags & PFD_SUPPORT_OPENGL) && p.cColorBits<=32) {
                pfd[result].nSize = 40;
                pfd[result].nVersion = 1;
                pfd[result].dwFlags = p.dwFlags;
                pfd[result].iPixelType = p.iPixelType;
                pfd[result].cColorBits = p.cColorBits;
                pfd[result].cRedBits = p.cRedBits;
                pfd[result].cRedShift = p.cRedShift;
                pfd[result].cGreenBits = p.cGreenBits;
                pfd[result].cGreenShift = p.cGreenShift;
                pfd[result].cBlueBits = p.cBlueBits;
                pfd[result].cBlueShift = p.cBlueShift;
                pfd[result].cAlphaBits = p.cAlphaBits;
                pfd[result].cAlphaShift = p.cAlphaShift;
                pfd[result].cAccumBits = p.cAccumBits;
                pfd[result].cAccumRedBits = p.cAccumRedBits;
                pfd[result].cAccumGreenBits = p.cAccumGreenBits;
                pfd[result].cAccumBlueBits = p.cAccumBlueBits;
                pfd[result].cAccumAlphaBits = p.cAccumAlphaBits;
                pfd[result].cDepthBits = p.cDepthBits;
                pfd[result].cStencilBits = p.cStencilBits;
                pfd[result].cAuxBuffers = p.cAuxBuffers;
                pfd[result].iLayerType = p.iLayerType;
                pfd[result].bReserved = p.bReserved;
                pfd[result].dwLayerMask = p.dwLayerMask;
                pfd[result].dwVisibleMask = p.dwVisibleMask;
                pfd[result].dwDamageMask = p.dwDamageMask;
                result++;
                fprintf(stderr, "Pixel Format: %d bit (%d%d%d%d) %s:%s depth=%d stencil=%d accum=%d\n", p.cColorBits, p.cRedBits, p.cBlueBits, p.cGreenBits, p.cAlphaBits, (p.dwFlags & K_PFD_GENERIC_FORMAT)?"not accelerated":"accelerated", (p.dwFlags & K_PFD_DOUBLEBUFFER)?"double buffered":"single buffered", p.cDepthBits, p.cStencilBits, p.cAccumBits);
            }
        }
    }
    return result;
}

#ifdef BOXEDWINE_64BIT_MMU
#include "kmmap.h"
#include "kerror.h"
#include "kfiledescriptor.h"
#include "memory.h"
#include "kprocess.h"
#include "kthread.h"
#include "kobject.h"
#include "kobjectaccess.h"
#include "../../source/emulation/hardmmu/hard_memory.h"

static U32 gran = 0x10;

void allocNativeMemory(struct Memory* memory, U32 page, U32 pageCount, U32 flags) {
    U32 granPage;
    U32 granCount;
    U32 i;    

    granPage = page & ~(gran-1);
    granCount = ((gran - 1) + pageCount + (page - granPage)) / gran;
    for (i=0; i < granCount; i++) {
        if (!(memory->nativeFlags[granPage] & NATIVE_FLAG_COMMITTED)) {
            U32 j;

            if (!VirtualAlloc((void*)((granPage << PAGE_SHIFT) | memory->id), gran << PAGE_SHIFT, MEM_COMMIT, PAGE_READWRITE)) {
                LPSTR messageBuffer = NULL;
                size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
                kpanic("failed to commit memory: %s", messageBuffer);
            }
            memory->allocated+=(gran << PAGE_SHIFT);
            for (j=0;j<gran;j++)
                memory->nativeFlags[granPage+j] |= NATIVE_FLAG_COMMITTED;
        }
        granPage+=gran;
    }
    for (i=0;i<pageCount;i++) {
        memory->flags[page+i] = flags;
        memory->flags[page+i] |= PAGE_IN_RAM;
        memory->ids[page+i] = memory->id;
    }
    memset(getNativeAddress(memory, page << PAGE_SHIFT), 0, pageCount << PAGE_SHIFT);
    //printf("allocated %X - %X\n", page << PAGE_SHIFT, (page+pageCount) << PAGE_SHIFT);
}

void freeNativeMemory(struct KProcess* process, U32 page, U32 pageCount) {
    U32 i;
    U32 granPage;
    U32 granCount;

    for (i=0;i<pageCount;i++) {
        DWORD oldProtect;

        if ((process->memory->nativeFlags[page] & NATIVE_FLAG_READONLY)) {
            if (!VirtualProtect(getNativeAddress(process->memory, (page+i) << PAGE_SHIFT), (1 << PAGE_SHIFT), PAGE_READWRITE, &oldProtect)) {
                LPSTR messageBuffer = NULL;
                size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
                kpanic("failed to unprotect memory: %s", messageBuffer);
            }
            process->memory->nativeFlags[page] &= ~NATIVE_FLAG_READONLY;
        }
        clearPageFromBlockCache(process->memory, NULL, page+i);
        process->memory->flags[page+i] = 0;
        process->memory->ids[page+i] = 0;
    }    

    granPage = page & ~(gran-1);
    granCount = ((gran - 1) + pageCount + (page - granPage)) / gran;
    for (i=0; i < granCount; i++) {
        U32 j;
        BOOL inUse = FALSE;

        for (j=0;j<gran;j++) {
            if (process->memory->flags[granPage+j] & PAGE_IN_RAM) {
                inUse = TRUE;
            }            
        }
        if (!inUse) {
            if (!VirtualFree((void*)((granPage << PAGE_SHIFT) | process->memory->id), gran, MEM_DECOMMIT)) {
                LPSTR messageBuffer = NULL;
                size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
                kpanic("failed to release memory: %s", messageBuffer);
            }
            for (j=0;j<gran;j++) {
                process->memory->nativeFlags[granPage+j]=0;
            }
            process->memory->allocated-=(gran << PAGE_SHIFT);
        }
        granPage+=gran;
    }    
}

static void* reserveNext4GBMemory() {
    void* p;
    U64 i=1;

    p = (void*)(i << 32);
    while (VirtualAlloc(p, 0x100000000l, MEM_RESERVE, PAGE_READWRITE)==0) {
        i++;
        p = (void*)(i << 32);
    } 
    return p;
}

void reserveNativeMemory(struct Memory* memory) {    
    memory->id = (U64)reserveNext4GBMemory();    
}

void releaseNativeMemory(struct Memory* memory) {
    U32 i;

    if (!VirtualFree((void*)memory->id, 0, MEM_RELEASE)) {
        LPSTR messageBuffer = NULL;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
        kpanic("failed to release memory: %s", messageBuffer);
    }
    for (i=0;i<NUMBER_OF_PAGES;i++) {
        clearPageFromBlockCache(memory, NULL, i);
    }
    memset(memory->flags, 0, sizeof(memory->flags));
    memset(memory->nativeFlags, 0, sizeof(memory->nativeFlags));
    memset(memory->ids, 0, sizeof(memory->ids));
    memory->allocated = 0;
#ifdef BOXEDWINE_VM
    freeExecutableMemory(memory);
    {
        U32 i;
        for (i=0;i<NUMBER_OF_PAGES;i++) {
            if (memory->process->opToAddressPages[i]) {
                kfree(memory->process->opToAddressPages[i], KALLOC_IP_CACHE);
                memory->process->opToAddressPages[i] = 0;
            }
            if (memory->process->hostToEip[i]) {
                kfree(memory->process->hostToEip[i], KALLOC_IP_CACHE);
                memory->process->hostToEip[i] = 0;
            }
        }
    }
    memory->x64Mem = 0;
    memory->x64MemPos = 0;
    memory->x64AvailableMem = 0;
    if (memory->executableMemoryMutex) {
        SDL_DestroyMutex(memory->executableMemoryMutex);
        memory->executableMemoryMutex = NULL;
    }
#endif
}

void makeCodePageReadOnly(struct Memory* memory, U32 page) {
    U32 granPage;
    DWORD oldProtect;

    granPage = page & ~(gran-1);
    // :TODO: would the granularity ever be more than 4k?  should I check: SYSTEM_INFO System_Info; GetSystemInfo(&System_Info);
    if (!(memory->nativeFlags[page] & NATIVE_FLAG_READONLY)) {
        if (!VirtualProtect(getNativeAddress(memory, page << PAGE_SHIFT), (1 << PAGE_SHIFT), PAGE_READONLY, &oldProtect)) {
            LPSTR messageBuffer = NULL;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
            kpanic("failed to protect memory: %s", messageBuffer);
        }
        memory->nativeFlags[page] |= NATIVE_FLAG_READONLY;
    }
}

BOOL clearCodePageReadOnly(struct Memory* memory, U32 page) {
    U32 granPage;
    DWORD oldProtect;
    BOOL result = FALSE;

    granPage = page & ~(gran-1);
    // :TODO: would the granularity ever be more than 4k?  should I check: SYSTEM_INFO System_Info; GetSystemInfo(&System_Info);
    if (memory->nativeFlags[page] & NATIVE_FLAG_READONLY) {
        if (!VirtualProtect(getNativeAddress(memory, page << PAGE_SHIFT), (1 << PAGE_SHIFT), PAGE_READWRITE, &oldProtect)) {
            LPSTR messageBuffer = NULL;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
            kpanic("failed to unprotect memory: %s", messageBuffer);
        }
        memory->nativeFlags[page] &= ~NATIVE_FLAG_READONLY;
        result = TRUE;
    }
    return result;
}

// for self modifying code this might be a bit slow 0x528f608
U32 getEmulatedEipFromHostRip(struct KThread* thread, U64 rip) {
    while (!thread->process->hostToEip[((U32)rip)>>PAGE_SHIFT] || !thread->process->hostToEip[((U32)rip)>>PAGE_SHIFT][rip & PAGE_MASK]) {
        rip--;
    }
    return thread->process->hostToEip[((U32)rip)>>PAGE_SHIFT][rip & PAGE_MASK];
}

void seg_mapper(struct KThread* thread, U32 address) ;
void x64_cmdEntry(struct CPU* cpu);

int seh_filter(unsigned int code, struct _EXCEPTION_POINTERS* ep, struct KThread* thread)
{
    struct CPU* cpu = &thread->cpu;

    if (cpu->inException) {
        EAX = (U32)ep->ContextRecord->Rax;
        ECX = (U32)ep->ContextRecord->Rcx;
        EDX = (U32)ep->ContextRecord->Rdx;
        EBX = (U32)ep->ContextRecord->Rbx;
        ESP = (U32)ep->ContextRecord->R11;
        EBP = (U32)ep->ContextRecord->Rbp;
        ESI = (U32)ep->ContextRecord->Rsi;
        EDI = (U32)ep->ContextRecord->Rdi;
        seg_mapper(thread, (U32)ep->ExceptionRecord->ExceptionInformation[1]);
        cpu->inException = FALSE;
        return EXCEPTION_CONTINUE_SEARCH;
    } 
    cpu->inException = TRUE;
    if (code == EXCEPTION_ACCESS_VIOLATION) {
#ifdef BOXEDWINE_VM           
        U32 inst = *((U32*)ep->ContextRecord->Rip);

        if (inst==0x288B4466 || inst==0xE8048B4A) {            
            U32 page = (U32)ep->ContextRecord->R8;
            U32 offset = (U32)ep->ContextRecord->R13;

            x64_translateEip(cpu, ((page << PAGE_SHIFT) | offset) - cpu->segAddress[CS]);
            if (inst==0xE8048B4A) {
                ep->ContextRecord->Rax = (U64)(cpu->opToAddressPages[page]);
            } else {
                ep->ContextRecord->Rax = (U64)(cpu->opToAddressPages[page][offset]);
            }
            cpu->inException = FALSE;
            return EXCEPTION_CONTINUE_EXECUTION;
        } else if ((U16)inst==0x53cd) { 
            U32 eip = cpu->eip.u32;

            if (cpu->done) {
                cpu->inException = FALSE;
                return EXCEPTION_EXECUTE_HANDLER;
            }
            ep->ContextRecord->Rip+=2;
            EAX = (U32)ep->ContextRecord->Rax;
            ECX = (U32)ep->ContextRecord->Rcx;
            EDX = (U32)ep->ContextRecord->Rdx;
            EBX = (U32)ep->ContextRecord->Rbx;
            ESP = (U32)ep->ContextRecord->R11;
            EBP = (U32)ep->ContextRecord->Rbp;
            ESI = (U32)ep->ContextRecord->Rsi;
            EDI = (U32)ep->ContextRecord->Rdi;
            cpu->flags &=~ FMASK_X64;
            cpu->flags |= ep->ContextRecord->EFlags & FMASK_X64;            
            x64_cmdEntry(cpu);            
            ep->ContextRecord->Rax = EAX;
            ep->ContextRecord->Rcx = ECX;
            ep->ContextRecord->Rdx = EDX;
            ep->ContextRecord->Rbx = EBX;
            ep->ContextRecord->R11 = ESP;
            ep->ContextRecord->Rbp = EBP;
            ep->ContextRecord->Rsi = ESI;
            ep->ContextRecord->Rdi = EDI;  
            ep->ContextRecord->EFlags &=~ FMASK_X64;
            ep->ContextRecord->EFlags |= (cpu->flags & FMASK_X64);
            if (cpu->done) {
                cpu->inException = FALSE;
                return EXCEPTION_EXECUTE_HANDLER;
            }
            if (eip!=cpu->eip.u32) {
                U32 page = cpu->eip.u32 >> PAGE_SHIFT;
                U32 offset = cpu->eip.u32 & PAGE_MASK;
                                   
                x64_translateEip(cpu, (page << PAGE_SHIFT) | offset);
                ep->ContextRecord->Rip = (U64)(cpu->opToAddressPages[page][offset]);                
                ep->ContextRecord->R10 = (U64)cpu->memory->id;
            }
            cpu->inException = FALSE;
            return EXCEPTION_CONTINUE_EXECUTION;
        } else {
            cpu->eip.u32 = getEmulatedEipFromHostRip(thread, ep->ContextRecord->Rip) - cpu->segAddress[CS];
            if ((ep->ExceptionRecord->ExceptionInformation[1] & 0xFFFFFFFF00000000l) == cpu->memory->id) {
                U32 address = (U32)ep->ExceptionRecord->ExceptionInformation[1];

#ifndef __TEST
                SDL_LockMutex(cpu->memory->executableMemoryMutex);
#endif
                if (cpu->memory->nativeFlags[address>>PAGE_SHIFT] & NATIVE_FLAG_READONLY) {               
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
#ifndef __TEST
                    SDL_UnlockMutex(cpu->memory->executableMemoryMutex);
#endif
                    cpu->inException = FALSE;
                    return EXCEPTION_CONTINUE_EXECUTION;
                }
#ifndef __TEST
                SDL_UnlockMutex(cpu->memory->executableMemoryMutex);
#endif
            }
             
            EAX = (U32)ep->ContextRecord->Rax;
            ECX = (U32)ep->ContextRecord->Rcx;
            EDX = (U32)ep->ContextRecord->Rdx;
            EBX = (U32)ep->ContextRecord->Rbx;
            ESP = (U32)ep->ContextRecord->R11;
            EBP = (U32)ep->ContextRecord->Rbp;
            ESI = (U32)ep->ContextRecord->Rsi;
            EDI = (U32)ep->ContextRecord->Rdi;
            seg_mapper(thread, (U32)ep->ExceptionRecord->ExceptionInformation[1]);
        }
#else
        U32 address = getHostAddress(thread, (void*)ep->ExceptionRecord->ExceptionInformation[1]);
        if (thread->process->memory->nativeFlags[address>>PAGE_SHIFT] & NATIVE_FLAG_READONLY) {
            DWORD oldProtect;
            U32 page = address>>PAGE_SHIFT;
            if (!VirtualProtect(getNativeAddress(thread->process->memory, address & 0xFFFFF000), (1 << PAGE_SHIFT), PAGE_READWRITE, &oldProtect)) {
                LPSTR messageBuffer = NULL;
                size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
                kpanic("failed to unprotect memory: %s", messageBuffer);
            }
            thread->process->memory->nativeFlags[page] &= ~NATIVE_FLAG_READONLY;
            clearPageFromBlockCache(thread->process->memory, thread, page);
            return EXCEPTION_CONTINUE_EXECUTION;
        } else {
            seg_mapper(thread->process->memory, address);
            return EXCEPTION_EXECUTE_HANDLER;
        }
#endif
    }
    cpu->inException = FALSE;
    return EXCEPTION_CONTINUE_SEARCH;
}

#ifdef BOXEDWINE_VM
#include "x64dynamic.h"

void getRegs(U64* regs) {
    CONTEXT context;
    
    RtlCaptureContext(&context);
    regs[0] = context.Rax;
    regs[1] = context.Rcx;
    regs[2] = context.Rdx;
    regs[3] = context.Rbx;
    regs[4] = context.Rsp;
    regs[5] = context.Rbp;
    regs[6] = context.Rsi;
    regs[7] = context.Rdi;
    regs[8] = context.R8;
    regs[9] = context.R9;
    regs[10] = context.R10;
    regs[11] = context.R11;
    regs[12] = context.R12;
    regs[13] = context.R13;
    regs[14] = context.R14;
    regs[15] = context.R15;
}

void setRegs(U64* regs) {
}

typedef void (*StartCPU)();

void pauseThread(struct KThread* thread) {
    DWORD dwVal = SuspendThread((HANDLE)thread->nativeHandle);
    CONTEXT ctx;
    struct CPU* cpu = &thread->cpu;

	if (INFINITE == dwVal) {
	    kpanic("pauseThread failed");
	}
    // :TODO: capture context
    
	ctx.ContextFlags = CONTEXT_CONTROL;    
	GetThreadContext((HANDLE)thread->nativeHandle, &ctx);

    EAX = (U32)ctx.Rax;
    ECX = (U32)ctx.Rcx;
    EDX = (U32)ctx.Rdx;
    EBX = (U32)ctx.Rbx;
    ESP = (U32)ctx.R11;
    EBP = (U32)ctx.Rbp;
    ESI = (U32)ctx.Rsi;
    EDI = (U32)ctx.Rdi;
    cpu->flags = ctx.EFlags;
    if ((ctx.Rip & 0xFFFFFFFF00000000l) != thread->process->memory->id) {
        kpanic("Pausing a thread in the middle of a cmd/syscall is not supported");
    }
    cpu->eip.u32 = getEmulatedEipFromHostRip(thread, ctx.Rip);
}

void resumeThread(struct KThread* thread) {
    CONTEXT ctx;
    struct CPU* cpu = &thread->cpu;

    ctx.Rax = EAX;
    ctx.Rcx = ECX;
    ctx.Rdx = EDX;
    ctx.Rbp = EBX;
    ctx.R11 = ESP;
    ctx.Rbp = EBP;
    ctx.Rsi = ESI;
    ctx.Rdi = EDI;
    ctx.Rip = (U64)x64_translateEip(cpu, cpu->eip.u32);
    ctx.ContextFlags = CONTEXT_CONTROL;    
    SetThreadContext((HANDLE)thread->nativeHandle, &ctx);
    ResumeThread((HANDLE)thread->nativeHandle);
}

void killThread(struct KThread* thread) {
    thread->cpu->done = TRUE;
    if (thread->waitingCondition) {
        BOXEDWINE_SIGNAL_ALL(thread->waitingCondition);
    }
    if (WaitForSingleObject((HANDLE)thread->nativeHandle, 1000000)!=WAIT_OBJECT_0) {
        klog("x64: killThread failed to gracefully stop thread after 1 second, it will now be terminated");
        TerminateThread((HANDLE)thread->nativeHandle, 0);        
    }
}

U32 platformThreadCount = 0;

DWORD WINAPI platformThreadProc(LPVOID lpThreadParameter) {
    struct KThread* thread = (struct KThread*)lpThreadParameter;
    struct CPU* cpu = &thread->cpu;
    U32 i;
    jmp_buf jmpBuf;

    cpu->enterHost = x64_cmdEntry;
    cpu->memOffset = cpu->memory->id;
    cpu->negMemOffset = (U64)(-(S64)cpu->memOffset);
    for (i=0;i<6;i++) {
        cpu->negSegAddress[i] = (U32)(-((S32)(cpu->segAddress[i])));
    }

    // :TODO: hopefully this will eventually go away.  For now this prevents a signal from being generated which isn't handled yet
    SDL_Delay(50);     
    __try {       
        if (!setjmp(jmpBuf)) {
            StartCPU startCPU = (StartCPU)x64_initCPU(cpu);        
            cpu->jmpBuf = &jmpBuf;            
            startCPU();
        }
    } __except(seh_filter(GetExceptionCode(), GetExceptionInformation(), thread)) {
        thread->cpu->nextBlock = 0;
    }
    if (thread->endCondition && thread->endMutex) {
        BOXEDWINE_LOCK(thread, thread->endMutex);
        BOXEDWINE_SIGNAL(thread->endCondition);
        BOXEDWINE_UNLOCK(thread, thread->endMutex);
    }
    platformThreadCount--;
    if (platformThreadCount==0) {
        SDL_Event sdlevent;
        sdlevent.type = SDL_QUIT;        
        SDL_PushEvent(&sdlevent);
    }
    return 0;
}

void platformStartThread(struct KThread* thread) {
    platformThreadCount++;
    thread->nativeHandle = (U64)CreateThread(NULL, 0, platformThreadProc, thread, 0, 0);        
}

void* allocExecutable64kBlock(struct Memory* memory) {
    U32 i;

    if (!memory->executableMemory) {
        memory->executableMemory = reserveNext4GBMemory();
    }
    for (i=0;i<0x10000;i++) {
        if (!memory->executable64kBlocks[i]) {
            memory->executable64kBlocks[i]=1;
            memory->x64AvailableMem += 64*1024; 
            return VirtualAlloc(memory->executableMemory+i*64*1024, 64*1024, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        }
    }
    kpanic("Ran out of code pages in x64dynamic");
    return 0;
}

void freeExecutableMemory(struct Memory* memory) {
    VirtualFree(memory->executableMemory, 0, MEM_RELEASE);
    memset(memory->executable64kBlocks, 0, sizeof(memory->executable64kBlocks));
    memory->executableMemory = NULL;
}

#else
void platformRunThreadSlice(struct KThread* thread) {
    __try {
        runThreadSlice(thread);
    } __except(seh_filter(GetExceptionCode(), GetExceptionInformation(), thread)) {
        thread->cpu->nextBlock = 0;
        thread->cpu->timeStampCounter+=thread->cpu->blockCounter & 0x7FFFFFFF;
    }
}
#endif
#endif
