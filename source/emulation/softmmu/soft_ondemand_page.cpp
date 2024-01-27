#include "boxedwine.h"
#include "../../util/ptrpool.h"

#ifdef BOXEDWINE_DEFAULT_MMU

#include "soft_ondemand_page.h"
#include "soft_ro_page.h"
#include "soft_no_page.h"
#include "soft_rw_page.h"
#include "soft_invalid_page.h"
#include "soft_wo_page.h"
#include "soft_ram.h"
#include "kmemory_soft.h"

static PtrPool<OnDemandPage> freePages;

OnDemandPage* OnDemandPage::alloc(U32 flags) {
    OnDemandPage* page = freePages.get();
    page->flags = flags;
    return page;
}

// called by PtrPool
void OnDemandPage::reset() {
}

void OnDemandPage::close() {
    flags = 0;
    freePages.put(this);
}

void OnDemandPage::ondemmand(U32 address) {
    U32 page = address >> K_PAGE_SHIFT;
    KMemory* memory = KThread::currentThread()->memory;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
    getMemData(memory)->setPageRamWithFlags(nullptr, page, flags, false);
}

U8 OnDemandPage::readb(U32 address) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    ondemmand(address);
    return data->memory->readb(address);
}

void OnDemandPage::writeb( U32 address, U8 value) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    ondemmand(address);
    data->memory->writeb(address, value);
}

U16 OnDemandPage::readw(U32 address) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    ondemmand(address);
    return data->memory->readw(address);
}

void OnDemandPage::writew(U32 address, U16 value) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    ondemmand(address);
    data->memory->writew(address, value);
}

U32 OnDemandPage::readd(U32 address) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    ondemmand(address);
    return data->memory->readd(address);
}

void OnDemandPage::writed(U32 address, U32 value) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    ondemmand(address);
    data->memory->writed(address, value);
}

U8* OnDemandPage::getReadPtr(U32 address, bool makeReady) {
    if (makeReady && canRead()) {
        KMemoryData* data = getMemData(KThread::currentThread()->memory);
        ondemmand(address);
        return data->getPage(address >> K_PAGE_SHIFT)->getReadPtr(address, true);
    }
    return NULL;
}

U8* OnDemandPage::getWritePtr(U32 address, U32 len, bool makeReady) {
    if (makeReady && canWrite()) {
        KMemoryData* data = getMemData(KThread::currentThread()->memory);
        ondemmand(address);
        return data->getPage(address >> K_PAGE_SHIFT)->getWritePtr(address, len, true);
    }
    return NULL;
}

#endif