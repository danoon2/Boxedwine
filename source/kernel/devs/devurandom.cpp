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
#include <stdlib.h>
#include <time.h>

class DevURandom : public FsVirtualOpenNode {
public:
    DevURandom(const BoxedPtr<FsNode>& node, U32 flags) : FsVirtualOpenNode(node, flags) {srand((U32)time(0));}
    virtual U32 readNative(U8* buffer, U32 len);
    virtual U32 writeNative(U8* buffer, U32 len) {return 0;}
};

U32 DevURandom::readNative(U8* buffer, U32 len) {
    for (U32 i=0;i<len;i++) {
        buffer[i] = (U8)rand();
    }
    return len;
}

FsOpenNode* openDevURandom(const BoxedPtr<FsNode>& node, U32 flags) {
    return new DevURandom(node, flags);
}

