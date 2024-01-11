#ifndef __KMEMORY_SOFT_H__
#define __KMEMORY_SOFT_H__

#ifdef BOXEDWINE_DEFAULT_MMU

class KMemoryData {
public:
    KMemoryData(KMemory* memory);

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
};
#endif

#endif