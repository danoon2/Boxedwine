#include "boxedwine.h"

static BOXEDWINE_MUTEX ramMutex;
int allocatedRamPages;

#if defined(_DEBUG) && defined(BOXEDWINE_BINARY_TRANSLATOR1)
#include "../../util/ptrpool.h"

static PtrPool<U8> freePages(0);

U8* ramPageAlloc() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U8* result = freePages.get();
    if (result) {
        result[-1] = 1;
        return result;
    }
    // not efficient, 3 pages allocated per page returned, prev page holds ref count, middle page hold actual page, 3rd page is marked no permission to catch overwrites
    U8* pages = (U8*)Platform::allocExecutable64kBlock(1);
    pages += K_PAGE_SIZE;
    for (int i = 0; i < 5; i++) {        
        Platform::updateNativePermission((U64)(pages+K_PAGE_SIZE), 0, K_PAGE_SIZE);
        freePages.put(pages);
        pages += 3 * K_PAGE_SIZE;
    }
    return ramPageAlloc();
}

void ramPageIncRef(U8* ram) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    if (ram[-1] == 255) {
        kpanic("max ram page ref count reached");
    }
    ram[-1]++;
}

void ramPageDecRef(U8* ram) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    ram[-1]--;
    if (ram[-1] == 0) {
        allocatedRamPages--;
        Platform::updateNativePermission((U64)ram, 0, K_PAGE_SIZE);
    }
}

U32 ramPageRefCount(U8* ram) {
    return ram[-1];
}
#else
U8* ramPageAlloc() {
    U8* ram = new U8[K_PAGE_SIZE+1];
    memset(ram, 0, K_PAGE_SIZE);
    ram[K_PAGE_SIZE] = 1;
    allocatedRamPages++;
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
    if (ram[K_PAGE_SIZE] == 0) {
        allocatedRamPages--;
        delete[] ram;
    }
}

U32 ramPageRefCount(U8* ram) {
    return ram[K_PAGE_SIZE];
}
#endif