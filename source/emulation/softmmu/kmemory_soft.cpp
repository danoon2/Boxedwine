#include "boxedwine.h"

#include "soft_invalid_page.h"
#include "kmemory_soft.h"
#include "soft_rw_page.h"
#include "soft_copy_on_write_page.h"
#include "soft_file_map.h"
#include "soft_code_page.h"
#include "devfb.h"

#include "soft_ram.h"

static RamPage callbackRam;
static U32 callbackRamPos;
MemInfo MemInfo::empty;
Page* KMemoryData::pageMap[8];

void MemInfo::updatePermissionCache() {
    if (type < HAS_RAM_PAGE_INDEX) {
        read = 0;
        write = 0;
        return;
    }
    if (flags & PAGE_READ) {
        read = 1;
    } else {
        read = 0;
    }
    if ((flags & PAGE_WRITE) && type == (U32)PageType::RAM_Page) {
        write = 1;
    } else {
        write = 0;
    }
}

void KMemoryData::shutdown() {
    ramPageRelease(callbackRam);
    callbackRam = 0;
    callbackRamPos = 0;
}

KMemoryData* getMemData(KMemory* memory) {
    return memory->data;
}

#ifdef BOXEDWINE_BINARY_TRANSLATOR
KMemoryData::KMemoryData(KMemory* memory) : BtMemory(memory), memory(memory)
#else
KMemoryData::KMemoryData(KMemory* memory) : memory(memory)
#endif
{
    if (!pageMap[0]) {
        // keep in sync with pageType.h
        pageMap[0] = new InvalidPage();
        pageMap[1] = new FilePage();
        pageMap[2] = new RWPage();
        pageMap[3] = new CodePage();
        pageMap[4] = new CopyOnWritePage();
    }
    memset(memInfo, 0, sizeof(memInfo));
    if(!callbackRam) {
        callbackRam = ramPageAlloc();
        addCallback(onExitSignal);
    }
    this->allocPages(nullptr, CALL_BACK_ADDRESS >> K_PAGE_SHIFT, 1, K_PROT_READ | K_PROT_EXEC, nullptr, &callbackRam);
}

KMemoryData::~KMemoryData() {
    for (int i = 0; i < K_NUMBER_OF_PAGES; i++) {
        releaseRam(i);
    }
}

void KMemoryData::releaseRam(U32 page) {
#ifdef _DEBUG
    if (page >= K_NUMBER_OF_PAGES) {
        kpanic("KMemoryData::releaseRam page out of bound");
    }
#endif
    if (hasRamPage(page)) {
        ramPageRelease(memInfo[page].ramPageIndex);
        memory->removeCodeBlock(page << K_PAGE_SHIFT, K_PAGE_SIZE);
    }
}

bool KMemoryData::hasRamPage(U32 page) {
#ifdef _DEBUG
    if (page >= K_NUMBER_OF_PAGES) {
        kpanic("KMemoryData::hasRamPage page out of bound");
    }
#endif
    return memInfo[page].type >= HAS_RAM_PAGE_INDEX;
}

void KMemoryData::setPageType(U32 page, PageType pageType, RamPage ramIndex, bool releasePreviousRam) {
#ifdef _DEBUG
    if (page >= K_NUMBER_OF_PAGES) {
        kpanic("KMemoryData::setPageType page out of bound");
    }
#endif
    if (releasePreviousRam) {
        releaseRam(page);
    }
    memInfo[page].type = (U32)pageType;
    memInfo[page].ramPageIndex = ramIndex;
    memInfo[page].updatePermissionCache();
    if (ramIndex) {
        ramPageRetain(ramIndex);
    }
}

void KMemoryData::addCallback(OpCallback func) {
    U64 funcAddress = (U64)func;
    U8* address = ramPageGet(callbackRam) + callbackRamPos;

    *address = 0xFE;
    address++;
    *address = 0x38;
    address++;
    *address = (U8)funcAddress;
    address++;
    *address = (U8)(funcAddress >> 8);
    address++;
    *address = (U8)(funcAddress >> 16);
    address++;
    *address = (U8)(funcAddress >> 24);
    callbackRamPos += 6;
    if (sizeof(func) == 8) {
        address++;
        *address = (U8)(funcAddress >> 32);
        address++;
        *address = (U8)(funcAddress >> 40);
        address++;
        *address = (U8)(funcAddress >> 48);
        address++;
        *address = (U8)(funcAddress >> 56);
        callbackRamPos += 4;
    }
}

void KMemoryData::allocPages(KThread* thread, U32 page, U32 pageCount, U8 permissions, const MappedFilePtr& mappedFile, const RamPage* ramPages) {
#ifdef _DEBUG
    if (page + pageCount >= K_NUMBER_OF_PAGES) {
        kpanic("KMemoryData::allocPages page out of bound");
    }
    if (mappedFile && ramPages) {
        kpanic("KMemoryData::allocPages mapped files should not contain ramPages");
    }
#endif
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
    for (U32 i = 0; i < pageCount; i++) {
        memInfo[page + i].flags = permissions;
        if (mappedFile) {
            setPageType(page + i, PageType::File_Page, mappedFile->key, true);
        } else if (ramPages) {
            setPageType(page + i, PageType::RAM_Page, ramPages[i], true);            
        } else {
            setPageType(page + i, PageType::Invalid_Page, 0, true);
        }
    }
}

bool isAlignedNativePage(U32 page) {
    U32 gran = Platform::getPageAllocationGranularity();
    return (page & ~(gran - 1)) == page;
}

// memory mutex should be held when calling this
bool KMemoryData::reserveAddress(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped, bool alignNative, U32 reservedFlag) {
    U32 i;

    for (i = startingPage; i < K_NUMBER_OF_PAGES; i++) {
        if (alignNative && !isAlignedNativePage(i)) {
            continue;
        }
        if (i + pageCount >= K_NUMBER_OF_PAGES) {
            return false;
        }
        U32 flags = memInfo[i].flags;

        if (flags == 0 || (canBeReMapped && (flags & PAGE_MAPPED))) {
            U32 j = 1;
            bool success = true;

            for (; j < pageCount; j++) {
                U32 nextPage = i + j; // could be done a different way, but this helps the static analysis
                U32 nextFlags = memInfo[nextPage].flags;

                if (nextPage < K_NUMBER_OF_PAGES && nextFlags != 0 && (!canBeReMapped || !(nextFlags & PAGE_MAPPED))) {
                    success = false;
                    break;
                }
            }
            if (success && startingPage < ADDRESS_PROCESS_MMAP_START && i >= ADDRESS_PROCESS_MMAP_START) {
                break; // don't allow user app to allocate in space we reserve for kernel space
            }
            if (success) {                
                *result = i;
                U32 pageEndIndex = i + pageCount;
                for (U32 pageIndex = i; pageIndex < pageEndIndex && pageIndex < K_NUMBER_OF_PAGES; pageIndex++) {
                    memInfo[pageIndex].flags = reservedFlag;
                }
                return true;
            }
            i += j; // no reason to check all the pages again
        }
    }
    return false;
}

void KMemoryData::protectPage(KThread* thread, U32 page, U32 permissions) {
#ifdef _DEBUG
    if (page >= K_NUMBER_OF_PAGES) {
        kpanic("KMemoryData::protectPage page out of bound");
    }
#endif
    U32 newFlags = memInfo[page].flags;

    newFlags &= ~PAGE_PERMISSION_MASK;
    newFlags |= (permissions & PAGE_PERMISSION_MASK);
    memInfo[page].flags = newFlags;
    memInfo[page].updatePermissionCache();    
}

bool KMemoryData::isPageAllocated(U32 page) {
#ifdef _DEBUG
    if (page >= K_NUMBER_OF_PAGES) {
        kpanic("KMemoryData::isPageAllocated page out of bound");
    }
#endif
    return memInfo[page].flags != 0;
}

void KMemoryData::setPagesInvalid(U32 page, U32 pageCount) {
    for (U32 i = page; i < page + pageCount; i++) {
        releaseRam(i);
        memInfo[i].type = (U32)PageType::Invalid_Page;
        memInfo[i].ramPageIndex = 0;
        memInfo[i].flags = 0;
        memInfo[i].updatePermissionCache();
    }
}

void KMemoryData::execvReset() {
    setPagesInvalid(0, K_NUMBER_OF_PAGES);    
    memory->code.removeAll();
    this->allocPages(KThread::currentThread(), CALL_BACK_ADDRESS >> K_PAGE_SHIFT, 1, K_PROT_READ | K_PROT_EXEC, nullptr, &callbackRam);
}

U64 KMemory::readq(U32 address) {
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
    if ((address & 0xFFF) < 0xFF9) {
        int index = address >> 12;
        if (data->memInfo[index].read) {
            return *(U64*)(&ramPageGet(data->memInfo[index].ramPageIndex)[address & 0xfff]);
        }
    }
#endif
    return readd(address) | ((U64)readd(address + 4) << 32);
}

U32 KMemory::readd(U32 address) {
    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
        if (data->memInfo[index].read) {
            return *(U32*)(&ramPageGet(data->memInfo[index].ramPageIndex)[address & 0xfff]);
        }
#endif
        return data->getPage(index)->readd(data->memInfo[index], address);
    } else {
        return readb(address) | (readb(address + 1) << 8) | (readb(address + 2) << 16) | (readb(address + 3) << 24);
    }
}

U16 KMemory::readw(U32 address) {
    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
        if (data->memInfo[index].read) {
            return *(U16*)(&ramPageGet(data->memInfo[index].ramPageIndex)[address & 0xfff]);
        }
#endif
        return data->getPage(index)->readw(data->memInfo[index], address);
    }
    return readb(address) | (readb(address + 1) << 8);
}

U8 KMemory::readb(U32 address) {
    if (address == 0x104F5563) {
        int ii = 0;
    }
    int index = address >> 12;
#if !defined(BOXEDWINE_BINARY_TRANSLATOR)
    if (data->memInfo[index].read) {
        return ramPageGet(data->memInfo[index].ramPageIndex)[address & 0xfff];
    }
#endif
    return data->getPage(index)->readb(data->memInfo[index], address);
}

void KMemory::writeq(U32 address, U64 value) {
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
    if ((address & 0xFFF) < 0xFF9) {
        int index = address >> 12;
        if (data->memInfo[index].write) {
            *(U64*)(&ramPageGet(data->memInfo[index].ramPageIndex)[address & 0xfff]) = value;
            return;
        }
    }
#endif
    writed(address, (U32)value); writed(address + 4, (U32)(value >> 32));
}

void KMemory::writed(U32 address, U32 value) {
    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
        if (data->memInfo[index].write)
            *(U32*)(&ramPageGet(data->memInfo[index].ramPageIndex)[address & 0xfff]) = value;
        else
#endif
            data->getPage(index)->writed(data->memInfo[index], address, value);
    } else {
        writeb(address, value);
        writeb(address + 1, value >> 8);
        writeb(address + 2, value >> 16);
        writeb(address + 3, value >> 24);
    }
}

void KMemory::writew(U32 address, U16 value) {
    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
        if (data->memInfo[index].write)
            *(U16*)(&ramPageGet(data->memInfo[index].ramPageIndex)[address & 0xfff]) = value;
        else
#endif
            data->getPage(index)->writew(data->memInfo[index], address, value);
    } else {
        writeb(address, (U8)value);
        writeb(address + 1, (U8)(value >> 8));
    }
}

void KMemory::writeb(U32 address, U8 value) {
    int index = address >> 12;
#if !defined(BOXEDWINE_BINARY_TRANSLATOR)
    if (data->memInfo[index].write)
        ramPageGet(data->memInfo[index].ramPageIndex)[address & 0xfff] = value;
    else
#endif
        data->getPage(index)->writeb(data->memInfo[index], address, value);
}

// used by futex, may point to shared memory
U8* KMemory::getIntPtr(U32 address, bool write) {
    U32 index = address >> K_PAGE_SHIFT;
    U32 offset = address & K_PAGE_MASK;
    U8* result;

    if (write) {
        result = data->getPage(index)->getWritePtr(this, data->memInfo[index], address, 4, true);
    } else {
        result = data->getPage(index)->getReadPtr(this, data->memInfo[index], address, true);
    }
    if (result) {
        result += offset;
    }
    return result;
}

U8* KMemory::getPtrForFutex(U32 address) {
    U32 index = address >> K_PAGE_SHIFT;
    U32 offset = address & K_PAGE_MASK;
    // if this page isn't shared, then when we clone, we might make it copy on write which can result it getting a new ram address
    // we want to prevent that if a futux is using that page
    data->memInfo[index].flags |= PAGE_FUTEX;
    U8* result = getIntPtr(address, true);
    if (result) {
        result += offset;
    }
    return result;
}

void KMemory::clone(KMemory* from, bool vfork) {
    // don't allow changes to the from pages while we are cloning
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(from->mutex);
    if (vfork) {
        delete this->data;
        this->data = from->data;
        return;
    }
    ::memcpy(data->memInfo, from->data->memInfo, sizeof(data->memInfo));

    for (int i = 0; i < 0x100000; i++) {
        bool isMappedFile = from->data->memInfo[i].type == (U32)PageType::File_Page;
        bool isUnallocatedRam = from->data->memInfo[i].type == (U32)PageType::Invalid_Page && from->data->memInfo[i].flags;
        if (mapShared(i) && (isMappedFile || isUnallocatedRam)) {
            from->data->getPage(i)->onDemand(this, data->memInfo[i], i);
            data->memInfo[i] = from->data->memInfo[i]; // since from changed because of onDemand
        }
        
        if (from->data->memInfo[i].type >= HAS_RAM_PAGE_INDEX) {
            ramPageRetain(data->memInfo[i].ramPageIndex);

            // CodePage will check copy on write
            if (from->data->memInfo[i].type != (U32)PageType::Code_Page) {
                data->memInfo[i].type = (U32)PageType::Copy_On_Write_Page;
                from->data->memInfo[i].type = (U32)PageType::Copy_On_Write_Page;
            }
            data->memInfo[i].write = 0;            
            from->data->memInfo[i].write = 0;
        }
    }
}

CodeBlock KMemory::findCodeBlockContaining(U32 address, U32 len) {
    U32 page = address >> K_PAGE_SHIFT;
    if (data->memInfo[page].type == (U32)PageType::Code_Page) {
        return code.getBlock(address, len);
    }
    return nullptr;
}

CodeBlock KMemory::getCodeBlock(U32 address) {
    U32 page = address >> K_PAGE_SHIFT;
    if (data->memInfo[page].type == (U32)PageType::Code_Page) {
        return code.getBlock(address);
    }
    return nullptr;
}

void KMemory::removeCodeBlock(U32 address, U32 len, bool becauseOfWrite) {    
    iterateAddressByPage(address, len, [this, becauseOfWrite](U32 address, U32 len) {
        U32 page = address >> K_PAGE_SHIFT;
        if (data->memInfo[page].type == (U32)PageType::Code_Page) {
            this->code.removeCode(this, address, len, true);
        }
    });
}

void KMemory::addCodeBlock(U32 address, CodeBlockParam block) {
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
        iterateAddressByPage(address, block->getEipLen(), [block, this](U32 address, U32 len) {
            U32 page = address >> K_PAGE_SHIFT;
            if (data->memInfo[page].type != (U32)PageType::Code_Page) {
                this->getIntPtr(address, true); // if this is a copy on write page, this will make a copy
                data->memInfo[page].type = (U32)PageType::Code_Page;
                data->memInfo[page].updatePermissionCache();
            }
            this->code.addCodeToPage(block, page, address, len);
        });
    }
}

void KMemoryData::markAddressDynamic(U32 address, U32 len) {
    memory->iterateAddressByPage(address, len, [this](U32 address, U32 len) {
        U32 page = address >> K_PAGE_SHIFT;
        if (memInfo[page].type == (U32)PageType::Code_Page) {
            CodePerPageData* codePage = this->memory->code.getCodePage(page, false);
            if (codePage) {
                codePage->markAddressDynamic(address, len);
            }
        }
    });
}

bool KMemoryData::isAddressDynamic(U32 address, U32 len) {    
    bool result = false;

    memory->iterateAddressByPage(address, len, [this, &result](U32 address, U32 len) {
        U32 page = address >> K_PAGE_SHIFT;
        if (memInfo[page].type == (U32)PageType::Code_Page) {
            CodePerPageData* codePage = this->memory->code.getCodePage(page, false);
            if (codePage) {
                result |= codePage->isAddressDynamic(address, len);
            }
        }
    });
    return result;
}

CodePage* KMemoryData::getOrCreateCodePage(U32 address) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
    U32 page = address >> K_PAGE_SHIFT;

    if (memInfo[page].type == (U32)PageType::File_Page) {
        // code probably linked to a block that didn't exist and we created a place holder instruction there to re-translate (see callRetranslateChunk)
        getPage(page)->onDemand(memory, memInfo[page], address);
    }
    if (memInfo[page].type == (U32)PageType::RAM_Page) {
        memInfo[page].type = (U32)PageType::Code_Page;
    }
    if (memInfo[page].type == (U32)PageType::Code_Page) {
        return (CodePage*)getPage(page);
    } else {
        U32 type = memInfo[page].type;
        kpanic("Unhandled code caching page type: %d", type);
        return nullptr;
    }
}

void KMemory::logPageFault(KThread* thread, U32 address) {
    U32 start = 0;
    CPU* cpu = thread->cpu;

    BString name = process->getModuleName(cpu->seg[CS].address + cpu->eip.u32);
    klog("%.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X %s at %.8X", cpu->seg[CS].address + cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, name.c_str(), process->getModuleEip(cpu->seg[CS].address + cpu->eip.u32));

    klog("Page Fault at %.8X", address);
    klog("Valid address ranges:");
    for (U32 i = 0; i < K_NUMBER_OF_PAGES; i++) {
        if (!start) {
            if (data->memInfo[i].flags) {
                start = i;
            }
        } else if (!data->memInfo[i].flags) {
            klog("    %.8X - %.8X", start * K_PAGE_SIZE, i * K_PAGE_SIZE);
            start = 0;
        }
    }
    klog("Mapped Files:");
    process->printMappedFiles();
    cpu->walkStack(cpu->eip.u32, EBP, 2);
    kpanic("pf");
}

void KMemory::performOnMemory(U32 address, U32 len, bool readOnly, std::function<bool(U8* ram, U32 len)> callback) {
    if (!len) {
        return;
    }
    U32 pageIndex = address >> K_PAGE_SHIFT;
    Page* page = data->getPage( pageIndex);
    U32 offset = address & K_PAGE_MASK;
    U32 todo = len;
    if (todo > K_PAGE_SIZE - offset) {
        todo = K_PAGE_SIZE - offset;
    }

    U8* ram = readOnly ? page->getReadPtr(this, data->memInfo[pageIndex], address, true) : page->getWritePtr(this, data->memInfo[pageIndex], address, todo, true);
    if (!ram) {
        int ii = 0;
    }
    if (!callback(ram+offset, todo)) {
        return;
    }
    address += todo;
    len -= todo;

    while (len > K_PAGE_SIZE) {
        pageIndex++;
        page = data->getPage(pageIndex);
        ram = readOnly ? page->getReadPtr(this, data->memInfo[pageIndex], address, true) : page->getWritePtr(this, data->memInfo[pageIndex], address, K_PAGE_SIZE, true);
        if (!ram) {
            int ii = 0;
        }
        if (!callback(ram, K_PAGE_SIZE)) {
            return;
        }
        address += K_PAGE_SIZE;
        len -= K_PAGE_SIZE;
    }

    if (len > 0) {
        pageIndex++;
        page = data->getPage(pageIndex);
        ram = readOnly ? page->getReadPtr(this, data->memInfo[pageIndex], address, true) : page->getWritePtr(this, data->memInfo[pageIndex], address, len, true);
        if (!ram) {
            int ii = 0;
        }
        callback(ram, len);
    }
}

// This doesn't mean the data can't be changed by another thread, it just means the pointer will stay valid
U8* KMemory::lockReadOnlyMemory(U32 address, U32 len) {
    U32 pageIndex = address >> K_PAGE_SHIFT;
    Page* page = data->getPage(pageIndex);
    U32 offset = address & K_PAGE_MASK;

    // if we cross a page boundry then we will need to make a copy
    if (len <= K_PAGE_SIZE - offset) {
        return page->getReadPtr(this, data->memInfo[pageIndex], address, true);
    }
    std::shared_ptr<U8[]> p = std::make_shared<U8[]>(len);
    memcpy(p.get(), address, len);
    lockedMemory.set(p.get(), p);    
    return p.get();
}

void KMemory::unlockMemory(U8 * lockedPointer) {
    lockedMemory.remove(lockedPointer);
}

void KMemory::unmapNativeMemory(U32 address, U32 size) {
    unmap(address - K_PAGE_SIZE, size + 2 * K_PAGE_SIZE);
}

U32 KMemory::mapNativeMemory(void* hostAddress, U32 size) {
    U32 result = 0;
    U32 pageCount = (size + K_PAGE_SIZE - 1) >> K_PAGE_SHIFT;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    if (!data->reserveAddress(ADDRESS_PROCESS_MMAP_START, pageCount+2, &result, false, true, PAGE_MAPPED)) {
        return 0;
    }
    result++;
    U8* address = (U8*)hostAddress;
    for (U32 i = 0; i < pageCount; i++) {
        data->memInfo[result + i].flags = PAGE_MAPPED | PAGE_READ | PAGE_WRITE;
        data->memInfo[result + i].type = (U32)PageType::RAM_Page;
        data->memInfo[result + i].ramPageIndex = ramPageAllocNative(address);
        address += K_PAGE_SIZE;
    }
    return result << K_PAGE_SHIFT;
}
