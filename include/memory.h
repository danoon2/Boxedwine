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
    BoxedPtr<KFile> file;
    U8** data;
};

#define K_PAGE_SIZE 4096
#define K_PAGE_MASK 0xFFF
#define K_PAGE_SHIFT 12
#define K_NUMBER_OF_PAGES 0x100000
#define K_ROUND_UP_TO_PAGE(x) ((x + 0xFFF) & 0xFFFFF000)

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

void memcopyFromNative(U32 address, const char* p, U32 len);
void memcopyToNative(U32 address, char* p, U32 len);

class KProcess;
class Page;

class CPU;
class DecodedOp;
class DecodedBlock;

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

    bool findFirstAvailablePage(U32 startingPage, U32 pageCount, U32* result, bool canBeMapped);
    void protectPage(U32 i, U32 permissions);
    void allocPages(U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const BoxedPtr<MappedFile>& mappedFile);
    bool isValidReadAddress(U32 address, U32 len);
    bool isValidWriteAddress(U32 address, U32 len);
    bool isPageAllocated(U32 page);
    DecodedBlock* getCodeBlock(U32 eip);
    void addCodeBlock(U32 startIp, DecodedBlock* block);

    U32 getPageFlags(U32 page);

    void onThreadChanged();

#ifdef BOXEDWINE_DEFAULT_MMU
private:
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
#endif

#ifdef BOXEDWINE_64BIT_MMU
    U8 flags[K_NUMBER_OF_PAGES];
    U8 nativeFlags[K_NUMBER_OF_PAGES];
    U32 allocated;
    U64 id;    
    void* codeCache[K_NUMBER_OF_PAGES]; // 4 MB 
    U64 ids[K_NUMBER_OF_PAGES];

    void removeBlock(DecodedBlock* block, U32 ip);
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
    void* internalAddCodeBlock(U32 startIp, DecodedBlock* block);
#endif
    void addCallback(OpCallback func);
};

#include "../source/emulation/softmmu/soft_page.h"
#include "../source/emulation/softmmu/soft_memory.h"
#endif
