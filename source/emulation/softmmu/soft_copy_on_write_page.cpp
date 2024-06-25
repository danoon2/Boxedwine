#include "boxedwine.h"

#include "soft_copy_on_write_page.h"
#include "soft_ram.h"
#include "kmemory_soft.h"

CopyOnWritePage* CopyOnWritePage::alloc(const KRamPtr& page, U32 address) {
    return new CopyOnWritePage(page, address);
}

void CopyOnWritePage::copyOnWrite(U32 address) {	
    KRamPtr ram;
    KMemory* memory = KThread::currentThread()->memory;
    KMemoryData* mem = getMemData(memory);
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
    U32 page = address >> K_PAGE_SHIFT;

    if (mem->getPage(page) != this) {
        return;
    }

    if (!memory->mapShared(page) && this->page.use_count()>1) {
        ram = ramPageAlloc();
        memcpy(ram.get(), this->page.get(), K_PAGE_SIZE);
    } else {
        ram = this->page;
    }    

    mem->setPageRam(ram, address >> K_PAGE_SHIFT, false);
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

U8* CopyOnWritePage::getReadPtr(KMemory* memory, U32 address, bool makeReady) {
    if (memory->canRead(address >> K_PAGE_SHIFT)) {
        return RWPage::getReadPtr(memory, address, makeReady);
    }
    return nullptr;
}

U8* CopyOnWritePage::getWritePtr(KMemory* memory, U32 address, U32 len, bool makeReady) {
    U32 page = address >> K_PAGE_SHIFT;
    if (memory->canWrite(page) && makeReady) {
        KMemoryData* data = getMemData(memory);
        copyOnWrite(address);
        return data->getPage(page)->getWritePtr(memory, address, len, true);
    }
    return nullptr;
}
