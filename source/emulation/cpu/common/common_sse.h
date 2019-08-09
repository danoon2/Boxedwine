#ifndef __COMMON_SSE_H__
#define __COMMON_SSE_H__

#undef SSE_0
#define SSE_0(name) void common_##name(CPU* cpu);
#undef SSE_RR
#define SSE_RR(name) void common_##name(CPU* cpu, U32 r1, U32 r2);
#undef SSE_RE
#define SSE_RE(name) void common_##name(CPU* cpu, U32 reg, U32 address);
#undef SSE_RR_I8
#define SSE_RR_I8(name) void common_##name(CPU* cpu, U32 r1, U32 r2, U8 imm);
#undef SSE_RE_I8
#define SSE_RE_I8(name) void common_##name(CPU* cpu, U32 reg, U32 address, U8 imm);
#include "../common/common_sse_def.h"

#endif