#include "boxedwine.h"
#include "soft_ram.h"

static BOXEDWINE_MUTEX ramMutex;
int allocatedRamPages;

// native x64 code instructions sometimes assume proper alignment, so make sure when they align an emulated address, the hardware address is also aligned the same 
// F-16 demo installer will crash if the page was allocated with just new on x64
#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "../../util/ptrpool.h"

static PtrPool<U8, false> freeRamPages(0);

void shutdownRam() {
    freeRamPages.deleteAll();
}

KRamPtr ramPageAlloc() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U8* result = freeRamPages.get();
    if (result) {
        KRamPtr ram(result, [](U8* p) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
            allocatedRamPages--;
            memset(p, 0, K_PAGE_SIZE);
            freeRamPages.put(p);
            });
        allocatedRamPages++;
        return ram;
    }
    
#ifdef BOXEDWINE_4K_PAGE_SIZE
    U8* pages = (U8*)Platform::reserveNativeMemory64k(8);

    U32 count = (8 * 16 / 2) - 1; // -1 so that there is an uncommitted page at the end
    for (U32 i = 0; i < count; i++) {
        pages += K_PAGE_SIZE; // keep uncommitted page between each committed so that if a read/write crosses a page boundry it will generate an exception
        Platform::commitNativeMemoryPage(pages);
        freeRamPages.put(pages);
        pages += K_PAGE_SIZE;
    }
#else
    U8* pages = (U8*)Platform::alloc64kBlock(1);
    for (int i = 0; i < 16; i++) {
        freeRamPages.put(pages);
        pages += K_PAGE_SIZE;
    }
#endif
    return ramPageAlloc();
}

#elif defined(BOXEDWINE_MULTI_THREADED)
KRamPtr ramPageAlloc() {
    std::align_val_t align = (std::align_val_t)16;
    U8* result = (U8*)operator new(K_PAGE_SIZE, align);
    memset(result, 0, K_PAGE_SIZE);
    KRamPtr ram(result, [align](U8* p) {
        allocatedRamPages--;
        operator delete(p, align);
        });
    allocatedRamPages++;
    return ram;
}
void shutdownRam() {
}
#else
KRamPtr ramPageAlloc() {
    U8* result = new U8[K_PAGE_SIZE];
    memset(result, 0, K_PAGE_SIZE);
    KRamPtr ram(result, [](U8* p) {
        allocatedRamPages--;
        delete[] p;
        });
    allocatedRamPages++;
    return ram;
}
void shutdownRam() {
}
#endif
