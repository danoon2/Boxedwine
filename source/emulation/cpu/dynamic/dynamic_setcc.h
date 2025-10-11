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
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, O);
    data->incrementEip(op->len);
}
void dynamic_setO_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, O, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setNO_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NO);
    data->incrementEip(op->len);
}
void dynamic_setNO_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, NO, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setB_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, B);
    data->incrementEip(op->len);
}
void dynamic_setB_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, B, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setNB_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NB);
    data->incrementEip(op->len);
}
void dynamic_setNB_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, NB, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setZ_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, Z);
    data->incrementEip(op->len);
}
void dynamic_setZ_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, Z, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setNZ_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NZ);
    data->incrementEip(op->len);
}
void dynamic_setNZ_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, NZ, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setBE_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, BE);
    data->incrementEip(op->len);
}
void dynamic_setBE_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, BE, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setNBE_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NBE);
    data->incrementEip(op->len);
}
void dynamic_setNBE_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, NBE, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setS_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, S);
    data->incrementEip(op->len);
}
void dynamic_setS_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, S, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setNS_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NS);
    data->incrementEip(op->len);
}
void dynamic_setNS_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, NS, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setP_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, P);
    data->incrementEip(op->len);
}
void dynamic_setP_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, P, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setNP_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NP);
    data->incrementEip(op->len);
}
void dynamic_setNP_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, NP, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setL_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, L);
    data->incrementEip(op->len);
}
void dynamic_setL_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, L, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setNL_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NL);
    data->incrementEip(op->len);
}
void dynamic_setNL_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, NL, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setLE_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, LE);
    data->incrementEip(op->len);
}
void dynamic_setLE_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, LE, true, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_setNLE_reg(DynamicData* data, DecodedOp* op) {
    data->setCPU(CPU::offsetofReg8(op->reg), DYN_8bit, NLE);
    data->incrementEip(op->len);
}
void dynamic_setNLE_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->setMem(DYN_ADDRESS, DYN_8bit, NLE, true, DYN_DEST);
    data->incrementEip(op->len);
}
