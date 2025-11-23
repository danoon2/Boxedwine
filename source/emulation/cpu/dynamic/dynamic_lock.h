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

void DynamicData::dynamic_cmpxchg8b_lock(DecodedOp* op) {
    call_R(common_cmpxchg8b_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}
void DynamicData::dynamic_cmpxchge32r32_lock(DecodedOp* op) {
    call_RI(common_cmpxchge32r32_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    currentLazyFlags = FLAGS_CMP32;
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchge16r16_lock(DecodedOp* op) {
    call_RI(common_cmpxchge16r16_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_CMP16;
}
void DynamicData::dynamic_cmpxchge8r8_lock(DecodedOp* op) {
    call_RI(common_cmpxchge8r8_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_CMP8;
}
void DynamicData::dynamic_xchge32r32_lock(DecodedOp* op) {
    call_RI(common_xchge32r32_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
}
void DynamicData::dynamic_xchge16r16_lock(DecodedOp* op) {
    call_RI(common_xchge16r16_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
}
void DynamicData::dynamic_xchge8r8_lock(DecodedOp* op) {
    call_RI(common_xchge8r8_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
}
void DynamicData::dynamic_xaddr32e32_lock(DecodedOp* op) {
    call_RI(common_xaddr32e32_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADD32;
}
void DynamicData::dynamic_xaddr16e16_lock(DecodedOp* op) {
    call_RI(common_xaddr16e16_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADD16;
}
void DynamicData::dynamic_xaddr8e8_lock(DecodedOp* op) {
    call_RI(common_xaddr8e8_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADD8;
}
void DynamicData::dynamic_adde32r32_lock(DecodedOp* op) {
    call_RI(common_adde32r32_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADD32;
}
void DynamicData::dynamic_adde16r16_lock(DecodedOp* op) {
    call_RI(common_adde16r16_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADD16;
}
void DynamicData::dynamic_adde8r8_lock(DecodedOp* op) {
    call_RI(common_adde8r8_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADD8;
}
void DynamicData::dynamic_add32_mem_lock(DecodedOp* op) {
    call_RI(common_add32_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADD32;
}
void DynamicData::dynamic_add16_mem_lock(DecodedOp* op) {
    call_RI(common_add16_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADD16;
}
void DynamicData::dynamic_add8_mem_lock(DecodedOp* op) {
    call_RI(common_add8_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADD8;
}
void DynamicData::dynamic_sube32r32_lock(DecodedOp* op) {
    call_RI(common_sube32r32_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_SUB32;
}
void DynamicData::dynamic_sube16r16_lock(DecodedOp* op) {
    call_RI(common_sube16r16_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_SUB16;
}
void DynamicData::dynamic_sube8r8_lock(DecodedOp* op) {
    call_RI(common_sube8r8_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_SUB8;
}
void DynamicData::dynamic_sub32_mem_lock(DecodedOp* op) {
    call_RI(common_sub32_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_SUB32;
}
void DynamicData::dynamic_sub16_mem_lock(DecodedOp* op) {
    call_RI(common_sub16_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_SUB16;
}
void DynamicData::dynamic_sub8_mem_lock(DecodedOp* op) {
    call_RI(common_sub8_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_SUB8;
}
void DynamicData::dynamic_ore32r32_lock(DecodedOp* op) {
    call_RI(common_ore32r32_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_OR32;
}
void DynamicData::dynamic_ore16r16_lock(DecodedOp* op) {
    call_RI(common_ore16r16_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_OR16;
}
void DynamicData::dynamic_ore8r8_lock(DecodedOp* op) {
    call_RI(common_ore8r8_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_OR8;
}
void DynamicData::dynamic_or32_mem_lock(DecodedOp* op) {
    call_RI(common_or32_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_OR32;
}
void DynamicData::dynamic_or16_mem_lock(DecodedOp* op) {
    call_RI(common_or16_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_OR16;
}
void DynamicData::dynamic_or8_mem_lock(DecodedOp* op) {
    call_RI(common_or8_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_OR8;
}
void DynamicData::dynamic_ande32r32_lock(DecodedOp* op) {
    call_RI(common_ande32r32_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_AND32;
}
void DynamicData::dynamic_ande16r16_lock(DecodedOp* op) {
    call_RI(common_ande16r16_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_AND16;
}
void DynamicData::dynamic_ande8r8_lock(DecodedOp* op) {
    call_RI(common_ande8r8_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_AND8;
}
void DynamicData::dynamic_and32_mem_lock(DecodedOp* op) {
    call_RI(common_and32_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_AND32;
}
void DynamicData::dynamic_and16_mem_lock(DecodedOp* op) {
    call_RI(common_and16_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_AND16;
}
void DynamicData::dynamic_and8_mem_lock(DecodedOp* op) {
    call_RI(common_and8_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_AND8;
}
void DynamicData::dynamic_xore32r32_lock(DecodedOp* op) {
    call_RI(common_xore32r32_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_XOR32;
}
void DynamicData::dynamic_xore16r16_lock(DecodedOp* op) {
    call_RI(common_xore16r16_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_XOR16;
}
void DynamicData::dynamic_xore8r8_lock(DecodedOp* op) {
    call_RI(common_xore8r8_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_XOR8;
}
void DynamicData::dynamic_xor32_mem_lock(DecodedOp* op) {
    call_RI(common_xor32_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_XOR32;
}
void DynamicData::dynamic_xor16_mem_lock(DecodedOp* op) {
    call_RI(common_xor16_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_XOR16;
}
void DynamicData::dynamic_xor8_mem_lock(DecodedOp* op) {
    call_RI(common_xor8_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_XOR8;
}
void DynamicData::dynamic_adce32r32_lock(DecodedOp* op) {
    call_RI(common_adce32r32_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADC32;
}
void DynamicData::dynamic_adce16r16_lock(DecodedOp* op) {
    call_RI(common_adce16r16_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADC16;
}
void DynamicData::dynamic_adce8r8_lock(DecodedOp* op) {
    call_RI(common_adce8r8_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADC8;
}
void DynamicData::dynamic_adc32_mem_lock(DecodedOp* op) {
    call_RI(common_adc32_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADC32;
}
void DynamicData::dynamic_adc16_mem_lock(DecodedOp* op) {
    call_RI(common_adc16_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADC16;
}
void DynamicData::dynamic_adc8_mem_lock(DecodedOp* op) {
    call_RI(common_adc8_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_ADC8;
}
void DynamicData::dynamic_sbbe32r32_lock(DecodedOp* op) {
    call_RI(common_sbbe32r32_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_SBB32;
}
void DynamicData::dynamic_sbbe16r16_lock(DecodedOp* op) {
    call_RI(common_sbbe16r16_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_SBB16;
}
void DynamicData::dynamic_sbbe8r8_lock(DecodedOp* op) {
    call_RI(common_sbbe8r8_lock, JitWidth::b32, calculateEaaV2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_SBB8;
}
void DynamicData::dynamic_sbb32_mem_lock(DecodedOp* op) {
    call_RI(common_sbb32_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_SBB32;
}
void DynamicData::dynamic_sbb16_mem_lock(DecodedOp* op) {
    call_RI(common_sbb16_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_SBB16;
}
void DynamicData::dynamic_sbb8_mem_lock(DecodedOp* op) {
    call_RI(common_sbb8_mem_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_SBB8;
}
void DynamicData::dynamic_inc32_mem32_lock(DecodedOp* op) {
    call_R(common_inc32_mem32_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_INC32;
}
void DynamicData::dynamic_inc16_mem16_lock(DecodedOp* op) {
    call_R(common_inc16_mem16_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_INC16;
}
void DynamicData::dynamic_inc8_mem8_lock(DecodedOp* op) {
    call_R(common_inc8_mem8_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_INC8;
}
void DynamicData::dynamic_dec32_mem32_lock(DecodedOp* op) {
    call_R(common_dec32_mem32_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_DEC32;
}
void DynamicData::dynamic_dec16_mem16_lock(DecodedOp* op) {
    call_R(common_dec16_mem16_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_DEC16;
}
void DynamicData::dynamic_dec8_mem8_lock(DecodedOp* op) {
    call_R(common_dec8_mem8_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_DEC8;
}
void DynamicData::dynamic_note32_lock(DecodedOp* op) {
    call_R(common_note32_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_note16_lock(DecodedOp* op) {
    call_R(common_note16_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_note8_lock(DecodedOp* op) {
    call_R(common_note8_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_nege32_lock(DecodedOp* op) {
    call_R(common_nege32_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NEG32;
}
void DynamicData::dynamic_nege16_lock(DecodedOp* op) {
    call_R(common_nege16_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NEG16;
}
void DynamicData::dynamic_nege8_lock(DecodedOp* op) {
    call_R(common_nege8_lock, JitWidth::b32, calculateEaaV2(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NEG8;
}
void DynamicData::dynamic_btse32_lock(DecodedOp* op) {
    call_RI(common_btse32_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}
void DynamicData::dynamic_btse16_lock(DecodedOp* op) {
    call_RI(common_btse16_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}

RegPtr DynamicData::calculateMask32(DecodedOp* op) {
    RegPtr reg = getTmpReg(op->reg, false, 1);
    RegPtr result = getTmpReg();

    movValue(JitWidth::b32, result, 1);
    andValue(JitWidth::b32, reg, 31);
    shlReg(JitWidth::b32, result, reg);
    return result;
}

RegPtr DynamicData::calculateMask16(DecodedOp* op) {
    RegPtr reg = getTmpReg(op->reg, false, 1);
    RegPtr result = getTmpReg();

    movValue(JitWidth::b32, result, 1);
    andValue(JitWidth::b32, reg, 15);
    shlReg(JitWidth::b32, result, reg);
    return result;
}

void DynamicData::dynamic_btse32r32_lock(DecodedOp* op) {
    call_RR(common_btse32r32_lock, JitWidth::b32, calculateEffectiveEaa32(op), JitWidth::b32, calculateMask32(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}

void DynamicData::dynamic_btse16r16_lock(DecodedOp* op) {
    call_RR(common_btse16r16_lock, JitWidth::b32, calculateEffectiveEaa16(op), JitWidth::b32, calculateMask16(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}

void DynamicData::dynamic_btre32_lock(DecodedOp* op) {
    call_RI(common_btre32_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}
void DynamicData::dynamic_btre16_lock(DecodedOp* op) {
    call_RI(common_btre16_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}
void DynamicData::dynamic_btre32r32_lock(DecodedOp* op) {
    call_RR(common_btre32r32_lock, JitWidth::b32, calculateEffectiveEaa32(op), JitWidth::b32, calculateMask32(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}
void DynamicData::dynamic_btre16r16_lock(DecodedOp* op) {
    call_RR(common_btre16r16_lock, JitWidth::b32, calculateEffectiveEaa16(op), JitWidth::b32, calculateMask16(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}
void DynamicData::dynamic_btce32_lock(DecodedOp* op) {
    call_RI(common_btce32_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}
void DynamicData::dynamic_btce16_lock(DecodedOp* op) {
    call_RI(common_btce16_lock, JitWidth::b32, calculateEaaV2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}
void DynamicData::dynamic_btce32r32_lock(DecodedOp* op) {
    call_RR(common_btce32r32_lock, JitWidth::b32, calculateEffectiveEaa32(op), JitWidth::b32, calculateMask32(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}
void DynamicData::dynamic_btce16r16_lock(DecodedOp* op) {
    call_RR(common_btce16r16_lock, JitWidth::b32, calculateEffectiveEaa16(op), JitWidth::b32, calculateMask16(op));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
}

#endif