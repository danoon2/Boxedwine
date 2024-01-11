#include "boxedwine.h"

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_invalid_page.h"

U8 InvalidPage::readb(U32 address) {	
    KThread::currentThread()->seg_mapper(address, true, false);
    return 0;
}

void InvalidPage::writeb(U32 address, U8 value) {
    KThread::currentThread()->seg_mapper(address, false, true);
}

U16 InvalidPage::readw(U32 address) {
    KThread::currentThread()->seg_mapper(address, true, false);
    return 0;
}

void InvalidPage::writew(U32 address, U16 value) {
    KThread::currentThread()->seg_mapper(address, false, true);
}

U32 InvalidPage::readd(U32 address) {
    KThread::currentThread()->seg_mapper(address, true, false);
    return 0;
}

void InvalidPage::writed(U32 address, U32 value) {
    KThread::currentThread()->seg_mapper(address, false, true);
}

U8* InvalidPage::getReadPtr(U32 address, bool makeReady) {
    return NULL;
}

U8* InvalidPage::getWritePtr(U32 address, U32 len, bool makeReady) {
    return NULL;
}

#endif