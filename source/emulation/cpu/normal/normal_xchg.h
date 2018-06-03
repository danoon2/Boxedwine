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

void OPCALL xchgr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U8 tmp = *cpu->reg8[op->rm];
    *cpu->reg8[op->rm] = *cpu->reg8[op->reg];
    *cpu->reg8[op->reg] = tmp;
    NEXT();
}
void OPCALL xchge8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U8 tmp = readb(eaa(cpu, op));
    writeb(eaa(cpu, op), *cpu->reg8[op->reg]);
    *cpu->reg8[op->reg] = tmp;
    NEXT();
}
void OPCALL xchgr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 tmp = cpu->reg[op->rm].u16;
    cpu->reg[op->rm].u16 = cpu->reg[op->reg].u16;
    cpu->reg[op->reg].u16 = tmp;
    NEXT();
}
void OPCALL xchge16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 tmp = readw(eaa(cpu, op));
    writew(eaa(cpu, op), cpu->reg[op->reg].u16);
    cpu->reg[op->reg].u16 = tmp;
    NEXT();
}
void OPCALL xchgr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 tmp = cpu->reg[op->rm].u32;
    cpu->reg[op->rm].u32 = cpu->reg[op->reg].u32;
    cpu->reg[op->reg].u32 = tmp;
    NEXT();
}
void OPCALL xchge32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 tmp = readd(eaa(cpu, op));
    writed(eaa(cpu, op), cpu->reg[op->reg].u32);
    cpu->reg[op->reg].u32 = tmp;
    NEXT();
}
void OPCALL cmpxchgr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = AX;
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    if (AX == cpu->src.u16) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    } else {
        AX = cpu->src.u16;
    }
    NEXT();
}
void OPCALL cmpxchge16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 address = eaa(cpu, op);
    cpu->dst.u16 = AX;
    cpu->src.u16 = readw(address);
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    if (AX == cpu->src.u16) {
        writew(address, cpu->reg[op->reg].u16);
    } else {
        AX = cpu->src.u16;
    }
    NEXT();
}
void OPCALL cmpxchgr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = EAX;
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    if (EAX == cpu->src.u32) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    } else {
        EAX = cpu->src.u32;
    }
    NEXT();
}
void OPCALL cmpxchge32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 address = eaa(cpu, op);
    cpu->dst.u32 = EAX;
    cpu->src.u32 = readd(address);
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    if (EAX == cpu->src.u32) {
        writed(address, cpu->reg[op->reg].u32);
    } else {
        EAX = cpu->src.u32;
    }
    NEXT();
}
