void dynamic_cmovO_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, O, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovO_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, O, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovO_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, O, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovO_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, O, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNO_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, O, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNO_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, O, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNO_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, O, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNO_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, O, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovB_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, B, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovB_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, B, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovB_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, B, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovB_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, B, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNB_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, B, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNB_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, B, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNB_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, B, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNB_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, B, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovZ_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, NZ, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovZ_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, NZ, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovZ_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, NZ, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovZ_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, NZ, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNZ_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, NZ, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNZ_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, NZ, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNZ_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, NZ, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNZ_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, NZ, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovBE_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, BE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovBE_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, BE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovBE_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, BE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovBE_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, BE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNBE_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, BE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNBE_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, BE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNBE_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, BE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNBE_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, BE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovS_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, S, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovS_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, S, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovS_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, S, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovS_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, S, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNS_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, S, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNS_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, S, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNS_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, S, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNS_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, S, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovP_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, P, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovP_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, P, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovP_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, P, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovP_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, P, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNP_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, NP, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNP_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, NP, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNP_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, NP, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNP_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, NP, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovL_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, L, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovL_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, L, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovL_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, L, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovL_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, L, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNL_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, L, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNL_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, L, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNL_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, L, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNL_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, L, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovLE_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, LE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovLE_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, LE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovLE_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, LE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovLE_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, LE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNLE_16_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, LE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNLE_16_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, LE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNLE_32_reg(DynamicData* data, DecodedOp* op) {
    setConditionInReg(data, LE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_SRC, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_cmovNLE_32_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa( op, DYN_ADDRESS);
    setConditionInReg(data, LE, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    endIf();
    INCREMENT_EIP(data, op);
}
