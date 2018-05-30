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

void OPCALL jumpO(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (cpu->getOF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpNO(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (!cpu->getOF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpB(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (cpu->getCF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpNB(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (!cpu->getCF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpZ(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (cpu->getZF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpNZ(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (!cpu->getZF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpBE(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (cpu->getZF() || cpu->getCF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpNBE(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (!cpu->getZF() && !cpu->getCF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpS(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (cpu->getSF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpNS(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (!cpu->getSF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpP(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (cpu->getPF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpNP(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (!cpu->getPF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpL(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (cpu->getSF()!=cpu->getOF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpNL(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (cpu->getSF()==cpu->getOF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpLE(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) cpu->eip.u32+=op->imm;
}
void OPCALL jumpNLE(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) cpu->eip.u32+=op->imm;
}
