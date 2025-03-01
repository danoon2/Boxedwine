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

#include "fsvirtualnode.h"
#include "kstat.h"

bool FsVirtualNode::remove() {
    return false;
}

U64 FsVirtualNode::lastModified() {
    return 0;
}

U64 FsVirtualNode::length() {
    return 0;
}

U32 FsVirtualNode::getMode() {
    return this->mode;
}

FsOpenNode* FsVirtualNode::open(U32 flags) {
    if ((flags & K_O_ACCMODE)==K_O_RDONLY) {
        if (!this->canRead())
            return nullptr;
    } else if ((flags & K_O_ACCMODE)==K_O_WRONLY) {
        if (!this->canWrite())
            return nullptr;
    } else {
        if (!this->canWrite())
            return nullptr;
        if (!this->canRead())
            return nullptr;
    }
    if (flags & K_O_CREAT) {
        //return 0;
    }
    if (flags & K_O_EXCL) {
        kdebug("What about exclusive virtual files");
    }
    if (flags & K_O_TRUNC) {
        kdebug("What about truncating a virtual file");
    }
    if (flags & K_O_APPEND) {
        kdebug("What about appending a virtual file");
    }
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(B(""), this->path, true);
    return this->openFunc(node, flags, this->openData);
}

U32 FsVirtualNode::getType(bool checkForLink) {
    if (this->isDirectory()) {
        return 4; // DT_DIR
    }
    if (this->mode & K__S_IFCHR) {
        return 2; // DT_CHR
    }
    if (this->mode & K_S_IFSOCK) {
        return 12; // DT_SOCK
    }
    return 8; // DT_REG
}

U32 FsVirtualNode::rename(BString path) {
    return -K_EIO;
}

U32 FsVirtualNode::removeDir() {
    kpanic("FsVirtualNode::removeDir not implemented");
    return 0;
}

U32 FsVirtualNode::setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) {
    klog("FsVirtualNode::setTimes not implemented");
    return 0;
}