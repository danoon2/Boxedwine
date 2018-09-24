#include "boxedwine.h"
void common_btr16r16(CPU* cpu, U32 maskReg, U32 reg) {
    U16 mask=1 << (cpu->reg[maskReg].u16 & 15);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u16 & mask);
}
void common_btr16(CPU* cpu, U16 mask, U32 reg) {
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u16 & mask);
}
void common_bte16r16(CPU* cpu, U32 address, U32 reg) {
    U16 mask=1 << (cpu->reg[reg].u16 & 15);
    U16 value;
    cpu->fillFlagsNoCF();
    address+=(((S16)cpu->reg[reg].u16)>>4)*2;
    value = readw(address);
    cpu->setCF(value & mask);
}
void common_bte16(CPU* cpu, U16 mask, U32 address, U32 reg) {
    U16 value;
    cpu->fillFlagsNoCF();
    value = readw(address);
    cpu->setCF(value & mask);
}
void common_btr32r32(CPU* cpu, U32 maskReg, U32 reg) {
    U32 mask=1 << (cpu->reg[maskReg].u32 & 31);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u32 & mask);
}
void common_btr32(CPU* cpu, U32 mask, U32 reg) {
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u32 & mask);
}
void common_bte32r32(CPU* cpu, U32 address, U32 reg) {
    U32 mask=1 << (cpu->reg[reg].u32 & 31);
    U32 value;
    cpu->fillFlagsNoCF();
    address+=(((S32)cpu->reg[reg].u32)>>5)*4;
    value = readd(address);
    cpu->setCF(value & mask);
}
void common_bte32(CPU* cpu, U32 mask, U32 address, U32 reg) {
    U32 value;
    cpu->fillFlagsNoCF();
    value = readd(address);
    cpu->setCF(value & mask);
}
void common_btsr16r16(CPU* cpu, U32 maskReg, U32 reg) {
    U16 mask=1 << (cpu->reg[maskReg].u16 & 15);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u16 & mask);
    cpu->reg[reg].u16 |= mask;
}
void common_btsr16(CPU* cpu, U16 mask, U32 reg) {
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u16 & mask);
    cpu->reg[reg].u16 |= mask;
}
void common_btse16r16(CPU* cpu, U32 address, U32 reg) {
    U16 mask=1 << (cpu->reg[reg].u16 & 15);
    U16 value;
    cpu->fillFlagsNoCF();
    address+=(((S16)cpu->reg[reg].u16)>>4)*2;
    value = readw(address);
    cpu->setCF(value & mask);
    writew(address, value | mask);
}
void common_btse16(CPU* cpu, U16 mask, U32 address, U32 reg) {
    U16 value;
    cpu->fillFlagsNoCF();
    value = readw(address);
    cpu->setCF(value & mask);
    writew(address, value | mask);
}
void common_btsr32r32(CPU* cpu, U32 maskReg, U32 reg) {
    U32 mask=1 << (cpu->reg[maskReg].u32 & 31);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u32 & mask);
    cpu->reg[reg].u32 |= mask;
}
void common_btsr32(CPU* cpu, U32 mask, U32 reg) {
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u32 & mask);
    cpu->reg[reg].u32 |= mask;
}
void common_btse32r32(CPU* cpu, U32 address, U32 reg) {
    U32 mask=1 << (cpu->reg[reg].u32 & 31);
    U32 value;
    cpu->fillFlagsNoCF();
    address+=(((S32)cpu->reg[reg].u32)>>5)*4;
    value = readd(address);
    cpu->setCF(value & mask);
    writed(address, value | mask);
}
void common_btse32(CPU* cpu, U32 mask, U32 address, U32 reg) {
    U32 value;
    cpu->fillFlagsNoCF();
    value = readd(address);
    cpu->setCF(value & mask);
    writed(address, value | mask);
}
void common_btrr16r16(CPU* cpu, U32 maskReg, U32 reg) {
    U16 mask=1 << (cpu->reg[maskReg].u16 & 15);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u16 & mask);
    cpu->reg[reg].u16 &= ~mask;
}
void common_btrr16(CPU* cpu, U16 mask, U32 reg) {
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u16 & mask);
    cpu->reg[reg].u16 &= ~mask;
}
void common_btre16r16(CPU* cpu, U32 address, U32 reg) {
    U16 mask=1 << (cpu->reg[reg].u16 & 15);
    U16 value;
    cpu->fillFlagsNoCF();
    address+=(((S16)cpu->reg[reg].u16)>>4)*2;
    value = readw(address);
    cpu->setCF(value & mask);
    writew(address, value & ~mask);
}
void common_btre16(CPU* cpu, U16 mask, U32 address, U32 reg) {
    U16 value;
    cpu->fillFlagsNoCF();
    value = readw(address);
    cpu->setCF(value & mask);
    writew(address, value & ~mask);
}
void common_btrr32r32(CPU* cpu, U32 maskReg, U32 reg) {
    U32 mask=1 << (cpu->reg[maskReg].u32 & 31);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u32 & mask);
    cpu->reg[reg].u32 &= ~mask;
}
void common_btrr32(CPU* cpu, U32 mask, U32 reg) {
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u32 & mask);
    cpu->reg[reg].u32 &= ~mask;
}
void common_btre32r32(CPU* cpu, U32 address, U32 reg) {
    U32 mask=1 << (cpu->reg[reg].u32 & 31);
    U32 value;
    cpu->fillFlagsNoCF();
    address+=(((S32)cpu->reg[reg].u32)>>5)*4;
    value = readd(address);
    cpu->setCF(value & mask);
    writed(address, value & ~mask);
}
void common_btre32(CPU* cpu, U32 mask, U32 address, U32 reg) {
    U32 value;
    cpu->fillFlagsNoCF();
    value = readd(address);
    cpu->setCF(value & mask);
    writed(address, value & ~mask);
}
void common_btcr16r16(CPU* cpu, U32 maskReg, U32 reg) {
    U16 mask=1 << (cpu->reg[maskReg].u16 & 15);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u16 & mask);
    cpu->reg[reg].u16 ^= mask;
}
void common_btcr16(CPU* cpu, U16 mask, U32 reg) {
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u16 & mask);
    cpu->reg[reg].u16 ^= mask;
}
void common_btce16r16(CPU* cpu, U32 address, U32 reg) {
    U16 mask=1 << (cpu->reg[reg].u16 & 15);
    U16 value;
    cpu->fillFlagsNoCF();
    address+=(((S16)cpu->reg[reg].u16)>>4)*2;
    value = readw(address);
    cpu->setCF(value & mask);
    writew(address, value ^ mask);
}
void common_btce16(CPU* cpu, U16 mask, U32 address, U32 reg) {
    U16 value;
    cpu->fillFlagsNoCF();
    value = readw(address);
    cpu->setCF(value & mask);
    writew(address, value ^ mask);
}
void common_btcr32r32(CPU* cpu, U32 maskReg, U32 reg) {
    U32 mask=1 << (cpu->reg[maskReg].u32 & 31);
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u32 & mask);
    cpu->reg[reg].u32 ^= mask;
}
void common_btcr32(CPU* cpu, U32 mask, U32 reg) {
    cpu->fillFlagsNoCF();
    cpu->setCF(cpu->reg[reg].u32 & mask);
    cpu->reg[reg].u32 ^= mask;
}
void common_btce32r32(CPU* cpu, U32 address, U32 reg) {
    U32 mask=1 << (cpu->reg[reg].u32 & 31);
    U32 value;
    cpu->fillFlagsNoCF();
    address+=(((S32)cpu->reg[reg].u32)>>5)*4;
    value = readd(address);
    cpu->setCF(value & mask);
    writed(address, value ^ mask);
}
void common_btce32(CPU* cpu, U32 mask, U32 address, U32 reg) {
    U32 value;
    cpu->fillFlagsNoCF();
    value = readd(address);
    cpu->setCF(value & mask);
    writed(address, value ^ mask);
}
void common_bsfr16r16(CPU* cpu, U32 srcReg, U32 dstReg) {
    U16 value=cpu->reg[srcReg].u16;
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U16 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        cpu->removeZF();
        cpu->reg[dstReg].u16=result;
    }
}
void common_bsfr16e16(CPU* cpu, U32 address, U32 dstReg) {
    U16 value=readw(address);
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U16 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        cpu->removeZF();
        cpu->reg[dstReg].u16=result;
    }
}
void common_bsfr32r32(CPU* cpu, U32 srcReg, U32 dstReg) {
    U32 value=cpu->reg[srcReg].u32;
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U32 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        cpu->removeZF();
        cpu->reg[dstReg].u32=result;
    }
}
void common_bsfr32e32(CPU* cpu, U32 address, U32 dstReg) {
    U32 value=readd(address);
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U32 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        cpu->removeZF();
        cpu->reg[dstReg].u32=result;
    }
}
void common_bsrr16r16(CPU* cpu, U32 srcReg, U32 dstReg) {
    U16 value=cpu->reg[srcReg].u16;
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U16 result = 15;
        while ((value & 0x8000)==0) { result--; value<<=1; }
        cpu->removeZF();
        cpu->reg[dstReg].u16=result;
    }
}
void common_bsrr16e16(CPU* cpu, U32 address, U32 dstReg) {
    U16 value=readw(address);
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U16 result = 15;
        while ((value & 0x8000)==0) { result--; value<<=1; }
        cpu->removeZF();
        cpu->reg[dstReg].u16=result;
    }
}
void common_bsrr32r32(CPU* cpu, U32 srcReg, U32 dstReg) {
    U32 value=cpu->reg[srcReg].u32;
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U32 result = 31;
        while ((value & 0x80000000)==0) { result--; value<<=1; }
        cpu->removeZF();
        cpu->reg[dstReg].u32=result;
    }
}
void common_bsrr32e32(CPU* cpu, U32 address, U32 dstReg) {
    U32 value=readd(address);
    cpu->fillFlagsNoZF();
    if (value==0) {
        cpu->addZF();
    } else {
        U32 result = 31;
        while ((value & 0x80000000)==0) { result--; value<<=1; }
        cpu->removeZF();
        cpu->reg[dstReg].u32=result;
    }
}
