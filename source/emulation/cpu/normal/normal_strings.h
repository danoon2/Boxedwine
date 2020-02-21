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

#ifndef __STRINGS_OP_H__
#define __STRINGS_OP_H__
#include "../common/cpu.h"
void movsb16(CPU* cpu, U32 base);
void movsb16r(CPU* cpu, U32 base);
void movsb32(CPU* cpu, U32 base);
void movsb32r(CPU* cpu, U32 base);
void movsw16(CPU* cpu, U32 base);
void movsw16r(CPU* cpu, U32 base);
void movsw32(CPU* cpu, U32 base);
void movsw32r(CPU* cpu, U32 base);
void movsd16(CPU* cpu, U32 base);
void movsd16r(CPU* cpu, U32 base);
void movsd32(CPU* cpu, U32 base);
void movsd32r(CPU* cpu, U32 base);
void cmpsb16(CPU* cpu, U32 rep_zero, U32 base);
void cmpsb16r(CPU* cpu, U32 rep_zero, U32 base);
void cmpsb32(CPU* cpu, U32 rep_zero, U32 base);
void cmpsb32r(CPU* cpu, U32 rep_zero, U32 base);
void cmpsw16(CPU* cpu, U32 rep_zero, U32 base);
void cmpsw16r(CPU* cpu, U32 rep_zero, U32 base);
void cmpsw32(CPU* cpu, U32 rep_zero, U32 base);
void cmpsw32r(CPU* cpu, U32 rep_zero, U32 base);
void cmpsd16(CPU* cpu, U32 rep_zero, U32 base);
void cmpsd16r(CPU* cpu, U32 rep_zero, U32 base);
void cmpsd32(CPU* cpu, U32 rep_zero, U32 base);
void cmpsd32r(CPU* cpu, U32 rep_zero, U32 base);
void stosb16(CPU* cpu);
void stosb16r(CPU* cpu);
void stosb32(CPU* cpu);
void stosb32r(CPU* cpu);
void stosw16(CPU* cpu);
void stosw16r(CPU* cpu);
void stosw32(CPU* cpu);
void stosw32r(CPU* cpu);
void stosd16(CPU* cpu);
void stosd16r(CPU* cpu);
void stosd32(CPU* cpu);
void stosd32r(CPU* cpu);
void lodsb16(CPU* cpu, U32 base);
void lodsb16r(CPU* cpu, U32 base);
void lodsb32(CPU* cpu, U32 base);
void lodsb32r(CPU* cpu, U32 base);
void lodsw16(CPU* cpu, U32 base);
void lodsw16r(CPU* cpu, U32 base);
void lodsw32(CPU* cpu, U32 base);
void lodsw32r(CPU* cpu, U32 base);
void lodsd16(CPU* cpu, U32 base);
void lodsd16r(CPU* cpu, U32 base);
void lodsd32(CPU* cpu, U32 base);
void lodsd32r(CPU* cpu, U32 base);
void scasb16(CPU* cpu, U32 rep_zero);
void scasb16r(CPU* cpu, U32 rep_zero);
void scasb32(CPU* cpu, U32 rep_zero);
void scasb32r(CPU* cpu, U32 rep_zero);
void scasw16(CPU* cpu, U32 rep_zero);
void scasw16r(CPU* cpu, U32 rep_zero);
void scasw32(CPU* cpu, U32 rep_zero);
void scasw32r(CPU* cpu, U32 rep_zero);
void scasd16(CPU* cpu, U32 rep_zero);
void scasd16r(CPU* cpu, U32 rep_zero);
void scasd32(CPU* cpu, U32 rep_zero);
void scasd32r(CPU* cpu, U32 rep_zero);
#endif
