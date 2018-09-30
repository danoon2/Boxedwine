#include "boxedwine.h"

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_copy_on_write_page.h"
#include "soft_ro_page.h"
#include "soft_rw_page.h"
#include "soft_invalid_page.h"
#include "soft_wo_page.h"
#include "soft_ram.h"
#include "soft_no_page.h"

CopyOnWritePage* CopyOnWritePage::alloc(U8* page, U32 address, U32 flags) {
    return new CopyOnWritePage(page, address, flags);
}

void CopyOnWritePage::copyOnWrite(U32 address) {	
    Memory* memory = KThread::currentThread()->memory;
    U32 page = address >> K_PAGE_SHIFT;
    bool read = this->canRead() | this->canExec();
    bool write = this->canWrite();
    U8* ram;

    if (ramPageRefCount(this->page)>1) {
        ram = ramPageAlloc();
        memcpy(ram, this->page, K_PAGE_SIZE);
    } else {
        ram = this->page;
        ramPageIncRef(ram);
    }    

    if (read && write) {
        memory->setPage(page, RWPage::alloc(ram, page << K_PAGE_SHIFT, this->flags));
    } else if (write) {
        memory->setPage(page, WOPage::alloc(ram, page << K_PAGE_SHIFT, this->flags));
    } else if (read) {
        memory->setPage(page, ROPage::alloc(ram, page << K_PAGE_SHIFT, this->flags));
    } else {
        memory->setPage(page, NOPage::alloc(ram, page << K_PAGE_SHIFT, this->flags));
    }
}

void CopyOnWritePage::writeb( U32 address, U8 value) {
    copyOnWrite(address);
    ::writeb(address, value);
}

void CopyOnWritePage::writew(U32 address, U16 value) {
    copyOnWrite(address);
    ::writew(address, value);
}

void CopyOnWritePage::writed(U32 address, U32 value) {
    copyOnWrite(address);
    ::writed(address, value);
}

U8* CopyOnWritePage::getCurrentReadPtr() {
    return this->page;
}

U8* CopyOnWritePage::getCurrentWritePtr() {
    return NULL;
}

U8* CopyOnWritePage::getReadAddress(U32 address, U32 len) {    
    return &this->page[address - this->address];
}

U8* CopyOnWritePage::getWriteAddress(U32 address, U32 len) {
    copyOnWrite(address);
    return KThread::currentThread()->memory->getPage(address>>K_PAGE_SHIFT)->getWriteAddress(address, len);
}

U8* CopyOnWritePage::getReadWriteAddress(U32 address, U32 len) {
    copyOnWrite(address);
    return KThread::currentThread()->memory->getPage(address>>K_PAGE_SHIFT)->getReadWriteAddress(address, len);
}

#endif