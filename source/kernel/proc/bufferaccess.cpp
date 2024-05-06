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

#include "bufferaccess.h"

BufferAccess::BufferAccess(const std::shared_ptr<FsNode>& node, U32 flags, BString* buffer) : FsOpenNode(node, flags) {
    this->buffer = buffer;
    this->pos = 0;
}

BufferAccess::BufferAccess(const std::shared_ptr<FsNode>& node, U32 flags, const BString& buffer) : FsOpenNode(node, flags) {
    this->storage = buffer; // make a copy
    this->buffer = &storage;
    this->pos = 0;
}

S64 BufferAccess::length() {
    return this->buffer->length();
}

bool BufferAccess::setLength(S64 len) {
    return false;
}

S64 BufferAccess::getFilePointer() {
    return this->pos;
}

S64 BufferAccess::seek(S64 pos) {
    if (pos>(S64)this->buffer->length())
        pos = (S64)this->buffer->length();
    return this->pos = (S32)pos;
}

U32 BufferAccess::readNative(U8* buffer, U32 len) {
    U32 pos = (U32)this->pos;
    if (pos+len>(U32)this->buffer->length())
        len = (U32)this->buffer->length()-pos;
    memcpy(buffer, this->buffer->c_str()+pos, len);
    this->pos+=len;
    return len;
}

U32 BufferAccess::writeNative(U8* buffer, U32 len) {    
    if (this->buffer->length() > pos) {
        if (pos == 0) {
            this->buffer->clear();
        } else {
            BString b = *this->buffer;
            this->buffer->clear();
            this->buffer->append(b.substr(0, pos));
        }
    }
    this->buffer->append((char*)buffer, len);
    this->pos+=len;
    return len;
}

U32 BufferAccess::ioctl(KThread* thread, U32 request) {
    return -K_ENODEV;
}

void BufferAccess::setAsync(bool isAsync) {
    if (isAsync)
        kwarn("BufferAccess::setAsync not implemented");
}

bool BufferAccess::isAsync() {
    return false;
}

void BufferAccess::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    kpanic("BufferAccess::waitForEvents not implemented");
}

U32 BufferAccess::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool BufferAccess::canMap() {
    return false;
}

void BufferAccess::close() {
}

bool BufferAccess::isOpen() {
    return true;
}

void BufferAccess::reopen() {
    this->pos = 0;
}

bool BufferAccess::isWriteReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

bool BufferAccess::isReadReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_WRONLY;
}