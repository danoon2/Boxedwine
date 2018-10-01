#include "../common/common_fpu.h"

#undef FPU_0
#define FPU_0(name) void OPCALL dynamic_##name(CPU* cpu, DecodedOp* op) {callHostFunction(NULL, common_##name, false, false, false, 1, 0, DYN_PARAM_CPU, false);INCREMENT_EIP(op->len);}
#undef FPU_R
#define FPU_R(name) void OPCALL dynamic_##name(CPU* cpu, DecodedOp* op) {callHostFunction(NULL, common_##name, false, false, false, 2, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false);INCREMENT_EIP(op->len);}
#undef FPU_A
#define FPU_A(name) void OPCALL dynamic_##name(CPU* cpu, DecodedOp* op) {calculateEaa(op, DYN_ADDRESS);callHostFunction(NULL, common_##name, false, false, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);INCREMENT_EIP(op->len);}
#include "../common/common_fpu_def.h"
