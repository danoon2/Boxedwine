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

#include "normal_shift.h"
void OPCALL rol8_reg_op(CPU* cpu, DecodedOp* op) {
    rol8_reg(cpu, op->reg, op->imm);
}
void OPCALL rol8_mem_op(CPU* cpu, DecodedOp* op) {
    rol8_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL rol8cl_reg_op(CPU* cpu, DecodedOp* op) {
    rol8cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL rol8cl_mem_op(CPU* cpu, DecodedOp* op) {
    rol8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL rol16_reg_op(CPU* cpu, DecodedOp* op) {
    rol16_reg(cpu, op->reg, op->imm);
}
void OPCALL rol16_mem_op(CPU* cpu, DecodedOp* op) {
    rol16_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL rol16cl_reg_op(CPU* cpu, DecodedOp* op) {
    rol16cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL rol16cl_mem_op(CPU* cpu, DecodedOp* op) {
    rol16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL rol32_reg_op(CPU* cpu, DecodedOp* op) {
    rol32_reg(cpu, op->reg, op->imm);
}
void OPCALL rol32_mem_op(CPU* cpu, DecodedOp* op) {
    rol32_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL rol32cl_reg_op(CPU* cpu, DecodedOp* op) {
    rol32cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL rol32cl_mem_op(CPU* cpu, DecodedOp* op) {
    rol32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL ror8_reg_op(CPU* cpu, DecodedOp* op) {
    ror8_reg(cpu, op->reg, op->imm);
}
void OPCALL ror8_mem_op(CPU* cpu, DecodedOp* op) {
    ror8_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL ror8cl_reg_op(CPU* cpu, DecodedOp* op) {
    ror8cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL ror8cl_mem_op(CPU* cpu, DecodedOp* op) {
    ror8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL ror16_reg_op(CPU* cpu, DecodedOp* op) {
    ror16_reg(cpu, op->reg, op->imm);
}
void OPCALL ror16_mem_op(CPU* cpu, DecodedOp* op) {
    ror16_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL ror16cl_reg_op(CPU* cpu, DecodedOp* op) {
    ror16cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL ror16cl_mem_op(CPU* cpu, DecodedOp* op) {
    ror16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL ror32_reg_op(CPU* cpu, DecodedOp* op) {
    ror32_reg(cpu, op->reg, op->imm);
}
void OPCALL ror32_mem_op(CPU* cpu, DecodedOp* op) {
    ror32_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL ror32cl_reg_op(CPU* cpu, DecodedOp* op) {
    ror32cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL ror32cl_mem_op(CPU* cpu, DecodedOp* op) {
    ror32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL rcl8_reg_op(CPU* cpu, DecodedOp* op) {
    rcl8_reg(cpu, op->reg, op->imm);
}
void OPCALL rcl8_mem_op(CPU* cpu, DecodedOp* op) {
    rcl8_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL rcl8cl_reg_op(CPU* cpu, DecodedOp* op) {
    rcl8cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL rcl8cl_mem_op(CPU* cpu, DecodedOp* op) {
    rcl8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL rcl16_reg_op(CPU* cpu, DecodedOp* op) {
    rcl16_reg(cpu, op->reg, op->imm);
}
void OPCALL rcl16_mem_op(CPU* cpu, DecodedOp* op) {
    rcl16_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL rcl16cl_reg_op(CPU* cpu, DecodedOp* op) {
    rcl16cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL rcl16cl_mem_op(CPU* cpu, DecodedOp* op) {
    rcl16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL rcl32_reg_op(CPU* cpu, DecodedOp* op) {
    rcl32_reg(cpu, op->reg, op->imm);
}
void OPCALL rcl32_mem_op(CPU* cpu, DecodedOp* op) {
    rcl32_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL rcl32cl_reg_op(CPU* cpu, DecodedOp* op) {
    rcl32cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL rcl32cl_mem_op(CPU* cpu, DecodedOp* op) {
    rcl32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL rcr8_reg_op(CPU* cpu, DecodedOp* op) {
    rcr8_reg(cpu, op->reg, op->imm);
}
void OPCALL rcr8_mem_op(CPU* cpu, DecodedOp* op) {
    rcr8_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL rcr8cl_reg_op(CPU* cpu, DecodedOp* op) {
    rcr8cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL rcr8cl_mem_op(CPU* cpu, DecodedOp* op) {
    rcr8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL rcr16_reg_op(CPU* cpu, DecodedOp* op) {
    rcr16_reg(cpu, op->reg, op->imm);
}
void OPCALL rcr16_mem_op(CPU* cpu, DecodedOp* op) {
    rcr16_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL rcr16cl_reg_op(CPU* cpu, DecodedOp* op) {
    rcr16cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL rcr16cl_mem_op(CPU* cpu, DecodedOp* op) {
    rcr16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL rcr32_reg_op(CPU* cpu, DecodedOp* op) {
    rcr32_reg(cpu, op->reg, op->imm);
}
void OPCALL rcr32_mem_op(CPU* cpu, DecodedOp* op) {
    rcr32_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL rcr32cl_reg_op(CPU* cpu, DecodedOp* op) {
    rcr32cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL rcr32cl_mem_op(CPU* cpu, DecodedOp* op) {
    rcr32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL shl8_reg_op(CPU* cpu, DecodedOp* op) {
    shl8_reg(cpu, op->reg, op->imm);
}
void OPCALL shl8_mem_op(CPU* cpu, DecodedOp* op) {
    shl8_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL shl8cl_reg_op(CPU* cpu, DecodedOp* op) {
    shl8cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL shl8cl_mem_op(CPU* cpu, DecodedOp* op) {
    shl8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL shl16_reg_op(CPU* cpu, DecodedOp* op) {
    shl16_reg(cpu, op->reg, op->imm);
}
void OPCALL shl16_mem_op(CPU* cpu, DecodedOp* op) {
    shl16_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL shl16cl_reg_op(CPU* cpu, DecodedOp* op) {
    shl16cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL shl16cl_mem_op(CPU* cpu, DecodedOp* op) {
    shl16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL shl32_reg_op(CPU* cpu, DecodedOp* op) {
    shl32_reg(cpu, op->reg, op->imm);
}
void OPCALL shl32_mem_op(CPU* cpu, DecodedOp* op) {
    shl32_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL shl32cl_reg_op(CPU* cpu, DecodedOp* op) {
    shl32cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL shl32cl_mem_op(CPU* cpu, DecodedOp* op) {
    shl32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL shr8_reg_op(CPU* cpu, DecodedOp* op) {
    shr8_reg(cpu, op->reg, op->imm);
}
void OPCALL shr8_mem_op(CPU* cpu, DecodedOp* op) {
    shr8_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL shr8cl_reg_op(CPU* cpu, DecodedOp* op) {
    shr8cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL shr8cl_mem_op(CPU* cpu, DecodedOp* op) {
    shr8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL shr16_reg_op(CPU* cpu, DecodedOp* op) {
    shr16_reg(cpu, op->reg, op->imm);
}
void OPCALL shr16_mem_op(CPU* cpu, DecodedOp* op) {
    shr16_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL shr16cl_reg_op(CPU* cpu, DecodedOp* op) {
    shr16cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL shr16cl_mem_op(CPU* cpu, DecodedOp* op) {
    shr16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL shr32_reg_op(CPU* cpu, DecodedOp* op) {
    shr32_reg(cpu, op->reg, op->imm);
}
void OPCALL shr32_mem_op(CPU* cpu, DecodedOp* op) {
    shr32_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL shr32cl_reg_op(CPU* cpu, DecodedOp* op) {
    shr32cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL shr32cl_mem_op(CPU* cpu, DecodedOp* op) {
    shr32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL sar8_reg_op(CPU* cpu, DecodedOp* op) {
    sar8_reg(cpu, op->reg, op->imm);
}
void OPCALL sar8_mem_op(CPU* cpu, DecodedOp* op) {
    sar8_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL sar8cl_reg_op(CPU* cpu, DecodedOp* op) {
    sar8cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL sar8cl_mem_op(CPU* cpu, DecodedOp* op) {
    sar8cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL sar16_reg_op(CPU* cpu, DecodedOp* op) {
    sar16_reg(cpu, op->reg, op->imm);
}
void OPCALL sar16_mem_op(CPU* cpu, DecodedOp* op) {
    sar16_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL sar16cl_reg_op(CPU* cpu, DecodedOp* op) {
    sar16cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL sar16cl_mem_op(CPU* cpu, DecodedOp* op) {
    sar16cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL sar32_reg_op(CPU* cpu, DecodedOp* op) {
    sar32_reg(cpu, op->reg, op->imm);
}
void OPCALL sar32_mem_op(CPU* cpu, DecodedOp* op) {
    sar32_mem(cpu, eaa(cpu, op), op->imm);
}
void OPCALL sar32cl_reg_op(CPU* cpu, DecodedOp* op) {
    sar32cl_reg(cpu, op->reg, CL & 0x1f);
}
void OPCALL sar32cl_mem_op(CPU* cpu, DecodedOp* op) {
    sar32cl_mem(cpu, eaa(cpu, op), CL & 0x1f);
}
void OPCALL normal_dshlr16r16(CPU* cpu, DecodedOp* op) {
    dshlr16r16(cpu, op->reg, op->rm, op->imm);
}
void OPCALL normal_dshle16r16(CPU* cpu, DecodedOp* op) {
    dshle16r16(cpu, op->reg, eaa(cpu, op), op->imm);
}
void OPCALL normal_dshlr32r32(CPU* cpu, DecodedOp* op) {
    dshlr32r32(cpu, op->reg, op->rm, op->imm);
}
void OPCALL normal_dshle32r32(CPU* cpu, DecodedOp* op) {
    dshle32r32(cpu, op->reg, eaa(cpu, op), op->imm);
}
void OPCALL normal_dshlclr16r16(CPU* cpu, DecodedOp* op) {
    dshlclr16r16(cpu, op->reg, op->rm);
}
void OPCALL normal_dshlcle16r16(CPU* cpu, DecodedOp* op) {
    dshlcle16r16(cpu, op->reg, eaa(cpu, op));
}
void OPCALL normal_dshlclr32r32(CPU* cpu, DecodedOp* op) {
    dshlclr32r32(cpu, op->reg, op->rm);
}
void OPCALL normal_dshlcle32r32(CPU* cpu, DecodedOp* op) {
    dshlcle32r32(cpu, op->reg, eaa(cpu, op));
}
void OPCALL normal_dshrr16r16(CPU* cpu, DecodedOp* op) {
    dshrr16r16(cpu, op->reg, op->rm, op->imm);
}
void OPCALL normal_dshre16r16(CPU* cpu, DecodedOp* op) {
    dshre16r16(cpu, op->reg, eaa(cpu, op), op->imm);
}
void OPCALL normal_dshrr32r32(CPU* cpu, DecodedOp* op) {
    dshrr32r32(cpu, op->reg, op->rm, op->imm);
}
void OPCALL normal_dshre32r32(CPU* cpu, DecodedOp* op) {
    dshre32r32(cpu, op->reg, eaa(cpu, op), op->imm);
}
void OPCALL normal_dshrclr16r16(CPU* cpu, DecodedOp* op) {
    dshrclr16r16(cpu, op->reg, op->rm);
}
void OPCALL normal_dshrcle16r16(CPU* cpu, DecodedOp* op) {
    dshrcle16r16(cpu, op->reg, eaa(cpu, op));
}
void OPCALL normal_dshrclr32r32(CPU* cpu, DecodedOp* op) {
    dshrclr32r32(cpu, op->reg, op->rm);
}
void OPCALL normal_dshrcle32r32(CPU* cpu, DecodedOp* op) {
    dshrcle32r32(cpu, op->reg, eaa(cpu, op));
}
