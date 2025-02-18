#ifndef __COMMON_SSE2_H__
#define __COMMON_SSE2_H__

#undef SSE2_0
#define SSE2_0(name) void common_##name(CPU* cpu);
#undef SSE2_E
#define SSE2_E(name) void common_##name(CPU* cpu, U32 address);
#undef SSE2_RR
#define SSE2_RR(name) void common_##name(CPU* cpu, U32 r1, U32 r2);
#undef SSE2_RR_SETS_FLAGS
#define SSE2_RR_SETS_FLAGS(name) void common_##name(CPU* cpu, U32 r1, U32 r2);
#undef SSE2_RE
#define SSE2_RE(name) void common_##name(CPU* cpu, U32 reg, U32 address);
#undef SSE2_RE_SETS_FLAGS
#define SSE2_RE_SETS_FLAGS(name) void common_##name(CPU* cpu, U32 reg, U32 address);
#undef SSE2_RR_I8
#define SSE2_RR_I8(name) void common_##name(CPU* cpu, U32 r1, U32 r2, U8 imm);
#undef SSE2_RE_I8
#define SSE2_RE_I8(name) void common_##name(CPU* cpu, U32 reg, U32 address, U8 imm);
#undef SSE2_RR_EDI
#define SSE2_RR_EDI(name) void common_##name(CPU* cpu, U32 r1, U32 r2, U32 address);
#include "../common/common_sse2_def.h"

#endif