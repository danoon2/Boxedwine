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
#include "soft_mmu.h"
#include "soft_invalid_page.h"
#include "soft_code_page.h"
#include "soft_file_map.h"
#include "soft_copy_on_write_page.h"
#include "kmemory_soft.h"

void MMU::setPageType(KMemory* memory, U32 page, PageType type) {
    setPage(memory, page, type, (RamPage)ramIndex);
}

void MMU::setPage(KMemory* memory, U32 page, PageType type, RamPage ram) {
    if (getPageType() == PageType::Code && type != PageType::Code) {
        memory->removeCode(page << K_PAGE_SHIFT, K_PAGE_SIZE, false);
    }
    if (type == PageType::None) {
        if (ramIndex && getPageType() != PageType::File) {
            ramPageRelease((RamPage)ramIndex);
        }
        ramIndex = 0;
        flags = 0;
        this->type = 0;
        canReadRam = 0;
        canWriteRam = 0;
        return;
    }
    // I have seen ramIndex == ram.value but it was moving from PageType::File to PageType::CopyOnWrite
    // so the ram index happen to equal the file key
    if (ramIndex != ram.value || type == PageType::File || getPageType() == PageType::File) {
        if (ramIndex && getPageType() != PageType::File) {
            ramPageRelease((RamPage)ramIndex);
        }
        if (ram.value && type != PageType::File) {
            ramPageRetain(ram);
        }
    }
    ramIndex = ram.value;
    this->type = (U8)type;
    onPageChanged();
}

void MMU::setPermissions(U32 permissions) {
#ifdef _DEBUG
    if (permissions & ~PAGE_PERMISSION_MASK) {
        kpanic("MMU::setPermissions oops");
    }
#endif
    this->flags &= ~PAGE_PERMISSION_MASK;
    this->flags |= (permissions & PAGE_PERMISSION_MASK);    
    onPageChanged();
}

void MMU::setFlags(U32 flags) {
    this->flags = flags;
}

void MMU::onPageChanged() {
    Page* page = getPage();
    canReadRam = page->canReadRam(this) && (flags & PAGE_READ) ? 1 : 0;
    canWriteRam = page->canWriteRam(this) && (flags & PAGE_WRITE) ? 1 : 0;
}

static InvalidPage pageInvalid;
static RWPage rwPage;
static FilePage filePage;
static CopyOnWritePage copyOnWritePage;
static CodePage codePage;

Page* Page::getRWPage() {
    return &rwPage;
}

Page* Page::getPage(PageType type) {
    switch (type) {
    case PageType::None:
        return &pageInvalid;
    case PageType::Ram:
        return &rwPage;
    case PageType::Code:
        return &codePage;
    case PageType::File:
        return &filePage;
    case PageType::CopyOnWrite:
        return &copyOnWritePage;
    default:
        kpanic_fmt("age::getPage unknown type: %d", (U32)type);
        return nullptr;
    }
}