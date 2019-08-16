#include "boxedwine.h"
#ifdef BOXEDWINE_DYNAMIC
#include <Windows.h>

void freeExecutable64kBlock(void* p) {
    VirtualFree(p, 0, MEM_RELEASE);
}

void* allocExecutable64kBlock(int count) {
    void* result = VirtualAlloc(NULL, 64*1024*count, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!result) {
        kpanic("allocExecutable64kBlock: VirtualAlloc failed");
    }
    return result;
}
#endif