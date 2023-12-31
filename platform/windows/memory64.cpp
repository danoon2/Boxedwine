#include "boxedwine.h"
#include <windows.h>

#ifdef BOXEDWINE_64BIT_MMU

#ifndef BOXEDWINE_MULTI_THREADED
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

void platformRunThreadSlice(KThread* thread) {
    __try {
        runThreadSlice(thread);
    } __except(seh_filter(GetExceptionCode(), GetExceptionInformation(), thread)) {
        thread->cpu->nextBlock = NULL;
    }
}
#endif
#endif