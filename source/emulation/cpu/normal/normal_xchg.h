#include "../common/common_xchg.h"
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

void OPCALL normal_xchgr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U8 tmp = *cpu->reg8[op->rm];
    *cpu->reg8[op->rm] = *cpu->reg8[op->reg];
    *cpu->reg8[op->reg] = tmp;
    NEXT();
}
void OPCALL normal_xchge8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 address = eaa(cpu, op);
    U8 tmp = cpu->memory->readb(address);
    cpu->memory->writeb(address, *cpu->reg8[op->reg]);
    *cpu->reg8[op->reg] = tmp;
    NEXT();
}
void OPCALL normal_xchgr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 tmp = cpu->reg[op->rm].u16;
    cpu->reg[op->rm].u16 = cpu->reg[op->reg].u16;
    cpu->reg[op->reg].u16 = tmp;
    NEXT();
}
void OPCALL normal_xchge16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 address = eaa(cpu, op);
    U16 tmp = cpu->memory->readw(address);
    cpu->memory->writew(address, cpu->reg[op->reg].u16);
    cpu->reg[op->reg].u16 = tmp;
    NEXT();
}
void OPCALL normal_xchgr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 tmp = cpu->reg[op->rm].u32;
    cpu->reg[op->rm].u32 = cpu->reg[op->reg].u32;
    cpu->reg[op->reg].u32 = tmp;
    NEXT();
}
void OPCALL normal_xchge32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 address = eaa(cpu, op);
    U32 tmp = cpu->memory->readd(address);
    cpu->memory->writed(address, cpu->reg[op->reg].u32);
    cpu->reg[op->reg].u32 = tmp;
    NEXT();
}
void OPCALL normal_cmpxchgr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_cmpxchgr8r8(cpu, op->reg, op->rm);
    NEXT();
}
void OPCALL normal_cmpxchge8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_cmpxchge8r8(cpu, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_cmpxchgr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_cmpxchgr16r16(cpu, op->reg, op->rm);
    NEXT();
}
void OPCALL normal_cmpxchge16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_cmpxchge16r16(cpu, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_cmpxchgr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_cmpxchgr32r32(cpu, op->reg, op->rm);
    NEXT();
}
void OPCALL normal_cmpxchge32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_cmpxchge32r32(cpu, eaa(cpu, op), op->reg);
    NEXT();
}
