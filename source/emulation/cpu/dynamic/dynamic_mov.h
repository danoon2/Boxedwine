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

void dynamic_movr8r8(DynamicData* data, DecodedOp* op) {
    loadRegStoreReg(data, op->reg, op->rm, DYN_8bit, DYN_ANY, true);
    INCREMENT_EIP(data, op);
}
void dynamic_move8r8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); 
    DynReg reg = loadReg(data, op->reg, DYN_SRC, DYN_8bit); 
    movToMemFromReg(data, DYN_ADDRESS, reg, DYN_8bit, true, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_movr8e8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); 
    storeRegFromMem(data, op->reg, DYN_8bit, DYN_ADDRESS, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr8(DynamicData* data, DecodedOp* op) {
    storeReg(data, op->reg, DYN_8bit, op->imm);
    INCREMENT_EIP(data, op);
}
void dynamic_move8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movToMemFromImm(data, DYN_ADDRESS, DYN_8bit, op->imm, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_movr16r16(DynamicData* data, DecodedOp* op) {
    loadRegStoreReg(data, op->reg, op->rm, DYN_16bit, DYN_ANY, true);
    INCREMENT_EIP(data, op);
}
void dynamic_move16r16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); 
    DynReg reg = loadReg(data, op->reg, DYN_SRC, DYN_16bit);
    movToMemFromReg(data, DYN_ADDRESS, reg, DYN_16bit, true, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_movr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    storeRegFromMem(data, op->reg, DYN_16bit, DYN_ADDRESS, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr16(DynamicData* data, DecodedOp* op) {
    storeReg(data, op->reg, DYN_16bit, op->imm);
    INCREMENT_EIP(data, op);
}
void dynamic_move16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movToMemFromImm(data, DYN_ADDRESS, DYN_16bit, op->imm, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_movr32r32(DynamicData* data, DecodedOp* op) {
    loadRegStoreReg(data, op->reg, op->rm, DYN_32bit, DYN_ANY, true);
    INCREMENT_EIP(data, op);
}
void dynamic_move32r32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); 
    DynReg reg = loadReg(data, op->reg, DYN_SRC, DYN_32bit);
    movToMemFromReg(data, DYN_ADDRESS, reg, DYN_32bit, true, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_movr32e32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    storeRegFromMem(data, op->reg, DYN_32bit, DYN_ADDRESS, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr32(DynamicData* data, DecodedOp* op) {
    storeReg(data, op->reg, DYN_32bit, op->imm);
    INCREMENT_EIP(data, op);
}
void dynamic_move32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movToMemFromImm(data, DYN_ADDRESS, DYN_32bit, op->imm, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_movr16s16(DynamicData* data, DecodedOp* op) {
    loadSegValue(data, op->rm, DYN_SRC);
    movToRegFromReg(data, DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit, false);
    storeReg(data, op->reg, DYN_SRC, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr32s16(DynamicData* data, DecodedOp* op) {
    loadSegValueStoreReg(data, op->reg, op->rm, DYN_ANY, true);
    INCREMENT_EIP(data, op);
}
void dynamic_move16s16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); 
    loadSegValue(data, op->reg, DYN_SRC);
    movToRegFromReg(data, DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit, false); 
    movToMemFromReg(data, DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_movs16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_movs16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_movAlOb(DynamicData* data, DecodedOp* op) {
    loadSegAddress(data, op->base, DYN_ADDRESS);
    instRegImm(data, '+', DYN_ADDRESS, DYN_32bit, op->data.disp);
    movFromMem(data, DYN_8bit, DYN_ADDRESS, true);
    storeReg(data, 0, DYN_CALL_RESULT, DYN_8bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movAxOw(DynamicData* data, DecodedOp* op) {
    loadSegAddress(data, op->base, DYN_ADDRESS);
    instRegImm(data, '+', DYN_ADDRESS, DYN_32bit, op->data.disp);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    storeReg(data, 0, DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movEaxOd(DynamicData* data, DecodedOp* op) {
    loadSegAddress(data, op->base, DYN_ADDRESS);
    instRegImm(data, '+', DYN_ADDRESS, DYN_32bit, op->data.disp);
    movFromMem(data, DYN_32bit, DYN_ADDRESS, true);
    storeReg(data, 0, DYN_CALL_RESULT, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movObAl(DynamicData* data, DecodedOp* op) {
    loadSegAddress(data, op->base, DYN_ADDRESS);
    instRegImm(data, '+', DYN_ADDRESS, DYN_32bit, op->data.disp);
    DynReg reg = loadReg(data, 0, DYN_SRC, DYN_8bit);
    movToMemFromReg(data, DYN_ADDRESS, reg, DYN_8bit, true, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_movOwAx(DynamicData* data, DecodedOp* op) {
    loadSegAddress(data, op->base, DYN_ADDRESS);
    instRegImm(data, '+', DYN_ADDRESS, DYN_32bit, op->data.disp);
    DynReg reg = loadReg(data, 0, DYN_SRC, DYN_16bit);
    movToMemFromReg(data, DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_movOdEax(DynamicData* data, DecodedOp* op) {
    loadSegAddress(data, op->base, DYN_ADDRESS);
    instRegImm(data, '+', DYN_ADDRESS, DYN_32bit, op->data.disp);
    DynReg reg = loadReg(data, 0, DYN_SRC, DYN_32bit);
    movToMemFromReg(data, DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_movGwXzR8(DynamicData* data, DecodedOp* op) {
    DynReg src = loadReg(data, op->rm, DYN_SRC, DYN_8bit);
    movToRegFromReg(data, DYN_SRC, DYN_16bit, src, DYN_8bit, false); 
    storeReg(data, op->reg, DYN_SRC, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGwXzE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); 
    movFromMem(data, DYN_8bit, DYN_ADDRESS, true);  
    movToRegFromReg(data, DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_8bit, false); 
    storeReg(data, op->reg, DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGwSxR8(DynamicData* data, DecodedOp* op) {
    DynReg src = loadReg(data, op->rm, DYN_SRC, DYN_8bit); 
    movToRegFromRegSignExtend(data, DYN_SRC, DYN_16bit, src, DYN_8bit, false); 
    storeReg(data, op->reg, DYN_SRC, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGwSxE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); 
    movFromMem(data, DYN_8bit, DYN_ADDRESS, true);  
    movToRegFromRegSignExtend(data, DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_8bit, false); 
    storeReg(data, op->reg, DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdXzR8(DynamicData* data, DecodedOp* op) {
    DynReg src = loadReg(data, op->rm, DYN_SRC, DYN_8bit); 
    movToRegFromReg(data, DYN_SRC, DYN_32bit, src, DYN_8bit, false); 
    storeReg(data, op->reg, DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdXzE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); 
    movFromMem(data, DYN_8bit, DYN_ADDRESS, true);
    movToRegFromReg(data, DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false);
    storeReg(data, op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdSxR8(DynamicData* data, DecodedOp* op) {
    DynReg src = loadReg(data, op->rm, DYN_SRC, DYN_8bit); 
    movToRegFromRegSignExtend(data, DYN_SRC, DYN_32bit, src, DYN_8bit, false); 
    storeReg(data, op->reg, DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdSxE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_8bit, DYN_ADDRESS, true);
    movToRegFromRegSignExtend(data, DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false);
    storeReg(data, op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdXzR16(DynamicData* data, DecodedOp* op) {
    DynReg src = loadReg(data, op->rm, DYN_SRC, DYN_16bit); 
    movToRegFromReg(data, DYN_SRC, DYN_32bit, src, DYN_16bit, false); 
    storeReg(data, op->reg, DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdXzE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    movToRegFromReg(data, DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    storeReg(data, op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdSxR16(DynamicData* data, DecodedOp* op) {
    DynReg src = loadReg(data, op->rm, DYN_SRC, DYN_16bit); 
    movToRegFromRegSignExtend(data, DYN_SRC, DYN_32bit, src, DYN_16bit, false); 
    storeReg(data, op->reg, DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdSxE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    movToRegFromRegSignExtend(data, DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    storeReg(data, op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movRdCRx(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_readCrx, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_movCRxRd(DynamicData* data, DecodedOp* op) {
    DynReg reg = loadReg(data, op->reg, DYN_SRC, DYN_32bit);
    callHostFunction(data, (void*)common_writeCrx, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, reg, DYN_PARAM_REG_32, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}

void dynamic_leaR16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    storeReg(data, op->reg, DYN_ADDRESS, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_leaR32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    storeReg(data, op->reg, DYN_ADDRESS, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
