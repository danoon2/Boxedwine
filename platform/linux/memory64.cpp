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
    U32 nativePageStart = memory->getNativePage(page);
    U32 nativePageStop = memory->getNativePage(page+pageCount-1);
    U32 nativePageCount = nativePageStop - nativePageStart + 1;

    if ((flags & PAGE_READ) || (flags & PAGE_EXEC)) {
        proto|=PROT_READ;
    }
    if (flags & PAGE_WRITE) {
        proto|=PROT_WRITE;
    }
    void* p = (char*)memory->id + (nativePageStart << K_NATIVE_PAGE_SHIFT);
    if (mprotect(p, nativePageCount << K_NATIVE_PAGE_SHIFT, PROT_READ | PROT_WRITE)<0) {
        kpanic("allocNativeMemory mprotect failed: %s", strerror(errno));
    }
    memory->allocated += pageCount<< K_PAGE_SHIFT;
    for (U32 i=0;i<pageCount;i++) {
        memory->flags[page+i] = flags |= PAGE_ALLOCATED;
        memset(getNativeAddress(memory, (page+i) << K_PAGE_SHIFT), 0, K_PAGE_SIZE);
    }
    // :TODO: figure out how to re-enable
    // if (mprotect(p, nativePageCount << K_NATIVE_PAGE_SHIFT, proto)<0) {
    //     kpanic("allocNativeMemory mprotect failed: %s", strerror(errno));
    // }
    for (U32 i=nativePageStart;i<=nativePageStop;i++) {
        if (!(memory->nativeFlags[i] & NATIVE_FLAG_COMMITTED)) {
            memory->nativeFlags[i] |= NATIVE_FLAG_COMMITTED;
            nativeMemoryPagesAllocated++;
        }
        U32 emulatedPageStart = memory->getEmulatedPage(nativePageStart+i);
        for (U32 j=0;j<K_NATIVE_PAGES_PER_PAGE;j++) {
            U32 pageFlags = GET_PAGE_PERMISSIONS(memory->flags[emulatedPageStart+j]);
            if (pageFlags != GET_PAGE_PERMISSIONS(flags)) {
                //updateNativePermission(memory, i);
            }
        }
    }
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
    
    kern_return_t result = vm_region_recurse_64(target_task, &addr, &lsize, &depth, (vm_region_info_t)&info, &count);
    if (result == KERN_INVALID_ADDRESS) {
        return false;
    } else if (result) {
        kpanic("isAddressRangeInUse vm_region_recurse_64 failed: %d", (int)result);
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
    memory->executableMemoryReleased();
    for (auto& p : memory->allocatedExecutableMemory) {
        munmap(p.memory, p.size);
    }
	memory->allocatedExecutableMemory.clear();
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
        // :TODO: re-enable for self modifying code
        if (mprotect((char*)memory->id + (page << K_NATIVE_PAGE_SHIFT), 1 << K_NATIVE_PAGE_SHIFT, PROT_READ)==-1) {
           kpanic("makeCodePageReadOnly mprotect failed: %s", strerror(errno));
        }
        memory->nativeFlags[page] |= NATIVE_FLAG_CODEPAGE_READONLY;
    }
}

bool clearCodePageReadOnly(Memory* memory, U32 page) {
    bool result = false;
    
    if (memory->nativeFlags[page] & NATIVE_FLAG_CODEPAGE_READONLY) {
        if (mprotect((char*)memory->id + (page << K_NATIVE_PAGE_SHIFT), 1 << K_NATIVE_PAGE_SHIFT, PROT_READ|PROT_WRITE)==-1) {
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
#ifdef __MACH__
#elif defined(__aarch64__)
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
    U32 page = address >> K_PAGE_SHIFT;
    U32 nativePage = thread->memory->getNativePage(page);
    if (thread->process->memory->nativeFlags[nativePage] & NATIVE_FLAG_CODEPAGE_READONLY) {
        U32 emulatedPage = thread->memory->getEmulatedPage(nativePage);
        for (int i=0;i<K_NATIVE_PAGES_PER_PAGE;i++) {
            thread->process->memory->clearCodePageFromCache(emulatedPage + i);
        }
        // will continue
    } else {
#ifdef __MACH__
#if defined(__aarch64__)
        bool readAccess = (((ucontext_t*)context)->uc_mcontext->__es.__esr & 1) == 0;
#else
        bool readAccess = (((ucontext_t*)context)->uc_mcontext->__es.__err & 1) == 0;
#endif
#elif defined (__aarch64__)
        DecodedOp* op = DecodedBlock::currentBlock->getOp(thread->cpu->getEipAddress());
        bool readAccess = true; // :TODO: ??? where is this in the signal info
        if (op) {
            readAccess = instructionInfo[op->inst].writeMemWidth == 0;
        }
        static const U64 ESR_ELx_WNR = 1U << 6;
        U64 esr;
        if (Aarch64GetESR((ucontext_t*)context, &esr)) {
            readAccess = (esr & ESR_ELx_WNR) == 0;
        }
#else
        bool readAccess = (((ucontext_t*)context)->uc_mcontext.gregs[REG_ERR] & 1) == 0;
#endif
        
        if (!readAccess && (thread->process->memory->flags[page] & PROT_WRITE)) {
            void* p = (void*)(thread->memory->id + (thread->memory->getNativePage(page) << K_NATIVE_PAGE_SHIFT));
            mprotect(p, K_NATIVE_PAGE_SIZE, PROT_READ | PROT_WRITE);
            return;
        }
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
#ifdef __MACH__
        // proc hand -p true -s false SIGBUS
        // in the debug output window, (lldb) enter the above command in order to run while debugging on Mac
        
        // set a break point on this line then enter the above commands.
        task_set_exception_ports(mach_task_self(), EXC_MASK_BAD_ACCESS, MACH_PORT_NULL, EXCEPTION_DEFAULT, 0);
#endif
    }
    runThreadSlice(thread);
}
#endif

#ifdef BOXEDWINE_BINARY_TRANSLATOR
void* allocExecutable64kBlock(Memory* memory, U32 count) {
    void* result = mmap(NULL, 64 * 1024 * count, PROT_EXEC | PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE | MAP_BOXEDWINE, -1, 0);
    if (result == MAP_FAILED) {
        kpanic("allocExecutable64kBlock: failed to commit memory : %s", strerror(errno));
    }
    return result;
}

void commitHostAddressSpaceMapping(Memory* memory, U32 page, U32 pageCount, U64 defaultValue) {
    // for K_NATIVE_PAGES_PER_PAGE = 1, 2, 4, or 8, this should align fine
    if (K_NATIVE_PAGES_PER_PAGE > sizeof(void*)) {
        kpanic("commitHostAddressSpaceMapping does not handle a page size of %d", K_NATIVE_PAGE_SIZE);
    }
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
