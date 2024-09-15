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

#ifndef __SOFT_RW_PAGE_H__
#define __SOFT_RW_PAGE_H__

#include "soft_page.h"

class RWPage : public Page {
public:    
    U8 readb(MemInfo& info, U32 address) override;
    void writeb(MemInfo& info, U32 address, U8 value) override;
    U16 readw(MemInfo& info, U32 address) override;
    void writew(MemInfo& info, U32 address, U16 value) override;
    U32 readd(MemInfo& info, U32 address) override;
    void writed(MemInfo& info, U32 address, U32 value) override;

    void onDemand(KMemory* memory, MemInfo& info, U32 address) override;
    U8* getReadPtr(KMemory* memory, MemInfo& info, U32 address, bool makeReady = false) override;
    U8* getWritePtr(KMemory* memory, MemInfo& info, U32 address, U32 len, bool makeReady = false) override;
};

#endif