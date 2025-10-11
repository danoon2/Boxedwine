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

void DynamicData::dynamic_setO_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, O);
    incrementEip(op->len);
}
void DynamicData::dynamic_setO_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, O, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNO_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NO);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNO_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, NO, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setB_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, B);
    incrementEip(op->len);
}
void DynamicData::dynamic_setB_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, B, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNB_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NB);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNB_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, NB, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setZ_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, Z);
    incrementEip(op->len);
}
void DynamicData::dynamic_setZ_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, Z, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNZ_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NZ);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNZ_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, NZ, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setBE_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, BE);
    incrementEip(op->len);
}
void DynamicData::dynamic_setBE_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, BE, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNBE_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NBE);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNBE_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, NBE, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setS_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, S);
    incrementEip(op->len);
}
void DynamicData::dynamic_setS_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, S, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNS_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NS);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNS_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, NS, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setP_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, P);
    incrementEip(op->len);
}
void DynamicData::dynamic_setP_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, P, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNP_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NP);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNP_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, NP, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setL_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, L);
    incrementEip(op->len);
}
void DynamicData::dynamic_setL_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, L, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNL_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NL);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNL_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, NL, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setLE_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, LE);
    incrementEip(op->len);
}
void DynamicData::dynamic_setLE_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, LE, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNLE_reg(DecodedOp* op) {
    setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NLE);
    incrementEip(op->len);
}
void DynamicData::dynamic_setNLE_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    setMem(DYN_ADDRESS, DYN_8bit, NLE, true, DYN_DEST);
    incrementEip(op->len);
}
