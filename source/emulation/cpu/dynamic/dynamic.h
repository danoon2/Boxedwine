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

class DynamicData {
public:
    CPU* cpu = nullptr;
    DecodedOp* firstOp = nullptr;

    const LazyFlags* currentLazyFlags = nullptr;;    
    BHashTable<U32, U32> eipToBufferPos;
    U32 currentEip = 0;
    U32 startingEip = 0;
    U32 lastOpEip = 0;

    bool canJumpInBlock(DecodedOp* op) {
        return currentEip < lastOpEip && currentEip + op->len + op->imm <= lastOpEip && currentEip + op->len + op->imm >= startingEip;
    }
};

typedef void (*pfnDynamicOp)(DynamicData* data, DecodedOp* op);

#endif