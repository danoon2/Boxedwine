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

#include "cpu.h"
#include "decoder.h"
void rol8_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1=*cpu->reg8[reg];
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (8 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 7));
    *cpu->reg8[reg] = result;
    CYCLES(1);
}
void rol8_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (8 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 7));
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
}
void rol8_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (8 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 7));
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
}
void rol8cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1;
    if (var2) {
    var2&=7;
    var1 = *cpu->reg8[reg];
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (8 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 7));
    *cpu->reg8[reg] = result;
    }
    CYCLES(4);
}
void rol8cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1;
    if (var2) {
    var2&=7;
    var1 = readb(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (8 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 7));
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void rol8cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1;
    if (var2) {
    var2&=7;
    var1 = readb(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (8 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 7));
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void rol16_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (16 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 15));
    cpu->reg[reg].u16 = result;
    CYCLES(1);
}
void rol16_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (16 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 15));
    writew(cpu->thread, eaa, result);
    CYCLES(3);
}
void rol16_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (16 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 15));
    writew(cpu->thread, eaa, result);
    CYCLES(3);
}
void rol16cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1;
    if (var2) {
    var2&=15;
    var1 = cpu->reg[reg].u16;
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (16 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 15));
    cpu->reg[reg].u16 = result;
    }
    CYCLES(4);
}
void rol16cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1;
    if (var2) {
    var2&=15;
    var1 = readw(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (16 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 15));
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void rol16cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1;
    if (var2) {
    var2&=15;
    var1 = readw(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (16 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 15));
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void rol32_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (32 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 31));
    cpu->reg[reg].u32 = result;
    CYCLES(1);
}
void rol32_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (32 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 31));
    writed(cpu->thread, eaa, result);
    CYCLES(3);
}
void rol32_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (32 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 31));
    writed(cpu->thread, eaa, result);
    CYCLES(3);
}
void rol32cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (32 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 31));
    cpu->reg[reg].u32 = result;
    }
    CYCLES(4);
}
void rol32cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (32 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 31));
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void rol32cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    fillFlagsNoCFOF(cpu);
    result = (var1 << var2) | (var1 >> (32 - var2));
    setCF(cpu, result & 1);
    setOF(cpu, (result & 1) ^ (result >> 31));
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void ror8_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1=*cpu->reg8[reg];
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (8 - var2));
    setCF(cpu, result & 0x80);
    setOF(cpu, (result ^ (result<<1)) & 0x80);
    *cpu->reg8[reg] = result;
    CYCLES(1);
}
void ror8_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (8 - var2));
    setCF(cpu, result & 0x80);
    setOF(cpu, (result ^ (result<<1)) & 0x80);
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
}
void ror8_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (8 - var2));
    setCF(cpu, result & 0x80);
    setOF(cpu, (result ^ (result<<1)) & 0x80);
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
}
void ror8cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1;
    if (var2) {
    var2&=7;
    var1 = *cpu->reg8[reg];
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (8 - var2));
    setCF(cpu, result & 0x80);
    setOF(cpu, (result ^ (result<<1)) & 0x80);
    *cpu->reg8[reg] = result;
    }
    CYCLES(4);
}
void ror8cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1;
    if (var2) {
    var2&=7;
    var1 = readb(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (8 - var2));
    setCF(cpu, result & 0x80);
    setOF(cpu, (result ^ (result<<1)) & 0x80);
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void ror8cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1;
    if (var2) {
    var2&=7;
    var1 = readb(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (8 - var2));
    setCF(cpu, result & 0x80);
    setOF(cpu, (result ^ (result<<1)) & 0x80);
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void ror16_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (16 - var2));
    setCF(cpu, result & 0x8000);
    setOF(cpu, (result ^ (result<<1)) & 0x8000);
    cpu->reg[reg].u16 = result;
    CYCLES(1);
}
void ror16_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (16 - var2));
    setCF(cpu, result & 0x8000);
    setOF(cpu, (result ^ (result<<1)) & 0x8000);
    writew(cpu->thread, eaa, result);
    CYCLES(3);
}
void ror16_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (16 - var2));
    setCF(cpu, result & 0x8000);
    setOF(cpu, (result ^ (result<<1)) & 0x8000);
    writew(cpu->thread, eaa, result);
    CYCLES(3);
}
void ror16cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1;
    if (var2) {
    var2&=15;
    var1 = cpu->reg[reg].u16;
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (16 - var2));
    setCF(cpu, result & 0x8000);
    setOF(cpu, (result ^ (result<<1)) & 0x8000);
    cpu->reg[reg].u16 = result;
    }
    CYCLES(4);
}
void ror16cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1;
    if (var2) {
    var2&=15;
    var1 = readw(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (16 - var2));
    setCF(cpu, result & 0x8000);
    setOF(cpu, (result ^ (result<<1)) & 0x8000);
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void ror16cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1;
    if (var2) {
    var2&=15;
    var1 = readw(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (16 - var2));
    setCF(cpu, result & 0x8000);
    setOF(cpu, (result ^ (result<<1)) & 0x8000);
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void ror32_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (32 - var2));
    setCF(cpu, result & 0x80000000);
    setOF(cpu, (result ^ (result<<1)) & 0x80000000);
    cpu->reg[reg].u32 = result;
    CYCLES(1);
}
void ror32_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (32 - var2));
    setCF(cpu, result & 0x80000000);
    setOF(cpu, (result ^ (result<<1)) & 0x80000000);
    writed(cpu->thread, eaa, result);
    CYCLES(3);
}
void ror32_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (32 - var2));
    setCF(cpu, result & 0x80000000);
    setOF(cpu, (result ^ (result<<1)) & 0x80000000);
    writed(cpu->thread, eaa, result);
    CYCLES(3);
}
void ror32cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (32 - var2));
    setCF(cpu, result & 0x80000000);
    setOF(cpu, (result ^ (result<<1)) & 0x80000000);
    cpu->reg[reg].u32 = result;
    }
    CYCLES(4);
}
void ror32cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (32 - var2));
    setCF(cpu, result & 0x80000000);
    setOF(cpu, (result ^ (result<<1)) & 0x80000000);
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void ror32cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    fillFlagsNoCFOF(cpu);
    result = (var1 >> var2) | (var1 << (32 - var2));
    setCF(cpu, result & 0x80000000);
    setOF(cpu, (result ^ (result<<1)) & 0x80000000);
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void rcl8_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1=*cpu->reg8[reg];
    fillFlagsNoOF(cpu);
    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (9-var2));
    setCF(cpu, ((var1 >> (8-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 7));
    *cpu->reg8[reg] = result;
    CYCLES(8);
}
void rcl8_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (9-var2));
    setCF(cpu, ((var1 >> (8-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 7));
    writeb(cpu->thread, eaa, result);
    CYCLES(10);
}
void rcl8_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (9-var2));
    setCF(cpu, ((var1 >> (8-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 7));
    writeb(cpu->thread, eaa, result);
    CYCLES(10);
}
void rcl8cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1;
    if (var2) {
    var2=var2 % 9;
    var1 = *cpu->reg8[reg];
    fillFlagsNoOF(cpu);
    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (9-var2));
    setCF(cpu, ((var1 >> (8-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 7));
    *cpu->reg8[reg] = result;
    }
    CYCLES(7);
}
void rcl8cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1;
    if (var2) {
    var2=var2 % 9;
    var1 = readb(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (9-var2));
    setCF(cpu, ((var1 >> (8-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 7));
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(9);
}
void rcl8cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1;
    if (var2) {
    var2=var2 % 9;
    var1 = readb(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (9-var2));
    setCF(cpu, ((var1 >> (8-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 7));
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(9);
}
void rcl16_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    fillFlagsNoOF(cpu);
    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (17-var2));
    setCF(cpu, ((var1 >> (16-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 15));
    cpu->reg[reg].u16 = result;
    CYCLES(8);
}
void rcl16_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (17-var2));
    setCF(cpu, ((var1 >> (16-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 15));
    writew(cpu->thread, eaa, result);
    CYCLES(10);
}
void rcl16_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (17-var2));
    setCF(cpu, ((var1 >> (16-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 15));
    writew(cpu->thread, eaa, result);
    CYCLES(10);
}
void rcl16cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1;
    if (var2) {
    var2=var2 % 17;
    var1 = cpu->reg[reg].u16;
    fillFlagsNoOF(cpu);
    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (17-var2));
    setCF(cpu, ((var1 >> (16-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 15));
    cpu->reg[reg].u16 = result;
    }
    CYCLES(7);
}
void rcl16cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1;
    if (var2) {
    var2=var2 % 17;
    var1 = readw(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (17-var2));
    setCF(cpu, ((var1 >> (16-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 15));
    writew(cpu->thread, eaa, result);
    }
    CYCLES(9);
}
void rcl16cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1;
    if (var2) {
    var2=var2 % 17;
    var1 = readw(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (17-var2));
    setCF(cpu, ((var1 >> (16-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 15));
    writew(cpu->thread, eaa, result);
    }
    CYCLES(9);
}
void rcl32_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    fillFlagsNoOF(cpu);
    if (var2==1) {
        result = (var1 << var2) | (cpu->flags & CF);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    setCF(cpu, ((var1 >> (32-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 31));
    cpu->reg[reg].u32 = result;
    CYCLES(8);
}
void rcl32_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    if (var2==1) {
        result = (var1 << var2) | (cpu->flags & CF);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    setCF(cpu, ((var1 >> (32-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 31));
    writed(cpu->thread, eaa, result);
    CYCLES(10);
}
void rcl32_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    if (var2==1) {
        result = (var1 << var2) | (cpu->flags & CF);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    setCF(cpu, ((var1 >> (32-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 31));
    writed(cpu->thread, eaa, result);
    CYCLES(10);
}
void rcl32cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    fillFlagsNoOF(cpu);
    if (var2==1) {
        result = (var1 << var2) | (cpu->flags & CF);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    setCF(cpu, ((var1 >> (32-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 31));
    cpu->reg[reg].u32 = result;
    }
    CYCLES(7);
}
void rcl32cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    fillFlagsNoOF(cpu);
    if (var2==1) {
        result = (var1 << var2) | (cpu->flags & CF);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    setCF(cpu, ((var1 >> (32-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 31));
    writed(cpu->thread, eaa, result);
    }
    CYCLES(9);
}
void rcl32cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    fillFlagsNoOF(cpu);
    if (var2==1) {
        result = (var1 << var2) | (cpu->flags & CF);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    setCF(cpu, ((var1 >> (32-var2)) & 1));
    setOF(cpu, (cpu->flags & CF) ^ (result >> 31));
    writed(cpu->thread, eaa, result);
    }
    CYCLES(9);
}
void rcr8_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1=*cpu->reg8[reg];
    fillFlagsNoOF(cpu);
    result = (var1 >> var2) | ((cpu->flags & CF) << (8-var2)) | (var1 << (9-var2));
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x80);
    *cpu->reg8[reg] = result;
    CYCLES(8);
}
void rcr8_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 >> var2) | ((cpu->flags & CF) << (8-var2)) | (var1 << (9-var2));
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x80);
    writeb(cpu->thread, eaa, result);
    CYCLES(10);
}
void rcr8_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 >> var2) | ((cpu->flags & CF) << (8-var2)) | (var1 << (9-var2));
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x80);
    writeb(cpu->thread, eaa, result);
    CYCLES(10);
}
void rcr8cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1;
    if (var2) {
    var2=var2 % 9;
    var1 = *cpu->reg8[reg];
    fillFlagsNoOF(cpu);
    result = (var1 >> var2) | ((cpu->flags & CF) << (8-var2)) | (var1 << (9-var2));
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x80);
    *cpu->reg8[reg] = result;
    }
    CYCLES(7);
}
void rcr8cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1;
    if (var2) {
    var2=var2 % 9;
    var1 = readb(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 >> var2) | ((cpu->flags & CF) << (8-var2)) | (var1 << (9-var2));
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x80);
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(9);
}
void rcr8cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1;
    if (var2) {
    var2=var2 % 9;
    var1 = readb(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 >> var2) | ((cpu->flags & CF) << (8-var2)) | (var1 << (9-var2));
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x80);
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(9);
}
void rcr16_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    fillFlagsNoOF(cpu);
    result = (var1 >> var2) | ((cpu->flags & CF) << (16-var2)) | (var1 << (17-var2));
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x8000);
    cpu->reg[reg].u16 = result;
    CYCLES(8);
}
void rcr16_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 >> var2) | ((cpu->flags & CF) << (16-var2)) | (var1 << (17-var2));
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x8000);
    writew(cpu->thread, eaa, result);
    CYCLES(10);
}
void rcr16_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 >> var2) | ((cpu->flags & CF) << (16-var2)) | (var1 << (17-var2));
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x8000);
    writew(cpu->thread, eaa, result);
    CYCLES(10);
}
void rcr16cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1;
    if (var2) {
    var2=var2 % 17;
    var1 = cpu->reg[reg].u16;
    fillFlagsNoOF(cpu);
    result = (var1 >> var2) | ((cpu->flags & CF) << (16-var2)) | (var1 << (17-var2));
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x8000);
    cpu->reg[reg].u16 = result;
    }
    CYCLES(7);
}
void rcr16cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1;
    if (var2) {
    var2=var2 % 17;
    var1 = readw(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 >> var2) | ((cpu->flags & CF) << (16-var2)) | (var1 << (17-var2));
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x8000);
    writew(cpu->thread, eaa, result);
    }
    CYCLES(9);
}
void rcr16cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1;
    if (var2) {
    var2=var2 % 17;
    var1 = readw(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    result = (var1 >> var2) | ((cpu->flags & CF) << (16-var2)) | (var1 << (17-var2));
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x8000);
    writew(cpu->thread, eaa, result);
    }
    CYCLES(9);
}
void rcr32_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    fillFlagsNoOF(cpu);
    if (var2==1) {
        result = (var1 >> var2) | ((cpu->flags & CF) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x80000000);
    cpu->reg[reg].u32 = result;
    CYCLES(8);
}
void rcr32_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    if (var2==1) {
        result = (var1 >> var2) | ((cpu->flags & CF) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x80000000);
    writed(cpu->thread, eaa, result);
    CYCLES(10);
}
void rcr32_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    fillFlagsNoOF(cpu);
    if (var2==1) {
        result = (var1 >> var2) | ((cpu->flags & CF) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x80000000);
    writed(cpu->thread, eaa, result);
    CYCLES(10);
}
void rcr32cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    fillFlagsNoOF(cpu);
    if (var2==1) {
        result = (var1 >> var2) | ((cpu->flags & CF) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x80000000);
    cpu->reg[reg].u32 = result;
    }
    CYCLES(7);
}
void rcr32cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    fillFlagsNoOF(cpu);
    if (var2==1) {
        result = (var1 >> var2) | ((cpu->flags & CF) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x80000000);
    writed(cpu->thread, eaa, result);
    }
    CYCLES(9);
}
void rcr32cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    fillFlagsNoOF(cpu);
    if (var2==1) {
        result = (var1 >> var2) | ((cpu->flags & CF) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    setCF(cpu, (var1 >> (var2 - 1)) & 1);
    setOF(cpu, (result ^ (result<<1)) & 0x80000000);
    writed(cpu->thread, eaa, result);
    }
    CYCLES(9);
}
void shl8_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1=*cpu->reg8[reg];
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    *cpu->reg8[reg] = result;
    CYCLES(1);
}
void shl8_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
}
void shl8_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
}
void shl8cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1=*cpu->reg8[reg];
    if (var2) {
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    *cpu->reg8[reg] = result;
    }
    CYCLES(4);
}
void shl8cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    if (var2) {
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void shl8cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    if (var2) {
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void shl16_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    cpu->reg[reg].u16 = result;
    CYCLES(1);
}
void shl16_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    writew(cpu->thread, eaa, result);
    CYCLES(3);
}
void shl16_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    writew(cpu->thread, eaa, result);
    CYCLES(3);
}
void shl16cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    if (var2) {
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    cpu->reg[reg].u16 = result;
    }
    CYCLES(4);
}
void shl16cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    if (var2) {
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void shl16cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    if (var2) {
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void shl32_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    cpu->reg[reg].u32 = result;
    CYCLES(1);
}
void shl32_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    writed(cpu->thread, eaa, result);
    CYCLES(3);
}
void shl32_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    writed(cpu->thread, eaa, result);
    CYCLES(3);
}
void shl32cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    cpu->reg[reg].u32 = result;
    }
    CYCLES(4);
}
void shl32cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void shl32cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void shr8_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1=*cpu->reg8[reg];
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    *cpu->reg8[reg] = result;
    CYCLES(1);
}
void shr8_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
}
void shr8_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
}
void shr8cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1=*cpu->reg8[reg];
    if (var2) {
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    *cpu->reg8[reg] = result;
    }
    CYCLES(4);
}
void shr8cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    if (var2) {
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void shr8cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    if (var2) {
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void shr16_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    cpu->reg[reg].u16 = result;
    CYCLES(1);
}
void shr16_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    writew(cpu->thread, eaa, result);
    CYCLES(3);
}
void shr16_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    writew(cpu->thread, eaa, result);
    CYCLES(3);
}
void shr16cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    if (var2) {
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    cpu->reg[reg].u16 = result;
    }
    CYCLES(4);
}
void shr16cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    if (var2) {
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void shr16cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    if (var2) {
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void shr32_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    cpu->reg[reg].u32 = result;
    CYCLES(1);
}
void shr32_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    writed(cpu->thread, eaa, result);
    CYCLES(3);
}
void shr32_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    writed(cpu->thread, eaa, result);
    CYCLES(3);
}
void shr32cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    cpu->reg[reg].u32 = result;
    }
    CYCLES(4);
}
void shr32cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void shr32cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void sar8_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1=*cpu->reg8[reg];
    result = (S8)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    *cpu->reg8[reg] = result;
    CYCLES(1);
}
void sar8_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = (S8)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
}
void sar8_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    result = (S8)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    writeb(cpu->thread, eaa, result);
    CYCLES(3);
}
void sar8cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U8 result;
    U8 var1=*cpu->reg8[reg];
    if (var2) {
    result = (S8)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    *cpu->reg8[reg] = result;
    }
    CYCLES(4);
}
void sar8cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    if (var2) {
    result = (S8)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void sar8cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U8 result;
    U8 var1=readb(cpu->thread, eaa);
    if (var2) {
    result = (S8)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    writeb(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void sar16_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    result = (S16)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    cpu->reg[reg].u16 = result;
    CYCLES(1);
}
void sar16_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = (S16)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    writew(cpu->thread, eaa, result);
    CYCLES(3);
}
void sar16_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    result = (S16)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    writew(cpu->thread, eaa, result);
    CYCLES(3);
}
void sar16cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U16 result;
    U16 var1=cpu->reg[reg].u16;
    if (var2) {
    result = (S16)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    cpu->reg[reg].u16 = result;
    }
    CYCLES(4);
}
void sar16cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    if (var2) {
    result = (S16)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void sar16cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U16 result;
    U16 var1=readw(cpu->thread, eaa);
    if (var2) {
    result = (S16)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    writew(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void sar32_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    result = (S32)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    cpu->reg[reg].u32 = result;
    CYCLES(1);
}
void sar32_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = (S32)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    writed(cpu->thread, eaa, result);
    CYCLES(3);
}
void sar32_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    result = (S32)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    writed(cpu->thread, eaa, result);
    CYCLES(3);
}
void sar32cl_reg(struct CPU* cpu, U32 reg, U32 var2) {
    U32 result;
    U32 var1=cpu->reg[reg].u32;
    if (var2) {
    result = (S32)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    cpu->reg[reg].u32 = result;
    }
    CYCLES(4);
}
void sar32cl_mem16(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = (S32)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
void sar32cl_mem32(struct CPU* cpu, U32 eaa, U32 var2) {
    U32 result;
    U32 var1=readd(cpu->thread, eaa);
    if (var2) {
    result = (S32)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    writed(cpu->thread, eaa, result);
    }
    CYCLES(4);
}
