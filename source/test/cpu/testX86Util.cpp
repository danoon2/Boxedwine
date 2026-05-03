/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#ifdef __TEST

#include "testX86Util.h"
#include "testCPU.h"

namespace TestX86 {

U32 widthMask(int width) {
    if (width == 8) return 0xff;
    if (width == 16) return 0xffff;
    return 0xffffffff;
}

U64 widthMask64(int width) {
    if (width == 8) return 0xff;
    if (width == 16) return 0xffff;
    return 0xffffffffULL;
}

U32 signBit(int width) {
    return 1u << (width - 1);
}

S64 signExtend(U32 value, int width) {
    if (width == 8) return (S8)value;
    if (width == 16) return (S16)value;
    return (S32)value;
}

asmjit::x86::Gp reg8(int reg) {
    static const asmjit::x86::Gp regs[] = {
        asmjit::x86::al,
        asmjit::x86::cl,
        asmjit::x86::dl,
        asmjit::x86::bl,
        asmjit::x86::ah,
        asmjit::x86::ch,
        asmjit::x86::dh,
        asmjit::x86::bh
    };
    return regs[reg];
}

asmjit::x86::Gp reg16(int reg) {
    static const asmjit::x86::Gp regs[] = {
        asmjit::x86::ax,
        asmjit::x86::cx,
        asmjit::x86::dx,
        asmjit::x86::bx,
        asmjit::x86::sp,
        asmjit::x86::bp,
        asmjit::x86::si,
        asmjit::x86::di
    };
    return regs[reg];
}

asmjit::x86::Gp reg32(int reg) {
    static const asmjit::x86::Gp regs[] = {
        asmjit::x86::eax,
        asmjit::x86::ecx,
        asmjit::x86::edx,
        asmjit::x86::ebx,
        asmjit::x86::esp,
        asmjit::x86::ebp,
        asmjit::x86::esi,
        asmjit::x86::edi
    };
    return regs[reg];
}

asmjit::x86::Gp regForWidth(int reg, int width) {
    if (width == 8) return reg8(reg);
    if (width == 16) return reg16(reg);
    return reg32(reg);
}

asmjit::x86::Mem memPtr(U32 address, int width) {
    if (width == 8) return asmjit::x86::byte_ptr(address);
    if (width == 16) return asmjit::x86::word_ptr(address);
    return asmjit::x86::dword_ptr(address);
}

asmjit::x86::Mem memPtr(asmjit::x86::Gp base, S32 disp, int width) {
    if (width == 8) return asmjit::x86::byte_ptr(base, disp);
    if (width == 16) return asmjit::x86::word_ptr(base, disp);
    return asmjit::x86::dword_ptr(base, disp);
}

asmjit::x86::Mem memPtr(asmjit::x86::Gp base, asmjit::x86::Gp index, int shift, S32 disp, int width) {
    if (width == 8) return asmjit::x86::byte_ptr(base, index, shift, disp);
    if (width == 16) return asmjit::x86::word_ptr(base, index, shift, disp);
    return asmjit::x86::dword_ptr(base, index, shift, disp);
}

int physicalReg8(int reg) {
    return reg & 3;
}

void applyRegValue(U32* regs, int reg, int width, U32 value) {
    value &= widthMask(width);
    if (width == 8) {
        int physical = physicalReg8(reg);
        U32 shift = reg >= 4 ? 8 : 0;
        regs[physical] = (regs[physical] & ~(0xffu << shift)) | (value << shift);
    } else if (width == 16) {
        regs[reg] = (regs[reg] & 0xffff0000) | value;
    } else {
        regs[reg] = value;
    }
}

void setRegValue(CPU* cpu, int reg, int width, U32 value) {
    value &= widthMask(width);
    if (width == 8) {
        *cpu->reg8[reg] = (U8)value;
    } else if (width == 16) {
        cpu->reg[reg].u16 = (U16)value;
    } else {
        cpu->reg[reg].u32 = value;
    }
}

U32 getRegValue(CPU* cpu, int reg, int width) {
    if (width == 8) return *cpu->reg8[reg];
    if (width == 16) return cpu->reg[reg].u16;
    return cpu->reg[reg].u32;
}

void writeRegs(CPU* cpu, const U32* regs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = regs[i];
    }
}

void verifyRegisters(CPU* cpu, const U32* expectedRegs, const char* name) {
    for (int i = 0; i < 8; ++i) {
        if (cpu->reg[i].u32 != expectedRegs[i]) {
            testFail("%s register value", name);
        }
    }
}

U32 actualFlags(CPU* cpu, bool includeDirectionFlag) {
    U32 flags = 0;
    if (cpu->getCF()) flags |= CF;
    if (cpu->getPF()) flags |= PF;
    if (cpu->getAF()) flags |= AF;
    if (cpu->getZF()) flags |= ZF;
    if (cpu->getSF()) flags |= SF;
    if (cpu->getOF()) flags |= OF;
    if (includeDirectionFlag && (cpu->flags & DF)) flags |= DF;
    return flags;
}

}

#endif
