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
#include "boxedwine.h"

#ifdef BOXEDWINE_64BIT_MMU

#include <string.h>
#include <setjmp.h>
#include "hard_memory.h"
#include "../cpu/binaryTranslation/btCodeMemoryWrite.h"
#include "../cpu/binaryTranslation/btCodeChunk.h"

Memory::Memory() : allocated(0), callbackPos(0) {
    memset(flags, 0, sizeof(flags));
    memset(nativeFlags, 0, sizeof(nativeFlags));
    memset(memOffsets, 0, sizeof(memOffsets));
#ifndef BOXEDWINE_BINARY_TRANSLATOR
    memset(codeCache, 0, sizeof(codeCache));    
#else
    if (!KSystem::useLargeAddressSpace) {
        this->eipToHostInstructionPages = new void** [K_NUMBER_OF_PAGES];
        memset(this->eipToHostInstructionPages, 0, K_NUMBER_OF_PAGES*sizeof(void**));
    } else {
        this->eipToHostInstructionPages = NULL;
    }
    this->eipToHostInstructionAddressSpaceMapping = NULL;
    memset(this->dynamicCodePageUpdateCount, 0, sizeof(this->dynamicCodePageUpdateCount));
    memset(this->committedEipPages, 0, sizeof(this->committedEipPages));
    this->executableMemoryId = 0;
#endif    
    reserveNativeMemory(this);

    allocNativeMemory(this, CALL_BACK_ADDRESS >> K_PAGE_SHIFT, K_NATIVE_PAGES_PER_PAGE, PAGE_READ | PAGE_EXEC | PAGE_WRITE);
    this->addCallback(onExitSignal);
#ifdef BOXEDWINE_DYNAMIC
    this->dynamicExecutableMemoryPos = 0;
    this->dynamicExecutableMemoryLen = 0;
#endif

    this->refCount = 1;
}

Memory::~Memory() {    
    releaseNativeMemory(this);
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    if (this->eipToHostInstructionPages) {
        delete[] this->eipToHostInstructionPages;
    }
#endif
}

void Memory::log_pf(KThread* thread, U32 address) {
    U32 start = 0;
    U32 i;
    CPU* cpu = thread->cpu;

    std::string name = thread->process->getModuleName(cpu->seg[CS].address+cpu->eip.u32);
    klog("%.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X %s at %.8X\n", cpu->seg[CS].address + cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, name.c_str(), thread->process->getModuleEip(cpu->seg[CS].address+cpu->eip.u32));

    klog("Page Fault at %.8X\n", address);
    klog("Valid address ranges:\n");
    for (i=0;i<K_NUMBER_OF_PAGES;i++) {
        if (!start) {
            if (thread->process->memory->isPageAllocated(i)) {
                start = i;
            }
        } else {
            if (!thread->process->memory->isPageAllocated(i)) {
                klog("    %.8X - %.8X\n", start*K_PAGE_SIZE, i*K_PAGE_SIZE);
                start = 0;
            }
        }
    }
    klog("Mapped Files:\n");
    thread->process->printMappedFiles();
    cpu->walkStack(cpu->eip.u32, EBP, 2);
    kpanic("pf");
}

void Memory::reset() {
    releaseNativeMemory(this);
    reserveNativeMemory(this);

    this->callbackPos = 0;
    allocNativeMemory(this, CALL_BACK_ADDRESS >> K_PAGE_SHIFT, K_NATIVE_PAGES_PER_PAGE, PAGE_READ | PAGE_EXEC | PAGE_WRITE);
    this->addCallback(onExitSignal);
}

void Memory::reset(U32 page, U32 pageCount) {
    freeNativeMemory(this, page, pageCount);        
}

void Memory::clone(Memory* from) {
    int i=0;    

    for (i=0;i<0x100000;i++) {
        if (from->isPageAllocated(i)) {
            if ((from->flags[i] & PAGE_SHARED) && (from->flags[i] & PAGE_WRITE)) {
                static U32 shown = 0;
                if (!shown) {
                    kdebug("forking a process with shared memory is not fully supported with BOXEDWINE_64BIT_MMU");
                    shown=1;
                }
            }
            allocNativeMemory(this, i, 1, from->flags[i]);
            memcpy(getNativeAddress(this, i << K_PAGE_SHIFT), getNativeAddress(from, i << K_PAGE_SHIFT), K_PAGE_SIZE);
        } else {
            this->flags[i] = from->flags[i];
        }     
    }
}

void zeroMemory(U32 address, int len) {
    memset(getNativeAddress(KThread::currentThread()->process->memory, address), 0, len);
}

void readMemory(U8* data, U32 address, int len) {
    memcpy(data, getNativeAddress(KThread::currentThread()->process->memory, address), len);
}

void writeMemory(U32 address, U8* data, int len) {
    memcpy(getNativeAddress(KThread::currentThread()->process->memory, address), data, len);
}

U32 Memory::mapNativeMemory(void* hostAddress, U32 size) {
    U32 i;
    U32 result = 0;
    U32 pageCount = (size >> K_PAGE_SHIFT) + 2; // 1 for size alignment, 1 for hostAddress alignment
    U64 hostStart = (U64)hostAddress & 0xFFFFFFFFFFFFF000l;
    U64 offset;
    
    for (int i = 0; i < K_NUMBER_OF_PAGES; i++) {
        if ((i << K_PAGE_SHIFT) + this->memOffsets[i] == hostStart) {
            return (i << (K_PAGE_SHIFT)) + ((U32)((U64)hostAddress) & K_PAGE_MASK);
        }
    }
    findFirstAvailablePage(0x10000, pageCount, &result, false);
    offset = hostStart - (result << K_PAGE_SHIFT);
    for (i = 0; i < pageCount; i++) {
        this->memOffsets[result + i] = offset;
        this->flags[result + i] = PAGE_MAPPED_HOST;
    }
    return (result << K_PAGE_SHIFT) + ((U32)((U64)hostAddress) & K_PAGE_MASK);
}

void Memory::allocPages(U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const BoxedPtr<MappedFile>& mappedFile) {
    for (int i = 0; i < pageCount; i++) {
        this->clearCodePageFromCache(page + i);
    }
    if ((permissions & PAGE_PERMISSION_MASK) || mappedFile) {
        allocNativeMemory(this, page, pageCount, permissions);
    } else {
        U32 i;
        freeNativeMemory(this, page, pageCount);
        for (i=0;i<pageCount;i++) {
            this->flags[i+page]=permissions;
        }
    }
    if (mappedFile) {
        bool addedWritePermission = false;
        if (!(permissions & PAGE_WRITE)) {
            for (U32 i=0;i<pageCount;i++) {
                this->flags[i+page]|=PAGE_WRITE;
            }
            addedWritePermission = true;
        }
        // :TODO: need to implement writing back to the file
        // :TODO: need to sync shared pages acrosss processes for hard_memory.cpp
        KThread::currentThread()->process->pread64(fd, page<<K_PAGE_SHIFT, pageCount << K_PAGE_SHIFT, offset);
        if (addedWritePermission) {
            for (U32 i=0;i<pageCount;i++) {
                this->flags[i+page]&=~PAGE_WRITE;
            }
        }
    }    
}

void Memory::protectPage(U32 i, U32 permissions) {
    if (!this->isPageAllocated(i) && (permissions & PAGE_PERMISSION_MASK)) {
        this->allocPages(i, 1, permissions, 0, 0, 0);
    } else {
        this->flags[i] &=~ PAGE_PERMISSION_MASK;
        this->flags[i] |= permissions;
    } 
}

bool Memory::findFirstAvailablePage(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped, bool alignNative) {
    U32 i;
    
    for (i=startingPage;i<K_NUMBER_OF_PAGES;i++) {
        if (alignNative && !isAlignedNativePage(i)) {
            continue;
        }
        if (i + pageCount >= K_NUMBER_OF_PAGES) {
            return false;
        }
        if (((this->flags[i] & (PAGE_MAPPED | PAGE_MAPPED_HOST)) == 0 && !this->isPageAllocated(i)) || (canBeReMapped && (this->flags[i] & PAGE_MAPPED))) {
            U32 j;
            bool success = true;

            for (j=1;j<pageCount;j++) {
                if (((this->flags[i+j] & (PAGE_MAPPED | PAGE_MAPPED_HOST)) || this->isPageAllocated(i+j)) && (!canBeReMapped || !(this->flags[i+j] & PAGE_MAPPED))) {
                    success = false;
                    break;
                }
            }
            if (success) {
                *result = i;
                return true;
            }
            i+=j; // no reason to check all the pages again
        }
    }
    return false;
}

bool Memory::isValidReadAddress(U32 address, U32 len) {
    U32 startPage = address>>K_PAGE_SHIFT;
    U32 endPage = (address+len-1)>>K_PAGE_SHIFT;
    for (U32 i=startPage;i<=endPage;i++) {
        if (!this->isPageAllocated(i)) {
            return false;
        }
        if (!(this->flags[i] & PAGE_READ)) {
            return false;
        }
    }
    return true;
}

bool Memory::isValidWriteAddress(U32 address, U32 len) {
    U32 startPage = address>>K_PAGE_SHIFT;
    U32 endPage = (address+len-1)>>K_PAGE_SHIFT;
    for (U32 i=startPage;i<=endPage;i++) {
        if (!this->isPageAllocated(i)) {
            return false;
        }
        if (!(this->flags[i] & PAGE_WRITE)) {
            return false;
        }
    }
    return true;
}

bool Memory::isPageAllocated(U32 page) {
    return (this->flags[page] & PAGE_ALLOCATED) != 0;
}

void memcopyFromNative(U32 address, const void* p, U32 len) {
    memcpy(getNativeAddress(KThread::currentThread()->process->memory, address), p, len);
}

void memcopyToNative(U32 address, void* p, U32 len) {
    memcpy(p, getNativeAddress(KThread::currentThread()->process->memory, address), len);
}

void writeNativeString(U32 address, const char* str) {	
    strcpy((char*)getNativeAddress(KThread::currentThread()->process->memory, address), str);
}

U32 writeNativeString2(U32 address, const char* str, U32 len) {	
    U32 count=0;

    while (*str && count<len-1) {
        writeb(address, *str);
        str++;
        address++;
        count++;
    }
    writeb(address, 0);
    return count;
}

void writeNativeStringW(U32 address, const char* str) {	
    while (*str) {
        writew(address, *str);
        str++;
        address+=2;
    }
    writew(address, 0);
}

char* getNativeString(U32 address, char* buffer, U32 cbResult) {
    if (!address) {
        buffer[0]=0;
        return buffer;
    }
    return (char*)getNativeAddress(KThread::currentThread()->memory, address);
}

char* getNativeStringW(U32 address, char* buffer, U32 cbResult) {
    char c;
    U32 i=0;

    if (!address) {
        buffer[0]=0;
        return buffer;
    }
    do {
        c = (char)readw(address);
        address+=2;
        buffer[i++] = c;
    } while(c && i<cbResult);
    buffer[cbResult-1]=0;
    return buffer;
}

U32 getNativeStringLen(U32 address) {
    return (U32)strlen((char*)getNativeAddress(KThread::currentThread()->memory, address));
}

U8 readb(U32 address) {
#ifdef LOG_OPS
    U8 result = *(U8*)getNativeAddress(thread->process->memory, address);
    if (thread->process->memory->log)
        fprintf(logFile, "readb %X @%X\n", result, address);
    return result;
#else
    return *(U8*)getNativeAddress(KThread::currentThread()->memory, address);
#endif
}

void writeb(U32 address, U8 value) {
#ifdef LOG_OPS
    if (thread->process->memory->log)
        fprintf(logFile, "writeb %X @%X\n", value, address);
#endif
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    Memory* m = KThread::currentThread()->memory;
    U32 page = address >> K_PAGE_SHIFT;
    U8 flags = m->nativeFlags[page];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        BtCodeMemoryWrite w((BtCPU*)KThread::currentThread()->cpu, address, 1);
        *(U8*)getNativeAddress(m, address) = value;
    } else if ((flags & NATIVE_FLAG_COMMITTED) || (m->flags[page] & PAGE_MAPPED_HOST)) {
        *(U8*)getNativeAddress(KThread::currentThread()->memory, address) = value;
    } else {
        kpanic("writeb about to crash");
    }
#else
    *(U8*)getNativeAddress(KThread::currentThread()->memory, address) = value;
#endif
}

U16 readw(U32 address) {
#ifdef LOG_OPS
    U16 result = *(U16*)getNativeAddress(thread->process->memory, address);
    if (thread->process->memory->log)
        fprintf(logFile, "readw %X @%X\n", result, address);
    return result;
#else
    return *(U16*)getNativeAddress(KThread::currentThread()->memory, address);
#endif
}

bool clearCodePageReadOnly(Memory* memory, U32 page);

void writew( U32 address, U16 value) {
#ifdef LOG_OPS
    if (thread->process->memory->log)
        fprintf(logFile, "writew %X @%X\n", value, address);
#endif
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    Memory* m = KThread::currentThread()->memory;
    U8 flags = m->nativeFlags[address >> K_PAGE_SHIFT];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        BtCodeMemoryWrite w((BtCPU*)KThread::currentThread()->cpu, address, 2);
        *(U16*)getNativeAddress(m, address) = value;
    } else if (flags & NATIVE_FLAG_COMMITTED) {
        *(U16*)getNativeAddress(KThread::currentThread()->memory, address) = value;
    } else {
        kpanic("writew about to crash");
    }
#else
    *(U16*)getNativeAddress(KThread::currentThread()->memory, address) = value;
#endif
}

U32 readd(U32 address) {
#ifdef LOG_OPS
    U32 result = *(U32*)getNativeAddress(thread->process->memory, address);
    if (thread->process->memory->log)
        fprintf(logFile, "readd %X @%X\n", result, address);
    return result;
#else
    return *(U32*)getNativeAddress(KThread::currentThread()->memory, address);
#endif
}

void writed(U32 address, U32 value) {
#ifdef LOG_OPS
    if (thread->process->memory->log)
        fprintf(logFile, "writed %X @%X\n", value, address);
#endif
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    Memory* m = KThread::currentThread()->memory;
    U32 page = address >> K_PAGE_SHIFT;
    U8 flags = m->nativeFlags[page];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        BtCodeMemoryWrite w((BtCPU*)KThread::currentThread()->cpu, address, 4);
        *(U32*)getNativeAddress(m, address) = value;
    } else if ((flags & NATIVE_FLAG_COMMITTED) || (m->flags[page] & PAGE_MAPPED_HOST)) {
        *(U32*)getNativeAddress(KThread::currentThread()->memory, address) = value;
    } else {
        kpanic("writed about to crash");
    }
#else
    *(U32*)getNativeAddress(KThread::currentThread()->memory, address) = value;
#endif
}

U64 readq(U32 address) {
    return *(U64*)getNativeAddress(KThread::currentThread()->memory, address);
}

void writeq(U32 address, U64 value) {
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    Memory* m = KThread::currentThread()->memory;
    U32 page = address >> K_PAGE_SHIFT;
    U8 flags = m->nativeFlags[page];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        BtCodeMemoryWrite w((BtCPU*)KThread::currentThread()->cpu, address, 8);
        *(U64*)getNativeAddress(m, address) = value;
    } else if ((flags & NATIVE_FLAG_COMMITTED) || (m->flags[page] & PAGE_MAPPED_HOST)) {
        *(U64*)getNativeAddress(KThread::currentThread()->memory, address) = value;
    } else {
        kpanic("writeq about to crash");
    }
#else
    *(U64*)getNativeAddress(KThread::currentThread()->memory, address) = value;
#endif
}

void Memory::addCallback(OpCallback func) {
    U64 funcAddress = (U64)func;

    U8* address = (U8*)getNativeAddress(this, CALL_BACK_ADDRESS)+this->callbackPos;
    
    *address=0xFE;
    address++;
    *address=0x38;
    address++;
    *address=(U8)funcAddress;
    address++;
    *address=(U8)(funcAddress >> 8);
    address++;
    *address=(U8)(funcAddress >> 16);
    address++;
    *address=(U8)(funcAddress >> 24);
    address++;
    *address=(U8)(funcAddress >> 32);
    address++;
    *address=(U8)(funcAddress >> 40);
    address++;
    *address=(U8)(funcAddress >> 48);
    address++;
    *address=(U8)(funcAddress >> 56);
    this->callbackPos+=12;
}

void Memory::map(U32 startPage, const std::vector<U8*>& pages, U32 permissions) {
    kpanic("64-bit mmu hasn't implemented shared memory");
}

#define BLOCKS_IN_CACHE 256
#define BLOCK_CACHE_SHIFT 4

class BlockCache {	
public:    
    DecodedBlock* block;
    BlockCache* next;
    U32 ip;
    BlockCache* linkTo;
    BlockCache* linkFrom;
};

static BlockCache* freeCacheBlocks;

BlockCache* allocCacheBlock() {
    if (freeCacheBlocks) {
        BlockCache* result = freeCacheBlocks;
        freeCacheBlocks = freeCacheBlocks->next;
        result->block = NULL;
        result->next = NULL;
        result->ip = 0;
        result->linkTo = NULL;
        result->linkFrom = NULL;
        return result;
    } else {
        BlockCache* cacheBlock = new BlockCache[1024];

        for (U32 i=0;i<1024;i++) {
            cacheBlock[i].next = freeCacheBlocks;
            freeCacheBlocks = &cacheBlock[i];
        }
        return allocCacheBlock();
    }
}

void freeCacheBlock(BlockCache* cacheBlock) {
    cacheBlock->next = freeCacheBlocks;
    freeCacheBlocks = cacheBlock;
}

DecodedBlock* Memory::getCodeBlock(U32 startIp) {
#ifndef BOXEDWINE_BINARY_TRANSLATOR
    BlockCache** cacheBlocks = (BlockCache**)this->codeCache[startIp >> K_PAGE_SHIFT];
    BlockCache* cacheBlock;
    if (!cacheBlocks)
        return NULL;
    cacheBlock = cacheBlocks[(startIp & 0xFFF)>>BLOCK_CACHE_SHIFT];
    while (cacheBlock && cacheBlock->block) {
        // if linkFrom is set, then this block shouldn't be used because it is a partial block,
        if (cacheBlock->ip == startIp && !cacheBlock->linkFrom)
            return cacheBlock->block;
        cacheBlock = cacheBlock->next;
    }
#endif
    return NULL;
}

void Memory::addCodeBlock(U32 startIp, DecodedBlock* block) {
#ifndef BOXEDWINE_BINARY_TRANSLATOR
    this->internalAddCodeBlock(startIp, block);
#endif
}

#ifndef BOXEDWINE_BINARY_TRANSLATOR
void* Memory::internalAddCodeBlock(U32 startIp, DecodedBlock* block) {
    BlockCache** cacheBlocks = (BlockCache**)this->codeCache[startIp >> K_PAGE_SHIFT];
    U32 index = (startIp & 0xFFF)>>BLOCK_CACHE_SHIFT;
    BlockCache* cacheBlock = allocCacheBlock();

    if (!cacheBlocks) {
        cacheBlocks = new BlockCache*[BLOCKS_IN_CACHE];
        memset(cacheBlocks, 0, sizeof(BlockCache*)*BLOCKS_IN_CACHE);
        this->codeCache[startIp >> K_PAGE_SHIFT] = cacheBlocks;
    }
    cacheBlock->next = cacheBlocks[index];
    cacheBlocks[index] = cacheBlock;
    cacheBlock->block = block;
    cacheBlock->ip = startIp;
    if (((startIp & K_PAGE_MASK)+block->bytes) > K_PAGE_SIZE) {
        if (block->bytes>K_PAGE_SIZE)
            kpanic("64-bit MMU wasn't expecting a code block to be bigger than one page");
        BlockCache* to = (BlockCache*)this->internalAddCodeBlock((startIp & ~K_PAGE_MASK)+K_PAGE_SIZE, block);
        cacheBlock->linkTo = to;
        to->linkFrom = cacheBlock;
    }
    makeCodePageReadOnly(this, this->getNativePage(startIp>>K_PAGE_SHIFT));
    return cacheBlock;
    return NULL;
}

void Memory::removeBlock(DecodedBlock* block, U32 ip) {
    BlockCache** cacheblocks = (BlockCache**)this->codeCache[ip >> K_PAGE_SHIFT];
    BlockCache* cacheBlock;
    BlockCache* prev = NULL;

    if (!cacheblocks)
        return;
    cacheBlock = cacheblocks[(ip & 0xFFF)>>BLOCK_CACHE_SHIFT];
    while (cacheBlock && cacheBlock->block) {
        if (cacheBlock->block == block) {
            if (prev) {
                prev->next = cacheBlock->next;
            } else {
                cacheblocks[(ip & 0xFFF)>>BLOCK_CACHE_SHIFT] = cacheBlock->next;
            }
            if (cacheBlock->linkTo) {
                cacheBlock->linkTo->linkFrom = NULL;
                this->removeBlock(block, cacheBlock->linkTo->ip);
            }
            if (cacheBlock->linkFrom) {
                cacheBlock->linkFrom->linkTo = NULL;
                this->removeBlock(block, cacheBlock->linkFrom->ip);
            }
            freeCacheBlock(cacheBlock);
            break;
        }
        prev = cacheBlock;
        cacheBlock = cacheBlock->next;
    }
}
#endif

#ifndef BOXEDWINE_BINARY_TRANSLATOR
static void OPCALL emptyOp(CPU* cpu, DecodedOp* op) {
    cpu->nextBlock = NULL;
    cpu->yield = true;
}
#endif

void Memory::clearCodePageFromCache(U32 page) {
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    if (KSystem::useLargeAddressSpace) {
        KThread* thread = KThread::currentThread();
        std::shared_ptr<KProcess> process;

        if (thread) {
            process = thread->process;
        }
        if (process && this->isEipPageCommitted(page)) {
            U64 offset = (U64)(page << K_PAGE_SHIFT) * sizeof(void*);
            U64* address64 = (U64*)((U8*)this->eipToHostInstructionAddressSpaceMapping + offset);
            for (U32 j = 0; j < K_PAGE_SIZE; j++, address64++) {
                *address64 = (U64)process->reTranslateChunkAddressFromR9;
            }
        }
    } else {
        void** table = this->eipToHostInstructionPages[page];
        if (table) {
            for (U32 i = 0; i < K_PAGE_SIZE; i++) {
                void* hostAddress = table[i];
                if (hostAddress) {
                    std::shared_ptr<BtCodeChunk> chunk = this->getCodeChunkContainingHostAddress(hostAddress);
                    if (chunk) {
                        i += chunk->getHostAddressLen();
                        chunk->release(this);
                    }
                }
            }
            delete[] table;
            this->eipToHostInstructionPages[page] = NULL;
        }
    }
    this->dynamicCodePageUpdateCount[page] = 0;
    
    U32 nativePage = this->getNativePage(page);
    U32 startingPage = this->getEmulatedPage(nativePage);
    for (int i=0;i<K_NATIVE_PAGES_PER_PAGE;i++) {
        if (this->eipToHostInstructionPages[startingPage+i]) {
            return;
        }
    }
    clearCodePageReadOnly(this, nativePage);
#else
    BlockCache** cacheblocks = (BlockCache**)this->codeCache[page];
    if (cacheblocks) {
        U32 i;
        for (i=0;i<BLOCKS_IN_CACHE;i++) {
            BlockCache* cacheBlock = cacheblocks[i];
            while (cacheBlock) {
                DecodedBlock* block = cacheBlock->block;
                BlockCache* c = cacheBlock->next;

                this->removeBlock(block, cacheBlock->ip);

                if (DecodedBlock::currentBlock == block) {
                    // we don't have a pointer to the current op, so just set them all
                    DecodedOp* op = DecodedBlock::currentBlock->op;
                    while (op) {
                        op->pfn = emptyOp; // This will cause the current block to return
                        op = op->next;
                    }
                    block->dealloc(true);
                } else {
                    block->dealloc(false);
                }          
                cacheBlock = c;
            }
        }
        delete[] cacheblocks;
        this->codeCache[page] = 0;
    }
    U32 nativePage = this->getNativePage(page);
    U32 startingPage = this->getEmulatedPage(nativePage);
    for (int i=0;i<K_NATIVE_PAGES_PER_PAGE;i++) {
        if (this->codeCache[startingPage+i]) {
            return;
        }
    }
    clearCodePageReadOnly(this, nativePage);
#endif
}

U32 Memory::getPageFlags(U32 page) {
    return this->flags[page];
}

void Memory::onThreadChanged() {
}

U8* getPhysicalAddress(U32 address, U32 len) {
    if (!address)
        return NULL;
    return (U8*)getNativeAddress(KThread::currentThread()->process->memory, address);
}

U8* getPhysicalReadAddress(U32 address, U32 len) {
    if (!address)
        return NULL;
    return (U8*)getNativeAddress(KThread::currentThread()->process->memory, address);
}

U8* getPhysicalWriteAddress(U32 address, U32 len) {
    if (!address)
        return NULL;
    return (U8*)getNativeAddress(KThread::currentThread()->process->memory, address);
}

#ifdef BOXEDWINE_BINARY_TRANSLATOR
// called when BtCodeChunk is being dealloc'd
void Memory::removeCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk) {
    U32 hostPage = (U32)(((size_t)chunk->getHostAddress()) >> K_PAGE_SHIFT);
    if (this->codeChunksByHostPage.count(hostPage)) {
        std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByHostPage[hostPage];
        chunks->remove(chunk);
        if (chunks->size() == 0) {
            this->codeChunksByHostPage.erase(hostPage);
        }
    }

    U32 emulationPage = (chunk->getEip()) >> K_PAGE_SHIFT;
    if (this->codeChunksByEmulationPage.count(emulationPage)) {
        std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByEmulationPage[emulationPage];
        chunks->remove(chunk);
        if (chunks->size() == 0) {
            this->codeChunksByEmulationPage.erase(emulationPage);
        }
    }
}

// called when BtCodeChunk is being alloc'd
void Memory::addCodeChunk(const std::shared_ptr<BtCodeChunk>& chunk) {
    U32 hostPage = (U32)(((size_t)chunk->getHostAddress()) >> K_PAGE_SHIFT);
    U32 emulationPage = (chunk->getEip()) >> K_PAGE_SHIFT;
#ifdef _DEBUG
    U32 lastPage = (U32)(((size_t)((U8*)chunk->getHostAddress() + chunk->getHostAddressLen())) >> K_PAGE_SHIFT);
    for (U32 i=hostPage;i<=lastPage;i++) {
        if (this->codeChunksByHostPage.count(hostPage)) {
            std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByHostPage[hostPage];
            for (auto& otherChunk : *chunks) {
                if (otherChunk->containsHostAddress(chunk->getHostAddress())) {
                    kpanic("Memory::addCodeChunk chunks can not overlap");
                }
            }
        }
    }
#endif
    std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > hostChunks = this->codeChunksByHostPage[hostPage];
    if (!hostChunks) {
        hostChunks = std::make_shared< std::list<std::shared_ptr<BtCodeChunk>> >();
        this->codeChunksByHostPage[hostPage] = hostChunks;
    }
    hostChunks->push_back(chunk);

    std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByEmulationPage[emulationPage];
    if (!chunks) {
        chunks = std::make_shared< std::list<std::shared_ptr<BtCodeChunk>> >();
        this->codeChunksByEmulationPage[emulationPage] = chunks;
    }
    chunks->push_back(chunk);
}

void Memory::makeNativePageDynamic(U32 nativePage) {
    U32 startPage = getEmulatedPage(nativePage);
    for (U32 i = 0; i < K_NATIVE_PAGES_PER_PAGE; i++) {
        U32 page = startPage + i;
        if (this->codeChunksByEmulationPage.count(page)) {
            std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByEmulationPage[page];
            for (auto& chunk : *chunks) {
                if (!chunk->isDynamicAware()) {
                    chunk->invalidateStartingAt(chunk->getEip());
                }
            }
        }
    }
    if (this->nativeFlags[nativePage] & NATIVE_FLAG_CODEPAGE_READONLY) {
        ::clearCodePageReadOnly(this, nativePage);
    }
    U32 page = startPage;
    U32 eip = page << K_PAGE_SHIFT;
    page--;
    // look to see if a chunk that starts in a previous page contains this address
    // chunks do not overlap, so find the first previous page with a chunk then
    // check on the chunks in that page
    while (page>0) {
        if (this->codeChunksByEmulationPage.count(page)) {
            std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByEmulationPage[page];
            for (auto& chunk : *chunks) {
                if (chunk->containsEip(eip)) {
                    chunk->invalidateStartingAt(eip);
                    return;
                }
            }
            break;
        }
        page--;
    }
}

// only called during code patching, if this become a performance problem maybe we could just it up
// like with soft_code_page
std::shared_ptr<BtCodeChunk> Memory::getCodeChunkContainingHostAddress(void* hostAddress) {
    U32 page = (U32)((size_t)hostAddress >> K_PAGE_SHIFT);
    if (this->codeChunksByHostPage.count(page)) {
        std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByHostPage[page];
        for (auto& chunk : *chunks) {
            if (chunk->containsHostAddress(hostAddress)) {
                return chunk;
            }
        }
    }
    page--;
    // look to see if a chunk that starts in a previous page contains this address
    // chunks do not overlap, so find the first previous page with a chunk then
    // check on the chunks in that page
    while (page>0) {
        if (this->codeChunksByHostPage.count(page)) {
            std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByHostPage[page];
            for (auto& chunk : *chunks) {
                if (chunk->containsHostAddress(hostAddress)) {
                    return chunk;
                }
            }
            break;
        }
        page--;
    }
    return NULL;
}

std::shared_ptr<BtCodeChunk> Memory::getCodeChunkContainingEip(U32 eip) {
    for (U32 i=0;i<K_MAX_X86_OP_LEN;i++) {
        void* hostAddress = getExistingHostAddress(eip-i);
        if (hostAddress) {
            std::shared_ptr<BtCodeChunk> result = this->getCodeChunkContainingHostAddress(hostAddress);
            if (result->containsEip(eip)) {
                return result;
            }
            return NULL;
        }
    }
    return NULL;
}

void Memory::invalideHostCode(U32 eip, U32 len) {
    for (U32 i=eip;i<eip+len;i++) {
        std::shared_ptr<BtCodeChunk> chunk = getCodeChunkContainingEip(i);
        if (chunk && !chunk->isDynamicAware()) {
            chunk->invalidateStartingAt(i);
            i=chunk->getEip()+chunk->getEipLen()-1;
        }
    }

    U32 startPage = this->getNativePage(eip >> K_PAGE_SHIFT);
    U32 endPage = this->getNativePagegetNativePage((eip+len) >> K_PAGE_SHIFT);
    for (U32 nativePage = startPage; nativePage <= endPage; nativePage++) {
        if (dynamicCodePageUpdateCount[nativePage]!=MAX_DYNAMIC_CODE_PAGE_COUNT) {
            dynamicCodePageUpdateCount[nativePage]++;
            if (dynamicCodePageUpdateCount[nativePage]==MAX_DYNAMIC_CODE_PAGE_COUNT) {
                this->makeNativePageDynamic(nativePage);
            }
        }
    }
}

// call during code translation, this needs to be fast
void* Memory::getExistingHostAddress(U32 eip) {
    if (KSystem::useLargeAddressSpace) {
        if (!this->isEipPageCommitted(eip >> K_PAGE_SHIFT)) {
            return NULL;
        }
        void* result = (void*)(*(U64*)(((U8*)this->eipToHostInstructionAddressSpaceMapping) + ((U64)eip * sizeof(void*))));
        if (result == KThread::currentThread()->process->reTranslateChunkAddressFromR9) {
            return NULL;
        }
        return result;
    } else {
        U32 page = eip >> K_PAGE_SHIFT;
        U32 offset = eip & K_PAGE_MASK;
        if (this->eipToHostInstructionPages[page])
            return this->eipToHostInstructionPages[page][offset];
        return NULL;
    }
}

bool Memory::isEipPageCommitted(U32 page) {
    return this->committedEipPages[page];
}

void Memory::setEipForHostMapping(U32 eip, void* host) {
    U32 page = eip >> K_PAGE_SHIFT;
    if (!this->isEipPageCommitted(page)) {
        commitHostAddressSpaceMapping(this, page, 1, (U64)KThread::currentThread()->process->reTranslateChunkAddressFromR9);
    }
    U64* address = (U64*)(((U8*)this->eipToHostInstructionAddressSpaceMapping) + ((U64)eip) * sizeof(void*));
    *address = (U64)host;
}

int powerOf2(U32 requestedSize, U32& size) {
    size = 2;
    U32 powerOf2Size = 1;
    while (size < requestedSize) {
        size <<= 1;
        powerOf2Size++;
    }
    return powerOf2Size;
}

void* Memory::allocateExcutableMemory(U32 requestedSize, U32* allocatedSize) {
    U32 size = 0;
    U32 powerOf2Size = powerOf2(requestedSize, size);

    if (powerOf2Size<EXECUTABLE_MIN_SIZE_POWER) {
        powerOf2Size = EXECUTABLE_MIN_SIZE_POWER;
        size = 1 << EXECUTABLE_MIN_SIZE_POWER;
    } else if (powerOf2Size>EXECUTABLE_MAX_SIZE_POWER) {
        kpanic("x64 code chunk was larger than 64k");
    }
    if (allocatedSize) {
        *allocatedSize = size;
    }
    U32 index = powerOf2Size - EXECUTABLE_MIN_SIZE_POWER;
    if (!this->freeExecutableMemory[index].empty()) {
        void* result = this->freeExecutableMemory[index].front();
        this->freeExecutableMemory[index].pop_front();
        return result;
    }
    void* result = (void*)(this->executableMemoryId | (this->nextExecutablePage << K_PAGE_SHIFT));
    U32 count = (size+65535)/65536;
    for (U32 i=0;i<count;i++) {
        allocExecutable64kBlock(this, this->nextExecutablePage);
        this->nextExecutablePage+=16;
    }
    
    count = 65536 / size;
    for (U32 i=1;i<count;i++) {
        this->freeExecutableMemory[index].push_back(((U8*)result) + size * i);
    }   
    return result;
}

void Memory::freeExcutableMemory(void* hostMemory, U32 actualSize) {
    memset(hostMemory, 0xcd, actualSize);
    
    U32 size = 0;
    U32 powerOf2Size = powerOf2(actualSize, size);
    U32 index = powerOf2Size - EXECUTABLE_MIN_SIZE_POWER;
    //this->freeExecutableMemory[index].push_back(hostMemory);

    // :TODO: when this recycled, make sure we delay the recycling in case another thread is also waiting in seh_filter 
    // for its turn to jump to this chunk at the same time another thread retranslated it
    //
    // I saw this in the Real Deal installer
}

void Memory::executableMemoryReleased() {
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    this->codeChunksByHostPage.clear();
    this->codeChunksByEmulationPage.clear();
    for (U32 i = 0; i < EXECUTABLE_SIZES; i++) {
        this->freeExecutableMemory[i].clear();
    }
#endif   
}
#endif

#endif
