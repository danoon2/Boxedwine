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

#ifndef __X64CODECHUNK_H__
#define __X64CODECHUNK_H__

#ifdef BOXEDWINE_X64

#include "../binaryTranslation/btCodeChunk.h"

class X64CodeChunk : public BtCodeChunk {
public:
	X64CodeChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic) : BtCodeChunk(instructionCount, eipInstructionAddress, hostInstructionIndex, hostInstructionBuffer, hostInstructionBufferLen, eip, eipLen, dynamic) {}

	// from BtCodeChunk
	bool retranslateSingleInstruction(BtCPU* cpu, U8* address) override;
};

#endif

#endif