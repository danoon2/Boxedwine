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

#include "../common/common_mmx.h"

#undef MMX_0
#define MMX_0(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {callHostFunction((void*)common_##name, false, 1, 0, DYN_PARAM_CPU, false);INCREMENT_EIP(data, op);}
#undef MMX_RR
#define MMX_RR(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);INCREMENT_EIP(data, op);}
#undef MMX_RE
#define MMX_RE(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {calculateEaa(op, DYN_ADDRESS);callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);INCREMENT_EIP(data, op);}
#undef MMX_RI
#define MMX_RI(name) void dynamic_##name(DynamicData* data, DecodedOp* op) {callHostFunction((void*)common_##name, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, (U8)op->imm, DYN_PARAM_CONST_32, false);INCREMENT_EIP(data, op);}
#include "../common/common_mmx_def.h"
