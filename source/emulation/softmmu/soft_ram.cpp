/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "soft_ram.h"

#define NATIVE_PAGE_REF_COUNT 0

static BOXEDWINE_MUTEX ramMutex;
int allocatedRamPages;

class RamInfo {
public:
    U16 refCount : 14;
    U16 isNative : 1;
    U16 isSystem : 1;
};

// not static so that the JIT and BT cores can access it directly
U8* ramPages[K_NUMBER_OF_PAGES];

static RamInfo refCounts[K_NUMBER_OF_PAGES];
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

void shutdownRam() {
    highWaterIndex = 1;
    freeIndexes.clear();
    for (U32 i = 0; i < K_NUMBER_OF_PAGES; i++) {
        if (refCounts[i].isNative) {
            ramPages[i] = nullptr;
        }
        refCounts[i].isNative = 0;
        refCounts[i].isSystem = 0;
        refCounts[i].refCount = 0;
    }
}

std::vector<U8*> pendingFreePages;

RamPage ramPageAllocNativeContinuous(U8* native, U32 pageCount) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U32 ramIndex  = highWaterIndex;

    highWaterIndex += pageCount;

    for (U32 i = 0; i < pageCount; i++) {
        ramPages[ramIndex + i] = native;
        refCounts[ramIndex + i].refCount = 1;
        refCounts[ramIndex + i].isNative = 1;
        native += K_PAGE_SIZE;
    }
    RamPage result;
    result.value = ramIndex;
    return result;
}

RamPage ramPageAlloc() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U32 index = allocIndex();

    if (!ramPages[index] && pendingFreePages.size()) {
        ramPages[index] = pendingFreePages.back();
        pendingFreePages.pop_back();
    }
#ifdef BOXEDWINE_4K_PAGE_SIZE
    if (!ramPages[index]) {
        U8* pages = (U8*)Platform::reserveNativeMemory64k(8);
        U32 count = (8 * 16 / 2) - 1; // -1 so that there is an uncommitted page at the end
        for (U32 i = 0; i < count; i++) {
            pages += K_PAGE_SIZE; // keep uncommitted page between each committed so that if a read/write crosses a page boundry it will generate an exception
            Platform::commitNativeMemoryPage(pages);
            if (i == 0) {
                ramPages[index] = pages;
            } else {
                pendingFreePages.push_back(pages);
            }
            pages += K_PAGE_SIZE;
        }
    }
#else
    if (!ramPages[index]) {
        U8* pages = (U8*)Platform::alloc64kBlock(1);
        for (int i = 0; i < 16; i++) {
            if (i == 0) {
                ramPages[index] = pages;
            } else {
                pendingFreePages.push_back(pages);
            }
            pages += K_PAGE_SIZE;
        }
    }
#endif
    memset(ramPages[index], 0, K_PAGE_SIZE);
    refCounts[index].refCount = 1;
    allocatedRamPages++;
    RamPage result;
    result.value = index;
    return result;
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
    RamPage result;
    result.value = index;
    return result;
}

RamPage ramPageAllocNative(U8* native) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U32 index = allocIndex();

    if (ramPages[index]) {
        delete[] ramPages[index];
    }
    ramPages[index] = native;
    refCounts[index].refCount = 1;
    refCounts[index].isNative = 1;
    RamPage result;
    result.value = index;
    return result;
}

void shutdownRam() {
    for (int i = 0; i < K_NUMBER_OF_PAGES; i++) {
        if (!refCounts[i].isNative && ramPages[i]) {
            delete[] ramPages[i];
        }
        refCounts[i].refCount = 0;
        refCounts[i].isNative = 0;
        refCounts[i].isSystem = 0;
        ramPages[i] = nullptr;
    }
}

#endif

RamPage ramPageAllocNativeContinuous(U8* native, U32 pageCount) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    U32 ramIndex = highWaterIndex;

    highWaterIndex += pageCount;

    for (U32 i = 0; i < pageCount; i++) {
        ramPages[ramIndex + i] = native;
        refCounts[ramIndex + i].refCount = 1;
        refCounts[ramIndex + i].isNative = 1;
        native += K_PAGE_SIZE;
    }
    RamPage result;
    result.value = ramIndex;
    return result;
}

void ramPageRetain(RamPage page) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    refCounts[page.value].refCount++;
}

U32 ramPageUseCount(RamPage page) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    return refCounts[page.value].refCount;
}

void ramPageMarkSystem(RamPage page, bool isSystem) {
    refCounts[page.value].isSystem = isSystem ? 1 : 0;
}

bool ramPageIsSystem(RamPage page) {
    return refCounts[page.value].isSystem != 0;
}

bool ramPageIsNative(RamPage page) {
    return refCounts[page.value].isNative != 0;
}

U8* ramPageGet(RamPage page) {
    return ramPages[page.value];
}

void ramPageRelease(RamPage page) {
    if (page.value == 0) {
        return;
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    refCounts[page.value].refCount--;
    if (refCounts[page.value].refCount == 0) {
        allocatedRamPages--;
        freeIndex(page.value);
        if (refCounts[page.value].isNative) {
            ramPages[page.value] = nullptr;
            refCounts[page.value].isNative = 0;
            refCounts[page.value].isSystem = 0;
        }
    }    
}