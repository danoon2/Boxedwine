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

#include "soft_rw_page.h"

class CopyOnWritePage : public RWPage {
public:
    void writeb(MemInfo& info, U32 address, U8 value) override;
    void writew(MemInfo& info, U32 address, U16 value) override;
    void writed(MemInfo& info, U32 address, U32 value) override;

    U8* getWritePtr(KMemory* memory, MemInfo& info, U32 address, U32 len, bool makeReady = false) override;

private:
    void copyOnWrite(MemInfo& info, U32 address);
};

#endif