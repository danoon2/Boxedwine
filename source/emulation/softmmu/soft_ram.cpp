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

    U8* pages = (U8*)Platform::alloc64kBlock(1);
#ifdef _DEBUG
    if (Platform::getPagePermissionGranularity() == 1) {
        // the page before and after the allocated ram will be allocated too and set to no permission so that read/write will generate exception
        Platform::updateNativePermission((U64)(pages), 0, K_PAGE_SIZE);
        pages += K_PAGE_SIZE;
        for (int i = 0; i < 7; i++) {
            pages[0] = 0;
            freeRamPages.put(pages);
            Platform::updateNativePermission((U64)(pages + K_PAGE_SIZE), 0, K_PAGE_SIZE);
            pages += 2 * K_PAGE_SIZE;
        }
    } else {
        for (int i = 0; i < 16; i++) {
            freeRamPages.put(pages);
            pages += K_PAGE_SIZE;
        }
    }
#else
    for (int i = 0; i < 16; i++) {
        freeRamPages.put(pages);
        pages += K_PAGE_SIZE;
    }
#endif
    return ramPageAlloc();
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
