#include "boxedwine.h"

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_wo_page.h"

WOPage* WOPage::alloc(KMemoryData* memory, U8* page, U32 address, U32 flags) {
    return new WOPage(memory, page, address, flags);
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

U8* WOPage::getReadPtr(U32 address, bool makeReady) {
    return NULL;
}

U8* WOPage::getWritePtr(U32 address, U32 len, bool makeReady) {
    return this->page;
}

#endif