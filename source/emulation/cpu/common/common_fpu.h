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

#ifndef __COMMON_FPU_H__
#define __COMMON_FPU_H__

#undef FPU_0
#define FPU_0(name) void common_##name(CPU* cpu);
#undef FPU_R
#define FPU_R(name) void common_##name(CPU* cpu, U32 reg);
#undef FPU_A
#define FPU_A(name) void common_##name(CPU* cpu, U32 address);
#include "../common/common_fpu_def.h"

#endif