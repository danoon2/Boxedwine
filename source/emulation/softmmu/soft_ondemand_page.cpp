#include "boxedwine.h"

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_ondemand_page.h"
#include "soft_ro_page.h"
#include "soft_no_page.h"
#include "soft_rw_page.h"
#include "soft_invalid_page.h"
#include "soft_wo_page.h"

OnDemandPage* OnDemandPage::alloc(U32 flags) {
    return new OnDemandPage(flags);
}

void OnDemandPage::ondemmand(U32 address) {
    Memory* memory = KThread::currentThread()->memory;
    U32 page = address >> K_PAGE_SHIFT;
    bool read = this->canRead() || this->canExec();
    bool write = this->canWrite();
    
    if (read && write) {
        memory->setPage(page, RWPage::alloc(NULL, page << K_PAGE_SHIFT, this->flags));
    } else if (write) {
        memory->setPage(page, WOPage::alloc(NULL, page << K_PAGE_SHIFT, this->flags));
    } else if (read) {
        memory->setPage(page, ROPage::alloc(NULL, page << K_PAGE_SHIFT, this->flags));
    } else {
        memory->setPage(page, NOPage::alloc(NULL, page << K_PAGE_SHIFT, this->flags));
    }
}

U8 OnDemandPage::readb(U32 address) {
    ondemmand(address);
    return ::readb(address);
}

void OnDemandPage::writeb( U32 address, U8 value) {
    ondemmand(address);
    ::writeb(address, value);
}

U16 OnDemandPage::readw(U32 address) {
    ondemmand(address);
    return ::readw(address);
}

void OnDemandPage::writew(U32 address, U16 value) {
    ondemmand(address);
    ::writew(address, value);
}

U32 OnDemandPage::readd(U32 address) {
    ondemmand(address);
    return ::readd(address);
}

void OnDemandPage::writed(U32 address, U32 value) {
    ondemmand(address);
    ::writed(address, value);
}

U8* OnDemandPage::getCurrentReadPtr() {
    return NULL;
}

U8* OnDemandPage::getCurrentWritePtr() {
    return NULL;
}

U8* OnDemandPage::getReadAddress(U32 address, U32 len) {    
    ondemmand(address);
    return KThread::currentThread()->memory->getPage(address>>K_PAGE_SHIFT)->getReadAddress(address, len);
}

U8* OnDemandPage::getWriteAddress(U32 address, U32 len) {
    ondemmand(address);
    return KThread::currentThread()->memory->getPage(address>>K_PAGE_SHIFT)->getWriteAddress(address, len);
}

U8* OnDemandPage::getReadWriteAddress(U32 address, U32 len) {
    ondemmand(address);
    return KThread::currentThread()->memory->getPage(address>>K_PAGE_SHIFT)->getReadWriteAddress(address, len);
}

#endif