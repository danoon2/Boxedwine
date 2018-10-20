#include "../normal/normal_shift.h"
void dynamic_rol8_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, OFFSET_REG8(op->reg), DYN_8bit);
        movToRegFromReg(DYN_DEST, DYN_8bit, DYN_SRC, DYN_8bit, false);
        instRegImm('>', DYN_SRC, DYN_8bit, 8-op->imm);
        instRegImm('<', DYN_DEST, DYN_8bit, op->imm);
        instRegReg('|', DYN_DEST, DYN_SRC, DYN_8bit, true);
        movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(rol8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rol8_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movFromMem(DYN_8bit, DYN_ADDRESS, false);
        movToRegFromReg(DYN_DEST, DYN_8bit, DYN_CALL_RESULT, DYN_8bit, false);
        instRegImm('>', DYN_CALL_RESULT, DYN_8bit, 8-op->imm);
        instRegImm('<', DYN_DEST, DYN_8bit, op->imm);
        instRegReg('|', DYN_DEST, DYN_CALL_RESULT, DYN_8bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_8bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rol8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rol8cl_reg_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(rol8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rol8cl_mem_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rol8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rol16_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit);
        movToRegFromReg(DYN_DEST, DYN_16bit, DYN_SRC, DYN_16bit, false);
        instRegImm('>', DYN_SRC, DYN_16bit, 16-op->imm);
        instRegImm('<', DYN_DEST, DYN_16bit, op->imm);
        instRegReg('|', DYN_DEST, DYN_SRC, DYN_16bit, true);
        movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_DEST, DYN_16bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(rol16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rol16_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movFromMem(DYN_16bit, DYN_ADDRESS, false);
        movToRegFromReg(DYN_DEST, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, false);
        instRegImm('>', DYN_CALL_RESULT, DYN_16bit, 16-op->imm);
        instRegImm('<', DYN_DEST, DYN_16bit, op->imm);
        instRegReg('|', DYN_DEST, DYN_CALL_RESULT, DYN_16bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_16bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rol16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rol16cl_reg_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(rol16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rol16cl_mem_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rol16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rol32_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit);
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_SRC, DYN_32bit, false);
        instRegImm('>', DYN_SRC, DYN_32bit, 32-op->imm);
        instRegImm('<', DYN_DEST, DYN_32bit, op->imm);
        instRegReg('|', DYN_DEST, DYN_SRC, DYN_32bit, true);
        movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_DEST, DYN_32bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(rol32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rol32_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movFromMem(DYN_32bit, DYN_ADDRESS, false);
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, false);
        instRegImm('>', DYN_CALL_RESULT, DYN_32bit, 32-op->imm);
        instRegImm('<', DYN_DEST, DYN_32bit, op->imm);
        instRegReg('|', DYN_DEST, DYN_CALL_RESULT, DYN_32bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_32bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rol32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rol32cl_reg_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(rol32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rol32cl_mem_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rol32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_ror8_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, OFFSET_REG8(op->reg), DYN_8bit);
        movToRegFromReg(DYN_DEST, DYN_8bit, DYN_SRC, DYN_8bit, false);
        instRegImm('<', DYN_SRC, DYN_8bit, 8-op->imm);
        instRegImm('>', DYN_DEST, DYN_8bit, op->imm);
        instRegReg('|', DYN_DEST, DYN_SRC, DYN_8bit, true);
        movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(ror8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_ror8_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movFromMem(DYN_8bit, DYN_ADDRESS, false);
        movToRegFromReg(DYN_DEST, DYN_8bit, DYN_CALL_RESULT, DYN_8bit, false);
        instRegImm('<', DYN_CALL_RESULT, DYN_8bit, 8-op->imm);
        instRegImm('>', DYN_DEST, DYN_8bit, op->imm);
        instRegReg('|', DYN_DEST, DYN_CALL_RESULT, DYN_8bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_8bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(ror8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_ror8cl_reg_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(ror8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_ror8cl_mem_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(ror8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_ror16_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit);
        movToRegFromReg(DYN_DEST, DYN_16bit, DYN_SRC, DYN_16bit, false);
        instRegImm('<', DYN_SRC, DYN_16bit, 16-op->imm);
        instRegImm('>', DYN_DEST, DYN_16bit, op->imm);
        instRegReg('|', DYN_DEST, DYN_SRC, DYN_16bit, true);
        movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_DEST, DYN_16bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(ror16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_ror16_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movFromMem(DYN_16bit, DYN_ADDRESS, false);
        movToRegFromReg(DYN_DEST, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, false);
        instRegImm('<', DYN_CALL_RESULT, DYN_16bit, 16-op->imm);
        instRegImm('>', DYN_DEST, DYN_16bit, op->imm);
        instRegReg('|', DYN_DEST, DYN_CALL_RESULT, DYN_16bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_16bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(ror16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_ror16cl_reg_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(ror16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_ror16cl_mem_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(ror16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_ror32_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit);
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_SRC, DYN_32bit, false);
        instRegImm('<', DYN_SRC, DYN_32bit, 32-op->imm);
        instRegImm('>', DYN_DEST, DYN_32bit, op->imm);
        instRegReg('|', DYN_DEST, DYN_SRC, DYN_32bit, true);
        movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_DEST, DYN_32bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(ror32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_ror32_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movFromMem(DYN_32bit, DYN_ADDRESS, false);
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, false);
        instRegImm('<', DYN_CALL_RESULT, DYN_32bit, 32-op->imm);
        instRegImm('>', DYN_DEST, DYN_32bit, op->imm);
        instRegReg('|', DYN_DEST, DYN_CALL_RESULT, DYN_32bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_32bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(ror32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_ror32cl_reg_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(ror32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_ror32cl_mem_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(ror32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcl8_reg_op(DynamicData* data, DecodedOp* op) {
    callHostFunction(rcl8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcl8_mem_op(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rcl8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcl8cl_reg_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(rcl8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcl8cl_mem_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rcl8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcl16_reg_op(DynamicData* data, DecodedOp* op) {
    callHostFunction(rcl16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcl16_mem_op(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rcl16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcl16cl_reg_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(rcl16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcl16cl_mem_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rcl16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcl32_reg_op(DynamicData* data, DecodedOp* op) {
    callHostFunction(rcl32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcl32_mem_op(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rcl32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcl32cl_reg_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(rcl32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcl32cl_mem_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rcl32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcr8_reg_op(DynamicData* data, DecodedOp* op) {
    callHostFunction(rcr8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcr8_mem_op(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rcr8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcr8cl_reg_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(rcr8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcr8cl_mem_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rcr8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcr16_reg_op(DynamicData* data, DecodedOp* op) {
    callHostFunction(rcr16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcr16_mem_op(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rcr16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcr16cl_reg_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(rcr16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcr16cl_mem_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rcr16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcr32_reg_op(DynamicData* data, DecodedOp* op) {
    callHostFunction(rcr32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcr32_mem_op(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rcr32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcr32cl_reg_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(rcr32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_rcr32cl_mem_op(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(rcr32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(op->len);
}
void dynamic_shl8_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        instCPUImm('<', OFFSET_REG8(op->reg), DYN_8bit, op->imm);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(shl8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHL8;
    INCREMENT_EIP(op->len);
}
void dynamic_shl8_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('<', DYN_ADDRESS, DYN_8bit, op->imm, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(shl8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHL8;
    INCREMENT_EIP(op->len);
}
void dynamic_shl8cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instCPUReg('<', OFFSET_REG8(op->reg), DYN_SRC, DYN_8bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(shl8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHL8;
    INCREMENT_EIP(op->len);
}
void dynamic_shl8cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instMemReg('<', DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(shl8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHL8;
    INCREMENT_EIP(op->len);
}
void dynamic_shl16_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        instCPUImm('<', CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, op->imm);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(shl16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHL16;
    INCREMENT_EIP(op->len);
}
void dynamic_shl16_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('<', DYN_ADDRESS, DYN_16bit, op->imm, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(shl16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHL16;
    INCREMENT_EIP(op->len);
}
void dynamic_shl16cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instCPUReg('<', CPU_OFFSET_OF(reg[op->reg].u16), DYN_SRC, DYN_16bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(shl16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHL16;
    INCREMENT_EIP(op->len);
}
void dynamic_shl16cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instMemReg('<', DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(shl16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHL16;
    INCREMENT_EIP(op->len);
}
void dynamic_shl32_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        instCPUImm('<', CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, op->imm);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(shl32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHL32;
    INCREMENT_EIP(op->len);
}
void dynamic_shl32_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('<', DYN_ADDRESS, DYN_32bit, op->imm, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(shl32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHL32;
    INCREMENT_EIP(op->len);
}
void dynamic_shl32cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instCPUReg('<', CPU_OFFSET_OF(reg[op->reg].u32), DYN_SRC, DYN_32bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(shl32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHL32;
    INCREMENT_EIP(op->len);
}
void dynamic_shl32cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instMemReg('<', DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(shl32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHL32;
    INCREMENT_EIP(op->len);
}
void dynamic_shr8_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        instCPUImm('>', OFFSET_REG8(op->reg), DYN_8bit, op->imm);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(shr8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHR8;
    INCREMENT_EIP(op->len);
}
void dynamic_shr8_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('>', DYN_ADDRESS, DYN_8bit, op->imm, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(shr8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHR8;
    INCREMENT_EIP(op->len);
}
void dynamic_shr8cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instCPUReg('>', OFFSET_REG8(op->reg), DYN_SRC, DYN_8bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(shr8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHR8;
    INCREMENT_EIP(op->len);
}
void dynamic_shr8cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instMemReg('>', DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(shr8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHR8;
    INCREMENT_EIP(op->len);
}
void dynamic_shr16_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        instCPUImm('>', CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, op->imm);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(shr16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHR16;
    INCREMENT_EIP(op->len);
}
void dynamic_shr16_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('>', DYN_ADDRESS, DYN_16bit, op->imm, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(shr16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHR16;
    INCREMENT_EIP(op->len);
}
void dynamic_shr16cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instCPUReg('>', CPU_OFFSET_OF(reg[op->reg].u16), DYN_SRC, DYN_16bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(shr16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHR16;
    INCREMENT_EIP(op->len);
}
void dynamic_shr16cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instMemReg('>', DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(shr16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHR16;
    INCREMENT_EIP(op->len);
}
void dynamic_shr32_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        instCPUImm('>', CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, op->imm);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(shr32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHR32;
    INCREMENT_EIP(op->len);
}
void dynamic_shr32_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('>', DYN_ADDRESS, DYN_32bit, op->imm, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(shr32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SHR32;
    INCREMENT_EIP(op->len);
}
void dynamic_shr32cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instCPUReg('>', CPU_OFFSET_OF(reg[op->reg].u32), DYN_SRC, DYN_32bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(shr32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHR32;
    INCREMENT_EIP(op->len);
}
void dynamic_shr32cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instMemReg('>', DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(shr32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SHR32;
    INCREMENT_EIP(op->len);
}
void dynamic_sar8_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        instCPUImm(')', OFFSET_REG8(op->reg), DYN_8bit, op->imm);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(sar8_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SAR8;
    INCREMENT_EIP(op->len);
}
void dynamic_sar8_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm(')', DYN_ADDRESS, DYN_8bit, op->imm, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(sar8_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SAR8;
    INCREMENT_EIP(op->len);
}
void dynamic_sar8cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instCPUReg(')', OFFSET_REG8(op->reg), DYN_SRC, DYN_8bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(sar8cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SAR8;
    INCREMENT_EIP(op->len);
}
void dynamic_sar8cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instMemReg(')', DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(sar8cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SAR8;
    INCREMENT_EIP(op->len);
}
void dynamic_sar16_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        instCPUImm(')', CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, op->imm);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(sar16_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SAR16;
    INCREMENT_EIP(op->len);
}
void dynamic_sar16_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm(')', DYN_ADDRESS, DYN_16bit, op->imm, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(sar16_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SAR16;
    INCREMENT_EIP(op->len);
}
void dynamic_sar16cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instCPUReg(')', CPU_OFFSET_OF(reg[op->reg].u16), DYN_SRC, DYN_16bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(sar16cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SAR16;
    INCREMENT_EIP(op->len);
}
void dynamic_sar16cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instMemReg(')', DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(sar16cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SAR16;
    INCREMENT_EIP(op->len);
}
void dynamic_sar32_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        instCPUImm(')', CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, op->imm);
        INCREMENT_EIP(op->len);
        return;
    }
    callHostFunction(sar32_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SAR32;
    INCREMENT_EIP(op->len);
}
void dynamic_sar32_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm(')', DYN_ADDRESS, DYN_32bit, op->imm, true);
        INCREMENT_EIP(op->len);
        return;
    }
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(sar32_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_SAR32;
    INCREMENT_EIP(op->len);
}
void dynamic_sar32cl_reg_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instCPUReg(')', CPU_OFFSET_OF(reg[op->reg].u32), DYN_SRC, DYN_32bit, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    callHostFunction(sar32cl_reg, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SAR32;
    INCREMENT_EIP(op->len);
}
void dynamic_sar32cl_mem_op(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags()) {
        calculateEaa(op, DYN_ADDRESS);
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
        instMemReg(')', DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true);
        INCREMENT_EIP(op->len);
        return;
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(sar32cl_mem, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, DYN_SRC, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_SAR32;
    INCREMENT_EIP(op->len);
}
void dynamic_dshlr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction(dshlr16r16, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void dynamic_dshle16r16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(dshle16r16, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void dynamic_dshlr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction(dshlr32r32, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void dynamic_dshle32r32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(dshle32r32, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void dynamic_dshlclr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction(dshlclr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void dynamic_dshlcle16r16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(dshlcle16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(op->len);
}
void dynamic_dshlclr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction(dshlclr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void dynamic_dshlcle32r32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(dshlcle32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(op->len);
}
void dynamic_dshrr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction(dshrr16r16, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void dynamic_dshre16r16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(dshre16r16, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void dynamic_dshrr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction(dshrr32r32, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void dynamic_dshre32r32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(dshre32r32, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void dynamic_dshrclr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction(dshrclr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void dynamic_dshrcle16r16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(dshrcle16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(op->len);
}
void dynamic_dshrclr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction(dshrclr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void dynamic_dshrcle32r32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(dshrcle32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(op->len);
}
