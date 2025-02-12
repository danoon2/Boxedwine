#include "boxedwine.h"

#include "soft_invalid_page.h"
#include "kmemory_soft.h"
#include "soft_rw_page.h"
#include "soft_copy_on_write_page.h"
#include "soft_file_map.h"
#include "soft_code_page.h"
#include "devfb.h"
#ifdef BOXEDWINE_DYNAMIC
#include "../cpu/dynamic/dynamic_memory.h"
#endif
#include "soft_ram.h"

static InvalidPage _invalidPage;
static InvalidPage* invalidPage = &_invalidPage;
static RamPage callbackRam;
static U32 callbackRamPos;

void KMemoryData::shutdown() {
    callbackRam.value = 0;
    callbackRamPos = 0;
}

KMemoryData* getMemData(KMemory* memory) {
    return memory->data;
}

#ifdef BOXEDWINE_BINARY_TRANSLATOR
KMemoryData::KMemoryData(KMemory* memory) : BtMemory(memory), memory(memory), mmuReadPtrAdjusted{ 0 }, mmuWritePtrAdjusted{ 0 }
#ifdef BOXEDWINE_4K_PAGE_SIZE
, mmuReadPtr{ 0 }
, mmuWritePtr{ 0 }
#endif
#else
KMemoryData::KMemoryData(KMemory* memory) : memory(memory), mmuReadPtr{ 0 }, mmuWritePtr{ 0 }
#endif
{
    ::memset(mmu, 0, sizeof(mmu));
    if(!callbackRam.value) {
        callbackRam = ramPageAlloc();
        addCallback(onExitSignal);
    }
    this->allocPages(nullptr, CALL_BACK_ADDRESS >> K_PAGE_SHIFT, 1, K_PROT_READ | K_PROT_EXEC, -1, 0, nullptr, &callbackRam);
#ifdef BOXEDWINE_DYNAMIC
    dynamicMemory = nullptr;
#endif
}

KMemoryData::~KMemoryData() {
    setPagesInvalid(0, K_NUMBER_OF_PAGES);
#ifdef BOXEDWINE_DYNAMIC
    if (dynamicMemory) {
        delete dynamicMemory;
    }
#endif
}

void KMemoryData::onPageChanged(U32 index) {
    Page* page = this->mmu[index].getPage();
    U32 address = index << K_PAGE_SHIFT;    
#ifndef BOXEDWINE_BINARY_TRANSLATOR
    if (mmu[index].canReadRam) {
        this->mmuReadPtr[index] = page->getRamPtr(&mmu[index], index, false);
    } else {
        this->mmuReadPtr[index] = nullptr;
    }
    if (mmu[index].canWriteRam) {
        this->mmuWritePtr[index] = page->getRamPtr(&mmu[index], index, true);
    } else {
        this->mmuWritePtr[index] = nullptr;
    }
#else
    U8* readPtr = page->getRamPtr(&mmu[index], index, false);
    if (mmu[index].canReadRam) {
        this->mmuReadPtrAdjusted[index] = readPtr - (index << K_PAGE_SHIFT);
#ifdef BOXEDWINE_4K_PAGE_SIZE
        this->mmuReadPtr[index] = readPtr;
#endif
    } else {
        this->mmuReadPtrAdjusted[index] = nullptr;
#ifdef BOXEDWINE_4K_PAGE_SIZE
        this->mmuReadPtr[index] = nullptr;
#endif
    }

    U8* writePtr = page->getRamPtr(&mmu[index], index, true);
    if (mmu[index].canWriteRam) {
        this->mmuWritePtrAdjusted[index] = writePtr - (index << K_PAGE_SHIFT);
#ifdef BOXEDWINE_4K_PAGE_SIZE
        this->mmuWritePtr[index] = writePtr;
#endif
    } else {
        this->mmuWritePtrAdjusted[index] = nullptr;
#ifdef BOXEDWINE_4K_PAGE_SIZE
        this->mmuWritePtr[index] = nullptr;
#endif
    }
#endif
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

// don't need to add a mutex, memory->mutex should be locked when call except for construction and execv (which should only have 1 thread)
void KMemoryData::allocPages(KThread* thread, U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const std::shared_ptr<MappedFile>& mappedFile, const RamPage* ramPages) {
#ifdef _DEBUG
    if (page + pageCount >= K_NUMBER_OF_PAGES) {
        kpanic("KMemoryData::allocPages page out of bound");
    }
    if (mappedFile && ramPages) {
        kpanic("KMemoryData::allocPages mapped files should not contain ramPages");
    }
#endif
    if (ramPages) {
        for (U32 i = 0; i < pageCount; i++) {
            mmu[page + i].setFlags(permissions);
            this->mmu[page + i].setPage(this, page + i, PageType::Ram, ramPages[i]);
            onPageChanged(page + i);
        }
    } else if (mappedFile) {
        if (offset & K_PAGE_MASK) {
            kpanic("mmap: wasn't expecting the offset to be in the middle of a page");
        }

        for (U32 i = 0; i < pageCount; i++) {
            mmu[page + i].setFlags(permissions);
            this->mmu[page + i].setPage(this, page + i, PageType::File, (RamPage)mappedFile->key);
            onPageChanged(page + i);
        }
    } else {
        for (U32 i = 0; i < pageCount; i++) {
            mmu[page + i].setFlags(permissions);
            this->mmu[page + i].setPage(this, page + i, PageType::Ram, (RamPage)0);
            onPageChanged(page + i);
        }
    }
}

bool isAlignedNativePage(U32 page) {
    U32 gran = Platform::getPageAllocationGranularity();
    return (page & ~(gran - 1)) == page;
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
        U32 flags = mmu[i].flags;
        if (flags == 0 || (canBeReMapped && (flags & PAGE_MAPPED))) {
            U32 j = 1;
            bool success = true;

            for (; j < pageCount; j++) {
                U32 nextPage = i + j; // could be done a different way, but this helps the static analysis
                U32 nextPageFlags = mmu[nextPage].flags;
                if (nextPage < K_NUMBER_OF_PAGES && nextPageFlags != 0 && (!canBeReMapped || !(nextPageFlags & PAGE_MAPPED))) {
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
                    mmu[pageIndex].flags = reservedFlag;
                }
                return true;
            }
            i += j; // no reason to check all the pages again
        }
    }
    return false;
}

void KMemoryData::protectPage(KThread* thread, U32 i, U32 permissions) {
    mmu[i].setPermissions(permissions);
    onPageChanged(i);
}

bool KMemoryData::isPageAllocated(U32 page) {
    return mmu[page].getPageType() != PageType::None;
}

void KMemoryData::setPagesInvalid(U32 page, U32 pageCount) {
    BOXEDWINE_CRITICAL_SECTION(memory->mutex);
    for (U32 i = page; i < page + pageCount; i++) {
        mmu[i].flags = 0;
        mmu[i].setPage(this, i, PageType::None, (RamPage)0);
        onPageChanged(i);
    }
}

void KMemoryData::execvReset() {
    setPagesInvalid(0, K_NUMBER_OF_PAGES);    
    this->allocPages(KThread::currentThread(), CALL_BACK_ADDRESS >> K_PAGE_SHIFT, 1, K_PROT_READ | K_PROT_EXEC, -1, 0, nullptr, &callbackRam);
}

U64 KMemory::readq(U32 address) {
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
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
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
        if (data->mmuReadPtr[index])
            return *(U32*)(&data->mmuReadPtr[index][address & 0xFFF]);
#endif
        return data->mmu[index].getPage()->readd(&data->mmu[index], address);
    } else {
        return readb(address) | (readb(address + 1) << 8) | (readb(address + 2) << 16) | (readb(address + 3) << 24);
    }
}

U16 KMemory::readw(U32 address) {
    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
        if (data->mmuReadPtr[index])
            return *(U16*)(&data->mmuReadPtr[index][address & 0xFFF]);
#endif
        return data->mmu[index].getPage()->readw(&data->mmu[index], address);
    }
    return readb(address) | (readb(address + 1) << 8);
}

U8 KMemory::readb(U32 address) {
    int index = address >> 12;
#if !defined(BOXEDWINE_BINARY_TRANSLATOR)
    if (data->mmuReadPtr[index])
        return data->mmuReadPtr[index][address & 0xFFF];
#endif
    return data->mmu[index].getPage()->readb(&data->mmu[index], address);
}

void KMemory::writeq(U32 address, U64 value) {
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
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
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
        if (data->mmuWritePtr[index])
            *(U32*)(&data->mmuWritePtr[index][address & 0xFFF]) = value;
        else
#endif
            data->mmu[index].getPage()->writed(&data->mmu[index], address, value);
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
        if (data->mmuWritePtr[index])
            *(U16*)(&data->mmuWritePtr[index][address & 0xFFF]) = value;
        else
#endif
            data->mmu[index].getPage()->writew(&data->mmu[index], address, value);
    } else {
        writeb(address, (U8)value);
        writeb(address + 1, (U8)(value >> 8));
    }
}

void KMemory::writeb(U32 address, U8 value) {
    int index = address >> 12;
#if !defined(BOXEDWINE_BINARY_TRANSLATOR)
    if (data->mmuWritePtr[index])
        data->mmuWritePtr[index][address & 0xFFF] = value;
    else
#endif
        data->mmu[index].getPage()->writeb(&data->mmu[index], address, value);
}

U8* KMemory::getRamPtr(U32 address, U32 len, bool write, bool futex) {
    U32 index = address >> K_PAGE_SHIFT;
    U32 offset = address & K_PAGE_MASK;

    if (len + offset > K_PAGE_SIZE) {
        kpanic("KMemory::getRamPtr");
    }
    U8* result = data->mmu[index].getPage()->getRamPtr(&data->mmu[index], index, write, true, offset, len);
    if (result && futex) {
        data->mmu[index].flags |= PAGE_FUTEX;
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

    for (int i = 0; i < 0x100000; i++) {
        if (mapShared(i)) {
            data->mmu[i].getPage()->onDemmand(&data->mmu[i], i);
        }
    }

    ::memcpy(data->mmu, from->data->mmu, sizeof(data->mmu));

    for (int i = 0; i < 0x100000; i++) {
        MMU& mmu = data->mmu[i];

        RamPage ramPageIndex = mmu.getRamPageIndex();
        if (ramPageIndex.value) {
            ramPageRetain(ramPageIndex);

            if (!mapShared(i) && (mmu.flags & PAGE_FUTEX)) {
                RamPage ramPage = ramPageAlloc();
                ::memcpy(ramPageGet(ramPage), ramPageGet(ramPageIndex), K_PAGE_SIZE);
                mmu.setPage(data, i, PageType::Ram, ramPage);
                ramPageRelease(ramPage); // setPage retains
            } else if (mmu.getPageType() == PageType::Code) {
                // CodePage will check copy on write
                //mmu.setPageType(PageType::CopyOnWrite);                
            } else {
                mmu.setPageType(data, i, PageType::CopyOnWrite);
                from->data->mmu[i].setPageType(from->data, i, PageType::CopyOnWrite);
                from->data->onPageChanged(i);
            }
        }
        data->onPageChanged(i);
    }
}

#ifndef BOXEDWINE_BINARY_TRANSLATOR
// normal core
CodeBlock KMemory::getCodeBlock(U32 address) {
    return data->codeCache.getCode(address);
}
#endif

CodeBlock KMemory::findCodeBlockContaining(U32 address, U32 len) {
    return data->codeCache.findCode(address, len);
}

void KMemory::removeCodeBlock(U32 address, U32 len) {
    data->codeCache.removeBlockAt(address, len);
}

void KMemory::addCodeBlock(CodeBlockParam block) {
    data->getOrCreateCodePage(block->getEip());
    data->codeCache.addCode(this, block);
}

CodePage* KMemoryData::getOrCreateCodePage(U32 address) {
    U32 pageIndex = address >> K_PAGE_SHIFT;
    PageType type = mmu[pageIndex].getPageType();

    CodePage* codePage = nullptr;
    if (type == PageType::Code) {
        codePage = (CodePage*)mmu[pageIndex].getPage();
    } else if (type == PageType::File) {
        mmu[pageIndex].getPage()->onDemmand(&mmu[pageIndex], pageIndex);
        return getOrCreateCodePage(address);
    } else if (type == PageType::Ram || type == PageType::CopyOnWrite) {
        mmu[pageIndex].setPageType(this, pageIndex, PageType::Code);
        onPageChanged(pageIndex);
    } else {
        kpanic("Unhandled code caching page type: %d", static_cast<int>(mmu[pageIndex].getPageType()));
        codePage = nullptr;
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

    U8* ram = page->getRamPtr(&data->mmu[pageIndex], pageIndex, !readOnly, true, offset, todo);
    if (!ram) {
        int ii = 0;
    }
    if (!callback(ram, todo)) {
        return;
    }
    address += todo;
    len -= todo;

    while (len > K_PAGE_SIZE) {
        pageIndex++;
        page = data->getPage(pageIndex);
        ram = page->getRamPtr(&data->mmu[pageIndex], pageIndex, !readOnly, true, 0, K_PAGE_SIZE);
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
        ram = page->getRamPtr(&data->mmu[pageIndex], pageIndex, !readOnly, true, 0, len);
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
        return page->getRamPtr(&data->mmu[pageIndex], pageIndex, false, true, offset, len);
    }
    std::shared_ptr<LockedMemory> p = std::make_shared<LockedMemory>();
    p->p = new U8[len];
    p->len = len;
    p->address = address;
    p->readOnly = true;
    memcpy(p->p, address, len);
    lockedMemory.set(p->p, p);
    return p->p;
}

void KMemory::unlockMemory(U8 * lockedPointer) {
    std::shared_ptr<LockedMemory> p = lockedMemory.get(lockedPointer);
    if (p) {
        if (!p->readOnly) {
            memcpy(p->address, p->p, p->len);
        }
        lockedMemory.remove(lockedPointer);
    }
}

U8* KMemory::lockReadWriteMemory(U32 address, U32 len) {
    U32 pageIndex = address >> K_PAGE_SHIFT;
    Page* page = data->getPage(pageIndex);
    U32 offset = address & K_PAGE_MASK;

    // if we cross a page boundry then we will need to make a copy
    if (len <= K_PAGE_SIZE - offset) {
        U8* result = page->getRamPtr(&data->mmu[pageIndex], pageIndex, true, true, offset, len);
        if (result) {
            return result;
        }
    }
    std::shared_ptr<LockedMemory> p = std::make_shared<LockedMemory>();
    p->p = new U8[len];
    p->len = len;
    p->address = address;
    p->readOnly = false;
    memcpy(p->p, address, len);
    lockedMemory.set(p->p, p);
    return p->p;
}

void KMemory::unmapNativeMemory(U32 address, U32 size) {
    unmap(address - K_PAGE_SIZE, size + 2 * K_PAGE_SIZE);
}

U32 KMemory::mapNativeMemory(void* hostAddress, U32 size) {
    U32 result = 0;
    U32 pageCount = (size + K_PAGE_SIZE - 1) >> K_PAGE_SHIFT;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    if (!data->reserveAddress(ADDRESS_PROCESS_MMAP_START, pageCount + 2, &result, false, true, PAGE_MAPPED)) {
        return 0;
    }
    result++; // guard page
    for (U32 i = 0; i < pageCount; i++) {
        MMU& mmu = data->mmu[result + i];
        mmu.setFlags(PAGE_MAPPED | PAGE_READ | PAGE_WRITE);
        RamPage ramPage = ramPageAllocNative((U8*)hostAddress + K_PAGE_SIZE * i);
        mmu.setPage(data, result + i, PageType::Ram, ramPage);
        ramPageRelease(ramPage); // setPage retains
    }
    return result << K_PAGE_SHIFT;
}
