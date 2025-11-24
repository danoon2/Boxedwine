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
void Jit::dynamic_xchgr8r8(DecodedOp* op) {
    // this is a weird case where we could write to the same hardware register, like AH and AL would both write to EAX.  When they sync back, the last one would win since writing back 8-bit regs doesn't mask the write back
    bool sameReg = (op->reg + 4 == op->rm) || (op->reg == op->rm + 4);
    if (sameReg) {
        RegPtr reg = getTmpReg8(op->reg > op->rm ? op->reg : op->rm);
        RegPtr rm = getTmpReg8(op->reg > op->rm ? op->rm : op->reg);
        RegPtr actualReg = getReg(op->reg & 3);

        shrValue(JitWidth::b16, reg, 8);
        shlValue(JitWidth::b16, rm, 8);
        orReg(JitWidth::b16, reg, rm);
        mov(JitWidth::b16, actualReg, reg);
    } else {
        RegPtr reg = getReg8(op->reg);
        RegPtr rm = getReg8(op->rm);
        xchgReg(JitWidth::b8, reg, rm);
    }
    incrementEip(op->len);
}
void Jit::dynamic_xchge8r8(DecodedOp* op) {
    readWriteMem(JitWidth::b8, calculateEaa(op), [op, this](RegPtr value) {
        RegPtr reg = getReg8(op->reg);
        xchgReg(JitWidth::b8, reg, value);
    });
    incrementEip(op->len);
}
void Jit::dynamic_xchgr16r16(DecodedOp* op) {
    RegPtr reg = getReg(op->reg);
    RegPtr rm = getReg(op->rm);
    xchgReg(JitWidth::b16, reg, rm);
    incrementEip(op->len);
}
void Jit::dynamic_xchge16r16(DecodedOp* op) {
    readWriteMem(JitWidth::b16, calculateEaa(op), [op, this](RegPtr value) {
        RegPtr reg = getReg(op->reg);
        xchgReg(JitWidth::b16, reg, value);
        });
    incrementEip(op->len);
}
void Jit::dynamic_xchgr32r32(DecodedOp* op) {
    RegPtr reg = getReg(op->reg);
    RegPtr rm = getReg(op->rm);
    xchgReg(JitWidth::b32, reg, rm);
    incrementEip(op->len);
}
void Jit::dynamic_xchge32r32(DecodedOp* op) {
    readWriteMem(JitWidth::b32, calculateEaa(op), [op, this](RegPtr value) {
        RegPtr reg = getReg(op->reg);
        xchgReg(JitWidth::b32, reg, value);
    });
    incrementEip(op->len);
}

// I didn't see Quake 2 or Cinebench trigger these, so for now they are low priority for inlining
void Jit::dynamic_cmpxchgr8r8(DecodedOp* op) {
    call_II(common_cmpxchgr8r8, op->reg, op->rm);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
void Jit::dynamic_cmpxchge8r8(DecodedOp* op) {
    call_RI(common_cmpxchge8r8, JitWidth::b32, calculateEaa(op), op->reg);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
void Jit::dynamic_cmpxchgr16r16(DecodedOp* op) {
    call_II(common_cmpxchgr16r16, op->reg, op->rm);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
void Jit::dynamic_cmpxchge16r16(DecodedOp* op) {
    call_RI(common_cmpxchge16r16, JitWidth::b32, calculateEaa(op), op->reg);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
void Jit::dynamic_cmpxchgr32r32(DecodedOp* op) {
    call_II(common_cmpxchgr32r32, op->reg, op->rm);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
void Jit::dynamic_cmpxchge32r32(DecodedOp* op) {
    call_RI(common_cmpxchge32r32, JitWidth::b32, calculateEaa(op), op->reg);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
