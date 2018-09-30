#include "boxedwine.h"

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_ro_page.h"

ROPage* ROPage::alloc(U8* page, U32 address, U32 flags) {
    return new ROPage(page, address, flags);
}

void ROPage::writeb(U32 address, U8 value) {
    KThread::currentThread()->seg_access(address, false, true);
}

void ROPage::writew(U32 address, U16 value) {
    KThread::currentThread()->seg_access(address, false, true);
}

void ROPage::writed(U32 address, U32 value) {
    KThread::currentThread()->seg_access(address, false, true);
}

U8* ROPage::getCurrentReadPtr() {
    return this->page;
}

U8* ROPage::getCurrentWritePtr() {
    return NULL;
}

U8* ROPage::getReadAddress(U32 address, U32 len) {    
    return &this->page[address - this->address];
}

U8* ROPage::getWriteAddress(U32 address, U32 len) {
    return NULL;
}

U8* ROPage::getReadWriteAddress(U32 address, U32 len) {
    return NULL;
}

#endif