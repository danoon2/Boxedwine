#include "../common/common_xchg.h"
void OPCALL dynamic_xchgr8r8(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, OFFSET_REG8(op->rm), DYN_8bit);
    movToCpuFromCpu(OFFSET_REG8(op->rm), OFFSET_REG8(op->reg), DYN_8bit, DYN_SRC, true);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_DEST, DYN_8bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xchge8r8(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_8bit, DYN_ADDRESS, false);
    movToRegFromCpu(DYN_DEST, OFFSET_REG8(op->reg), DYN_8bit);
    movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_8bit, true, true);
    movToCpuFromReg(OFFSET_REG8(op->reg), DYN_CALL_RESULT, DYN_8bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xchgr16r16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[op->rm].u16), DYN_16bit);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->rm].u16), CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit, DYN_SRC, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_DEST, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xchge16r16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, false);
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_16bit, true, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xchgr32r32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->rm].u32), CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_SRC, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_DEST, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xchge32r32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_CALL_RESULT, DYN_32bit, true);
    movToMemFromReg(DYN_ADDRESS, DYN_DEST, DYN_32bit, true, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpxchgr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_cmpxchgr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpxchge16r16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_cmpxchge16r16, false, false, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpxchgr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_cmpxchgr32r32, false, false, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpxchge32r32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_cmpxchge32r32, false, false, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
