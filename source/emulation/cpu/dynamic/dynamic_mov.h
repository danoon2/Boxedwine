void dynamic_movr8r8(DynamicData* data, DecodedOp* op) {
    movToCpuFromCpu(CPU::offsetofReg8(op->reg), CPU::offsetofReg8(op->rm), DYN_8bit, DYN_ANY, true);
    INCREMENT_EIP(data, op);
}
void dynamic_move8r8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToRegFromCpu(DYN_SRC, CPU::offsetofReg8(op->reg), DYN_8bit); movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr8e8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToCpuFromMem(CPU::offsetofReg8(op->reg), DYN_8bit, DYN_ADDRESS, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr8(DynamicData* data, DecodedOp* op) {
    movToCpu(CPU::offsetofReg8(op->reg), DYN_8bit, op->imm);
    INCREMENT_EIP(data, op);
}
void dynamic_move8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToMemFromImm(DYN_ADDRESS, DYN_8bit, op->imm, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr16r16(DynamicData* data, DecodedOp* op) {
    movToCpuFromCpu(CPU::offsetofReg16(op->reg), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_ANY, true);
    INCREMENT_EIP(data, op);
}
void dynamic_move16r16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToRegFromCpu(DYN_SRC, CPU::offsetofReg16(op->reg), DYN_16bit); movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToCpuFromMem(CPU::offsetofReg16(op->reg), DYN_16bit, DYN_ADDRESS, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr16(DynamicData* data, DecodedOp* op) {
    movToCpu(CPU::offsetofReg16(op->reg), DYN_16bit, op->imm);
    INCREMENT_EIP(data, op);
}
void dynamic_move16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToMemFromImm(DYN_ADDRESS, DYN_16bit, op->imm, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr32r32(DynamicData* data, DecodedOp* op) {
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_ANY, true);
    INCREMENT_EIP(data, op);
}
void dynamic_move32r32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToRegFromCpu(DYN_SRC, CPU::offsetofReg32(op->reg), DYN_32bit); movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr32e32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr32(DynamicData* data, DecodedOp* op) {
    movToCpu(CPU::offsetofReg32(op->reg), DYN_32bit, op->imm);
    INCREMENT_EIP(data, op);
}
void dynamic_move32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToMemFromImm(DYN_ADDRESS, DYN_32bit, op->imm, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr16s16(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU::offsetofSegValue(op->rm), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit, false);
    movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_SRC, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movr32s16(DynamicData* data, DecodedOp* op) {
    movToCpuFromCpu(CPU::offsetofReg32(op->reg), CPU::offsetofSegValue(op->rm), DYN_32bit, DYN_ANY, true);
    INCREMENT_EIP(data, op);
}
void dynamic_move16s16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToRegFromCpu(DYN_SRC, CPU::offsetofSegValue(op->reg), DYN_32bit); movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit, false); movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movs16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_movs16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_movAlOb(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU::offsetofSegAddress(op->base), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);
    movFromMem(DYN_8bit, DYN_DEST, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[0].u8), DYN_CALL_RESULT, DYN_8bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movAxOw(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU::offsetofSegAddress(op->base), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);
    movFromMem(DYN_16bit, DYN_DEST, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[0].u16), DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movEaxOd(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU::offsetofSegAddress(op->base), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);
    movFromMem(DYN_32bit, DYN_DEST, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[0].u32), DYN_CALL_RESULT, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movObAl(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU::offsetofSegAddress(op->base), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[0].u8), DYN_8bit);
    movToMemFromReg(DYN_DEST, DYN_SRC, DYN_8bit, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movOwAx(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU::offsetofSegAddress(op->base), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[0].u16), DYN_16bit);
    movToMemFromReg(DYN_DEST, DYN_SRC, DYN_16bit, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movOdEax(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU::offsetofSegAddress(op->base), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[0].u32), DYN_32bit);
    movToMemFromReg(DYN_DEST, DYN_SRC, DYN_32bit, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGwXzR8(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg8(op->rm), DYN_8bit); movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit, false); movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_SRC, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGwXzE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);  movToRegFromReg(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_8bit, false); movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGwSxR8(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg8(op->rm), DYN_8bit); movToRegFromRegSignExtend(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit, false); movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_SRC, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGwSxE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);  movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_8bit, false); movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdXzR8(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg8(op->rm), DYN_8bit); movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false); movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdXzE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);  movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false); movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_CALL_RESULT, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdSxR8(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg8(op->rm), DYN_8bit); movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false); movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdSxE8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS, true);  movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_8bit, false); movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_CALL_RESULT, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdXzR16(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg16(op->rm), DYN_16bit); movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false); movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdXzE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);  movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false); movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_CALL_RESULT, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdSxR16(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg16(op->rm), DYN_16bit); movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false); movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movGdSxE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS, true);  movToRegFromRegSignExtend(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false); movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_CALL_RESULT, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_movRdCRx(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_readCrx, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_movCRxRd(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg32(op->reg), DYN_32bit);
    callHostFunction((void*)common_writeCrx, true, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(data, op);
}

void dynamic_leaR16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_ADDRESS, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_leaR32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_ADDRESS, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
