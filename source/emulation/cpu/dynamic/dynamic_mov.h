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

void DynamicData::dynamic_movr8r8(DecodedOp* op) {
    mov(DYN_8bit, getReg8(op->reg), getReadOnlyReg8(op->rm));
    incrementEip(op->len);
}
void DynamicData::dynamic_move8r8(DecodedOp* op) {
    write(DYN_8bit, calculateEaa2(op), getReadOnlyReg8(op->reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_movr8e8(DecodedOp* op) {
    mov(DYN_8bit, getReg8(op->reg), read(DYN_8bit, calculateEaa2(op)));
    incrementEip(op->len);
}
void DynamicData::dynamic_movr8(DecodedOp* op) {
    movValue(DYN_8bit, getReg8(op->reg), op->imm);
    incrementEip(op->len);
}
void DynamicData::dynamic_move8(DecodedOp* op) {
    writeValue(DYN_8bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr16r16(DecodedOp* op) {
    mov(DYN_16bit, getReg(op->reg), getReadOnlyReg(op->rm));
    incrementEip(op->len);
}
void DynamicData::dynamic_move16r16(DecodedOp* op) {
    write(DYN_16bit, calculateEaa2(op), getReadOnlyReg(op->reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_movr16e16(DecodedOp* op) {
    mov(DYN_16bit, getReg(op->reg), read(DYN_16bit, calculateEaa2(op)));
    incrementEip(op->len);
}
void DynamicData::dynamic_movr16(DecodedOp* op) {
    movValue(DYN_16bit, getReg(op->reg), op->imm);
    incrementEip(op->len);
}
void DynamicData::dynamic_move16(DecodedOp* op) {
    writeValue(DYN_16bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr32r32(DecodedOp* op) {
    mov(DYN_32bit, getReg(op->reg), getReadOnlyReg(op->rm));
    incrementEip(op->len);
}
void DynamicData::dynamic_move32r32(DecodedOp* op) {
    write(DYN_32bit, calculateEaa2(op), getReadOnlyReg(op->reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_movr32e32(DecodedOp* op) {
    mov(DYN_32bit, getReg(op->reg), read(DYN_32bit, calculateEaa2(op)));
    incrementEip(op->len);
}
void DynamicData::dynamic_movr32(DecodedOp* op) {
    movValue(DYN_32bit, getReg(op->reg), op->imm);
    incrementEip(op->len);
}
void DynamicData::dynamic_move32(DecodedOp* op) {
    writeValue(DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr16s16(DecodedOp* op) {
    mov(DYN_16bit, getReg(op->reg), getReadOnlySegValue(op->rm));
    incrementEip(op->len);
}
void DynamicData::dynamic_movr32s16(DecodedOp* op) {
    mov(DYN_32bit, getReg(op->reg), getReadOnlySegValue(op->rm));
    incrementEip(op->len);
}
void DynamicData::dynamic_move16s16(DecodedOp* op) {
    write(DYN_16bit, calculateEaa2(op), getReadOnlySegValue(op->reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_movs16e16(DecodedOp* op) {
    cpu->thread->process->hasSetSeg[op->reg] = true;
    // the weird getTmpReg is to override the behavior of read, to make it not use getTmpRegForCallResult so that EAX is available to use for callAndReturn
    RegPtr result = callAndReturn_IR(common_setSegment, op->reg, DYN_16bit, read(DYN_16bit, calculateEaa2(op), nullptr, nullptr, false, getTmpReg()));
    IfNot(DYN_32bit, result);
        blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_movs16r16(DecodedOp* op) {
    cpu->thread->process->hasSetSeg[op->reg] = true;
    RegPtr result = callAndReturn_IR(common_setSegment, op->rm, DYN_16bit, getReadOnlyReg(op->reg));
    IfNot(DYN_32bit, result);
        blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_movAlOb(DecodedOp* op) {    
    RegPtr reg;
    if (cpu->thread->process->hasSetSeg[op->base]) {
        reg = getTmpSegAddress(op->base);
        addValue(DYN_32bit, reg, op->data.disp, false);
    } else {
        reg = getTmpReg();
        movValue(DYN_32bit, reg, op->data.disp);        
    }
    mov(DYN_8bit, getReg8(0), read(DYN_8bit, reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_movAxOw(DecodedOp* op) {
    RegPtr reg;
    if (cpu->thread->process->hasSetSeg[op->base]) {
        reg = getTmpSegAddress(op->base);
        addValue(DYN_32bit, reg, op->data.disp, false);
    } else {
        reg = getTmpReg();
        movValue(DYN_32bit, reg, op->data.disp);        
    }
    mov(DYN_16bit, getReg(0), read(DYN_16bit, reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_movEaxOd(DecodedOp* op) {
    RegPtr reg;
    if (cpu->thread->process->hasSetSeg[op->base]) {
        reg = getTmpSegAddress(op->base);
        addValue(DYN_32bit, reg, op->data.disp, false);
    } else {
        reg = getTmpReg();
        movValue(DYN_32bit, reg, op->data.disp);
    }
    mov(DYN_32bit, getReg(0), read(DYN_32bit, reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_movObAl(DecodedOp* op) {
    RegPtr reg;
    if (cpu->thread->process->hasSetSeg[op->base]) {
        reg = getTmpSegAddress(op->base);
        addValue(DYN_32bit, reg, op->data.disp, false);
    } else {
        reg = getTmpReg();
        movValue(DYN_32bit, reg, op->data.disp);
    }
    write(DYN_8bit, reg, getReg8(0));
    incrementEip(op->len);
}
void DynamicData::dynamic_movOwAx(DecodedOp* op) {
    RegPtr reg;
    if (cpu->thread->process->hasSetSeg[op->base]) {
        reg = getTmpSegAddress(op->base);
        addValue(DYN_32bit, reg, op->data.disp, false);
    } else {
        reg = getTmpReg();
        movValue(DYN_32bit, reg, op->data.disp);
    }
    write(DYN_16bit, reg, getReg(0));
    incrementEip(op->len);
}
void DynamicData::dynamic_movOdEax(DecodedOp* op) {
    RegPtr reg;
    if (cpu->thread->process->hasSetSeg[op->base]) {
        reg = getTmpSegAddress(op->base);
        addValue(DYN_32bit, reg, op->data.disp, false);
    } else {
        reg = getTmpReg();
        movValue(DYN_32bit, reg, op->data.disp);
    }
    write(DYN_32bit, reg, getReg(0));
    incrementEip(op->len);
}
void DynamicData::dynamic_movGwXzR8(DecodedOp* op) {
    movzx(DYN_16bit, getReg(op->reg), DYN_8bit, getReadOnlyReg8(op->rm));
    incrementEip(op->len);
}
void DynamicData::dynamic_movGwXzE8(DecodedOp* op) {
    movzx(DYN_16bit, getReg(op->reg), DYN_8bit, read(DYN_8bit, calculateEaa2(op)));
    incrementEip(op->len);
}
void DynamicData::dynamic_movGwSxR8(DecodedOp* op) {
    movsx(DYN_16bit, getReg(op->reg), DYN_8bit, getReadOnlyReg8(op->rm));
    incrementEip(op->len);
}
void DynamicData::dynamic_movGwSxE8(DecodedOp* op) {
    movsx(DYN_16bit, getReg(op->reg), DYN_8bit, read(DYN_8bit, calculateEaa2(op)));
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdXzR8(DecodedOp* op) {
    movzx(DYN_32bit, getReg(op->reg), DYN_8bit, getReadOnlyReg8(op->rm));
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdXzE8(DecodedOp* op) {
    movzx(DYN_32bit, getReg(op->reg), DYN_8bit, read(DYN_8bit, calculateEaa2(op)));
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdSxR8(DecodedOp* op) {
    movsx(DYN_32bit, getReg(op->reg), DYN_8bit, getReadOnlyReg8(op->rm));
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdSxE8(DecodedOp* op) {
    movsx(DYN_32bit, getReg(op->reg), DYN_8bit, read(DYN_8bit, calculateEaa2(op)));
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdXzR16(DecodedOp* op) {
    movzx(DYN_32bit, getReg(op->reg), DYN_16bit, getReadOnlyReg(op->rm));
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdXzE16(DecodedOp* op) {
    movzx(DYN_32bit, getReg(op->reg), DYN_16bit, read(DYN_16bit, calculateEaa2(op)));
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdSxR16(DecodedOp* op) {
    movsx(DYN_32bit, getReg(op->reg), DYN_16bit, getReadOnlyReg(op->rm));
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdSxE16(DecodedOp* op) {
    movsx(DYN_32bit, getReg(op->reg), DYN_16bit, read(DYN_16bit, calculateEaa2(op)));
    incrementEip(op->len);
}
void DynamicData::dynamic_movRdCRx(DecodedOp* op) {
    RegPtr result = callAndReturn_II(common_readCrx, op->rm, op->reg);
    IfNot(DYN_32bit, result);
        blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_movCRxRd(DecodedOp* op) {
    RegPtr result = callAndReturn_IR(common_writeCrx, op->rm, DYN_32bit, getReadOnlyReg(op->reg));
    IfNot(DYN_32bit, result);
        blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_leaR16(DecodedOp* op) {
    mov(DYN_16bit, getReg(op->reg), calculateEaa2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_leaR32(DecodedOp* op) {
    mov(DYN_32bit, getReg(op->reg), calculateEaa2(op));
    incrementEip(op->len);
}
