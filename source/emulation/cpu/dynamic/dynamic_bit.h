#include "../common/common_bit.h"
void dynamic_btr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btr16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btr16, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_bte16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_bte16r16, false, 3, 0, DYN_PARAM_CPU, false, (U32)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
    data->currentLazyFlags=FLAGS_NONE;
}
void dynamic_bte16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bte16, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
    data->currentLazyFlags=FLAGS_NONE;
}
void dynamic_btr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btr32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btr32, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_bte32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_bte32r32, false, 3, 0, DYN_PARAM_CPU, false, (U32)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_bte32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bte32, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btsr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btsr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btsr16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btsr16, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btse16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btse16r16, false, 3, 0, DYN_PARAM_CPU, false, (U32)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
    data->currentLazyFlags=FLAGS_NONE;
}
void dynamic_btse16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_btse16, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
    data->currentLazyFlags=FLAGS_NONE;
}
void dynamic_btsr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btsr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btsr32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btsr32, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btse32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btse32r32, false, 3, 0, DYN_PARAM_CPU, false, (U32)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btse32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_btse32, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btrr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btrr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btrr16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btrr16, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btre16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btre16r16, false, 3, 0, DYN_PARAM_CPU, false, (U32)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
    data->currentLazyFlags=FLAGS_NONE;
}
void dynamic_btre16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_btre16, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
    data->currentLazyFlags=FLAGS_NONE;
}
void dynamic_btrr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btrr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btrr32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btrr32, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btre32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btre32r32, false, 3, 0, DYN_PARAM_CPU, false, (U32)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btre32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_btre32, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btcr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btcr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btcr16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btcr16, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btce16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btce16r16, false, 3, 0, DYN_PARAM_CPU, false, (U32)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
    data->currentLazyFlags=FLAGS_NONE;
}
void dynamic_btce16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_btce16, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
    data->currentLazyFlags=FLAGS_NONE;
}
void dynamic_btcr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btcr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btcr32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btcr32, false, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btce32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_btce32r32, false, 3, 0, DYN_PARAM_CPU, false, (U32)op, DYN_PARAM_CONST_PTR, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_btce32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_btce32, false, 4, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_bsfr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_bsfr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_bsfr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bsfr16e16, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_bsfr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_bsfr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_bsfr32e32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bsfr32e32, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_bsrr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_bsrr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_bsrr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bsrr16e16, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_bsrr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_bsrr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->rm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_bsrr32e32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bsrr32e32, false, 3, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, op->reg, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
