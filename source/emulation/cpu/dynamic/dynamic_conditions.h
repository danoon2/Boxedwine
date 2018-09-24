void OPCALL dynamic_cmovO_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_o, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovO_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_o, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovO_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_o, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovO_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_o, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNO_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_no, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNO_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_no, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNO_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_no, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNO_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_no, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovB_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_b, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovB_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_b, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovB_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_b, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovB_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_b, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNB_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nb, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNB_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nb, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNB_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nb, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNB_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nb, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovZ_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_z, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovZ_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_z, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovZ_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_z, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovZ_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_z, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNZ_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nz, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNZ_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nz, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNZ_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nz, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNZ_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nz, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovBE_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_be, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovBE_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_be, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovBE_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_be, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovBE_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_be, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNBE_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nbe, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNBE_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nbe, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNBE_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nbe, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNBE_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nbe, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovS_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_s, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovS_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_s, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovS_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_s, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovS_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_s, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNS_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_ns, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNS_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_ns, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNS_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_ns, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNS_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_ns, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovP_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_p, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovP_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_p, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovP_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_p, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovP_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_p, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNP_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_np, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNP_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_np, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNP_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_np, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNP_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_np, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovL_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_l, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovL_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_l, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovL_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_l, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovL_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_l, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNL_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nl, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNL_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nl, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNL_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nl, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNL_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nl, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovLE_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_le, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovLE_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_le, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovLE_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_le, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovLE_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_le, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNLE_16_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nle, op, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNLE_16_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nle, op, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNLE_32_reg(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nle, op, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNLE_32_mem(CPU* cpu, DecodedOp* op) {
    movCC(common_condition_nle, op, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
