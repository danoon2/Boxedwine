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
    onDemmand(mmu, address >> K_PAGE_SHIFT);
    return Page::getRWPage()->readb(mmu, address);
}

void FilePage::writeb(MMU* mmu, U32 address, U8 value) {
    onDemmand(mmu, address >> K_PAGE_SHIFT);
    Page::getRWPage()->writeb(mmu, address, value);
}

U16 FilePage::readw(MMU* mmu, U32 address) {
    onDemmand(mmu, address >> K_PAGE_SHIFT);
    return Page::getRWPage()->readw(mmu, address);
}

void FilePage::writew(MMU* mmu, U32 address, U16 value) {
    onDemmand(mmu, address >> K_PAGE_SHIFT);
    Page::getRWPage()->writew(mmu, address, value);
}

U32 FilePage::readd(MMU* mmu, U32 address) {
    onDemmand(mmu, address >> K_PAGE_SHIFT);
    return Page::getRWPage()->readd(mmu, address);
}

void FilePage::writed(MMU* mmu, U32 address, U32 value) {
    onDemmand(mmu, address >> K_PAGE_SHIFT);
    Page::getRWPage()->writed(mmu, address, value);
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
    onDemmand(mmu, page);
    return Page::getRWPage()->getRamPtr(mmu, page, write, force, offset, len);
}

void FilePage::onDemmand(MMU* mmu, U32 pageIndex) {
    KThread* thread = KThread::currentThread();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->memory->mutex);
    if (mmu->getPageType() != PageType::File) {
        return;
    }
    MappedFilePtr mappedFile = thread->process->getMappedFile(mmu->ramIndex);
    U32 address = pageIndex << K_PAGE_SHIFT;
    U32 mappedOffset = (address - mappedFile->address) & 0xfffff000;
    U64 fileOffset = mappedOffset + mappedFile->offset;
    U32 fileOffsetPage = (U32)(fileOffset >> K_PAGE_SHIFT);
    RamPage ramPage;

    PageType pageType = PageType::Ram;

    if (mappedFile->systemCacheEntry && fileOffsetPage < (U32)mappedFile->systemCacheEntry->data.size()) {
        ramPage = mappedFile->systemCacheEntry->data[fileOffsetPage];
        if (ramPage.value) {
            ramPageRetain(ramPage);
            pageType = PageType::CopyOnWrite;
        }
    }

    if (!ramPage.value) {
        ramPage = ramPageAlloc();
        mappedFile->file->preadNative(ramPageGet(ramPage), fileOffset, K_PAGE_SIZE);
        if (mappedFile->systemCacheEntry && fileOffsetPage < (U32)mappedFile->systemCacheEntry->data.size()) {
            ramPageRetain(ramPage);
            ramPageMarkSystem(ramPage, true);
            mappedFile->systemCacheEntry->data[fileOffsetPage] = ramPage;
            pageType = PageType::CopyOnWrite;
        }
    }
    mmu->setPage(getMemData(thread->memory), pageIndex, pageType, ramPage);
    ramPageRelease(ramPage); // setPageType retained ramPage
    getMemData(thread->memory)->onPageChanged(pageIndex);
}