#include "boxedwine.h"

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_no_page.h"

NOPage* NOPage::alloc(KMemoryData* memory, U8* page, U32 address, U32 flags) {
    return new NOPage(memory, page, address, flags);
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

U8* NOPage::getReadPtr(U32 address, bool makeReady) {
    return NULL;
}

U8* NOPage::getWritePtr(U32 address, U32 len, bool makeReady) {
    return NULL;
}

#endif