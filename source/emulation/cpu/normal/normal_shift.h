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

#ifndef __SHIFT_OP_H__
#define __SHIFT_OP_H__
#include "../common/cpu.h"
void rol8_reg(CPU* cpu, U32 reg, U32 var2);
void rol8_mem(CPU* cpu, U32 eaa, U32 var2);
void rol8cl_reg(CPU* cpu, U32 reg, U32 var2);
void rol8cl_mem(CPU* cpu, U32 eaa, U32 var2);
void rol16_reg(CPU* cpu, U32 reg, U32 var2);
void rol16_mem(CPU* cpu, U32 eaa, U32 var2);
void rol16cl_reg(CPU* cpu, U32 reg, U32 var2);
void rol16cl_mem(CPU* cpu, U32 eaa, U32 var2);
void rol32_reg(CPU* cpu, U32 reg, U32 var2);
void rol32_mem(CPU* cpu, U32 eaa, U32 var2);
void rol32cl_reg(CPU* cpu, U32 reg, U32 var2);
void rol32cl_mem(CPU* cpu, U32 eaa, U32 var2);
void ror8_reg(CPU* cpu, U32 reg, U32 var2);
void ror8_mem(CPU* cpu, U32 eaa, U32 var2);
void ror8cl_reg(CPU* cpu, U32 reg, U32 var2);
void ror8cl_mem(CPU* cpu, U32 eaa, U32 var2);
void ror16_reg(CPU* cpu, U32 reg, U32 var2);
void ror16_mem(CPU* cpu, U32 eaa, U32 var2);
void ror16cl_reg(CPU* cpu, U32 reg, U32 var2);
void ror16cl_mem(CPU* cpu, U32 eaa, U32 var2);
void ror32_reg(CPU* cpu, U32 reg, U32 var2);
void ror32_mem(CPU* cpu, U32 eaa, U32 var2);
void ror32cl_reg(CPU* cpu, U32 reg, U32 var2);
void ror32cl_mem(CPU* cpu, U32 eaa, U32 var2);
void rcl8_reg(CPU* cpu, U32 reg, U32 var2);
void rcl8_mem(CPU* cpu, U32 eaa, U32 var2);
void rcl8cl_reg(CPU* cpu, U32 reg, U32 var2);
void rcl8cl_mem(CPU* cpu, U32 eaa, U32 var2);
void rcl16_reg(CPU* cpu, U32 reg, U32 var2);
void rcl16_mem(CPU* cpu, U32 eaa, U32 var2);
void rcl16cl_reg(CPU* cpu, U32 reg, U32 var2);
void rcl16cl_mem(CPU* cpu, U32 eaa, U32 var2);
void rcl32_reg(CPU* cpu, U32 reg, U32 var2);
void rcl32_mem(CPU* cpu, U32 eaa, U32 var2);
void rcl32cl_reg(CPU* cpu, U32 reg, U32 var2);
void rcl32cl_mem(CPU* cpu, U32 eaa, U32 var2);
void rcr8_reg(CPU* cpu, U32 reg, U32 var2);
void rcr8_mem(CPU* cpu, U32 eaa, U32 var2);
void rcr8cl_reg(CPU* cpu, U32 reg, U32 var2);
void rcr8cl_mem(CPU* cpu, U32 eaa, U32 var2);
void rcr16_reg(CPU* cpu, U32 reg, U32 var2);
void rcr16_mem(CPU* cpu, U32 eaa, U32 var2);
void rcr16cl_reg(CPU* cpu, U32 reg, U32 var2);
void rcr16cl_mem(CPU* cpu, U32 eaa, U32 var2);
void rcr32_reg(CPU* cpu, U32 reg, U32 var2);
void rcr32_mem(CPU* cpu, U32 eaa, U32 var2);
void rcr32cl_reg(CPU* cpu, U32 reg, U32 var2);
void rcr32cl_mem(CPU* cpu, U32 eaa, U32 var2);
void shl8_reg(CPU* cpu, U32 reg, U32 var2);
void shl8_mem(CPU* cpu, U32 eaa, U32 var2);
void shl8cl_reg(CPU* cpu, U32 reg, U32 var2);
void shl8cl_mem(CPU* cpu, U32 eaa, U32 var2);
void shl16_reg(CPU* cpu, U32 reg, U32 var2);
void shl16_mem(CPU* cpu, U32 eaa, U32 var2);
void shl16cl_reg(CPU* cpu, U32 reg, U32 var2);
void shl16cl_mem(CPU* cpu, U32 eaa, U32 var2);
void shl32_reg(CPU* cpu, U32 reg, U32 var2);
void shl32_mem(CPU* cpu, U32 eaa, U32 var2);
void shl32cl_reg(CPU* cpu, U32 reg, U32 var2);
void shl32cl_mem(CPU* cpu, U32 eaa, U32 var2);
void shr8_reg(CPU* cpu, U32 reg, U32 var2);
void shr8_mem(CPU* cpu, U32 eaa, U32 var2);
void shr8cl_reg(CPU* cpu, U32 reg, U32 var2);
void shr8cl_mem(CPU* cpu, U32 eaa, U32 var2);
void shr16_reg(CPU* cpu, U32 reg, U32 var2);
void shr16_mem(CPU* cpu, U32 eaa, U32 var2);
void shr16cl_reg(CPU* cpu, U32 reg, U32 var2);
void shr16cl_mem(CPU* cpu, U32 eaa, U32 var2);
void shr32_reg(CPU* cpu, U32 reg, U32 var2);
void shr32_mem(CPU* cpu, U32 eaa, U32 var2);
void shr32cl_reg(CPU* cpu, U32 reg, U32 var2);
void shr32cl_mem(CPU* cpu, U32 eaa, U32 var2);
void sar8_reg(CPU* cpu, U32 reg, U32 var2);
void sar8_mem(CPU* cpu, U32 eaa, U32 var2);
void sar8cl_reg(CPU* cpu, U32 reg, U32 var2);
void sar8cl_mem(CPU* cpu, U32 eaa, U32 var2);
void sar16_reg(CPU* cpu, U32 reg, U32 var2);
void sar16_mem(CPU* cpu, U32 eaa, U32 var2);
void sar16cl_reg(CPU* cpu, U32 reg, U32 var2);
void sar16cl_mem(CPU* cpu, U32 eaa, U32 var2);
void sar32_reg(CPU* cpu, U32 reg, U32 var2);
void sar32_mem(CPU* cpu, U32 eaa, U32 var2);
void sar32cl_reg(CPU* cpu, U32 reg, U32 var2);
void sar32cl_mem(CPU* cpu, U32 eaa, U32 var2);
#endif
