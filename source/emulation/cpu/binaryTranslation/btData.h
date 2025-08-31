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

#ifndef __BTDATA_H__
#define __BTDATA_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR

#include "btCpu.h"

#define CPU_OFFSET_OP_PAGES (U32)(offsetof(BtCPU, opCache))

class TodoJump {
public:
    TodoJump() = default;
    TodoJump(U32 eip, U32 bufferPos, U32 opIndex) : eip(eip), bufferPos(bufferPos), opIndex(opIndex) {}
    U32 eip = 0;
    U32 bufferPos = 0;
    U32 opIndex = 0;
};

class BtData {
public:
    BtData();
    virtual ~BtData();    
    virtual void* commit(KMemory* memory);
    void makeLive(U8* hostAddress);

    U32 ip = 0;
    U32 startOfDataIp = 0;
    U32 startOfOpIp = 0;
    U32 calculatedEipLen = 0;
    bool done = false;
    U32* ipAddress = nullptr;
    U32* ipAddressBufferPos = nullptr;
    U32 ipAddressCount = 0;
    U32 ipAddressBufferSize = 0;
    U32 ipAddressBuffer[64] = { 0 };
    U32 ipAddressBufferPosBuffer[64] = { 0 };
    U8* buffer = nullptr;
    U32 bufferSize = 0;
    U32 bufferPos = 0;
    U8 bufferInternal[512] = { 0 };

    std::vector<TodoJump> todoJump;
    S32 stopAfterInstruction = -1;

    DecodedOp* currentOp = nullptr;
    DecodedOp* firstOp = nullptr;

    BtData* firstPass = nullptr;
    bool needLargeIfJmpReg = false;

    void mapAddress(U32 ip, U32 bufferPos);
    U8 calculateEipLen(U32 eip);
    U32 getHostOffsetFromEip(U32 ip);
    bool needsToPreverveFlags();

    void write8(U8 data);
    void write16(U16 data);
    void write32(U32 data);
    void write64(U64 data);
    void write64Buffer(U8* buffer, U64 value);
    void write32Buffer(U8* buffer, U32 value);
    void write16Buffer(U8* buffer, U16 value);

    virtual void jumpTo(U32 eip) = 0;
    virtual void resetForNewOp() = 0;
    virtual void translateInstruction() = 0;
    virtual void reset();
};

#endif
#endif