#ifndef __KMEMORY_SOFT_H__
#define __KMEMORY_SOFT_H__

class CodePage;

#include "codePageData.h"

#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "../cpu/binaryTranslation/btMemory.h"

class KMemoryData : public BtMemory {
#else
class KMemoryData {
#endif
public:
    static void shutdown();
    
    KMemoryData(KMemory* memory);
    ~KMemoryData();

    void setPage(U32 index, Page* page);
    void addCallback(OpCallback func);
    void setPageRam(RamPage ram, U32 page, bool copyOnWrite = false);
    Page* getPage(U32 page) {return mmu[page];};
    void allocPages(KThread* thread, U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const std::shared_ptr<MappedFile>& mappedFile, const RamPage* ramPages = nullptr);
    bool reserveAddress(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped, bool alignNative, U32 reservedFlag);
    void protectPage(KThread* thread, U32 i, U32 permissions);
    void setPagesInvalid(U32 page, U32 pageCount);
    bool isPageAllocated(U32 page);
    void execvReset();    
    void onPageChanged(U32 page);

    KMemory* memory;

    Page* mmu[K_NUMBER_OF_PAGES];    
    U8 flags[K_NUMBER_OF_PAGES];

    CodePage* getOrCreateCodePage(U32 address);

    // you need to add the full emulated address to the page to get the host page instead of just an offset
    // this will speed things up in the binary translator
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    U8* mmuReadPtrAdjusted[K_NUMBER_OF_PAGES];
    U8* mmuWritePtrAdjusted[K_NUMBER_OF_PAGES];
#ifdef BOXEDWINE_4K_PAGE_SIZE
    U8* mmuReadPtr[K_NUMBER_OF_PAGES];
    U8* mmuWritePtr[K_NUMBER_OF_PAGES];
#endif
#else
    U8* mmuReadPtr[K_NUMBER_OF_PAGES];
    U8* mmuWritePtr[K_NUMBER_OF_PAGES];
#endif  

#ifdef BOXEDWINE_DYNAMIC
    DynamicMemory* dynamicMemory;
#endif

    CodeCache codeCache;
};

KMemoryData* getMemData(KMemory* memory);

#endif
