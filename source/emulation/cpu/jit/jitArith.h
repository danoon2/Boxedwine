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

void Jit::dynamic_addr8r8(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b8, &Jit::addReg, FLAGS_ADD8);
}
void Jit::dynamic_adde8r8(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b8, &Jit::addReg, FLAGS_ADD8);
}
void Jit::dynamic_addr8e8(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b8, &Jit::addReg, FLAGS_ADD8);
}
void Jit::dynamic_add8_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b8, &Jit::addValue, FLAGS_ADD8);
}
void Jit::dynamic_add8_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b8, &Jit::addValue, FLAGS_ADD8);
}
void Jit::dynamic_addr16r16(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b16, &Jit::addReg, FLAGS_ADD16);
}
void Jit::dynamic_adde16r16(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b16, &Jit::addReg, FLAGS_ADD16);
}
void Jit::dynamic_addr16e16(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b16, &Jit::addReg, FLAGS_ADD16);
}
void Jit::dynamic_add16_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b16, &Jit::addValue, FLAGS_ADD16);
}
void Jit::dynamic_add16_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b16, &Jit::addValue, FLAGS_ADD16);
}
void Jit::dynamic_addr32r32(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b32, &Jit::addReg, FLAGS_ADD32);
}
void Jit::dynamic_adde32r32(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b32, &Jit::addReg, FLAGS_ADD32);
}
void Jit::dynamic_addr32e32(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b32, &Jit::addReg, FLAGS_ADD32);
}
void Jit::dynamic_add32_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b32, &Jit::addValue, FLAGS_ADD32);
}
void Jit::dynamic_add32_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b32, &Jit::addValue, FLAGS_ADD32);
}
void Jit::dynamic_orr8r8(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b8, &Jit::orReg, FLAGS_OR8);
}
void Jit::dynamic_ore8r8(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b8, &Jit::orReg, FLAGS_OR8);
}
void Jit::dynamic_orr8e8(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b8, &Jit::orReg, FLAGS_OR8);
}
void Jit::dynamic_or8_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b8, &Jit::orValue, FLAGS_OR8);
}
void Jit::dynamic_or8_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b8, &Jit::orValue, FLAGS_OR8);
}
void Jit::dynamic_orr16r16(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b16, &Jit::orReg, FLAGS_OR16);
}
void Jit::dynamic_ore16r16(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b16, &Jit::orReg, FLAGS_OR16);
}
void Jit::dynamic_orr16e16(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b16, &Jit::orReg, FLAGS_OR16);
}
void Jit::dynamic_or16_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b16, &Jit::orValue, FLAGS_OR16);
}
void Jit::dynamic_or16_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b16, &Jit::orValue, FLAGS_OR16);
}
void Jit::dynamic_orr32r32(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b32, &Jit::orReg, FLAGS_OR32);
}
void Jit::dynamic_ore32r32(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b32, &Jit::orReg, FLAGS_OR32);
}
void Jit::dynamic_orr32e32(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b32, &Jit::orReg, FLAGS_OR32);
}
void Jit::dynamic_or32_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b32, &Jit::orValue, FLAGS_OR32);
}
void Jit::dynamic_or32_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b32, &Jit::orValue, FLAGS_OR32);
}
void Jit::dynamic_adcr8r8(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b8, &Jit::addReg, FLAGS_ADC8, true, true);
}
void Jit::dynamic_adce8r8(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b8, &Jit::addReg, FLAGS_ADC8, true, true);
}
void Jit::dynamic_adcr8e8(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b8, &Jit::addReg, FLAGS_ADC8, true, true);
}
void Jit::dynamic_adc8_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b8, &Jit::addValue, FLAGS_ADC8, true, true, &Jit::addReg);
}
void Jit::dynamic_adc8_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b8, &Jit::addValue, FLAGS_ADC8, true, true, &Jit::addReg);
}
void Jit::dynamic_adcr16r16(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b16, &Jit::addReg, FLAGS_ADC16, true, true);
}
void Jit::dynamic_adce16r16(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b16, &Jit::addReg, FLAGS_ADC16, true, true);
}
void Jit::dynamic_adcr16e16(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b16, &Jit::addReg, FLAGS_ADC16, true, true);
}
void Jit::dynamic_adc16_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b16, &Jit::addValue, FLAGS_ADC16, true, true, &Jit::addReg);
}
void Jit::dynamic_adc16_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b16, &Jit::addValue, FLAGS_ADC16, true, true, &Jit::addReg);
}
void Jit::dynamic_adcr32r32(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b32, &Jit::addReg, FLAGS_ADC32, true, true);
}
void Jit::dynamic_adce32r32(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b32, &Jit::addReg, FLAGS_ADC32, true, true);
}
void Jit::dynamic_adcr32e32(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b32, &Jit::addReg, FLAGS_ADC32, true, true);
}
void Jit::dynamic_adc32_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b32, &Jit::addValue, FLAGS_ADC32, true, true, &Jit::addReg);
}
void Jit::dynamic_adc32_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b32, &Jit::addValue, FLAGS_ADC32, true, true, &Jit::addReg);
}
void Jit::dynamic_sbbr8r8(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b8, &Jit::subReg, FLAGS_SBB8, true, true);
}
void Jit::dynamic_sbbe8r8(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b8, &Jit::subReg, FLAGS_SBB8, true, true);
}
void Jit::dynamic_sbbr8e8(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b8, &Jit::subReg, FLAGS_SBB8, true, true);
}
void Jit::dynamic_sbb8_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b8, &Jit::subValue, FLAGS_SBB8, true, true, &Jit::subReg);
}
void Jit::dynamic_sbb8_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b8, &Jit::subValue, FLAGS_SBB8, true, true, &Jit::subReg);
}
void Jit::dynamic_sbbr16r16(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b16, &Jit::subReg, FLAGS_SBB16, true, true);
}
void Jit::dynamic_sbbe16r16(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b16, &Jit::subReg, FLAGS_SBB16, true, true);
}
void Jit::dynamic_sbbr16e16(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b16, &Jit::subReg, FLAGS_SBB16, true, true);
}
void Jit::dynamic_sbb16_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b16, &Jit::subValue, FLAGS_SBB16, true, true, &Jit::subReg);
}
void Jit::dynamic_sbb16_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b16, &Jit::subValue, FLAGS_SBB16, true, true, &Jit::subReg);
}
void Jit::dynamic_sbbr32r32(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b32, &Jit::subReg, FLAGS_SBB32, true, true);
}
void Jit::dynamic_sbbe32r32(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b32, &Jit::subReg, FLAGS_SBB32, true, true);
}
void Jit::dynamic_sbbr32e32(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b32, &Jit::subReg, FLAGS_SBB32, true, true);
}
void Jit::dynamic_sbb32_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b32, &Jit::subValue, FLAGS_SBB32, true, true, &Jit::subReg);
}
void Jit::dynamic_sbb32_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b32, &Jit::subValue, FLAGS_SBB32, true, true, &Jit::subReg);
}
void Jit::dynamic_andr8r8(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b8, &Jit::andReg, FLAGS_AND8);
}
void Jit::dynamic_ande8r8(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b8, &Jit::andReg, FLAGS_AND8);
}
void Jit::dynamic_andr8e8(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b8, &Jit::andReg, FLAGS_AND8);
}
void Jit::dynamic_and8_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b8, &Jit::andValue, FLAGS_AND8);
}
void Jit::dynamic_and8_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b8, &Jit::andValue, FLAGS_AND8);
}
void Jit::dynamic_andr16r16(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b16, &Jit::andReg, FLAGS_AND16);
}
void Jit::dynamic_ande16r16(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b16, &Jit::andReg, FLAGS_AND16);
}
void Jit::dynamic_andr16e16(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b16, &Jit::andReg, FLAGS_AND16);
}
void Jit::dynamic_and16_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b16, &Jit::andValue, FLAGS_AND16);
}
void Jit::dynamic_and16_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b16, &Jit::andValue, FLAGS_AND16);
}
void Jit::dynamic_andr32r32(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b32, &Jit::andReg, FLAGS_AND32);
}
void Jit::dynamic_ande32r32(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b32, &Jit::andReg, FLAGS_AND32);
}
void Jit::dynamic_andr32e32(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b32, &Jit::andReg, FLAGS_AND32);
}
void Jit::dynamic_and32_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b32, &Jit::andValue, FLAGS_AND32);
}
void Jit::dynamic_and32_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b32, &Jit::andValue, FLAGS_AND32);
}
void Jit::dynamic_subr8r8(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b8, &Jit::subReg, FLAGS_SUB8);
}
void Jit::dynamic_sube8r8(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b8, &Jit::subReg, FLAGS_SUB8);
}
void Jit::dynamic_subr8e8(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b8, &Jit::subReg, FLAGS_SUB8);
}
void Jit::dynamic_sub8_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b8, &Jit::subValue, FLAGS_SUB8);
}
void Jit::dynamic_sub8_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b8, &Jit::subValue, FLAGS_SUB8);
}
void Jit::dynamic_subr16r16(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b16, &Jit::subReg, FLAGS_SUB16);
}
void Jit::dynamic_sube16r16(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b16, &Jit::subReg, FLAGS_SUB16);
}
void Jit::dynamic_subr16e16(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b16, &Jit::subReg, FLAGS_SUB16);
}
void Jit::dynamic_sub16_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b16, &Jit::subValue, FLAGS_SUB16);
}
void Jit::dynamic_sub16_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b16, &Jit::subValue, FLAGS_SUB16);
}
void Jit::dynamic_subr32r32(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b32, &Jit::subReg, FLAGS_SUB32);
}
void Jit::dynamic_sube32r32(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b32, &Jit::subReg, FLAGS_SUB32);
}
void Jit::dynamic_subr32e32(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b32, &Jit::subReg, FLAGS_SUB32);
}
void Jit::dynamic_sub32_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b32, &Jit::subValue, FLAGS_SUB32);
}
void Jit::dynamic_sub32_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b32, &Jit::subValue, FLAGS_SUB32);
}
void Jit::dynamic_xorr8r8(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b8, &Jit::xorReg, FLAGS_XOR8);
}
void Jit::dynamic_xore8r8(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b8, &Jit::xorReg, FLAGS_XOR8);
}
void Jit::dynamic_xorr8e8(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b8, &Jit::xorReg, FLAGS_XOR8);
}
void Jit::dynamic_xor8_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b8, &Jit::xorValue, FLAGS_XOR8);
}
void Jit::dynamic_xor8_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b8, &Jit::xorValue, FLAGS_XOR8);
}
void Jit::dynamic_xorr16r16(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b16, &Jit::xorReg, FLAGS_XOR16);
}
void Jit::dynamic_xore16r16(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b16, &Jit::xorReg, FLAGS_XOR16);
}
void Jit::dynamic_xorr16e16(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b16, &Jit::xorReg, FLAGS_XOR16);
}
void Jit::dynamic_xor16_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b16, &Jit::xorValue, FLAGS_XOR16);
}
void Jit::dynamic_xor16_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b16, &Jit::xorValue, FLAGS_XOR16);
}
void Jit::dynamic_xorr32r32(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b32, &Jit::xorReg, FLAGS_XOR32);
}
void Jit::dynamic_xore32r32(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b32, &Jit::xorReg, FLAGS_XOR32);
}
void Jit::dynamic_xorr32e32(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b32, &Jit::xorReg, FLAGS_XOR32);
}
void Jit::dynamic_xor32_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b32, &Jit::xorValue, FLAGS_XOR32);
}
void Jit::dynamic_xor32_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b32, &Jit::xorValue, FLAGS_XOR32);
}
void Jit::dynamic_cmpr8r8(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b8, &Jit::subReg, FLAGS_SUB8, false);
}
void Jit::dynamic_cmpe8r8(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b8, &Jit::subReg, FLAGS_SUB8, false);
}
void Jit::dynamic_cmpr8e8(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b8, &Jit::subReg, FLAGS_SUB8, false);
}
void Jit::dynamic_cmp8_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b8, &Jit::subValue, FLAGS_SUB8, false);
}
void Jit::dynamic_cmp8_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b8, &Jit::subValue, FLAGS_SUB8, false);
}
void Jit::dynamic_cmpr16r16(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b16, &Jit::subReg, FLAGS_SUB16, false);
}
void Jit::dynamic_cmpe16r16(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b16, &Jit::subReg, FLAGS_SUB16, false);
}
void Jit::dynamic_cmpr16e16(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b16, &Jit::subReg, FLAGS_SUB16, false);
}
void Jit::dynamic_cmp16_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b16, &Jit::subValue, FLAGS_SUB16, false);
}
void Jit::dynamic_cmp16_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b16, &Jit::subValue, FLAGS_SUB16, false);
}
void Jit::dynamic_cmpr32r32(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b32, &Jit::subReg, FLAGS_SUB32, false);
}
void Jit::dynamic_cmpe32r32(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b32, &Jit::subReg, FLAGS_SUB32, false);
}
void Jit::dynamic_cmpr32e32(DecodedOp* op) {
    dynamic_RM(op, JitWidth::b32, &Jit::subReg, FLAGS_SUB32, false);
}
void Jit::dynamic_cmp32_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b32, &Jit::subValue, FLAGS_SUB32, false);
}
void Jit::dynamic_cmp32_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b32, &Jit::subValue, FLAGS_SUB32, false);
}
void Jit::dynamic_testr8r8(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b8, &Jit::andReg, FLAGS_AND8, false);
}
void Jit::dynamic_teste8r8(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b8, &Jit::andReg, FLAGS_AND8, false);
}
void Jit::dynamic_test8_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b8, &Jit::andValue, FLAGS_AND8, false);
}
void Jit::dynamic_test8_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b8, &Jit::andValue, FLAGS_AND8, false);
}
void Jit::dynamic_testr16r16(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b16, &Jit::andReg, FLAGS_AND16, false);
}
void Jit::dynamic_teste16r16(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b16, &Jit::andReg, FLAGS_AND16, false);
}
void Jit::dynamic_test16_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b16, &Jit::andValue, FLAGS_AND16, false);
}
void Jit::dynamic_test16_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b16, &Jit::andValue, FLAGS_AND16, false);
}
void Jit::dynamic_testr32r32(DecodedOp* op) {
    dynamic_RR(op, JitWidth::b32, &Jit::andReg, FLAGS_AND32, false);
}
void Jit::dynamic_teste32r32(DecodedOp* op) {
    dynamic_MR(op, JitWidth::b32, &Jit::andReg, FLAGS_AND32, false);
}
void Jit::dynamic_test32_reg(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b32, &Jit::andValue, FLAGS_AND32, false);
}
void Jit::dynamic_test32_mem(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b32, &Jit::andValue, FLAGS_AND32, false);
}
void Jit::dynamic_notr8(DecodedOp* op) {
    dynamic_R(op, JitWidth::b8, &Jit::notReg2, nullptr);
}
void Jit::dynamic_note8(DecodedOp* op) {
    dynamic_M(op, JitWidth::b8, &Jit::notReg2, nullptr);
}
void Jit::dynamic_notr16(DecodedOp* op) {
    dynamic_R(op, JitWidth::b16, &Jit::notReg2, nullptr);
}
void Jit::dynamic_note16(DecodedOp* op) {
    dynamic_M(op, JitWidth::b16, &Jit::notReg2, nullptr);
}
void Jit::dynamic_notr32(DecodedOp* op) {
    dynamic_R(op, JitWidth::b32, &Jit::notReg2, nullptr);
}
void Jit::dynamic_note32(DecodedOp* op) {
    dynamic_M(op, JitWidth::b32, &Jit::notReg2, nullptr);
}
void Jit::dynamic_negr8(DecodedOp* op) {
    dynamic_R(op, JitWidth::b8, &Jit::negReg2, FLAGS_NEG8);
}
void Jit::dynamic_nege8(DecodedOp* op) {
    dynamic_M(op, JitWidth::b8, &Jit::negReg2, FLAGS_NEG8);
}
void Jit::dynamic_negr16(DecodedOp* op) {
    dynamic_R(op, JitWidth::b16, &Jit::negReg2, FLAGS_NEG16);
}
void Jit::dynamic_nege16(DecodedOp* op) {
    dynamic_M(op, JitWidth::b16, &Jit::negReg2, FLAGS_NEG16);
}
void Jit::dynamic_negr32(DecodedOp* op) {
    dynamic_R(op, JitWidth::b32, &Jit::negReg2, FLAGS_NEG32);
}
void Jit::dynamic_nege32(DecodedOp* op) {
    dynamic_M(op, JitWidth::b32, &Jit::negReg2, FLAGS_NEG32);
}
// mul: The OF and CF flags are set to 0 if the upper half of the result is 0; otherwise, they are set to 1. The SF, ZF, AF, and PF flags are undefined.
// imul: For the one operand form of the instruction, the CF and OF flags are set when significant bits are carried into the upper half of the result and cleared when the result fits exactly in the lower half of the result.For the two - and three - operand forms of the instruction, the CF and OF flags are set when the result must be truncated to fit in the destination operand size and cleared when the result fits exactly in the destination operand size.The SF, ZF, AF, and PF flags are undefined.
void Jit::dynamic_mulR8(DecodedOp* op) {
    // AX = AL * src;
    dynamic_R(op, JitWidth::b8, &Jit::mulReg, nullptr, false);
    if (op->needsToSetFlags(cpu)) {
        (FLAGS_NONE);
        If(JitWidth::b8, getReadOnlyReg8(4)); {
            orCPUFlagsImmV2(CF | OF);
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
}
void Jit::dynamic_mulE8(DecodedOp* op) {
    // getTmpReg will register the reg we want the read from memory to use, by default it will be EAX, but mul needs to this to be unused
    dynamic_M(op, JitWidth::b8, &Jit::mulReg, nullptr, false, getTmpReg8());
    if (op->needsToSetFlags(cpu)) {
        storeLazyFlags(FLAGS_NONE);
        If(JitWidth::b8, getReadOnlyReg8(4)); {
            orCPUFlagsImmV2(CF | OF);
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
}
void Jit::dynamic_imulR8(DecodedOp* op) {
    dynamic_R(op, JitWidth::b8, &Jit::imulReg, nullptr, false);
    if (op->needsToSetFlags(cpu)) {
        storeLazyFlags(FLAGS_NONE);
        RegPtr ah = getReadOnlyReg8(4);
        If(JitWidth::b8, ah); {
            IfEqual(JitWidth::b8, ah, 0xff); {
                andCPUFlagsImmV2(~(CF | OF));
            } StartElse(); {
                orCPUFlagsImmV2(CF | OF);
            } EndIf();
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
}
void Jit::dynamic_imulE8(DecodedOp* op) {
    dynamic_M(op, JitWidth::b8, &Jit::imulReg, nullptr, false, getTmpReg8());
    if (op->needsToSetFlags(cpu)) {
        storeLazyFlags(FLAGS_NONE);
        RegPtr ah = getReadOnlyReg8(4);
        If(JitWidth::b8, ah); {
            IfEqual(JitWidth::b8, ah, 0xff); {
                andCPUFlagsImmV2(~(CF | OF));
            } StartElse(); {
                orCPUFlagsImmV2(CF | OF);
            } EndIf();
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
}
void Jit::dynamic_mulR16(DecodedOp* op) {
    dynamic_R(op, JitWidth::b16, &Jit::mulReg, nullptr, false);
    if (op->needsToSetFlags(cpu)) {
        storeLazyFlags(FLAGS_NONE);
        If(JitWidth::b16, getReadOnlyReg(2)); {
            orCPUFlagsImmV2(CF | OF);
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
}
void Jit::dynamic_mulE16(DecodedOp* op) {
    dynamic_M(op, JitWidth::b16, &Jit::mulReg, nullptr, false, getTmpReg());
    if (op->needsToSetFlags(cpu)) {
        storeLazyFlags(FLAGS_NONE);
        If(JitWidth::b16, getReadOnlyReg(2)); {
            orCPUFlagsImmV2(CF | OF);
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
}
void Jit::dynamic_imulR16(DecodedOp* op) {
    dynamic_R(op, JitWidth::b16, &Jit::imulReg, nullptr, false);
    if (op->needsToSetFlags(cpu)) {
        storeLazyFlags(FLAGS_NONE);
        RegPtr dx = getReadOnlyReg(2);
        If(JitWidth::b16, dx); {
            IfEqual(JitWidth::b16, dx, 0xffff); {
                andCPUFlagsImmV2(~(CF | OF));
            } StartElse(); {
                orCPUFlagsImmV2(CF | OF);
            } EndIf();
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
}
void Jit::dynamic_imulE16(DecodedOp* op) {
    dynamic_M(op, JitWidth::b16, &Jit::imulReg, nullptr, false, getTmpReg());
    if (op->needsToSetFlags(cpu)) {
        storeLazyFlags(FLAGS_NONE);
        RegPtr dx = getReadOnlyReg(2);
        If(JitWidth::b16, dx); {
            IfEqual(JitWidth::b16, dx, 0xffff); {
                andCPUFlagsImmV2(~(CF | OF));
            } StartElse(); {
                orCPUFlagsImmV2(CF | OF);
            } EndIf();
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
}
void Jit::dynamic_mulR32(DecodedOp* op) {
    dynamic_R(op, JitWidth::b32, &Jit::mulReg, nullptr, false);
    if (op->needsToSetFlags(cpu)) {
        storeLazyFlags(FLAGS_NONE);
        If(JitWidth::b32, getReadOnlyReg(2)); {
            orCPUFlagsImmV2(CF | OF);
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
}
void Jit::dynamic_mulE32(DecodedOp* op) {
    dynamic_M(op, JitWidth::b32, &Jit::mulReg, nullptr, false, getTmpReg());
    if (op->needsToSetFlags(cpu)) {
        storeLazyFlags(FLAGS_NONE);
        If(JitWidth::b32, getReadOnlyReg(2)); {
            orCPUFlagsImmV2(CF | OF);
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
}
void Jit::dynamic_imulR32(DecodedOp* op) {
    dynamic_R(op, JitWidth::b32, &Jit::imulReg, nullptr, false);
    if (op->needsToSetFlags(cpu)) {
        storeLazyFlags(FLAGS_NONE);
        RegPtr edx = getReadOnlyReg(2);
        If(JitWidth::b32, edx); {
            IfEqual(JitWidth::b32, edx, 0xffffffff); {
                andCPUFlagsImmV2(~(CF | OF));
            } StartElse(); {
                orCPUFlagsImmV2(CF | OF);
            } EndIf();
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
}
void Jit::dynamic_imulE32(DecodedOp* op) {
    dynamic_M(op, JitWidth::b32, &Jit::imulReg, nullptr, false, getTmpReg());
    if (op->needsToSetFlags(cpu)) {
        storeLazyFlags(FLAGS_NONE);
        RegPtr edx = getReadOnlyReg(2);
        If(JitWidth::b32, edx); {
            IfEqual(JitWidth::b32, edx, 0xffffffff); {
                andCPUFlagsImmV2(~(CF | OF));
            } StartElse(); {
                orCPUFlagsImmV2(CF | OF);
            } EndIf();
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
}

static void dynamic_prepareException(CPU* cpu, U32 code, U32 error) {
    common_prepareException(cpu, code, error);
    cpu->nextOp = cpu->getNextOp();
}

void Jit::div8(DecodedOp* op, RegPtr src, bool isSigned, InstDiv callback) {
    /*
    if (src == 0) {
        cpu->prepareException(EXCEPTION_DIVIDE, 0);
        return 0;
    }

    U16 quo = AX / src;
    U8 rem = AX % src;

    if (quo > 255) {
        cpu->prepareException(EXCEPTION_DIVIDE, 1);
        return 0;
    }
    AL = (U8)quo;
    AH = rem;
    return 1;
    */
    IfNot(JitWidth::b8, src);
        call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 0);
    blockExit();
    EndIf();

    // this code is fragile, the order of creating the tmps here matters for the x86 JIT implementation
    RegPtr eax = getTmpRegForCallResult();
    mov(JitWidth::b16, eax, getReadOnlyReg(0));
    RegPtr remainder = getTmpRegWithHint(2);
    RegPtr tmp = getTmpReg();

    // do 16-bit div instead of 8-bit so that we can detect the overflow and generate an exception
    (this->*callback)(JitWidth::b16, eax, src, remainder);

    xorReg(JitWidth::b32, tmp, tmp);
    mov8(tmp, false, eax, true);

    if (isSigned) {
        If(JitWidth::b32, tmp);
            subValue(JitWidth::b32, tmp, 0xff);
            If(JitWidth::b32, tmp);
                call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 1);
                blockExit();
            EndIf();
        EndIf();
    } else {
        If(JitWidth::b32, tmp);
            call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 1);
            blockExit();
        EndIf();
    }
    mov8(eax, true, remainder, false);
    mov(JitWidth::b16, getReg(0), eax);
    incrementEip(op->len);
}

void Jit::div16(DecodedOp* op, RegPtr src, bool isSigned, InstDiv callback) {
    /*
    U32 num = ((U32)DX << 16) | AX;

    if (src==0) {
        cpu->prepareException(EXCEPTION_DIVIDE, 0);
        return 0;
    }
    U32 quo=num/src;
    U16 rem=(U16)(num % src);
    U16 quo16=(U16)quo;
    if (quo!=(U32)quo16) {
        cpu->prepareException(EXCEPTION_DIVIDE, 1);
        return 0;
    }
    DX=rem;
    AX=quo16;
    return 1;
    */
    IfNot(JitWidth::b16, src);
        call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 0);
        blockExit();
    EndIf();

    RegPtr eax = getTmpRegForCallResult();
    xorReg(JitWidth::b32, eax, eax);
    mov(JitWidth::b16, eax, getReadOnlyReg(0));
    RegPtr remainder = getTmpReg(2, false, 2);
    RegPtr tmp = getTmpReg();

    // combine DX:AX into EAX for 32-bit div
    shlValue(JitWidth::b32, remainder, 16);
    orReg(JitWidth::b32, eax, remainder);

    // do 32-bit div instead of 16-bit so that we can detect the overflow and generate an exception
    (this->*callback)(JitWidth::b32, eax, src, remainder);

    mov(JitWidth::b32, tmp, eax);
    shrValue(JitWidth::b32, tmp, 16);

    if (isSigned) {
        If(JitWidth::b32, tmp);
            subValue(JitWidth::b32, tmp, 0xffff);
            If(JitWidth::b32, tmp);
                call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 1);
                blockExit();
            EndIf();
        EndIf();
    } else {
        If(JitWidth::b32, tmp);
            call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 1);
            blockExit();
        EndIf();
    }
    tmp = nullptr;
    mov(JitWidth::b16, getReg(0), eax);
    mov(JitWidth::b16, getReg(2), remainder);
    incrementEip(op->len);
}

void Jit::div32(DecodedOp* op, RegPtr src, InstDiv callback, std::function<void()> fallback) {
    /*
    U64 num = ((U64)EDX << 32) | EAX;

    if (src==0)	{
        cpu->prepareException(EXCEPTION_DIVIDE, 0);
        return 0;
    }

    U64 quo=num/src;
    U32 rem=(U32)(num % src);
    U32 quo32=(U32)quo;
    if (quo!=(U64)quo32) {
        cpu->prepareException(EXCEPTION_DIVIDE, 1);
        return 0;
    }
    EDX=rem;
    EAX=quo32;
    return 1;
    */
    IfNot(JitWidth::b32, src);
        call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 0);
        blockExit();
    EndIf();

    RegPtr remainder = getReadOnlyReg(2, false, 2); // read only because in the fallback scenario, we don't want to write it back
    // only take the inline path if we are guaranteed it won't overflow
    If(JitWidth::b32, remainder);
        fallback();
    StartElse();
#ifdef BOXEDWINE_DYNAMIC32
        RegPtr eax = getTmpRegForCallResult();
        mov(JitWidth::b32, eax, getReadOnlyReg(0));
        (this->*callback)(JitWidth::b32, eax, src, remainder);
        mov(JitWidth::b32, getReg(0, -1, false), eax);
        mov(JitWidth::b32, getReg(2, -1, false), remainder);
#else
        RegPtr eax = getReg(0);
        (this->*callback)(JitWidth::b32, eax, src, remainder);
#endif
    EndIf();
    incrementEip(op->len);
}

void Jit::dynamic_divR8(DecodedOp* op) {
    RegPtr src = getTmpReg8(op->reg);
    movzx(JitWidth::b32, src, JitWidth::b8, src);
    div8(op, src, false, &Jit::divRegRegWithRemainder);
}
void Jit::dynamic_divE8(DecodedOp* op) {
    // getTmpReg to help x86 JIT, so that its tmp EAX hardware reg is still available for the div
    RegPtr src = read(JitWidth::b8, calculateEaa(op), nullptr, nullptr, false, getTmpReg8());
    movzx(JitWidth::b32, src, JitWidth::b8, src);
    div8(op, src, false, &Jit::divRegRegWithRemainder);
}
void Jit::dynamic_idivR8(DecodedOp* op) {
    RegPtr src = getTmpReg8(op->reg);
    movsx(JitWidth::b32, src, JitWidth::b8, src);
    div8(op, src, true, &Jit::idivRegRegWithRemainder);
}
void Jit::dynamic_idivE8(DecodedOp* op) {
    RegPtr src = read(JitWidth::b8, calculateEaa(op), nullptr, nullptr, false, getTmpReg8());
    movsx(JitWidth::b32, src, JitWidth::b8, src);
    div8(op, src, true, &Jit::idivRegRegWithRemainder);
}
void Jit::dynamic_divR16(DecodedOp* op) {
    RegPtr src = getTmpReg(op->reg);
    movzx(JitWidth::b32, src, JitWidth::b16, src);
    div16(op, src, false, &Jit::divRegRegWithRemainder);
}
void Jit::dynamic_divE16(DecodedOp* op) {
    // getTmpReg to help x86 JIT, so that its tmp EAX hardware reg is still available for the div
    RegPtr src = read(JitWidth::b16, calculateEaa(op), nullptr, nullptr, false, getTmpReg());
    movzx(JitWidth::b32, src, JitWidth::b16, src);
    div16(op, src, false, &Jit::divRegRegWithRemainder);
}
void Jit::dynamic_idivR16(DecodedOp* op) {
    RegPtr src = getTmpReg(op->reg);
    movsx(JitWidth::b32, src, JitWidth::b16, src);
    div16(op, src, true, &Jit::idivRegRegWithRemainder);
}
void Jit::dynamic_idivE16(DecodedOp* op) {
    RegPtr src = read(JitWidth::b16, calculateEaa(op), nullptr, nullptr, false, getTmpReg());
    movsx(JitWidth::b32, src, JitWidth::b16, src);
    div16(op, src, true, &Jit::idivRegRegWithRemainder);
}
void Jit::dynamic_divR32(DecodedOp* op) {
    div32(op, getReadOnlyReg(op->reg), &Jit::divRegRegWithRemainder, [op, this]() {
        RegPtr result = callAndReturn_R(::div32, JitWidth::b32, getReadOnlyReg(op->reg));
        IfNot(JitWidth::b32, result);
        blockDone(true);
        EndIf();
    });
}
void Jit::dynamic_divE32(DecodedOp* op) {
    RegPtr src = read(JitWidth::b32, calculateEaa(op), nullptr, nullptr, false, getTmpReg());
    div32(op, src, &Jit::divRegRegWithRemainder, [op, src, this]() {
        RegPtr result = callAndReturn_R(::div32, JitWidth::b32, src);
        IfNot(JitWidth::b32, result);
        blockDone(true);
        EndIf();
    });
}
void Jit::dynamic_idivR32(DecodedOp* op) {
    div32(op, getReadOnlyReg(op->reg), &Jit::idivRegRegWithRemainder, [op, this]() {
        RegPtr result = callAndReturn_RS(::idiv32, JitWidth::b32, getReadOnlyReg(op->reg));
        IfNot(JitWidth::b32, result);
        blockDone(true);
        EndIf();
    });
}
void Jit::dynamic_idivE32(DecodedOp* op) {
    RegPtr src = read(JitWidth::b32, calculateEaa(op), nullptr, nullptr, false, getTmpReg());
    div32(op, src, &Jit::idivRegRegWithRemainder, [op, src, this]() {
        RegPtr result = callAndReturn_RS(::idiv32, JitWidth::b32, src);
        IfNot(JitWidth::b32, result);
        blockDone(true);
        EndIf();
    });
}
void Jit::dynamic_dimulcr16r16(DecodedOp* op) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);
    if (!needsToSetFlags) {
        imulRRI(JitWidth::b16, getReg(op->reg), getReadOnlyReg(op->rm), op->imm);
    } else {
        storeLazyFlags(FLAGS_NONE);
        RegPtr src1 = getTmpReg(op->rm);
        movsx(JitWidth::b32, src1, JitWidth::b16, src1);
        RegPtr result = getTmpReg();
        imulRRI(JitWidth::b32, result, src1, (S32)((S16)(op->imm)));
        mov(JitWidth::b16, getReg(op->reg), result);
        shrValue(JitWidth::b32, result, 16);
        If(JitWidth::b32, result); {
            IfEqual(JitWidth::b32, result, 0xffff); {
                andCPUFlagsImmV2(~(CF | OF));
            } StartElse(); {
                orCPUFlagsImmV2(CF | OF);
            } EndIf();
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void Jit::dynamic_dimulcr16e16(DecodedOp* op) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);
    if (!needsToSetFlags) {
        imulRRI(JitWidth::b16, getReg(op->reg), read(JitWidth::b16, calculateEaa(op)), op->imm);
    } else {
        RegPtr src1 = read(JitWidth::b16, calculateEaa(op));
        storeLazyFlags(FLAGS_NONE); // after read in case of exception
        movsx(JitWidth::b32, src1, JitWidth::b16, src1);
        RegPtr result = getTmpReg();
        imulRRI(JitWidth::b32, result, src1, (S32)((S16)(op->imm)));
        mov(JitWidth::b16, getReg(op->reg), result);
        shrValue(JitWidth::b32, result, 16);
        If(JitWidth::b32, result); {
            IfEqual(JitWidth::b32, result, 0xffff); {
                andCPUFlagsImmV2(~(CF | OF));
            } StartElse(); {
                orCPUFlagsImmV2(CF | OF);
            } EndIf();
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void Jit::dynamic_dimulcr32r32(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        imulRRI(JitWidth::b32, getReg(op->reg), getReadOnlyReg(op->rm), op->imm);
    } else {
        call_RII(common_dimul32, JitWidth::b32, getReadOnlyReg(op->rm), op->imm, op->reg);
    }
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void Jit::dynamic_dimulcr32e32(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        imulRRI(JitWidth::b32, getReg(op->reg), read(JitWidth::b32, calculateEaa(op)), op->imm);
    } else {
        call_RII(common_dimul32, JitWidth::b32, read(JitWidth::b32, calculateEaa(op)), op->imm, op->reg);
    }
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void Jit::dynamic_dimulr16r16(DecodedOp* op) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);
    if (!needsToSetFlags) {
        imulRR(JitWidth::b16, getReg(op->reg), getReadOnlyReg(op->rm));
    } else {
        storeLazyFlags(FLAGS_NONE);
        RegPtr src = getTmpReg(op->rm);
        movsx(JitWidth::b32, src, JitWidth::b16, src);
        RegPtr result = getTmpReg(op->reg);
        movsx(JitWidth::b32, result, JitWidth::b16, result);
        imulRR(JitWidth::b32, result, src);
        mov(JitWidth::b16, getReg(op->reg), result);
        shrValue(JitWidth::b32, result, 16);
        If(JitWidth::b32, result); {
            IfEqual(JitWidth::b32, result, 0xffff); {
                andCPUFlagsImmV2(~(CF | OF));
            } StartElse(); {
                orCPUFlagsImmV2(CF | OF);
            } EndIf();
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void Jit::dynamic_dimulr16e16(DecodedOp* op) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);
    if (!needsToSetFlags) {
        imulRR(JitWidth::b16, getReg(op->reg), read(JitWidth::b16, calculateEaa(op)));
    } else {
        RegPtr src = read(JitWidth::b16, calculateEaa(op));
        storeLazyFlags(FLAGS_NONE); // after read in case of exception
        movsx(JitWidth::b32, src, JitWidth::b16, src);
        RegPtr result = getTmpReg(op->reg);
        movsx(JitWidth::b32, result, JitWidth::b16, result);
        imulRR(JitWidth::b32, result, src);
        mov(JitWidth::b16, getReg(op->reg), result);
        shrValue(JitWidth::b32, result, 16);
        If(JitWidth::b32, result); {
            IfEqual(JitWidth::b32, result, 0xffff); {
                andCPUFlagsImmV2(~(CF | OF));
            } StartElse(); {
                orCPUFlagsImmV2(CF | OF);
            } EndIf();
        } StartElse(); {
            andCPUFlagsImmV2(~(CF | OF));
        } EndIf();
    }
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void Jit::dynamic_dimulr32r32(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        imulRR(JitWidth::b32, getReg(op->reg), getReadOnlyReg(op->rm));
    } else {
        call_RRI(common_dimul32, JitWidth::b32, getReadOnlyReg(op->rm), JitWidth::b32, getReadOnlyReg(op->reg), op->reg);
    }
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void Jit::dynamic_dimulr32e32(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        imulRR(JitWidth::b32, getReg(op->reg), read(JitWidth::b32, calculateEaa(op)));
    } else {
        call_RRI(common_dimul32, JitWidth::b32, read(JitWidth::b32, calculateEaa(op)), JitWidth::b32, getReadOnlyReg(op->reg), op->reg);
    }
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
