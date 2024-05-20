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

#define CODE_ENTRIES 128
#define CODE_ENTRIES_SHIFT 5
#define CODE_ENTRIES_MASK 31

#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "../cpu/binaryTranslation/btCodeChunk.h"
#define InternalCodeBlock std::shared_ptr<BtCodeChunk>
#else
#define InternalCodeBlock std::shared_ptr<DecodedBlock>
#endif

class CodePage : public RWPage {
protected:
    CodePage(const KRamPtr& page, U32 address);
    ~CodePage();

public:
    static CodePage* alloc(const KRamPtr& page, U32 address);

    // from Page
    void writeb(U32 address, U8 value) override;
    void writew(U32 address, U16 value) override;
    void writed(U32 address, U32 value) override;
    U8* getReadPtr(KMemory* memory, U32 address, bool makeReady = false) override;
    U8* getWritePtr(KMemory* memory, U32 address, U32 len, bool makeReady = false) override;
    Type getType() override { return Type::Code_Page; }

    void addCode(U32 eip, CodeBlock block, U32 len);
    CodeBlock findCode(U32 eip, U32 len);
#ifndef BOXEDWINE_BINARY_TRANSLATOR
    CodeBlock getCode(U32 eip);
#endif

    void removeBlockAt(U32 address, U32 len);
    bool isOffsetDynamic(U32 offset, U32 len);
    void markOffsetDynamic(U32 offset, U32 len);    

    class CodePageEntry {
    public:
        CodePageEntry() : block(nullptr), start(0), stop(0), nextEntry(nullptr), prevEntry(nullptr), next(nullptr), prev(nullptr), page(nullptr) {}

        void reset() {
            block = nullptr;
            start = 0;
            stop = 0;
            nextEntry = nullptr;
            prevEntry = nullptr;
            next = nullptr;
            prev = nullptr;
            page = nullptr;
            sharedBlock = nullptr;
        }

        CodeBlock block;
        U32 start;
        U32 stop;

        // within single bucket
        CodePageEntry* nextEntry;
        CodePageEntry* prevEntry;

        // between page links
        CodePageEntry* next;
        CodePageEntry* prev;

        CodePage* page;

        // block is just a pointer that can be shared across more than one CodePageEntry, sharedBlock is used just to manage its life time and properly deallocate it
        InternalCodeBlock sharedBlock;
    };  

private:
    void addCode(U32 eip, CodeBlock& block, const InternalCodeBlock& sharedBlock, U32 len, CodePageEntry* link);
    void copyOnWrite();
    void removeBlock(CodePageEntry* entry, U32 offset = 0);
    void removeEntry(CodePageEntry* entry, U32 offset);
    CodePage::CodePageEntry* findEntry(U32 start, U32 stop);
    void addEntry(U32 start, U32 stop, CodePageEntry* entry);

    CodePageEntry* entries[CODE_ENTRIES];
    BOXEDWINE_MUTEX mutex;

    int entryCount;
    U8 writeCount;
    U8* writeCountsPerByte;
};

#endif
