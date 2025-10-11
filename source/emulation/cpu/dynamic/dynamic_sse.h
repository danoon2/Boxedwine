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
#define SSE_0(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {data->callHostFunction((void*)common_##name, false, 1, 0, DYN_PARAM_CPU, false);data->incrementEip(op->len);}
#undef SSE_RR
#define SSE_RR(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {data->callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);data->incrementEip(op->len);}
#undef SSE_RR_SETS_FLAGS
#define SSE_RR_SETS_FLAGS(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {data->currentLazyFlags=FLAGS_NONE;data->callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);data->incrementEip(op->len);}
#undef SSE_RE
#define SSE_RE(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {data->calculateEaa(op, DYN_ADDRESS);data->callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);data->incrementEip(op->len);}
#undef SSE_RE_SETS_FLAGS
#define SSE_RE_SETS_FLAGS(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {data->currentLazyFlags=FLAGS_NONE;data->calculateEaa(op, DYN_ADDRESS);data->callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);data->incrementEip(op->len);}
#undef SSE_RR_I8
#define SSE_RR_I8(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {data->callHostFunction((void*)common_##name, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, (U8)op->imm, DYN_PARAM_CONST_32, false);data->incrementEip(op->len);}
#undef SSE_RE_I8
#define SSE_RE_I8(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {data->calculateEaa(op, DYN_ADDRESS);data->callHostFunction((void*)common_##name, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true, (U8)op->imm, DYN_PARAM_CONST_32, false);data->incrementEip(op->len);}

#undef SSE_RR_EDI
#define SSE_RR_EDI(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {DynReg reg = data->loadReg(7, DYN_SRC, DYN_32bit); data->loadSegAddress(op->base, DYN_ADDRESS); data->instRegReg('+', DYN_ADDRESS, reg, DYN_32bit, true); data->callHostFunction((void*)common_##name, false, 4, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);data->incrementEip(op->len);}

#include "../common/common_sse_def.h"
