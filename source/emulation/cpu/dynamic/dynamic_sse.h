#include "../common/common_sse.h"

#undef SSE_0
#define SSE_0(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {callHostFunction((void*)common_##name, false, 1, 0, DYN_PARAM_CPU, false);INCREMENT_EIP(data, op);}
#undef SSE_RR
#define SSE_RR(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);INCREMENT_EIP(data, op);}
#undef SSE_RE
#define SSE_RE(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {calculateEaa(op, DYN_ADDRESS);callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);INCREMENT_EIP(data, op);}
#undef SSE_RR_I8
#define SSE_RR_I8(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {callHostFunction((void*)common_##name, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, (U8)op->imm, DYN_PARAM_CONST_32, false);INCREMENT_EIP(data, op);}
#undef SSE_RE_I8
#define SSE_RE_I8(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {calculateEaa(op, DYN_ADDRESS);callHostFunction((void*)common_##name, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, (U8)op->imm, DYN_PARAM_CONST_32, false);INCREMENT_EIP(data, op);}

#undef SSE_RR_EDI
#define SSE_RR_EDI(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[7].u32), DYN_32bit); movToRegFromCpu(DYN_ADDRESS, CPU::offsetofSegAddress(op->base), DYN_32bit); instRegReg('+', DYN_ADDRESS, DYN_SRC, DYN_32bit, true); callHostFunction((void*)common_##name, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);INCREMENT_EIP(data, op);}

#include "../common/common_sse_def.h"
