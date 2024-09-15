#include "boxedwine.h"

#include "soft_copy_on_write_page.h"
#include "soft_ram.h"
#include "kmemory_soft.h"

void CopyOnWritePage::copyOnWrite(MemInfo& info, U32 address) {
    KMemory* memory = KThread::currentThread()->memory;
    KMemoryData* mem = getMemData(memory);
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
    U32 page = address >> K_PAGE_SHIFT;

    if (info.type != (U32)PageType::Copy_On_Write_Page) {
        return;
    }

    if (!(info.flags & PAGE_SHARED) && ramPageUseCount(info.ramPageIndex)>1) {
        RamPage ram = ramPageAlloc();
        memcpy(ramPageGet(ram), ramPageGet(info.ramPageIndex), K_PAGE_SIZE);
        ramPageRelease(info.ramPageIndex);
        info.ramPageIndex = ram;
    }    
    info.type = (U32)PageType::RAM_Page;
    info.updatePermissionCache();
}

void CopyOnWritePage::writeb(MemInfo& info, U32 address, U8 value) {
    copyOnWrite(info, address);
    KThread::currentThread()->memory->writeb(address, value);
}

void CopyOnWritePage::writew(MemInfo& info, U32 address, U16 value) {
    copyOnWrite(info, address);
    KThread::currentThread()->memory->writew(address, value);
}

void CopyOnWritePage::writed(MemInfo& info, U32 address, U32 value) {
    copyOnWrite(info, address);
    KThread::currentThread()->memory->writed(address, value);
}

U8* CopyOnWritePage::getWritePtr(KMemory* memory, MemInfo& info, U32 address, U32 len, bool makeReady) {
    U32 page = address >> K_PAGE_SHIFT;
    if (memory->canWrite(page) && makeReady) {
        KMemoryData* data = getMemData(memory);
        copyOnWrite(info, address);
        return data->getPage(page)->getWritePtr(memory, info, address, len, true);
    }
    return nullptr;
}
