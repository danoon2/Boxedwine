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
#ifdef BOXEDWINE_X64
#include "x64CodeChunk.h"
#include "x64Asm.h"
#include "x64CPU.h"

bool X64CodeChunk::retranslateSingleInstruction(BtCPU* btCPU, U8* address) {
    x64CPU* cpu = (x64CPU*)btCPU;
    U8* startofHostInstruction;
    U32 index;
    U32 eip = this->getEipThatContainsHostAddress(address, &startofHostInstruction, &index) - cpu->seg[CS].address;
    X64Asm data(cpu);
    data.ip = eip;
    data.startOfDataIp = eip;
    data.translateInstruction();
    U32 eipLen = data.ip - data.startOfOpIp;
    U32 hostLen = data.bufferPos;
    if (eipLen == this->emulatedInstructionLen[index] && hostLen == this->hostInstructionLen[index]) {
        Platform::writeCodeToMemory(startofHostInstruction, hostLen, [startofHostInstruction, &data, hostLen]() {
            memcpy(startofHostInstruction, data.buffer, hostLen);
            });
        return true;
    }
    return false;
}

#endif
