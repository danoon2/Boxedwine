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

void OPCALL addr8r8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] + *cpu->reg8[op->rm];
}
void OPCALL adde8r8_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) + *cpu->reg8[op->reg]);
}
void OPCALL addr8e8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] + readb(cpu->thread, eaa(cpu, op));
}
void OPCALL add8_reg_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] + op->imm;
}
void OPCALL add8_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) + op->imm);
}
void OPCALL addr16r16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 + cpu->reg[op->rm].u16;
}
void OPCALL adde16r16_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) + cpu->reg[op->reg].u16);
}
void OPCALL addr16e16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 + readw(cpu->thread, eaa(cpu, op));
}
void OPCALL add16_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 + op->imm;
}
void OPCALL add16_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) + op->imm);
}
void OPCALL addr32r32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 + cpu->reg[op->rm].u32;
}
void OPCALL adde32r32_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) + cpu->reg[op->reg].u32);
}
void OPCALL addr32e32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 + readd(cpu->thread, eaa(cpu, op));
}
void OPCALL add32_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 + op->imm;
}
void OPCALL add32_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) + op->imm);
}
void OPCALL orr8r8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] | *cpu->reg8[op->rm];
}
void OPCALL ore8r8_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) | *cpu->reg8[op->reg]);
}
void OPCALL orr8e8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] | readb(cpu->thread, eaa(cpu, op));
}
void OPCALL or8_reg_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] | op->imm;
}
void OPCALL or8_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) | op->imm);
}
void OPCALL orr16r16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 | cpu->reg[op->rm].u16;
}
void OPCALL ore16r16_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) | cpu->reg[op->reg].u16);
}
void OPCALL orr16e16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 | readw(cpu->thread, eaa(cpu, op));
}
void OPCALL or16_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 | op->imm;
}
void OPCALL or16_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) | op->imm);
}
void OPCALL orr32r32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 | cpu->reg[op->rm].u32;
}
void OPCALL ore32r32_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) | cpu->reg[op->reg].u32);
}
void OPCALL orr32e32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 | readd(cpu->thread, eaa(cpu, op));
}
void OPCALL or32_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 | op->imm;
}
void OPCALL or32_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) | op->imm);
}
void OPCALL adcr8r8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] + *cpu->reg8[op->rm] + cpu->lazyFlags->getCF(cpu);
}
void OPCALL adce8r8_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) + *cpu->reg8[op->reg] + cpu->lazyFlags->getCF(cpu));
}
void OPCALL adcr8e8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] + readb(cpu->thread, eaa(cpu, op)) + cpu->lazyFlags->getCF(cpu);
}
void OPCALL adc8_reg_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] + op->imm + cpu->lazyFlags->getCF(cpu);
}
void OPCALL adc8_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) + op->imm + cpu->lazyFlags->getCF(cpu));
}
void OPCALL adcr16r16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 + cpu->reg[op->rm].u16 + cpu->lazyFlags->getCF(cpu);
}
void OPCALL adce16r16_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) + cpu->reg[op->reg].u16 + cpu->lazyFlags->getCF(cpu));
}
void OPCALL adcr16e16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 + readw(cpu->thread, eaa(cpu, op)) + cpu->lazyFlags->getCF(cpu);
}
void OPCALL adc16_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 + op->imm + cpu->lazyFlags->getCF(cpu);
}
void OPCALL adc16_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) + op->imm + cpu->lazyFlags->getCF(cpu));
}
void OPCALL adcr32r32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 + cpu->reg[op->rm].u32 + cpu->lazyFlags->getCF(cpu);
}
void OPCALL adce32r32_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) + cpu->reg[op->reg].u32 + cpu->lazyFlags->getCF(cpu));
}
void OPCALL adcr32e32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 + readd(cpu->thread, eaa(cpu, op)) + cpu->lazyFlags->getCF(cpu);
}
void OPCALL adc32_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 + op->imm + cpu->lazyFlags->getCF(cpu);
}
void OPCALL adc32_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) + op->imm + cpu->lazyFlags->getCF(cpu));
}
void OPCALL sbbr8r8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] - *cpu->reg8[op->rm] - cpu->lazyFlags->getCF(cpu);
}
void OPCALL sbbe8r8_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) - *cpu->reg8[op->reg] - cpu->lazyFlags->getCF(cpu));
}
void OPCALL sbbr8e8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] - readb(cpu->thread, eaa(cpu, op)) - cpu->lazyFlags->getCF(cpu);
}
void OPCALL sbb8_reg_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] - op->imm - cpu->lazyFlags->getCF(cpu);
}
void OPCALL sbb8_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) - op->imm - cpu->lazyFlags->getCF(cpu));
}
void OPCALL sbbr16r16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 - cpu->reg[op->rm].u16 - cpu->lazyFlags->getCF(cpu);
}
void OPCALL sbbe16r16_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) - cpu->reg[op->reg].u16 - cpu->lazyFlags->getCF(cpu));
}
void OPCALL sbbr16e16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 - readw(cpu->thread, eaa(cpu, op)) - cpu->lazyFlags->getCF(cpu);
}
void OPCALL sbb16_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 - op->imm - cpu->lazyFlags->getCF(cpu);
}
void OPCALL sbb16_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) - op->imm - cpu->lazyFlags->getCF(cpu));
}
void OPCALL sbbr32r32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 - cpu->reg[op->rm].u32 - cpu->lazyFlags->getCF(cpu);
}
void OPCALL sbbe32r32_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) - cpu->reg[op->reg].u32 - cpu->lazyFlags->getCF(cpu));
}
void OPCALL sbbr32e32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 - readd(cpu->thread, eaa(cpu, op)) - cpu->lazyFlags->getCF(cpu);
}
void OPCALL sbb32_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 - op->imm - cpu->lazyFlags->getCF(cpu);
}
void OPCALL sbb32_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) - op->imm - cpu->lazyFlags->getCF(cpu));
}
void OPCALL andr8r8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] & *cpu->reg8[op->rm];
}
void OPCALL ande8r8_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) & *cpu->reg8[op->reg]);
}
void OPCALL andr8e8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] & readb(cpu->thread, eaa(cpu, op));
}
void OPCALL and8_reg_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] & op->imm;
}
void OPCALL and8_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) & op->imm);
}
void OPCALL andr16r16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 & cpu->reg[op->rm].u16;
}
void OPCALL ande16r16_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) & cpu->reg[op->reg].u16);
}
void OPCALL andr16e16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 & readw(cpu->thread, eaa(cpu, op));
}
void OPCALL and16_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 & op->imm;
}
void OPCALL and16_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) & op->imm);
}
void OPCALL andr32r32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 & cpu->reg[op->rm].u32;
}
void OPCALL ande32r32_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) & cpu->reg[op->reg].u32);
}
void OPCALL andr32e32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 & readd(cpu->thread, eaa(cpu, op));
}
void OPCALL and32_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 & op->imm;
}
void OPCALL and32_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) & op->imm);
}
void OPCALL subr8r8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] - *cpu->reg8[op->rm];
}
void OPCALL sube8r8_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) - *cpu->reg8[op->reg]);
}
void OPCALL subr8e8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] - readb(cpu->thread, eaa(cpu, op));
}
void OPCALL sub8_reg_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] - op->imm;
}
void OPCALL sub8_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) - op->imm);
}
void OPCALL subr16r16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 - cpu->reg[op->rm].u16;
}
void OPCALL sube16r16_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) - cpu->reg[op->reg].u16);
}
void OPCALL subr16e16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 - readw(cpu->thread, eaa(cpu, op));
}
void OPCALL sub16_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 - op->imm;
}
void OPCALL sub16_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) - op->imm);
}
void OPCALL subr32r32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 - cpu->reg[op->rm].u32;
}
void OPCALL sube32r32_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) - cpu->reg[op->reg].u32);
}
void OPCALL subr32e32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 - readd(cpu->thread, eaa(cpu, op));
}
void OPCALL sub32_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 - op->imm;
}
void OPCALL sub32_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) - op->imm);
}
void OPCALL xorr8r8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] ^ *cpu->reg8[op->rm];
}
void OPCALL xore8r8_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) ^ *cpu->reg8[op->reg]);
}
void OPCALL xorr8e8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] ^ readb(cpu->thread, eaa(cpu, op));
}
void OPCALL xor8_reg_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] ^ op->imm;
}
void OPCALL xor8_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) ^ op->imm);
}
void OPCALL xorr16r16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 ^ cpu->reg[op->rm].u16;
}
void OPCALL xore16r16_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) ^ cpu->reg[op->reg].u16);
}
void OPCALL xorr16e16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 ^ readw(cpu->thread, eaa(cpu, op));
}
void OPCALL xor16_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 ^ op->imm;
}
void OPCALL xor16_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) ^ op->imm);
}
void OPCALL xorr32r32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 ^ cpu->reg[op->rm].u32;
}
void OPCALL xore32r32_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) ^ cpu->reg[op->reg].u32);
}
void OPCALL xorr32e32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 ^ readd(cpu->thread, eaa(cpu, op));
}
void OPCALL xor32_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 ^ op->imm;
}
void OPCALL xor32_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) ^ op->imm);
}
void OPCALL cmpr8r8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] - *cpu->reg8[op->rm];
}
void OPCALL cmpe8r8_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) - *cpu->reg8[op->reg]);
}
void OPCALL cmpr8e8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] - readb(cpu->thread, eaa(cpu, op));
}
void OPCALL cmp8_reg_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] - op->imm;
}
void OPCALL cmp8_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) - op->imm);
}
void OPCALL cmpr16r16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 - cpu->reg[op->rm].u16;
}
void OPCALL cmpe16r16_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) - cpu->reg[op->reg].u16);
}
void OPCALL cmpr16e16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 - readw(cpu->thread, eaa(cpu, op));
}
void OPCALL cmp16_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 - op->imm;
}
void OPCALL cmp16_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) - op->imm);
}
void OPCALL cmpr32r32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 - cpu->reg[op->rm].u32;
}
void OPCALL cmpe32r32_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) - cpu->reg[op->reg].u32);
}
void OPCALL cmpr32e32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 - readd(cpu->thread, eaa(cpu, op));
}
void OPCALL cmp32_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 - op->imm;
}
void OPCALL cmp32_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) - op->imm);
}
void OPCALL testr8r8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] & *cpu->reg8[op->rm];
}
void OPCALL teste8r8_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) & *cpu->reg8[op->reg]);
}
void OPCALL testr8e8_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] & readb(cpu->thread, eaa(cpu, op));
}
void OPCALL test8_reg_noflags(CPU* cpu, DecodedOp* op) {
    *cpu->reg8[op->reg] = *cpu->reg8[op->reg] & op->imm;
}
void OPCALL test8_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa) & op->imm);
}
void OPCALL testr16r16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 & cpu->reg[op->rm].u16;
}
void OPCALL teste16r16_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) & cpu->reg[op->reg].u16);
}
void OPCALL testr16e16_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 & readw(cpu->thread, eaa(cpu, op));
}
void OPCALL test16_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->reg[op->reg].u16 & op->imm;
}
void OPCALL test16_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa) & op->imm);
}
void OPCALL testr32r32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 & cpu->reg[op->rm].u32;
}
void OPCALL teste32r32_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) & cpu->reg[op->reg].u32);
}
void OPCALL testr32e32_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 & readd(cpu->thread, eaa(cpu, op));
}
void OPCALL test32_reg_noflags(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u32 = cpu->reg[op->reg].u32 & op->imm;
}
void OPCALL test32_mem_noflags(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa) & op->imm);
}
void OPCALL addr8r8(struct CPU* cpu, struct Op* op);
void OPCALL adde8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL adde8r8_16(struct CPU* cpu, struct Op* op);
// ADD Eb,Gb
void decode000_noflags(struct Op* op) {
    if (op->func == addr8r8) {
        op->func = addr8r8_noflags;
    } else if (op->func == adde8r8_32) {
        op->func = adde8r8_32_noflags;
    } else if (op->func == adde8r8_16) {
        op->func = adde8r8_16_noflags;
    } else {
        kpanic("decode000_noflags error");
    }
}
void OPCALL addr16r16(struct CPU* cpu, struct Op* op);
void OPCALL adde16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL adde16r16_16(struct CPU* cpu, struct Op* op);
// ADD Ew,Gw
void decode001_noflags(struct Op* op) {
    if (op->func == addr16r16) {
        op->func = addr16r16_noflags;
    } else if (op->func == adde16r16_32) {
        op->func = adde16r16_32_noflags;
    } else if (op->func == adde16r16_16) {
        op->func = adde16r16_16_noflags;
    } else {
        kpanic("decode001_noflags error");
    }
}
void OPCALL addr32r32(struct CPU* cpu, struct Op* op);
void OPCALL adde32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL adde32r32_16(struct CPU* cpu, struct Op* op);
// ADD Ed,Gd
void decode201_noflags(struct Op* op) {
    if (op->func == addr32r32) {
        op->func = addr32r32_noflags;
    } else if (op->func == adde32r32_32) {
        op->func = adde32r32_32_noflags;
    } else if (op->func == adde32r32_16) {
        op->func = adde32r32_16_noflags;
    } else {
        kpanic("decode201_noflags error");
    }
}
void OPCALL addr8r8(struct CPU* cpu, struct Op* op);
void OPCALL addr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL addr8e8_16(struct CPU* cpu, struct Op* op);
// ADD Gb,Eb
void decode002_noflags(struct Op* op) {
    if (op->func == addr8r8) {
        op->func = addr8r8_noflags;
    } else if (op->func == addr8e8_32) {
        op->func = addr8e8_32_noflags;
    } else if (op->func == addr8e8_16) {
        op->func = addr8e8_16_noflags;
    } else {
        kpanic("decode002_noflags error");
    }
}
void OPCALL addr16r16(struct CPU* cpu, struct Op* op);
void OPCALL addr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL addr16e16_16(struct CPU* cpu, struct Op* op);
// ADD Gw,Ew
void decode003_noflags(struct Op* op) {
    if (op->func == addr16r16) {
        op->func = addr16r16_noflags;
    } else if (op->func == addr16e16_32) {
        op->func = addr16e16_32_noflags;
    } else if (op->func == addr16e16_16) {
        op->func = addr16e16_16_noflags;
    } else {
        kpanic("decode003_noflags error");
    }
}
void OPCALL addr32r32(struct CPU* cpu, struct Op* op);
void OPCALL addr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL addr32e32_16(struct CPU* cpu, struct Op* op);
// ADD Gd,Ed
void decode203_noflags(struct Op* op) {
    if (op->func == addr32r32) {
        op->func = addr32r32_noflags;
    } else if (op->func == addr32e32_32) {
        op->func = addr32e32_32_noflags;
    } else if (op->func == addr32e32_16) {
        op->func = addr32e32_16_noflags;
    } else {
        kpanic("decode203_noflags error");
    }
}
void OPCALL add8_reg(struct CPU* cpu, struct Op* op);
// ADD Al,Ib
void decode004_noflags(struct Op* op) {
    if (op->func == add8_reg) {
        op->func = add8_reg_noflags;
    } else {
        kpanic("decode004_noflags error");
    }
}
void OPCALL add16_reg(struct CPU* cpu, struct Op* op);
// ADD Ax,Iw
void decode005_noflags(struct Op* op) {
    if (op->func == add16_reg) {
        op->func = add16_reg_noflags;
    } else {
        kpanic("decode005_noflags error");
    }
}
void OPCALL add32_reg(struct CPU* cpu, struct Op* op);
// ADD Eax,Id
void decode205_noflags(struct Op* op) {
    if (op->func == add32_reg) {
        op->func = add32_reg_noflags;
    } else {
        kpanic("decode205_noflags error");
    }
}
void OPCALL orr8r8(struct CPU* cpu, struct Op* op);
void OPCALL ore8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL ore8r8_16(struct CPU* cpu, struct Op* op);
// OR Eb,Gb
void decode008_noflags(struct Op* op) {
    if (op->func == orr8r8) {
        op->func = orr8r8_noflags;
    } else if (op->func == ore8r8_32) {
        op->func = ore8r8_32_noflags;
    } else if (op->func == ore8r8_16) {
        op->func = ore8r8_16_noflags;
    } else {
        kpanic("decode008_noflags error");
    }
}
void OPCALL orr16r16(struct CPU* cpu, struct Op* op);
void OPCALL ore16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL ore16r16_16(struct CPU* cpu, struct Op* op);
// OR Ew,Gw
void decode009_noflags(struct Op* op) {
    if (op->func == orr16r16) {
        op->func = orr16r16_noflags;
    } else if (op->func == ore16r16_32) {
        op->func = ore16r16_32_noflags;
    } else if (op->func == ore16r16_16) {
        op->func = ore16r16_16_noflags;
    } else {
        kpanic("decode009_noflags error");
    }
}
void OPCALL orr32r32(struct CPU* cpu, struct Op* op);
void OPCALL ore32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL ore32r32_16(struct CPU* cpu, struct Op* op);
// OR Ed,Gd
void decode209_noflags(struct Op* op) {
    if (op->func == orr32r32) {
        op->func = orr32r32_noflags;
    } else if (op->func == ore32r32_32) {
        op->func = ore32r32_32_noflags;
    } else if (op->func == ore32r32_16) {
        op->func = ore32r32_16_noflags;
    } else {
        kpanic("decode209_noflags error");
    }
}
void OPCALL orr8r8(struct CPU* cpu, struct Op* op);
void OPCALL orr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL orr8e8_16(struct CPU* cpu, struct Op* op);
// OR Gb,Eb
void decode00a_noflags(struct Op* op) {
    if (op->func == orr8r8) {
        op->func = orr8r8_noflags;
    } else if (op->func == orr8e8_32) {
        op->func = orr8e8_32_noflags;
    } else if (op->func == orr8e8_16) {
        op->func = orr8e8_16_noflags;
    } else {
        kpanic("decode00a_noflags error");
    }
}
void OPCALL orr16r16(struct CPU* cpu, struct Op* op);
void OPCALL orr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL orr16e16_16(struct CPU* cpu, struct Op* op);
// OR Gw,Ew
void decode00b_noflags(struct Op* op) {
    if (op->func == orr16r16) {
        op->func = orr16r16_noflags;
    } else if (op->func == orr16e16_32) {
        op->func = orr16e16_32_noflags;
    } else if (op->func == orr16e16_16) {
        op->func = orr16e16_16_noflags;
    } else {
        kpanic("decode00b_noflags error");
    }
}
void OPCALL orr32r32(struct CPU* cpu, struct Op* op);
void OPCALL orr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL orr32e32_16(struct CPU* cpu, struct Op* op);
// OR Gd,Ed
void decode20b_noflags(struct Op* op) {
    if (op->func == orr32r32) {
        op->func = orr32r32_noflags;
    } else if (op->func == orr32e32_32) {
        op->func = orr32e32_32_noflags;
    } else if (op->func == orr32e32_16) {
        op->func = orr32e32_16_noflags;
    } else {
        kpanic("decode20b_noflags error");
    }
}
void OPCALL or8_reg(struct CPU* cpu, struct Op* op);
// OR Al,Ib
void decode00c_noflags(struct Op* op) {
    if (op->func == or8_reg) {
        op->func = or8_reg_noflags;
    } else {
        kpanic("decode00c_noflags error");
    }
}
void OPCALL or16_reg(struct CPU* cpu, struct Op* op);
// OR Ax,Iw
void decode00d_noflags(struct Op* op) {
    if (op->func == or16_reg) {
        op->func = or16_reg_noflags;
    } else {
        kpanic("decode00d_noflags error");
    }
}
void OPCALL or32_reg(struct CPU* cpu, struct Op* op);
// OR Eax,Id
void decode20d_noflags(struct Op* op) {
    if (op->func == or32_reg) {
        op->func = or32_reg_noflags;
    } else {
        kpanic("decode20d_noflags error");
    }
}
void OPCALL adcr8r8(struct CPU* cpu, struct Op* op);
void OPCALL adce8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL adce8r8_16(struct CPU* cpu, struct Op* op);
// ADC Eb,Gb
void decode010_noflags(struct Op* op) {
    if (op->func == adcr8r8) {
        op->func = adcr8r8_noflags;
    } else if (op->func == adce8r8_32) {
        op->func = adce8r8_32_noflags;
    } else if (op->func == adce8r8_16) {
        op->func = adce8r8_16_noflags;
    } else {
        kpanic("decode010_noflags error");
    }
}
void OPCALL adcr16r16(struct CPU* cpu, struct Op* op);
void OPCALL adce16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL adce16r16_16(struct CPU* cpu, struct Op* op);
// ADC Ew,Gw
void decode011_noflags(struct Op* op) {
    if (op->func == adcr16r16) {
        op->func = adcr16r16_noflags;
    } else if (op->func == adce16r16_32) {
        op->func = adce16r16_32_noflags;
    } else if (op->func == adce16r16_16) {
        op->func = adce16r16_16_noflags;
    } else {
        kpanic("decode011_noflags error");
    }
}
void OPCALL adcr32r32(struct CPU* cpu, struct Op* op);
void OPCALL adce32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL adce32r32_16(struct CPU* cpu, struct Op* op);
// ADC Ed,Gd
void decode211_noflags(struct Op* op) {
    if (op->func == adcr32r32) {
        op->func = adcr32r32_noflags;
    } else if (op->func == adce32r32_32) {
        op->func = adce32r32_32_noflags;
    } else if (op->func == adce32r32_16) {
        op->func = adce32r32_16_noflags;
    } else {
        kpanic("decode211_noflags error");
    }
}
void OPCALL adcr8r8(struct CPU* cpu, struct Op* op);
void OPCALL adcr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL adcr8e8_16(struct CPU* cpu, struct Op* op);
// ADC Gb,Eb
void decode012_noflags(struct Op* op) {
    if (op->func == adcr8r8) {
        op->func = adcr8r8_noflags;
    } else if (op->func == adcr8e8_32) {
        op->func = adcr8e8_32_noflags;
    } else if (op->func == adcr8e8_16) {
        op->func = adcr8e8_16_noflags;
    } else {
        kpanic("decode012_noflags error");
    }
}
void OPCALL adcr16r16(struct CPU* cpu, struct Op* op);
void OPCALL adcr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL adcr16e16_16(struct CPU* cpu, struct Op* op);
// ADC Gw,Ew
void decode013_noflags(struct Op* op) {
    if (op->func == adcr16r16) {
        op->func = adcr16r16_noflags;
    } else if (op->func == adcr16e16_32) {
        op->func = adcr16e16_32_noflags;
    } else if (op->func == adcr16e16_16) {
        op->func = adcr16e16_16_noflags;
    } else {
        kpanic("decode013_noflags error");
    }
}
void OPCALL adcr32r32(struct CPU* cpu, struct Op* op);
void OPCALL adcr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL adcr32e32_16(struct CPU* cpu, struct Op* op);
// ADC Gd,Ed
void decode213_noflags(struct Op* op) {
    if (op->func == adcr32r32) {
        op->func = adcr32r32_noflags;
    } else if (op->func == adcr32e32_32) {
        op->func = adcr32e32_32_noflags;
    } else if (op->func == adcr32e32_16) {
        op->func = adcr32e32_16_noflags;
    } else {
        kpanic("decode213_noflags error");
    }
}
void OPCALL adc8_reg(struct CPU* cpu, struct Op* op);
// ADC Al,Ib
void decode014_noflags(struct Op* op) {
    if (op->func == adc8_reg) {
        op->func = adc8_reg_noflags;
    } else {
        kpanic("decode014_noflags error");
    }
}
void OPCALL adc16_reg(struct CPU* cpu, struct Op* op);
// ADC Ax,Iw
void decode015_noflags(struct Op* op) {
    if (op->func == adc16_reg) {
        op->func = adc16_reg_noflags;
    } else {
        kpanic("decode015_noflags error");
    }
}
void OPCALL adc32_reg(struct CPU* cpu, struct Op* op);
// ADC Eax,Id
void decode215_noflags(struct Op* op) {
    if (op->func == adc32_reg) {
        op->func = adc32_reg_noflags;
    } else {
        kpanic("decode215_noflags error");
    }
}
void OPCALL sbbr8r8(struct CPU* cpu, struct Op* op);
void OPCALL sbbe8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL sbbe8r8_16(struct CPU* cpu, struct Op* op);
// SBB Eb,Gb
void decode018_noflags(struct Op* op) {
    if (op->func == sbbr8r8) {
        op->func = sbbr8r8_noflags;
    } else if (op->func == sbbe8r8_32) {
        op->func = sbbe8r8_32_noflags;
    } else if (op->func == sbbe8r8_16) {
        op->func = sbbe8r8_16_noflags;
    } else {
        kpanic("decode018_noflags error");
    }
}
void OPCALL sbbr16r16(struct CPU* cpu, struct Op* op);
void OPCALL sbbe16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL sbbe16r16_16(struct CPU* cpu, struct Op* op);
// SBB Ew,Gw
void decode019_noflags(struct Op* op) {
    if (op->func == sbbr16r16) {
        op->func = sbbr16r16_noflags;
    } else if (op->func == sbbe16r16_32) {
        op->func = sbbe16r16_32_noflags;
    } else if (op->func == sbbe16r16_16) {
        op->func = sbbe16r16_16_noflags;
    } else {
        kpanic("decode019_noflags error");
    }
}
void OPCALL sbbr32r32(struct CPU* cpu, struct Op* op);
void OPCALL sbbe32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL sbbe32r32_16(struct CPU* cpu, struct Op* op);
// SBB Ed,Gd
void decode219_noflags(struct Op* op) {
    if (op->func == sbbr32r32) {
        op->func = sbbr32r32_noflags;
    } else if (op->func == sbbe32r32_32) {
        op->func = sbbe32r32_32_noflags;
    } else if (op->func == sbbe32r32_16) {
        op->func = sbbe32r32_16_noflags;
    } else {
        kpanic("decode219_noflags error");
    }
}
void OPCALL sbbr8r8(struct CPU* cpu, struct Op* op);
void OPCALL sbbr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL sbbr8e8_16(struct CPU* cpu, struct Op* op);
// SBB Gb,Eb
void decode01a_noflags(struct Op* op) {
    if (op->func == sbbr8r8) {
        op->func = sbbr8r8_noflags;
    } else if (op->func == sbbr8e8_32) {
        op->func = sbbr8e8_32_noflags;
    } else if (op->func == sbbr8e8_16) {
        op->func = sbbr8e8_16_noflags;
    } else {
        kpanic("decode01a_noflags error");
    }
}
void OPCALL sbbr16r16(struct CPU* cpu, struct Op* op);
void OPCALL sbbr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL sbbr16e16_16(struct CPU* cpu, struct Op* op);
// SBB Gw,Ew
void decode01b_noflags(struct Op* op) {
    if (op->func == sbbr16r16) {
        op->func = sbbr16r16_noflags;
    } else if (op->func == sbbr16e16_32) {
        op->func = sbbr16e16_32_noflags;
    } else if (op->func == sbbr16e16_16) {
        op->func = sbbr16e16_16_noflags;
    } else {
        kpanic("decode01b_noflags error");
    }
}
void OPCALL sbbr32r32(struct CPU* cpu, struct Op* op);
void OPCALL sbbr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL sbbr32e32_16(struct CPU* cpu, struct Op* op);
// SBB Gd,Ed
void decode21b_noflags(struct Op* op) {
    if (op->func == sbbr32r32) {
        op->func = sbbr32r32_noflags;
    } else if (op->func == sbbr32e32_32) {
        op->func = sbbr32e32_32_noflags;
    } else if (op->func == sbbr32e32_16) {
        op->func = sbbr32e32_16_noflags;
    } else {
        kpanic("decode21b_noflags error");
    }
}
void OPCALL sbb8_reg(struct CPU* cpu, struct Op* op);
// SBB Al,Ib
void decode01c_noflags(struct Op* op) {
    if (op->func == sbb8_reg) {
        op->func = sbb8_reg_noflags;
    } else {
        kpanic("decode01c_noflags error");
    }
}
void OPCALL sbb16_reg(struct CPU* cpu, struct Op* op);
// SBB Ax,Iw
void decode01d_noflags(struct Op* op) {
    if (op->func == sbb16_reg) {
        op->func = sbb16_reg_noflags;
    } else {
        kpanic("decode01d_noflags error");
    }
}
void OPCALL sbb32_reg(struct CPU* cpu, struct Op* op);
// SBB Eax,Id
void decode21d_noflags(struct Op* op) {
    if (op->func == sbb32_reg) {
        op->func = sbb32_reg_noflags;
    } else {
        kpanic("decode21d_noflags error");
    }
}
void OPCALL andr8r8(struct CPU* cpu, struct Op* op);
void OPCALL ande8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL ande8r8_16(struct CPU* cpu, struct Op* op);
// AND Eb,Gb
void decode020_noflags(struct Op* op) {
    if (op->func == andr8r8) {
        op->func = andr8r8_noflags;
    } else if (op->func == ande8r8_32) {
        op->func = ande8r8_32_noflags;
    } else if (op->func == ande8r8_16) {
        op->func = ande8r8_16_noflags;
    } else {
        kpanic("decode020_noflags error");
    }
}
void OPCALL andr16r16(struct CPU* cpu, struct Op* op);
void OPCALL ande16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL ande16r16_16(struct CPU* cpu, struct Op* op);
// AND Ew,Gw
void decode021_noflags(struct Op* op) {
    if (op->func == andr16r16) {
        op->func = andr16r16_noflags;
    } else if (op->func == ande16r16_32) {
        op->func = ande16r16_32_noflags;
    } else if (op->func == ande16r16_16) {
        op->func = ande16r16_16_noflags;
    } else {
        kpanic("decode021_noflags error");
    }
}
void OPCALL andr32r32(struct CPU* cpu, struct Op* op);
void OPCALL ande32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL ande32r32_16(struct CPU* cpu, struct Op* op);
// AND Ed,Gd
void decode221_noflags(struct Op* op) {
    if (op->func == andr32r32) {
        op->func = andr32r32_noflags;
    } else if (op->func == ande32r32_32) {
        op->func = ande32r32_32_noflags;
    } else if (op->func == ande32r32_16) {
        op->func = ande32r32_16_noflags;
    } else {
        kpanic("decode221_noflags error");
    }
}
void OPCALL andr8r8(struct CPU* cpu, struct Op* op);
void OPCALL andr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL andr8e8_16(struct CPU* cpu, struct Op* op);
// AND Gb,Eb
void decode022_noflags(struct Op* op) {
    if (op->func == andr8r8) {
        op->func = andr8r8_noflags;
    } else if (op->func == andr8e8_32) {
        op->func = andr8e8_32_noflags;
    } else if (op->func == andr8e8_16) {
        op->func = andr8e8_16_noflags;
    } else {
        kpanic("decode022_noflags error");
    }
}
void OPCALL andr16r16(struct CPU* cpu, struct Op* op);
void OPCALL andr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL andr16e16_16(struct CPU* cpu, struct Op* op);
// AND Gw,Ew
void decode023_noflags(struct Op* op) {
    if (op->func == andr16r16) {
        op->func = andr16r16_noflags;
    } else if (op->func == andr16e16_32) {
        op->func = andr16e16_32_noflags;
    } else if (op->func == andr16e16_16) {
        op->func = andr16e16_16_noflags;
    } else {
        kpanic("decode023_noflags error");
    }
}
void OPCALL andr32r32(struct CPU* cpu, struct Op* op);
void OPCALL andr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL andr32e32_16(struct CPU* cpu, struct Op* op);
// AND Gd,Ed
void decode223_noflags(struct Op* op) {
    if (op->func == andr32r32) {
        op->func = andr32r32_noflags;
    } else if (op->func == andr32e32_32) {
        op->func = andr32e32_32_noflags;
    } else if (op->func == andr32e32_16) {
        op->func = andr32e32_16_noflags;
    } else {
        kpanic("decode223_noflags error");
    }
}
void OPCALL and8_reg(struct CPU* cpu, struct Op* op);
// AND Al,Ib
void decode024_noflags(struct Op* op) {
    if (op->func == and8_reg) {
        op->func = and8_reg_noflags;
    } else {
        kpanic("decode024_noflags error");
    }
}
void OPCALL and16_reg(struct CPU* cpu, struct Op* op);
// AND Ax,Iw
void decode025_noflags(struct Op* op) {
    if (op->func == and16_reg) {
        op->func = and16_reg_noflags;
    } else {
        kpanic("decode025_noflags error");
    }
}
void OPCALL and32_reg(struct CPU* cpu, struct Op* op);
// AND Eax,Id
void decode225_noflags(struct Op* op) {
    if (op->func == and32_reg) {
        op->func = and32_reg_noflags;
    } else {
        kpanic("decode225_noflags error");
    }
}
void OPCALL subr8r8(struct CPU* cpu, struct Op* op);
void OPCALL sube8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL sube8r8_16(struct CPU* cpu, struct Op* op);
// SUB Eb,Gb
void decode028_noflags(struct Op* op) {
    if (op->func == subr8r8) {
        op->func = subr8r8_noflags;
    } else if (op->func == sube8r8_32) {
        op->func = sube8r8_32_noflags;
    } else if (op->func == sube8r8_16) {
        op->func = sube8r8_16_noflags;
    } else {
        kpanic("decode028_noflags error");
    }
}
void OPCALL subr16r16(struct CPU* cpu, struct Op* op);
void OPCALL sube16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL sube16r16_16(struct CPU* cpu, struct Op* op);
// SUB Ew,Gw
void decode029_noflags(struct Op* op) {
    if (op->func == subr16r16) {
        op->func = subr16r16_noflags;
    } else if (op->func == sube16r16_32) {
        op->func = sube16r16_32_noflags;
    } else if (op->func == sube16r16_16) {
        op->func = sube16r16_16_noflags;
    } else {
        kpanic("decode029_noflags error");
    }
}
void OPCALL subr32r32(struct CPU* cpu, struct Op* op);
void OPCALL sube32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL sube32r32_16(struct CPU* cpu, struct Op* op);
// SUB Ed,Gd
void decode229_noflags(struct Op* op) {
    if (op->func == subr32r32) {
        op->func = subr32r32_noflags;
    } else if (op->func == sube32r32_32) {
        op->func = sube32r32_32_noflags;
    } else if (op->func == sube32r32_16) {
        op->func = sube32r32_16_noflags;
    } else {
        kpanic("decode229_noflags error");
    }
}
void OPCALL subr8r8(struct CPU* cpu, struct Op* op);
void OPCALL subr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL subr8e8_16(struct CPU* cpu, struct Op* op);
// SUB Gb,Eb
void decode02a_noflags(struct Op* op) {
    if (op->func == subr8r8) {
        op->func = subr8r8_noflags;
    } else if (op->func == subr8e8_32) {
        op->func = subr8e8_32_noflags;
    } else if (op->func == subr8e8_16) {
        op->func = subr8e8_16_noflags;
    } else {
        kpanic("decode02a_noflags error");
    }
}
void OPCALL subr16r16(struct CPU* cpu, struct Op* op);
void OPCALL subr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL subr16e16_16(struct CPU* cpu, struct Op* op);
// SUB Gw,Ew
void decode02b_noflags(struct Op* op) {
    if (op->func == subr16r16) {
        op->func = subr16r16_noflags;
    } else if (op->func == subr16e16_32) {
        op->func = subr16e16_32_noflags;
    } else if (op->func == subr16e16_16) {
        op->func = subr16e16_16_noflags;
    } else {
        kpanic("decode02b_noflags error");
    }
}
void OPCALL subr32r32(struct CPU* cpu, struct Op* op);
void OPCALL subr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL subr32e32_16(struct CPU* cpu, struct Op* op);
// SUB Gd,Ed
void decode22b_noflags(struct Op* op) {
    if (op->func == subr32r32) {
        op->func = subr32r32_noflags;
    } else if (op->func == subr32e32_32) {
        op->func = subr32e32_32_noflags;
    } else if (op->func == subr32e32_16) {
        op->func = subr32e32_16_noflags;
    } else {
        kpanic("decode22b_noflags error");
    }
}
void OPCALL sub8_reg(struct CPU* cpu, struct Op* op);
// SUB Al,Ib
void decode02c_noflags(struct Op* op) {
    if (op->func == sub8_reg) {
        op->func = sub8_reg_noflags;
    } else {
        kpanic("decode02c_noflags error");
    }
}
void OPCALL sub16_reg(struct CPU* cpu, struct Op* op);
// SUB Ax,Iw
void decode02d_noflags(struct Op* op) {
    if (op->func == sub16_reg) {
        op->func = sub16_reg_noflags;
    } else {
        kpanic("decode02d_noflags error");
    }
}
void OPCALL sub32_reg(struct CPU* cpu, struct Op* op);
// SUB Eax,Id
void decode22d_noflags(struct Op* op) {
    if (op->func == sub32_reg) {
        op->func = sub32_reg_noflags;
    } else {
        kpanic("decode22d_noflags error");
    }
}
void OPCALL xorr8r8(struct CPU* cpu, struct Op* op);
void OPCALL xore8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL xore8r8_16(struct CPU* cpu, struct Op* op);
// XOR Eb,Gb
void decode030_noflags(struct Op* op) {
    if (op->func == xorr8r8) {
        op->func = xorr8r8_noflags;
    } else if (op->func == xore8r8_32) {
        op->func = xore8r8_32_noflags;
    } else if (op->func == xore8r8_16) {
        op->func = xore8r8_16_noflags;
    } else {
        kpanic("decode030_noflags error");
    }
}
void OPCALL xorr16r16(struct CPU* cpu, struct Op* op);
void OPCALL xore16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL xore16r16_16(struct CPU* cpu, struct Op* op);
// XOR Ew,Gw
void decode031_noflags(struct Op* op) {
    if (op->func == xorr16r16) {
        op->func = xorr16r16_noflags;
    } else if (op->func == xore16r16_32) {
        op->func = xore16r16_32_noflags;
    } else if (op->func == xore16r16_16) {
        op->func = xore16r16_16_noflags;
    } else {
        kpanic("decode031_noflags error");
    }
}
void OPCALL xorr32r32(struct CPU* cpu, struct Op* op);
void OPCALL xore32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL xore32r32_16(struct CPU* cpu, struct Op* op);
// XOR Ed,Gd
void decode231_noflags(struct Op* op) {
    if (op->func == xorr32r32) {
        op->func = xorr32r32_noflags;
    } else if (op->func == xore32r32_32) {
        op->func = xore32r32_32_noflags;
    } else if (op->func == xore32r32_16) {
        op->func = xore32r32_16_noflags;
    } else {
        kpanic("decode231_noflags error");
    }
}
void OPCALL xorr8r8(struct CPU* cpu, struct Op* op);
void OPCALL xorr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL xorr8e8_16(struct CPU* cpu, struct Op* op);
// XOR Gb,Eb
void decode032_noflags(struct Op* op) {
    if (op->func == xorr8r8) {
        op->func = xorr8r8_noflags;
    } else if (op->func == xorr8e8_32) {
        op->func = xorr8e8_32_noflags;
    } else if (op->func == xorr8e8_16) {
        op->func = xorr8e8_16_noflags;
    } else {
        kpanic("decode032_noflags error");
    }
}
void OPCALL xorr16r16(struct CPU* cpu, struct Op* op);
void OPCALL xorr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL xorr16e16_16(struct CPU* cpu, struct Op* op);
// XOR Gw,Ew
void decode033_noflags(struct Op* op) {
    if (op->func == xorr16r16) {
        op->func = xorr16r16_noflags;
    } else if (op->func == xorr16e16_32) {
        op->func = xorr16e16_32_noflags;
    } else if (op->func == xorr16e16_16) {
        op->func = xorr16e16_16_noflags;
    } else {
        kpanic("decode033_noflags error");
    }
}
void OPCALL xorr32r32(struct CPU* cpu, struct Op* op);
void OPCALL xorr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL xorr32e32_16(struct CPU* cpu, struct Op* op);
// XOR Gd,Ed
void decode233_noflags(struct Op* op) {
    if (op->func == xorr32r32) {
        op->func = xorr32r32_noflags;
    } else if (op->func == xorr32e32_32) {
        op->func = xorr32e32_32_noflags;
    } else if (op->func == xorr32e32_16) {
        op->func = xorr32e32_16_noflags;
    } else {
        kpanic("decode233_noflags error");
    }
}
void OPCALL xor8_reg(struct CPU* cpu, struct Op* op);
// XOR Al,Ib
void decode034_noflags(struct Op* op) {
    if (op->func == xor8_reg) {
        op->func = xor8_reg_noflags;
    } else {
        kpanic("decode034_noflags error");
    }
}
void OPCALL xor16_reg(struct CPU* cpu, struct Op* op);
// XOR Ax,Iw
void decode035_noflags(struct Op* op) {
    if (op->func == xor16_reg) {
        op->func = xor16_reg_noflags;
    } else {
        kpanic("decode035_noflags error");
    }
}
void OPCALL xor32_reg(struct CPU* cpu, struct Op* op);
// XOR Eax,Id
void decode235_noflags(struct Op* op) {
    if (op->func == xor32_reg) {
        op->func = xor32_reg_noflags;
    } else {
        kpanic("decode235_noflags error");
    }
}
void OPCALL inc8_reg_noflags(struct CPU* cpu, struct Op* op) {
    *cpu->reg8[op->r1] = *cpu->reg8[op->r1]+1;
    CYCLES(1);
    NEXT();
}
void OPCALL inc8_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa)+1);
    CYCLES(3);
    NEXT();
}
void OPCALL inc8_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa)+1);
    CYCLES(3);
    NEXT();
}
void OPCALL inc16_reg_noflags(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = cpu->reg[op->r1].u16+1;
    CYCLES(1);
    NEXT();
}
void OPCALL inc16_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa)+1);
    CYCLES(3);
    NEXT();
}
void OPCALL inc16_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa)+1);
    CYCLES(3);
    NEXT();
}
void OPCALL inc32_reg_noflags(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = cpu->reg[op->r1].u32+1;
    CYCLES(1);
    NEXT();
}
void OPCALL inc32_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa)+1);
    CYCLES(3);
    NEXT();
}
void OPCALL inc32_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa)+1);
    CYCLES(3);
    NEXT();
}
void OPCALL dec8_reg_noflags(struct CPU* cpu, struct Op* op) {
    *cpu->reg8[op->r1] = *cpu->reg8[op->r1]-1;
    CYCLES(1);
    NEXT();
}
void OPCALL dec8_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa)-1);
    CYCLES(3);
    NEXT();
}
void OPCALL dec8_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    writeb(cpu->thread, eaa, readb(cpu->thread, eaa)-1);
    CYCLES(3);
    NEXT();
}
void OPCALL dec16_reg_noflags(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = cpu->reg[op->r1].u16-1;
    CYCLES(1);
    NEXT();
}
void OPCALL dec16_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa)-1);
    CYCLES(3);
    NEXT();
}
void OPCALL dec16_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    writew(cpu->thread, eaa, readw(cpu->thread, eaa)-1);
    CYCLES(3);
    NEXT();
}
void OPCALL dec32_reg_noflags(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = cpu->reg[op->r1].u32-1;
    CYCLES(1);
    NEXT();
}
void OPCALL dec32_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa)-1);
    CYCLES(3);
    NEXT();
}
void OPCALL dec32_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    writed(cpu->thread, eaa, readd(cpu->thread, eaa)-1);
    CYCLES(3);
    NEXT();
}
void OPCALL rol8_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 reg=op->r1;
    U8 result;
    U8 var1=*cpu->reg8[reg];
    result = (var1 << var2) | (var1 >> (8 - var2));
    *cpu->reg8[reg] = result;
    CYCLES(1);
    NEXT();
}
void OPCALL rol8_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = (var1 << var2) | (var1 >> (8 - var2));
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL rol8_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = (var1 << var2) | (var1 >> (8 - var2));
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL rol8cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U8 result;
    U8 var1;
    if (var2) {
    var2&=7;
    var1 = *cpu->reg8[reg];
    result = (var1 << var2) | (var1 >> (8 - var2));
    *cpu->reg8[reg] = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL rol8cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1;
    if (var2) {
    var2&=7;
    var1 = readb(cpu->thread, eaa);
    result = (var1 << var2) | (var1 >> (8 - var2));
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL rol8cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1;
    if (var2) {
    var2&=7;
    var1 = readb(cpu->thread, eaa);
    result = (var1 << var2) | (var1 >> (8 - var2));
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL rol16_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 reg=op->r1;
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    result = (var1 << var2) | (var1 >> (16 - var2));
    cpu->reg[reg].u16 = result;
    CYCLES(1);
    NEXT();
}
void OPCALL rol16_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = (var1 << var2) | (var1 >> (16 - var2));
    writew(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL rol16_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = (var1 << var2) | (var1 >> (16 - var2));
    writew(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL rol16cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U16 result;
    U16 var1;
    if (var2) {
    var2&=15;
    var1 = cpu->reg[reg].u16;
    result = (var1 << var2) | (var1 >> (16 - var2));
    cpu->reg[reg].u16 = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL rol16cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1;
    if (var2) {
    var2&=15;
    var1 = readw(cpu->thread, eaa);
    result = (var1 << var2) | (var1 >> (16 - var2));
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL rol16cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1;
    if (var2) {
    var2&=15;
    var1 = readw(cpu->thread, eaa);
    result = (var1 << var2) | (var1 >> (16 - var2));
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL rol32_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    result = (var1 << var2) | (var1 >> (32 - var2));
    cpu->reg[reg].u32 = result;
    CYCLES(1);
    NEXT();
}
void OPCALL rol32_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = (var1 << var2) | (var1 >> (32 - var2));
    writed(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL rol32_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = (var1 << var2) | (var1 >> (32 - var2));
    writed(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL rol32cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    result = (var1 << var2) | (var1 >> (32 - var2));
    cpu->reg[reg].u32 = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL rol32cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = (var1 << var2) | (var1 >> (32 - var2));
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL rol32cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = (var1 << var2) | (var1 >> (32 - var2));
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL ror8_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 reg=op->r1;
    U8 result;
    U8 var1=*cpu->reg8[reg];
    result = (var1 >> var2) | (var1 << (8 - var2));
    *cpu->reg8[reg] = result;
    CYCLES(1);
    NEXT();
}
void OPCALL ror8_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = (var1 >> var2) | (var1 << (8 - var2));
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL ror8_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = (var1 >> var2) | (var1 << (8 - var2));
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL ror8cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U8 result;
    U8 var1;
    if (var2) {
    var2&=7;
    var1 = *cpu->reg8[reg];
    result = (var1 >> var2) | (var1 << (8 - var2));
    *cpu->reg8[reg] = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL ror8cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1;
    if (var2) {
    var2&=7;
    var1 = readb(cpu->thread, eaa);
    result = (var1 >> var2) | (var1 << (8 - var2));
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL ror8cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1;
    if (var2) {
    var2&=7;
    var1 = readb(cpu->thread, eaa);
    result = (var1 >> var2) | (var1 << (8 - var2));
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL ror16_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 reg=op->r1;
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    result = (var1 >> var2) | (var1 << (16 - var2));
    cpu->reg[reg].u16 = result;
    CYCLES(1);
    NEXT();
}
void OPCALL ror16_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = (var1 >> var2) | (var1 << (16 - var2));
    writew(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL ror16_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = (var1 >> var2) | (var1 << (16 - var2));
    writew(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL ror16cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U16 result;
    U16 var1;
    if (var2) {
    var2&=15;
    var1 = cpu->reg[reg].u16;
    result = (var1 >> var2) | (var1 << (16 - var2));
    cpu->reg[reg].u16 = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL ror16cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1;
    if (var2) {
    var2&=15;
    var1 = readw(cpu->thread, eaa);
    result = (var1 >> var2) | (var1 << (16 - var2));
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL ror16cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1;
    if (var2) {
    var2&=15;
    var1 = readw(cpu->thread, eaa);
    result = (var1 >> var2) | (var1 << (16 - var2));
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL ror32_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    result = (var1 >> var2) | (var1 << (32 - var2));
    cpu->reg[reg].u32 = result;
    CYCLES(1);
    NEXT();
}
void OPCALL ror32_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = (var1 >> var2) | (var1 << (32 - var2));
    writed(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL ror32_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = (var1 >> var2) | (var1 << (32 - var2));
    writed(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL ror32cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    result = (var1 >> var2) | (var1 << (32 - var2));
    cpu->reg[reg].u32 = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL ror32cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = (var1 >> var2) | (var1 << (32 - var2));
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL ror32cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = (var1 >> var2) | (var1 << (32 - var2));
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL rcl8_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 reg=op->r1;
    U8 result;
    U8 var1=*cpu->reg8[reg];
    result = (var1 << var2) | (getCF(cpu) << (var2-1)) | (var1 >> (9-var2));
    *cpu->reg8[reg] = result;
    CYCLES(8);
    NEXT();
}
void OPCALL rcl8_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = (var1 << var2) | (getCF(cpu) << (var2-1)) | (var1 >> (9-var2));
    writeb(cpu->thread, eaa, result);
    CYCLES(10);
    NEXT();
}
void OPCALL rcl8_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = (var1 << var2) | (getCF(cpu) << (var2-1)) | (var1 >> (9-var2));
    writeb(cpu->thread, eaa, result);
    CYCLES(10);
    NEXT();
}
void OPCALL rcl8cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U8 result;
    U8 var1;
    if (var2) {
    var2=var2 % 9;
    var1 = *cpu->reg8[reg];
    result = (var1 << var2) | (getCF(cpu) << (var2-1)) | (var1 >> (9-var2));
    *cpu->reg8[reg] = result;
    }
    CYCLES(7);
    NEXT();
}
void OPCALL rcl8cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1;
    if (var2) {
    var2=var2 % 9;
    var1 = readb(cpu->thread, eaa);
    result = (var1 << var2) | (getCF(cpu) << (var2-1)) | (var1 >> (9-var2));
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(9);
    NEXT();
}
void OPCALL rcl8cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1;
    if (var2) {
    var2=var2 % 9;
    var1 = readb(cpu->thread, eaa);
    result = (var1 << var2) | (getCF(cpu) << (var2-1)) | (var1 >> (9-var2));
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(9);
    NEXT();
}
void OPCALL rcl16_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 reg=op->r1;
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    result = (var1 << var2) | (getCF(cpu) << (var2-1)) | (var1 >> (17-var2));
    cpu->reg[reg].u16 = result;
    CYCLES(8);
    NEXT();
}
void OPCALL rcl16_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = (var1 << var2) | (getCF(cpu) << (var2-1)) | (var1 >> (17-var2));
    writew(cpu->thread, eaa, result);
    CYCLES(10);
    NEXT();
}
void OPCALL rcl16_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = (var1 << var2) | (getCF(cpu) << (var2-1)) | (var1 >> (17-var2));
    writew(cpu->thread, eaa, result);
    CYCLES(10);
    NEXT();
}
void OPCALL rcl16cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U16 result;
    U16 var1;
    if (var2) {
    var2=var2 % 17;
    var1 = cpu->reg[reg].u16;
    result = (var1 << var2) | (getCF(cpu) << (var2-1)) | (var1 >> (17-var2));
    cpu->reg[reg].u16 = result;
    }
    CYCLES(7);
    NEXT();
}
void OPCALL rcl16cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1;
    if (var2) {
    var2=var2 % 17;
    var1 = readw(cpu->thread, eaa);
    result = (var1 << var2) | (getCF(cpu) << (var2-1)) | (var1 >> (17-var2));
    writew(cpu->thread, eaa, result);
    }
    CYCLES(9);
    NEXT();
}
void OPCALL rcl16cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1;
    if (var2) {
    var2=var2 % 17;
    var1 = readw(cpu->thread, eaa);
    result = (var1 << var2) | (getCF(cpu) << (var2-1)) | (var1 >> (17-var2));
    writew(cpu->thread, eaa, result);
    }
    CYCLES(9);
    NEXT();
}
void OPCALL rcl32_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2==1) {
        result = (var1 << var2) | getCF(cpu);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    cpu->reg[reg].u32 = result;
    CYCLES(8);
    NEXT();
}
void OPCALL rcl32_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2==1) {
        result = (var1 << var2) | getCF(cpu);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    writed(cpu->thread, eaa, result);
    CYCLES(10);
    NEXT();
}
void OPCALL rcl32_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2==1) {
        result = (var1 << var2) | getCF(cpu);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    writed(cpu->thread, eaa, result);
    CYCLES(10);
    NEXT();
}
void OPCALL rcl32cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    if (var2==1) {
        result = (var1 << var2) | getCF(cpu);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    cpu->reg[reg].u32 = result;
    }
    CYCLES(7);
    NEXT();
}
void OPCALL rcl32cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    if (var2==1) {
        result = (var1 << var2) | getCF(cpu);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    writed(cpu->thread, eaa, result);
    }
    CYCLES(9);
    NEXT();
}
void OPCALL rcl32cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    if (var2==1) {
        result = (var1 << var2) | getCF(cpu);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    writed(cpu->thread, eaa, result);
    }
    CYCLES(9);
    NEXT();
}
void OPCALL rcr8_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 reg=op->r1;
    U8 result;
    U8 var1=*cpu->reg8[reg];
    result = (var1 >> var2) | (getCF(cpu) << (8-var2)) | (var1 << (9-var2));
    *cpu->reg8[reg] = result;
    CYCLES(8);
    NEXT();
}
void OPCALL rcr8_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = (var1 >> var2) | (getCF(cpu) << (8-var2)) | (var1 << (9-var2));
    writeb(cpu->thread, eaa, result);
    CYCLES(10);
    NEXT();
}
void OPCALL rcr8_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = (var1 >> var2) | (getCF(cpu) << (8-var2)) | (var1 << (9-var2));
    writeb(cpu->thread, eaa, result);
    CYCLES(10);
    NEXT();
}
void OPCALL rcr8cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U8 result;
    U8 var1;
    if (var2) {
    var2=var2 % 9;
    var1 = *cpu->reg8[reg];
    result = (var1 >> var2) | (getCF(cpu) << (8-var2)) | (var1 << (9-var2));
    *cpu->reg8[reg] = result;
    }
    CYCLES(7);
    NEXT();
}
void OPCALL rcr8cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1;
    if (var2) {
    var2=var2 % 9;
    var1 = readb(cpu->thread, eaa);
    result = (var1 >> var2) | (getCF(cpu) << (8-var2)) | (var1 << (9-var2));
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(9);
    NEXT();
}
void OPCALL rcr8cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1;
    if (var2) {
    var2=var2 % 9;
    var1 = readb(cpu->thread, eaa);
    result = (var1 >> var2) | (getCF(cpu) << (8-var2)) | (var1 << (9-var2));
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(9);
    NEXT();
}
void OPCALL rcr16_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 reg=op->r1;
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    result = (var1 >> var2) | (getCF(cpu) << (16-var2)) | (var1 << (17-var2));
    cpu->reg[reg].u16 = result;
    CYCLES(8);
    NEXT();
}
void OPCALL rcr16_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = (var1 >> var2) | (getCF(cpu) << (16-var2)) | (var1 << (17-var2));
    writew(cpu->thread, eaa, result);
    CYCLES(10);
    NEXT();
}
void OPCALL rcr16_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = (var1 >> var2) | (getCF(cpu) << (16-var2)) | (var1 << (17-var2));
    writew(cpu->thread, eaa, result);
    CYCLES(10);
    NEXT();
}
void OPCALL rcr16cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U16 result;
    U16 var1;
    if (var2) {
    var2=var2 % 17;
    var1 = cpu->reg[reg].u16;
    result = (var1 >> var2) | (getCF(cpu) << (16-var2)) | (var1 << (17-var2));
    cpu->reg[reg].u16 = result;
    }
    CYCLES(7);
    NEXT();
}
void OPCALL rcr16cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1;
    if (var2) {
    var2=var2 % 17;
    var1 = readw(cpu->thread, eaa);
    result = (var1 >> var2) | (getCF(cpu) << (16-var2)) | (var1 << (17-var2));
    writew(cpu->thread, eaa, result);
    }
    CYCLES(9);
    NEXT();
}
void OPCALL rcr16cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1;
    if (var2) {
    var2=var2 % 17;
    var1 = readw(cpu->thread, eaa);
    result = (var1 >> var2) | (getCF(cpu) << (16-var2)) | (var1 << (17-var2));
    writew(cpu->thread, eaa, result);
    }
    CYCLES(9);
    NEXT();
}
void OPCALL rcr32_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2==1) {
        result = (var1 >> var2) | (getCF(cpu) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    cpu->reg[reg].u32 = result;
    CYCLES(8);
    NEXT();
}
void OPCALL rcr32_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2==1) {
        result = (var1 >> var2) | (getCF(cpu) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    writed(cpu->thread, eaa, result);
    CYCLES(10);
    NEXT();
}
void OPCALL rcr32_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2==1) {
        result = (var1 >> var2) | (getCF(cpu) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    writed(cpu->thread, eaa, result);
    CYCLES(10);
    NEXT();
}
void OPCALL rcr32cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    if (var2==1) {
        result = (var1 >> var2) | (getCF(cpu) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    cpu->reg[reg].u32 = result;
    }
    CYCLES(7);
    NEXT();
}
void OPCALL rcr32cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    if (var2==1) {
        result = (var1 >> var2) | (getCF(cpu) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    writed(cpu->thread, eaa, result);
    }
    CYCLES(9);
    NEXT();
}
void OPCALL rcr32cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    if (var2==1) {
        result = (var1 >> var2) | (getCF(cpu) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    writed(cpu->thread, eaa, result);
    }
    CYCLES(9);
    NEXT();
}
void OPCALL shl8_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 reg=op->r1;
    U8 result;
    U8 var1=*cpu->reg8[reg];
    result = var1 << var2;
    *cpu->reg8[reg] = result;
    CYCLES(1);
    NEXT();
}
void OPCALL shl8_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = var1 << var2;
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL shl8_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = var1 << var2;
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL shl8cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U8 result;
    U8 var1=*cpu->reg8[reg];
    if (var2) {
    result = var1 << var2;
    *cpu->reg8[reg] = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shl8cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    if (var2) {
    result = var1 << var2;
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shl8cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    if (var2) {
    result = var1 << var2;
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shl16_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 reg=op->r1;
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    result = var1 << var2;
    cpu->reg[reg].u16 = result;
    CYCLES(1);
    NEXT();
}
void OPCALL shl16_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = var1 << var2;
    writew(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL shl16_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = var1 << var2;
    writew(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL shl16cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    if (var2) {
    result = var1 << var2;
    cpu->reg[reg].u16 = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shl16cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    if (var2) {
    result = var1 << var2;
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shl16cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    if (var2) {
    result = var1 << var2;
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shl32_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    result = var1 << var2;
    cpu->reg[reg].u32 = result;
    CYCLES(1);
    NEXT();
}
void OPCALL shl32_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = var1 << var2;
    writed(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL shl32_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = var1 << var2;
    writed(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL shl32cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    result = var1 << var2;
    cpu->reg[reg].u32 = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shl32cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = var1 << var2;
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shl32cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = var1 << var2;
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shr8_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 reg=op->r1;
    U8 result;
    U8 var1=*cpu->reg8[reg];
    result = var1 >> var2;
    *cpu->reg8[reg] = result;
    CYCLES(1);
    NEXT();
}
void OPCALL shr8_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = var1 >> var2;
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL shr8_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = var1 >> var2;
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL shr8cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U8 result;
    U8 var1=*cpu->reg8[reg];
    if (var2) {
    result = var1 >> var2;
    *cpu->reg8[reg] = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shr8cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    if (var2) {
    result = var1 >> var2;
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shr8cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    if (var2) {
    result = var1 >> var2;
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shr16_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 reg=op->r1;
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    result = var1 >> var2;
    cpu->reg[reg].u16 = result;
    CYCLES(1);
    NEXT();
}
void OPCALL shr16_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = var1 >> var2;
    writew(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL shr16_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = var1 >> var2;
    writew(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL shr16cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    if (var2) {
    result = var1 >> var2;
    cpu->reg[reg].u16 = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shr16cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    if (var2) {
    result = var1 >> var2;
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shr16cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    if (var2) {
    result = var1 >> var2;
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shr32_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    result = var1 >> var2;
    cpu->reg[reg].u32 = result;
    CYCLES(1);
    NEXT();
}
void OPCALL shr32_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = var1 >> var2;
    writed(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL shr32_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = var1 >> var2;
    writed(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL shr32cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    result = var1 >> var2;
    cpu->reg[reg].u32 = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shr32cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = var1 >> var2;
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL shr32cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = var1 >> var2;
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL sar8_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 reg=op->r1;
    U8 result;
    U8 var1=*cpu->reg8[reg];
    result = (S8)var1 >> var2;
    *cpu->reg8[reg] = result;
    CYCLES(1);
    NEXT();
}
void OPCALL sar8_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = (S8)var1 >> var2;
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL sar8_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = (S8)var1 >> var2;
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL sar8cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U8 result;
    U8 var1=*cpu->reg8[reg];
    if (var2) {
    result = (S8)var1 >> var2;
    *cpu->reg8[reg] = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL sar8cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    if (var2) {
    result = (S8)var1 >> var2;
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL sar8cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U8 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    if (var2) {
    result = (S8)var1 >> var2;
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL sar16_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 reg=op->r1;
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    result = (S16)var1 >> var2;
    cpu->reg[reg].u16 = result;
    CYCLES(1);
    NEXT();
}
void OPCALL sar16_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = (S16)var1 >> var2;
    writew(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL sar16_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = (S16)var1 >> var2;
    writew(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL sar16cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    if (var2) {
    result = (S16)var1 >> var2;
    cpu->reg[reg].u16 = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL sar16cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    if (var2) {
    result = (S16)var1 >> var2;
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL sar16cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U16 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    if (var2) {
    result = (S16)var1 >> var2;
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL sar32_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    result = (S32)var1 >> var2;
    cpu->reg[reg].u32 = result;
    CYCLES(1);
    NEXT();
}
void OPCALL sar32_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = (S32)var1 >> var2;
    writed(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL sar32_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = op->data1;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = (S32)var1 >> var2;
    writed(cpu->thread, eaa, result);
    CYCLES(3);
    NEXT();
}
void OPCALL sar32cl_reg_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 reg=op->r1;
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    result = (S32)var1 >> var2;
    cpu->reg[reg].u32 = result;
    }
    CYCLES(4);
    NEXT();
}
void OPCALL sar32cl_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa16(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = (S32)var1 >> var2;
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
void OPCALL sar32cl_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 var2 = CL & 0x1f;
    U32 eaa=eaa32(cpu, op);
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = (S32)var1 >> var2;
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
    NEXT();
}
