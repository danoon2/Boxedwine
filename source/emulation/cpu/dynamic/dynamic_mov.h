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
    loadRegStoreReg(op->reg, op->rm, DYN_8bit, DYN_SRC, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_move8r8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    DynReg reg = loadReg(op->reg, DYN_SRC, DYN_8bit); 
    movToMemFromReg(DYN_ADDRESS, reg, DYN_8bit, true, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr8e8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    storeRegFromMem(op->reg, DYN_8bit, DYN_ADDRESS, true, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr8(DecodedOp* op) {
    storeReg(op->reg, DYN_8bit, op->imm);
    incrementEip(op->len);
}
void DynamicData::dynamic_move8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToMemFromImm(DYN_ADDRESS, DYN_8bit, op->imm, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr16r16(DecodedOp* op) {
    loadRegStoreReg(op->reg, op->rm, DYN_16bit, DYN_SRC, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_move16r16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    DynReg reg = loadReg(op->reg, DYN_SRC, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, reg, DYN_16bit, true, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr16e16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    storeRegFromMem(op->reg, DYN_16bit, DYN_ADDRESS, true, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr16(DecodedOp* op) {
    storeReg(op->reg, DYN_16bit, op->imm);
    incrementEip(op->len);
}
void DynamicData::dynamic_move16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToMemFromImm(DYN_ADDRESS, DYN_16bit, op->imm, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr32r32(DecodedOp* op) {
    loadRegStoreReg(op->reg, op->rm, DYN_32bit, DYN_SRC, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_move32r32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    DynReg reg = loadReg(op->reg, DYN_SRC, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, reg, DYN_32bit, true, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr32e32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    storeRegFromMem(op->reg, DYN_32bit, DYN_ADDRESS, true, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr32(DecodedOp* op) {
    storeReg(op->reg, DYN_32bit, op->imm);
    incrementEip(op->len);
}
void DynamicData::dynamic_move32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToMemFromImm(DYN_ADDRESS, DYN_32bit, op->imm, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr16s16(DecodedOp* op) {
    loadSegValue(op->rm, DYN_SRC);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit, false);
    storeReg(op->reg, DYN_SRC, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movr32s16(DecodedOp* op) {
    loadSegValueStoreReg(op->reg, op->rm, DYN_SRC, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_move16s16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    loadSegValue(op->reg, DYN_SRC);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit, false); 
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_movs16e16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_movs16r16(DecodedOp* op) {
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_movAlOb(DecodedOp* op) {
    loadSegAddress(op->base, DYN_ADDRESS);
    addRegImm(DYN_ADDRESS, DYN_32bit, op->data.disp);
    movFromMem(DYN_8bit, DYN_ADDRESS, true);
    storeReg(0, DYN_CALL_RESULT, DYN_8bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movAxOw(DecodedOp* op) {
    loadSegAddress(op->base, DYN_ADDRESS);
    addRegImm(DYN_ADDRESS, DYN_32bit, op->data.disp);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    storeReg(0, DYN_CALL_RESULT, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movEaxOd(DecodedOp* op) {
    loadSegAddress(op->base, DYN_ADDRESS);
    addRegImm(DYN_ADDRESS, DYN_32bit, op->data.disp);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    storeReg(0, DYN_CALL_RESULT, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movObAl(DecodedOp* op) {
    loadSegAddress(op->base, DYN_ADDRESS);
    addRegImm(DYN_ADDRESS, DYN_32bit, op->data.disp);
    DynReg reg = loadReg(0, DYN_SRC, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, reg, DYN_8bit, true, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_movOwAx(DecodedOp* op) {
    loadSegAddress(op->base, DYN_ADDRESS);
    addRegImm(DYN_ADDRESS, DYN_32bit, op->data.disp);
    DynReg reg = loadReg(0, DYN_SRC, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_movOdEax(DecodedOp* op) {
    loadSegAddress(op->base, DYN_ADDRESS);
    addRegImm(DYN_ADDRESS, DYN_32bit, op->data.disp);
    DynReg reg = loadReg(0, DYN_SRC, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_movGwXzR8(DecodedOp* op) {
    DynReg src = loadReg(op->rm, DYN_SRC, DYN_8bit);
    movToRegFromReg(DYN_SRC, DYN_16bit, src, DYN_8bit, false); 
    storeReg(op->reg, DYN_SRC, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movGwXzE8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    movFromMem(DYN_8bit, DYN_ADDRESS, true);  
    movToRegFromReg(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_8bit, false); 
    storeReg(op->reg, DYN_CALL_RESULT, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movGwSxR8(DecodedOp* op) {
    DynReg src = loadReg(op->rm, DYN_SRC, DYN_8bit); 
    movToRegFromRegSignExtend(DYN_SRC, DYN_16bit, src, DYN_8bit, false); 
    storeReg(op->reg, DYN_SRC, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movGwSxE8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    movFromMem(DYN_8bit, DYN_ADDRESS, true);  
    movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_8bit, false); 
    storeReg(op->reg, DYN_CALL_RESULT, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdXzR8(DecodedOp* op) {
    DynReg src = loadReg(op->rm, DYN_SRC, DYN_8bit); 
    movToRegFromReg(DYN_SRC, DYN_32bit, src, DYN_8bit, false); 
    storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdXzE8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    movFromMem(DYN_8bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false);
    storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdSxR8(DecodedOp* op) {
    DynReg src = loadReg(op->rm, DYN_SRC, DYN_8bit); 
    movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, src, DYN_8bit, false); 
    storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdSxE8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_8bit, DYN_ADDRESS, true);
    movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false);
    storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdXzR16(DecodedOp* op) {
    DynReg src = loadReg(op->rm, DYN_SRC, DYN_16bit); 
    movToRegFromReg(DYN_SRC, DYN_32bit, src, DYN_16bit, false); 
    storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdXzE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdSxR16(DecodedOp* op) {
    DynReg src = loadReg(op->rm, DYN_SRC, DYN_16bit); 
    movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, src, DYN_16bit, false); 
    storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movGdSxE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_movRdCRx(DecodedOp* op) {
    callHostFunction((void*)common_readCrx, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_movCRxRd(DecodedOp* op) {
    DynReg reg = loadReg(op->reg, DYN_SRC, DYN_32bit);
    callHostFunction((void*)common_writeCrx, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, reg, DYN_PARAM_REG_32, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}

void DynamicData::dynamic_leaR16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    storeReg(op->reg, DYN_ADDRESS, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_leaR32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    storeReg(op->reg, DYN_ADDRESS, DYN_32bit, true);
    incrementEip(op->len);
}
