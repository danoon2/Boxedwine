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
#include "jitMMX.h"

void start_mmx(CPU* cpu) {
    cpu->fpu.startMMX();
}

void JitMMX::startMMX() {
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.sw), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.top), 0);
    writeCPUValue(JitWidth::b8, offsetof(CPU, fpu.isMMXInUse), 1);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.isRegCached), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.isRegCached) + 4, 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.tags[0]), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.tags[0]) + 4, 0);
}

void JitMMX::dynamic_emms(DecodedOp* op) {
    writeCPUValue(JitWidth::b8, offsetof(CPU, fpu.isMMXInUse), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.isRegCached), 0);
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.isRegCached) + 4, 0);

    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.tags[0]), TAG_Empty | (TAG_Empty << 8) | (TAG_Empty << 16) | (TAG_Empty << 24));
    writeCPUValue(JitWidth::b32, offsetof(CPU, fpu.tags[0]) + 4, TAG_Empty | (TAG_Empty << 8) | (TAG_Empty << 16) | (TAG_Empty << 24));

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
void JitMMX::dynamic_movPqR32(DecodedOp* op) {
    // MMX_reg* rmrq = cpu->fpu.getMMX(r1);
    // rmrq->ud.d0 = cpu->reg[r2].u32;
    // rmrq->ud.d1 = 0;
    // cpu->fpu.startMMX();
    startMMX();
    loadMMXFromReg(DynMMXReg(op->reg), getReadOnlyReg(op->rm));
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitMMX::dynamic_movPqE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        startMMX();
        loadMMXFromMem32(DynMMXReg(op->reg), address, offset, 0, 0);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitMMX::dynamic_movR32Pq(DecodedOp* op) {
    // cpu->reg[r1].u32 = cpu->reg_mmx[r2].ud.d0;
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    storeMMXToReg(DynMMXReg(op->rm), getReg(op->reg, -1, false));
    incrementEip(op->len);
}

void JitMMX::dynamic_movE32Pq(DecodedOp* op) {
    write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // writed(address, cpu->reg_mmx[reg].ud.d0);
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        storeMMXToMem32(DynMMXReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    });
}

void JitMMX::dynamic_movPqMmx(DecodedOp* op) {
    // cpu->reg_mmx[r1].q = cpu->reg_mmx[r2].q;
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    storeCpuMMXReg(DynMMXReg(op->rm), op->reg);
    incrementEip(op->len);
}

void JitMMX::dynamic_movMmxPq(DecodedOp* op) {
    // cpu->reg_mmx[r1].q = cpu->reg_mmx[r2].q;
    dynamic_movPqMmx(op);
}

void JitMMX::dynamic_movPqE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        // cpu->reg_mmx[reg].q = readq(address);
        startMMX();
        loadMMXFromMem64(DynMMXReg(op->reg), address, offset, 0, 0);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitMMX::dynamic_movE64Pq(DecodedOp* op) {
    // writeq(address, cpu->reg_mmx[reg].q);
    write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // writed(address, cpu->reg_mmx[reg].ud.d0);
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        storeMMXToMem64(DynMMXReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    });
}

void JitMMX::opMmxMmx(DecodedOp* op, MmxMmxCallback callback) {
    loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    (this->*callback)((DynMMXReg)op->reg, (DynMMXReg)op->rm);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitMMX::opMmxE64(DecodedOp* op, MmxMmxCallback callback) {
    read(JitWidth::b64, calculateEaa(op), [op, callback, this](RegPtr address, RegPtr offset) {
        // cpu->reg_mmx[reg].q = readq(address);
        DynMMXReg tmpMMX = getTmpMMX(op->reg);
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        loadMMXFromMem64(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynMMXReg)op->reg, tmpMMX);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitMMX::opMmx(DecodedOp* op, MmxImmCallback callback) {
    loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    (this->*callback)((DynMMXReg)op->reg, op->imm);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitMMX::dynamic_pmovmskbR32Mmx(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    pmovmskbMmxMmx(getReg(op->reg, -1, false), (DynMMXReg)op->rm);
    incrementEip(op->len);
}

void JitMMX::dynamic_pextrwR32Mmx(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    pextrwRegMmx(getReg(op->reg, -1, false), (DynMMXReg)op->rm, op->imm & 3);
    incrementEip(op->len);
}

void JitMMX::dynamic_pextrwE16Mmx(DecodedOp* op) {
    write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        RegPtr tmp = getTmpReg();
        pextrwRegMmx(tmp, (DynMMXReg)op->reg, op->imm & 3);
        write(JitWidth::b16, address, offset, 0, 0, tmp);
        incrementEip(op->len);
    });
}

void JitMMX::dynamic_pinsrwMmxR32(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
    pinsrwMmxReg((DynMMXReg)op->reg, getReadOnlyReg(op->rm), op->imm & 3);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitMMX::dynamic_pinsrwMmxE16(DecodedOp* op) {
    read(JitWidth::b16, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        RegPtr tmp = getTmpReg();
        read(JitWidth::b16, tmp, address, offset, 0, 0);
        pinsrwMmxReg((DynMMXReg)op->reg, tmp, op->imm & 3);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitMMX::dynamic_pshufwMmxMmx(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    pshufwMmxMmx((DynMMXReg)op->reg, (DynMMXReg)op->rm, op->imm);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitMMX::dynamic_pshufwMmxE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        // cpu->reg_mmx[reg].q = readq(address);
        DynMMXReg tmpMMX = getTmpMMX(op->reg);
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        loadMMXFromMem64(tmpMMX, address, offset, 0, 0);
        pshufwMmxMmx((DynMMXReg)op->reg, tmpMMX, op->imm);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitMMX::dynamic_maskmovqEDIMmxMmx(DecodedOp* op) {
    RegPtr address;
    if (op->base < 6 && cpu->thread->process->hasSetSeg[op->base]) {
        address = getTmpReg(7);
        addReg(JitWidth::b32, address, getReadOnlySegAddress(op->base));
    } else {
        address = getReadOnlyReg(7);
    }
    write(JitWidth::b64, std::move(address), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuMMXReg(DynMMXReg(op->reg), op->reg);
        loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
        addReg(JitWidth::b32, address, offset);
        maskmovq(DynMMXReg(op->reg), DynMMXReg(op->rm), address);
        incrementEip(op->len);
    });
}

#endif