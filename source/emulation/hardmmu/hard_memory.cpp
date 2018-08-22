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

extern jmp_buf runBlockJump;

Memory::Memory() : allocated(0), callbackPos(0) {
    memset(flags, 0, sizeof(flags));
    memset(nativeFlags, 0, sizeof(nativeFlags));
    memset(codeCache, 0, sizeof(codeCache));
    memset(ids, 0, sizeof(ids));
    reserveNativeMemory(this);

    allocNativeMemory(this, CALL_BACK_ADDRESS >> K_PAGE_SHIFT, 1, PAGE_READ | PAGE_EXEC | PAGE_WRITE);
    this->addCallback(onExitSignal);
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
            if (thread->process->memory->ids[i]) {
                start = i;
            }
        } else {
            if (!thread->process->memory->ids[i]) {
                printf("    %.8X - %.8X\n", start*K_PAGE_SIZE, i*K_PAGE_SIZE);
                start = 0;
            }
        }
    }
    printf("Mapped Files:\n");
    for (auto& n : thread->process->getMappedFiles()) {
        const BoxedPtr<MappedFile>& mappedFile = n.second;
        printf("    %.8X - %.8X %s\n", mappedFile->address, mappedFile->address+(int)mappedFile->len, mappedFile->file->openFile->node->path.c_str());
    }
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
        if (from->ids[i]) {
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
    U32 i;
    U32 result = 0;    
    U32 pageCount = (size>>K_PAGE_SHIFT)+2; // 1 for size alignment, 1 for hostAddress alignment
    U64 hostStart = (U64)hostAddress & 0xFFFFFFFFFFFFF000l;
    U64 offset;

    findFirstAvailablePage(0x10000, pageCount, &result, false);
    offset = hostStart - (result << K_PAGE_SHIFT);
    // host = client + offset
    for (i=0;i<pageCount;i++) {
        this->ids[result+i] = offset;
    }
    return (result << K_PAGE_SHIFT) + ((U32)hostAddress & K_PAGE_MASK);
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
    if (!this->ids[i] && (permissions & PAGE_PERMISSION_MASK)) {
        this->allocPages(i, 1, permissions, 0, 0, 0);
    } else {
        this->flags[i] &=~ PAGE_PERMISSION_MASK;
        this->flags[i] |= permissions;
    } 
}

bool Memory::findFirstAvailablePage(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped) {
    U32 i;
    
    for (i=startingPage;i<K_NUMBER_OF_PAGES;i++) {
        if (((this->flags[i] & PAGE_MAPPED) == 0 && this->ids[i]==0) || (canBeReMapped && (this->flags[i] & PAGE_MAPPED))) {
            U32 j;
            bool success = true;

            for (j=1;j<pageCount;j++) {
                if (((this->flags[i+j] & PAGE_MAPPED) || this->ids[i+j]!=0) && (!canBeReMapped || !(this->flags[i+j] & PAGE_MAPPED))) {
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
    return this->ids[address >> K_PAGE_SHIFT] != 0 && (this->flags[address >> K_PAGE_SHIFT] & PAGE_READ);
}

bool Memory::isValidWriteAddress(U32 address, U32 len) {
    return this->ids[address >> K_PAGE_SHIFT] != 0 && (this->flags[address >> K_PAGE_SHIFT] & PAGE_WRITE);
}

bool Memory::isPageAllocated(U32 address) {
    return this->ids[address >> K_PAGE_SHIFT] != 0;
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
        buffer=0;
        return buffer;
    }
    return (char*)getNativeAddress(KThread::currentThread()->memory, address);
}

char* getNativeStringW(U32 address, char* buffer, U32 cbResult) {
    char c;
    U32 i=0;

    if (!address) {
        buffer=0;
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
    *(U8*)getNativeAddress(KThread::currentThread()->memory, address) = value;
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
    *(U16*)getNativeAddress(KThread::currentThread()->memory, address) = value;
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
    *(U32*)getNativeAddress(KThread::currentThread()->memory, address) = value;
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
    return 0;
}

void Memory::addCodeBlock(U32 startIp, DecodedBlock* block) {
    this->internalAddCodeBlock(startIp, block);
}

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
    return cacheBlock;
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
            if (prev)
                prev->next = cacheBlock->next;
            else
                cacheblocks[(ip & 0xFFF)>>BLOCK_CACHE_SHIFT] = cacheBlock->next;
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

void Memory::clearCodePageFromCache(U32 page) {
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
}

U32 Memory::getPageFlags(U32 page) {
    return this->flags[page];
}

U8* getPhysicalAddress(U32 address) {
    return (U8*)getNativeAddress(KThread::currentThread()->process->memory, address);
}

#endif