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

static std::unordered_map<U64, RamInfo> refCounts;
static std::vector<RAM_TYPE> freeIndexes;

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
    refCounts[foundIndex].isNative = 1;
    RamPage result;
    result.value = foundIndex;
    return result;
}

#else

struct alignas(K_PAGE_SIZE) AlignedU8 {
    U8 data;
};

std::vector<AlignedU8*> allocatedPages;

RamPage ramPageAlloc() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    if (freeIndexes.size()) {
        RamPage found;
        found.value = freeIndexes.back();
        freeIndexes.pop_back();
        refCounts[found.value].refCount = 1;
        memset((U8*)(found.value << K_PAGE_SHIFT), 0, K_PAGE_SIZE);
        return found;
    }    

    AlignedU8* result = new AlignedU8[64]; // need to create a few since an aligned new will over allocate to make the alignment work.
    allocatedPages.push_back(result);
    RAM_TYPE index = ((RAM_TYPE)result) >> K_PAGE_SHIFT;
    for (int i = 1; i < 64; i++) {        
        freeIndexes.push_back(index + i);
    }
    memset(result, 0, K_PAGE_SIZE);
    RamPage found;
    found.value = index;

    refCounts[index].refCount = 1;
    allocatedRamPages++;
    return found;
}

RamPage ramPageAllocNative(U8* native) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    RAM_TYPE index = ((RAM_TYPE)native) >> K_PAGE_SHIFT;

    refCounts[index].refCount = 1;
    refCounts[index].isNative = 1;
    RamPage result;
    result.value = index;
    return result;
}

void shutdownRam() {
    refCounts.clear();
    for (AlignedU8* p : allocatedPages) {
        delete[] p;
    }
    allocatedPages.clear();
}

#endif

RamPage ramPageAllocNativeContinuous(U8* native, U32 pageCount) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    RAM_TYPE ramIndex = ((RAM_TYPE)native) >> K_PAGE_SHIFT;

    for (U32 i = 0; i < pageCount; i++) {
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

void ramPageRelease(RamPage page) {
    if (page.value == 0) {
        return;
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ramMutex);
    refCounts[page.value].refCount--;
    if (refCounts[page.value].refCount == 0) {
        allocatedRamPages--;        
        if (refCounts[page.value].isNative) {
            refCounts[page.value].isNative = 0;
            refCounts[page.value].isSystem = 0;
        } else {
            freeIndexes.push_back(page.value);
        }
    }    
}