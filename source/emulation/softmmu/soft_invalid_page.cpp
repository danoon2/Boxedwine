#include "boxedwine.h"

#include "soft_invalid_page.h"
#include "kmemory_soft.h"
#include "soft_mmu.h"

U8 InvalidPage::readb(MMU* mmu, U32 address) {
    KThread* thread = KThread::currentThread();
    thread->seg_mapper(address, true, false);
    return 0;
}

void InvalidPage::writeb(MMU* mmu, U32 address, U8 value) {
    KThread* thread = KThread::currentThread();
    thread->seg_mapper(address, false, true);
}

U16 InvalidPage::readw(MMU* mmu, U32 address) {
    KThread* thread = KThread::currentThread();
    thread->seg_mapper(address, true, false);
    return 0;
}

void InvalidPage::writew(MMU* mmu, U32 address, U16 value) {
    KThread* thread = KThread::currentThread();
    thread->seg_mapper(address, false, true);
}

U32 InvalidPage::readd(MMU* mmu, U32 address) {
    KThread* thread = KThread::currentThread();
    thread->seg_mapper(address, true, false);
    return 0;
}

void InvalidPage::writed(MMU* mmu, U32 address, U32 value) {
    KThread* thread = KThread::currentThread();
    thread->seg_mapper(address, false, true);
}

bool InvalidPage::canReadRam(MMU* mmu) {
    return false;
}

bool InvalidPage::canWriteRam(MMU* mmu) {
    return false;
}

U8* InvalidPage::getRamPtr(MMU* mmu, U32 page, bool write, bool force, U32 offset, U32 len) {
    return nullptr;
}

void InvalidPage::onDemmand(MMU* mmu, U32 page) {
}
