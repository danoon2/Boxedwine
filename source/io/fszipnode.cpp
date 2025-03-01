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
#include "fszipnode.h"
#include "fsfilenode.h"
#include UNISTD
#include <fcntl.h>
#include "fszipopennode.h"
#include UTIME

FsZipNode::FsZipNode(const fsZipInfo& zipInfo, const std::shared_ptr<FsZip>& fsZip) : fsZip(fsZip) {
    this->zipInfo = zipInfo;
}

bool FsZipNode::moveToFileSystem(std::shared_ptr<FsNode> node) {
    if (node->isDirectory())
        return false;
    if (node->isLink()) {
        U32 to = ::open((node->nativePath+EXT_LINK).c_str(), O_WRONLY | O_CREAT | O_BINARY, 0666);
        ::write(to, node->link.c_str(), node->link.length());
        ::close(to);
        return true;
    }

    FsOpenNode* from = this->open(node, K_O_RDONLY);
    bool result = false;
    U32 to = ::open(node->nativePath.c_str(), O_WRONLY | O_CREAT | O_BINARY, 0666);
    if (to != 0xFFFFFFFF) {
        U8 buffer[4096];
        U32 read = from->readNative(buffer, 4096);
        while (read) {
            if (::write(to, buffer, read) != (int)read)
                return false;
            read = from->readNative(buffer, 4096);
        }
        ::close(to);

        struct utimbuf settime = { 0, 0 };

        settime.actime = this->zipInfo.lastModified / 1000;
        settime.modtime = this->zipInfo.lastModified / 1000;
        utime(node->nativePath.c_str(), &settime);

        result = true;
    }
    from->close();
    return result;
}

U64 FsZipNode::lastModified() {
    return this->zipInfo.lastModified;
}

U64 FsZipNode::length() {
    return this->zipInfo.length;
}

FsOpenNode* FsZipNode::open(std::shared_ptr<FsNode> node, U32 flags) {
    std::shared_ptr<FsZipNode> zipNode = shared_from_this();
    return new FsZipOpenNode(node, zipNode, flags, (U64)this->zipInfo.offset);
}

#endif
