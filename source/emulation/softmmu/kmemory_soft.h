#ifndef __KMEMORY_SOFT_H__
#define __KMEMORY_SOFT_H__

#include "pageType.h"
#include "meminfostruct.h"

class CodePage;

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
    
    void addCallback(OpCallback func);
    Page* getPage(U32 page) {return pageMap[memInfo[page].type];};
    void allocPages(KThread* thread, U32 page, U32 pageCount, U8 permissions, const MappedFilePtr& mappedFile, const RamPage* ramPages = nullptr);
    bool reserveAddress(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped, bool alignNative, U32 reservedFlag); // memory mutex should be held when calling this
    void protectPage(KThread* thread, U32 page, U32 permissions);
    void setPagesInvalid(U32 page, U32 pageCount);
    bool isPageAllocated(U32 page);
    void execvReset();    

    KMemory* memory;

    MemInfo memInfo[K_NUMBER_OF_PAGES];

    CodePage* getOrCreateCodePage(U32 address);
    bool isAddressDynamic(U32 address, U32 len);
    void markAddressDynamic(U32 address, U32 len);

private:
    void releaseRam(U32 page);
    bool hasRamPage(U32 page);
    void setPageType(U32 page, PageType pageType, RamPage ramIndex, bool releasePreviousRam);

    static Page* pageMap[8];
};

KMemoryData* getMemData(KMemory* memory);

#endif
