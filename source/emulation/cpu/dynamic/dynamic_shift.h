#include "../normal/normal_shift.h"
void OPCALL dynamic_rol8_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, rol8_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rol8_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rol8_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rol8cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, rol8cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rol8cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rol8cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rol16_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, rol16_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rol16_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rol16_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rol16cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, rol16cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rol16cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rol16cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rol32_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, rol32_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rol32_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rol32_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rol32cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, rol32cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rol32cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rol32cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ror8_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, ror8_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ror8_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, ror8_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ror8cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, ror8cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ror8cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, ror8cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ror16_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, ror16_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ror16_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, ror16_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ror16cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, ror16cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ror16cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, ror16cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ror32_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, ror32_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ror32_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, ror32_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ror32cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, ror32cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_ror32cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, ror32cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcl8_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, rcl8_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcl8_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rcl8_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcl8cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, rcl8cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcl8cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rcl8cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcl16_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, rcl16_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcl16_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rcl16_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcl16cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, rcl16cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcl16cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rcl16cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcl32_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, rcl32_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcl32_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rcl32_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcl32cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, rcl32cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcl32cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rcl32cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcr8_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, rcr8_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcr8_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rcr8_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcr8cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, rcr8cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcr8cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rcr8cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcr16_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, rcr16_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcr16_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rcr16_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcr16cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, rcr16cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcr16cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rcr16cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcr32_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, rcr32_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcr32_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rcr32_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcr32cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, rcr32cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rcr32cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, rcr32cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shl8_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, shl8_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shl8_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, shl8_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shl8cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, shl8cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shl8cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, shl8cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shl16_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, shl16_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shl16_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, shl16_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shl16cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, shl16cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shl16cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, shl16cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shl32_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, shl32_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shl32_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, shl32_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shl32cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, shl32cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shl32cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, shl32cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shr8_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, shr8_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shr8_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, shr8_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shr8cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, shr8cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shr8cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, shr8cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shr16_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, shr16_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shr16_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, shr16_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shr16cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, shr16cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shr16cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, shr16cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shr32_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, shr32_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shr32_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, shr32_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shr32cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, shr32cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_shr32cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, shr32cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sar8_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, sar8_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sar8_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, sar8_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sar8cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, sar8cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sar8cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, sar8cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sar16_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, sar16_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sar16_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, sar16_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sar16cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, sar16cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sar16cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, sar16cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sar32_reg_op(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, sar32_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sar32_mem_op(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, sar32_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sar32cl_reg_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    callHostFunction(NULL, sar32cl_reg, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sar32cl_mem_op(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[1].u8), DYN_8bit);
    instRegImm('&', DYN_SRC, DYN_8bit, 0x1F);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, sar32cl_mem, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshlr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, dshlr16r16, false, false, false, 4, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->rm, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshle16r16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, dshle16r16, false, false, false, 4, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshlr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, dshlr32r32, false, false, false, 4, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->rm, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshle32r32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, dshle32r32, false, false, false, 4, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshlclr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, dshlclr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->rm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshlcle16r16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, dshlcle16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshlclr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, dshlclr32r32, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->rm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshlcle32r32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, dshlcle32r32, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshrr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, dshrr16r16, false, false, false, 4, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->rm, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshre16r16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, dshre16r16, false, false, false, 4, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshrr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, dshrr32r32, false, false, false, 4, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->rm, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshre32r32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, dshre32r32, false, false, false, 4, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshrclr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, dshrclr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->rm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshrcle16r16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, dshrcle16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshrclr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, dshrclr32r32, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->rm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_dshrcle32r32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, dshrcle32r32, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
