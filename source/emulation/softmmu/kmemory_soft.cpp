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

KMemoryData::KMemoryData(KMemory* memory) : memory(memory) {
    ::memset(mmu, 0, sizeof(mmu));
#ifdef BOXEDWINE_MEM_CACHE
    ::memset(readCache, 0, sizeof(readCache));
    ::memset(writeCache, 0, sizeof(writeCache));
#endif
#ifdef BOXEDWINE_WASM_JIT
    ::memset(wasmReadPageBase,  0, sizeof(wasmReadPageBase));
    ::memset(wasmWritePageBase, 0, sizeof(wasmWritePageBase));
#endif
    if(!callbackRam.value) {
        callbackRam = ramPageAlloc();
        addCallback(onExitSignal);
    }
    this->allocPages(nullptr, CALL_BACK_ADDRESS >> K_PAGE_SHIFT, 1, K_PROT_READ | K_PROT_EXEC, -1, 0, nullptr, &callbackRam);
    // in case another thread is using it right when we free it
    codeMemory.delayedFree = 1000; // 10s
#ifdef BOXEDWINE_HOST_EXCEPTIONS
    codeMemory.delayedFreeCallback = [this](void* p, U32 len) {
        auto startIt = this->jitAddressToEip.upper_bound((U8*)p);
        auto endIt = this->jitAddressToEip.lower_bound((U8*)p + len);
        auto it = startIt;

        while (it != endIt) {
            it = this->jitAddressToEip.erase(it);
        }
    };
#endif
    codeMemory.isCodeMemory = true;
}

KMemoryData::~KMemoryData() {
    codeMemory.freeAll();
    opCache.clear();
    setPagesInvalid(0, K_NUMBER_OF_PAGES);
}

bool KMemoryData::isPageValid(U32 page) {
    return getPage(page) != invalidPage;
}

void KMemoryData::onPageChanged(U32 index) {
#ifdef BOXEDWINE_MEM_CACHE
    if (mmu[index].canReadRam) {
        readCache[index] = (U8*)((mmu[index].ramIndex << K_PAGE_SHIFT) - (index << K_PAGE_SHIFT));
    } else {
        readCache[index] = (U8*)((U8*)0 - (index << K_PAGE_SHIFT));
    }
    if (mmu[index].canWriteRam) {
        writeCache[index] = (U8*)((mmu[index].ramIndex << K_PAGE_SHIFT) - (index << K_PAGE_SHIFT));
    } else {
        writeCache[index] = (U8*)((U8*)0 - (index << K_PAGE_SHIFT));
    }
#endif
#ifdef BOXEDWINE_WASM_JIT
    // Encode `0` for no-access; the JIT inline check tests for 0 and
    // falls to the existing helper. ramPageGet returns a U8* into the
    // Emscripten heap, which under wasm32 is just a 32-bit linear-mem
    // offset; the truncation is identity.
    wasmReadPageBase[index]  = mmu[index].canReadRam
        ? (U32)(uintptr_t)ramPageGet((RamPage)mmu[index].ramIndex) : 0;
    wasmWritePageBase[index] = mmu[index].canWriteRam
        ? (U32)(uintptr_t)ramPageGet((RamPage)mmu[index].ramIndex) : 0;
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
        if (i + pageCount > K_NUMBER_OF_PAGES) {
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
    if (mmu[i].getPageType() == PageType::Code && (mmu[i].flags & PAGE_EXEC) && !(permissions & PAGE_EXEC)) {
        // not really a write, but this more of a hint to the JIT to treat this as code that might change a lot
        memory->removeCode(KThread::currentThread(), i << K_PAGE_SHIFT, K_PAGE_SIZE, true);
    }
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
#ifdef BOXEDWINE_HOST_EXCEPTIONS
bool KMemoryData::findOpFromJitAddress(U8* jitAddress, U32& eipOfOp) {
    if (jitAddressToEip.empty()) {
        return false;
	}
    auto it = jitAddressToEip.lower_bound(jitAddress);
    if (it == jitAddressToEip.end()) {
        it = std::prev(it);
    } else if (it != jitAddressToEip.begin()) {
        if (it->first == jitAddress) {
            eipOfOp = it->second.eip;
            return true;
        }
        it = std::prev(it);
    }

    if (jitAddress >= it->first && jitAddress < it->first + it->second.len) {
        eipOfOp = it->second.eip;
        return true;
    }
    return false;
}
#endif