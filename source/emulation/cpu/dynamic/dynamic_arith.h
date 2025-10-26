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
    dynamic_RR(op, DYN_8bit, &DynamicData::addReg);
}
void DynamicData::dynamic_adde8r8(DecodedOp* op) {
    dynamic_MR(op, DYN_8bit, &DynamicData::addReg);
}
void DynamicData::dynamic_addr8e8(DecodedOp* op) {
    dynamic_RM(op, DYN_8bit, &DynamicData::addReg);
}
void DynamicData::dynamic_add8_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::addValue);
}
void DynamicData::dynamic_add8_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::addValue);
}
void DynamicData::dynamic_addr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::addReg);
}
void DynamicData::dynamic_adde16r16(DecodedOp* op) {
    dynamic_MR(op, DYN_16bit, &DynamicData::addReg);
}
void DynamicData::dynamic_addr16e16(DecodedOp* op) {
    dynamic_RM(op, DYN_16bit, &DynamicData::addReg);
}
void DynamicData::dynamic_add16_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::addValue);
}
void DynamicData::dynamic_add16_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::addValue);
}
void DynamicData::dynamic_addr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::addReg);
}
void DynamicData::dynamic_adde32r32(DecodedOp* op) {
    dynamic_MR(op, DYN_32bit, &DynamicData::addReg);
}
void DynamicData::dynamic_addr32e32(DecodedOp* op) {
    dynamic_RM(op, DYN_32bit, &DynamicData::addReg);
}
void DynamicData::dynamic_add32_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::addValue);
}
void DynamicData::dynamic_add32_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::addValue);
}
void DynamicData::dynamic_orr8r8(DecodedOp* op) {
    dynamic_RR(op, DYN_8bit, &DynamicData::orReg);
}
void DynamicData::dynamic_ore8r8(DecodedOp* op) {
    dynamic_MR(op, DYN_8bit, &DynamicData::orReg);
}
void DynamicData::dynamic_orr8e8(DecodedOp* op) {
    dynamic_RM(op, DYN_8bit, &DynamicData::orReg);
}
void DynamicData::dynamic_or8_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::orValue);
}
void DynamicData::dynamic_or8_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::orValue);
}
void DynamicData::dynamic_orr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::orReg);
}
void DynamicData::dynamic_ore16r16(DecodedOp* op) {
    dynamic_MR(op, DYN_16bit, &DynamicData::orReg);
}
void DynamicData::dynamic_orr16e16(DecodedOp* op) {
    dynamic_RM(op, DYN_16bit, &DynamicData::orReg);
}
void DynamicData::dynamic_or16_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::orValue);
}
void DynamicData::dynamic_or16_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::orValue);
}
void DynamicData::dynamic_orr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::orReg);
}
void DynamicData::dynamic_ore32r32(DecodedOp* op) {
    dynamic_MR(op, DYN_32bit, &DynamicData::orReg);
}
void DynamicData::dynamic_orr32e32(DecodedOp* op) {
    dynamic_RM(op, DYN_32bit, &DynamicData::orReg);
}
void DynamicData::dynamic_or32_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::orValue);
}
void DynamicData::dynamic_or32_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::orValue);
}
void DynamicData::dynamic_adcr8r8(DecodedOp* op) {
    dynamic_RR(op, DYN_8bit, &DynamicData::adcReg);
}
void DynamicData::dynamic_adce8r8(DecodedOp* op) {
    dynamic_MR(op, DYN_8bit, &DynamicData::adcReg);
}
void DynamicData::dynamic_adcr8e8(DecodedOp* op) {
    dynamic_RM(op, DYN_8bit, &DynamicData::adcReg);
}
void DynamicData::dynamic_adc8_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::adcValue);
}
void DynamicData::dynamic_adc8_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::adcValue);
}
void DynamicData::dynamic_adcr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::adcReg);
}
void DynamicData::dynamic_adce16r16(DecodedOp* op) {
    dynamic_MR(op, DYN_16bit, &DynamicData::adcReg);
}
void DynamicData::dynamic_adcr16e16(DecodedOp* op) {
    dynamic_RM(op, DYN_16bit, &DynamicData::adcReg);
}
void DynamicData::dynamic_adc16_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::adcValue);
}
void DynamicData::dynamic_adc16_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::adcValue);
}
void DynamicData::dynamic_adcr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::adcReg);
}
void DynamicData::dynamic_adce32r32(DecodedOp* op) {
    dynamic_MR(op, DYN_32bit, &DynamicData::adcReg);
}
void DynamicData::dynamic_adcr32e32(DecodedOp* op) {
    dynamic_RM(op, DYN_32bit, &DynamicData::adcReg);
}
void DynamicData::dynamic_adc32_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::adcValue);
}
void DynamicData::dynamic_adc32_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::adcValue);
}
void DynamicData::dynamic_sbbr8r8(DecodedOp* op) {
    dynamic_RR(op, DYN_8bit, &DynamicData::sbbReg);
}
void DynamicData::dynamic_sbbe8r8(DecodedOp* op) {
    dynamic_MR(op, DYN_8bit, &DynamicData::sbbReg);
}
void DynamicData::dynamic_sbbr8e8(DecodedOp* op) {
    dynamic_RM(op, DYN_8bit, &DynamicData::sbbReg);
}
void DynamicData::dynamic_sbb8_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::sbbValue);
}
void DynamicData::dynamic_sbb8_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::sbbValue);
}
void DynamicData::dynamic_sbbr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::sbbReg);
}
void DynamicData::dynamic_sbbe16r16(DecodedOp* op) {
    dynamic_MR(op, DYN_16bit, &DynamicData::sbbReg);
}
void DynamicData::dynamic_sbbr16e16(DecodedOp* op) {
    dynamic_RM(op, DYN_16bit, &DynamicData::sbbReg);
}
void DynamicData::dynamic_sbb16_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::sbbValue);
}
void DynamicData::dynamic_sbb16_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::sbbValue);
}
void DynamicData::dynamic_sbbr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::sbbReg);
}
void DynamicData::dynamic_sbbe32r32(DecodedOp* op) {
    dynamic_MR(op, DYN_32bit, &DynamicData::sbbReg);
}
void DynamicData::dynamic_sbbr32e32(DecodedOp* op) {
    dynamic_RM(op, DYN_32bit, &DynamicData::sbbReg);
}
void DynamicData::dynamic_sbb32_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::sbbValue);
}
void DynamicData::dynamic_sbb32_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::sbbValue);
}
void DynamicData::dynamic_andr8r8(DecodedOp* op) {
    dynamic_RR(op, DYN_8bit, &DynamicData::andReg);
}
void DynamicData::dynamic_ande8r8(DecodedOp* op) {
    dynamic_MR(op, DYN_8bit, &DynamicData::andReg);
}
void DynamicData::dynamic_andr8e8(DecodedOp* op) {
    dynamic_RM(op, DYN_8bit, &DynamicData::andReg);
}
void DynamicData::dynamic_and8_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::andValue);
}
void DynamicData::dynamic_and8_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::andValue);
}
void DynamicData::dynamic_andr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::andReg);
}
void DynamicData::dynamic_ande16r16(DecodedOp* op) {
    dynamic_MR(op, DYN_16bit, &DynamicData::andReg);
}
void DynamicData::dynamic_andr16e16(DecodedOp* op) {
    dynamic_RM(op, DYN_16bit, &DynamicData::andReg);
}
void DynamicData::dynamic_and16_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::andValue);
}
void DynamicData::dynamic_and16_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::andValue);
}
void DynamicData::dynamic_andr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::andReg);
}
void DynamicData::dynamic_ande32r32(DecodedOp* op) {
    dynamic_MR(op, DYN_32bit, &DynamicData::andReg);
}
void DynamicData::dynamic_andr32e32(DecodedOp* op) {
    dynamic_RM(op, DYN_32bit, &DynamicData::andReg);
}
void DynamicData::dynamic_and32_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::andValue);
}
void DynamicData::dynamic_and32_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::andValue);
}
void DynamicData::dynamic_subr8r8(DecodedOp* op) {
    dynamic_RR(op, DYN_8bit, &DynamicData::subReg);
}
void DynamicData::dynamic_sube8r8(DecodedOp* op) {
    dynamic_MR(op, DYN_8bit, &DynamicData::subReg);
}
void DynamicData::dynamic_subr8e8(DecodedOp* op) {
    dynamic_RM(op, DYN_8bit, &DynamicData::subReg);
}
void DynamicData::dynamic_sub8_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::subValue);
}
void DynamicData::dynamic_sub8_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::subValue);
}
void DynamicData::dynamic_subr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::subReg);
}
void DynamicData::dynamic_sube16r16(DecodedOp* op) {
    dynamic_MR(op, DYN_16bit, &DynamicData::subReg);
}
void DynamicData::dynamic_subr16e16(DecodedOp* op) {
    dynamic_RM(op, DYN_16bit, &DynamicData::subReg);
}
void DynamicData::dynamic_sub16_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::subValue);
}
void DynamicData::dynamic_sub16_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::subValue);
}
void DynamicData::dynamic_subr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::subReg);
}
void DynamicData::dynamic_sube32r32(DecodedOp* op) {
    dynamic_MR(op, DYN_32bit, &DynamicData::subReg);
}
void DynamicData::dynamic_subr32e32(DecodedOp* op) {
    dynamic_RM(op, DYN_32bit, &DynamicData::subReg);
}
void DynamicData::dynamic_sub32_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::subValue);
}
void DynamicData::dynamic_sub32_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::subValue);
}
void DynamicData::dynamic_xorr8r8(DecodedOp* op) {
    dynamic_RR(op, DYN_8bit, &DynamicData::xorReg);
}
void DynamicData::dynamic_xore8r8(DecodedOp* op) {
    dynamic_MR(op, DYN_8bit, &DynamicData::xorReg);
}
void DynamicData::dynamic_xorr8e8(DecodedOp* op) {
    dynamic_RM(op, DYN_8bit, &DynamicData::xorReg);
}
void DynamicData::dynamic_xor8_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::xorValue);
}
void DynamicData::dynamic_xor8_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::xorValue);
}
void DynamicData::dynamic_xorr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::xorReg);
}
void DynamicData::dynamic_xore16r16(DecodedOp* op) {
    dynamic_MR(op, DYN_16bit, &DynamicData::xorReg);
}
void DynamicData::dynamic_xorr16e16(DecodedOp* op) {
    dynamic_RM(op, DYN_16bit, &DynamicData::xorReg);
}
void DynamicData::dynamic_xor16_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::xorValue);
}
void DynamicData::dynamic_xor16_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::xorValue);
}
void DynamicData::dynamic_xorr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::xorReg);
}
void DynamicData::dynamic_xore32r32(DecodedOp* op) {
    dynamic_MR(op, DYN_32bit, &DynamicData::xorReg);
}
void DynamicData::dynamic_xorr32e32(DecodedOp* op) {
    dynamic_RM(op, DYN_32bit, &DynamicData::xorReg);
}
void DynamicData::dynamic_xor32_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::xorValue);
}
void DynamicData::dynamic_xor32_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::xorValue);
}
void DynamicData::dynamic_cmpr8r8(DecodedOp* op) {
    dynamic_RR(op, DYN_8bit, &DynamicData::cmpReg);
}
void DynamicData::dynamic_cmpe8r8(DecodedOp* op) {
    dynamic_MR(op, DYN_8bit, &DynamicData::cmpReg, false);
}
void DynamicData::dynamic_cmpr8e8(DecodedOp* op) {
    dynamic_RM(op, DYN_8bit, &DynamicData::cmpReg, false);
}
void DynamicData::dynamic_cmp8_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::cmpValue, false);
}
void DynamicData::dynamic_cmp8_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::cmpValue, false);
}
void DynamicData::dynamic_cmpr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::cmpReg, false);
}
void DynamicData::dynamic_cmpe16r16(DecodedOp* op) {
    dynamic_MR(op, DYN_16bit, &DynamicData::cmpReg, false);
}
void DynamicData::dynamic_cmpr16e16(DecodedOp* op) {
    dynamic_RM(op, DYN_16bit, &DynamicData::cmpReg, false);
}
void DynamicData::dynamic_cmp16_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::cmpValue, false);
}
void DynamicData::dynamic_cmp16_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::cmpValue, false);
}
void DynamicData::dynamic_cmpr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::cmpReg, false);
}
void DynamicData::dynamic_cmpe32r32(DecodedOp* op) {
    dynamic_MR(op, DYN_32bit, &DynamicData::cmpReg, false);
}
void DynamicData::dynamic_cmpr32e32(DecodedOp* op) {
    dynamic_RM(op, DYN_32bit, &DynamicData::cmpReg, false);
}
void DynamicData::dynamic_cmp32_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::cmpValue, false);
}
void DynamicData::dynamic_cmp32_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::cmpValue, false);
}
void DynamicData::dynamic_testr8r8(DecodedOp* op) {
    dynamic_RR(op, DYN_8bit, &DynamicData::testReg);
}
void DynamicData::dynamic_teste8r8(DecodedOp* op) {
    dynamic_MR(op, DYN_8bit, &DynamicData::testReg, false);
}
void DynamicData::dynamic_test8_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::testValue, false);
}
void DynamicData::dynamic_test8_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::testValue, false);
}
void DynamicData::dynamic_testr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::testReg, false);
}
void DynamicData::dynamic_teste16r16(DecodedOp* op) {
    dynamic_MR(op, DYN_16bit, &DynamicData::testReg, false);
}
void DynamicData::dynamic_test16_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::testValue, false);
}
void DynamicData::dynamic_test16_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::testValue, false);
}
void DynamicData::dynamic_testr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::testReg, false);
}
void DynamicData::dynamic_teste32r32(DecodedOp* op) {
    dynamic_MR(op, DYN_32bit, &DynamicData::testReg, false);
}
void DynamicData::dynamic_test32_reg(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::testValue, false);
}
void DynamicData::dynamic_test32_mem(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::testValue, false);
}
void DynamicData::dynamic_notr8(DecodedOp* op) {
    dynamic_R(op, DYN_8bit, &DynamicData::notReg2);
}
void DynamicData::dynamic_note8(DecodedOp* op) {
    dynamic_M(op, DYN_8bit, &DynamicData::notReg2);
}
void DynamicData::dynamic_notr16(DecodedOp* op) {
    dynamic_R(op, DYN_16bit, &DynamicData::notReg2);
}
void DynamicData::dynamic_note16(DecodedOp* op) {
    dynamic_M(op, DYN_16bit, &DynamicData::notReg2);
}
void DynamicData::dynamic_notr32(DecodedOp* op) {
    dynamic_R(op, DYN_32bit, &DynamicData::notReg2);
}
void DynamicData::dynamic_note32(DecodedOp* op) {
    dynamic_M(op, DYN_32bit, &DynamicData::notReg2);
}
void DynamicData::dynamic_negr8(DecodedOp* op) {
    dynamic_R(op, DYN_8bit, &DynamicData::negReg2);
}
void DynamicData::dynamic_nege8(DecodedOp* op) {
    dynamic_M(op, DYN_8bit, &DynamicData::negReg2);
}
void DynamicData::dynamic_negr16(DecodedOp* op) {
    dynamic_R(op, DYN_16bit, &DynamicData::negReg2);
}
void DynamicData::dynamic_nege16(DecodedOp* op) {
    dynamic_M(op, DYN_16bit, &DynamicData::negReg2);
}
void DynamicData::dynamic_negr32(DecodedOp* op) {
    dynamic_R(op, DYN_32bit, &DynamicData::negReg2);
}
void DynamicData::dynamic_nege32(DecodedOp* op) {
    dynamic_M(op, DYN_32bit, &DynamicData::negReg2);
}
void DynamicData::dynamic_mulR8(DecodedOp* op) {
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        loadReg(0, DYN_DEST, DYN_8bit);
        movToRegFromReg(DYN_DEST, DYN_16bit, DYN_DEST, DYN_8bit, false);
        loadReg(op->reg, DYN_SRC, DYN_8bit);
        movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit, false);
        imulRegReg(DYN_DEST, DYN_SRC, DYN_16bit, true);
        storeReg(0, DYN_DEST, DYN_16bit, false);
    } else {
        callHostFunction((void*)common_mul8, false, 2, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CPU_REG_8, false);
        currentLazyFlags = FLAGS_NONE;
    }    
    incrementEip(op->len);
}
void DynamicData::dynamic_mulE8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    movFromMem(DYN_8bit, DYN_ADDRESS, true);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        loadReg(0, DYN_DEST, DYN_8bit);
        movToRegFromReg(DYN_DEST, DYN_16bit, DYN_DEST, DYN_8bit, false);
        movToRegFromReg(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_8bit, false);
        imulRegReg(DYN_DEST, DYN_CALL_RESULT, DYN_16bit, true);
        storeReg(0, DYN_DEST, DYN_16bit, false);
    } else {
        callHostFunction((void*)common_mul8, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_imulR8(DecodedOp* op) {
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        loadReg(0, DYN_DEST, DYN_8bit);
        movToRegFromRegSignExtend(DYN_DEST, DYN_16bit, DYN_DEST, DYN_8bit, false);
        loadReg(op->reg, DYN_SRC, DYN_8bit);
        movToRegFromRegSignExtend(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit, false);
        imulRegReg(DYN_DEST, DYN_SRC, DYN_16bit, true);
        storeReg(0, DYN_DEST, DYN_16bit, false);
    } else {
        callHostFunction((void*)common_imul8, false, 2, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CPU_REG_8, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_imulE8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_8bit, DYN_ADDRESS, true);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        loadReg(0, DYN_DEST, DYN_8bit);
        movToRegFromRegSignExtend(DYN_DEST, DYN_16bit, DYN_DEST, DYN_8bit, false);
        movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_8bit, false);
        imulRegReg(DYN_DEST, DYN_CALL_RESULT, DYN_16bit, true);
        storeReg(0, DYN_DEST, DYN_16bit, false);
    } else {
        callHostFunction((void*)common_imul8, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_mulR16(DecodedOp* op) {
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        loadReg(0, DYN_DEST, DYN_16bit);
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit, false);
        loadReg(op->reg, DYN_SRC, DYN_16bit);
        movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false);
        imulRegReg(DYN_DEST, DYN_SRC, DYN_32bit, true);
        storeReg(0, DYN_DEST, DYN_16bit, false);
        shrRegImm(DYN_DEST, DYN_32bit, 16);
        storeReg(2, DYN_DEST, DYN_16bit, true);
    } else {
        callHostFunction((void*)common_mul16, false, 2, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CPU_REG_16, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_mulE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        loadReg(0, DYN_DEST, DYN_16bit);
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit, false);
        movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
        imulRegReg(DYN_DEST, DYN_CALL_RESULT, DYN_32bit, true);
        storeReg(0, DYN_DEST, DYN_16bit, false);
        shrRegImm(DYN_DEST, DYN_32bit, 16);
        storeReg(2, DYN_DEST, DYN_16bit, true);
    } else {
        callHostFunction((void*)common_mul16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_imulR16(DecodedOp* op) {
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        loadReg(0, DYN_DEST, DYN_16bit);
        movToRegFromRegSignExtend(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit, false);
        loadReg(op->reg, DYN_SRC, DYN_16bit);
        movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false);
        imulRegReg(DYN_DEST, DYN_SRC, DYN_32bit, true);
        storeReg(0, DYN_DEST, DYN_16bit, false);
        shrRegImm(DYN_DEST, DYN_32bit, 16);
        storeReg(2, DYN_DEST, DYN_16bit, true);
    } else {
        callHostFunction((void*)common_imul16, false, 2, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CPU_REG_16, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_imulE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        loadReg(0, DYN_DEST, DYN_16bit);
        movToRegFromRegSignExtend(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit, false);
        movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
        imulRegReg(DYN_DEST, DYN_SRC, DYN_32bit, true);
        storeReg(0, DYN_DEST, DYN_16bit, false);
        shrRegImm(DYN_DEST, DYN_32bit, 16);
        storeReg(2, DYN_DEST, DYN_16bit, true);
    } else {
        callHostFunction((void*)common_imul16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_mulR32(DecodedOp* op) {
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        loadReg(0, DYN_CALL_RESULT, DYN_32bit);
        if (op->reg == 0) {
            mulRegReg64(DYN_DEST, DYN_CALL_RESULT, DYN_CALL_RESULT, false);
        } else {
            loadReg(op->reg, DYN_SRC, DYN_32bit);
            mulRegReg64(DYN_DEST, DYN_CALL_RESULT, DYN_SRC, true);
        }
        storeReg(0, DYN_CALL_RESULT, DYN_32bit, true);
        storeReg(2, DYN_DEST, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_mul32, false, 2, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CPU_REG_32, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_mulE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
        loadReg(0, DYN_CALL_RESULT, DYN_32bit);
        mulRegReg64(DYN_DEST, DYN_CALL_RESULT, DYN_SRC, true);
        storeReg(0, DYN_CALL_RESULT, DYN_32bit, true);
        storeReg(2, DYN_DEST, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_mul32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_imulR32(DecodedOp* op) {
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        loadReg(0, DYN_CALL_RESULT, DYN_32bit);
        if (op->reg == 0) {
            imulRegReg64(DYN_DEST, DYN_CALL_RESULT, DYN_CALL_RESULT, false);
        } else {
            loadReg(op->reg, DYN_SRC, DYN_32bit);
            imulRegReg64(DYN_DEST, DYN_CALL_RESULT, DYN_SRC, true);
        }
        storeReg(0, DYN_CALL_RESULT, DYN_32bit, true);
        storeReg(2, DYN_DEST, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_imul32, false, 2, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CPU_REG_32, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_imulE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
        loadReg(0, DYN_CALL_RESULT, DYN_32bit);
        imulRegReg64(DYN_DEST, DYN_CALL_RESULT, DYN_SRC, true);
        storeReg(0, DYN_CALL_RESULT, DYN_32bit, true);
        storeReg(2, DYN_DEST, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_imul32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}

static void dynamic_prepareException(CPU* cpu, int code, int error) {
    common_prepareException(cpu, code, error);
    cpu->nextOp = cpu->getNextOp();
}

void DynamicData::div8(DecodedOp* op, DynReg src, bool isSigned, InstDiv callback) {
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
    IfNot(src, false);
        callHostFunction((void*)dynamic_prepareException, true, 3, 0, DYN_PARAM_CPU, false, EXCEPTION_DIVIDE, DYN_PARAM_CONST_32, false, 0, DYN_PARAM_CONST_32, false);
        blockExit();
    EndIf();
    loadReg(0, DYN_CALL_RESULT, DYN_16bit);
    (this->*callback)(DYN_CALL_RESULT, src, DYN_DEST, DYN_16bit);    
    movToRegFromReg(DYN_ADDRESS, DYN_32bit, (DynReg)4, DYN_8bit, false);
    if (isSigned) {
        If(DYN_ADDRESS, false);
            subRegImm(DYN_ADDRESS, DYN_32bit, 0xff);
            If(DYN_ADDRESS, true);
                callHostFunction((void*)dynamic_prepareException, true, 3, 0, DYN_PARAM_CPU, false, EXCEPTION_DIVIDE, DYN_PARAM_CONST_32, false, 0, DYN_PARAM_CONST_32, false);
                blockExit();
            EndIf();
        EndIf();
    } else {                
        If(DYN_ADDRESS, true);
            callHostFunction((void*)dynamic_prepareException, true, 3, 0, DYN_PARAM_CPU, false, EXCEPTION_DIVIDE, DYN_PARAM_CONST_32, false, 0, DYN_PARAM_CONST_32, false);
            blockExit();
        EndIf();
    }    
    movToRegFromReg((DynReg)4, DYN_8bit, DYN_DEST, DYN_8bit, true); // AH = rem;
    storeReg(0, DYN_CALL_RESULT, DYN_16bit, true);
    incrementEip(op->len);
}

void DynamicData::div16(DecodedOp* op, DynReg src, bool isSigned, InstDiv callback) {
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
    IfNot(src, false);
    callHostFunction((void*)dynamic_prepareException, true, 3, 0, DYN_PARAM_CPU, false, EXCEPTION_DIVIDE, DYN_PARAM_CONST_32, false, 0, DYN_PARAM_CONST_32, false);
    blockExit();
    EndIf();
    loadReg(2, DYN_DEST, DYN_16bit);
    shlRegImm(DYN_DEST, DYN_32bit, 16);
    xorRegReg(DYN_CALL_RESULT, DYN_CALL_RESULT, DYN_32bit, false);
    loadReg(0, DYN_CALL_RESULT, DYN_16bit);    
    orRegReg(DYN_CALL_RESULT, DYN_DEST, DYN_32bit, true);
    (this->*callback)(DYN_CALL_RESULT, src, DYN_DEST, DYN_32bit);
    movToRegFromReg(DYN_ADDRESS, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, false);
    shrRegImm(DYN_ADDRESS, DYN_32bit, 16);
    if (isSigned) {
        If(DYN_ADDRESS, false);
            subRegImm(DYN_ADDRESS, DYN_32bit, 0xffff);
            If(DYN_ADDRESS, true);
                callHostFunction((void*)dynamic_prepareException, true, 3, 0, DYN_PARAM_CPU, false, EXCEPTION_DIVIDE, DYN_PARAM_CONST_32, false, 0, DYN_PARAM_CONST_32, false);
                blockExit();
            EndIf();
        EndIf();
    } else {
        If(DYN_ADDRESS, true);
            callHostFunction((void*)dynamic_prepareException, true, 3, 0, DYN_PARAM_CPU, false, EXCEPTION_DIVIDE, DYN_PARAM_CONST_32, false, 0, DYN_PARAM_CONST_32, false);
            blockExit();
        EndIf();
    }
    storeReg(0, DYN_CALL_RESULT, DYN_16bit, true);
    storeReg(2, DYN_DEST, DYN_16bit, true);
    incrementEip(op->len);
}

void DynamicData::div32(DecodedOp* op, DynReg src, InstDiv callback, std::function<void()> fallback) {
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
    IfNot(src, false);
        callHostFunction((void*)dynamic_prepareException, true, 3, 0, DYN_PARAM_CPU, false, EXCEPTION_DIVIDE, DYN_PARAM_CONST_32, false, 0, DYN_PARAM_CONST_32, false);
        blockExit();
    EndIf();
    loadReg(2, DYN_DEST, DYN_32bit);
    If(DYN_DEST, true);
        fallback();
    StartElse();
        loadReg(0, DYN_CALL_RESULT, DYN_32bit);
        (this->*callback)(DYN_CALL_RESULT, src, DYN_DEST, DYN_32bit);
        storeReg(0, DYN_CALL_RESULT, DYN_32bit, true);
        storeReg(2, DYN_DEST, DYN_32bit, true);        
    EndIf();
    incrementEip(op->len);
}

void DynamicData::dynamic_divR8(DecodedOp* op) {        
    loadReg(op->reg, DYN_SRC, DYN_8bit);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    div8(op, DYN_SRC, false, &DynamicData::divRegRegWithRemainder);
}
void DynamicData::dynamic_divE8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    movFromMem(DYN_8bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, true);
    div8(op, DYN_SRC, false, &DynamicData::divRegRegWithRemainder);
}

void DynamicData::dynamic_idivR8(DecodedOp* op) {
    loadReg(op->reg, DYN_SRC, DYN_8bit);
    movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    div8(op, DYN_SRC, true, &DynamicData::idivRegRegWithRemainder);
}
void DynamicData::dynamic_idivE8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_8bit, DYN_ADDRESS, true);
    movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, true);
    div8(op, DYN_SRC, true, &DynamicData::idivRegRegWithRemainder);
}
void DynamicData::dynamic_divR16(DecodedOp* op) {
    loadReg(op->reg, DYN_SRC, DYN_16bit);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false);
    div16(op, DYN_SRC, false, &DynamicData::divRegRegWithRemainder);
}
void DynamicData::dynamic_divE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, true);
    div16(op, DYN_SRC, false, &DynamicData::divRegRegWithRemainder);
}
void DynamicData::dynamic_idivR16(DecodedOp* op) {
    loadReg(op->reg, DYN_SRC, DYN_16bit);
    movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false);
    div16(op, DYN_SRC, true, &DynamicData::idivRegRegWithRemainder);
}
void DynamicData::dynamic_idivE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, true);
    div16(op, DYN_SRC, true, &DynamicData::idivRegRegWithRemainder);
}
void DynamicData::dynamic_divR32(DecodedOp* op) {
    loadReg(op->reg, DYN_SRC, DYN_32bit);
    div32(op, DYN_SRC, &DynamicData::divRegRegWithRemainder, [op, this]() {
        callHostFunction((void*)::div32, true, 2, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CPU_REG_32, false);
        IfNot(DYN_CALL_RESULT, true);
            blockDone(true);
        EndIf();
    });
}
void DynamicData::dynamic_divE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); 
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    div32(op, DYN_SRC, &DynamicData::divRegRegWithRemainder, [op, this]() {
        callHostFunction((void*)::div32, true, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, false);
        IfNot(DYN_CALL_RESULT, true);
            blockDone(true);
        EndIf();
    });
}
void DynamicData::dynamic_idivR32(DecodedOp* op) {
    loadReg(op->reg, DYN_SRC, DYN_32bit);
    div32(op, DYN_SRC, &DynamicData::idivRegRegWithRemainder, [op, this]() {
        callHostFunction((void*)idiv32, true, 2, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CPU_REG_32, false);
        IfNot(DYN_CALL_RESULT, true);
            blockDone(true);
        EndIf();
    });
}
void DynamicData::dynamic_idivE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    div32(op, DYN_SRC, &DynamicData::idivRegRegWithRemainder, [op, this]() {
        callHostFunction((void*)idiv32, true, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, false);
        IfNot(DYN_CALL_RESULT, true);
            blockDone(true);
        EndIf();
    });
}
void DynamicData::dynamic_dimulcr16r16(DecodedOp* op) {
    loadReg(op->rm, DYN_SRC, DYN_16bit);
    if (!(op->needsToSetFlags(cpu) & (CF|OF))) {
        imulRegImm(DYN_SRC, DYN_16bit, op->imm);
        storeReg(op->reg, DYN_SRC, DYN_16bit, true);
    } else {
        callHostFunction((void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_16, true, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulcr16e16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        imulRegImm(DYN_CALL_RESULT, DYN_16bit, op->imm);
        storeReg(op->reg, DYN_CALL_RESULT, DYN_16bit, true);
    } else {
        callHostFunction((void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulcr32r32(DecodedOp* op) {
    loadReg(op->rm, DYN_SRC, DYN_32bit);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        imulRegImm(DYN_SRC, DYN_32bit, op->imm);
        storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulcr32e32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        imulRegImm(DYN_CALL_RESULT, DYN_32bit, op->imm);
        storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulr16r16(DecodedOp* op) {
    loadReg(op->rm, DYN_SRC, DYN_16bit);
    loadReg(op->reg, DYN_DEST, DYN_16bit);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        imulRegReg(DYN_DEST, DYN_SRC, DYN_16bit, true);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
    } else {
        callHostFunction((void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, DYN_DEST, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, op->reg, DYN_PARAM_CONST_32, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulr16e16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    loadReg(op->reg, DYN_SRC, DYN_16bit);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        imulRegReg(DYN_SRC, DYN_CALL_RESULT, DYN_16bit, true);
        storeReg(op->reg, DYN_SRC, DYN_16bit, true);
    } else {
        callHostFunction((void*)common_dimul16, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, op->reg, DYN_PARAM_CONST_32, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulr32r32(DecodedOp* op) {
    loadReg(op->rm, DYN_SRC, DYN_32bit);
    loadReg(op->reg, DYN_DEST, DYN_32bit);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        imulRegReg(DYN_DEST, DYN_SRC, DYN_32bit, true);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, DYN_DEST, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulr32e32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    loadReg(op->reg, DYN_DEST, DYN_32bit);
    if (!(op->needsToSetFlags(cpu) & (CF | OF))) {
        imulRegReg(DYN_DEST, DYN_CALL_RESULT, DYN_32bit, true);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_dimul32, false, 4, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, DYN_DEST, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
