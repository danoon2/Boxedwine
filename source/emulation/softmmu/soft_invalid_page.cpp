#include "boxedwine.h"

#include "soft_invalid_page.h"
#include "kmemory_soft.h"

void InvalidPage::onDemand(KMemory* memory, MemInfo& info, U32 address) {
    KMemoryData* mem = getMemData(memory);
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
    if (info.type != (U32)PageType::Invalid_Page) {
        return;
    }
    info.type = (U32)PageType::RAM_Page;
    info.ramPageIndex = ramPageAlloc();
    info.updatePermissionCache();
}

U8 InvalidPage::readb(MemInfo& info, U32 address) {
    KThread* thread = KThread::currentThread();

    if (info.flags & PAGE_READ) {
        onDemand(thread->memory, info, address);
        return thread->memory->readb(address);
    }
    if (info.flags) {
        thread->seg_access(address, true, false);
    } else {
        thread->seg_mapper(address, true, false);
    }
    return 0;
}

void InvalidPage::writeb(MemInfo& info, U32 address, U8 value) {
    KThread* thread = KThread::currentThread();

    if (info.flags & PAGE_WRITE) {
        onDemand(thread->memory, info, address);
        thread->memory->writeb(address, value);
    } else if (info.flags) {
        thread->seg_access(address, true, false);
    } else {
        thread->seg_mapper(address, true, false);
    }
}

U16 InvalidPage::readw(MemInfo& info, U32 address) {
    KThread* thread = KThread::currentThread();

    if (info.flags & PAGE_READ) {
        onDemand(thread->memory, info, address);
        return thread->memory->readw(address);
    }
    if (info.flags) {
        thread->seg_access(address, true, false);
    } else {
        thread->seg_mapper(address, true, false);
    }
    return 0;
}

void InvalidPage::writew(MemInfo& info, U32 address, U16 value) {
    KThread* thread = KThread::currentThread();

    if (info.flags & PAGE_WRITE) {
        onDemand(thread->memory, info, address);
        thread->memory->writew(address, value);
    } else if (info.flags) {
        thread->seg_access(address, true, false);
    } else {
        thread->seg_mapper(address, true, false);
    }
}

U32 InvalidPage::readd(MemInfo& info, U32 address) {
    KThread* thread = KThread::currentThread();

    if (info.flags & PAGE_READ) {
        onDemand(thread->memory, info, address);
        return thread->memory->readd(address);
    }
    if (info.flags) {
        thread->seg_access(address, true, false);
    } else {
        thread->seg_mapper(address, true, false);
    }
    return 0;
}

void InvalidPage::writed(MemInfo& info, U32 address, U32 value) {
    KThread* thread = KThread::currentThread();

    if (info.flags & PAGE_WRITE) {
        onDemand(thread->memory, info, address);
        thread->memory->writed(address, value);
    } else if (info.flags) {
        thread->seg_access(address, true, false);
    } else {
        thread->seg_mapper(address, true, false);
    }
}

U8* InvalidPage::getReadPtr(KMemory* memory, MemInfo& info, U32 address, bool makeReady) {
    if (makeReady && (info.flags & PAGE_READ)) {
        U32 page = address >> K_PAGE_SHIFT;
        KMemoryData* data = getMemData(memory);
        onDemand(memory, info, address);
        return data->getPage(page)->getReadPtr(memory, info, address, true);
    }
    return nullptr;
}

U8* InvalidPage::getWritePtr(KMemory* memory, MemInfo& info, U32 address, U32 len, bool makeReady) {    
    if (makeReady && (info.flags & PAGE_WRITE)) {
        U32 page = address >> K_PAGE_SHIFT;
        KMemoryData* data = getMemData(memory);
        onDemand(memory, info, address);
        return data->getPage(page)->getWritePtr(memory, info, address, len, true);
    }
    return nullptr;
}
