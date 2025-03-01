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

#ifndef __ARMV8BTCODECHUNK_H__
#define __ARMV8BTCODECHUNK_H__

#ifdef BOXEDWINE_ARMV8BT

#include "../binaryTranslation/btCodeChunk.h"

class Armv8CodeChunk : public BtCodeChunk {
public:
	Armv8CodeChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic);
	virtual bool retranslateSingleInstruction(BtCPU* cpu, U8* address) override;
	virtual void clearInstructionCache(U8* hostAddress, U32 len) override;
};

#endif

#endif
