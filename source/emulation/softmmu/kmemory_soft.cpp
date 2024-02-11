#include "boxedwine.h"

#include "soft_invalid_page.h"
#include "kmemory_soft.h"
#include "soft_native_page.h"
#include "soft_ro_page.h"
#include "soft_rw_page.h"
#include "soft_wo_page.h"
#include "soft_no_page.h"
#include "soft_copy_on_write_page.h"
#include "soft_file_map.h"
#include "soft_code_page.h"

#include "soft_ram.h"

static InvalidPage _invalidPage;
static InvalidPage* invalidPage = &_invalidPage;
static U8* callbackRam;
static U32 callbackRamPos;

KMemoryData* getMemData(KMemory* memory) {
    return memory->data;
}

#ifdef BOXEDWINE_BINARY_TRANSLATOR
KMemoryData::KMemoryData(KMemory* memory) : BtMemory(memory), memory(memory), mmuReadPtr{ 0 }, mmuWritePtr{ 0 }, mmuReadPtrAdjusted{ 0 }, mmuWritePtrAdjusted{ 0 }, delayedReset(nullptr)
#else
KMemoryData::KMemoryData(KMemory* memory) : memory(memory), mmuReadPtr{ 0 }, mmuWritePtr{ 0 }
#endif
{
    for (int i = 0; i < K_NUMBER_OF_PAGES; i++) {
        this->mmu[i] = invalidPage;
    }
    if(!callbackRam) {
        callbackRam = ramPageAlloc();
        addCallback(onExitSignal);
    }
    this->allocPages(nullptr, CALL_BACK_ADDRESS >> K_PAGE_SHIFT, 1, K_PROT_READ | K_PROT_EXEC, -1, 0, nullptr, &callbackRam);
}

KMemoryData::~KMemoryData() {
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    if (delayedReset) {
        delete delayedReset;
    }
#endif
    for (int i = 0; i < K_NUMBER_OF_PAGES; i++) {
        mmu[i]->close();
    }
}

void KMemoryData::setPage(U32 index, Page* page) {
    Page* p = this->mmu[index];
    this->mmu[index] = page;
    U32 address = index << K_PAGE_SHIFT;
    this->mmuReadPtr[index] = page->getReadPtr(memory, address);
    this->mmuWritePtr[index] = page->getWritePtr(memory, address, K_PAGE_SIZE);
    if (p != page) {
        p->close();
    }
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    U8* readPtr = page->getReadPtr(memory, address);
    if (readPtr) {
        if (readPtr - (index << K_PAGE_SHIFT) == nullptr) {
            int ii = 0;
        }
        this->mmuReadPtrAdjusted[index] = readPtr - (index << K_PAGE_SHIFT);
    } else {
        this->mmuReadPtrAdjusted[index] = nullptr;
    }
    U8* writePtr = page->getWritePtr(memory, address, K_PAGE_SHIFT);
    if (writePtr) {
        if (writePtr - (index << K_PAGE_SHIFT) == nullptr) {
            int ii = 0;
        }
        this->mmuWritePtrAdjusted[index] = writePtr - (index << K_PAGE_SHIFT);
    } else {
        this->mmuWritePtrAdjusted[index] = nullptr;
    }
#endif
}

void KMemoryData::addCallback(OpCallback func) {
    U64 funcAddress = (U64)func;
    U8* address = callbackRam + callbackRamPos;

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

void KMemoryData::setPageRam(U8* ram, U32 pageIndex, bool copyOnWrite) {
    bool read = memory->canRead(pageIndex) || memory->canExec(pageIndex);
    bool write = memory->canWrite(pageIndex);
    
    if (copyOnWrite && !memory->mapShared(pageIndex)) {
        setPage(pageIndex, CopyOnWritePage::alloc(ram, pageIndex << K_PAGE_SHIFT));
    } else if (read && write) {
        setPage(pageIndex, RWPage::alloc(ram, pageIndex << K_PAGE_SHIFT));
    } else if (write) {
        setPage(pageIndex, WOPage::alloc(ram, pageIndex << K_PAGE_SHIFT));
    } else if (read) {
        setPage(pageIndex, ROPage::alloc(ram, pageIndex << K_PAGE_SHIFT));
    } else {
        setPage(pageIndex, NOPage::alloc(ram, pageIndex << K_PAGE_SHIFT));
    }
}

void KMemoryData::allocPages(KThread* thread, U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const std::shared_ptr<MappedFile>& mappedFile, U8** ramPages) {
    for (U32 i = 0; i < pageCount; i++) {
        if (page + i == 0xd06cf) {
            int ii = 0;
        }
        memory->flags[page + i] = permissions;
    }
    if (ramPages) {
        bool read = permissions & K_PROT_READ;
        bool write = permissions & K_PROT_WRITE;

        if (read && write) {
            for (U32 i = 0; i < pageCount; i++) {
                this->setPage(page + i, RWPage::alloc(ramPages[i], page << K_PAGE_SHIFT));
            }
        } else if (write) {
            for (U32 i = 0; i < pageCount; i++) {
                this->setPage(page + i, WOPage::alloc(ramPages[i], page << K_PAGE_SHIFT));
            }
        } else if (read) {
            for (U32 i = 0; i < pageCount; i++) {
                this->setPage(page + i, ROPage::alloc(ramPages[i], page << K_PAGE_SHIFT));
            }
        } else {
            for (U32 i = 0; i < pageCount; i++) {
                this->setPage(page + i, NOPage::alloc(ramPages[i], page << K_PAGE_SHIFT));
            }
        }
    } else if (mappedFile) {
        U32 filePage = (U32)(offset >> K_PAGE_SHIFT);

        if (offset & K_PAGE_MASK) {
            kpanic("mmap: wasn't expecting the offset to be in the middle of a page");
        }

        for (U32 i = 0; i < pageCount; i++) {
            this->setPage(page + i, FilePage::alloc(mappedFile, filePage++));
        }
    } else {
        for (U32 i = 0; i < pageCount; i++) {
            this->setPage(page + i, invalidPage);
        }
    }
}

bool isAlignedNativePage(U32 page) {
    return (page & ~(K_NATIVE_PAGES_PER_PAGE - 1)) == page;
}

bool KMemoryData::reserveAddress(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped, bool alignNative, U32 reservedFlag) {
    U32 i;

    for (i = startingPage; i < K_NUMBER_OF_PAGES; i++) {
        if (alignNative && !isAlignedNativePage(i)) {
            continue;
        }
        if (i + pageCount >= K_NUMBER_OF_PAGES) {
            return false;
        }
        if (memory->flags[i] == 0 || (canBeReMapped && (memory->flags[i] & PAGE_MAPPED))) {
            U32 j = 1;
            bool success = true;

            for (; j < pageCount; j++) {
                U32 nextPage = i + j; // could be done a different way, but this helps the static analysis
                if (nextPage < K_NUMBER_OF_PAGES && memory->flags[nextPage] != 0 && (!canBeReMapped || !(memory->flags[nextPage] & PAGE_MAPPED))) {
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
                    memory->flags[pageIndex] = reservedFlag;
                }
                return true;
            }
            i += j; // no reason to check all the pages again
        }
    }
    return false;
}

void KMemoryData::protectPage(KThread* thread, U32 i, U32 permissions) {
    Page* page = this->getPage(i);

    U32 newFlags = memory->flags[i];
    newFlags &= ~PAGE_PERMISSION_MASK;
    newFlags |= (permissions & PAGE_PERMISSION_MASK);
    memory->flags[i] = newFlags;

    // we already have ram backing, so we need to preserve it and maybe change the page object
    if (page->getType() == Page::Type::RO_Page || page->getType() == Page::Type::RW_Page || page->getType() == Page::Type::WO_Page || page->getType() == Page::Type::NO_Page) {
        RWPage* p = (RWPage*)page;

        if ((permissions & PAGE_READ) && (permissions & PAGE_WRITE)) {
            if (page->getType() != Page::Type::RW_Page) {
                this->setPage(i, RWPage::alloc(p->page, p->address));
            }
        } else if (permissions & PAGE_WRITE) {
            if (page->getType() != Page::Type::WO_Page) {
                this->setPage(i, WOPage::alloc(p->page, p->address));
            }
        } else if (permissions & PAGE_READ) {
            if (page->getType() != Page::Type::RO_Page) {
                this->setPage(i, ROPage::alloc(p->page, p->address));
            }
        } else {
            if (page->getType() != Page::Type::NO_Page) {
                this->setPage(i, NOPage::alloc(p->page, p->address));
            }
        }
    }
}

bool KMemoryData::isPageAllocated(U32 page) {
    return memory->flags[page] != 0;
}

void KMemoryData::setPagesInvalid(U32 page, U32 pageCount) {
    for (U32 i = page; i < page + pageCount; i++) {
        this->setPage(i, invalidPage);
        memory->flags[i] = 0;
    }
}

#ifdef BOXEDWINE_BINARY_TRANSLATOR
void KMemoryData::clearDelayedReset() {
    if (delayedReset) {
        delete delayedReset;
        delayedReset = nullptr;
    }
}
#endif

void KMemoryData::execvReset() {
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    KMemoryData* newData = new KMemoryData(memory);
    newData->delayedReset = this;
    memory->data = newData;
    memset(memory->flags, 0, sizeof(memory->flags));
#else
    setPagesInvalid(0, K_NUMBER_OF_PAGES);    
#endif    
    this->allocPages(KThread::currentThread(), CALL_BACK_ADDRESS >> K_PAGE_SHIFT, 1, K_PROT_READ | K_PROT_EXEC, -1, 0, nullptr, &callbackRam);
}

U64 KMemory::readq(U32 address) {
#ifndef UNALIGNED_MEMORY
    if ((address & 0xFFF) < 0xFF9) {
        int index = address >> 12;
        if (data->mmuReadPtr[index]) {
            return *(U64*)(&data->mmuReadPtr[index][address & 0xFFF]);
        }
    }
#endif
    return readd(address) | ((U64)readd(address + 4) << 32);
}

U32 KMemory::readd(U32 address) {
    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
#ifndef UNALIGNED_MEMORY
        if (data->mmuReadPtr[index])
            return *(U32*)(&data->mmuReadPtr[index][address & 0xFFF]);
#endif
        return data->mmu[index]->readd(address);
    } else {
        return readb(address) | (readb(address + 1) << 8) | (readb(address + 2) << 16) | (readb(address + 3) << 24);
    }
}

U16 KMemory::readw(U32 address) {
    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
#ifndef UNALIGNED_MEMORY
        if (data->mmuReadPtr[index])
            return *(U16*)(&data->mmuReadPtr[index][address & 0xFFF]);
#endif
        return data->mmu[index]->readw(address);
    }
    return readb(address) | (readb(address + 1) << 8);
}

U8 KMemory::readb(U32 address) {
    int index = address >> 12;
    if (data->mmuReadPtr[index])
        return data->mmuReadPtr[index][address & 0xFFF];
    return data->mmu[index]->readb(address);
}

void KMemory::writeq(U32 address, U64 value) {
#ifndef UNALIGNED_MEMORY
    if ((address & 0xFFF) < 0xFF9) {
        int index = address >> 12;
        if (data->mmuWritePtr[index]) {
            *(U64*)(&data->mmuWritePtr[index][address & 0xFFF]) = value;
            return;
        }
    }
#endif
    writed(address, (U32)value); writed(address + 4, (U32)(value >> 32));
}

void KMemory::writed(U32 address, U32 value) {
    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
#ifndef UNALIGNED_MEMORY
        if (data->mmuWritePtr[index])
            *(U32*)(&data->mmuWritePtr[index][address & 0xFFF]) = value;
        else
#endif
            data->mmu[index]->writed(address, value);
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
#ifndef UNALIGNED_MEMORY
        if (data->mmuWritePtr[index])
            *(U16*)(&data->mmuWritePtr[index][address & 0xFFF]) = value;
        else
#endif
            data->mmu[index]->writew(address, value);
    } else {
        writeb(address, (U8)value);
        writeb(address + 1, (U8)(value >> 8));
    }
}

void KMemory::writeb(U32 address, U8 value) {
    int index = address >> 12;
    if (data->mmuWritePtr[index])
        data->mmuWritePtr[index][address & 0xFFF] = value;
    else
        data->mmu[index]->writeb(address, value);
}

// used by futex, may point to shared memory
U8* KMemory::getIntPtr(U32 address) {
    U32 index = address >> K_PAGE_SHIFT;
    U32 offset = address & K_PAGE_MASK;
    return data->mmu[index]->getReadPtr(this, address, true) + offset;
}

void KMemory::clone(KMemory* from) {
    ::memcpy(flags, from->flags, sizeof(flags));
    for (int i = 0; i < 0x100000; i++) {
        Page* page = from->data->getPage(i);
        if (page->getType() == Page::Type::Invalid_Page && flags[i] != 0) {
            if (mapShared(i)) {
                // convert to regular page with its own ram so that the ram can be shared
                InvalidPage* p = (InvalidPage*)page;
                p->ondemmand(from, i);
                // fall through
            } else {
                continue;
            }
        }
        if (page->getType() == Page::Type::File_Page) {
            FilePage* p = (FilePage*)page;
            if (mapShared(i)) {
                p->ondemmandFile(i << K_PAGE_SHIFT);
                // fall through
            } else {
                data->setPage(i, FilePage::alloc(p->mapped, p->index));
                continue;
            }
        }
        page = from->data->getPage(i); // above code could have changed this
        if (page->getType() == Page::Type::RO_Page || page->getType() == Page::Type::RW_Page || page->getType() == Page::Type::WO_Page || page->getType() == Page::Type::NO_Page) {
            RWPage* p = (RWPage*)page;
            if (!mapShared(i)) {
                if (page->getType() == Page::Type::WO_Page) {
                    U8* ram = ramPageAlloc();
                    ::memcpy(ram, p->page, K_PAGE_SIZE);
                    data->setPage(i, WOPage::alloc(ram, p->address));
                } else if (page->getType() == Page::Type::NO_Page) {
                    U8* ram = ramPageAlloc();
                    ::memcpy(ram, p->page, K_PAGE_SIZE);
                    data->setPage(i, NOPage::alloc(ram, p->address));
                } else {
                    data->setPage(i, CopyOnWritePage::alloc(p->page, p->address));
                    from->data->setPage(i, CopyOnWritePage::alloc(p->page, p->address));
                }
            } else {
                if (page->getType() == Page::Type::RO_Page) {
                    data->setPage(i, ROPage::alloc(p->page, p->address));
                } else if (page->getType() == Page::Type::RW_Page) {
                    data->setPage(i, RWPage::alloc(p->page, p->address));
                } else if (page->getType() == Page::Type::WO_Page) {
                    data->setPage(i, WOPage::alloc(p->page, p->address));
                } else if (page->getType() == Page::Type::NO_Page) {
                    data->setPage(i, NOPage::alloc(p->page, p->address));
                }
            }
        } else if (page->getType() == Page::Type::Code_Page) {
            // CodePage will check copy on write
            CodePage* p = (CodePage*)page;
            data->setPage(i, CodePage::alloc(p->page, p->address));
        }  else if (page->getType() == Page::Type::Copy_On_Write_Page) {
            CopyOnWritePage* p = (CopyOnWritePage*)page;
            data->setPage(i, CopyOnWritePage::alloc(p->page, p->address));
        } else if (page->getType() == Page::Type::Native_Page) {
            NativePage* p = (NativePage*)page;
            data->setPage(i, NativePage::alloc(p->nativeAddress, p->address));
        } else if (page->getType() == Page::Type::Invalid_Page) {
            
        } else {
            kpanic("unhandled case when cloning memory: page type = %d", static_cast<int>(page->getType()));
        }
    }
}

#ifndef BOXEDWINE_BINARY_TRANSLATOR
// normal core
CodeBlock KMemory::getCodeBlock(U32 address) {
    Page* page = data->getPage(address >> K_PAGE_SHIFT);
    if (page->getType() == Page::Type::Code_Page) {
        CodePage* codePage = (CodePage*)page;
        return codePage->getCode(address);
    }
    return nullptr;
}
#endif

CodeBlock KMemory::findCodeBlockContaining(U32 address, U32 len) {
    while (len) {
        Page* page = data->getPage(address >> K_PAGE_SHIFT);
        U32 offset = address & K_PAGE_MASK;
        U32 available = K_PAGE_SIZE - offset;
        U32 todo = len;
        if (todo > available) {
            todo = available;
        }
        if (page->getType() == Page::Type::Code_Page) {
            CodePage* codePage = (CodePage*)page;
            CodeBlock result = codePage->findCode(address, todo);
            if (result) {
                return result;
            }
        }  
        address += available;
        len -= todo;
    }
    return nullptr;
}

void KMemory::removeCodeBlock(U32 address, U32 len) {
    Page* page = data->getPage(address >> K_PAGE_SHIFT);
    
    if (page->getType() == Page::Type::Code_Page) {
        CodePage* codePage = (CodePage*)page;
        codePage->removeBlockAt(address, len);
    }
}

void KMemory::addCodeBlock(U32 address, CodeBlock block) {
    CodePage* codePage = data->getOrCreateCodePage(address);
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    codePage->addCode(address, block, block->getEipLen());
#else
    codePage->addCode(address, block, block->bytes);
#endif
}

void KMemoryData::markAddressDynamic(U32 address, U32 len) {
    Page* page = getPage(address >> K_PAGE_SHIFT);

    if (page->getType() == Page::Type::Code_Page) {
        CodePage* codePage = (CodePage*)page;
        U32 offset = address & K_PAGE_MASK;
        if (offset + len > K_PAGE_SIZE) {
            U32 todo = len - (K_PAGE_SIZE - offset);
            markAddressDynamic(address + len - todo, todo);
        }
        codePage->markOffsetDynamic(offset, len);
    }
}

bool KMemoryData::isAddressDynamic(U32 address, U32 len) {    
    bool result = false;

    Page* page = getPage(address >> K_PAGE_SHIFT);

    if (page->getType() == Page::Type::Code_Page) {
        CodePage* codePage = (CodePage*)page;
        U32 offset = address & K_PAGE_MASK;
        if (offset + len > K_PAGE_SIZE) {
            U32 todo = len - (K_PAGE_SIZE - offset);
            result = isAddressDynamic(address + len - todo, todo);
        }
        result |= codePage->isOffsetDynamic(offset, len);
    }
    return result;
}

CodePage* KMemoryData::getOrCreateCodePage(U32 address) {
    Page* page = getPage(address >> K_PAGE_SHIFT);

    CodePage* codePage = nullptr;
    if (page->getType() == Page::Type::Code_Page) {
        codePage = (CodePage*)page;
    } else {
        if (page->getType() == Page::Type::RO_Page || page->getType() == Page::Type::RW_Page || page->getType() == Page::Type::Copy_On_Write_Page) {
            RWPage* p = (RWPage*)page;
            codePage = CodePage::alloc(p->page, p->address);
            setPage(address >> K_PAGE_SHIFT, codePage);
        } else if (page->getType() == Page::Type::File_Page) {
            // code probably linked to a block that didn't exist and we created a place holder instruction there to re-translate (see callRetranslateChunk)
            FilePage* p = (FilePage*)page;
            p->ondemmandFile(address);
            return getOrCreateCodePage(address);
        } else {
            kpanic("Unhandled code caching page type: %d", static_cast<int>(page->getType()));
            codePage = nullptr;
        }
    }
    return codePage;
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
            if (data->getPage(i) != invalidPage) {
                start = i;
            }
        } else {
            if (data->getPage(i) == invalidPage) {
                klog("    %.8X - %.8X", start * K_PAGE_SIZE, i * K_PAGE_SIZE);
                start = 0;
            }
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
    Page* page = data->getPage(pageIndex);
    U32 offset = address & K_PAGE_MASK;
    U32 todo = len;
    if (todo > K_PAGE_SIZE - offset) {
        todo = K_PAGE_SIZE - offset;
    }

    U8* ram = readOnly ? page->getReadPtr(this, address, true) : page->getWritePtr(this, address, todo, true);
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
        ram = readOnly ? page->getReadPtr(this, address, true) : page->getWritePtr(this, address, K_PAGE_SIZE, true);
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
        ram = readOnly ? page->getReadPtr(this, address, true) : page->getWritePtr(this, address, len, true);
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
        return page->getReadPtr(this, address, true);
    }

    kpanic("KMemory::lockReadOnlyMemory need to implement");
    return nullptr;
}

void KMemory::unlockMemory(U8 * lockedPointer) {
    // :TODO: nothing todo until lockReadOnlyMemory supports crossing page boundry
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
    for (U32 i = 0; i < pageCount; i++) {
        flags[result + i] = PAGE_MAPPED | PAGE_READ | PAGE_WRITE;
        getMemData(this)->setPage(result + i, NativePage::alloc((U8*)hostAddress + K_PAGE_SIZE * i, (result << K_PAGE_SHIFT) + K_PAGE_SIZE * i));
    }
    return result << K_PAGE_SHIFT;
}
