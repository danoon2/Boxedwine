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

void common_dimul16(CPU* cpu, U32 arg1, U32 arg2, U32 regResult);
void common_dimul32(CPU* cpu, U32 arg1, U32 arg2, U32 regResult);
void common_imul8(CPU* cpu, U8 src);
void common_mul8(CPU* cpu, U8 src);
void common_imul16(CPU* cpu, U16 src);
void common_mul16(CPU* cpu, U16 src);
void common_imul32(CPU* cpu, U32 src);
void common_mul32(CPU* cpu, U32 src);
