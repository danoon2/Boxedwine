void OPCALL dynamic_jumpO(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_o, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpNO(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_no, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpB(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_b, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpNB(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_nb, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpZ(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_z, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpNZ(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_nz, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpBE(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_be, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpNBE(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_nbe, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpS(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_s, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpNS(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_ns, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpP(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_p, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpNP(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_np, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpL(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_l, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpNL(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_nl, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpLE(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_le, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
void OPCALL dynamic_jumpNLE(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len);
    callHostFunction(blockNext2, common_condition_nle, true, false, true, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->imm);
    blockNext1();
}
