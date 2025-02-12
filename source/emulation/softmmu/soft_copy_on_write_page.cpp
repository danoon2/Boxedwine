#include "boxedwine.h"

#include "soft_copy_on_write_page.h"
#include "soft_ram.h"
#include "kmemory_soft.h"

CopyOnWritePage* CopyOnWritePage::alloc(RamPage page, U32 address) {
    return new CopyOnWritePage(page, address);
}

void CopyOnWritePage::copyOnWrite(U32 address) {	
    RamPage ram;
    KMemory* memory = KThread::currentThread()->memory;
    KMemoryData* mem = getMemData(memory);
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
    U32 page = address >> K_PAGE_SHIFT;

    if (mem->getPage(page) != this) {
        return;
    }

    if (!memory->mapShared(page) && ramPageUseCount(this->page)>1) {
        ram = ramPageAlloc();
        memcpy(ramPageGet(ram), ramPageGet(this->page), K_PAGE_SIZE);
    } else {
        ram = this->page;
        ramPageRetain(ram);
    }    

    mem->setPageRam(ram, address >> K_PAGE_SHIFT, false);
    ramPageRelease(ram); // setPageRam will retain
}

void CopyOnWritePage::writeb(U32 address, U8 value) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    copyOnWrite(address);
    data->memory->writeb(address, value);
}

void CopyOnWritePage::writew(U32 address, U16 value) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    copyOnWrite(address);
    data->memory->writew(address, value);
}

void CopyOnWritePage::writed(U32 address, U32 value) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    copyOnWrite(address);
    data->memory->writed(address, value);
}

U8 CopyOnWritePage::readb(U32 address) {
    if (KThread::currentThread()->memory->canRead(address >> K_PAGE_SHIFT)) {
        return RWPage::readb(address);
    }
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

U16 CopyOnWritePage::readw(U32 address) {
    if (KThread::currentThread()->memory->canRead(address >> K_PAGE_SHIFT)) {
        return RWPage::readw(address);
    }
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

U32 CopyOnWritePage::readd(U32 address) {
    if (KThread::currentThread()->memory->canRead(address >> K_PAGE_SHIFT)) {
        return RWPage::readd(address);
    }
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

U8* CopyOnWritePage::getRamPtr(KMemory* memory, U32 page, bool write, bool force, U32 offset, U32 len) {
    if (memory->canRead(page) && !write) {
        return RWPage::getRamPtr(memory, page, write, force, offset, len);
    }

    if (force && memory->canWrite(page) && write) {
        KMemoryData* data = getMemData(memory);
        copyOnWrite((page << K_PAGE_SHIFT) + offset);
        return data->getPage(page)->getRamPtr(memory, page, write, force, offset, len);
    }
    return nullptr;
}