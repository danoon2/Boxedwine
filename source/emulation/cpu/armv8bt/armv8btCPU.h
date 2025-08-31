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

#ifndef __ARMV8BT_CPU_H__
#define __ARMV8BT_CPU_H__

#ifdef BOXEDWINE_ARMV8BT
#include "../binaryTranslation/btCpu.h"

#include "armv8btAsm.h"

class Armv8btCPU : public BtCPU {
public:
    Armv8btCPU(KMemory* memory);
    
    virtual void restart() override;
    virtual void* init() override;

    U8* parity_lookup;                

    SSE sseConstants[6];

    ALIGN(U8 fpuState[512], 16);
	U64 originalCpuRegs[16];	
    void* reTranslateChunkAddress;
    void* reTranslateChunkAddressFromReg;
    void* jmpAndTranslateIfNecessary;
#ifdef _DEBUG
    U32 fromEip;
    U64 exceptionRegs[32];
#endif
#ifdef __TEST
    void addReturnFromTest();
#endif

    virtual void link(BtData* data, void* hostAddress) override;

    virtual void setSeg(U32 index, U32 address, U32 value) override;

#ifdef __TEST
    virtual void postTestRun() override {};
#endif
protected:
    virtual BtData* getData1() override { data1.reset(); return &data1; }
    virtual BtData* getData2() override { data2.reset(); return &data2; }
    Armv8btAsm data1;
    Armv8btAsm data2;
private:      
    void writeJumpAmount(BtData* data, U32 pos, U32 toLocation, U8* offset);
};
#endif
#endif
