#include "../common/common_bit.h"
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

void OPCALL normal_btr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btr16r16(cpu, op->rm, op->reg);
    NEXT();
}
void OPCALL normal_btr16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btr16(cpu, op->imm, op->reg);
    NEXT();
}
void OPCALL normal_bte16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_bte16r16(cpu, op, op->reg);
    NEXT();
}
void OPCALL normal_bte16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_bte16(cpu, op->imm, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_btr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btr32r32(cpu, op->rm, op->reg);
    NEXT();
}
void OPCALL normal_btr32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btr32(cpu, op->imm, op->reg);
    NEXT();
}
void OPCALL normal_bte32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_bte32r32(cpu, op, op->reg);
    NEXT();
}
void OPCALL normal_bte32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_bte32(cpu, op->imm, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_btsr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btsr16r16(cpu, op->rm, op->reg);
    NEXT();
}
void OPCALL normal_btsr16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btsr16(cpu, op->imm, op->reg);
    NEXT();
}
void OPCALL normal_btse16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btse16r16(cpu, op, op->reg);
    NEXT();
}
void OPCALL normal_btse16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btse16(cpu, op->imm, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_btsr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btsr32r32(cpu, op->rm, op->reg);
    NEXT();
}
void OPCALL normal_btsr32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btsr32(cpu, op->imm, op->reg);
    NEXT();
}
void OPCALL normal_btse32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btse32r32(cpu, op, op->reg);
    NEXT();
}
void OPCALL normal_btse32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btse32(cpu, op->imm, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_btrr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btrr16r16(cpu, op->rm, op->reg);
    NEXT();
}
void OPCALL normal_btrr16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btrr16(cpu, op->imm, op->reg);
    NEXT();
}
void OPCALL normal_btre16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btre16r16(cpu, op, op->reg);
    NEXT();
}
void OPCALL normal_btre16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btre16(cpu, op->imm, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_btrr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btrr32r32(cpu, op->rm, op->reg);
    NEXT();
}
void OPCALL normal_btrr32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btrr32(cpu, op->imm, op->reg);
    NEXT();
}
void OPCALL normal_btre32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btre32r32(cpu, op, op->reg);
    NEXT();
}
void OPCALL normal_btre32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btre32(cpu, op->imm, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_btcr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btcr16r16(cpu, op->rm, op->reg);
    NEXT();
}
void OPCALL normal_btcr16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btcr16(cpu, op->imm, op->reg);
    NEXT();
}
void OPCALL normal_btce16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btce16r16(cpu, op, op->reg);
    NEXT();
}
void OPCALL normal_btce16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btce16(cpu, op->imm, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_btcr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btcr32r32(cpu, op->rm, op->reg);
    NEXT();
}
void OPCALL normal_btcr32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btcr32(cpu, op->imm, op->reg);
    NEXT();
}
void OPCALL normal_btce32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btce32r32(cpu, op, op->reg);
    NEXT();
}
void OPCALL normal_btce32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_btce32(cpu, op->imm, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_bsfr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_bsfr16r16(cpu, op->rm, op->reg);
    NEXT();
}
void OPCALL normal_bsfr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_bsfr16e16(cpu, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_bsfr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_bsfr32r32(cpu, op->rm, op->reg);
    NEXT();
}
void OPCALL normal_bsfr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_bsfr32e32(cpu, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_bsrr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_bsrr16r16(cpu, op->rm, op->reg);
    NEXT();
}
void OPCALL normal_bsrr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_bsrr16e16(cpu, eaa(cpu, op), op->reg);
    NEXT();
}
void OPCALL normal_bsrr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_bsrr32r32(cpu, op->rm, op->reg);
    NEXT();
}
void OPCALL normal_bsrr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_bsrr32e32(cpu, eaa(cpu, op), op->reg);
    NEXT();
}
