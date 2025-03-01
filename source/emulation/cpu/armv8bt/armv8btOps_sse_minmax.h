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

#ifndef __ARMV8BTOPS_SSE_MINMAX_H__
#define __ARMV8BTOPS_SSE_MINMAX_H__

void opMaxpsXmm(Armv8btAsm* data);
void opMaxpsE128(Armv8btAsm* data);
void opMaxssXmm(Armv8btAsm* data);
void opMaxssE32(Armv8btAsm* data);
void opMinpsXmm(Armv8btAsm* data);
void opMinpsE128(Armv8btAsm* data);
void opMinssXmm(Armv8btAsm* data);
void opMinssE32(Armv8btAsm* data);
void opMaxpdXmmXmm(Armv8btAsm* data);
void opMaxpdXmmE128(Armv8btAsm* data);
void opMaxsdXmmXmm(Armv8btAsm* data);
void opMaxsdXmmE64(Armv8btAsm* data);
void opMinpdXmmXmm(Armv8btAsm* data);
void opMinpdXmmE128(Armv8btAsm* data);
void opMinsdXmmXmm(Armv8btAsm* data);
void opMinsdXmmE64(Armv8btAsm* data);

#endif