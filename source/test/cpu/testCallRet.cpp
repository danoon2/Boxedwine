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

#include "testCallRet.h"
#include "testCPU.h"

#define cpu (testContext().cpu)
#define memory (testContext().memory)
#define pushCode8 testPushCode8
#define pushCode16 testPushCode16
#define pushCode32 testPushCode32
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

constexpr U32 REG_GUARD = 0xA55A0000;
constexpr U32 INITIAL_FLAGS = CF | PF | AF | ZF | SF | OF | DF;
constexpr U32 STACK_START = 0x1000;
constexpr U32 TARGET_OFFSET = 0x40;
constexpr U32 MEM_TARGET_OFFSET = 0x80;
constexpr U32 MEM_BASE = 0x3000;
constexpr U32 RESULT_OFFSET = 0x3800;
constexpr U8 RESULT_VALUE = 0x5a;
constexpr U16 GUARD16_BEFORE = 0xaaaa;
constexpr U16 GUARD16_AFTER = 0xbbbb;
constexpr U32 GUARD32_BEFORE = 0xaaaaaaaa;
constexpr U32 GUARD32_AFTER = 0xbbbbbbbb;

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

U32 stackAddress(U32 sp) {
    return cpu->seg[SS].address + sp;
}

U32 heapAddress(U32 offset) {
    return cpu->seg[DS].address + offset;
}

void beginInstruction(bool big) {
    newInstruction(INITIAL_FLAGS);
    cpu->big = big;
}

void initRegisters(U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    cpu->reg[R_SP].u32 = STACK_START;
    expectedRegs[R_SP] = STACK_START;
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
    if (cpu->flags & DF) flags |= DF;
    return flags;
}

void verifyFlagsUnchanged(const char* name) {
    if ((actualFlags() & INITIAL_FLAGS) != INITIAL_FLAGS) {
        failed("%s flags changed", name);
    }
}

void emitPaddingTo(U32 offset) {
    while (testContext().codeIp < TEST_CODE_ADDRESS + offset) {
        pushCode8(0x90);
    }
}

void emitTarget(U32 offset, bool big) {
    emitPaddingTo(offset);
    pushCode8(0xc6);
    if (big) {
        pushCode8(0x05);
        pushCode32(RESULT_OFFSET);
    } else {
        pushCode8(0x06);
        pushCode16((U16)RESULT_OFFSET);
    }
    pushCode8(RESULT_VALUE);
}

void emitCallRel(U32 targetOffset, bool big) {
    pushCode8(0xe8);
    if (big) {
        pushCode32(targetOffset - 5);
    } else {
        pushCode16((U16)(targetOffset - 3));
    }
}

void emitCallFar(U32 targetOffset, U16 selector, bool big) {
    pushCode8(0x9a);
    if (big) {
        pushCode32(targetOffset);
    } else {
        pushCode16((U16)targetOffset);
    }
    pushCode16(selector);
}

void emitCallReg(int reg) {
    pushCode8(0xff);
    pushCode8(0xd0 | reg);
}

void emitCallMem(U32 offset, bool big) {
    pushCode8(0xff);
    if (big) {
        pushCode8(0x15);
        pushCode32(offset);
    } else {
        pushCode8(0x16);
        pushCode16((U16)offset);
    }
}

void emitCallFarMem(U32 offset, bool big) {
    pushCode8(0xff);
    if (big) {
        pushCode8(0x1d);
        pushCode32(offset);
    } else {
        pushCode8(0x1e);
        pushCode16((U16)offset);
    }
}

void emitRet(bool withImmediate, U16 imm) {
    pushCode8(withImmediate ? 0xc2 : 0xc3);
    if (withImmediate) {
        pushCode16(imm);
    }
}

void writeFarCallStackGuard(int width, U32 sp) {
    if (width == 16) {
        memory->writew(stackAddress(sp - 6), GUARD16_BEFORE);
        memory->writew(stackAddress(sp - 4), 0xcccc);
        memory->writew(stackAddress(sp - 2), 0xdddd);
        memory->writew(stackAddress(sp), GUARD16_AFTER);
    } else {
        memory->writed(stackAddress(sp - 12), GUARD32_BEFORE);
        memory->writed(stackAddress(sp - 8), 0xcccccccc);
        memory->writed(stackAddress(sp - 4), 0xdddddddd);
        memory->writed(stackAddress(sp), GUARD32_AFTER);
    }
}

void writeCallStackGuard(int width, U32 sp) {
    if (width == 16) {
        memory->writew(stackAddress((sp - 4) & 0xffff), GUARD16_BEFORE);
        memory->writew(stackAddress((sp - 2) & 0xffff), 0xcccc);
        memory->writew(stackAddress(sp), GUARD16_AFTER);
    } else {
        memory->writed(stackAddress(sp - 8), GUARD32_BEFORE);
        memory->writed(stackAddress(sp - 4), 0xcccccccc);
        memory->writed(stackAddress(sp), GUARD32_AFTER);
    }
}

void verifyFarCallStack(int width, U32 expectedSp, U32 returnAddress, U16 returnSelector, const char* name) {
    if (width == 16) {
        if (cpu->reg[R_SP].u16 != (U16)expectedSp ||
                memory->readw(stackAddress(expectedSp)) != (U16)returnAddress ||
                memory->readw(stackAddress(expectedSp + 2)) != returnSelector ||
                memory->readw(stackAddress(expectedSp - 2)) != GUARD16_BEFORE ||
                memory->readw(stackAddress(expectedSp + 4)) != GUARD16_AFTER) {
            failed("%s far stack", name);
        }
    } else if (cpu->reg[R_SP].u32 != expectedSp ||
            memory->readd(stackAddress(expectedSp)) != returnAddress ||
            memory->readd(stackAddress(expectedSp + 4)) != returnSelector ||
            memory->readd(stackAddress(expectedSp - 4)) != GUARD32_BEFORE ||
            memory->readd(stackAddress(expectedSp + 8)) != GUARD32_AFTER) {
        failed("%s far stack", name);
    }
}

void verifyCallStack(int width, U32 expectedSp, U32 returnAddress, const char* name) {
    if (width == 16) {
        if (cpu->reg[R_SP].u16 != (U16)expectedSp ||
                memory->readw(stackAddress(expectedSp)) != (U16)returnAddress ||
                memory->readw(stackAddress((expectedSp - 2) & 0xffff)) != GUARD16_BEFORE ||
                memory->readw(stackAddress((expectedSp + 2) & 0xffff)) != GUARD16_AFTER) {
            failed("%s stack", name);
        }
    } else if (cpu->reg[R_SP].u32 != expectedSp ||
            memory->readd(stackAddress(expectedSp)) != returnAddress ||
            memory->readd(stackAddress(expectedSp - 4)) != GUARD32_BEFORE ||
            memory->readd(stackAddress(expectedSp + 4)) != GUARD32_AFTER) {
        failed("%s stack", name);
    }
}

void writeRetStack(int width, U32 sp, U32 returnOffset) {
    if (width == 16) {
        memory->writew(stackAddress((sp - 2) & 0xffff), GUARD16_BEFORE);
        memory->writew(stackAddress(sp), (U16)returnOffset);
        memory->writew(stackAddress((sp + 2) & 0xffff), GUARD16_AFTER);
    } else {
        memory->writed(stackAddress(sp - 4), GUARD32_BEFORE);
        memory->writed(stackAddress(sp), returnOffset);
        memory->writed(stackAddress(sp + 4), GUARD32_AFTER);
    }
}

void verifyRetStack(int width, U32 oldSp, U32 expectedSp, const char* name) {
    if (width == 16) {
        if (cpu->reg[R_SP].u16 != (U16)expectedSp ||
                memory->readw(stackAddress((oldSp - 2) & 0xffff)) != GUARD16_BEFORE ||
                memory->readw(stackAddress((oldSp + 2) & 0xffff)) != GUARD16_AFTER) {
            failed("%s stack", name);
        }
    } else if (cpu->reg[R_SP].u32 != expectedSp ||
            memory->readd(stackAddress(oldSp - 4)) != GUARD32_BEFORE ||
            memory->readd(stackAddress(oldSp + 4)) != GUARD32_AFTER) {
        failed("%s stack", name);
    }
}

void verifyTargetRan(const char* name) {
    if (memory->readb(heapAddress(RESULT_OFFSET)) != RESULT_VALUE) {
        failed("%s target", name);
    }
}

void setRegTarget(int reg, int width, U32 targetOffset) {
    if (width == 16) {
        cpu->reg[reg].u32 = (cpu->reg[reg].u32 & 0xffff0000) | (targetOffset & 0xffff);
    } else {
        cpu->reg[reg].u32 = targetOffset;
    }
}

void runCallRelCase(bool big, const char* name) {
    U32 expectedRegs[8];
    int width = big ? 32 : 16;
    U32 returnOffset = big ? 5 : 3;
    U32 expectedSp = STACK_START - width / 8;

    beginInstruction(big);
    emitCallRel(TARGET_OFFSET, big);
    emitTarget(TARGET_OFFSET, big);
    initRegisters(expectedRegs);
    writeCallStackGuard(width, STACK_START);
    expectedRegs[R_SP] = expectedSp;

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyCallStack(width, expectedSp, returnOffset, name);
    verifyTargetRan(name);
    verifyFlagsUnchanged(name);
}

void runCallFarCase(bool startBig, U16 targetSelector, bool targetBig, const char* name) {
    U32 expectedRegs[8];
    int width = startBig ? 32 : 16;
    U32 returnOffset = startBig ? 7 : 5;
    U32 expectedSp = STACK_START - (startBig ? 8 : 4);
    U16 returnSelector = targetSelector == TEST_CODE_SEG ? TEST_CODE_SEG_16 : TEST_CODE_SEG;

    beginInstruction(startBig);
    cpu->seg[CS].value = returnSelector;
    emitCallFar(TARGET_OFFSET, targetSelector, startBig);
    emitTarget(TARGET_OFFSET, targetBig);
    initRegisters(expectedRegs);
    writeFarCallStackGuard(width, STACK_START);
    expectedRegs[R_SP] = expectedSp;

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyFarCallStack(width, expectedSp, returnOffset, returnSelector, name);
    verifyTargetRan(name);
    if (cpu->seg[CS].value != targetSelector || (cpu->big != 0) != targetBig) {
        failed("%s target segment", name);
    }
    verifyFlagsUnchanged(name);
}

void runCallRegCase(int reg, bool big, const char* name) {
    U32 expectedRegs[8];
    int width = big ? 32 : 16;
    U32 targetOffset = TARGET_OFFSET + reg * 0x10;
    U32 stackStart = reg == R_SP ? targetOffset : STACK_START;
    U32 expectedSp = stackStart - width / 8;

    beginInstruction(big);
    emitCallReg(reg);
    emitTarget(targetOffset, big);
    initRegisters(expectedRegs);
    cpu->reg[R_SP].u32 = stackStart;
    expectedRegs[R_SP] = stackStart;
    setRegTarget(reg, width, targetOffset);
    expectedRegs[reg] = cpu->reg[reg].u32;
    writeCallStackGuard(width, stackStart);
    expectedRegs[R_SP] = big ? expectedSp : ((expectedRegs[R_SP] & 0xffff0000) | (expectedSp & 0xffff));

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyCallStack(width, expectedSp, 2, name);
    verifyTargetRan(name);
    verifyFlagsUnchanged(name);
}

void runCallMemCase(bool big, const char* name) {
    U32 expectedRegs[8];
    int width = big ? 32 : 16;
    U32 returnOffset = big ? 6 : 4;
    U32 expectedSp = STACK_START - width / 8;

    beginInstruction(big);
    emitCallMem(MEM_BASE, big);
    emitTarget(MEM_TARGET_OFFSET, big);
    initRegisters(expectedRegs);
    writeCallStackGuard(width, STACK_START);
    if (big) {
        memory->writed(heapAddress(MEM_BASE), MEM_TARGET_OFFSET);
    } else {
        memory->writew(heapAddress(MEM_BASE), MEM_TARGET_OFFSET);
    }
    expectedRegs[R_SP] = expectedSp;

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyCallStack(width, expectedSp, returnOffset, name);
    verifyTargetRan(name);
    verifyFlagsUnchanged(name);
}

void runCallFarMemCase(bool startBig, U16 targetSelector, bool targetBig, const char* name) {
    U32 expectedRegs[8];
    int width = startBig ? 32 : 16;
    U32 returnOffset = startBig ? 6 : 4;
    U32 expectedSp = STACK_START - (startBig ? 8 : 4);
    U16 returnSelector = targetSelector == TEST_CODE_SEG ? TEST_CODE_SEG_16 : TEST_CODE_SEG;

    beginInstruction(startBig);
    cpu->seg[CS].value = returnSelector;
    emitCallFarMem(MEM_BASE, startBig);
    emitTarget(MEM_TARGET_OFFSET, targetBig);
    initRegisters(expectedRegs);
    writeFarCallStackGuard(width, STACK_START);
    if (startBig) {
        memory->writed(heapAddress(MEM_BASE), MEM_TARGET_OFFSET);
        memory->writed(heapAddress(MEM_BASE + 4), targetSelector);
    } else {
        memory->writew(heapAddress(MEM_BASE), MEM_TARGET_OFFSET);
        memory->writew(heapAddress(MEM_BASE + 2), targetSelector);
    }
    expectedRegs[R_SP] = expectedSp;

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyFarCallStack(width, expectedSp, returnOffset, returnSelector, name);
    verifyTargetRan(name);
    if (cpu->seg[CS].value != targetSelector || (cpu->big != 0) != targetBig) {
        failed("%s target segment", name);
    }
    verifyFlagsUnchanged(name);
}

void runRetCase(bool big, bool withImmediate, U16 imm, const char* name) {
    U32 expectedRegs[8];
    int width = big ? 32 : 16;
    U32 itemSize = width / 8;
    U32 oldSp = STACK_START - itemSize - imm;
    U32 expectedSp = STACK_START;

    beginInstruction(big);
    emitRet(withImmediate, imm);
    emitTarget(TARGET_OFFSET, big);
    initRegisters(expectedRegs);
    cpu->reg[R_SP].u32 = oldSp;
    expectedRegs[R_SP] = expectedSp;
    writeRetStack(width, oldSp, TARGET_OFFSET);

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifyRetStack(width, oldSp, expectedSp, name);
    verifyTargetRan(name);
    verifyFlagsUnchanged(name);
}

void runCallRegCases(bool big, const char* name) {
    for (int reg = 0; reg < 8; ++reg) {
        runCallRegCase(reg, big, name);
    }
}

void runCallFarCases(bool startBig, const char* name) {
    runCallFarCase(startBig, TEST_CODE_SEG_16, false, name);
    runCallFarCase(startBig, TEST_CODE_SEG, true, name);
}

void runCallFarMemCases(bool startBig, const char* name) {
    runCallFarMemCase(startBig, TEST_CODE_SEG_16, false, name);
    runCallFarMemCase(startBig, TEST_CODE_SEG, true, name);
}

} // namespace

void testRetn16Iw_0x0c2() { runRetCase(false, true, 0x10, "ret iw 16 0c2"); }
void testRetn32Iw_0x2c2() { runRetCase(true, true, 0x10, "ret iw 32 2c2"); }
void testRetn16_0x0c3() { runRetCase(false, false, 0, "ret 16 0c3"); }
void testRetn32_0x2c3() { runRetCase(true, false, 0, "ret 32 2c3"); }
void testCallFar16_0x09a() { runCallFarCases(false, "call far 16 09a"); }
void testCallFar32_0x29a() { runCallFarCases(true, "call far 32 29a"); }
void testCallJw_0x0e8() { runCallRelCase(false, "call jw 0e8"); }
void testCallJd_0x2e8() { runCallRelCase(true, "call jd 2e8"); }
void testCallR16_0x0ff() { runCallRegCases(false, "call r16 0ff"); }
void testCallE16_0x0ff() { runCallMemCase(false, "call e16 0ff"); }
void testCallFarE16_0x0ff() { runCallFarMemCases(false, "call far e16 0ff"); }
void testCallR32_0x2ff() { runCallRegCases(true, "call r32 2ff"); }
void testCallE32_0x2ff() { runCallMemCase(true, "call e32 2ff"); }
void testCallFarE32_0x2ff() { runCallFarMemCases(true, "call far e32 2ff"); }

#endif
