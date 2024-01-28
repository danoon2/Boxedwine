void dynamic_pushEw_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popEw_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushEw_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    INCREMENT_EIP(data, op);
}
void dynamic_popEw_mem(DynamicData* data, DecodedOp* op) {    
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    calculateEaa(op, DYN_ADDRESS);
    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushEd_reg(DynamicData* data, DecodedOp* op) {
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        movToRegFromCpu(DYN_ADDRESS, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
        instRegImm('-', DYN_ADDRESS, DYN_32bit, 4);
        movToRegFromCpu(DYN_SRC, CPU::offsetofReg32(op->reg), DYN_32bit);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, false, true);
        movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_ADDRESS, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_popEd_reg(DynamicData* data, DecodedOp* op) {
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        movToRegFromCpu(DYN_ADDRESS, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
        movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);
        instCPUImm('+', CPU_OFFSET_OF(reg[4].u32), DYN_32bit, 4);
    } else {
        callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
        movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_CALL_RESULT, DYN_32bit, true);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_pushEd_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        movToRegFromCpu(DYN_ADDRESS, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
        instRegImm('-', DYN_ADDRESS, DYN_32bit, 4);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, false, true);
        movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_ADDRESS, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_popEd_mem(DynamicData* data, DecodedOp* op) {    
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
        movFromMem(DYN_32bit, DYN_SRC, true);
        calculateEaa(op, DYN_ADDRESS);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true);
        instCPUImm('+', CPU_OFFSET_OF(reg[4].u32), DYN_32bit, 4);
    } else {
        callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
        calculateEaa(op, DYN_ADDRESS);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_pushSeg16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofSegValue(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popSeg16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_peek16, true, 2, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false);
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(stackMask), DYN_32bit);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
    movToRegFromReg(DYN_ADDRESS, DYN_32bit, DYN_SRC, DYN_32bit, false);
    instRegImm('+', DYN_SRC, DYN_32bit, 2);
    instRegReg('&', DYN_SRC, DYN_DEST, DYN_32bit, true);
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(stackNotMask), DYN_32bit);
    instRegReg('&', DYN_ADDRESS, DYN_DEST, DYN_32bit, true);
    instRegReg('|', DYN_SRC, DYN_ADDRESS, DYN_32bit, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushSeg32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofSegValue(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popSeg32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_peek32, true, 2, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false);
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(stackMask), DYN_32bit);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
    movToRegFromReg(DYN_ADDRESS, DYN_32bit, DYN_SRC, DYN_32bit, false);
    instRegImm('+', DYN_SRC, DYN_32bit, 4);
    instRegReg('&', DYN_SRC, DYN_DEST, DYN_32bit, true);
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(stackNotMask), DYN_32bit);
    instRegReg('&', DYN_ADDRESS, DYN_DEST, DYN_32bit, true);
    instRegReg('|', DYN_SRC, DYN_ADDRESS, DYN_32bit, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushA16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_pushA16, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_pushA32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_pushA32, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popA16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_popA16, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popA32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_popA32, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_push16imm(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_push32imm(DynamicData* data, DecodedOp* op) {
     if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
         movToRegFromCpu(DYN_ADDRESS, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
         instRegImm('-', DYN_ADDRESS, DYN_32bit, 4);
         movToMemFromImm(DYN_ADDRESS, DYN_32bit, op->imm, false);
         movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_ADDRESS, DYN_32bit, true);
     } else {
         callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
     }
    INCREMENT_EIP(data, op);
}
void dynamic_popf16(DynamicData* data, DecodedOp* op) {
    movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_NONE);
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    callHostFunction((void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, FMASK_ALL & 0xFFFF, DYN_PARAM_CONST_16, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_popf32(DynamicData* data, DecodedOp* op) {
    movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_NONE);
    callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
    callHostFunction((void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, FMASK_ALL, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_pushf16(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(flags), DYN_32bit);
    instRegImm('|', DYN_SRC, DYN_32bit, 2);
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushf32(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(flags), DYN_32bit);
    instRegImm('|', DYN_SRC, DYN_32bit, 2);
    instRegImm('&', DYN_SRC, DYN_32bit, 0xFCFFFF);
    dynamic_pushReg32(data, DYN_SRC, true);
    INCREMENT_EIP(data, op);
}
