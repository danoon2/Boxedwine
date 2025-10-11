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
void dynamic_rol8_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        if (!(op->imm & 7)) {
            data->incrementEip(op->len);
            return;
        }
        data->loadReg(op->reg, DYN_SRC, DYN_8bit, true);
        data->movToRegFromReg(DYN_DEST, DYN_8bit, DYN_SRC, DYN_8bit, false);
        data->instRegImm('>', DYN_SRC, DYN_8bit, 8-op->imm);
        data->instRegImm('<', DYN_DEST, DYN_8bit, op->imm);
        data->instRegReg('|', DYN_DEST, DYN_SRC, DYN_8bit, true);
        data->storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)rol8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rol8_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        if (!(op->imm & 7)) {
            data->incrementEip(op->len);
            return;
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->movFromMem(DYN_8bit, DYN_ADDRESS, false);
        data->movToRegFromReg(DYN_DEST, DYN_8bit, DYN_CALL_RESULT, DYN_8bit, false);
        data->instRegImm('>', DYN_CALL_RESULT, DYN_8bit, 8-op->imm);
        data->instRegImm('<', DYN_DEST, DYN_8bit, op->imm);
        data->instRegReg('|', DYN_DEST, DYN_CALL_RESULT, DYN_8bit, true);
        data->movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_8bit, true, true, DYN_SRC);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rol8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rol8cl_reg_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)rol8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rol8cl_mem_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rol8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rol16_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        if (!(op->imm & 0xf)) {
            data->incrementEip(op->len);
            return;
        }
        data->loadReg(op->reg, DYN_SRC, DYN_16bit, true);
        data->movToRegFromReg(DYN_DEST, DYN_16bit, DYN_SRC, DYN_16bit, false);
        data->instRegImm('>', DYN_SRC, DYN_16bit, 16-op->imm);
        data->instRegImm('<', DYN_DEST, DYN_16bit, op->imm);
        data->instRegReg('|', DYN_DEST, DYN_SRC, DYN_16bit, true);
        data->storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)rol16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rol16_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        if (!(op->imm & 0xf)) {
            data->incrementEip(op->len);
            return;
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->movFromMem(DYN_16bit, DYN_ADDRESS, false);
        data->movToRegFromReg(DYN_DEST, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, false);
        data->instRegImm('>', DYN_CALL_RESULT, DYN_16bit, 16-op->imm);
        data->instRegImm('<', DYN_DEST, DYN_16bit, op->imm);
        data->instRegReg('|', DYN_DEST, DYN_CALL_RESULT, DYN_16bit, true);
        data->movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_16bit, true, true, DYN_SRC);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rol16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rol16cl_reg_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)rol16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rol16cl_mem_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rol16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rol32_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->loadReg(op->reg, DYN_SRC, DYN_32bit, true);
        data->movToRegFromReg(DYN_DEST, DYN_32bit, DYN_SRC, DYN_32bit, false);
        data->instRegImm('>', DYN_SRC, DYN_32bit, 32-op->imm);
        data->instRegImm('<', DYN_DEST, DYN_32bit, op->imm);
        data->instRegReg('|', DYN_DEST, DYN_SRC, DYN_32bit, true);
        data->storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)rol32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rol32_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->movFromMem(DYN_32bit, DYN_ADDRESS, false);
        data->movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, false);
        data->instRegImm('>', DYN_CALL_RESULT, DYN_32bit, 32-op->imm);
        data->instRegImm('<', DYN_DEST, DYN_32bit, op->imm);
        data->instRegReg('|', DYN_DEST, DYN_CALL_RESULT, DYN_32bit, true);
        data->movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_32bit, true, true, DYN_SRC);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rol32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rol32cl_reg_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)rol32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rol32cl_mem_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rol32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_ror8_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        if (!(op->imm & 7)) {
            data->incrementEip(op->len);
            return;
        }
        data->loadReg(op->reg, DYN_SRC, DYN_8bit, true);
        data->movToRegFromReg(DYN_DEST, DYN_8bit, DYN_SRC, DYN_8bit, false);
        data->instRegImm('<', DYN_SRC, DYN_8bit, 8-op->imm);
        data->instRegImm('>', DYN_DEST, DYN_8bit, op->imm);
        data->instRegReg('|', DYN_DEST, DYN_SRC, DYN_8bit, true);
        data->storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)ror8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_ror8_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        if (!(op->imm & 7)) {
            data->incrementEip(op->len);
            return;
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->movFromMem(DYN_8bit, DYN_ADDRESS, false);
        data->movToRegFromReg(DYN_DEST, DYN_8bit, DYN_CALL_RESULT, DYN_8bit, false);
        data->instRegImm('<', DYN_CALL_RESULT, DYN_8bit, 8-op->imm);
        data->instRegImm('>', DYN_DEST, DYN_8bit, op->imm);
        data->instRegReg('|', DYN_DEST, DYN_CALL_RESULT, DYN_8bit, true);
        data->movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_8bit, true, true, DYN_SRC);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)ror8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_ror8cl_reg_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)ror8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_ror8cl_mem_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)ror8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_ror16_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        if (!(op->imm & 0xf)) {
            data->incrementEip(op->len);
            return;
        }
        data->loadReg(op->reg, DYN_SRC, DYN_16bit, true);
        data->movToRegFromReg(DYN_DEST, DYN_16bit, DYN_SRC, DYN_16bit, false);
        data->instRegImm('<', DYN_SRC, DYN_16bit, 16-op->imm);
        data->instRegImm('>', DYN_DEST, DYN_16bit, op->imm);
        data->instRegReg('|', DYN_DEST, DYN_SRC, DYN_16bit, true);
        data->storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)ror16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_ror16_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        if (!(op->imm & 0xf)) {
            data->incrementEip(op->len);
            return;
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->movFromMem(DYN_16bit, DYN_ADDRESS, false);
        data->movToRegFromReg(DYN_DEST, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, false);
        data->instRegImm('<', DYN_CALL_RESULT, DYN_16bit, 16-op->imm);
        data->instRegImm('>', DYN_DEST, DYN_16bit, op->imm);
        data->instRegReg('|', DYN_DEST, DYN_CALL_RESULT, DYN_16bit, true);
        data->movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_16bit, true, true, DYN_SRC);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)ror16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_ror16cl_reg_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)ror16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_ror16cl_mem_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)ror16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_ror32_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->loadReg(op->reg, DYN_SRC, DYN_32bit, true);
        data->movToRegFromReg(DYN_DEST, DYN_32bit, DYN_SRC, DYN_32bit, false);
        data->instRegImm('<', DYN_SRC, DYN_32bit, 32-op->imm);
        data->instRegImm('>', DYN_DEST, DYN_32bit, op->imm);
        data->instRegReg('|', DYN_DEST, DYN_SRC, DYN_32bit, true);
        data->storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)ror32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_ror32_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->movFromMem(DYN_32bit, DYN_ADDRESS, false);
        data->movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, false);
        data->instRegImm('<', DYN_CALL_RESULT, DYN_32bit, 32-op->imm);
        data->instRegImm('>', DYN_DEST, DYN_32bit, op->imm);
        data->instRegReg('|', DYN_DEST, DYN_CALL_RESULT, DYN_32bit, true);
        data->movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_32bit, true, true, DYN_SRC);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)ror32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_ror32cl_reg_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)ror32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_ror32cl_mem_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)ror32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcl8_reg_op(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)rcl8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcl8_mem_op(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rcl8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcl8cl_reg_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)rcl8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcl8cl_mem_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rcl8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcl16_reg_op(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)rcl16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcl16_mem_op(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rcl16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcl16cl_reg_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)rcl16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcl16cl_mem_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rcl16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcl32_reg_op(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)rcl32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcl32_mem_op(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rcl32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcl32cl_reg_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)rcl32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcl32cl_mem_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rcl32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcr8_reg_op(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)rcr8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcr8_mem_op(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rcr8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcr8cl_reg_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)rcr8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcr8cl_mem_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rcr8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcr16_reg_op(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)rcr16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcr16_mem_op(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rcr16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcr16cl_reg_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)rcr16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcr16cl_mem_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rcr16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcr32_reg_op(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)rcr32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcr32_mem_op(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rcr32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcr32cl_reg_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)rcr32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_rcr32cl_mem_op(DynamicData* data, DecodedOp* op) {
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)rcr32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_shl8_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->instCPUImm('<', op->reg, DYN_8bit, op->imm, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)shl8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHL8;
    data->incrementEip(op->len);
}
void dynamic_shl8_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm('<', DYN_ADDRESS, DYN_8bit, op->imm, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)shl8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHL8;
    data->incrementEip(op->len);
}
void dynamic_shl8cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instCPUReg('<', op->reg, reg, DYN_8bit, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)shl8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHL8;
    data->incrementEip(op->len);
}
void dynamic_shl8cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instMemReg('<', DYN_ADDRESS, reg, DYN_8bit, true, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)shl8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHL8;
    data->incrementEip(op->len);
}
void dynamic_shl16_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->instCPUImm('<', op->reg, DYN_16bit, op->imm, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)shl16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHL16;
    data->incrementEip(op->len);
}
void dynamic_shl16_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm('<', DYN_ADDRESS, DYN_16bit, op->imm, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)shl16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHL16;
    data->incrementEip(op->len);
}
void dynamic_shl16cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instCPUReg('<', op->reg, reg, DYN_16bit, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)shl16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHL16;
    data->incrementEip(op->len);
}
void dynamic_shl16cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instMemReg('<', DYN_ADDRESS, reg, DYN_16bit, true, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)shl16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHL16;
    data->incrementEip(op->len);
}
void dynamic_shl32_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->instCPUImm('<', op->reg, DYN_32bit, op->imm, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)shl32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHL32;
    data->incrementEip(op->len);
}
void dynamic_shl32_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm('<', DYN_ADDRESS, DYN_32bit, op->imm, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)shl32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHL32;
    data->incrementEip(op->len);
}
void dynamic_shl32cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instCPUReg('<', op->reg, reg, DYN_32bit, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)shl32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHL32;
    data->incrementEip(op->len);
}
void dynamic_shl32cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instMemReg('<', DYN_ADDRESS, reg, DYN_32bit, true, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)shl32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHL32;
    data->incrementEip(op->len);
}
void dynamic_shr8_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->instCPUImm('>', op->reg, DYN_8bit, op->imm, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)shr8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHR8;
    data->incrementEip(op->len);
}
void dynamic_shr8_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm('>', DYN_ADDRESS, DYN_8bit, op->imm, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)shr8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHR8;
    data->incrementEip(op->len);
}
void dynamic_shr8cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instCPUReg('>', op->reg, reg, DYN_8bit, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)shr8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHR8;
    data->incrementEip(op->len);
}
void dynamic_shr8cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instMemReg('>', DYN_ADDRESS, reg, DYN_8bit, true, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)shr8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHR8;
    data->incrementEip(op->len);
}
void dynamic_shr16_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->instCPUImm('>', op->reg, DYN_16bit, op->imm, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)shr16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHR16;
    data->incrementEip(op->len);
}
void dynamic_shr16_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm('>', DYN_ADDRESS, DYN_16bit, op->imm, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)shr16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHR16;
    data->incrementEip(op->len);
}
void dynamic_shr16cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instCPUReg('>', op->reg, reg, DYN_16bit, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)shr16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHR16;
    data->incrementEip(op->len);
}
void dynamic_shr16cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instMemReg('>', DYN_ADDRESS, reg, DYN_16bit, true, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)shr16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHR16;
    data->incrementEip(op->len);
}
void dynamic_shr32_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->instCPUImm('>', op->reg, DYN_32bit, op->imm, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)shr32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHR32;
    data->incrementEip(op->len);
}
void dynamic_shr32_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm('>', DYN_ADDRESS, DYN_32bit, op->imm, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)shr32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHR32;
    data->incrementEip(op->len);
}
void dynamic_shr32cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instCPUReg('>', op->reg, reg, DYN_32bit, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)shr32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHR32;
    data->incrementEip(op->len);
}
void dynamic_shr32cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instMemReg('>', DYN_ADDRESS, reg, DYN_32bit, true, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)shr32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHR32;
    data->incrementEip(op->len);
}
void dynamic_sar8_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->instCPUImm(')', op->reg, DYN_8bit, op->imm, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)sar8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SAR8;
    data->incrementEip(op->len);
}
void dynamic_sar8_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm(')', DYN_ADDRESS, DYN_8bit, op->imm, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)sar8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SAR8;
    data->incrementEip(op->len);
}
void dynamic_sar8cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instCPUReg(')', op->reg, reg, DYN_8bit, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)sar8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SAR8;
    data->incrementEip(op->len);
}
void dynamic_sar8cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instMemReg(')', DYN_ADDRESS, reg, DYN_8bit, true, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)sar8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SAR8;
    data->incrementEip(op->len);
}
void dynamic_sar16_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->instCPUImm(')', op->reg, DYN_16bit, op->imm, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)sar16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SAR16;
    data->incrementEip(op->len);
}
void dynamic_sar16_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm(')', DYN_ADDRESS, DYN_16bit, op->imm, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)sar16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SAR16;
    data->incrementEip(op->len);
}
void dynamic_sar16cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instCPUReg(')', op->reg, reg, DYN_16bit, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)sar16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SAR16;
    data->incrementEip(op->len);
}
void dynamic_sar16cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instMemReg(')', DYN_ADDRESS, reg, DYN_16bit, true, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)sar16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SAR16;
    data->incrementEip(op->len);
}
void dynamic_sar32_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->instCPUImm(')', op->reg, DYN_32bit, op->imm, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->callHostFunction((void*)sar32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SAR32;
    data->incrementEip(op->len);
}
void dynamic_sar32_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm(')', DYN_ADDRESS, DYN_32bit, op->imm, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)sar32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SAR32;
    data->incrementEip(op->len);
}
void dynamic_sar32cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instCPUReg(')', op->reg, reg, DYN_32bit, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->callHostFunction((void*)sar32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SAR32;
    data->incrementEip(op->len);
}
void dynamic_sar32cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->calculateEaa(op, DYN_ADDRESS);
        DynReg reg = data->loadReg(1, DYN_SRC, DYN_8bit);
        data->instMemReg(')', DYN_ADDRESS, reg, DYN_32bit, true, true, DYN_DEST);
        data->incrementEip(op->len);
        return;
    }
    data->loadReg(1, DYN_SRC, DYN_8bit, true);
    data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)sar32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SAR32;
    data->incrementEip(op->len);
}
void dynamic_dshlr16r16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)dshlr16r16, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_dshle16r16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)dshle16r16, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_dshlr32r32(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)dshlr32r32, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_dshle32r32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)dshle32r32, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_dshlclr16r16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)dshlclr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_dshlcle16r16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)dshlcle16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}
void dynamic_dshlclr32r32(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)dshlclr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_dshlcle32r32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)dshlcle32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}
void dynamic_dshrr16r16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)dshrr16r16, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_dshre16r16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)dshre16r16, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_dshrr32r32(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)dshrr32r32, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_dshre32r32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)dshre32r32, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_dshrclr16r16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)dshrclr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_dshrcle16r16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)dshrcle16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}
void dynamic_dshrclr32r32(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)dshrclr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_dshrcle32r32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)dshrcle32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}
