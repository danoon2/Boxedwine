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
        if (!(op->imm & 0x7)) {
            incrementEip(op->len);
            return;
        }
        loadReg(op->reg, DYN_DEST, DYN_8bit);
        rolRegImm(DYN_DEST, DYN_8bit, op->imm);
        storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)rol8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol8_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 0x7)) {
            incrementEip(op->len);
            return;
        }
        calculateEaa(op, DYN_ADDRESS);
        readWriteMem(DYN_8bit, DYN_ADDRESS, DYN_DEST, true, [op, this]() {
            rolRegImm(DYN_CALL_RESULT, DYN_8bit, op->imm);
        });
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rol8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol8cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_DEST, DYN_8bit);
        rolRegReg(DYN_DEST, DYN_SRC, DYN_8bit, true);
        storeReg(op->reg, DYN_DEST, DYN_8bit, true);
    } else {
        // might as well call rol8cl_reg, since in this case we would need to call fillFlags
        callHostFunction((void*)rol8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_rol8cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(cpu)) {
        readWriteMem(DYN_8bit, DYN_ADDRESS, DYN_DEST, true, [this]() {
            rolRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_8bit, true);
        });
    } else {
        callHostFunction((void*)rol8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_rol16_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 0xf)) {
            incrementEip(op->len);
            return;
        }
        loadReg(op->reg, DYN_DEST, DYN_16bit);
        rolRegImm(DYN_DEST, DYN_16bit, op->imm);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)rol16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol16_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 0xf)) {
            incrementEip(op->len);
            return;
        }
        calculateEaa(op, DYN_ADDRESS);
        readWriteMem(DYN_16bit, DYN_ADDRESS, DYN_DEST, true, [op, this]() {
            rolRegImm(DYN_CALL_RESULT, DYN_16bit, op->imm);
        });
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rol16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol16cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_DEST, DYN_16bit);
        rolRegReg(DYN_DEST, DYN_SRC, DYN_16bit, true);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
    } else {
        // might as well call rol16cl_reg, since in this case we would need to call fillFlags
        callHostFunction((void*)rol16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_rol16cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(cpu)) {
        readWriteMem(DYN_16bit, DYN_ADDRESS, DYN_DEST, true, [this]() {
            rolRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_16bit, true);
        });
    } else {
        callHostFunction((void*)rol16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_rol32_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        rolRegImm(DYN_DEST, DYN_32bit, op->imm);
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
        readWriteMem(DYN_32bit, DYN_ADDRESS, DYN_DEST, true, [op, this]() {
            rolRegImm(DYN_CALL_RESULT, DYN_32bit, op->imm);
        });
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)rol32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_rol32cl_reg_op(DecodedOp* op) {
    // if (var2) {
    //     U32 var1 = cpu->reg[reg].u32;
    //     cpu->fillFlagsNoCFOF();
    //     U32 result = (var1 << var2) | (var1 >> (32 - var2));
    //     cpu->setCF(result & 1);
    //     cpu->setOF((result & 1) ^ (result >> 31));
    //     cpu->reg[reg].u32 = result;
    // }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        rolRegReg(DYN_DEST, DYN_SRC, DYN_32bit, true);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
    } else {
        // might as well call rol32cl_reg, since in this case we would need to call fillFlags
        callHostFunction((void*)rol32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_rol32cl_mem_op(DecodedOp* op) {
    // if (var2) {
    //     U32 var1 = cpu->memory->readd(eaa);
    //     cpu->fillFlagsNoCFOF();
    //     U32 result = (var1 << var2) | (var1 >> (32 - var2));
    //     cpu->setCF(result & 1);
    //     cpu->setOF((result & 1) ^ (result >> 31));
    //     cpu->memory->writed(eaa, result);
    // }
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(cpu)) {
        readWriteMem(DYN_32bit, DYN_ADDRESS, DYN_DEST, true, [this]() {
            rolRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_32bit, true);
        });
    } else {
        callHostFunction((void*)rol32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_ror8_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 0x7)) {
            incrementEip(op->len);
            return;
        }
        loadReg(op->reg, DYN_DEST, DYN_8bit);
        rorRegImm(DYN_DEST, DYN_8bit, op->imm);
        storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)ror8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror8_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 0x7)) {
            incrementEip(op->len);
            return;
        }
        calculateEaa(op, DYN_ADDRESS);
        readWriteMem(DYN_8bit, DYN_ADDRESS, DYN_DEST, true, [op, this]() {
            rorRegImm(DYN_CALL_RESULT, DYN_8bit, op->imm);
        });
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)ror8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror8cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_DEST, DYN_8bit);
        rorRegReg(DYN_DEST, DYN_SRC, DYN_8bit, true);
        storeReg(op->reg, DYN_DEST, DYN_8bit, true);
    } else {
        // might as well call ror8cl_reg, since in this case we would need to call fillFlags
        callHostFunction((void*)ror8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_ror8cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(cpu)) {
        readWriteMem(DYN_8bit, DYN_ADDRESS, DYN_DEST, true, [this]() {
            rorRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_8bit, true);
        });
    } else {
        callHostFunction((void*)ror8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_ror16_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 0xf)) {
            incrementEip(op->len);
            return;
        }
        loadReg(op->reg, DYN_DEST, DYN_16bit);
        rorRegImm(DYN_DEST, DYN_16bit, op->imm);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)ror16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror16_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        if (!(op->imm & 0xf)) {
            incrementEip(op->len);
            return;
        }
        calculateEaa(op, DYN_ADDRESS);
        readWriteMem(DYN_16bit, DYN_ADDRESS, DYN_DEST, true, [op, this]() {
            rorRegImm(DYN_CALL_RESULT, DYN_16bit, op->imm);
        });
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)ror16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror16cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_DEST, DYN_16bit);
        rorRegReg(DYN_DEST, DYN_SRC, DYN_16bit, true);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
    } else {
        // might as well call ror16cl_reg, since in this case we would need to call fillFlags
        callHostFunction((void*)ror16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_ror16cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(cpu)) {
        readWriteMem(DYN_16bit, DYN_ADDRESS, DYN_DEST, true, [this]() {
            rorRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_16bit, true);
        });
    } else {
        callHostFunction((void*)ror16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_ror32_reg_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        rorRegImm(DYN_DEST, DYN_32bit, op->imm);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        incrementEip(op->len);
        return;
    }
    callHostFunction((void*)ror32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror32_mem_op(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        calculateEaa(op, DYN_ADDRESS);
        readWriteMem(DYN_32bit, DYN_ADDRESS, DYN_DEST, true, [op, this]() {
            rorRegImm(DYN_CALL_RESULT, DYN_32bit, op->imm);
        });
        incrementEip(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)ror32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_ror32cl_reg_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        rorRegReg(DYN_DEST, DYN_SRC, DYN_32bit, true);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
    } else {
        // might as well call ror32cl_reg, since in this case we would need to call fillFlags
        callHostFunction((void*)ror32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_ror32cl_mem_op(DecodedOp* op) {
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(cpu)) {
        readWriteMem(DYN_32bit, DYN_ADDRESS, DYN_DEST, true, [this]() {
            rorRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_32bit, true);
        });
    } else {
        callHostFunction((void*)ror32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_8, true);
        currentLazyFlags = FLAGS_NONE;
    }
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
void DynamicData::shift_reg_op(DecodedOp* op, DynWidth width, InstCPUImm instCPUImm, InstRegImm instRegImm, const LazyFlags* lazyFlags) {
    if (!op->needsToSetFlags(cpu)) {
        (this->*instCPUImm)(op->reg, width, op->imm, DYN_DEST);
    } else {
        loadReg(op->reg, DYN_DEST, width);
        storeLazyFlagsDst(DYN_DEST, width, false);
        storeLazyFlagsSrc(width, op->imm);
        (this->*instRegImm)(DYN_DEST, width, op->imm);
        storeLazyFlagsResult(DYN_DEST, width, false);
        storeReg(op->reg, DYN_DEST, width, true);
        storeLazyFlags(lazyFlags);
        currentLazyFlags = lazyFlags;
    }
    incrementEip(op->len);
}
void DynamicData::shift_mem_op(DecodedOp* op, DynWidth width, InstMemImm instMemImm, InstRegImm instRegImm, const LazyFlags* lazyFlags) {
    calculateEaa(op, DYN_ADDRESS);
    if (!op->needsToSetFlags(cpu)) {
        (this->*instMemImm)(DYN_ADDRESS, width, op->imm, true, DYN_DEST);
    } else {
        storeLazyFlagsSrc(width, op->imm);
        readWriteMem(width, DYN_ADDRESS, DYN_DEST, true, [width, op, instRegImm, this]() {
            storeLazyFlagsDst(DYN_CALL_RESULT, width, false);
            (this->*instRegImm)(DYN_CALL_RESULT, width, op->imm);
            storeLazyFlagsResult(DYN_CALL_RESULT, width, false);
        });
        storeLazyFlags(lazyFlags);
        currentLazyFlags = lazyFlags;
    }
    incrementEip(op->len);
}
void DynamicData::shift_cl_reg_op(DecodedOp* op, DynWidth width, InstCPUReg instCPUReg, InstRegReg instRegReg, const LazyFlags* lazyFlags) {
    loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    if (!op->needsToSetFlags(cpu)) {
        (this->*instCPUReg)(op->reg, DYN_SRC, width, true, DYN_DEST);
    } else {
        If(DYN_SRC, false);
            loadReg(op->reg, DYN_DEST, width);
            storeLazyFlagsSrc(DYN_SRC, width, false);
            storeLazyFlagsDst(DYN_DEST, width, false);
            (this->*instRegReg)(DYN_DEST, DYN_SRC, width, true);
            storeLazyFlagsResult(DYN_DEST, width, false);
            storeReg(op->reg, DYN_DEST, width, true);
            storeLazyFlags(lazyFlags);
        EndIf();
    }
    currentLazyFlags = nullptr; // since its conditionally set
    incrementEip(op->len);
}
void DynamicData::shift_cl_mem_op(DecodedOp* op, DynWidth width, InstMemReg instMemReg, InstRegReg instRegReg, const LazyFlags* lazyFlags) {
    loadReg(1, DYN_SRC, DYN_8bit); // for x86 we need it in DYN_SRC (ecx)
    andRegImm(DYN_SRC, DYN_8bit, 0x1F);
    calculateEaa(op, DYN_ADDRESS);
    if (!op->needsToSetFlags(cpu)) {
        (this->*instMemReg)(DYN_ADDRESS, DYN_SRC, width, true, true, DYN_DEST);
    } else {
        If(DYN_SRC, false, true);
            storeLazyFlagsSrc(DYN_SRC, width, false);
            readWriteMem(width, DYN_ADDRESS, DYN_DEST, true, [width, op, instRegReg, this]() {
                storeLazyFlagsDst(DYN_CALL_RESULT, width, false);
                (this->*instRegReg)(DYN_CALL_RESULT, DYN_SRC, width, true);
                storeLazyFlagsResult(DYN_CALL_RESULT, width, false);
            });
        storeLazyFlags(lazyFlags);
        EndIf(true);        
    }
    currentLazyFlags = nullptr; // since its conditionally set
    incrementEip(op->len);
}
void DynamicData::dynamic_shl8_reg_op(DecodedOp* op) {
    shift_reg_op(op, DYN_8bit, &DynamicData::shlCPUImm, &DynamicData::shlRegImm, FLAGS_SHL8);
}
void DynamicData::dynamic_shl8_mem_op(DecodedOp* op) {
    shift_mem_op(op, DYN_8bit, &DynamicData::shlMemImm, &DynamicData::shlRegImm, FLAGS_SHL8);
}
void DynamicData::dynamic_shl8cl_reg_op(DecodedOp* op) {
    shift_cl_reg_op(op, DYN_8bit, &DynamicData::shlCPUReg, &DynamicData::shlRegReg, FLAGS_SHL8);
}
void DynamicData::dynamic_shl8cl_mem_op(DecodedOp* op) {
    shift_cl_mem_op(op, DYN_8bit, &DynamicData::shlMemReg, &DynamicData::shlRegReg, FLAGS_SHL8);
}
void DynamicData::dynamic_shl16_reg_op(DecodedOp* op) {
    shift_reg_op(op, DYN_16bit, &DynamicData::shlCPUImm, &DynamicData::shlRegImm, FLAGS_SHL16);
}
void DynamicData::dynamic_shl16_mem_op(DecodedOp* op) {
    shift_mem_op(op, DYN_16bit, &DynamicData::shlMemImm, &DynamicData::shlRegImm, FLAGS_SHL16);
}
void DynamicData::dynamic_shl16cl_reg_op(DecodedOp* op) {
    shift_cl_reg_op(op, DYN_16bit, &DynamicData::shlCPUReg, &DynamicData::shlRegReg, FLAGS_SHL16);
}
void DynamicData::dynamic_shl16cl_mem_op(DecodedOp* op) {
    shift_cl_mem_op(op, DYN_16bit, &DynamicData::shlMemReg, &DynamicData::shlRegReg, FLAGS_SHL16);
}
void DynamicData::dynamic_shl32_reg_op(DecodedOp* op) {
    shift_reg_op(op, DYN_32bit, &DynamicData::shlCPUImm, &DynamicData::shlRegImm, FLAGS_SHL32);
}
void DynamicData::dynamic_shl32_mem_op(DecodedOp* op) {
    shift_mem_op(op, DYN_32bit, &DynamicData::shlMemImm, &DynamicData::shlRegImm, FLAGS_SHL32);
}
void DynamicData::dynamic_shl32cl_reg_op(DecodedOp* op) {
    shift_cl_reg_op(op, DYN_32bit, &DynamicData::shlCPUReg, &DynamicData::shlRegReg, FLAGS_SHL32);
}
void DynamicData::dynamic_shl32cl_mem_op(DecodedOp* op) {
    shift_cl_mem_op(op, DYN_32bit, &DynamicData::shlMemReg, &DynamicData::shlRegReg, FLAGS_SHL32);
}
void DynamicData::dynamic_shr8_reg_op(DecodedOp* op) {
    shift_reg_op(op, DYN_8bit, &DynamicData::shrCPUImm, &DynamicData::shrRegImm, FLAGS_SHR8);
}
void DynamicData::dynamic_shr8_mem_op(DecodedOp* op) {
    shift_mem_op(op, DYN_8bit, &DynamicData::shrMemImm, &DynamicData::shrRegImm, FLAGS_SHR8);
}
void DynamicData::dynamic_shr8cl_reg_op(DecodedOp* op) {
    shift_cl_reg_op(op, DYN_8bit, &DynamicData::shrCPUReg, &DynamicData::shrRegReg, FLAGS_SHR8);
}
void DynamicData::dynamic_shr8cl_mem_op(DecodedOp* op) {
    shift_cl_mem_op(op, DYN_8bit, &DynamicData::shrMemReg, &DynamicData::shrRegReg, FLAGS_SHR8);
}
void DynamicData::dynamic_shr16_reg_op(DecodedOp* op) {
    shift_reg_op(op, DYN_16bit, &DynamicData::shrCPUImm, &DynamicData::shrRegImm, FLAGS_SHR16);
}
void DynamicData::dynamic_shr16_mem_op(DecodedOp* op) {
    shift_mem_op(op, DYN_16bit, &DynamicData::shrMemImm, &DynamicData::shrRegImm, FLAGS_SHR16);
}
void DynamicData::dynamic_shr16cl_reg_op(DecodedOp* op) {
    shift_cl_reg_op(op, DYN_16bit, &DynamicData::shrCPUReg, &DynamicData::shrRegReg, FLAGS_SHR16);
}
void DynamicData::dynamic_shr16cl_mem_op(DecodedOp* op) {
    shift_cl_mem_op(op, DYN_16bit, &DynamicData::shrMemReg, &DynamicData::shrRegReg, FLAGS_SHR16);
}
void DynamicData::dynamic_shr32_reg_op(DecodedOp* op) {
    shift_reg_op(op, DYN_32bit, &DynamicData::shrCPUImm, &DynamicData::shrRegImm, FLAGS_SHR32);
}
void DynamicData::dynamic_shr32_mem_op(DecodedOp* op) {
    shift_mem_op(op, DYN_32bit, &DynamicData::shrMemImm, &DynamicData::shrRegImm, FLAGS_SHR32);
}
void DynamicData::dynamic_shr32cl_reg_op(DecodedOp* op) {
    shift_cl_reg_op(op, DYN_32bit, &DynamicData::shrCPUReg, &DynamicData::shrRegReg, FLAGS_SHR32);
}
void DynamicData::dynamic_shr32cl_mem_op(DecodedOp* op) {
    shift_cl_mem_op(op, DYN_32bit, &DynamicData::shrMemReg, &DynamicData::shrRegReg, FLAGS_SHR32);
}
void DynamicData::dynamic_sar8_reg_op(DecodedOp* op) {
    shift_reg_op(op, DYN_8bit, &DynamicData::sarCPUImm, &DynamicData::sarRegImm, FLAGS_SAR8);
}
void DynamicData::dynamic_sar8_mem_op(DecodedOp* op) {
    shift_mem_op(op, DYN_8bit, &DynamicData::sarMemImm, &DynamicData::sarRegImm, FLAGS_SAR8);
}
void DynamicData::dynamic_sar8cl_reg_op(DecodedOp* op) {
    shift_cl_reg_op(op, DYN_8bit, &DynamicData::sarCPUReg, &DynamicData::sarRegReg, FLAGS_SAR8);
}
void DynamicData::dynamic_sar8cl_mem_op(DecodedOp* op) {
    shift_cl_mem_op(op, DYN_8bit, &DynamicData::sarMemReg, &DynamicData::sarRegReg, FLAGS_SAR8);
}
void DynamicData::dynamic_sar16_reg_op(DecodedOp* op) {
    shift_reg_op(op, DYN_16bit, &DynamicData::sarCPUImm, &DynamicData::sarRegImm, FLAGS_SAR16);
}
void DynamicData::dynamic_sar16_mem_op(DecodedOp* op) {
    shift_mem_op(op, DYN_16bit, &DynamicData::sarMemImm, &DynamicData::sarRegImm, FLAGS_SAR16);
}
void DynamicData::dynamic_sar16cl_reg_op(DecodedOp* op) {
    shift_cl_reg_op(op, DYN_16bit, &DynamicData::sarCPUReg, &DynamicData::sarRegReg, FLAGS_SAR16);
}
void DynamicData::dynamic_sar16cl_mem_op(DecodedOp* op) {
    shift_cl_mem_op(op, DYN_16bit, &DynamicData::sarMemReg, &DynamicData::sarRegReg, FLAGS_SAR16);
}
void DynamicData::dynamic_sar32_reg_op(DecodedOp* op) {
    shift_reg_op(op, DYN_32bit, &DynamicData::sarCPUImm, &DynamicData::sarRegImm, FLAGS_SAR32);
}
void DynamicData::dynamic_sar32_mem_op(DecodedOp* op) {
    shift_mem_op(op, DYN_32bit, &DynamicData::sarMemImm, &DynamicData::sarRegImm, FLAGS_SAR32);
}
void DynamicData::dynamic_sar32cl_reg_op(DecodedOp* op) {
    shift_cl_reg_op(op, DYN_32bit, &DynamicData::sarCPUReg, &DynamicData::sarRegReg, FLAGS_SAR32);
}
void DynamicData::dynamic_sar32cl_mem_op(DecodedOp* op) {
    shift_cl_mem_op(op, DYN_32bit, &DynamicData::sarMemReg, &DynamicData::sarRegReg, FLAGS_SAR32);
}
void DynamicData::dynamic_dshlr16r16(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);
    loadReg(op->reg, DYN_SRC, DYN_16bit);
    if (needsFlags) {
        storeLazyFlagsSrc(DYN_16bit, op->imm);
        storeLazyFlagsDst(DYN_SRC, DYN_16bit, false);
    }
    loadReg(op->rm, DYN_DEST, DYN_16bit);
    shlRegImm(DYN_SRC, DYN_16bit, op->imm);
    shrRegImm(DYN_DEST, DYN_16bit, 16 - op->imm);
    orRegReg(DYN_DEST, DYN_SRC, DYN_16bit, true);

    if (needsFlags) {
        storeLazyFlagsResult(DYN_DEST, DYN_16bit, false);
        storeLazyFlags(FLAGS_DSHL16);
        currentLazyFlags = FLAGS_DSHL16;
    }
    storeReg(op->reg, DYN_DEST, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshle16r16(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);

    calculateEaa(op, DYN_ADDRESS);
    if (needsFlags) {
        storeLazyFlagsSrc(DYN_16bit, op->imm);
        storeLazyFlags(FLAGS_DSHL16);
        currentLazyFlags = FLAGS_DSHL16;
    }
    readWriteMem(DYN_16bit, DYN_ADDRESS, DYN_DEST, true, [needsFlags, op, this]() {
        if (needsFlags) {
            storeLazyFlagsDst(DYN_CALL_RESULT, DYN_16bit, false);
        }
        loadReg(op->reg, DYN_SRC, DYN_16bit);
        shlRegImm(DYN_CALL_RESULT, DYN_16bit, op->imm);
        shrRegImm(DYN_SRC, DYN_16bit, 16 - op->imm);
        orRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_16bit, true);
        if (needsFlags) {
            storeLazyFlagsResult(DYN_CALL_RESULT, DYN_16bit, false);
        }
    });
    incrementEip(op->len);
}
void DynamicData::dynamic_dshlr32r32(DecodedOp* op) {
    // cpu->src.u32 = imm;
    // cpu->dst.u32 = cpu->reg[reg].u32;
    // cpu->result.u32 = (cpu->reg[reg].u32 << imm) | (cpu->reg[rm].u32 >> (32 - imm));
    // cpu->reg[reg].u32 = cpu->result.u32;
    // cpu->lazyFlags = FLAGS_DSHL32;
    U32 needsFlags = op->needsToSetFlags(cpu);
    loadReg(op->reg, DYN_SRC, DYN_32bit);
    if (needsFlags) {
        storeLazyFlagsSrc(DYN_32bit, op->imm);
        storeLazyFlagsDst(DYN_SRC, DYN_32bit, false);
    } 
    loadReg(op->rm, DYN_DEST, DYN_32bit);
    shlRegImm(DYN_SRC, DYN_32bit, op->imm);
    shrRegImm(DYN_DEST, DYN_32bit, 32 - op->imm);
    orRegReg(DYN_DEST, DYN_SRC, DYN_32bit, true);    

    if (needsFlags) {
        storeLazyFlagsResult(DYN_DEST, DYN_32bit, false);
        storeLazyFlags(FLAGS_DSHL32);
        currentLazyFlags = FLAGS_DSHL32;
    }
    storeReg(op->reg, DYN_DEST, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshle32r32(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);

    calculateEaa(op, DYN_ADDRESS);
    if (needsFlags) {
        storeLazyFlagsSrc(DYN_32bit, op->imm);
        storeLazyFlags(FLAGS_DSHL32);
        currentLazyFlags = FLAGS_DSHL32;
    }
    readWriteMem(DYN_32bit, DYN_ADDRESS, DYN_DEST, true, [needsFlags, op, this]() {
        if (needsFlags) {
            storeLazyFlagsDst(DYN_CALL_RESULT, DYN_32bit, false);
        }
        loadReg(op->reg, DYN_SRC, DYN_32bit);
        shlRegImm(DYN_CALL_RESULT, DYN_32bit, op->imm);
        shrRegImm(DYN_SRC, DYN_32bit, 32 - op->imm);
        orRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_32bit, true);
        if (needsFlags) {
            storeLazyFlagsResult(DYN_CALL_RESULT, DYN_32bit, false);
        }
    });
    incrementEip(op->len);
}
void DynamicData::dynamic_dshlclr16r16(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_32bit, 0x1f); // intentional 32-bit and on 8-bit data so that the top bits are cleared and the next If statement work, If works on 32-bit data
    If(DYN_SRC, false);
        loadReg(op->reg, DYN_DEST, DYN_16bit);
        if (needsFlags) {
            storeLazyFlagsSrc(DYN_SRC, DYN_16bit, false);
            storeLazyFlagsDst(DYN_DEST, DYN_16bit, false);
        }
        loadReg(op->rm, DYN_ADDRESS, DYN_16bit);
        shlRegReg(DYN_DEST, DYN_SRC, DYN_16bit, false);
        // DYN_SRC = 16 - DYN_SRC
        movToReg(DYN_CALL_RESULT, DYN_8bit, 16);
        subRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_8bit, false);
        movToRegFromReg(DYN_SRC, DYN_8bit, DYN_CALL_RESULT, DYN_8bit, true);
        shrRegReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true);
        orRegReg(DYN_DEST, DYN_ADDRESS, DYN_16bit, true);

        if (needsFlags) {
            storeLazyFlagsResult(DYN_DEST, DYN_16bit, false);
            storeLazyFlags(FLAGS_DSHL16);
        }
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
    EndIf();
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_dshlcle16r16(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_32bit, 0x1f); // intentional 32-bit and on 8-bit data so that the top bits are cleared and the next If statement work, If works on 32-bit data
    If(DYN_SRC, false, true);
        calculateEaa(op, DYN_ADDRESS);
    
        if (needsFlags) {
            storeLazyFlagsSrc(DYN_SRC, DYN_16bit, false);
        }
        // can't use readWriteMem, not enough temp registers
        movFromMem(DYN_16bit, DYN_ADDRESS, false);

        if (needsFlags) {                
            storeLazyFlagsDst(DYN_CALL_RESULT, DYN_16bit, false);
        }
        shlRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_16bit, false);

        // DYN_SRC = 16 - DYN_SRC
        movToReg(DYN_DEST, DYN_8bit, 16);
        subRegReg(DYN_DEST, DYN_SRC, DYN_8bit, false);
        movToRegFromReg(DYN_SRC, DYN_8bit, DYN_DEST, DYN_8bit, true);

        loadReg(op->reg, DYN_DEST, DYN_16bit);
        shrRegReg(DYN_DEST, DYN_SRC, DYN_16bit, true);
        orRegReg(DYN_CALL_RESULT, DYN_DEST, DYN_16bit, true);
        if (needsFlags) {
            storeLazyFlagsResult(DYN_CALL_RESULT, DYN_16bit, false);
            storeLazyFlags(FLAGS_DSHL16);
        }
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_DEST);
    EndIf(true);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}

void DynamicData::dynamic_dshlclr32r32(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_32bit, 0x1f); // intentional 32-bit and on 8-bit data so that the top bits are cleared and the next If statement work, If works on 32-bit data
    If(DYN_SRC, false);
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        if (needsFlags) {
            storeLazyFlagsSrc(DYN_SRC, DYN_32bit, false);
            storeLazyFlagsDst(DYN_DEST, DYN_32bit, false);
        }
        loadReg(op->rm, DYN_ADDRESS, DYN_32bit);
        shlRegReg(DYN_DEST, DYN_SRC, DYN_32bit, false);
        // DYN_SRC = 32 - DYN_SRC
        movToReg(DYN_CALL_RESULT, DYN_8bit, 32);
        subRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_8bit, false);
        movToRegFromReg(DYN_SRC, DYN_8bit, DYN_CALL_RESULT, DYN_8bit, true);
        shrRegReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true);
        orRegReg(DYN_DEST, DYN_ADDRESS, DYN_32bit, true);

        if (needsFlags) {
            storeLazyFlagsResult(DYN_DEST, DYN_32bit, false);
            storeLazyFlags(FLAGS_DSHL32);
        }
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
    EndIf();
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_dshlcle32r32(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_32bit, 0x1f); // intentional 32-bit and on 8-bit data so that the top bits are cleared and the next If statement work, If works on 32-bit data
    If(DYN_SRC, false, true);
        calculateEaa(op, DYN_ADDRESS);

        if (needsFlags) {
            storeLazyFlagsSrc(DYN_SRC, DYN_32bit, false);
        }
        // can't use readWriteMem, not enough temp registers
        movFromMem(DYN_32bit, DYN_ADDRESS, false);

        if (needsFlags) {
            storeLazyFlagsDst(DYN_CALL_RESULT, DYN_32bit, false);
        }
        shlRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_32bit, false);

        // DYN_SRC = 32 - DYN_SRC
        movToReg(DYN_DEST, DYN_8bit, 32);
        subRegReg(DYN_DEST, DYN_SRC, DYN_8bit, false);
        movToRegFromReg(DYN_SRC, DYN_8bit, DYN_DEST, DYN_8bit, true);

        loadReg(op->reg, DYN_DEST, DYN_32bit);
        shrRegReg(DYN_DEST, DYN_SRC, DYN_32bit, true);
        orRegReg(DYN_CALL_RESULT, DYN_DEST, DYN_32bit, true);
        if (needsFlags) {
            storeLazyFlagsResult(DYN_CALL_RESULT, DYN_32bit, false);
            storeLazyFlags(FLAGS_DSHL32);
        }
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
    EndIf(true);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
void DynamicData::dynamic_dshrr16r16(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);
    loadReg(op->reg, DYN_SRC, DYN_16bit);
    if (needsFlags) {
        storeLazyFlagsSrc(DYN_16bit, op->imm);
        storeLazyFlagsDst(DYN_SRC, DYN_16bit, false);
    }
    loadReg(op->rm, DYN_DEST, DYN_16bit);
    shrRegImm(DYN_SRC, DYN_16bit, op->imm);
    shlRegImm(DYN_DEST, DYN_16bit, 16 - op->imm);
    orRegReg(DYN_DEST, DYN_SRC, DYN_16bit, true);

    if (needsFlags) {
        storeLazyFlagsResult(DYN_DEST, DYN_16bit, false);
        storeLazyFlags(FLAGS_DSHR16);
        currentLazyFlags = FLAGS_DSHR16;
    }
    storeReg(op->reg, DYN_DEST, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshre16r16(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);

    calculateEaa(op, DYN_ADDRESS);
    if (needsFlags) {
        storeLazyFlagsSrc(DYN_16bit, op->imm);
        storeLazyFlags(FLAGS_DSHR16);
        currentLazyFlags = FLAGS_DSHR16;
    }
    readWriteMem(DYN_16bit, DYN_ADDRESS, DYN_DEST, true, [needsFlags, op, this]() {
        if (needsFlags) {
            storeLazyFlagsDst(DYN_CALL_RESULT, DYN_16bit, false);
        }
        loadReg(op->reg, DYN_SRC, DYN_16bit);
        shrRegImm(DYN_CALL_RESULT, DYN_16bit, op->imm);
        shlRegImm(DYN_SRC, DYN_16bit, 16 - op->imm);
        orRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_16bit, true);
        if (needsFlags) {
            storeLazyFlagsResult(DYN_CALL_RESULT, DYN_16bit, false);
        }
    });
    incrementEip(op->len);
}
void DynamicData::dynamic_dshrr32r32(DecodedOp* op) {
    // cpu->src.u32 = imm;
    // cpu->dst.u32 = cpu->reg[reg].u32;
    // cpu->result.u32 = (cpu->reg[reg].u32 >> imm) | (cpu->reg[rm].u32 << (32 - imm));
    // cpu->reg[reg].u32 = cpu->result.u32;
    U32 needsFlags = op->needsToSetFlags(cpu);
    loadReg(op->reg, DYN_SRC, DYN_32bit);
    if (needsFlags) {
        storeLazyFlagsDst(DYN_SRC, DYN_32bit, false);
        storeLazyFlagsSrc(DYN_32bit, op->imm);
        storeLazyFlags(FLAGS_DSHR32);
        currentLazyFlags = FLAGS_DSHR32;
    }
    loadReg(op->rm, DYN_DEST, DYN_32bit);
    shrRegImm(DYN_SRC, DYN_32bit, op->imm);
    shlRegImm(DYN_DEST, DYN_32bit, (32 - op->imm));
    orRegReg(DYN_SRC, DYN_DEST, DYN_32bit, true);
    if (needsFlags) {
        storeLazyFlagsResult(DYN_SRC, DYN_32bit, false);
    }
    storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_dshre32r32(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);

    calculateEaa(op, DYN_ADDRESS);
    if (needsFlags) {
        storeLazyFlagsSrc(DYN_32bit, op->imm);
        storeLazyFlags(FLAGS_DSHR32);
        currentLazyFlags = FLAGS_DSHR32;
    }
    readWriteMem(DYN_32bit, DYN_ADDRESS, DYN_DEST, true, [needsFlags, op, this]() {
        if (needsFlags) {
            storeLazyFlagsDst(DYN_CALL_RESULT, DYN_32bit, false);
        }
        loadReg(op->reg, DYN_SRC, DYN_32bit);
        shrRegImm(DYN_CALL_RESULT, DYN_32bit, op->imm);
        shlRegImm(DYN_SRC, DYN_32bit, 32 - op->imm);
        orRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_32bit, true);
        if (needsFlags) {
            storeLazyFlagsResult(DYN_CALL_RESULT, DYN_32bit, false);
        }
    });
    incrementEip(op->len);
}
void DynamicData::dynamic_dshrclr16r16(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_32bit, 0x1f); // intentional 32-bit and on 8-bit data so that the top bits are cleared and the next If statement work, If works on 32-bit data
    If(DYN_SRC, false);
        loadReg(op->reg, DYN_DEST, DYN_16bit);
        if (needsFlags) {
            storeLazyFlagsSrc(DYN_SRC, DYN_16bit, false);
            storeLazyFlagsDst(DYN_DEST, DYN_16bit, false);
        }
        loadReg(op->rm, DYN_ADDRESS, DYN_16bit);
        shrRegReg(DYN_DEST, DYN_SRC, DYN_16bit, false);
        // DYN_SRC = 16 - DYN_SRC
        movToReg(DYN_CALL_RESULT, DYN_8bit, 16);
        subRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_8bit, false);
        movToRegFromReg(DYN_SRC, DYN_8bit, DYN_CALL_RESULT, DYN_8bit, true);
        shlRegReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true);
        orRegReg(DYN_DEST, DYN_ADDRESS, DYN_16bit, true);

        if (needsFlags) {
            storeLazyFlagsResult(DYN_DEST, DYN_16bit, false);
            storeLazyFlags(FLAGS_DSHR16);
        }
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
    EndIf();
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_dshrcle16r16(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_32bit, 0x1f); // intentional 32-bit and on 8-bit data so that the top bits are cleared and the next If statement work, If works on 32-bit data
    If(DYN_SRC, false, true);
        calculateEaa(op, DYN_ADDRESS);

        if (needsFlags) {
            storeLazyFlagsSrc(DYN_SRC, DYN_16bit, false);
        }
        // can't use readWriteMem, not enough temp registers
        movFromMem(DYN_16bit, DYN_ADDRESS, false);

        if (needsFlags) {
            storeLazyFlagsDst(DYN_CALL_RESULT, DYN_16bit, false);
        }
        shrRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_16bit, false);

        // DYN_SRC = 16 - DYN_SRC
        movToReg(DYN_DEST, DYN_8bit, 16);
        subRegReg(DYN_DEST, DYN_SRC, DYN_8bit, false);
        movToRegFromReg(DYN_SRC, DYN_8bit, DYN_DEST, DYN_8bit, true);

        loadReg(op->reg, DYN_DEST, DYN_16bit);
        shlRegReg(DYN_DEST, DYN_SRC, DYN_16bit, true);
        orRegReg(DYN_CALL_RESULT, DYN_DEST, DYN_16bit, true);
        if (needsFlags) {
            storeLazyFlagsResult(DYN_CALL_RESULT, DYN_16bit, false);
            storeLazyFlags(FLAGS_DSHR16);
        }
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_DEST);
    EndIf(true);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
void DynamicData::dynamic_dshrclr32r32(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_32bit, 0x1f); // intentional 32-bit and on 8-bit data so that the top bits are cleared and the next If statement work, If works on 32-bit data
    If(DYN_SRC, false);
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        if (needsFlags) {
            storeLazyFlagsSrc(DYN_SRC, DYN_32bit, false);
            storeLazyFlagsDst(DYN_DEST, DYN_32bit, false);
        }
        loadReg(op->rm, DYN_ADDRESS, DYN_32bit);
        shrRegReg(DYN_DEST, DYN_SRC, DYN_32bit, false);
        // DYN_SRC = 32 - DYN_SRC
        movToReg(DYN_CALL_RESULT, DYN_8bit, 32);
        subRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_8bit, false);
        movToRegFromReg(DYN_SRC, DYN_8bit, DYN_CALL_RESULT, DYN_8bit, true);
        shlRegReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true);
        orRegReg(DYN_DEST, DYN_ADDRESS, DYN_32bit, true);

        if (needsFlags) {
            storeLazyFlagsResult(DYN_DEST, DYN_32bit, false);
            storeLazyFlags(FLAGS_DSHR32);
        }
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
    EndIf();
    incrementEip(op->len);
    currentLazyFlags = nullptr;
}
void DynamicData::dynamic_dshrcle32r32(DecodedOp* op) {
    U32 needsFlags = op->needsToSetFlags(cpu);
    loadReg(1, DYN_SRC, DYN_8bit);
    andRegImm(DYN_SRC, DYN_32bit, 0x1f); // intentional 32-bit and on 8-bit data so that the top bits are cleared and the next If statement work, If works on 32-bit data
    If(DYN_SRC, false, true);
        calculateEaa(op, DYN_ADDRESS);

        if (needsFlags) {
            storeLazyFlagsSrc(DYN_SRC, DYN_32bit, false);
        }
        // can't use readWriteMem, not enough temp registers
        movFromMem(DYN_32bit, DYN_ADDRESS, false);

        if (needsFlags) {
            storeLazyFlagsDst(DYN_CALL_RESULT, DYN_32bit, false);
        }
        shrRegReg(DYN_CALL_RESULT, DYN_SRC, DYN_32bit, false);

        // DYN_SRC = 32 - DYN_SRC
        movToReg(DYN_DEST, DYN_8bit, 32);
        subRegReg(DYN_DEST, DYN_SRC, DYN_8bit, false);
        movToRegFromReg(DYN_SRC, DYN_8bit, DYN_DEST, DYN_8bit, true);

        loadReg(op->reg, DYN_DEST, DYN_32bit);
        shlRegReg(DYN_DEST, DYN_SRC, DYN_32bit, true);
        orRegReg(DYN_CALL_RESULT, DYN_DEST, DYN_32bit, true);
        if (needsFlags) {
            storeLazyFlagsResult(DYN_CALL_RESULT, DYN_32bit, false);
            storeLazyFlags(FLAGS_DSHR32);
        }
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
    EndIf(true);
    currentLazyFlags = nullptr;
    incrementEip(op->len);
}
