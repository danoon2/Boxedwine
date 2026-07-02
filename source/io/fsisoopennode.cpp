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
#include "fsisoopennode.h"
#include "fsiso.h"
#include "kerror.h"

FsIsoOpenNode::FsIsoOpenNode(const std::shared_ptr<FsNode>& node, U32 flags, U64 dataOffset, U64 fileLength, const std::shared_ptr<FsIso>& iso)
    : FsOpenNode(node, flags), iso(iso), dataOffset(dataOffset), fileLength(fileLength), pos(0) {
}

S64 FsIsoOpenNode::length() {
    return (S64)fileLength;
}

S64 FsIsoOpenNode::getFilePointer() {
    return (S64)pos;
}

S64 FsIsoOpenNode::seek(S64 newPos) {
    if (newPos < 0)
        newPos = 0;
    if (newPos > (S64)fileLength)
        newPos = (S64)fileLength;
    pos = (U64)newPos;
    return (S64)pos;
}

U32 FsIsoOpenNode::ioctl(KThread* thread, U32 request) {
    return (U32)-K_ENODEV;
}

U32 FsIsoOpenNode::readNative(U8* buffer, U32 len) {
    if (pos >= fileLength)
        return 0;
    U32 toRead = (U32)std::min((U64)len, fileLength - pos);
    U32 result = iso->readData(dataOffset + pos, buffer, toRead);
    pos += result;
    return result;
}

BOXEDWINE_MUTEX* FsIsoOpenNode::getReadMutex() {
    return &iso->readMutex;
}
