#include "boxedwine.h"
#include "soft_ram.h"
#include "kmemory.h"

#define NATIVE_PAGE_REF_COUNT 0

static BOXEDWINE_MUTEX ramMutex;
int allocatedRamPages;

class RefCounts {
public:
    U16 refCount : 15;
    U16 isNative : 1;
};

// not static so that the JIT and BT cores can access it directly
U8* ramPages[K_NUMBER_OF_PAGES];

static RefCounts refCounts[K_NUMBER_OF_PAGES];
static std::vector<U32> freeIndexes;
static U32 highWaterIndex = 1; // if nothing in freeIndex then we pull from here

static U32 allocIndex() {
    if (freeIndexes.size()) {
        U32 result = freeIndexes.back();
        freeIndexes.pop_back();
        return result;
    }
    if (highWaterIndex >= K_NUMBER_OF_PAGES) {
        kpanic("soft_ram.cpp getFreeIndex: ran out of emulated RAM (4GB)");
    }
    U32 result = highWaterIndex;
    highWaterIndex++;
    return result;
}

static void freeIndex(U32 index) {
    freeIndexes.push_back(index);
}

// native x64 code instructions sometimes assume proper alignment, so make sure when they align an emulated address, the hardware address is also aligned the same 
// F-16 demo installer will crash if the page was allocated with just new on x64
#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "../../util/ptrpool.h"

static PtrPool<U8, false> freeRamPages(0);

void shutdownRam() {
    freeRamPages.deleteAll();
}

RamPage ramPageAlloc() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);    
    U32 index = allocIndex();

    if (!ramPages[index]) {
        U8* pages = (U8*)Platform::alloc64kBlock(1);

        ramPages[index] = pages;
        pages += K_PAGE_SIZE;
        for (int i = 0; i < 7; i++) {
            freeIndex(highWaterIndex);
            ramPages[highWaterIndex++] = pages;
            pages += K_PAGE_SIZE;
        }
    }
    memset(ramPages[index], 0, K_PAGE_SIZE);
    refCounts[index].refCount = 1;
    allocatedRamPages++;
    return index;
}

RamPage ramPageAllocNative(U8* native) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    bool found = false;
    U32 foundIndex = 0;

    for (U32 i = 0; i < freeIndexes.size(); i++) {
        U32 index = freeIndexes.at(i);
        if (!ramPages[index]) {
            found = true;
            foundIndex = index;
            freeIndexes.erase(freeIndexes.begin() + i);
            break;
        }
    }
    if (!found) {
        foundIndex = highWaterIndex++;
    }
    ramPages[foundIndex] = native;
    refCounts[foundIndex].refCount = 1;
    return foundIndex;
}

#else
RamPage ramPageAlloc() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U32 index = allocIndex();

    if (!ramPages[index]) {
        ramPages[index] = new U8[K_PAGE_SIZE];
    }
    memset(ramPages[index], 0, K_PAGE_SIZE);
    refCounts[index].refCount = 1;
    allocatedRamPages++;
    return index;
}

RamPage ramPageAllocNative(U8* native) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U32 index = allocIndex();

    if (ramPages[index]) {
        delete[] ramPages[index];
    }
    ramPages[index] = native;
    refCounts[index].refCount = 1;
    return index;
}

void shutdownRam() {
    for (int i = 0; i < K_NUMBER_OF_PAGES; i++) {
        if (!refCounts[i].isNative && ramPages[i]) {
            delete[] ramPages[i];
        }
        refCounts[i].refCount = 0;
        refCounts[i].isNative = 0;
        ramPages[i] = nullptr;
    }
}

#endif

void ramPageRetain(RamPage page) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    refCounts[page].refCount++;
}

U32 ramPageUseCount(RamPage page) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    return refCounts[page].refCount;
}

U8* ramPageGet(RamPage page) {
    return ramPages[page];
}

void ramPageRelease(RamPage page) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    refCounts[page].refCount--;
    if (refCounts[page].refCount == 0) {
        allocatedRamPages--;
        freeIndex(page);
        if (refCounts[page].isNative) {
            ramPages[page] = nullptr;
            refCounts[page].isNative = 0;
        }
    }
}