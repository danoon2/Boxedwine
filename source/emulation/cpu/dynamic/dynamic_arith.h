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
    // AX = AL * src;
    dynamic_R(op, DYN_8bit, &DynamicData::mulReg, false);
}
void DynamicData::dynamic_mulE8(DecodedOp* op) {
    // getTmpReg will register the reg we want the read from memory to use, by default it will be EAX, but mul needs to this to be unused
    dynamic_M(op, DYN_8bit, &DynamicData::mulReg, false, getTmpReg());
}
void DynamicData::dynamic_imulR8(DecodedOp* op) {
    dynamic_R(op, DYN_8bit, &DynamicData::imulReg, false);
}
void DynamicData::dynamic_imulE8(DecodedOp* op) {
    dynamic_M(op, DYN_8bit, &DynamicData::imulReg, false, getTmpReg());
}
void DynamicData::dynamic_mulR16(DecodedOp* op) {
    dynamic_R(op, DYN_16bit, &DynamicData::mulReg, false);
}
void DynamicData::dynamic_mulE16(DecodedOp* op) {    
    dynamic_M(op, DYN_16bit, &DynamicData::mulReg, false, getTmpReg());
}
void DynamicData::dynamic_imulR16(DecodedOp* op) {
    dynamic_R(op, DYN_16bit, &DynamicData::imulReg, false);
}
void DynamicData::dynamic_imulE16(DecodedOp* op) {
    dynamic_M(op, DYN_16bit, &DynamicData::imulReg, false, getTmpReg());
}
void DynamicData::dynamic_mulR32(DecodedOp* op) {
    dynamic_R(op, DYN_32bit, &DynamicData::mulReg, false);
}
void DynamicData::dynamic_mulE32(DecodedOp* op) {
    dynamic_M(op, DYN_32bit, &DynamicData::mulReg, false, getTmpReg());
}
void DynamicData::dynamic_imulR32(DecodedOp* op) {
    dynamic_R(op, DYN_32bit, &DynamicData::imulReg, false);
}
void DynamicData::dynamic_imulE32(DecodedOp* op) {
    dynamic_M(op, DYN_32bit, &DynamicData::imulReg, false, getTmpReg());
}

static void dynamic_prepareException(CPU* cpu, U32 code, U32 error) {
    common_prepareException(cpu, code, error);
    cpu->nextOp = cpu->getNextOp();
}

void DynamicData::div8(DecodedOp* op, RegPtr src, bool isSigned, InstDiv2 callback) {
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
    IfNot(DYN_8bit, src);
        call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 0);
        blockExit();
    EndIf();

    // this code is fragile, the order of creating the tmps here matters for the x86 JIT implementation
    RegPtr eax = getTmpRegForCallResult();
    mov(DYN_16bit, eax, getReadOnlyReg(0));
    RegPtr remainder = getTmpReg();
    RegPtr tmp = getTmpReg();    

    // do 16-bit div instead of 8-bit so that we can detect the overflow and generate an exception
    (this->*callback)(DYN_16bit, eax, src, remainder);

    xorReg(DYN_32bit, tmp, tmp, false);
    mov8(tmp, false, eax, true);

    if (isSigned) {
        If(DYN_32bit, tmp);
            subValue(DYN_32bit, tmp, 0xff, false);
            If(DYN_32bit, tmp);
                call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 1);
                blockExit();
            EndIf();
        EndIf();
    } else {
        If(DYN_32bit, tmp);
            call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 1);
            blockExit();
        EndIf();
    }
    mov8(eax, true, remainder, false);
    mov(DYN_16bit, getReg(0), eax);
    incrementEip(op->len);
}

void DynamicData::div16(DecodedOp* op, RegPtr src, bool isSigned, InstDiv2 callback) {
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
    IfNot(DYN_16bit, src);
        call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 0);
        blockExit();
    EndIf();

    // this code is fragile, the order of creating the tmps here matters for the x86 JIT implementation
    RegPtr eax = getTmpRegForCallResult();
    xorReg(DYN_32bit, eax, eax, false);
    mov(DYN_16bit, eax, getReadOnlyReg(0));
    RegPtr remainder = getTmpReg(2);
    RegPtr tmp = getTmpReg();

    // combine DX:AX into EAX for 32-bit div
    shlValue(DYN_32bit, remainder, 16, false);    
    orReg(DYN_32bit, eax, remainder, false);

    // do 32-bit div instead of 16-bit so that we can detect the overflow and generate an exception
    (this->*callback)(DYN_32bit, eax, src, remainder);

    mov(DYN_32bit, tmp, eax);
    shrValue(DYN_32bit, tmp, 16, false);

    if (isSigned) {
        If(DYN_32bit, tmp);
            subValue(DYN_32bit, tmp, 0xffff, false);
            If(DYN_32bit, tmp);
                call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 1);
                blockExit();
            EndIf();
        EndIf();
    } else {
        If(DYN_32bit, tmp);
            call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 1);
            blockExit();
        EndIf();
    }
    tmp = nullptr;
    mov(DYN_16bit, getReg(0), eax);
    mov(DYN_16bit, getReg(2), remainder);
    incrementEip(op->len);
}

void DynamicData::div32(DecodedOp* op, RegPtr src, InstDiv2 callback, std::function<void()> fallback) {
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
    IfNot(DYN_32bit, src);
        call_II(dynamic_prepareException, EXCEPTION_DIVIDE, 0);
        blockExit();
    EndIf();
    
    RegPtr remainder = getReadOnlyReg(2, false, 2); // read only because in the fallback scenario, we don't want to write it back
    // only take the inline path if we are guaranteed it won't overflow
    If(DYN_32bit, remainder);
        fallback();
    StartElse();
#ifdef BOXEDWINE_DYNAMIC32
        RegPtr eax = getTmpRegForCallResult();        
        mov(DYN_32bit, eax, getReadOnlyReg(0));
        (this->*callback)(DYN_32bit, eax, src, remainder);
        mov(DYN_32bit, getReg(0), eax);
        mov(DYN_32bit, getReg(2), remainder);
#else
        RegPtr eax = getReg(0);
        (this->*callback)(DYN_32bit, eax, src, remainder);
#endif
    EndIf();
    incrementEip(op->len);
}

void DynamicData::dynamic_divR8(DecodedOp* op) {
    RegPtr src = getTmpReg8(op->reg);
    movzx(DYN_32bit, src, DYN_8bit, src);
    div8(op, src, false, &DynamicData::divRegRegWithRemainder2);
}
void DynamicData::dynamic_divE8(DecodedOp* op) {
    // getTmpReg to help x86 JIT, so that its tmp EAX hardware reg is still available for the div
    RegPtr src = read(DYN_8bit, calculateEaa2(op), nullptr, nullptr, false, getTmpReg());
    movzx(DYN_32bit, src, DYN_8bit, src);
    div8(op, src, false, &DynamicData::divRegRegWithRemainder2);
}
void DynamicData::dynamic_idivR8(DecodedOp* op) {
    RegPtr src = getTmpReg8(op->reg);
    movsx(DYN_32bit, src, DYN_8bit, src);
    div8(op, src, true, &DynamicData::idivRegRegWithRemainder2);
}
void DynamicData::dynamic_idivE8(DecodedOp* op) {
    RegPtr src = read(DYN_8bit, calculateEaa2(op), nullptr, nullptr, false, getTmpReg());
    movsx(DYN_32bit, src, DYN_8bit, src);
    div8(op, src, true, &DynamicData::idivRegRegWithRemainder2);
}
void DynamicData::dynamic_divR16(DecodedOp* op) {
    RegPtr src = getTmpReg(op->reg);
    movzx(DYN_32bit, src, DYN_16bit, src);
    div16(op, src, false, &DynamicData::divRegRegWithRemainder2);
}
void DynamicData::dynamic_divE16(DecodedOp* op) {
    // getTmpReg to help x86 JIT, so that its tmp EAX hardware reg is still available for the div
    RegPtr src = read(DYN_16bit, calculateEaa2(op), nullptr, nullptr, false, getTmpReg());
    movzx(DYN_32bit, src, DYN_16bit, src);
    div16(op, src, false, &DynamicData::divRegRegWithRemainder2);
}
void DynamicData::dynamic_idivR16(DecodedOp* op) {
    RegPtr src = getTmpReg(op->reg);
    movsx(DYN_32bit, src, DYN_16bit, src);
    div16(op, src, true, &DynamicData::idivRegRegWithRemainder2);
}
void DynamicData::dynamic_idivE16(DecodedOp* op) {
    RegPtr src = read(DYN_16bit, calculateEaa2(op), nullptr, nullptr, false, getTmpReg());
    movsx(DYN_32bit, src, DYN_16bit, src);
    div16(op, src, true, &DynamicData::idivRegRegWithRemainder2);
}
void DynamicData::dynamic_divR32(DecodedOp* op) {
    div32(op, getReadOnlyReg(op->reg), &DynamicData::divRegRegWithRemainder2, [op, this]() {
        RegPtr result = callAndReturn_R(::div32, DYN_32bit, getReadOnlyReg(op->reg));
        IfNot(DYN_32bit, result);
            blockDone(true);
        EndIf();
    });
}
void DynamicData::dynamic_divE32(DecodedOp* op) {
    RegPtr src = read(DYN_32bit, calculateEaa2(op), nullptr, nullptr, false, getTmpReg());
    div32(op, src, &DynamicData::divRegRegWithRemainder2, [op, src, this]() {
        RegPtr result = callAndReturn_R(::div32, DYN_32bit, src);
        IfNot(DYN_32bit, result);
            blockDone(true);
        EndIf();
    });
}
void DynamicData::dynamic_idivR32(DecodedOp* op) {
    div32(op, getReadOnlyReg(op->reg), &DynamicData::idivRegRegWithRemainder2, [op, this]() {
        RegPtr result = callAndReturn_RS(::idiv32, DYN_32bit, getReadOnlyReg(op->reg));
        IfNot(DYN_32bit, result);
            blockDone(true);
        EndIf();
    });
}
void DynamicData::dynamic_idivE32(DecodedOp* op) {
    RegPtr src = read(DYN_32bit, calculateEaa2(op), nullptr, nullptr, false, getTmpReg());
    div32(op, src, &DynamicData::idivRegRegWithRemainder2, [op, src, this]() {
        RegPtr result = callAndReturn_RS(::idiv32, DYN_32bit, src);
        IfNot(DYN_32bit, result);
            blockDone(true);
        EndIf();
    });
}
void DynamicData::dynamic_dimulcr16r16(DecodedOp* op) {
    RegPtr dest = getReg(op->reg); // don't inline with imulRRI, we don't want the sync back to happen until after imulRRI
    imulRRI(DYN_16bit, dest, getReadOnlyReg(op->rm), op->imm, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulcr16e16(DecodedOp* op) {
    RegPtr dest = getReg(op->reg); // don't inline with imulRRI, we don't want the sync back to happen until after imulRRI
    imulRRI(DYN_16bit, dest, read(DYN_16bit, calculateEaa2(op)), op->imm, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulcr32r32(DecodedOp* op) {
    RegPtr dest = getReg(op->reg); // don't inline with imulRRI, we don't want the sync back to happen until after imulRRI
    imulRRI(DYN_32bit, dest, getReadOnlyReg(op->rm), op->imm, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulcr32e32(DecodedOp* op) {
    RegPtr dest = getReg(op->reg); // don't inline with imulRRI, we don't want the sync back to happen until after imulRRI
    imulRRI(DYN_32bit, dest, read(DYN_32bit, calculateEaa2(op)), op->imm, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulr16r16(DecodedOp* op) {
    RegPtr dest = getReg(op->reg); // don't inline with imulRR, we don't want the sync back to happen until after imulRRI
    imulRR(DYN_16bit, dest, getReadOnlyReg(op->rm), true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulr16e16(DecodedOp* op) {
    RegPtr dest = getReg(op->reg); // don't inline with imulRR, we don't want the sync back to happen until after imulRRI
    imulRR(DYN_16bit, dest, read(DYN_16bit, calculateEaa2(op)), true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulr32r32(DecodedOp* op) {
    RegPtr dest = getReg(op->reg); // don't inline with imulRR, we don't want the sync back to happen until after imulRRI
    imulRR(DYN_32bit, dest, getReadOnlyReg(op->rm), true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dimulr32e32(DecodedOp* op) {
    RegPtr dest = getReg(op->reg); // don't inline with imulRR, we don't want the sync back to happen until after imulRRI
    imulRR(DYN_32bit, dest, read(DYN_32bit, calculateEaa2(op)), true);
    incrementEip(op->len);
}
