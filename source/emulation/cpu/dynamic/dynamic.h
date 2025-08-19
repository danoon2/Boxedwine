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

#ifndef __DYNAMIC_H__
#define __DYNAMIC_H__

#include "../common/cpu.h"

class DynamicJump {
public:
    DynamicJump() = default;
    DynamicJump(U32 eip, U32 bufferPos) : eip(eip), bufferPos(bufferPos) {}
    U32 eip = 0;
    U32 bufferPos = 0;
};

class DynamicData {
public:
    CPU* cpu = nullptr;
    DecodedOp* firstOp = nullptr;

    bool done = false;
    bool isFunction = false;
    std::vector<DynamicJump> jumps;
    BHashTable<U32, U32> eipToBufferPos;
    U32 currentEip = 0;
};

typedef void (*pfnDynamicOp)(DynamicData* data, DecodedOp* op);

#endif