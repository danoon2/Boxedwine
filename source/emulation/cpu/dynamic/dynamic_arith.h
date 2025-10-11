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

void DynamicData::dynamic_addr8r8(DecodedOp* op) {
    dynamic_arithRR(op, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void DynamicData::dynamic_adde8r8(DecodedOp* op) {
    dynamic_arithMR(op, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void DynamicData::dynamic_addr8e8(DecodedOp* op) {
    dynamic_arithRM(op, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void DynamicData::dynamic_add8_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void DynamicData::dynamic_add8_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_8bit, '+', false, true, FLAGS_ADD8);
}
void DynamicData::dynamic_addr16r16(DecodedOp* op) {
    dynamic_arithRR(op, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void DynamicData::dynamic_adde16r16(DecodedOp* op) {
    dynamic_arithMR(op, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void DynamicData::dynamic_addr16e16(DecodedOp* op) {
    dynamic_arithRM(op, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void DynamicData::dynamic_add16_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void DynamicData::dynamic_add16_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_16bit, '+', false, true, FLAGS_ADD16);
}
void DynamicData::dynamic_addr32r32(DecodedOp* op) {
    dynamic_arithRR(op, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void DynamicData::dynamic_adde32r32(DecodedOp* op) {
    dynamic_arithMR(op, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void DynamicData::dynamic_addr32e32(DecodedOp* op) {
    dynamic_arithRM(op, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void DynamicData::dynamic_add32_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void DynamicData::dynamic_add32_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_32bit, '+', false, true, FLAGS_ADD32);
}
void DynamicData::dynamic_orr8r8(DecodedOp* op) {
    dynamic_arithRR(op, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void DynamicData::dynamic_ore8r8(DecodedOp* op) {
    dynamic_arithMR(op, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void DynamicData::dynamic_orr8e8(DecodedOp* op) {
    dynamic_arithRM(op, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void DynamicData::dynamic_or8_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void DynamicData::dynamic_or8_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_8bit, '|', false, true, FLAGS_OR8);
}
void DynamicData::dynamic_orr16r16(DecodedOp* op) {
    dynamic_arithRR(op, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void DynamicData::dynamic_ore16r16(DecodedOp* op) {
    dynamic_arithMR(op, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void DynamicData::dynamic_orr16e16(DecodedOp* op) {
    dynamic_arithRM(op, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void DynamicData::dynamic_or16_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void DynamicData::dynamic_or16_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_16bit, '|', false, true, FLAGS_OR16);
}
void DynamicData::dynamic_orr32r32(DecodedOp* op) {
    dynamic_arithRR(op, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void DynamicData::dynamic_ore32r32(DecodedOp* op) {
    dynamic_arithMR(op, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void DynamicData::dynamic_orr32e32(DecodedOp* op) {
    dynamic_arithRM(op, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void DynamicData::dynamic_or32_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void DynamicData::dynamic_or32_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_32bit, '|', false, true, FLAGS_OR32);
}
void DynamicData::dynamic_adcr8r8(DecodedOp* op) {
    dynamic_arithRR(op, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void DynamicData::dynamic_adce8r8(DecodedOp* op) {
    dynamic_arithMR(op, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void DynamicData::dynamic_adcr8e8(DecodedOp* op) {
    dynamic_arithRM(op, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void DynamicData::dynamic_adc8_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void DynamicData::dynamic_adc8_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_8bit, '+', true, true, FLAGS_ADC8);
}
void DynamicData::dynamic_adcr16r16(DecodedOp* op) {
    dynamic_arithRR(op, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void DynamicData::dynamic_adce16r16(DecodedOp* op) {
    dynamic_arithMR(op, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void DynamicData::dynamic_adcr16e16(DecodedOp* op) {
    dynamic_arithRM(op, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void DynamicData::dynamic_adc16_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void DynamicData::dynamic_adc16_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_16bit, '+', true, true, FLAGS_ADC16);
}
void DynamicData::dynamic_adcr32r32(DecodedOp* op) {
    dynamic_arithRR(op, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void DynamicData::dynamic_adce32r32(DecodedOp* op) {
    dynamic_arithMR(op, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void DynamicData::dynamic_adcr32e32(DecodedOp* op) {
    dynamic_arithRM(op, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void DynamicData::dynamic_adc32_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void DynamicData::dynamic_adc32_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_32bit, '+', true, true, FLAGS_ADC32);
}
void DynamicData::dynamic_sbbr8r8(DecodedOp* op) {
    dynamic_arithRR(op, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void DynamicData::dynamic_sbbe8r8(DecodedOp* op) {
    dynamic_arithMR(op, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void DynamicData::dynamic_sbbr8e8(DecodedOp* op) {
    dynamic_arithRM(op, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void DynamicData::dynamic_sbb8_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void DynamicData::dynamic_sbb8_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_8bit, '-', true, true, FLAGS_SBB8);
}
void DynamicData::dynamic_sbbr16r16(DecodedOp* op) {
    dynamic_arithRR(op, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void DynamicData::dynamic_sbbe16r16(DecodedOp* op) {
    dynamic_arithMR(op, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void DynamicData::dynamic_sbbr16e16(DecodedOp* op) {
    dynamic_arithRM(op, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void DynamicData::dynamic_sbb16_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void DynamicData::dynamic_sbb16_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_16bit, '-', true, true, FLAGS_SBB16);
}
void DynamicData::dynamic_sbbr32r32(DecodedOp* op) {
    dynamic_arithRR(op, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void DynamicData::dynamic_sbbe32r32(DecodedOp* op) {
    dynamic_arithMR(op, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void DynamicData::dynamic_sbbr32e32(DecodedOp* op) {
    dynamic_arithRM(op, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void DynamicData::dynamic_sbb32_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void DynamicData::dynamic_sbb32_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_32bit, '-', true, true, FLAGS_SBB32);
}
void DynamicData::dynamic_andr8r8(DecodedOp* op) {
    dynamic_arithRR(op, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void DynamicData::dynamic_ande8r8(DecodedOp* op) {
    dynamic_arithMR(op, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void DynamicData::dynamic_andr8e8(DecodedOp* op) {
    dynamic_arithRM(op, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void DynamicData::dynamic_and8_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void DynamicData::dynamic_and8_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_8bit, '&', false, true, FLAGS_AND8);
}
void DynamicData::dynamic_andr16r16(DecodedOp* op) {
    dynamic_arithRR(op, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void DynamicData::dynamic_ande16r16(DecodedOp* op) {
    dynamic_arithMR(op, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void DynamicData::dynamic_andr16e16(DecodedOp* op) {
    dynamic_arithRM(op, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void DynamicData::dynamic_and16_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void DynamicData::dynamic_and16_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_16bit, '&', false, true, FLAGS_AND16);
}
void DynamicData::dynamic_andr32r32(DecodedOp* op) {
    dynamic_arithRR(op, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void DynamicData::dynamic_ande32r32(DecodedOp* op) {
    dynamic_arithMR(op, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void DynamicData::dynamic_andr32e32(DecodedOp* op) {
    dynamic_arithRM(op, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void DynamicData::dynamic_and32_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void DynamicData::dynamic_and32_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_32bit, '&', false, true, FLAGS_AND32);
}
void DynamicData::dynamic_subr8r8(DecodedOp* op) {
    dynamic_arithRR(op, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void DynamicData::dynamic_sube8r8(DecodedOp* op) {
    dynamic_arithMR(op, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void DynamicData::dynamic_subr8e8(DecodedOp* op) {
    dynamic_arithRM(op, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void DynamicData::dynamic_sub8_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void DynamicData::dynamic_sub8_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_8bit, '-', false, true, FLAGS_SUB8);
}
void DynamicData::dynamic_subr16r16(DecodedOp* op) {
    dynamic_arithRR(op, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void DynamicData::dynamic_sube16r16(DecodedOp* op) {
    dynamic_arithMR(op, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void DynamicData::dynamic_subr16e16(DecodedOp* op) {
    dynamic_arithRM(op, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void DynamicData::dynamic_sub16_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void DynamicData::dynamic_sub16_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_16bit, '-', false, true, FLAGS_SUB16);
}
void DynamicData::dynamic_subr32r32(DecodedOp* op) {
    dynamic_arithRR(op, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void DynamicData::dynamic_sube32r32(DecodedOp* op) {
    dynamic_arithMR(op, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void DynamicData::dynamic_subr32e32(DecodedOp* op) {
    dynamic_arithRM(op, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void DynamicData::dynamic_sub32_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void DynamicData::dynamic_sub32_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_32bit, '-', false, true, FLAGS_SUB32);
}
void DynamicData::dynamic_xorr8r8(DecodedOp* op) {
    dynamic_arithRR(op, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void DynamicData::dynamic_xore8r8(DecodedOp* op) {
    dynamic_arithMR(op, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void DynamicData::dynamic_xorr8e8(DecodedOp* op) {
    dynamic_arithRM(op, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void DynamicData::dynamic_xor8_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void DynamicData::dynamic_xor8_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_8bit, '^', false, true, FLAGS_XOR8);
}
void DynamicData::dynamic_xorr16r16(DecodedOp* op) {
    dynamic_arithRR(op, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void DynamicData::dynamic_xore16r16(DecodedOp* op) {
    dynamic_arithMR(op, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void DynamicData::dynamic_xorr16e16(DecodedOp* op) {
    dynamic_arithRM(op, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void DynamicData::dynamic_xor16_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void DynamicData::dynamic_xor16_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_16bit, '^', false, true, FLAGS_XOR16);
}
void DynamicData::dynamic_xorr32r32(DecodedOp* op) {
    dynamic_arithRR(op, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void DynamicData::dynamic_xore32r32(DecodedOp* op) {
    dynamic_arithMR(op, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void DynamicData::dynamic_xorr32e32(DecodedOp* op) {
    dynamic_arithRM(op, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void DynamicData::dynamic_xor32_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void DynamicData::dynamic_xor32_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_32bit, '^', false, true, FLAGS_XOR32);
}
void DynamicData::dynamic_cmpr8r8(DecodedOp* op) {
    dynamic_arithRR(op, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void DynamicData::dynamic_cmpe8r8(DecodedOp* op) {
    dynamic_arithMR(op, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void DynamicData::dynamic_cmpr8e8(DecodedOp* op) {
    dynamic_arithRM(op, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void DynamicData::dynamic_cmp8_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void DynamicData::dynamic_cmp8_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_8bit, '-', false, false, FLAGS_CMP8);
}
void DynamicData::dynamic_cmpr16r16(DecodedOp* op) {
    dynamic_arithRR(op, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void DynamicData::dynamic_cmpe16r16(DecodedOp* op) {
    dynamic_arithMR(op, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void DynamicData::dynamic_cmpr16e16(DecodedOp* op) {
    dynamic_arithRM(op, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void DynamicData::dynamic_cmp16_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void DynamicData::dynamic_cmp16_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_16bit, '-', false, false, FLAGS_CMP16);
}
void DynamicData::dynamic_cmpr32r32(DecodedOp* op) {
    dynamic_arithRR(op, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void DynamicData::dynamic_cmpe32r32(DecodedOp* op) {
    dynamic_arithMR(op, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void DynamicData::dynamic_cmpr32e32(DecodedOp* op) {
    dynamic_arithRM(op, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void DynamicData::dynamic_cmp32_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void DynamicData::dynamic_cmp32_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_32bit, '-', false, false, FLAGS_CMP32);
}
void DynamicData::dynamic_testr8r8(DecodedOp* op) {
    dynamic_arithRR(op, DYN_8bit, '&', false, false, FLAGS_TEST8);
}
void DynamicData::dynamic_teste8r8(DecodedOp* op) {
    dynamic_arithMR(op, DYN_8bit, '&', false, false, FLAGS_TEST8);
}
void DynamicData::dynamic_test8_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_8bit, '&', false, false, FLAGS_TEST8);
}
void DynamicData::dynamic_test8_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_8bit, '&', false, false, FLAGS_TEST8);
}
void DynamicData::dynamic_testr16r16(DecodedOp* op) {
    dynamic_arithRR(op, DYN_16bit, '&', false, false, FLAGS_TEST16);
}
void DynamicData::dynamic_teste16r16(DecodedOp* op) {
    dynamic_arithMR(op, DYN_16bit, '&', false, false, FLAGS_TEST16);
}
void DynamicData::dynamic_test16_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_16bit, '&', false, false, FLAGS_TEST16);
}
void DynamicData::dynamic_test16_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_16bit, '&', false, false, FLAGS_TEST16);
}
void DynamicData::dynamic_testr32r32(DecodedOp* op) {
    dynamic_arithRR(op, DYN_32bit, '&', false, false, FLAGS_TEST32);
}
void DynamicData::dynamic_teste32r32(DecodedOp* op) {
    dynamic_arithMR(op, DYN_32bit, '&', false, false, FLAGS_TEST32);
}
void DynamicData::dynamic_test32_reg(DecodedOp* op) {
    dynamic_arithRI(op, DYN_32bit, '&', false, false, FLAGS_TEST32);
}
void DynamicData::dynamic_test32_mem(DecodedOp* op) {
    dynamic_arithMI(op, DYN_32bit, '&', false, false, FLAGS_TEST32);
}
void DynamicData::dynamic_notr8(DecodedOp* op) {
    notCPU(op->reg, DYN_8bit, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_note8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    notMem(DYN_ADDRESS, DYN_8bit, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_notr16(DecodedOp* op) {
    notCPU(op->reg, DYN_16bit, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_note16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    notMem(DYN_ADDRESS, DYN_16bit, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_notr32(DecodedOp* op) {
    notCPU(op->reg, DYN_32bit, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_note32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    notMem(DYN_ADDRESS, DYN_32bit, true, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_negr8(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        negCPU(op->reg, DYN_8bit, DYN_DEST);
    } else {
        loadRegStoreSrc(op->reg, DYN_8bit, DYN_DEST, false);
        negReg(DYN_DEST, DYN_8bit);
        storeLazyFlagsResult(DYN_DEST, DYN_8bit, false);
        storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        storeLazyFlags(FLAGS_NEG8);
        currentLazyFlags=FLAGS_NEG8;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_nege8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    if (!op->needsToSetFlags(cpu)) {
        negMem(DYN_ADDRESS, DYN_8bit, true, DYN_DEST);
    } else {
        storeLazyFlagsSrcFromMem(DYN_8bit, DYN_ADDRESS, false, false);
        negReg(DYN_CALL_RESULT, DYN_8bit);
        storeLazyFlagsResult(DYN_CALL_RESULT, DYN_8bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true, DYN_DEST);
        storeLazyFlags(FLAGS_NEG8);
        currentLazyFlags=FLAGS_NEG8;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_negr16(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        negCPU(op->reg, DYN_16bit, DYN_DEST);
    } else {
        loadRegStoreSrc(op->reg, DYN_16bit, DYN_DEST, false);
        negReg(DYN_DEST, DYN_16bit);
        storeLazyFlagsResult(DYN_DEST, DYN_16bit, false);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        storeLazyFlags(FLAGS_NEG16);
        currentLazyFlags=FLAGS_NEG16;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_nege16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    if (!op->needsToSetFlags(cpu)) {
        negMem(DYN_ADDRESS, DYN_16bit, true, DYN_DEST);
    } else {
        storeLazyFlagsSrcFromMem(DYN_16bit, DYN_ADDRESS, false, false);
        negReg(DYN_CALL_RESULT, DYN_16bit);
        storeLazyFlagsResult(DYN_CALL_RESULT, DYN_16bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_DEST);
        storeLazyFlags(FLAGS_NEG16);
        currentLazyFlags=FLAGS_NEG16;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_negr32(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        negCPU(op->reg, DYN_32bit, DYN_DEST);
    } else {
        loadRegStoreSrc(op->reg, DYN_32bit, DYN_DEST, false);
        negReg(DYN_DEST, DYN_32bit);
        storeLazyFlagsResult(DYN_DEST, DYN_32bit, false);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        storeLazyFlags(FLAGS_NEG32);
        currentLazyFlags=FLAGS_NEG32;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_nege32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    if (!op->needsToSetFlags(cpu)) {
        negMem(DYN_ADDRESS, DYN_32bit, true, DYN_DEST);
    } else {
        storeLazyFlagsSrcFromMem(DYN_32bit, DYN_ADDRESS, false, false);
        negReg(DYN_CALL_RESULT, DYN_32bit);
        storeLazyFlagsResult(DYN_CALL_RESULT, DYN_32bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
        storeLazyFlags(FLAGS_NEG32);
        currentLazyFlags=FLAGS_NEG32;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_mulR8(DecodedOp* op) {
    callHostFunction((void*)common_mul8, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg8(op->reg), DYN_PARAM_CPU_ADDRESS_8, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_mulE8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_mul8, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_8, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_imulR8(DecodedOp* op) {
    callHostFunction((void*)common_imul8, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg8(op->reg), DYN_PARAM_CPU_ADDRESS_8, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_imulE8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_imul8, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_8, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_mulR16(DecodedOp* op) {
    callHostFunction((void*)common_mul16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_mulE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_mul16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_imulR16(DecodedOp* op) {
    callHostFunction((void*)common_imul16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_imulE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_imul16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_mulR32(DecodedOp* op) {
    callHostFunction((void*)common_mul32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_mulE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_mul32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_imulR32(DecodedOp* op) {
    callHostFunction((void*)common_imul32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_imulE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_imul32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_divR8(DecodedOp* op) {
    callHostFunction((void*)div8, true, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg8(op->reg), DYN_PARAM_CPU_ADDRESS_8, false);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_divE8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);
    callHostFunction((void*)div8, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_8, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_idivR8(DecodedOp* op) {
    DynReg reg = loadReg(op->reg, DYN_SRC, DYN_8bit);
    movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, reg, DYN_8bit, false); // ARM with -O2 needs this
    callHostFunction((void*)idiv8, true, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_idivE8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);
    movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false); // ARM with -O2 needs this
    callHostFunction((void*)idiv8, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_divR16(DecodedOp* op) {
    callHostFunction((void*)div16, true, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_divE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)div16, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_idivR16(DecodedOp* op) {
    DynReg reg = loadReg(op->reg, DYN_SRC, DYN_16bit);
    movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, reg, DYN_16bit, false); // ARM64 with -O2 needs this
    callHostFunction((void*)idiv16, true, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_idivE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false); // ARM64 with -O2 needs this
    callHostFunction((void*)idiv16, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_divR32(DecodedOp* op) {
    callHostFunction((void*)div32, true, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_divE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);
    callHostFunction((void*)div32, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_idivR32(DecodedOp* op) {
    callHostFunction((void*)idiv32, true, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_idivE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);
    callHostFunction((void*)idiv32, true, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulcr16r16(DecodedOp* op) {
    DynReg reg = loadReg(op->rm, DYN_DEST, DYN_16bit);
    callHostFunction((void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, reg, DYN_PARAM_REG_16, true, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulcr16e16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulcr32r32(DecodedOp* op) {
    DynReg reg = loadReg(op->rm, DYN_DEST, DYN_32bit);
    callHostFunction((void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, reg, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulcr32e32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulr16r16(DecodedOp* op) {
    DynReg regRM = loadReg(op->rm, DYN_DEST, DYN_16bit);
    DynReg reg = loadReg(op->reg, DYN_SRC, DYN_16bit);
    callHostFunction((void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, regRM, DYN_PARAM_REG_16, true, reg, DYN_PARAM_REG_16, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulr16e16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);
    DynReg reg = loadReg(op->reg, DYN_SRC, DYN_16bit);
    callHostFunction((void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, reg, DYN_PARAM_REG_16, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulr32r32(DecodedOp* op) {
    DynReg regRM = loadReg(op->rm, DYN_DEST, DYN_32bit);
    DynReg reg = loadReg(op->reg, DYN_SRC, DYN_32bit);
    callHostFunction((void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, regRM, DYN_PARAM_REG_32, true, reg, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulr32e32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS, true);
    DynReg reg = loadReg(op->reg, DYN_SRC, DYN_32bit);
    callHostFunction((void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, reg, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
