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

#include "testBit.h"
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

constexpr U32 REG_GUARD = 0x6a000000;
constexpr U32 MEM_BASE = 0x0300;
constexpr U32 SETCC_RESULT = 0x0700;
constexpr U32 MEM_GUARD = 0xcdcdcdcd;
constexpr U32 CF_MASK = CF;
constexpr U32 ZF_MASK = ZF;

enum RegIndex {
    R_AX = 0,
    R_CX = 1,
    R_DX = 2,
    R_BX = 3,
    R_SP = 4,
    R_BP = 5,
    R_SI = 6,
    R_DI = 7
};

enum BitOp {
    BIT_BT,
    BIT_BTS,
    BIT_BTR,
    BIT_BTC
};

enum ScanOp {
    SCAN_BSF,
    SCAN_BSR
};

struct BitCase {
    U32 base;
    U32 bit;
};

struct ScanCase {
    U32 src;
    U32 expected;
    bool zero;
};

const BitCase BIT_CASES_16[] = {
    {0xfffd, 1},
    {0x0010, 4},
    {0xfffd, 33},
    {0x0010, 36},
    {0x8000, 15},
    {0x0001, 0},
};

const BitCase BIT_CASES_32[] = {
    {0xfffdffff, 17},
    {0x00100000, 20},
    {0xfffdffff, 81},
    {0x00100000, 84},
    {0x80000000, 31},
    {0x00000001, 0},
};

const ScanCase BSF_CASES_16[] = {
    {0x0001, 0, false},
    {0x0002, 1, false},
    {0x8000, 15, false},
    {0x8010, 4, false},
    {0x0000, 0, true},
};

const ScanCase BSF_CASES_32[] = {
    {0x00000001, 0, false},
    {0x00000002, 1, false},
    {0x80000000, 31, false},
    {0x80010000, 16, false},
    {0x00000000, 0, true},
};

const ScanCase BSR_CASES_16[] = {
    {0x8030, 15, false},
    {0x4030, 14, false},
    {0x0001, 0, false},
    {0x0800, 11, false},
    {0x0000, 0, true},
};

const ScanCase BSR_CASES_32[] = {
    {0x80000300, 31, false},
    {0x40000300, 30, false},
    {0x00000001, 0, false},
    {0x00080001, 19, false},
    {0x00000000, 0, true},
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

U8 bitOpcode(BitOp op) {
    switch (op) {
    case BIT_BT: return 0xa3;
    case BIT_BTS: return 0xab;
    case BIT_BTR: return 0xb3;
    default: return 0xbb;
    }
}

U8 group8RegField(BitOp op) {
    switch (op) {
    case BIT_BT: return 4;
    case BIT_BTS: return 5;
    case BIT_BTR: return 6;
    default: return 7;
    }
}

U8 scanOpcode(ScanOp op) {
    return op == SCAN_BSF ? 0xbc : 0xbd;
}

const char* bitName(BitOp op) {
    switch (op) {
    case BIT_BT: return "bt";
    case BIT_BTS: return "bts";
    case BIT_BTR: return "btr";
    default: return "btc";
    }
}

const char* scanName(ScanOp op) {
    return op == SCAN_BSF ? "bsf" : "bsr";
}

bool mutates(BitOp op) {
    return op != BIT_BT;
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

void emitBitRegReg(BitOp op, int dst, int src) {
    pushCode8(0x0f);
    pushCode8(bitOpcode(op));
    pushCode8(0xc0 | (src << 3) | dst);
}

void emitBitMemReg(BitOp op, int src, U32 address, bool big, bool lockPrefix) {
    if (lockPrefix) {
        pushCode8(0xf0);
    }
    pushCode8(0x0f);
    pushCode8(bitOpcode(op));
    emitDirectAddressModRM(src, address, big);
}

void emitGroup8RegImm(BitOp op, int dst, U8 bit) {
    pushCode8(0x0f);
    pushCode8(0xba);
    pushCode8(0xc0 | (group8RegField(op) << 3) | dst);
    pushCode8(bit);
}

void emitGroup8MemImm(BitOp op, U32 address, bool big, U8 bit, bool lockPrefix) {
    if (lockPrefix) {
        pushCode8(0xf0);
    }
    pushCode8(0x0f);
    pushCode8(0xba);
    emitDirectAddressModRM(group8RegField(op), address, big);
    pushCode8(bit);
}

void emitScanRegReg(ScanOp op, int dst, int src) {
    pushCode8(0x0f);
    pushCode8(scanOpcode(op));
    pushCode8(0xc0 | (dst << 3) | src);
}

void emitScanRegMem(ScanOp op, int dst, U32 address, bool big) {
    pushCode8(0x0f);
    pushCode8(scanOpcode(op));
    emitDirectAddressModRM(dst, address, big);
}

void emitSetccMem(U8 opcode, U32 address, bool big) {
    pushCode8(0x0f);
    pushCode8(opcode);
    emitDirectAddressModRM(0, address, big);
}

void emitCmpEaxEax() {
    pushCode8(0x39);
    pushCode8(0xc0);
}

U32 readWidth(U32 address, int width) {
    return width == 16 ? memory->readw(address) : memory->readd(address);
}

void writeWidth(U32 address, int width, U32 value) {
    if (width == 16) {
        memory->writew(address, value);
    } else {
        memory->writed(address, value);
    }
}

void initRegs(U32* regs) {
    for (int i = 0; i < 8; ++i) {
        regs[i] = REG_GUARD | (0x100 + i);
    }
}

void writeRegsLocal(const U32* regs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = regs[i];
    }
}

U32 valueForWidth(U32 value, int width) {
    return value & widthMask(width);
}

U32 bitMaskFor(BitOp op, int width, U32 base, U32 bit) {
    U32 mask = widthMask(width);
    U32 bitMask = 1u << (bit % width);
    U32 result = base & mask;
    if (op == BIT_BTS) {
        result |= bitMask;
    } else if (op == BIT_BTR) {
        result &= ~bitMask;
    } else if (op == BIT_BTC) {
        result ^= bitMask;
    }
    return result & mask;
}

bool bitCarry(int width, U32 base, U32 bit) {
    return ((base >> (bit % width)) & 1) != 0;
}

U32 memoryBitAddress(U32 baseAddress, int width, U32 bit) {
    return baseAddress + (bit / width) * (width / 8);
}

void prepareMemory(U32 address, int width, U32 value) {
    memory->writed(address - 4, MEM_GUARD);
    memory->writed(address + width / 8, MEM_GUARD);
    writeWidth(address, width, value);
}

void verifyMemory(U32 address, int width, U32 expected, const char* name) {
    if (readWidth(address, width) != (expected & widthMask(width))) {
        failed("%s memory result", name);
    }
    if (memory->readd(address - 4) != MEM_GUARD || memory->readd(address + width / 8) != MEM_GUARD) {
        failed("%s memory guard", name);
    }
}

void verifyCF(bool expected, const char* name) {
    if (cpu->getCF() != expected) {
        failed("%s cf", name);
    }
}

void verifyZF(bool expected, const char* name) {
    if (cpu->getZF() != expected) {
        failed("%s zf", name);
    }
}

void runBitRegCase(BitOp op, int width, int dst, int src, const BitCase& data, int flagMode, const char* name) {
    if (dst == src) {
        return;
    }

    char caseName[160];
    snprintf(caseName, sizeof(caseName), "%s %s reg dst=%d src=%d base=%x bit=%u mode=%d", name, bitName(op), dst, src, data.base, data.bit, flagMode);
    U32 regs[8];
    U32 expectedRegs[8];
    initRegs(regs);
    applyRegValue(regs, dst, width, data.base);
    applyRegValue(regs, src, width, data.bit);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }

    U32 expectedValue = bitMaskFor(op, width, data.base, data.bit);
    bool expectedCF = bitCarry(width, data.base, data.bit);
    applyRegValue(expectedRegs, dst, width, expectedValue);

    newInstruction(expectedCF ? 0 : CF);
    cpu->big = width == 32;
    emitBitRegReg(op, dst, src);
    if (flagMode == 1) {
        emitCmpEaxEax();
    } else if (flagMode == 2) {
        emitSetccMem(0x92, SETCC_RESULT, cpu->big);
    }
    writeRegsLocal(regs);
    memory->writeb(TEST_HEAP_ADDRESS + SETCC_RESULT, 0xcc);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    if (flagMode == 0) {
        verifyCF(expectedCF, caseName);
    } else if (flagMode == 2 && memory->readb(TEST_HEAP_ADDRESS + SETCC_RESULT) != (U8)(expectedCF ? 1 : 0)) {
        failed("%s setc", caseName);
    }
}

void runBitMemCase(BitOp op, int width, int src, const BitCase& data, int flagMode, bool lockPrefix, bool unaligned, const char* name) {
    if (lockPrefix && !mutates(op)) {
        return;
    }

    char caseName[160];
    snprintf(caseName, sizeof(caseName), "%s %s mem src=%d base=%x bit=%u mode=%d lock=%d unaligned=%d", name, bitName(op), src, data.base, data.bit, flagMode, lockPrefix ? 1 : 0, unaligned ? 1 : 0);
    U32 regs[8];
    U32 expectedRegs[8];
    initRegs(regs);
    applyRegValue(regs, src, width, data.bit);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }

    U32 baseOffset = MEM_BASE + (unaligned ? 1 : 0);
    U32 actualOffset = memoryBitAddress(baseOffset, width, data.bit);
    U32 linear = TEST_HEAP_ADDRESS + actualOffset;
    U32 expectedValue = bitMaskFor(op, width, data.base, data.bit);
    bool expectedCF = bitCarry(width, data.base, data.bit);

    newInstruction(expectedCF ? 0 : CF);
    cpu->big = width == 32;
    emitBitMemReg(op, src, baseOffset, cpu->big, lockPrefix);
    if (flagMode == 1) {
        emitCmpEaxEax();
    } else if (flagMode == 2) {
        emitSetccMem(0x92, SETCC_RESULT, cpu->big);
    }
    writeRegsLocal(regs);
    prepareMemory(linear, width, data.base);
    memory->writeb(TEST_HEAP_ADDRESS + SETCC_RESULT, 0xcc);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    verifyMemory(linear, width, expectedValue, caseName);
    if (flagMode == 0) {
        verifyCF(expectedCF, caseName);
    } else if (flagMode == 2 && memory->readb(TEST_HEAP_ADDRESS + SETCC_RESULT) != (U8)(expectedCF ? 1 : 0)) {
        failed("%s setc", caseName);
    }
}

void runBitCases(BitOp op, int width, const BitCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int flagMode = 0; flagMode < 3; ++flagMode) {
            for (int dst = 0; dst < 8; ++dst) {
                for (int src = 0; src < 8; ++src) {
                    runBitRegCase(op, width, dst, src, cases[i], flagMode, name);
                }
            }
            for (int src = 0; src < 8; ++src) {
                runBitMemCase(op, width, src, cases[i], flagMode, false, false, name);
                if (mutates(op)) {
                    runBitMemCase(op, width, src, cases[i], flagMode, true, false, name);
                    runBitMemCase(op, width, src, cases[i], flagMode, true, true, name);
                }
            }
        }
    }
}

void runGroup8RegCase(BitOp op, int width, int dst, const BitCase& data, int flagMode, const char* name) {
    char caseName[160];
    snprintf(caseName, sizeof(caseName), "%s %s reg dst=%d base=%x bit=%u mode=%d", name, bitName(op), dst, data.base, data.bit, flagMode);
    U32 regs[8];
    U32 expectedRegs[8];
    initRegs(regs);
    applyRegValue(regs, dst, width, data.base);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }

    U32 expectedValue = bitMaskFor(op, width, data.base, data.bit);
    bool expectedCF = bitCarry(width, data.base, data.bit);
    applyRegValue(expectedRegs, dst, width, expectedValue);

    newInstruction(expectedCF ? 0 : CF);
    cpu->big = width == 32;
    emitGroup8RegImm(op, dst, (U8)data.bit);
    if (flagMode == 1) {
        emitCmpEaxEax();
    } else if (flagMode == 2) {
        emitSetccMem(0x92, SETCC_RESULT, cpu->big);
    }
    writeRegsLocal(regs);
    memory->writeb(TEST_HEAP_ADDRESS + SETCC_RESULT, 0xcc);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    if (flagMode == 0) {
        verifyCF(expectedCF, caseName);
    } else if (flagMode == 2 && memory->readb(TEST_HEAP_ADDRESS + SETCC_RESULT) != (U8)(expectedCF ? 1 : 0)) {
        failed("%s setc", caseName);
    }
}

void runGroup8MemCase(BitOp op, int width, const BitCase& data, int flagMode, bool lockPrefix, bool unaligned, const char* name) {
    if (lockPrefix && !mutates(op)) {
        return;
    }

    char caseName[160];
    snprintf(caseName, sizeof(caseName), "%s %s mem base=%x bit=%u mode=%d lock=%d unaligned=%d", name, bitName(op), data.base, data.bit, flagMode, lockPrefix ? 1 : 0, unaligned ? 1 : 0);
    U32 regs[8];
    U32 expectedRegs[8];
    initRegs(regs);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }

    U32 offset = MEM_BASE + (unaligned ? 1 : 0);
    U32 linear = TEST_HEAP_ADDRESS + offset;
    U32 expectedValue = bitMaskFor(op, width, data.base, data.bit);
    bool expectedCF = bitCarry(width, data.base, data.bit);

    newInstruction(expectedCF ? 0 : CF);
    cpu->big = width == 32;
    emitGroup8MemImm(op, offset, cpu->big, (U8)data.bit, lockPrefix);
    if (flagMode == 1) {
        emitCmpEaxEax();
    } else if (flagMode == 2) {
        emitSetccMem(0x92, SETCC_RESULT, cpu->big);
    }
    writeRegsLocal(regs);
    prepareMemory(linear, width, data.base);
    memory->writeb(TEST_HEAP_ADDRESS + SETCC_RESULT, 0xcc);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    verifyMemory(linear, width, expectedValue, caseName);
    if (flagMode == 0) {
        verifyCF(expectedCF, caseName);
    } else if (flagMode == 2 && memory->readb(TEST_HEAP_ADDRESS + SETCC_RESULT) != (U8)(expectedCF ? 1 : 0)) {
        failed("%s setc", caseName);
    }
}

void runGroup8Cases(int width, const BitCase* cases, size_t count, const char* name) {
    const BitOp ops[] = {BIT_BT, BIT_BTS, BIT_BTR, BIT_BTC};
    for (BitOp op : ops) {
        for (size_t i = 0; i < count; ++i) {
            for (int flagMode = 0; flagMode < 3; ++flagMode) {
                for (int dst = 0; dst < 8; ++dst) {
                    runGroup8RegCase(op, width, dst, cases[i], flagMode, name);
                }
                runGroup8MemCase(op, width, cases[i], flagMode, false, false, name);
                if (mutates(op)) {
                    runGroup8MemCase(op, width, cases[i], flagMode, true, false, name);
                    runGroup8MemCase(op, width, cases[i], flagMode, true, true, name);
                }
            }
        }
    }
}

void runScanRegCase(ScanOp op, int width, int dst, int src, const ScanCase& data, int flagMode, const char* name) {
    char caseName[160];
    snprintf(caseName, sizeof(caseName), "%s %s reg dst=%d src=%d srcValue=%x mode=%d", name, scanName(op), dst, src, data.src, flagMode);
    U32 regs[8];
    U32 expectedRegs[8];
    initRegs(regs);
    applyRegValue(regs, dst, width, 0x55555555);
    applyRegValue(regs, src, width, data.src);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }
    if (!data.zero) {
        applyRegValue(expectedRegs, dst, width, data.expected);
    }

    newInstruction(data.zero ? 0 : ZF);
    cpu->big = width == 32;
    emitScanRegReg(op, dst, src);
    if (flagMode == 1) {
        emitCmpEaxEax();
    } else if (flagMode == 2) {
        emitSetccMem(0x94, SETCC_RESULT, cpu->big);
    }
    writeRegsLocal(regs);
    memory->writeb(TEST_HEAP_ADDRESS + SETCC_RESULT, 0xcc);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    if (flagMode == 0) {
        verifyZF(data.zero, caseName);
    } else if (flagMode == 2 && memory->readb(TEST_HEAP_ADDRESS + SETCC_RESULT) != (U8)(data.zero ? 1 : 0)) {
        failed("%s setz", caseName);
    }
}

void runScanMemCase(ScanOp op, int width, int dst, const ScanCase& data, int flagMode, const char* name) {
    char caseName[160];
    snprintf(caseName, sizeof(caseName), "%s %s mem dst=%d srcValue=%x mode=%d", name, scanName(op), dst, data.src, flagMode);
    U32 regs[8];
    U32 expectedRegs[8];
    initRegs(regs);
    applyRegValue(regs, dst, width, 0x55555555);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }
    if (!data.zero) {
        applyRegValue(expectedRegs, dst, width, data.expected);
    }

    newInstruction(data.zero ? 0 : ZF);
    cpu->big = width == 32;
    emitScanRegMem(op, dst, MEM_BASE, cpu->big);
    if (flagMode == 1) {
        emitCmpEaxEax();
    } else if (flagMode == 2) {
        emitSetccMem(0x94, SETCC_RESULT, cpu->big);
    }
    writeRegsLocal(regs);
    prepareMemory(TEST_HEAP_ADDRESS + MEM_BASE, width, data.src);
    memory->writeb(TEST_HEAP_ADDRESS + SETCC_RESULT, 0xcc);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    verifyMemory(TEST_HEAP_ADDRESS + MEM_BASE, width, data.src, caseName);
    if (flagMode == 0) {
        verifyZF(data.zero, caseName);
    } else if (flagMode == 2 && memory->readb(TEST_HEAP_ADDRESS + SETCC_RESULT) != (U8)(data.zero ? 1 : 0)) {
        failed("%s setz", caseName);
    }
}

void runScanCases(ScanOp op, int width, const ScanCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int flagMode = 0; flagMode < 3; ++flagMode) {
            for (int dst = 0; dst < 8; ++dst) {
                for (int src = 0; src < 8; ++src) {
                    runScanRegCase(op, width, dst, src, cases[i], flagMode, name);
                }
                runScanMemCase(op, width, dst, cases[i], flagMode, name);
            }
        }
    }
}

} // namespace

void testBtE16R16_0x1a3() { runBitCases(BIT_BT, 16, BIT_CASES_16, caseCount(BIT_CASES_16), "bt e16,r16 1a3"); }
void testBtE32R32_0x3a3() { runBitCases(BIT_BT, 32, BIT_CASES_32, caseCount(BIT_CASES_32), "bt e32,r32 3a3"); }
void testBtsE16R16_0x1ab() { runBitCases(BIT_BTS, 16, BIT_CASES_16, caseCount(BIT_CASES_16), "bts e16,r16 1ab"); }
void testBtsE32R32_0x3ab() { runBitCases(BIT_BTS, 32, BIT_CASES_32, caseCount(BIT_CASES_32), "bts e32,r32 3ab"); }
void testBtrE16R16_0x1b3() { runBitCases(BIT_BTR, 16, BIT_CASES_16, caseCount(BIT_CASES_16), "btr e16,r16 1b3"); }
void testBtrE32R32_0x3b3() { runBitCases(BIT_BTR, 32, BIT_CASES_32, caseCount(BIT_CASES_32), "btr e32,r32 3b3"); }
void testBtcE16R16_0x1bb() { runBitCases(BIT_BTC, 16, BIT_CASES_16, caseCount(BIT_CASES_16), "btc e16,r16 1bb"); }
void testBtcE32R32_0x3bb() { runBitCases(BIT_BTC, 32, BIT_CASES_32, caseCount(BIT_CASES_32), "btc e32,r32 3bb"); }
void testGroup8E16Ib_0x1ba() { runGroup8Cases(16, BIT_CASES_16, caseCount(BIT_CASES_16), "group8 e16,ib 1ba"); }
void testGroup8E32Ib_0x3ba() { runGroup8Cases(32, BIT_CASES_32, caseCount(BIT_CASES_32), "group8 e32,ib 3ba"); }
void testBsfR16E16_0x1bc() { runScanCases(SCAN_BSF, 16, BSF_CASES_16, caseCount(BSF_CASES_16), "bsf r16,e16 1bc"); }
void testBsfR32E32_0x3bc() { runScanCases(SCAN_BSF, 32, BSF_CASES_32, caseCount(BSF_CASES_32), "bsf r32,e32 3bc"); }
void testBsrR16E16_0x1bd() { runScanCases(SCAN_BSR, 16, BSR_CASES_16, caseCount(BSR_CASES_16), "bsr r16,e16 1bd"); }
void testBsrR32E32_0x3bd() { runScanCases(SCAN_BSR, 32, BSR_CASES_32, caseCount(BSR_CASES_32), "bsr r32,e32 3bd"); }

#endif
