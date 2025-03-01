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

FsOpenNode::FsOpenNode(std::shared_ptr<FsNode> node, U32 flags) : node(node), flags(flags), listNode(this) {
    node->addOpenNode(&this->listNode);
}

FsOpenNode::~FsOpenNode() {
    this->node->removeOpenNode(this);
}

U32 FsOpenNode::internalRead(KThread* thread, U32 address, U32 len) {
    U32 result = 0;
    KMemory* memory = thread->memory;

    memory->performOnMemory(address, len, false, [&result, this](U8* ram, U32 len) {
        U32 read = this->readNative(ram, len);
        if ((S32)read < 0) {
            return false;
        }
        result += read;
        return read == len;
        });
    return result;
}

U32 FsOpenNode::read(KThread* thread, U32 address, U32 len) {
    BOXEDWINE_MUTEX* mutex = getReadMutex();
    if (mutex) {
        // this will reduce thashing in the zip file
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(*mutex);
        return internalRead(thread, address, len);
    } else {
        return internalRead(thread, address, len);
    }
}

U32 FsOpenNode::write(KThread* thread, U32 address, U32 len) {
    U32 result = 0;
    KMemory* memory = thread->memory;

    memory->performOnMemory(address, len, true, [&result, this](U8* ram, U32 len) {
        U32 written = this->writeNative(ram, len);
        if ((S32)written < 0) {
            return false;
        }
        result += written;
        return written == len;
        });
    return result;
}

void FsOpenNode::loadDirEntries() {
    BOXEDWINE_CRITICAL_SECTION;
    if (this->dirEntries.size()==0 && this->node) {
        this->dirEntries.reserve(2 + node->getChildCount());
        this->dirEntries.push_back(this->node);
        std::shared_ptr<FsNode> parent = this->node->getParent().lock();
        if (parent) {
            this->dirEntries.push_back(parent);
        }
        this->node->getAllChildren(this->dirEntries);        
    }
}

U32 FsOpenNode::getDirectoryEntryCount() {
    this->loadDirEntries();
    return (U32)this->dirEntries.size();
}

std::shared_ptr<FsNode> FsOpenNode::getDirectoryEntry(U32 index, BString& name) {
    if (!this->node) {
        return nullptr;
    }
    this->loadDirEntries();
    if (index==0)
        name = B(".");
    else if (index==1 && this->node->getParent().lock())
        name = B("..");
    else
        name = this->dirEntries[index]->name;
    return this->dirEntries[index];
}