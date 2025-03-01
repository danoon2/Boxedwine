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

#ifndef __NORMAL_CPU_H__
#define __NORMAL_CPU_H__

#include "../common/cpu.h"

class NormalCPU : public CPU {
public:
    NormalCPU(KMemory* memory);

    static void clearCache();

    // from CPU
    void run() override;
    DecodedBlock* getNextBlock() override;

    static OpCallback getFunctionForOp(DecodedOp* op);

    static DecodedBlock* getBlockForInspectionButNotUsed(CPU* cpu, U32 address, bool big);
    static DecodedOp* decodeSingleOp(CPU* cpu, U32 address);

    OpCallback firstOp;
};

#endif