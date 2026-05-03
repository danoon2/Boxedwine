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

#include "testLoop.h"
#include "testCPU.h"

#define cpu (testContext().cpu)
#define pushCode8 testPushCode8
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

constexpr U32 REG_GUARD = 0xA55A0000;
constexpr U32 LOOP_FLAG_MASK = CF | PF | AF | ZF | SF | OF | DF;

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

struct LoopCase {
    U32 ecx;
    U32 flags;
};

struct CmpLoopCase {
    U32 ecx;
    U32 eax;
    U32 expectedFlags;
};

struct JcxzCase {
    U32 ecx;
    U32 flags;
};

const LoopCase LOOP_CASES[] = {
    {0x00000000, 0},
    {0x00000000, ZF | CF | PF | AF | SF | OF | DF},
    {0x00000001, 0},
    {0x00000001, ZF | CF | PF | AF | SF | OF | DF},
    {0x00000002, 0},
    {0x00000002, ZF | CF | PF | AF | SF | OF | DF},
    {0x12340000, 0},
    {0x12340000, ZF | CF | PF | AF | SF | OF | DF},
    {0x12340001, 0},
    {0x12340001, ZF | CF | PF | AF | SF | OF | DF},
    {0xffffffff, 0},
    {0xffffffff, ZF | CF | PF | AF | SF | OF | DF}
};

const CmpLoopCase CMP_LOOP_CASES[] = {
    {0x00000002, 0x00000000, ZF | PF},
    {0x00000002, 0x00000001, 0},
    {0x00000001, 0x00000000, ZF | PF},
    {0x00000000, 0x00000001, 0}
};

const JcxzCase JCXZ_CASES[] = {
    {0x00000000, 0},
    {0x00000000, ZF | CF | PF | AF | SF | OF | DF},
    {0x00000001, 0},
    {0x00000001, ZF | CF | PF | AF | SF | OF | DF},
    {0x00010000, 0},
    {0x00010000, ZF | CF | PF | AF | SF | OF | DF},
    {0x12340000, 0},
    {0x12340000, ZF | CF | PF | AF | SF | OF | DF}
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

U32 currentFlags() {
    U32 flags = 0;
    if (cpu->getCF()) flags |= CF;
    if (cpu->getPF()) flags |= PF;
    if (cpu->getAF()) flags |= AF;
    if (cpu->getZF()) flags |= ZF;
    if (cpu->getSF()) flags |= SF;
    if (cpu->getOF()) flags |= OF;
    if (cpu->flags & DF) flags |= DF;
    return flags;
}

void verifyFlags(U32 expectedFlags, const char* name) {
    if (((currentFlags() ^ expectedFlags) & LOOP_FLAG_MASK) != 0) {
        failed("%s flags", name);
    }
}

U32 countValue(U32 ecx, bool big) {
    return big ? ecx : (ecx & 0xffff);
}

U32 decrementCount(U32 ecx, bool big) {
    if (big) {
        return ecx - 1;
    }
    return (ecx & 0xffff0000) | ((ecx - 1) & 0xffff);
}

bool loopTaken(U8 opcode, U32 countAfter, U32 flags, bool big) {
    bool countNotZero = countValue(countAfter, big) != 0;
    bool zf = (flags & ZF) != 0;

    if (opcode == 0xe0) {
        return countNotZero && !zf;
    }
    if (opcode == 0xe1) {
        return countNotZero && zf;
    }
    return countNotZero;
}

bool jcxzTaken(U32 ecx, bool big) {
    return countValue(ecx, big) == 0;
}

void emitTakenMarkerInstruction(U8 opcode) {
    pushCode8(opcode);
    pushCode8(4);      // jump over mov al,2 and the short jmp
    pushCode8(0xb0);   // mov al,2
    pushCode8(2);
    pushCode8(0xeb);   // jmp over mov al,3
    pushCode8(2);
    pushCode8(0xb0);   // mov al,3
    pushCode8(3);
}

void emitCmpAxZero() {
    pushCode8(0x83);
    pushCode8(0xf8);
    pushCode8(0);
}

void runLoopDirectFlagCase(U8 opcode, bool big, const LoopCase& data, const char* name) {
    U32 expectedRegs[8];
    U32 countAfter = decrementCount(data.ecx, big);
    bool taken = loopTaken(opcode, countAfter, data.flags, big);

    beginInstruction(data.flags, big);
    emitTakenMarkerInstruction(opcode);
    initRegisters(expectedRegs);
    cpu->reg[R_CX].u32 = data.ecx;
    expectedRegs[R_CX] = countAfter;
    expectedRegs[R_AX] = (expectedRegs[R_AX] & 0xffffff00) | (taken ? 3 : 2);

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyFlags(data.flags, name);
}

void runLoopCmpFlagCase(U8 opcode, bool big, const CmpLoopCase& data, const char* name) {
    U32 expectedRegs[8];
    U32 countAfter = decrementCount(data.ecx, big);
    bool taken = loopTaken(opcode, countAfter, data.expectedFlags, big);

    beginInstruction(0, big);
    emitCmpAxZero();
    emitTakenMarkerInstruction(opcode);
    initRegisters(expectedRegs);
    cpu->reg[R_AX].u32 = data.eax;
    cpu->reg[R_CX].u32 = data.ecx;
    expectedRegs[R_AX] = (data.eax & 0xffffff00) | (taken ? 3 : 2);
    expectedRegs[R_CX] = countAfter;

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyFlags(data.expectedFlags, name);
}

void runLoopCases(U8 opcode, bool big, const char* name) {
    for (size_t i = 0; i < caseCount(LOOP_CASES); ++i) {
        runLoopDirectFlagCase(opcode, big, LOOP_CASES[i], name);
    }

    if (opcode == 0xe0 || opcode == 0xe1) {
        for (size_t i = 0; i < caseCount(CMP_LOOP_CASES); ++i) {
            runLoopCmpFlagCase(opcode, big, CMP_LOOP_CASES[i], name);
        }
    }
}

void runJcxzCase(U8 opcode, bool big, const JcxzCase& data, const char* name) {
    U32 expectedRegs[8];
    bool taken = jcxzTaken(data.ecx, big);

    beginInstruction(data.flags, big);
    emitTakenMarkerInstruction(opcode);
    initRegisters(expectedRegs);
    cpu->reg[R_CX].u32 = data.ecx;
    expectedRegs[R_CX] = data.ecx;
    expectedRegs[R_AX] = (expectedRegs[R_AX] & 0xffffff00) | (taken ? 3 : 2);

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyFlags(data.flags, name);
}

void runJcxzCases(U8 opcode, bool big, const char* name) {
    for (size_t i = 0; i < caseCount(JCXZ_CASES); ++i) {
        runJcxzCase(opcode, big, JCXZ_CASES[i], name);
    }
}

} // namespace

void testLoopnz_0x0e0() { runLoopCases(0xe0, false, "loopnz rel8 0e0"); }
void testLoopnz_0x2e0() { runLoopCases(0xe0, true, "loopnz rel8 2e0"); }
void testLoopz_0x0e1() { runLoopCases(0xe1, false, "loopz rel8 0e1"); }
void testLoopz_0x2e1() { runLoopCases(0xe1, true, "loopz rel8 2e1"); }
void testLoop_0x0e2() { runLoopCases(0xe2, false, "loop rel8 0e2"); }
void testLoop_0x2e2() { runLoopCases(0xe2, true, "loop rel8 2e2"); }
void testJcxz_0x0e3() { runJcxzCases(0xe3, false, "jcxz rel8 0e3"); }
void testJecxz_0x2e3() { runJcxzCases(0xe3, true, "jecxz rel8 2e3"); }

#endif
