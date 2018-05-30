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
        kwarn("KFile::setBlocking not implemented");
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

KFileLock* KFile::getLock(KFileLock* lock) {
    FsOpenNode* openNode = this->openFile;
    BoxedPtr<FsNode> node = openNode->node;
    U64 l1 = lock->l_start;
    U64 l2 = l1+lock->l_len;

    if (lock->l_len == 0)
        l2 = 0xFFFFFFFF;
    for( auto& n : node->locks ) {
        KFileLock* next = &n;
        U64 s1 = next->l_start;
        U64 s2 = s1+next->l_len;
        
        if (next->l_len == 0)
            s2 = 0xFFFFFFFF;
        if ((s1>=l1 && s1<=l2) || (s2>=l1 && s2<=l2)) {
            return next;
        }
    }
    return 0;
}

U32 KFile::setLock(KFileLock* lock, bool wait) {    
    // :TODO: unlock, auto remove lock if process exits
    if (lock->l_type == K_F_UNLCK) {
    } else {
        this->openFile->node->locks.push_back(*lock);
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

void KFile::waitForEvents(U32 events) {
    this->openFile->waitForEvents(events);
}

U32 KFile::write(U32 buffer, U32 len) {
    return this->openFile->write(buffer, len);
}

U32 KFile::writeNative(U8* buffer, U32 len) {
    return this->openFile->writeNative(buffer, len);
}

U32 KFile::read(U32 buffer, U32 len) {
    return this->openFile->read(buffer, len);
}

U32 KFile::readNative(U8* buffer, U32 len) {
    return this->openFile->readNative(buffer, len);
}

U32 KFile::stat(U32 address, bool is64) {
    FsOpenNode* openNode = this->openFile;
    BoxedPtr<FsNode> node = openNode->node;
    U64 len = node->length();

    KSystem::writeStat(node->path, address, is64, 1, node->id, node->getMode(), node->rdev, len, FS_BLOCK_SIZE, (len+FS_BLOCK_SIZE-1)/FS_BLOCK_SIZE, node->lastModified(), node->getHardLinkCount());
    return 0;
}

U32 KFile::map(U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return this->openFile->map(address, len, prot, flags, off);
}

bool KFile::canMap() {
    return this->openFile->canMap();
}

S64 KFile::seek(S64 pos) {
    return this->openFile->seek(pos);
}

S64 KFile::getPos() {
    return this->openFile->getFilePointer();
}

U32 KFile::ioctl(U32 request) {
    return this->openFile->ioctl(request);
}

bool KFile::supportsLocks() {
    return true;
}

S64 KFile::length() {
    return this->openFile->length();
}
