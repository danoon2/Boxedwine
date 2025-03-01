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

#ifndef __SOFT_MMU_H__
#define __SOFT_MMU_H__

#include "soft_page.h"

class MMU {
public:
    U32 ramIndex : 20;
    U32 type : 3;
    U32 pad : 1;    
    U32 flags : 6;        
    U32 canReadRam : 1;    
    U32 canWriteRam : 1;

    RamPage getRamPageIndex() {
        if (getPageType() != PageType::File) {
            return (RamPage)ramIndex;
        }
        return (RamPage)0;
    }

    PageType getPageType() {
        return (PageType)type;
    }

    Page* getPage() {
        return Page::getPage(getPageType());
    }

    void setPageType(KMemoryData* mem, U32 page, PageType type);
    void setPage(KMemoryData* mem, U32 page, PageType type, RamPage ram);
    void setPermissions(U32 permissions);
    void setFlags(U32 flags);

private:
    void onPageChanged();
};

#endif