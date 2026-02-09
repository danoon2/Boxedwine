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

void Jit::dynamic_set_R(DecodedOp* op, JitConditional condition) {;
    RegPtr reg;
    if (condition == JitConditional::B) {
        reg = getCF();
    } else {
        reg = getTmpReg8();
        getCondition(condition, reg);
    }
    mov(JitWidth::b8, getReg8(op->reg), reg);
}

void Jit::dynamic_set_M(DecodedOp* op, JitConditional condition) {
    RegPtr reg = getTmpReg8();
    getCondition(condition, reg);
    write(JitWidth::b8, calculateEaa(op), reg);
}

void Jit::dynamic_setO_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::O);
}
void Jit::dynamic_setO_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::O);
}
void Jit::dynamic_setNO_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::NO);
}
void Jit::dynamic_setNO_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::NO);
}
void Jit::dynamic_setB_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::B);
}
void Jit::dynamic_setB_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::B);
}
void Jit::dynamic_setNB_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::NB);
}
void Jit::dynamic_setNB_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::NB);
}
void Jit::dynamic_setZ_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::Z);
}
void Jit::dynamic_setZ_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::Z);
}
void Jit::dynamic_setNZ_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::NZ);
}
void Jit::dynamic_setNZ_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::NZ);
}
void Jit::dynamic_setBE_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::BE);
}
void Jit::dynamic_setBE_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::BE);
}
void Jit::dynamic_setNBE_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::NBE);
}
void Jit::dynamic_setNBE_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::NBE);
}
void Jit::dynamic_setS_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::S);
}
void Jit::dynamic_setS_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::S);
}
void Jit::dynamic_setNS_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::NS);
}
void Jit::dynamic_setNS_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::NS);
}
void Jit::dynamic_setP_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::P);
}
void Jit::dynamic_setP_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::P);
}
void Jit::dynamic_setNP_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::NP);
}
void Jit::dynamic_setNP_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::NP);
}
void Jit::dynamic_setL_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::L);
}
void Jit::dynamic_setL_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::L);
}
void Jit::dynamic_setNL_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::NL);
}
void Jit::dynamic_setNL_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::NL);
}
void Jit::dynamic_setLE_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::LE);
}
void Jit::dynamic_setLE_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::LE);
}
void Jit::dynamic_setNLE_reg(DecodedOp* op) {
    dynamic_set_R(op, JitConditional::NLE);
}
void Jit::dynamic_setNLE_mem(DecodedOp* op) {
    dynamic_set_M(op, JitConditional::NLE);
}
