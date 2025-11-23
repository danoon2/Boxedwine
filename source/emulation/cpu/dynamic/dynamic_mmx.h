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
#define MMX_0(name) void DynamicData::dynamic_##name(DecodedOp* op) {call(common_##name);incrementEip(op->len);}
#undef MMX_RR
#define MMX_RR(name) void DynamicData::dynamic_##name(DecodedOp* op) {call_II(common_##name, op->reg, op->rm);incrementEip(op->len);}
#undef MMX_RE
#define MMX_RE(name) void DynamicData::dynamic_##name(DecodedOp* op) {call_IR(common_##name, op->reg, JitWidth::b32, calculateEaaV2(op));incrementEip(op->len);}
#undef MMX_RI
#define MMX_RI(name) void DynamicData::dynamic_##name(DecodedOp* op) {call_II8(common_##name, op->reg, op->imm);incrementEip(op->len);}
#include "../common/common_mmx_def.h"
