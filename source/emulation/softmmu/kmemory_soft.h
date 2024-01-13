#ifndef __KMEMORY_SOFT_H__
#define __KMEMORY_SOFT_H__

#ifdef BOXEDWINE_DEFAULT_MMU

#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "../cpu/binaryTranslation/btMemory.h"

class KMemoryData : public BtMemory {
#else
class KMemoryData {
#endif
public:
    KMemoryData(KMemory* memory);
    ~KMemoryData();

    void setPage(U32 index, Page* page);
    void addCallback(OpCallback func);
    void setPageRamWithFlags(U8* ram, U32 page, U8 flags, bool copyOnWrite = false);
    bool isPageMapped(U32 page);
    Page* getPage(U32 page) {return mmu[page];};
    void allocPages(KThread* thread, U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const BoxedPtr<MappedFile>& mappedFile, U8** ramPages = nullptr);
    bool reserveAddress(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped, bool alignNative, U32 reservedFlag);
    void protectPage(KThread* thread, U32 i, U32 permissions);
    U32 getPageFlags(U32 page);
    void setPagesInvalid(U32 page, U32 pageCount);
    bool isPageAllocated(U32 page);
    void execvReset();    

    KMemory* memory;

    Page* mmu[K_NUMBER_OF_PAGES];

    U8* mmuReadPtr[K_NUMBER_OF_PAGES];
    U8* mmuWritePtr[K_NUMBER_OF_PAGES];

    // you need to add the full emulated address to the page to get the host page instead of just an offset
    // this will speed things up in the binary translator
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    U8* mmuReadPtrAdjusted[K_NUMBER_OF_PAGES];
    U8* mmuWritePtrAdjusted[K_NUMBER_OF_PAGES];

    void clearDelayedReset();
private:
    KMemoryData* delayedReset;
#endif   
};

KMemoryData* getMemData(KMemory* memory);

#endif

#endif