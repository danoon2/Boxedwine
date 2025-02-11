#include "boxedwine.h"

#include "soft_wo_page.h"

WOPage* WOPage::alloc(RamPage page, U32 address) {
    return new WOPage(page, address);
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

U8* WOPage::getRamPtr(KMemory* memory, U32 page, bool write, bool force, U32 offset, U32 len) {
    if (write) {
        return RWPage::getRamPtr(memory, page, write, force, offset, len);
    }
    return nullptr;
}