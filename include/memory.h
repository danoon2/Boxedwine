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

#ifndef __MEMORY_H__
#define __MEMORY_H__

class KFile;

class MappedFileCache : public BoxedPtrBase {
public:
    MappedFileCache(const std::string& name) : name(name) {}
    virtual ~MappedFileCache();
    const std::string name;
    std::shared_ptr<KFile> file;
    U8** data;
};

#define K_PAGE_SIZE 4096
#define K_PAGE_MASK 0xFFF
#define K_PAGE_SHIFT 12
#define K_NUMBER_OF_PAGES 0x100000
#define K_ROUND_UP_TO_PAGE(x) ((x + 0xFFF) & 0xFFFFF000)
#define K_MAX_X86_OP_LEN 15

class Memory;
class KProcess;
class KThread;
class MappedFile;

U8 readb(U32 address);
void writeb(U32 address, U8 value);
U16 readw(U32 address);
void writew(U32 address, U16 value);
U32 readd(U32 address);
void writed(U32 address, U32 value);

U64 readq(U32 address);
void writeq(U32 address, U64 value);

void zeroMemory(U32 address, int len);
void readMemory(U8* data, U32 address, int len);
void writeMemory(U32 address, U8* data, int len);

U8* getPhysicalReadAddress(U32 address, U32 len);
U8* getPhysicalWriteAddress(U32 address, U32 len);
U8* getPhysicalAddress(U32 address, U32 len);

char* getNativeString(U32 address, char* buffer, U32 cbBuffer);
char* getNativeStringW(U32 address, char* buffer, U32 cbBuffer);
void writeNativeString(U32 address, const char* str);
U32 writeNativeString2(U32 address, const char* str, U32 len);
void writeNativeStringW(U32 address, const char* str);
U32 getNativeStringLen(U32 address);

void memcopyFromNative(U32 address, const void* p, U32 len);
void memcopyToNative(U32 address, void* p, U32 len);

class KProcess;
class Page;

class CPU;
class DecodedOp;
class DecodedBlock;
class BtCodeChunk;

typedef void (OPCALL *OpCallback)(CPU* cpu, DecodedOp* op);

class Memory {
public:   
    Memory();
    ~Memory();

    void log_pf(KThread* thread, U32 address);
    void clone(Memory* from);
    void reset();
    void reset(U32 page, U32 pageCount);

    void map(U32 startPage, const std::vector<U8*>& pages, U32 permissions);
    U32 mapNativeMemory(void* buf, U32 len);
    void unmapNativeMemory(U32 address, U32 len);

    bool findFirstAvailablePage(U32 startingPage, U32 pageCount, U32* result, bool canBeMapped, bool alignNative = false);
    bool isAlignedNativePage(U32 page) { return (page & ~(K_NATIVE_PAGES_PER_PAGE - 1)) == page;}
    U32 getNativePage(U32 page) { return (page << K_PAGE_SHIFT) >> K_NATIVE_PAGE_SHIFT;}
    U32 getEmulatedPage(U32 nativePage) {return (nativePage << K_NATIVE_PAGE_SHIFT) >> K_PAGE_SHIFT;}
    void protectPage(U32 i, U32 permissions);
    void allocPages(U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const BoxedPtr<MappedFile>& mappedFile);
    bool isValidReadAddress(U32 address, U32 len);
    bool isValidWriteAddress(U32 address, U32 len);
    bool isPageAllocated(U32 page);
    DecodedBlock* getCodeBlock(U32 eip);
    void addCodeBlock(U32 startIp, DecodedBlock* block);

    U32 getPageFlags(U32 page);

    void onThreadChanged();

    void incRefCount() { this->refCount++;}
	void decRefCount() { this->refCount--; if (this->refCount == 0) { delete this; } }
    U32 getRefCount() { return this->refCount;}

private:
    U32 refCount;
public: 

#ifdef BOXEDWINE_DEFAULT_MMU
    Page* mmu[K_NUMBER_OF_PAGES];

    U8* mmuReadPtr[K_NUMBER_OF_PAGES];
    U8* mmuWritePtr[K_NUMBER_OF_PAGES];

public:
    void setPage(U32 index, Page* page);
    inline Page* getPage(U32 index) {return this->mmu[index];}

    static Page** currentMMU;
    static U8** currentMMUReadPtr;
    static U8** currentMMUWritePtr;
#endif

#ifdef BOXEDWINE_DYNAMIC
    std::vector<void*> dynamicExecutableMemory;
    U32 dynamicExecutableMemoryPos;
    U32 dynamicExecutableMemoryLen;
#endif

#ifdef BOXEDWINE_64BIT_MMU
    U8 flags[K_NUMBER_OF_PAGES];
    U8 nativeFlags[K_NATIVE_NUMBER_OF_PAGES]; // this is based on the granularity for permissions, Platform::getPagePermissionGranularity.  It is 
    U32 allocated;
    U64 id; 

    // this will contain id in each page unless that page was mapped to native host memory
    U64 memOffsets[K_NUMBER_OF_PAGES];
#define MAX_DYNAMIC_CODE_PAGE_COUNT 0xFF
    U8 dynamicCodePageUpdateCount[K_NATIVE_NUMBER_OF_PAGES];

#ifdef BOXEDWINE_BINARY_TRANSLATOR
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
    void invalideHostCode(U32 eip, U32 len);
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
    void setEipPageCommitted(U32 page) {this->committedEipPages[page] = true;}
    void setEipForHostMapping(U32 eip, void* host);
#else
    void* codeCache[K_NUMBER_OF_PAGES]; // 4 MB 
    void removeBlock(DecodedBlock* block, U32 ip);    
#endif        
    void clearCodePageFromCache(U32 page);
#endif

#ifdef LOG_OPS
    U32 log;
#endif

private:
#ifdef BOXEDWINE_DEFAULT_MMU
    static U8* callbackRam;
    static U32 callbackRamPos;    
    U8* nativeAddressStart;
#endif
#ifdef BOXEDWINE_64BIT_MMU
    U32 callbackPos;
#ifndef BOXEDWINE_BINARY_TRANSLATOR
    void* internalAddCodeBlock(U32 startIp, DecodedBlock* block);
#endif
#endif
    void addCallback(OpCallback func);
};

#include "../source/emulation/softmmu/soft_page.h"
#include "../source/emulation/softmmu/soft_memory.h"
#endif
