#include "boxedwine.h"

#include "soft_copy_on_write_page.h"
#include "soft_ram.h"
#include "kmemory_soft.h"
#include "soft_mmu.h"

void CopyOnWritePage::writeb(MMU* mmu, U32 address, U8 value) {
    onDemmand(mmu, address >> K_PAGE_SHIFT);
    RWPage::writeb(mmu, address, value);
}

void CopyOnWritePage::writew(MMU* mmu, U32 address, U16 value) {
    onDemmand(mmu, address >> K_PAGE_SHIFT);
    RWPage::writew(mmu, address, value);
}

void CopyOnWritePage::writed(MMU* mmu, U32 address, U32 value) {
    onDemmand(mmu, address >> K_PAGE_SHIFT);
    RWPage::writed(mmu, address, value);
}

bool CopyOnWritePage::canWriteRam(MMU* mmu) {
    return false;
}

U8* CopyOnWritePage::getRamPtr(MMU* mmu, U32 page, bool write, bool force, U32 offset, U32 len) {
    if (!write) {
        return RWPage::getRamPtr(mmu, page, write, force, offset, len);
    }
    if (!force) {
        return nullptr;
    }
    onDemmand(mmu, page);
    return RWPage::getRamPtr(mmu, page, write, force, offset, len);
}

void CopyOnWritePage::onDemmand(MMU* mmu, U32 pageIndex) {
    KMemory* memory = KThread::currentThread()->memory;
    KMemoryData* mem = getMemData(memory);
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);

    if (mmu->getPageType() != PageType::CopyOnWrite) {
        return;
    }

    if (!memory->mapShared(pageIndex) && ramPageUseCount((RamPage)mmu->ramIndex) > 1) {
        RamPage ramIndex = ramPageAlloc();
        RamPage currentRamPage = mmu->getRamPageIndex();

        memcpy(ramPageGet(ramIndex), ramPageGet(currentRamPage), K_PAGE_SIZE);
        mmu->setPage(getMemData(memory), pageIndex, PageType::Ram, ramIndex);
        ramPageRelease(ramIndex);
    } else {
        mmu->setPageType(getMemData(memory), pageIndex, PageType::Ram);
    }
    getMemData(memory)->onPageChanged(pageIndex);
}
