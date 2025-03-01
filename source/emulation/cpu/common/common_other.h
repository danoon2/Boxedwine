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

U32 common_bound16(CPU* cpu, U32 reg, U32 address);
U32 common_bound32(CPU* cpu, U32 reg, U32 address);
void common_int99(CPU* cpu);
void common_int9A(CPU* cpu);
void common_int9B(CPU* cpu);
void common_int3(CPU* cpu);
void common_intIb(CPU* cpu);
void common_cpuid(CPU* cpu);
void common_loopnz32(CPU* cpu, U32 offset1, U32 offset2);
void common_loopnz(CPU* cpu, U32 offset1, U32 offset2);
void common_loopz32(CPU* cpu, U32 offset1, U32 offset2);
void common_loopz(CPU* cpu, U32 offset1, U32 offset2);
void common_loop32(CPU* cpu, U32 offset1, U32 offset2);
void common_loop(CPU* cpu, U32 offset1, U32 offset2);
void common_jcxz32(CPU* cpu, U32 offset1, U32 offset2);
void common_jcxz(CPU* cpu, U32 offset1, U32 offset2);
void common_larr16r16(CPU* cpu, U32 dstReg, U32 srcReg);
void common_larr16e16(CPU* cpu, U32 reg, U32 address);
U32 common_lar(CPU* cpu, U32 selector, U32 ar);
void common_lslr16r16(CPU* cpu, U32 dstReg, U32 srcReg);
void common_lslr16e16(CPU* cpu, U32 reg, U32 address);
void common_lslr32r32(CPU* cpu, U32 dstReg, U32 srcReg);
void common_lslr32e32(CPU* cpu, U32 reg, U32 address);
U32 common_lsl(CPU* cpu, U32 selector, U32 limit);
void common_verre16(CPU* cpu, U32 address);
void common_verwe16(CPU* cpu, U32 address);
void common_verr(CPU* cpu, U32 selector);
void common_verw(CPU* cpu, U32 selector);
void common_cmpxchg8b(CPU* cpu, U32 address);
#ifdef BOXEDWINE_MULTI_THREADED
void common_cmpxchg8b_lock(CPU* cpu, U32 address);
#endif
void common_fxsave(CPU* cpu, U32 address);
void common_fxrstor(CPU* cpu, U32 address);
void common_xsave(CPU* cpu, U32 address);
void common_xrstor(CPU* cpu, U32 address);