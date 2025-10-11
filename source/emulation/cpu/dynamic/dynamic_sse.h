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

#include "../common/common_sse.h"

#undef SSE_0
#define SSE_0(name) void DynamicData::dynamic_##name(DecodedOp* op) {callHostFunction((void*)common_##name, false, 1, 0, DYN_PARAM_CPU, false);incrementEip(op->len);}
#undef SSE_RR
#define SSE_RR(name) void DynamicData::dynamic_##name(DecodedOp* op) {callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);incrementEip(op->len);}
#undef SSE_RR_SETS_FLAGS
#define SSE_RR_SETS_FLAGS(name) void DynamicData::dynamic_##name(DecodedOp* op) {currentLazyFlags=FLAGS_NONE;callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);incrementEip(op->len);}
#undef SSE_RE
#define SSE_RE(name) void DynamicData::dynamic_##name(DecodedOp* op) {calculateEaa(op, DYN_ADDRESS);callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);incrementEip(op->len);}
#undef SSE_RE_SETS_FLAGS
#define SSE_RE_SETS_FLAGS(name) void DynamicData::dynamic_##name(DecodedOp* op) {currentLazyFlags=FLAGS_NONE;calculateEaa(op, DYN_ADDRESS);callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);incrementEip(op->len);}
#undef SSE_RR_I8
#define SSE_RR_I8(name) void DynamicData::dynamic_##name(DecodedOp* op) {callHostFunction((void*)common_##name, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, (U8)op->imm, DYN_PARAM_CONST_32, false);incrementEip(op->len);}
#undef SSE_RE_I8
#define SSE_RE_I8(name) void DynamicData::dynamic_##name(DecodedOp* op) {calculateEaa(op, DYN_ADDRESS);callHostFunction((void*)common_##name, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, (U8)op->imm, DYN_PARAM_CONST_32, false);incrementEip(op->len);}

#undef SSE_RR_EDI
#define SSE_RR_EDI(name) void DynamicData::dynamic_##name(DecodedOp* op) {DynReg reg = loadReg(7, DYN_SRC, DYN_32bit); loadSegAddress(op->base, DYN_ADDRESS); instRegReg('+', DYN_ADDRESS, reg, DYN_32bit, true); callHostFunction((void*)common_##name, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);incrementEip(op->len);}

#include "../common/common_sse_def.h"
