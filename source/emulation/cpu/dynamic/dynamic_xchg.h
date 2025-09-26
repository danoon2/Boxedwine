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
void dynamic_xchgr8r8(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(data, DYN_DEST, CPU::offsetofReg8(op->rm), DYN_8bit);
    movToCpuFromCpu(data, CPU::offsetofReg8(op->rm), CPU::offsetofReg8(op->reg), DYN_8bit, DYN_SRC, true);
    movToCpuFromReg(data, CPU::offsetofReg8(op->reg), DYN_DEST, DYN_8bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_xchge8r8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_8bit, DYN_ADDRESS, false);
    movToRegFromCpu(data, DYN_DEST, CPU::offsetofReg8(op->reg), DYN_8bit);
    movToCpuFromReg(data, CPU::offsetofReg8(op->reg), DYN_CALL_RESULT, DYN_8bit, true);
    movToMemFromReg(data, DYN_ADDRESS, DYN_DEST, DYN_8bit, true, true, DYN_SRC);    
    INCREMENT_EIP(data, op);
}
void dynamic_xchgr16r16(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(data, DYN_DEST, CPU::offsetofReg16(op->rm), DYN_16bit);
    movToCpuFromCpu(data, CPU::offsetofReg16(op->rm), CPU::offsetofReg16(op->reg), DYN_16bit, DYN_SRC, true);
    movToCpuFromReg(data, CPU::offsetofReg16(op->reg), DYN_DEST, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_xchge16r16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, false);
    movToRegFromCpu(data, DYN_DEST, CPU::offsetofReg16(op->reg), DYN_16bit);
    movToCpuFromReg(data, CPU::offsetofReg16(op->reg), DYN_CALL_RESULT, DYN_16bit, true);
    movToMemFromReg(data, DYN_ADDRESS, DYN_DEST, DYN_16bit, true, true, DYN_SRC);    
    INCREMENT_EIP(data, op);
}
void dynamic_xchgr32r32(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(data, DYN_DEST, CPU::offsetofReg32(op->rm), DYN_32bit);
    movToCpuFromCpu(data, CPU::offsetofReg32(op->rm), CPU::offsetofReg32(op->reg), DYN_32bit, DYN_SRC, true);
    movToCpuFromReg(data, CPU::offsetofReg32(op->reg), DYN_DEST, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_xchge32r32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_32bit, DYN_ADDRESS, false);
    movToRegFromCpu(data, DYN_DEST, CPU::offsetofReg32(op->reg), DYN_32bit);
    movToCpuFromReg(data, CPU::offsetofReg32(op->reg), DYN_CALL_RESULT, DYN_32bit, true);
    movToMemFromReg(data, DYN_ADDRESS, DYN_DEST, DYN_32bit, true, true, DYN_SRC);
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchgr8r8(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_cmpxchgr8r8, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_CMP8;
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchge8r8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_cmpxchge8r8, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_CMP8;
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchgr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_cmpxchgr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_CMP16;
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchge16r16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_cmpxchge16r16, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_CMP16;
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchgr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_cmpxchgr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_CMP32;
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchge32r32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_cmpxchge32r32, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_CMP32;
    INCREMENT_EIP(data, op);
}
