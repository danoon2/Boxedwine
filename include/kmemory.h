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

#ifndef __KMEMORY_H__
#define __KMEMORY_H__

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

class DecodedOp;
class Page;
class KMemoryData;
class KProcess;

#ifdef BOXEDWINE_MULTI_THREADED   
struct LockData8 {
    U8 data;
};
struct LockData16 {
    U16 data;
};
struct LockData32 {
    U32 data;
};
struct LockData64 {
    U64 data;
};
#endif

class NativeContinuousMemory {
public:
    NativeContinuousMemory(U8* p, U32 address, U32 len) : p(p), address(address), len(len) {}
    ~NativeContinuousMemory();
    U8* p;
    U32 address;
    U32 len;
};

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

    U32 mapPages(KThread* thread, U32 startPage, const std::vector<RamPage>& pages, U32 permissions);
    U32 mapNativeMemory(void* hostAddress, U32 size);
    void unmapNativeMemory(U32 address, U32 size);
    U32 ensureContinuousNative_unsafe(U32 page, U32 pageCount);

    bool isPageAllocated(U32 page);
    bool isPageNative(U32 page);
    bool canWrite(U32 address, U32 len);
    bool canRead(U32 address, U32 len);

    void clone(KMemory* from, bool cloneVM);
    void execvReset(bool cloneVM);

    void memcpy(U32 address, const void* p, U32 len);
    void memcpy(void* p, U32 address, U32 len);
    void memcpy(U32 dest, U32 src, U32 len);
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
    U8* lockReadWriteMemory(U32 address, U32 len);
    void unlockMemory(U8* lockedPointer);

    U8* getRamPtr(U32 address, U32 len, bool write, bool futex = false);

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

    DecodedOp* getDecodedOp(U32 address);
    DecodedOp** getDecodedOpLocation(U32 address);

    void addCode_nolock(U32 address, U32 len, DecodedOp* op, U32 opCount);
    bool removeCode(U32 address, U32 len, bool becauseOfWrite);
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    void removeCodeBlock(DecodedOp* op, bool clearOps = true);
#endif
    bool isAddressDynamic(U32 address, U32 len);
    void threadCleanup(U32 threadId);
    void clearOpCache();
    void clearPageWriteCounts(U32 pageIndex);

    void* allocCodeMemory(U32 len);
    bool isCode(void* p);

    BOXEDWINE_MUTEX mutex;
    KMemoryData* deleteOnNextLoop = nullptr;    
private:
    friend KMemoryData* getMemData(KMemory* memory);
    friend KMemoryData;
    friend class BtCPU;

    KMemoryData* data;    
    KProcess* process;

    class LockedMemory {
    public:
        ~LockedMemory() {
            if (p) {
                delete[] p;
            }
        }
        U8* p = nullptr;
        U32 len = 0;
        U32 address = 0;
        bool readOnly = true;
    };

    BHashTable<U8*, std::shared_ptr<LockedMemory>> lockedMemory;
    BOXEDWINE_MUTEX lockedMemoryMutex;   
    
    std::vector<std::shared_ptr<NativeContinuousMemory>> nativeContinuousMemory;
};

#endif
