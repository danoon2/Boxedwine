#ifndef __COMMON_MMX_H__
#define __COMMON_MMX_H__

#undef MMX_0
#define MMX_0(name) void common_##name(CPU* cpu);
#undef MMX_RR
#define MMX_RR(name) void common_##name(CPU* cpu, U32 r1, U32 r2);
#undef MMX_RE
#define MMX_RE(name) void common_##name(CPU* cpu, U32 reg, U32 address);
#undef MMX_RI
#define MMX_RI(name) void common_##name(CPU* cpu, U32 reg, U8 imm);
#include "../common/common_mmx_def.h"

#endif