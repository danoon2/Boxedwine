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

#include "../normal/normal_shift.h"
void DynamicData::dynamic_rol8_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::rolValue);
}
void DynamicData::dynamic_rol8_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::rolValue);
}
void DynamicData::dynamic_rol8cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_8bit, &DynamicData::rolReg);
}
void DynamicData::dynamic_rol8cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_8bit, &DynamicData::rolReg);
}
void DynamicData::dynamic_rol16_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::rolValue);
}
void DynamicData::dynamic_rol16_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::rolValue);
}
void DynamicData::dynamic_rol16cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_16bit, &DynamicData::rolReg);
}
void DynamicData::dynamic_rol16cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_16bit, &DynamicData::rolReg);
}
void DynamicData::dynamic_rol32_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::rolValue);
}
void DynamicData::dynamic_rol32_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::rolValue);
}
void DynamicData::dynamic_rol32cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_32bit, &DynamicData::rolReg);
}
void DynamicData::dynamic_rol32cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_32bit, &DynamicData::rolReg);
}

void DynamicData::dynamic_ror8_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::rorValue);
}
void DynamicData::dynamic_ror8_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::rorValue);
}
void DynamicData::dynamic_ror8cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_8bit, &DynamicData::rorReg);
}
void DynamicData::dynamic_ror8cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_8bit, &DynamicData::rorReg);
}
void DynamicData::dynamic_ror16_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::rorValue);
}
void DynamicData::dynamic_ror16_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::rorValue);
}
void DynamicData::dynamic_ror16cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_16bit, &DynamicData::rorReg);
}
void DynamicData::dynamic_ror16cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_16bit, &DynamicData::rorReg);
}
void DynamicData::dynamic_ror32_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::rorValue);
}
void DynamicData::dynamic_ror32_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::rorValue);
}
void DynamicData::dynamic_ror32cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_32bit, &DynamicData::rorReg);
}
void DynamicData::dynamic_ror32cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_32bit, &DynamicData::rorReg);
}

void DynamicData::dynamic_rcl8_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::rclValue);
}
void DynamicData::dynamic_rcl8_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::rclValue);
}
void DynamicData::dynamic_rcl8cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_8bit, &DynamicData::rclReg);
}
void DynamicData::dynamic_rcl8cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_8bit, &DynamicData::rclReg);
}
void DynamicData::dynamic_rcl16_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::rclValue);
}
void DynamicData::dynamic_rcl16_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::rclValue);
}
void DynamicData::dynamic_rcl16cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_16bit, &DynamicData::rclReg);
}
void DynamicData::dynamic_rcl16cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_16bit, &DynamicData::rclReg);
}
void DynamicData::dynamic_rcl32_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::rclValue);
}
void DynamicData::dynamic_rcl32_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::rclValue);
}
void DynamicData::dynamic_rcl32cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_32bit, &DynamicData::rclReg);
}
void DynamicData::dynamic_rcl32cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_32bit, &DynamicData::rclReg);
}

void DynamicData::dynamic_rcr8_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::rcrValue);
}
void DynamicData::dynamic_rcr8_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::rcrValue);
}
void DynamicData::dynamic_rcr8cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_8bit, &DynamicData::rcrReg);
}
void DynamicData::dynamic_rcr8cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_8bit, &DynamicData::rcrReg);
}
void DynamicData::dynamic_rcr16_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::rcrValue);
}
void DynamicData::dynamic_rcr16_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::rcrValue);
}
void DynamicData::dynamic_rcr16cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_16bit, &DynamicData::rcrReg);
}
void DynamicData::dynamic_rcr16cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_16bit, &DynamicData::rcrReg);
}
void DynamicData::dynamic_rcr32_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::rcrValue);
}
void DynamicData::dynamic_rcr32_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::rcrValue);
}
void DynamicData::dynamic_rcr32cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_32bit, &DynamicData::rcrReg);
}
void DynamicData::dynamic_rcr32cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_32bit, &DynamicData::rcrReg);
}

void DynamicData::dynamic_shl8_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::shlValue);
}
void DynamicData::dynamic_shl8_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::shlValue);
}
void DynamicData::dynamic_shl8cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_8bit, &DynamicData::shlReg);
}
void DynamicData::dynamic_shl8cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_8bit, &DynamicData::shlReg);
}
void DynamicData::dynamic_shl16_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::shlValue);
}
void DynamicData::dynamic_shl16_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::shlValue);
}
void DynamicData::dynamic_shl16cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_16bit, &DynamicData::shlReg);
}
void DynamicData::dynamic_shl16cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_16bit, &DynamicData::shlReg);
}
void DynamicData::dynamic_shl32_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::shlValue);
}
void DynamicData::dynamic_shl32_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::shlValue);
}
void DynamicData::dynamic_shl32cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_32bit, &DynamicData::shlReg);
}
void DynamicData::dynamic_shl32cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_32bit, &DynamicData::shlReg);
}

void DynamicData::dynamic_shr8_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::shrValue);
}
void DynamicData::dynamic_shr8_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::shrValue);
}
void DynamicData::dynamic_shr8cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_8bit, &DynamicData::shrReg);
}
void DynamicData::dynamic_shr8cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_8bit, &DynamicData::shrReg);
}
void DynamicData::dynamic_shr16_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::shrValue);
}
void DynamicData::dynamic_shr16_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::shrValue);
}
void DynamicData::dynamic_shr16cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_16bit, &DynamicData::shrReg);
}
void DynamicData::dynamic_shr16cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_16bit, &DynamicData::shrReg);
}
void DynamicData::dynamic_shr32_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::shrValue);
}
void DynamicData::dynamic_shr32_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::shrValue);
}
void DynamicData::dynamic_shr32cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_32bit, &DynamicData::shrReg);
}
void DynamicData::dynamic_shr32cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_32bit, &DynamicData::shrReg);
}

void DynamicData::dynamic_sar8_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_8bit, &DynamicData::sarValue);
}
void DynamicData::dynamic_sar8_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_8bit, &DynamicData::sarValue);
}
void DynamicData::dynamic_sar8cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_8bit, &DynamicData::sarReg);
}
void DynamicData::dynamic_sar8cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_8bit, &DynamicData::sarReg);
}
void DynamicData::dynamic_sar16_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_16bit, &DynamicData::sarValue);
}
void DynamicData::dynamic_sar16_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_16bit, &DynamicData::sarValue);
}
void DynamicData::dynamic_sar16cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_16bit, &DynamicData::sarReg);
}
void DynamicData::dynamic_sar16cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_16bit, &DynamicData::sarReg);
}
void DynamicData::dynamic_sar32_reg_op(DecodedOp* op) {
    dynamic_RI(op, DYN_32bit, &DynamicData::sarValue);
}
void DynamicData::dynamic_sar32_mem_op(DecodedOp* op) {
    dynamic_MI(op, DYN_32bit, &DynamicData::sarValue);
}
void DynamicData::dynamic_sar32cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, DYN_32bit, &DynamicData::sarReg);
}
void DynamicData::dynamic_sar32cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, DYN_32bit, &DynamicData::sarReg);
}
void DynamicData::dynamic_dshlr16r16(DecodedOp* op) {
    dynamic_RRI(op, DYN_16bit, &DynamicData::shldValue);
}
void DynamicData::dynamic_dshle16r16(DecodedOp* op) {
    dynamic_MRI(op, DYN_16bit, &DynamicData::shldValue);
}
void DynamicData::dynamic_dshlr32r32(DecodedOp* op) {
    dynamic_RRI(op, DYN_32bit, &DynamicData::shldValue);
}
void DynamicData::dynamic_dshle32r32(DecodedOp* op) {
    dynamic_MRI(op, DYN_32bit, &DynamicData::shldValue);
}
void DynamicData::dynamic_dshlclr16r16(DecodedOp* op) {
    dynamic_RR_Cl(op, DYN_16bit, &DynamicData::shldReg);
}
void DynamicData::dynamic_dshlcle16r16(DecodedOp* op) {
    dynamic_MR_Cl(op, DYN_16bit, &DynamicData::shldReg);
}
void DynamicData::dynamic_dshlclr32r32(DecodedOp* op) {
    dynamic_RR_Cl(op, DYN_32bit, &DynamicData::shldReg);
}
void DynamicData::dynamic_dshlcle32r32(DecodedOp* op) {
    dynamic_MR_Cl(op, DYN_32bit, &DynamicData::shldReg);
}
void DynamicData::dynamic_dshrr16r16(DecodedOp* op) {
    dynamic_RRI(op, DYN_16bit, &DynamicData::shrdValue);
}
void DynamicData::dynamic_dshre16r16(DecodedOp* op) {
    dynamic_MRI(op, DYN_16bit, &DynamicData::shrdValue);
}
void DynamicData::dynamic_dshrr32r32(DecodedOp* op) {
    dynamic_RRI(op, DYN_32bit, &DynamicData::shrdValue);
}
void DynamicData::dynamic_dshre32r32(DecodedOp* op) {
    dynamic_MRI(op, DYN_32bit, &DynamicData::shrdValue);
}
void DynamicData::dynamic_dshrclr16r16(DecodedOp* op) {
    dynamic_RR_Cl(op, DYN_16bit, &DynamicData::shrdReg);
}
void DynamicData::dynamic_dshrcle16r16(DecodedOp* op) {
    dynamic_MR_Cl(op, DYN_16bit, &DynamicData::shrdReg);
}
void DynamicData::dynamic_dshrclr32r32(DecodedOp* op) {
    dynamic_RR_Cl(op, DYN_32bit, &DynamicData::shrdReg);
}
void DynamicData::dynamic_dshrcle32r32(DecodedOp* op) {
    dynamic_MR_Cl(op, DYN_32bit, &DynamicData::shrdReg);
}
