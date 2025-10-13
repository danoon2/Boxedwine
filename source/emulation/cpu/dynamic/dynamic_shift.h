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
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 7)) {
            incrementEip(op->len);
            return;
        }
        loadReg(op->reg, DYN_SRC, DYN_8bit);
        movToRegFromReg(DYN_DEST, DYN_8bit, DYN_SRC, DYN_8bit, false);
        shrRegImm(DYN_SRC, DYN_8bit, 8-op->imm);
        shlRegImm(DYN_DEST, DYN_8bit, op->imm);
        orRegReg(DYN_DEST, DYN_SRC, DYN_8bit, true);
        storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)rol8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol8_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 7)) {
            incrementEip(op->len);
            return;
        }
        calculateEaa(op, DYN_ADDRESS);
        movFromMem(DYN_8bit, DYN_ADDRESS, false);
        movToRegFromReg(DYN_DEST, DYN_8bit, DYN_CALL_RESULT, DYN_8bit, false);
        shrRegImm(DYN_CALL_RESULT, DYN_8bit, 8-op->imm);
        shlRegImm(DYN_DEST, DYN_8bit, op->imm);
        orRegReg(DYN_DEST, DYN_CALL_RESULT, DYN_8bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_8bit, true, true, DYN_SRC);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rol8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol8cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)rol8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol8cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rol8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol16_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 0xf)) {
            incrementEip(op->len);
            return;
        }
        loadReg(op->reg, DYN_SRC, DYN_16bit);
        movToRegFromReg(DYN_DEST, DYN_16bit, DYN_SRC, DYN_16bit, false);
        shrRegImm(DYN_SRC, DYN_16bit, 16-op->imm);
        shlRegImm(DYN_DEST, DYN_16bit, op->imm);
        orRegReg(DYN_DEST, DYN_SRC, DYN_16bit, true);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)rol16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol16_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 0xf)) {
            incrementEip(op->len);
            return;
        }
        calculateEaa(op, DYN_ADDRESS);
        movFromMem(DYN_16bit, DYN_ADDRESS, false);
        movToRegFromReg(DYN_DEST, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, false);
        shrRegImm(DYN_CALL_RESULT, DYN_16bit, 16-op->imm);
        shlRegImm(DYN_DEST, DYN_16bit, op->imm);
        orRegReg(DYN_DEST, DYN_CALL_RESULT, DYN_16bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_16bit, true, true, DYN_SRC);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rol16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol16cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)rol16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol16cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rol16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol32_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_SRC, DYN_32bit);
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_SRC, DYN_32bit, false);
        shrRegImm(DYN_SRC, DYN_32bit, 32-op->imm);
        shlRegImm(DYN_DEST, DYN_32bit, op->imm);
        orRegReg(DYN_DEST, DYN_SRC, DYN_32bit, true);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)rol32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol32_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        movFromMem(DYN_32bit, DYN_ADDRESS, false);
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, false);
        shrRegImm(DYN_CALL_RESULT, DYN_32bit, 32-op->imm);
        shlRegImm(DYN_DEST, DYN_32bit, op->imm);
        orRegReg(DYN_DEST, DYN_CALL_RESULT, DYN_32bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_32bit, true, true, DYN_SRC);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rol32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol32cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)rol32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol32cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rol32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror8_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 7)) {
            incrementEip(op->len);
            return;
        }
        loadReg(op->reg, DYN_SRC, DYN_8bit);
        movToRegFromReg(DYN_DEST, DYN_8bit, DYN_SRC, DYN_8bit, false);
        shlRegImm(DYN_SRC, DYN_8bit, 8-op->imm);
        shrRegImm(DYN_DEST, DYN_8bit, op->imm);
        orRegReg(DYN_DEST, DYN_SRC, DYN_8bit, true);
        storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)ror8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror8_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 7)) {
            incrementEip(op->len);
            return;
        }
        calculateEaa(op, DYN_ADDRESS);
        movFromMem(DYN_8bit, DYN_ADDRESS, false);
        movToRegFromReg(DYN_DEST, DYN_8bit, DYN_CALL_RESULT, DYN_8bit, false);
        shlRegImm(DYN_CALL_RESULT, DYN_8bit, 8-op->imm);
        shrRegImm(DYN_DEST, DYN_8bit, op->imm);
        orRegReg(DYN_DEST, DYN_CALL_RESULT, DYN_8bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_8bit, true, true, DYN_SRC);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)ror8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror8cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)ror8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror8cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)ror8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror16_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 0xf)) {
            incrementEip(op->len);
            return;
        }
        loadReg(op->reg, DYN_SRC, DYN_16bit);
        movToRegFromReg(DYN_DEST, DYN_16bit, DYN_SRC, DYN_16bit, false);
        shlRegImm(DYN_SRC, DYN_16bit, 16-op->imm);
        shrRegImm(DYN_DEST, DYN_16bit, op->imm);
        orRegReg(DYN_DEST, DYN_SRC, DYN_16bit, true);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)ror16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror16_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 0xf)) {
            incrementEip(op->len);
            return;
        }
        calculateEaa(op, DYN_ADDRESS);
        movFromMem(DYN_16bit, DYN_ADDRESS, false);
        movToRegFromReg(DYN_DEST, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, false);
        shlRegImm(DYN_CALL_RESULT, DYN_16bit, 16-op->imm);
        shrRegImm(DYN_DEST, DYN_16bit, op->imm);
        orRegReg(DYN_DEST, DYN_CALL_RESULT, DYN_16bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_16bit, true, true, DYN_SRC);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)ror16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror16cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)ror16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror16cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)ror16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror32_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_SRC, DYN_32bit);
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_SRC, DYN_32bit, false);
        shlRegImm(DYN_SRC, DYN_32bit, 32-op->imm);
        shrRegImm(DYN_DEST, DYN_32bit, op->imm);
        orRegReg(DYN_DEST, DYN_SRC, DYN_32bit, true);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)ror32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror32_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        movFromMem(DYN_32bit, DYN_ADDRESS, false);
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, false);
        shlRegImm(DYN_CALL_RESULT, DYN_32bit, 32-op->imm);
        shrRegImm(DYN_DEST, DYN_32bit, op->imm);
        orRegReg(DYN_DEST, DYN_CALL_RESULT, DYN_32bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_32bit, true, true, DYN_SRC);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)ror32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror32cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)ror32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror32cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)ror32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcl8_reg_op(DecodedOp* op) {
    callHostFunction((void*)rcl8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcl8_mem_op(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rcl8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcl8cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)rcl8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcl8cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rcl8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcl16_reg_op(DecodedOp* op) {
    callHostFunction((void*)rcl16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcl16_mem_op(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rcl16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcl16cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)rcl16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcl16cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rcl16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcl32_reg_op(DecodedOp* op) {
    callHostFunction((void*)rcl32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcl32_mem_op(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rcl32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcl32cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)rcl32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcl32cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rcl32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcr8_reg_op(DecodedOp* op) {
    callHostFunction((void*)rcr8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcr8_mem_op(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rcr8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcr8cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)rcr8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcr8cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rcr8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcr16_reg_op(DecodedOp* op) {
    callHostFunction((void*)rcr16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcr16_mem_op(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rcr16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcr16cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)rcr16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcr16cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rcr16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcr32_reg_op(DecodedOp* op) {
    callHostFunction((void*)rcr32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcr32_mem_op(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rcr32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcr32cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)rcr32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rcr32cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rcr32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_shl8_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        shlCPUImm(op->reg, DYN_8bit, op->imm, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)shl8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SHL8;
    incrementEip(op->len);
}
void DynamicData::dynamic_shl8_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        shlMemImm(DYN_ADDRESS, DYN_8bit, op->imm, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)shl8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SHL8;
    incrementEip(op->len);
}
void DynamicData::dynamic_shl8cl_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        shlCPUReg(op->reg, DYN_SRC, DYN_8bit, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)shl8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SHL8;
    incrementEip(op->len);
}
void DynamicData::dynamic_shl8cl_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        shlMemReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)shl8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SHL8;
    incrementEip(op->len);
}
void DynamicData::dynamic_shl16_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        shlCPUImm(op->reg, DYN_16bit, op->imm, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)shl16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SHL16;
    incrementEip(op->len);
}
void DynamicData::dynamic_shl16_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        shlMemImm(DYN_ADDRESS, DYN_16bit, op->imm, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)shl16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SHL16;
    incrementEip(op->len);
}
void DynamicData::dynamic_shl16cl_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        shlCPUReg(op->reg, DYN_SRC, DYN_16bit, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)shl16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SHL16;
    incrementEip(op->len);
}
void DynamicData::dynamic_shl16cl_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        shlMemReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)shl16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SHL16;
    incrementEip(op->len);
}
void DynamicData::dynamic_shl32_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        shlCPUImm(op->reg, DYN_32bit, op->imm, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)shl32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SHL32;
    incrementEip(op->len);
}
void DynamicData::dynamic_shl32_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        shlMemImm(DYN_ADDRESS, DYN_32bit, op->imm, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)shl32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SHL32;
    incrementEip(op->len);
}
void DynamicData::dynamic_shl32cl_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        shlCPUReg(op->reg, DYN_SRC, DYN_32bit, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)shl32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SHL32;
    incrementEip(op->len);
}
void DynamicData::dynamic_shl32cl_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        shlMemReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)shl32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SHL32;
    incrementEip(op->len);
}
void DynamicData::dynamic_shr8_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        shrCPUImm(op->reg, DYN_8bit, op->imm, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)shr8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SHR8;
    incrementEip(op->len);
}
void DynamicData::dynamic_shr8_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        shrMemImm(DYN_ADDRESS, DYN_8bit, op->imm, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)shr8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SHR8;
    incrementEip(op->len);
}
void DynamicData::dynamic_shr8cl_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        shrCPUReg(op->reg, DYN_SRC, DYN_8bit, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)shr8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SHR8;
    incrementEip(op->len);
}
void DynamicData::dynamic_shr8cl_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        shrMemReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)shr8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SHR8;
    incrementEip(op->len);
}
void DynamicData::dynamic_shr16_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        shrCPUImm(op->reg, DYN_16bit, op->imm, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)shr16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SHR16;
    incrementEip(op->len);
}
void DynamicData::dynamic_shr16_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        shrMemImm(DYN_ADDRESS, DYN_16bit, op->imm, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)shr16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SHR16;
    incrementEip(op->len);
}
void DynamicData::dynamic_shr16cl_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        shrCPUReg(op->reg, DYN_SRC, DYN_16bit, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)shr16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SHR16;
    incrementEip(op->len);
}
void DynamicData::dynamic_shr16cl_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        shrMemReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)shr16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SHR16;
    incrementEip(op->len);
}
void DynamicData::dynamic_shr32_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        shrCPUImm(op->reg, DYN_32bit, op->imm, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)shr32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SHR32;
    incrementEip(op->len);
}
void DynamicData::dynamic_shr32_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        shrMemImm(DYN_ADDRESS, DYN_32bit, op->imm, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)shr32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SHR32;
    incrementEip(op->len);
}
void DynamicData::dynamic_shr32cl_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        shrCPUReg(op->reg, DYN_SRC, DYN_32bit, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)shr32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SHR32;
    incrementEip(op->len);
}
void DynamicData::dynamic_shr32cl_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        shrMemReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)shr32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SHR32;
    incrementEip(op->len);
}
void DynamicData::dynamic_sar8_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        sarCPUImm(op->reg, DYN_8bit, op->imm, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)sar8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SAR8;
    incrementEip(op->len);
}
void DynamicData::dynamic_sar8_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        sarMemImm(DYN_ADDRESS, DYN_8bit, op->imm, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)sar8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SAR8;
    incrementEip(op->len);
}
void DynamicData::dynamic_sar8cl_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        sarCPUReg(op->reg, DYN_SRC, DYN_8bit, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)sar8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SAR8;
    incrementEip(op->len);
}
void DynamicData::dynamic_sar8cl_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        sarMemReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)sar8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SAR8;
    incrementEip(op->len);
}
void DynamicData::dynamic_sar16_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        sarCPUImm(op->reg, DYN_16bit, op->imm, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)sar16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SAR16;
    incrementEip(op->len);
}
void DynamicData::dynamic_sar16_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        sarMemImm(DYN_ADDRESS, DYN_16bit, op->imm, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)sar16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SAR16;
    incrementEip(op->len);
}
void DynamicData::dynamic_sar16cl_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        sarCPUReg(op->reg, DYN_SRC, DYN_16bit, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)sar16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SAR16;
    incrementEip(op->len);
}
void DynamicData::dynamic_sar16cl_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        sarMemReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)sar16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SAR16;
    incrementEip(op->len);
}
void DynamicData::dynamic_sar32_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        sarCPUImm(op->reg, DYN_32bit, op->imm, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)sar32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SAR32;
    incrementEip(op->len);
}
void DynamicData::dynamic_sar32_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        sarMemImm(DYN_ADDRESS, DYN_32bit, op->imm, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)sar32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_SAR32;
    incrementEip(op->len);
}
void DynamicData::dynamic_sar32cl_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        sarCPUReg(op->reg, DYN_SRC, DYN_32bit, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction((void*)sar32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SAR32;
    incrementEip(op->len);
}
void DynamicData::dynamic_sar32cl_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
        sarMemReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true, DYN_DEST);
        incrementEip(op->len);
        return;
    }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)sar32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_SAR32;
    incrementEip(op->len);
}
void DynamicData::dynamic_dshlr16r16(DecodedOp* op) {
    callHostFunction((void*)dshlr16r16, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshle16r16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)dshle16r16, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshlr32r32(DecodedOp* op) {
    callHostFunction((void*)dshlr32r32, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshle32r32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)dshle32r32, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshlclr16r16(DecodedOp* op) {
    callHostFunction((void*)dshlclr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshlcle16r16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)dshlcle16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshlclr32r32(DecodedOp* op) {
    callHostFunction((void*)dshlclr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshlcle32r32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)dshlcle32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshrr16r16(DecodedOp* op) {
    callHostFunction((void*)dshrr16r16, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshre16r16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)dshre16r16, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshrr32r32(DecodedOp* op) {
    callHostFunction((void*)dshrr32r32, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshre32r32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)dshre32r32, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshrclr16r16(DecodedOp* op) {
    callHostFunction((void*)dshrclr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshrcle16r16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)dshrcle16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshrclr32r32(DecodedOp* op) {
    callHostFunction((void*)dshrclr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshrcle32r32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)dshrcle32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}
