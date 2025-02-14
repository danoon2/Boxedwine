#ifndef __CODEPAGE_DATA_H__
#define __CODEPAGE_DATA_H__

#define CODE_ENTRIES 128
#define CODE_ENTRIES_SHIFT 5
#define CODE_ENTRIES_MASK 31

#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "../cpu/binaryTranslation/btCodeChunk.h"
#define InternalCodeBlock std::shared_ptr<BtCodeChunk>
#else
#define InternalCodeBlock std::shared_ptr<DecodedBlock>
#endif

class CodePageData {
public:
    CodePageData();
    ~CodePageData();

    void addCode(KMemory* memory, CodeBlockParam block);
    CodeBlock findCode(U32 eip, U32 len);
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    U32 getEipFromHost(U8* host);
#else
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

        CodePageData* page;

        // block is just a pointer that can be shared across more than one CodePageEntry, sharedBlock is used just to manage its life time and properly deallocate it
        InternalCodeBlock sharedBlock;
    };

private:    
    void addCode(KMemory* memory, U32 eip, CodeBlockParam block, const InternalCodeBlock& sharedBlock, U32 len, CodePageEntry* link);    
    void removeBlock(CodePageEntry* entry, U32 offset = 0);
    void removeEntry(CodePageEntry* entry, U32 offset);
    CodePageData::CodePageEntry* findEntry(U32 start, U32 stop);
    void addEntry(U32 start, U32 stop, CodePageEntry* entry);
    void nolock_removeBlockAt(U32 address, U32 len);

    CodePageEntry* entries[CODE_ENTRIES];
    BOXEDWINE_MUTEX mutex;

    int entryCount;
    U8 writeCount;
    U8* writeCountsPerByte;
};

class CodeCache {
public:
    CodeCache();
    ~CodeCache();

    void addCode(KMemory* memory, CodeBlockParam block);
    CodeBlock findCode(U32 eip, U32 len);
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    U32 getEipFromHost(U8* host);
#else
    CodeBlock getCode(U32 eip);
#endif
    void removeBlockAt(U32 address, U32 len);
    void markAddressDynamic(U32 address, U32 len);
    bool isAddressDynamic(U32 address, U32 len);

private:
    friend class CodePageData;

    CodePageData* getCodePageData(U32 address, bool create);
    CodePageData** pageData[0x400];
};

#endif