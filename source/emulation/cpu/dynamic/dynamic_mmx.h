#include "../common/common_mmx.h"

#undef MMX_0
#define MMX_0(name) void OPCALL dynamic_##name(CPU* cpu, DecodedOp* op) {callHostFunction(common_##name, false, 1, 0, DYN_PARAM_CPU, false);INCREMENT_EIP(op->len);}
#undef MMX_RR
#define MMX_RR(name) void OPCALL dynamic_##name(CPU* cpu, DecodedOp* op) {callHostFunction(common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);INCREMENT_EIP(op->len);}
#undef MMX_RE
#define MMX_RE(name) void OPCALL dynamic_##name(CPU* cpu, DecodedOp* op) {calculateEaa(op, DYN_ADDRESS);callHostFunction(common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);INCREMENT_EIP(op->len);}
#undef MMX_RI
#define MMX_RI(name) void OPCALL dynamic_##name(CPU* cpu, DecodedOp* op) {callHostFunction(common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, (U8)op->imm, DYN_PARAM_CONST_32, false);INCREMENT_EIP(op->len);}
#include "../common/common_mmx_def.h"
