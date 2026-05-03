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

#include "testCMov.h"
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

constexpr U32 REG_GUARD = 0xa55a0000;
constexpr U32 DEST_VALUE = 0x33334444;
constexpr U32 SRC_VALUE = 0x11112222;
constexpr U32 MEM_SRC = 0x0200;
constexpr U32 MEM_CMP = 0x0240;
constexpr U32 FLAGS_MASK = CF | PF | AF | ZF | SF | OF;

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
    {0x00008000, 0x00001000},
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
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

void emitDirectAddressModRM(int regField, U32 address, bool big) {
    if (big) {
        pushCode8(0x05 | (regField << 3));
        pushCode32(address);
    } else {
        pushCode8(0x06 | (regField << 3));
        pushCode16((U16)address);
    }
}

void emitCmpMemImm(U32 address, U32 rhs, bool big) {
    pushCode8(0x81);
    emitDirectAddressModRM(7, address, big);
    if (big) {
        pushCode32(rhs);
    } else {
        pushCode16((U16)rhs);
    }
}

void emitCmovRegReg(U8 opcode, int dst, int src) {
    pushCode8(0x0f);
    pushCode8(opcode);
    pushCode8(0xc0 | (dst << 3) | src);
}

void emitCmovRegMem(U8 opcode, int dst, U32 address, bool big) {
    pushCode8(0x0f);
    pushCode8(opcode);
    emitDirectAddressModRM(dst, address, big);
}

void emitTrailingCmp() {
    pushCode8(0x83);
    pushCode8(0xf8);
    pushCode8(0);
}

void initRegisters(U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
}

void verifyFlags(U32 expectedFlags, const char* name) {
    if (((actualFlags(cpu) ^ expectedFlags) & FLAGS_MASK) != 0) {
        failed("%s flags", name);
    }
}

void applyCmovResult(U32* expectedRegs, int dst, U32 sourceValue, bool taken, int width) {
    if (taken) {
        applyRegValue(expectedRegs, dst, width, sourceValue);
    }
}

void runRegCase(U8 opcode, bool big, int dst, int src, U32 flags, bool assertFlags, bool trailingCmp, const char* name) {
    U32 expectedRegs[8];
    int width = big ? 32 : 16;
    bool taken = conditionTaken(opcode, flags);

    newInstruction(flags);
    cpu->big = big;
    emitCmovRegReg(opcode, dst, src);
    if (trailingCmp) {
        emitTrailingCmp();
    }
    initRegisters(expectedRegs);
    cpu->reg[dst].u32 = DEST_VALUE;
    cpu->reg[src].u32 = SRC_VALUE;
    expectedRegs[dst] = cpu->reg[dst].u32;
    expectedRegs[src] = cpu->reg[src].u32;
    applyCmovResult(expectedRegs, dst, cpu->reg[src].u32, taken, width);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (assertFlags) {
        verifyFlags(flags, name);
    }
}

void runMemCase(U8 opcode, bool big, int dst, U32 flags, bool assertFlags, bool trailingCmp, const char* name) {
    U32 expectedRegs[8];
    int width = big ? 32 : 16;
    bool taken = conditionTaken(opcode, flags);

    newInstruction(flags);
    cpu->big = big;
    emitCmovRegMem(opcode, dst, MEM_SRC, big);
    if (trailingCmp) {
        emitTrailingCmp();
    }
    initRegisters(expectedRegs);
    cpu->reg[dst].u32 = DEST_VALUE;
    expectedRegs[dst] = DEST_VALUE;
    memory->writed(TEST_HEAP_ADDRESS + MEM_SRC, SRC_VALUE);
    applyCmovResult(expectedRegs, dst, SRC_VALUE, taken, width);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (memory->readd(TEST_HEAP_ADDRESS + MEM_SRC) != SRC_VALUE) {
        failed("%s memory source", name);
    }
    if (assertFlags) {
        verifyFlags(flags, name);
    }
}

void runCmpRegCase(U8 opcode, bool big, int dst, int src, const CmpCase& data, bool trailingCmp, const char* name) {
    U32 expectedRegs[8];
    int width = big ? 32 : 16;
    U32 expectedFlags = flagsForCmp(data.lhs, data.rhs, width);
    bool taken = conditionTaken(opcode, expectedFlags);

    newInstruction(0);
    cpu->big = big;
    emitCmpMemImm(MEM_CMP, data.rhs, big);
    emitCmovRegReg(opcode, dst, src);
    if (trailingCmp) {
        emitTrailingCmp();
    }
    initRegisters(expectedRegs);
    cpu->reg[dst].u32 = DEST_VALUE;
    cpu->reg[src].u32 = SRC_VALUE;
    expectedRegs[dst] = cpu->reg[dst].u32;
    expectedRegs[src] = cpu->reg[src].u32;
    memory->writed(TEST_HEAP_ADDRESS + MEM_CMP, data.lhs);
    applyCmovResult(expectedRegs, dst, cpu->reg[src].u32, taken, width);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (!trailingCmp) {
        verifyFlags(expectedFlags, name);
    }
}

void runCmpMemCase(U8 opcode, bool big, int dst, const CmpCase& data, bool trailingCmp, const char* name) {
    U32 expectedRegs[8];
    int width = big ? 32 : 16;
    U32 expectedFlags = flagsForCmp(data.lhs, data.rhs, width);
    bool taken = conditionTaken(opcode, expectedFlags);

    newInstruction(0);
    cpu->big = big;
    emitCmpMemImm(MEM_CMP, data.rhs, big);
    emitCmovRegMem(opcode, dst, MEM_SRC, big);
    if (trailingCmp) {
        emitTrailingCmp();
    }
    initRegisters(expectedRegs);
    cpu->reg[dst].u32 = DEST_VALUE;
    expectedRegs[dst] = DEST_VALUE;
    memory->writed(TEST_HEAP_ADDRESS + MEM_CMP, data.lhs);
    memory->writed(TEST_HEAP_ADDRESS + MEM_SRC, SRC_VALUE);
    applyCmovResult(expectedRegs, dst, SRC_VALUE, taken, width);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (memory->readd(TEST_HEAP_ADDRESS + MEM_SRC) != SRC_VALUE) {
        failed("%s memory source", name);
    }
    if (!trailingCmp) {
        verifyFlags(expectedFlags, name);
    }
}

void runCmovOpcode(U8 opcode, bool big, const char* name) {
    for (U32 bits = 0; bits < 64; ++bits) {
        U32 flags = 0;
        if ((bits & 0x01) != 0) flags |= CF;
        if ((bits & 0x02) != 0) flags |= PF;
        if ((bits & 0x04) != 0) flags |= AF;
        if ((bits & 0x08) != 0) flags |= ZF;
        if ((bits & 0x10) != 0) flags |= SF;
        if ((bits & 0x20) != 0) flags |= OF;
        for (int dst = 0; dst < 8; ++dst) {
            for (int src = 0; src < 8; ++src) {
                runRegCase(opcode, big, dst, src, flags, true, false, name);
            }
            runMemCase(opcode, big, dst, flags, true, false, name);
        }
    }

    for (size_t i = 0; i < caseCount(CMP_CASES); ++i) {
        for (int dst = 0; dst < 8; ++dst) {
            for (int src = 0; src < 8; ++src) {
                runCmpRegCase(opcode, big, dst, src, CMP_CASES[i], false, name);
                runCmpRegCase(opcode, big, dst, src, CMP_CASES[i], true, name);
            }
            runCmpMemCase(opcode, big, dst, CMP_CASES[i], false, name);
            runCmpMemCase(opcode, big, dst, CMP_CASES[i], true, name);
        }
    }
}

void runCmovWidth(bool big) {
    for (U8 opcode = 0x40; opcode <= 0x4f; ++opcode) {
        runCmovOpcode(opcode, big, big ? "cmov r32/e32" : "cmov r16/e16");
    }
}

} // namespace

void testCmovR16E16_0x140_0x14f() {
    runCmovWidth(false);
}

void testCmovR32E32_0x340_0x34f() {
    runCmovWidth(true);
}

#endif
