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

void DynamicData::dynamic_inc8_reg(DecodedOp* op) {
    dynamic_R(op, DYN_8bit, &DynamicData::incReg);
}
void DynamicData::dynamic_inc8_mem8(DecodedOp* op) {    
    dynamic_M(op, DYN_8bit, &DynamicData::incReg);
}
void DynamicData::dynamic_inc16_reg(DecodedOp* op) {
    dynamic_R(op, DYN_16bit, &DynamicData::incReg);
}
void DynamicData::dynamic_inc16_mem16(DecodedOp* op) {    
    dynamic_M(op, DYN_16bit, &DynamicData::incReg);
}
void DynamicData::dynamic_inc32_reg(DecodedOp* op) {
    dynamic_R(op, DYN_32bit, &DynamicData::incReg);
}
void DynamicData::dynamic_inc32_mem32(DecodedOp* op) {    
    dynamic_M(op, DYN_32bit, &DynamicData::incReg);
}
void DynamicData::dynamic_dec8_reg(DecodedOp* op) {
    dynamic_R(op, DYN_8bit, &DynamicData::decReg);
}
void DynamicData::dynamic_dec8_mem8(DecodedOp* op) {    
    dynamic_M(op, DYN_8bit, &DynamicData::decReg);
}
void DynamicData::dynamic_dec16_reg(DecodedOp* op) {
    dynamic_R(op, DYN_16bit, &DynamicData::decReg);
}
void DynamicData::dynamic_dec16_mem16(DecodedOp* op) {    
    dynamic_M(op, DYN_16bit, &DynamicData::decReg);
}
void DynamicData::dynamic_dec32_reg(DecodedOp* op) {
    dynamic_R(op, DYN_32bit, &DynamicData::decReg);
}
void DynamicData::dynamic_dec32_mem32(DecodedOp* op) {    
    dynamic_M(op, DYN_32bit, &DynamicData::decReg);
}
