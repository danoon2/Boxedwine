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

#ifndef __ARMV8BTOPS_SHIFT_H__
#define __ARMV8BTOPS_SHIFT_H__

void opRolR8I8(Armv8btAsm* data);
void opRolE8I8(Armv8btAsm* data);
void opRorR8I8(Armv8btAsm* data);
void opRorE8I8(Armv8btAsm* data);
void opRclR8I8(Armv8btAsm* data);
void opRclE8I8(Armv8btAsm* data);
void opRcrR8I8(Armv8btAsm* data);
void opRcrE8I8(Armv8btAsm* data);
void opShlR8I8(Armv8btAsm* data);
void opShlE8I8(Armv8btAsm* data);
void opShrR8I8(Armv8btAsm* data);
void opShrE8I8(Armv8btAsm* data);
void opSarR8I8(Armv8btAsm* data);
void opSarE8I8(Armv8btAsm* data);
void opRolR16I8(Armv8btAsm* data);
void opRolE16I8(Armv8btAsm* data);
void opRorR16I8(Armv8btAsm* data);
void opRorE16I8(Armv8btAsm* data);
void opRclR16I8(Armv8btAsm* data);
void opRclE16I8(Armv8btAsm* data);
void opRcrR16I8(Armv8btAsm* data);
void opRcrE16I8(Armv8btAsm* data);
void opShlR16I8(Armv8btAsm* data);
void opShlE16I8(Armv8btAsm* data);
void opShrR16I8(Armv8btAsm* data);
void opShrE16I8(Armv8btAsm* data);
void opSarR16I8(Armv8btAsm* data);
void opSarE16I8(Armv8btAsm* data);
void opRolR32I8(Armv8btAsm* data);
void opRolE32I8(Armv8btAsm* data);
void opRorR32I8(Armv8btAsm* data);
void opRorE32I8(Armv8btAsm* data);
void opRclR32I8(Armv8btAsm* data);
void opRclE32I8(Armv8btAsm* data);
void opRcrR32I8(Armv8btAsm* data);
void opRcrE32I8(Armv8btAsm* data);
void opShlR32I8(Armv8btAsm* data);
void opShlE32I8(Armv8btAsm* data);
void opShrR32I8(Armv8btAsm* data);
void opShrE32I8(Armv8btAsm* data);
void opSarR32I8(Armv8btAsm* data);
void opSarE32I8(Armv8btAsm* data);
void opRolR8Cl(Armv8btAsm* data);
void opRolE8Cl(Armv8btAsm* data);
void opRorR8Cl(Armv8btAsm* data);
void opRorE8Cl(Armv8btAsm* data);
void opRclR8Cl(Armv8btAsm* data);
void opRclE8Cl(Armv8btAsm* data);
void opRcrR8Cl(Armv8btAsm* data);
void opRcrE8Cl(Armv8btAsm* data);
void opShlR8Cl(Armv8btAsm* data);
void opShlE8Cl(Armv8btAsm* data);
void opShrR8Cl(Armv8btAsm* data);
void opShrE8Cl(Armv8btAsm* data);
void opSarR8Cl(Armv8btAsm* data);
void opSarE8Cl(Armv8btAsm* data);

void opRolR16Cl(Armv8btAsm* data);
void opRolE16Cl(Armv8btAsm* data);
void opRorR16Cl(Armv8btAsm* data);
void opRorE16Cl(Armv8btAsm* data);
void opRclR16Cl(Armv8btAsm* data);
void opRclE16Cl(Armv8btAsm* data);
void opRcrR16Cl(Armv8btAsm* data);
void opRcrE16Cl(Armv8btAsm* data);
void opShlR16Cl(Armv8btAsm* data);
void opShlE16Cl(Armv8btAsm* data);
void opShrR16Cl(Armv8btAsm* data);
void opShrE16Cl(Armv8btAsm* data);
void opSarR16Cl(Armv8btAsm* data);
void opSarE16Cl(Armv8btAsm* data);

void opRolR32Cl(Armv8btAsm* data);
void opRolE32Cl(Armv8btAsm* data);
void opRorR32Cl(Armv8btAsm* data);
void opRorE32Cl(Armv8btAsm* data);
void opRclR32Cl(Armv8btAsm* data);
void opRclE32Cl(Armv8btAsm* data);
void opRcrR32Cl(Armv8btAsm* data);
void opRcrE32Cl(Armv8btAsm* data);
void opShlR32Cl(Armv8btAsm* data);
void opShlE32Cl(Armv8btAsm* data);
void opShrR32Cl(Armv8btAsm* data);
void opShrE32Cl(Armv8btAsm* data);
void opSarR32Cl(Armv8btAsm* data);
void opSarE32Cl(Armv8btAsm* data);

void opDshlR16R16(Armv8btAsm* data);
void opDshlE16R16(Armv8btAsm* data);
void opDshlClR16R16(Armv8btAsm* data);
void opDshlClE16R16(Armv8btAsm* data);
void opDshrR16R16(Armv8btAsm* data);
void opDshrE16R16(Armv8btAsm* data);
void opDshrClR16R16(Armv8btAsm* data);
void opDshrClE16R16(Armv8btAsm* data);
void opDshlR32R32(Armv8btAsm* data);
void opDshlE32R32(Armv8btAsm* data);
void opDshlClR32R32(Armv8btAsm* data);
void opDshlClE32R32(Armv8btAsm* data);
void opDshrR32R32(Armv8btAsm* data);
void opDshrE32R32(Armv8btAsm* data);
void opDshrClR32R32(Armv8btAsm* data);
void opDshrClE32R32(Armv8btAsm* data);
#endif