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

BufferAccess::BufferAccess(const BoxedPtr<FsNode>& node, U32 flags, const std::string& buffer) : FsOpenNode(node, flags) {	
    this->buffer = buffer;
    this->pos = 0;
}

S64 BufferAccess::length() {
    return this->buffer.length();
}

bool BufferAccess::setLength(S64 len) {
    return false;
}

S64 BufferAccess::getFilePointer() {
    return this->pos;
}

S64 BufferAccess::seek(S64 pos) {
    if (pos>(S64)this->buffer.length())
        pos = (S64)this->buffer.length();
    return this->pos = (S32)pos;
}

U32 BufferAccess::readNative(U8* buffer, U32 len) {
    U32 pos = (U32)this->pos;
    if (pos+len>this->buffer.length())
        len = this->buffer.length()-pos;
    memcpy(buffer, this->buffer.c_str()+pos, len);
    this->pos+=len;
    return len;
}

U32 BufferAccess::writeNative(U8* buffer, U32 len) {    
    this->buffer+=(char*)buffer;
    return len;
}

U32 BufferAccess::ioctl( U32 request) {
    return -K_ENODEV;
}

void BufferAccess::setAsync(bool isAsync) {
    if (isAsync)
        kwarn("BufferAccess::setAsync not implemented");
}

bool BufferAccess::isAsync() {
    return false;
}

void BufferAccess::waitForEvents(U32 events) {
    kpanic("BufferAccess::waitForEvents not implemented");
}

U32 BufferAccess::map(U32 address, U32 len, S32 prot, S32 flags, U64 off) {
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