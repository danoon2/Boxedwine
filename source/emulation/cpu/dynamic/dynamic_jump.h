void dynamic_jumpO(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_o, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNO(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_no, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpB(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_b, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNB(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nb, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpZ(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_z, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNZ(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nz, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpBE(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_be, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNBE(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nbe, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpS(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_s, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNS(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_ns, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpP(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_p, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNP(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_np, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpL(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_l, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNL(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nl, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpLE(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_le, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNLE(DynamicData* data, DecodedOp* op) {
    callHostFunction(common_condition_nle, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
