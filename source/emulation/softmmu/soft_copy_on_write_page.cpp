#include "boxedwine.h"

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
        memory->mmu[page] = RWPage::alloc(ram, page << K_PAGE_SHIFT, this->flags);
    } else if (write) {
        memory->mmu[page] = WOPage::alloc(ram, page << K_PAGE_SHIFT, this->flags);
    } else if (read) {
        memory->mmu[page] = ROPage::alloc(ram, page << K_PAGE_SHIFT, this->flags);
    } else {
        memory->mmu[page] = NOPage::alloc(ram, page << K_PAGE_SHIFT, this->flags);
    }

    this->close();
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

U8* CopyOnWritePage::physicalAddress(U32 address) {
    copyOnWrite(address);
    return ::getPhysicalAddress(address);
}