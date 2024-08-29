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

// :TODO: read/write operations in a single op to the same address should not have to look up the physical RAM address twice
#include "arith.h"
#include "incdec.h"
#include "pushpop.h"
#include "strings_op.h"
#include "shift_op.h"
#include "instructions.h"

void OPCALL pushSeg16(struct CPU* cpu, struct Op* op) {
    push16(cpu, cpu->segValue[op->r1]);
    CYCLES(1);
    NEXT();
}

void OPCALL pushSeg32(struct CPU* cpu, struct Op* op) {
    push32(cpu, cpu->segValue[op->r1]);
    CYCLES(1);
    NEXT();
}

void OPCALL popSeg16(struct CPU* cpu, struct Op* op) {
    if (!cpu_setSegment(cpu, op->r1, peek16(cpu, 0)))
        return;
    ESP = (ESP & cpu->stackNotMask) | ((ESP + 2 ) & cpu->stackMask);
    CYCLES(3);
    NEXT();
}

void OPCALL popSeg32(struct CPU* cpu, struct Op* op) {
    if (!cpu_setSegment(cpu, op->r1, peek32(cpu, 0)))
        return;
    ESP = (ESP & cpu->stackNotMask) | ((ESP + 4 ) & cpu->stackMask);
    CYCLES(3);
    NEXT();
}

// OF is undefined
void OPCALL daa(struct CPU* cpu, struct Op* op) {
    instruction_daa(cpu);
    CYCLES(3);
    NEXT();
}

void OPCALL das(struct CPU* cpu, struct Op* op) {
    instruction_das(cpu);
    CYCLES(3);
    NEXT();
}

void OPCALL aaa(struct CPU* cpu, struct Op* op) {
    instruction_aaa(cpu);
    CYCLES(3);
    NEXT();
}

void OPCALL aas(struct CPU* cpu, struct Op* op) {
    instruction_aas(cpu);
    CYCLES(3);
    NEXT();
}

// not PF safe
void OPCALL pusha(struct CPU* cpu, struct Op* op) {
    U16 sp = SP;
    push16(cpu, AX);
    push16(cpu, CX);
    push16(cpu, DX);
    push16(cpu, BX);
    push16(cpu, sp);
    push16(cpu, BP);
    push16(cpu, SI);
    push16(cpu, DI);
    CYCLES(5);
    NEXT();
}

void OPCALL popa(struct CPU* cpu, struct Op* op) {
    DI = pop16(cpu);
    SI = pop16(cpu);
    BP = pop16(cpu);
    pop16(cpu);
    BX = pop16(cpu);
    DX = pop16(cpu);
    CX = pop16(cpu);
    AX = pop16(cpu);
    CYCLES(5);
    NEXT();
}

void OPCALL pushad(struct CPU* cpu, struct Op* op) {
    U32 esp = ESP;
    push32(cpu, EAX);
    push32(cpu, ECX);
    push32(cpu, EDX);
    push32(cpu, EBX);
    push32(cpu, esp);
    push32(cpu, EBP);
    push32(cpu, ESI);
    push32(cpu, EDI);
    CYCLES(5);
    NEXT();
}

void OPCALL popad(struct CPU* cpu, struct Op* op) {
    EDI = pop32(cpu);
    ESI = pop32(cpu);
    EBP = pop32(cpu);
    pop32(cpu);
    EBX = pop32(cpu);
    EDX = pop32(cpu);
    ECX = pop32(cpu);
    EAX = pop32(cpu);
    CYCLES(5);
    NEXT();
}

void OPCALL bound_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);

    CYCLES(8);
    if (cpu->reg[op->r1].u16<readw(cpu->thread, eaa) || cpu->reg[op->r1].u16>readw(cpu->thread, eaa+2)) {
        exception(cpu, EXCEPTION_BOUND);
        return;
    }    
    NEXT();
}

void OPCALL bound_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);

    CYCLES(8);
    if (cpu->reg[op->r1].u16<readw(cpu->thread, eaa) || cpu->reg[op->r1].u16>readw(cpu->thread, eaa+2)) {
        exception(cpu, EXCEPTION_BOUND);
        return;
    }    
    NEXT();
}

void OPCALL boundd_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);

    CYCLES(8);
    if (cpu->reg[op->r1].u32<readd(cpu->thread, eaa) || cpu->reg[op->r1].u32>readd(cpu->thread, eaa+4)) {
        exception(cpu, EXCEPTION_BOUND);
        return;
    }    
    NEXT();
}

void OPCALL boundd_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);

    CYCLES(8);
    if (cpu->reg[op->r1].u32<readd(cpu->thread, eaa) || cpu->reg[op->r1].u32>readd(cpu->thread, eaa+4)) {
        exception(cpu, EXCEPTION_BOUND);
        return;
    }    
    NEXT();
}

void OPCALL push16data(struct CPU* cpu, struct Op* op) {
    push16(cpu, op->data1);
    CYCLES(1);
    NEXT();
}

void OPCALL push32data(struct CPU* cpu, struct Op* op) {
    push32(cpu, op->data1);
    CYCLES(1);
    NEXT();
}

void OPCALL dimulcr16r16(struct CPU* cpu, struct Op* op) {
    S32 res=(S16)(cpu->reg[op->r2].u16) * (S32)op->data1;
    fillFlagsNoCFOF(cpu);
    if ((res >= -32767) && (res <= 32767)) {
        removeFlag(CF|OF);
    } else {
        addFlag(CF|OF);
    }
    cpu->reg[op->r1].u16 = res;
    CYCLES(10);
    NEXT();
}

void OPCALL dimulcr16e16_16(struct CPU* cpu, struct Op* op) {
    S32 res=(S16)(readw(cpu->thread, eaa16(cpu, op))) * (S32)op->data1;
    fillFlagsNoCFOF(cpu);
    if ((res >= -32767) && (res <= 32767)) {
        removeFlag(CF|OF);
    } else {
        addFlag(CF|OF);
    }
    cpu->reg[op->r1].u16 = res;
    CYCLES(10);
    NEXT();
}

void OPCALL dimulcr16e16_32(struct CPU* cpu, struct Op* op) {
    S32 res=(S16)(readw(cpu->thread, eaa32(cpu, op))) * (S32)op->data1;
    fillFlagsNoCFOF(cpu);
    if ((res >= -32767) && (res <= 32767)) {
        removeFlag(CF|OF);
    } else {
        addFlag(CF|OF);
    }
    cpu->reg[op->r1].u16 = res;
    CYCLES(10);
    NEXT();
}

void OPCALL dimulcr32r32(struct CPU* cpu, struct Op* op) {
    S64 res=(S32)(cpu->reg[op->r2].u32) * (S64)((S32)op->data1);
    fillFlagsNoCFOF(cpu);
    if (res>=-2147483647l && res<=2147483647l) {
        removeFlag(CF|OF);
    } else {
        addFlag(CF|OF);
    }
    cpu->reg[op->r1].u32 = (S32)res;
    CYCLES(10);
    NEXT();
}

void OPCALL dimulcr32e32_16(struct CPU* cpu, struct Op* op) {
    S64 res=(S32)(readd(cpu->thread, eaa16(cpu, op))) * (S64)((S32)op->data1);
    fillFlagsNoCFOF(cpu);
    if (res>=-2147483647l && res<=2147483647l) {
        removeFlag(CF|OF);
    } else {
        addFlag(CF|OF);
    }
    cpu->reg[op->r1].u32 = (S32)res;
    CYCLES(10);
    NEXT();
}

void OPCALL dimulcr32e32_32(struct CPU* cpu, struct Op* op) {
    S64 res=(S32)(readd(cpu->thread, eaa32(cpu, op))) * (S64)((S32)op->data1);
    fillFlagsNoCFOF(cpu);
    if (res>=-2147483647l && res<=2147483647l) {
        removeFlag(CF|OF);
    } else {
        addFlag(CF|OF);
    }
    cpu->reg[op->r1].u32 = (S32)res;
    CYCLES(10);
    NEXT();
}

void OPCALL dimulr16r16(struct CPU* cpu, struct Op* op) {
    S32 res=(S16)(cpu->reg[op->r2].u16) * (S32)cpu->reg[op->r1].u16;
    fillFlagsNoCFOF(cpu);
    if ((res >= -32767) && (res <= 32767)) {
        removeFlag(CF|OF);
    } else {
        addFlag(CF|OF);
    }
    cpu->reg[op->r1].u16 = res;
    CYCLES(10);
    NEXT();
}

void OPCALL dimulr16e16_16(struct CPU* cpu, struct Op* op) {
    S32 res=(S16)(readw(cpu->thread, eaa16(cpu, op))) * (S32)cpu->reg[op->r1].u16;
    fillFlagsNoCFOF(cpu);
    if ((res >= -32767) && (res <= 32767)) {
        removeFlag(CF|OF);
    } else {
        addFlag(CF|OF);
    }
    cpu->reg[op->r1].u16 = res;
    CYCLES(10);
    NEXT();
}

void OPCALL dimulr16e16_32(struct CPU* cpu, struct Op* op) {
    S32 res=(S16)(readw(cpu->thread, eaa32(cpu, op))) * (S32)cpu->reg[op->r1].u16;
    fillFlagsNoCFOF(cpu);
    if ((res >= -32767) && (res <= 32767)) {
        removeFlag(CF|OF);
    } else {
        addFlag(CF|OF);
    }
    cpu->reg[op->r1].u16 = res;
    CYCLES(10);
    NEXT();
}

void OPCALL dimulr32r32(struct CPU* cpu, struct Op* op) {
    S64 res=(S32)(cpu->reg[op->r2].u32) * (S64)((S32)cpu->reg[op->r1].u32);
    fillFlagsNoCFOF(cpu);
    if (res>=-2147483647l && res<=2147483647l) {
        removeFlag(CF|OF);
    } else {
        addFlag(CF|OF);
    }
    cpu->reg[op->r1].u32 = (S32)res;
    CYCLES(10);
    NEXT();
}

void OPCALL dimulr32e32_16(struct CPU* cpu, struct Op* op) {
    S64 res=(S32)(readd(cpu->thread, eaa16(cpu, op))) * (S64)((S32)cpu->reg[op->r1].u32);
    fillFlagsNoCFOF(cpu);
    if (res>=-2147483647l && res<=2147483647l) {
        removeFlag(CF|OF);
    } else {
        addFlag(CF|OF);
    }
    cpu->reg[op->r1].u32 = (S32)res;
    CYCLES(10);
    NEXT();
}

void OPCALL dimulr32e32_32(struct CPU* cpu, struct Op* op) {
    S64 res=(S32)(readd(cpu->thread, eaa32(cpu, op))) * (S64)((S32)cpu->reg[op->r1].u32);
    fillFlagsNoCFOF(cpu);
    if (res>=-2147483647l && res<=2147483647l) {
        removeFlag(CF|OF);
    } else {
        addFlag(CF|OF);
    }
    cpu->reg[op->r1].u32 = (S32)res;
    CYCLES(10);
    NEXT();
}

void OPCALL jumpO(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (getOF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
    }    
}

void OPCALL jumpNO(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (!getOF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
    }
}

void OPCALL jumpB(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (getCF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);	
        runBlock(cpu, cpu->nextBlock);
    }    
}

void OPCALL jumpNB(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (!getCF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);	
        runBlock(cpu, cpu->nextBlock);
    }    
}

void OPCALL jumpZ(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (getZF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);	
        runBlock(cpu, cpu->nextBlock);
    }    
}

void OPCALL jumpNZ(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (!getZF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);	
        runBlock(cpu, cpu->nextBlock);
    }
}

void OPCALL jumpBE(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (getZF(cpu) || getCF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);	
        runBlock(cpu, cpu->nextBlock);
    }
}

void OPCALL jumpNBE(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (!getZF(cpu) && !getCF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);	
        runBlock(cpu, cpu->nextBlock);
    }
}

void OPCALL jumpS(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (getSF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);	
        runBlock(cpu, cpu->nextBlock);
    }
}

void OPCALL jumpNS(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (!getSF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);	
        runBlock(cpu, cpu->nextBlock);
    }
}

void OPCALL jumpP(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (getPF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);	
        runBlock(cpu, cpu->nextBlock);
    }
}

void OPCALL jumpNP(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (!getPF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
    }	
}

void OPCALL jumpL(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (getSF(cpu)!=getOF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
    }	
}

void OPCALL jumpNL(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (getSF(cpu)==getOF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
    }	
}

void OPCALL jumpLE(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (getZF(cpu) || getSF(cpu)!=getOF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
    }	    
}

void OPCALL jumpNLE(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    CYCLES(1);
    if (!getZF(cpu) && getSF(cpu)==getOF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1>0)
            runBlock(cpu, cpu->nextBlock);
    } else {
        cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
    }	    
}

void OPCALL xchgr8r8(struct CPU* cpu, struct Op* op) {
    U8 tmp = *cpu->reg8[op->r1];
    *cpu->reg8[op->r1] = *cpu->reg8[op->r2];
    *cpu->reg8[op->r2] = tmp;
    CYCLES(3);
    NEXT();
}

void OPCALL xchge8r8_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    U8 tmp = readb(cpu->thread, eaa);
    writeb(cpu->thread, eaa, *cpu->reg8[op->r1]);
    *cpu->reg8[op->r1] = tmp;
    CYCLES(4);
    NEXT();
}

void OPCALL xchge8r8_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    U8 tmp = readb(cpu->thread, eaa);
    writeb(cpu->thread, eaa, *cpu->reg8[op->r1]);
    *cpu->reg8[op->r1] = tmp;
    CYCLES(4);
    NEXT();
}

void OPCALL xchgr16r16(struct CPU* cpu, struct Op* op) {
    U16 tmp = cpu->reg[op->r1].u16;
    cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    cpu->reg[op->r2].u16 = tmp;
    CYCLES(3);
    NEXT();
}

void OPCALL xchge16r16_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    U16 tmp = readw(cpu->thread, eaa);
    writew(cpu->thread, eaa, cpu->reg[op->r1].u16);
    cpu->reg[op->r1].u16 = tmp;
    CYCLES(4);
    NEXT();
}

void OPCALL xchge16r16_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    U16 tmp = readw(cpu->thread, eaa);
    writew(cpu->thread, eaa, cpu->reg[op->r1].u16);
    cpu->reg[op->r1].u16 = tmp;
    CYCLES(4);
    NEXT();
}

void OPCALL xchgr32r32(struct CPU* cpu, struct Op* op) {
    U32 tmp = cpu->reg[op->r1].u32;
    cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    cpu->reg[op->r2].u32 = tmp;
    CYCLES(3);
    NEXT();
}

void OPCALL xchge32r32_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    U32 tmp = readd(cpu->thread, eaa);
    writed(cpu->thread, eaa, cpu->reg[op->r1].u32);
    cpu->reg[op->r1].u32 = tmp;
    CYCLES(4);
    NEXT();
}

void OPCALL xchge32r32_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    U32 tmp = readd(cpu->thread, eaa);
    writed(cpu->thread, eaa, cpu->reg[op->r1].u32);
    cpu->reg[op->r1].u32 = tmp;
    CYCLES(4);
    NEXT();
}

void OPCALL movr8r8(struct CPU* cpu, struct Op* op) {
    *cpu->reg8[op->r1] = *cpu->reg8[op->r2];
    CYCLES(1);
    NEXT();
}

void OPCALL mov8_reg(struct CPU* cpu, struct Op* op) {
    *cpu->reg8[op->r1] = op->data1;
    CYCLES(1);
    NEXT();
}

void OPCALL mov8_mem16(struct CPU* cpu, struct Op* op) {
    writeb(cpu->thread, eaa16(cpu, op), op->data1);
    CYCLES(1);
    NEXT();
}

void OPCALL mov8_mem32(struct CPU* cpu, struct Op* op) {
    writeb(cpu->thread, eaa32(cpu, op), op->data1);
    CYCLES(1);
    NEXT();
}

void OPCALL movr8e8_16(struct CPU* cpu, struct Op* op) {
    *cpu->reg8[op->r1] = readb(cpu->thread, eaa16(cpu, op));
    CYCLES(1);
    NEXT();
}

void OPCALL movr8e8_32(struct CPU* cpu, struct Op* op) {
    *cpu->reg8[op->r1] = readb(cpu->thread, eaa32(cpu, op));
    CYCLES(1);
    NEXT();
}

void OPCALL move8r8_16(struct CPU* cpu, struct Op* op) {
    writeb(cpu->thread, eaa16(cpu, op), *cpu->reg8[op->r1]);
    CYCLES(1);
    NEXT();
}

void OPCALL move8r8_32(struct CPU* cpu, struct Op* op) {
    writeb(cpu->thread, eaa32(cpu, op), *cpu->reg8[op->r1]);
    CYCLES(1);
    NEXT();
}

void OPCALL movr16r16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    CYCLES(1);
    NEXT();
}

void OPCALL mov16_reg(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = op->data1;
    CYCLES(1);
    NEXT();
}

void OPCALL mov16_mem16(struct CPU* cpu, struct Op* op) {
    writew(cpu->thread, eaa16(cpu, op), op->data1);
    CYCLES(1);
    NEXT();
}

void OPCALL mov16_mem32(struct CPU* cpu, struct Op* op) {
    writew(cpu->thread, eaa32(cpu, op), op->data1);
    CYCLES(1);
    NEXT();
}

void OPCALL movr16e16_16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    CYCLES(1);
    NEXT();
}

void OPCALL movr16e16_32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    CYCLES(1);
    NEXT();
}

void OPCALL move16r16_16(struct CPU* cpu, struct Op* op) {
    writew(cpu->thread, eaa16(cpu, op), cpu->reg[op->r1].u16);
    CYCLES(1);
    NEXT();
}

void OPCALL move16r16_32(struct CPU* cpu, struct Op* op) {
    writew(cpu->thread, eaa32(cpu, op), cpu->reg[op->r1].u16);
    CYCLES(1);
    NEXT();
}

void OPCALL movr32r32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    CYCLES(1);
    NEXT();
}

void OPCALL mov32_reg(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = op->data1;
    CYCLES(1);
    NEXT();
}

void OPCALL mov32_mem16(struct CPU* cpu, struct Op* op) {
    writed(cpu->thread, eaa16(cpu, op), op->data1);
    CYCLES(1);
    NEXT();
}

void OPCALL mov32_mem32(struct CPU* cpu, struct Op* op) {
    writed(cpu->thread, eaa32(cpu, op), op->data1);
    CYCLES(1);
    NEXT();
}

void OPCALL movr32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    CYCLES(1);
    NEXT();
}

void OPCALL movr32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    CYCLES(1);
    NEXT();
}

void OPCALL move32r32_16(struct CPU* cpu, struct Op* op) {
    writed(cpu->thread, eaa16(cpu, op), cpu->reg[op->r1].u32);
    CYCLES(1);
    NEXT();
}

void OPCALL move32r32_32(struct CPU* cpu, struct Op* op) {
    writed(cpu->thread, eaa32(cpu, op), cpu->reg[op->r1].u32);
    CYCLES(1);
    NEXT();
}

void OPCALL movr16s16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = cpu->segValue[op->r2];
    CYCLES(1);
    NEXT();
}

void OPCALL movr32s16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = cpu->segValue[op->r2];
    CYCLES(1);
    NEXT();
}

void OPCALL move16s16_16(struct CPU* cpu, struct Op* op) {
    writew(cpu->thread, eaa16(cpu, op), cpu->segValue[op->r1]);
    CYCLES(1);
    NEXT();
}

void OPCALL move16s16_32(struct CPU* cpu, struct Op* op) {
    writew(cpu->thread, eaa32(cpu, op), cpu->segValue[op->r1]);
    CYCLES(1);
    NEXT();
}

void OPCALL movxz8r16r16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = *cpu->reg8[op->r2];
    CYCLES(3);
    NEXT();
}

void OPCALL movxz8r16e16_16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = readb(cpu->thread, eaa16(cpu, op));
    CYCLES(3);
    NEXT();
}

void OPCALL movxz8r16e16_32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = readb(cpu->thread, eaa32(cpu, op));
    CYCLES(3);
    NEXT();
}

void OPCALL movxz8r32r32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = *cpu->reg8[op->r2];
    CYCLES(3);
    NEXT();
}

void OPCALL movxz8r32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = readb(cpu->thread, eaa16(cpu, op));
    CYCLES(3);
    NEXT();
}

void OPCALL movxz8r32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = readb(cpu->thread, eaa32(cpu, op));
    CYCLES(3);
    NEXT();
}

void OPCALL movsx8r16r16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = (S8)*cpu->reg8[op->r2];
    CYCLES(3);
    NEXT();
}

void OPCALL movsx8r16e16_16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = (S8)readb(cpu->thread, eaa16(cpu, op));
    CYCLES(3);
    NEXT();
}

void OPCALL movsx8r16e16_32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = (S8)readb(cpu->thread, eaa32(cpu, op));
    CYCLES(3);
    NEXT();
}

void OPCALL movsx8r32r32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (S8)(*cpu->reg8[op->r2]);
    CYCLES(3);
    NEXT();
}

void OPCALL movsx8r32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (S8)readb(cpu->thread, eaa16(cpu, op));
    CYCLES(3);
    NEXT();
}

void OPCALL movsx8r32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (S8)readb(cpu->thread, eaa32(cpu, op));
    CYCLES(3);
    NEXT();
}

void OPCALL movsx16r32r32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (S16)cpu->reg[op->r2].u16;
    CYCLES(3);
    NEXT();
}

void OPCALL movsx16r32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (S16)readw(cpu->thread, eaa16(cpu, op));
    CYCLES(3);
    NEXT();
}

void OPCALL movsx16r32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (S16)readw(cpu->thread, eaa32(cpu, op));
    CYCLES(3);
    NEXT();
}

void OPCALL movxz16r32r32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = cpu->reg[op->r2].u16;
    CYCLES(3);
    NEXT();
}

void OPCALL movxz16r32e32_16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = readw(cpu->thread, eaa16(cpu, op));
    CYCLES(3);
    NEXT();
}

void OPCALL movxz16r32e32_32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = readw(cpu->thread, eaa32(cpu, op));
    CYCLES(3);
    NEXT();
}

void OPCALL lear16_16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = eaa16(cpu, op);
    CYCLES(1);
    NEXT();
}

void OPCALL lear16_32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = eaa32(cpu, op);
    CYCLES(1);
    NEXT();
}

void OPCALL lear32_16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = eaa16(cpu, op);
    CYCLES(1);
    NEXT();
}

void OPCALL lear32_32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = eaa32(cpu, op);
    CYCLES(1);
    NEXT();
}

void OPCALL movs16r16(struct CPU* cpu, struct Op* op) {
    if (!cpu_setSegment(cpu, op->r2, cpu->reg[op->r1].u16))
        return;
    CYCLES(2);
    NEXT();
}

void OPCALL movs16e16_16(struct CPU* cpu, struct Op* op) {
    if (!cpu_setSegment(cpu, op->r1, readw(cpu->thread, eaa16(cpu, op))))
        return;
    CYCLES(3);
    NEXT();
}

void OPCALL movs16e16_32(struct CPU* cpu, struct Op* op) {
    if (!cpu_setSegment(cpu, op->r1, readw(cpu->thread, eaa32(cpu, op))))
        return;
    CYCLES(3);
    NEXT();
}

void OPCALL pope16_16(struct CPU* cpu, struct Op* op) {
    writew(cpu->thread, eaa16(cpu, op), pop16(cpu));
    CYCLES(3);
    NEXT();
}

void OPCALL pope16_32(struct CPU* cpu, struct Op* op) {
    writew(cpu->thread, eaa32(cpu, op), pop16(cpu));
    CYCLES(3);
    NEXT();
}

void OPCALL pope32_16(struct CPU* cpu, struct Op* op) {
    writed(cpu->thread, eaa16(cpu, op), pop32(cpu));
    CYCLES(3);
    NEXT();
}

void OPCALL pope32_32(struct CPU* cpu, struct Op* op) {
    writed(cpu->thread, eaa32(cpu, op), pop32(cpu));
    CYCLES(3);
    NEXT();
}

void OPCALL cbw(struct CPU* cpu, struct Op* op) {
    AX = (S8)AL;
    CYCLES(3);
    NEXT();
}

void OPCALL cbwe(struct CPU* cpu, struct Op* op) {
    EAX = (S16)AX;
    CYCLES(3);
    NEXT();
}

void OPCALL cwd(struct CPU* cpu, struct Op* op) {
    if (((S16)AX) < 0)
        DX = 0xFFFF;
    else
        DX = 0;
    CYCLES(2);
    NEXT();
}

void OPCALL cwq(struct CPU* cpu, struct Op* op) {
    if (((S32)EAX) < 0)
        EDX = 0xFFFFFFFF;
    else
        EDX = 0;
    CYCLES(2);
    NEXT();
}

void OPCALL pushf16(struct CPU* cpu, struct Op* op) {
    fillFlags(cpu);
    push16(cpu, cpu->flags|2);
    CYCLES(3);
    NEXT();
}

void OPCALL pushf32(struct CPU* cpu, struct Op* op) {
    fillFlags(cpu);
    push32(cpu, (cpu->flags|2) & 0xFCFFFF);
    CYCLES(3);
    NEXT();
}

void OPCALL popf16(struct CPU* cpu, struct Op* op) {
    cpu->lazyFlags = FLAGS_NONE;
    setFlags(cpu, pop16(cpu), FMASK_ALL & 0xFFFF);
    CYCLES(4);
    NEXT();
}

void OPCALL popf32(struct CPU* cpu, struct Op* op) {
    cpu->lazyFlags = FLAGS_NONE;
    setFlags(cpu, pop32(cpu), FMASK_ALL);
    CYCLES(4);
    NEXT();
}

void OPCALL sahf(struct CPU* cpu, struct Op* op) {
    fillFlags(cpu);
    setFlags(cpu, AH, FMASK_ALL & 0xFF);
    CYCLES(2);
    NEXT();
}

void OPCALL lahf(struct CPU* cpu, struct Op* op) {
    fillFlags(cpu);
    AH = (cpu->flags & (SF|ZF|AF|PF|CF)) | 2;
    CYCLES(2);
    NEXT();
}

void OPCALL movAl(struct CPU* cpu, struct Op* op) {
    AL = readb(cpu->thread, cpu->segAddress[op->base]+op->data1);
    CYCLES(1);
    NEXT();
}

void OPCALL movAx(struct CPU* cpu, struct Op* op) {
    AX = readw(cpu->thread, cpu->segAddress[op->base]+op->data1);
    CYCLES(1);
    NEXT();
}

void OPCALL movEax(struct CPU* cpu, struct Op* op) {
    EAX = readd(cpu->thread, cpu->segAddress[op->base]+op->data1);
    CYCLES(1);
    NEXT();
}

void OPCALL movDirectAl(struct CPU* cpu, struct Op* op) {
    writeb(cpu->thread, cpu->segAddress[op->base]+op->data1, AL);
    CYCLES(1);
    NEXT();
}

void OPCALL movDirectAx(struct CPU* cpu, struct Op* op) {
    writew(cpu->thread, cpu->segAddress[op->base]+op->data1, AX);
    CYCLES(1);
    NEXT();
}

void OPCALL movDirectEax(struct CPU* cpu, struct Op* op) {
    writed(cpu->thread, cpu->segAddress[op->base]+op->data1, EAX);
    CYCLES(1);
    NEXT();
}

void OPCALL retnIw16(struct CPU* cpu, struct Op* op) {
    U16 eip = pop16(cpu);	
    SP = SP+op->data1;
    DONE();
    cpu->eip.u32 = eip;
    CYCLES(3);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL retnIw32(struct CPU* cpu, struct Op* op) {
    U32 eip = pop32(cpu);		
    ESP = ESP+op->data1;
    DONE();	
    cpu->eip.u32 = eip;
    CYCLES(3);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL retn16(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32 = pop16(cpu);
    CYCLES(2);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL retn32(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32 = pop32(cpu);
    CYCLES(2);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL leave16(struct CPU* cpu, struct Op* op) {
    SP = BP;
    BP = pop16(cpu);
    CYCLES(3);
    NEXT();
}

void OPCALL enter16(struct CPU* cpu, struct Op* op) {
    cpu_enter16(cpu, op->data1, op->r1);
    CYCLES(15);
    NEXT();
}

void OPCALL leave32(struct CPU* cpu, struct Op* op) {
    ESP = EBP;
    EBP = pop32(cpu);
    CYCLES(3);
    NEXT();
}

void OPCALL enter32(struct CPU* cpu, struct Op* op) {
    cpu_enter32(cpu, op->data1, op->r1);
    CYCLES(15);
    NEXT();
}

void OPCALL syscall_op(struct CPU* cpu, struct Op* op);

void OPCALL salc(struct CPU* cpu, struct Op* op) {
    if (getCF(cpu))
        AL = 0xFF;
    else
        AL = 0;
    CYCLES(2); // :TODO:
    NEXT();
}

void OPCALL aam(struct CPU* cpu, struct Op* op) {
     instruction_aam(cpu, op->data1);
    CYCLES(18);
    NEXT();
}

void OPCALL aad(struct CPU* cpu, struct Op* op) {
    instruction_aad(cpu, op->data1);
    CYCLES(10);
    NEXT();
}


void OPCALL xlat16(struct CPU* cpu, struct Op* op) {
    AL = readb(cpu->thread, cpu->segAddress[op->base] + (U16)(BX + AL));
    CYCLES(4);
    NEXT();
}

void OPCALL xlat32(struct CPU* cpu, struct Op* op) {
    AL = readb(cpu->thread, cpu->segAddress[op->base] + EBX + AL);
    CYCLES(4);
    NEXT();
}

void OPCALL loopnz16(struct CPU* cpu, struct Op* op) {
    CX--;
    DONE();
    cpu->eip.u32+=op->eipCount;
    if (CX!=0 && !getZF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
    } else {
        cpu->nextBlock = getBlock1(cpu);
    }
    CYCLES(7);
}

void OPCALL loopnz32(struct CPU* cpu, struct Op* op) {
    ECX--;
    DONE();
    cpu->eip.u32+=op->eipCount;
    if (ECX!=0 && !getZF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
    } else {
        cpu->nextBlock = getBlock1(cpu);
    }
    CYCLES(7);
}

void OPCALL loopz16(struct CPU* cpu, struct Op* op) {
    CX--;
    DONE();
    cpu->eip.u32+=op->eipCount;
    if (CX!=0 && getZF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
    } else {
        cpu->nextBlock = getBlock1(cpu);
    }
    CYCLES(7);
}

void OPCALL loopz32(struct CPU* cpu, struct Op* op) {
    ECX--;
    DONE();
    cpu->eip.u32+=op->eipCount;
    if (ECX!=0 && getZF(cpu)) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
    } else {
        cpu->nextBlock = getBlock1(cpu);
    }
    CYCLES(7);
}

void OPCALL loop16(struct CPU* cpu, struct Op* op) {
    CX--;
    DONE();
    cpu->eip.u32+=op->eipCount;
    if (CX!=0) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
    } else {
        cpu->nextBlock = getBlock1(cpu);
    }
    CYCLES(5);
}

void OPCALL loop32(struct CPU* cpu, struct Op* op) {
    ECX--;
    DONE();
    cpu->eip.u32+=op->eipCount;
    if (ECX!=0) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
    } else {
        cpu->nextBlock = getBlock1(cpu);
    }
    CYCLES(5);
}

void OPCALL jcxz16(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    if (CX==0) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
    } else {
        cpu->nextBlock = getBlock1(cpu);
    }
    CYCLES(5);
}

void OPCALL jcxz32(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    if (ECX==0) {
        cpu->eip.u32+=op->data1;
        cpu->nextBlock = getBlock2(cpu);
    } else {
        cpu->nextBlock = getBlock1(cpu);
    }
    CYCLES(5);
}

void OPCALL callJw(struct CPU* cpu, struct Op* op) {
    push16(cpu, cpu->eip.u32 + op->eipCount);
    DONE();
    cpu->eip.u32 += op->eipCount + op->data1;
    cpu->nextBlock = getBlock1(cpu);
    CYCLES(1);
}

void OPCALL callJd(struct CPU* cpu, struct Op* op) {
    push32(cpu, cpu->eip.u32 + op->eipCount);
    DONE();
    cpu->eip.u32 += op->eipCount + (S32)op->data1;
    cpu->nextBlock = getBlock1(cpu);
    CYCLES(1);
}

void OPCALL jump(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32 += op->eipCount + (S32)op->data1;
    cpu->nextBlock = getBlock1(cpu);
    CYCLES(1);
}

void OPCALL cmc(struct CPU* cpu, struct Op* op) {
    fillFlags(cpu);
    setCF(cpu, !(cpu->flags & CF));
    CYCLES(2);
    NEXT();
}

void OPCALL neg8_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u8 = *cpu->reg8[op->r1];
    cpu->result.u8 = 0-cpu->dst.u8;
    *cpu->reg8[op->r1] = cpu->result.u8;
    cpu->lazyFlags = FLAGS_NEG8;
    CYCLES(1);
    NEXT();
}

void OPCALL neg8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->result.u8 = 0-cpu->dst.u8;
    writeb(cpu->thread, eaa, cpu->result.u8);
    cpu->lazyFlags = FLAGS_NEG8;
    CYCLES(3);
    NEXT();
}

void OPCALL neg8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u8 = readb(cpu->thread, eaa);
    cpu->result.u8 = 0-cpu->dst.u8;
    writeb(cpu->thread, eaa, cpu->result.u8);
    cpu->lazyFlags = FLAGS_NEG8;
    CYCLES(3);
    NEXT();
}

void OPCALL mul8_reg(struct CPU* cpu, struct Op* op) {
    AX = AL * (*cpu->reg8[op->r1]);
    fillFlagsNoCFOF(cpu);
    if (AX>0xFF) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(11);
    NEXT();
}

void OPCALL mul8_mem16(struct CPU* cpu, struct Op* op) {
    AX = AL * readb(cpu->thread, eaa16(cpu, op));
    fillFlagsNoCFOF(cpu);
    if (AX>0xFF) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(11);
    NEXT();
}

void OPCALL mul8_mem32(struct CPU* cpu, struct Op* op) {
    AX = AL * readb(cpu->thread, eaa32(cpu, op));
    fillFlagsNoCFOF(cpu);
    if (AX>0xFF) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(11);
    NEXT();
}

void OPCALL imul8_reg(struct CPU* cpu, struct Op* op) {
    AX = (S16)((S8)AL) * (S8)(*cpu->reg8[op->r1]);
    fillFlagsNoCFOF(cpu);
    if ((S16)AX<-128 || (S16)AX>127) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(11);
    NEXT();
}

void OPCALL imul8_mem16(struct CPU* cpu, struct Op* op) {
    AX = (S16)((S8)AL) * (S8)readb(cpu->thread, eaa16(cpu, op));
    fillFlagsNoCFOF(cpu);
    if ((S16)AX<-128 || (S16)AX>127) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(11);
    NEXT();
}

void OPCALL imul8_mem32(struct CPU* cpu, struct Op* op) {
    AX = (S16)((S8)AL) * (S8)readb(cpu->thread, eaa32(cpu, op));
    fillFlagsNoCFOF(cpu);
    if ((S16)AX<-128 || (S16)AX>127) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(11);
    NEXT();
}

U32 div8(struct CPU* cpu, U8 src) {
    U16 quo;
    U8 rem;

    if (src==0) {
        cpu_exception(cpu, EXCEPTION_DIVIDE, 0);
        return 0;
    }

    quo = AX / src;
    rem = AX % src;

    if (quo > 255) {
        cpu_exception(cpu, EXCEPTION_DIVIDE, 1);
        return 0;
    }
    AL = (U8)quo;
    AH = rem;
    return 1;
}

void OPCALL div8_reg(struct CPU* cpu, struct Op* op) {
    CYCLES(17);
    if (div8(cpu, *cpu->reg8[op->r1])) {
        NEXT();
    }
}

void OPCALL div8_mem16(struct CPU* cpu, struct Op* op) {
    CYCLES(17);
    if (div8(cpu, readb(cpu->thread, eaa16(cpu, op)))) {
        NEXT();
    }
}

void OPCALL div8_mem32(struct CPU* cpu, struct Op* op) {
    CYCLES(17);
    if (div8(cpu, readb(cpu->thread, eaa32(cpu, op)))) {
        NEXT();
    }
}

U32 idiv8(struct CPU* cpu, S8 src) {
    S16 quo;
    S8 quo8;
    S8 rem;

    if (src==0) {
        cpu_exception(cpu, EXCEPTION_DIVIDE, 0);
        return 0;
    }

    quo = (S16)AX / src;
    quo8 = (S8)quo;
    rem = (S16)AX % src;

    if (quo != quo8) {
        cpu_exception(cpu, EXCEPTION_DIVIDE, 1);
        return 0;
    }
    AL = quo8;
    AH = rem;
    return 1;
}

void OPCALL idiv8_reg(struct CPU* cpu, struct Op* op) {
    CYCLES(22);
    if (idiv8(cpu, (S8)(*cpu->reg8[op->r1]))) {
        NEXT();
    }
}

void OPCALL idiv8_mem16(struct CPU* cpu, struct Op* op) {
    CYCLES(22);
    if (idiv8(cpu, (S8)readb(cpu->thread, eaa16(cpu, op)))) {
        NEXT();
    }
}

void OPCALL idiv8_mem32(struct CPU* cpu, struct Op* op) {
    CYCLES(22);
    if (idiv8(cpu, (S8)readb(cpu->thread, eaa32(cpu, op)))) {
        NEXT();
    }
}

void OPCALL not8_reg(struct CPU* cpu, struct Op* op) {
    *cpu->reg8[op->r1] = ~ *cpu->reg8[op->r1];
    CYCLES(1);
    NEXT();
}

void OPCALL not8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    writeb(cpu->thread, eaa, ~readb(cpu->thread, eaa));
    CYCLES(3);
    NEXT();
}

void OPCALL not8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    writeb(cpu->thread, eaa, ~readb(cpu->thread, eaa));
    CYCLES(3);
    NEXT();
}

void OPCALL neg16_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = 0-cpu->dst.u16;
    cpu->reg[op->r1].u16 = cpu->result.u16;
    cpu->lazyFlags = FLAGS_NEG16;
    CYCLES(1);
    NEXT();
}

void OPCALL neg16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->result.u16 = 0-cpu->dst.u16;
    writew(cpu->thread, eaa, cpu->result.u16);
    cpu->lazyFlags = FLAGS_NEG16;
    CYCLES(3);
    NEXT();
}

void OPCALL neg16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u16 = readw(cpu->thread, eaa);
    cpu->result.u16 = 0-cpu->dst.u16;
    writew(cpu->thread, eaa, cpu->result.u16);
    cpu->lazyFlags = FLAGS_NEG16;
    CYCLES(3);
    NEXT();
}

void OPCALL mul16_reg(struct CPU* cpu, struct Op* op) {
    U32 result = (U32)AX * cpu->reg[op->r1].u16;
    AX = (U16)result;
    DX = (U16)(result >> 16);
    fillFlagsNoCFOF(cpu);
    if (DX) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(11);
    NEXT();
}

void OPCALL mul16_mem16(struct CPU* cpu, struct Op* op) {
    U32 result = (U32)AX * readw(cpu->thread, eaa16(cpu, op));
    AX = (U16)result;
    DX = (U16)(result >> 16);
    fillFlagsNoCFOF(cpu);
    if (DX) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(11);
    NEXT();
}

void OPCALL mul16_mem32(struct CPU* cpu, struct Op* op) {
    U32 result = (U32)AX * readw(cpu->thread, eaa32(cpu, op));
    AX = (U16)result;
    DX = (U16)(result >> 16);
    fillFlagsNoCFOF(cpu);
    if (DX) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(11);
    NEXT();
}

void OPCALL imul16_reg(struct CPU* cpu, struct Op* op) {
    S32 result = (S32)((S16)AX) * (S16)cpu->reg[op->r1].u16;
    AX = (S16)result;
    DX = (S16)(result >> 16);
    fillFlagsNoCFOF(cpu);
    if (result>32767 || result<-32768) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(11);
    NEXT();
}

void OPCALL imul16_mem16(struct CPU* cpu, struct Op* op) {
    S32 result = (S32)((S16)AX) * (S16)readw(cpu->thread, eaa16(cpu, op));
    AX = (S16)result;
    DX = (S16)(result >> 16);
    fillFlagsNoCFOF(cpu);
    if (result>32767 || result<-32768) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(11);
    NEXT();
}

void OPCALL imul16_mem32(struct CPU* cpu, struct Op* op) {
    S32 result = (S32)((S16)AX) * (S16)readw(cpu->thread, eaa32(cpu, op));
    AX = (S16)result;
    DX = (S16)(result >> 16);
    fillFlagsNoCFOF(cpu);
    if (result>32767 || result<-32768) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(11);
    NEXT();
}

U32 div16(struct CPU* cpu, U16 src) {	
    U32 num = ((U32)DX << 16) | AX;
    U32 quo;
    U16 rem;
    U16 quo16;

    if (src==0) {	
        cpu_exception(cpu, EXCEPTION_DIVIDE, 0);
        return 0;
    }
    quo=num/src;
    rem=(U16)(num % src);
    quo16=(U16)quo;
    if (quo!=(U32)quo16) {
        cpu_exception(cpu, EXCEPTION_DIVIDE, 1);
        return 0;
    }
    DX=rem;
    AX=quo16;
    return 1;
}

void OPCALL div16_reg(struct CPU* cpu, struct Op* op) {
    CYCLES(25);
    if (div16(cpu, cpu->reg[op->r1].u16)) {
        NEXT();
    }
}

void OPCALL div16_mem16(struct CPU* cpu, struct Op* op) {
    CYCLES(25);
    if (div16(cpu, readw(cpu->thread, eaa16(cpu, op)))) {
        NEXT();
    }
}

void OPCALL div16_mem32(struct CPU* cpu, struct Op* op) {
    CYCLES(25);
    if (div16(cpu, readw(cpu->thread, eaa32(cpu, op)))) {
        NEXT();
    }
}

U32 idiv16(struct CPU* cpu, S16 src) {
    S32 num = (S32)(((U32)DX << 16) | AX);
    S32 quo;
    S16 rem;
    S16 quo16s;

    if (src==0) {
        cpu_exception(cpu, EXCEPTION_DIVIDE, 0);
        return 0;
    }
    quo=num/src;
    rem=(S16)(num % src);
    quo16s=(S16)quo;
    if (quo!=(S32)quo16s) {
        cpu_exception(cpu, EXCEPTION_DIVIDE, 1);
        return 0;
    }
    DX=rem;
    AX=quo16s;
    return 1;
}

void OPCALL idiv16_reg(struct CPU* cpu, struct Op* op) {
    CYCLES(30);
    if (idiv16(cpu, (S16)cpu->reg[op->r1].u16)) {
        NEXT();
    }
}

void OPCALL idiv16_mem16(struct CPU* cpu, struct Op* op) {
    CYCLES(30);
    if (idiv16(cpu, (S16)readw(cpu->thread, eaa16(cpu, op)))) {
        NEXT();
    }
}

void OPCALL idiv16_mem32(struct CPU* cpu, struct Op* op) {
    CYCLES(30);
    if (idiv16(cpu, (S16)readw(cpu->thread, eaa32(cpu, op)))) {
        NEXT();
    }
}

void OPCALL not16_reg(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = ~cpu->reg[op->r1].u16;
    CYCLES(1);
    NEXT();
}

void OPCALL not16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    writew(cpu->thread, eaa, ~readw(cpu->thread, eaa));
    CYCLES(3);
    NEXT();
}

void OPCALL not16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    writew(cpu->thread, eaa, ~readw(cpu->thread, eaa));
    CYCLES(3);
    NEXT();
}

void OPCALL neg32_reg(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = 0-cpu->dst.u32;
    cpu->reg[op->r1].u32 = cpu->result.u32;
    cpu->lazyFlags = FLAGS_NEG32;
    CYCLES(1);
    NEXT();
}

void OPCALL neg32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->result.u32 = 0-cpu->dst.u32;
    writed(cpu->thread, eaa, cpu->result.u32);
    cpu->lazyFlags = FLAGS_NEG32;
    CYCLES(3);
    NEXT();
}

void OPCALL neg32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->result.u32 = 0-cpu->dst.u32;
    writed(cpu->thread, eaa, cpu->result.u32);
    cpu->lazyFlags = FLAGS_NEG32;
    CYCLES(3);
    NEXT();
}

void OPCALL mul32_reg(struct CPU* cpu, struct Op* op) {
    U64 result = (U64)EAX * cpu->reg[op->r1].u32;
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
    fillFlagsNoCFOF(cpu);
    if (EDX) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(10);
    NEXT();
}

void OPCALL mul32_mem16(struct CPU* cpu, struct Op* op) {
    U64 result = (U64)EAX * readd(cpu->thread, eaa16(cpu, op));
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
    fillFlagsNoCFOF(cpu);
    if (EDX) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(10);
    NEXT();
}

void OPCALL mul32_mem32(struct CPU* cpu, struct Op* op) {
    U64 result = (U64)EAX * readd(cpu->thread, eaa32(cpu, op));
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
    fillFlagsNoCFOF(cpu);
    if (EDX) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(10);
    NEXT();
}

void OPCALL imul32_reg(struct CPU* cpu, struct Op* op) {
    S64 result = (S64)((S32)EAX) * (S32)cpu->reg[op->r1].u32;
    EAX = (S32)result;
    EDX = (S32)(result >> 32);
    fillFlagsNoCFOF(cpu);
    if (result>0x7fffffffl || result<-0x7fffffffl) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(10);
    NEXT();
}

void OPCALL imul32_mem16(struct CPU* cpu, struct Op* op) {
    S64 result = (S64)((S32)EAX) * (S32)readd(cpu->thread, eaa16(cpu, op));
    EAX = (S32)result;
    EDX = (S32)(result >> 32);
    fillFlagsNoCFOF(cpu);
    if (result>0x7fffffffl || result<-0x7fffffffl) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(10);
    NEXT();
}

void OPCALL imul32_mem32(struct CPU* cpu, struct Op* op) {
    S64 result = (S64)((S32)EAX) * (S32)readd(cpu->thread, eaa32(cpu, op));
    EAX = (S32)result;
    EDX = (S32)(result >> 32);
    fillFlagsNoCFOF(cpu);
    if (result>0x7fffffffl || result<-0x7fffffffl) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
    CYCLES(10);
    NEXT();
}

U32 div32(struct CPU* cpu, U32 src) {	
    U64 num = ((U64)EDX << 32) | EAX;
    U64 quo;
    U32 rem;
    U32 quo32;

    if (src==0)	{
        cpu_exception(cpu, EXCEPTION_DIVIDE, 0);
        return 0;
    }

    quo=num/src;
    rem=(U32)(num % src);
    quo32=(U32)quo;
    if (quo!=(U64)quo32) {
        cpu_exception(cpu, EXCEPTION_DIVIDE, 1);
        return 0;
    }
    EDX=rem;
    EAX=quo32;
    return 1;
}

void OPCALL div32_reg(struct CPU* cpu, struct Op* op) {
    CYCLES(41);
    if (div32(cpu, cpu->reg[op->r1].u32)) {
        NEXT();
    }
}

void OPCALL div32_mem16(struct CPU* cpu, struct Op* op) {
    CYCLES(41);
    if (div32(cpu, readd(cpu->thread, eaa16(cpu, op)))) {
        NEXT();
    }
}

void OPCALL div32_mem32(struct CPU* cpu, struct Op* op) {
    CYCLES(41);
    if (div32(cpu, readd(cpu->thread, eaa32(cpu, op)))) {
        NEXT();
    }
}

U32 idiv32(struct CPU* cpu, S32 src) {
    S64 num = (S64)(((U64)EDX << 32) | EAX);
    S64 quo;
    S32 rem;
    S32 quo32s;

    if (src==0) {
        cpu_exception(cpu, EXCEPTION_DIVIDE, 0);
        return 0;
    }

    quo=num/src;
    rem=(S32)(num % src);
    quo32s=(S32)quo;
    if (quo!=(S64)quo32s) {
        cpu_exception(cpu, EXCEPTION_DIVIDE, 1);
        return 0;
    }
    EDX=rem;
    EAX=quo32s;
    return 1;
}

void OPCALL idiv32_reg(struct CPU* cpu, struct Op* op) {
    CYCLES(46);
    if (idiv32(cpu, (S32)cpu->reg[op->r1].u32)) {
        NEXT();
    }
}

void OPCALL idiv32_mem16(struct CPU* cpu, struct Op* op) {
    CYCLES(46);
    if (idiv32(cpu, (S32)readd(cpu->thread, eaa16(cpu, op)))) {
        NEXT();
    }
}

void OPCALL idiv32_mem32(struct CPU* cpu, struct Op* op) {
    CYCLES(46);
    if (idiv32(cpu, (S32)readd(cpu->thread, eaa32(cpu, op)))) {
        NEXT();
    }
}

void OPCALL not32_reg(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = ~cpu->reg[op->r1].u32;
    CYCLES(1);
    NEXT();
}

void OPCALL not32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    writed(cpu->thread, eaa, ~readd(cpu->thread, eaa));
    CYCLES(3);
    NEXT();
}

void OPCALL not32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    writed(cpu->thread, eaa, ~readd(cpu->thread, eaa));
    CYCLES(3);
    NEXT();
}

void OPCALL clc(struct CPU* cpu, struct Op* op) {
    fillFlags(cpu);
    cpu->flags &= ~CF;
    CYCLES(2);
    NEXT();
}

void OPCALL stc(struct CPU* cpu, struct Op* op) {
    fillFlags(cpu);
    cpu->flags |= CF;
    CYCLES(2);
    NEXT();
}

void OPCALL cld(struct CPU* cpu, struct Op* op) {
    removeFlag(DF);
    cpu->df = 1;
    CYCLES(2);
    NEXT();
}

void OPCALL cli(struct CPU* cpu, struct Op* op) {
    fillFlags(cpu);
    removeFlag(IF);
    CYCLES(7);
    NEXT();
}

void OPCALL sti(struct CPU* cpu, struct Op* op) {
    fillFlags(cpu);
    cpu->flags |= IF;
    CYCLES(7);
    NEXT();
}

void OPCALL std(struct CPU* cpu, struct Op* op) {
    addFlag(DF);
    cpu->df = -1;
    CYCLES(2);
    NEXT();
}

void OPCALL callEv16_reg(struct CPU* cpu, struct Op* op) {
    push16(cpu, cpu->eip.u32+op->eipCount);
    DONE();
    cpu->eip.u32 = cpu->reg[op->r1].u16;
    CYCLES(2);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL callEv16_mem16(struct CPU* cpu, struct Op* op) {
    U32 oldeip = cpu->eip.u32+op->eipCount;	
    U32 neweip = readw(cpu->thread, eaa16(cpu, op));
    push16(cpu, oldeip);
    DONE();
    cpu->eip.u32 = neweip;
    CYCLES(4);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL callEv16_mem32(struct CPU* cpu, struct Op* op) {
    U32 oldeip = cpu->eip.u32+op->eipCount;	
    U32 neweip = readw(cpu->thread, eaa32(cpu, op));
    push16(cpu, oldeip);
    DONE();
    cpu->eip.u32 = neweip;
    CYCLES(4);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL callEp16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    U16 newip = readw(cpu->thread, eaa);
    U16 newcs = readw(cpu->thread, eaa+2);
    fillFlags(cpu);
    DONE();
    cpu_call(cpu, 0, newcs, newip, cpu->eip.u32 + op->eipCount);
    CYCLES(4);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL callEp16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    U16 newip = readw(cpu->thread, eaa);
    U16 newcs = readw(cpu->thread, eaa+2);
    fillFlags(cpu);
    DONE();
    cpu_call(cpu, 0, newcs, newip, cpu->eip.u32 + op->eipCount);
    CYCLES(4);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL jmpEv16_reg(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32 = cpu->reg[op->r1].u16;
    CYCLES(2);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL jmpEv16_mem16(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32 = readw(cpu->thread, eaa16(cpu, op));
    CYCLES(2);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL jmpEv16_mem32(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32 = readw(cpu->thread, eaa32(cpu, op));
    CYCLES(2);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL pushEv16_reg(struct CPU* cpu, struct Op* op) {
    push16(cpu, cpu->reg[op->r1].u16);
    CYCLES(1);
    NEXT();
}

void OPCALL pushEv16_mem16(struct CPU* cpu, struct Op* op) {
    push16(cpu, readw(cpu->thread, eaa16(cpu, op)));
    CYCLES(2);
    NEXT();
}

void OPCALL pushEv16_mem32(struct CPU* cpu, struct Op* op) {
    push16(cpu, readw(cpu->thread, eaa32(cpu, op)));
    CYCLES(2);
    NEXT();
}

void OPCALL pushEd_reg(struct CPU* cpu, struct Op* op) {
    push32(cpu, cpu->reg[op->r1].u32);
    CYCLES(1);
    NEXT();
}

void OPCALL pushEd_mem16(struct CPU* cpu, struct Op* op) {
    push32(cpu, readd(cpu->thread, eaa16(cpu, op)));
    CYCLES(2);
    NEXT();
}

void OPCALL pushEd_mem32(struct CPU* cpu, struct Op* op) {
    push32(cpu, readd(cpu->thread, eaa32(cpu, op)));
    CYCLES(2);
    NEXT();
}

void OPCALL callNear32_reg(struct CPU* cpu, struct Op* op) {
    push32(cpu, cpu->eip.u32+op->eipCount);
    DONE();
    cpu->eip.u32 = cpu->reg[op->r1].u32;
    CYCLES(2);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
    cpu->nextBlock->startFunction = 1;
}

void OPCALL callNear32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eip = cpu->eip.u32+op->eipCount;	
    U32 neweip = readd(cpu->thread, eaa16(cpu, op));
    push32(cpu, eip);
    DONE();
    cpu->eip.u32 = neweip;
    CYCLES(4);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
    cpu->nextBlock->startFunction = 1;
}

void OPCALL callNear32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eip = cpu->eip.u32+op->eipCount;	
    U32 neweip = readd(cpu->thread, eaa32(cpu, op));
    push32(cpu, eip);
    DONE();
    cpu->eip.u32 = neweip;
    CYCLES(4);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
    cpu->nextBlock->startFunction = 1;
}

void OPCALL jmpNear32_reg(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32 = cpu->reg[op->r1].u32;
    CYCLES(2);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL jmpNear32_mem16(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32 = readd(cpu->thread, eaa16(cpu, op));
    CYCLES(4);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL jmpNear32_mem32(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32 = readd(cpu->thread, eaa32(cpu, op));
    CYCLES(4);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL rdtsc(struct CPU* cpu, struct Op* op) {
#ifdef INCLUDE_CYCLES
    U64 t = cpu->timeStampCounter+cpu->blockCounter;
#else
    U64 t = cpu->timeStampCounter+op->data1;
#endif
#ifdef LOG_OPS
    t = 1;
#endif
    EAX = (U32)t;
    EDX = (U32)(t >> 32);
    CYCLES(1);
    NEXT();
}

void cpuid(struct CPU* cpu) {
    switch (EAX) {
        case 0:	/* Vendor ID String and maximum level? */
            EAX=2;  /* Maximum level */
            EBX='G' | ('e' << 8) | ('n' << 16) | ('u'<< 24);
            EDX='i' | ('n' << 8) | ('e' << 16) | ('I'<< 24);
            ECX='n' | ('t' << 8) | ('e' << 16) | ('l'<< 24);
            break;
        case 1:	/* get processor type/family/model/stepping and feature flags */
            EAX=0x633;		/* intel pentium 2 */
            EBX=0;			/* Not Supported */
            ECX=0;			/* No features */
            EDX=0x00000011;	/* FPU+TimeStamp/RDTSC */
            EDX|= (1<<5);     /* MSR */
            EDX|= (1<<15);    /* support CMOV instructions */
            EDX|= (1<<13);    /* PTE Global Flag */
            EDX|= (1<<8);     /* CMPXCHG8B instruction */
            EDX|= (1<<23);    // MMX
            break;
        case 2: // TLB and cache
            EAX=0x3020101;
            EBX=0;
            ECX=0;
            EDX=0x0C040843;
            break;
        case 0x80000000:
            EAX = 0;
            break;
        default:
            kwarn("Unhandled CPUID Function %X", EAX);
            EAX=0;
            EBX=0;
            ECX=0;
            EDX=0;
            break;
    }
}

void OPCALL cpuid_op(struct CPU* cpu, struct Op* op) {
    cpuid(cpu);
    CYCLES(14);
    NEXT();
}

void OPCALL btr16r16(struct CPU* cpu, struct Op* op) {	
    U32 mask=1 << (cpu->reg[op->r2].u16 & 15);
    fillFlagsNoCF(cpu);
    setCF(cpu, cpu->reg[op->r1].u16 & mask);
    CYCLES(4);
    NEXT();
}

void OPCALL bte16r16_16(struct CPU* cpu, struct Op* op) {
    U32 mask=1 << (cpu->reg[op->r1].u16 & 15);
    U32 address = eaa16(cpu, op);

    fillFlagsNoCF(cpu);
    address+=(((S16)cpu->reg[op->r1].u16)>>4)*2;
    setCF(cpu, (readw(cpu->thread, address) & mask));
    CYCLES(9);
    NEXT();
}

void OPCALL bte16r16_32(struct CPU* cpu, struct Op* op) {
    U32 mask=1 << (cpu->reg[op->r1].u16 & 15);
    U32 address = eaa32(cpu, op);

    fillFlagsNoCF(cpu);
    address+=(((S16)cpu->reg[op->r1].u16)>>4)*2;
    setCF(cpu, (readw(cpu->thread, address) & mask));
    CYCLES(9);
    NEXT();
}

void OPCALL btr32r32(struct CPU* cpu, struct Op* op) {
    U32 mask=1 << (cpu->reg[op->r2].u32 & 31);
    fillFlagsNoCF(cpu);
    setCF(cpu, cpu->reg[op->r1].u32 & mask);
    CYCLES(4);
    NEXT();
}

void OPCALL bte32r32_16(struct CPU* cpu, struct Op* op) {
    U32 mask=1 << (cpu->reg[op->r1].u32 & 31);
    U32 address = eaa16(cpu, op);

    fillFlagsNoCF(cpu);
    address+=(((S32)cpu->reg[op->r1].u32)>>5)*4;
    setCF(cpu, readd(cpu->thread, address) & mask);
    CYCLES(9);
    NEXT();
}

void OPCALL bte32r32_32(struct CPU* cpu, struct Op* op) {
    U32 mask=1 << (cpu->reg[op->r1].u32 & 31);
    U32 address = eaa32(cpu, op);

    fillFlagsNoCF(cpu);
    address+=(((S32)cpu->reg[op->r1].u32)>>5)*4;
    setCF(cpu, readd(cpu->thread, address) & mask);
    CYCLES(9);
    NEXT();
}

void OPCALL dshlr16r16(struct CPU* cpu, struct Op* op) {
    U32 result;
    U32 tmp;
    cpu->src.u32 = op->data1;
    cpu->dst.u32 = cpu->reg[op->r1].u16;
    cpu->dst2.u32 = cpu->reg[op->r2].u16;
    tmp = (cpu->dst.u32<<16)|cpu->dst2.u32;
    result=tmp << cpu->src.u8;
    if (op->data1>16) result |= ((U32)(cpu->reg[op->r2].u16) << (op->data1 - 16));
    cpu->result.u16=(U16)(result >> 16);
    cpu->reg[op->r1].u16 = cpu->result.u16;
    cpu->lazyFlags=FLAGS_DSHL16;
    CYCLES(4);
    NEXT();
}

void OPCALL dshle16r16_16(struct CPU* cpu, struct Op* op) {
    U32 result;
    U32 address = eaa16(cpu, op);
    U32 tmp;

    cpu->src.u32 = op->data1;
    cpu->dst.u32 = readw(cpu->thread, address);
    cpu->dst2.u32 = cpu->reg[op->r1].u16;
    tmp = (cpu->dst.u32<<16)|cpu->dst2.u32;
    result=tmp << cpu->src.u8;
    if (op->data1>16) result |= ((U32)(cpu->reg[op->r1].u16) << (op->data1 - 16));
    cpu->result.u16=(U16)(result >> 16);
    writew(cpu->thread, address, cpu->result.u16);
    cpu->lazyFlags=FLAGS_DSHL16;
    CYCLES(4);
    NEXT();
}

void OPCALL dshle16r16_32(struct CPU* cpu, struct Op* op) {
    U32 result;
    U32 address = eaa32(cpu, op);
    U32 tmp;

    cpu->src.u32 = op->data1;
    cpu->dst.u32 = readw(cpu->thread, address);
    cpu->dst2.u32 = cpu->reg[op->r1].u16;
    tmp = (cpu->dst.u32<<16)|cpu->dst2.u32;
    result=tmp << cpu->src.u8;
    if (op->data1>16) result |= ((U32)(cpu->reg[op->r1].u16) << (op->data1 - 16));
    cpu->result.u16=(U16)(result >> 16);
    writew(cpu->thread, address, cpu->result.u16);
    cpu->lazyFlags=FLAGS_DSHL16;
    CYCLES(4);
    NEXT();
}

void OPCALL dshlr32r32(struct CPU* cpu, struct Op* op) {
    cpu->src.u32=op->data1;
    cpu->dst.u32=cpu->reg[op->r1].u32;
    cpu->result.u32=(cpu->reg[op->r1].u32 << op->data1) | (cpu->reg[op->r2].u32 >> (32-op->data1));
    cpu->reg[op->r1].u32 = cpu->result.u32;	
    cpu->lazyFlags=FLAGS_DSHL32;
    CYCLES(4);
    NEXT();
}

void OPCALL dshle32r32_16(struct CPU* cpu, struct Op* op) {
    U32 address = eaa16(cpu, op);

    cpu->src.u32=op->data1;
    cpu->dst.u32=readd(cpu->thread, address);
    cpu->result.u32=(cpu->dst.u32 << op->data1) | (cpu->reg[op->r1].u32 >> (32-op->data1));
    writed(cpu->thread, address, cpu->result.u32);
    cpu->lazyFlags=FLAGS_DSHL32;
    CYCLES(4);
    NEXT();
}

void OPCALL dshle32r32_32(struct CPU* cpu, struct Op* op) {
    U32 address = eaa32(cpu, op);

    cpu->src.u32=op->data1;
    cpu->dst.u32=readd(cpu->thread, address);
    cpu->result.u32=(cpu->dst.u32 << op->data1) | (cpu->reg[op->r1].u32 >> (32-op->data1));
    writed(cpu->thread, address, cpu->result.u32);
    cpu->lazyFlags=FLAGS_DSHL32;
    CYCLES(4);
    NEXT();
}

void OPCALL dshlclr16r16(struct CPU* cpu, struct Op* op) {
    if (CL & 0x1f) {
        U32 result;
        U32 tmp;

        cpu->src.u32 = CL;
        cpu->dst.u32 = cpu->reg[op->r1].u16;
        cpu->dst2.u32 = cpu->reg[op->r2].u16;
        tmp = (cpu->dst.u32<<16)|cpu->dst2.u32;
        result=tmp << cpu->src.u8;
        if (cpu->src.u32>16) result |= ((U32)(cpu->reg[op->r2].u16) << (cpu->src.u32 - 16));
        cpu->result.u16=(U16)(result >> 16);
        cpu->reg[op->r1].u16 = cpu->result.u16;
        cpu->lazyFlags=FLAGS_DSHL16;
    }
    CYCLES(4);
    NEXT();
}

void OPCALL dshlcle16r16_16(struct CPU* cpu, struct Op* op) {
    if (CL & 0x1f) {
        U32 result;
        U32 address = eaa16(cpu, op);
        U32 tmp;

        cpu->src.u32 = CL;
        cpu->dst.u32 = readw(cpu->thread, address);
        cpu->dst2.u32 = cpu->reg[op->r1].u16;
        tmp = (cpu->dst.u32<<16)|cpu->dst2.u32;
        result=tmp << cpu->src.u8;
        if (cpu->src.u32>16) result |= ((U32)(cpu->reg[op->r1].u16) << (cpu->src.u32 - 16));
        cpu->result.u16=(U16)(result >> 16);
        writew(cpu->thread, address, cpu->result.u16);
        cpu->lazyFlags=FLAGS_DSHL16;
    }
    CYCLES(5);
    NEXT();
}

void OPCALL dshlcle16r16_32(struct CPU* cpu, struct Op* op) {
    if (CL & 0x1f) {
        U32 result;
        U32 address = eaa32(cpu, op);
        U32 tmp;

        cpu->src.u32 = CL;
        cpu->dst.u32 = readw(cpu->thread, address);
        cpu->dst2.u32 = cpu->reg[op->r1].u16;
        tmp = (cpu->dst.u32<<16)|cpu->dst2.u32;
        result=tmp << cpu->src.u8;
        if (cpu->src.u32>16) result |= ((U32)(cpu->reg[op->r1].u16) << (cpu->src.u32 - 16));
        cpu->result.u16=(U16)(result >> 16);
        writew(cpu->thread, address, cpu->result.u16);
        cpu->lazyFlags=FLAGS_DSHL16;
    }
    CYCLES(5);
    NEXT();
}

void OPCALL dshlclr32r32(struct CPU* cpu, struct Op* op) {
    if (CL & 0x1f) {
        cpu->src.u32=CL & 0x1f;
        cpu->dst.u32=cpu->reg[op->r1].u32;
        cpu->result.u32=(cpu->dst.u32 << cpu->src.u32);
        cpu->result.u32|=(cpu->reg[op->r2].u32 >> (32-cpu->src.u32));
        cpu->reg[op->r1].u32 = cpu->result.u32;	
        cpu->lazyFlags=FLAGS_DSHL32;
    }
    CYCLES(4);
    NEXT();
}

void OPCALL dshlcle32r32_16(struct CPU* cpu, struct Op* op) {
    if (CL & 0x1f) {
        U32 address = eaa16(cpu, op);

        cpu->src.u32=CL & 0x1f;
        cpu->dst.u32=readd(cpu->thread, address);
        cpu->result.u32=(cpu->dst.u32 << cpu->src.u32);
        cpu->result.u32|=(cpu->reg[op->r1].u32 >> (32-cpu->src.u32));
        writed(cpu->thread, address, cpu->result.u32);
        cpu->lazyFlags=FLAGS_DSHL32;
    }
    CYCLES(5);
    NEXT();
}

void OPCALL dshlcle32r32_32(struct CPU* cpu, struct Op* op) {
    if (CL & 0x1f) {
        U32 address = eaa32(cpu, op);

        cpu->src.u32=CL & 0x1f;
        cpu->dst.u32=readd(cpu->thread, address);
        cpu->result.u32=(cpu->dst.u32 << cpu->src.u32);
        cpu->result.u32|=(cpu->reg[op->r1].u32 >> (32-cpu->src.u32));
        writed(cpu->thread, address, cpu->result.u32);
        cpu->lazyFlags=FLAGS_DSHL32;
    }
    CYCLES(5);
    NEXT();
}

void OPCALL btsr16r16(struct CPU* cpu, struct Op* op) {	
    U32 mask=1 << (cpu->reg[op->r2].u16 & 15);
    fillFlagsNoCF(cpu);
    setCF(cpu, cpu->reg[op->r1].u16 & mask);
    cpu->reg[op->r1].u16|=mask;
    CYCLES(7);
    NEXT();
}

void OPCALL btse16r16_16(struct CPU* cpu, struct Op* op) {
    U32 mask=1 << (cpu->reg[op->r1].u16 & 15);
    U32 address = eaa16(cpu, op);
    U16 value;

    fillFlagsNoCF(cpu);
    address+=(((S16)cpu->reg[op->r1].u16)>>4)*2;
    value = readw(cpu->thread, address);
    setCF(cpu, value & mask);
    writew(cpu->thread, address, value | mask);
    CYCLES(13);
    NEXT();
}

void OPCALL btse16r16_32(struct CPU* cpu, struct Op* op) {
    U32 mask=1 << (cpu->reg[op->r1].u16 & 15);
    U32 address = eaa32(cpu, op);
    U16 value;

    fillFlagsNoCF(cpu);
    address+=(((S16)cpu->reg[op->r1].u16)>>4)*2;
    value = readw(cpu->thread, address);
    setCF(cpu, value & mask);
    writew(cpu->thread, address, value | mask);
    CYCLES(13);
    NEXT();
}

void OPCALL btsr32r32(struct CPU* cpu, struct Op* op) {
    U32 mask=1 << (cpu->reg[op->r2].u32 & 31);
    fillFlagsNoCF(cpu);
    setCF(cpu, cpu->reg[op->r1].u32 & mask);
    cpu->reg[op->r1].u32|=mask;
    CYCLES(7);
    NEXT();
}

void OPCALL btse32r32_16(struct CPU* cpu, struct Op* op) {
    U32 mask=1 << (cpu->reg[op->r1].u32 & 31);
    U32 address = eaa16(cpu, op);
    U32 value;

    fillFlagsNoCF(cpu);
    address+=(((S32)cpu->reg[op->r1].u32)>>5)*4;
    value = readd(cpu->thread, address);
    setCF(cpu, value & mask);
    writed(cpu->thread, address, value|mask);
    CYCLES(13);
    NEXT();
}

void OPCALL btse32r32_32(struct CPU* cpu, struct Op* op) {
    U32 mask=1 << (cpu->reg[op->r1].u32 & 31);
    U32 address = eaa32(cpu, op);
    U32 value;

    fillFlagsNoCF(cpu);
    address+=(((S32)cpu->reg[op->r1].u32)>>5)*4;
    value = readd(cpu->thread, address);
    setCF(cpu, value & mask);
    writed(cpu->thread, address, value|mask);
    CYCLES(13);
    NEXT();
}

void OPCALL dshrr16r16(struct CPU* cpu, struct Op* op) {
    U32 result;

    cpu->src.u32 = op->data1;
    cpu->dst.u32 = (cpu->reg[op->r1].u16)|((U32)(cpu->reg[op->r2].u16)<<16);
    result=cpu->dst.u32 >> cpu->src.u8;
    if (op->data1>16) result |= ((U32)(cpu->reg[op->r2].u16) << (32 - op->data1));
    cpu->result.u16=(U16)result;
    cpu->reg[op->r1].u16 = cpu->result.u16;
    cpu->lazyFlags=FLAGS_DSHR16;
    CYCLES(4);
    NEXT();
}

void OPCALL dshre16r16_16(struct CPU* cpu, struct Op* op) {
    U32 result;
    U32 address = eaa16(cpu, op);

    cpu->src.u32 = op->data1;
    cpu->dst.u32 = readw(cpu->thread, address)|((U32)(cpu->reg[op->r1].u16)<<16);
    result=cpu->dst.u32 >> cpu->src.u8;
    if (op->data1>16) result |= ((U32)(cpu->reg[op->r1].u16) << (32 - op->data1));
    cpu->result.u16=(U16)result;
    writew(cpu->thread, address, cpu->result.u16);
    cpu->lazyFlags=FLAGS_DSHR16;
    CYCLES(4);
    NEXT();
}

void OPCALL dshre16r16_32(struct CPU* cpu, struct Op* op) {
    U32 result;
    U32 address = eaa32(cpu, op);

    cpu->src.u32 = op->data1;
    cpu->dst.u32 = readw(cpu->thread, address)|((U32)(cpu->reg[op->r1].u16)<<16);
    result=cpu->dst.u32 >> cpu->src.u8;
    if (op->data1>16) result |= ((U32)(cpu->reg[op->r1].u16) << (32 - op->data1));
    cpu->result.u16=(U16)result;
    writew(cpu->thread, address, cpu->result.u16);
    cpu->lazyFlags=FLAGS_DSHR16;
    CYCLES(4);
    NEXT();
}

void OPCALL dshrr32r32(struct CPU* cpu, struct Op* op) {
    cpu->src.u32=op->data1;
    cpu->dst.u32=cpu->reg[op->r1].u32;
    cpu->result.u32=(cpu->reg[op->r1].u32 >> op->data1) | (cpu->reg[op->r2].u32 << (32-op->data1));
    cpu->reg[op->r1].u32 = cpu->result.u32;	
    cpu->lazyFlags=FLAGS_DSHR32;
    CYCLES(4);
    NEXT();
}

void OPCALL dshre32r32_16(struct CPU* cpu, struct Op* op) {
    U32 address = eaa16(cpu, op);

    cpu->src.u32=op->data1;
    cpu->dst.u32=readd(cpu->thread, address);
    cpu->result.u32=(cpu->dst.u32 >> op->data1) | (cpu->reg[op->r1].u32 << (32-op->data1));
    writed(cpu->thread, address, cpu->result.u32);
    cpu->lazyFlags=FLAGS_DSHR32;
    CYCLES(4);
    NEXT();
}

void OPCALL dshre32r32_32(struct CPU* cpu, struct Op* op) {
    U32 address = eaa32(cpu, op);

    cpu->src.u32=op->data1;
    cpu->dst.u32=readd(cpu->thread, address);
    cpu->result.u32=(cpu->dst.u32 >> op->data1) | (cpu->reg[op->r1].u32 << (32-op->data1));
    writed(cpu->thread, address, cpu->result.u32);
    cpu->lazyFlags=FLAGS_DSHR32;
    CYCLES(4);
    NEXT();
}

void OPCALL dshrclr16r16(struct CPU* cpu, struct Op* op) {
    if (CL & 0x1f) {
        U32 result;

        cpu->src.u32 = CL;
        cpu->dst.u32 = (cpu->reg[op->r1].u16)|((U32)(cpu->reg[op->r2].u16)<<16);
        result=cpu->dst.u32 >> cpu->src.u8;
        if (cpu->src.u32>16) result |= ((U32)(cpu->reg[op->r2].u16) << (32 - cpu->src.u32));
        cpu->result.u16=(U16)result;
        cpu->reg[op->r1].u16 = cpu->result.u16;
        cpu->lazyFlags=FLAGS_DSHR16;
        }
    CYCLES(4);
    NEXT();
}

void OPCALL dshrcle16r16_16(struct CPU* cpu, struct Op* op) {
    if (CL & 0x1f) {
        U32 result;
        U32 address = eaa16(cpu, op);

        cpu->src.u32 = CL;
        cpu->dst.u32 = readw(cpu->thread, address)|((U32)(cpu->reg[op->r1].u16)<<16);
        result=cpu->dst.u32 >> cpu->src.u8;
        if (cpu->src.u32>16) result |= ((U32)(cpu->reg[op->r1].u16) << (32 - cpu->src.u32));
        cpu->result.u16=(U16)result;
        writew(cpu->thread, address, cpu->result.u16);
        cpu->lazyFlags=FLAGS_DSHR16;
    }
    CYCLES(4);
    NEXT();
}

void OPCALL dshrcle16r16_32(struct CPU* cpu, struct Op* op) {
    if (CL & 0x1f) {
        U32 result;
        U32 address = eaa32(cpu, op);

        cpu->src.u32 = CL;
        cpu->dst.u32 = readw(cpu->thread, address)|((U32)(cpu->reg[op->r1].u16)<<16);
        result=cpu->dst.u32 >> cpu->src.u8;
        if (cpu->src.u32>16) result |= ((U32)(cpu->reg[op->r1].u16) << (32 - cpu->src.u32));
        cpu->result.u16=(U16)result;
        writew(cpu->thread, address, cpu->result.u16);
        cpu->lazyFlags=FLAGS_DSHR16;
    }
    CYCLES(4);
    NEXT();
}

void OPCALL dshrclr32r32(struct CPU* cpu, struct Op* op) {
    if (CL & 0x1f) {
        cpu->src.u32=CL & 0x1f;
        cpu->dst.u32=cpu->reg[op->r1].u32;
        cpu->result.u32=(cpu->dst.u32 >> cpu->src.u32);
        cpu->result.u32 |= (cpu->reg[op->r2].u32 << (32-cpu->src.u32));
        cpu->reg[op->r1].u32 = cpu->result.u32;	
        cpu->lazyFlags=FLAGS_DSHR32;
    }
    CYCLES(4);
    NEXT();
}

void OPCALL dshrcle32r32_16(struct CPU* cpu, struct Op* op) {
    if (CL & 0x1f) {
        U32 address = eaa16(cpu, op);

        cpu->src.u32=CL & 0x1f;
        cpu->dst.u32=readd(cpu->thread, address);
        cpu->result.u32=(cpu->dst.u32 >> cpu->src.u32);
        cpu->result.u32 |= (cpu->reg[op->r1].u32 << (32-cpu->src.u32));
        writed(cpu->thread, address, cpu->result.u32);
        cpu->lazyFlags=FLAGS_DSHR32;
    }
    CYCLES(4);
    NEXT();
}

void OPCALL dshrcle32r32_32(struct CPU* cpu, struct Op* op) {
    if (CL & 0x1f) {
        U32 address = eaa32(cpu, op);

        cpu->src.u32=CL & 0x1f;
        cpu->dst.u32=readd(cpu->thread, address);
        cpu->result.u32=(cpu->dst.u32 >> cpu->src.u32);
        cpu->result.u32 |= (cpu->reg[op->r1].u32 << (32-cpu->src.u32));
        writed(cpu->thread, address, cpu->result.u32);
        cpu->lazyFlags=FLAGS_DSHR32;
    }
    CYCLES(4);
    NEXT();
}

void OPCALL cmpxchgr16r16(struct CPU* cpu, struct Op* op) {
    cpu->dst.u16 = AX;
    cpu->src.u16 = cpu->reg[op->r1].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    if (AX == cpu->src.u16) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    } else {
        AX = cpu->src.u16;
    }
    CYCLES(5);
    NEXT();
}

void OPCALL cmpxchge16r16_16(struct CPU* cpu, struct Op* op) {
    U32 address = eaa16(cpu, op);
    cpu->dst.u16 = AX;
    cpu->src.u16 = readw(cpu->thread, address);
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    if (AX == cpu->src.u16) {
        writew(cpu->thread, address, cpu->reg[op->r1].u16);
    } else {
        AX = cpu->src.u16;
    }
    CYCLES(6);
    NEXT();
}

void OPCALL cmpxchge16r16_32(struct CPU* cpu, struct Op* op) {
    U32 address = eaa32(cpu, op);
    cpu->dst.u16 = AX;
    cpu->src.u16 = readw(cpu->thread, address);
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    if (AX == cpu->src.u16) {
        writew(cpu->thread, address, cpu->reg[op->r1].u16);
    } else {
        AX = cpu->src.u16;
    }
    CYCLES(6);
    NEXT();
}

void OPCALL cmpxchgr32r32(struct CPU* cpu, struct Op* op) {
    cpu->dst.u32 = EAX;
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    if (EAX == cpu->src.u32) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    } else {
        EAX = cpu->src.u32;
    }
    CYCLES(5);
    NEXT();
}

void OPCALL cmpxchge32r32_16(struct CPU* cpu, struct Op* op) {
    U32 address = eaa16(cpu, op);
    cpu->dst.u32 = EAX;
    cpu->src.u32 = readd(cpu->thread, address);
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    if (EAX == cpu->src.u32) {
        writed(cpu->thread, address, cpu->reg[op->r1].u32);
    } else {
        EAX = cpu->src.u32;
    }
    CYCLES(6);
    NEXT();
}

void OPCALL cmpxchge32r32_32(struct CPU* cpu, struct Op* op) {
    U32 address = eaa32(cpu, op);
    cpu->dst.u32 = EAX;
    cpu->src.u32 = readd(cpu->thread, address);
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    if (EAX == cpu->src.u32) {
        writed(cpu->thread, address, cpu->reg[op->r1].u32);
    } else {
        EAX = cpu->src.u32;
    }
    CYCLES(6);
    NEXT();
}

void OPCALL nop(struct CPU* cpu, struct Op* op) {
    CYCLES(1);
    NEXT();
}

void OPCALL bsrr16r16(struct CPU* cpu, struct Op* op) {
    U16 value = cpu->reg[op->r2].u16;
    if (value==0) {
        addFlag(ZF);
    } else {
        U32 result = 15;
        while ((value & 0x8000)==0) { result--; value<<=1; }
        removeFlag(ZF);
        cpu->reg[op->r1].u16 = result;
    }
    cpu->lazyFlags = FLAGS_NONE;
    CYCLES(7);
    NEXT();
}

void OPCALL bsrr16e16_16(struct CPU* cpu, struct Op* op) {
    U16 value = readw(cpu->thread, eaa16(cpu, op));
    if (value==0) {
        addFlag(ZF);
    } else {
        U32 result = 15;
        while ((value & 0x8000)==0) { result--; value<<=1; }
        removeFlag(ZF);
        cpu->reg[op->r1].u16 = result;
    }
    cpu->lazyFlags = FLAGS_NONE;
    CYCLES(7);
    NEXT();
}

void OPCALL bsrr16e16_32(struct CPU* cpu, struct Op* op) {
    U16 value = readw(cpu->thread, eaa32(cpu, op));
    if (value==0) {
        addFlag(ZF);
    } else {
        U32 result = 15;
        while ((value & 0x8000)==0) { result--; value<<=1; }
        removeFlag(ZF);
        cpu->reg[op->r1].u16 = result;
    }
    cpu->lazyFlags = FLAGS_NONE;
    CYCLES(7);
    NEXT();
}

void OPCALL bsrr32r32(struct CPU* cpu, struct Op* op) {
    U32 value = cpu->reg[op->r2].u32;
    if (value==0) {
        addFlag(ZF);
    } else {
        U32 result = 31;
        while ((value & 0x80000000)==0) { result--; value<<=1; }
        removeFlag(ZF);
        cpu->reg[op->r1].u32 = result;
    }
    cpu->lazyFlags = FLAGS_NONE;
    CYCLES(7);
    NEXT();
}

void OPCALL bsrr32e32_16(struct CPU* cpu, struct Op* op) {
    U32 value = readd(cpu->thread, eaa16(cpu, op));
    if (value==0) {
        addFlag(ZF);
    } else {
        U32 result = 31;
        while ((value & 0x80000000)==0) { result--; value<<=1; }
        removeFlag(ZF);
        cpu->reg[op->r1].u32 = result;
    }
    cpu->lazyFlags = FLAGS_NONE;
    CYCLES(7);
    NEXT();
}

void OPCALL bsrr32e32_32(struct CPU* cpu, struct Op* op) {
    U32 value = readd(cpu->thread, eaa32(cpu, op));
    if (value==0) {
        addFlag(ZF);
    } else {
        U32 result = 31;
        while ((value & 0x80000000)==0) { result--; value<<=1; }
        removeFlag(ZF);
        cpu->reg[op->r1].u32 = result;
    }
    cpu->lazyFlags = FLAGS_NONE;
    CYCLES(7);
    NEXT();
}

void OPCALL bt_reg(struct CPU* cpu, struct Op* op) {
    fillFlagsNoCF(cpu);
    setCF(cpu, cpu->reg[op->r1].u32 & op->data1);
    CYCLES(4);
    NEXT();
}

void OPCALL bt_mem16(struct CPU* cpu, struct Op* op) {
    fillFlagsNoCF(cpu);
    setCF(cpu, readd(cpu->thread, eaa16(cpu, op)) & op->data1);
    CYCLES(4);
    NEXT();
}

void OPCALL bt_mem32(struct CPU* cpu, struct Op* op) {
    fillFlagsNoCF(cpu);
    setCF(cpu, readd(cpu->thread, eaa32(cpu, op)) & op->data1);
    CYCLES(4);
    NEXT();
}

void OPCALL bts_reg(struct CPU* cpu, struct Op* op) {
    fillFlagsNoCF(cpu);
    setCF(cpu, cpu->reg[op->r1].u32 & op->data1);
    cpu->reg[op->r1].u32 |= op->data1;
    CYCLES(7);
    NEXT();
}

void OPCALL bts_mem16(struct CPU* cpu, struct Op* op) {
    U32 address = eaa16(cpu, op);
    U32 value = readd(cpu->thread, address);

    fillFlagsNoCF(cpu);
    setCF(cpu, value & op->data1);
    writed(cpu->thread, address, value | op->data1);
    CYCLES(8);
    NEXT();
}

void OPCALL bts_mem32(struct CPU* cpu, struct Op* op) {
    U32 address = eaa32(cpu, op);
    U32 value = readd(cpu->thread, address);

    fillFlagsNoCF(cpu);
    setCF(cpu, value & op->data1);
    writed(cpu->thread, address, value | op->data1);
    CYCLES(8);
    NEXT();
}

void OPCALL btr_reg(struct CPU* cpu, struct Op* op) {
    fillFlagsNoCF(cpu);
    setCF(cpu, cpu->reg[op->r1].u32 & op->data1);
    cpu->reg[op->r1].u32 &= ~op->data1;
    CYCLES(7);
    NEXT();
}

void OPCALL btr_mem16(struct CPU* cpu, struct Op* op) {
    U32 address = eaa16(cpu, op);
    U32 value = readd(cpu->thread, address);

    fillFlagsNoCF(cpu);
    setCF(cpu, value & op->data1);
    writed(cpu->thread, address, value & ~op->data1);
    CYCLES(8);
    NEXT();
}

void OPCALL btr_mem32(struct CPU* cpu, struct Op* op) {
    U32 address = eaa32(cpu, op);
    U32 value = readd(cpu->thread, address);

    fillFlagsNoCF(cpu);
    setCF(cpu, value & op->data1);
    writed(cpu->thread, address, value & ~op->data1);
    CYCLES(8);
    NEXT();
}

void OPCALL btrr16r16(struct CPU* cpu, struct Op* op) {
    U32 mask=1 << (cpu->reg[op->r2].u16 & 15);

    fillFlagsNoCF(cpu);
    setCF(cpu, cpu->reg[op->r1].u16 & mask);
    cpu->reg[op->r1].u16 &= ~mask;
    CYCLES(7);
    NEXT();
}

void OPCALL btre16r16_16(struct CPU* cpu, struct Op* op) {
    U32 address = eaa16(cpu, op);
    U16 mask=1 << (cpu->reg[op->r1].u16 & 15);
    U16 value;

    fillFlagsNoCF(cpu);
    address+=(((S16)cpu->reg[op->r1].u16)>>4)*2;
    value = readw(cpu->thread, address);
    setCF(cpu, value & mask);
    writew(cpu->thread, address, value & ~mask);
    CYCLES(13);
    NEXT();
}

void OPCALL btre16r16_32(struct CPU* cpu, struct Op* op) {
    U32 address = eaa32(cpu, op)+(cpu->reg[op->r1].u16 >> 4)*2;
    U16 mask=1 << (cpu->reg[op->r1].u16 & 15);
    U16 value;

    fillFlagsNoCF(cpu);
    address+=(((S16)cpu->reg[op->r1].u16)>>4)*2;
    value = readw(cpu->thread, address);
    setCF(cpu, value & mask);
    writew(cpu->thread, address, value & ~mask);
    CYCLES(13);
    NEXT();
}

void OPCALL btrr32r32(struct CPU* cpu, struct Op* op) {
    U32 mask=1 << (cpu->reg[op->r2].u32 & 31);

    fillFlagsNoCF(cpu);
    setCF(cpu, cpu->reg[op->r1].u32 & mask);
    cpu->reg[op->r1].u32 &= ~mask;
    CYCLES(7);
    NEXT();
}

void OPCALL btre32r32_16(struct CPU* cpu, struct Op* op) {
    U32 address = eaa16(cpu, op);
    U32 mask=1 << (cpu->reg[op->r1].u32 & 31);
    U32 value;

    fillFlagsNoCF(cpu);
    address+=(((S32)cpu->reg[op->r1].u32)>>5)*4;
    value = readd(cpu->thread, address);
    setCF(cpu, value & mask);
    writed(cpu->thread, address, value & ~mask);
    CYCLES(13);
    NEXT();
}

void OPCALL btre32r32_32(struct CPU* cpu, struct Op* op) {
    U32 address = eaa32(cpu, op);
    U32 mask=1 << (cpu->reg[op->r1].u32 & 31);
    U32 value;

    fillFlagsNoCF(cpu);
    address+=(((S32)cpu->reg[op->r1].u32)>>5)*4;
    value = readd(cpu->thread, address);
    setCF(cpu, value & mask);
    writed(cpu->thread, address, value & ~mask);
    CYCLES(13);
    NEXT();
}

void OPCALL btc_reg(struct CPU* cpu, struct Op* op) {
    fillFlagsNoCF(cpu);
    setCF(cpu, cpu->reg[op->r1].u32 & op->data1);
    cpu->reg[op->r1].u32 ^= op->data1;
    CYCLES(7);
    NEXT();
}

void OPCALL btc_mem16(struct CPU* cpu, struct Op* op) {
    U32 address = eaa16(cpu, op);
    U32 value = readd(cpu->thread, address);

    fillFlagsNoCF(cpu);
    setCF(cpu, value & op->data1);
    writed(cpu->thread, address, value ^ op->data1);
    CYCLES(8);
    NEXT();
}

void OPCALL btc_mem32(struct CPU* cpu, struct Op* op) {
    U32 address = eaa32(cpu, op);
    U32 value = readd(cpu->thread, address);

    fillFlagsNoCF(cpu);
    setCF(cpu, value & op->data1);
    writed(cpu->thread, address, value ^ op->data1);
    CYCLES(8);
    NEXT();
}

void OPCALL btcr32r32(struct CPU* cpu, struct Op* op) {
    U32 mask = 1 << cpu->reg[op->r2].u32 & 31;
    fillFlagsNoCF(cpu);
    setCF(cpu, cpu->reg[op->r1].u32 & mask);
    cpu->reg[op->r1].u32 ^= mask;
    CYCLES(7);
    NEXT();
}

void OPCALL btce32r32_16(struct CPU* cpu, struct Op* op) {
    U32 mask = 1 << cpu->reg[op->r1].u32 & 31;
    U32 address = eaa16(cpu, op)+((((S32)cpu->reg[op->r1].u32)>>3) & ~3);
    U32 value = readd(cpu->thread, address);

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    writed(cpu->thread, address, value ^ mask);
    CYCLES(13);
    NEXT();
}

void OPCALL btce32r32_32(struct CPU* cpu, struct Op* op) {
    U32 mask = 1 << cpu->reg[op->r1].u32 & 31;
    U32 address = eaa32(cpu, op)+((((S32)cpu->reg[op->r1].u32)>>3) & ~3);
    U32 value = readd(cpu->thread, address);

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    writed(cpu->thread, address, value ^ mask);
    CYCLES(13);
    NEXT();
}

void OPCALL bsfr16r16(struct CPU* cpu, struct Op* op) {
    U16 value=cpu->reg[op->r2].u16;

    fillFlagsNoZF(cpu);
    
    if (value==0) {
        addFlag(ZF);
    } else {
        U16 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        removeFlag(ZF);
        cpu->reg[op->r1].u16=result;
    }
    CYCLES(6);
    NEXT();
}

void OPCALL bsfr16e16_16(struct CPU* cpu, struct Op* op) {
    U16 value=readw(cpu->thread, eaa16(cpu, op));

    fillFlagsNoZF(cpu);
    
    if (value==0) {
        addFlag(ZF);
    } else {
        U16 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        removeFlag(ZF);
        cpu->reg[op->r1].u16=result;
    }
    CYCLES(6);
    NEXT();
}

void OPCALL bsfr16e16_32(struct CPU* cpu, struct Op* op) {
    U16 value=readw(cpu->thread, eaa32(cpu, op));

    fillFlagsNoZF(cpu);
    
    if (value==0) {
        addFlag(ZF);
    } else {
        U16 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        removeFlag(ZF);
        cpu->reg[op->r1].u16=result;
    }
    CYCLES(6);
    NEXT();
}

void OPCALL bsfr32r32(struct CPU* cpu, struct Op* op) {
    U32 value=cpu->reg[op->r2].u32;

    fillFlagsNoZF(cpu);
    
    if (value==0) {
        addFlag(ZF);
    } else {
        U32 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        removeFlag(ZF);
        cpu->reg[op->r1].u32=result;
    }
    CYCLES(6);
    NEXT();
}

void OPCALL bsfr32e32_16(struct CPU* cpu, struct Op* op) {
    U32 value=readd(cpu->thread, eaa16(cpu, op));

    fillFlagsNoZF(cpu);
    
    if (value==0) {
        addFlag(ZF);
    } else {
        U32 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        removeFlag(ZF);
        cpu->reg[op->r1].u32=result;
    }
    CYCLES(6);
    NEXT();
}

void OPCALL bsfr32e32_32(struct CPU* cpu, struct Op* op) {
    U32 value=readd(cpu->thread, eaa32(cpu, op));

    fillFlagsNoZF(cpu);
    
    if (value==0) {
        addFlag(ZF);
    } else {
        U32 result = 0;
        while ((value & 0x01)==0) { result++; value>>=1; }
        removeFlag(ZF);
        cpu->reg[op->r1].u32=result;
    }
    CYCLES(6);
    NEXT();
}

void OPCALL cmpxchgg8b_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    U64 value1 = ((U64)EDX) << 32 | EAX;
    U64 value2 = readq(cpu->thread, eaa);

    fillFlags(cpu);
    if (value1 == value2) {
        addFlag(ZF);
        writed(cpu->thread, eaa, EBX);
        writed(cpu->thread, eaa+4, ECX);
    } else {
        removeFlag(ZF);
        EDX = (U32)(value2 >> 32);
        EAX = (U32)value2;
    }
    CYCLES(10);
    NEXT();
}

void OPCALL cmpxchgg8b_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    U64 value1 = ((U64)EDX) << 32 | EAX;
    U64 value2 = readq(cpu->thread, eaa);

    fillFlags(cpu);
    if (value1 == value2) {
        addFlag(ZF);
        writed(cpu->thread, eaa, EBX);
        writed(cpu->thread, eaa + 4, ECX);
    }
    else {
        removeFlag(ZF);
        EDX = (U32)(value2 >> 32);
        EAX = (U32)value2;
    }
    CYCLES(10);
    NEXT();
}

void OPCALL bswap32(struct CPU* cpu, struct Op* op) {
    U32 val = cpu->reg[op->r1].u32;

    cpu->reg[op->r1].u32 = (((val & 0xff000000) >> 24) | ((val & 0x00ff0000) >>  8) | ((val & 0x0000ff00) <<  8) | ((val & 0x000000ff) << 24));
    CYCLES(1);
    NEXT();
}

void OPCALL xadd32r32r32(struct CPU* cpu, struct Op* op) {
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->dst.u32 = cpu->reg[op->r2].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->r1].u32 = cpu->dst.u32;
    cpu->reg[op->r2].u32 =  cpu->result.u32;
    CYCLES(3);
    NEXT();
}

void OPCALL xadd32r32e32_16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->r1].u32 = cpu->dst.u32;
    writed(cpu->thread, eaa, cpu->result.u32);
    CYCLES(4);
    NEXT();
}

void OPCALL xadd32r32e32_32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->src.u32 = cpu->reg[op->r1].u32;
    cpu->dst.u32 = readd(cpu->thread, eaa);
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->r1].u32 = cpu->dst.u32;
    writed(cpu->thread, eaa, cpu->result.u32);
    CYCLES(4);
    NEXT();
}

extern U64 sysCallTime;

void OPCALL int99(struct CPU* cpu, struct Op* op) {
    U32 index = peek32(cpu, 0);
    U64 startTime = getMicroCounter();

    if (index<int99CallbackSize && int99Callback[index]) {
        int99Callback[index](cpu);
    } else {
        kpanic("Uknown int 99 call: %d", index);
    }
    sysCallTime+=(getMicroCounter()-startTime);  
    NEXT();
}

void OPCALL retf32(struct CPU* cpu, struct Op* op) {
    fillFlags(cpu);
    DONE();
    cpu->eip.u32+=op->eipCount;
    cpu_ret(cpu, 1, op->data1, cpu->eip.u32);
    CYCLES(4);    
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL retf16(struct CPU* cpu, struct Op* op) {
    fillFlags(cpu);
    DONE();
    cpu->eip.u32+=op->eipCount;
    cpu_ret(cpu, 0, op->data1, cpu->eip.u32);
    CYCLES(4);    
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL callAp(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    cpu_call(cpu, 0, op->eData, op->data1, cpu->eip.u32);
    CYCLES(4);    
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL jmpAp(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    cpu_jmp(cpu, 0, op->eData, op->data1, cpu->eip.u32);
    CYCLES(4);    
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL emptyOp(struct CPU* cpu, struct Op* op) {
    cpu->nextBlock = getBlock1(cpu);
}

void OPCALL iret32(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    cpu_iret(cpu, 1, cpu->eip.u32);
    CYCLES(10);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL loadSegment16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    U32 val = readw(cpu->thread, eaa); // make sure all reads are done before writing something in case of a PF
    U32 selector = readw(cpu->thread, eaa+2);

    if (!cpu_setSegment(cpu, op->data1, selector))
        return;
    cpu->reg[op->r1].u16 = val;
    CYCLES(4);
    NEXT();
}

void OPCALL loadSegment16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    U32 val = readw(cpu->thread, eaa); // make sure all reads are done before writing something in case of a PF
    U32 selector = readw(cpu->thread, eaa+2);

    if (!cpu_setSegment(cpu, op->data1, selector))
        return;
    cpu->reg[op->r1].u16 = val;
    CYCLES(4);
    NEXT();
}

void OPCALL callFar(struct CPU* cpu, struct Op* op) {
    DONE();
    cpu->eip.u32+=op->eipCount;
    cpu_call(cpu, 1, op->eData, op->data1, cpu->eip.u32);
    CYCLES(4);
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL intOp(struct CPU* cpu, struct Op* op) {
    DONE();
    signalIllegalInstruction(cpu->thread, 5);// 5=ILL_PRVOPC  // :TODO: just a guess
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL larr16r16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = cpu_lar(cpu, cpu->reg[op->r2].u16, cpu->reg[op->r1].u16);
    CYCLES(8);
    NEXT();
}

void OPCALL lare16r16_16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = cpu_lar(cpu, readw(cpu->thread, eaa16(cpu, op)), cpu->reg[op->r1].u16);
    CYCLES(8);
    NEXT();
}

void OPCALL lare16r16_32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = cpu_lar(cpu, readw(cpu->thread, eaa32(cpu, op)), cpu->reg[op->r1].u16);
    CYCLES(8);
    NEXT();
}

void OPCALL lslr16r16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = cpu_lsl(cpu, cpu->reg[op->r2].u16, cpu->reg[op->r1].u16);
    CYCLES(8);
    NEXT();
}

void OPCALL lsle16r16_16(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = cpu_lsl(cpu, readw(cpu->thread, eaa16(cpu, op)), cpu->reg[op->r1].u16);
    CYCLES(8);
    NEXT();
}

void OPCALL lsle16r16_32(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u16 = cpu_lsl(cpu, readw(cpu->thread, eaa32(cpu, op)), cpu->reg[op->r1].u16);
    CYCLES(8);
    NEXT();
}

void OPCALL jmpEp16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    U16 newip = readw(cpu->thread, eaa);
    U16 newcs = readw(cpu->thread, eaa+2);
    fillFlags(cpu);
    DONE();
    cpu_jmp(cpu, 0, newcs, newip, cpu->eip.u32 + op->eipCount);
    CYCLES(4); // just a guess
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL jmpEp16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    U16 newip = readw(cpu->thread, eaa);
    U16 newcs = readw(cpu->thread, eaa+2);
    fillFlags(cpu);
    DONE();
    cpu_jmp(cpu, 0, newcs, newip, cpu->eip.u32 + op->eipCount);
    CYCLES(4); // just a guess
    cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
}

void OPCALL lmsw_reg(struct CPU* cpu, struct Op* op) {
    CYCLES(8);
    if (!cpu_lmsw(cpu, cpu->reg[op->r1].u16)) {
        DONE();
        cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
    } else {
        NEXT();
    }
}

void OPCALL lmsw_mem16(struct CPU* cpu, struct Op* op) {
    CYCLES(8);
    if (!cpu_lmsw(cpu, readw(cpu->thread, eaa16(cpu, op)))) {
        DONE();
        cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
    } else {
        NEXT();
    }
}

void OPCALL lmsw_mem32(struct CPU* cpu, struct Op* op) {
    CYCLES(8);
    if (!cpu_lmsw(cpu, readw(cpu->thread, eaa32(cpu, op)))) {
        DONE();
        cpu->nextBlock = getBlock(cpu, cpu->eip.u32);
    } else {
        NEXT();
    }
}

void OPCALL smsw_reg(struct CPU* cpu, struct Op* op) {
    CYCLES(4);
    cpu->reg[op->r1].u16 = cpu->cr0;
    NEXT();
}

void OPCALL smsw_mem16(struct CPU* cpu, struct Op* op) {
    CYCLES(4);
    writew(cpu->thread, eaa16(cpu, op), cpu->cr0);
    NEXT();
}

void OPCALL smsw_mem32(struct CPU* cpu, struct Op* op) {
    CYCLES(4);
    writew(cpu->thread, eaa32(cpu, op), cpu->cr0);
    NEXT();
}

void OPCALL sidt_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    CYCLES(4);
    writew(cpu->thread, eaa, 0); // limit
    writed(cpu->thread, eaa+2, 0); // base
#ifdef _DEBUG
    klog("sidt not implemented");
#endif
    NEXT();
}

void OPCALL sidt_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    CYCLES(4);
    writew(cpu->thread, eaa, 0); // limit
    writed(cpu->thread, eaa+2, 0); // base
#ifdef _DEBUG
    klog("sidt not implemented");
#endif
    NEXT();
}

void OPCALL sgdt_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    CYCLES(4);
    writew(cpu->thread, eaa, 0); // limit
    writed(cpu->thread, eaa+2, 0); // base
#ifdef _DEBUG
    klog("sgdt not implemented");
#endif
    NEXT();
}

void OPCALL sgdt_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    CYCLES(4);
    writew(cpu->thread, eaa, 0); // limit
    writed(cpu->thread, eaa+2, 0); // base
#ifdef _DEBUG
    klog("sgdt not implemented");
#endif
    NEXT();
}

void OPCALL bt16_reg(struct CPU* cpu, struct Op* op) {
    U16 value = cpu->reg[op->r1].u16;
    U16 mask = op->data1;

    fillFlagsNoCF(cpu);
    if ((value & mask)==0) {
        removeFlag(CF);
    } else {
        addFlag(CF);
    }
    CYCLES(4);
    NEXT();
}

void OPCALL bt16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    U16 value = readw(cpu->thread, eaa);
    U16 mask = op->data1;

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    CYCLES(4);
    NEXT();
}

void OPCALL bt16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    U16 value = readw(cpu->thread, eaa);
    U16 mask = op->data1;

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    CYCLES(4);
    NEXT();
}

void OPCALL bts16_reg(struct CPU* cpu, struct Op* op) {
    U16 value = cpu->reg[op->r1].u16;
    U16 mask = op->data1;

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    cpu->reg[op->r1].u16 |= mask;
    CYCLES(7);
    NEXT();
}

void OPCALL bts16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    U16 value = readw(cpu->thread, eaa);
    U16 mask = op->data1;

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    writew(cpu->thread, eaa, value | mask);
    CYCLES(8);
    NEXT();
}

void OPCALL bts16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    U16 value = readw(cpu->thread, eaa);
    U16 mask = op->data1;

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    writew(cpu->thread, eaa, value | mask);
    CYCLES(8);
    NEXT();
}

void OPCALL btr16_reg(struct CPU* cpu, struct Op* op) {
    U16 value = cpu->reg[op->r1].u16;
    U16 mask = op->data1;

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    cpu->reg[op->r1].u16 &= ~mask;
    CYCLES(7);
    NEXT();
}

void OPCALL btr16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    U16 value = readw(cpu->thread, eaa);
    U16 mask = op->data1;

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    writew(cpu->thread, eaa, value & ~mask);
    CYCLES(8);
    NEXT();
}

void OPCALL btr16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    U16 value = readw(cpu->thread, eaa);
    U16 mask = op->data1;

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    writew(cpu->thread, eaa, value & ~mask);
    CYCLES(8);
    NEXT();
}

void OPCALL btc16_reg(struct CPU* cpu, struct Op* op) {
    U16 value = cpu->reg[op->r1].u16;
    U16 mask = op->data1;

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    cpu->reg[op->r1].u16 ^= mask;
    CYCLES(7);
    NEXT();
}

void OPCALL btc16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    U16 value = readw(cpu->thread, eaa);
    U16 mask = op->data1;

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    writew(cpu->thread, eaa, value ^ mask);
    CYCLES(8);
    NEXT();
}

void OPCALL btc16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    U16 value = readw(cpu->thread, eaa);
    U16 mask = op->data1;

    fillFlagsNoCF(cpu);
    setCF(cpu, value & mask);
    writew(cpu->thread, eaa, value ^ mask);
    CYCLES(8);
    NEXT();
}

void OPCALL insb16_r_op(struct CPU* cpu, struct Op* op) {
    kpanic("INSB on port %X not implemented", DX);
}

void OPCALL insb16_op(struct CPU* cpu, struct Op* op) {
    kpanic("INSB on port %X not implemented", DX);
}

void OPCALL insb32_r_op(struct CPU* cpu, struct Op* op) {
    kpanic("INSB on port %X not implemented", DX);
}

void OPCALL insb32_op(struct CPU* cpu, struct Op* op) {
    kpanic("INSB on port %X not implemented", DX);
}

void OPCALL insw16_r_op(struct CPU* cpu, struct Op* op) {
    kpanic("INSW on port %X not implemented", DX);
}

void OPCALL insw16_op(struct CPU* cpu, struct Op* op) {
    kpanic("INSW on port %X not implemented", DX);
}

void OPCALL insw32_r_op(struct CPU* cpu, struct Op* op) {
    kpanic("INSW on port %X not implemented", DX);
}

void OPCALL insw32_op(struct CPU* cpu, struct Op* op) {
    kpanic("INSW on port %X not implemented", DX);
}

void OPCALL insd16_r_op(struct CPU* cpu, struct Op* op) {
    kpanic("INSD on port %X not implemented", DX);
}

void OPCALL insd16_op(struct CPU* cpu, struct Op* op) {
    kpanic("INSD on port %X not implemented", DX);
}

void OPCALL insd32_r_op(struct CPU* cpu, struct Op* op) {
    kpanic("INSD on port %X not implemented", DX);
}

void OPCALL insd32_op(struct CPU* cpu, struct Op* op) {
    kpanic("INSD on port %X not implemented", DX);
}