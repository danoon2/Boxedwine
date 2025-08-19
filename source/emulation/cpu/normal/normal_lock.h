/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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

#ifdef BOXEDWINE_MULTI_THREADED

#include "../common/common_lock.h"

void OPCALL normal_cmpxchg8b_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_cmpxchg8b_lock(cpu, eaa(cpu, op));
    NEXT();
}

void OPCALL normal_cmpxchge32r32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_cmpxchge32r32_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_cmpxchge16r16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_cmpxchge16r16_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_cmpxchge8r8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_cmpxchge8r8_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_xchge32r32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_xchge32r32_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_xchge16r16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_xchge16r16_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_xchge8r8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_xchge8r8_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_xaddr32e32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_xaddr32e32_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_xaddr16e16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_xaddr16e16_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_xaddr8e8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_xaddr8e8_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_adde32r32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_adde32r32_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_adde16r16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_adde16r16_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_adde8r8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_adde8r8_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_add32_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_add32_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_add16_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_add16_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_add8_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_add8_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_sube32r32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_sube32r32_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_sube16r16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_sube16r16_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_sube8r8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_sube8r8_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_sub32_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_sub32_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_sub16_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_sub16_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_sub8_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_sub8_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_ore32r32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_ore32r32_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_ore16r16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_ore16r16_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_ore8r8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_ore8r8_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_or32_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_or32_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_or16_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_or16_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_or8_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_or8_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_ande32r32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_ande32r32_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_ande16r16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_ande16r16_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_ande8r8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_ande8r8_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_and32_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_and32_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_and16_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_and16_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_and8_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_and8_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_xore32r32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_xore32r32_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_xore16r16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_xore16r16_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_xore8r8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_xore8r8_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_xor32_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_xor32_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_xor16_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_xor16_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_xor8_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_xor8_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_adce32r32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_adce32r32_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_adce16r16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_adce16r16_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_adce8r8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_adce8r8_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_adc32_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_adc32_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_adc16_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_adc16_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_adc8_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_adc8_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_sbbe32r32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_sbbe32r32_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_sbbe16r16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_sbbe16r16_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_sbbe8r8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_sbbe8r8_lock(cpu, eaa(cpu, op), op->reg);
    NEXT();
}

void OPCALL normal_sbb32_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_sbb32_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_sbb16_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_sbb16_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_sbb8_mem_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_sbb8_mem_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_inc32_mem32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_inc32_mem32_lock(cpu, eaa(cpu, op));
    NEXT();
}

void OPCALL normal_inc16_mem16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_inc16_mem16_lock(cpu, eaa(cpu, op));
    NEXT();
}

void OPCALL normal_inc8_mem8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_inc8_mem8_lock(cpu, eaa(cpu, op));
    NEXT();
}

void OPCALL normal_dec32_mem32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_dec32_mem32_lock(cpu, eaa(cpu, op));
    NEXT();
}

void OPCALL normal_dec16_mem16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_dec16_mem16_lock(cpu, eaa(cpu, op));
    NEXT();
}

void OPCALL normal_dec8_mem8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_dec8_mem8_lock(cpu, eaa(cpu, op));
    NEXT();
}

void OPCALL normal_note32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_note32_lock(cpu, eaa(cpu, op));
    NEXT();
}

void OPCALL normal_note16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_note16_lock(cpu, eaa(cpu, op));
    NEXT();
}

void OPCALL normal_note8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_note8_lock(cpu, eaa(cpu, op));
    NEXT();
}

void OPCALL normal_nege32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_nege32_lock(cpu, eaa(cpu, op));
    NEXT();
}

void OPCALL normal_nege16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_nege16_lock(cpu, eaa(cpu, op));
    NEXT();
}

void OPCALL normal_nege8_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_nege8_lock(cpu, eaa(cpu, op));
    NEXT();
}

#define eaa1_bit(cpu, op, offset) cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->data.disp + (offset))
#define eaa3_bit(cpu, op, offset) cpu->seg[op->base].address + cpu->reg[op->rm].u32 + (cpu->reg[op->sibIndex].u32 << + op->sibScale) + op->data.disp + (offset)
#define eaa_bit(cpu, op, offset) (op->ea16)?(eaa1_bit(cpu, op, offset)):(eaa3_bit(cpu, op, offset))

void OPCALL normal_btse32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btse32_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_btse16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btse16_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_btse32r32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);

    U32 address = eaa_bit(cpu, op, ((((S32)cpu->reg[op->reg].u32) >> 5) * 4));
    U32 mask = 1 << (cpu->reg[op->reg].u32 & 31);
    common_btse32r32_lock(cpu, address, mask);

    NEXT();
}

void OPCALL normal_btse16r16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);

    U32 address = eaa_bit(cpu, op, ((((S16)cpu->reg[op->reg].u16) >> 4) * 2));
    U16 mask = 1 << (cpu->reg[op->reg].u16 & 15);
    common_btse16r16_lock(cpu, address, mask);

    NEXT();
}

void OPCALL normal_btre32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btre32_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_btre16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btre16_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_btre32r32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);

    U32 address = eaa_bit(cpu, op, ((((S32)cpu->reg[op->reg].u32) >> 5) * 4));
    U32 mask = 1 << (cpu->reg[op->reg].u32 & 31);
    common_btre32r32_lock(cpu, address, mask);

    NEXT();
}

void OPCALL normal_btre16r16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);

    U32 address = eaa_bit(cpu, op, ((((S16)cpu->reg[op->reg].u16) >> 4) * 2));
    U16 mask = 1 << (cpu->reg[op->reg].u16 & 15);
    common_btre16r16_lock(cpu, address, mask);

    NEXT();
}

void OPCALL normal_btce32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btce32_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_btce16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btce16_lock(cpu, eaa(cpu, op), op->imm);
    NEXT();
}

void OPCALL normal_btce32r32_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);

    U32 address = eaa_bit(cpu, op, ((((S32)cpu->reg[op->reg].u32) >> 5) * 4));
    U32 mask = 1 << (cpu->reg[op->reg].u32 & 31);
    common_btce32r32_lock(cpu, address, mask);

    NEXT();
}

void OPCALL normal_btce16r16_lock(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);

    U32 address = eaa_bit(cpu, op, ((((S16)cpu->reg[op->reg].u16) >> 4) * 2));
    U16 mask = 1 << (cpu->reg[op->reg].u16 & 15);
    common_btce16r16_lock(cpu, address, mask);

    NEXT();
}

#endif