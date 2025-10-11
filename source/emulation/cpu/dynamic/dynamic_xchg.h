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
    loadReg(op->rm, DYN_DEST, DYN_8bit, true);
    loadRegStoreReg(op->rm, op->reg, DYN_8bit, DYN_SRC, true);
    storeReg(op->reg, DYN_DEST, DYN_8bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_xchge8r8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_8bit, DYN_ADDRESS, false);
    loadReg(op->reg, DYN_DEST, DYN_8bit, true);
    storeReg(op->reg, DYN_CALL_RESULT, DYN_8bit, true);
    movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_8bit, true, true, DYN_SRC);    
    incrementEip(op->len);
}
void DynamicData::dynamic_xchgr16r16(DecodedOp* op) {
    loadReg(op->rm, DYN_DEST, DYN_16bit, true);
    loadRegStoreReg(op->rm, op->reg, DYN_16bit, DYN_SRC, true);
    storeReg(op->reg, DYN_DEST, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_xchge16r16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, false);
    loadReg(op->reg, DYN_DEST, DYN_16bit, true);
    storeReg(op->reg, DYN_CALL_RESULT, DYN_16bit, true);
    movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_16bit, true, true, DYN_SRC);    
    incrementEip(op->len);
}
void DynamicData::dynamic_xchgr32r32(DecodedOp* op) {
    loadReg(op->rm, DYN_DEST, DYN_32bit, true);
    loadRegStoreReg(op->rm, op->reg, DYN_32bit, DYN_SRC, true);
    storeReg(op->reg, DYN_DEST, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_xchge32r32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    loadReg(op->reg, DYN_DEST, DYN_32bit, true);
    storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_32bit, true, true, DYN_SRC);
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchgr8r8(DecodedOp* op) {
    callHostFunction((void*)common_cmpxchgr8r8, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_CMP8;
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchge8r8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_cmpxchge8r8, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_CMP8;
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchgr16r16(DecodedOp* op) {
    callHostFunction((void*)common_cmpxchgr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_CMP16;
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchge16r16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_cmpxchge16r16, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_CMP16;
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchgr32r32(DecodedOp* op) {
    callHostFunction((void*)common_cmpxchgr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_CMP32;
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchge32r32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_cmpxchge32r32, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_CMP32;
    incrementEip(op->len);
}
