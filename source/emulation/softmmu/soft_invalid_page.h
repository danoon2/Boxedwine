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

#ifndef __SOFT_INVALID_PAGE_H__
#define __SOFT_INVALID_PAGE_H__

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_page.h"

class InvalidPage : public Page {
public:
    InvalidPage() : Page(Invalid_Page, 0) {}

    U8 readb(U32 address);
    void writeb(U32 address, U8 value);
    U16 readw(U32 address);
    void writew(U32 address, U16 value);
    U32 readd(U32 address);
    void writed(U32 address, U32 value);
    U8* getCurrentReadPtr();
    U8* getCurrentWritePtr();
    U8* getReadAddress(U32 address, U32 len);
    U8* getWriteAddress(U32 address, U32 len);
    U8* getReadWriteAddress(U32 address, U32 len);
    bool inRam() {return false;}
    void close() {}
};

extern InvalidPage* invalidPage;

#endif

#endif