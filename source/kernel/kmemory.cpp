/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "../emulation/softmmu/kmemory_soft.h"
#include "../emulation/cpu/dynamic/dynamic_memory.h"
#include "../emulation/softmmu/soft_ram.h"
#include "../emulation/softmmu/soft_page.h"
#include "../emulation/softmmu/soft_rw_page.h"
#include "../emulation/softmmu/soft_copy_on_write_page.h"

MappedFileCache::~MappedFileCache() {
    for (RamPage& page : data) {
        ramPageRelease(page);
    }
}

void KMemory::shutdown() {
    KMemoryData::shutdown();
}

KMemory::KMemory(KProcess* process) : process(process) {
    data = new KMemoryData(this);    
}

KMemory::~KMemory() {
    if (data) {
        delete data;
    }
    if (deleteOnNextLoop) {
        delete deleteOnNextLoop;
    }
}

void KMemory::cleanup() {
    BOXEDWINE_CRITICAL_SECTION;
    if (data) {
        delete data;
        data = nullptr;
    }
    // don't delete dynamic memory yet, we might return to it to finish
}

KMemory* KMemory::create(KProcess* process) {
    return new KMemory(process);
}

U32 KMemory::mlock(U32 addr, U32 len) {
    return 0;
}

U32 KMemory::mmap(KThread* thread, U32 addr, U32 len, S32 prot, S32 flags, FD fildes, U64 off, bool remap) {
    bool shared = (flags & K_MAP_SHARED) != 0;
    bool priv = (flags & K_MAP_PRIVATE) != 0;
    bool read = (prot & K_PROT_READ) != 0;
    bool write = (prot & K_PROT_WRITE) != 0;
    bool exec = (prot & K_PROT_EXEC) != 0;
    U32 pageStart = addr >> K_PAGE_SHIFT;
    U32 pageCount = (len + K_PAGE_SIZE - 1) >> K_PAGE_SHIFT;
    KFileDescriptorPtr fd;

    if (0xFFFFFFFF - addr < len) {
        return -K_EINVAL;
    }
    if ((shared && priv) || (!shared && !priv)) {
        return -K_EINVAL;
    }

    if (!(flags & K_MAP_ANONYMOUS) && fildes >= 0) {
        fd = this->process->getFileDescriptor(fildes);
        if (!fd) {
            return -K_EBADF;
        }
        if (!fd->kobject->canMap()) {
            return -K_EACCES;
        }
        if (len == 0 || (off & 0xFFF) != 0) {
            return -K_EINVAL;
        }
        if ((!fd->canRead() && read) || (!priv && (!fd->canWrite() && write))) {
            return -K_EACCES;
        }
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    if (flags & (K_MAP_FIXED | K_MAP_FIXED_NOREPLACE)) {
        if (addr & (K_PAGE_SIZE - 1)) {
            klog_fmt("tried to call mmap with invalid address: %X", addr);
            return -K_EINVAL;
        }
        if (flags & K_MAP_FIXED_NOREPLACE) {
            if (!remap && addr != 0 && pageStart + pageCount > ADDRESS_PROCESS_MMAP_START) {
                return -K_ENOMEM;
            }
            for (U32 page = pageStart; page < pageStart + pageCount; page++) {
                if (isPageMapped(page)) {
                    return -K_EEXIST;
                }
            }
        }
    } else {
        if (pageStart + pageCount > ADDRESS_PROCESS_MMAP_START) {
            return -K_ENOMEM;
        }
        if (pageStart == 0) {
            // :TODO: this seems like a hack, there must be something wrong with how I implemented mmap
            if (KSystem::wineMajorVersion >= 7 && (flags & K_MAP_BOXEDWINE) == 0) {
                pageStart = 0x10000;
            } else {
                pageStart = ADDRESS_PROCESS_MMAP_START;
            }
        }
        if (!data->reserveAddress(pageStart, pageCount, &pageStart, false, addr == 0, PAGE_MAPPED)) {
            return -K_ENOMEM;
        }
        addr = pageStart << K_PAGE_SHIFT;
    }
    if (fd) {
        U32 result = fd->kobject->map(thread, addr, len, prot, flags, off);
        if (result) {
            return result;
        }
    }

    // even if there are no permissions, it is important for MAP_ANONYMOUS|MAP_FIXED existing memory to be 0'd out
    // if (write || read || exec)
    {
        U32 permissions = PAGE_MAPPED;

        if (write) {
            permissions |= PAGE_WRITE;
        }
        if (read) {
            permissions |= PAGE_READ;
        }
        if (exec) {
            permissions |= PAGE_EXEC;
        }
        if (shared) {
            permissions |= PAGE_SHARED;
        }
        if (fd) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->mappedFilesMutex);
            std::shared_ptr<MappedFile> mappedFile = std::make_shared<MappedFile>();

            mappedFile->address = pageStart << K_PAGE_SHIFT;
            mappedFile->len = ((U64)pageCount) << K_PAGE_SHIFT;
            mappedFile->offset = off;
            mappedFile->file = std::dynamic_pointer_cast<KFile>(fd->kobject);
            mappedFile->key = this->process->nextMappedFileIndex++;
            bool addFileToSystemCache = true;
            if (addFileToSystemCache) {
                std::shared_ptr<MappedFileCache> cache = KSystem::getFileCache(mappedFile->file->openFile->node->path);
                if (!cache) {
                    cache = std::make_shared<MappedFileCache>(mappedFile->file->openFile->node->path);
                    KSystem::setFileCache(mappedFile->file->openFile->node->path, cache);
                    cache->file = mappedFile->file;
                    U32 size = ((U32)((fd->kobject->length() + K_PAGE_SIZE - 1) >> K_PAGE_SHIFT));
                    cache->data.resize(size);
                }
                mappedFile->systemCacheEntry = cache;
            }
            this->process->mappedFiles.set(mappedFile->key, mappedFile);
            this->data->allocPages(thread, pageStart, pageCount, permissions, fildes, off, mappedFile);
        } else {
            this->data->allocPages(thread, pageStart, pageCount, permissions, 0, 0, nullptr);
        }
    }
    return addr;
}

U32 KMemory::mprotect(KThread* thread, U32 address, U32 len, U32 prot) {
    bool read = (prot & K_PROT_READ) != 0;
    bool write = (prot & K_PROT_WRITE) != 0;
    bool exec = (prot & K_PROT_EXEC) != 0;
    U32 pageStart = address >> K_PAGE_SHIFT;
    U32 pageCount = (len + K_PAGE_SIZE - 1) >> K_PAGE_SHIFT;
    U32 permissions = 0;

    if (write)
        permissions |= PAGE_WRITE;
    if (read)
        permissions |= PAGE_READ;
    if (exec)
        permissions |= PAGE_EXEC;

    for (U32 i = pageStart; i < pageStart + pageCount; i++) {
        if (!isPageMapped(i)) {
            return -K_ENOMEM;
        }
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    for (U32 i = pageStart; i < pageStart + pageCount; i++) {
        this->data->protectPage(thread, i, permissions);
    }
    return 0;
}

U32 KMemory::mremap(KThread* thread, U32 oldaddress, U32 oldsize, U32 newsize, U32 flags) {
    if (flags > 1) {
        kpanic_fmt("mremap not implemented: flags=%X", flags);
    }
    // is page aligned
    if (oldaddress & 0xFFF) {
        return -K_EINVAL;
    }
    if (newsize == 0) {
        return -K_EINVAL;
    }
    if (oldsize == 0) {
        kpanic("mremap not implemented for oldsize==0");
    }
    U32 oldPageCount = oldsize >> K_PAGE_SHIFT;
    U32 oldPageStart = oldaddress >> K_PAGE_SHIFT;
    U32 pageFlags = getPageFlags(oldPageStart);

    for (U32 i = 0; i < oldPageCount; i++) {
        if (getPageFlags(oldPageStart + i) != pageFlags) {
            return -K_EFAULT;
        }
    }
    if (newsize < oldsize) {
        this->unmap(oldaddress + newsize, oldsize - newsize);
        return oldaddress;
    } else {
        U32 prot = 0;
        U32 f = 0;
        if (pageFlags & PAGE_READ) {
            prot |= K_PROT_READ;
        }
        if (pageFlags & PAGE_WRITE) {
            prot |= K_PROT_WRITE;
        }
        if (pageFlags & PAGE_EXEC) {
            prot |= K_PROT_EXEC;
        }
        if (pageFlags & PAGE_SHARED) {
            f |= K_MAP_SHARED;
        } else {
            f |= K_MAP_PRIVATE;
        }        
        U32 result = this->mmap(thread, oldaddress + oldsize, newsize - oldsize, prot, f | K_MAP_FIXED_NOREPLACE, -1, 0, true);
        if (result == oldaddress + oldsize) {
            return oldaddress;
        }
        if ((flags & 1) != 0) { // MREMAP_MAYMOVE
            result = this->mmap(thread, 0, newsize, prot, f | K_MAP_ANONYMOUS, -1, 0);
            this->memcpy(result, oldaddress, oldsize);
            this->unmap(oldaddress, oldsize);
            return result;
        }
        return -K_ENOMEM;
    }
}

U32 KMemory::unmap(U32 address, U32 len) {
    U32 pageStart = address >> K_PAGE_SHIFT;
    U32 pageCount = (len + K_PAGE_SIZE - 1) >> K_PAGE_SHIFT;

    this->data->setPagesInvalid(pageStart, pageCount);
    return 0;
}

U32 KMemory::mapPages(KThread* thread, U32 startPage, const std::vector<RamPage>& pages, U32 permissions) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    if (startPage == 0 && !data->reserveAddress(ADDRESS_PROCESS_MMAP_START, (U32)pages.size(), &startPage, false, false, PAGE_MAPPED)) {
        return 0;        
    }
    this->data->allocPages(thread, startPage, (U32)pages.size(), permissions | PAGE_MAPPED, 0, 0, nullptr, pages.data());
    return startPage << K_PAGE_SHIFT;
}

bool KMemory::isPageAllocated(U32 page) {
    return data->isPageAllocated(page);
}

bool KMemory::isPageNative(U32 page) {
    return data->isPageNative(page);
}

bool KMemory::canWrite(U32 address, U32 len) {
    bool result = true;

    iteratePages(address, len, [this, &result](U32 page) {
        if (getPageFlags(page) & PAGE_WRITE) {
            return true;
        }
        result = false;
        return false;
        });
    return result;
}

bool KMemory::canRead(U32 address, U32 len) {
    bool result = true;

    iteratePages(address, len, [this, &result](U32 page) {
        if (getPageFlags(page) & PAGE_READ) {
            return true;
        }
        result = false;
        return false;
        });
    return result;
}

void KMemory::execvReset(bool cloneVM) {
    if (!cloneVM) {
#ifdef BOXEDWINE_BINARY_TRANSLATOR
        deleteOnNextLoop = data;
        data = new KMemoryData(this);
#else
        data->execvReset();
#endif
    } else {
        // data no longer shared with parent
        data = new KMemoryData(this);
    }
}

void KMemory::memcpy(U32 address, const void* p, U32 len) {
    U8* ptr = (U8*)p;

    performOnMemory(address, len, false, [&ptr](U8* ram, U32 len) {
        ::memcpy(ram, ptr, len);
        ptr += len;
        return true;
        });
}

void KMemory::memcpy(U32 dest, U32 src, U32 len) {
    for (U32 i = 0; i < len; i++, src++, dest++) {
        writeb(dest, readb(src));
    }
}

void KMemory::memcpy(void* p, U32 address, U32 len) {
    U8* ptr = (U8*)p;

    performOnMemory(address, len, true, [&ptr](U8* ram, U32 len) {
        ::memcpy(ptr, ram, len);
        ptr += len;
        return true;
        });
}

void KMemory::strcpy(U32 address, const char* str) {
    if (!str) {
        return;
    }

    while (true) {
        writeb(address, *str);
        if (*str == 0) {
            break;
        }
        address++;
        str++;
    }
}

void KMemory::memset(U32 address, char value, U32 len) {
    performOnMemory(address, len, false, [value] (U8* ram, U32 len) {
        ::memset(ram, value, len);
        return true;
        });
}

int KMemory::memcmp(U32 address, const void* p, U32 len) {
    U8* ptr = (U8*)p;
    int result = 0;

    performOnMemory(address, len, false, [&result, &ptr](U8* ram, U32 len) {
        result = ::memcmp(ptr, ram, len);
        ptr += len;
        return result == 0;
        });

    return result;
}

int KMemory::strlen(U32 address) {
    int result = 0;
    if (!address) {
        return 0;
    }
    while (true) {
        if (readb(address) == 0) {
            return result;
        }
        address++;
        result++;
    }
}

BString KMemory::readString(U32 address) {
    if (!address) {
        return BString::empty;
    }
    BString result;
    while (true) {
        char ch = (char)readb(address);
        if (ch == 0) {
            break;
        }
        address++;
        result += ch;
    }
    return result;
}

BString KMemory::readStringW(U32 address) {
    BString result;
    while (true) {
        char ch = (char)readw(address);
        if (ch == 0) {
            break;
        }
        address += 2;
        result += ch;
    }
    return result;
}

void KMemory::iteratePages(U32 address, U32 len, std::function<bool(U32 page)> callback) {
    U32 startPage = address >> K_PAGE_SHIFT;
    U32 stopPage = (address + len - 1) >> K_PAGE_SHIFT;
    for (U32 i = 0; i <= (stopPage - startPage); i++) {
        if (!callback(i+startPage)) {
            break;
        }
    }
}

U32 KMemory::getPageFlags(U32 page) {
    return data->mmu[page].flags;
}

DecodedOp** KMemory::getDecodedOpLocation(U32 address) {
    return data->opCache.getLocation(address);
}

DecodedOp* KMemory::getDecodedOp(U32 address) {
    return data->opCache.get(address);
}

void KMemory::threadCleanup(U32 threadId) {
    if (data) {
        data->opCache.threadCleanup(threadId);
    }
}

void KMemory::clearOpCache() {
    data->opCache.clear();
}

void* KMemory::allocCodeMemory(U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex)
    return data->codeMemory.alloc(len);
}

bool KMemory::isCode(void* p) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex)
    return data->codeMemory.containsAddress(p);
}

#ifdef BOXEDWINE_BINARY_TRANSLATOR
void KMemory::removeCodeBlock(DecodedOp* op, bool clearOps) {
    DecodedOp* blockOp = op->blockStart;    
    U32 blockLen = blockOp->blockLen;
    U32 blockOpCount = blockOp->blockOpCount;
    DecodedOp* nextOp = blockOp;
        
    for (U32 i = 0; i < blockOpCount; i++) {
        if (nextOp->blockStart != blockOp && nextOp->inst != Done) {
            kpanic("KMemory::removeCodeBlock nextOp->blockStart");
        }
        nextOp->blockStart = nullptr;
        nextOp->blockOpCount = 0;
        nextOp->blockLen = 0;
        nextOp->pfnJitCode = nullptr;
        nextOp = nextOp->next;
    }
    if (clearOps) {
        data->opCache.remove(blockOp->eip, blockLen, false);
    }
    data->codeMemory.free(blockOp->pfnJitCode);
}
#endif

static void opCallback(DecodedOp* op, void* p) {
    if (op->blockStart) {
        ((KMemory*)p)->removeCodeBlock(op);
    }
}

static void opCallbackCheck(DecodedOp* op, void* p) {
    if (!(op->flags & OP_FLAG_EMULATED_OP)) {
        *((bool*)p) = true;
    } else {
        int ii = 0;
    }
}

bool KMemory::removeCode(U32 address, U32 len, bool becauseOfWrite) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex)
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    if (becauseOfWrite) {
        bool foundOp = false;
        data->opCache.iterateOps(address, len, opCallbackCheck, &foundOp);
        if (!foundOp) {
            return false;
        }
    }
    data->opCache.iterateOps(address, len, opCallback, this);  
#endif
    return data->opCache.remove(address, len, becauseOfWrite);
}

void KMemory::clearPageWriteCounts(U32 pageIndex) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex)
    data->opCache.clearPageWriteCounts(pageIndex);
}

void KMemory::addCode_nolock(U32 address, U32 len, DecodedOp* op, U32 opCount) {
    iteratePages(address, len, [this](U32 page) {
        this->data->getOrCreateCodePage(page << K_PAGE_SHIFT);
        return true;
        });
    data->opCache.add(op, address, opCount);
}

bool KMemory::isAddressDynamic(U32 address, U32 len) {
    return data->opCache.isAddressDynamic(address, len);
}

void KMemory::logPageFault(KThread* thread, U32 address) {
    U32 start = 0;
    CPU* cpu = thread->cpu;

    BString name = process->getModuleName(cpu->seg[CS].address + cpu->eip.u32);
    klog_fmt("%.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X %s at %.8X", cpu->seg[CS].address + cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, name.c_str(), process->getModuleEip(cpu->seg[CS].address + cpu->eip.u32));

    klog_fmt("Page Fault at %.8X", address);
    klog("Valid address ranges:");
    for (U32 i = 0; i < K_NUMBER_OF_PAGES; i++) {
        if (!start) {
            if (data->isPageValid(i)) {
                start = i;
            }
        } else {
            if (!data->isPageValid(i)) {
                klog_fmt("    %.8X - %.8X", start * K_PAGE_SIZE, i * K_PAGE_SIZE);
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
        kpanic("KMemory::performOnMemory failed to get ram");
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
            kpanic("KMemory::performOnMemory failed to get ram");
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
            kpanic("KMemory::performOnMemory failed to get ram");
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
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockedMemoryMutex);
    lockedMemory.set(p->p, p);
    return p->p;
}

void KMemory::unlockMemory(U8* lockedPointer) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockedMemoryMutex);
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
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(lockedMemoryMutex);
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
        mmu.setPage(this, result + i, PageType::Ram, ramPage);
        ramPageRelease(ramPage); // setPage retains
    }
    return result << K_PAGE_SHIFT;
}

U64 KMemory::readq(U32 address) {
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
    if ((address & 0xFFF) < 0xFF9) {
        int index = address >> 12;
        MMU& mmu = data->mmu[index];
        if (mmu.canReadRam) {
            return *(U64*)(&(ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF]));
        }
    }
#endif
    return readd(address) | ((U64)readd(address + 4) << 32);
}

U32 KMemory::readd(U32 address) {
    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
        MMU& mmu = data->mmu[index];
        if (mmu.canReadRam)
            return *(U32*)(&(ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF]));
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
        MMU& mmu = data->mmu[index];
        if (mmu.canReadRam)
            return *(U16*)(&(ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF]));
#endif
        return data->mmu[index].getPage()->readw(&data->mmu[index], address);
    }
    return readb(address) | (readb(address + 1) << 8);
}

U8 KMemory::readb(U32 address) {
    int index = address >> 12;
#if !defined(BOXEDWINE_BINARY_TRANSLATOR)
    MMU& mmu = data->mmu[index];
    if (mmu.canReadRam)
        return ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF];
#endif
    return data->mmu[index].getPage()->readb(&data->mmu[index], address);
}

void KMemory::writeq(U32 address, U64 value) {
#if !defined(UNALIGNED_MEMORY) && !defined(BOXEDWINE_BINARY_TRANSLATOR)
    if ((address & 0xFFF) < 0xFF9) {
        int index = address >> 12;
        MMU& mmu = data->mmu[index];
        if (mmu.canWriteRam) {
            *(U64*)(&(ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF])) = value;
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
        MMU& mmu = data->mmu[index];
        if (mmu.canWriteRam)
            *(U32*)(&(ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF])) = value;
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
        MMU& mmu = data->mmu[index];
        if (mmu.canWriteRam)
            *(U16*)(&(ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF])) = value;
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
    MMU& mmu = data->mmu[index];
    if (mmu.canWriteRam)
        ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF] = value;
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
                mmu.setPage(this, i, PageType::Ram, ramPage);
                ramPageRelease(ramPage); // setPage retains
            } else if (mmu.getPageType() == PageType::Code) {
                // CodePage will check copy on write
                //mmu.setPageType(PageType::CopyOnWrite);                
            } else {
                mmu.setPageType(this, i, PageType::CopyOnWrite);
                from->data->mmu[i].setPageType(from, i, PageType::CopyOnWrite);
                from->data->onPageChanged(i);
            }
        }
        data->onPageChanged(i);
    }
}