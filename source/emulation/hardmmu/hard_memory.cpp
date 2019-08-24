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
#include "../cpu/x64/x64CodeChunk.h"

#ifdef BOXEDWINE_X64
#include "../cpu/x64/x64Asm.h"
#endif

Memory::Memory() : allocated(0), callbackPos(0) {
    memset(flags, 0, sizeof(flags));
    memset(nativeFlags, 0, sizeof(nativeFlags));
#ifndef BOXEDWINE_X64
    memset(codeCache, 0, sizeof(codeCache));
    memset(ids, 0, sizeof(ids));
#else
    memset(this->codeChunksByHostPage, 0, sizeof(this->codeChunksByHostPage));
    memset(this->codeChunksByEmulationPage, 0, sizeof(this->codeChunksByEmulationPage));
    memset(this->eipToHostInstruction, 0, sizeof(this->eipToHostInstruction));
    memset(this->freeExecutableMemory, 0, sizeof(this->freeExecutableMemory));
    memset(this->dynamicCodePageUpdateCount, 0, sizeof(this->dynamicCodePageUpdateCount));
#endif    
    reserveNativeMemory(this);

    allocNativeMemory(this, CALL_BACK_ADDRESS >> K_PAGE_SHIFT, 1, PAGE_READ | PAGE_EXEC | PAGE_WRITE);
    this->addCallback(onExitSignal);
#ifdef BOXEDWINE_DYNAMIC
    this->dynamicExecutableMemoryPos = 0;
    this->dynamicExecutableMemoryLen = 0;
#endif
}

Memory::~Memory() {
    releaseNativeMemory(this);
}

void Memory::log_pf(KThread* thread, U32 address) {
    U32 start = 0;
    U32 i;
    CPU* cpu = thread->cpu;

    std::string name = thread->process->getModuleName(cpu->seg[CS].address+cpu->eip.u32);
    printf("%.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X %s at %.8X\n", cpu->seg[CS].address + cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, name.c_str(), thread->process->getModuleEip(cpu->seg[CS].address+cpu->eip.u32));

    printf("Page Fault at %.8X\n", address);
    printf("Valid address ranges:\n");
    for (i=0;i<K_NUMBER_OF_PAGES;i++) {
        if (!start) {
            if (thread->process->memory->isPageAllocated(i)) {
                start = i;
            }
        } else {
            if (!thread->process->memory->isPageAllocated(i)) {
                printf("    %.8X - %.8X\n", start*K_PAGE_SIZE, i*K_PAGE_SIZE);
                start = 0;
            }
        }
    }
    printf("Mapped Files:\n");
    thread->process->printMappedFiles();
    cpu->walkStack(cpu->eip.u32, EBP, 2);
    kpanic("pf");
}

void Memory::reset() {
    releaseNativeMemory(this);
    reserveNativeMemory(this);

    this->callbackPos = 0;
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
                    klog("forking a process with shared memory is not fully supported with BOXEDWINE_64BIT_MMU");
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
    kpanic("x64 mapNativeMemory is depricated");
    return 0;
}

void Memory::allocPages(U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const BoxedPtr<MappedFile>& mappedFile) {
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
        KThread::currentThread()->process->pread64(fd, page<<K_PAGE_SHIFT, pageCount << K_PAGE_SHIFT, offset);
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

bool Memory::findFirstAvailablePage(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped) {
    U32 i;
    
    for (i=startingPage;i<K_NUMBER_OF_PAGES;i++) {
        if (((this->flags[i] & PAGE_MAPPED) == 0 && !this->isPageAllocated(i)) || (canBeReMapped && (this->flags[i] & PAGE_MAPPED))) {
            U32 j;
            bool success = true;

            for (j=1;j<pageCount;j++) {
                if (((this->flags[i+j] & PAGE_MAPPED) || this->isPageAllocated(i+j)) && (!canBeReMapped || !(this->flags[i+j] & PAGE_MAPPED))) {
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

void memcopyFromNative(U32 address, const char* p, U32 len) {
    memcpy(getNativeAddress(KThread::currentThread()->process->memory, address), p, len);
}

void memcopyToNative(U32 address, char* p, U32 len) {
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
#ifdef BOXEDWINE_X64
    Memory* m = KThread::currentThread()->memory;
    U8 flags = m->nativeFlags[address >> K_PAGE_SHIFT];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        X64AsmCodeMemoryWrite w((x64CPU*)KThread::currentThread()->cpu, address, 1);
        *(U8*)getNativeAddress(m, address) = value;
    } else if (flags & NATIVE_FLAG_COMMITTED) {
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

void writew( U32 address, U16 value) {
#ifdef LOG_OPS
    if (thread->process->memory->log)
        fprintf(logFile, "writew %X @%X\n", value, address);
#endif
#ifdef BOXEDWINE_X64
    Memory* m = KThread::currentThread()->memory;
    U8 flags = m->nativeFlags[address >> K_PAGE_SHIFT];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        X64AsmCodeMemoryWrite w((x64CPU*)KThread::currentThread()->cpu, address, 2);
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
#ifdef BOXEDWINE_X64
    Memory* m = KThread::currentThread()->memory;
    U8 flags = m->nativeFlags[address >> K_PAGE_SHIFT];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        X64AsmCodeMemoryWrite w((x64CPU*)KThread::currentThread()->cpu, address, 4);
        *(U32*)getNativeAddress(m, address) = value;
    } else if (flags & NATIVE_FLAG_COMMITTED) {
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
#ifdef BOXEDWINE_X64
    Memory* m = KThread::currentThread()->memory;
    U8 flags = m->nativeFlags[address >> K_PAGE_SHIFT];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        X64AsmCodeMemoryWrite w((x64CPU*)KThread::currentThread()->cpu, address, 8);
        *(U64*)getNativeAddress(m, address) = value;
    } else if (flags & NATIVE_FLAG_COMMITTED) {
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

    if (callbackPos==0) {
        U32 page = CALL_BACK_ADDRESS >> K_PAGE_SHIFT;
        allocNativeMemory(this, page, 1, PAGE_READ|PAGE_EXEC);
    }

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
#ifndef BOXEDWINE_X64
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
#ifndef BOXEDWINE_X64
    this->internalAddCodeBlock(startIp, block);
#endif
}

#ifndef BOXEDWINE_X64
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
    makeCodePageReadOnly(this, startIp>>K_PAGE_SHIFT);
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

static void OPCALL emptyOp(CPU* cpu, DecodedOp* op) {
    cpu->nextBlock = NULL;
    cpu->yield = true;
}

void Memory::clearCodePageFromCache(U32 page) {
#ifdef BOXEDWINE_X64
    void** table = this->eipToHostInstruction[page];
    if (table) {
        for (U32 i=0;i<K_PAGE_SIZE;i++) {
            void* hostAddress = table[i];
            if (hostAddress) {
                X64CodeChunk* chunk = this->getCodeChunkContainingHostAddress(hostAddress);
                if (chunk) { 
                    i+=chunk->getHostAddressLen();
                    chunk->dealloc();                
                }
            }
        }
        delete[] table;
        this->eipToHostInstruction[page] = NULL;
    }
    this->dynamicCodePageUpdateCount[page] = 0;
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
#endif
    clearCodePageReadOnly(this, page);
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

#ifdef BOXEDWINE_X64
// called when X64CodeChunk is being dealloc'd
void Memory::removeCodeChunk(X64CodeChunk* chunk) {
    U32 hostPage = ((U32)chunk->getHostAddress()) >> K_PAGE_SHIFT;
    U32 emulationPage = (chunk->getEip()) >> K_PAGE_SHIFT;
    X64CodeChunk* result = this->codeChunksByHostPage[hostPage];    
    if (this->codeChunksByHostPage[hostPage] == chunk) {
        this->codeChunksByHostPage[hostPage] = chunk->getNextChunkInHostPage();
    }
    result = this->codeChunksByEmulationPage[emulationPage];    
    if (this->codeChunksByEmulationPage[emulationPage] == chunk) {
        this->codeChunksByEmulationPage[emulationPage] = chunk->getNextChunkInEmulationPage();
    }
    chunk->removeFromList();
}

// called when X64CodeChunk is being alloc'd
void Memory::addCodeChunk(X64CodeChunk* chunk) {
    U32 hostPage = ((U32)chunk->getHostAddress()) >> K_PAGE_SHIFT;
    U32 emulationPage = (chunk->getEip()) >> K_PAGE_SHIFT;
#ifdef _DEBUG
    U32 lastPage = hostPage+(((U32)chunk->getHostAddress()+chunk->getHostAddressLen()) >> K_PAGE_SHIFT);
    for (U32 i=hostPage;i<=lastPage;i++) {
        X64CodeChunk* result = this->codeChunksByHostPage[i];
        while (result) {
            if (result->containsHostAddress(chunk->getHostAddress())) {
                kpanic("Memory::addCodeChunk chunks can not overlap");
            }
            result = result->getNextChunkInHostPage();
        }
    }
#endif
    X64CodeChunk* result = this->codeChunksByHostPage[hostPage];
    this->codeChunksByHostPage[hostPage] = chunk;

    if (result) {
        chunk->addNextHostChunk(result);
    }

    result = this->codeChunksByEmulationPage[emulationPage];
    this->codeChunksByEmulationPage[emulationPage] = chunk;

    if (result) {
        chunk->addNextEmulationChunk(result);
    }
}

void Memory::makePageDynamic(U32 page) {
    X64CodeChunk* result = this->codeChunksByEmulationPage[page];
    while (result) {
        if (!result->isDynamicAware()) {
            result->invalidateStartingAt(result->getEip());
        }
        result = result->getNextChunkInEmulationPage();
    }
    if (this->nativeFlags[page] & NATIVE_FLAG_CODEPAGE_READONLY) {
        ::clearCodePageReadOnly(this, page);
    }
    U32 eip = page << K_PAGE_SHIFT;
    page--;
    // look to see if a chunk that starts in a previous page contains this address
    // chunks do not overlap, so find the first previous page with a chunk then
    // check on the chunks in that page
    while (page>0) {
        if (this->codeChunksByEmulationPage[page]) {
            X64CodeChunk* result = this->codeChunksByEmulationPage[page];
            while (result) {
                if (result->containsEip(eip)) {
                    result->invalidateStartingAt(eip);
                    return;
                }
                result = result->getNextChunkInHostPage();
            }
            break;
        }
        page--;
    }
}

// only called during code patching, if this become a performance problem maybe we could just it up
// like with soft_code_page
X64CodeChunk* Memory::getCodeChunkContainingHostAddress(void* hostAddress) {
    U32 page = ((U32)hostAddress) >> K_PAGE_SHIFT;
    X64CodeChunk* result = this->codeChunksByHostPage[page];
    while (result) {
        if (result->containsHostAddress(hostAddress)) {
            return result;
        }
        result = result->getNextChunkInHostPage();
    }
    page--;
    // look to see if a chunk that starts in a previous page contains this address
    // chunks do not overlap, so find the first previous page with a chunk then
    // check on the chunks in that page
    while (page>0) {
        if (this->codeChunksByHostPage[page]) {
            X64CodeChunk* result = this->codeChunksByHostPage[page];
            while (result) {
                if (result->containsHostAddress(hostAddress)) {
                    return result;
                }
                result = result->getNextChunkInHostPage();
            }
            break;
        }
        page--;
    }
    return NULL;
}

X64CodeChunk* Memory::getCodeChunkContainingEip(U32 eip) {
    for (U32 i=0;i<K_MAX_X86_OP_LEN;i++) {
        void* hostAddress = getExistingHostAddress(eip-i);
        if (hostAddress) {
            X64CodeChunk* result = this->getCodeChunkContainingHostAddress(hostAddress);
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
        X64CodeChunk* chunk = getCodeChunkContainingEip(i);
        if (chunk && !chunk->isDynamicAware()) {
            chunk->invalidateStartingAt(i);
            i=chunk->getEip()+chunk->getEipLen()-1;
        }
    }

    U32 startPage = eip >> K_PAGE_SHIFT;
    U32 endPage = (eip+len) >> K_PAGE_SHIFT;
    for (U32 page = startPage; page <= endPage; page++) {
        if (dynamicCodePageUpdateCount[page]!=MAX_DYNAMIC_CODE_PAGE_COUNT) {
            dynamicCodePageUpdateCount[page]++;
            if (dynamicCodePageUpdateCount[page]==MAX_DYNAMIC_CODE_PAGE_COUNT) {
                this->makePageDynamic(page);
            }
        }
    }
}

// call during code translation, this needs to be fast
void* Memory::getExistingHostAddress(U32 eip) {
    U32 page = eip >> K_PAGE_SHIFT;
    U32 offset = eip & K_PAGE_MASK;
    if (this->eipToHostInstruction[page])
        return this->eipToHostInstruction[page][offset];
    return NULL;
}

void* Memory::allocateExcutableMemory(U32 requestedSize, U32* allocatedSize) {
    U32 size = 2; 
    U32 powerOf2Size = 1;
    while (size < requestedSize) {
        size <<= 1; 
        powerOf2Size++;
    }
    if (powerOf2Size<EXECUTABLE_MIN_SIZE_POWER) {
        powerOf2Size = EXECUTABLE_MIN_SIZE_POWER;
        size = 1 << EXECUTABLE_MIN_SIZE_POWER;
    } else if (powerOf2Size>EXECUTABLE_MAX_SIZE_POWER) {
        kpanic("x64 code chunk was larger than 64k");
    }
    if (allocatedSize) {
        *allocatedSize = size;
    }
    if (this->freeExecutableMemory[powerOf2Size-EXECUTABLE_MIN_SIZE_POWER]) {
        void* result = this->freeExecutableMemory[powerOf2Size-EXECUTABLE_MIN_SIZE_POWER];
        void* next = *(void**)result;
        this->freeExecutableMemory[powerOf2Size-EXECUTABLE_MIN_SIZE_POWER] = next;
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
        void* nextFree = this->freeExecutableMemory[powerOf2Size-EXECUTABLE_MIN_SIZE_POWER];
        this->freeExecutableMemory[powerOf2Size-EXECUTABLE_MIN_SIZE_POWER] = ((U8*)result)+size*i;
        *((void**)this->freeExecutableMemory[powerOf2Size-EXECUTABLE_MIN_SIZE_POWER]) = nextFree;
    }   
    return result;
}

void Memory::freeExcutableMemory(void* hostMemory, U32 size) {
    memset(hostMemory, 0xcd, size);
    // :TODO: when this recycled, make sure we delay the recycling in case another thread is also waiting in seh_filter 
    // for its turn to jump to this chunk at the same time another thread retranslated it
    //
    // I saw this in the Real Deal installer
}

void Memory::executableMemoryReleased() {
#ifdef BOXEDWINE_X64
    memset(this->codeChunksByHostPage, 0, sizeof(this->codeChunksByHostPage));
    memset(this->codeChunksByEmulationPage, 0, sizeof(this->codeChunksByEmulationPage));
    memset(this->freeExecutableMemory, 0, sizeof(this->freeExecutableMemory));
#endif   
}
#endif

#endif