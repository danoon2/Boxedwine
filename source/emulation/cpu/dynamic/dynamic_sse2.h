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
#define SSE2_0(name) void DynamicData::dynamic_##name(DecodedOp* op) {call(common_##name);incrementEip(op->len);}
#undef SSE2_E
#define SSE2_E(name) void DynamicData::dynamic_##name(DecodedOp* op) {call_R(common_##name, JitWidth::b32, calculateEaaV2(op));incrementEip(op->len);}
#undef SSE2_RR
#define SSE2_RR(name) void DynamicData::dynamic_##name(DecodedOp* op) {call_II(common_##name, op->reg, op->rm);incrementEip(op->len);}
#undef SSE2_RR_SETS_FLAGS
#define SSE2_RR_SETS_FLAGS(name) void DynamicData::dynamic_##name(DecodedOp* op) {currentLazyFlags=FLAGS_NONE;call_II(common_##name, op->reg, op->rm);incrementEip(op->len);}
#undef SSE2_RE
#define SSE2_RE(name) void DynamicData::dynamic_##name(DecodedOp* op) {call_IR(common_##name, op->reg, JitWidth::b32, calculateEaaV2(op));incrementEip(op->len);}
#undef SSE2_RE_SETS_FLAGS
#define SSE2_RE_SETS_FLAGS(name) void DynamicData::dynamic_##name(DecodedOp* op) {currentLazyFlags=FLAGS_NONE;call_IR(common_##name, op->reg, JitWidth::b32, calculateEaaV2(op));incrementEip(op->len);}
#undef SSE2_RR_I8
#define SSE2_RR_I8(name) void DynamicData::dynamic_##name(DecodedOp* op) {call_III8(common_##name, op->reg, op->rm, op->imm);incrementEip(op->len);}
#undef SSE2_RE_I8
#define SSE2_RE_I8(name) void DynamicData::dynamic_##name(DecodedOp* op) {call_IRI8(common_##name, op->reg, JitWidth::b32, calculateEaaV2(op), op->imm);incrementEip(op->len);}

#undef SSE2_RR_EDI
#define SSE2_RR_EDI(name) void DynamicData::dynamic_##name(DecodedOp* op) {RegPtr address = getTmpReg(7); addReg(JitWidth::b32, address, getReadOnlySegAddress(op->base)); call_IIR(common_##name, op->reg, op->rm, JitWidth::b32, address);incrementEip(op->len);}

#include "../common/common_sse2_def.h"
