#include "../common/common_bit.h"
void OPCALL dynamic_btr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btr16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btr16, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_16, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bte16r16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_bte16r16, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bte16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_bte16, false, false, false, 4, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_16, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btr32r32, false, false, false, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btr32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btr32, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bte32r32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_bte32r32, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bte32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_bte32, false, false, false, 4, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btsr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btsr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btsr16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btsr16, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_16, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btse16r16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_btse16r16, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btse16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_btse16, false, false, false, 4, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_16, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btsr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btsr32r32, false, false, false, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btsr32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btsr32, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btse32r32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_btse32r32, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btse32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_btse32, false, false, false, 4, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btrr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btrr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btrr16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btrr16, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_16, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btre16r16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_btre16r16, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btre16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_btre16, false, false, false, 4, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_16, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btrr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btrr32r32, false, false, false, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btrr32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btrr32, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btre32r32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_btre32r32, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btre32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_btre32, false, false, false, 4, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btcr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btcr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btcr16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btcr16, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_16, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btce16r16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_btce16r16, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btce16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_btce16, false, false, false, 4, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_16, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btcr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btcr32r32, false, false, false, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btcr32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_btcr32, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btce32r32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_btce32r32, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_btce32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_btce32, false, false, false, 4, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bsfr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_bsfr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bsfr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_bsfr16e16, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bsfr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_bsfr32r32, false, false, false, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bsfr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_bsfr32e32, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bsrr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_bsrr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bsrr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_bsrr16e16, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bsrr32r32(CPU* cpu, DecodedOp* op) {
    callHostFunction(NULL, common_bsrr32r32, false, false, false, 3, 0, DYN_PARAM_CPU, op->rm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bsrr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(NULL, common_bsrr32e32, false, false, false, 3, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
