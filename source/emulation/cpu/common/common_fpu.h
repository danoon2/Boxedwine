#ifndef __COMMON_FPU_H__
#define __COMMON_FPU_H__

#undef FPU_0
#define FPU_0(name) void common_##name(CPU* cpu);
#undef FPU_R
#define FPU_R(name) void common_##name(CPU* cpu, U32 reg);
#undef FPU_A
#define FPU_A(name) void common_##name(CPU* cpu, U32 address);
#include "../common/common_fpu_def.h"

#endif