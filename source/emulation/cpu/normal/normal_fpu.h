#include "../common/common_fpu.h"

#undef FPU_0
#define FPU_0(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu);NEXT();}
#undef FPU_R
#define FPU_R(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu, op->reg);NEXT();}
#undef FPU_A
#define FPU_A(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu, eaa(cpu, op));NEXT();}
#include "../common/common_fpu_def.h"
