#include "../common/common_arith.h"
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

void OPCALL normal_addr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_adde8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_addr8e8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = cpu->memory->readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_add8_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_add8_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_addr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_adde16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_addr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->memory->readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_add16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_add16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_addr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_adde32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_addr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->memory->readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_add32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_add32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_orr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_ore8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_orr8e8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = cpu->memory->readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_or8_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_or8_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_orr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_ore16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_orr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->memory->readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_or16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_or16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_orr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_ore32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_orr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->memory->readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_or32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_or32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_adcr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_adce8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_adcr8e8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = cpu->memory->readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_adc8_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_adc8_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_adcr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_adce16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_adcr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->memory->readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_adc16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_adc16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_adcr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_adce32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_adcr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->memory->readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_adc32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_adc32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_sbbr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_sbbe8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_sbbr8e8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = cpu->memory->readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_sbb8_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_sbb8_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_sbbr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_sbbe16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_sbbr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->memory->readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_sbb16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_sbb16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_sbbr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_sbbe32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_sbbr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->memory->readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_sbb32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_sbb32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_andr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_ande8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_andr8e8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = cpu->memory->readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_and8_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_and8_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_andr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_ande16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_andr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->memory->readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_and16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_and16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_andr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_ande32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_andr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->memory->readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_and32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_and32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_subr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_sube8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_subr8e8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = cpu->memory->readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_sub8_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_sub8_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_subr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_sube16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_subr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->memory->readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_sub16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_sub16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_subr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_sube32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_subr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->memory->readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_sub32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_sub32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_xorr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_xore8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_xorr8e8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = cpu->memory->readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_xor8_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_xor8_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_xorr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_xore16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_xorr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->memory->readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_xor16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_xor16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_xorr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_xore32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_xorr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->memory->readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_xor32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_xor32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_cmpr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    NEXT();
}
void OPCALL normal_cmpe8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    NEXT();
}
void OPCALL normal_cmpr8e8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = cpu->memory->readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    NEXT();
}
void OPCALL normal_cmp8_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    NEXT();
}
void OPCALL normal_cmp8_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    NEXT();
}
void OPCALL normal_cmpr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    NEXT();
}
void OPCALL normal_cmpe16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    NEXT();
}
void OPCALL normal_cmpr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->memory->readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    NEXT();
}
void OPCALL normal_cmp16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    NEXT();
}
void OPCALL normal_cmp16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    NEXT();
}
void OPCALL normal_cmpr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    NEXT();
}
void OPCALL normal_cmpe32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    NEXT();
}
void OPCALL normal_cmpr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->memory->readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    NEXT();
}
void OPCALL normal_cmp32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    NEXT();
}
void OPCALL normal_cmp32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    NEXT();
}
void OPCALL normal_testr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
    NEXT();
}
void OPCALL normal_teste8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
    NEXT();
}
void OPCALL normal_test8_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
    NEXT();
}
void OPCALL normal_test8_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = cpu->memory->readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
    NEXT();
}
void OPCALL normal_testr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
    NEXT();
}
void OPCALL normal_teste16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
    NEXT();
}
void OPCALL normal_test16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
    NEXT();
}
void OPCALL normal_test16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = cpu->memory->readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
    NEXT();
}
void OPCALL normal_testr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
    NEXT();
}
void OPCALL normal_teste32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
    NEXT();
}
void OPCALL normal_test32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
    NEXT();
}
void OPCALL normal_test32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = cpu->memory->readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
    NEXT();
}
void OPCALL normal_notr8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    *cpu->reg8[op->reg] = ~*cpu->reg8[op->reg];
    NEXT();
}
void OPCALL normal_note8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->memory->writeb(eaa, ~cpu->memory->readb(eaa));
    NEXT();
}
void OPCALL normal_notr16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = ~cpu->reg[op->reg].u16;
    NEXT();
}
void OPCALL normal_note16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->memory->writew(eaa, ~cpu->memory->readw(eaa));
    NEXT();
}
void OPCALL normal_notr32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = ~cpu->reg[op->reg].u32;
    NEXT();
}
void OPCALL normal_note32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->memory->writed(eaa, ~cpu->memory->readd(eaa));
    NEXT();
}
void OPCALL normal_negr8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u8 = 0;
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_NEG8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
    NEXT();
}
void OPCALL normal_nege8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = 0;
    cpu->src.u8 = cpu->memory->readb(eaa);
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_NEG8;
    cpu->memory->writeb(eaa,  cpu->result.u8);
    NEXT();
}
void OPCALL normal_negr16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u16 = 0;
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_NEG16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
    NEXT();
}
void OPCALL normal_nege16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = 0;
    cpu->src.u16 = cpu->memory->readw(eaa);
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_NEG16;
    cpu->memory->writew(eaa,  cpu->result.u16);
    NEXT();
}
void OPCALL normal_negr32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->dst.u32 = 0;
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_NEG32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_nege32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = 0;
    cpu->src.u32 = cpu->memory->readd(eaa);
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_NEG32;
    cpu->memory->writed(eaa,  cpu->result.u32);
    NEXT();
}
void OPCALL normal_mulR8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_mul8(cpu, (*cpu->reg8[op->reg]));
    NEXT();
}
void OPCALL normal_mulE8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_mul8(cpu, cpu->memory->readb(eaa(cpu, op)));
    NEXT();
}
void OPCALL normal_imulR8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_imul8(cpu, (*cpu->reg8[op->reg]));
    NEXT();
}
void OPCALL normal_imulE8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_imul8(cpu, cpu->memory->readb(eaa(cpu, op)));
    NEXT();
}
void OPCALL normal_mulR16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_mul16(cpu, cpu->reg[op->reg].u16);
    NEXT();
}
void OPCALL normal_mulE16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_mul16(cpu, cpu->memory->readw(eaa(cpu, op)));
    NEXT();
}
void OPCALL normal_imulR16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_imul16(cpu, cpu->reg[op->reg].u16);
    NEXT();
}
void OPCALL normal_imulE16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_imul16(cpu, cpu->memory->readw(eaa(cpu, op)));
    NEXT();
}
void OPCALL normal_mulR32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_mul32(cpu, cpu->reg[op->reg].u32);
    NEXT();
}
void OPCALL normal_mulE32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_mul32(cpu, cpu->memory->readd(eaa(cpu, op)));
    NEXT();
}
void OPCALL normal_imulR32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_imul32(cpu, cpu->reg[op->reg].u32);
    NEXT();
}
void OPCALL normal_imulE32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_imul32(cpu, cpu->memory->readd(eaa(cpu, op)));
    NEXT();
}
void OPCALL normal_divR8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (div8(cpu, *cpu->reg8[op->reg])) {NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_divE8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (div8(cpu, cpu->memory->readb(eaa(cpu, op)))) {NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_idivR8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (idiv8(cpu, *cpu->reg8[op->reg])) {NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_idivE8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (idiv8(cpu, cpu->memory->readb(eaa(cpu, op)))) {NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_divR16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (div16(cpu, cpu->reg[op->reg].u16)) {NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_divE16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (div16(cpu, cpu->memory->readw(eaa(cpu, op)))) {NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_idivR16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (idiv16(cpu, cpu->reg[op->reg].u16)) {NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_idivE16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (idiv16(cpu, cpu->memory->readw(eaa(cpu, op)))) {NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_divR32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (div32(cpu, cpu->reg[op->reg].u32)) {NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_divE32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (div32(cpu, cpu->memory->readd(eaa(cpu, op)))) {NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_idivR32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (idiv32(cpu, cpu->reg[op->reg].u32)) {NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_idivE32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (idiv32(cpu, cpu->memory->readd(eaa(cpu, op)))) {NEXT();} else {NEXT_DONE();}
}
void OPCALL normal_dimulcr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_dimul16(cpu, cpu->reg[op->rm].u16, op->imm, op->reg);
    NEXT();
}
void OPCALL normal_dimulcr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_dimul16(cpu, cpu->memory->readw(eaa(cpu, op)), op->imm, op->reg);
    NEXT();
}
void OPCALL normal_dimulcr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_dimul32(cpu, cpu->reg[op->rm].u32, op->imm, op->reg);
    NEXT();
}
void OPCALL normal_dimulcr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_dimul32(cpu, cpu->memory->readd(eaa(cpu, op)), op->imm, op->reg);
    NEXT();
}
void OPCALL normal_dimulr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_dimul16(cpu, cpu->reg[op->rm].u16, cpu->reg[op->reg].u16, op->reg);
    NEXT();
}
void OPCALL normal_dimulr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_dimul16(cpu, cpu->memory->readw(eaa(cpu, op)), cpu->reg[op->reg].u16, op->reg);
    NEXT();
}
void OPCALL normal_dimulr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_dimul32(cpu, cpu->reg[op->rm].u32, cpu->reg[op->reg].u32, op->reg);
    NEXT();
}
void OPCALL normal_dimulr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_dimul32(cpu, cpu->memory->readd(eaa(cpu, op)), cpu->reg[op->reg].u32, op->reg);
    NEXT();
}
