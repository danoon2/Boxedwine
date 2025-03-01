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

#include "fsfilenode.h"
#include "fsdiropennode.h"

FsDirOpenNode::FsDirOpenNode(std::shared_ptr<FsNode> node, U32 flags) : FsOpenNode(node, flags), pos(0) {
}

S64 FsDirOpenNode::length() {	
    return this->node->getChildCount();
}

bool FsDirOpenNode::setLength(S64 len) {
    return false;
}

S64 FsDirOpenNode::getFilePointer() {
    return this->pos;
}

S64 FsDirOpenNode::seek(S64 pos) {
    if (pos == 0 && this->pos == 0) {
        return 0;
    }
    if (pos>=0 && pos<=this->getDirectoryEntryCount())
        this->pos = (S32)pos;
    else
        this->pos = this->getDirectoryEntryCount();
    return this->pos;
}

void FsDirOpenNode::close() {
}

bool FsDirOpenNode::isOpen() {
    return true;
}

U32 FsDirOpenNode::ioctl(KThread* thread, U32 request) {
    return -K_ENODEV;
}

void FsDirOpenNode::setAsync(bool isAsync) {
    if (isAsync)
        kdebug("FsDirOpenNode::setAsync not implemented");
}

bool FsDirOpenNode::isAsync() {
    return false;
}

void FsDirOpenNode::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    kdebug("FsDirOpenNode::waitForEvents not implemented");
}

bool FsDirOpenNode::isWriteReady() {
    return false;
}

bool FsDirOpenNode::isReadReady() {
    return false;
}

U32 FsDirOpenNode::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool FsDirOpenNode::canMap() {
    return false;
}

U32 FsDirOpenNode::readNative(U8* buffer, U32 len) {
    return 0;
}

U32 FsDirOpenNode::writeNative(U8* buffer, U32 len) {
    return 0;
}

void FsDirOpenNode::reopen() {
    this->pos = 0;
}
