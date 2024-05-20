#include "boxedwine.h"

#include "soft_no_page.h"

NOPage* NOPage::alloc(const KRamPtr& page, U32 address) {
    return new NOPage(page, address);
}

U8 NOPage::readb(U32 address) {
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

U16 NOPage::readw(U32 address) {
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

U32 NOPage::readd(U32 address) {
    KThread::currentThread()->seg_access(address, true, false);
    return 0;
}

void NOPage::writeb(U32 address, U8 value) {
    KThread::currentThread()->seg_access(address, false, true);
}

void NOPage::writew(U32 address, U16 value) {
    KThread::currentThread()->seg_access(address, false, true);
}

void NOPage::writed(U32 address, U32 value) {
    KThread::currentThread()->seg_access(address, false, true);
}

U8* NOPage::getReadPtr(KMemory* memory, U32 address, bool makeReady) {
    return nullptr;
}

U8* NOPage::getWritePtr(KMemory* memory, U32 address, U32 len, bool makeReady) {
    return nullptr;
}