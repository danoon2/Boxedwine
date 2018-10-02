void OPCALL dynamic_setO_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_o, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setO_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setNO_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_no, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNO_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setB_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_b, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setB_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setNB_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nb, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNB_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setZ_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_z, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setZ_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setNZ_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nz, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNZ_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setBE_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_be, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setBE_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setNBE_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nbe, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNBE_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setS_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_s, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setS_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setNS_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_ns, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNS_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setP_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_p, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setP_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setNP_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_np, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNP_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setL_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_l, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setL_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setNL_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nl, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNL_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setLE_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_le, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setLE_mem(CPU* cpu, DecodedOp* op) {
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
void OPCALL dynamic_setNLE_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nle, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 1);
    startElse();
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, 0);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNLE_mem(CPU* cpu, DecodedOp* op) {
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
