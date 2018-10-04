void dynamic_setO_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_o, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setO_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_o, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setNO_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_no, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setNO_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_no, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setB_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_b, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setB_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_b, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setNB_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nb, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setNB_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nb, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setZ_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_z, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setZ_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_z, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setNZ_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nz, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setNZ_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nz, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setBE_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_be, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setBE_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_be, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setNBE_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nbe, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setNBE_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nbe, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setS_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_s, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setS_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_s, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setNS_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_ns, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setNS_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_ns, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setP_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_p, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setP_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_p, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setNP_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_np, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setNP_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_np, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setL_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_l, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setL_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_l, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setNL_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nl, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setNL_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nl, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setLE_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_le, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setLE_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_le, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
void dynamic_setNLE_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nle, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void dynamic_setNLE_mem(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nle, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToReg(DYN_SRC, DYN_8bit, 1);
    startElse();
    movToReg(DYN_SRC, DYN_8bit, 0);
    endIf();
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(op->len);
}
