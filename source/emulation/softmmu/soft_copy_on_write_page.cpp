#include "boxedwine.h"

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_copy_on_write_page.h"
#include "soft_ram.h"
#include "kmemory_soft.h"

CopyOnWritePage* CopyOnWritePage::alloc(KMemoryData* memory, U8* page, U32 address, U32 flags) {
    return new CopyOnWritePage(memory, page, address, flags);
}

void CopyOnWritePage::copyOnWrite(U32 address) {	
    U8* ram;

    if (!mapShared() && ramPageRefCount(this->page)>1) {
        ram = ramPageAlloc();
        memcpy(ram, this->page, K_PAGE_SIZE);
    } else {
        ram = this->page;
        ramPageIncRef(ram);
    }    

    memory->setPageRamWithFlags(ram, address >> K_PAGE_SHIFT, flags, false);
    ramPageDecRef(ram); // setPageRamWithFlags will increment this
}

void CopyOnWritePage::writeb( U32 address, U8 value) {
    KMemoryData* data = memory;
    copyOnWrite(address);
    data->memory->writeb(address, value);
}

void CopyOnWritePage::writew(U32 address, U16 value) {
    KMemoryData* data = memory;
    copyOnWrite(address);
    data->memory->writew(address, value);
}

void CopyOnWritePage::writed(U32 address, U32 value) {
    KMemoryData* data = memory;
    copyOnWrite(address);
    data->memory->writed(address, value);
}

U8 CopyOnWritePage::readb(U32 address) {
    if (canRead()) {
        return RWPage::readb(address);
    }
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

U16 CopyOnWritePage::readw(U32 address) {
    if (canRead()) {
        return RWPage::readw(address);
    }
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

U32 CopyOnWritePage::readd(U32 address) {
    if (canRead()) {
        return RWPage::readd(address);
    }
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

U8* CopyOnWritePage::getReadPtr(U32 address, bool makeReady) {
    if (canRead()) {
        return RWPage::getReadPtr(address, makeReady);
    }
    return nullptr;
}

U8* CopyOnWritePage::getWritePtr(U32 address, U32 len, bool makeReady) {
    if (canWrite() && makeReady) {
        KMemoryData* data = memory;
        copyOnWrite(address);
        return data->getPage(address >> K_PAGE_SHIFT)->getWritePtr(address, len, true);
    }
    return nullptr;
}

#endif