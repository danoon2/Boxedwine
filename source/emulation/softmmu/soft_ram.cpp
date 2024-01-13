#include "boxedwine.h"

static BOXEDWINE_MUTEX ramMutex;

U8* ramPageAlloc() {
    U8* ram = new U8[K_PAGE_SIZE+1];
    memset(ram, 0, K_PAGE_SIZE);
    ram[K_PAGE_SIZE] = 1;
    return ram;
}

void ramPageIncRef(U8* ram) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    if (ram[K_PAGE_SIZE]==255) {
        kpanic("max ram page ref count reached");
    }
    ram[K_PAGE_SIZE]++;
}

void ramPageDecRef(U8* ram) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    ram[K_PAGE_SIZE]--;
    if (ram[K_PAGE_SIZE]==0)
        delete[] ram;
}

U32 ramPageRefCount(U8* ram) {
    return ram[K_PAGE_SIZE];
}