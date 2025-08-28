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

#include "soft_invalid_page.h"
#include "kmemory_soft.h"
#include "soft_rw_page.h"
#include "soft_copy_on_write_page.h"
#include "soft_file_map.h"
#include "soft_code_page.h"
#include "devfb.h"
#ifdef BOXEDWINE_DYNAMIC
#include "../cpu/dynamic/dynamic_memory.h"
#endif
#include "soft_ram.h"

static InvalidPage _invalidPage;
static InvalidPage* invalidPage = &_invalidPage;
static RamPage callbackRam;
static U32 callbackRamPos;

void KMemoryData::shutdown() {
    callbackRam.value = 0;
    callbackRamPos = 0;
}

KMemoryData* getMemData(KMemory* memory) {
    return memory->data;
}

#ifdef BOXEDWINE_BINARY_TRANSLATOR
KMemoryData::KMemoryData(KMemory* memory) : memory(memory), mmuReadPtrAdjusted{ 0 }, mmuWritePtrAdjusted{ 0 }
#ifdef BOXEDWINE_4K_PAGE_SIZE
, mmuReadPtr{ 0 }
, mmuWritePtr{ 0 }
#endif
#else
KMemoryData::KMemoryData(KMemory* memory) : memory(memory)
#endif
{
    ::memset(mmu, 0, sizeof(mmu));
    if(!callbackRam.value) {
        callbackRam = ramPageAlloc();
        addCallback(onExitSignal);
    }
    this->allocPages(nullptr, CALL_BACK_ADDRESS >> K_PAGE_SHIFT, 1, K_PROT_READ | K_PROT_EXEC, -1, 0, nullptr, &callbackRam);
#ifdef BOXEDWINE_DYNAMIC
    dynamicMemory = nullptr;
#endif
}

KMemoryData::~KMemoryData() {
    codeMemory.freeAll();
    opCache.clear();
    setPagesInvalid(0, K_NUMBER_OF_PAGES);
#ifdef BOXEDWINE_DYNAMIC
    if (dynamicMemory) {
        delete dynamicMemory;
    }
#endif
}

bool KMemoryData::isPageValid(U32 page) {
    return getPage(page) == invalidPage;
}

void KMemoryData::onPageChanged(U32 index) {
    Page* page = this->mmu[index].getPage();
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    U8* readPtr = page->getRamPtr(&mmu[index], index, false);
    if (mmu[index].canReadRam) {
        this->mmuReadPtrAdjusted[index] = readPtr - (index << K_PAGE_SHIFT);
#ifdef BOXEDWINE_4K_PAGE_SIZE
        this->mmuReadPtr[index] = readPtr;
#endif
    } else {
        this->mmuReadPtrAdjusted[index] = nullptr;
#ifdef BOXEDWINE_4K_PAGE_SIZE
        this->mmuReadPtr[index] = nullptr;
#endif
    }

    U8* writePtr = page->getRamPtr(&mmu[index], index, true);
    if (mmu[index].canWriteRam) {
        this->mmuWritePtrAdjusted[index] = writePtr - (index << K_PAGE_SHIFT);
#ifdef BOXEDWINE_4K_PAGE_SIZE
        this->mmuWritePtr[index] = writePtr;
#endif
    } else {
        this->mmuWritePtrAdjusted[index] = nullptr;
#ifdef BOXEDWINE_4K_PAGE_SIZE
        this->mmuWritePtr[index] = nullptr;
#endif
    }
#endif
}

void KMemoryData::addCallback(OpCallback func) {
    U64 funcAddress = (U64)func;
    U8* address = ramPageGet(callbackRam) + callbackRamPos;

    *address = 0xFE;
    address++;
    *address = 0x38;
    address++;
    *address = (U8)funcAddress;
    address++;
    *address = (U8)(funcAddress >> 8);
    address++;
    *address = (U8)(funcAddress >> 16);
    address++;
    *address = (U8)(funcAddress >> 24);
    callbackRamPos += 6;
    if (sizeof(func) == 8) {
        address++;
        *address = (U8)(funcAddress >> 32);
        address++;
        *address = (U8)(funcAddress >> 40);
        address++;
        *address = (U8)(funcAddress >> 48);
        address++;
        *address = (U8)(funcAddress >> 56);
        callbackRamPos += 4;
    }
}

// don't need to add a mutex, memory->mutex should be locked when call except for construction and execv (which should only have 1 thread)
void KMemoryData::allocPages(KThread* thread, U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const std::shared_ptr<MappedFile>& mappedFile, const RamPage* ramPages) {
#ifdef _DEBUG
    if (page + pageCount >= K_NUMBER_OF_PAGES) {
        kpanic("KMemoryData::allocPages page out of bound");
    }
    if (mappedFile && ramPages) {
        kpanic("KMemoryData::allocPages mapped files should not contain ramPages");
    }
#endif
    if (ramPages) {
        for (U32 i = 0; i < pageCount; i++) {
            mmu[page + i].setFlags(permissions);
            this->mmu[page + i].setPage(memory, page + i, PageType::Ram, ramPages[i]);
            onPageChanged(page + i);
        }
    } else if (mappedFile) {
        if (offset & K_PAGE_MASK) {
            kpanic("mmap: wasn't expecting the offset to be in the middle of a page");
        }

        for (U32 i = 0; i < pageCount; i++) {
            mmu[page + i].setFlags(permissions);
            this->mmu[page + i].setPage(memory, page + i, PageType::File, (RamPage)mappedFile->key);
            onPageChanged(page + i);
        }
    } else {
        for (U32 i = 0; i < pageCount; i++) {
            mmu[page + i].setFlags(permissions);
            this->mmu[page + i].setPage(memory, page + i, PageType::Ram, (RamPage)0);
            onPageChanged(page + i);
        }
    }
}

bool isAlignedNativePage(U32 page) {
    U32 gran = Platform::getPageAllocationGranularity();
    return (page & ~(gran - 1)) == page;
}

bool KMemoryData::reserveAddress(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped, bool alignNative, U32 reservedFlag) {
    U32 i;

    for (i = startingPage; i < K_NUMBER_OF_PAGES; i++) {
        if (alignNative && !isAlignedNativePage(i)) {
            continue;
        }
        if (i + pageCount >= K_NUMBER_OF_PAGES) {
            return false;
        }
        U32 flags = mmu[i].flags;
        if (flags == 0 || (canBeReMapped && (flags & PAGE_MAPPED))) {
            U32 j = 1;
            bool success = true;

            for (; j < pageCount; j++) {
                U32 nextPage = i + j; // could be done a different way, but this helps the static analysis
                U32 nextPageFlags = mmu[nextPage].flags;
                if (nextPage < K_NUMBER_OF_PAGES && nextPageFlags != 0 && (!canBeReMapped || !(nextPageFlags & PAGE_MAPPED))) {
                    success = false;
                    break;
                }
            }
            if (success && startingPage < ADDRESS_PROCESS_MMAP_START && i >= ADDRESS_PROCESS_MMAP_START) {
                break; // don't allow user app to allocate in space we reserve for kernel space
            }
            if (success) {
                *result = i;
                U32 pageEndIndex = i + pageCount;
                for (U32 pageIndex = i; pageIndex < pageEndIndex && pageIndex < K_NUMBER_OF_PAGES; pageIndex++) {
                    mmu[pageIndex].flags = reservedFlag;
                }
                return true;
            }
            i += j; // no reason to check all the pages again
        }
    }
    return false;
}

void KMemoryData::protectPage(KThread* thread, U32 i, U32 permissions) {
    mmu[i].setPermissions(permissions);
    onPageChanged(i);
}

bool KMemoryData::isPageAllocated(U32 page) {
    return mmu[page].getPageType() != PageType::None;
}

bool KMemoryData::isPageNative(U32 page) {
    return mmu[page].getPageType() == PageType::Ram && ramPageIsNative(mmu[page].getRamPageIndex());
}

void KMemoryData::setPagesInvalid(U32 page, U32 pageCount) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
    for (U32 i = page; i < page + pageCount; i++) {
        mmu[i].flags = 0;
        mmu[i].setPage(memory, i, PageType::None, (RamPage)0);
        onPageChanged(i);
    }
}

void KMemoryData::execvReset() {
    setPagesInvalid(0, K_NUMBER_OF_PAGES);
    opCache.clear();
    this->allocPages(KThread::currentThread(), CALL_BACK_ADDRESS >> K_PAGE_SHIFT, 1, K_PROT_READ | K_PROT_EXEC, -1, 0, nullptr, &callbackRam);
}

CodePage* KMemoryData::getOrCreateCodePage(U32 address) {
    U32 pageIndex = address >> K_PAGE_SHIFT;
    PageType type = mmu[pageIndex].getPageType();

    CodePage* codePage = nullptr;
    if (type == PageType::Code) {
        codePage = (CodePage*)mmu[pageIndex].getPage();
    } else if (type == PageType::File) {
        mmu[pageIndex].getPage()->onDemmand(&mmu[pageIndex], pageIndex);
        return getOrCreateCodePage(address);
    } else if (type == PageType::Ram || type == PageType::CopyOnWrite) {
        mmu[pageIndex].setPageType(memory, pageIndex, PageType::Code);
        onPageChanged(pageIndex);
    } else {
        kpanic_fmt("Unhandled code caching page type: %d", static_cast<int>(mmu[pageIndex].getPageType()));
        codePage = nullptr;
    }
    return codePage;
}
