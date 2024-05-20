#include "boxedwine.h"
#include "../emulation/softmmu/kmemory_soft.h"
#include "../emulation/cpu/dynamic/dynamic_memory.h"
#include "../emulation/softmmu/soft_ram.h"
#include "../emulation/softmmu/soft_page.h"
#include "../emulation/softmmu/soft_rw_page.h"
#include "../emulation/softmmu/soft_copy_on_write_page.h"

MappedFileCache::~MappedFileCache() {
    delete[] this->data;
}

void KMemory::shutdown() {
    KMemoryData::shutdown();
}

KMemory::KMemory(KProcess* process) : process(process) {
    data = new KMemoryData(this);    
#ifdef BOXEDWINE_DYNAMIC
    dynamicMemory = nullptr;
#endif
}

KMemory::~KMemory() {
    if (data) {
        delete data;
    }
#ifdef BOXEDWINE_DYNAMIC
    if (dynamicMemory) {
        delete dynamicMemory;
    }
#endif
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
    KFileDescriptor* fd = nullptr;

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
            klog("tried to call mmap with invalid address: %X", addr);
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
            if (KSystem::wineMajorVersion >= 7) {
                pageStart = 0x10000;
            } else {
                pageStart = ADDRESS_PROCESS_MMAP_START;
            }
        }
        if (!data->reserveAddress(pageStart, pageCount, &pageStart, addr != 0, true, PAGE_MAPPED)) {
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
            bool addFileToSystemCache = true;
            if (addFileToSystemCache) {
                std::shared_ptr<MappedFileCache> cache = KSystem::getFileCache(mappedFile->file->openFile->node->path);
                if (!cache) {
                    cache = std::make_shared<MappedFileCache>(mappedFile->file->openFile->node->path);
                    KSystem::setFileCache(mappedFile->file->openFile->node->path, cache);
                    cache->file = mappedFile->file;
                    U32 size = ((U32)((fd->kobject->length() + K_PAGE_SIZE - 1) >> K_PAGE_SHIFT));
                    cache->data = new KRamPtr[size];
                    cache->dataSize = size;
                    ::memset(cache->data, 0, size * sizeof(KRamPtr));
                }
                mappedFile->systemCacheEntry = cache;
            }
            this->process->mappedFiles.set(mappedFile->address, mappedFile);
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
        kpanic("mremap not implemented: flags=%X", flags);
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

            U32 pageStart = result >> K_PAGE_SHIFT;
            
            for (U32 i = 0; i < oldPageCount; i++) {
                Page* oldPage = data->mmu[oldPageStart + i];
                Page* newPage = data->mmu[pageStart + i];

                Page::Type oldType = oldPage->getType();
                if (oldType == Page::Type::Invalid_Page) {
                    continue; // valid page but hasn't been read from or written to yet
                } else if (oldType == Page::Type::NO_Page || oldType == Page::Type::RO_Page || oldType == Page::Type::WO_Page || oldType == Page::Type::RW_Page) {
                    RWPage* rwPage = (RWPage*)oldPage;
                    data->setPageRam(rwPage->page, pageStart + i);
                } else if (oldType == Page::Type::Copy_On_Write_Page) {
                    CopyOnWritePage* rwPage = (CopyOnWritePage*)oldPage;
                    data->setPageRam(rwPage->page, pageStart + i, true);
                } else {
                    kpanic("KMemory::mremap not implemented for page type");
                }
            }
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

U32 KMemory::mapPages(KThread* thread, U32 startPage, const std::vector<KRamPtr>& pages, U32 permissions) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    if (startPage == 0 && !data->reserveAddress(ADDRESS_PROCESS_MMAP_START, (U32)pages.size(), &startPage, false, false, PAGE_MAPPED)) {
        return 0;        
    }
    this->data->allocPages(thread, startPage, (U32)pages.size(), permissions | PAGE_MAPPED, 0, 0, nullptr, (KRamPtr*)pages.data());
    return startPage << K_PAGE_SHIFT;
}

bool KMemory::isPageAllocated(U32 page) {
    return data->isPageAllocated(page);
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
        data->execvReset();
    } else {
        std::shared_ptr<KProcess> parent = KSystem::getProcess(process->parentId);
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
    return data->flags[page];
}
