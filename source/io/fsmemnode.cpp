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
#include "fsmemnode.h"
#include "fsmemopennode.h"
#include "kstat.h"

FsMemNode::FsMemNode(U32 id, U32 rdev, BString path) : FsNode(Type::Memory, id, rdev, path, B(""), B(""), false, nullptr), openNode(nullptr) {
}

U32 FsMemNode::rename(BString path) {
    kpanic("FsMemNode::rename not supported");
    return -1;
}

bool FsMemNode::remove() {
    kpanic("FsMemNode::remove not supported");
    return false;
}

U64 FsMemNode::lastModified() {
    return this->openNode->lastModified();
}

U64 FsMemNode::length() {
    return this->openNode->length();
}

FsOpenNode* FsMemNode::open(U32 flags) {
    kpanic("FsMemNode::open not supported");
    return nullptr;
}

U32 FsMemNode::getType(bool checkForLink) {
    return 8; // DT_REG
}

U32 FsMemNode::getMode() {
    return K__S_IWRITE | K__S_IREAD | (this->getType(false) << 12);
}

U32 FsMemNode::removeDir() {
    kpanic("FsMemNode::removeDir not supported");
    return -1;
}

U32 FsMemNode::setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) {
    kpanic("FsMemNode::setTimes not supported");
    return 0;
}