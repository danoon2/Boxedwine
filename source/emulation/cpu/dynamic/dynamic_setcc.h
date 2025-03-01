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

void dynamic_setO_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, O);
    INCREMENT_EIP(data, op);
}
void dynamic_setO_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, O, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setNO_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, NO);
    INCREMENT_EIP(data, op);
}
void dynamic_setNO_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, NO, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setB_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, B);
    INCREMENT_EIP(data, op);
}
void dynamic_setB_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, B, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setNB_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, NB);
    INCREMENT_EIP(data, op);
}
void dynamic_setNB_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, NB, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setZ_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, Z);
    INCREMENT_EIP(data, op);
}
void dynamic_setZ_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, Z, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setNZ_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, NZ);
    INCREMENT_EIP(data, op);
}
void dynamic_setNZ_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, NZ, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setBE_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, BE);
    INCREMENT_EIP(data, op);
}
void dynamic_setBE_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, BE, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setNBE_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, NBE);
    INCREMENT_EIP(data, op);
}
void dynamic_setNBE_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, NBE, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setS_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, S);
    INCREMENT_EIP(data, op);
}
void dynamic_setS_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, S, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setNS_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, NS);
    INCREMENT_EIP(data, op);
}
void dynamic_setNS_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, NS, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setP_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, P);
    INCREMENT_EIP(data, op);
}
void dynamic_setP_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, P, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setNP_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, NP);
    INCREMENT_EIP(data, op);
}
void dynamic_setNP_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, NP, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setL_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, L);
    INCREMENT_EIP(data, op);
}
void dynamic_setL_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, L, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setNL_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, NL);
    INCREMENT_EIP(data, op);
}
void dynamic_setNL_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, NL, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setLE_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, LE);
    INCREMENT_EIP(data, op);
}
void dynamic_setLE_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, LE, true);
    INCREMENT_EIP(data, op);
}
void dynamic_setNLE_reg(DynamicData* data, DecodedOp* op) {
    setCPU(data, CPU::offsetofReg8(op->reg), DYN_8bit, NLE);
    INCREMENT_EIP(data, op);
}
void dynamic_setNLE_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(data, DYN_ADDRESS, DYN_8bit, NLE, true);
    INCREMENT_EIP(data, op);
}
