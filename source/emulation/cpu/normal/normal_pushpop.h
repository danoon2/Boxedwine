#include "../common/common_pushpop.h"
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

void OPCALL normal_pushEw_reg(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->push16(cpu->reg[op->reg].u16);
    NEXT();
}
void OPCALL normal_popEw_reg(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = cpu->pop16();
    NEXT();
}
void OPCALL normal_pushEw_mem(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->push16(cpu->memory->readw(eaa(cpu, op)));
    NEXT();
}
void OPCALL normal_popEw_mem(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->memory->writew(eaa(cpu, op), cpu->pop16());
    NEXT();
}
void OPCALL normal_pushEd_reg(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->push32(cpu->reg[op->reg].u32);
    NEXT();
}
void OPCALL normal_popEd_reg(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = cpu->pop32();
    NEXT();
}
void OPCALL normal_pushEd_mem(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->push32(cpu->memory->readd(eaa(cpu, op)));
    NEXT();
}
void OPCALL normal_popEd_mem(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->memory->writed(eaa(cpu, op), cpu->pop32());
    NEXT();
}
void OPCALL normal_pushSeg16(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->push16(cpu->seg[op->reg].value);
    NEXT();
}
void OPCALL normal_popSeg16(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    if (cpu->setSegment(op->reg, cpu->peek16(0))) {ESP = (ESP & cpu->stackNotMask) | ((ESP + 2 ) & cpu->stackMask); NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_pushSeg32(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->push32(cpu->seg[op->reg].value);
    NEXT();
}
void OPCALL normal_popSeg32(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    if (cpu->setSegment(op->reg, cpu->peek32(0))) {ESP = (ESP & cpu->stackNotMask) | ((ESP + 4 ) & cpu->stackMask); NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_pushA16(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    common_pushA16(cpu);
    NEXT();
}
void OPCALL normal_pushA32(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    common_pushA32(cpu);
    NEXT();
}
void OPCALL normal_popA16(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    common_popA16(cpu);
    NEXT();
}
void OPCALL normal_popA32(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    common_popA32(cpu);
    NEXT();
}
void OPCALL normal_push16imm(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->push16(op->imm);
    NEXT();
}
void OPCALL normal_push32imm(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->push32(op->imm);
    NEXT();
}
void OPCALL normal_popf16(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->lazyFlags = FLAGS_NONE;
    cpu->setFlags(cpu->pop16(), FMASK_ALL & 0xFFFF);
    NEXT();
}
void OPCALL normal_popf32(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->lazyFlags = FLAGS_NONE;
    cpu->setFlags(cpu->pop32(), FMASK_ALL);
    NEXT();
}
void OPCALL normal_pushf16(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->fillFlags();
    cpu->push16((cpu->flags|2));
    NEXT();
}
void OPCALL normal_pushf32(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    cpu->fillFlags();
    cpu->push32((cpu->flags|2) & 0xFCFFFF);
    NEXT();
}
