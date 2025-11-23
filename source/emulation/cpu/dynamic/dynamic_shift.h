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
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_II(rol8_reg, op->reg, op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_RI(op, JitWidth::b8, &DynamicData::rolValue, nullptr);
    }
}
void DynamicData::dynamic_rol8_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RI(rol8_mem, JitWidth::b32, calculateEaaV2(op), op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_MI(op, JitWidth::b8, &DynamicData::rolValue, nullptr);
    }
}
void DynamicData::dynamic_rol8cl_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_IR(rol8cl_reg, op->reg, JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_R_Cl(op, JitWidth::b8, &DynamicData::rolReg, nullptr);
    }
}
void DynamicData::dynamic_rol8cl_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RR(rol8cl_mem, JitWidth::b32, calculateEaaV2(op), JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_M_Cl(op, JitWidth::b8, &DynamicData::rolReg, nullptr);
    }
}
void DynamicData::dynamic_rol16_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_II(rol16_reg, op->reg, op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_RI(op, JitWidth::b16, &DynamicData::rolValue, nullptr);
    }
}
void DynamicData::dynamic_rol16_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RI(rol16_mem, JitWidth::b32, calculateEaaV2(op), op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_MI(op, JitWidth::b16, &DynamicData::rolValue, nullptr);
    }
}
void DynamicData::dynamic_rol16cl_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_IR(rol16cl_reg, op->reg, JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_R_Cl(op, JitWidth::b16, &DynamicData::rolReg, nullptr);
    }
}
void DynamicData::dynamic_rol16cl_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RR(rol16cl_mem, JitWidth::b32, calculateEaaV2(op), JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_M_Cl(op, JitWidth::b16, &DynamicData::rolReg, nullptr);
    }
}
void DynamicData::dynamic_rol32_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_II(rol32_reg, op->reg, op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_RI(op, JitWidth::b32, &DynamicData::rolValue, nullptr);
    }
}
void DynamicData::dynamic_rol32_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RI(rol32_mem, JitWidth::b32, calculateEaaV2(op), op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_MI(op, JitWidth::b32, &DynamicData::rolValue, nullptr);
    }
}
void DynamicData::dynamic_rol32cl_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_IR(rol32cl_reg, op->reg, JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_R_Cl(op, JitWidth::b32, &DynamicData::rolReg, nullptr);
    }
}
void DynamicData::dynamic_rol32cl_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RR(rol32cl_mem, JitWidth::b32, calculateEaaV2(op), JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_M_Cl(op, JitWidth::b32, &DynamicData::rolReg, nullptr);
    }
}
void DynamicData::dynamic_ror8_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_II(ror8_reg, op->reg, op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_RI(op, JitWidth::b8, &DynamicData::rorValue, nullptr);
    }
}
void DynamicData::dynamic_ror8_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RI(ror8_mem, JitWidth::b32, calculateEaaV2(op), op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_MI(op, JitWidth::b8, &DynamicData::rorValue, nullptr);
    }
}
void DynamicData::dynamic_ror8cl_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_IR(ror8cl_reg, op->reg, JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_R_Cl(op, JitWidth::b8, &DynamicData::rorReg, nullptr);
    }
}
void DynamicData::dynamic_ror8cl_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RR(ror8cl_mem, JitWidth::b32, calculateEaaV2(op), JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_M_Cl(op, JitWidth::b8, &DynamicData::rorReg, nullptr);
    }
}
void DynamicData::dynamic_ror16_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_II(ror16_reg, op->reg, op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_RI(op, JitWidth::b16, &DynamicData::rorValue, nullptr);
    }
}
void DynamicData::dynamic_ror16_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RI(ror16_mem, JitWidth::b32, calculateEaaV2(op), op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_MI(op, JitWidth::b16, &DynamicData::rorValue, nullptr);
    }
}
void DynamicData::dynamic_ror16cl_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_IR(ror16cl_reg, op->reg, JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_R_Cl(op, JitWidth::b16, &DynamicData::rorReg, nullptr);
    }
}
void DynamicData::dynamic_ror16cl_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RR(ror16cl_mem, JitWidth::b32, calculateEaaV2(op), JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_M_Cl(op, JitWidth::b16, &DynamicData::rorReg, nullptr);
    }
}
void DynamicData::dynamic_ror32_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_II(ror32_reg, op->reg, op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_RI(op, JitWidth::b32, &DynamicData::rorValue, nullptr);
    }
}
void DynamicData::dynamic_ror32_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RI(ror32_mem, JitWidth::b32, calculateEaaV2(op), op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_MI(op, JitWidth::b32, &DynamicData::rorValue, nullptr);
    }
}
void DynamicData::dynamic_ror32cl_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_IR(ror32cl_reg, op->reg, JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_R_Cl(op, JitWidth::b32, &DynamicData::rorReg, nullptr);
    }
}
void DynamicData::dynamic_ror32cl_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RR(ror32cl_mem, JitWidth::b32, calculateEaaV2(op), JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_M_Cl(op, JitWidth::b32, &DynamicData::rorReg, nullptr);
    }
}
void DynamicData::dynamic_rcl8_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_II(rcl8_reg, op->reg, op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_RI(op, JitWidth::b8, &DynamicData::rclValue, nullptr);
    }
}
void DynamicData::dynamic_rcl8_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RI(rcl8_mem, JitWidth::b32, calculateEaaV2(op), op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_MI(op, JitWidth::b8, &DynamicData::rclValue, nullptr);
    }
}
void DynamicData::dynamic_rcl8cl_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_IR(rcl8cl_reg, op->reg, JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_R_Cl(op, JitWidth::b8, &DynamicData::rclReg, nullptr);
    }
}
void DynamicData::dynamic_rcl8cl_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RR(rcl8cl_mem, JitWidth::b32, calculateEaaV2(op), JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_M_Cl(op, JitWidth::b8, &DynamicData::rclReg, nullptr);
    }
}
void DynamicData::dynamic_rcl16_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_II(rcl16_reg, op->reg, op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_RI(op, JitWidth::b16, &DynamicData::rclValue, nullptr);
    }
}
void DynamicData::dynamic_rcl16_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RI(rcl16_mem, JitWidth::b32, calculateEaaV2(op), op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_MI(op, JitWidth::b16, &DynamicData::rclValue, nullptr);
    }
}
void DynamicData::dynamic_rcl16cl_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_IR(rcl16cl_reg, op->reg, JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_R_Cl(op, JitWidth::b16, &DynamicData::rclReg, nullptr);
    }
}
void DynamicData::dynamic_rcl16cl_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RR(rcl16cl_mem, JitWidth::b32, calculateEaaV2(op), JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_M_Cl(op, JitWidth::b16, &DynamicData::rclReg, nullptr);
    }
}
void DynamicData::dynamic_rcl32_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_II(rcl32_reg, op->reg, op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_RI(op, JitWidth::b32, &DynamicData::rclValue, nullptr);
    }
}
void DynamicData::dynamic_rcl32_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RI(rcl32_mem, JitWidth::b32, calculateEaaV2(op), op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_MI(op, JitWidth::b32, &DynamicData::rclValue, nullptr);
    }
}
void DynamicData::dynamic_rcl32cl_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_IR(rcl32cl_reg, op->reg, JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_R_Cl(op, JitWidth::b32, &DynamicData::rclReg, nullptr);
    }
}
void DynamicData::dynamic_rcl32cl_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RR(rcl32cl_mem, JitWidth::b32, calculateEaaV2(op), JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_M_Cl(op, JitWidth::b32, &DynamicData::rclReg, nullptr);
    }
}
void DynamicData::dynamic_rcr8_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_II(rcr8_reg, op->reg, op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_RI(op, JitWidth::b8, &DynamicData::rcrValue, nullptr);
    }
}
void DynamicData::dynamic_rcr8_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RI(rcr8_mem, JitWidth::b32, calculateEaaV2(op), op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_MI(op, JitWidth::b8, &DynamicData::rcrValue, nullptr);
    }
    currentLazyFlags = FLAGS_NONE;
}
void DynamicData::dynamic_rcr8cl_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_IR(rcr8cl_reg, op->reg, JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_R_Cl(op, JitWidth::b8, &DynamicData::rcrReg, nullptr);
    }
    currentLazyFlags = FLAGS_NONE;
}
void DynamicData::dynamic_rcr8cl_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RR(rcr8cl_mem, JitWidth::b32, calculateEaaV2(op), JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_M_Cl(op, JitWidth::b8, &DynamicData::rcrReg, nullptr);
    }
}
void DynamicData::dynamic_rcr16_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_II(rcr16_reg, op->reg, op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_RI(op, JitWidth::b16, &DynamicData::rcrValue, nullptr);
    }
}
void DynamicData::dynamic_rcr16_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RI(rcr16_mem, JitWidth::b32, calculateEaaV2(op), op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_MI(op, JitWidth::b16, &DynamicData::rcrValue, nullptr);
    }
}
void DynamicData::dynamic_rcr16cl_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_IR(rcr16cl_reg, op->reg, JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_R_Cl(op, JitWidth::b16, &DynamicData::rcrReg, nullptr);
    }
}
void DynamicData::dynamic_rcr16cl_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RR(rcr16cl_mem, JitWidth::b32, calculateEaaV2(op), JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_M_Cl(op, JitWidth::b16, &DynamicData::rcrReg, nullptr);
    }
}
void DynamicData::dynamic_rcr32_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_II(rcr32_reg, op->reg, op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_RI(op, JitWidth::b32, &DynamicData::rcrValue, nullptr);
    }
}
void DynamicData::dynamic_rcr32_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RI(rcr32_mem, JitWidth::b32, calculateEaaV2(op), op->imm);
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_MI(op, JitWidth::b32, &DynamicData::rcrValue, nullptr);
    }
}
void DynamicData::dynamic_rcr32cl_reg_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_IR(rcr32cl_reg, op->reg, JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_R_Cl(op, JitWidth::b32, &DynamicData::rcrReg, nullptr);
    }
}
void DynamicData::dynamic_rcr32cl_mem_op(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & (CF | OF)) {
        call_RR(rcr32cl_mem, JitWidth::b32, calculateEaaV2(op), JitWidth::b8, getReadOnlyReg8(1));
        currentLazyFlags = FLAGS_NONE;
        incrementEip(op->len);
    } else {
        dynamic_M_Cl(op, JitWidth::b32, &DynamicData::rcrReg, nullptr);
    }
}
void DynamicData::dynamic_shl8_reg_op(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b8, &DynamicData::shlValue, FLAGS_SHL8);
}
void DynamicData::dynamic_shl8_mem_op(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b8, &DynamicData::shlValue, FLAGS_SHL8);
}
void DynamicData::dynamic_shl8cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, JitWidth::b8, &DynamicData::shlReg, FLAGS_SHL8);
}
void DynamicData::dynamic_shl8cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, JitWidth::b8, &DynamicData::shlReg, FLAGS_SHL8);
}
void DynamicData::dynamic_shl16_reg_op(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b16, &DynamicData::shlValue, FLAGS_SHL16);
}
void DynamicData::dynamic_shl16_mem_op(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b16, &DynamicData::shlValue, FLAGS_SHL16);
}
void DynamicData::dynamic_shl16cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, JitWidth::b16, &DynamicData::shlReg, FLAGS_SHL16);
}
void DynamicData::dynamic_shl16cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, JitWidth::b16, &DynamicData::shlReg, FLAGS_SHL16);
}
void DynamicData::dynamic_shl32_reg_op(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b32, &DynamicData::shlValue, FLAGS_SHL32);
}
void DynamicData::dynamic_shl32_mem_op(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b32, &DynamicData::shlValue, FLAGS_SHL32);
}
void DynamicData::dynamic_shl32cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, JitWidth::b32, &DynamicData::shlReg, FLAGS_SHL32);
}
void DynamicData::dynamic_shl32cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, JitWidth::b32, &DynamicData::shlReg, FLAGS_SHL32);
}
void DynamicData::dynamic_shr8_reg_op(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b8, &DynamicData::shrValue, FLAGS_SHR8);
}
void DynamicData::dynamic_shr8_mem_op(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b8, &DynamicData::shrValue, FLAGS_SHR8);
}
void DynamicData::dynamic_shr8cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, JitWidth::b8, &DynamicData::shrReg, FLAGS_SHR8);
}
void DynamicData::dynamic_shr8cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, JitWidth::b8, &DynamicData::shrReg, FLAGS_SHR8);
}
void DynamicData::dynamic_shr16_reg_op(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b16, &DynamicData::shrValue, FLAGS_SHR16);
}
void DynamicData::dynamic_shr16_mem_op(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b16, &DynamicData::shrValue, FLAGS_SHR16);
}
void DynamicData::dynamic_shr16cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, JitWidth::b16, &DynamicData::shrReg, FLAGS_SHR16);
}
void DynamicData::dynamic_shr16cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, JitWidth::b16, &DynamicData::shrReg, FLAGS_SHR16);
}
void DynamicData::dynamic_shr32_reg_op(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b32, &DynamicData::shrValue, FLAGS_SHR32);
}
void DynamicData::dynamic_shr32_mem_op(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b32, &DynamicData::shrValue, FLAGS_SHR32);
}
void DynamicData::dynamic_shr32cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, JitWidth::b32, &DynamicData::shrReg, FLAGS_SHR32);
}
void DynamicData::dynamic_shr32cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, JitWidth::b32, &DynamicData::shrReg, FLAGS_SHR32);
}
void DynamicData::dynamic_sar8_reg_op(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b8, &DynamicData::sarValue, FLAGS_SAR8);
}
void DynamicData::dynamic_sar8_mem_op(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b8, &DynamicData::sarValue, FLAGS_SAR8);
}
void DynamicData::dynamic_sar8cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, JitWidth::b8, &DynamicData::sarReg, FLAGS_SAR8);
}
void DynamicData::dynamic_sar8cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, JitWidth::b8, &DynamicData::sarReg, FLAGS_SAR8);
}
void DynamicData::dynamic_sar16_reg_op(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b16, &DynamicData::sarValue, FLAGS_SAR16);
}
void DynamicData::dynamic_sar16_mem_op(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b16, &DynamicData::sarValue, FLAGS_SAR16);
}
void DynamicData::dynamic_sar16cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, JitWidth::b16, &DynamicData::sarReg, FLAGS_SAR16);
}
void DynamicData::dynamic_sar16cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, JitWidth::b16, &DynamicData::sarReg, FLAGS_SAR16);
}
void DynamicData::dynamic_sar32_reg_op(DecodedOp* op) {
    dynamic_RI(op, JitWidth::b32, &DynamicData::sarValue, FLAGS_SAR32);
}
void DynamicData::dynamic_sar32_mem_op(DecodedOp* op) {
    dynamic_MI(op, JitWidth::b32, &DynamicData::sarValue, FLAGS_SAR32);
}
void DynamicData::dynamic_sar32cl_reg_op(DecodedOp* op) {
    dynamic_R_Cl(op, JitWidth::b32, &DynamicData::sarReg, FLAGS_SAR32);
}
void DynamicData::dynamic_sar32cl_mem_op(DecodedOp* op) {
    dynamic_M_Cl(op, JitWidth::b32, &DynamicData::sarReg, FLAGS_SAR32);
}

void DynamicData::dshift(DecodedOp* op, JitWidth width, InstRegRegImm callback, const LazyFlags* flags) {
    RegPtr dest = getReg(op->reg);
    RegPtr src2 = op->reg == op->rm ? dest : getReadOnlyReg(op->rm);
    U32 needsToSetFlags = op->needsToSetFlags(cpu);

    if (needsToSetFlags) {
        storeLazyFlags(flags);
        currentLazyFlags = flags;
        if (flags && flags->usesSrc(needsToSetFlags)) {
            storeLazyFlagsSrc(op->imm);
        }
        if (flags && flags->usesDst(needsToSetFlags)) {
            storeLazyFlagsDest(dest);
        }
    }
    (this->*callback)(width, dest, src2, op->imm);
    if (flags && flags->usesResult(needsToSetFlags)) {
        storeLazyFlagsResult(dest);
    }
    incrementEip(op->len);
}

void DynamicData::dshiftM(DecodedOp* op, JitWidth width, InstRegRegImm callback, const LazyFlags* flags) {
    RegPtr src2 = getReadOnlyReg(op->reg);
    U32 needsToSetFlags = op->needsToSetFlags(cpu);

    if (needsToSetFlags) {
        storeLazyFlags(flags);
        currentLazyFlags = flags;
        if (flags && flags->usesSrc(needsToSetFlags)) {
            storeLazyFlagsSrc(op->imm);
        }
    }

    readWriteMem(width, calculateEaaV2(op), [needsToSetFlags, flags, src2, op, width, callback, this](RegPtr value) {
        if (flags && flags->usesDst(needsToSetFlags)) {
            storeLazyFlagsDest(value);
        }
        (this->*callback)(width, value, src2, op->imm);
        if (flags && flags->usesResult(needsToSetFlags)) {
            storeLazyFlagsResult(value);
        }
        });
    incrementEip(op->len);
}

void DynamicData::dynamic_dshlr16r16(DecodedOp* op) {
    dshift(op, JitWidth::b16, &DynamicData::shldValue, FLAGS_DSHL16);
}
void DynamicData::dynamic_dshle16r16(DecodedOp* op) {
    dshiftM(op, JitWidth::b16, &DynamicData::shldValue, FLAGS_DSHL16);
}
void DynamicData::dynamic_dshlr32r32(DecodedOp* op) {
    dshift(op, JitWidth::b32, &DynamicData::shldValue, FLAGS_DSHL32);
}
void DynamicData::dynamic_dshle32r32(DecodedOp* op) {
    dshiftM(op, JitWidth::b32, &DynamicData::shldValue, FLAGS_DSHL32);
}
void DynamicData::dshiftClM(DecodedOp* op, JitWidth width, InstRegRegCl callback, const LazyFlags* flags) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);

    if (needsToSetFlags) {
        RegPtr src = getTmpReg8(1, false, 1);
        andValue(JitWidth::b32, src, 0x1f);
        If(JitWidth::b8, src, true); {
            storeLazyFlags(flags);
            if (flags && flags->usesSrc(needsToSetFlags)) {
                storeLazyFlagsSrc(src);
            }
            readWriteMem(width, calculateEaaV2(op), [src, needsToSetFlags, flags, op, width, callback, this](RegPtr value) {
                if (flags && flags->usesDst(needsToSetFlags)) {
                    storeLazyFlagsDest(value);
                }
                (this->*callback)(width, value, getReadOnlyReg(op->reg), src);
                if (flags && flags->usesResult(needsToSetFlags)) {
                    storeLazyFlagsResult(value);
                }
                });
        } EndIf(true);
        currentLazyFlags = flags;
    } else {
        readWriteMem(width, calculateEaaV2(op), [op, width, callback, this](RegPtr value) {
            (this->*callback)(width, value, getReadOnlyReg(op->reg), getReadOnlyReg(1, true, 1));
            });
    }
    incrementEip(op->len);
}
void DynamicData::dshiftCl(DecodedOp* op, JitWidth width, InstRegRegCl callback, const LazyFlags* flags) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);

    if (needsToSetFlags) {
        RegPtr src = getTmpReg8(1, false, 1);

        andValue(JitWidth::b32, src, 0x1f);
        If(JitWidth::b8, src); {
            RegPtr dest = getReg(op->reg);
            RegPtr src2 = op->reg == op->rm ? dest : getReadOnlyReg(op->rm);

            storeLazyFlags(flags);
            if (flags && flags->usesSrc(needsToSetFlags)) {
                storeLazyFlagsSrc(src);
            }
            if (flags && flags->usesDst(needsToSetFlags)) {
                storeLazyFlagsDest(dest);
            }
            (this->*callback)(width, dest, src2, src);
            if (flags && flags->usesResult(needsToSetFlags)) {
                storeLazyFlagsResult(dest);
            }
        } EndIf();
        currentLazyFlags = flags;
    } else {
        (this->*callback)(width, getReg(op->reg), getReadOnlyReg(op->rm), getReadOnlyReg(1, true, 1));
    }
    incrementEip(op->len);
}

void DynamicData::dynamic_dshlclr16r16(DecodedOp* op) {
    dshiftCl(op, JitWidth::b16, &DynamicData::shldReg, FLAGS_DSHL16);
}
void DynamicData::dynamic_dshlcle16r16(DecodedOp* op) {
    dshiftClM(op, JitWidth::b16, &DynamicData::shldReg, FLAGS_DSHL16);
}
void DynamicData::dynamic_dshlclr32r32(DecodedOp* op) {
    dshiftCl(op, JitWidth::b32, &DynamicData::shldReg, FLAGS_DSHL32);
}
void DynamicData::dynamic_dshlcle32r32(DecodedOp* op) {
    dshiftClM(op, JitWidth::b32, &DynamicData::shldReg, FLAGS_DSHL32);
}
void DynamicData::dynamic_dshrr16r16(DecodedOp* op) {
    dshift(op, JitWidth::b16, &DynamicData::shrdValue, FLAGS_DSHR16);
}
void DynamicData::dynamic_dshre16r16(DecodedOp* op) {
    dshiftM(op, JitWidth::b16, &DynamicData::shrdValue, FLAGS_DSHR16);
}
void DynamicData::dynamic_dshrr32r32(DecodedOp* op) {
    dshift(op, JitWidth::b32, &DynamicData::shrdValue, FLAGS_DSHR32);
}
void DynamicData::dynamic_dshre32r32(DecodedOp* op) {
    dshiftM(op, JitWidth::b32, &DynamicData::shrdValue, FLAGS_DSHR32);
}
void DynamicData::dynamic_dshrclr16r16(DecodedOp* op) {
    dshiftCl(op, JitWidth::b16, &DynamicData::shrdReg, FLAGS_DSHR16);
}
void DynamicData::dynamic_dshrcle16r16(DecodedOp* op) {
    dshiftClM(op, JitWidth::b16, &DynamicData::shrdReg, FLAGS_DSHR16);
}
void DynamicData::dynamic_dshrclr32r32(DecodedOp* op) {
    dshiftCl(op, JitWidth::b32, &DynamicData::shrdReg, FLAGS_DSHR32);
}
void DynamicData::dynamic_dshrcle32r32(DecodedOp* op) {
    dshiftClM(op, JitWidth::b32, &DynamicData::shrdReg, FLAGS_DSHR32);
}
