void OPCALL dynamic_addr8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->rm), DYN_8bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('+', DYN_DEST, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adde8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegReg('+', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_addr8e8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u8), DYN_8bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('+', DYN_DEST, DYN_READ_RESULT, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_add8_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegImm('+', DYN_DEST, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_add8_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegImm('+', DYN_READ_RESULT, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_addr16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->rm].u16), DYN_16bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('+', DYN_DEST, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adde16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegReg('+', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_addr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u16), DYN_16bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('+', DYN_DEST, DYN_READ_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_add16_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegImm('+', DYN_DEST, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_add16_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegImm('+', DYN_READ_RESULT, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_addr32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->rm].u32), DYN_32bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('+', DYN_DEST, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adde32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegReg('+', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_addr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u32), DYN_32bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('+', DYN_DEST, DYN_READ_RESULT, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_add32_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegImm('+', DYN_DEST, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_add32_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegImm('+', DYN_READ_RESULT, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_orr8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->rm), DYN_8bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('|', DYN_DEST, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ore8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegReg('|', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_orr8e8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u8), DYN_8bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('|', DYN_DEST, DYN_READ_RESULT, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_or8_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegImm('|', DYN_DEST, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_or8_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegImm('|', DYN_READ_RESULT, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_orr16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->rm].u16), DYN_16bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('|', DYN_DEST, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ore16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegReg('|', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_orr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u16), DYN_16bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('|', DYN_DEST, DYN_READ_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_or16_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegImm('|', DYN_DEST, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_or16_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegImm('|', DYN_READ_RESULT, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_orr32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->rm].u32), DYN_32bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('|', DYN_DEST, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ore32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegReg('|', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_orr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u32), DYN_32bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('|', DYN_DEST, DYN_READ_RESULT, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_or32_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegImm('|', DYN_DEST, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_or32_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegImm('|', DYN_READ_RESULT, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_OR32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adcr8r8(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->rm), DYN_8bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('+', DYN_DEST, DYN_SRC, DYN_8bit);
    movToRegFromReg(DYN_CALL_RESULT, DYN_8bit, DYN_CALL_RESULT, DYN_32bit);
    instRegReg('+', DYN_DEST, DYN_CALL_RESULT, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adce8r8(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegReg('+', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_8bit, DYN_SRC, DYN_32bit);
    instRegReg('+', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adcr8e8(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u8), DYN_8bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('+', DYN_DEST, DYN_READ_RESULT, DYN_8bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_8bit, DYN_SRC, DYN_32bit);
    instRegReg('+', DYN_DEST, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adc8_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegImm('+', DYN_DEST, DYN_8bit, op->imm);
    movToRegFromReg(DYN_CALL_RESULT, DYN_8bit, DYN_CALL_RESULT, DYN_32bit);
    instRegReg('+', DYN_DEST, DYN_CALL_RESULT, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adc8_mem(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegImm('+', DYN_READ_RESULT, DYN_8bit, op->imm);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_8bit, DYN_SRC, DYN_32bit);
    instRegReg('+', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adcr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->rm].u16), DYN_16bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('+', DYN_DEST, DYN_SRC, DYN_16bit);
    movToRegFromReg(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_32bit);
    instRegReg('+', DYN_DEST, DYN_CALL_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adce16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegReg('+', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit);
    instRegReg('+', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adcr16e16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u16), DYN_16bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('+', DYN_DEST, DYN_READ_RESULT, DYN_16bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit);
    instRegReg('+', DYN_DEST, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adc16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegImm('+', DYN_DEST, DYN_16bit, op->imm);
    movToRegFromReg(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_32bit);
    instRegReg('+', DYN_DEST, DYN_CALL_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adc16_mem(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegImm('+', DYN_READ_RESULT, DYN_16bit, op->imm);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit);
    instRegReg('+', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adcr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->rm].u32), DYN_32bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('+', DYN_DEST, DYN_SRC, DYN_32bit);
    instRegReg('+', DYN_DEST, DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adce32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegReg('+', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    instRegReg('+', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adcr32e32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u32), DYN_32bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('+', DYN_DEST, DYN_READ_RESULT, DYN_32bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    instRegReg('+', DYN_DEST, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adc32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegImm('+', DYN_DEST, DYN_32bit, op->imm);
    instRegReg('+', DYN_DEST, DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_adc32_mem(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegImm('+', DYN_READ_RESULT, DYN_32bit, op->imm);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    instRegReg('+', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADC32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbbr8r8(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->rm), DYN_8bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_SRC, DYN_8bit);
    movToRegFromReg(DYN_CALL_RESULT, DYN_8bit, DYN_CALL_RESULT, DYN_32bit);
    instRegReg('-', DYN_DEST, DYN_CALL_RESULT, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbbe8r8(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_8bit, DYN_SRC, DYN_32bit);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbbr8e8(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u8), DYN_8bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_READ_RESULT, DYN_8bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_8bit, DYN_SRC, DYN_32bit);
    instRegReg('-', DYN_DEST, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbb8_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegImm('-', DYN_DEST, DYN_8bit, op->imm);
    movToRegFromReg(DYN_CALL_RESULT, DYN_8bit, DYN_CALL_RESULT, DYN_32bit);
    instRegReg('-', DYN_DEST, DYN_CALL_RESULT, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbb8_mem(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegImm('-', DYN_READ_RESULT, DYN_8bit, op->imm);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_8bit, DYN_SRC, DYN_32bit);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbbr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->rm].u16), DYN_16bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_SRC, DYN_16bit);
    movToRegFromReg(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_32bit);
    instRegReg('-', DYN_DEST, DYN_CALL_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbbe16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbbr16e16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u16), DYN_16bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_READ_RESULT, DYN_16bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit);
    instRegReg('-', DYN_DEST, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbb16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegImm('-', DYN_DEST, DYN_16bit, op->imm);
    movToRegFromReg(DYN_CALL_RESULT, DYN_16bit, DYN_CALL_RESULT, DYN_32bit);
    instRegReg('-', DYN_DEST, DYN_CALL_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbb16_mem(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegImm('-', DYN_READ_RESULT, DYN_16bit, op->imm);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_32bit);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbbr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->rm].u32), DYN_32bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_SRC, DYN_32bit);
    instRegReg('-', DYN_DEST, DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbbe32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbbr32e32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u32), DYN_32bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_READ_RESULT, DYN_32bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    instRegReg('-', DYN_DEST, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbb32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegImm('-', DYN_DEST, DYN_32bit, op->imm);
    instRegReg('-', DYN_DEST, DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sbb32_mem(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, oldCF), DYN_CALL_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegImm('-', DYN_READ_RESULT, DYN_32bit, op->imm);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, oldCF), DYN_32bit);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SBB32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_andr8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->rm), DYN_8bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('&', DYN_DEST, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ande8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegReg('&', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_andr8e8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u8), DYN_8bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('&', DYN_DEST, DYN_READ_RESULT, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_and8_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegImm('&', DYN_DEST, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_and8_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegImm('&', DYN_READ_RESULT, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_andr16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->rm].u16), DYN_16bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('&', DYN_DEST, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ande16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegReg('&', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_andr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u16), DYN_16bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('&', DYN_DEST, DYN_READ_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_and16_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegImm('&', DYN_DEST, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_and16_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegImm('&', DYN_READ_RESULT, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_andr32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->rm].u32), DYN_32bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('&', DYN_DEST, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ande32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegReg('&', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_andr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u32), DYN_32bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('&', DYN_DEST, DYN_READ_RESULT, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_and32_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegImm('&', DYN_DEST, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_and32_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegImm('&', DYN_READ_RESULT, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_AND32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_subr8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->rm), DYN_8bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sube8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_subr8e8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u8), DYN_8bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_READ_RESULT, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sub8_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegImm('-', DYN_DEST, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sub8_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegImm('-', DYN_READ_RESULT, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_subr16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->rm].u16), DYN_16bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sube16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_subr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u16), DYN_16bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_READ_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sub16_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegImm('-', DYN_DEST, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sub16_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegImm('-', DYN_READ_RESULT, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_subr32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->rm].u32), DYN_32bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sube32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_subr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u32), DYN_32bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_READ_RESULT, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sub32_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegImm('-', DYN_DEST, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sub32_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegImm('-', DYN_READ_RESULT, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_SUB32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xorr8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->rm), DYN_8bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('^', DYN_DEST, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xore8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegReg('^', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xorr8e8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u8), DYN_8bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('^', DYN_DEST, DYN_READ_RESULT, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xor8_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegImm('^', DYN_DEST, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xor8_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegImm('^', DYN_READ_RESULT, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xorr16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->rm].u16), DYN_16bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('^', DYN_DEST, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xore16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegReg('^', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xorr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u16), DYN_16bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('^', DYN_DEST, DYN_READ_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xor16_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegImm('^', DYN_DEST, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xor16_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegImm('^', DYN_READ_RESULT, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xorr32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->rm].u32), DYN_32bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('^', DYN_DEST, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xore32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegReg('^', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xorr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u32), DYN_32bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('^', DYN_DEST, DYN_READ_RESULT, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xor32_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegImm('^', DYN_DEST, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xor32_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegImm('^', DYN_READ_RESULT, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_XOR32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpr8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->rm), DYN_8bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpe8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpr8e8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u8), DYN_8bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_READ_RESULT, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmp8_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegImm('-', DYN_DEST, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmp8_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegImm('-', DYN_READ_RESULT, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpr16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->rm].u16), DYN_16bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpe16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u16), DYN_16bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_READ_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmp16_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegImm('-', DYN_DEST, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmp16_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegImm('-', DYN_READ_RESULT, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpr32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->rm].u32), DYN_32bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpe32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegReg('-', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u32), DYN_32bit, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('-', DYN_DEST, DYN_READ_RESULT, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmp32_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegImm('-', DYN_DEST, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmp32_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegImm('-', DYN_READ_RESULT, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_CMP32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_testr8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->rm), DYN_8bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegReg('&', DYN_DEST, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_TEST8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_teste8r8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegReg('&', DYN_READ_RESULT, DYN_SRC, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_TEST8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_test8_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instRegImm('&', DYN_DEST, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_TEST8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_test8_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u8), DYN_8bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u8), DYN_8bit, DYN_ADDRESS);
    instRegImm('&', DYN_READ_RESULT, DYN_8bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_TEST8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_testr16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->rm].u16), DYN_16bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegReg('&', DYN_DEST, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_TEST16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_teste16r16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegReg('&', DYN_READ_RESULT, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_TEST16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_test16_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instRegImm('&', DYN_DEST, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_TEST16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_test16_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u16), DYN_16bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u16), DYN_16bit, DYN_ADDRESS);
    instRegImm('&', DYN_READ_RESULT, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_TEST16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_testr32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->rm].u32), DYN_32bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegReg('&', DYN_DEST, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_TEST32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_teste32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegReg('&', DYN_READ_RESULT, DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_TEST32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_test32_reg(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instRegImm('&', DYN_DEST, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_TEST32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_test32_mem(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, src.u32), DYN_32bit, op->imm);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, dst.u32), DYN_32bit, DYN_ADDRESS);
    instRegImm('&', DYN_READ_RESULT, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_TEST32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_notr8(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, OFFSET_REG8(op->reg), DYN_8bit);
    instReg('~', DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_note8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_8bit, DYN_ADDRESS);
    instReg('~', DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_notr16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, reg[op->reg].u16), DYN_16bit);
    instReg('~', DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_note16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    instReg('~', DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_notr32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, reg[op->reg].u32), DYN_32bit);
    instReg('~', DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_note32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS);
    instReg('~', DYN_READ_RESULT, DYN_32bit);
    ;
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_negr8(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST);
    instReg('-', DYN_DEST, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_DEST, DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_NEG8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_nege8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u8), DYN_8bit, DYN_ADDRESS);
    instReg('-', DYN_READ_RESULT, DYN_8bit);
    movToCpuFromReg(offsetof(CPU, result.u8), DYN_READ_RESULT, DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_8bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_NEG8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_negr16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u16), offsetof(CPU, reg[op->reg].u16), DYN_16bit, DYN_DEST);
    instReg('-', DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_DEST, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_NEG16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_nege16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u16), DYN_16bit, DYN_ADDRESS);
    instReg('-', DYN_READ_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, result.u16), DYN_READ_RESULT, DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_16bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_NEG16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_negr32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_DEST);
    instReg('-', DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_NEG32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_nege32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(offsetof(CPU, src.u32), DYN_32bit, DYN_ADDRESS);
    instReg('-', DYN_READ_RESULT, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_READ_RESULT, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_NEG32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_mulR8(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_mul8, false, false, false, 2, 0, DYN_PARAM_CPU, OFFSET_REG8(op->reg), DYN_PARAM_CPU_ADDRESS_8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_mulE8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS);
    callHostFunction(NULL, common_mul8, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_imulR8(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_imul8, false, false, false, 2, 0, DYN_PARAM_CPU, OFFSET_REG8(op->reg), DYN_PARAM_CPU_ADDRESS_8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_imulE8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS);
    callHostFunction(NULL, common_imul8, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_mulR16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_mul16, false, false, false, 2, 0, DYN_PARAM_CPU, offsetof(CPU, reg[op->reg].u16), DYN_PARAM_CPU_ADDRESS_16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_mulE16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS);
    callHostFunction(NULL, common_mul16, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_imulR16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_imul16, false, false, false, 2, 0, DYN_PARAM_CPU, offsetof(CPU, reg[op->reg].u16), DYN_PARAM_CPU_ADDRESS_16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_imulE16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS);
    callHostFunction(NULL, common_imul16, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_mulR32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_mul32, false, false, false, 2, 0, DYN_PARAM_CPU, offsetof(CPU, reg[op->reg].u32), DYN_PARAM_CPU_ADDRESS_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_mulE32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS);
    callHostFunction(NULL, common_mul32, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_imulR32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_imul32, false, false, false, 2, 0, DYN_PARAM_CPU, offsetof(CPU, reg[op->reg].u32), DYN_PARAM_CPU_ADDRESS_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_imulE32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS);
    callHostFunction(NULL, common_imul32, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_divR8(CPU* cpu, DecodedOp* op) {
    callHostFunction(blockDone, div8, true, false, true, 2, 0, DYN_PARAM_CPU, OFFSET_REG8(op->reg), DYN_PARAM_CPU_ADDRESS_8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_divE8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS);
    callHostFunction(blockDone, div8, true, false, true, 2, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_idivR8(CPU* cpu, DecodedOp* op) {
    callHostFunction(blockDone, idiv8, true, false, true, 2, 0, DYN_PARAM_CPU, OFFSET_REG8(op->reg), DYN_PARAM_CPU_ADDRESS_8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_idivE8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_8bit, DYN_ADDRESS);
    callHostFunction(blockDone, idiv8, true, false, true, 2, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_divR16(CPU* cpu, DecodedOp* op) {
    callHostFunction(blockDone, div16, true, false, true, 2, 0, DYN_PARAM_CPU, offsetof(CPU, reg[op->reg].u16), DYN_PARAM_CPU_ADDRESS_16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_divE16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS);
    callHostFunction(blockDone, div16, true, false, true, 2, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_idivR16(CPU* cpu, DecodedOp* op) {
    callHostFunction(blockDone, idiv16, true, false, true, 2, 0, DYN_PARAM_CPU, offsetof(CPU, reg[op->reg].u16), DYN_PARAM_CPU_ADDRESS_16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_idivE16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS);
    callHostFunction(blockDone, idiv16, true, false, true, 2, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_divR32(CPU* cpu, DecodedOp* op) {
    callHostFunction(blockDone, div32, true, false, true, 2, 0, DYN_PARAM_CPU, offsetof(CPU, reg[op->reg].u32), DYN_PARAM_CPU_ADDRESS_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_divE32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS);
    callHostFunction(blockDone, div32, true, false, true, 2, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_idivR32(CPU* cpu, DecodedOp* op) {
    callHostFunction(blockDone, idiv32, true, false, true, 2, 0, DYN_PARAM_CPU, offsetof(CPU, reg[op->reg].u32), DYN_PARAM_CPU_ADDRESS_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_idivE32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS);
    callHostFunction(blockDone, idiv32, true, false, true, 2, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dimulcr16r16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, reg[op->rm].u16), DYN_16bit);
    callHostFunction(NULL, common_dimul16, false, false, false, 4, 0, DYN_PARAM_CPU, DYN_DEST, DYN_PARAM_REG_16, op->imm, DYN_PARAM_CONST_16, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dimulcr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS);
    callHostFunction(NULL, common_dimul16, false, false, false, 4, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_16, op->imm, DYN_PARAM_CONST_16, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dimulcr32r32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, reg[op->rm].u32), DYN_32bit);
    callHostFunction(NULL, common_dimul32, false, false, false, 4, 0, DYN_PARAM_CPU, DYN_DEST, DYN_PARAM_REG_32,op->imm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dimulcr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS);
    callHostFunction(NULL, common_dimul32, false, false, false, 4, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_32,op->imm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dimulr16r16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, reg[op->rm].u16), DYN_16bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].u16), DYN_16bit);
    callHostFunction(NULL, common_dimul16, false, false, false, 4, 0, DYN_PARAM_CPU, DYN_DEST, DYN_PARAM_REG_16, DYN_SRC, DYN_PARAM_REG_16, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dimulr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_16bit, DYN_ADDRESS);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].u16), DYN_16bit);
    callHostFunction(NULL, common_dimul16, false, false, false, 4, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_16, DYN_SRC, DYN_PARAM_REG_16, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dimulr32r32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, reg[op->rm].u32), DYN_32bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].u32), DYN_32bit);
    callHostFunction(NULL, common_dimul32, false, false, false, 4, 0, DYN_PARAM_CPU, DYN_DEST, DYN_PARAM_REG_32,DYN_SRC, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dimulr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS); movFromMem(DYN_32bit, DYN_ADDRESS);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].u32), DYN_32bit);
    callHostFunction(NULL, common_dimul32, false, false, false, 4, 0, DYN_PARAM_CPU, DYN_READ_RESULT, DYN_PARAM_REG_32,DYN_SRC, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
