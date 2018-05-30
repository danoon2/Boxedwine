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
#ifdef BOXEDWINE_64BIT_MMU
#include "memory.h"
#include "log.h"
#include "kscheduler.h"
#include "kprocess.h"
#include "kalloc.h"
#include "ksignal.h"
#include "ksystem.h"
#include "hard_memory.h"
#include "../cpu/decodedata.h"
#include "decoder.h"
#include "kthread.h"

#include <string.h>
#include <setjmp.h>

extern jmp_buf runBlockJump;

void log_pf(struct KThread* thread, U32 address) {
    U32 start = 0;
    U32 i;
    struct CPU* cpu = &thread->cpu;

    printf("%.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X %s at %.8X\n", cpu->segAddress[CS] + cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, getModuleName(cpu, cpu->segAddress[CS]+cpu->eip.u32), getModuleEip(cpu, cpu->segAddress[CS]+cpu->eip.u32));

    printf("Page Fault at %.8X\n", address);
    printf("Valid address ranges:\n");
    for (i=0;i<NUMBER_OF_PAGES;i++) {
        if (!start) {
            if (thread->process->memory->flags[i] & PAGE_IN_RAM) {
                start = i;
            }
        } else {
            if (!(thread->process->memory->flags[i] & PAGE_IN_RAM)) {
                printf("    %.8X - %.8X\n", start*PAGE_SIZE, i*PAGE_SIZE);
                start = 0;
            }
        }
    }
    printf("Mapped Files:\n");
    for (i=0;i<MAX_MAPPED_FILE;i++) {
        if (thread->process->mappedFiles[i].refCount)
            printf("    %.8X - %.8X %s\n", thread->process->mappedFiles[i].address, thread->process->mappedFiles[i].address+(int)thread->process->mappedFiles[i].len, thread->process->mappedFiles[i].file->openFile->node->path);
    }
    walkStack(cpu, cpu->eip.u32, EBP, 2);
    kpanic("pf");
}

void seg_mapper(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;
    if (memory->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && memory->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL) {
        U32 eip = thread->cpu.eip.u32;

        memory->process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        memory->process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        memory->process->sigActions[K_SIGSEGV].sigInfo[2] = 1; // SEGV_MAPERR
        memory->process->sigActions[K_SIGSEGV].sigInfo[3] = address;
        runSignal(thread, thread, K_SIGSEGV, EXCEPTION_PAGE_FAULT, 0);
    } else {
        log_pf(thread, address);
    }
}

void seg_access(struct KThread* thread, U32 address) {
    struct Memory* memory = thread->process->memory;
    if (memory->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_IGN && memory->process->sigActions[K_SIGSEGV].handlerAndSigAction!=K_SIG_DFL) {
        U32 eip = thread->cpu.eip.u32;

        memory->process->sigActions[K_SIGSEGV].sigInfo[0] = K_SIGSEGV;		
        memory->process->sigActions[K_SIGSEGV].sigInfo[1] = 0;
        memory->process->sigActions[K_SIGSEGV].sigInfo[2] = 2; // SEGV_ACCERR
        memory->process->sigActions[K_SIGSEGV].sigInfo[3] = address;
        runSignal(thread, thread, K_SIGSEGV, EXCEPTION_PERMISSION, 0);
        printf("seg fault %X\n", address);
    } else {
        log_pf(thread, address);
    }
}

struct Memory* allocMemory(struct KProcess* process) {
    struct Memory* memory = (struct Memory*)kalloc(sizeof(struct Memory), KALLOC_MEMORY);    
    initMemory(memory);
    memory->process = process;
    reserveNativeMemory(memory);
    return memory;
}

void initMemory(struct Memory* memory) {
    memset(memory, 0, sizeof(struct Memory));
}

void resetMemory(struct Memory* memory) {
    releaseNativeMemory(memory);
    reserveNativeMemory(memory);
}

void cloneMemory(struct KThread* toThread, struct KThread* fromThread) {
    struct Memory* from = fromThread->process->memory;
    struct Memory* memory = toThread->process->memory;

    int i=0;    
    for (i=0;i<0x100000;i++) {
        if (from->flags[i] & PAGE_IN_RAM) {            
            if ((from->flags[i] & PAGE_SHARED) && (from->flags[i] & PAGE_WRITE)) {
                static U32 shown = 0;
                if (!shown) {
                    klog("forking a process with shared memory is not fully supported with BOXEDWINE_64BIT_MMU");
                    shown=1;
                }
            }
            allocNativeMemory(memory, i, 1, from->flags[i]);
            memcpy(getNativeAddress(toThread->process->memory, i << PAGE_SHIFT), getNativeAddress(fromThread->process->memory, i << PAGE_SHIFT), PAGE_SIZE);
        } else {
            memory->flags[i] = from->flags[i];
        }     
    }
}

void freeMemory(struct Memory* memory) {
    releaseNativeMemory(memory);
    kfree(memory, KALLOC_MEMORY);
}

void zeroMemory(struct KThread* thread, U32 address, int len) {
    memset(getNativeAddress(thread->process->memory, address), 0, len);
}

void readMemory(struct KThread* thread, U8* data, U32 address, int len) {
    memcpy(data, getNativeAddress(thread->process->memory, address), len);
}

void writeMemory(struct KThread* thread, U32 address, U8* data, int len) {
    memcpy(getNativeAddress(thread->process->memory, address), data, len);
}

BOOL findFirstAvailablePage(struct Memory* memory, U32 startingPage, U32 pageCount, U32* result, BOOL canBeReMapped) {
    U32 i;
    
    for (i=startingPage;i<NUMBER_OF_PAGES;i++) {
        if ((memory->flags[i] & (PAGE_MAPPED|PAGE_IN_RAM)) == 0 || (canBeReMapped && (memory->flags[i] & PAGE_MAPPED))) {
            U32 j;
            BOOL success = TRUE;

            for (j=1;j<pageCount;j++) {
                if ((memory->flags[i+j] & (PAGE_MAPPED|PAGE_IN_RAM)) && (!canBeReMapped || !(memory->flags[i+j] & PAGE_MAPPED))) {
                    success = FALSE;
                    break;
                }
            }
            if (success) {
                *result = i;
                return TRUE;
            }
            i+=j; // no reason to check all the pages again
        }
    }
    return FALSE;
}

U32 mapNativeMemory(struct Memory* memory, void* hostAddress, U32 size) {
    U32 i;
    U32 result = 0;    
    U32 pageCount = (size>>PAGE_SHIFT)+2; // 1 for size alignment, 1 for hostAddress alignment
    U64 hostStart = (U64)hostAddress & 0xFFFFFFFFFFFFF000l;
    U64 offset;

    findFirstAvailablePage(memory, 0x10000, pageCount, &result, FALSE);
    offset = hostStart - (result << PAGE_SHIFT);
    // host = client + offset
    for (i=0;i<pageCount;i++) {
        memory->ids[result+i] = offset;
    }
    return (result << PAGE_SHIFT) + ((U32)hostAddress & PAGE_MASK);
}

void unmapNativeMemory(struct Memory* memory, U32 address, U32 size) {
}

void releaseMemory(struct KThread* thread, U32 startingPage, U32 pageCount) {
    freeNativeMemory(thread->process, startingPage, pageCount);
}

void allocPages(struct KThread* thread, U32 page, U32 pageCount, U8 permissions, U32 fildes, U64 offset, U32 cacheIndex) {
    if ((permissions & PAGE_PERMISSION_MASK) || fildes) {
        allocNativeMemory(thread->process->memory, page, pageCount, permissions);
    } else {
        U32 i;
        releaseMemory(thread, page, pageCount);
        for (i=0;i<pageCount;i++) {
            thread->process->memory->flags[i+page]=permissions;
        }
    }
    if (fildes) {
        struct KFileDescriptor* fd = getFileDescriptor(thread->process, fildes);
        U64 pos = fd->kobject->openFile->func->getFilePointer(fd->kobject->openFile);
        fd->kobject->openFile->func->seek(fd->kobject->openFile, offset);
        fd->kobject->openFile->func->read(thread, fd->kobject->openFile, page << PAGE_SHIFT, pageCount << PAGE_SHIFT);        
        fd->kobject->openFile->func->seek(fd->kobject->openFile, pos);
    }    
}

void protectPage(struct KThread* thread, U32 i, U32 permissions) {
    struct Memory* memory = thread->process->memory;

    if (!(memory->flags[i] & PAGE_IN_RAM) && (permissions & PAGE_PERMISSION_MASK)) {
        allocPages(thread, i, 1, permissions, 0, 0, 0);
    } else {
        memory->flags[i] &=~ PAGE_PERMISSION_MASK;
        memory->flags[i] |= permissions;
    } 
}

void freePage(struct KThread* thread, U32 page) {
    freeNativeMemory(thread->process, page, 1);
}

void memcopyFromNative(struct KThread* thread, U32 address, const char* p, U32 len) {
    memcpy(getNativeAddress(thread->process->memory, address), p, len);
}

void memcopyToNative(struct KThread* thread, U32 address, char* p, U32 len) {
    memcpy(p, getNativeAddress(thread->process->memory, address), len);
}

void writeNativeString(struct KThread* thread, U32 address, const char* str) {	
    strcpy(getNativeAddress(thread->process->memory, address), str);
}

U32 writeNativeString2(struct KThread* thread, U32 address, const char* str, U32 len) {	
    U32 count=0;

    while (*str && count<len-1) {
        writeb(thread, address, *str);
        str++;
        address++;
        count++;
    }
    writeb(thread, address, 0);
    return count;
}

void writeNativeStringW(struct KThread* thread, U32 address, const char* str) {	
    while (*str) {
        writew(thread, address, *str);
        str++;
        address+=2;
    }
    writew(thread, address, 0);
}

char* getNativeString(struct KThread* thread, U32 address, char* buffer, U32 cbResult) {
    if (!address) {
        buffer=0;
        return buffer;
    }
    return (char*)getNativeAddress(thread->process->memory, address);
}

char* getNativeStringW(struct KThread* thread, U32 address, char* buffer, U32 cbResult) {
    char c;
    U32 i=0;

    if (!address) {
        buffer=0;
        return buffer;
    }
    do {
        c = (char)readw(thread, address);
        address+=2;
        buffer[i++] = c;
    } while(c && i<cbResult);
    buffer[cbResult-1]=0;
    return buffer;
}

U32 getNativeStringLen(struct KThread* thread, U32 address) {
    return (U32)strlen((char*)getNativeAddress(thread->process->memory, address));
}

U8 readb(struct KThread* thread, U32 address) {
#ifdef LOG_OPS
    U8 result = *(U8*)getNativeAddress(thread->process->memory, address);
    if (thread->process->memory->log)
        fprintf(logFile, "readb %X @%X\n", result, address);
    return result;
#else
    return *(U8*)getNativeAddress(thread->process->memory, address);
#endif
}

void writeb(struct KThread* thread, U32 address, U8 value) {
#ifdef LOG_OPS
    if (thread->process->memory->log)
        fprintf(logFile, "writeb %X @%X\n", value, address);
#endif
    *(U8*)getNativeAddress(thread->process->memory, address) = value;
}

U16 readw(struct KThread* thread, U32 address) {
#ifdef LOG_OPS
    U16 result = *(U16*)getNativeAddress(thread->process->memory, address);
    if (thread->process->memory->log)
        fprintf(logFile, "readw %X @%X\n", result, address);
    return result;
#else
    return *(U16*)getNativeAddress(thread->process->memory, address);
#endif
}

void writew(struct KThread* thread, U32 address, U16 value) {
#ifdef LOG_OPS
    if (thread->process->memory->log)
        fprintf(logFile, "writew %X @%X\n", value, address);
#endif
    *(U16*)getNativeAddress(thread->process->memory, address) = value;
}

U32 readd(struct KThread* thread, U32 address) {
#ifdef LOG_OPS
    U32 result = *(U32*)getNativeAddress(thread->process->memory, address);
    if (thread->process->memory->log)
        fprintf(logFile, "readd %X @%X\n", result, address);
    return result;
#else
    return *(U32*)getNativeAddress(thread->process->memory, address);
#endif
}

void writed(struct KThread* thread, U32 address, U32 value) {
#ifdef LOG_OPS
    if (thread->process->memory->log)
        fprintf(logFile, "writed %X @%X\n", value, address);
#endif
    *(U32*)getNativeAddress(thread->process->memory, address) = value;
}

static U8* callbackAddress;
static int callbackPos;
static U32 callbacks[512];

#define CALLBACK_OP_SIZE 12

void addCallback(void (OPCALL *func)(struct CPU*, struct Op*)) {
    U64 funcAddress = (U64)func;
    U8* address = callbackAddress+callbackPos*CALLBACK_OP_SIZE;
    callbacks[callbackPos++] = (U32)address;
    
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
}

void initCallbacks() {
    callbackAddress = kalloc(4096, 0);
    addCallback(onExitSignal);
}

void initCallbacksInProcess(struct KProcess* process) {
    U32 page = CALL_BACK_ADDRESS >> PAGE_SHIFT;
    allocNativeMemory(process->memory, page, 1, PAGE_READ|PAGE_EXEC);
    memcpy((U8*)process->memory->id+CALL_BACK_ADDRESS, callbackAddress, 4096);
}

PblMap* codeCache;
void initBlockCache() {
    codeCache = pblMapNewHashMap();
}

#define BLOCKS_IN_CACHE 256
#define BLOCK_CACHE_SHIFT 4

struct BlockCache {	
    struct Block* block;
    struct BlockCache* next;
};

struct BlockCache* freeCacheBlocks;

struct BlockCache* allocCacheBlock() {
    if (freeCacheBlocks) {
        struct BlockCache* result = freeCacheBlocks;
        freeCacheBlocks = freeCacheBlocks->next;
        return result;
    } else {
        U32 count = 1024*1023/sizeof(struct BlockCache);
        U32 i;
        struct BlockCache* cacheBlock = (struct BlockCache*)kalloc(1024*1023, KALLOC_OP);

        for (i=0;i<count;i++) {
            cacheBlock->next = freeCacheBlocks;
            freeCacheBlocks = cacheBlock;
            cacheBlock++;
        }
        return allocCacheBlock();
    }
}

void freeCacheBlock(struct BlockCache* cacheBlock) {
    cacheBlock->next = freeCacheBlocks;
    freeCacheBlocks = cacheBlock;
}

struct Block* getBlockFromCache(struct Memory* memory, U32 ip) {
    struct BlockCache** cacheblocks = (struct BlockCache**)memory->codeCache[ip >> PAGE_SHIFT];
    struct BlockCache* cacheBlock;
    if (!cacheblocks)
        return NULL;
    cacheBlock = cacheblocks[(ip & 0xFFF)>>BLOCK_CACHE_SHIFT];
    while (cacheBlock && cacheBlock->block) {
        if (cacheBlock->block->ip == ip)
            return cacheBlock->block;
        cacheBlock = cacheBlock->next;
    }
    return 0;
}

void addBlockToCache(struct Memory* memory, struct Block* block, U32 ip) {
    struct BlockCache** cacheBlocks = memory->codeCache[ip >> PAGE_SHIFT];
    U32 index = (ip & 0xFFF)>>BLOCK_CACHE_SHIFT;
    struct BlockCache* cacheBlock = allocCacheBlock();

    if (!cacheBlocks) {
        cacheBlocks = (struct BlockCache**)kalloc(sizeof(struct BlockCache*)*BLOCKS_IN_CACHE, 0);
        memory->codeCache[ip >> PAGE_SHIFT] = cacheBlocks;
    }
    cacheBlock->next = cacheBlocks[index];
    cacheBlocks[index] = cacheBlock;
    cacheBlock->block = block;
}

void removeBlockFromPage(struct Memory* memory, struct Block* block, U32 page, U32 ip) {
    struct BlockCache** cacheblocks = (struct BlockCache**)memory->codeCache[page];
    struct BlockCache* cacheBlock;
    struct BlockCache* prev = NULL;

    if (!cacheblocks)
        return;
    cacheBlock = cacheblocks[(ip & 0xFFF)>>BLOCK_CACHE_SHIFT];
    while (cacheBlock && cacheBlock->block) {
        if (cacheBlock->block == block) {
            if (prev)
                prev->next = cacheBlock->next;
            else
                cacheblocks[(ip & 0xFFF)>>BLOCK_CACHE_SHIFT] = cacheBlock->next;
            freeCacheBlock(cacheBlock);
            break;
        }
        prev = cacheBlock;
        cacheBlock = cacheBlock->next;
    }
}

void clearPageFromBlockCache(struct Memory* memory, struct KThread* thread, U32 page) {
    struct BlockCache** cacheblocks = (struct BlockCache**)memory->codeCache[page];
    if (cacheblocks) {
        U32 i;
        for (i=0;i<BLOCKS_IN_CACHE;i++) {
            struct BlockCache* cacheBlock = cacheblocks[i];
            while (cacheBlock) {
                struct Block* block = cacheBlock->block;
                struct BlockCache* c;

                // does this block spanned more than one page?
                if (block->page[0]!=page) {
                    removeBlockFromPage(memory, block, block->page[0], block->ip);
                } else if (block->page[1] && block->page[1]!=page) {
                    removeBlockFromPage(memory, block, block->page[1], block->page[1] << PAGE_SHIFT);
                }

                if (thread && thread->cpu.currentBlock == block) {
                    delayFreeBlockAndKillCurrentBlock(block);
                } else {
                    freeBlock(block);
                }
                c = cacheBlock;                
                cacheBlock = cacheBlock->next;
                freeCacheBlock(c);
            }
        }
        kfree(cacheblocks, 0);
        memory->codeCache[page] = 0;
    }   
#if BOXEDWINE_VM
    {
        void** entries = memory->process->opToAddressPages[page];

        if (entries) {
            U32 i;
            for (i=0;i<PAGE_SIZE;i++) {
                if (memory->process->opToAddressPages[page][i]) {
                    U64 host = (U64)memory->process->opToAddressPages[page][i];
                    if (memory->process->hostToEip[((U32)host)>>PAGE_SHIFT]) {
                        memory->process->hostToEip[((U32)host)>>PAGE_SHIFT][host & 0xFFF]=0;
                    }
                }
            }
            memory->process->opToAddressPages[page] = NULL;
            kfree(entries, KALLOC_IP_CACHE);
        }
    }
#endif
}

struct Block* getBlock(struct CPU* cpu, U32 eip) {
    U32 ip;
    struct Block* block;
    if (cpu->big)
        ip = cpu->segAddress[CS] + eip;
    else
        ip = cpu->segAddress[CS] + (eip & 0xFFFF);
    block = getBlockFromCache(cpu->memory, ip);
    if (!block) {
        block = decodeBlock(cpu, eip);
        block->ip = ip;
        block->page[0] = (ip >> PAGE_SHIFT);
        addBlockToCache(cpu->memory, block, ip);
        makeCodePageReadOnly(cpu->memory, ip >> PAGE_SHIFT);
        if ((ip & PAGE_MASK) + block->eipCount >= PAGE_SIZE) {
            U32 finished = PAGE_SIZE-(ip & PAGE_MASK);
                
            if (cpu->big)
                ip = cpu->segAddress[CS] + eip + finished;
            else
                ip = cpu->segAddress[CS] + (((eip & 0xFFFF) + finished) & 0xFFFF);
            makeCodePageReadOnly(cpu->memory, ip >> PAGE_SHIFT);

            // this page will never return this block because block->ip doesn't exist in the page, but we add it to the page 
            // so that when the page is cleared we will remove the block from both pages
            addBlockToCache(cpu->memory, block, ip);
            block->page[1] = (ip >> PAGE_SHIFT);
        }        
    }
    return block;
}

U8 FETCH8(struct DecodeData* data) {
    U32 address;

    if (data->cpu->big)
        address = data->ip + data->cpu->segAddress[CS];
    else
        address = (data->ip & 0xFFFF) + data->cpu->segAddress[CS];
    data->ip++;
    return readb(data->cpu->thread, address);
}

U16 FETCH16(struct DecodeData* data) {
    U32 address;

    if (data->cpu->big)
        address = data->ip + data->cpu->segAddress[CS];
    else
        address = (data->ip & 0xFFFF) + data->cpu->segAddress[CS];
    data->ip+=2;
    return readw(data->cpu->thread, address);
}

U32 FETCH32(struct DecodeData* data) {
    U32 address;

    if (data->cpu->big)
        address = data->ip + data->cpu->segAddress[CS];
    else
        address = (data->ip & 0xFFFF) + data->cpu->segAddress[CS];
    data->ip+=4;
    return readd(data->cpu->thread, address);
}

U32 getMemoryAllocated(struct Memory* memory) {
    return memory->allocated;
}

U8* getPhysicalAddress(struct KThread* thread, U32 address) {
    return getNativeAddress(thread->process->memory, address);
}

void initDecodeData(struct DecodeData* data) {
}

BOOL isValidReadAddress(struct KThread* thread, U32 address) {
    return (thread->process->memory->flags[address >> PAGE_SHIFT] & PAGE_IN_RAM)!=0;
}

BOOL isPageInMemory(struct Memory* memory, U32 page) {
    return (memory->flags[page] & PAGE_IN_RAM)!=0;
}

void* allocMappable(struct Memory* memory, U32 pageCount) {
    kpanic("Shared memory not supported with BOXEDWINE_64BIT_MMU yet");
    return 0;
}

void freeMappable(struct Memory* memory, void* address) {    
}

void mapMappable(struct KThread* thread, U32 page, U32 pageCount, void* address, U32 permissions) {    
}

void unmapMappable(struct KThread* thread, U32 page, U32 pageCount) {
}

void initRAM(U32 pages) {

}

U32 getPageCount() {
    return 262144; // 1GB
}

U32 getFreePageCount() {
    return 196608; // 768MB
}

void closeMemoryMapped(struct MapedFiles* mapped) {
    mapped->refCount--;
    if (mapped->refCount == 0) {
        closeKObject(mapped->file);
        mapped->file = 0;

    }
}

#endif