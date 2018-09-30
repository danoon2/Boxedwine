#include "boxedwine.h"
#ifdef BOXEDWINE_DYNAMIC
#include <Windows.h>

void freeExecutable64kBlock(void* p) {
    VirtualFree(p, 0, MEM_RELEASE);
}

void* allocExecutable64kBlock() {
    void* result = VirtualAlloc(NULL, 64*1024, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!result) {
        kpanic("allocExecutable64kBlock: VirtualAlloc failed");
    }
    return result;
}
#endif