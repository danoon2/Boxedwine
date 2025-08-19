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

void common_cmpxchg8b_lock(CPU* cpu, U32 address);
void common_cmpxchge32r32_lock(CPU* cpu, U32 address, U32 srcReg);
void common_cmpxchge16r16_lock(CPU* cpu, U32 address, U32 srcReg);
void common_cmpxchge8r8_lock(CPU* cpu, U32 address, U32 srcReg);
void common_xchge32r32_lock(CPU* cpu, U32 address, U32 reg);
void common_xchge16r16_lock(CPU* cpu, U32 address, U32 reg);
void common_xchge8r8_lock(CPU* cpu, U32 address, U32 reg);
void common_xaddr32e32_lock(CPU* cpu, U32 address, U32 reg);
void common_xaddr16e16_lock(CPU* cpu, U32 address, U32 reg);
void common_xaddr8e8_lock(CPU* cpu, U32 address, U32 reg);
void common_adde32r32_lock(CPU* cpu, U32 address, U32 reg);
void common_adde16r16_lock(CPU* cpu, U32 address, U32 reg);
void common_adde8r8_lock(CPU* cpu, U32 address, U32 reg);
void common_add32_mem_lock(CPU* cpu, U32 address, U32 value);
void common_add16_mem_lock(CPU* cpu, U32 address, U32 value);
void common_add8_mem_lock(CPU* cpu, U32 address, U32 value);
void common_sube32r32_lock(CPU* cpu, U32 address, U32 reg);
void common_sube16r16_lock(CPU* cpu, U32 address, U32 reg);
void common_sube8r8_lock(CPU* cpu, U32 address, U32 reg);
void common_sub32_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_sub16_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_sub8_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_ore32r32_lock(CPU* cpu, U32 address, U32 reg);
void common_ore16r16_lock(CPU* cpu, U32 address, U32 reg);
void common_ore8r8_lock(CPU* cpu, U32 address, U32 reg);
void common_or32_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_or16_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_or8_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_ande32r32_lock(CPU* cpu, U32 address, U32 reg);
void common_ande16r16_lock(CPU* cpu, U32 address, U32 reg);
void common_ande8r8_lock(CPU* cpu, U32 address, U32 reg);
void common_and32_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_and16_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_and8_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_xore32r32_lock(CPU* cpu, U32 address, U32 reg);
void common_xore16r16_lock(CPU* cpu, U32 address, U32 reg);
void common_xore8r8_lock(CPU* cpu, U32 address, U32 reg);
void common_xor32_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_xor16_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_xor8_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_adce32r32_lock(CPU* cpu, U32 address, U32 reg);
void common_adce16r16_lock(CPU* cpu, U32 address, U32 reg);
void common_adce8r8_lock(CPU* cpu, U32 address, U32 reg);
void common_adc32_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_adc16_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_adc8_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_sbbe32r32_lock(CPU* cpu, U32 address, U32 reg);
void common_sbbe16r16_lock(CPU* cpu, U32 address, U32 reg);
void common_sbbe8r8_lock(CPU* cpu, U32 address, U32 reg);
void common_sbb32_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_sbb16_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_sbb8_mem_lock(CPU* cpu, U32 address, U32 imm);
void common_inc32_mem32_lock(CPU* cpu, U32 address);
void common_inc16_mem16_lock(CPU* cpu, U32 address);
void common_inc8_mem8_lock(CPU* cpu, U32 address);
void common_dec32_mem32_lock(CPU* cpu, U32 address);
void common_dec16_mem16_lock(CPU* cpu, U32 address);
void common_dec8_mem8_lock(CPU* cpu, U32 address);
void common_note32_lock(CPU* cpu, U32 address);
void common_note16_lock(CPU* cpu, U32 address);
void common_note8_lock(CPU* cpu, U32 address);
void common_nege32_lock(CPU* cpu, U32 address);
void common_nege16_lock(CPU* cpu, U32 address);
void common_nege8_lock(CPU* cpu, U32 address);
void common_btse32_lock(CPU* cpu, U32 address, U32 mask);
void common_btse16_lock(CPU* cpu, U32 address, U16 mask);
void common_btse32r32_lock(CPU* cpu, U32 address, U32 mask);
void common_btse16r16_lock(CPU* cpu, U32 address, U16 mask);
void common_btre32_lock(CPU* cpu, U32 address, U32 mask);
void common_btre16_lock(CPU* cpu, U32 address, U16 mask);
void common_btre32r32_lock(CPU* cpu, U32 address, U32 mask);
void common_btre16r16_lock(CPU* cpu, U32 address, U16 mask);
void common_btce32_lock(CPU* cpu, U32 address, U32 mask);
void common_btce16_lock(CPU* cpu, U32 address, U16 mask);
void common_btce32r32_lock(CPU* cpu, U32 address, U32 mask);
void common_btce16r16_lock(CPU* cpu, U32 address, U16 mask);
