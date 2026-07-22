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

void Jit::dynamic_jump(DecodedOp* op, JitConditional condition) {
    if (canJumpInBlock(op)) {
        U32 target = currentEip + op->len + op->imm;
        if (target <= currentEip) {
            exitToRunLoopIfPendingSignal(currentEip);
        }
        JumpIfCondition(condition, target);
    } else {
        IfCondition(condition); {
            blockNext1(currentEip + op->len + op->imm, op);
        }StartElse(); {
            blockNext2(currentEip + op->len, op);
        } EndIf();
}
}

void Jit::dynamic_jumpO(DecodedOp* op) {
    dynamic_jump(op, JitConditional::O);
}
void Jit::dynamic_jumpNO(DecodedOp* op) {
    dynamic_jump(op, JitConditional::NO);
}
void Jit::dynamic_jumpB(DecodedOp* op) {
    dynamic_jump(op, JitConditional::B);
}
void Jit::dynamic_jumpNB(DecodedOp* op) {
    dynamic_jump(op, JitConditional::NB);
}
void Jit::dynamic_jumpZ(DecodedOp* op) {
    dynamic_jump(op, JitConditional::Z);
}
void Jit::dynamic_jumpNZ(DecodedOp* op) {
    dynamic_jump(op, JitConditional::NZ);
}
void Jit::dynamic_jumpBE(DecodedOp* op) {
    dynamic_jump(op, JitConditional::BE);
}
void Jit::dynamic_jumpNBE(DecodedOp* op) {
    dynamic_jump(op, JitConditional::NBE);
}
void Jit::dynamic_jumpS(DecodedOp* op) {
    dynamic_jump(op, JitConditional::S);
}
void Jit::dynamic_jumpNS(DecodedOp* op) {
    dynamic_jump(op, JitConditional::NS);
}
void Jit::dynamic_jumpP(DecodedOp* op) {
    dynamic_jump(op, JitConditional::P);
}
void Jit::dynamic_jumpNP(DecodedOp* op) {
    dynamic_jump(op, JitConditional::NP);
}
void Jit::dynamic_jumpL(DecodedOp* op) {
    dynamic_jump(op, JitConditional::L);
}
void Jit::dynamic_jumpNL(DecodedOp* op) {
    dynamic_jump(op, JitConditional::NL);
}
void Jit::dynamic_jumpLE(DecodedOp* op) {
    dynamic_jump(op, JitConditional::LE);
}
void Jit::dynamic_jumpNLE(DecodedOp* op) {
    dynamic_jump(op, JitConditional::NLE);
}
