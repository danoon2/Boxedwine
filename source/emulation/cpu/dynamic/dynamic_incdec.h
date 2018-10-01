void OPCALL dynamic_inc8_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
    movToCpuFromCpu(CPU_OFFSET_OF(dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST, false);
    instRegImm('+', DYN_DEST, DYN_8bit, 1);
    movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_DEST, DYN_8bit, false);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_INC8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_inc8_mem32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(CPU_OFFSET_OF(dst.u8), DYN_8bit, DYN_ADDRESS, false, false);
    instRegImm('+', DYN_CALL_RESULT, DYN_8bit, 1);
    movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_CALL_RESULT, DYN_8bit, false);
    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_INC8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_inc16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
    movToCpuFromCpu(CPU_OFFSET_OF(dst.u16), CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_DEST, false);
    instRegImm('+', DYN_DEST, DYN_16bit, 1);
    movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_DEST, DYN_16bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_DEST, DYN_16bit, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_INC16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_inc16_mem32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(CPU_OFFSET_OF(dst.u16), DYN_16bit, DYN_ADDRESS, false, false);
    instRegImm('+', DYN_CALL_RESULT, DYN_16bit, 1);
    movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_CALL_RESULT, DYN_16bit, false);
    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_INC16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_inc32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
    movToCpuFromCpu(CPU_OFFSET_OF(dst.u32), CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_DEST, false);
    instRegImm('+', DYN_DEST, DYN_32bit, 1);
    movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_DEST, DYN_32bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_DEST, DYN_32bit, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_INC32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_inc32_mem32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(CPU_OFFSET_OF(dst.u32), DYN_32bit, DYN_ADDRESS, false, false);
    instRegImm('+', DYN_CALL_RESULT, DYN_32bit, 1);
    movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_CALL_RESULT, DYN_32bit, false);
    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_INC32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dec8_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
    movToCpuFromCpu(CPU_OFFSET_OF(dst.u8), OFFSET_REG8(op->reg), DYN_8bit, DYN_DEST, false);
    instRegImm('-', DYN_DEST, DYN_8bit, 1);
    movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_DEST, DYN_8bit, false);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_DEC8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dec8_mem32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(CPU_OFFSET_OF(dst.u8), DYN_8bit, DYN_ADDRESS, false, false);
    instRegImm('-', DYN_CALL_RESULT, DYN_8bit, 1);
    movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_CALL_RESULT, DYN_8bit, false);
    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_DEC8);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dec16_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
    movToCpuFromCpu(CPU_OFFSET_OF(dst.u16), CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_DEST, false);
    instRegImm('-', DYN_DEST, DYN_16bit, 1);
    movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_DEST, DYN_16bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_DEST, DYN_16bit, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_DEC16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dec16_mem32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(CPU_OFFSET_OF(dst.u16), DYN_16bit, DYN_ADDRESS, false, false);
    instRegImm('-', DYN_CALL_RESULT, DYN_16bit, 1);
    movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_CALL_RESULT, DYN_16bit, false);
    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_DEC16);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dec32_reg(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
    movToCpuFromCpu(CPU_OFFSET_OF(dst.u32), CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_DEST, false);
    instRegImm('-', DYN_DEST, DYN_32bit, 1);
    movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_DEST, DYN_32bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_DEST, DYN_32bit, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_DEC32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dec32_mem32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_getCF, true, false, false, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromMem(CPU_OFFSET_OF(dst.u32), DYN_32bit, DYN_ADDRESS, false, false);
    instRegImm('-', DYN_CALL_RESULT, DYN_32bit, 1);
    movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_CALL_RESULT, DYN_32bit, false);
    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_DEC32);
    INCREMENT_EIP(op->len);
}
