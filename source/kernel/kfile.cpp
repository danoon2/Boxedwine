/*
 *  Copyright (C) 2016  The BoxedWine Team
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
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    return this->openFile->write(thread, buffer, len);
}

U32 KFile::writeNative(U8* buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    return this->openFile->writeNative(buffer, len);
}

U32 KFile::read(KThread* thread, U32 buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    return this->openFile->read(thread, buffer, len);
}

U32 KFile::readNative(U8* buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    return this->openFile->readNative(buffer, len);
}

U32 KFile::stat(KProcess* process, U32 address, bool is64) {
    FsOpenNode* openNode = this->openFile;
    std::shared_ptr<FsNode> node = openNode->node;
    U64 len = (U64)openNode->length();

    KSystem::writeStat(process, node->path, address, is64, 1, node->id, node->getMode(), node->rdev, len, FS_BLOCK_SIZE, (len+FS_BLOCK_SIZE-1)/FS_BLOCK_SIZE, node->lastModified(), node->getHardLinkCount());
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
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    S64 previousOffset = this->openFile->getFilePointer();
    this->openFile->seek(offset);
    U32 result = this->openFile->read(thread, buffer, len);
    this->openFile->seek(previousOffset);
    return result;
}

U32 KFile::pwrite(KThread* thread, U32 buffer, S64 offset, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(filePosMutex);
    S64 previousOffset = this->openFile->getFilePointer();
    this->openFile->seek(offset);
    U32 result = this->openFile->write(thread, buffer, len);
    this->openFile->seek(previousOffset);
    return result;
}