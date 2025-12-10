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

void Jit::dynamic_movr8r8(DecodedOp* op) {
    mov(JitWidth::b8, getReg8(op->reg), getReadOnlyReg8(op->rm));
    incrementEip(op->len);
}
void Jit::dynamic_move8r8(DecodedOp* op) {
    write(JitWidth::b8, calculateEaa(op), getReadOnlyReg8(op->reg));
    incrementEip(op->len);
}
void Jit::dynamic_movr8e8(DecodedOp* op) {
    mov(JitWidth::b8, getReg8(op->reg), read(JitWidth::b8, calculateEaa(op)));
    incrementEip(op->len);
}
void Jit::dynamic_movr8(DecodedOp* op) {
    movValue(JitWidth::b8, getReg8(op->reg), op->imm);
    incrementEip(op->len);
}
void Jit::dynamic_move8(DecodedOp* op) {
    writeValue(JitWidth::b8, calculateEaa(op), op->imm);
    incrementEip(op->len);
}
void Jit::dynamic_movr16r16(DecodedOp* op) {
    mov(JitWidth::b16, getReg(op->reg), getReadOnlyReg(op->rm));
    incrementEip(op->len);
}
void Jit::dynamic_move16r16(DecodedOp* op) {
    write(JitWidth::b16, calculateEaa(op), getReadOnlyReg(op->reg));
    incrementEip(op->len);
}
void Jit::dynamic_movr16e16(DecodedOp* op) {
    mov(JitWidth::b16, getReg(op->reg), read(JitWidth::b16, calculateEaa(op)));
    incrementEip(op->len);
}
void Jit::dynamic_movr16(DecodedOp* op) {
    movValue(JitWidth::b16, getReg(op->reg), op->imm);
    incrementEip(op->len);
}
void Jit::dynamic_move16(DecodedOp* op) {
    writeValue(JitWidth::b16, calculateEaa(op), op->imm);
    incrementEip(op->len);
}
void Jit::dynamic_movr32r32(DecodedOp* op) {
    mov(JitWidth::b32, getReg(op->reg, -1, false), getReadOnlyReg(op->rm));
    incrementEip(op->len);
}
void Jit::dynamic_move32r32(DecodedOp* op) {
    write(JitWidth::b32, calculateEaa(op), getReadOnlyReg(op->reg));
    incrementEip(op->len);
}
void Jit::dynamic_movr32e32(DecodedOp* op) {
    mov(JitWidth::b32, getReg(op->reg, -1, false), read(JitWidth::b32, calculateEaa(op)));
    incrementEip(op->len);
}
void Jit::dynamic_movr32(DecodedOp* op) {
    movValue(JitWidth::b32, getReg(op->reg, -1, false), op->imm);
    incrementEip(op->len);
}
void Jit::dynamic_move32(DecodedOp* op) {
    writeValue(JitWidth::b32, calculateEaa(op), op->imm);
    incrementEip(op->len);
}
void Jit::dynamic_movr16s16(DecodedOp* op) {
    mov(JitWidth::b16, getReg(op->reg), getReadOnlySegValue(op->rm));
    incrementEip(op->len);
}
void Jit::dynamic_movr32s16(DecodedOp* op) {
    mov(JitWidth::b32, getReg(op->reg, -1, false), getReadOnlySegValue(op->rm));
    incrementEip(op->len);
}
void Jit::dynamic_move16s16(DecodedOp* op) {
    write(JitWidth::b16, calculateEaa(op), getReadOnlySegValue(op->reg));
    incrementEip(op->len);
}
void Jit::dynamic_movs16e16(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_movs16r16(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_movAlOb(DecodedOp* op) {
    RegPtr reg;
    if (cpu->thread->process->hasSetSeg[op->base]) {
        reg = getTmpSegAddress(op->base);
        addValue(JitWidth::b32, reg, op->data.disp);
    } else {
        reg = getTmpReg();
        movValue(JitWidth::b32, reg, op->data.disp);
    }
    mov(JitWidth::b8, getReg8(0), read(JitWidth::b8, reg));
    incrementEip(op->len);
}
void Jit::dynamic_movAxOw(DecodedOp* op) {
    RegPtr reg;
    if (cpu->thread->process->hasSetSeg[op->base]) {
        reg = getTmpSegAddress(op->base);
        addValue(JitWidth::b32, reg, op->data.disp);
    } else {
        reg = getTmpReg();
        movValue(JitWidth::b32, reg, op->data.disp);
    }
    mov(JitWidth::b16, getReg(0), read(JitWidth::b16, reg));
    incrementEip(op->len);
}
void Jit::dynamic_movEaxOd(DecodedOp* op) {
    RegPtr reg;
    if (cpu->thread->process->hasSetSeg[op->base]) {
        reg = getTmpSegAddress(op->base);
        addValue(JitWidth::b32, reg, op->data.disp);
    } else {
        reg = getTmpReg();
        movValue(JitWidth::b32, reg, op->data.disp);
    }
    mov(JitWidth::b32, getReg(0, -1, false), read(JitWidth::b32, reg));
    incrementEip(op->len);
}
void Jit::dynamic_movObAl(DecodedOp* op) {
    RegPtr reg;
    if (cpu->thread->process->hasSetSeg[op->base]) {
        reg = getTmpSegAddress(op->base);
        addValue(JitWidth::b32, reg, op->data.disp);
    } else {
        reg = getTmpReg();
        movValue(JitWidth::b32, reg, op->data.disp);
    }
    write(JitWidth::b8, reg, getReadOnlyReg8(0));
    incrementEip(op->len);
}
void Jit::dynamic_movOwAx(DecodedOp* op) {
    RegPtr reg;
    if (cpu->thread->process->hasSetSeg[op->base]) {
        reg = getTmpSegAddress(op->base);
        addValue(JitWidth::b32, reg, op->data.disp);
    } else {
        reg = getTmpReg();
        movValue(JitWidth::b32, reg, op->data.disp);
    }
    write(JitWidth::b16, reg, getReadOnlyReg(0));
    incrementEip(op->len);
}
void Jit::dynamic_movOdEax(DecodedOp* op) {
    RegPtr reg;
    if (cpu->thread->process->hasSetSeg[op->base]) {
        reg = getTmpSegAddress(op->base);
        addValue(JitWidth::b32, reg, op->data.disp);
    } else {
        reg = getTmpReg();
        movValue(JitWidth::b32, reg, op->data.disp);
    }
    write(JitWidth::b32, reg, getReadOnlyReg(0));
    incrementEip(op->len);
}
void Jit::dynamic_movGwXzR8(DecodedOp* op) {
    movzx(JitWidth::b16, getReg(op->reg), JitWidth::b8, getReadOnlyReg8(op->rm));
    incrementEip(op->len);
}
void Jit::dynamic_movGwXzE8(DecodedOp* op) {
    movzx(JitWidth::b16, getReg(op->reg), JitWidth::b8, read(JitWidth::b8, calculateEaa(op)));
    incrementEip(op->len);
}
void Jit::dynamic_movGwSxR8(DecodedOp* op) {
    movsx(JitWidth::b16, getReg(op->reg), JitWidth::b8, getReadOnlyReg8(op->rm));
    incrementEip(op->len);
}
void Jit::dynamic_movGwSxE8(DecodedOp* op) {
    movsx(JitWidth::b16, getReg(op->reg), JitWidth::b8, read(JitWidth::b8, calculateEaa(op)));
    incrementEip(op->len);
}
void Jit::dynamic_movGdXzR8(DecodedOp* op) {
    movzx(JitWidth::b32, getReg(op->reg, -1, false), JitWidth::b8, getReadOnlyReg8(op->rm));
    incrementEip(op->len);
}
void Jit::dynamic_movGdXzE8(DecodedOp* op) {
    movzx(JitWidth::b32, getReg(op->reg, -1, false), JitWidth::b8, read(JitWidth::b8, calculateEaa(op)));
    incrementEip(op->len);
}
void Jit::dynamic_movGdSxR8(DecodedOp* op) {
    movsx(JitWidth::b32, getReg(op->reg, -1, false), JitWidth::b8, getReadOnlyReg8(op->rm));
    incrementEip(op->len);
}
void Jit::dynamic_movGdSxE8(DecodedOp* op) {
    movsx(JitWidth::b32, getReg(op->reg, -1, false), JitWidth::b8, read(JitWidth::b8, calculateEaa(op)));
    incrementEip(op->len);
}
void Jit::dynamic_movGdXzR16(DecodedOp* op) {
    movzx(JitWidth::b32, getReg(op->reg, -1, false), JitWidth::b16, getReadOnlyReg(op->rm));
    incrementEip(op->len);
}
void Jit::dynamic_movGdXzE16(DecodedOp* op) {
    movzx(JitWidth::b32, getReg(op->reg, -1, false), JitWidth::b16, read(JitWidth::b16, calculateEaa(op)));
    incrementEip(op->len);
}
void Jit::dynamic_movGdSxR16(DecodedOp* op) {
    movsx(JitWidth::b32, getReg(op->reg, -1, false), JitWidth::b16, getReadOnlyReg(op->rm));
    incrementEip(op->len);
}
void Jit::dynamic_movGdSxE16(DecodedOp* op) {
    movsx(JitWidth::b32, getReg(op->reg, -1, false), JitWidth::b16, read(JitWidth::b16, calculateEaa(op)));
    incrementEip(op->len);
}
void Jit::dynamic_movRdCRx(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_movCRxRd(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_leaR16(DecodedOp* op) {
    mov(JitWidth::b16, getReg(op->reg), calculateEaa(op));
    incrementEip(op->len);
}
void Jit::dynamic_leaR32(DecodedOp* op) {
    mov(JitWidth::b32, getReg(op->reg, -1, false), calculateEaa(op));
    incrementEip(op->len);
}
