/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_X86_UTIL_H__
#define __TEST_X86_UTIL_H__

#include "testAsmJit.h"

namespace TestX86 {

U32 widthMask(int width);
U64 widthMask64(int width);
U32 signBit(int width);
S64 signExtend(U32 value, int width);

asmjit::x86::Gp reg8(int reg);
asmjit::x86::Gp reg16(int reg);
asmjit::x86::Gp reg32(int reg);
asmjit::x86::Gp regForWidth(int reg, int width);

asmjit::x86::Mem memPtr(U32 address, int width);
asmjit::x86::Mem memPtr(asmjit::x86::Gp base, S32 disp, int width);
asmjit::x86::Mem memPtr(asmjit::x86::Gp base, asmjit::x86::Gp index, int shift, S32 disp, int width);

int physicalReg8(int reg);
void applyRegValue(U32* regs, int reg, int width, U32 value);
void setRegValue(CPU* cpu, int reg, int width, U32 value);
U32 getRegValue(CPU* cpu, int reg, int width);
void writeRegs(CPU* cpu, const U32* regs);
void verifyRegisters(CPU* cpu, const U32* expectedRegs, const char* name);
U32 actualFlags(CPU* cpu, bool includeDirectionFlag = false);

}

#endif
