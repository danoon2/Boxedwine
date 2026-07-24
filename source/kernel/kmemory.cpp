/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
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
#include "../emulation/softmmu/soft_ram.h"
#include "../emulation/softmmu/soft_page.h"
#include "../emulation/softmmu/soft_rw_page.h"
#include "../emulation/softmmu/soft_copy_on_write_page.h"
#include "../emulation/cpu/normal/normalCPU.h"

#ifdef BOXEDWINE_JIT
#include "../emulation/cpu/jit/jitCodeLifecycle.h"
#endif

MappedFileCache::MappedFileCache(BString name, const std::shared_ptr<KFile>& file, U64 length)
    : name(name), file(file), length(length) {
}

MappedFileCache::~MappedFileCache() {
    if (mappingLeaseCount) {
        kwarn_fmt("Mapped file cache %s destroyed with %u live mapping leases",
            name.c_str(), mappingLeaseCount);
    }
    for (RamPage& page : data) {
        ramPageRelease(page);
    }
}

RamPage MappedFileCache::getOrCreatePage(U32 pageIndex, bool shared) {
    while (true) {
        U64 generation;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
            if (pageIndex < data.size() && data[pageIndex].value) {
                if (shared) {
                    possiblyDirty[pageIndex] = true;
                }
                ramPageRetain(data[pageIndex]);
                return data[pageIndex];
            }
            generation = mutationGeneration;
        }

        RamPage loaded = ramPageAlloc();
        file->preadNativeUncached(ramPageGet(loaded), (U64)pageIndex << K_PAGE_SHIFT, K_PAGE_SIZE);
#ifdef __TEST
        std::function<void()> afterPageRead;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
            afterPageRead = testAfterPageReadHook;
        }
        if (afterPageRead) {
            afterPageRead();
        }
#endif

        RamPage result;
        bool installed = false;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutationMutex);
            {
                BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
                if (pageIndex < data.size() && data[pageIndex].value) {
                    result = data[pageIndex];
                    ramPageRetain(result);
                    if (shared) {
                        possiblyDirty[pageIndex] = true;
                    }
                } else if (generation == mutationGeneration) {
                    if (data.size() <= pageIndex) {
                        data.resize(pageIndex + 1);
                        possiblyDirty.resize(pageIndex + 1, false);
                    }
                    ramPageRetain(loaded);
                    ramPageMarkSystem(loaded, true);
                    data[pageIndex] = loaded;
                    if (shared) {
                        possiblyDirty[pageIndex] = true;
                    }
                    result = loaded;
                    installed = true;
                }
            }
            if (result.value) {
                if (!installed) {
                    ramPageRelease(loaded);
                }
                return result;
            }
        }
        ramPageRelease(loaded);
    }
}

#ifdef __TEST
void MappedFileCache::setTestAfterPageReadHook(const std::function<void()>& hook) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    testAfterPageReadHook = hook;
}

void MappedFileCache::setTestAfterFinalRetirementPreparationHook(const std::function<void()>& hook) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    if (hook) {
        testAfterFinalRetirementPreparationHook = std::make_shared<std::function<void()>>(hook);
    } else {
        testAfterFinalRetirementPreparationHook.reset();
    }
}

void MappedFileCache::setTestFailWritebackPreparationAllocation(bool fail) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    testFailWritebackPreparationAllocation = fail;
}

U32 MappedFileCache::getMappingLeaseCountForTest() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    return mappingLeaseCount;
}

U32 MappedFileCache::getRetirementAccountingFinalizeCountForTest() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    return testRetirementAccountingFinalizeCount;
}

KWritebackResult MappedFileCache::testWritebackPreparedBytes(U64 offset, const U8* buffer, U32 len,
    const std::function<void()>& afterPreparation) {
    std::shared_ptr<KFile> target;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
        target = writeFile;
    }
    if (!target) {
        KWritebackResult result;
        result.preparationError = -K_EIO;
        return result;
    }
    struct TestWritebackContext {
        U64 offset;
        const U8* buffer;
        U32 len;
        const std::function<void()>* afterPreparation;
    } context = {offset, buffer, len, &afterPreparation};
    KWritebackResult result = target->writeback(&context,
        [](void* opaque, std::vector<KFile::WritebackRange>& ranges) -> S32 {
            TestWritebackContext& context = *static_cast<TestWritebackContext*>(opaque);
            // Model Task 5's byte snapshot inside the mutation transaction.
            ranges.push_back({context.offset,
                std::vector<U8>(context.buffer, context.buffer + context.len)});
            (*context.afterPreparation)();
            return 0;
        });
    return result;
}
#endif

void MappedFileCache::overlayRead(U64 offset, U8* buffer, U32 len) {
    while (len) {
        U64 pageIndex = offset >> K_PAGE_SHIFT;
        U32 pageOffset = (U32)(offset & K_PAGE_MASK);
        U32 chunk = std::min<U32>(len, K_PAGE_SIZE - pageOffset);
        RamPage page;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
            if (pageIndex <= std::numeric_limits<size_t>::max() &&
                    pageIndex < data.size() &&
                    data[(size_t)pageIndex].value) {
                page = data[(size_t)pageIndex];
                ramPageRetain(page);
            }
        }
        if (page.value) {
            memcpy(buffer, ramPageGet(page) + pageOffset, chunk);
            ramPageRelease(page);
        }
        offset += chunk;
        buffer += chunk;
        len -= chunk;
    }
}

void MappedFileCache::updateWrite(U64 offset, const U8* buffer, U32 len) {
    if (!len) {
        return;
    }
    U64 maxLength = ~(U64)0;
    if ((U64)len > maxLength - offset) {
        len = (U32)(maxLength - offset);
        if (!len) {
            return;
        }
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutationMutex);

    struct ResidentRange {
        RamPage page;
        U32 offset;
        U32 len;
    };
    std::vector<ResidentRange> holeRanges;
    U64 writeEnd = offset + len;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
        ++mutationGeneration;
        U64 oldLength = this->length;
        if (writeEnd > oldLength) {
            this->length = writeEnd;
            if (offset > oldLength && !data.empty()) {
                U64 firstPage = oldLength >> K_PAGE_SHIFT;
                U64 lastPage = (offset - 1) >> K_PAGE_SHIFT;
                if (firstPage < data.size()) {
                    lastPage = std::min<U64>(lastPage, data.size() - 1);
                    for (U64 pageIndex = firstPage; pageIndex <= lastPage; ++pageIndex) {
                        RamPage page = data[(size_t)pageIndex];
                        if (!page.value) {
                            continue;
                        }
                        U64 pageStart = pageIndex << K_PAGE_SHIFT;
                        U64 zeroStart = std::max(oldLength, pageStart);
                        U64 zeroEnd = std::min(offset, pageStart + K_PAGE_SIZE);
                        holeRanges.push_back({page, (U32)(zeroStart - pageStart), (U32)(zeroEnd - zeroStart)});
                        ramPageRetain(page);
                    }
                }
            }
        }
    }
    for (const ResidentRange& range : holeRanges) {
        memset(ramPageGet(range.page) + range.offset, 0, range.len);
        ramPageRelease(range.page);
    }

    while (len) {
        U64 pageIndex = offset >> K_PAGE_SHIFT;
        U32 pageOffset = (U32)(offset & K_PAGE_MASK);
        U32 chunk = std::min<U32>(len, K_PAGE_SIZE - pageOffset);
        RamPage page;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
            if (pageIndex < data.size() && data[(size_t)pageIndex].value) {
                page = data[(size_t)pageIndex];
                ramPageRetain(page);
            }
        }
        if (page.value) {
            memcpy(ramPageGet(page) + pageOffset, buffer, chunk);
            ramPageRelease(page);
        }
        offset += chunk;
        buffer += chunk;
        len -= chunk;
    }
}

void MappedFileCache::setLength(U64 length) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutationMutex);
    struct ResidentRange {
        RamPage page;
        U32 offset;
        U32 len;
    };
    std::vector<ResidentRange> residentRanges;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
        ++mutationGeneration;
        U64 oldLength = this->length;
        this->length = length;
        if (oldLength == length) {
            return;
        }

        U64 rangeStart = std::min(oldLength, length);
        U64 rangeEnd = std::max(oldLength, length);
        U64 firstPage = rangeStart >> K_PAGE_SHIFT;
        U64 lastPage = (rangeEnd - 1) >> K_PAGE_SHIFT;
        if (!data.empty() && firstPage < data.size()) {
            lastPage = std::min<U64>(lastPage, data.size() - 1);
            for (U64 pageIndex = firstPage; pageIndex <= lastPage; ++pageIndex) {
                RamPage page = data[(size_t)pageIndex];
                if (!page.value) {
                    continue;
                }
                U64 pageStart = pageIndex << K_PAGE_SHIFT;
                U64 zeroStart = std::max(rangeStart, pageStart);
                U64 zeroEnd = std::min(rangeEnd, pageStart + K_PAGE_SIZE);
                residentRanges.push_back({page, (U32)(zeroStart - pageStart), (U32)(zeroEnd - zeroStart)});
                ramPageRetain(page);
            }
        }
    }
    for (const ResidentRange& range : residentRanges) {
        memset(ramPageGet(range.page) + range.offset, 0, range.len);
        ramPageRelease(range.page);
    }
}

void MappedFileCache::setWriteFile(const std::shared_ptr<KFile>& file) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    // Keep the first writable descriptor alive. Replacing it here could release
    // the previous KFile while the caller owns the identity gate.
    if (!this->writeFile) {
        this->writeFile = file;
    }
}

void MappedFileCache::retainMapping() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    if (mappingLeaseCount == std::numeric_limits<U32>::max()) {
        kpanic("mapped file cache lease count overflow");
    }
    ++mappingLeaseCount;
}

S32 MappedFileCache::snapshotWritebackRangesLocked(U64 offset, U64 len,
    std::vector<KWritebackRange>& ranges, U64& preparedBytes) {
    preparedBytes = 0;
    if (!len || offset >= length || data.empty() || possiblyDirty.empty()) {
        return 0;
    }
    const U64 requestedEnd = offset + len;
    const U64 flushEnd = std::min(requestedEnd, length);
    if (offset >= flushEnd) {
        return 0;
    }

    try {
#ifdef __TEST
        if (testFailWritebackPreparationAllocation) {
            throw std::bad_alloc();
        }
#endif
        const U64 residentPageCount = (U64)std::min(data.size(), possiblyDirty.size());
        U64 firstPage = offset >> K_PAGE_SHIFT;
        if (firstPage >= residentPageCount) {
            return 0;
        }
        U64 lastPage = (flushEnd - 1) >> K_PAGE_SHIFT;
        lastPage = std::min(lastPage, residentPageCount - 1);

        for (U64 pageIndex = firstPage; pageIndex <= lastPage; ++pageIndex) {
            if (!data[(size_t)pageIndex].value || !possiblyDirty[(size_t)pageIndex]) {
                continue;
            }
            const U64 pageStart = pageIndex << K_PAGE_SHIFT;
            const U64 writeStart = std::max(offset, pageStart);
            const U64 writeEnd = std::min(flushEnd, pageStart + K_PAGE_SIZE);
            if (writeStart >= writeEnd) {
                continue;
            }
            const U64 byteCount = writeEnd - writeStart;
            if (byteCount > std::numeric_limits<U64>::max() - preparedBytes) {
                ranges.clear();
                preparedBytes = 0;
                return -K_EFBIG;
            }
            KWritebackRange range;
            range.offset = writeStart;
            range.bytes.resize((size_t)byteCount);
            memcpy(range.bytes.data(),
                ramPageGet(data[(size_t)pageIndex]) + (size_t)(writeStart - pageStart),
                (size_t)byteCount);
            ranges.push_back(std::move(range));
            preparedBytes += byteCount;
        }
    } catch (const std::bad_alloc&) {
        ranges.clear();
        preparedBytes = 0;
        return -K_ENOMEM;
    } catch (const std::length_error&) {
        ranges.clear();
        preparedBytes = 0;
        return -K_ENOMEM;
    }
    return 0;
}

U32 MappedFileCache::flush(U64 offset, U64 len) {
    if (!len) {
        return 0;
    }
    if (len > std::numeric_limits<U64>::max() - offset) {
        return -K_EINVAL;
    }

    std::shared_ptr<KFile> target;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
        target = writeFile;
    }
    // A cache created only by private or read-only mappings has no writable
    // backing owner and cannot contain guest-visible shared writes.
    if (!target) {
        return 0;
    }

    struct FlushContext {
        MappedFileCache* cache;
        U64 offset;
        U64 len;
        U64 preparedBytes = 0;
    } context = {this, offset, len};
    KWritebackResult result = target->writeback(&context,
        [](void* opaque, std::vector<KFile::WritebackRange>& ranges) -> S32 {
            FlushContext& context = *static_cast<FlushContext*>(opaque);
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(context.cache->mutationMutex);
            {
                BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(context.cache->mutex);
                return context.cache->snapshotWritebackRangesLocked(
                    context.offset, context.len, ranges, context.preparedBytes);
            }
        });

    if (result.preparationError) {
        return (U32)result.preparationError;
    }
    if (result.ioError) {
        return (U32)result.ioError;
    }
    return result.bytesWritten == context.preparedBytes ? 0 : (U32)-K_EIO;
}

U32 MappedFileCache::retireMapping(bool& accountingCommitted) {
    accountingCommitted = false;
    std::shared_ptr<KFile> target;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
        target = writeFile ? writeFile : file;
    }
    if (!target) {
        return -K_EIO;
    }

    struct RetirementContext {
        MappedFileCache* cache;
        U64 preparedBytes = 0;
        bool accountingCommitted = false;
#ifdef __TEST
        std::shared_ptr<std::function<void()>> afterPreparation;
#endif
    } context = {this};
    KWritebackResult result = target->writeback(&context,
        [](void* opaque, std::vector<KFile::WritebackRange>& ranges) -> S32 {
            RetirementContext& context = *static_cast<RetirementContext*>(opaque);
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(context.cache->mutationMutex);
            S32 preparationResult = 0;
            {
                BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(context.cache->mutex);
                if (!context.cache->mappingLeaseCount) {
                    return -K_EINVAL;
                }
                if (context.cache->mappingLeaseCount == 1 && context.cache->writeFile) {
                    preparationResult = context.cache->snapshotWritebackRangesLocked(
                        0, context.cache->length, ranges, context.preparedBytes);
#ifdef __TEST
                    context.afterPreparation = context.cache->testAfterFinalRetirementPreparationHook;
#endif
                }
            }
#ifdef __TEST
            if (context.afterPreparation) {
                (*context.afterPreparation)();
            }
#endif
            return preparationResult;
        },
        [](void* opaque) noexcept {
            RetirementContext& context = *static_cast<RetirementContext*>(opaque);
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(context.cache->mutex);
            if (!context.cache->mappingLeaseCount) {
                kpanic("mapped file cache lease count underflow");
            }
            --context.cache->mappingLeaseCount;
#ifdef __TEST
            ++context.cache->testRetirementAccountingFinalizeCount;
#endif
            context.accountingCommitted = true;
        });
    accountingCommitted = context.accountingCommitted;

    if (result.preparationError) {
        return (U32)result.preparationError;
    }
    if (result.ioError) {
        return (U32)result.ioError;
    }
    return result.bytesWritten == context.preparedBytes ? 0 : (U32)-K_EIO;
}

void KMemory::shutdown() {
    KMemoryData::shutdown();
}

KMemory::KMemory(KProcess* process) : process(process) {
    data = new KMemoryData(this);    
}

KMemory::~KMemory() {
    if (data) {
#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED)
        std::vector<void*> jitOps;
        data->opCache.collectAllJitBlocks(jitOps);
        jitMemoryInvalidated(this, jitOps);
#elif defined(BOXEDWINE_JIT)
        jitMemoryInvalidated(this, {});
#endif
        delete data;
    }
    if (deleteOnNextLoop) {
        delete deleteOnNextLoop;
    }
}

void KMemory::cleanup() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    if (data) {
#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED)
        std::vector<void*> jitOps;
        data->opCache.collectAllJitBlocks(jitOps);
        jitMemoryInvalidated(this, jitOps);
#elif defined(BOXEDWINE_JIT)
        jitMemoryInvalidated(this, {});
#endif
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
    bool write = (prot & K_PROT_WRITE) != 0;
    // from https://man7.org/linux/man-pages/man2/mprotect.2.html
    // On some hardware architectures(e.g., i386), PROT_WRITE implies PROT_READ.
    bool read = (prot & K_PROT_READ) != 0 || write;
    bool exec = (prot & K_PROT_EXEC) != 0;
    U32 pageStart = addr >> K_PAGE_SHIFT;
    U32 pageCount = (U32)(((U64)len + K_PAGE_SIZE - 1) >> K_PAGE_SHIFT); // U64 to avoid overflow
    KFileDescriptorPtr fd;
    std::shared_ptr<KFile> file;
    MappedFilePtr mappedFile;
    bool fileMapValidated = false;
    bool mappedLeaseRetained = false;
    bool mappingCommitted = false;
    const bool fixedReplacement =
        (flags & K_MAP_FIXED) && !(flags & K_MAP_FIXED_NOREPLACE);
    std::vector<MappedFilePtr> replacementRetirements;

    if (0xFFFFFFFF - addr < len || len == 0) {
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
        const U64 firstFilePage = off >> K_PAGE_SHIFT;
        if (firstFilePage > K_MAX_MAPPED_FILE_CACHE_PAGE ||
            (U64)(pageCount - 1) > K_MAX_MAPPED_FILE_CACHE_PAGE - firstFilePage) {
            return -K_EINVAL;
        }
        if (!fd->canRead() || (!priv && (!fd->canWrite() && write))) {
            return -K_EACCES;
        }
        file = std::dynamic_pointer_cast<KFile>(fd->kobject);
        if (!file) {
            return -K_EACCES;
        }
        if (shared && fd->canWrite()) {
            // Positioned-write capability is retained by the live open node.
            // Reject before taking the memory lock or reserving any address so
            // an unavailable append-safe target needs no mmap unwind.
            if (!file->openFile->canWriteNativeAt()) {
                return -K_EIO;
            }
        }
    }
    if (fixedReplacement) {
        if (addr & (K_PAGE_SIZE - 1)) {
            return -K_EINVAL;
        }
        // Validate the replacement before removing the old mapping.  A file
        // mapper is allowed to reject this request, and MAP_FIXED must not
        // destroy the existing range when that validation fails.
        if (fd) {
            U32 result = fd->kobject->map(thread, addr, len, prot, flags, off);
            if (result) {
                return result;
            }
            fileMapValidated = true;
        }
    }

    // Cache lookup and lease accounting acquire filePos -> identity -> cache.
    // Complete them before taking the guest-memory lock so descriptor I/O,
    // which copies to or from guest memory while holding those file locks,
    // cannot form an ABBA cycle with mmap.
    if (fd) {
        try {
#ifdef __TEST
            if (testFailMappedFileRecordAllocation) {
                throw std::bad_alloc();
            }
#endif
            mappedFile = std::make_shared<MappedFile>();
            mappedFile->file = file;
            mappedFile->shared = shared;
            mappedFile->mayWrite = priv || fd->canWrite();
            mappedFile->systemCacheEntry = file->getOrCreateMappedFileCache(
                file->openFile->node->path, shared && fd->canWrite());
            if (!mappedFile->systemCacheEntry) {
                return -K_EIO;
            }
            mappedFile->lease = std::make_shared<MappedFileLease>(mappedFile->systemCacheEntry);
            file->retainMappedFileCacheLease(mappedFile->systemCacheEntry);
            mappedLeaseRetained = true;
        } catch (const std::bad_alloc&) {
            return -K_ENOMEM;
        } catch (const std::length_error&) {
            return -K_ENOMEM;
        }
    }

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

    U32 mappingResult = [&]() -> U32 {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);

        if (mappedFile) {
            try {
                {
                    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->mappedFilesMutex);
                    // unmap can add at most one right split when replacing one
                    // contiguous range. Reserve for that split and this record
                    // before a destructive MAP_FIXED replacement begins.
                    process->mappedFiles.reserve(process->mappedFiles.size() +
                        (fixedReplacement ? 2 : 1));
                }
            } catch (const std::bad_alloc&) {
                return -K_ENOMEM;
            } catch (const std::length_error&) {
                return -K_ENOMEM;
            }
        }

        bool reservedAddress = false;
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
            reservedAddress = true;
        }

        if (fd && !fileMapValidated) {
            U32 result = fd->kobject->map(thread, addr, len, prot, flags, off);
            if (result) {
                if (reservedAddress) {
                    data->setPagesInvalid(pageStart, pageCount);
                }
                return result;
            }
        }

        if (mappedFile) {
            mappedFile->address = pageStart << K_PAGE_SHIFT;
            mappedFile->len = ((U64)pageCount) << K_PAGE_SHIFT;
            mappedFile->offset = off;
            mappedFile->key = this->process->nextMappedFileIndex++;
            if (fixedReplacement) {
                U32 result = this->unmapLocked(addr, len, replacementRetirements);
                if (result) {
                    return result;
                }
            }
            {
                BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->mappedFilesMutex);
                // Capacity is already reserved. U32 hashing and shared_ptr
                // copy/assignment are non-throwing, so no fallible operation
                // remains after the replacement unmap.
                this->process->mappedFiles.set(mappedFile->key, mappedFile);
            }
            this->data->allocPages(thread, pageStart, pageCount, permissions, fildes, off, mappedFile);
        } else {
            if (fixedReplacement) {
                U32 result = this->unmapLocked(addr, len, replacementRetirements);
                if (result) {
                    return result;
                }
            }
            this->data->allocPages(thread, pageStart, pageCount, permissions, 0, 0, nullptr);
        }
        mappingCommitted = true;
        return addr;
    }();

    if (mappedLeaseRetained && !mappingCommitted) {
        bool retire = false;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->mappedFilesMutex);
            retire = mappedFile->lease->removePiece();
        }
        if (retire) {
            U32 retireResult = mappedFile->lease->retire();
            if ((S32)retireResult < 0 && !mappedFile->lease->isRetired()) {
                process->queuePendingMappedFileRetirement(mappedFile, (S32)retireResult);
            }
        }
    }
    if (!mappingCommitted) {
        return mappingResult;
    }

    for (const MappedFilePtr& mapping : replacementRetirements) {
        U32 result = mapping->lease->retire();
        if ((S32)result < 0) {
            if (!mapping->lease->isRetired()) {
                process->queuePendingMappedFileRetirement(mapping, (S32)result);
            }
        }
    }
    return mappingResult;
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

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    for (U32 i = pageStart; i < pageStart + pageCount; i++) {
        if (!isPageMapped(i)) {
            return -K_ENOMEM;
        }
    }
    if (write) {
        const U64 protectStart = (U64)pageStart << K_PAGE_SHIFT;
        const U64 protectEnd =
            protectStart + ((U64)pageCount << K_PAGE_SHIFT);
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->mappedFilesMutex);
        for (const auto& entry : process->mappedFiles) {
            const MappedFilePtr& mapping = entry.value;
            const U64 mappingStart = mapping->address;
            const U64 mappingEnd = mappingStart + mapping->len;
            if (!mapping->mayWrite &&
                mappingStart < protectEnd && mappingEnd > protectStart) {
                return -K_EACCES;
            }
        }
    }
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
    U64 roundedOldSize = ((U64)oldsize + K_PAGE_MASK) & ~(U64)K_PAGE_MASK;
    U64 roundedNewSize = ((U64)newsize + K_PAGE_MASK) & ~(U64)K_PAGE_MASK;
    constexpr U64 ADDRESS_SPACE_SIZE = 0x100000000ULL;
    if ((U64)oldaddress + roundedOldSize > ADDRESS_SPACE_SIZE ||
        (U64)oldaddress + roundedNewSize > ADDRESS_SPACE_SIZE) {
        return -K_EINVAL;
    }
    U32 oldPageCount = (U32)(roundedOldSize >> K_PAGE_SHIFT);
    U32 oldPageStart = oldaddress >> K_PAGE_SHIFT;
    U32 pageFlags = getPageFlags(oldPageStart);

    for (U32 i = 0; i < oldPageCount; i++) {
        if (getPageFlags(oldPageStart + i) != pageFlags) {
            return -K_EFAULT;
        }
    }
    if (roundedNewSize < roundedOldSize) {
        U32 result = this->unmap((U32)((U64)oldaddress + roundedNewSize),
            (U32)(roundedOldSize - roundedNewSize));
        if (result) {
            return result;
        }
        return oldaddress;
    } else if (roundedNewSize > roundedOldSize) {
        const U64 oldRangeStart = oldaddress;
        const U64 oldRangeEnd = oldRangeStart + roundedOldSize;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->mappedFilesMutex);
            for (const auto& entry : process->mappedFiles) {
                const MappedFilePtr& mapping = entry.value;
                const U64 mappingStart = mapping->address;
                const U64 mappingEnd = mappingStart + mapping->len;
                if (mappingStart < oldRangeEnd && mappingEnd > oldRangeStart) {
                    // Growing or moving a file-backed mapping must retain its
                    // cache, file offset, and lease. The generic mmap paths
                    // below cannot do that without the original descriptor,
                    // which may already be closed.
                    return -K_ENOMEM;
                }
            }
        }
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
        U32 oldEnd = (U32)((U64)oldaddress + roundedOldSize);
        U32 growth = (U32)(roundedNewSize - roundedOldSize);
        U32 result = this->mmap(thread, oldEnd, growth, prot, f | K_MAP_FIXED_NOREPLACE, -1, 0, true);
        if (result == oldEnd) {
            return oldaddress;
        }
        if ((flags & 1) != 0) { // MREMAP_MAYMOVE
            result = this->mmap(thread, 0, newsize, prot, f | K_MAP_ANONYMOUS, -1, 0);
            if ((S32)result < 0) {
                return result;
            }
            this->memcpy(result, oldaddress, oldsize);
            U32 unmapResult = this->unmap(oldaddress, (U32)roundedOldSize);
            if (unmapResult) {
                this->unmap(result, (U32)roundedNewSize);
                return unmapResult;
            }
            return result;
        }
        return -K_ENOMEM;
    }
    return oldaddress;
}

U32 KMemory::unmapLocked(U32 address, U32 len, std::vector<MappedFilePtr>& retirements) {
    if (!len || (address & K_PAGE_MASK)) {
        return -K_EINVAL;
    }
    U64 rangeStart = address;
    U64 rangeEnd = rangeStart + len;
    if (rangeEnd > 0x100000000ULL) {
        return -K_EINVAL;
    }
    rangeEnd = (rangeEnd + K_PAGE_MASK) & ~(U64)K_PAGE_MASK;
    if (rangeEnd > 0x100000000ULL) {
        return -K_EINVAL;
    }
    U32 pageStart = address >> K_PAGE_SHIFT;
    U32 pageCount = (U32)((rangeEnd - rangeStart) >> K_PAGE_SHIFT);
    struct PreparedSplit {
        MappedFilePtr original;
        MappedFilePtr right;
    };
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->mappedFilesMutex);
    std::vector<MappedFilePtr> mappings;
    std::vector<PreparedSplit> splits;
    bool preparedCodeInvalidation = false;
    try {
        mappings.reserve(process->mappedFiles.size());
        retirements.reserve(process->mappedFiles.size());
        splits.reserve(process->mappedFiles.size());
        for (const auto& entry : process->mappedFiles) {
            mappings.push_back(entry.value);
        }
        for (const MappedFilePtr& mapping : mappings) {
            U64 mappingStart = mapping->address;
            U64 mappingEnd = mappingStart + mapping->len;
            U64 overlapStart = std::max(rangeStart, mappingStart);
            U64 overlapEnd = std::min(rangeEnd, mappingEnd);
            if (mappingStart < overlapStart && overlapEnd < mappingEnd) {
#ifdef __TEST
                if (testFailMappedFileRecordAllocation) {
                    throw std::bad_alloc();
                }
#endif
                MappedFilePtr right = std::make_shared<MappedFile>(*mapping);
                right->address = (U32)overlapEnd;
                right->len = mappingEnd - overlapEnd;
                right->advanceFileOffset(overlapEnd - mappingStart);
                right->key = process->nextMappedFileIndex++;
                splits.push_back({mapping, right});
            }
        }
        for (U32 page = pageStart; page < pageStart + pageCount; ++page) {
            if (data->mmu[page].getPageType() == PageType::Code) {
                prepareCodeInvalidation((U32)rangeStart,
                    (U32)(rangeEnd - rangeStart));
                preparedCodeInvalidation = true;
                break;
            }
        }
        // Insertion can allocate. Do it before invalidating pages or changing
        // any live record so ENOMEM leaves the mapping whole.
        for (const PreparedSplit& split : splits) {
            process->mappedFiles.set(split.right->key, split.right);
        }
    } catch (const std::bad_alloc&) {
        discardPreparedCodeInvalidation();
        for (const PreparedSplit& split : splits) {
            process->mappedFiles.remove(split.right->key);
        }
        return -K_ENOMEM;
    } catch (const std::length_error&) {
        discardPreparedCodeInvalidation();
        for (const PreparedSplit& split : splits) {
            process->mappedFiles.remove(split.right->key);
        }
        return -K_ENOMEM;
    }

    this->data->setPagesInvalid(pageStart, pageCount, preparedCodeInvalidation);
    for (const MappedFilePtr& mapping : mappings) {
        U64 mappingStart = mapping->address;
        U64 mappingEnd = mappingStart + mapping->len;
        U64 overlapStart = std::max(rangeStart, mappingStart);
        U64 overlapEnd = std::min(rangeEnd, mappingEnd);
        if (overlapStart >= overlapEnd) {
            continue;
        }

        bool keepLeft = mappingStart < overlapStart;
        bool keepRight = overlapEnd < mappingEnd;
        if (!keepLeft && !keepRight) {
            process->mappedFiles.remove(mapping->key);
            if (mapping->lease && mapping->lease->removePiece()) {
                retirements.push_back(mapping);
            }
        } else if (keepLeft && keepRight) {
            MappedFilePtr right;
            for (const PreparedSplit& split : splits) {
                if (split.original == mapping) {
                    right = split.right;
                    break;
                }
            }
            if (!right) {
                kpanic("prepared mapped-file split was not found");
            }
            mapping->len = overlapStart - mappingStart;
            if (mapping->lease) {
                mapping->lease->addPiece();
            }

            U32 rightStartPage = right->address >> K_PAGE_SHIFT;
            U32 rightPageCount = (U32)(right->len >> K_PAGE_SHIFT);
            for (U32 page = rightStartPage; page < rightStartPage + rightPageCount; ++page) {
                MMU& mmu = data->mmu[page];
                if (mmu.getPageType() == PageType::File && mmu.ramIndex == mapping->key) {
                    mmu.setPage(this, page, PageType::File, RamPage{right->key});
                    data->onPageChanged(page);
                }
            }
        } else if (keepLeft) {
            mapping->len = overlapStart - mappingStart;
        } else {
            mapping->address = (U32)overlapEnd;
            mapping->len = mappingEnd - overlapEnd;
            mapping->advanceFileOffset(overlapEnd - mappingStart);
        }
    }
    return 0;
}

U32 KMemory::unmap(U32 address, U32 len) {
    process->retryPendingMappedFileRetirements();
    std::vector<MappedFilePtr> retirements;
    U32 result;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
        result = unmapLocked(address, len, retirements);
    }
    if (result) {
        return result;
    }
    for (const MappedFilePtr& mapping : retirements) {
        U32 retireResult = mapping->lease->retire();
        if ((S32)retireResult < 0) {
            if (!mapping->lease->isRetired()) {
                process->queuePendingMappedFileRetirement(mapping, (S32)retireResult);
            }
        }
    }
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

void KMemory::preflightWrite(U32 address, U32 len) {
    for (U32 i = 0; i < len; ++i) {
        U32 current = address + i;
        U32 flags = getPageFlags(current >> K_PAGE_SHIFT);
        if (flags & PAGE_WRITE) {
            continue;
        }
        KThread* thread = KThread::currentThread();
        if (flags) {
            thread->seg_access(current, false, true);
        } else {
            thread->seg_mapper(current, false, true);
        }
    }
}

void KMemory::execvReset(bool cloneVM) {
#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED)
    // Full-owner invalidation retires every broker/table slot through the MT
    // module registry. Do not sweep DecodedOps first: exec can run inside a
    // compiled frame, and CLONE_VM detach still leaves the old op cache live
    // in the other wrapper.
    jitMemoryInvalidated(this, {});
#elif defined(BOXEDWINE_JIT)
    jitMemoryInvalidated(this, {});
#endif
    if (!cloneVM) {
        data->execvReset();
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
    if (dest + len < dest || src + len < src) {
        kpanic("integer overflow in memcpy");
	}
	if (dest < src) {
        for (U32 i = 0; i < len; i++, src++, dest++) {
            writeb(dest, readb(src));
        }
    } else {
        dest += len - 1;
        src += len - 1;
        for (U32 i = 0; i < len; i++, src--, dest--) {
            writeb(dest, readb(src));
        }
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
    if (len == 0) {
        return;
    }
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
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    if (data) {
        data->opCache.threadCleanup(threadId);
    }
}

void KMemory::clearOpCache() {
#if defined(BOXEDWINE_JIT)
    // Collect installed entries before the cache frees the DecodedOps, then
    // notify the selected backend so it can also discard unpublished work.
    std::vector<void*> jitOps;
    data->opCache.collectAllJitBlocks(jitOps);
    jitMemoryInvalidated(this, jitOps);
#endif
    data->opCache.clear();
}

void KMemory::prepareCodeInvalidation(U32 address, U32 len) {
    if (codeInvalidationPrepared) {
        kpanic("nested code invalidation preparation");
    }

    preparedCodeBlocks.clear();
    preparedCodeRemovalRanges.clear();
#if defined(BOXEDWINE_JIT)
    preparedBackendDecodedOps.clear();
    preparedBackendJitOps.clear();
    preparedBackendInvalidations.clear();
#endif
    U32 codeMemoryFreeCount = 0;

#ifdef __TEST
    if (testFailCodeInvalidationPreparation) {
        throw std::bad_alloc();
    }
#endif

    try {
#if defined(BOXEDWINE_JIT)
        struct CollectContext {
            std::vector<PreparedCodeInvalidationBlock>* blocks;
        } context = {&preparedCodeBlocks};
        data->opCache.iterateOps(address, len,
            [](U32 opAddress, DecodedOp* op, void* opaque) {
                if (!op->blockStart) {
                    return;
                }
                CollectContext& context = *static_cast<CollectContext*>(opaque);
                DecodedOp* blockStart = op->blockStart;
                for (const PreparedCodeInvalidationBlock& block : *context.blocks) {
                    if (block.blockStart == blockStart) {
                        return;
                    }
                }
                U32 prefixLen = 0;
                DecodedOp* current = blockStart;
                while (current != op) {
                    prefixLen += current->len;
                    current = current->next;
                }
                context.blocks->push_back(
                    {opAddress - prefixLen, blockStart, {}, {}, nullptr});
            }, &context);

        for (PreparedCodeInvalidationBlock& block : preparedCodeBlocks) {
            DecodedOp* current = block.blockStart;
            U32 blockOpCount = block.blockStart->blockOpCount;
            preparedCodeRemovalRanges.push_back(
                {block.address, block.blockStart->blockLen});
            if (block.blockStart->pfnJitCode && jitUsesCodeMemory()) {
                block.codeMemoryToFree = (void*)block.blockStart->pfnJitCode;
                ++codeMemoryFreeCount;
            }
            block.decodedOps.reserve(blockOpCount);
            block.jitOps.reserve(blockOpCount);
            for (U32 i = 0; i < blockOpCount; ++i) {
                block.decodedOps.push_back(current);
                if (current->pfnJitCode) {
                    block.jitOps.push_back((void*)current->pfnJitCode);
                }
                current = current->next;
            }
        }
        if (jitAggregatesPreparedCodeInvalidation()) {
            for (const PreparedCodeInvalidationBlock& block :
                    preparedCodeBlocks) {
                if (block.decodedOps.size() >
                        preparedBackendDecodedOps.max_size() -
                            preparedBackendDecodedOps.size() ||
                        block.jitOps.size() >
                        preparedBackendJitOps.max_size() -
                            preparedBackendJitOps.size()) {
                    throw std::length_error(
                        "prepared backend invalidation is too large");
                }
                preparedBackendDecodedOps.insert(
                    preparedBackendDecodedOps.end(),
                    block.decodedOps.begin(), block.decodedOps.end());
                preparedBackendJitOps.insert(
                    preparedBackendJitOps.end(),
                    block.jitOps.begin(), block.jitOps.end());
            }
            if (!preparedBackendDecodedOps.empty() ||
                    !preparedBackendJitOps.empty()) {
                preparedBackendInvalidations.push_back(
                    prepareJitCodeInvalidation(this,
                        preparedBackendDecodedOps, preparedBackendJitOps));
            }
        } else {
            preparedBackendInvalidations.reserve(preparedCodeBlocks.size());
            for (const PreparedCodeInvalidationBlock& block :
                    preparedCodeBlocks) {
                preparedBackendInvalidations.push_back(
                    prepareJitCodeInvalidation(
                        this, block.decodedOps, block.jitOps));
            }
        }
#endif
        preparedCodeRemovalRanges.push_back({address, len});
        data->opCache.prepareRemoveRanges(preparedCodeRemovalRanges);
        if (codeMemoryFreeCount >
            pendingCodeMemoryFrees.max_size() - pendingCodeMemoryFrees.size()) {
            throw std::length_error("pending code-memory frees are too large");
        }
        pendingCodeMemoryFrees.reserve(
            pendingCodeMemoryFrees.size() + codeMemoryFreeCount);
        codeInvalidationPrepared = true;
    } catch (...) {
        data->opCache.finishPreparedRemove();
#if defined(BOXEDWINE_JIT)
        preparedBackendInvalidations.clear();
        preparedBackendDecodedOps.clear();
        preparedBackendJitOps.clear();
#endif
        preparedCodeBlocks.clear();
        preparedCodeRemovalRanges.clear();
        throw;
    }
}

void KMemory::commitPreparedCodeInvalidation() {
    if (!codeInvalidationPrepared) {
        kpanic("code invalidation was not prepared");
    }

#if defined(BOXEDWINE_JIT)
    for (PreparedCodeInvalidationBlock& block : preparedCodeBlocks) {
        DecodedOp* blockOp = block.blockStart;
        DecodedOp* nextOp = blockOp;
        U32 blockOpCount = blockOp->blockOpCount;
        for (U32 i = 0; i < blockOpCount; ++i) {
            if (nextOp->blockStart != blockOp && nextOp->inst != Done) {
                kpanic("KMemory::commitPreparedCodeInvalidation nextOp->blockStart");
            }
#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED)
            if (nextOp->flags2 & OP_FLAG2_WASM_JIT_RELOC_HAZARD) {
                __atomic_exchange_n(&nextOp->pfnJitCode, nullptr, __ATOMIC_SEQ_CST);
            } else {
                nextOp->pfnJitCode = nullptr;
            }
#endif
            nextOp->blockStart = nullptr;
            nextOp->blockOpCount = 0;
            nextOp->blockLen = 0;
            if (nextOp->runCount) {
                nextOp->runCount = 1;
            }
            nextOp->pfn = NormalCPU::getFunctionForOp(nextOp);
            nextOp->flags &= ~OP_FLAG_JIT;
#if !defined(BOXEDWINE_WASM_JIT) || !defined(BOXEDWINE_MULTI_THREADED)
            nextOp->pfnJitCode = nullptr;
#endif
            nextOp->jitLen = 0;
            nextOp = nextOp->next;
        }
    }
    for (PreparedJitCodeInvalidation& invalidation :
            preparedBackendInvalidations) {
        invalidation.commit();
    }
#endif

    for (const auto& range : preparedCodeRemovalRanges) {
        data->opCache.remove(range.first, range.second, false);
    }
    data->opCache.finishPreparedRemove();

#if defined(BOXEDWINE_JIT)
    for (const PreparedCodeInvalidationBlock& block : preparedCodeBlocks) {
        if (!block.codeMemoryToFree) {
            continue;
        }
        try {
            data->codeMemory.free(block.codeMemoryToFree);
        } catch (...) {
            // Capacity for every prepared free was reserved before mutation.
            pendingCodeMemoryFrees.push_back(block.codeMemoryToFree);
        }
    }
#endif
    codeInvalidationPrepared = false;
#if defined(BOXEDWINE_JIT)
    preparedBackendInvalidations.clear();
    preparedBackendDecodedOps.clear();
    preparedBackendJitOps.clear();
#endif
    preparedCodeBlocks.clear();
    preparedCodeRemovalRanges.clear();
}

void KMemory::discardPreparedCodeInvalidation() {
    if (!codeInvalidationPrepared) {
        return;
    }
    data->opCache.finishPreparedRemove();
    codeInvalidationPrepared = false;
#if defined(BOXEDWINE_JIT)
    preparedBackendInvalidations.clear();
    preparedBackendDecodedOps.clear();
    preparedBackendJitOps.clear();
#endif
    preparedCodeBlocks.clear();
    preparedCodeRemovalRanges.clear();
}

void KMemory::retryPendingCodeMemoryFrees() {
#if defined(BOXEDWINE_JIT)
    while (!pendingCodeMemoryFrees.empty()) {
        try {
            data->codeMemory.free(pendingCodeMemoryFrees.back());
            pendingCodeMemoryFrees.pop_back();
        } catch (...) {
            return;
        }
    }
#endif
}

void* KMemory::allocCodeMemory(U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex)
    retryPendingCodeMemoryFrees();
    return data->codeMemory.alloc(len);
}

bool KMemory::isCode(void* p) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex)
    return data->codeMemory.containsAddress(p);
}

#if defined(BOXEDWINE_JIT)
void KMemory::clearJit(DecodedOp* op) {
    DecodedOp* nextOp = op->blockStart;
    U32 blockOpCount = nextOp->blockOpCount;
    void* start = nextOp->pfnJitCode;

    for (U32 i = 0; i < blockOpCount; i++) {
        nextOp->flags &= ~OP_FLAG_JIT;
        nextOp->pfn = NormalCPU::getFunctionForOp(nextOp);
        nextOp->pfnJitCode = nullptr;
        nextOp->jitLen = 0;
        nextOp->blockStart = nullptr;
        nextOp->blockOpCount = 0;
        nextOp->blockLen = 0;
        nextOp->runCount = 0;
        nextOp = nextOp->next;
    }
    if (start && jitUsesCodeMemory()) {
        data->codeMemory.free(start);
    }
}

void KMemory::removeCodeBlock(U32 address, DecodedOp* op, bool clearOps) {
    if (!op || !op->blockStart) {
        return;
    }
    DecodedOp* blockOp = op->blockStart;
    U32 blockLen = blockOp->blockLen;
    U32 blockOpCount = blockOp->blockOpCount;
    DecodedOp* nextOp = blockOp;
    KThread* thread = KThread::currentThread();
    DecodedOp* currentOp = thread->memory->getDecodedOp(thread->cpu->getEipAddress());
    void* pMem = (void*)blockOp->pfnJitCode;
    U32 jitLen = 0;

    std::vector<void*> jitOps;
    std::vector<DecodedOp*> decodedOps;
    decodedOps.reserve(blockOpCount);
    jitOps.reserve(blockOpCount);
    for (U32 i = 0; i < blockOpCount; ++i) {
        if (nextOp->blockStart != blockOp && nextOp->inst != Done) {
            kpanic("KMemory::removeCodeBlock nextOp->blockStart");
        }
        if (nextOp->pfnJitCode) {
            jitOps.push_back(nextOp->pfnJitCode);
        }
        decodedOps.push_back(nextOp);
        nextOp = nextOp->next;
    }
    PreparedJitCodeInvalidation backendInvalidation =
        prepareJitCodeInvalidation(this, decodedOps, jitOps);

    nextOp = blockOp;
    for (U32 i = 0; i < blockOpCount; i++) {
        if (nextOp->blockStart != blockOp && nextOp->inst != Done) {
            kpanic("KMemory::removeCodeBlock nextOp->blockStart");
        }
#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED)
        void* jitCode;
        if (nextOp->flags2 & OP_FLAG2_WASM_JIT_RELOC_HAZARD) {
            jitCode = __atomic_exchange_n(
                &nextOp->pfnJitCode, nullptr, __ATOMIC_SEQ_CST);
        } else {
            // Relocation-free MT functions have no reclaimable shared state.
            // Their worker-local table slot is cleared by an ordered worker
            // message, so the branch's historical benign-stale dispatch is
            // sufficient and keeps ordinary JIT calls free of wasm atomics.
            jitCode = nextOp->pfnJitCode;
            nextOp->pfnJitCode = nullptr;
        }
#else
        void* jitCode = nextOp->pfnJitCode;
#endif
        nextOp->blockStart = nullptr;
        nextOp->blockOpCount = 0;
        nextOp->blockLen = 0;
        if (nextOp->runCount) {
            nextOp->runCount = 1; // so that tracing won't stub it out again
        }
        nextOp->pfn = NormalCPU::getFunctionForOp(nextOp);
        nextOp->flags &= ~OP_FLAG_JIT;
        jitLen += nextOp->jitLen;
#if !defined(BOXEDWINE_WASM_JIT) || !defined(BOXEDWINE_MULTI_THREADED)
        nextOp->pfnJitCode = nullptr;
#endif
        nextOp->jitLen = 0;
        nextOp = nextOp->next;
    }
    backendInvalidation.commit();
    if (clearOps) {
        data->opCache.remove(address, blockLen, false);
    }        
    if (pMem && jitUsesCodeMemory()) {
        data->codeMemory.free(pMem);
    }
#ifdef _DEBUG1
    klog_fmt("removed active code block eip = %x - %x host %llx - %llx", thread->cpu->getEipAddress(), thread->cpu->getEipAddress() + blockLen, (U64)pMem, (U64)pMem + jitLen);
#endif
}
#endif

#if defined(BOXEDWINE_JIT)
static void opCallback(U32 address, DecodedOp* op, void* p) {
    KMemory* memory = (KMemory*)p;
    if (op->blockStart) {
        // Normal and JIT cpu's don't need to track eip on an op except for this case, but I don't think it's really worth increasing
        // the size of DecodedOp just for this
        DecodedOp* nextOp = op->blockStart;
        U32 len = 0;
        while (nextOp != op) {
            len += nextOp->len;
            nextOp = nextOp->next;
        }
        U32 blockAddress = address - len;
        memory->removeCodeBlock(blockAddress, op->blockStart, true);
    }
}
#endif
void KMemory::removeCode(KThread* thread, U32 address, U32 len, bool becauseOfWrite) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex)
#if defined(BOXEDWINE_JIT)
    data->opCache.iterateOps(address, len, opCallback, this);
    data->opCache.remove(address, len, becauseOfWrite);
#else
    data->opCache.remove(address, len, becauseOfWrite);
#endif
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
	// The returned guest address includes the host pointer's low-page offset.
    // Align back to the first native data page, then include both guards.
    U64 dataStart = (U64)address & ~(U64)K_PAGE_MASK;
    if (dataStart < K_PAGE_SIZE) {
        kwarn_fmt("Ignoring invalid native unmap address %.8X", address);
        return;
    }
    U64 dataBytes = (U64)(address & K_PAGE_MASK) + size;
    U64 dataPageCount = (dataBytes + K_PAGE_MASK) >> K_PAGE_SHIFT;
    U64 totalBytes = (dataPageCount + 2) << K_PAGE_SHIFT;
    U64 guardStart = dataStart - K_PAGE_SIZE;
    if (guardStart + totalBytes > 0x100000000ULL || totalBytes > std::numeric_limits<U32>::max()) {
        kwarn_fmt("Ignoring overflowing native unmap address %.8X size %u", address, size);
        return;
    }
    U32 result = unmap((U32)guardStart, (U32)totalBytes);
    if (result) {
        kwarn_fmt("Native unmap failed for %.8X size %u: %d", address, size, (S32)result);
    }
}

U32 KMemory::mapNativeMemory(void* hostAddress, U32 size) {
    U32 result = 0;
    U32 offset = (U32)(((RAM_TYPE)hostAddress) & 0xFFF);
    // ramPageAllocNative expect host address to be aligned to a page
    if (offset) {
        hostAddress = (U8*)hostAddress - offset;
    }
    U64 adjustedSize = (U64)size + offset;
    U64 pageCount64 = (adjustedSize + K_PAGE_MASK) >> K_PAGE_SHIFT;
    if (pageCount64 + 2 > std::numeric_limits<U32>::max()) {
        return 0;
    }
    U32 pageCount = (U32)pageCount64;
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
        data->onPageChanged(result + i);
        ramPageRelease(ramPage); // setPage retains
    }
    return (result << K_PAGE_SHIFT) + offset;
}

U32 KMemory::ensureContinuousNative_unsafe(U32 page, U32 pageCount) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    U8* ram = getRamPtr(page << K_PAGE_SHIFT, K_PAGE_SIZE, false);
    bool isContinuous = true;
    U32 chunks = (pageCount + (64 * 1024) - 1) / (64 * 1024);

    // :TODO: what if ram == null

    for (U32 i = 1; i < chunks * 16; i++) {
        U8* pageRam = getRamPtr((page + i) << K_PAGE_SHIFT, K_PAGE_SIZE, false);
        if (ram != pageRam - K_PAGE_SIZE * i) {
            isContinuous = false;
            break;
        }
    }
    if (isContinuous) {
        return pageCount * K_PAGE_SIZE;
    }    
    chunks *= 4;
    U8* nativeMemory = Platform::alloc64kBlock(chunks);
    U32 allocatedPages = chunks * 8;
    U32 validPages = 0;
    for (U32 i = 0; i < allocatedPages; i++) {
        if (canRead(i + page)) {
            validPages++;
        } else {
            break;
        }
    }
    // :TODO: not thread safe, what if another thread modifies the memory between memcpy and setPage
    memcpy(nativeMemory, (page << K_PAGE_SHIFT), (validPages << K_PAGE_SHIFT));
    RamPage ramPage = ramPageAllocNativeContinuous(nativeMemory, chunks * 8);
    klog_fmt("ensureContinuousNative %d pages", (U32)(chunks * 8));
    for (U32 i = 0; i < validPages; i++) {
        MMU& mmu = data->mmu[page + i];
        mmu.setPage(this, page + i, mmu.getPageType(), ramPage);
        data->onPageChanged(page + i);
        ramPageRelease(ramPage); // setPage retains
        ramPage.value++;
    }
    // :TODO: maybe figure out a way to release the memory when all the ram pages are unmapped rather than waiting for the process to exit?
    nativeContinuousMemory.push_back(std::make_shared<NativeContinuousMemory>(nativeMemory, page << K_PAGE_SHIFT, allocatedPages << K_PAGE_SHIFT));
    return validPages * K_PAGE_SIZE;
}

NativeContinuousMemory::~NativeContinuousMemory() {
    Platform::releaseNativeMemory(p, len);
}

U64 KMemory::readq(U32 address) {
#if !defined(UNALIGNED_MEMORY)
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

void KMemory::updateDebugMemoryWriteTrapActive() {
    bool active = false;
    if (this->process) {
        this->process->iterateThreads([this, &active](KThread* thread) {
            if (thread && thread->memory == this && thread->hasMemoryWriteBreakpointEnabled()) {
                active = true;
                return false;
            }
            return true;
        });
    }
    this->debugMemoryWriteTrapActive.store(active, std::memory_order_relaxed);
}

void KMemory::checkDebugTrapOnMemoryWriteSlow(U32 address, U32 len) {
    KThread* thread = KThread::currentThread();
    if (thread && thread->memory == this && !thread->inSignal && thread->hasMemoryWriteBreakpointEnabled()) {
        thread->checkDebugTrapOnMemoryWrite(address, len);
    }
}

U32 KMemory::readd(U32 address) {
	return readdInline(address);
}

U16 KMemory::readw(U32 address) {
    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
#if !defined(UNALIGNED_MEMORY)
        MMU& mmu = data->mmu[index];
        if (mmu.canReadRam) {
            return *(U16*)(&(ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF]));
        }
#endif
        return data->mmu[index].getPage()->readw(&data->mmu[index], address);
    }
    return readb(address) | (readb(address + 1) << 8);
}

U8 KMemory::readb(U32 address) {
    int index = address >> 12;
    MMU& mmu = data->mmu[index];
    if (mmu.canReadRam) {
        return ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF];
    }
    return data->mmu[index].getPage()->readb(&data->mmu[index], address);
}

void KMemory::writeq(U32 address, U64 value) {
#if !defined(UNALIGNED_MEMORY)
    if ((address & 0xFFF) < 0xFF9) {
        int index = address >> 12;
        MMU& mmu = data->mmu[index];
        if (mmu.canWriteRam) {
            *(U64*)(&(ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF])) = value;
            checkDebugTrapOnMemoryWrite(address, 8);
            return;
        }
    }
#endif
    if ((address & K_PAGE_MASK) > K_PAGE_SIZE - 8) {
        preflightWrite(address, 8);
    }
    writed(address, (U32)value); writed(address + 4, (U32)(value >> 32));
}

void KMemory::writed(U32 address, U32 value) {
    writedInline(address, value);
}

void KMemory::writew(U32 address, U16 value) {
    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
#if !defined(UNALIGNED_MEMORY)
        MMU& mmu = data->mmu[index];
        if (mmu.canWriteRam)
            *(U16*)(&(ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF])) = value;
        else
#endif
            data->mmu[index].getPage()->writew(&data->mmu[index], address, value);
        checkDebugTrapOnMemoryWrite(address, 2);
    } else {
        preflightWrite(address, 2);
        writeb(address, (U8)value);
        writeb(address + 1, (U8)(value >> 8));
    }
}

void KMemory::writeb(U32 address, U8 value) {
    int index = address >> 12;
    MMU& mmu = data->mmu[index];
    if (mmu.canWriteRam) {
        ramPageGet((RamPage)mmu.ramIndex)[address & 0xFFF] = value;
    } else {
        data->mmu[index].getPage()->writeb(&data->mmu[index], address, value);
    }
    checkDebugTrapOnMemoryWrite(address, 1);
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
    cloneLocked(from, vfork);
}

void KMemory::cloneLocked(KMemory* from, bool vfork) {
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
