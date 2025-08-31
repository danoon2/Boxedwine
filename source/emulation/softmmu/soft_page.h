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

#ifndef __SOFT_PAGE_H__
#define __SOFT_PAGE_H__

#include "platformBoxedwine.h"
#include "soft_ram.h"

class KThread;
class KMemory;
class MMU;

enum class PageType {
    None = 0,
    Ram = 1,
    Code = 2,
    File = 3,
    CopyOnWrite = 4
};

class Page {
public:
    virtual U8 readb(MMU* mmu, U32 address) = 0;
    virtual void writeb(MMU* mmu, U32 address, U8 value) = 0;
    virtual U16 readw(MMU* mmu, U32 address) = 0;
    virtual void writew(MMU* mmu, U32 address, U16 value) = 0;
    virtual U32 readd(MMU* mmu, U32 address) = 0;
    virtual void writed(MMU* mmu, U32 address, U32 value) = 0;

    virtual bool canReadRam(MMU* mmu) = 0;
    virtual bool canWriteRam(MMU* mmu) = 0;

    // if address == 0 and len == 0, then its assumed to be the entire page
    virtual U8* getRamPtr(MMU* mmu, U32 page, bool write = false, bool force = false, U32 offset = 0, U32 len = 0) = 0;

    // file pages will read the file and convert to ram pages
    // on demand ram pages will allocate the ram
    virtual void onDemmand(MMU* mmu, U32 pageIndex) = 0;

    static Page* getPage(PageType type);
    static Page* getRWPage();
};

#endif
