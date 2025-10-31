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
    call(common_cmpxchg8b_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_cmpxchge32r32_lock(DecodedOp* op) {
    call(common_cmpxchge32r32_lock, DYN_32bit, calculateEaa2(op), op->reg);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchge16r16_lock(DecodedOp* op) {
    call(common_cmpxchge16r16_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_cmpxchge8r8_lock(DecodedOp* op) {
    call(common_cmpxchge8r8_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_xchge32r32_lock(DecodedOp* op) {
    call(common_xchge32r32_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
}
void DynamicData::dynamic_xchge16r16_lock(DecodedOp* op) {
    call(common_xchge16r16_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
}
void DynamicData::dynamic_xchge8r8_lock(DecodedOp* op) {
    call(common_xchge8r8_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
}
void DynamicData::dynamic_xaddr32e32_lock(DecodedOp* op) {
    call(common_xaddr32e32_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_xaddr16e16_lock(DecodedOp* op) {
    call(common_xaddr16e16_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_xaddr8e8_lock(DecodedOp* op) {
    call(common_xaddr8e8_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_adde32r32_lock(DecodedOp* op) {
    call(common_adde32r32_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_adde16r16_lock(DecodedOp* op) {
    call(common_adde16r16_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_adde8r8_lock(DecodedOp* op) {
    call(common_adde8r8_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_add32_mem_lock(DecodedOp* op) {
    call(common_add32_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_add16_mem_lock(DecodedOp* op) {
    call(common_add16_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_add8_mem_lock(DecodedOp* op) {
    call(common_add8_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_sube32r32_lock(DecodedOp* op) {
    call(common_sube32r32_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_sube16r16_lock(DecodedOp* op) {
    call(common_sube16r16_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_sube8r8_lock(DecodedOp* op) {
    call(common_sube8r8_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_sub32_mem_lock(DecodedOp* op) {
    call(common_sub32_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_sub16_mem_lock(DecodedOp* op) {
    call(common_sub16_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_sub8_mem_lock(DecodedOp* op) {
    call(common_sub8_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_ore32r32_lock(DecodedOp* op) {
    call(common_ore32r32_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_ore16r16_lock(DecodedOp* op) {
    call(common_ore16r16_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_ore8r8_lock(DecodedOp* op) {
    call(common_ore8r8_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_or32_mem_lock(DecodedOp* op) {
    call(common_or32_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_or16_mem_lock(DecodedOp* op) {
    call(common_or16_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_or8_mem_lock(DecodedOp* op) {
    call(common_or8_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_ande32r32_lock(DecodedOp* op) {
    call(common_ande32r32_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_ande16r16_lock(DecodedOp* op) {
    call(common_ande16r16_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_ande8r8_lock(DecodedOp* op) {
    call(common_ande8r8_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_and32_mem_lock(DecodedOp* op) {
    call(common_and32_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_and16_mem_lock(DecodedOp* op) {
    call(common_and16_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_and8_mem_lock(DecodedOp* op) {
    call(common_and8_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_xore32r32_lock(DecodedOp* op) {
    call(common_xore32r32_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_xore16r16_lock(DecodedOp* op) {
    call(common_xore16r16_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_xore8r8_lock(DecodedOp* op) {
    call(common_xore8r8_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_xor32_mem_lock(DecodedOp* op) {
    call(common_xor32_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_xor16_mem_lock(DecodedOp* op) {
    call(common_xor16_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_xor8_mem_lock(DecodedOp* op) {
    call(common_xor8_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_adce32r32_lock(DecodedOp* op) {
    call(common_adce32r32_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_adce16r16_lock(DecodedOp* op) {
    call(common_adce16r16_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_adce8r8_lock(DecodedOp* op) {
    call(common_adce8r8_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_adc32_mem_lock(DecodedOp* op) {
    call(common_adc32_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_adc16_mem_lock(DecodedOp* op) {
    call(common_adc16_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_adc8_mem_lock(DecodedOp* op) {
    call(common_adc8_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_sbbe32r32_lock(DecodedOp* op) {
    call(common_sbbe32r32_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_sbbe16r16_lock(DecodedOp* op) {
    call(common_sbbe16r16_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_sbbe8r8_lock(DecodedOp* op) {
    call(common_sbbe8r8_lock, DYN_32bit, calculateEaa2(op), op->reg);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_sbb32_mem_lock(DecodedOp* op) {
    call(common_sbb32_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_sbb16_mem_lock(DecodedOp* op) {
    call(common_sbb16_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_sbb8_mem_lock(DecodedOp* op) {
    call(common_sbb8_mem_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_inc32_mem32_lock(DecodedOp* op) {
    call(common_inc32_mem32_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_inc16_mem16_lock(DecodedOp* op) {
    call(common_inc16_mem16_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_inc8_mem8_lock(DecodedOp* op) {
    call(common_inc8_mem8_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_dec32_mem32_lock(DecodedOp* op) {
    call(common_dec32_mem32_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_dec16_mem16_lock(DecodedOp* op) {
    call(common_dec16_mem16_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_dec8_mem8_lock(DecodedOp* op) {
    call(common_dec8_mem8_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_note32_lock(DecodedOp* op) {
    call(common_note32_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_note16_lock(DecodedOp* op) {
    call(common_note16_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_note8_lock(DecodedOp* op) {
    call(common_note8_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_nege32_lock(DecodedOp* op) {
    call(common_nege32_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_nege16_lock(DecodedOp* op) {
    call(common_nege16_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_nege8_lock(DecodedOp* op) {
    call(common_nege8_lock, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_btse32_lock(DecodedOp* op) {
    call(common_btse32_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_btse16_lock(DecodedOp* op) {
    call(common_btse16_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}

RegPtr DynamicData::calculateEffectiveEaa32(DecodedOp* op) {
    RegPtr result = calculateEaa2(op);
    RegPtr reg = getTmpReg(op->reg);
    
    sarValue(DYN_32bit, reg, 5, false);
    shlValue(DYN_32bit, reg, 2, false);
    addReg(DYN_32bit, result, reg, false);
    return result;
}

RegPtr DynamicData::calculateEffectiveEaa16(DecodedOp* op) {
    RegPtr result = calculateEaa2(op);
    RegPtr reg = getTmpReg(op->reg);

    sarValue(DYN_16bit, reg, 4, false);
    shlValue(DYN_16bit, reg, 1, false);
    movzx(DYN_32bit, reg, DYN_16bit, reg);
    addReg(DYN_32bit, result, reg, false);
    return result;
}

RegPtr DynamicData::calculateMask32(DecodedOp* op) {
    RegPtr reg = getTmpReg(op->reg, false, 1);
    RegPtr result = getTmpReg();    

    movValue(DYN_32bit, result, 1);
    andValue(DYN_32bit, reg, 31, false);
    shlReg(DYN_32bit, result, reg, false);
    return result;
}

RegPtr DynamicData::calculateMask16(DecodedOp* op) {
    RegPtr reg = getTmpReg(op->reg, false, 1);
    RegPtr result = getTmpReg();    

    movValue(DYN_32bit, result, 1);
    andValue(DYN_32bit, reg, 15, false);
    shlReg(DYN_32bit, result, reg, false);
    return result;
}

void DynamicData::dynamic_btse32r32_lock(DecodedOp* op) {
    call(common_btse32r32_lock, DYN_32bit, calculateEffectiveEaa32(op), DYN_32bit, calculateMask32(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}

void DynamicData::dynamic_btse16r16_lock(DecodedOp* op) {
    call(common_btse16r16_lock, DYN_32bit, calculateEffectiveEaa16(op), DYN_32bit, calculateMask16(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}

void DynamicData::dynamic_btre32_lock(DecodedOp* op) {
    call(common_btre32_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_btre16_lock(DecodedOp* op) {
    call(common_btre16_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_btre32r32_lock(DecodedOp* op) {
    call(common_btre32r32_lock, DYN_32bit, calculateEffectiveEaa32(op), DYN_32bit, calculateMask32(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_btre16r16_lock(DecodedOp* op) {
    call(common_btre16r16_lock, DYN_32bit, calculateEffectiveEaa16(op), DYN_32bit, calculateMask16(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_btce32_lock(DecodedOp* op) {
    call(common_btce32_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_btce16_lock(DecodedOp* op) {
    call(common_btce16_lock, DYN_32bit, calculateEaa2(op), op->imm);
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_btce32r32_lock(DecodedOp* op) {
    call(common_btce32r32_lock, DYN_32bit, calculateEffectiveEaa32(op), DYN_32bit, calculateMask32(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_btce16r16_lock(DecodedOp* op) {
    call(common_btce16r16_lock, DYN_32bit, calculateEffectiveEaa16(op), DYN_32bit, calculateMask16(op));
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}

#endif