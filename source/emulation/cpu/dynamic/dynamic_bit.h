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

#include "../common/common_bit.h"
void DynamicData::dynamic_btr16r16(DecodedOp* op) {
    callHostFunction((void*)common_btr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btr16(DecodedOp* op) {
    callHostFunction((void*)common_btr16, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_bte16r16(DecodedOp* op) {
    callHostFunction((void*)common_bte16r16, false, 3, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
    currentLazyFlags=FLAGS_NONE;
}
void DynamicData::dynamic_bte16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bte16, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
    currentLazyFlags=FLAGS_NONE;
}
void DynamicData::dynamic_btr32r32(DecodedOp* op) {
    callHostFunction((void*)common_btr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btr32(DecodedOp* op) {
    callHostFunction((void*)common_btr32, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_bte32r32(DecodedOp* op) {
    callHostFunction((void*)common_bte32r32, false, 3, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_bte32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bte32, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btsr16r16(DecodedOp* op) {
    callHostFunction((void*)common_btsr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btsr16(DecodedOp* op) {
    callHostFunction((void*)common_btsr16, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btse16r16(DecodedOp* op) {
    callHostFunction((void*)common_btse16r16, false, 3, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
    currentLazyFlags=FLAGS_NONE;
}
void DynamicData::dynamic_btse16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_btse16, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
    currentLazyFlags=FLAGS_NONE;
}
void DynamicData::dynamic_btsr32r32(DecodedOp* op) {
    callHostFunction((void*)common_btsr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btsr32(DecodedOp* op) {
    callHostFunction((void*)common_btsr32, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btse32r32(DecodedOp* op) {
    callHostFunction((void*)common_btse32r32, false, 3, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btse32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_btse32, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btrr16r16(DecodedOp* op) {
    callHostFunction((void*)common_btrr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btrr16(DecodedOp* op) {
    callHostFunction((void*)common_btrr16, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btre16r16(DecodedOp* op) {
    callHostFunction((void*)common_btre16r16, false, 3, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
    currentLazyFlags=FLAGS_NONE;
}
void DynamicData::dynamic_btre16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_btre16, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
    currentLazyFlags=FLAGS_NONE;
}
void DynamicData::dynamic_btrr32r32(DecodedOp* op) {
    callHostFunction((void*)common_btrr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btrr32(DecodedOp* op) {
    callHostFunction((void*)common_btrr32, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btre32r32(DecodedOp* op) {
    callHostFunction((void*)common_btre32r32, false, 3, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btre32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_btre32, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btcr16r16(DecodedOp* op) {
    callHostFunction((void*)common_btcr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btcr16(DecodedOp* op) {
    callHostFunction((void*)common_btcr16, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btce16r16(DecodedOp* op) {
    callHostFunction((void*)common_btce16r16, false, 3, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
    currentLazyFlags=FLAGS_NONE;
}
void DynamicData::dynamic_btce16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_btce16, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
    currentLazyFlags=FLAGS_NONE;
}
void DynamicData::dynamic_btcr32r32(DecodedOp* op) {
    callHostFunction((void*)common_btcr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btcr32(DecodedOp* op) {
    callHostFunction((void*)common_btcr32, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btce32r32(DecodedOp* op) {
    callHostFunction((void*)common_btce32r32, false, 3, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_btce32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_btce32, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_bsfr16r16(DecodedOp* op) {
    callHostFunction((void*)common_bsfr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_bsfr16e16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bsfr16e16, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_bsfr32r32(DecodedOp* op) {
    callHostFunction((void*)common_bsfr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_bsfr32e32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bsfr32e32, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_bsrr16r16(DecodedOp* op) {
    callHostFunction((void*)common_bsrr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_bsrr16e16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bsrr16e16, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_bsrr32r32(DecodedOp* op) {
    callHostFunction((void*)common_bsrr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_bsrr32e32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bsrr32e32, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
