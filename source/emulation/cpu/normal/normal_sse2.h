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

#include "../common/common_sse2.h"

#undef SSE2_0
#define SSE2_0(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu);NEXT();}
#undef SSE2_E
#define SSE2_E(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu, eaa(cpu, op));NEXT();}
#undef SSE2_RR
#define SSE2_RR(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu, op->reg, op->rm);NEXT();}
#undef SSE2_RR_SETS_FLAGS
#define SSE2_RR_SETS_FLAGS(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); cpu->fillFlags(); common_##name(cpu, op->reg, op->rm);NEXT();}
#undef SSE2_RE
#define SSE2_RE(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu, op->reg, eaa(cpu, op));NEXT();}
#undef SSE2_RE_SETS_FLAGS
#define SSE2_RE_SETS_FLAGS(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); cpu->fillFlags(); common_##name(cpu, op->reg, eaa(cpu, op));NEXT();}
#undef SSE2_RR_I8
#define SSE2_RR_I8(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu, op->reg, op->rm, (U8)op->imm);NEXT();}
#undef SSE2_RE_I8
#define SSE2_RE_I8(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu, op->reg, eaa(cpu, op), (U8)op->imm);NEXT();}

#undef SSE2_RR_EDI
#define SSE2_RR_EDI(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu, op->reg, op->rm, EDI+cpu->seg[op->base].address);NEXT();}

#include "../common/common_sse2_def.h"
