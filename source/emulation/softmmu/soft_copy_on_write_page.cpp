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

#include "soft_copy_on_write_page.h"
#include "soft_ram.h"
#include "kmemory_soft.h"
#include "soft_mmu.h"

void CopyOnWritePage::writeb(MMU* mmu, U32 address, U8 value) {
    onDemmand(mmu, address >> K_PAGE_SHIFT);
    RWPage::writeb(mmu, address, value);
}

void CopyOnWritePage::writew(MMU* mmu, U32 address, U16 value) {
    onDemmand(mmu, address >> K_PAGE_SHIFT);
    RWPage::writew(mmu, address, value);
}

void CopyOnWritePage::writed(MMU* mmu, U32 address, U32 value) {
    onDemmand(mmu, address >> K_PAGE_SHIFT);
    RWPage::writed(mmu, address, value);
}

bool CopyOnWritePage::canWriteRam(MMU* mmu) {
    return false;
}

U8* CopyOnWritePage::getRamPtr(MMU* mmu, U32 page, bool write, bool force, U32 offset, U32 len) {
    if (!write) {
        return RWPage::getRamPtr(mmu, page, write, force, offset, len);
    }
    if (!force) {
        return nullptr;
    }
    onDemmand(mmu, page);
    return RWPage::getRamPtr(mmu, page, write, force, offset, len);
}

void CopyOnWritePage::onDemmand(MMU* mmu, U32 pageIndex) {
    KMemory* memory = KThread::currentThread()->memory;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);

    if (mmu->getPageType() != PageType::CopyOnWrite) {
        return;
    }

    if (!memory->mapShared(pageIndex) && ramPageUseCount((RamPage)mmu->ramIndex) > 1) {
        RamPage ramIndex = ramPageAlloc();
        RamPage currentRamPage = mmu->getRamPageIndex();

        memcpy(ramPageGet(ramIndex), ramPageGet(currentRamPage), K_PAGE_SIZE);
        mmu->setPage(getMemData(memory), pageIndex, PageType::Ram, ramIndex);
        ramPageRelease(ramIndex);
    } else {
        mmu->setPageType(getMemData(memory), pageIndex, PageType::Ram);
    }
    getMemData(memory)->onPageChanged(pageIndex);
}
