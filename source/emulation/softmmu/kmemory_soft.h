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

#ifndef __KMEMORY_SOFT_H__
#define __KMEMORY_SOFT_H__

class CodePage;

#include "codePageData.h"
#include "soft_mmu.h"
#include "../../source/util/bnativeheap.h"

class KMemoryData {
public:
    static void shutdown();
    
    KMemoryData(KMemory* memory);
    ~KMemoryData();

    void addCallback(OpCallback func);
    Page* getPage(U32 page) {return mmu[page].getPage();};
    bool isPageValid(U32 page);
    void allocPages(KThread* thread, U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const std::shared_ptr<MappedFile>& mappedFile, const RamPage* ramPages = nullptr);
    bool reserveAddress(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped, bool alignNative, U32 reservedFlag);
    void protectPage(KThread* thread, U32 i, U32 permissions);
    void setPagesInvalid(U32 page, U32 pageCount);
    bool isPageAllocated(U32 page);
    bool isPageNative(U32 page);
    void execvReset();    
    void onPageChanged(U32 page);

    KMemory* memory;

    MMU mmu[K_NUMBER_OF_PAGES];

    CodePage* getOrCreateCodePage(U32 address);

    // you need to add the full emulated address to the page to get the host page instead of just an offset
    // this will speed things up in the binary translator
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    U8* mmuReadPtrAdjusted[K_NUMBER_OF_PAGES];
    U8* mmuWritePtrAdjusted[K_NUMBER_OF_PAGES];
#ifdef BOXEDWINE_4K_PAGE_SIZE
    U8* mmuReadPtr[K_NUMBER_OF_PAGES];
    U8* mmuWritePtr[K_NUMBER_OF_PAGES];
#endif
#endif  

#ifdef BOXEDWINE_DYNAMIC
    DynamicMemory* dynamicMemory;
#endif

    DecodedOpCache opCache;
    BNativeHeap codeMemory;
};

KMemoryData* getMemData(KMemory* memory);

#endif
