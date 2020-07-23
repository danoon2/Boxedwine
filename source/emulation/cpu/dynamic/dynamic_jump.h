void dynamic_jumpO(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, O, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNO(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, O, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpB(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, B, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNB(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, B, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpZ(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, NZ, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNZ(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, NZ, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpBE(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, BE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNBE(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, BE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpS(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, S, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNS(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, S, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpP(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, P, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNP(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, NP, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpL(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, L, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNL(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, L, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpLE(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, LE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
void dynamic_jumpNLE(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, LE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    INCREMENT_EIP(data, op->len);
    blockNext2();
    startElse();
    INCREMENT_EIP(data, op->imm+op->len);
    blockNext1();
    endIf();
}
