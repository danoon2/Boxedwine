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

U32 common_bound16(CPU* cpu, U32 reg, U32 address){
    if (cpu->reg[reg].u16<cpu->memory->readw(address) || cpu->reg[reg].u16>cpu->memory->readw(address+2)) {
        cpu->prepareException(EXCEPTION_BOUND, 0);
        return 0;
    } else { 
        return 1;
    }
}
U32 common_bound32(CPU* cpu, U32 reg, U32 address){
    if (cpu->reg[reg].u32<cpu->memory->readd(address) || cpu->reg[reg].u32>cpu->memory->readd(address+4)) {
        cpu->prepareException(EXCEPTION_BOUND, 0);
        return 0;
    } else { 
        return 1;
    }
}
void common_int99(CPU* cpu){
    U32 index = cpu->peek32(0);
    callOpenGL(cpu, index);    
}
void common_int9A(CPU* cpu) {
#ifdef BOXEDWINE_VULKAN
    U32 index = cpu->peek32(0);
    callVulkan(cpu, index);
#endif
}
void common_int9B(CPU* cpu) {
    U32 index = cpu->peek32(0);
    callX11(cpu, index);
}
void common_intIb(CPU* cpu){
    cpu->thread->signalIllegalInstruction(5);// 5=ILL_PRVOPC  // :TODO: just a guess
}
void common_int3(CPU* cpu) {
    cpu->thread->signalTrap(1);// 1=TRAP_BRKPT
}
void common_ud2(CPU* cpu) {
    cpu->thread->signalIllegalInstruction(5);// 5=ILL_PRVOPC  // :TODO: just a guess
}
void common_cpuid(CPU* cpu){
    cpu->cpuid();
}
void common_larr16r16(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->reg[dstReg].u16 = cpu->lar(cpu->reg[srcReg].u16, cpu->reg[dstReg].u16);
}
void common_larr16e16(CPU* cpu, U32 reg, U32 address){
    cpu->reg[reg].u16 = cpu->lar(cpu->memory->readw(address), cpu->reg[reg].u16);
}

U32 common_lar(CPU* cpu, U32 selector, U32 ar) {
    return cpu->lar(selector, ar);
}

void common_lslr16r16(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->reg[dstReg].u16 = cpu->lsl(cpu->reg[srcReg].u16, cpu->reg[dstReg].u16);
}
void common_lslr16e16(CPU* cpu, U32 reg, U32 address){
    cpu->reg[reg].u16 = cpu->lsl(cpu->memory->readw(address), cpu->reg[reg].u16);
}

void common_lslr32r32(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->reg[dstReg].u32 = cpu->lsl(cpu->reg[srcReg].u32, cpu->reg[dstReg].u32);
}
void common_lslr32e32(CPU* cpu, U32 reg, U32 address){
    cpu->reg[reg].u32 = cpu->lsl(cpu->memory->readw(address), cpu->reg[reg].u32); // intentional 16-bit read
}

U32 common_lsl(CPU* cpu, U32 selector, U32 limit) {
    return cpu->lsl(selector, limit);
}

void common_verre16(CPU* cpu, U32 address){
    cpu->verr(cpu->memory->readw(address));
}
void common_verwe16(CPU* cpu, U32 address){
    cpu->verw(cpu->memory->readw(address));
}

void common_verr(CPU* cpu, U32 selector){
    cpu->verr(selector);
}
void common_verw(CPU* cpu, U32 selector){
    cpu->verw(selector);
}

void common_cmpxchg8b(CPU* cpu, U32 address){
    U64 value1 = ((U64)EDX) << 32 | EAX;
    U64 value2 = cpu->memory->readq(address);
    cpu->fillFlags();
    if (value1 == value2) {
        cpu->addZF();
        cpu->memory->writed(address, EBX);
        cpu->memory->writed(address + 4, ECX);
    } else {
        cpu->removeZF();
        EDX = (U32)(value2 >> 32);
        EAX = (U32)value2;
    }
}

void common_fxsave(CPU* cpu, U32 address) {
    cpu->memory->writew(address + 0, (U16)cpu->fpu.CW());
    cpu->memory->writew(address + 2, (U16)cpu->fpu.SW());
    cpu->memory->writeb(address + 4, cpu->fpu.GetAbridgedTag(cpu));
    cpu->memory->writeb(address + 5, 0);
    cpu->memory->writew(address + 6, 0); // fop
    cpu->memory->writed(address + 8, 0); // fip
    cpu->memory->writew(address + 12, 0); // f cs
    cpu->memory->writew(address + 14, 0); // reserved
    cpu->memory->writed(address + 16, 0); // f dp
    cpu->memory->writew(address + 20, 0); // f ds
    cpu->memory->writew(address + 22, 0); // reserved
    cpu->memory->writed(address + 24, 0x1F80); // mxcsr
    cpu->memory->writed(address + 28, 0xFFFF); // mxcsr mask

    for (int i=0;i<8;i++) {
        U32 index = (i - cpu->fpu.GetTop()) & 7;
        cpu->fpu.ST80(cpu, address+32+index*16, i);
    }
    for (int i=0;i<8;i++) {
        cpu->memory->writeq(address+160+i*16, cpu->xmm[i].pi.u64[0]);
        cpu->memory->writeq(address+168+i*16, cpu->xmm[i].pi.u64[1]);
    }
}

void common_fxrstor(CPU* cpu, U32 address) {
    cpu->fpu.SetCW(cpu->memory->readw(address));
    cpu->fpu.SetSW(cpu->memory->readw(address+2));
    cpu->fpu.SetTagFromAbridged(cpu->memory->readb(address+4));
    for (int i=0;i<8;i++) {
        U32 index = (i - cpu->fpu.GetTop()) & 7;
        cpu->fpu.LD80(i, cpu->memory->readq(address + 32 + index * 16), cpu->memory->readw(address + 40 + index * 16));
    }
    for (int i=0;i<8;i++) {
        cpu->xmm[i].pi.u64[0] = cpu->memory->readq(address+160+i*16);
        cpu->xmm[i].pi.u64[1] = cpu->memory->readq(address+168+i*16);
    }
}

void common_xsave(CPU* cpu, U32 address) {
    kpanic("xsave not implemented");
}

void common_xrstor(CPU* cpu, U32 address) {
    kpanic("xrstore not implemented");
}
