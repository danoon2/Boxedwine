#ifndef __KMEMORY_H__
#define __KMEMORY_H__

class KMemory;
#ifdef BOXEDWINE_DYNAMIC
class DynamicMemory;
#endif

#define K_PAGE_SIZE 4096
#define K_PAGE_MASK 0xFFF
#define K_PAGE_SHIFT 12
#define K_NUMBER_OF_PAGES 0x100000
#define K_ROUND_UP_TO_PAGE(x) ((x + 0xFFF) & 0xFFFFF000)

#define PAGE_READ 0x01
#define PAGE_WRITE 0x02
#define PAGE_EXEC 0x04
#define PAGE_SHARED 0x08
#define PAGE_FUTEX 0x10
#define PAGE_MAPPED 0x20
#define PAGE_ALLOCATED 0x40
#define PAGE_MAPPED_HOST 0x80
#define PAGE_PERMISSION_MASK 0x07

#define GET_PAGE_PERMISSIONS(flags) (flags & PAGE_PERMISSION_MASK)

#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "../source/emulation/cpu/binaryTranslation/btCodeChunk.h"
#define CodeBlock std::shared_ptr<BtCodeChunk>
#else
#define CodeBlock DecodedBlock*
#endif

class DecodedBlock;
class Page;
class KMemoryData;
class KProcess;
class BtMemory;

class KMemory {
private:
    KMemory(KProcess* process);
public:
    static KMemory* create(KProcess* process);
    static void shutdown();
    
    ~KMemory();
    void cleanup(); // called when the process is done but the last thread might still need to return

    U32 mlock(U32 addr, U32 len);
    U32 mmap(KThread* thread, U32 addr, U32 len, S32 prot, S32 flags, FD fildes, U64 off, bool remap = false);
    U32 mprotect(KThread* thread, U32 address, U32 len, U32 prot);
    U32 mremap(KThread* thread, U32 oldaddress, U32 oldsize, U32 newsize, U32 flags);
    U32 unmap(U32 address, U32 len);

    U32 mapPages(KThread* thread, U32 startPage, const std::vector<KRamPtr>& pages, U32 permissions);
    U32 mapNativeMemory(void* hostAddress, U32 size);
    void unmapNativeMemory(U32 address, U32 size);

    bool isPageAllocated(U32 page);
    bool canWrite(U32 address, U32 len);
    bool canRead(U32 address, U32 len);

    void clone(KMemory* from, bool cloneVM);
    void execvReset(bool cloneVM);

    void memcpy(U32 address, const void* p, U32 len);
    void memcpy(void* p, U32 address, U32 len);
    void strcpy(U32 address, const char* str);
    void memset(U32 address, char value, U32 len);
    int memcmp(U32 address, const void* p, U32 len);
    int strlen(U32 address);

    U64 readq(U32 address);
    U32 readd(U32 address);
    U16 readw(U32 address);
    U8  readb(U32 address);
    void writeq(U32 address, U64 value);
    void writed(U32 address, U32 value);
    void writew(U32 address, U16 value);
    void writeb(U32 address, U8 value);

    BString readString(U32 address);
    BString readStringW(U32 address);

    // This doesn't mean the data can't be changed by another thread, it just means the pointer will stay valid
    //
    // it is prefered to use performOnLocked Memory since that won't make any copies of data
    U8* lockReadOnlyMemory(U32 address, U32 len);
    void unlockMemory(U8* lockedPointer);

    U8* getIntPtr(U32 address, bool write = false);
    U8* getPtrForFutex(U32 address);

    // caller is responsible for making sure the address+len is valid
    void iteratePages(U32 address, U32 len, std::function<bool(U32 page)> callback);

    // caller is responsible for making sure the address+len is valid
    void performOnMemory(U32 address, U32 len, bool readOnly, std::function<bool(U8* ram, U32 len)> callback);
    
    void logPageFault(KThread* thread, U32 address);
    
    U32 getPageFlags(U32 page);

    bool canRead(U32 page) { return (getPageFlags(page) & PAGE_READ) != 0; }
    bool canWrite(U32 page) { return (getPageFlags(page) & PAGE_WRITE) != 0; }
    bool canExec(U32 page) { return (getPageFlags(page) & PAGE_EXEC) != 0; }
    bool mapShared(U32 page) { return (getPageFlags(page) & PAGE_SHARED) != 0; }
    bool isPageMapped(U32 page) { return (getPageFlags(page) & PAGE_MAPPED) != 0; }

#ifndef BOXEDWINE_BINARY_TRANSLATOR
    CodeBlock getCodeBlock(U32 address);
#endif
    CodeBlock findCodeBlockContaining(U32 address, U32 len);
    void addCodeBlock(U32 address, CodeBlock block);
    void removeCodeBlock(U32 address, U32 len);

#ifdef BOXEDWINE_DYNAMIC
    DynamicMemory* dynamicMemory;
#endif
    BOXEDWINE_MUTEX mutex;
private:
    friend KMemoryData* getMemData(KMemory* memory);
    friend KMemoryData;
    friend BtMemory;        

    KMemoryData* data;
    KProcess* process;
    BHashTable< U8*, std::shared_ptr<U8[]>> lockedMemory;
};

#endif
