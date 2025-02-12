#include "boxedwine.h"

#include "soft_ro_page.h"

ROPage* ROPage::alloc(RamPage page, U32 address) {
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

U8* ROPage::getRamPtr(KMemory* memory, U32 page, bool write, bool force, U32 offset, U32 len) {
    if (write) {
        return nullptr;
    }
    return RWPage::getRamPtr(memory, page, write, force, offset, len);
}