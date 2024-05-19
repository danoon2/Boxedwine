#include "boxedwine.h"

#include "soft_invalid_page.h"
#include "kmemory_soft.h"

void InvalidPage::ondemmand(KMemory* memory, U32 page) {
    KMemoryData* mem = getMemData(memory);
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
    if (mem->getPage(page) != this) {
        return;
    }
    getMemData(memory)->setPageRam(nullptr, page, false);
}

U8 InvalidPage::readb(U32 address) {
    KThread* thread = KThread::currentThread();
    KMemoryData* data = getMemData(thread->memory);
    U32 page = address >> K_PAGE_SHIFT;
    if (thread->memory->canRead(page)) {
        ondemmand(thread->memory, page);
        return data->memory->readb(address);
    }
    thread->seg_mapper(address, true, false);
    return 0;
}

void InvalidPage::writeb(U32 address, U8 value) {
    KThread* thread = KThread::currentThread();
    KMemoryData* data = getMemData(thread->memory);
    U32 page = address >> K_PAGE_SHIFT;
    if (thread->memory->canWrite(page)) {
        ondemmand(thread->memory, page);
        data->memory->writeb(address, value);
    } else {
        thread->seg_mapper(address, false, true);
    }
}

U16 InvalidPage::readw(U32 address) {
    KThread* thread = KThread::currentThread();
    KMemoryData* data = getMemData(thread->memory);
    U32 page = address >> K_PAGE_SHIFT;
    if (thread->memory->canRead(page)) {
        ondemmand(thread->memory, page);
        return data->memory->readw(address);
    }
    thread->seg_mapper(address, true, false);
    return 0;
}

void InvalidPage::writew(U32 address, U16 value) {
    KThread* thread = KThread::currentThread();
    KMemoryData* data = getMemData(thread->memory);
    U32 page = address >> K_PAGE_SHIFT;
    if (thread->memory->canWrite(page)) {
        ondemmand(thread->memory, page);
        data->memory->writew(address, value);
    } else {
        thread->seg_mapper(address, false, true);
    }
}

U32 InvalidPage::readd(U32 address) {
    KThread* thread = KThread::currentThread();
    KMemoryData* data = getMemData(thread->memory);
    U32 page = address >> K_PAGE_SHIFT;
    if (thread->memory->canRead(page)) {
        ondemmand(thread->memory, page);
        return data->memory->readd(address);
    }
    thread->seg_mapper(address, true, false);
    return 0;
}

void InvalidPage::writed(U32 address, U32 value) {
    KThread* thread = KThread::currentThread();
    KMemoryData* data = getMemData(thread->memory);
    U32 page = address >> K_PAGE_SHIFT;
    if (thread->memory->canWrite(page)) {
        ondemmand(thread->memory, page);
        data->memory->writed(address, value);
    } else {
        thread->seg_mapper(address, false, true);
    }
}

U8* InvalidPage::getReadPtr(KMemory* memory, U32 address, bool makeReady) {
    KMemoryData* data = getMemData(memory);
    U32 page = address >> K_PAGE_SHIFT;
    if (makeReady && memory->canRead(page)) {
        ondemmand(memory, page);
        return data->getPage(page)->getReadPtr(memory, address, true);
    }
    return nullptr;
}

U8* InvalidPage::getWritePtr(KMemory* memory, U32 address, U32 len, bool makeReady) {
    U32 page = address >> K_PAGE_SHIFT;
    if (makeReady && memory->canWrite(page)) {
        KMemoryData* data = getMemData(memory);
        ondemmand(memory, page);
        return data->getPage(page)->getWritePtr(memory, address, len, true);
    }
    return nullptr;
}
