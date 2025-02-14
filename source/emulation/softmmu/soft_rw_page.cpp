#include "boxedwine.h"

#include "soft_rw_page.h"
#include "soft_ram.h"
#include "soft_mmu.h"
#include "kmemory_soft.h"

U8 RWPage::readb(MMU* mmu, U32 address) {
    U8* ram = internalGetRamPtr(mmu, address, false);

    if (!ram) {
        KThread* thread = KThread::currentThread();
        thread->seg_access(address, true, false);
    }

    U32 offset = address & K_PAGE_MASK;
    return ram[offset];
}

void RWPage::writeb(MMU* mmu, U32 address, U8 value) {
    U8* ram = internalGetRamPtr(mmu, address, true);

    if (!ram) {
        KThread* thread = KThread::currentThread();
        thread->seg_access(address, false, true);
    }
    U32 offset = address & K_PAGE_MASK;
    ram[offset] = value;
}

U16 RWPage::readw(MMU* mmu, U32 address) {
    U8* ram = internalGetRamPtr(mmu, address, false);

    if (!ram) {
        KThread* thread = KThread::currentThread();
        thread->seg_access(address, true, false);
    }

    U32 offset = address & K_PAGE_MASK;
#ifdef UNALIGNED_MEMORY
    return ram[offset] | ((U16)ram[offset + 1] << 8);
#else
    return *(U16*)(&ram[offset]);
#endif
}

void RWPage::writew(MMU* mmu, U32 address, U16 value) {
    U8* ram = internalGetRamPtr(mmu, address, true);

    if (!ram) {
        KThread* thread = KThread::currentThread();
        thread->seg_access(address, false, true);
    }

    U32 offset = address & K_PAGE_MASK;
#ifdef UNALIGNED_MEMORY
    this->ram[offset] = (U8)value;
    this->ram[offset + 1] = (U8)(value >> 8);
#else
    *(U16*)(&ram[offset]) = value;
#endif
}

U32 RWPage::readd(MMU* mmu, U32 address) {
    U8* ram = internalGetRamPtr(mmu, address, false);

    if (!ram) {
        KThread* thread = KThread::currentThread();
        thread->seg_access(address, true, false);
    }

    U32 offset = address & K_PAGE_MASK;
#ifdef UNALIGNED_MEMORY
    return ram[offset] | ((U32)ram[offset + 1] << 8) | ((U32)ram[offset + 2] << 16) | ((U32)ram[offset + 3] << 24);
#else
    return *(U32*)(&ram[offset]);
#endif
}

void RWPage::writed(MMU* mmu, U32 address, U32 value) {
    U8* ram = internalGetRamPtr(mmu, address, true);

    if (!ram) {
        KThread* thread = KThread::currentThread();
        thread->seg_access(address, false, true);
    }

    U32 offset = address & K_PAGE_MASK;
#ifdef UNALIGNED_MEMORY
    ram[offset] = (U8)value;
    ram[offset] = (U8)(value >> 8);
    ram[offset] = (U8)(value >> 16);
    ram[offset] = (U8)(value >> 24);
#else
    * (U32*)(&ram[offset]) = value;
#endif
}

bool RWPage::canReadRam(MMU* mmu) {
    return mmu->ramIndex && (mmu->flags & (PAGE_READ | PAGE_EXEC));
}

bool RWPage::canWriteRam(MMU* mmu) {
    return mmu->ramIndex && (mmu->flags & PAGE_WRITE);
}

U8* RWPage::internalGetRamPtr(MMU* mmu, U32 address, bool write) {
    if (write && !(mmu->flags & PAGE_WRITE)) {
        return nullptr;
    }
    if (!write && !(mmu->flags & (PAGE_READ | PAGE_EXEC))) {
        return nullptr;
    }
    if (mmu->ramIndex == 0) {
        KMemory* memory = KThread::currentThread()->memory;
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
        if (mmu->ramIndex == 0) {
            RamPage ram = ramPageAlloc();
            U32 pageIndex = address >> K_PAGE_SHIFT;
            mmu->setPage(getMemData(memory), pageIndex, PageType::Ram, ram);
            ramPageRelease(ram);
            getMemData(memory)->onPageChanged(address >> K_PAGE_SHIFT);
        }
    }
    return ramPageGet((RamPage)mmu->ramIndex);
}

U8* RWPage::getRamPtr(MMU* mmu, U32 page, bool write, bool force, U32 offset, U32 len) {
    U8* result = internalGetRamPtr(mmu, page << K_PAGE_SHIFT, write);

    if (result && offset) {
        result += offset;
    }
    return result;
}

void RWPage::onDemmand(MMU* mmu, U32 pageIndex) {
    if (mmu->ramIndex == 0) {
        KMemory* memory = KThread::currentThread()->memory;
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
        if (mmu->ramIndex == 0) {
            RamPage ram = ramPageAlloc();
            mmu->setPage(getMemData(memory), pageIndex, PageType::Ram, ram);
            ramPageRelease(ram);
            getMemData(memory)->onPageChanged(pageIndex);
        }
    }
}