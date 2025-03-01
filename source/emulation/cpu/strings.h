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

#ifndef __STRINGS_OP_H__
#define __STRINGS_OP_H__
#include "cpu.h"
void movsb32_r(struct CPU* cpu, U32 base);
void movsb16_r(struct CPU* cpu, U32 base);
void movsb32(struct CPU* cpu, U32 base);
void movsb16(struct CPU* cpu, U32 base);
void movsw32_r(struct CPU* cpu, U32 base);
void movsw16_r(struct CPU* cpu, U32 base);
void movsw32(struct CPU* cpu, U32 base);
void movsw16(struct CPU* cpu, U32 base);
void movsd32_r(struct CPU* cpu, U32 base);
void movsd16_r(struct CPU* cpu, U32 base);
void movsd32(struct CPU* cpu, U32 base);
void movsd16(struct CPU* cpu, U32 base);
void cmpsb32_r(struct CPU* cpu, U32 rep_zero, U32 base);
void cmpsb16_r(struct CPU* cpu, U32 rep_zero, U32 base);
void cmpsb32(struct CPU* cpu, U32 rep_zero, U32 base);
void cmpsb16(struct CPU* cpu, U32 rep_zero, U32 base);
void cmpsw32_r(struct CPU* cpu, U32 rep_zero, U32 base);
void cmpsw16_r(struct CPU* cpu, U32 rep_zero, U32 base);
void cmpsw32(struct CPU* cpu, U32 rep_zero, U32 base);
void cmpsw16(struct CPU* cpu, U32 rep_zero, U32 base);
void cmpsd32_r(struct CPU* cpu, U32 rep_zero, U32 base);
void cmpsd16_r(struct CPU* cpu, U32 rep_zero, U32 base);
void cmpsd32(struct CPU* cpu, U32 rep_zero, U32 base);
void cmpsd16(struct CPU* cpu, U32 rep_zero, U32 base);
void stosb32_r(struct CPU* cpu);
void stosb16_r(struct CPU* cpu);
void stosb32(struct CPU* cpu);
void stosb16(struct CPU* cpu);
void stosw32_r(struct CPU* cpu);
void stosw16_r(struct CPU* cpu);
void stosw32(struct CPU* cpu);
void stosw16(struct CPU* cpu);
void stosd32_r(struct CPU* cpu);
void stosd16_r(struct CPU* cpu);
void stosd32(struct CPU* cpu);
void stosd16(struct CPU* cpu);
void lodsb32_r(struct CPU* cpu, U32 base);
void lodsb16_r(struct CPU* cpu, U32 base);
void lodsb32(struct CPU* cpu, U32 base);
void lodsb16(struct CPU* cpu, U32 base);
void lodsw32_r(struct CPU* cpu, U32 base);
void lodsw16_r(struct CPU* cpu, U32 base);
void lodsw32(struct CPU* cpu, U32 base);
void lodsw16(struct CPU* cpu, U32 base);
void lodsd32_r(struct CPU* cpu, U32 base);
void lodsd16_r(struct CPU* cpu, U32 base);
void lodsd32(struct CPU* cpu, U32 base);
void lodsd16(struct CPU* cpu, U32 base);
void scasb32_r(struct CPU* cpu, U32 rep_zero);
void scasb16_r(struct CPU* cpu, U32 rep_zero);
void scasb32(struct CPU* cpu, U32 rep_zero);
void scasb16(struct CPU* cpu, U32 rep_zero);
void scasw32_r(struct CPU* cpu, U32 rep_zero);
void scasw16_r(struct CPU* cpu, U32 rep_zero);
void scasw32(struct CPU* cpu, U32 rep_zero);
void scasw16(struct CPU* cpu, U32 rep_zero);
void scasd32_r(struct CPU* cpu, U32 rep_zero);
void scasd16_r(struct CPU* cpu, U32 rep_zero);
void scasd32(struct CPU* cpu, U32 rep_zero);
void scasd16(struct CPU* cpu, U32 rep_zero);
#endif
