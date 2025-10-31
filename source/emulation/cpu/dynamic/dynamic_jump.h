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

void DynamicData::dynamic_jumpIfRegSet(DecodedOp* op, DynReg reg, bool doneWithReg) {
    // currentEip > lastOpEip this will just if we don't jump there is a next instruction
    if (canJumpInBlock(op)) {
        incrementEip(op->len + op->imm);
        JumpIf(reg, true, currentEip + op->len + op->imm);
        incrementEip((U32)(-(S32)(op->imm)));
    } else {
        If(reg, doneWithReg);
        incrementEip(op->imm + op->len);
        blockNext1(op);
        StartElse();
        incrementEip(op->len);
        blockNext2(op);
        EndIf();
    }
}

void DynamicData::dynamic_jumpIfRegNotSet(DecodedOp* op, DynReg reg, bool doneWithReg) {
    if (canJumpInBlock(op)) {
        incrementEip(op->len + op->imm);
        JumpIfNot(reg, true, currentEip + op->len + op->imm);
        incrementEip((U32)(-(S32)(op->imm)));
    } else {
        IfNot(reg, doneWithReg);
        incrementEip(op->imm + op->len);
        blockNext1(op);
        StartElse();
        incrementEip(op->len);
        blockNext2(op);
        EndIf();
    }
}

void DynamicData::dynamic_jump(DecodedOp* op, DynConditional condition) {
    if (canJumpInBlock(op)) {
        incrementEip(op->len + op->imm);
        JumpIfCondition(condition, currentEip + op->len + op->imm);
        incrementEip((U32)(-(S32)(op->imm)));
    } else {
        IfCondition(condition);
            incrementEip(op->imm + op->len);
            blockNext1(op);
        StartElse();
            incrementEip(op->len);
            blockNext2(op);
        EndIf();
    }
}

void DynamicData::dynamic_jumpO(DecodedOp* op) {
    dynamic_jump(op, O);
}
void DynamicData::dynamic_jumpNO(DecodedOp* op) {
    dynamic_jump(op, NO);
}
void DynamicData::dynamic_jumpB(DecodedOp* op) {
    dynamic_jump(op, B);
}
void DynamicData::dynamic_jumpNB(DecodedOp* op) {
    dynamic_jump(op, NB);
}
void DynamicData::dynamic_jumpZ(DecodedOp* op) {
    dynamic_jump(op, Z);
}
void DynamicData::dynamic_jumpNZ(DecodedOp* op) {
    dynamic_jump(op, NZ);
}
void DynamicData::dynamic_jumpBE(DecodedOp* op) {
    dynamic_jump(op, BE);
}
void DynamicData::dynamic_jumpNBE(DecodedOp* op) {
    dynamic_jump(op, NBE);
}
void DynamicData::dynamic_jumpS(DecodedOp* op) {
    dynamic_jump(op, S);
}
void DynamicData::dynamic_jumpNS(DecodedOp* op) {
    dynamic_jump(op, NS);
}
void DynamicData::dynamic_jumpP(DecodedOp* op) {
    dynamic_jump(op, P);
}
void DynamicData::dynamic_jumpNP(DecodedOp* op) {
    dynamic_jump(op, NP);
}
void DynamicData::dynamic_jumpL(DecodedOp* op) {
    dynamic_jump(op, L);
}
void DynamicData::dynamic_jumpNL(DecodedOp* op) {
    dynamic_jump(op, NL);
}
void DynamicData::dynamic_jumpLE(DecodedOp* op) {
    dynamic_jump(op, LE);
}
void DynamicData::dynamic_jumpNLE(DecodedOp* op) {
    dynamic_jump(op, NLE);
}
