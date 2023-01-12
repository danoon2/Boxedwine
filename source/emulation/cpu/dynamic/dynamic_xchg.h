#include "../common/common_xchg.h"
void dynamic_xchgr8r8(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, OFFSET_REG8(op->rm), DYN_8bit);
    movToCpuFromCpu(OFFSET_REG8(op->rm), OFFSET_REG8(op->reg), DYN_8bit, DYN_SRC, true);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_xchge8r8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_8bit, DYN_ADDRESS, false);
    movToRegFromCpu(DYN_DEST, OFFSET_REG8(op->reg), DYN_8bit);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_CALL_RESULT, DYN_8bit, true);
    movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_8bit, true, true);    
    INCREMENT_EIP(data, op);
}
void dynamic_xchgr16r16(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF_REG16(op->rm), DYN_16bit);
    movToCpuFromCpu(CPU_OFFSET_OF_REG16(op->rm), CPU_OFFSET_OF_REG16(op->reg), DYN_16bit, DYN_SRC, true);
    movToCpuFromReg(CPU_OFFSET_OF_REG16(op->reg), DYN_DEST, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_xchge16r16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, false);
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF_REG16(op->reg), DYN_16bit);
    movToCpuFromReg(CPU_OFFSET_OF_REG16(op->reg), DYN_CALL_RESULT, DYN_16bit, true);
    movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_16bit, true, true);    
    INCREMENT_EIP(data, op);
}
void dynamic_xchgr32r32(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF_REG32(op->rm), DYN_32bit);
    movToCpuFromCpu(CPU_OFFSET_OF_REG32(op->rm), CPU_OFFSET_OF_REG32(op->reg), DYN_32bit, DYN_SRC, true);
    movToCpuFromReg(CPU_OFFSET_OF_REG32(op->reg), DYN_DEST, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_xchge32r32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF_REG32(op->reg), DYN_32bit);
    movToCpuFromReg(CPU_OFFSET_OF_REG32(op->reg), DYN_CALL_RESULT, DYN_32bit, true);
    movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_32bit, true, true);
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchgr8r8(DynamicData* data, DecodedOp* op) {
    callHostFunction(DYN_HOST_FN(common_cmpxchgr8r8), false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_CMP8;
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchge8r8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(DYN_HOST_FN(common_cmpxchge8r8), false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_CMP8;
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchgr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction(DYN_HOST_FN(common_cmpxchgr16r16), false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_CMP16;
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchge16r16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(DYN_HOST_FN(common_cmpxchge16r16), false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_CMP16;
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchgr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction(DYN_HOST_FN(common_cmpxchgr32r32), false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_CMP32;
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchge32r32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(DYN_HOST_FN(common_cmpxchge32r32), false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_CMP32;
    INCREMENT_EIP(data, op);
}
