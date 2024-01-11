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

void OPCALL normal_movr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    *cpu->reg8[op->reg] = *cpu->reg8[op->rm];
    NEXT();
}
void OPCALL normal_move8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->memory->writeb(eaa(cpu, op), *cpu->reg8[op->reg]);
    NEXT();
}
void OPCALL normal_movr8e8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    *cpu->reg8[op->reg] = cpu->memory->readb(eaa(cpu, op));
    NEXT();
}
void OPCALL normal_movr8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    *cpu->reg8[op->reg] = (U8)op->imm;;
    NEXT();
}
void OPCALL normal_move8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->memory->writeb(eaa(cpu, op), (U8)op->imm);
    NEXT();
}
void OPCALL normal_movr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    NEXT();
}
void OPCALL normal_move16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->memory->writew(eaa(cpu, op), cpu->reg[op->reg].u16);
    NEXT();
}
void OPCALL normal_movr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    NEXT();
}
void OPCALL normal_movr16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = (U16)op->imm;
    NEXT();
}
void OPCALL normal_move16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->memory->writew(eaa(cpu, op), (U16)op->imm);
    NEXT();
}
void OPCALL normal_movr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    NEXT();
}
void OPCALL normal_move32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->memory->writed(eaa(cpu, op), cpu->reg[op->reg].u32);
    NEXT();
}
void OPCALL normal_movr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    NEXT();
}
void OPCALL normal_movr32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = op->imm;
    NEXT();
}
void OPCALL normal_move32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->memory->writed(eaa(cpu, op), op->imm);
    NEXT();
}
void OPCALL normal_movr16s16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = cpu->seg[op->rm].value;;
    NEXT();
}
void OPCALL normal_movr32s16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = cpu->seg[op->rm].value;
    NEXT();
}
void OPCALL normal_move16s16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->memory->writew(eaa(cpu, op), cpu->seg[op->reg].value);
    NEXT();
}
void OPCALL normal_movs16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->setSegment(op->reg, cpu->memory->readw(eaa(cpu, op)))) {
        NEXT();
    } else {
        NEXT_DONE();
    };
}
void OPCALL normal_movs16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->setSegment(op->rm, cpu->reg[op->reg].u16)) {
        NEXT();
    } else {
        NEXT_DONE();
    };
}
void OPCALL normal_movAlOb(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    AL = cpu->memory->readb(cpu->seg[op->base].address+op->disp);
    NEXT();
}
void OPCALL normal_movAxOw(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    AX = cpu->memory->readw(cpu->seg[op->base].address+op->disp);
    NEXT();
}
void OPCALL normal_movEaxOd(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    EAX = cpu->memory->readd(cpu->seg[op->base].address+op->disp);
    NEXT();
}
void OPCALL normal_movObAl(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->memory->writeb(cpu->seg[op->base].address+op->disp, AL);
    NEXT();
}
void OPCALL normal_movOwAx(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->memory->writew(cpu->seg[op->base].address+op->disp, AX);
    NEXT();
}
void OPCALL normal_movOdEax(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->memory->writed(cpu->seg[op->base].address+op->disp, EAX);
    NEXT();
}
void OPCALL normal_movGwXzR8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = *cpu->reg8[op->rm];
    NEXT();
}
void OPCALL normal_movGwXzE8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = cpu->memory->readb(eaa(cpu, op));
    NEXT();
}
void OPCALL normal_movGwSxR8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = (S8)*cpu->reg8[op->rm];
    NEXT();
}
void OPCALL normal_movGwSxE8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = (S8)cpu->memory->readb(eaa(cpu, op));
    NEXT();
}
void OPCALL normal_movGdXzR8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = *cpu->reg8[op->rm];
    NEXT();
}
void OPCALL normal_movGdXzE8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = cpu->memory->readb(eaa(cpu, op));
    NEXT();
}
void OPCALL normal_movGdSxR8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = (S8)(*cpu->reg8[op->rm]);
    NEXT();
}
void OPCALL normal_movGdSxE8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = (S8)cpu->memory->readb(eaa(cpu, op));
    NEXT();
}
void OPCALL normal_movGdXzR16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = cpu->reg[op->rm].u16;
    NEXT();
}
void OPCALL normal_movGdXzE16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = cpu->memory->readw(eaa(cpu, op));
    NEXT();
}
void OPCALL normal_movGdSxR16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = (S16)cpu->reg[op->rm].u16;
    NEXT();
}
void OPCALL normal_movGdSxE16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = (S16)cpu->memory->readw(eaa(cpu, op));
    NEXT();
}
void OPCALL normal_leaR16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = eaa(cpu, op);
    NEXT();
}
void OPCALL normal_leaR32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = eaa(cpu, op);
    NEXT();
}
void OPCALL normal_movRdCRx(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->readCrx(op->rm, op->reg)) {
        NEXT();
    } else {
        NEXT_DONE();
    };
}
void OPCALL normal_movCRxRd(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->writeCrx(op->rm, cpu->reg[op->reg].u32)) {
        NEXT();
    } else {
        NEXT_DONE();
    };
}