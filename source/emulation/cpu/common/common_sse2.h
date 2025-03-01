/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

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