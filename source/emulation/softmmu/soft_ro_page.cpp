#include "boxedwine.h"

#include "soft_ro_page.h"

ROPage* ROPage::alloc(const KRamPtr& page, U32 address) {
    return new ROPage(page, address);
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

U8* ROPage::getReadPtr(KMemory* memory, U32 address, bool makeReady) {
    return this->ram;
}

U8* ROPage::getWritePtr(KMemory* memory, U32 address, U32 len, bool makeReady) {
    return nullptr;
}