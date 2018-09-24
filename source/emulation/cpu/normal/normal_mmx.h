/*
 *  Copyright (C) 2016  The BoxedWine Team
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
#define MMX_0(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu);NEXT();}
#undef MMX_RR
#define MMX_RR(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu, op->reg, op->rm);NEXT();}
#undef MMX_RE
#define MMX_RE(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu, op->reg, eaa(cpu, op));NEXT();}
#undef MMX_RI
#define MMX_RI(name) void OPCALL normal_##name(CPU* cpu, DecodedOp* op) {START_OP(cpu, op); common_##name(cpu, op->reg, (U8)op->imm);NEXT();}
#include "../common/common_mmx_def.h"
