#include "boxedwine.h"

#include "soft_wo_page.h"

WOPage* WOPage::alloc(const KRamPtr& page, U32 address) {
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

U8* WOPage::getReadPtr(KMemory* memory, U32 address, bool makeReady) {
    return nullptr;
}

U8* WOPage::getWritePtr(KMemory* memory, U32 address, U32 len, bool makeReady) {
    return this->ram;
}