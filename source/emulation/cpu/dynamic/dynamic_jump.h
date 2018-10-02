void OPCALL dynamic_jumpO(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_o, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpNO(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_no, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpB(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_b, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpNB(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nb, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpZ(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_z, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpNZ(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nz, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpBE(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_be, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpNBE(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nbe, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpS(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_s, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpNS(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_ns, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpP(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_p, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpNP(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_np, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpL(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_l, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpNL(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nl, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpLE(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_le, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
void OPCALL dynamic_jumpNLE(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nle, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(op->imm+op->len);
    blockNext1();
    endIf();
}
