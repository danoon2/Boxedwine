/*
 *  Copyright (C) 2016  The BoxedWine Team
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

#ifndef __SOFT_COPY_ON_WRITE_PAGE_H__
#define __SOFT_COPY_ON_WRITE_PAGE_H__

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_rw_page.h"

class CopyOnWritePage : public RWPage {
protected:
    CopyOnWritePage(KMemoryData* memory, U8* page, U32 address, U32 flags) : RWPage(memory, page, address, flags, Copy_On_Write_Page){}

public:
    static CopyOnWritePage* alloc(KMemoryData* memory, U8* page, U32 address, U32 flags);

    virtual U8 readb(U32 address) override;
    virtual void writeb(U32 address, U8 value) override;
    virtual U16 readw(U32 address) override;
    virtual void writew(U32 address, U16 value) override;
    virtual U32 readd(U32 address) override;
    virtual void writed(U32 address, U32 value) override;
    virtual U8* getReadPtr(U32 address, bool makeReady = false) override;
    virtual U8* getWritePtr(U32 address, U32 len, bool makeReady = false) override;

private:
    void copyOnWrite(U32 address);
};

#endif

#endif