#include "boxedwine.h"

#include "soft_rw_page.h"
#include "soft_ram.h"

U8 RWPage::readb(MemInfo& info, U32 address) {
    if (info.read) {
        return ramPageGet(info.ramPageIndex)[address & K_PAGE_MASK];
    }
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

void RWPage::writeb(MemInfo& info, U32 address, U8 value) {
    if (!info.write) {
        KThread::currentThread()->seg_access(address, false, true);
    }
    ramPageGet(info.ramPageIndex)[address & K_PAGE_MASK] = value;
}

U16 RWPage::readw(MemInfo& info, U32 address) {
    if (!info.read) {
        KThread::currentThread()->seg_access(address, true, false);
    }
    // bounds have already been checked in KMemory::readw(U32 address)
#ifdef UNALIGNED_MEMORY
    U8* ram = getRamPageAtIndex(info.ramPageIndex);
    U32 offset = address & K_PAGE_MASK;
    return ram[offset] | ((U16)ram[offset+1] << 8);
#else
    return *(U16*)(&ramPageGet(info.ramPageIndex)[address & K_PAGE_MASK]);
#endif
}

void RWPage::writew(MemInfo& info, U32 address, U16 value) {
    if (!info.write) {
        KThread::currentThread()->seg_access(address, false, true);
    }
#ifdef UNALIGNED_MEMORY
    U8* ram = getRamPageAtIndex(info.ramPageIndex);
    U32 offset = address & K_PAGE_MASK;
    ram[offset] = (U8)value;
    ram[offset+1] = (U8)(value >> 8);
#else
    *(U16*)(&ramPageGet(info.ramPageIndex)[address & K_PAGE_MASK]) = value;
#endif
}

U32 RWPage::readd(MemInfo& info, U32 address) {
    if (!info.read) {
        KThread::currentThread()->seg_access(address, true, false);
}
#ifdef UNALIGNED_MEMORY
    U8* ram = getRamPageAtIndex(info.ramPageIndex);
    U32 offset = address & K_PAGE_MASK;
    return ram[offset] | ((U32)this->ram[offset +1] << 8) | ((U32)this->ram[offset +2] << 16) | ((U32)this->ram[offset +3] << 24);
#else
    return *(U32*)(&ramPageGet(info.ramPageIndex)[address & K_PAGE_MASK]);
#endif
}

void RWPage::writed(MemInfo & info, U32 address, U32 value) {
    if (!info.write) {
        KThread::currentThread()->seg_access(address, false, true);
    }
#ifdef UNALIGNED_MEMORY
    U8* ram = getRamPageAtIndex(info.ramPageIndex);
    U32 offset = address & K_PAGE_MASK;
    this->ram[offset] = (U8)value;
    this->ram[offset + 1] = (U8)(value >> 8);
    this->ram[offset + 2] = (U8)(value >> 16);
    this->ram[offset + 3] = (U8)(value >> 24);
#else
    *(U32*)(&ramPageGet(info.ramPageIndex)[address & K_PAGE_MASK]) = value;
#endif
}

U8* RWPage::getReadPtr(KMemory* memory, MemInfo& info, U32 address, bool makeReady) {
    if (info.read) {
        return ramPageGet(info.ramPageIndex);
    }
    return nullptr;
}

U8* RWPage::getWritePtr(KMemory* memory, MemInfo& info, U32 address, U32 len, bool makeReady) {
    if (info.write) {
        return ramPageGet(info.ramPageIndex);
    }
    return nullptr;
}

void RWPage::onDemand(KMemory* memory, MemInfo& info, U32 address) {
}