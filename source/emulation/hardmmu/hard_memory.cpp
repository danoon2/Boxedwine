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
#endif    
    reserveNativeMemory();

    allocNativeMemory(CALL_BACK_ADDRESS >> K_PAGE_SHIFT, K_NATIVE_PAGES_PER_PAGE, PAGE_READ | PAGE_EXEC | PAGE_WRITE);
    this->addCallback(onExitSignal);
#ifdef BOXEDWINE_DYNAMIC
    this->dynamicExecutableMemoryPos = 0;
    this->dynamicExecutableMemoryLen = 0;
#endif

    this->refCount = 1;
}

Memory::~Memory() {    
    releaseNativeMemory();
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
    releaseNativeMemory();
    reserveNativeMemory();

    this->callbackPos = 0;
    allocNativeMemory(CALL_BACK_ADDRESS >> K_PAGE_SHIFT, K_NATIVE_PAGES_PER_PAGE, PAGE_READ | PAGE_EXEC | PAGE_WRITE);
    this->addCallback(onExitSignal);
}

void Memory::releaseNativeMemory() {
    U32 i;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->executableMemoryMutex);
    for (i = 0; i < K_NUMBER_OF_PAGES; i++) {
        clearCodePageFromCache(i);
    }
    clearAllNeedsMemoryOffset();
    Platform::releaseNativeMemory((void*)this->id, 0x100000000l);
    memset(this->flags, 0, sizeof(this->flags));
    memset(this->nativeFlags, 0, sizeof(this->nativeFlags));
    memset(this->memOffsets, 0, sizeof(this->memOffsets));
    this->allocated = 0;
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    executableMemoryReleased();
    for (auto& p : this->allocatedExecutableMemory) {
        Platform::releaseNativeMemory(p.memory, p.size);
    }
    this->allocatedExecutableMemory.clear();
    if (KSystem::useLargeAddressSpace) {
        Platform::releaseNativeMemory((char*)this->eipToHostInstructionAddressSpaceMapping, 0x800000000l);
        this->eipToHostInstructionAddressSpaceMapping = NULL;
    }
#endif
}

void Memory::commitHostAddressSpaceMapping(U32 page, U32 pageCount, U64 defaultValue) {
    for (U32 i=0;i<pageCount;i++) {
        if (!this->isEipPageCommitted(page+i)) {
            U8* address = (U8*)this->eipToHostInstructionAddressSpaceMapping+((U64)(page+i))*K_PAGE_SIZE*sizeof(void*);
            // won't worry about granularity size (Platform::getPageAllocationGranularity()) since Platform::commitNativeMemory doesn't require a multiple of it
            Platform::commitNativeMemory(address, (sizeof(void*) << K_PAGE_SHIFT));
            U64* address64 = (U64*)address;
            for (U32 j=0;j<K_PAGE_SIZE;j++, address64++) {
                *address64 = defaultValue;
            }
            this->setEipPageCommitted(page+i);
        }
    }
}

void Memory::reset(U32 page, U32 pageCount) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(pageMutex);
    this->clearNeedsMemoryOffset(page, pageCount);
    freeNativeMemory(page, pageCount);        
}

void Memory::clone(Memory* from) {
    int i=0;    

    for (i=0;i<0x100000;i++) {
        if (from->isPageAllocated(i)) {
            if (from->flags[i] & PAGE_MAPPED_HOST) {
                this->flags[i] = from->flags[i];
                this->memOffsets[i] = from->memOffsets[i];
                continue;
            }
            bool changedWritePermission = false;
            if (!(from->flags[i] & PAGE_WRITE)) {
                changedWritePermission = true;
            }
            bool changedReadPermission = false;
            if (!from->isShared(i) && !(from->flags[i] & PAGE_READ)) {
                changedReadPermission = true;
                from->updateNativePermission(i, 1, PAGE_READ);
            }
            allocNativeMemory(i, 1, from->flags[i] | PAGE_WRITE);
            memcpy(getNativeAddress(this, i << K_PAGE_SHIFT), getNativeAddress(from, i << K_PAGE_SHIFT), K_PAGE_SIZE);
            if (changedWritePermission) {
                updatePagePermission(i, 1);
            }
            if (changedReadPermission) {
                from->updatePagePermission(i, 1);
            }
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

void Memory::unmapNativeMemory(U32 address, U32 size) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(pageMutex);
    U32 pageCount = (size >> K_PAGE_SHIFT) + 2; // 1 for size alignment, 1 for hostAddress alignment
    U64 pageStart = address >> K_PAGE_SHIFT;

    for (U32 i = 0; i < pageCount; i++) {
        this->memOffsets[i + pageStart] = this->id;
        this->flags[i + pageStart] = 0;
    }
}

U32 Memory::mapNativeMemory(void* hostAddress, U32 size) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(pageMutex);
    U32 i;
    U32 result = 0;
    U64 hostStart = (U64)hostAddress & 0xFFFFFFFFFFFFF000l;
    U64 hostEnd = ((U64)hostAddress + size) & 0xFFFFFFFFFFFFF000l;
    U32 pageCount = (U32)(hostEnd - hostStart + K_PAGE_MASK) >> K_PAGE_SHIFT;
    U64 offset;
    
    for (int i = 0; i < K_NUMBER_OF_PAGES; i++) {
        if ((i << K_PAGE_SHIFT) + this->memOffsets[i] == hostStart) {
            return (i << (K_PAGE_SHIFT)) + ((U32)((U64)hostAddress) & K_PAGE_MASK);
        }
    }
    findFirstAvailablePage(0x10000, pageCount, &result, false, true);
    offset = hostStart - (result << K_PAGE_SHIFT);
    for (i = 0; i < pageCount; i++) {
        this->memOffsets[result + i] = offset;
        this->flags[result + i] = PAGE_MAPPED_HOST | PAGE_READ | PAGE_WRITE;
    }
    return (result << K_PAGE_SHIFT) + ((U32)((U64)hostAddress) & K_PAGE_MASK);
}

void Memory::allocPages(U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const BoxedPtr<MappedFile>& mappedFile) {    
    for (U32 i = 0; i < pageCount; i++) {
        this->clearCodePageFromCache(page + i);
    }
    this->clearNeedsMemoryOffset(page, pageCount);
    if ((permissions & PAGE_PERMISSION_MASK) || mappedFile) {
        if ((permissions & PAGE_SHARED) == 0) {
            allocNativeMemory(page, pageCount, permissions);
        } else {
            bool needToLoad = false;       
            
            freeNativeMemory(page, pageCount);

            if (!mappedFile->systemCacheEntry->data[0]) {
                U32 len = pageCount << K_PAGE_SHIFT;
                U8* ram = new U8[len]; // make it continuous

                //klog("shared ram %.08X%.08X @%.08X len=%X\n", (U32)((U64)ram >> 32), (U32)((U64)ram), (page << K_PAGE_SHIFT), len);
                memset(ram, 0, len);
                needToLoad = true;
                mappedFile->systemCacheEntry->data[0] = ram;
                if (offset) {
                    kpanic("allocPages doesn't support offset with shared memory");
                }
            }
            U64 offset = (U64)mappedFile->systemCacheEntry->data[0] - (page << K_PAGE_SHIFT);
            for (U32 i = 0; i < pageCount; i++) {
                this->memOffsets[page + i] = offset;
                this->flags[page + i] = PAGE_MAPPED_HOST | PAGE_ALLOCATED | permissions;
            }
            // if the native page wasn't removed from memory because the allocation granularity is more than 1 page and a near by page is in use, 
            // then if we don't mark the page as read only, it won't generate an exception and the shared memory won't be used.  updatePagePermission
            // will see that these pages are shared and will use a strict (lowest permission) for all pages in the granulaty
            updatePagePermission(page, pageCount);
            if (!needToLoad) {
                return;
            }
        }
    } else {
        U32 i;
        freeNativeMemory(page, pageCount);
        for (i=0;i<pageCount;i++) {
            this->flags[i+page]=permissions;
        }
    }
    if (mappedFile) {
        bool addedWritePermission = false;
        
        // shared pages aren't in the normal native range
        if (!(permissions & PAGE_WRITE)) {
            for (U32 i=0;i<pageCount;i++) {
                this->flags[i+page]|=PAGE_WRITE;
            }
            addedWritePermission = true;
            updateNativePermission(page, pageCount, PAGE_WRITE|PAGE_READ);
        }
        // :TODO: need to implement writing back to the file
        KThread::currentThread()->process->pread64(fd, page<<K_PAGE_SHIFT, pageCount << K_PAGE_SHIFT, offset);
        if (addedWritePermission) {
            for (U32 i=0;i<pageCount;i++) {
                this->flags[i+page]&=~PAGE_WRITE;
            }
            updatePagePermission(page, pageCount);
        }
    }    
}

void Memory::protectPage(U32 i, U32 permissions) {    
    if (!this->isPageAllocated(i) && (permissions & PAGE_PERMISSION_MASK)) {
        this->allocPages(i, 1, permissions, 0, 0, 0);
    } else {
        this->flags[i] &=~ PAGE_PERMISSION_MASK;
        this->flags[i] |= permissions;
        if (this->isPageAllocated(i)) {
            updatePagePermission(i, 1);
        }
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

bool Memory::isPageMapped(U32 page) {
    return (this->flags[page] & PAGE_MAPPED) != 0;
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
    U32 nativePage = m->getNativePage(page);
    U8 flags = m->nativeFlags[nativePage];

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

void writew( U32 address, U16 value) {
#ifdef LOG_OPS
    if (thread->process->memory->log)
        fprintf(logFile, "writew %X @%X\n", value, address);
#endif
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    Memory* m = KThread::currentThread()->memory;
    U32 page = address >> K_PAGE_SHIFT;
    U32 nativePage = m->getNativePage(page);
    U8 flags = m->nativeFlags[nativePage];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        BtCodeMemoryWrite w((BtCPU*)KThread::currentThread()->cpu, address, 2);
        *(U16*)getNativeAddress(m, address) = value;
    } else if (flags & NATIVE_FLAG_COMMITTED || (m->flags[page] & PAGE_MAPPED_HOST)) {
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
    U32 nativePage = m->getNativePage(page);
    U8 flags = m->nativeFlags[nativePage];

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
    U32 nativePage = m->getNativePage(page);
    U8 flags = m->nativeFlags[nativePage];

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
    makeCodePageReadOnly(this->getNativePage(startIp>>K_PAGE_SHIFT));
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
                *address64 = (U64)process->reTranslateChunkAddressFromReg;
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
    
    U32 nativePage = this->getNativePage(page);
    U32 startingPage = this->getEmulatedPage(nativePage);
    this->dynamicCodePageUpdateCount[nativePage] = 0;
    if (this->eipToHostInstructionPages) {
        for (int i=0;i<K_NATIVE_PAGES_PER_PAGE;i++) {
            if (this->eipToHostInstructionPages[startingPage+i]) {
                return;
            }
        }
    }
    clearCodePageReadOnly(nativePage);
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
    clearCodePageReadOnly(nativePage);
#endif
}

U32 Memory::getPageFlags(U32 page) {
    return this->flags[page];
}

bool Memory::isShared(U32 page) {
    return (this->flags[page] & PAGE_SHARED) != 0;
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
        clearCodePageReadOnly(nativePage);
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

void Memory::makeCodePageReadOnly(U32 nativePage) {
    if (!(this->nativeFlags[nativePage] & NATIVE_FLAG_CODEPAGE_READONLY)) {
        if (this->dynamicCodePageUpdateCount[nativePage] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
            kpanic("makeCodePageReadOnly: tried to make a dynamic code page read-only");
        }
        this->nativeFlags[nativePage] |= NATIVE_FLAG_CODEPAGE_READONLY;
        this->updatePagePermission(this->getEmulatedPage(nativePage), K_NATIVE_PAGES_PER_PAGE);
    }
}

bool Memory::clearCodePageReadOnly(U32 nativePage) {
    bool result = false;

    if (this->nativeFlags[nativePage] & NATIVE_FLAG_CODEPAGE_READONLY) {
        this->nativeFlags[nativePage] &= ~NATIVE_FLAG_CODEPAGE_READONLY;
        this->updatePagePermission(this->getEmulatedPage(nativePage), K_NATIVE_PAGES_PER_PAGE);
        result = true;
    }
    return result;
}

void Memory::reserveNativeMemory() {
    this->id = (U64)Platform::reserveNativeMemory(false);
    for (int i = 0; i < K_NUMBER_OF_PAGES; i++) {
        this->memOffsets[i] = this->id;
    }
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    if (KSystem::useLargeAddressSpace) {
        this->eipToHostInstructionAddressSpaceMapping = Platform::reserveNativeMemory(true);
    }
#endif
}
void Memory::clearHostCodeForWriting(U32 nativePage, U32 count) {
    U32 addressStart = getEmulatedPage(nativePage) << K_PAGE_SHIFT;
    U32 addressStop = getEmulatedPage(nativePage + count) << K_PAGE_SHIFT;
    for (U32 i= addressStart;i < addressStop;i++) {
        std::shared_ptr<BtCodeChunk> chunk = getCodeChunkContainingEip(i);
        if (chunk && !chunk->isDynamicAware()) {
            chunk->invalidateStartingAt(i);
            i=chunk->getEip()+chunk->getEipLen()-1;
        }
    }

    for (U32 page = nativePage; page < nativePage + count; page++) {
        if (this->nativeFlags[page] & NATIVE_FLAG_CODEPAGE_READONLY) {
            if (dynamicCodePageUpdateCount[page] != MAX_DYNAMIC_CODE_PAGE_COUNT) {
                dynamicCodePageUpdateCount[page]++;
                if (dynamicCodePageUpdateCount[page] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
                    this->makeNativePageDynamic(page);
                }
            }
            clearCodePageReadOnly(page);
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
        if (result == KThread::currentThread()->process->reTranslateChunkAddressFromReg) {
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
        commitHostAddressSpaceMapping(page, 1, (U64)KThread::currentThread()->process->reTranslateChunkAddressFromReg);
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

bool Memory::isAddressExecutable(void* address) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(executableMemoryMutex);
    for (auto& p : this->allocatedExecutableMemory) {
        if (address >= p.memory && address < (U8*)p.memory + p.size) {
            return true;
        }
    }
    return false;
}

void* Memory::allocateExcutableMemory(U32 requestedSize, U32* allocatedSize) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(executableMemoryMutex);
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
    U32 count = (size+65535)/65536;
    void* result = Platform::allocExecutable64kBlock(count);
    this->allocatedExecutableMemory.push_back(Memory::AllocatedMemory(result, count*64*1024));
    count = 65536 / size;
    for (U32 i=1;i<count;i++) {
        this->freeExecutableMemory[index].push_back(((U8*)result) + size * i);
    }   
    return result;
}

void Memory::freeExcutableMemory(void* hostMemory, U32 actualSize) {
    Platform::writeCodeToMemory(hostMemory, actualSize, [hostMemory, actualSize] {
        memset(hostMemory, 0xcd, actualSize);
        });

    //U32 size = 0;
    //U32 powerOf2Size = powerOf2(actualSize, size);
    //U32 index = powerOf2Size - EXECUTABLE_MIN_SIZE_POWER;
    //this->freeExecutableMemory[index].push_back(hostMemory);

    // :TODO: when this recycled, make sure we delay the recycling in case another thread is also waiting in seh_filter 
    // for its turn to jump to this chunk at the same time another thread retranslated it
    //
    // I saw this in the Real Deal installer
}

void Memory::executableMemoryReleased() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(executableMemoryMutex);
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    this->codeChunksByHostPage.clear();
    this->codeChunksByEmulationPage.clear();
    for (U32 i = 0; i < EXECUTABLE_SIZES; i++) {
        this->freeExecutableMemory[i].clear();
    }
#endif   
}
#endif

static U32 getNativePermissionIndex(U32 page) {
    return (page << K_PAGE_SHIFT) >> K_NATIVE_PAGE_SHIFT;
}

void Memory::allocNativeMemory(U32 page, U32 pageCount, U32 flags) {
    U32 gran = Platform::getPageAllocationGranularity();
    U32 permissionGran = Platform::getPagePermissionGranularity();
    U32 granPage = page & ~(gran - 1);
    U32 granCount = ((gran - 1) + pageCount + (page - granPage)) / gran;    
    U32 permPerAllocPage = gran / permissionGran;

    for (U32 i = 0; i < granCount; i++) {
        U64 address = (this->id | (granPage << K_PAGE_SHIFT));
        U32 nativePermissionIndex = getNativePermissionIndex(granPage);
        if (!(this->nativeFlags[nativePermissionIndex] & NATIVE_FLAG_COMMITTED)) {            
            Platform::allocateNativeMemory(address);
            this->allocated += (gran << K_PAGE_SHIFT);
            for (U32 j = 0; j < permPerAllocPage; j++) {
                this->nativeFlags[nativePermissionIndex + j] |= NATIVE_FLAG_COMMITTED;
            }
        } else {
            // so that the memset works below
#ifdef _DEBUG
            if (permissionGran > gran) {
                kpanic("Wasn't expecting a larger permission size than the allocation size");
            }
#endif
            Platform::updateNativePermission(address, PAGE_READ | PAGE_WRITE, gran << K_PAGE_SHIFT);
        }
        granPage += gran;
    }
    for (U32 i = 0; i < pageCount; i++) {
        this->flags[page + i] = flags | PAGE_ALLOCATED;
        this->memOffsets[page + i] = this->id;
    }
    
    memset(getNativeAddress(this, page << K_PAGE_SHIFT), 0, pageCount << K_PAGE_SHIFT);

    granPage = page & ~(gran - 1);
    U32 granPageCount = granCount * gran;
    this->updatePagePermission(granPage, granPageCount);
    //printf("allocated %X - %X\n", page << PAGE_SHIFT, (page+pageCount) << PAGE_SHIFT);
}

void Memory::freeNativeMemory(U32 page, U32 pageCount) {    
    for (U32 i = 0; i < pageCount; i++) {
        U32 nativePermissionIndex = getNativePermissionIndex(page + i);
        this->nativeFlags[nativePermissionIndex] &= ~NATIVE_FLAG_CODEPAGE_READONLY;
        this->clearCodePageFromCache(page + i);
        this->flags[page + i] = 0;
        this->memOffsets[page + i] = this->id;
    }

    U32 gran = Platform::getPageAllocationGranularity();
    U32 permissionGran = Platform::getPagePermissionGranularity();
    U32 permPerAllocPage = gran / permissionGran;
    U32 granPage = page & ~(gran - 1);
    U32 granCount = ((gran - 1) + pageCount + (page - granPage)) / gran;
    for (U32 i = 0; i < granCount; i++) {
        U32 nativePermissionIndex = getNativePermissionIndex(granPage);
        if (this->nativeFlags[nativePermissionIndex] & NATIVE_FLAG_COMMITTED) {
            bool inUse = false;

            for (U32 j = 0; j < gran; j++) {
                if (this->isPageAllocated(granPage + j)) {
                    inUse = true;
                    break;
                }
            }
            if (!inUse) {
                U64 address = (this->id | (granPage << K_PAGE_SHIFT));
                Platform::freeNativeMemory(address);
                for (U32 j = 0; j < permPerAllocPage; j++) {
                    this->nativeFlags[nativePermissionIndex + j] = 0;
                }
                this->allocated -= (gran << K_PAGE_SHIFT);
            } else {
                updatePagePermission(granPage, gran);
            }
        }
        granPage += gran;
    }
}

void Memory::updatePagePermission(U32 page, U32 pageCount) {
    U32 permissionGran = Platform::getPagePermissionGranularity();
    U32 permissionGranPage = page & ~(permissionGran - 1);
    U32 permissionGranCount = ((permissionGran - 1) + pageCount + (page - permissionGranPage)) / permissionGran;    

    // could be mixed (M1 is 16K permission)
    for (U32 i = 0; i < permissionGranCount; i++) {
        bool hasShared = false;
        for (U32 i = 0; i < permissionGran; i++) {
            if (isShared(i + permissionGranPage)) {
                hasShared = true;
                break;
            } else if (flags[i + permissionGranPage] & PAGE_MAPPED_HOST) {
                hasShared = true;
                break;
            }
        }
        U32 permissions = 0;
        
        // shared needs to have 0 permission so that it generates an exception
        if (!hasShared) {
            permissions = this->flags[permissionGranPage];
            for (U32 j = 1; j < permissionGran; j++) {
                // :TODO: this should always be &, in order to use the most restrictive but this slows things down too much because of the generated exceptions
                permissions |= this->flags[permissionGranPage + j];
            }
        }
        U64 address = (this->id | (permissionGranPage << K_PAGE_SHIFT));
        U32 index = getNativePermissionIndex(permissionGranPage);
        if (this->nativeFlags[index] & NATIVE_FLAG_CODEPAGE_READONLY) {
            permissions &= ~PAGE_WRITE;
        }
        if (this->nativeFlags[index] & NATIVE_FLAG_COMMITTED) {
            this->nativeFlags[index] &= ~PAGE_PERMISSION_MASK;
            this->nativeFlags[index] |= (permissions & (PAGE_READ | PAGE_WRITE));
            Platform::updateNativePermission(address, permissions);
        }
        permissionGranPage += permissionGran;
    }
}

void Memory::updateNativePermission(U32 page, U32 pageCount, U32 permission) {
    U32 permissionGran = Platform::getPagePermissionGranularity();
    U32 permissionGranPage = page & ~(permissionGran - 1);
    U32 permissionGranCount = ((permissionGran - 1) + pageCount + (page - permissionGranPage)) / permissionGran;

    for (U32 i = 0; i < permissionGranCount; i++) {
        U64 address = (this->id | (permissionGranPage << K_PAGE_SHIFT));
        U32 nativePage = getNativePage(permissionGranPage);
        if (this->nativeFlags[nativePage] & NATIVE_FLAG_COMMITTED) {
            Platform::updateNativePermission(address, permission);
            this->nativeFlags[nativePage] &= ~PAGE_PERMISSION_MASK;
            this->nativeFlags[nativePage] |= (permission & (PAGE_READ | PAGE_WRITE));
        }
        permissionGranPage += permissionGran;
    }
}
#endif
