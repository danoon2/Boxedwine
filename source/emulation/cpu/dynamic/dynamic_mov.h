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
    data->loadRegStoreReg(op->reg, op->rm, DYN_8bit, DYN_SRC, true);
    data->incrementEip(op->len);
}
void dynamic_move8r8(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS); 
    DynReg reg = data->loadReg(op->reg, DYN_SRC, DYN_8bit); 
    data->movToMemFromReg(DYN_ADDRESS, reg, DYN_8bit, true, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_movr8e8(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS); 
    data->storeRegFromMem(op->reg, DYN_8bit, DYN_ADDRESS, true, true);
    data->incrementEip(op->len);
}
void dynamic_movr8(DynamicData* data, DecodedOp* op) {
    data->storeReg(op->reg, DYN_8bit, op->imm);
    data->incrementEip(op->len);
}
void dynamic_move8(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS); data->movToMemFromImm(DYN_ADDRESS, DYN_8bit, op->imm, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_movr16r16(DynamicData* data, DecodedOp* op) {
    data->loadRegStoreReg(op->reg, op->rm, DYN_16bit, DYN_SRC, true);
    data->incrementEip(op->len);
}
void dynamic_move16r16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS); 
    DynReg reg = data->loadReg(op->reg, DYN_SRC, DYN_16bit);
    data->movToMemFromReg(DYN_ADDRESS, reg, DYN_16bit, true, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_movr16e16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->storeRegFromMem(op->reg, DYN_16bit, DYN_ADDRESS, true, true);
    data->incrementEip(op->len);
}
void dynamic_movr16(DynamicData* data, DecodedOp* op) {
    data->storeReg(op->reg, DYN_16bit, op->imm);
    data->incrementEip(op->len);
}
void dynamic_move16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS); data->movToMemFromImm(DYN_ADDRESS, DYN_16bit, op->imm, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_movr32r32(DynamicData* data, DecodedOp* op) {
    data->loadRegStoreReg(op->reg, op->rm, DYN_32bit, DYN_SRC, true);
    data->incrementEip(op->len);
}
void dynamic_move32r32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS); 
    DynReg reg = data->loadReg(op->reg, DYN_SRC, DYN_32bit);
    data->movToMemFromReg(DYN_ADDRESS, reg, DYN_32bit, true, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_movr32e32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->storeRegFromMem(op->reg, DYN_32bit, DYN_ADDRESS, true, true);
    data->incrementEip(op->len);
}
void dynamic_movr32(DynamicData* data, DecodedOp* op) {
    data->storeReg(op->reg, DYN_32bit, op->imm);
    data->incrementEip(op->len);
}
void dynamic_move32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS); data->movToMemFromImm(DYN_ADDRESS, DYN_32bit, op->imm, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_movr16s16(DynamicData* data, DecodedOp* op) {
    data->loadSegValue(op->rm, DYN_SRC);
    data->movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit, false);
    data->storeReg(op->reg, DYN_SRC, DYN_16bit, true);
    data->incrementEip(op->len);
}
void dynamic_movr32s16(DynamicData* data, DecodedOp* op) {
    data->loadSegValueStoreReg(op->reg, op->rm, DYN_SRC, true);
    data->incrementEip(op->len);
}
void dynamic_move16s16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS); 
    data->loadSegValue(op->reg, DYN_SRC);
    data->movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit, false); 
    data->movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_movs16e16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    data->IfNot(DYN_CALL_RESULT, true);
    data->blockDone(true);
    data->EndIf();
    data->incrementEip(op->len);
}
void dynamic_movs16r16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    data->IfNot(DYN_CALL_RESULT, true);
    data->blockDone(true);
    data->EndIf();
    data->incrementEip(op->len);
}
void dynamic_movAlOb(DynamicData* data, DecodedOp* op) {
    data->loadSegAddress(op->base, DYN_ADDRESS);
    data->instRegImm('+', DYN_ADDRESS, DYN_32bit, op->data.disp);
    data->movFromMem(DYN_8bit, DYN_ADDRESS, true);
    data->storeReg(0, DYN_CALL_RESULT, DYN_8bit, true);
    data->incrementEip(op->len);
}
void dynamic_movAxOw(DynamicData* data, DecodedOp* op) {
    data->loadSegAddress(op->base, DYN_ADDRESS);
    data->instRegImm('+', DYN_ADDRESS, DYN_32bit, op->data.disp);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->storeReg(0, DYN_CALL_RESULT, DYN_16bit, true);
    data->incrementEip(op->len);
}
void dynamic_movEaxOd(DynamicData* data, DecodedOp* op) {
    data->loadSegAddress(op->base, DYN_ADDRESS);
    data->instRegImm('+', DYN_ADDRESS, DYN_32bit, op->data.disp);
    data->movFromMem(DYN_32bit, DYN_ADDRESS, true);
    data->storeReg(0, DYN_CALL_RESULT, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_movObAl(DynamicData* data, DecodedOp* op) {
    data->loadSegAddress(op->base, DYN_ADDRESS);
    data->instRegImm('+', DYN_ADDRESS, DYN_32bit, op->data.disp);
    DynReg reg = data->loadReg(0, DYN_SRC, DYN_8bit);
    data->movToMemFromReg(DYN_ADDRESS, reg, DYN_8bit, true, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_movOwAx(DynamicData* data, DecodedOp* op) {
    data->loadSegAddress(op->base, DYN_ADDRESS);
    data->instRegImm('+', DYN_ADDRESS, DYN_32bit, op->data.disp);
    DynReg reg = data->loadReg(0, DYN_SRC, DYN_16bit);
    data->movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_movOdEax(DynamicData* data, DecodedOp* op) {
    data->loadSegAddress(op->base, DYN_ADDRESS);
    data->instRegImm('+', DYN_ADDRESS, DYN_32bit, op->data.disp);
    DynReg reg = data->loadReg(0, DYN_SRC, DYN_32bit);
    data->movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_movGwXzR8(DynamicData* data, DecodedOp* op) {
    DynReg src = data->loadReg(op->rm, DYN_SRC, DYN_8bit);
    data->movToRegFromReg(DYN_SRC, DYN_16bit, src, DYN_8bit, false); 
    data->storeReg(op->reg, DYN_SRC, DYN_16bit, true);
    data->incrementEip(op->len);
}
void dynamic_movGwXzE8(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS); 
    data->movFromMem(DYN_8bit, DYN_ADDRESS, true);  
    data->movToRegFromReg(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_8bit, false); 
    data->storeReg(op->reg, DYN_CALL_RESULT, DYN_16bit, true);
    data->incrementEip(op->len);
}
void dynamic_movGwSxR8(DynamicData* data, DecodedOp* op) {
    DynReg src = data->loadReg(op->rm, DYN_SRC, DYN_8bit); 
    data->movToRegFromRegSignExtend(DYN_SRC, DYN_16bit, src, DYN_8bit, false); 
    data->storeReg(op->reg, DYN_SRC, DYN_16bit, true);
    data->incrementEip(op->len);
}
void dynamic_movGwSxE8(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS); 
    data->movFromMem(DYN_8bit, DYN_ADDRESS, true);  
    data->movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_8bit, false); 
    data->storeReg(op->reg, DYN_CALL_RESULT, DYN_16bit, true);
    data->incrementEip(op->len);
}
void dynamic_movGdXzR8(DynamicData* data, DecodedOp* op) {
    DynReg src = data->loadReg(op->rm, DYN_SRC, DYN_8bit); 
    data->movToRegFromReg(DYN_SRC, DYN_32bit, src, DYN_8bit, false); 
    data->storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_movGdXzE8(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS); 
    data->movFromMem(DYN_8bit, DYN_ADDRESS, true);
    data->movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false);
    data->storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_movGdSxR8(DynamicData* data, DecodedOp* op) {
    DynReg src = data->loadReg(op->rm, DYN_SRC, DYN_8bit); 
    data->movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, src, DYN_8bit, false); 
    data->storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_movGdSxE8(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_8bit, DYN_ADDRESS, true);
    data->movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false);
    data->storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_movGdXzR16(DynamicData* data, DecodedOp* op) {
    DynReg src = data->loadReg(op->rm, DYN_SRC, DYN_16bit); 
    data->movToRegFromReg(DYN_SRC, DYN_32bit, src, DYN_16bit, false); 
    data->storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_movGdXzE16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    data->storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_movGdSxR16(DynamicData* data, DecodedOp* op) {
    DynReg src = data->loadReg(op->rm, DYN_SRC, DYN_16bit); 
    data->movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, src, DYN_16bit, false); 
    data->storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_movGdSxE16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    data->storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_movRdCRx(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_readCrx, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->IfNot(DYN_CALL_RESULT, true);
    data->blockDone(true);
    data->EndIf();
    data->incrementEip(op->len);
}
void dynamic_movCRxRd(DynamicData* data, DecodedOp* op) {
    DynReg reg = data->loadReg(op->reg, DYN_SRC, DYN_32bit);
    data->callHostFunction((void*)common_writeCrx, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, reg, DYN_PARAM_REG_32, true);
    data->IfNot(DYN_CALL_RESULT, true);
    data->blockDone(true);
    data->EndIf();
    data->incrementEip(op->len);
}

void dynamic_leaR16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->storeReg(op->reg, DYN_ADDRESS, DYN_16bit, true);
    data->incrementEip(op->len);
}
void dynamic_leaR32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->storeReg(op->reg, DYN_ADDRESS, DYN_32bit, true);
    data->incrementEip(op->len);
}
