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

#ifndef __KMEMORY_SOFT_H__
#define __KMEMORY_SOFT_H__

class CodePage;
class DecodedOp;

#include "codePageData.h"
#include "soft_mmu.h"
#include "../../source/util/bnativeheap.h"

class JitData {
public:
    JitData() : len(0), eip(0) {}
    JitData(U32 len, U32 eip) : len(len), eip(eip) {}
    U32 len;
    U32 eip;
};

class KMemoryData {
public:
    static void shutdown();
    
    KMemoryData(KMemory* memory);
    ~KMemoryData();

    void addCallback(OpCallback func);
    Page* getPage(U32 page) {return mmu[page].getPage();};
    bool isPageValid(U32 page);
    void allocPages(KThread* thread, U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const std::shared_ptr<MappedFile>& mappedFile, const RamPage* ramPages = nullptr);
    bool reserveAddress(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped, bool alignNative, U32 reservedFlag);
    void protectPage(KThread* thread, U32 i, U32 permissions);
    void setPagesInvalid(U32 page, U32 pageCount);
    bool isPageAllocated(U32 page);
    bool isPageNative(U32 page);
    void execvReset();    
    void onPageChanged(U32 page);

    KMemory* memory;

    MMU mmu[K_NUMBER_OF_PAGES];
#ifdef BOXEDWINE_MEM_CACHE
#ifndef BOXEDWINE_HOST_EXCEPTIONS
#error BOXEDWINE_MEM_CACHE requires BOXEDWINE_HOST_EXCEPTIONS
#endif
    U8* readCache[K_NUMBER_OF_PAGES];
    U8* writeCache[K_NUMBER_OF_PAGES];
#endif
#ifdef BOXEDWINE_WASM_JIT
    // Inline TLB for the WASM JIT. Each entry is a 32-bit linear-memory
    // offset to the start of the page's RAM, or 0 if direct access is
    // not allowed (CodePage, RO/no-perm, on-demand-not-allocated). The
    // JIT codegen reads `wasmReadPageBase[addr>>12]`; on a non-zero
    // entry it does `i32.load{8_u,16_u,_}` from `entry + addr` and
    // skips the helper. Populated in onPageChanged() exactly the same
    // way as readCache/writeCache above, but always with a 0 sentinel
    // for no-access (no SIGSEGV trick — WASM can't catch it).
    U32 wasmReadPageBase[K_NUMBER_OF_PAGES];
    U32 wasmWritePageBase[K_NUMBER_OF_PAGES];
#endif
    CodePage* getOrCreateCodePage(U32 address);

    DecodedOpCache opCache;
    BNativeHeap codeMemory;

#ifdef BOXEDWINE_HOST_EXCEPTIONS
    bool findOpFromJitAddress(U8* jitAddress, U32& eipOfOp);
    std::map<U8*, JitData> jitAddressToEip;
#endif
};

KMemoryData* getMemData(KMemory* memory);

inline U32 KMemory::readdInline(U32 address) {
    if ((address & 0xFFF) < 0xFFD) {
        U32 index = address >> 12;
        MMU& mmu = data->mmu[index];
#if !defined(UNALIGNED_MEMORY)
        if (mmu.canReadRam) {
            return *(U32*)(&(ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF]));
        }
#endif
        return mmu.getPage()->readd(&mmu, address);
    }
    return readb(address) | (readb(address + 1) << 8) | (readb(address + 2) << 16) | (readb(address + 3) << 24);
}

inline void KMemory::writedInline(U32 address, U32 value) {
    if ((address & 0xFFF) < 0xFFD) {
        U32 index = address >> 12;
        MMU& mmu = data->mmu[index];
#if !defined(UNALIGNED_MEMORY)
        if (mmu.canWriteRam) {
            *(U32*)(&(ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF])) = value;
            return;
        }
#endif
        mmu.getPage()->writed(&mmu, address, value);
    } else {
        writeb(address, value);
        writeb(address + 1, value >> 8);
        writeb(address + 2, value >> 16);
        writeb(address + 3, value >> 24);
    }
}

#endif
