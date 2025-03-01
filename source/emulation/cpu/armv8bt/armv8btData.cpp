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

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
#include "../binaryTranslation/btCodeChunk.h"
#include "armv8btCodeChunk.h"

Armv8btData::Armv8btData(Armv8btCPU* cpu) : cpu(cpu) {
    this->resetForNewOp();

    this->fpuTopRegSet = false;
    this->fpuOffsetRegSet = false;
    this->clearCachedFpuRegs();
}

void Armv8btData::clearCachedFpuRegs() {
    memset(this->isFpuRegCached, 0, sizeof(isFpuRegCached));
}

void Armv8btData::reset() {
    BtData::reset();
    clearCachedFpuRegs();
    this->fpuTopRegSet = false;
    this->fpuOffsetRegSet = false;
}

void Armv8btData::resetForNewOp() {
    this->startOfOpIp = this->ip;
}

std::shared_ptr<BtCodeChunk> Armv8btData::createChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic) {
    return std::make_shared<Armv8CodeChunk>(instructionCount, eipInstructionAddress, hostInstructionIndex, hostInstructionBuffer, hostInstructionBufferLen, eip, eipLen, dynamic);
}

#endif
