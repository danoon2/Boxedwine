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

void OPCALL pushEw_reg(CPU* cpu, DecodedOp* op){
    cpu->push16(cpu->reg[op->reg].u16);
}
void OPCALL popEw_reg(CPU* cpu, DecodedOp* op){
    cpu->reg[op->reg].u16 = cpu->pop16();
}
void OPCALL pushEw_mem(CPU* cpu, DecodedOp* op){
    cpu->push16(readw(eaa(cpu, op)));
}
void OPCALL popEw_mem(CPU* cpu, DecodedOp* op){
     writew(eaa(cpu, op), cpu->pop16());
}
void OPCALL pushEd_reg(CPU* cpu, DecodedOp* op){
    cpu->push32(cpu->reg[op->reg].u32);
}
void OPCALL popEd_reg(CPU* cpu, DecodedOp* op){
    cpu->reg[op->reg].u32 = cpu->pop32();
}
void OPCALL pushEd_mem(CPU* cpu, DecodedOp* op){
    cpu->push32(readd(eaa(cpu, op)));
}
void OPCALL popEd_mem(CPU* cpu, DecodedOp* op){
     writed(eaa(cpu, op), cpu->pop32());
}
void OPCALL pushSeg16(CPU* cpu, DecodedOp* op){
    cpu->push16(cpu->seg[op->reg].value);
}
void OPCALL popSeg16(CPU* cpu, DecodedOp* op){
    if (cpu->setSegment(op->reg, cpu->peek16(0))) {cpu->pop16(); cpu->eip.u32+=op->len;}
}
void OPCALL pushSeg32(CPU* cpu, DecodedOp* op){
    cpu->push32(cpu->seg[op->reg].value);
}
void OPCALL popSeg32(CPU* cpu, DecodedOp* op){
    if (cpu->setSegment(op->reg, cpu->peek32(0))) {cpu->pop32(); cpu->eip.u32+=op->len;}
}
void OPCALL pushA16(CPU* cpu, DecodedOp* op){
    U16 sp = SP;
    cpu->push16(AX);
    cpu->push16(CX);
    cpu->push16(DX);
    cpu->push16(BX);
    cpu->push16(sp);
    cpu->push16(BP);
    cpu->push16(SI);
    cpu->push16(DI);
}
void OPCALL pushA32(CPU* cpu, DecodedOp* op){
    U32 sp = ESP;
    cpu->push32(EAX);
    cpu->push32(ECX);
    cpu->push32(EDX);
    cpu->push32(EBX);
    cpu->push32(sp);
    cpu->push32(EBP);
    cpu->push32(ESI);
    cpu->push32(EDI);
}
void OPCALL popA16(CPU* cpu, DecodedOp* op){
    DI = cpu->pop16();
    SI = cpu->pop16();
    BP = cpu->pop16();
    cpu->pop16();
    BX = cpu->pop16();
    DX = cpu->pop16();
    CX = cpu->pop16();
    AX = cpu->pop16();
}
void OPCALL popA32(CPU* cpu, DecodedOp* op){
    EDI = cpu->pop32();
    ESI = cpu->pop32();
    EBP = cpu->pop32();
    cpu->pop32();
    EBX = cpu->pop32();
    EDX = cpu->pop32();
    ECX = cpu->pop32();
    EAX = cpu->pop32();
}
void OPCALL push16imm(CPU* cpu, DecodedOp* op){
    cpu->push16(op->imm);
}
void OPCALL push32imm(CPU* cpu, DecodedOp* op){
    cpu->push32(op->imm);
}
void OPCALL popf16(CPU* cpu, DecodedOp* op){
    cpu->lazyFlags = FLAGS_NONE;
    cpu->setFlags(cpu->pop16(), FMASK_ALL & 0xFFFF);
}
void OPCALL popf32(CPU* cpu, DecodedOp* op){
    cpu->lazyFlags = FLAGS_NONE;
    cpu->setFlags(cpu->pop32(), FMASK_ALL);
}
void OPCALL pushf16(CPU* cpu, DecodedOp* op){
    cpu->fillFlags();
    cpu->push16(cpu->flags|2);
}
void OPCALL pushf32(CPU* cpu, DecodedOp* op){
    cpu->fillFlags();
    cpu->push32((cpu->flags|2) & 0xFCFFFF);
}
