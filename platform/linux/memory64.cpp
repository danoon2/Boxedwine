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
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#include "../../source/emulation/hardmmu/hard_memory.h"

U32 nativeMemoryPagesAllocated;

#ifdef BOXEDWINE_64BIT_MMU
void allocNativeMemory(Memory* memory, U32 page, U32 pageCount, U32 flags) {
    U32 proto = 0;
    
    if ((flags & PAGE_READ) || (flags & PAGE_EXEC)) {
        proto|=PROT_READ;
    }
    if (flags & PAGE_WRITE) {
        proto|=PROT_WRITE;
    }
    void* p = (char*)memory->id + (page << K_PAGE_SHIFT);
    if (mprotect(p, pageCount << K_PAGE_SHIFT, PAGE_READ|PROT_WRITE)<0) {
        kpanic("allocNativeMemory mprotect failed: %s", strerror(errno));
    }
    memory->allocated += pageCount<< K_PAGE_SHIFT;
    for (int i=0;i<(int)pageCount;i++) {
        memory->flags[page+i] = flags |= PAGE_ALLOCATED;
        memory->nativeFlags[page+i] |= NATIVE_FLAG_COMMITTED;
    }
    memset(getNativeAddress(memory, page << K_PAGE_SHIFT), 0, pageCount << K_PAGE_SHIFT);
    nativeMemoryPagesAllocated+=pageCount;
}

void freeNativeMemory(Memory* memory, U32 page, U32 pageCount) {
    for (int i=0;i<(int)pageCount;i++) {
        memory->clearCodePageFromCache(page + i);
        if (memory->nativeFlags[page+i] & NATIVE_FLAG_CODEPAGE_READONLY) {
            memory->nativeFlags[page+i] &= ~ NATIVE_FLAG_CODEPAGE_READONLY;
        }
        if (memory->nativeFlags[page+i] & NATIVE_FLAG_COMMITTED) {
            memory->nativeFlags[page+i] &= ~ NATIVE_FLAG_COMMITTED;
            mprotect((char*)memory->id + ((page+i) << K_PAGE_SHIFT), 1 << K_PAGE_SHIFT, PROT_NONE);
        }
        memory->flags[page+i] = 0;
    }
    nativeMemoryPagesAllocated-=pageCount;
}

static U64 nextMemoryId = 2;
#include <unistd.h>
#include <sys/mman.h>

#ifdef __MACH__
#include <mach/mach.h>

static bool isAddressRangeInUse(void* p, U64 len) {
    // get task for pid
    vm_map_t target_task = mach_task_self();
    
    vm_address_t iter = (vm_address_t)p;
    vm_address_t addr = iter;
    vm_size_t lsize = 0;
    uint32_t depth;
    struct vm_region_submap_info_64 info;
    mach_msg_type_number_t count = VM_REGION_SUBMAP_INFO_COUNT_64;
    
    if (vm_region_recurse_64(target_task, &addr, &lsize, &depth, (vm_region_info_t)&info, &count))
    {
        kpanic("isAddressRangeInUse vm_region_recurse_64 failed");
    }
    if (addr>=(U64)p+len) {
        return false;
    }
    return true;
}
#else

static bool isAddressRangeInUse(void* p, U64 len) {
    FILE* file=fopen("/proc/self/maps","r");
    if(!file){
        kpanic("reservereNext4GBMemory : cannot open /proc/self/maps, %s\n",strerror(errno));
        return false;
    }
    
    char buf[1024];
    while( !feof(file) ){
        char addr1[20],addr2[20];
        
        fgets(buf,1024,file);
        
        int index=0;
        int startIndex=0;
        
        //addr1
        while(buf[index]!='-'){
            addr1[index-startIndex]=buf[index];
            index++;
        }
        addr1[index]='\0';
        index++;
        //addr2
        startIndex=index;
        while(buf[index]!='\t' && buf[index]!=' '){
            addr2[index-startIndex]=buf[index];
            index++;
        }
        addr2[index-startIndex]='\0';
        
        unsigned long startAddress;
        unsigned long endAddress;
        sscanf(addr1,"%lx",(long unsigned *)&startAddress );
        sscanf(addr2,"%lx",(long unsigned *)&endAddress );
        if (startAddress>=(U64)p && startAddress<(U64)p+len) {
            fclose(file);
            return true;
        }
        if (endAddress>=(U64)p && endAddress<(U64)p+len) {
            fclose(file);
            return true;
        }
        if (startAddress<(U64)p + len && endAddress>(U64)p) {
            fclose(file);
            return true;
        }
    }
    fclose(file);
    return false;
}
#endif

static void* reserveNext4GBMemory() {
    void* p;

    while (true) {
        nextMemoryId++;
        p = (void*)(nextMemoryId << 32);
        
        if (isAddressRangeInUse(p, 0x100000000l)) {
            continue;
        }
        if (mmap(p, 0x100000000l, PROT_NONE, MAP_ANONYMOUS|MAP_FIXED|MAP_PRIVATE, -1, 0)==p) {
            break;
        }
    }
    return p;
}

static void* reserveNext32GBMemory() {
    void* p;

    while (true) {
        nextMemoryId++;
        p = (void*)(nextMemoryId << 32);
        
        if (isAddressRangeInUse(p, 0x800000000l)) {
            continue;
        }
        if (mmap(p, 0x800000000l, PROT_NONE, MAP_ANONYMOUS|MAP_FIXED|MAP_PRIVATE, -1, 0)==p) {
            break;
        }
    }
    return p;
}

void reserveNativeMemory(Memory* memory) {
    memory->id = (U64)reserveNext4GBMemory();
    for (int i = 0; i < K_NUMBER_OF_PAGES; i++) {
        memory->memOffsets[i] = memory->id;
    }
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    memory->executableMemoryId = (U64)reserveNext4GBMemory();
    memory->nextExecutablePage = 0;
    if (KSystem::useLargeAddressSpace) {
        memory->eipToHostInstructionAddressSpaceMapping = reserveNext32GBMemory();
    }
#endif
}

void releaseNativeMemory(Memory* memory) {
    for (int i=0;i<K_NUMBER_OF_PAGES;i++) {
        memory->clearCodePageFromCache(i);
    }
    memset(memory->flags, 0, sizeof(memory->flags));
    memset(memory->nativeFlags, 0, sizeof(memory->nativeFlags));
    memory->allocated = 0;
    munmap((char*)memory->id, 0x100000000l);
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    if (memory->eipToHostInstructionAddressSpaceMapping) {
        munmap((char*)memory->eipToHostInstructionAddressSpaceMapping, 0x800000000l);
        memory->eipToHostInstructionAddressSpaceMapping = NULL;
    }
#endif
}

void makeCodePageReadOnly(Memory* memory, U32 page) {
    if (!(memory->nativeFlags[page] & NATIVE_FLAG_CODEPAGE_READONLY)) {
        if (memory->dynamicCodePageUpdateCount[page]==MAX_DYNAMIC_CODE_PAGE_COUNT) {
            kpanic("makeCodePageReadOnly: tried to make a dynamic code page read-only");
        }
        if (mprotect((char*)memory->id + (page << K_PAGE_SHIFT), 1 << K_PAGE_SHIFT, PROT_READ)==-1) {
            kpanic("makeCodePageReadOnly mprotect failed: %s", strerror(errno));
        }
        memory->nativeFlags[page] |= NATIVE_FLAG_CODEPAGE_READONLY;
    }
}

bool clearCodePageReadOnly(Memory* memory, U32 page) {
    bool result = false;
    
    if (memory->nativeFlags[page] & NATIVE_FLAG_CODEPAGE_READONLY) {
        if (mprotect((char*)memory->id + (page << K_PAGE_SHIFT), 1 << K_PAGE_SHIFT, PROT_READ|PROT_WRITE)==-1) {
            kpanic("clearCodePageReadOnly mprotect failed: %s", strerror(errno));
        }
        memory->nativeFlags[page] &= ~NATIVE_FLAG_CODEPAGE_READONLY;
        result = true;
    }
    return result;
}

#ifndef BOXEDWINE_MULTI_THREADED
#include <signal.h>
#define __USE_GNU
#define _XOPEN_SOURCE
#include <ucontext.h>

// from llvm
#if defined(__aarch64__)
// Android headers in the older NDK releases miss this definition.
struct __sanitizer_esr_context {
  struct _aarch64_ctx head;
  uint64_t esr;
};
static bool Aarch64GetESR(ucontext_t *ucontext, U64 *esr) {
  static const U32 kEsrMagic = 0x45535201;
  U8 *aux = ucontext->uc_mcontext.__reserved;
  while (true) {
    _aarch64_ctx *ctx = (_aarch64_ctx *)aux;
    if (ctx->size == 0) break;
    if (ctx->magic == kEsrMagic) {
      *esr = ((__sanitizer_esr_context *)ctx)->esr;
      return true;
    }
    aux += ctx->size;
  }
  return false;
}
#endif

static void handler(int sig, siginfo_t* info, void* context)
{
    KThread* thread = KThread::currentThread();
    U32 address = getHostAddress(thread, (void*)info->si_addr);
    if (thread->process->memory->nativeFlags[address>>K_PAGE_SHIFT] & NATIVE_FLAG_CODEPAGE_READONLY) {
        U32 page = address>>K_PAGE_SHIFT;
        thread->process->memory->clearCodePageFromCache(page);
        // will continue
    } else {
#ifdef __MACH__
        bool readAccess = (((ucontext_t*)context)->uc_mcontext->__es.__err & 1) == 0;
#elif defined (__aarch64__)
        bool readAccess = true; // :TODO: ???
        static const U64 ESR_ELx_WNR = 1U << 6;
        U64 esr;
        if (Aarch64GetESR((ucontext_t*)context, &esr)) {
            readAccess = (esr & ESR_ELx_WNR) == 0;
        }
#else
        bool readAccess = (((ucontext_t*)context)->uc_mcontext.gregs[REG_ERR] & 1) == 0;
#endif
        if (info->si_code==SEGV_MAPERR) {
            thread->seg_mapper(address, readAccess, !readAccess, true);
        } else {
            thread->seg_access(address, readAccess, !readAccess, true);
        }
        // above functions will long jmp out
    }
}

void platformRunThreadSlice(KThread* thread) {
    static bool initializedHandler = false;
    if (!initializedHandler) {
        struct sigaction sa;
        struct sigaction oldsa;
        sa.sa_sigaction = handler;
        sa.sa_flags = SA_SIGINFO;
#ifdef __MACH__
        sigaction(SIGBUS, &sa, &oldsa);
#else
        sigaction(SIGSEGV, &sa, &oldsa);
#endif
        initializedHandler = true;
    }
    runThreadSlice(thread);
}
#endif

#ifdef BOXEDWINE_BINARY_TRANSLATOR
void allocExecutable64kBlock(Memory* memory, U32 page) {
    if (mmap((void*)((page << K_PAGE_SHIFT) | memory->executableMemoryId), 64*1024, PROT_EXEC | PROT_WRITE | PROT_READ,MAP_ANONYMOUS|MAP_FIXED|MAP_PRIVATE, -1, 0)==MAP_FAILED) {
        kpanic("allocExecutable64kBlock: failed to commit memory 0x%x: %s", (page << K_PAGE_SHIFT), strerror(errno));
    }
}

void commitHostAddressSpaceMapping(Memory* memory, U32 page, U32 pageCount, U64 defaultValue) {
    for (U32 i=0;i<pageCount;i++) {
        if (!memory->isEipPageCommitted(page+i)) {
            U8* address = (U8*)memory->eipToHostInstructionAddressSpaceMapping+((U64)(page+i))*K_PAGE_SIZE*sizeof(void*);
            if (mprotect(address, sizeof(void*) << K_PAGE_SHIFT, PAGE_READ|PROT_WRITE)<0) {
                kpanic("commitHostAddressSpaceMapping mprotect failed: %s", strerror(errno));
            }
            U64* address64 = (U64*)address;
            for (U32 j=0;j<K_PAGE_SIZE;j++, address64++) {
                *address64 = defaultValue;
            }
            memory->setEipPageCommitted(page+i);
        }
    }
}
#endif

#endif
