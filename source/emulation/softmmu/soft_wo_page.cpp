#include "boxedwine.h"

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_wo_page.h"

WOPage* WOPage::alloc(U8* page, U32 address, U32 flags) {
    return new WOPage(page, address, flags);
}

U8 WOPage::readb(U32 address) {
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

U16 WOPage::readw(U32 address) {
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

U32 WOPage::readd(U32 address) {
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

U8* WOPage::getCurrentReadPtr() {
    return NULL;
}

U8* WOPage::getCurrentWritePtr() {
    return this->page;
}

U8* WOPage::getReadAddress(U32 address, U32 len) {    
    return NULL;
}

U8* WOPage::getWriteAddress(U32 address, U32 len) {
    return &this->page[address - this->address];
}

U8* WOPage::getReadWriteAddress(U32 address, U32 len) {
    return NULL;
}

#endif