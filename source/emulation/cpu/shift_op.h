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

#include "shift.h"
void OPCALL rol8_reg_op(struct CPU* cpu, struct Op* op) {
    rol8_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL rol8_mem16_op(struct CPU* cpu, struct Op* op) {
    rol8_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL rol8_mem32_op(struct CPU* cpu, struct Op* op) {
    rol8_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL rol8cl_reg_op(struct CPU* cpu, struct Op* op) {
    rol8cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL rol8cl_mem16_op(struct CPU* cpu, struct Op* op) {
    rol8cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rol8cl_mem32_op(struct CPU* cpu, struct Op* op) {
    rol8cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rol16_reg_op(struct CPU* cpu, struct Op* op) {
    rol16_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL rol16_mem16_op(struct CPU* cpu, struct Op* op) {
    rol16_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL rol16_mem32_op(struct CPU* cpu, struct Op* op) {
    rol16_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL rol16cl_reg_op(struct CPU* cpu, struct Op* op) {
    rol16cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL rol16cl_mem16_op(struct CPU* cpu, struct Op* op) {
    rol16cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rol16cl_mem32_op(struct CPU* cpu, struct Op* op) {
    rol16cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rol32_reg_op(struct CPU* cpu, struct Op* op) {
    rol32_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL rol32_mem16_op(struct CPU* cpu, struct Op* op) {
    rol32_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL rol32_mem32_op(struct CPU* cpu, struct Op* op) {
    rol32_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL rol32cl_reg_op(struct CPU* cpu, struct Op* op) {
    rol32cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL rol32cl_mem16_op(struct CPU* cpu, struct Op* op) {
    rol32cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rol32cl_mem32_op(struct CPU* cpu, struct Op* op) {
    rol32cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL ror8_reg_op(struct CPU* cpu, struct Op* op) {
    ror8_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL ror8_mem16_op(struct CPU* cpu, struct Op* op) {
    ror8_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL ror8_mem32_op(struct CPU* cpu, struct Op* op) {
    ror8_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL ror8cl_reg_op(struct CPU* cpu, struct Op* op) {
    ror8cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL ror8cl_mem16_op(struct CPU* cpu, struct Op* op) {
    ror8cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL ror8cl_mem32_op(struct CPU* cpu, struct Op* op) {
    ror8cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL ror16_reg_op(struct CPU* cpu, struct Op* op) {
    ror16_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL ror16_mem16_op(struct CPU* cpu, struct Op* op) {
    ror16_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL ror16_mem32_op(struct CPU* cpu, struct Op* op) {
    ror16_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL ror16cl_reg_op(struct CPU* cpu, struct Op* op) {
    ror16cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL ror16cl_mem16_op(struct CPU* cpu, struct Op* op) {
    ror16cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL ror16cl_mem32_op(struct CPU* cpu, struct Op* op) {
    ror16cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL ror32_reg_op(struct CPU* cpu, struct Op* op) {
    ror32_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL ror32_mem16_op(struct CPU* cpu, struct Op* op) {
    ror32_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL ror32_mem32_op(struct CPU* cpu, struct Op* op) {
    ror32_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL ror32cl_reg_op(struct CPU* cpu, struct Op* op) {
    ror32cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL ror32cl_mem16_op(struct CPU* cpu, struct Op* op) {
    ror32cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL ror32cl_mem32_op(struct CPU* cpu, struct Op* op) {
    ror32cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcl8_reg_op(struct CPU* cpu, struct Op* op) {
    rcl8_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL rcl8_mem16_op(struct CPU* cpu, struct Op* op) {
    rcl8_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL rcl8_mem32_op(struct CPU* cpu, struct Op* op) {
    rcl8_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL rcl8cl_reg_op(struct CPU* cpu, struct Op* op) {
    rcl8cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL rcl8cl_mem16_op(struct CPU* cpu, struct Op* op) {
    rcl8cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcl8cl_mem32_op(struct CPU* cpu, struct Op* op) {
    rcl8cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcl16_reg_op(struct CPU* cpu, struct Op* op) {
    rcl16_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL rcl16_mem16_op(struct CPU* cpu, struct Op* op) {
    rcl16_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL rcl16_mem32_op(struct CPU* cpu, struct Op* op) {
    rcl16_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL rcl16cl_reg_op(struct CPU* cpu, struct Op* op) {
    rcl16cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL rcl16cl_mem16_op(struct CPU* cpu, struct Op* op) {
    rcl16cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcl16cl_mem32_op(struct CPU* cpu, struct Op* op) {
    rcl16cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcl32_reg_op(struct CPU* cpu, struct Op* op) {
    rcl32_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL rcl32_mem16_op(struct CPU* cpu, struct Op* op) {
    rcl32_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL rcl32_mem32_op(struct CPU* cpu, struct Op* op) {
    rcl32_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL rcl32cl_reg_op(struct CPU* cpu, struct Op* op) {
    rcl32cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL rcl32cl_mem16_op(struct CPU* cpu, struct Op* op) {
    rcl32cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcl32cl_mem32_op(struct CPU* cpu, struct Op* op) {
    rcl32cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcr8_reg_op(struct CPU* cpu, struct Op* op) {
    rcr8_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL rcr8_mem16_op(struct CPU* cpu, struct Op* op) {
    rcr8_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL rcr8_mem32_op(struct CPU* cpu, struct Op* op) {
    rcr8_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL rcr8cl_reg_op(struct CPU* cpu, struct Op* op) {
    rcr8cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL rcr8cl_mem16_op(struct CPU* cpu, struct Op* op) {
    rcr8cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcr8cl_mem32_op(struct CPU* cpu, struct Op* op) {
    rcr8cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcr16_reg_op(struct CPU* cpu, struct Op* op) {
    rcr16_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL rcr16_mem16_op(struct CPU* cpu, struct Op* op) {
    rcr16_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL rcr16_mem32_op(struct CPU* cpu, struct Op* op) {
    rcr16_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL rcr16cl_reg_op(struct CPU* cpu, struct Op* op) {
    rcr16cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL rcr16cl_mem16_op(struct CPU* cpu, struct Op* op) {
    rcr16cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcr16cl_mem32_op(struct CPU* cpu, struct Op* op) {
    rcr16cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcr32_reg_op(struct CPU* cpu, struct Op* op) {
    rcr32_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL rcr32_mem16_op(struct CPU* cpu, struct Op* op) {
    rcr32_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL rcr32_mem32_op(struct CPU* cpu, struct Op* op) {
    rcr32_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL rcr32cl_reg_op(struct CPU* cpu, struct Op* op) {
    rcr32cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL rcr32cl_mem16_op(struct CPU* cpu, struct Op* op) {
    rcr32cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL rcr32cl_mem32_op(struct CPU* cpu, struct Op* op) {
    rcr32cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shl8_reg_op(struct CPU* cpu, struct Op* op) {
    shl8_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL shl8_mem16_op(struct CPU* cpu, struct Op* op) {
    shl8_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL shl8_mem32_op(struct CPU* cpu, struct Op* op) {
    shl8_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL shl8cl_reg_op(struct CPU* cpu, struct Op* op) {
    shl8cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL shl8cl_mem16_op(struct CPU* cpu, struct Op* op) {
    shl8cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shl8cl_mem32_op(struct CPU* cpu, struct Op* op) {
    shl8cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shl16_reg_op(struct CPU* cpu, struct Op* op) {
    shl16_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL shl16_mem16_op(struct CPU* cpu, struct Op* op) {
    shl16_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL shl16_mem32_op(struct CPU* cpu, struct Op* op) {
    shl16_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL shl16cl_reg_op(struct CPU* cpu, struct Op* op) {
    shl16cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL shl16cl_mem16_op(struct CPU* cpu, struct Op* op) {
    shl16cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shl16cl_mem32_op(struct CPU* cpu, struct Op* op) {
    shl16cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shl32_reg_op(struct CPU* cpu, struct Op* op) {
    shl32_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL shl32_mem16_op(struct CPU* cpu, struct Op* op) {
    shl32_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL shl32_mem32_op(struct CPU* cpu, struct Op* op) {
    shl32_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL shl32cl_reg_op(struct CPU* cpu, struct Op* op) {
    shl32cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL shl32cl_mem16_op(struct CPU* cpu, struct Op* op) {
    shl32cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shl32cl_mem32_op(struct CPU* cpu, struct Op* op) {
    shl32cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shr8_reg_op(struct CPU* cpu, struct Op* op) {
    shr8_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL shr8_mem16_op(struct CPU* cpu, struct Op* op) {
    shr8_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL shr8_mem32_op(struct CPU* cpu, struct Op* op) {
    shr8_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL shr8cl_reg_op(struct CPU* cpu, struct Op* op) {
    shr8cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL shr8cl_mem16_op(struct CPU* cpu, struct Op* op) {
    shr8cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shr8cl_mem32_op(struct CPU* cpu, struct Op* op) {
    shr8cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shr16_reg_op(struct CPU* cpu, struct Op* op) {
    shr16_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL shr16_mem16_op(struct CPU* cpu, struct Op* op) {
    shr16_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL shr16_mem32_op(struct CPU* cpu, struct Op* op) {
    shr16_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL shr16cl_reg_op(struct CPU* cpu, struct Op* op) {
    shr16cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL shr16cl_mem16_op(struct CPU* cpu, struct Op* op) {
    shr16cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shr16cl_mem32_op(struct CPU* cpu, struct Op* op) {
    shr16cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shr32_reg_op(struct CPU* cpu, struct Op* op) {
    shr32_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL shr32_mem16_op(struct CPU* cpu, struct Op* op) {
    shr32_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL shr32_mem32_op(struct CPU* cpu, struct Op* op) {
    shr32_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL shr32cl_reg_op(struct CPU* cpu, struct Op* op) {
    shr32cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL shr32cl_mem16_op(struct CPU* cpu, struct Op* op) {
    shr32cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL shr32cl_mem32_op(struct CPU* cpu, struct Op* op) {
    shr32cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL sar8_reg_op(struct CPU* cpu, struct Op* op) {
    sar8_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL sar8_mem16_op(struct CPU* cpu, struct Op* op) {
    sar8_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL sar8_mem32_op(struct CPU* cpu, struct Op* op) {
    sar8_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL sar8cl_reg_op(struct CPU* cpu, struct Op* op) {
    sar8cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL sar8cl_mem16_op(struct CPU* cpu, struct Op* op) {
    sar8cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL sar8cl_mem32_op(struct CPU* cpu, struct Op* op) {
    sar8cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL sar16_reg_op(struct CPU* cpu, struct Op* op) {
    sar16_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL sar16_mem16_op(struct CPU* cpu, struct Op* op) {
    sar16_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL sar16_mem32_op(struct CPU* cpu, struct Op* op) {
    sar16_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL sar16cl_reg_op(struct CPU* cpu, struct Op* op) {
    sar16cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL sar16cl_mem16_op(struct CPU* cpu, struct Op* op) {
    sar16cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL sar16cl_mem32_op(struct CPU* cpu, struct Op* op) {
    sar16cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL sar32_reg_op(struct CPU* cpu, struct Op* op) {
    sar32_reg(cpu, op->r1, op->data1);
    NEXT();
}
void OPCALL sar32_mem16_op(struct CPU* cpu, struct Op* op) {
    sar32_mem16(cpu, eaa16(cpu, op), op->data1);
    NEXT();
}
void OPCALL sar32_mem32_op(struct CPU* cpu, struct Op* op) {
    sar32_mem32(cpu, eaa32(cpu, op), op->data1);
    NEXT();
}
void OPCALL sar32cl_reg_op(struct CPU* cpu, struct Op* op) {
    sar32cl_reg(cpu, op->r1, CL & 0x1f);
    NEXT();
}
void OPCALL sar32cl_mem16_op(struct CPU* cpu, struct Op* op) {
    sar32cl_mem16(cpu, eaa16(cpu, op), CL & 0x1f);
    NEXT();
}
void OPCALL sar32cl_mem32_op(struct CPU* cpu, struct Op* op) {
    sar32cl_mem32(cpu, eaa32(cpu, op), CL & 0x1f);
    NEXT();
}
