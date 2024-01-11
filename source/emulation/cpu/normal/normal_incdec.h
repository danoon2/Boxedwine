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

void OPCALL normal_inc8_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u8=*cpu->reg8[op->reg];
    cpu->result.u8=cpu->dst.u8 + 1;
    cpu->lazyFlags = FLAGS_INC8;
    *cpu->reg8[op->reg] = cpu->result.u8;
    NEXT();
}
void OPCALL normal_inc8_mem32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u8= cpu->memory->readb(eaa);
    cpu->result.u8=cpu->dst.u8 + 1;
    cpu->lazyFlags = FLAGS_INC8;
    cpu->memory->writeb(eaa, cpu->result.u8);
    NEXT();
}
void OPCALL normal_inc16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u16=cpu->reg[op->reg].u16;
    cpu->result.u16=cpu->dst.u16 + 1;
    cpu->lazyFlags = FLAGS_INC16;
    cpu->reg[op->reg].u16 = cpu->result.u16;
    NEXT();
}
void OPCALL normal_inc16_mem32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u16= cpu->memory->readw(eaa);
    cpu->result.u16=cpu->dst.u16 + 1;
    cpu->lazyFlags = FLAGS_INC16;
    cpu->memory->writew(eaa, cpu->result.u16);
    NEXT();
}
void OPCALL normal_inc32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u32=cpu->reg[op->reg].u32;
    cpu->result.u32=cpu->dst.u32 + 1;
    cpu->lazyFlags = FLAGS_INC32;
    cpu->reg[op->reg].u32 = cpu->result.u32;
    NEXT();
}
void OPCALL normal_inc32_mem32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u32= cpu->memory->readd(eaa);
    cpu->result.u32=cpu->dst.u32 + 1;
    cpu->lazyFlags = FLAGS_INC32;
    cpu->memory->writed(eaa, cpu->result.u32);
    NEXT();
}
void OPCALL normal_dec8_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u8=*cpu->reg8[op->reg];
    cpu->result.u8=cpu->dst.u8 - 1;
    cpu->lazyFlags = FLAGS_DEC8;
    *cpu->reg8[op->reg] = cpu->result.u8;
    NEXT();
}
void OPCALL normal_dec8_mem32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u8= cpu->memory->readb(eaa);
    cpu->result.u8=cpu->dst.u8 - 1;
    cpu->lazyFlags = FLAGS_DEC8;
    cpu->memory->writeb(eaa, cpu->result.u8);
    NEXT();
}
void OPCALL normal_dec16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u16=cpu->reg[op->reg].u16;
    cpu->result.u16=cpu->dst.u16 - 1;
    cpu->lazyFlags = FLAGS_DEC16;
    cpu->reg[op->reg].u16 = cpu->result.u16;
    NEXT();
}
void OPCALL normal_dec16_mem32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u16= cpu->memory->readw(eaa);
    cpu->result.u16=cpu->dst.u16 - 1;
    cpu->lazyFlags = FLAGS_DEC16;
    cpu->memory->writew(eaa, cpu->result.u16);
    NEXT();
}
void OPCALL normal_dec32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u32=cpu->reg[op->reg].u32;
    cpu->result.u32=cpu->dst.u32 - 1;
    cpu->lazyFlags = FLAGS_DEC32;
    cpu->reg[op->reg].u32 = cpu->result.u32;
    NEXT();
}
void OPCALL normal_dec32_mem32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF=cpu->getCF();
    cpu->dst.u32= cpu->memory->readd(eaa);
    cpu->result.u32=cpu->dst.u32 - 1;
    cpu->lazyFlags = FLAGS_DEC32;
    cpu->memory->writed(eaa, cpu->result.u32);
    NEXT();
}
