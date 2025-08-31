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

#ifndef __ARMV8BT_DATA_H__
#define __ARMV8BT_DATA_H__

#ifdef BOXEDWINE_ARMV8BT

#include "arm8btFlags.h"
#include "../binaryTranslation/btData.h"

class Armv8btCPU;

class Armv8btData : public BtData {
public:
    Armv8btData(Armv8btCPU* cpu);    

    void resetForNewOp() override;
    void* commit(KMemory* memory) override;

    Armv8btCPU* cpu;

    bool fpuTopRegSet;
    bool fpuOffsetRegSet;
    bool isFpuRegCached[8];
    void clearCachedFpuRegs();

    virtual void reset() override;
};
#endif
#endif
