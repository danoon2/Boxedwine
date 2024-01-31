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

#include "boxedwine.h"
void rol8_reg(CPU* cpu, U32 reg, U32 var2) {
    U8 var1=*cpu->reg8[reg];
    if (!(var2 & 7)) {
        if (var2 & 0x18) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 1);
            cpu->setOF((var1 & 1) ^ (var1 >> 7));
        }
        return;
    }
    var2 &= 7;
    cpu->fillFlagsNoCFOF();
    U8 result = (var1 << var2) | (var1 >> (8 - var2));
    cpu->setCF(result & 1);
    cpu->setOF((result & 1) ^ (result >> 7));
    *cpu->reg8[reg] = result;
}
void rol8_mem(CPU* cpu, U32 eaa, U32 var2) {
    U8 var1=cpu->memory->readb(eaa);
    if (!(var2 & 7)) {
        if (var2 & 0x18) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 1);
            cpu->setOF((var1 & 1) ^ (var1 >> 7));
        }
        return;
    }
    var2 &= 7;
    cpu->fillFlagsNoCFOF();
    U8 result = (var1 << var2) | (var1 >> (8 - var2));
    cpu->setCF(result & 1);
    cpu->setOF((result & 1) ^ (result >> 7));
    cpu->memory->writeb(eaa, result);
}
void rol8cl_reg(CPU* cpu, U32 reg, U32 var2) {
    U8 var1 = *cpu->reg8[reg];
    if (!(var2 & 7)) {
        if (var2 & 0x18) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 1);
            cpu->setOF((var1 & 1) ^ (var1 >> 7));
        }
        return;
    }
    var2 &= 7;    
    cpu->fillFlagsNoCFOF();
    U8 result = (var1 << var2) | (var1 >> (8 - var2));
    cpu->setCF(result & 1);
    cpu->setOF((result & 1) ^ (result >> 7));
    *cpu->reg8[reg] = result;
}
void rol8cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    U8 var1 = cpu->memory->readb(eaa);
    if (!(var2 & 7)) {
        if (var2 & 0x18) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 1);
            cpu->setOF((var1 & 1) ^ (var1 >> 7));
        }
        return;
    }
    var2 &= 7;    
    cpu->fillFlagsNoCFOF();
    U8 result = (var1 << var2) | (var1 >> (8 - var2));
    cpu->setCF(result & 1);
    cpu->setOF((result & 1) ^ (result >> 7));
    cpu->memory->writeb(eaa, result);
}
void rol16_reg(CPU* cpu, U32 reg, U32 var2) {
    U16 var1=cpu->reg[reg].u16;
    if (!(var2 & 0xf)) {
        if (var2 & 0x10) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 1);
            cpu->setOF((var1 & 1) ^ (var1 >> 15));
        }
        return;
    }
    var2 &= 0xf;
    cpu->fillFlagsNoCFOF();
    U16 result = (var1 << var2) | (var1 >> (16 - var2));
    cpu->setCF(result & 1);
    cpu->setOF((result & 1) ^ (result >> 15));
    cpu->reg[reg].u16 = result;
}
void rol16_mem(CPU* cpu, U32 eaa, U32 var2) {
    U16 var1=cpu->memory->readw(eaa);
    if (!(var2 & 0xf)) {
        if (var2 & 0x10) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 1);
            cpu->setOF((var1 & 1) ^ (var1 >> 15));
        }
        return;
    }
    var2 &= 0xf;
    cpu->fillFlagsNoCFOF();
    U16 result = (var1 << var2) | (var1 >> (16 - var2));
    cpu->setCF(result & 1);
    cpu->setOF((result & 1) ^ (result >> 15));
    cpu->memory->writew(eaa, result);
}
void rol16cl_reg(CPU* cpu, U32 reg, U32 var2) {
    U16 var1 = cpu->reg[reg].u16;
    if (!(var2 & 0xf)) {
        if (var2 & 0x10) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 1);
            cpu->setOF((var1 & 1) ^ (var1 >> 15));
        }
        return;
    }
    var2 &= 15;    
    cpu->fillFlagsNoCFOF();
    U16 result = (var1 << var2) | (var1 >> (16 - var2));
    cpu->setCF(result & 1);
    cpu->setOF((result & 1) ^ (result >> 15));
    cpu->reg[reg].u16 = result;
}
void rol16cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    U16 var1 = cpu->memory->readw(eaa);
    if (!(var2 & 0xf)) {
        if (var2 & 0x10) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 1);
            cpu->setOF((var1 & 1) ^ (var1 >> 15));
        }
        return;
    }
    var2 &= 15;    
    cpu->fillFlagsNoCFOF();
    U16 result = (var1 << var2) | (var1 >> (16 - var2));
    cpu->setCF(result & 1);
    cpu->setOF((result & 1) ^ (result >> 15));
    cpu->memory->writew(eaa, result);
}
void rol32_reg(CPU* cpu, U32 reg, U32 var2) {
    U32 var1=cpu->reg[reg].u32;
    cpu->fillFlagsNoCFOF();
    U32 result = (var1 << var2) | (var1 >> (32 - var2));
    cpu->setCF(result & 1);
    cpu->setOF((result & 1) ^ (result >> 31));
    cpu->reg[reg].u32 = result;
}
void rol32_mem(CPU* cpu, U32 eaa, U32 var2) {
    U32 var1=cpu->memory->readd(eaa);
    cpu->fillFlagsNoCFOF();
    U32 result = (var1 << var2) | (var1 >> (32 - var2));
    cpu->setCF(result & 1);
    cpu->setOF((result & 1) ^ (result >> 31));
    cpu->memory->writed(eaa, result);
}
void rol32cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U32 var1 = cpu->reg[reg].u32;
        cpu->fillFlagsNoCFOF();
        U32 result = (var1 << var2) | (var1 >> (32 - var2));
        cpu->setCF(result & 1);
        cpu->setOF((result & 1) ^ (result >> 31));
        cpu->reg[reg].u32 = result;
    }
}
void rol32cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        U32 var1 = cpu->memory->readd(eaa);
        cpu->fillFlagsNoCFOF();
        U32 result = (var1 << var2) | (var1 >> (32 - var2));
        cpu->setCF(result & 1);
        cpu->setOF((result & 1) ^ (result >> 31));
        cpu->memory->writed(eaa, result);
    }
}
void ror8_reg(CPU* cpu, U32 reg, U32 var2) {
    U8 var1=*cpu->reg8[reg];
    if (!(var2 & 7)) {
        if (var2 & 0x18) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 0x80);
            cpu->setOF((var1 ^ (var1 << 1)) & 0x80);
        }
        return;
    }
    var2 &= 7;
    cpu->fillFlagsNoCFOF();
    U8 result = (var1 >> var2) | (var1 << (8 - var2));
    cpu->setCF(result & 0x80);
    cpu->setOF((result ^ (result<<1)) & 0x80);
    *cpu->reg8[reg] = result;
}
void ror8_mem(CPU* cpu, U32 eaa, U32 var2) {
    U8 var1=cpu->memory->readb(eaa);
    if (!(var2 & 7)) {
        if (var2 & 0x18) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 0x80);
            cpu->setOF((var1 ^ (var1 << 1)) & 0x80);
        }
        return;
    }
    var2 &= 7;
    cpu->fillFlagsNoCFOF();
    U8 result = (var1 >> var2) | (var1 << (8 - var2));
    cpu->setCF(result & 0x80);
    cpu->setOF((result ^ (result<<1)) & 0x80);
    cpu->memory->writeb(eaa, result);
}
void ror8cl_reg(CPU* cpu, U32 reg, U32 var2) {
    U8 var1 = *cpu->reg8[reg];
    if (!(var2 & 7)) {
        if (var2 & 0x18) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 0x80);
            cpu->setOF((var1 ^ (var1 << 1)) & 0x80);
        }
        return;
    }
    var2 &= 7;    
    cpu->fillFlagsNoCFOF();
    U8 result = (var1 >> var2) | (var1 << (8 - var2));
    cpu->setCF(result & 0x80);
    cpu->setOF((result ^ (result<<1)) & 0x80);
    *cpu->reg8[reg] = result;
}
void ror8cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    U8 var1 = cpu->memory->readb(eaa);
    if (!(var2 & 7)) {
        if (var2 & 0x18) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 0x80);
            cpu->setOF((var1 ^ (var1 << 1)) & 0x80);
        }
        return;
    }
    var2 &= 7;    
    cpu->fillFlagsNoCFOF();
    U8 result = (var1 >> var2) | (var1 << (8 - var2));
    cpu->setCF(result & 0x80);
    cpu->setOF((result ^ (result<<1)) & 0x80);
    cpu->memory->writeb(eaa, result);
}
void ror16_reg(CPU* cpu, U32 reg, U32 var2) {
    U16 var1=cpu->reg[reg].u16;
    if (!(var2 & 0xf)) {
        if (var2 & 0x10) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 0x8000);
            cpu->setOF((var1 ^ (var1 << 1)) & 0x8000);
        }
        return;
    }
    var2 &= 0xf;
    cpu->fillFlagsNoCFOF();
    U16 result = (var1 >> var2) | (var1 << (16 - var2));
    cpu->setCF(result & 0x8000);
    cpu->setOF((result ^ (result<<1)) & 0x8000);
    cpu->reg[reg].u16 = result;
}
void ror16_mem(CPU* cpu, U32 eaa, U32 var2) {
    U16 var1=cpu->memory->readw(eaa);
    if (!(var2 & 0xf)) {
        if (var2 & 0x10) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 0x8000);
            cpu->setOF((var1 ^ (var1 << 1)) & 0x8000);
        }
        return;
    }
    var2 &= 0xf;
    cpu->fillFlagsNoCFOF();
    U16 result = (var1 >> var2) | (var1 << (16 - var2));
    cpu->setCF(result & 0x8000);
    cpu->setOF((result ^ (result<<1)) & 0x8000);
    cpu->memory->writew(eaa, result);
}
void ror16cl_reg(CPU* cpu, U32 reg, U32 var2) {
    U16 var1 = cpu->reg[reg].u16;
    if (!(var2 & 0xf)) {
        if (var2 & 0x10) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 0x8000);
            cpu->setOF((var1 ^ (var1 << 1)) & 0x8000);
        }
        return;
    }
    var2 &= 15;    
    cpu->fillFlagsNoCFOF();
    U16 result = (var1 >> var2) | (var1 << (16 - var2));
    cpu->setCF(result & 0x8000);
    cpu->setOF((result ^ (result<<1)) & 0x8000);
    cpu->reg[reg].u16 = result;
}
void ror16cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    U16 var1 = cpu->memory->readw(eaa);
    if (!(var2 & 0xf)) {
        if (var2 & 0x10) {
            cpu->fillFlagsNoCFOF();
            cpu->setCF(var1 & 0x8000);
            cpu->setOF((var1 ^ (var1 << 1)) & 0x8000);
        }
        return;
    }
    var2 &= 15;    
    cpu->fillFlagsNoCFOF();
    U32 result = (var1 >> var2) | (var1 << (16 - var2));
    cpu->setCF(result & 0x8000);
    cpu->setOF((result ^ (result<<1)) & 0x8000);
    cpu->memory->writew(eaa, result);
}
void ror32_reg(CPU* cpu, U32 reg, U32 var2) {
    U32 var1=cpu->reg[reg].u32;
    cpu->fillFlagsNoCFOF();
    U32 result = (var1 >> var2) | (var1 << (32 - var2));
    cpu->setCF(result & 0x80000000);
    cpu->setOF((result ^ (result<<1)) & 0x80000000);
    cpu->reg[reg].u32 = result;
}
void ror32_mem(CPU* cpu, U32 eaa, U32 var2) {
    U32 var1=cpu->memory->readd(eaa);
    cpu->fillFlagsNoCFOF();
    U32 result = (var1 >> var2) | (var1 << (32 - var2));
    cpu->setCF(result & 0x80000000);
    cpu->setOF((result ^ (result<<1)) & 0x80000000);
    cpu->memory->writed(eaa, result);
}
void ror32cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U32 var1 = cpu->reg[reg].u32;
        cpu->fillFlagsNoCFOF();
        U32 result = (var1 >> var2) | (var1 << (32 - var2));
        cpu->setCF(result & 0x80000000);
        cpu->setOF((result ^ (result<<1)) & 0x80000000);
        cpu->reg[reg].u32 = result;
    }
}
void ror32cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        U32 var1 = cpu->memory->readd(eaa);
        cpu->fillFlagsNoCFOF();
        U32 result = (var1 >> var2) | (var1 << (32 - var2));
        cpu->setCF(result & 0x80000000);
        cpu->setOF((result ^ (result<<1)) & 0x80000000);
        cpu->memory->writed(eaa, result);
    }
}
void rcl8_reg(CPU* cpu, U32 reg, U32 var2) {
    U8 var1=*cpu->reg8[reg];
    cpu->fillFlagsNoOF();
    U8 result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (9-var2));
    cpu->setCF(((var1 >> (8-var2)) & 1));
    cpu->setOF((cpu->flags & CF) ^ (result >> 7));
    *cpu->reg8[reg] = result;
}
void rcl8_mem(CPU* cpu, U32 eaa, U32 var2) {
    U8 var1=cpu->memory->readb(eaa);
    cpu->fillFlagsNoOF();
    U8 result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (9-var2));
    cpu->setCF(((var1 >> (8-var2)) & 1));
    cpu->setOF((cpu->flags & CF) ^ (result >> 7));
    cpu->memory->writeb(eaa, result);
}
void rcl8cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        var2=var2 % 9;
        U8 var1 = *cpu->reg8[reg];
        cpu->fillFlagsNoOF();
        U8 result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (9-var2));
        cpu->setCF(((var1 >> (8-var2)) & 1));
        cpu->setOF((cpu->flags & CF) ^ (result >> 7));
        *cpu->reg8[reg] = result;
    }
}
void rcl8cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        var2=var2 % 9;
        U8 var1 = cpu->memory->readb(eaa);
        cpu->fillFlagsNoOF();
        U8 result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (9-var2));
        cpu->setCF(((var1 >> (8-var2)) & 1));
        cpu->setOF((cpu->flags & CF) ^ (result >> 7));
        cpu->memory->writeb(eaa, result);
    }
}
void rcl16_reg(CPU* cpu, U32 reg, U32 var2) {
    U16 var1=cpu->reg[reg].u16;
    cpu->fillFlagsNoOF();
    U16 result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (17-var2));
    cpu->setCF(((var1 >> (16-var2)) & 1));
    cpu->setOF((cpu->flags & CF) ^ (result >> 15));
    cpu->reg[reg].u16 = result;
}
void rcl16_mem(CPU* cpu, U32 eaa, U32 var2) {
    U16 var1=cpu->memory->readw(eaa);
    cpu->fillFlagsNoOF();
    U16 result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (17-var2));
    cpu->setCF(((var1 >> (16-var2)) & 1));
    cpu->setOF((cpu->flags & CF) ^ (result >> 15));
    cpu->memory->writew(eaa, result);
}
void rcl16cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        var2=var2 % 17;
        U16 var1 = cpu->reg[reg].u16;
        cpu->fillFlagsNoOF();
        U16 result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (17-var2));
        cpu->setCF(((var1 >> (16-var2)) & 1));
        cpu->setOF((cpu->flags & CF) ^ (result >> 15));
        cpu->reg[reg].u16 = result;
    }
}
void rcl16cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        var2=var2 % 17;
        U16 var1 = cpu->memory->readw(eaa);
        cpu->fillFlagsNoOF();
        U16 result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (17-var2));
        cpu->setCF(((var1 >> (16-var2)) & 1));
        cpu->setOF((cpu->flags & CF) ^ (result >> 15));
        cpu->memory->writew(eaa, result);
    }
}
void rcl32_reg(CPU* cpu, U32 reg, U32 var2) {
    U32 result = 0;
    U32 var1=cpu->reg[reg].u32;
    cpu->fillFlagsNoOF();
    if (var2==1) {
        result = (var1 << var2) | (cpu->flags & CF);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    cpu->setCF(((var1 >> (32-var2)) & 1));
    cpu->setOF((cpu->flags & CF) ^ (result >> 31));
    cpu->reg[reg].u32 = result;
}
void rcl32_mem(CPU* cpu, U32 eaa, U32 var2) {
    U32 result = 0;
    U32 var1=cpu->memory->readd(eaa);
    cpu->fillFlagsNoOF();
    if (var2==1) {
        result = (var1 << var2) | (cpu->flags & CF);
    } else {
        result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
    }
    cpu->setCF(((var1 >> (32-var2)) & 1));
    cpu->setOF((cpu->flags & CF) ^ (result >> 31));
    cpu->memory->writed(eaa, result);
}
void rcl32cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U32 result = 0;
        U32 var1 = cpu->reg[reg].u32;
        cpu->fillFlagsNoOF();
        if (var2==1) {
            result = (var1 << var2) | (cpu->flags & CF);
        } else {
            result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
        }
        cpu->setCF(((var1 >> (32-var2)) & 1));
        cpu->setOF((cpu->flags & CF) ^ (result >> 31));
        cpu->reg[reg].u32 = result;
    }
}
void rcl32cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        U32 result = 0;
        U32 var1 = cpu->memory->readd(eaa);
        cpu->fillFlagsNoOF();
        if (var2==1) {
            result = (var1 << var2) | (cpu->flags & CF);
        } else {
            result = (var1 << var2) | ((cpu->flags & CF) << (var2-1)) | (var1 >> (33-var2));
        }
        cpu->setCF(((var1 >> (32-var2)) & 1));
        cpu->setOF((cpu->flags & CF) ^ (result >> 31));
        cpu->memory->writed(eaa, result);
    }
}
void rcr8_reg(CPU* cpu, U32 reg, U32 var2) {
    U8 var1=*cpu->reg8[reg];
    cpu->fillFlagsNoOF();
    U8 result = (var1 >> var2) | ((cpu->flags & CF) << (8-var2)) | (var1 << (9-var2));
    cpu->setCF((var1 >> (var2 - 1)) & 1);
    cpu->setOF((result ^ (result<<1)) & 0x80);
    *cpu->reg8[reg] = result;
}
void rcr8_mem(CPU* cpu, U32 eaa, U32 var2) {
    U8 var1=cpu->memory->readb(eaa);
    cpu->fillFlagsNoOF();
    U8 result = (var1 >> var2) | ((cpu->flags & CF) << (8-var2)) | (var1 << (9-var2));
    cpu->setCF((var1 >> (var2 - 1)) & 1);
    cpu->setOF((result ^ (result<<1)) & 0x80);
    cpu->memory->writeb(eaa, result);
}
void rcr8cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        var2=var2 % 9;
        U8 var1 = *cpu->reg8[reg];
        cpu->fillFlagsNoOF();
        U8 result = (var1 >> var2) | ((cpu->flags & CF) << (8-var2)) | (var1 << (9-var2));
        cpu->setCF((var1 >> (var2 - 1)) & 1);
        cpu->setOF((result ^ (result<<1)) & 0x80);
        *cpu->reg8[reg] = result;
    }
}
void rcr8cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        var2=var2 % 9;
        U8 var1 = cpu->memory->readb(eaa);
        cpu->fillFlagsNoOF();
        U8 result = (var1 >> var2) | ((cpu->flags & CF) << (8-var2)) | (var1 << (9-var2));
        cpu->setCF((var1 >> (var2 - 1)) & 1);
        cpu->setOF((result ^ (result<<1)) & 0x80);
        cpu->memory->writeb(eaa, result);
    }
}
void rcr16_reg(CPU* cpu, U32 reg, U32 var2) {
    U16 var1=cpu->reg[reg].u16;
    cpu->fillFlagsNoOF();
    U16 result = (var1 >> var2) | ((cpu->flags & CF) << (16-var2)) | (var1 << (17-var2));
    cpu->setCF((var1 >> (var2 - 1)) & 1);
    cpu->setOF((result ^ (result<<1)) & 0x8000);
    cpu->reg[reg].u16 = result;
}
void rcr16_mem(CPU* cpu, U32 eaa, U32 var2) {
    U16 var1=cpu->memory->readw(eaa);
    cpu->fillFlagsNoOF();
    U16 result = (var1 >> var2) | ((cpu->flags & CF) << (16-var2)) | (var1 << (17-var2));
    cpu->setCF((var1 >> (var2 - 1)) & 1);
    cpu->setOF((result ^ (result<<1)) & 0x8000);
    cpu->memory->writew(eaa, result);
}
void rcr16cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        var2=var2 % 17;
        U16 var1 = cpu->reg[reg].u16;
        cpu->fillFlagsNoOF();
        U16 result = (var1 >> var2) | ((cpu->flags & CF) << (16-var2)) | (var1 << (17-var2));
        cpu->setCF((var1 >> (var2 - 1)) & 1);
        cpu->setOF((result ^ (result<<1)) & 0x8000);
        cpu->reg[reg].u16 = result;
    }
}
void rcr16cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        var2=var2 % 17;
        U16 var1 = cpu->memory->readw(eaa);
        cpu->fillFlagsNoOF();
        U16 result = (var1 >> var2) | ((cpu->flags & CF) << (16-var2)) | (var1 << (17-var2));
        cpu->setCF((var1 >> (var2 - 1)) & 1);
        cpu->setOF((result ^ (result<<1)) & 0x8000);
        cpu->memory->writew(eaa, result);
    }
}
void rcr32_reg(CPU* cpu, U32 reg, U32 var2) {
    U32 result = 0;
    U32 var1=cpu->reg[reg].u32;
    cpu->fillFlagsNoOF();
    if (var2==1) {
        result = (var1 >> var2) | ((cpu->flags & CF) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    cpu->setCF((var1 >> (var2 - 1)) & 1);
    cpu->setOF((result ^ (result<<1)) & 0x80000000);
    cpu->reg[reg].u32 = result;
}
void rcr32_mem(CPU* cpu, U32 eaa, U32 var2) {
    U32 result = 0;
    U32 var1=cpu->memory->readd(eaa);
    cpu->fillFlagsNoOF();
    if (var2==1) {
        result = (var1 >> var2) | ((cpu->flags & CF) << 31);
    } else {
        result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
    }
    cpu->setCF((var1 >> (var2 - 1)) & 1);
    cpu->setOF((result ^ (result<<1)) & 0x80000000);
    cpu->memory->writed(eaa, result);
}
void rcr32cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U32 result = 0;
        U32 var1 = cpu->reg[reg].u32;        
        cpu->fillFlagsNoOF();
        if (var2==1) {
            result = (var1 >> var2) | ((cpu->flags & CF) << 31);
        } else {
            result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
        }
        cpu->setCF((var1 >> (var2 - 1)) & 1);
        cpu->setOF((result ^ (result<<1)) & 0x80000000);
        cpu->reg[reg].u32 = result;
    }
}
void rcr32cl_mem(CPU* cpu, U32 eaa, U32 var2) {    
    if (var2) {
        U32 result = 0;
        U32 var1 = cpu->memory->readd(eaa);
        cpu->fillFlagsNoOF();
        if (var2==1) {
            result = (var1 >> var2) | ((cpu->flags & CF) << 31);
        } else {
            result = (var1 >> var2) | ((cpu->flags & CF) << (32-var2)) | (var1 << (33-var2));
        }
        cpu->setCF((var1 >> (var2 - 1)) & 1);
        cpu->setOF((result ^ (result<<1)) & 0x80000000);
        cpu->memory->writed(eaa, result);
    }
}
void shl8_reg(CPU* cpu, U32 reg, U32 var2) {
    U8 var1=*cpu->reg8[reg];
    U8 result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    *cpu->reg8[reg] = result;
}
void shl8_mem(CPU* cpu, U32 eaa, U32 var2) {
    U8 var1=cpu->memory->readb(eaa);
    U8 result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    cpu->memory->writeb(eaa, result);
}
void shl8cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U8 var1 = *cpu->reg8[reg];
        U8 result = var1 << var2;
        cpu->lazyFlags = FLAGS_SHL8;
        cpu->result.u8 = result;
        cpu->src.u8=var2;
        cpu->dst.u8 = var1;
        *cpu->reg8[reg] = result;
    }
}
void shl8cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        U8 var1 = cpu->memory->readb(eaa);
        U8 result = var1 << var2;
        cpu->lazyFlags = FLAGS_SHL8;
        cpu->result.u8 = result;
        cpu->src.u8=var2;
        cpu->dst.u8 = var1;
        cpu->memory->writeb(eaa, result);
    }
}
void shl16_reg(CPU* cpu, U32 reg, U32 var2) {
    U16 var1=cpu->reg[reg].u16;
    U16 result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    cpu->reg[reg].u16 = result;
}
void shl16_mem(CPU* cpu, U32 eaa, U32 var2) {
    U16 var1=cpu->memory->readw(eaa);
    U16 result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    cpu->memory->writew(eaa, result);
}
void shl16cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U16 var1 = cpu->reg[reg].u16;
        U16 result = var1 << var2;
        cpu->lazyFlags = FLAGS_SHL16;
        cpu->result.u16 = result;
        cpu->src.u16=var2;
        cpu->dst.u16 = var1;
        cpu->reg[reg].u16 = result;
    }
}
void shl16cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        U16 var1 = cpu->memory->readw(eaa);
        U16 result = var1 << var2;
        cpu->lazyFlags = FLAGS_SHL16;
        cpu->result.u16 = result;
        cpu->src.u16=var2;
        cpu->dst.u16 = var1;
        cpu->memory->writew(eaa, result);
    }
}
void shl32_reg(CPU* cpu, U32 reg, U32 var2) {
    U32 var1=cpu->reg[reg].u32;
    U32 result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    cpu->reg[reg].u32 = result;
}
void shl32_mem(CPU* cpu, U32 eaa, U32 var2) {
    U32 var1=cpu->memory->readd(eaa);
    U32 result = var1 << var2;
    cpu->lazyFlags = FLAGS_SHL32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    cpu->memory->writed(eaa, result);
}
void shl32cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U32 var1 = cpu->reg[reg].u32;
        U32 result = var1 << var2;
        cpu->lazyFlags = FLAGS_SHL32;
        cpu->result.u32 = result;
        cpu->src.u32=var2;
        cpu->dst.u32 = var1;
        cpu->reg[reg].u32 = result;
    }
}
void shl32cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        U32 var1 = cpu->memory->readd(eaa);
        U32 result = var1 << var2;
        cpu->lazyFlags = FLAGS_SHL32;
        cpu->result.u32 = result;
        cpu->src.u32=var2;
        cpu->dst.u32 = var1;
        cpu->memory->writed(eaa, result);
    }
}
void shr8_reg(CPU* cpu, U32 reg, U32 var2) {
    U8 var1=*cpu->reg8[reg];
    U8 result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    *cpu->reg8[reg] = result;
}
void shr8_mem(CPU* cpu, U32 eaa, U32 var2) {
    U8 var1=cpu->memory->readb(eaa);
    U8 result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    cpu->memory->writeb(eaa, result);
}
void shr8cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U8 var1 = *cpu->reg8[reg];
        U8 result = var1 >> var2;
        cpu->lazyFlags = FLAGS_SHR8;
        cpu->result.u8 = result;
        cpu->src.u8=var2;
        cpu->dst.u8 = var1;
        *cpu->reg8[reg] = result;
    }
}
void shr8cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        U8 var1 = cpu->memory->readb(eaa);
        U8 result = var1 >> var2;
        cpu->lazyFlags = FLAGS_SHR8;
        cpu->result.u8 = result;
        cpu->src.u8=var2;
        cpu->dst.u8 = var1;
        cpu->memory->writeb(eaa, result);
    }
}
void shr16_reg(CPU* cpu, U32 reg, U32 var2) {
    U16 var1=cpu->reg[reg].u16;
    U16 result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    cpu->reg[reg].u16 = result;
}
void shr16_mem(CPU* cpu, U32 eaa, U32 var2) {
    U16 var1=cpu->memory->readw(eaa);
    U16 result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    cpu->memory->writew(eaa, result);
}
void shr16cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U16 var1 = cpu->reg[reg].u16;
        U16 result = var1 >> var2;
        cpu->lazyFlags = FLAGS_SHR16;
        cpu->result.u16 = result;
        cpu->src.u16=var2;
        cpu->dst.u16 = var1;
        cpu->reg[reg].u16 = result;
    }
}
void shr16cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        U16 var1 = cpu->memory->readw(eaa);
        U16 result = var1 >> var2;
        cpu->lazyFlags = FLAGS_SHR16;
        cpu->result.u16 = result;
        cpu->src.u16=var2;
        cpu->dst.u16 = var1;
        cpu->memory->writew(eaa, result);
    }
}
void shr32_reg(CPU* cpu, U32 reg, U32 var2) {
    U32 var1=cpu->reg[reg].u32;
    U32 result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    cpu->reg[reg].u32 = result;
}
void shr32_mem(CPU* cpu, U32 eaa, U32 var2) {
    U32 var1=cpu->memory->readd(eaa);
    U32 result = var1 >> var2;
    cpu->lazyFlags = FLAGS_SHR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    cpu->memory->writed(eaa, result);
}
void shr32cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U32 var1 = cpu->reg[reg].u32;
        U32 result = var1 >> var2;
        cpu->lazyFlags = FLAGS_SHR32;
        cpu->result.u32 = result;
        cpu->src.u32=var2;
        cpu->dst.u32 = var1;
        cpu->reg[reg].u32 = result;
    }
}
void shr32cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        U32 var1 = cpu->memory->readd(eaa);
        U32 result = var1 >> var2;
        cpu->lazyFlags = FLAGS_SHR32;
        cpu->result.u32 = result;
        cpu->src.u32=var2;
        cpu->dst.u32 = var1;
        cpu->memory->writed(eaa, result);
    }
}
void sar8_reg(CPU* cpu, U32 reg, U32 var2) {
    U8 var1=*cpu->reg8[reg];
    U8 result = (S8)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    *cpu->reg8[reg] = result;
}
void sar8_mem(CPU* cpu, U32 eaa, U32 var2) {
    U8 var1=cpu->memory->readb(eaa);
    U8 result = (S8)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR8;
    cpu->result.u8 = result;
    cpu->src.u8=var2;
    cpu->dst.u8 = var1;
    cpu->memory->writeb(eaa, result);
}
void sar8cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U8 var1 = *cpu->reg8[reg];
        U8 result = (S8)var1 >> var2;
        cpu->lazyFlags = FLAGS_SAR8;
        cpu->result.u8 = result;
        cpu->src.u8=var2;
        cpu->dst.u8 = var1;
        *cpu->reg8[reg] = result;
    }
}
void sar8cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        U8 var1 = cpu->memory->readb(eaa);
        U8 result = (S8)var1 >> var2;
        cpu->lazyFlags = FLAGS_SAR8;
        cpu->result.u8 = result;
        cpu->src.u8=var2;
        cpu->dst.u8 = var1;
        cpu->memory->writeb(eaa, result);
    }
}
void sar16_reg(CPU* cpu, U32 reg, U32 var2) {
    U16 var1=cpu->reg[reg].u16;
    U16 result = (S16)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    cpu->reg[reg].u16 = result;
}
void sar16_mem(CPU* cpu, U32 eaa, U32 var2) {
    U16 var1=cpu->memory->readw(eaa);
    U16 result = (S16)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR16;
    cpu->result.u16 = result;
    cpu->src.u16=var2;
    cpu->dst.u16 = var1;
    cpu->memory->writew(eaa, result);
}
void sar16cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U16 var1 = cpu->reg[reg].u16;
        U16 result = (S16)var1 >> var2;
        cpu->lazyFlags = FLAGS_SAR16;
        cpu->result.u16 = result;
        cpu->src.u16=var2;
        cpu->dst.u16 = var1;
        cpu->reg[reg].u16 = result;
    }
}
void sar16cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        U16 var1 = cpu->memory->readw(eaa);
        U16 result = (S16)var1 >> var2;
        cpu->lazyFlags = FLAGS_SAR16;
        cpu->result.u16 = result;
        cpu->src.u16=var2;
        cpu->dst.u16 = var1;
        cpu->memory->writew(eaa, result);
    }
}
void sar32_reg(CPU* cpu, U32 reg, U32 var2) {
    U32 var1=cpu->reg[reg].u32;
    U32 result = (S32)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    cpu->reg[reg].u32 = result;
}
void sar32_mem(CPU* cpu, U32 eaa, U32 var2) {
    U32 var1=cpu->memory->readd(eaa);
    U32 result = (S32)var1 >> var2;
    cpu->lazyFlags = FLAGS_SAR32;
    cpu->result.u32 = result;
    cpu->src.u32=var2;
    cpu->dst.u32 = var1;
    cpu->memory->writed(eaa, result);
}
void sar32cl_reg(CPU* cpu, U32 reg, U32 var2) {
    if (var2) {
        U32 var1 = cpu->reg[reg].u32;
        U32 result = (S32)var1 >> var2;
        cpu->lazyFlags = FLAGS_SAR32;
        cpu->result.u32 = result;
        cpu->src.u32=var2;
        cpu->dst.u32 = var1;
        cpu->reg[reg].u32 = result;
    }
}
void sar32cl_mem(CPU* cpu, U32 eaa, U32 var2) {
    if (var2) {
        U32 var1 = cpu->memory->readd(eaa);
        U32 result = (S32)var1 >> var2;
        cpu->lazyFlags = FLAGS_SAR32;
        cpu->result.u32 = result;
        cpu->src.u32=var2;
        cpu->dst.u32 = var1;
        cpu->memory->writed(eaa, result);
    }
}
