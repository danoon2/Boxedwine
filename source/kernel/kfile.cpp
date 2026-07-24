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

void KWritebackResult::recordIo(U64 bytes, S32 error) {
    if (bytes > std::numeric_limits<U64>::max() - bytesWritten) {
        bytesWritten = std::numeric_limits<U64>::max();
        if (!ioError) {
            ioError = -K_EFBIG;
        }
    } else {
        bytesWritten += bytes;
    }
    if (error && !ioError) {
        ioError = error;
    }
}

KFile::KFile(FsOpenNode* openFile) : KObject(KTYPE_FILE) {
    this->openFile = openFile;
}

KFile::~KFile() {
    delete this->openFile;
}

void KFile::setBlocking(bool blocking) {
    if (blocking) {
        kdebug("KFile::setBlocking not implemented");
    }
}

bool KFile::isBlocking() {
    return false;
}

void KFile::setAsync(bool isAsync) {
    this->openFile->setAsync(isAsync);
}

bool KFile::isAsync() {
    return this->openFile->isAsync();
}

void KFile::unlockAll(U32 pid) {
    FsOpenNode* openNode = this->openFile;
    std::shared_ptr<FsNode> node = openNode->node;
    node->unlockAll(pid);
}

KFileLock* KFile::getLock(KFileLock* lock) {
    FsOpenNode* openNode = this->openFile;
    std::shared_ptr<FsNode> node = openNode->node;
    return node->getLock(lock, true);
}

U32 KFile::setLock(KFileLock* lock, bool wait) {    
    if (lock->l_type == K_F_UNLCK) {
        this->openFile->node->unlock(lock);
    } else {
        if (wait) {
            return this->openFile->node->addLockAndWait(lock, true);
        }
        else {
            return this->openFile->node->addLock(lock);
        }
    }
    return 0;
}

bool KFile::isOpen() {
    return true;
}

bool KFile::isReadReady() {
    return this->openFile->isReadReady();
}

bool KFile::isWriteReady() {
    return this->openFile->isWriteReady();
}

void KFile::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    this->openFile->waitForEvents(parentCondition, events);
}

U32 KFile::write(KThread* thread, U32 buffer, U32 len) {
    std::shared_ptr<MappedFileCache> cache;
    U32 result;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    std::shared_ptr<FsFileIdentity> identity = openFile->node->getFileIdentity();
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX_NR(identity->mutationOperationMutex);
        cache = KSystem::getFileCache(identity);
        result = this->openFile->write(thread, buffer, len, cache);
    }
    return result;
}

U32 KFile::writeNative(U8* buffer, U32 len) {
    std::shared_ptr<MappedFileCache> cache;
    U32 result;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    std::shared_ptr<FsFileIdentity> identity = openFile->node->getFileIdentity();
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX_NR(identity->mutationOperationMutex);
        result = this->openFile->writeNative(buffer, len);
        if ((S32)result > 0) {
            S64 end = this->openFile->getFilePointer();
#ifdef __TEST
            if (identity->testAfterBackingMutationBeforeCacheNotification) {
                identity->testAfterBackingMutationBeforeCacheNotification();
            }
#endif
            cache = KSystem::getFileCache(identity);
            if (cache) {
                cache->updateWrite(end - result, buffer, result);
            }
        }
    }
    return result;
}

U32 KFile::read(KThread* thread, U32 buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    return this->openFile->read(thread, buffer, len);
}

U32 KFile::readNative(U8* buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    S64 offset = this->openFile->getFilePointer();
    U32 result = this->openFile->readNative(buffer, len);
    if ((S32)result > 0) {
        std::shared_ptr<MappedFileCache> cache = KSystem::getFileCache(openFile->node->getFileIdentity());
        if (cache) {
            cache->overlayRead(offset, buffer, result);
        }
    }
    return result;
}

U32 KFile::stat(KProcess* process, U32 address, bool is64) {
    FsOpenNode* openNode = this->openFile;
    std::shared_ptr<FsNode> node = openNode->node;
    U64 len = (U64)openNode->length();

    KSystem::writeStat(process, node->path, address, is64, 1, node->getId(), node->getMode(), node->rdev, len, FS_BLOCK_SIZE, (len + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE, node->lastAccessed(), node->lastAccessedNano(), node->lastModified(), node->lastModifiedNano(), node->lastStatusChanged(), node->lastStatusChangedNano(), node->getHardLinkCount());
    return 0;
}

U32 KFile::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return this->openFile->map(thread, address, len, prot, flags, off);
}

bool KFile::canMap() {
    return this->openFile->canMap();
}

BString KFile::selfFd() {
    return this->openFile->node->path;
}

S64 KFile::seek(S64 pos) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    return this->openFile->seek(pos);
}

S64 KFile::getPos() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    return this->openFile->getFilePointer();
}

U32 KFile::ioctl(KThread* thread, U32 request) {
    return this->openFile->ioctl(thread, request);
}

bool KFile::supportsLocks() {
    return true;
}

S64 KFile::length() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    return this->openFile->length();
}

U32 KFile::pread(KThread* thread, U32 buffer, S64 offset, U32 len) {
    if (offset < 0) {
        return -K_EINVAL;
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    S64 previousOffset = this->openFile->getFilePointer();
    if (previousOffset < 0) {
        return -K_EIO;
    }
    if (this->openFile->seek(offset) < 0) {
        return -K_EINVAL;
    }
    U32 result = this->openFile->read(thread, buffer, len);
    if (this->openFile->seek(previousOffset) < 0) {
        return -K_EIO;
    }
    return result;
}

U32 KFile::pwrite(KThread* thread, U32 buffer, S64 offset, U32 len) {
    if (offset < 0) {
        return -K_EINVAL;
    }
    std::shared_ptr<MappedFileCache> cache;
    U32 result;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    std::shared_ptr<FsFileIdentity> identity = openFile->node->getFileIdentity();
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX_NR(identity->mutationOperationMutex);
        S64 previousOffset = this->openFile->getFilePointer();
        if (this->openFile->seek(offset) < 0) {
            return -K_EINVAL;
        }
        cache = KSystem::getFileCache(identity);
        result = this->openFile->write(thread, buffer, len, cache);
        this->openFile->seek(previousOffset);
    }
    return result;
}

U32 KFile::preadNative(U8* buffer, S64 offset, U32 len) {
    if (offset < 0) {
        return -K_EINVAL;
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    S64 previousOffset = this->openFile->getFilePointer();
    if (previousOffset < 0) {
        return -K_EIO;
    }
    if (this->openFile->seek(offset) < 0) {
        return -K_EINVAL;
    }
    U32 result = this->openFile->readNative(buffer, len);
    bool restoreFailed = this->openFile->seek(previousOffset) < 0;
    if ((S32)result > 0) {
        std::shared_ptr<MappedFileCache> cache = KSystem::getFileCache(openFile->node->getFileIdentity());
        if (cache) {
            cache->overlayRead(offset, buffer, result);
        }
    }
    if (restoreFailed) {
        return -K_EIO;
    }
    return result;
}

U32 KFile::pwriteNative(U8* buffer, S64 offset, U32 len) {
    std::shared_ptr<MappedFileCache> cache;
    U32 result;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    std::shared_ptr<FsFileIdentity> identity = openFile->node->getFileIdentity();
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX_NR(identity->mutationOperationMutex);
        S64 previousOffset = this->openFile->getFilePointer();
        this->openFile->seek(offset);
        result = this->openFile->writeNative(buffer, len);
        if ((S32)result > 0) {
            S64 end = this->openFile->getFilePointer();
#ifdef __TEST
            if (identity->testAfterBackingMutationBeforeCacheNotification) {
                identity->testAfterBackingMutationBeforeCacheNotification();
            }
#endif
            cache = KSystem::getFileCache(identity);
            if (cache) {
                cache->updateWrite(end - result, buffer, result);
            }
        }
        this->openFile->seek(previousOffset);
    }
    return result;
}

U32 KFile::preadNativeUncached(U8* buffer, S64 offset, U32 len) {
    if (offset < 0) {
        return -K_EINVAL;
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    S64 previousOffset = this->openFile->getFilePointer();
    if (previousOffset < 0) {
        return -K_EIO;
    }
    if (this->openFile->seek(offset) < 0) {
        return -K_EINVAL;
    }
    U32 result = this->openFile->readNative(buffer, len);
    if (this->openFile->seek(previousOffset) < 0) {
        return -K_EIO;
    }
    return result;
}

KWritebackResult KFile::writeback(void* context, PrepareWriteback prepare,
    CommitWriteback commit) {
    // Owned snapshots deliberately outlive both guards. In particular, any
    // future destructor work cannot recursively enter the identity gate.
    std::vector<WritebackRange> ranges;
    KWritebackResult result;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    std::shared_ptr<FsFileIdentity> identity = openFile->node->getFileIdentity();
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX_NR(identity->mutationOperationMutex);
        try {
            result.preparationError = prepare(context, ranges);
        } catch (const std::bad_alloc&) {
            result.preparationError = -K_ENOMEM;
        } catch (const std::length_error&) {
            result.preparationError = -K_ENOMEM;
        }
        if (result.preparationError) {
            return result;
        }
        for (WritebackRange& range : ranges) {
            U64 rangeProgress = 0;
            while (rangeProgress < range.bytes.size()) {
                if (range.offset > std::numeric_limits<U64>::max() - rangeProgress) {
                    result.recordIo(0, -K_EFBIG);
                    return result;
                }
                U32 requested = (U32)std::min<U64>(
                    range.bytes.size() - rangeProgress, std::numeric_limits<U32>::max());
                FsWriteResult writeResult = this->openFile->writeNativeAt(
                    range.bytes.data() + (size_t)rangeProgress,
                    range.offset + rangeProgress, requested);
                result.recordIo(writeResult.bytesWritten, writeResult.error);
                rangeProgress += writeResult.bytesWritten;
                if (writeResult.error || writeResult.bytesWritten != requested) {
                    return result;
                }
            }
        }
        if (commit) {
            commit(context);
        }
    }
    return result;
}

U32 KFile::setLength(U64 length) {
    if (length > (U64)std::numeric_limits<S64>::max()) {
        return -K_EINVAL;
    }
    if (openFile->node->type == FsNode::Type::Memory && length > (U64)std::numeric_limits<U32>::max()) {
        return -K_EFBIG;
    }
    std::shared_ptr<MappedFileCache> cache;
    U32 result = 0;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    std::shared_ptr<FsFileIdentity> identity = openFile->node->getFileIdentity();
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX_NR(identity->mutationOperationMutex);
        if (!openFile->setLength((S64)length)) {
            return -K_EIO;
        }
#ifdef __TEST
        if (identity->testAfterBackingMutationBeforeCacheNotification) {
            identity->testAfterBackingMutationBeforeCacheNotification();
        }
#endif
        cache = KSystem::getFileCache(identity);
        if (cache) {
            cache->setLength(length);
        }
    }
    return result;
}

std::shared_ptr<MappedFileCache> KFile::getOrCreateMappedFileCache(BString name, bool writable) {
    // The cache owner deliberately outlives both guards. Final writeback is an
    // explicit lease retirement and never runs from cache destruction.
    std::shared_ptr<MappedFileCache> cache;
    std::shared_ptr<KFile> self = std::dynamic_pointer_cast<KFile>(shared_from_this());
    if (writable && !openFile->canWriteNativeAt()) {
        return nullptr;
    }
#ifdef __TEST
    std::shared_ptr<FsFileIdentity> testIdentity = openFile->node->getFileIdentity();
    if (testIdentity->testBeforeMappedCacheFileLock) {
        testIdentity->testBeforeMappedCacheFileLock();
    }
#endif
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    std::shared_ptr<FsFileIdentity> identity = openFile->node->getFileIdentity();
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX_NR(identity->mutationOperationMutex);
        U64 backingLength = (U64)openFile->length();
#ifdef __TEST
        if (identity->testAfterMappedLengthReadBeforeCacheReconcile) {
            identity->testAfterMappedLengthReadBeforeCacheReconcile();
        }
#endif
        cache = KSystem::getOrCreateFileCache(identity, name, self, backingLength, writable);
    }
    return cache;
}

void KFile::retainMappedFileCacheLease(const std::shared_ptr<MappedFileCache>& cache) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    std::shared_ptr<FsFileIdentity> identity = openFile->node->getFileIdentity();
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX_NR(identity->mutationOperationMutex);
        cache->retainMapping();
    }
}
