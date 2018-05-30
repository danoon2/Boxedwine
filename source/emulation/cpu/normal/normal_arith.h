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

void OPCALL addr8r8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL adde8r8(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL addr8e8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL add8_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL add8_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL addr16r16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL adde16r16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL addr16e16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL add16_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL add16_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL addr32r32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL adde32r32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL addr32e32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL add32_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL add32_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL orr8r8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL ore8r8(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL orr8e8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL or8_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL or8_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 | cpu->src.u8;
    cpu->lazyFlags = FLAGS_OR8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL orr16r16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL ore16r16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL orr16e16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL or16_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL or16_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 | cpu->src.u16;
    cpu->lazyFlags = FLAGS_OR16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL orr32r32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL ore32r32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL orr32e32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL or32_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL or32_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 | cpu->src.u32;
    cpu->lazyFlags = FLAGS_OR32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL adcr8r8(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL adce8r8(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL adcr8e8(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL adc8_reg(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL adc8_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL adcr16r16(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL adce16r16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL adcr16e16(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL adc16_reg(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL adc16_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL adcr32r32(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL adce32r32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL adcr32e32(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL adc32_reg(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL adc32_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32 + cpu->oldCF;
    cpu->lazyFlags = FLAGS_ADC32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL sbbr8r8(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL sbbe8r8(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL sbbr8e8(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL sbb8_reg(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL sbb8_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL sbbr16r16(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL sbbe16r16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL sbbr16e16(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL sbb16_reg(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL sbb16_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL sbbr32r32(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL sbbe32r32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL sbbr32e32(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL sbb32_reg(CPU* cpu, DecodedOp* op) {
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL sbb32_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->oldCF = cpu->getCF();
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32 - cpu->oldCF;
    cpu->lazyFlags = FLAGS_SBB32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL andr8r8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL ande8r8(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL andr8e8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL and8_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL and8_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_AND8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL andr16r16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL ande16r16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL andr16e16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL and16_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL and16_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_AND16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL andr32r32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL ande32r32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL andr32e32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL and32_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL and32_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_AND32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL subr8r8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL sube8r8(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL subr8e8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL sub8_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL sub8_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_SUB8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL subr16r16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL sube16r16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL subr16e16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL sub16_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL sub16_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_SUB16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL subr32r32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL sube32r32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL subr32e32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL sub32_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL sub32_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_SUB32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL xorr8r8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL xore8r8(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL xorr8e8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL xor8_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL xor8_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 ^ cpu->src.u8;
    cpu->lazyFlags = FLAGS_XOR8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL xorr16r16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL xore16r16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL xorr16e16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL xor16_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL xor16_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 ^ cpu->src.u16;
    cpu->lazyFlags = FLAGS_XOR16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL xorr32r32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL xore32r32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL xorr32e32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL xor32_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL xor32_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 ^ cpu->src.u32;
    cpu->lazyFlags = FLAGS_XOR32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL cmpr8r8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
}
void OPCALL cmpe8r8(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
}
void OPCALL cmpr8e8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
}
void OPCALL cmp8_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
}
void OPCALL cmp8_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
}
void OPCALL cmpr16r16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
}
void OPCALL cmpe16r16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
}
void OPCALL cmpr16e16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
}
void OPCALL cmp16_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
}
void OPCALL cmp16_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
}
void OPCALL cmpr32r32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
}
void OPCALL cmpe32r32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
}
void OPCALL cmpr32e32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
}
void OPCALL cmp32_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
}
void OPCALL cmp32_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
}
void OPCALL testr8r8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
}
void OPCALL teste8r8(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
}
void OPCALL testr8e8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = readb(eaa(cpu, op));
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
}
void OPCALL test8_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = *cpu->reg8[op->reg];
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
}
void OPCALL test8_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = readb(eaa);
    cpu->src.u8 = op->imm;
    cpu->result.u8 = cpu->dst.u8 & cpu->src.u8;
    cpu->lazyFlags = FLAGS_TEST8;
}
void OPCALL testr16r16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
}
void OPCALL teste16r16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
}
void OPCALL testr16e16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = readw(eaa(cpu, op));
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
}
void OPCALL test16_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = cpu->reg[op->reg].u16;
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
}
void OPCALL test16_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = readw(eaa);
    cpu->src.u16 = op->imm;
    cpu->result.u16 = cpu->dst.u16 & cpu->src.u16;
    cpu->lazyFlags = FLAGS_TEST16;
}
void OPCALL testr32r32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
}
void OPCALL teste32r32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
}
void OPCALL testr32e32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = readd(eaa(cpu, op));
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
}
void OPCALL test32_reg(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = cpu->reg[op->reg].u32;
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
}
void OPCALL test32_mem(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = readd(eaa);
    cpu->src.u32 = op->imm;
    cpu->result.u32 = cpu->dst.u32 & cpu->src.u32;
    cpu->lazyFlags = FLAGS_TEST32;
}
void OPCALL notr8(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = ~*cpu->reg8[op->reg];
}
void OPCALL note8(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(eaa, ~readb(eaa));
}
void OPCALL notr16(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = ~cpu->reg[op->reg].u16;
}
void OPCALL note16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(eaa, ~readw(eaa));
}
void OPCALL notr32(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = ~cpu->reg[op->reg].u32;
}
void OPCALL note32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(eaa, ~readd(eaa));
}
void OPCALL negr8(CPU* cpu, DecodedOp* op) {
    cpu->dst.u8 = 0;
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_NEG8;
    *cpu->reg8[op->reg] =  cpu->result.u8;
}
void OPCALL nege8(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u8 = 0;
    cpu->src.u8 = readb(eaa);
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_NEG8;
    writeb(eaa,  cpu->result.u8);
}
void OPCALL negr16(CPU* cpu, DecodedOp* op) {
    cpu->dst.u16 = 0;
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_NEG16;
    cpu->reg[op->reg].u16 =  cpu->result.u16;
}
void OPCALL nege16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u16 = 0;
    cpu->src.u16 = readw(eaa);
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_NEG16;
    writew(eaa,  cpu->result.u16);
}
void OPCALL negr32(CPU* cpu, DecodedOp* op) {
    cpu->dst.u32 = 0;
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_NEG32;
    cpu->reg[op->reg].u32 =  cpu->result.u32;
}
void OPCALL nege32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    cpu->dst.u32 = 0;
    cpu->src.u32 = readd(eaa);
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_NEG32;
    writed(eaa,  cpu->result.u32);
}
void OPCALL mulR8(CPU* cpu, DecodedOp* op) {
    AX = AL * (*cpu->reg8[op->reg]);
    if (AH) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void OPCALL mulE8(CPU* cpu, DecodedOp* op) {
    AX = AL * readb(eaa(cpu, op));
    if (AH) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void OPCALL imulR8(CPU* cpu, DecodedOp* op) {
    AX = (S16)((S8)AL) * (S8)((*cpu->reg8[op->reg]));
    if ((S16)AX<-128 || (S16)AX>127) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void OPCALL imulE8(CPU* cpu, DecodedOp* op) {
    AX = (S16)((S8)AL) * (S8)(readb(eaa(cpu, op)));
    if ((S16)AX<-128 || (S16)AX>127) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void OPCALL mulR16(CPU* cpu, DecodedOp* op) {
    U32 result = (U32)AX * cpu->reg[op->reg].u16;
    cpu->fillFlagsNoCFOF();
    AX = (U16)result;
    DX = (U16)(result >> 16);
    if (DX) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void OPCALL mulE16(CPU* cpu, DecodedOp* op) {
    U32 result = (U32)AX * readw(eaa(cpu, op));
    cpu->fillFlagsNoCFOF();
    AX = (U16)result;
    DX = (U16)(result >> 16);
    if (DX) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void OPCALL imulR16(CPU* cpu, DecodedOp* op) {
    S32 result = (S32)((S16)AX) * ((S16)(cpu->reg[op->reg].u16));
    cpu->fillFlagsNoCFOF();
    AX = (U16)result;
    DX = (U16)(result >> 16);
    if (result>32767 || result<-32768) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void OPCALL imulE16(CPU* cpu, DecodedOp* op) {
    S32 result = (S32)((S16)AX) * ((S16)(readw(eaa(cpu, op))));
    cpu->fillFlagsNoCFOF();
    AX = (U16)result;
    DX = (U16)(result >> 16);
    if (result>32767 || result<-32768) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void OPCALL mulR32(CPU* cpu, DecodedOp* op) {
    U64 result = (U64)EAX * cpu->reg[op->reg].u32;
    cpu->fillFlagsNoCFOF();
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
    if (EDX) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void OPCALL mulE32(CPU* cpu, DecodedOp* op) {
    U64 result = (U64)EAX * readd(eaa(cpu, op));
    cpu->fillFlagsNoCFOF();
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
    if (EDX) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void OPCALL imulR32(CPU* cpu, DecodedOp* op) {
    S64 result = (S64)((S32)EAX) * ((S32)(cpu->reg[op->reg].u32));
    cpu->fillFlagsNoCFOF();
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
    if (result>0x7fffffffl || result<-0x7fffffffl) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void OPCALL imulE32(CPU* cpu, DecodedOp* op) {
    S64 result = (S64)((S32)EAX) * ((S32)(readd(eaa(cpu, op))));
    cpu->fillFlagsNoCFOF();
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
    if (result>0x7fffffffl || result<-0x7fffffffl) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void OPCALL divR8(CPU* cpu, DecodedOp* op) {
    if (div8(cpu, *cpu->reg8[op->reg])) cpu->eip.u32+=op->len;
}
void OPCALL divE8(CPU* cpu, DecodedOp* op) {
    if (div8(cpu, readb(eaa(cpu, op)))) cpu->eip.u32+=op->len;
}
void OPCALL idivR8(CPU* cpu, DecodedOp* op) {
    if (idiv8(cpu, *cpu->reg8[op->reg])) cpu->eip.u32+=op->len;
}
void OPCALL idivE8(CPU* cpu, DecodedOp* op) {
    if (idiv8(cpu, readb(eaa(cpu, op)))) cpu->eip.u32+=op->len;
}
void OPCALL divR16(CPU* cpu, DecodedOp* op) {
    if (div16(cpu, cpu->reg[op->reg].u16)) cpu->eip.u32+=op->len;
}
void OPCALL divE16(CPU* cpu, DecodedOp* op) {
    if (div16(cpu, readw(eaa(cpu, op)))) cpu->eip.u32+=op->len;
}
void OPCALL idivR16(CPU* cpu, DecodedOp* op) {
    if (idiv16(cpu, cpu->reg[op->reg].u16)) cpu->eip.u32+=op->len;
}
void OPCALL idivE16(CPU* cpu, DecodedOp* op) {
    if (idiv16(cpu, readw(eaa(cpu, op)))) cpu->eip.u32+=op->len;
}
void OPCALL divR32(CPU* cpu, DecodedOp* op) {
    if (div32(cpu, cpu->reg[op->reg].u32)) cpu->eip.u32+=op->len;
}
void OPCALL divE32(CPU* cpu, DecodedOp* op) {
    if (div32(cpu, readd(eaa(cpu, op)))) cpu->eip.u32+=op->len;
}
void OPCALL idivR32(CPU* cpu, DecodedOp* op) {
    if (idiv32(cpu, cpu->reg[op->reg].u32)) cpu->eip.u32+=op->len;
}
void OPCALL idivE32(CPU* cpu, DecodedOp* op) {
    if (idiv32(cpu, readd(eaa(cpu, op)))) cpu->eip.u32+=op->len;
}
void OPCALL dimulcr16r16(CPU* cpu, DecodedOp* op) {
    S32 res=(S16)(cpu->reg[op->rm].u16) * (S32)((S16)op->imm);
    cpu->fillFlagsNoCFOF();
    if ((res >= -32767) && (res <= 32767)) {
        cpu->removeFlag(CF|OF);
    } else {
        cpu->addFlag(CF|OF);
    }
    cpu->reg[op->reg].u16 = (U16)res;
}
void OPCALL dimulcr16e16(CPU* cpu, DecodedOp* op) {
    S32 res=(S16)(readw(eaa(cpu, op))) * (S32)((S16)op->imm);
    cpu->fillFlagsNoCFOF();
    if ((res >= -32767) && (res <= 32767)) {
        cpu->removeFlag(CF|OF);
    } else {
        cpu->addFlag(CF|OF);
    }
    cpu->reg[op->reg].u16 = (U16)res;
}
void OPCALL dimulcr32r32(CPU* cpu, DecodedOp* op) {
    S64 res=(S32)(cpu->reg[op->rm].u32) * (S64)((S32)op->imm);
    cpu->fillFlagsNoCFOF();
    if (res>=-2147483647l && res<=2147483647l) {
        cpu->removeFlag(CF|OF);
    } else {
        cpu->addFlag(CF|OF);
    }
    cpu->reg[op->reg].u32 = (U32)res;
}
void OPCALL dimulcr32e32(CPU* cpu, DecodedOp* op) {
    S64 res=(S32)(readd(eaa(cpu, op))) * (S64)((S32)op->imm);
    cpu->fillFlagsNoCFOF();
    if (res>=-2147483647l && res<=2147483647l) {
        cpu->removeFlag(CF|OF);
    } else {
        cpu->addFlag(CF|OF);
    }
    cpu->reg[op->reg].u32 = (U32)res;
}
void OPCALL dimulr16r16(CPU* cpu, DecodedOp* op) {
    S32 res=(S16)(cpu->reg[op->rm].u16) * (S32)((S16)cpu->reg[op->reg].u16);
    cpu->fillFlagsNoCFOF();
    if ((res >= -32767) && (res <= 32767)) {
        cpu->removeFlag(CF|OF);
    } else {
        cpu->addFlag(CF|OF);
    }
    cpu->reg[op->reg].u16 = (U16)res;
}
void OPCALL dimulr16e16(CPU* cpu, DecodedOp* op) {
    S32 res=(S16)(readw(eaa(cpu, op))) * (S32)((S16)cpu->reg[op->reg].u16);
    cpu->fillFlagsNoCFOF();
    if ((res >= -32767) && (res <= 32767)) {
        cpu->removeFlag(CF|OF);
    } else {
        cpu->addFlag(CF|OF);
    }
    cpu->reg[op->reg].u16 = (U16)res;
}
void OPCALL dimulr32r32(CPU* cpu, DecodedOp* op) {
    S64 res=(S32)(cpu->reg[op->rm].u32) * (S64)((S32)cpu->reg[op->reg].u32);
    cpu->fillFlagsNoCFOF();
    if (res>=-2147483647l && res<=2147483647l) {
        cpu->removeFlag(CF|OF);
    } else {
        cpu->addFlag(CF|OF);
    }
    cpu->reg[op->reg].u32 = (U32)res;
}
void OPCALL dimulr32e32(CPU* cpu, DecodedOp* op) {
    S64 res=(S32)(readd(eaa(cpu, op))) * (S64)((S32)cpu->reg[op->reg].u32);
    cpu->fillFlagsNoCFOF();
    if (res>=-2147483647l && res<=2147483647l) {
        cpu->removeFlag(CF|OF);
    } else {
        cpu->addFlag(CF|OF);
    }
    cpu->reg[op->reg].u32 = (U32)res;
}
