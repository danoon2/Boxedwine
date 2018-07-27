#include "boxedwine.h"

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

U8* ROPage::physicalAddress(U32 address) {
    return 0;
}