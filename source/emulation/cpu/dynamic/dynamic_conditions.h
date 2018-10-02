void OPCALL dynamic_cmovO_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_o, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovO_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_o, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovO_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_o, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovO_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_o, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNO_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_no, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNO_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_no, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNO_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_no, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNO_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_no, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovB_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_b, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovB_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_b, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovB_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_b, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovB_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_b, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNB_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nb, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNB_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_nb, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNB_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nb, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNB_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_nb, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovZ_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_z, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovZ_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_z, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovZ_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_z, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovZ_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_z, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNZ_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nz, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNZ_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_nz, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNZ_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nz, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNZ_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_nz, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovBE_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_be, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovBE_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_be, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovBE_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_be, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovBE_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_be, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNBE_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nbe, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNBE_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_nbe, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNBE_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nbe, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNBE_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_nbe, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovS_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_s, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovS_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_s, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovS_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_s, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovS_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_s, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNS_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_ns, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNS_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_ns, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNS_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_ns, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNS_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_ns, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovP_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_p, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovP_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_p, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovP_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_p, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovP_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_p, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNP_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_np, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNP_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_np, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNP_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_np, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNP_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_np, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovL_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_l, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovL_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_l, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovL_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_l, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovL_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_l, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNL_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nl, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNL_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_nl, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNL_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nl, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNL_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_nl, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovLE_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_le, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovLE_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_le, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovLE_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_le, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovLE_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_le, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNLE_16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nle, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u16), CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNLE_16_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_nle, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNLE_32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_condition_nle, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmovNLE_32_mem(CPU* cpu, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    callHostFunction(common_condition_nle, true, 1, 0, DYN_PARAM_CPU, false);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(op->len);
}
