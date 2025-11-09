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

#include "../common/common_xchg.h"
void DynamicData::dynamic_xchgr8r8(DecodedOp* op) {
    // this is a weird case where we could write to the same hardware register, like AH and AL would both write to EAX.  When they sync back, the last one would win since writing back 8-bit regs doesn't mask the write back
    bool sameReg = (op->reg + 4 == op->rm) || (op->reg == op->rm + 4);
    if (sameReg) {
        RegPtr reg = getTmpReg8(op->reg > op->rm ? op->reg : op->rm);
        RegPtr rm = getTmpReg8(op->reg > op->rm ? op->rm : op->reg);
        RegPtr actualReg = getReg(op->reg & 3);

        shrValue(DYN_16bit, reg, 8, false);
        shlValue(DYN_16bit, rm, 8, false);
        orReg(DYN_16bit, reg, rm, false);
        mov(DYN_16bit, actualReg, reg);
    } else {
        RegPtr reg = getReg8(op->reg);
        RegPtr rm = getReg8(op->rm);
        xchgReg(DYN_8bit, reg, rm);
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_xchge8r8(DecodedOp* op) {
    readWriteMem(DYN_8bit, calculateEaa(op), [op, this](RegPtr value) {
        RegPtr reg = getReg8(op->reg);
        xchgReg(DYN_8bit, reg, value);
    });
    incrementEip(op->len);
}
void DynamicData::dynamic_xchgr16r16(DecodedOp* op) {
    RegPtr reg = getReg(op->reg);
    RegPtr rm = getReg(op->rm);
    xchgReg(DYN_16bit, reg, rm);
    incrementEip(op->len);
}
void DynamicData::dynamic_xchge16r16(DecodedOp* op) {
    readWriteMem(DYN_16bit, calculateEaa(op), [op, this](RegPtr value) {
        RegPtr reg = getReg(op->reg);
        xchgReg(DYN_16bit, reg, value);
    });
    incrementEip(op->len);
}
void DynamicData::dynamic_xchgr32r32(DecodedOp* op) {
    RegPtr reg = getReg(op->reg);
    RegPtr rm = getReg(op->rm);
    xchgReg(DYN_32bit, reg, rm);
    incrementEip(op->len);
}
void DynamicData::dynamic_xchge32r32(DecodedOp* op) {
    readWriteMem(DYN_32bit, calculateEaa(op), [op, this](RegPtr value) {
        RegPtr reg = getReg(op->reg);
        xchgReg(DYN_32bit, reg, value);
    });
    incrementEip(op->len);
}

// I didn't see Quake 2 or Cinebench trigger these, so for now they are low priority for inlining
void DynamicData::dynamic_cmpxchgr8r8(DecodedOp* op) {
    call_II(common_cmpxchgr8r8, op->reg, op->rm);
    currentLazyFlags=nullptr;
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchge8r8(DecodedOp* op) {
    call_RI(common_cmpxchge8r8, DYN_32bit, calculateEaa(op), op->reg);
    currentLazyFlags=nullptr;
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchgr16r16(DecodedOp* op) {
    call_II(common_cmpxchgr16r16, op->reg, op->rm);
    currentLazyFlags=nullptr;
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchge16r16(DecodedOp* op) {
    call_RI(common_cmpxchge16r16, DYN_32bit, calculateEaa(op), op->reg);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchgr32r32(DecodedOp* op) {
    call_II(common_cmpxchgr32r32, op->reg, op->rm);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchge32r32(DecodedOp* op) {
    call_RI(common_cmpxchge32r32, DYN_32bit, calculateEaa(op), op->reg);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
