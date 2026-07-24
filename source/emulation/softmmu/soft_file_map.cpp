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

#include "soft_file_map.h"
#include "kmemory_soft.h"
#include "soft_ram.h"
#include "soft_mmu.h"

U8 FilePage::readb(MMU* mmu, U32 address) {
    if (!loadPage(mmu, address >> K_PAGE_SHIFT, true, false)) {
        return 0;
    }
    return Page::getRWPage()->readb(mmu, address);
}

void FilePage::writeb(MMU* mmu, U32 address, U8 value) {
    if (!loadPage(mmu, address >> K_PAGE_SHIFT, false, true)) {
        return;
    }
    mmu->getPage()->writeb(mmu, address, value);
}

U16 FilePage::readw(MMU* mmu, U32 address) {
    if (!loadPage(mmu, address >> K_PAGE_SHIFT, true, false)) {
        return 0;
    }
    return Page::getRWPage()->readw(mmu, address);
}

void FilePage::writew(MMU* mmu, U32 address, U16 value) {
    if (!loadPage(mmu, address >> K_PAGE_SHIFT, false, true)) {
        return;
    }
    mmu->getPage()->writew(mmu, address, value);
}

U32 FilePage::readd(MMU* mmu, U32 address) {
    if (!loadPage(mmu, address >> K_PAGE_SHIFT, true, false)) {
        return 0;
    }
    return Page::getRWPage()->readd(mmu, address);
}

void FilePage::writed(MMU* mmu, U32 address, U32 value) {
    if (!loadPage(mmu, address >> K_PAGE_SHIFT, false, true)) {
        return;
    }
    mmu->getPage()->writed(mmu, address, value);
}

bool FilePage::canReadRam(MMU* mmu) {
    return false;
}

bool FilePage::canWriteRam(MMU* mmu) {
    return false;
}

U8* FilePage::getRamPtr(MMU* mmu, U32 page, bool write, bool force, U32 offset, U32 len) {
    if (!force) {
        return nullptr;
    }
    if (!loadPage(mmu, page, !write, write)) {
        return nullptr;
    }
    return mmu->getPage()->getRamPtr(mmu, page, write, force, offset, len);
}

void FilePage::onDemmand(MMU* mmu, U32 pageIndex) {
    loadPage(mmu, pageIndex, true, false);
}

bool FilePage::loadPage(MMU* mmu, U32 pageIndex, bool readFault, bool writeFault) {
    KThread* thread = KThread::currentThread();
    U32 address = pageIndex << K_PAGE_SHIFT;
    while (true) {
        MappedFilePtr mappedFile;
        std::shared_ptr<MappedFileCache> cache;
        std::shared_ptr<KFile> file;
        U64 fileOffset = 0;
        U32 mappingKey = 0;
        bool shared = false;
        bool invalidOffset = false;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->memory->mutex);
            if (mmu->getPageType() != PageType::File) {
                return true;
            }
            mappingKey = (U32)mmu->ramIndex;
            mappedFile = thread->process->getMappedFile(mappingKey);
            U64 mappedOffset = mappedFile && address >= mappedFile->address
                ? ((U64)address - mappedFile->address) & ~(U64)K_PAGE_MASK
                : std::numeric_limits<U64>::max();
            if (!mappedFile || !mappedFile->tryGetFileOffset(mappedOffset, fileOffset) ||
                (fileOffset >> K_PAGE_SHIFT) > K_MAX_MAPPED_FILE_CACHE_PAGE) {
                mmu->setFlags(0);
                mmu->setPage(thread->memory, pageIndex, PageType::None, (RamPage)0);
                getMemData(thread->memory)->onPageChanged(pageIndex);
                invalidOffset = true;
            } else {
                cache = mappedFile->systemCacheEntry;
                file = mappedFile->file;
                shared = thread->memory->mapShared(pageIndex);
            }
        }
        if (invalidOffset) {
            thread->seg_mapper(address, readFault, writeFault);
            return false;
        }

        // Backing reads and cache creation can acquire file/cache locks. Keep
        // them outside KMemory::mutex, then validate that the mapping snapshot
        // still describes this page before installing the loaded RamPage.
        RamPage ramPage;
        PageType pageType = PageType::Ram;
        if (cache) {
            ramPage = cache->getOrCreatePage((U32)(fileOffset >> K_PAGE_SHIFT), shared);
            pageType = PageType::CopyOnWrite;
        } else {
            ramPage = ramPageAlloc();
            file->preadNativeUncached(ramPageGet(ramPage), fileOffset, K_PAGE_SIZE);
        }

        bool retry = false;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->memory->mutex);
            if (mmu->getPageType() != PageType::File) {
                ramPageRelease(ramPage);
                return true;
            }
            MappedFilePtr currentMappedFile =
                thread->process->getMappedFile((U32)mmu->ramIndex);
            U64 currentMappedOffset =
                currentMappedFile && address >= currentMappedFile->address
                ? ((U64)address - currentMappedFile->address) & ~(U64)K_PAGE_MASK
                : std::numeric_limits<U64>::max();
            U64 currentFileOffset = 0;
            if ((U32)mmu->ramIndex != mappingKey ||
                currentMappedFile != mappedFile ||
                !currentMappedFile->tryGetFileOffset(currentMappedOffset, currentFileOffset) ||
                currentFileOffset != fileOffset ||
                thread->memory->mapShared(pageIndex) != shared) {
                retry = true;
            } else {
                mmu->setPage(thread->memory, pageIndex, pageType, ramPage);
                getMemData(thread->memory)->onPageChanged(pageIndex);
            }
        }
        ramPageRelease(ramPage); // setPage retained ramPage when installed
        if (!retry) {
            return true;
        }
    }
}
