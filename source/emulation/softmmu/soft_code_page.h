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

#ifndef __CODE_PAGE_H__
#define __CODE_PAGE_H__

#include "soft_rw_page.h"

class CodePage : public RWPage {
protected:
    CodePage(RamPage page, U32 address);
    ~CodePage();

public:
    static CodePage* alloc(RamPage page, U32 address);

    // from Page
    void writeb(U32 address, U8 value) override;
    void writew(U32 address, U16 value) override;
    void writed(U32 address, U32 value) override;
    U8* getReadPtr(KMemory* memory, U32 address, bool makeReady = false) override;
    U8* getWritePtr(KMemory* memory, U32 address, U32 len, bool makeReady = false) override;
    Type getType() override { return Type::Code_Page; }
    void close() override;

private:
    void copyOnWrite();
    BOXEDWINE_MUTEX mutex;
};

#endif
