#include "boxedwine.h"

static BOXEDWINE_MUTEX ramMutex;
int allocatedRamPages;

// native x64 code instructions sometimes assume proper alignment, so make sure when they align an emulated address, the hardware address is also aligned the same 
// F-16 demo installer will crash if the page was allocated with just new on x64
#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "../../util/ptrpool.h"

static PtrPool<U8> freePages(0);
static std::unordered_map<U8*, U32> ramCounts;

U8* ramPageAlloc() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U8* result = freePages.get();
    if (result) {
        ramCounts[result] = 1;
        return result;
    }

    U8* pages = (U8*)Platform::alloc64kBlock(1);
#ifdef _DEBUG1 
    // the page before and after the allocated ram will be allocated too and set to no permission so that read/write will generate exception
    Platform::updateNativePermission((U64)(pages), 0, K_PAGE_SIZE);
    pages += K_PAGE_SIZE;
    for (int i = 0; i < 7; i++) {
        freePages.put(pages);
        Platform::updateNativePermission((U64)(pages + K_PAGE_SIZE), 0, K_PAGE_SIZE);
        pages += 2 * K_PAGE_SIZE;
    }
#else
    for (int i = 0; i < 16; i++) {
        freePages.put(pages);
        pages += K_PAGE_SIZE;
    }
#endif
    return ramPageAlloc();
}

void ramPageIncRef(U8* ram) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    if (ramCounts[ram] == 255) {
        kpanic("max ram page ref count reached");
    }
    ramCounts[ram]++;
}

void ramPageDecRef(U8* ram) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    ramCounts[ram]--;
    if (ramCounts[ram] == 0) {
        allocatedRamPages--;
        Platform::updateNativePermission((U64)ram, 0, K_PAGE_SIZE);
    }
}

U32 ramPageRefCount(U8* ram) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    return ramCounts[ram];
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