#include "boxedwine.h"

#ifdef BOXEDWINE_64BIT_MMU
#include "kmemory_hard.h"
#include "../cpu/binaryTranslation/btCodeChunk.h"
#include "../cpu/binaryTranslation/btCodeMemoryWrite.h"
#include "../cpu/binaryTranslation/btMemory.h"

KMemoryData* getMemData(KMemory* memory) {
    return memory->data;
}

KMemoryData::KMemoryData(KMemory* memory) : BtMemory(memory), callbackPos(0), memory(memory), delayedReset(nullptr)
{
    memset(flags, 0, sizeof(flags));
    memset(nativeFlags, 0, sizeof(nativeFlags));
    memset(memOffsets, 0, sizeof(memOffsets));    

    memset(this->dynamicCodePageUpdateCount, 0, sizeof(this->dynamicCodePageUpdateCount));
    

    reserveNativeMemory();

    allocNativeMemory(CALL_BACK_ADDRESS >> K_PAGE_SHIFT, K_NATIVE_PAGES_PER_PAGE, PAGE_READ | PAGE_EXEC | PAGE_WRITE);
    this->addCallback(onExitSignal);
}

KMemoryData::~KMemoryData() {
    releaseNativeMemory();
    if (delayedReset) {
        delete delayedReset;
    }
}

void KMemoryData::releaseNativeMemory() {
    clearAllNeedsMemoryOffset();
    Platform::releaseNativeMemory((void*)this->id, 0x100000000l);
    memset(this->flags, 0, sizeof(this->flags));
    memset(this->nativeFlags, 0, sizeof(this->nativeFlags));
    memset(this->memOffsets, 0, sizeof(this->memOffsets));
    this->allocated = 0;
    BtMemory::releaseNativeMemory();
}

bool KMemoryData::reserveAddress(U32 startingPage, U32 pageCount, U32* result, bool canBeReMapped, bool alignNative, U32 reservedFlag) {
    U32 i;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);

    for (i = startingPage; i < K_NUMBER_OF_PAGES; i++) {
        if (alignNative && !isAlignedNativePage(i)) {
            continue;
        }
        if (i + pageCount >= K_NUMBER_OF_PAGES) {
            return false;
        }
        if (((this->flags[i] & (PAGE_MAPPED | PAGE_MAPPED_HOST)) == 0 && !this->isPageAllocated(i)) || (canBeReMapped && (this->flags[i] & PAGE_MAPPED))) {
            U32 j;
            bool success = true;

            for (j = 1; j < pageCount; j++) {
                if (((this->flags[i + j] & (PAGE_MAPPED | PAGE_MAPPED_HOST)) || this->isPageAllocated(i + j)) && (!canBeReMapped || !(this->flags[i + j] & PAGE_MAPPED))) {
                    success = false;
                    break;
                }
            }
            if (success) {
                *result = i;
                for (U32 pageIndex = 0; j < pageCount; pageIndex++) {
                    flags[i + pageIndex] |= reservedFlag;
                }
                return true;
            }
            i += j; // no reason to check all the pages again
        }
    }
    return false;
}

void KMemoryData::clearDelayedReset() {
    if (delayedReset) {
        delete delayedReset;
        delayedReset = nullptr;
    }
}

void KMemoryData::execvReset() {
    KMemoryData* newData = new KMemoryData(memory);
    newData->delayedReset = this;
    memory->data = newData;
}

void KMemoryData::setPagesInvalid(U32 page, U32 pageCount) {
    this->clearNeedsMemoryOffset(page, pageCount);
    freeNativeMemory(page, pageCount);
}

bool KMemoryData::isPageAllocated(U32 page) {
    return (flags[page] & PAGE_ALLOCATED) != 0;
}

void KMemoryData::protectPage(KThread* thread, U32 i, U32 permissions) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    if (!this->isPageAllocated(i) && (permissions & PAGE_PERMISSION_MASK)) {
        U8 flag = this->flags[i] & ~PAGE_PERMISSION_MASK;
        this->allocPages(thread, i, 1, permissions | flag, 0, 0, 0);
    } else {
        this->flags[i] &= ~PAGE_PERMISSION_MASK;
        this->flags[i] |= permissions;
        if (this->isPageAllocated(i)) {
            updatePagePermission(i, 1);
        }
    }
}

void KMemoryData::allocPages(KThread* thread, U32 page, U32 pageCount, U8 permissions, FD fd, U64 offset, const BoxedPtr<MappedFile>& mappedFile, U8** ramPages) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);

    for (U32 i = 0; i < pageCount; i++) {
        clearCodePageFromCache(page + i);
    }
    this->clearNeedsMemoryOffset(page, pageCount);
    if ((permissions & PAGE_PERMISSION_MASK) || mappedFile) {
        if ((permissions & PAGE_SHARED) == 0) {
            allocNativeMemory(page, pageCount, permissions);
        } else {
            bool needToLoad = false;

            freeNativeMemory(page, pageCount);

            if (!mappedFile->systemCacheEntry->data[0]) {
                U32 len = pageCount << K_PAGE_SHIFT;
                U8* ram = new U8[len]; // make it continuous

                //klog("shared ram %.08X%.08X @%.08X len=%X\n", (U32)((U64)ram >> 32), (U32)((U64)ram), (page << K_PAGE_SHIFT), len);
                memset(ram, 0, len);
                needToLoad = true;
                mappedFile->systemCacheEntry->data[0] = ram;
                if (offset) {
                    kpanic("allocPages doesn't support offset with shared memory");
                }
            }
            U64 offset = (U64)mappedFile->systemCacheEntry->data[0] - (page << K_PAGE_SHIFT);
            for (U32 i = 0; i < pageCount; i++) {
                this->memOffsets[page + i] = offset;
                this->flags[page + i] = PAGE_MAPPED_HOST | PAGE_ALLOCATED | permissions;
            }
            // if the native page wasn't removed from memory because the allocation granularity is more than 1 page and a near by page is in use, 
            // then if we don't mark the page as read only, it won't generate an exception and the shared memory won't be used.  updatePagePermission
            // will see that these pages are shared and will use a strict (lowest permission) for all pages in the granulaty
            updatePagePermission(page, pageCount);
            if (!needToLoad) {
                return;
            }
        }
    } else {
        U32 i;
        freeNativeMemory(page, pageCount);
        for (i = 0; i < pageCount; i++) {
            this->flags[i + page] = permissions;
        }
    }
    if (mappedFile) {
        bool addedWritePermission = false;

        // shared pages aren't in the normal native range
        if (!(permissions & PAGE_WRITE)) {
            for (U32 i = 0; i < pageCount; i++) {
                this->flags[i + page] |= PAGE_WRITE;
            }
            addedWritePermission = true;
            updateNativePermission(page, pageCount, PAGE_WRITE | PAGE_READ);
        }
        // :TODO: need to implement writing back to the file
        thread->process->pread64(thread, fd, page << K_PAGE_SHIFT, pageCount << K_PAGE_SHIFT, offset);
        if (addedWritePermission) {
            for (U32 i = 0; i < pageCount; i++) {
                this->flags[i + page] &= ~PAGE_WRITE;
            }
            updatePagePermission(page, pageCount);
        }
    }
}

bool KMemoryData::isValidReadAddress(U32 address, U32 len) {
    U32 startPage = address >> K_PAGE_SHIFT;
    U32 endPage = (address + len - 1) >> K_PAGE_SHIFT;
    for (U32 i = startPage; i <= endPage; i++) {
        if (!this->isPageAllocated(i)) {
            return false;
        }
        if (!(this->flags[i] & PAGE_READ)) {
            return false;
        }
    }
    return true;
}

bool KMemoryData::isValidWriteAddress(U32 address, U32 len) {
    U32 startPage = address >> K_PAGE_SHIFT;
    U32 endPage = (address + len - 1) >> K_PAGE_SHIFT;
    for (U32 i = startPage; i <= endPage; i++) {
        if (!this->isPageAllocated(i)) {
            return false;
        }
        if (!(this->flags[i] & PAGE_WRITE)) {
            return false;
        }
    }
    return true;
}

void KMemoryData::addCallback(OpCallback func) {
    U64 funcAddress = (U64)func;

    U8* address = (U8*)getNativeAddress(CALL_BACK_ADDRESS) + this->callbackPos;

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
    address++;
    *address = (U8)(funcAddress >> 32);
    address++;
    *address = (U8)(funcAddress >> 40);
    address++;
    *address = (U8)(funcAddress >> 48);
    address++;
    *address = (U8)(funcAddress >> 56);
    this->callbackPos += 12;
}

void* KMemoryData::getNativeAddress(U32 address) {
    U32 page = address >> K_PAGE_SHIFT;
#ifdef _DEBUG    
    if (!isPageAllocated(page) && (flags[page] & PAGE_MAPPED_HOST) == 0) {
        memory->logPageFault(KThread::currentThread(), KThread::currentThread()->cpu->eip.u32);
        kpanic("bad memory access");
    }
#endif
    return (void*)(address + memOffsets[page]);
}

void KMemoryData::unmapNativeMemory(U32 address, U32 size) {
    U32 pageCount = (size >> K_PAGE_SHIFT) + 2; // 1 for size alignment, 1 for hostAddress alignment
    U64 pageStart = address >> K_PAGE_SHIFT;

    for (U32 i = 0; i < pageCount; i++) {
        this->memOffsets[i + pageStart] = this->id;
        this->flags[i + pageStart] = 0;
    }
}

U32 KMemoryData::mapNativeMemory(void* hostAddress, U32 size) {
    U32 i;
    U32 result = 0;
    U64 hostStart = (U64)hostAddress & 0xFFFFFFFFFFFFF000l;
    U64 hostEnd = ((U64)hostAddress + size) & 0xFFFFFFFFFFFFF000l;
    U32 pageCount = (U32)(hostEnd - hostStart + K_PAGE_MASK) >> K_PAGE_SHIFT;
    U64 offset;

    for (int i = 0; i < K_NUMBER_OF_PAGES; i++) {
        if ((i << K_PAGE_SHIFT) + this->memOffsets[i] == hostStart) {
            return (i << (K_PAGE_SHIFT)) + ((U32)((U64)hostAddress) & K_PAGE_MASK);
        }
    }
    if (reserveAddress(0x10000, pageCount, &result, false, true, PAGE_MAPPED_HOST)) {
        offset = hostStart - (result << K_PAGE_SHIFT);
        for (i = 0; i < pageCount; i++) {
            this->memOffsets[result + i] = offset;
            this->flags[result + i] = PAGE_MAPPED_HOST | PAGE_READ | PAGE_WRITE;
        }
        return (result << K_PAGE_SHIFT) + ((U32)((U64)hostAddress) & K_PAGE_MASK);
    }
    return 0;
}

void KMemoryData::makeNativePageDynamic(U32 nativePage) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    U32 startPage = getEmulatedPage(nativePage);
    for (U32 i = 0; i < K_NATIVE_PAGES_PER_PAGE; i++) {
        U32 page = startPage + i;
        if (this->codeChunksByEmulationPage.count(page)) {
            std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByEmulationPage[page];
            for (auto& chunk : *chunks) {
                if (!chunk->isDynamicAware()) {
                    chunk->invalidateStartingAt(chunk->getEip());
                }
            }
        }
    }
    if (this->nativeFlags[nativePage] & NATIVE_FLAG_CODEPAGE_READONLY) {
        clearCodePageReadOnly(nativePage);
    }
    U32 page = startPage;
    U32 eip = page << K_PAGE_SHIFT;
    page--;
    // look to see if a chunk that starts in a previous page contains this address
    // chunks do not overlap, so find the first previous page with a chunk then
    // check on the chunks in that page
    while (page > 0) {
        if (this->codeChunksByEmulationPage.count(page)) {
            std::shared_ptr< std::list<std::shared_ptr<BtCodeChunk>> > chunks = this->codeChunksByEmulationPage[page];
            for (auto& chunk : *chunks) {
                if (chunk->containsEip(eip)) {
                    chunk->invalidateStartingAt(eip);
                    return;
                }
            }
            break;
        }
        page--;
    }
}

void KMemoryData::makeCodePageReadOnly(U32 nativePage) {
    if (!(this->nativeFlags[nativePage] & NATIVE_FLAG_CODEPAGE_READONLY)) {
        if (this->dynamicCodePageUpdateCount[nativePage] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
            kpanic("makeCodePageReadOnly: tried to make a dynamic code page read-only");
        }
        this->nativeFlags[nativePage] |= NATIVE_FLAG_CODEPAGE_READONLY;
        this->updatePagePermission(this->getEmulatedPage(nativePage), K_NATIVE_PAGES_PER_PAGE);
    }
}

bool KMemoryData::clearCodePageReadOnly(U32 nativePage) {
    bool result = false;

    if (this->nativeFlags[nativePage] & NATIVE_FLAG_CODEPAGE_READONLY) {
        this->nativeFlags[nativePage] &= ~NATIVE_FLAG_CODEPAGE_READONLY;
        this->updatePagePermission(this->getEmulatedPage(nativePage), K_NATIVE_PAGES_PER_PAGE);
        result = true;
    }
    return result;
}

void KMemoryData::reserveNativeMemory() {
    this->id = (U64)Platform::reserveNativeMemory(false);
    for (int i = 0; i < K_NUMBER_OF_PAGES; i++) {
        this->memOffsets[i] = this->id;
    }
    BtMemory::reserveNativeMemory();
}

void KMemoryData::clearHostCodeForWriting(U32 nativePage, U32 count) {
    U32 addressStart = getEmulatedPage(nativePage) << K_PAGE_SHIFT;
    U32 addressStop = getEmulatedPage(nativePage + count) << K_PAGE_SHIFT;
    for (U32 i = addressStart; i < addressStop; i++) {
        std::shared_ptr<BtCodeChunk> chunk = getCodeChunkContainingEip(i);
        if (chunk && !chunk->isDynamicAware()) {
            chunk->invalidateStartingAt(i);
            i = chunk->getEip() + chunk->getEipLen() - 1;
        }
    }

    for (U32 page = nativePage; page < nativePage + count; page++) {
        if (this->nativeFlags[page] & NATIVE_FLAG_CODEPAGE_READONLY) {
            if (dynamicCodePageUpdateCount[page] != MAX_DYNAMIC_CODE_PAGE_COUNT) {
                dynamicCodePageUpdateCount[page]++;
                if (dynamicCodePageUpdateCount[page] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
                    this->makeNativePageDynamic(page);
                }
            }
            clearCodePageReadOnly(page);
        }
    }
}

void KMemoryData::clearCodePageFromCache(U32 page) {
    BtMemory::clearCodePageFromCache(page);
    U32 nativePage = this->getNativePage(page);
    U32 startingPage = this->getEmulatedPage(nativePage);
    if (this->eipToHostInstructionPages) {
        for (int i = 0; i < K_NATIVE_PAGES_PER_PAGE; i++) {
            if (this->eipToHostInstructionPages[startingPage + i]) {
                return;
            }
        }
    }
    this->dynamicCodePageUpdateCount[nativePage] = 0;
    clearCodePageReadOnly(nativePage);
}

static U32 getNativePermissionIndex(U32 page) {
    return (page << K_PAGE_SHIFT) >> K_NATIVE_PAGE_SHIFT;
}

void KMemoryData::allocNativeMemory(U32 page, U32 pageCount, U32 flags) {
    U32 gran = Platform::getPageAllocationGranularity();
    U32 permissionGran = Platform::getPagePermissionGranularity();
    U32 granPage = page & ~(gran - 1);
    U32 granCount = ((gran - 1) + pageCount + (page - granPage)) / gran;
    U32 permPerAllocPage = gran / permissionGran;

    for (U32 i = 0; i < granCount; i++) {
        U64 address = (this->id | (granPage << K_PAGE_SHIFT));
        U32 nativePermissionIndex = getNativePermissionIndex(granPage);
        if (!(this->nativeFlags[nativePermissionIndex] & NATIVE_FLAG_COMMITTED)) {
            Platform::allocateNativeMemory(address);
            this->allocated += (gran << K_PAGE_SHIFT);
            for (U32 j = 0; j < permPerAllocPage; j++) {
                this->nativeFlags[nativePermissionIndex + j] |= NATIVE_FLAG_COMMITTED;
            }
        } else {
            // so that the memset works below
#ifdef _DEBUG
            if (permissionGran > gran) {
                kpanic("Wasn't expecting a larger permission size than the allocation size");
            }
#endif
            Platform::updateNativePermission(address, PAGE_READ | PAGE_WRITE, gran << K_PAGE_SHIFT);
        }
        granPage += gran;
    }
    for (U32 i = 0; i < pageCount; i++) {
        this->flags[page + i] = flags | PAGE_ALLOCATED;
        this->memOffsets[page + i] = this->id;
    }

    ::memset(getNativeAddress(page << K_PAGE_SHIFT), 0, pageCount << K_PAGE_SHIFT);

    granPage = page & ~(gran - 1);
    U32 granPageCount = granCount * gran;
    this->updatePagePermission(granPage, granPageCount);
    //printf("allocated %X - %X\n", page << PAGE_SHIFT, (page+pageCount) << PAGE_SHIFT);
}

void KMemoryData::freeNativeMemory(U32 page, U32 pageCount) {
    for (U32 i = 0; i < pageCount; i++) {
        U32 nativePermissionIndex = getNativePermissionIndex(page + i);
        this->nativeFlags[nativePermissionIndex] &= ~NATIVE_FLAG_CODEPAGE_READONLY;
        this->clearCodePageFromCache(page + i);
        this->flags[page + i] = 0;
        this->memOffsets[page + i] = this->id;
    }

    U32 gran = Platform::getPageAllocationGranularity();
    U32 permissionGran = Platform::getPagePermissionGranularity();
    U32 permPerAllocPage = gran / permissionGran;
    U32 granPage = page & ~(gran - 1);
    U32 granCount = ((gran - 1) + pageCount + (page - granPage)) / gran;
    for (U32 i = 0; i < granCount; i++) {
        U32 nativePermissionIndex = getNativePermissionIndex(granPage);
        if (this->nativeFlags[nativePermissionIndex] & NATIVE_FLAG_COMMITTED) {
            bool inUse = false;

            for (U32 j = 0; j < gran; j++) {
                if (this->isPageAllocated(granPage + j)) {
                    inUse = true;
                    break;
                }
            }
            if (!inUse) {
                U64 address = (this->id | (granPage << K_PAGE_SHIFT));
                Platform::freeNativeMemory(address);
                for (U32 j = 0; j < permPerAllocPage; j++) {
                    this->nativeFlags[nativePermissionIndex + j] = 0;
                }
                this->allocated -= (gran << K_PAGE_SHIFT);
            } else {
                updatePagePermission(granPage, gran);
            }
        }
        granPage += gran;
    }
}

bool KMemoryData::isShared(U32 page) {
    return (this->flags[page] & PAGE_SHARED) != 0;
}

void KMemoryData::updatePagePermission(U32 page, U32 pageCount) {
    U32 permissionGran = Platform::getPagePermissionGranularity();
    U32 permissionGranPage = page & ~(permissionGran - 1);
    U32 permissionGranCount = ((permissionGran - 1) + pageCount + (page - permissionGranPage)) / permissionGran;

    // could be mixed (M1 is 16K permission)
    for (U32 i = 0; i < permissionGranCount; i++) {
        bool hasShared = false;
        for (U32 i = 0; i < permissionGran; i++) {
            if (isShared(i + permissionGranPage)) {
                hasShared = true;
                break;
            } else if (flags[i + permissionGranPage] & PAGE_MAPPED_HOST) {
                hasShared = true;
                break;
            }
        }
        U32 permissions = 0;

        // shared needs to have 0 permission so that it generates an exception
        if (!hasShared) {
            permissions = this->flags[permissionGranPage];
            for (U32 j = 1; j < permissionGran; j++) {
                // :TODO: this should always be &, in order to use the most restrictive but this slows things down too much because of the generated exceptions
                permissions |= this->flags[permissionGranPage + j];
            }
        }
        U64 address = (this->id | (permissionGranPage << K_PAGE_SHIFT));
        U32 index = getNativePermissionIndex(permissionGranPage);
        if (this->nativeFlags[index] & NATIVE_FLAG_CODEPAGE_READONLY) {
            permissions &= ~PAGE_WRITE;
        }
        if (this->nativeFlags[index] & NATIVE_FLAG_COMMITTED) {
            this->nativeFlags[index] &= ~PAGE_PERMISSION_MASK;
            this->nativeFlags[index] |= (permissions & (PAGE_READ | PAGE_WRITE));
            Platform::updateNativePermission(address, permissions);
        }
        permissionGranPage += permissionGran;
    }
}

void KMemoryData::updateNativePermission(U32 page, U32 pageCount, U32 permission) {
    U32 permissionGran = Platform::getPagePermissionGranularity();
    U32 permissionGranPage = page & ~(permissionGran - 1);
    U32 permissionGranCount = ((permissionGran - 1) + pageCount + (page - permissionGranPage)) / permissionGran;

    for (U32 i = 0; i < permissionGranCount; i++) {
        U64 address = (this->id | (permissionGranPage << K_PAGE_SHIFT));
        U32 nativePage = getNativePage(permissionGranPage);
        if (this->nativeFlags[nativePage] & NATIVE_FLAG_COMMITTED) {
            Platform::updateNativePermission(address, permission);
            this->nativeFlags[nativePage] &= ~PAGE_PERMISSION_MASK;
            this->nativeFlags[nativePage] |= (permission & (PAGE_READ | PAGE_WRITE));
        }
        permissionGranPage += permissionGran;
    }
}

void KMemory::logPageFault(KThread* thread, U32 address) {
    U32 start = 0;
    U32 i;
    CPU* cpu = thread->cpu;

    BString name = thread->process->getModuleName(cpu->seg[CS].address + cpu->eip.u32);
    klog("%.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X %s at %.8X\n", cpu->seg[CS].address + cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, name.c_str(), thread->process->getModuleEip(cpu->seg[CS].address + cpu->eip.u32));

    klog("Page Fault at %.8X\n", address);
    klog("Valid address ranges:\n");
    for (i = 0; i < K_NUMBER_OF_PAGES; i++) {
        if (!start) {
            if (thread->process->memory->isPageAllocated(i)) {
                start = i;
            }
        } else {
            if (!thread->process->memory->isPageAllocated(i)) {
                klog("    %.8X - %.8X\n", start * K_PAGE_SIZE, i * K_PAGE_SIZE);
                start = 0;
            }
        }
    }
    klog("Mapped Files:\n");
    thread->process->printMappedFiles();
    cpu->walkStack(cpu->eip.u32, EBP, 2);
    kpanic("pf");
}

void KMemory::clone(KMemory* from) {
    int i = 0;
    KMemoryData* fromData = getMemData(from);
    KMemoryData* toData = getMemData(this);

    for (i = 0; i < 0x100000; i++) {
        if (from->isPageAllocated(i)) {
            if (fromData->flags[i] & PAGE_MAPPED_HOST) {
                toData->flags[i] = fromData->flags[i];
                toData->memOffsets[i] = fromData->memOffsets[i];
                continue;
            }
            bool changedWritePermission = false;
            if (!(fromData->flags[i] & PAGE_WRITE)) {
                changedWritePermission = true;
            }
            bool changedReadPermission = false;
            if (!fromData->isShared(i) && !(fromData->flags[i] & PAGE_READ)) {
                changedReadPermission = true;
                fromData->updateNativePermission(i, 1, PAGE_READ);
            }
            data->allocNativeMemory(i, 1, fromData->flags[i] | PAGE_WRITE);
            ::memcpy(data->getNativeAddress(i << K_PAGE_SHIFT), fromData->getNativeAddress(i << K_PAGE_SHIFT), K_PAGE_SIZE);
            if (changedWritePermission) {
                data->updatePagePermission(i, 1);
            }
            if (changedReadPermission) {
                fromData->updatePagePermission(i, 1);
            }
        } else {
            toData->flags[i] = fromData->flags[i];
        }
    }
}

U8 KMemory::readb(U32 address) {
#ifdef LOG_OPS
    U8 result = *(U8*)data->getNativeAddress(address);
    if (log)
        fprintf(logFile, "readb %X @%X\n", result, address);
    return result;
#else
    return *(U8*)data->getNativeAddress(address);
#endif
}

void KMemory::writeb(U32 address, U8 value) {
#ifdef LOG_OPS
    if (log)
        fprintf(logFile, "writeb %X @%X\n", value, address);
#endif

    U32 page = address >> K_PAGE_SHIFT;
    U32 nativePage = data->getNativePage(page);
    U8 flags = data->nativeFlags[nativePage];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        BtCodeMemoryWrite w((BtCPU*)KThread::currentThread()->cpu, address, 1);
        *(U8*)data->getNativeAddress(address) = value;
    } else if ((flags & NATIVE_FLAG_COMMITTED) || (data->flags[page] & PAGE_MAPPED_HOST)) {
        *(U8*)data->getNativeAddress(address) = value;
    } else {
        kpanic("writeb about to crash");
    }
}

U16 KMemory::readw(U32 address) {
#ifdef LOG_OPS
    U16 result = *(U16*)data->getNativeAddress(address);
    if (log)
        fprintf(logFile, "readw %X @%X\n", result, address);
    return result;
#else
    return *(U16*)data->getNativeAddress(address);
#endif
}

void KMemory::writew(U32 address, U16 value) {
#ifdef LOG_OPS
    if (thread->process->memory->log)
        fprintf(logFile, "writew %X @%X\n", value, address);
#endif

    U32 page = address >> K_PAGE_SHIFT;
    U32 nativePage = data->getNativePage(page);
    U8 flags = data->nativeFlags[nativePage];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        BtCodeMemoryWrite w((BtCPU*)KThread::currentThread()->cpu, address, 2);
        *(U16*)data->getNativeAddress(address) = value;
    } else if (flags & NATIVE_FLAG_COMMITTED || (data->flags[page] & PAGE_MAPPED_HOST)) {
        *(U16*)data->getNativeAddress(address) = value;
    } else {
        kpanic("writew about to crash");
    }
}

U32 KMemory::readd(U32 address) {
#ifdef LOG_OPS
    U32 result = *(U32*)data->getNativeAddress(address);
    if (log)
        fprintf(logFile, "readd %X @%X\n", result, address);
    return result;
#else
    return *(U32*)data->getNativeAddress(address);
#endif
}

void KMemory::writed(U32 address, U32 value) {
#ifdef LOG_OPS
    if (log)
        fprintf(logFile, "writed %X @%X\n", value, address);
#endif

    U32 page = address >> K_PAGE_SHIFT;
    U32 nativePage = data->getNativePage(page);
    U8 flags = data->nativeFlags[nativePage];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        BtCodeMemoryWrite w((BtCPU*)KThread::currentThread()->cpu, address, 4);
        *(U32*)data->getNativeAddress(address) = value;
    } else if ((flags & NATIVE_FLAG_COMMITTED) || (data->flags[page] & PAGE_MAPPED_HOST)) {
        *(U32*)data->getNativeAddress(address) = value;
    } else {
        kpanic("writed about to crash");
    }
}

U64 KMemory::readq(U32 address) {
    return *(U64*)data->getNativeAddress(address);
}

void KMemory::writeq(U32 address, U64 value) {
    U32 page = address >> K_PAGE_SHIFT;
    U32 nativePage = data->getNativePage(page);
    U8 flags = data->nativeFlags[nativePage];

    if (flags & NATIVE_FLAG_CODEPAGE_READONLY) {
        BtCodeMemoryWrite w((BtCPU*)KThread::currentThread()->cpu, address, 8);
        *(U64*)data->getNativeAddress(address) = value;
    } else if ((flags & NATIVE_FLAG_COMMITTED) || (data->flags[page] & PAGE_MAPPED_HOST)) {
        *(U64*)data->getNativeAddress(address) = value;
    } else {
        kpanic("writeq about to crash");
    }
}

U8* KMemory::getIntPtr(U32 address) {
    if (!address) {
        return 0;
    }
    return (U8*)data->getNativeAddress(address);
}

void KMemory::performOnMemory(U32 address, U32 len, bool readOnly, std::function<bool(U8* ram, U32 len)> callback) {
    if (!len) {
        return;
    }
    if (readOnly) {
        callback((U8*)data->getNativeAddress(address), len);
    } else {
        BtCodeMemoryWrite((BtCPU*)KThread::currentThread()->cpu, address, len);
        callback((U8*)data->getNativeAddress(address), len);
    }
}

// This doesn't mean the data can't be changed by another thread, it just means the pointer will stay valid
U8* KMemory::lockReadOnlyMemory(U32 address, U32 len) {
    return (U8*)data->getNativeAddress(address);
}

void KMemory::unlockMemory(U8* lockedPointer) {
    // :TODO: nothing todo until lockMemory supports crossing page boundry
}

// not needed
CodeBlock KMemory::getCodeBlock(U32 address) {
    return NULL;
}

void KMemory::addCodeBlock(U32 address, CodeBlock block) {
}

#endif