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

void common_btr16r16(CPU* cpu, U32 maskReg, U32 reg);
void common_btr16(CPU* cpu, U16 mask, U32 reg);
void common_bte16r16(CPU* cpu, DecodedOp* op, U32 reg);
void common_bte16(CPU* cpu, U16 mask, U32 address, U32 reg);
void common_btr32r32(CPU* cpu, U32 maskReg, U32 reg);
void common_btr32(CPU* cpu, U32 mask, U32 reg);
void common_bte32r32(CPU* cpu, DecodedOp* op, U32 reg);
void common_bte32(CPU* cpu, U32 mask, U32 address, U32 reg);
void common_btsr16r16(CPU* cpu, U32 maskReg, U32 reg);
void common_btsr16(CPU* cpu, U16 mask, U32 reg);
void common_btse16r16(CPU* cpu, DecodedOp* op, U32 reg);
void common_btse16(CPU* cpu, U16 mask, U32 address, U32 reg);
void common_btsr32r32(CPU* cpu, U32 maskReg, U32 reg);
void common_btsr32(CPU* cpu, U32 mask, U32 reg);
void common_btse32r32(CPU* cpu, DecodedOp* op, U32 reg);
void common_btse32(CPU* cpu, U32 mask, U32 address, U32 reg);
void common_btrr16r16(CPU* cpu, U32 maskReg, U32 reg);
void common_btrr16(CPU* cpu, U16 mask, U32 reg);
void common_btre16r16(CPU* cpu, DecodedOp* op, U32 reg);
void common_btre16(CPU* cpu, U16 mask, U32 address, U32 reg);
void common_btrr32r32(CPU* cpu, U32 maskReg, U32 reg);
void common_btrr32(CPU* cpu, U32 mask, U32 reg);
void common_btre32r32(CPU* cpu, DecodedOp* op, U32 reg);
void common_btre32(CPU* cpu, U32 mask, U32 address, U32 reg);
void common_btcr16r16(CPU* cpu, U32 maskReg, U32 reg);
void common_btcr16(CPU* cpu, U16 mask, U32 reg);
void common_btce16r16(CPU* cpu, DecodedOp* op, U32 reg);
void common_btce16(CPU* cpu, U16 mask, U32 address, U32 reg);
void common_btcr32r32(CPU* cpu, U32 maskReg, U32 reg);
void common_btcr32(CPU* cpu, U32 mask, U32 reg);
void common_btce32r32(CPU* cpu, DecodedOp* op, U32 reg);
void common_btce32(CPU* cpu, U32 mask, U32 address, U32 reg);
void common_bsfr16r16(CPU* cpu, U32 srcReg, U32 dstReg);
void common_bsfr16e16(CPU* cpu, U32 address, U32 dstReg);
void common_bsfr32r32(CPU* cpu, U32 srcReg, U32 dstReg);
void common_bsfr32e32(CPU* cpu, U32 address, U32 dstReg);
void common_bsrr16r16(CPU* cpu, U32 srcReg, U32 dstReg);
void common_bsrr16e16(CPU* cpu, U32 address, U32 dstReg);
void common_bsrr32r32(CPU* cpu, U32 srcReg, U32 dstReg);
void common_bsrr32e32(CPU* cpu, U32 address, U32 dstReg);
