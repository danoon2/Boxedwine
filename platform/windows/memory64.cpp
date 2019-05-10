#include "boxedwine.h"
#include <windows.h>

#ifdef BOXEDWINE_64BIT_MMU
static U32 gran = 0x10;

#include "../../source/emulation/hardmmu/hard_memory.h"

void allocNativeMemory(Memory* memory, U32 page, U32 pageCount, U32 flags) {
    U32 granPage;
    U32 granCount;
    U32 i;    

    granPage = page & ~(gran-1);
    granCount = ((gran - 1) + pageCount + (page - granPage)) / gran;
    for (i=0; i < granCount; i++) {
        if (!(memory->nativeFlags[granPage] & NATIVE_FLAG_COMMITTED)) {
            U32 j;

            if (!VirtualAlloc((void*)((granPage << K_PAGE_SHIFT) | memory->id), gran << K_PAGE_SHIFT, MEM_COMMIT, PAGE_READWRITE)) {
                LPSTR messageBuffer = NULL;
                size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
                kpanic("failed to commit memory: %s", messageBuffer);
            }
            memory->allocated+=(gran << K_PAGE_SHIFT);
            for (j=0;j<gran;j++)
                memory->nativeFlags[granPage+j] |= NATIVE_FLAG_COMMITTED;
        }
        granPage+=gran;
    }
    for (i=0;i<pageCount;i++) {
        memory->flags[page+i] = flags;
        memory->ids[page+i] = memory->id;
    }
    memset(getNativeAddress(memory, page << K_PAGE_SHIFT), 0, pageCount << K_PAGE_SHIFT);
    //printf("allocated %X - %X\n", page << PAGE_SHIFT, (page+pageCount) << PAGE_SHIFT);
}

void freeNativeMemory(Memory* memory, U32 page, U32 pageCount) {
    U32 i;
    U32 granPage;
    U32 granCount;

    for (i=0;i<pageCount;i++) {
        DWORD oldProtect;

        if ((memory->nativeFlags[page] & NATIVE_FLAG_CODEPAGE_READONLY)) {
            if (!VirtualProtect(getNativeAddress(memory, (page+i) << K_PAGE_SHIFT), (1 << K_PAGE_SHIFT), PAGE_READWRITE, &oldProtect)) {
                LPSTR messageBuffer = NULL;
                size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
                kpanic("failed to unprotect memory: %s", messageBuffer);
            }
            memory->nativeFlags[page] &= ~NATIVE_FLAG_CODEPAGE_READONLY;
        }
        memory->clearCodePageFromCache(page+i);
        memory->flags[page+i] = 0;
        memory->ids[page+i] = 0;
    }    

    granPage = page & ~(gran-1);
    granCount = ((gran - 1) + pageCount + (page - granPage)) / gran;
    for (i=0; i < granCount; i++) {
        U32 j;
        BOOL inUse = FALSE;

        for (j=0;j<gran;j++) {
            if (memory->ids[granPage+j]) {
                inUse = TRUE;
                break;
            }            
        }
        if (!inUse) {
            if (!VirtualFree((void*)((granPage << K_PAGE_SHIFT) | memory->id), gran, MEM_DECOMMIT)) {
                LPSTR messageBuffer = NULL;
                size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
                kpanic("failed to release memory: %s", messageBuffer);
            }
            for (j=0;j<gran;j++) {
                memory->nativeFlags[granPage+j]=0;
            }
            memory->allocated-=(gran << K_PAGE_SHIFT);
        }
        granPage+=gran;
    }  
}

#ifdef BOXEDWINE_X64
void allocExecutable64kBlock(Memory* memory, U32 page) {
    if (!VirtualAlloc((void*)((page << K_PAGE_SHIFT) | memory->executableMemoryId), 64*1024, MEM_COMMIT, PAGE_EXECUTE_READWRITE)) {
        LPSTR messageBuffer = NULL;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
        kpanic("failed to commit memory: %s", messageBuffer);
    }
}
#endif

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

void reserveNativeMemory(Memory* memory) {    
    memory->id = (U64)reserveNext4GBMemory();
#ifdef BOXEDWINE_X64
    memory->executableMemoryId = (U64)reserveNext4GBMemory();
    memory->nextExecutablePage = 0;
#endif
}

void releaseNativeMemory(Memory* memory) {
    U32 i;

    for (i=0;i<K_NUMBER_OF_PAGES;i++) {
        memory->clearCodePageFromCache(i);
    }

    if (!VirtualFree((void*)memory->id, 0, MEM_RELEASE)) {
        LPSTR messageBuffer = NULL;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
        kpanic("failed to release memory: %s", messageBuffer);
    }    
    memset(memory->flags, 0, sizeof(memory->flags));
    memset(memory->nativeFlags, 0, sizeof(memory->nativeFlags));
    memset(memory->ids, 0, sizeof(memory->ids));    
    memory->allocated = 0;
#ifdef BOXEDWINE_X64
    memory->executableMemoryReleased();    
    if (!VirtualFree((void*)memory->executableMemoryId, 0, MEM_RELEASE)) {
        LPSTR messageBuffer = NULL;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
        kpanic("failed to release executable memory: %s", messageBuffer);
    } 
    memory->executableMemoryId = 0;
#endif
}

void makeCodePageReadOnly(Memory* memory, U32 page) {
    DWORD oldProtect;

    // :TODO: would the granularity ever be more than 4k?  should I check: SYSTEM_INFO System_Info; GetSystemInfo(&System_Info);
    if (!(memory->nativeFlags[page] & NATIVE_FLAG_CODEPAGE_READONLY)) {
        if (!VirtualProtect(getNativeAddress(memory, page << K_PAGE_SHIFT), (1 << K_PAGE_SHIFT), PAGE_READONLY, &oldProtect)) {
            LPSTR messageBuffer = NULL;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
            kpanic("failed to protect memory: %s", messageBuffer);
        }
        memory->nativeFlags[page] |= NATIVE_FLAG_CODEPAGE_READONLY;
    }
}

bool clearCodePageReadOnly(Memory* memory, U32 page) {
    DWORD oldProtect;
    bool result = false;

    // :TODO: would the granularity ever be more than 4k?  should I check: SYSTEM_INFO System_Info; GetSystemInfo(&System_Info);
    if (memory->nativeFlags[page] & NATIVE_FLAG_CODEPAGE_READONLY) {
        if (!VirtualProtect(getNativeAddress(memory, page << K_PAGE_SHIFT), (1 << K_PAGE_SHIFT), PAGE_READWRITE, &oldProtect)) {
            LPSTR messageBuffer = NULL;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
            kpanic("failed to unprotect memory: %s", messageBuffer);
        }
        memory->nativeFlags[page] &= ~NATIVE_FLAG_CODEPAGE_READONLY;
        result = true;
    }
    return result;
}

static int seh_filter(unsigned int code, struct _EXCEPTION_POINTERS* ep, KThread* thread)
{
    if (code == EXCEPTION_ACCESS_VIOLATION) {
        U32 address = getHostAddress(thread, (void*)ep->ExceptionRecord->ExceptionInformation[1]);
        if (thread->process->memory->nativeFlags[address>>K_PAGE_SHIFT] & NATIVE_FLAG_CODEPAGE_READONLY) {
            DWORD oldProtect;
            U32 page = address>>K_PAGE_SHIFT;

            if (!VirtualProtect(getNativeAddress(thread->process->memory, address & 0xFFFFF000), (1 << K_PAGE_SHIFT), PAGE_READWRITE, &oldProtect)) {
                LPSTR messageBuffer = NULL;
                size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
                kpanic("failed to unprotect memory: %s", messageBuffer);
            }
            thread->process->memory->nativeFlags[page] &= ~NATIVE_FLAG_CODEPAGE_READONLY;
            thread->process->memory->clearCodePageFromCache(page);
            return EXCEPTION_CONTINUE_EXECUTION;
        } else {
            thread->seg_mapper(address, ep->ExceptionRecord->ExceptionInformation[0]==0, ep->ExceptionRecord->ExceptionInformation[0]!=0, false);
            return EXCEPTION_EXECUTE_HANDLER;
        }
    }   
    return EXCEPTION_CONTINUE_SEARCH;
}
#ifndef BOXEDWINE_MULTI_THREADED
void platformRunThreadSlice(KThread* thread) {
    __try {
        runThreadSlice(thread);
    } __except(seh_filter(GetExceptionCode(), GetExceptionInformation(), thread)) {
        thread->cpu->nextBlock = NULL;
    }
}
#endif
#endif