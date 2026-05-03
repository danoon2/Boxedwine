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

#include "testJmp.h"
#include "testCPU.h"
#include "testX86Util.h"

#define cpu (testContext().cpu)
#define memory (testContext().memory)
#define pushCode8 testPushCode8
#define pushCode16 testPushCode16
#define pushCode32 testPushCode32
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

using namespace TestX86;

constexpr U32 REG_GUARD = 0xA55A0000;
constexpr U32 JUMP_FLAG_MASK = CF | PF | AF | ZF | SF | OF;
constexpr U32 STACK_GUARD = 0xDEADBEEF;
constexpr U32 INITIAL_FLAGS = CF | PF | AF | ZF | SF | OF | DF;
constexpr U32 STACK_START = 4096;
constexpr U32 TARGET_OFFSET = 0x140;
constexpr U32 MEM_TARGET_OFFSET = 0x180;
constexpr U32 MEM_BASE = 0x3000;

enum RegId {
    R_AX,
    R_CX,
    R_DX,
    R_BX,
    R_SP,
    R_BP,
    R_SI,
    R_DI
};

struct CmpCase {
    U32 lhs;
    U32 rhs;
};

const CmpCase CMP_CASES[] = {
    {0, 0},
    {1, 0},
    {0, 1},
    {0xffffffff, 0},
    {0, 0xffffffff},
    {0xffffffff, 0xffffffff},
    {0x7fffffff, 0xffffffff},
    {0x80000000, 0x00001000},
    {0x00008000, 0x00001000}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

void beginInstruction(U32 flags, bool big) {
    newInstruction(flags);
    cpu->big = big;
}

void initRegisters(U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
}

void verifyRegisters(const U32* expectedRegs, const char* name) {
    for (int i = 0; i < 8; ++i) {
        if (cpu->reg[i].u32 != expectedRegs[i]) {
            failed("%s register value", name);
        }
    }
}

U32 actualFlags() {
    U32 flags = 0;
    if (cpu->getCF()) flags |= CF;
    if (cpu->getPF()) flags |= PF;
    if (cpu->getAF()) flags |= AF;
    if (cpu->getZF()) flags |= ZF;
    if (cpu->getSF()) flags |= SF;
    if (cpu->getOF()) flags |= OF;
    return flags;
}

void verifyFlags(U32 expectedFlags, const char* name) {
    if (((actualFlags() ^ expectedFlags) & JUMP_FLAG_MASK) != 0) {
        failed("%s flags", name);
    }
}

U32 parityFlag(U32 value) {
    U8 low = (U8)value;
    low ^= low >> 4;
    low &= 0x0f;
    return ((0x6996 >> low) & 1) == 0 ? PF : 0;
}

U32 flagsForCmp(U32 lhs, U32 rhs, int width) {
    U32 mask = widthMask(width);
    U32 sign = signBit(width);
    U32 result = (lhs - rhs) & mask;
    U32 flags = 0;

    lhs &= mask;
    rhs &= mask;
    if (lhs < rhs) flags |= CF;
    if (((lhs ^ rhs) & (lhs ^ result) & sign) != 0) flags |= OF;
    if (((lhs ^ rhs ^ result) & 0x10) != 0) flags |= AF;
    if (result == 0) flags |= ZF;
    if ((result & sign) != 0) flags |= SF;
    flags |= parityFlag(result);
    return flags;
}

bool conditionTaken(U8 opcode, U32 flags) {
    bool cf = (flags & CF) != 0;
    bool pf = (flags & PF) != 0;
    bool zf = (flags & ZF) != 0;
    bool sf = (flags & SF) != 0;
    bool of = (flags & OF) != 0;

    switch (opcode & 0x0f) {
    case 0x0: return of;
    case 0x1: return !of;
    case 0x2: return cf;
    case 0x3: return !cf;
    case 0x4: return zf;
    case 0x5: return !zf;
    case 0x6: return cf || zf;
    case 0x7: return !cf && !zf;
    case 0x8: return sf;
    case 0x9: return !sf;
    case 0xa: return pf;
    case 0xb: return !pf;
    case 0xc: return sf != of;
    case 0xd: return sf == of;
    case 0xe: return zf || (sf != of);
    default: return !zf && (sf == of);
    }
}

void emitShortConditionalJump(U8 opcode) {
    pushCode8(opcode);
    pushCode8(4);      // jump over mov al,2 and the short jmp
    pushCode8(0xb0);   // mov al,2
    pushCode8(2);
    pushCode8(0xeb);   // jmp over mov al,3
    pushCode8(2);
    pushCode8(0xb0);   // mov al,3
    pushCode8(3);
}

void emitCmpEcXImm(U32 rhs, bool big) {
    pushCode8(0x81);
    pushCode8(0xf9);
    if (big) {
        pushCode32(rhs);
    } else {
        pushCode16((U16)rhs);
    }
}

void emitMovAccumulator(U32 value, bool big) {
    pushCode8(0xb8);
    if (big) {
        pushCode32(value);
    } else {
        pushCode16((U16)value);
    }
}

void emitJmpRel(S32 displacement, bool big) {
    pushCode8(0xe9);
    if (big) {
        pushCode32(displacement);
    } else {
        pushCode16((U16)displacement);
    }
}

void emitJmpReg(int reg) {
    pushCode8(0xff);
    pushCode8(0xe0 | reg);
}

void emitJmpMem(U32 offset, bool big) {
    pushCode8(0xff);
    if (big) {
        pushCode8(0x25);
        pushCode32(offset);
    } else {
        pushCode8(0x26);
        pushCode16((U16)offset);
    }
}

void emitJmpFarMem(U32 offset, bool big) {
    pushCode8(0xff);
    if (big) {
        pushCode8(0x2d);
        pushCode32(offset);
    } else {
        pushCode8(0x2e);
        pushCode16((U16)offset);
    }
}

U32 stackGuardAddress() {
    return cpu->seg[SS].address + 4092;
}

U32 heapAddress(U32 offset) {
    return cpu->seg[DS].address + offset;
}

U32 currentAccumulator(bool big) {
    return big ? cpu->reg[R_AX].u32 : cpu->reg[R_AX].u16;
}

void runForwardDirectJmpCase(bool big, const char* name) {
    U32 expectedRegs[8];
    U32 expectedFlags = INITIAL_FLAGS;
    S32 displacement = big ? 5 : 3;

    beginInstruction(INITIAL_FLAGS, big);
    emitMovAccumulator(0x10, big);
    emitJmpRel(displacement, big);
    emitMovAccumulator(0xffff, big);
    emitMovAccumulator(0x11, big);
    initRegisters(expectedRegs);
    expectedRegs[R_AX] = 0x11;
    cpu->reg[R_AX].u32 = 0;
    cpu->reg[R_SP].u32 = 4096;
    expectedRegs[R_SP] = 4096;
    memory->writed(stackGuardAddress(), STACK_GUARD);

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyFlags(expectedFlags, name);
    if (memory->readd(stackGuardAddress()) != STACK_GUARD) {
        failed("%s stack guard", name);
    }
    if (currentAccumulator(big) != 0x11) {
        failed("%s accumulator", name);
    }
}

void runBackwardDirectJmpCase(bool big, const char* name) {
    U32 expectedRegs[8];
    U32 expectedFlags = INITIAL_FLAGS;
    S32 firstDisp = big ? 10 : 6;
    S32 skipEndDisp = big ? 10 : 6;
    S32 backwardDisp = big ? -15 : -9;

    beginInstruction(INITIAL_FLAGS, big);
    emitJmpRel(firstDisp, big);
    emitMovAccumulator(0x21, big);
    emitJmpRel(skipEndDisp, big);
    emitJmpRel(backwardDisp, big);
    emitMovAccumulator(0xffff, big);
    initRegisters(expectedRegs);
    expectedRegs[R_AX] = 0x21;
    cpu->reg[R_AX].u32 = 0;
    cpu->reg[R_SP].u32 = 4096;
    expectedRegs[R_SP] = 4096;
    memory->writed(stackGuardAddress(), STACK_GUARD);

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyFlags(expectedFlags, name);
    if (memory->readd(stackGuardAddress()) != STACK_GUARD) {
        failed("%s stack guard", name);
    }
    if (currentAccumulator(big) != 0x21) {
        failed("%s accumulator", name);
    }
}

void runDirectJmpCases(bool big, const char* name) {
    runForwardDirectJmpCase(big, name);
    runBackwardDirectJmpCase(big, name);
}

void emitPaddingTo(U32 offset) {
    while (testContext().codeIp < TEST_CODE_ADDRESS + offset) {
        pushCode8(0x90);
    }
}

void emitNearJmpTarget(U32 offset, bool big, U32 value) {
    emitPaddingTo(offset);
    emitMovAccumulator(value, big);
}

void runNearJmpRegCase(int reg, bool big, const char* name) {
    U32 expectedRegs[8];
    int width = big ? 32 : 16;
    U32 targetOffset = TARGET_OFFSET + reg * 0x10;

    beginInstruction(INITIAL_FLAGS, big);
    emitJmpReg(reg);
    emitNearJmpTarget(targetOffset, big, 0x33);
    initRegisters(expectedRegs);
    cpu->reg[R_AX].u32 = 0;
    expectedRegs[R_AX] = 0x33;
    cpu->reg[R_SP].u32 = STACK_START;
    expectedRegs[R_SP] = STACK_START;
    if (width == 16) {
        cpu->reg[reg].u32 = (cpu->reg[reg].u32 & 0xffff0000) | (targetOffset & 0xffff);
        expectedRegs[reg] = cpu->reg[reg].u32;
    } else {
        cpu->reg[reg].u32 = targetOffset;
        expectedRegs[reg] = targetOffset;
    }
    if (reg == R_AX) {
        expectedRegs[R_AX] = 0x33;
    } else if (reg == R_SP) {
        expectedRegs[R_SP] = targetOffset;
    }
    memory->writed(stackGuardAddress(), STACK_GUARD);

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyFlags(INITIAL_FLAGS, name);
    if (memory->readd(stackGuardAddress()) != STACK_GUARD) {
        failed("%s stack guard", name);
    }
}

void runNearJmpMemCase(bool big, const char* name) {
    U32 expectedRegs[8];
    int width = big ? 32 : 16;

    beginInstruction(INITIAL_FLAGS, big);
    emitJmpMem(MEM_BASE, big);
    emitNearJmpTarget(MEM_TARGET_OFFSET, big, 0x44);
    initRegisters(expectedRegs);
    cpu->reg[R_AX].u32 = 0;
    cpu->reg[R_SP].u32 = STACK_START;
    expectedRegs[R_AX] = 0x44;
    expectedRegs[R_SP] = STACK_START;
    if (width == 16) {
        memory->writew(heapAddress(MEM_BASE), MEM_TARGET_OFFSET);
    } else {
        memory->writed(heapAddress(MEM_BASE), MEM_TARGET_OFFSET);
    }
    memory->writed(stackGuardAddress(), STACK_GUARD);

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyFlags(INITIAL_FLAGS, name);
    if (memory->readd(stackGuardAddress()) != STACK_GUARD) {
        failed("%s stack guard", name);
    }
}

void runFarJmpMemCase(bool startBig, U16 targetSelector, bool targetBig, const char* name) {
    U32 expectedRegs[8];

    beginInstruction(INITIAL_FLAGS, startBig);
    emitJmpFarMem(MEM_BASE, startBig);
    emitNearJmpTarget(MEM_TARGET_OFFSET, targetBig, 0x55);
    initRegisters(expectedRegs);
    cpu->reg[R_AX].u32 = 0;
    cpu->reg[R_SP].u32 = STACK_START;
    expectedRegs[R_AX] = 0x55;
    expectedRegs[R_SP] = STACK_START;
    if (startBig) {
        memory->writed(heapAddress(MEM_BASE), MEM_TARGET_OFFSET);
        memory->writed(heapAddress(MEM_BASE + 4), targetSelector);
    } else {
        memory->writew(heapAddress(MEM_BASE), MEM_TARGET_OFFSET);
        memory->writew(heapAddress(MEM_BASE + 2), targetSelector);
    }
    memory->writed(stackGuardAddress(), STACK_GUARD);

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyFlags(INITIAL_FLAGS, name);
    if (memory->readd(stackGuardAddress()) != STACK_GUARD) {
        failed("%s stack guard", name);
    }
    if (cpu->seg[CS].value != targetSelector || (cpu->big != 0) != targetBig) {
        failed("%s target segment", name);
    }
}

void runNearJmpCases(bool big, const char* name) {
    for (int reg = 0; reg < 8; ++reg) {
        runNearJmpRegCase(reg, big, name);
    }
    runNearJmpMemCase(big, name);
}

void runFarJmpCases(bool startBig, const char* name) {
    runFarJmpMemCase(startBig, TEST_CODE_SEG_16, false, name);
    runFarJmpMemCase(startBig, TEST_CODE_SEG, true, name);
}

void runDirectFlagCase(U8 opcode, bool big, U32 flags, const char* name) {
    U32 expectedRegs[8];
    bool jumped = conditionTaken(opcode, flags);

    beginInstruction(flags, big);
    emitShortConditionalJump(opcode);
    initRegisters(expectedRegs);
    expectedRegs[R_AX] = (expectedRegs[R_AX] & 0xffffff00) | (jumped ? 3 : 2);

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyFlags(flags, name);
}

void runCmpCase(U8 opcode, bool big, const CmpCase& data, const char* name) {
    U32 expectedRegs[8];
    int width = big ? 32 : 16;
    U32 expectedFlags = flagsForCmp(data.lhs, data.rhs, width);
    bool jumped = conditionTaken(opcode, expectedFlags);

    beginInstruction(0, big);
    emitCmpEcXImm(data.rhs, big);
    emitShortConditionalJump(opcode);
    initRegisters(expectedRegs);
    if (big) {
        cpu->reg[R_CX].u32 = data.lhs;
    } else {
        cpu->reg[R_CX].u32 = (cpu->reg[R_CX].u32 & 0xffff0000) | (data.lhs & 0xffff);
    }
    expectedRegs[R_CX] = cpu->reg[R_CX].u32;
    expectedRegs[R_AX] = (expectedRegs[R_AX] & 0xffffff00) | (jumped ? 3 : 2);

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyFlags(expectedFlags, name);
}

void runJccCases(U8 opcode, bool big, const char* name) {
    for (U32 flags = 0; flags < 64; ++flags) {
        U32 cpuFlags = 0;
        if ((flags & 0x01) != 0) cpuFlags |= CF;
        if ((flags & 0x02) != 0) cpuFlags |= PF;
        if ((flags & 0x04) != 0) cpuFlags |= AF;
        if ((flags & 0x08) != 0) cpuFlags |= ZF;
        if ((flags & 0x10) != 0) cpuFlags |= SF;
        if ((flags & 0x20) != 0) cpuFlags |= OF;
        runDirectFlagCase(opcode, big, cpuFlags, name);
    }

    for (size_t i = 0; i < caseCount(CMP_CASES); ++i) {
        runCmpCase(opcode, big, CMP_CASES[i], name);
    }
}

} // namespace

void testJO_0x070() { runJccCases(0x70, false, "jo rel8 070"); }
void testJO_0x270() { runJccCases(0x70, true, "jo rel8 270"); }
void testJNO_0x071() { runJccCases(0x71, false, "jno rel8 071"); }
void testJNO_0x271() { runJccCases(0x71, true, "jno rel8 271"); }
void testJB_0x072() { runJccCases(0x72, false, "jb rel8 072"); }
void testJB_0x272() { runJccCases(0x72, true, "jb rel8 272"); }
void testJNB_0x073() { runJccCases(0x73, false, "jnb rel8 073"); }
void testJNB_0x273() { runJccCases(0x73, true, "jnb rel8 273"); }
void testJZ_0x074() { runJccCases(0x74, false, "jz rel8 074"); }
void testJZ_0x274() { runJccCases(0x74, true, "jz rel8 274"); }
void testJNZ_0x075() { runJccCases(0x75, false, "jnz rel8 075"); }
void testJNZ_0x275() { runJccCases(0x75, true, "jnz rel8 275"); }
void testJBE_0x076() { runJccCases(0x76, false, "jbe rel8 076"); }
void testJBE_0x276() { runJccCases(0x76, true, "jbe rel8 276"); }
void testJNBE_0x077() { runJccCases(0x77, false, "jnbe rel8 077"); }
void testJNBE_0x277() { runJccCases(0x77, true, "jnbe rel8 277"); }
void testJS_0x078() { runJccCases(0x78, false, "js rel8 078"); }
void testJS_0x278() { runJccCases(0x78, true, "js rel8 278"); }
void testJNS_0x079() { runJccCases(0x79, false, "jns rel8 079"); }
void testJNS_0x279() { runJccCases(0x79, true, "jns rel8 279"); }
void testJP_0x07a() { runJccCases(0x7a, false, "jp rel8 07a"); }
void testJP_0x27a() { runJccCases(0x7a, true, "jp rel8 27a"); }
void testJNP_0x07b() { runJccCases(0x7b, false, "jnp rel8 07b"); }
void testJNP_0x27b() { runJccCases(0x7b, true, "jnp rel8 27b"); }
void testJL_0x07c() { runJccCases(0x7c, false, "jl rel8 07c"); }
void testJL_0x27c() { runJccCases(0x7c, true, "jl rel8 27c"); }
void testJNL_0x07d() { runJccCases(0x7d, false, "jnl rel8 07d"); }
void testJNL_0x27d() { runJccCases(0x7d, true, "jnl rel8 27d"); }
void testJLE_0x07e() { runJccCases(0x7e, false, "jle rel8 07e"); }
void testJLE_0x27e() { runJccCases(0x7e, true, "jle rel8 27e"); }
void testJNLE_0x07f() { runJccCases(0x7f, false, "jnle rel8 07f"); }
void testJNLE_0x27f() { runJccCases(0x7f, true, "jnle rel8 27f"); }
void testJmpJw_0x0e9() { runDirectJmpCases(false, "jmp rel16 0e9"); }
void testJmpJd_0x2e9() { runDirectJmpCases(true, "jmp rel32 2e9"); }
void testJmpE16_0x0ff() { runNearJmpCases(false, "jmp e16 0ff"); }
void testJmpE32_0x2ff() { runNearJmpCases(true, "jmp e32 2ff"); }
void testJmpFarE16_0x0ff() { runFarJmpCases(false, "jmp far e16 0ff"); }
void testJmpFarE32_0x2ff() { runFarJmpCases(true, "jmp far e32 2ff"); }

#endif
