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

#include "boxedwine.h"
void common_cmpxchgr8r8(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->dst.u8 = AL;
    cpu->src.u8 = *cpu->reg8[dstReg];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    if (AL == cpu->src.u8) {
        *cpu->reg8[dstReg] = *cpu->reg8[srcReg];
    } else {
        AL = cpu->src.u8;
    }
}
void common_cmpxchge8r8(CPU* cpu, U32 address, U32 srcReg){
    cpu->dst.u8 = AL;
    cpu->src.u8 = cpu->memory->readb(address);
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    if (AL == cpu->src.u8) {
        cpu->memory->writeb(address, *cpu->reg8[srcReg]);
    } else {
        AL = cpu->src.u8;
    }
}
void common_cmpxchgr16r16(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->dst.u16 = AX;
    cpu->src.u16 = cpu->reg[dstReg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    if (AX == cpu->src.u16) {
        cpu->reg[dstReg].u16 = cpu->reg[srcReg].u16;
    } else {
        AX = cpu->src.u16;
    }
}
void common_cmpxchge16r16(CPU* cpu, U32 address, U32 srcReg){
    cpu->dst.u16 = AX;
    cpu->src.u16 = cpu->memory->readw(address);
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    if (AX == cpu->src.u16) {
        cpu->memory->writew(address, cpu->reg[srcReg].u16);
    } else {
        AX = cpu->src.u16;
    }
}
void common_cmpxchgr32r32(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->dst.u32 = EAX;
    cpu->src.u32 = cpu->reg[dstReg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    if (EAX == cpu->src.u32) {
        cpu->reg[dstReg].u32 = cpu->reg[srcReg].u32;
    } else {
        EAX = cpu->src.u32;
    }
}
void common_cmpxchge32r32(CPU* cpu, U32 address, U32 srcReg){
    cpu->dst.u32 = EAX;
    cpu->src.u32 = cpu->memory->readd(address);
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    if (EAX == cpu->src.u32) {
        cpu->memory->writed(address, cpu->reg[srcReg].u32);
    } else {
        EAX = cpu->src.u32;
    }
}