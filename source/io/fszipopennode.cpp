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

#ifdef BOXEDWINE_ZLIB

#include "fszipopennode.h"
#include "fszipnode.h"
#include "fszip.h"

#include <fcntl.h>
#include UNISTD

FsZipOpenNode::FsZipOpenNode(std::shared_ptr<FsNode> node, std::shared_ptr<FsZipNode>& zipNode, U32 flags, U64 offset, U64 dataOffset, U32 compressionMethod, BString zipPath) : FsOpenNode(node, flags), zipNode(zipNode), pos(0), offset(offset), dataOffset(dataOffset), compressionMethod(compressionMethod) {
    if (compressionMethod == 0 && dataOffset && zipPath.length()) {
        directHandle = ::open(zipPath.c_str(), O_RDONLY | O_BINARY);
    }
}

FsZipOpenNode::~FsZipOpenNode() {
    close();
}

S64 FsZipOpenNode::length() {
    return this->zipNode->length();
}

bool FsZipOpenNode::setLength(S64 len) {
    // if this file was open for write, it would have been copied to the file system
    kpanic("FsZipOpenNode::setLength should not have been called"); 
    return false;
}

S64 FsZipOpenNode::getFilePointer() {
    return this->pos;
}

S64 FsZipOpenNode::seek(S64 pos) {
    if (!open) {
        return -K_EBADF;
    }
    if (pos < 0) {
        return -K_EINVAL;
    }
    this->pos = pos;
    return this->pos;
}

void FsZipOpenNode::close() {
    open = false;
    if (directHandle != 0xFFFFFFFF) {
        ::close(directHandle);
        directHandle = 0xFFFFFFFF;
    }
}

bool FsZipOpenNode::isOpen() {
    return open;
}

U32 FsZipOpenNode::ioctl(KThread* thread, U32 request) {
    return -K_ENODEV;
}

void FsZipOpenNode::setAsync(bool isAsync) {
    if (isAsync)
        kdebug("FsZipOpenNode::setAsync not implemented");
}

bool FsZipOpenNode::isAsync() {
    return false;
}

void FsZipOpenNode::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    kdebug("FsZipOpenNode::waitForEvents not implemented");
}

bool FsZipOpenNode::isWriteReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

bool FsZipOpenNode::isReadReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_WRONLY;
}

U32 FsZipOpenNode::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool FsZipOpenNode::canMap() {
    return true;
}

U32 FsZipOpenNode::readNative(U8* buffer, U32 len) {
    if (!open) {
        return -K_EBADF;
    }
    if (pos >= length() || !len) {
        return 0;
    }
    len = (U32)std::min<U64>(std::min<U32>(len, 0x7ffff000), length() - pos);
    if (directHandle != 0xFFFFFFFF) {
        if (lseek64(directHandle, dataOffset + this->pos, SEEK_SET) < 0) {
            return -K_EIO;
        }
        S32 read = (S32)::read(directHandle, buffer, len);
        if (read <= 0) {
            return -K_EIO;
        }
        U32 result = (U32)read;
        this->pos += result;
        return result;
    }

    std::shared_ptr<FsZip> fsZip = zipNode->fsZip;
    if (fsZip) {
        S32 result = fsZip->readZip(offset, pos, buffer, len);
        if (result > 0) {
            pos += result;
        }
        return result;
    }
    return -K_EIO;
}

U32 FsZipOpenNode::writeNative(U8* buffer, U32 len) {
    kpanic("FsZipOpenNode::writeNative not implemented");
    return 0;
}

void FsZipOpenNode::reopen() {
    close();
    this->pos = 0;
    open = true;
    if (compressionMethod == 0 && dataOffset && zipNode->fsZip) {
        directHandle = ::open(zipNode->fsZip->zipPath.c_str(), O_RDONLY | O_BINARY);
    }
}

BOXEDWINE_MUTEX* FsZipOpenNode::getReadMutex() {
    std::shared_ptr<FsZip> fsZip = zipNode->fsZip;
    if (fsZip) {
        return &fsZip->readMutex;
    }
    return nullptr;
}

#endif
