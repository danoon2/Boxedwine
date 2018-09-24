void OPCALL dynamic_setO_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_o, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setO_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_o, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNO_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_no, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNO_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_no, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setB_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_b, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setB_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_b, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNB_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_nb, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNB_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_nb, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setZ_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_z, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setZ_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_z, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNZ_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_nz, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNZ_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_nz, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setBE_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_be, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setBE_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_be, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNBE_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_nbe, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNBE_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_nbe, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setS_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_s, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setS_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_s, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNS_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_ns, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNS_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_ns, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setP_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_p, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setP_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_p, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNP_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_np, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNP_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_np, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setL_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_l, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setL_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_l, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNL_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_nl, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNL_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_nl, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setLE_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_le, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setLE_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_le, op, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNLE_reg(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_nle, op);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_setNLE_mem(CPU* cpu, DecodedOp* op) {
    setCC(common_condition_nle, op, true);
    INCREMENT_EIP(op->len);
}
