#include "boxedwine.h"

#include "soft_code_page.h"
#include "soft_ram.h"
#include "kmemory_soft.h"
#include "soft_mmu.h"


void CodePage::onDemmand(MMU* mmu, U32 pageIndex) {
    KThread* thread = KThread::currentThread();
    if (!thread->memory->mapShared(pageIndex) && ramPageUseCount((RamPage)mmu->ramIndex) > 1) {
        RamPage ram = ramPageAlloc();
        ::memcpy(ramPageGet(ram), ramPageGet((RamPage)mmu->ramIndex), K_PAGE_SIZE);
        mmu->setPage(getMemData(thread->memory), pageIndex, PageType::Code, ram);
        getMemData(thread->memory)->onPageChanged(pageIndex);
    }
}

void CodePage::writeb(MMU* mmu, U32 address, U8 value) {
    if (!RWPage::canWriteRam(mmu)) {
        KThread::currentThread()->seg_access(address, false, true);
    }
    if (value != this->readb(mmu, address)) {
        KMemory* memory = KThread::currentThread()->memory;
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
        U8 currentValue = this->readb(mmu, address);
        if (currentValue == value) {
            return;
        }
        memory->removeCodeBlock(address, 1);
        onDemmand(mmu, address >> K_PAGE_SHIFT);
        Page::getRWPage()->writeb(mmu, address, value);
    }
}

void CodePage::writew(MMU* mmu, U32 address, U16 value) {
    if (!RWPage::canWriteRam(mmu)) {
        KThread::currentThread()->seg_access(address, false, true);
    }
    U16 currentValue = this->readw(mmu, address);
    if (value != currentValue) {
        KMemory* memory = KThread::currentThread()->memory;
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
        currentValue = this->readw(mmu, address);
        if (value == currentValue) {
            return;
        }
        U32 startAddress = 0;
        U32 len = 0;
        if ((U8)currentValue != (U8)value) {
            startAddress = address;
            len++;
        }
        if ((U8)(currentValue >> 8) != (U8)(value >> 8)) {
            if (!startAddress) {
                startAddress = address + 1;
            }
            len++;
        }
        memory->removeCodeBlock(startAddress, len);
        onDemmand(mmu, address >> K_PAGE_SHIFT);
        RWPage::writew(mmu, address, value);
    }
}

void CodePage::writed(MMU* mmu, U32 address, U32 value) {
    if (!RWPage::canWriteRam(mmu)) {
        KThread::currentThread()->seg_access(address, false, true);
    }
    U32 currentValue = this->readd(mmu, address);
    if (value != currentValue) {
        KMemory* memory = KThread::currentThread()->memory;
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
        currentValue = this->readd(mmu, address);
        if (value == currentValue) {
            return;
        }
        U32 startAddress = 0;
        U32 endAddress = 0;
        if ((U8)currentValue != (U8)value) {
            startAddress = address;
            endAddress = address;
        }
        if ((U8)(currentValue >> 8) != (U8)(value >> 8)) {
            if (!startAddress) {
                startAddress = address + 1;
            }
            endAddress = address + 1;
        }
        if ((U8)(currentValue >> 16) != (U8)(value >> 16)) {
            if (!startAddress) {
                startAddress = address + 2;
            }
            endAddress = address + 2;
        }
        if ((U8)(currentValue >> 24) != (U8)(value >> 24)) {
            if (!startAddress) {
                startAddress = address + 3;
            }
            endAddress = address + 3;
        }
        memory->removeCodeBlock(startAddress, endAddress - startAddress + 1);
        onDemmand(mmu, address >> K_PAGE_SHIFT);
        RWPage::writed(mmu, address, value);
    }
}

bool CodePage::canWriteRam(MMU* mmu) {
    return false;
}

U8* CodePage::getRamPtr(MMU* mmu, U32 page, bool write, bool force, U32 offset, U32 len) {
    if (!write) {
        return Page::getRWPage()->getRamPtr(mmu, page, write, force, offset, len);
    }
    if (RWPage::canWriteRam(mmu) && force) {
        if (!len && !offset) {
            len = K_PAGE_SIZE;
        }
        KMemory* memory = KThread::currentThread()->memory;
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
        memory->removeCodeBlock((page << K_PAGE_SHIFT) + offset, len);
        onDemmand(mmu, page);
        return Page::getRWPage()->getRamPtr(mmu, page, write, force, offset, len);
    }
    return nullptr;
}