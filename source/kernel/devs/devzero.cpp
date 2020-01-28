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

#include "../../io/fsvirtualopennode.h"

class DevZero : public FsVirtualOpenNode {
public:
    DevZero(const BoxedPtr<FsNode>& node, U32 flags) : FsVirtualOpenNode(node, flags) {}
    virtual U32  map(U32 address, U32 len, S32 prot, S32 flags, U64 off);
    virtual bool canMap();
    virtual U32 readNative(U8* buffer, U32 len);
    virtual U32 writeNative(U8* buffer, U32 len);
};

U32 DevZero::readNative(U8* buffer, U32 len) {
    memset(buffer, 0, len);
    return len;
}

U32 DevZero::writeNative(U8* buffer, U32 len) {
    return len;
}

U32 DevZero::map(U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return KThread::currentThread()->process->mmap(address, len, prot, flags, -1, off);
}

bool DevZero::canMap() {
    return true;
}

FsOpenNode* openDevZero(const BoxedPtr<FsNode>& node, U32 flags, U32 data) {
    return new DevZero(node, flags);
}