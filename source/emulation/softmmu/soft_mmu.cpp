#include "boxedwine.h"
#include "soft_mmu.h"
#include "soft_invalid_page.h"
#include "soft_code_page.h"
#include "soft_file_map.h"
#include "soft_copy_on_write_page.h"
#include "kmemory_soft.h"

void MMU::setPageType(KMemoryData* mem, U32 page, PageType type) {
    setPage(mem, page, type, (RamPage)ramIndex);
}

void MMU::setPage(KMemoryData* mem, U32 page, PageType type, RamPage ram) {
    if (getPageType() == PageType::Code && type != PageType::Code) {
        mem->codeCache.removeBlockAt(page << K_PAGE_SHIFT, K_PAGE_SIZE);
    }
    if (type == PageType::None) {
        if (ramIndex && getPageType() != PageType::File) {
            ramPageRelease((RamPage)ramIndex);
        }
        ramIndex = 0;
        flags = 0;
        this->type = 0;
        canReadRam = 0;
        canWriteRam = 0;
        return;
    }
    // I have seen ramIndex == ram.value but it was moving from PageType::File to PageType::CopyOnWrite
    // so the ram index happen to equal the file key
    if (ramIndex != ram.value || type == PageType::File || getPageType() == PageType::File) {
        if (ramIndex && getPageType() != PageType::File) {
            ramPageRelease((RamPage)ramIndex);
        }
        if (ram.value && type != PageType::File) {
            ramPageRetain(ram);
        }
    }
    ramIndex = ram.value;
    this->type = (U8)type;
    onPageChanged();
}

void MMU::setPermissions(U32 permissions) {
#ifdef _DEBUG
    if (permissions & ~PAGE_PERMISSION_MASK) {
        kpanic("MMU::setPermissions oops");
    }
#endif
    this->flags &= ~PAGE_PERMISSION_MASK;
    this->flags |= (permissions & PAGE_PERMISSION_MASK);    
}

void MMU::setFlags(U32 flags) {
    this->flags = flags;
}

void MMU::onPageChanged() {
    Page* page = getPage();
    canReadRam = page->canReadRam(this) && (flags & PAGE_READ) ? 1 : 0;
    canWriteRam = page->canWriteRam(this) && (flags & PAGE_WRITE) ? 1 : 0;
}

static InvalidPage pageInvalid;
static RWPage rwPage;
static FilePage filePage;
static CopyOnWritePage copyOnWritePage;
static CodePage codePage;

Page* Page::getRWPage() {
    return &rwPage;
}

Page* Page::getPage(PageType type) {
    switch (type) {
    case PageType::None:
        return &pageInvalid;
    case PageType::Ram:
        return &rwPage;
    case PageType::Code:
        return &codePage;
    case PageType::File:
        return &filePage;
    case PageType::CopyOnWrite:
        return &copyOnWritePage;
    default:
        kpanic("age::getPage unknown type: %d", (U32)type);
        return nullptr;
    }
}