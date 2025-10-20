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

#ifdef BOXEDWINE_DYNAMIC
#include "dynamicMMX.h"

void start_mmx(CPU* cpu) {
    cpu->fpu.startMMX();
}

void DynamicCodeGenMMX::startMMX() {
    movToCpu(offsetof(CPU, fpu.sw), DYN_32bit, 0);
    movToCpu(offsetof(CPU, fpu.top), DYN_32bit, 0);
    movToCpu(offsetof(CPU, fpu.isMMXInUse), DYN_8bit, 1);
    movToCpu(offsetof(CPU, fpu.tags[0]), DYN_32bit, 0);
    movToCpu(offsetof(CPU, fpu.tags[0]) + 4, DYN_32bit, 0);
    movToCpu(offsetof(CPU, fpu.isRegCached), DYN_32bit, 0);
    movToCpu(offsetof(CPU, fpu.isRegCached) + 4, DYN_32bit, 0);
}

void DynamicCodeGenMMX::dynamic_emms(DecodedOp* op) {
    movToCpu(offsetof(CPU, fpu.isMMXInUse), DYN_8bit, 0);
    movToCpu(offsetof(CPU, fpu.isRegCached), DYN_32bit, 0);
    movToCpu(offsetof(CPU, fpu.isRegCached) + 4, DYN_32bit, 0);

    movToCpu(offsetof(CPU, fpu.tags[0]), DYN_32bit, TAG_Empty | (TAG_Empty << 8) | (TAG_Empty << 16) | (TAG_Empty << 24));
    movToCpu(offsetof(CPU, fpu.tags[0]) + 4, DYN_32bit, TAG_Empty | (TAG_Empty << 8) | (TAG_Empty << 16) | (TAG_Empty << 24));
    incrementEip(op->len);
}

/*

132F0004  mov         dword ptr [edi+1D0h],0
132F000E  mov         dword ptr [edi+1D4h],0
132F0018  mov         byte ptr [edi+289h],1
132F001F  mov         dword ptr [edi+1C0h],3030303h
132F0029  mov         dword ptr [edi+1C4h],3030303h
132F0033  mov         dword ptr [edi+280h],0
132F003D  mov         dword ptr [edi+284h],0
132F0047  mov         ecx,dword ptr [edi+0Ch]
132F004A  movd        mm0,ecx
132F004D  movq        mmword ptr [edi+200h],mm0
132F0054  add         dword ptr [edi+64h],3

*/
void DynamicCodeGenMMX::dynamic_movPqR32(DecodedOp* op) {
    // MMX_reg* rmrq = cpu->fpu.getMMX(r1);
    // rmrq->ud.d0 = cpu->reg[r2].u32;
    // rmrq->ud.d1 = 0;
    // cpu->fpu.startMMX();
    startMMX();
    loadReg(op->rm, DYN_SRC, DYN_32bit);
    loadMMXFromReg(DynMMXReg(op->reg), DYN_SRC);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenMMX::dynamic_movPqE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        startMMX();
        loadMMXFromMem32(DynMMXReg(op->reg), address, offset, 0, 0);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movPqE32(op);
    }, true);
}

void DynamicCodeGenMMX::dynamic_movR32Pq(DecodedOp* op) {
    // cpu->reg[r1].u32 = cpu->reg_mmx[r2].ud.d0;
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    storeMMXToReg(DynMMXReg(op->rm), DYN_SRC);
    storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}

void DynamicCodeGenMMX::dynamic_movE32Pq(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        // writed(address, cpu->reg_mmx[reg].ud.d0);
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        storeMMXToMem32(DynMMXReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movE32Pq(op);
    });
}

void DynamicCodeGenMMX::dynamic_movPqMmx(DecodedOp* op) {
    // cpu->reg_mmx[r1].q = cpu->reg_mmx[r2].q;
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    storeCpuMMXReg(DynMMXReg(op->rm), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenMMX::dynamic_movMmxPq(DecodedOp* op) {
    // cpu->reg_mmx[r1].q = cpu->reg_mmx[r2].q;
    dynamic_movPqMmx(op);
}

void DynamicCodeGenMMX::dynamic_movPqE64(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_64bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        // cpu->reg_mmx[reg].q = readq(address);
        startMMX();
        loadMMXFromMem64(DynMMXReg(op->reg), address, offset, 0, 0);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movPqE64(op);
    }, true);
}

void DynamicCodeGenMMX::dynamic_movE64Pq(DecodedOp* op) {
    // writeq(address, cpu->reg_mmx[reg].q);
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_64bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        // writed(address, cpu->reg_mmx[reg].ud.d0);
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        storeMMXToMem64(DynMMXReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movE64Pq(op);
    });
}

void DynamicCodeGenMMX::opMmxMmx(DecodedOp* op, MmxMmxCallback callback) {
    loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    (this->*callback)((DynMMXReg)op->reg, (DynMMXReg)op->rm);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenMMX::opMmxE64(DecodedOp* op, MmxMmxCallback callback, std::function<void()> fallback) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_64bit, DYN_ADDRESS, true, [op, callback, this](DynReg address, DynReg offset) {
        // cpu->reg_mmx[reg].q = readq(address);
        DynMMXReg tmpMMX = getTmpMMX(op->reg);
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        loadMMXFromMem64(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynMMXReg)op->reg, tmpMMX);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    }, true);
}

void DynamicCodeGenMMX::opMmx(DecodedOp* op, MmxImmCallback callback) {
    loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    (this->*callback)((DynMMXReg)op->reg, op->imm);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenMMX::dynamic_pmovmskbR32Mmx(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    pmovmskbMmxMmx(DYN_SRC, (DynMMXReg)op->rm);
    storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}

void DynamicCodeGenMMX::dynamic_pextrwR32Mmx(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    pextrwRegMmx(DYN_SRC, (DynMMXReg)op->rm, op->imm & 3);
    storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}

void DynamicCodeGenMMX::dynamic_pextrwE16Mmx(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_16bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        pextrwRegMmx(DYN_DEST, (DynMMXReg)op->reg, op->imm & 3);
        writeMem(DYN_DEST, DYN_16bit, address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_pextrwE16Mmx(op);
    }, true);
}

void DynamicCodeGenMMX::dynamic_pinsrwMmxR32(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
    loadReg(op->rm, DYN_SRC, DYN_32bit);
    pinsrwMmxReg((DynMMXReg)op->reg, DYN_SRC, op->imm & 3);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenMMX::dynamic_pinsrwMmxE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        readMem(DYN_SRC, DYN_16bit, address, offset, 0, 0);
        pinsrwMmxReg((DynMMXReg)op->reg, DYN_SRC, op->imm & 3);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_pinsrwMmxE16(op);
    }, true);
}

void DynamicCodeGenMMX::dynamic_pshufwMmxMmx(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    pshufwMmxMmx((DynMMXReg)op->reg, (DynMMXReg)op->rm, op->imm);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenMMX::dynamic_pshufwMmxE64(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_64bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        // cpu->reg_mmx[reg].q = readq(address);
        DynMMXReg tmpMMX = getTmpMMX(op->reg);
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        loadMMXFromMem64(tmpMMX, address, offset, 0, 0);
        pshufwMmxMmx((DynMMXReg)op->reg, tmpMMX, op->imm);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_pshufwMmxE64(op);
    }, true);
}

void DynamicCodeGenMMX::dynamic_maskmovqEDIMmxMmx(DecodedOp* op) {
    loadReg(7, DYN_ADDRESS, DYN_32bit);
    if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
        loadSegAddress(op->base, DYN_SRC);
        addRegReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true);
    }
    movToMem(DYN_ADDRESS, DYN_64bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
        addRegReg(address, offset, DYN_32bit, false);
        maskmovq(DynMMXReg(op->reg), DynMMXReg(op->rm), address);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_maskmovqEDIMmxMmx(op);
    });
}

#endif