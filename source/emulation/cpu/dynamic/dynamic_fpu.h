#include "../common/common_fpu.h"

#undef FPU_0
#define FPU_0(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {callHostFunction((void*)common_##name, false, 1, 0, DYN_PARAM_CPU, false);INCREMENT_EIP(data, op);}
#undef FPU_R
#define FPU_R(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {callHostFunction((void*)common_##name, false, 2, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false);INCREMENT_EIP(data, op);}
#undef FPU_A
#define FPU_A(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {calculateEaa(op, DYN_ADDRESS);callHostFunction((void*)common_##name, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);INCREMENT_EIP(data, op);}
#include "../common/common_fpu_def.h"
