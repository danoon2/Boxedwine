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

#ifndef __CODE_PAGE_H__
#define __CODE_PAGE_H__

#include "soft_rw_page.h"

class CodePage : public RWPage {
public:
    void writeb(MMU* mmu, U32 address, U8 value) override;
    void writew(MMU* mmu, U32 address, U16 value) override;
    void writed(MMU* mmu, U32 address, U32 value) override;
    bool canWriteRam(MMU* mmu) override;
    U8* getRamPtr(MMU* mmu, U32 page, bool write = false, bool force = false, U32 offset = 0, U32 len = 0) override;
    void onDemmand(MMU* mmu, U32 pageIndex) override;
};

#endif
