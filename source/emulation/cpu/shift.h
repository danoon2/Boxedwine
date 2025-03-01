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

#ifndef __SHIFT_OP_H__
#define __SHIFT_OP_H__
#include "cpu.h"
void rol8_reg(struct CPU* cpu, U32 reg, U32 value);
void rol8_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rol8_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rol8cl_reg(struct CPU* cpu, U32 reg, U32 value);
void rol8cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rol8cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rol16_reg(struct CPU* cpu, U32 reg, U32 value);
void rol16_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rol16_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rol16cl_reg(struct CPU* cpu, U32 reg, U32 value);
void rol16cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rol16cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rol32_reg(struct CPU* cpu, U32 reg, U32 value);
void rol32_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rol32_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rol32cl_reg(struct CPU* cpu, U32 reg, U32 value);
void rol32cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rol32cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void ror8_reg(struct CPU* cpu, U32 reg, U32 value);
void ror8_mem16(struct CPU* cpu, U32 eaa, U32 value);
void ror8_mem32(struct CPU* cpu, U32 eaa, U32 value);
void ror8cl_reg(struct CPU* cpu, U32 reg, U32 value);
void ror8cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void ror8cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void ror16_reg(struct CPU* cpu, U32 reg, U32 value);
void ror16_mem16(struct CPU* cpu, U32 eaa, U32 value);
void ror16_mem32(struct CPU* cpu, U32 eaa, U32 value);
void ror16cl_reg(struct CPU* cpu, U32 reg, U32 value);
void ror16cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void ror16cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void ror32_reg(struct CPU* cpu, U32 reg, U32 value);
void ror32_mem16(struct CPU* cpu, U32 eaa, U32 value);
void ror32_mem32(struct CPU* cpu, U32 eaa, U32 value);
void ror32cl_reg(struct CPU* cpu, U32 reg, U32 value);
void ror32cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void ror32cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rcl8_reg(struct CPU* cpu, U32 reg, U32 value);
void rcl8_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rcl8_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rcl8cl_reg(struct CPU* cpu, U32 reg, U32 value);
void rcl8cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rcl8cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rcl16_reg(struct CPU* cpu, U32 reg, U32 value);
void rcl16_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rcl16_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rcl16cl_reg(struct CPU* cpu, U32 reg, U32 value);
void rcl16cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rcl16cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rcl32_reg(struct CPU* cpu, U32 reg, U32 value);
void rcl32_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rcl32_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rcl32cl_reg(struct CPU* cpu, U32 reg, U32 value);
void rcl32cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rcl32cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rcr8_reg(struct CPU* cpu, U32 reg, U32 value);
void rcr8_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rcr8_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rcr8cl_reg(struct CPU* cpu, U32 reg, U32 value);
void rcr8cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rcr8cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rcr16_reg(struct CPU* cpu, U32 reg, U32 value);
void rcr16_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rcr16_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rcr16cl_reg(struct CPU* cpu, U32 reg, U32 value);
void rcr16cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rcr16cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rcr32_reg(struct CPU* cpu, U32 reg, U32 value);
void rcr32_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rcr32_mem32(struct CPU* cpu, U32 eaa, U32 value);
void rcr32cl_reg(struct CPU* cpu, U32 reg, U32 value);
void rcr32cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void rcr32cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void shl8_reg(struct CPU* cpu, U32 reg, U32 value);
void shl8_mem16(struct CPU* cpu, U32 eaa, U32 value);
void shl8_mem32(struct CPU* cpu, U32 eaa, U32 value);
void shl8cl_reg(struct CPU* cpu, U32 reg, U32 value);
void shl8cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void shl8cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void shl16_reg(struct CPU* cpu, U32 reg, U32 value);
void shl16_mem16(struct CPU* cpu, U32 eaa, U32 value);
void shl16_mem32(struct CPU* cpu, U32 eaa, U32 value);
void shl16cl_reg(struct CPU* cpu, U32 reg, U32 value);
void shl16cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void shl16cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void shl32_reg(struct CPU* cpu, U32 reg, U32 value);
void shl32_mem16(struct CPU* cpu, U32 eaa, U32 value);
void shl32_mem32(struct CPU* cpu, U32 eaa, U32 value);
void shl32cl_reg(struct CPU* cpu, U32 reg, U32 value);
void shl32cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void shl32cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void shr8_reg(struct CPU* cpu, U32 reg, U32 value);
void shr8_mem16(struct CPU* cpu, U32 eaa, U32 value);
void shr8_mem32(struct CPU* cpu, U32 eaa, U32 value);
void shr8cl_reg(struct CPU* cpu, U32 reg, U32 value);
void shr8cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void shr8cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void shr16_reg(struct CPU* cpu, U32 reg, U32 value);
void shr16_mem16(struct CPU* cpu, U32 eaa, U32 value);
void shr16_mem32(struct CPU* cpu, U32 eaa, U32 value);
void shr16cl_reg(struct CPU* cpu, U32 reg, U32 value);
void shr16cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void shr16cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void shr32_reg(struct CPU* cpu, U32 reg, U32 value);
void shr32_mem16(struct CPU* cpu, U32 eaa, U32 value);
void shr32_mem32(struct CPU* cpu, U32 eaa, U32 value);
void shr32cl_reg(struct CPU* cpu, U32 reg, U32 value);
void shr32cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void shr32cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void sar8_reg(struct CPU* cpu, U32 reg, U32 value);
void sar8_mem16(struct CPU* cpu, U32 eaa, U32 value);
void sar8_mem32(struct CPU* cpu, U32 eaa, U32 value);
void sar8cl_reg(struct CPU* cpu, U32 reg, U32 value);
void sar8cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void sar8cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void sar16_reg(struct CPU* cpu, U32 reg, U32 value);
void sar16_mem16(struct CPU* cpu, U32 eaa, U32 value);
void sar16_mem32(struct CPU* cpu, U32 eaa, U32 value);
void sar16cl_reg(struct CPU* cpu, U32 reg, U32 value);
void sar16cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void sar16cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
void sar32_reg(struct CPU* cpu, U32 reg, U32 value);
void sar32_mem16(struct CPU* cpu, U32 eaa, U32 value);
void sar32_mem32(struct CPU* cpu, U32 eaa, U32 value);
void sar32cl_reg(struct CPU* cpu, U32 reg, U32 value);
void sar32cl_mem16(struct CPU* cpu, U32 eaa, U32 value);
void sar32cl_mem32(struct CPU* cpu, U32 eaa, U32 value);
#endif
