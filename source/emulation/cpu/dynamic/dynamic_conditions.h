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

void DynamicData::dynamic_cmov_R(JitWidth width, DecodedOp* op, JitConditional condition) {
    IfCondition(condition); {
        RegPtr dest = getReg(op->reg);
        mov(width, dest, getReadOnlyReg(op->rm));
    } EndIf();
    incrementEip(op->len);
}

void DynamicData::dynamic_cmov_M(JitWidth width, DecodedOp* op, JitConditional condition) {
    IfCondition(condition); {
        RegPtr dest = getReg(op->reg);
        mov(width, dest, read(width, calculateEaaV2(op)));
    } EndIf();
    incrementEip(op->len);
}

void DynamicData::dynamic_cmovO_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::O);
}
void DynamicData::dynamic_cmovO_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::O);
}
void DynamicData::dynamic_cmovO_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::O);
}
void DynamicData::dynamic_cmovO_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::O);
}
void DynamicData::dynamic_cmovNO_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::NO);
}
void DynamicData::dynamic_cmovNO_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::NO);
}
void DynamicData::dynamic_cmovNO_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::NO);
}
void DynamicData::dynamic_cmovNO_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::NO);
}
void DynamicData::dynamic_cmovB_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::B);
}
void DynamicData::dynamic_cmovB_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::B);
}
void DynamicData::dynamic_cmovB_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::B);
}
void DynamicData::dynamic_cmovB_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::B);
}
void DynamicData::dynamic_cmovNB_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::NB);
}
void DynamicData::dynamic_cmovNB_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::NB);
}
void DynamicData::dynamic_cmovNB_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::NB);
}
void DynamicData::dynamic_cmovNB_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::NB);
}
void DynamicData::dynamic_cmovZ_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::Z);
}
void DynamicData::dynamic_cmovZ_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::Z);
}
void DynamicData::dynamic_cmovZ_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::Z);
}
void DynamicData::dynamic_cmovZ_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::Z);
}
void DynamicData::dynamic_cmovNZ_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::NZ);
}
void DynamicData::dynamic_cmovNZ_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::NZ);
}
void DynamicData::dynamic_cmovNZ_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::NZ);
}
void DynamicData::dynamic_cmovNZ_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::NZ);
}
void DynamicData::dynamic_cmovBE_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::BE);
}
void DynamicData::dynamic_cmovBE_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::BE);
}
void DynamicData::dynamic_cmovBE_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::BE);
}
void DynamicData::dynamic_cmovBE_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::BE);
}
void DynamicData::dynamic_cmovNBE_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::NBE);
}
void DynamicData::dynamic_cmovNBE_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::NBE);
}
void DynamicData::dynamic_cmovNBE_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::NBE);
}
void DynamicData::dynamic_cmovNBE_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::NBE);
}
void DynamicData::dynamic_cmovS_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::S);
}
void DynamicData::dynamic_cmovS_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::S);
}
void DynamicData::dynamic_cmovS_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::S);
}
void DynamicData::dynamic_cmovS_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::S);
}
void DynamicData::dynamic_cmovNS_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::NS);
}
void DynamicData::dynamic_cmovNS_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::NS);
}
void DynamicData::dynamic_cmovNS_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::NS);
}
void DynamicData::dynamic_cmovNS_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::NS);
}
void DynamicData::dynamic_cmovP_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::P);
}
void DynamicData::dynamic_cmovP_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::P);
}
void DynamicData::dynamic_cmovP_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::P);
}
void DynamicData::dynamic_cmovP_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::P);
}
void DynamicData::dynamic_cmovNP_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::NP);
}
void DynamicData::dynamic_cmovNP_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::NP);
}
void DynamicData::dynamic_cmovNP_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::NP);
}
void DynamicData::dynamic_cmovNP_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::NP);
}
void DynamicData::dynamic_cmovL_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::L);
}
void DynamicData::dynamic_cmovL_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::L);
}
void DynamicData::dynamic_cmovL_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::L);
}
void DynamicData::dynamic_cmovL_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::L);
}
void DynamicData::dynamic_cmovNL_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::NL);
}
void DynamicData::dynamic_cmovNL_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::NL);
}
void DynamicData::dynamic_cmovNL_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::NL);
}
void DynamicData::dynamic_cmovNL_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::NL);
}
void DynamicData::dynamic_cmovLE_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::LE);
}
void DynamicData::dynamic_cmovLE_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::LE);
}
void DynamicData::dynamic_cmovLE_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::LE);
}
void DynamicData::dynamic_cmovLE_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::LE);
}
void DynamicData::dynamic_cmovNLE_16_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b16, op, JitConditional::NLE);
}
void DynamicData::dynamic_cmovNLE_16_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b16, op, JitConditional::NLE);
}
void DynamicData::dynamic_cmovNLE_32_reg(DecodedOp* op) {
    dynamic_cmov_R(JitWidth::b32, op, JitConditional::NLE);
}
void DynamicData::dynamic_cmovNLE_32_mem(DecodedOp* op) {
    dynamic_cmov_M(JitWidth::b32, op, JitConditional::NLE);
}