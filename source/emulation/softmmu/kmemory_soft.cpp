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

KMemoryData* KMemoryData::create(KMemory* memory) {
    if (ramPageUseLinearMemoryAdjacent()) {
        U8* linearMemoryBase = nullptr;
        U8* storage = ramPageReserveLinearMemoryData(sizeof(KMemoryData), &linearMemoryBase);
        if (storage) {
            std::shared_ptr<LinearMemoryBacking> backing = ramPageCreateLinearMemoryBacking();
            if (backing) {
                try {
                    return new (storage) KMemoryData(memory, true, linearMemoryBase, backing);
                } catch (...) {
                    ramPageReleaseLinearMemoryData(storage, sizeof(KMemoryData));
                    throw;
                }
            }
            ramPageReleaseLinearMemoryData(storage, sizeof(KMemoryData));
        }
        return new KMemoryData(memory, false);
    }
    return new KMemoryData(memory, ramPageUseLinearMemory());
}

void KMemoryData::destroy(KMemoryData* data) {
    if (!data) {
        return;
    }
    if (data->linearMemoryAdjacent) {
        U8* storage = (U8*)data;
        data->~KMemoryData();
        ramPageReleaseLinearMemoryData(storage, sizeof(KMemoryData));
    } else {
        delete data;
    }
}

KMemoryData::KMemoryData(KMemory* memory, bool enableLinearMemory, U8* adjacentLinearMemoryBase,
    const std::shared_ptr<LinearMemoryBacking>& backing) : memory(memory) {
    linearMemoryBase = enableLinearMemory ? (adjacentLinearMemoryBase ? adjacentLinearMemoryBase : ramPageReserveLinearMemory()) : nullptr;
    linearMemoryAdjacent = adjacentLinearMemoryBase != nullptr;
    if (linearMemoryBase) {
        linearMemoryMappings.resize(K_NUMBER_OF_PAGES);
        linearMemoryBacking = backing;
    }
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
    if (!linearMemoryAdjacent) {
        ramPageReleaseLinearMemory(linearMemoryBase);
    }
    linearMemoryBase = nullptr;
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
    if (linearMemoryBase && !resettingLinearMemory) {
        U32 pageCount = ramPageLinearMemoryPageCount();
        U32 firstPage = index & ~(pageCount - 1);
        auto updateGroup = [this, pageCount](U32 groupFirstPage) {
            RamPage pages[16];
            if (pageCount > 16) {
                kpanic("linear memory: unsupported native page size");
            }
            for (U32 i = 0; i < pageCount; i++) {
                pages[i] = mmu[groupFirstPage + i].getRamPageIndex();
            }
            bool canRead = true;
            bool canWrite = true;
            for (U32 i = 0; i < pageCount; i++) {
                canRead = canRead && mmu[groupFirstPage + i].canReadRam != 0;
                canWrite = canWrite && mmu[groupFirstPage + i].canWriteRam != 0;
            }
            canWrite = canRead && canWrite;
            U32 newMapping = ramPageUpdateLinearMemory(linearMemoryBase, groupFirstPage, pages, pageCount, canRead, canWrite, linearMemoryMappings[groupFirstPage]);
            std::fill(linearMemoryMappings.begin() + groupFirstPage, linearMemoryMappings.begin() + groupFirstPage + pageCount, newMapping);
        };
        updateGroup(firstPage);
    }
}

bool KMemoryData::allocLinearMemoryBlock(U32 page) {
    U32 pageCount = ramPageLinearMemoryPageCount();
    if (!useLinearMemoryJit() || pageCount <= 1) {
        return false;
    }
    U32 firstPage = page & ~(pageCount - 1);
    RamPage oldPages[16];
    RamPage newPages[16];
    if (pageCount > 16) {
        kpanic("linear memory: unsupported native page size");
    }
    for (U32 i = 0; i < pageCount; i++) {
        MMU& entry = mmu[firstPage + i];
        oldPages[i] = entry.getRamPageIndex();
        if (entry.getPageType() != PageType::Ram || memory->mapShared(firstPage + i)) {
            return false;
        }
        // Never move a live neighbor into a new backing block. Other guest
        // threads can access ordinary RAM without taking memory->mutex, so a
        // copy here could race a write and lose data. A completely untouched
        // anonymous group can be materialized atomically without copying.
        if (oldPages[i].value) {
            return false;
        }
    }
    bool allocated = linearMemoryAdjacent ? ramPageAllocLinearMemoryBackingBlock(linearMemoryBacking, firstPage, newPages, pageCount) : ramPageAllocLinearMemoryBlock(newPages, pageCount);
    if (!allocated) {
        return false;
    }
    for (U32 i = 0; i < pageCount; i++) {
        memset(ramPageGet(newPages[i]), 0, K_PAGE_SIZE);
    }
    for (U32 i = 0; i < pageCount; i++) {
        mmu[firstPage + i].setPage(memory, firstPage + i, PageType::Ram, newPages[i]);
        ramPageRelease(newPages[i]);
    }
    for (U32 i = 0; i < pageCount; i++) {
        onPageChanged(firstPage + i);
    }
    return true;
}

bool KMemoryData::materializeLinearMemoryCopyOnWriteBlock(U32 page) {
    U32 pageCount = ramPageLinearMemoryPageCount();
    if (!useLinearMemoryJit() || pageCount <= 1 || !(mmu[page].flags & PAGE_WRITE)) {
        return false;
    }
    U32 firstPage = page & ~(pageCount - 1);
    RamPage oldPages[16];
    RamPage newPages[16];
    if (pageCount > 16) {
        kpanic("linear memory: unsupported native page size");
    }
    bool allExclusive = true;
    bool allShared = true;
    for (U32 i = 0; i < pageCount; i++) {
        MMU& entry = mmu[firstPage + i];
        oldPages[i] = entry.getRamPageIndex();
        if (entry.getPageType() != PageType::CopyOnWrite || !oldPages[i].value || !(entry.flags & PAGE_READ) || memory->mapShared(firstPage + i)) {
            return false;
        }
        U32 useCount = ramPageUseCount(oldPages[i]);
        allExclusive = allExclusive && useCount == 1;
        allShared = allShared && useCount > 1;
    }

    if (allExclusive && linearMemoryMappings[firstPage]) {
        // A sibling already took its private copies, leaving this aligned block
        // exclusively owned. Merely make the entire host-page group writable.
        for (U32 i = 0; i < pageCount; i++) {
            mmu[firstPage + i].setPageType(memory, firstPage + i, PageType::Ram);
        }
    } else if (allShared) {
        // COW pages cannot be modified without memory->mutex, which the caller
        // holds. Their old backing is therefore stable while the whole group is
        // copied, and no live writable RAM page has to be migrated.
        bool allocated = linearMemoryAdjacent ? ramPageAllocLinearMemoryBackingBlock(linearMemoryBacking, firstPage, newPages, pageCount) : ramPageAllocLinearMemoryBlock(newPages, pageCount);
        if (!allocated) {
            return false;
        }
        for (U32 i = 0; i < pageCount; i++) {
            memcpy(ramPageGet(newPages[i]), ramPageGet(oldPages[i]), K_PAGE_SIZE);
        }
        for (U32 i = 0; i < pageCount; i++) {
            mmu[firstPage + i].setPage(memory, firstPage + i, PageType::Ram, newPages[i]);
            ramPageRelease(newPages[i]);
        }
    } else {
        // Do not move an exclusively owned neighbor: ordinary reads do not
        // take memory->mutex and could still hold its old backing pointer.
        return false;
    }
    for (U32 i = 0; i < pageCount; i++) {
        onPageChanged(firstPage + i);
    }
    return true;
}

bool KMemoryData::getLinearMemoryGuestAddress(U64 hostAddress, U32& guestAddress) {
    U64 base = (U64)linearMemoryBase;
    if (!base || hostAddress < base || hostAddress - base >= ramPageLinearMemoryApertureSize()) {
        return false;
    }
    guestAddress = (U32)(hostAddress - base);
    return true;
}

bool KMemoryData::useLinearMemoryJit() const {
    return linearMemoryBase != nullptr;
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

bool isAlignedNativePage(U32 page, U32 alignmentPhase, bool linearMemory) {
    U32 gran = Platform::getPageAllocationGranularity();
    if (linearMemory) {
        gran = std::max<U32>(gran, ramPageLinearMemoryPageCount());
    }
    return (page & (gran - 1)) == (alignmentPhase & (gran - 1));
}

bool KMemoryData::reserveAddress(U32 startingPage, U32 pageCount, U32* result,
    bool canBeReMapped, bool alignNative, U32 reservedFlag, U32 alignmentPhase) {
    U32 i;

    for (i = startingPage; i < K_NUMBER_OF_PAGES; i++) {
        if (alignNative && !isAlignedNativePage(i, alignmentPhase, useLinearMemoryJit())) {
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
    U32 oldFlags = mmu[i].flags;
    if (mmu[i].getPageType() == PageType::Code && (oldFlags & PAGE_EXEC) && !(permissions & PAGE_EXEC)) {
        // not really a write, but this more of a hint to the JIT to treat this as code that might change a lot
        memory->removeCode(KThread::currentThread(), i << K_PAGE_SHIFT, K_PAGE_SIZE, true);
    } else if ((oldFlags & PAGE_WRITE) && !(oldFlags & PAGE_EXEC) && (permissions & PAGE_EXEC)) {
#ifndef BOXEDWINE_DISABLE_WX_REMOVE_CODE
        // Writes to a plain writable RAM page bypass CodePage write hooks. If
        // the page later becomes executable, remember the bytes as dynamic code.
        memory->removeCode(thread, i << K_PAGE_SHIFT, K_PAGE_SIZE, true);
#endif
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

void KMemoryData::setPagesInvalid(U32 page, U32 pageCount, bool codeAlreadyPrepared) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
    if (codeAlreadyPrepared) {
        memory->commitPreparedCodeInvalidation();
    }
    bool resetLinearMemory = linearMemoryBase && page == 0 && pageCount == K_NUMBER_OF_PAGES;
    if (resetLinearMemory) {
        ramPageResetLinearMemory(linearMemoryBase);
        std::fill(linearMemoryMappings.begin(), linearMemoryMappings.end(), 0);
        resettingLinearMemory = true;
    } else if (linearMemoryBase) {
        U32 linearPageCount = ramPageLinearMemoryPageCount();
        U32 firstPage = page & ~(linearPageCount - 1);
        U32 lastPage = (page + pageCount + linearPageCount - 1) & ~(linearPageCount - 1);
        auto groupMapped = [this, linearPageCount](U32 groupFirstPage) {
            for (U32 i = 0; i < linearPageCount; i++) {
                if (linearMemoryMappings[groupFirstPage + i]) {
                    return true;
                }
            }
            return false;
        };
        // Collapse adjacent page removals without remapping the PROT_NONE gaps;
        // broad sparse remaps caused measurable TLB disruption in Cinebench.
        for (U32 i = firstPage; i < lastPage;) {
            if (!groupMapped(i)) {
                i += linearPageCount;
                continue;
            }
            U32 runStart = i;
            do {
                i += linearPageCount;
            } while (i < lastPage && groupMapped(i));
            ramPageRemoveLinearMemory(linearMemoryBase, runStart, i - runStart);
            std::fill(linearMemoryMappings.begin() + runStart, linearMemoryMappings.begin() + i, 0);
        }
        resettingLinearMemory = true;
    }
    for (U32 i = page; i < page + pageCount; i++) {
        if (codeAlreadyPrepared && mmu[i].getPageType() == PageType::Code) {
            opCache.clearPageWriteCounts(i);
        }
        mmu[i].flags = 0;
        mmu[i].setPage(memory, i, PageType::None, (RamPage)0, codeAlreadyPrepared);
        onPageChanged(i);
    }
    resettingLinearMemory = false;
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
