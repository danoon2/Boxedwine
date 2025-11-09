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
    dynamic_RR(op, DYN_16bit, &DynamicData::btReg, false);
}
void DynamicData::dynamic_btr16(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::btValue, false);
}
void DynamicData::dynamic_bte16r16(DecodedOp* op) {
    dynamic_MR_effective(op, DYN_16bit, &DynamicData::btReg, false);
}
void DynamicData::dynamic_bte16(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::btValue, false);
}
void DynamicData::dynamic_btr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::btReg, false);
}
void DynamicData::dynamic_btr32(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::btValue, false);
}
void DynamicData::dynamic_bte32r32(DecodedOp* op) {
    dynamic_MR_effective(op, DYN_32bit, &DynamicData::btReg, false);
}
void DynamicData::dynamic_bte32(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::btValue);
}
void DynamicData::dynamic_btsr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::btsReg);
}
void DynamicData::dynamic_btsr16(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::btsValue);
}
void DynamicData::dynamic_btse16r16(DecodedOp* op) {
    dynamic_MR_effective(op, DYN_16bit, &DynamicData::btsReg);
}
void DynamicData::dynamic_btse16(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::btsValue);
}
void DynamicData::dynamic_btsr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::btsReg);
}
void DynamicData::dynamic_btsr32(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::btsValue);
}
void DynamicData::dynamic_btse32r32(DecodedOp* op) {
    dynamic_MR_effective(op, DYN_32bit, &DynamicData::btsReg);
}
void DynamicData::dynamic_btse32(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::btsValue);
}
void DynamicData::dynamic_btrr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::btrReg);
}
void DynamicData::dynamic_btrr16(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::btrValue);
}
void DynamicData::dynamic_btre16r16(DecodedOp* op) {
    dynamic_MR_effective(op, DYN_16bit, &DynamicData::btrReg);
}
void DynamicData::dynamic_btre16(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::btrValue);
}
void DynamicData::dynamic_btrr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::btrReg);
}
void DynamicData::dynamic_btrr32(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::btrValue);
}
void DynamicData::dynamic_btre32r32(DecodedOp* op) {
    dynamic_MR_effective(op, DYN_32bit, &DynamicData::btrReg);
}
void DynamicData::dynamic_btre32(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::btrValue);
}
void DynamicData::dynamic_btcr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::btcReg);
}
void DynamicData::dynamic_btcr16(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::btcValue);
}
void DynamicData::dynamic_btce16r16(DecodedOp* op) {
    dynamic_MR_effective(op, DYN_16bit, &DynamicData::btcReg);
}
void DynamicData::dynamic_btce16(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::btcValue);
}
void DynamicData::dynamic_btcr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::btcReg);
}
void DynamicData::dynamic_btcr32(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::btcValue);
}
void DynamicData::dynamic_btce32r32(DecodedOp* op) {
    dynamic_MR_effective(op, DYN_32bit, &DynamicData::btcReg);
}
void DynamicData::dynamic_btce32(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::btcValue);
}
void DynamicData::dynamic_bsfr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::bsfReg);
}
void DynamicData::dynamic_bsfr16e16(DecodedOp* op) {
    dynamic_RM(op, DYN_16bit, &DynamicData::bsfReg);
}
void DynamicData::dynamic_bsfr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::bsfReg);
}
void DynamicData::dynamic_bsfr32e32(DecodedOp* op) {
    dynamic_RM(op, DYN_32bit, &DynamicData::bsfReg);
}
void DynamicData::dynamic_bsrr16r16(DecodedOp* op) {
    dynamic_RR(op, DYN_16bit, &DynamicData::bsrReg);
}
void DynamicData::dynamic_bsrr16e16(DecodedOp* op) {
    dynamic_RM(op, DYN_16bit, &DynamicData::bsrReg);
}
void DynamicData::dynamic_bsrr32r32(DecodedOp* op) {
    dynamic_RR(op, DYN_32bit, &DynamicData::bsrReg);
}
void DynamicData::dynamic_bsrr32e32(DecodedOp* op) {
    dynamic_RM(op, DYN_32bit, &DynamicData::bsrReg);
}
