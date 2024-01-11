#ifndef __KMEMORY_HARD_H__
#define __KMEMORY_HARD_H__

#ifdef BOXEDWINE_64BIT_MMU

class BtCodeChunk;

#define NATIVE_FLAG_COMMITTED 0x08
#define NATIVE_FLAG_CODEPAGE_READONLY 0x10

#define K_MAX_X86_OP_LEN 15

class KMemoryData {
public:
    KMemoryData(KMemory* memory);
    ~KMemoryData();

    bool isAlignedNativePage(U32 page) { return (page & ~(K_NATIVE_PAGES_PER_PAGE - 1)) == page; }
    U32 getNativePage(U32 page) { return (page << K_PAGE_SHIFT) >> K_NATIVE_PAGE_SHIFT; }
    U32 getEmulatedPage(U32 nativePage) { return (nativePage << K_NATIVE_PAGE_SHIFT) >> K_PAGE_SHIFT; }

    // needed by kmemory
    bool isPageAllocated(U32 page);
    bool isPageMapped(U32 page);
    void allocPages(KThread* thread, U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const BoxedPtr<MappedFile>& mappedFile, U8** ramPages = nullptr);
    void protectPage(KThread* thread, U32 i, U32 permissions);
    void setPagesInvalid(U32 page, U32 pageCount);
    U32 getPageFlags(U32 page);
    void execvReset();
    void clearDelayedReset();
    bool reserveAddress(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped, bool alignNative, U32 reservedFlag);
    
    U8 flags[K_NUMBER_OF_PAGES];
    U8 nativeFlags[K_NATIVE_NUMBER_OF_PAGES]; // this is based on the granularity for permissions, Platform::getPagePermissionGranularity. 
    U32 allocated;
    U64 id;

    // this will contain id in each page unless that page was mapped to native host memory
    U64 memOffsets[K_NUMBER_OF_PAGES];
private:
    std::unordered_map<U32, std::unordered_map<U32, U32> > needsMemoryOffset; // first index is page, second index is offset
public:
    bool doesInstructionNeedMemoryOffset(U32 eip) {
        U32 page = eip >> K_PAGE_SHIFT;
        U32 offset = eip & K_PAGE_MASK;
        if (this->needsMemoryOffset.count(page) && this->needsMemoryOffset[page].count(offset)) {
            return this->needsMemoryOffset[page][offset] > 0;
        }
        return false;
    }
    void clearNeedsMemoryOffset(U32 page, U32 pageCount) {
        for (U32 i = 0; i < pageCount; i++) {
            if (this->needsMemoryOffset.count(page + i)) {
                this->needsMemoryOffset.erase(page + i);
            }
        }
    }
    void setNeedsMemoryOffset(U32 eip) {
        U32 page = eip >> K_PAGE_SHIFT;
        U32 offset = eip & K_PAGE_MASK;
        this->needsMemoryOffset[page][offset] = 1;
    }

    void clearAllNeedsMemoryOffset() {
        this->needsMemoryOffset.clear();
    }

#define MAX_DYNAMIC_CODE_PAGE_COUNT 0xFF
    U8 dynamicCodePageUpdateCount[K_NATIVE_NUMBER_OF_PAGES];

    BOXEDWINE_MUTEX executableMemoryMutex;
private:
#define EXECUTABLE_MIN_SIZE_POWER 7
#define EXECUTABLE_MAX_SIZE_POWER 22
#define EXECUTABLE_SIZES 16

    std::unordered_map<U32, std::shared_ptr< std::list< std::shared_ptr<BtCodeChunk> > >> codeChunksByHostPage;
    std::unordered_map<U32, std::shared_ptr< std::list< std::shared_ptr<BtCodeChunk> > >> codeChunksByEmulationPage;

    std::list<void*> freeExecutableMemory[EXECUTABLE_SIZES];
public:
    std::shared_ptr<BtCodeChunk> getCodeChunkContainingHostAddress(void* hostAddress);
    void clearHostCodeForWriting(U32 nativePage, U32 count);
    bool clearCodePageReadOnly(U32 nativePage);
    void makeCodePageReadOnly(U32 nativePage);
    void reserveNativeMemory();
    void releaseNativeMemory();
    void commitHostAddressSpaceMapping(U32 page, U32 pageCount, U64 defaultValue);
    void* getNativeAddress(U32 address);

    std::shared_ptr<BtCodeChunk> getCodeChunkContainingEip(U32 eip);
    void addCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk);
    void removeCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk);
    void makeNativePageDynamic(U32 nativePage);
    void* getExistingHostAddress(U32 eip);
    void* allocateExcutableMemory(U32 size, U32* allocatedSize);
    void freeExcutableMemory(void* hostMemory, U32 size);
    void executableMemoryReleased();
    bool isAddressExecutable(void* address);

    void allocNativeMemory(U32 page, U32 pageCount, U32 flags);
    void freeNativeMemory(U32 page, U32 pageCount);
    void updatePagePermission(U32 page, U32 pageCount); // called after page permission has changed, code will give the native page the highest permission possible
    void updateNativePermission(U32 page, U32 pageCount, U32 permission); // for a native page change so that it can be read or written too now, updatePagePermission should be called when done to restore correct permissions

    bool isValidReadAddress(U32 address, U32 len);
    bool isValidWriteAddress(U32 address, U32 len);

    void unmapNativeMemory(U32 address, U32 size);
    U32 mapNativeMemory(void* hostAddress, U32 size);

    class AllocatedMemory {
    public:
        AllocatedMemory(void* memory, U32 size) : memory(memory), size(size) {}
        void* memory;
        U32 size;
    };
    std::list<AllocatedMemory> allocatedExecutableMemory;
private:
    bool committedEipPages[K_NUMBER_OF_PAGES];

public:
    void*** eipToHostInstructionPages;
    void* eipToHostInstructionAddressSpaceMapping;
    bool isEipPageCommitted(U32 page);
    void setEipPageCommitted(U32 page) { this->committedEipPages[page] = true; }
    void setEipForHostMapping(U32 eip, void* host);
    void clearCodePageFromCache(U32 page);

    bool isShared(U32 page);

private:
    U32 callbackPos;
    KMemory* memory;
    KMemoryData* delayedReset;
    BOXEDWINE_MUTEX mutex;

    void addCallback(OpCallback func);
};

KMemoryData* getMemData(KMemory* memory);
#endif
#endif