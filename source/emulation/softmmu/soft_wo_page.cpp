#include "boxedwine.h"

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

U8* WOPage::physicalAddress(U32 address) {
    return 0;
}