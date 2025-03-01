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

void dynamic_jumpO(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, O, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNO(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, O, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpB(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, B, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNB(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, B, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpZ(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, NZ, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNZ(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, NZ, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpBE(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, BE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNBE(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, BE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpS(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, S, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNS(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, S, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpP(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, P, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNP(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, NP, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpL(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, L, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNL(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, L, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpLE(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, LE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNLE(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, LE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
