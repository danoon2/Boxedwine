#include "boxedwine.h"

static BOXEDWINE_MUTEX ramMutex;
int allocatedRamPages;

// native x64 code instructions sometimes assume proper alignment, so make sure when they align an emulated address, the hardware address is also aligned the same 
// F-16 demo installer will crash if the page was allocated with just new on x64
#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "../../util/ptrpool.h"

static PtrPool<U8, false> freeRamPages(0);
static BHashTable<U8*, U32> ramCounts;

U8* ramPageAlloc() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U8* result = freeRamPages.get();
    if (result) {
        ramCounts.set(result, 1);
        allocatedRamPages++;
        return result;
    }

    U8* pages = (U8*)Platform::alloc64kBlock(1);
#ifdef _DEBUG
    // the page before and after the allocated ram will be allocated too and set to no permission so that read/write will generate exception
    Platform::updateNativePermission((U64)(pages), 0, K_PAGE_SIZE);
    pages += K_PAGE_SIZE;
    for (int i = 0; i < 7; i++) {
        freeRamPages.put(pages);
        Platform::updateNativePermission((U64)(pages + K_PAGE_SIZE), 0, K_PAGE_SIZE);
        pages += 2 * K_PAGE_SIZE;
    }
#else
    for (int i = 0; i < 16; i++) {
        freeRamPages.put(pages);
        pages += K_PAGE_SIZE;
    }
#endif
    return ramPageAlloc();
}

void ramPageIncRef(U8* ram) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U32 count = 0;
    if (!ramCounts.get(ram, count) || count == 255) {
        kpanic("max ram page ref count reached");
    }
    ramCounts.set(ram, count+1);
}

void ramPageDecRef(U8* ram) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U32 count = 0;
    if (!ramCounts.get(ram, count)) {
        kpanic("ramPageDecRef failed to get count");
    }

    count--;
    if (count == 0) {
        allocatedRamPages--;
        memset(ram, 0, K_PAGE_SIZE);
        freeRamPages.put(ram);
        ramCounts.remove(ram);
    } else {
        ramCounts.set(ram, count);
    }
}

U32 ramPageRefCount(U8* ram) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U32 count = 0;
    ramCounts.get(ram, count);
    return count;
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
