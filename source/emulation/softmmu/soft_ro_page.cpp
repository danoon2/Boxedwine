#include "boxedwine.h"

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_ro_page.h"

ROPage* ROPage::alloc(KMemoryData* memory, U8* page, U32 address, U32 flags) {
    return new ROPage(memory, page, address, flags);
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

U8* ROPage::getReadPtr(U32 address, bool makeReady) {
    return this->page;
}

U8* ROPage::getWritePtr(U32 address, U32 len, bool makeReady) {
    return NULL;
}

#endif