void OPCALL dynamic_movr8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(OFFSET_REG8(op->reg), OFFSET_REG8(op->rm), DYN_8bit, DYN_ANY);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_move8r8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToRegFromCpu(DYN_SRC, OFFSET_REG8(op->reg), DYN_8bit); movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movr8e8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToCpuFromMem(OFFSET_REG8(op->reg), DYN_8bit, DYN_ADDRESS);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movr8(CPU* cpu, DecodedOp* op) {
    movToCpu(OFFSET_REG8(op->reg), DYN_8bit, op->imm);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_move8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToMemFromImm(DYN_ADDRESS, DYN_8bit, op->imm);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movr16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, reg[op->reg].u16), offsetof(CPU, reg[op->rm].u16), DYN_16bit, DYN_ANY);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_move16r16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].u16), DYN_16bit); movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToCpuFromMem(offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_ADDRESS);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movr16(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, reg[op->reg].u16), DYN_16bit, op->imm);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_move16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToMemFromImm(DYN_ADDRESS, DYN_16bit, op->imm);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movr32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, reg[op->reg].u32), offsetof(CPU, reg[op->rm].u32), DYN_32bit, DYN_ANY);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_move32r32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].u32), DYN_32bit); movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToCpuFromMem(offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_ADDRESS);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movr32(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, reg[op->reg].u32), DYN_32bit, op->imm);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_move32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToMemFromImm(DYN_ADDRESS, DYN_32bit, op->imm);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movr16s16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, seg[op->rm].value), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit);
     movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_SRC, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movr32s16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, reg[op->reg].u32), offsetof(CPU, seg[op->rm].value), DYN_32bit, DYN_ANY);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_move16s16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToRegFromCpu(DYN_SRC, offsetof(CPU, seg[op->reg].value), DYN_32bit); movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit); movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movs16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    callHostFunction(blockDone, common_setSegment, true, false, true, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32,  DYN_READ_RESULT, DYN_PARAM_REG_16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movs16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(blockDone, common_setSegment, true, false, true, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32,  offsetof(CPU, reg[op->reg].u16), DYN_PARAM_CPU_ADDRESS_16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movAlOb(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, seg[op->base].address), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);
    movFromMem(DYN_8bit, DYN_DEST);
    movToCpuFromReg(offsetof(CPU, reg[0].u8), DYN_READ_RESULT, DYN_8bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movAxOw(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, seg[op->base].address), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);
    movFromMem(DYN_16bit, DYN_DEST);
    movToCpuFromReg(offsetof(CPU, reg[0].u16), DYN_READ_RESULT, DYN_16bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movEaxOd(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, seg[op->base].address), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);
    movFromMem(DYN_32bit, DYN_DEST);
    movToCpuFromReg(offsetof(CPU, reg[0].u32), DYN_READ_RESULT, DYN_32bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movObAl(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, seg[op->base].address), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[0].u8), DYN_8bit);
    movToMemFromReg(DYN_DEST, DYN_SRC, DYN_8bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movOwAx(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, seg[op->base].address), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[0].u16), DYN_16bit);
    movToMemFromReg(DYN_DEST, DYN_SRC, DYN_16bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movOdEax(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, seg[op->base].address), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->disp);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[0].u32), DYN_32bit);
    movToMemFromReg(DYN_DEST, DYN_SRC, DYN_32bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movGwXzR8(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, OFFSET_REG8(op->rm), DYN_8bit); movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit); movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_SRC, DYN_16bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movGwXzE8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS);  movToRegFromReg(DYN_READ_RESULT, DYN_16bit, DYN_READ_RESULT, DYN_8bit); movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_READ_RESULT, DYN_16bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movGwSxR8(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, OFFSET_REG8(op->rm), DYN_8bit); movToRegFromRegSignExtend(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit); movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_SRC, DYN_16bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movGwSxE8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS);  movToRegFromRegSignExtend(DYN_READ_RESULT, DYN_16bit, DYN_READ_RESULT, DYN_8bit); movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_READ_RESULT, DYN_16bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movGdXzR8(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, OFFSET_REG8(op->rm), DYN_8bit); movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit); movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_SRC, DYN_32bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movGdXzE8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS);  movToRegFromReg(DYN_READ_RESULT, DYN_32bit, DYN_READ_RESULT, DYN_8bit); movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_READ_RESULT, DYN_32bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movGdSxR8(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, OFFSET_REG8(op->rm), DYN_8bit); movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit); movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_SRC, DYN_32bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movGdSxE8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS);  movToRegFromRegSignExtend(DYN_READ_RESULT, DYN_32bit, DYN_READ_RESULT, DYN_8bit); movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_READ_RESULT, DYN_32bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movGdXzR16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->rm].u16), DYN_16bit); movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit); movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_SRC, DYN_32bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movGdXzE16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS);  movToRegFromReg(DYN_READ_RESULT, DYN_32bit, DYN_READ_RESULT, DYN_16bit); movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_READ_RESULT, DYN_32bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movGdSxR16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->rm].u16), DYN_16bit); movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit); movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_SRC, DYN_32bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_movGdSxE16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS);  movToRegFromRegSignExtend(DYN_READ_RESULT, DYN_32bit, DYN_READ_RESULT, DYN_16bit); movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_READ_RESULT, DYN_32bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_leaR16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_ADDRESS, DYN_16bit);;
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_leaR32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_ADDRESS, DYN_32bit);;
    INCREMENT_EIP(op->len);
}
