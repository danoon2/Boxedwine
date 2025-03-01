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

#ifndef __ARMV8BTOPS_SSE_SHUFFLE_H__
#define __ARMV8BTOPS_SSE_SHUFFLE_H__

void opPshufwMmxMmx(Armv8btAsm* data);
void opPshufwMmxE64(Armv8btAsm* data);
void opPshufdXmmXmm(Armv8btAsm* data);
void opPshufdXmmE128(Armv8btAsm* data);
void opPshufhwXmmXmm(Armv8btAsm* data);
void opPshufhwXmmE128(Armv8btAsm* data);
void opPshuflwXmmXmm(Armv8btAsm* data);
void opPshuflwXmmE128(Armv8btAsm* data);
void opShufpdXmmXmm(Armv8btAsm* data);
void opShufpdXmmE128(Armv8btAsm* data);
void opShufpsXmmXmm(Armv8btAsm* data);
void opShufpsXmmE128(Armv8btAsm* data);
#endif