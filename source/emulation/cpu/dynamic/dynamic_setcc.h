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

void DynamicData::dynamic_set_R(DecodedOp* op, DynConditional condition) {
    {
        RegPtr reg = getReg8(op->reg);
        setReg(condition, reg);
    }
    incrementEip(op->len);
}

void DynamicData::dynamic_set_M(DecodedOp* op, DynConditional condition) {
    RegPtr reg = getTmpReg();
    setReg(condition, reg);
    write(DYN_8bit, calculateEaa2(op), reg);
    incrementEip(op->len);
}

void DynamicData::dynamic_setO_reg(DecodedOp* op) {
    dynamic_set_R(op, O);
}
void DynamicData::dynamic_setO_mem(DecodedOp* op) {
    dynamic_set_M(op, O);
}
void DynamicData::dynamic_setNO_reg(DecodedOp* op) {
    dynamic_set_R(op, NO);
}
void DynamicData::dynamic_setNO_mem(DecodedOp* op) {
    dynamic_set_M(op, NO);
}
void DynamicData::dynamic_setB_reg(DecodedOp* op) {
    dynamic_set_R(op, B);
}
void DynamicData::dynamic_setB_mem(DecodedOp* op) {
    dynamic_set_M(op, B);
}
void DynamicData::dynamic_setNB_reg(DecodedOp* op) {
    dynamic_set_R(op, NB);
}
void DynamicData::dynamic_setNB_mem(DecodedOp* op) {
    dynamic_set_M(op, NB);
}
void DynamicData::dynamic_setZ_reg(DecodedOp* op) {
    dynamic_set_R(op, Z);
}
void DynamicData::dynamic_setZ_mem(DecodedOp* op) {
    dynamic_set_M(op, Z);
}
void DynamicData::dynamic_setNZ_reg(DecodedOp* op) {
    dynamic_set_R(op, NZ);
}
void DynamicData::dynamic_setNZ_mem(DecodedOp* op) {
    dynamic_set_M(op, NZ);
}
void DynamicData::dynamic_setBE_reg(DecodedOp* op) {
    dynamic_set_R(op, BE);
}
void DynamicData::dynamic_setBE_mem(DecodedOp* op) {
    dynamic_set_M(op, BE);
}
void DynamicData::dynamic_setNBE_reg(DecodedOp* op) {
    dynamic_set_R(op, NBE);
}
void DynamicData::dynamic_setNBE_mem(DecodedOp* op) {
    dynamic_set_M(op, NBE);
}
void DynamicData::dynamic_setS_reg(DecodedOp* op) {
    dynamic_set_R(op, S);
}
void DynamicData::dynamic_setS_mem(DecodedOp* op) {
    dynamic_set_M(op, S);
}
void DynamicData::dynamic_setNS_reg(DecodedOp* op) {
    dynamic_set_R(op, NS);
}
void DynamicData::dynamic_setNS_mem(DecodedOp* op) {
    dynamic_set_M(op, NS);
}
void DynamicData::dynamic_setP_reg(DecodedOp* op) {
    dynamic_set_R(op, P);
}
void DynamicData::dynamic_setP_mem(DecodedOp* op) {
    dynamic_set_M(op, P);
}
void DynamicData::dynamic_setNP_reg(DecodedOp* op) {
    dynamic_set_R(op, NP);
}
void DynamicData::dynamic_setNP_mem(DecodedOp* op) {
    dynamic_set_M(op, NP);
}
void DynamicData::dynamic_setL_reg(DecodedOp* op) {
    dynamic_set_R(op, L);
}
void DynamicData::dynamic_setL_mem(DecodedOp* op) {
    dynamic_set_M(op, L);
}
void DynamicData::dynamic_setNL_reg(DecodedOp* op) {
    dynamic_set_R(op, NL);
}
void DynamicData::dynamic_setNL_mem(DecodedOp* op) {
    dynamic_set_M(op, NL);
}
void DynamicData::dynamic_setLE_reg(DecodedOp* op) {
    dynamic_set_R(op, LE);
}
void DynamicData::dynamic_setLE_mem(DecodedOp* op) {
    dynamic_set_M(op, LE);
}
void DynamicData::dynamic_setNLE_reg(DecodedOp* op) {
    dynamic_set_R(op, NLE);
}
void DynamicData::dynamic_setNLE_mem(DecodedOp* op) {
    dynamic_set_M(op, NLE);
}
