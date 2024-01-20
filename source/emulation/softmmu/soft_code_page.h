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

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_rw_page.h"

#define CODE_ENTRIES 128
#define CODE_ENTRIES_SHIFT 5

class CodePage : public RWPage {
protected:
    CodePage(U8* page, U32 address, U32 flags);
    ~CodePage();

public:
    static CodePage* alloc(U8* page, U32 address, U32 flags);

    virtual void writeb(U32 address, U8 value) override;
    virtual void writew(U32 address, U16 value) override;
    virtual void writed(U32 address, U32 value) override;
    virtual U8* getReadPtr(U32 address, bool makeReady = false) override;
    virtual U8* getWritePtr(U32 address, U32 len, bool makeReady = false) override;
    virtual Type getType() override { return Code_Page; }

    void addCode(U32 eip, CodeBlock block, U32 len);
    CodeBlock getCode(U32 eip);
    CodeBlock findCode(U32 eip, U32 len);

    void removeBlockAt(U32 address, U32 len);

    class CodePageEntry {
    public:
        CodeBlock block;
        U32 offset;
	    U32 len;
        CodePageEntry* next;
	    CodePageEntry* prev;
	    CodePageEntry* linkedPrev;
	    CodePageEntry* linkedNext;
        CodePage* page;

        void reset() {
            block = nullptr;
            offset = 0;
            len = 0;
            next = nullptr;
            prev = nullptr;
            linkedPrev = nullptr;
            linkedNext = nullptr;
            page = nullptr;
        }
    };    

private:
    CodePageEntry* findEntry(U32 address, U32 len);
    void addCode(U32 eip, CodeBlock block, U32 len, CodePageEntry* link);
    void copyOnWrite();

    CodePageEntry* entries[CODE_ENTRIES];
    int entryCount;

    CodePageEntry* getEntry(U32 eip);

    static CodePageEntry* allocCodePageEntry();
    void freeCodePageEntry(CodePageEntry* entry);
};

#endif

#endif