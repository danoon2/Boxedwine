/*
 *  Copyright (C) 2016  The BoxedWine Team
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

void OPCALL normal_jumpO(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getOF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpNO(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getOF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpB(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getCF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpNB(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getCF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpZ(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpNZ(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpBE(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getCF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpNBE(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && !cpu->getCF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpS(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpNS(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getSF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpP(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getPF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpNP(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getPF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpL(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()!=cpu->getOF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpNL(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()==cpu->getOF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpLE(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
void OPCALL normal_jumpNLE(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {cpu->eip.u32+=op->imm; NEXT_BRANCH1();} else {NEXT_BRANCH2();}
}
