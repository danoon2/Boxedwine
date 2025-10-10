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

void dynamic_addr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void dynamic_adde8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void dynamic_addr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void dynamic_add8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void dynamic_add8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void dynamic_addr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void dynamic_adde16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void dynamic_addr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void dynamic_add16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void dynamic_add16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void dynamic_addr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void dynamic_adde32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void dynamic_addr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void dynamic_add32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void dynamic_add32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void dynamic_orr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void dynamic_ore8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void dynamic_orr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void dynamic_or8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void dynamic_or8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void dynamic_orr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void dynamic_ore16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void dynamic_orr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void dynamic_or16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void dynamic_or16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void dynamic_orr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void dynamic_ore32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void dynamic_orr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void dynamic_or32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void dynamic_or32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void dynamic_adcr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void dynamic_adce8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void dynamic_adcr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void dynamic_adc8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void dynamic_adc8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void dynamic_adcr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void dynamic_adce16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void dynamic_adcr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void dynamic_adc16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void dynamic_adc16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void dynamic_adcr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void dynamic_adce32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void dynamic_adcr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void dynamic_adc32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void dynamic_adc32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void dynamic_sbbr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void dynamic_sbbe8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void dynamic_sbbr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void dynamic_sbb8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void dynamic_sbb8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void dynamic_sbbr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void dynamic_sbbe16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void dynamic_sbbr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void dynamic_sbb16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void dynamic_sbb16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void dynamic_sbbr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void dynamic_sbbe32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void dynamic_sbbr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void dynamic_sbb32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void dynamic_sbb32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void dynamic_andr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void dynamic_ande8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void dynamic_andr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void dynamic_and8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void dynamic_and8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void dynamic_andr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void dynamic_ande16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void dynamic_andr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void dynamic_and16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void dynamic_and16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void dynamic_andr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void dynamic_ande32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void dynamic_andr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void dynamic_and32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void dynamic_and32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void dynamic_subr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void dynamic_sube8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void dynamic_subr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void dynamic_sub8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void dynamic_sub8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void dynamic_subr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void dynamic_sube16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void dynamic_subr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void dynamic_sub16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void dynamic_sub16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void dynamic_subr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void dynamic_sube32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void dynamic_subr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void dynamic_sub32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void dynamic_sub32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void dynamic_xorr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void dynamic_xore8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void dynamic_xorr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void dynamic_xor8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void dynamic_xor8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void dynamic_xorr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void dynamic_xore16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void dynamic_xorr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void dynamic_xor16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void dynamic_xor16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void dynamic_xorr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void dynamic_xore32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void dynamic_xorr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void dynamic_xor32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void dynamic_xor32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void dynamic_cmpr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void dynamic_cmpe8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void dynamic_cmpr8e8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void dynamic_cmp8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void dynamic_cmp8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void dynamic_cmpr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void dynamic_cmpe16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void dynamic_cmpr16e16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void dynamic_cmp16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void dynamic_cmp16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void dynamic_cmpr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void dynamic_cmpe32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void dynamic_cmpr32e32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRM(data, op, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void dynamic_cmp32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void dynamic_cmp32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void dynamic_testr8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_8bit, '&', false, false, FLAGS_TEST8);
}
void dynamic_teste8r8(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_8bit, '&', false, false, FLAGS_TEST8);
}
void dynamic_test8_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_8bit, '&', false, false, FLAGS_TEST8);
}
void dynamic_test8_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_8bit, '&', false, false, FLAGS_TEST8);
}
void dynamic_testr16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_16bit, '&', false, false, FLAGS_TEST16);
}
void dynamic_teste16r16(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_16bit, '&', false, false, FLAGS_TEST16);
}
void dynamic_test16_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_16bit, '&', false, false, FLAGS_TEST16);
}
void dynamic_test16_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_16bit, '&', false, false, FLAGS_TEST16);
}
void dynamic_testr32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithRR(data, op, DYN_32bit, '&', false, false, FLAGS_TEST32);
}
void dynamic_teste32r32(DynamicData* data, DecodedOp* op) {
    dynamic_arithMR(data, op, DYN_32bit, '&', false, false, FLAGS_TEST32);
}
void dynamic_test32_reg(DynamicData* data, DecodedOp* op) {
    dynamic_arithRI(data, op, DYN_32bit, '&', false, false, FLAGS_TEST32);
}
void dynamic_test32_mem(DynamicData* data, DecodedOp* op) {
    dynamic_arithMI(data, op, DYN_32bit, '&', false, false, FLAGS_TEST32);
}
void dynamic_notr8(DynamicData* data, DecodedOp* op) {
    instCPU(data, '~', CPU::offsetofReg8(op->reg), DYN_8bit, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_note8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    instMem(data, '~', DYN_ADDRESS, DYN_8bit, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_notr16(DynamicData* data, DecodedOp* op) {
    instCPU(data, '~', CPU::offsetofReg16(op->reg), DYN_16bit, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_note16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    instMem(data, '~', DYN_ADDRESS, DYN_16bit, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_notr32(DynamicData* data, DecodedOp* op) {
    instCPU(data, '~', CPU::offsetofReg32(op->reg), DYN_32bit, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_note32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    instMem(data, '~', DYN_ADDRESS, DYN_32bit, true, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_negr8(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        instCPU(data, '-', CPU::offsetofReg8(op->reg), DYN_8bit, DYN_DEST);
    } else {
        loadRegStoreSrc(data, op->reg, DYN_8bit, DYN_DEST, false);
        instReg(data, '-', DYN_DEST, DYN_8bit);
        storeLazyFlagsResult(data, DYN_DEST, DYN_8bit, false);
        storeReg(data, op->reg, DYN_DEST, DYN_8bit, true);
        storeLazyFlags(data, FLAGS_NEG8);
        data->currentLazyFlags=FLAGS_NEG8;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_nege8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    if (!op->needsToSetFlags(data->cpu)) {
        instMem(data, '-', DYN_ADDRESS, DYN_8bit, true, DYN_DEST);
    } else {
        storeLazyFlagsSrcFromMem(data, DYN_8bit, DYN_ADDRESS, false, false);
        instReg(data, '-', DYN_CALL_RESULT, DYN_8bit);
        storeLazyFlagsResult(data, DYN_CALL_RESULT, DYN_8bit, false);
        movToMemFromReg(data, DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true, DYN_DEST);
        storeLazyFlags(data, FLAGS_NEG8);
        data->currentLazyFlags=FLAGS_NEG8;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_negr16(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        instCPU(data, '-', CPU::offsetofReg16(op->reg), DYN_16bit, DYN_DEST);
    } else {
        loadRegStoreSrc(data, op->reg, DYN_16bit, DYN_DEST, false);
        instReg(data, '-', DYN_DEST, DYN_16bit);
        storeLazyFlagsResult(data, DYN_DEST, DYN_16bit, false);
        storeReg(data, op->reg, DYN_DEST, DYN_16bit, true);
        storeLazyFlags(data, FLAGS_NEG16);
        data->currentLazyFlags=FLAGS_NEG16;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_nege16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    if (!op->needsToSetFlags(data->cpu)) {
        instMem(data, '-', DYN_ADDRESS, DYN_16bit, true, DYN_DEST);
    } else {
        storeLazyFlagsSrcFromMem(data, DYN_16bit, DYN_ADDRESS, false, false);
        instReg(data, '-', DYN_CALL_RESULT, DYN_16bit);
        storeLazyFlagsResult(data, DYN_CALL_RESULT, DYN_16bit, false);
        movToMemFromReg(data, DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_DEST);
        storeLazyFlags(data, FLAGS_NEG16);
        data->currentLazyFlags=FLAGS_NEG16;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_negr32(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        instCPU(data, '-', CPU::offsetofReg32(op->reg), DYN_32bit, DYN_DEST);
    } else {
        loadRegStoreSrc(data, op->reg, DYN_32bit, DYN_DEST, false);
        instReg(data, '-', DYN_DEST, DYN_32bit);
        storeLazyFlagsResult(data, DYN_DEST, DYN_32bit, false);
        storeReg(data, op->reg, DYN_DEST, DYN_32bit, true);
        storeLazyFlags(data, FLAGS_NEG32);
        data->currentLazyFlags=FLAGS_NEG32;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_nege32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    if (!op->needsToSetFlags(data->cpu)) {
        instMem(data, '-', DYN_ADDRESS, DYN_32bit, true, DYN_DEST);
    } else {
        storeLazyFlagsSrcFromMem(data, DYN_32bit, DYN_ADDRESS, false, false);
        instReg(data, '-', DYN_CALL_RESULT, DYN_32bit);
        storeLazyFlagsResult(data, DYN_CALL_RESULT, DYN_32bit, false);
        movToMemFromReg(data, DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
        storeLazyFlags(data, FLAGS_NEG32);
        data->currentLazyFlags=FLAGS_NEG32;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_mulR8(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_mul8, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg8(op->reg), DYN_PARAM_CPU_ADDRESS_8, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_mulE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_8bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_mul8, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_8, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_imulR8(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_imul8, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg8(op->reg), DYN_PARAM_CPU_ADDRESS_8, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_imulE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_8bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_imul8, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_8, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_mulR16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_mul16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_mulE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_mul16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    INCREMENT_EIP(data, op);
}
void dynamic_imulR16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_imul16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_imulE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_imul16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    INCREMENT_EIP(data, op);
}
void dynamic_mulR32(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_mul32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_mulE32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_32bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_mul32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_imulR32(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_imul32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_imulE32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_32bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_imul32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_divR8(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)div8, true, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg8(op->reg), DYN_PARAM_CPU_ADDRESS_8, false);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_divE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_8bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)div8, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_8, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_idivR8(DynamicData* data, DecodedOp* op) {
    DynReg reg = loadReg(data, op->reg, DYN_SRC, DYN_8bit);
    movToRegFromRegSignExtend(data, DYN_SRC, DYN_32bit, reg, DYN_8bit, false); // ARM with -O2 needs this
    callHostFunction(data, (void*)idiv8, true, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_idivE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_8bit, DYN_ADDRESS, true);
    movToRegFromRegSignExtend(data, DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false); // ARM with -O2 needs this
    callHostFunction(data, (void*)idiv8, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_divR16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)div16, true, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_divE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)div16, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_idivR16(DynamicData* data, DecodedOp* op) {
    DynReg reg = loadReg(data, op->reg, DYN_SRC, DYN_16bit);
    movToRegFromRegSignExtend(data, DYN_SRC, DYN_32bit, reg, DYN_16bit, false); // ARM64 with -O2 needs this
    callHostFunction(data, (void*)idiv16, true, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_idivE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    movToRegFromRegSignExtend(data, DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false); // ARM64 with -O2 needs this
    callHostFunction(data, (void*)idiv16, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_divR32(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)div32, true, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_divE32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_32bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)div32, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_idivR32(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)idiv32, true, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_idivE32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_32bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)idiv32, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_dimulcr16r16(DynamicData* data, DecodedOp* op) {
    DynReg reg = loadReg(data, op->rm, DYN_DEST, DYN_16bit);
    callHostFunction(data, (void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, reg, DYN_PARAM_REG_16, true, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulcr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulcr32r32(DynamicData* data, DecodedOp* op) {
    DynReg reg = loadReg(data, op->rm, DYN_DEST, DYN_32bit);
    callHostFunction(data, (void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, reg, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulcr32e32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_32bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulr16r16(DynamicData* data, DecodedOp* op) {
    DynReg regRM = loadReg(data, op->rm, DYN_DEST, DYN_16bit);
    DynReg reg = loadReg(data, op->reg, DYN_SRC, DYN_16bit);
    callHostFunction(data, (void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, regRM, DYN_PARAM_REG_16, true, reg, DYN_PARAM_REG_16, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    DynReg reg = loadReg(data, op->reg, DYN_SRC, DYN_16bit);
    callHostFunction(data, (void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, reg, DYN_PARAM_REG_16, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulr32r32(DynamicData* data, DecodedOp* op) {
    DynReg regRM = loadReg(data, op->rm, DYN_DEST, DYN_32bit);
    DynReg reg = loadReg(data, op->reg, DYN_SRC, DYN_32bit);
    callHostFunction(data, (void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, regRM, DYN_PARAM_REG_32, true, reg, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_dimulr32e32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS); movFromMem(data, DYN_32bit, DYN_ADDRESS, true);
    DynReg reg = loadReg(data, op->reg, DYN_SRC, DYN_32bit);
    callHostFunction(data, (void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, reg, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
