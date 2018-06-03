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

void OPCALL btr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask=1 << (cpu->reg[op->rm].u16 & 15);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u16 & mask);
    NEXT();
}
void OPCALL btr16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask = (U16)op->imm;
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u16 & mask);
    NEXT();
}
void OPCALL bte16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask=1 << (cpu->reg[op->reg].u16 & 15);
    U32 address = eaa(cpu, op);
    U16 value;
    cpu->fillFlagsNoCF();
    address+=(((S16)cpu->reg[op->reg].u16)>>4)*2;
    value = readw(address);
    cpu->setCF(value & mask);
    NEXT();
}
void OPCALL bte16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask = (U16)op->imm;
    U32 address = eaa(cpu, op);
    U16 value;
    cpu->fillFlagsNoCF();
    value = readw(address);
    cpu->setCF(value & mask);
    NEXT();
}
void OPCALL btr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask=1 << (cpu->reg[op->rm].u32 & 31);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u32 & mask);
    NEXT();
}
void OPCALL btr32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask = op->imm;
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u32 & mask);
    NEXT();
}
void OPCALL bte32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask=1 << (cpu->reg[op->reg].u32 & 31);
    U32 address = eaa(cpu, op);
    U32 value;
    cpu->fillFlagsNoCF();
    address+=(((S32)cpu->reg[op->reg].u32)>>5)*4;
    value = readd(address);
    cpu->setCF(value & mask);
    NEXT();
}
void OPCALL bte32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask = op->imm;
    U32 address = eaa(cpu, op);
    U32 value;
    cpu->fillFlagsNoCF();
    value = readd(address);
    cpu->setCF(value & mask);
    NEXT();
}
void OPCALL btsr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask=1 << (cpu->reg[op->rm].u16 & 15);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u16 & mask);
    cpu->reg[op->reg].u16 |= mask;
    NEXT();
}
void OPCALL btsr16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask = (U16)op->imm;
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u16 & mask);
    cpu->reg[op->reg].u16 |= mask;
    NEXT();
}
void OPCALL btse16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask=1 << (cpu->reg[op->reg].u16 & 15);
    U32 address = eaa(cpu, op);
    U16 value;
    cpu->fillFlagsNoCF();
    address+=(((S16)cpu->reg[op->reg].u16)>>4)*2;
    value = readw(address);
    cpu->setCF(value & mask);
    writew(address, value | mask);
    NEXT();
}
void OPCALL btse16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask = (U16)op->imm;
    U32 address = eaa(cpu, op);
    U16 value;
    cpu->fillFlagsNoCF();
    value = readw(address);
    cpu->setCF(value & mask);
    writew(address, value | mask);
    NEXT();
}
void OPCALL btsr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask=1 << (cpu->reg[op->rm].u32 & 31);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u32 & mask);
    cpu->reg[op->reg].u32 |= mask;
    NEXT();
}
void OPCALL btsr32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask = op->imm;
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u32 & mask);
    cpu->reg[op->reg].u32 |= mask;
    NEXT();
}
void OPCALL btse32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask=1 << (cpu->reg[op->reg].u32 & 31);
    U32 address = eaa(cpu, op);
    U32 value;
    cpu->fillFlagsNoCF();
    address+=(((S32)cpu->reg[op->reg].u32)>>5)*4;
    value = readd(address);
    cpu->setCF(value & mask);
    writed(address, value | mask);
    NEXT();
}
void OPCALL btse32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask = op->imm;
    U32 address = eaa(cpu, op);
    U32 value;
    cpu->fillFlagsNoCF();
    value = readd(address);
    cpu->setCF(value & mask);
    writed(address, value | mask);
    NEXT();
}
void OPCALL btrr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask=1 << (cpu->reg[op->rm].u16 & 15);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u16 & mask);
    cpu->reg[op->reg].u16 &= ~mask;
    NEXT();
}
void OPCALL btrr16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask = (U16)op->imm;
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u16 & mask);
    cpu->reg[op->reg].u16 &= ~mask;
    NEXT();
}
void OPCALL btre16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask=1 << (cpu->reg[op->reg].u16 & 15);
    U32 address = eaa(cpu, op);
    U16 value;
    cpu->fillFlagsNoCF();
    address+=(((S16)cpu->reg[op->reg].u16)>>4)*2;
    value = readw(address);
    cpu->setCF(value & mask);
    writew(address, value & ~mask);
    NEXT();
}
void OPCALL btre16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask = (U16)op->imm;
    U32 address = eaa(cpu, op);
    U16 value;
    cpu->fillFlagsNoCF();
    value = readw(address);
    cpu->setCF(value & mask);
    writew(address, value & ~mask);
    NEXT();
}
void OPCALL btrr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask=1 << (cpu->reg[op->rm].u32 & 31);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u32 & mask);
    cpu->reg[op->reg].u32 &= ~mask;
    NEXT();
}
void OPCALL btrr32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask = op->imm;
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u32 & mask);
    cpu->reg[op->reg].u32 &= ~mask;
    NEXT();
}
void OPCALL btre32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask=1 << (cpu->reg[op->reg].u32 & 31);
    U32 address = eaa(cpu, op);
    U32 value;
    cpu->fillFlagsNoCF();
    address+=(((S32)cpu->reg[op->reg].u32)>>5)*4;
    value = readd(address);
    cpu->setCF(value & mask);
    writed(address, value & ~mask);
    NEXT();
}
void OPCALL btre32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask = op->imm;
    U32 address = eaa(cpu, op);
    U32 value;
    cpu->fillFlagsNoCF();
    value = readd(address);
    cpu->setCF(value & mask);
    writed(address, value & ~mask);
    NEXT();
}
void OPCALL btcr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask=1 << (cpu->reg[op->rm].u16 & 15);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u16 & mask);
    cpu->reg[op->reg].u16 ^= mask;
    NEXT();
}
void OPCALL btcr16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask = (U16)op->imm;
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u16 & mask);
    cpu->reg[op->reg].u16 ^= mask;
    NEXT();
}
void OPCALL btce16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask=1 << (cpu->reg[op->reg].u16 & 15);
    U32 address = eaa(cpu, op);
    U16 value;
    cpu->fillFlagsNoCF();
    address+=(((S16)cpu->reg[op->reg].u16)>>4)*2;
    value = readw(address);
    cpu->setCF(value & mask);
    writew(address, value ^ mask);
    NEXT();
}
void OPCALL btce16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 mask = (U16)op->imm;
    U32 address = eaa(cpu, op);
    U16 value;
    cpu->fillFlagsNoCF();
    value = readw(address);
    cpu->setCF(value & mask);
    writew(address, value ^ mask);
    NEXT();
}
void OPCALL btcr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask=1 << (cpu->reg[op->rm].u32 & 31);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u32 & mask);
    cpu->reg[op->reg].u32 ^= mask;
    NEXT();
}
void OPCALL btcr32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask = op->imm;
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[op->reg].u32 & mask);
    cpu->reg[op->reg].u32 ^= mask;
    NEXT();
}
void OPCALL btce32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask=1 << (cpu->reg[op->reg].u32 & 31);
    U32 address = eaa(cpu, op);
    U32 value;
    cpu->fillFlagsNoCF();
    address+=(((S32)cpu->reg[op->reg].u32)>>5)*4;
    value = readd(address);
    cpu->setCF(value & mask);
    writed(address, value ^ mask);
    NEXT();
}
void OPCALL btce32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 mask = op->imm;
    U32 address = eaa(cpu, op);
    U32 value;
    cpu->fillFlagsNoCF();
    value = readd(address);
    cpu->setCF(value & mask);
    writed(address, value ^ mask);
    NEXT();
}
void OPCALL bsfr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 value=cpu->reg[op->rm].u16;
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U16 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        cpu->removeZF();
        cpu->reg[op->reg].u16=result;
    }
    NEXT();
}
void OPCALL bsfr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 value=readw(eaa(cpu, op));
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U16 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        cpu->removeZF();
        cpu->reg[op->reg].u16=result;
    }
    NEXT();
}
void OPCALL bsfr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 value=cpu->reg[op->rm].u32;
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U32 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        cpu->removeZF();
        cpu->reg[op->reg].u32=result;
    }
    NEXT();
}
void OPCALL bsfr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 value=readd(eaa(cpu, op));
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U32 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        cpu->removeZF();
        cpu->reg[op->reg].u32=result;
    }
    NEXT();
}
void OPCALL bsrr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 value=cpu->reg[op->rm].u16;
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U16 result = 15;
        while ((value & 0x8000)==0) { result--; value<<=1; }
        cpu->removeZF();
        cpu->reg[op->reg].u16=result;
    }
    NEXT();
}
void OPCALL bsrr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 value=readw(eaa(cpu, op));
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U16 result = 15;
        while ((value & 0x8000)==0) { result--; value<<=1; }
        cpu->removeZF();
        cpu->reg[op->reg].u16=result;
    }
    NEXT();
}
void OPCALL bsrr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 value=cpu->reg[op->rm].u32;
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U32 result = 31;
        while ((value & 0x80000000)==0) { result--; value<<=1; }
        cpu->removeZF();
        cpu->reg[op->reg].u32=result;
    }
    NEXT();
}
void OPCALL bsrr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 value=readd(eaa(cpu, op));
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U32 result = 31;
        while ((value & 0x80000000)==0) { result--; value<<=1; }
        cpu->removeZF();
        cpu->reg[op->reg].u32=result;
    }
    NEXT();
}
