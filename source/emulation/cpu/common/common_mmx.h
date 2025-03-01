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

#ifndef __COMMON_MMX_H__
#define __COMMON_MMX_H__

#undef MMX_0
#define MMX_0(name) void common_##name(CPU* cpu);
#undef MMX_RR
#define MMX_RR(name) void common_##name(CPU* cpu, U32 r1, U32 r2);
#undef MMX_RE
#define MMX_RE(name) void common_##name(CPU* cpu, U32 reg, U32 address);
#undef MMX_RI
#define MMX_RI(name) void common_##name(CPU* cpu, U32 reg, U8 imm);
#include "../common/common_mmx_def.h"

#endif