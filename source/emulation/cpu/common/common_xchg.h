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

void common_cmpxchgr8r8(CPU* cpu, U32 dstReg, U32 srcReg);
void common_cmpxchge8r8(CPU* cpu, U32 address, U32 srcReg);
void common_cmpxchgr16r16(CPU* cpu, U32 dstReg, U32 srcReg);
void common_cmpxchge16r16(CPU* cpu, U32 address, U32 srcReg);
void common_cmpxchgr32r32(CPU* cpu, U32 dstReg, U32 srcReg);
void common_cmpxchge32r32(CPU* cpu, U32 address, U32 srcReg);
