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

#include "testFPU.h"
#include "testCPU.h"

#include <cmath>
#include <limits>

#define cpu (testContext().cpu)
#define memory (testContext().memory)
#define pushCode8 testPushCode8
#define pushCode16 testPushCode16
#define pushCode32 testPushCode32
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

constexpr U32 MEM_BASE = 0x1000;
constexpr U32 EXPECT_BASE = MEM_BASE + 0x800;
constexpr U32 FLOAT_GUARD = 0xCDCDCDCD;
constexpr U32 STATUS_MASK = 0x4700;
constexpr U32 FPU_STATUS_ZE = 0x0004;
constexpr U32 FPU_STATUS_ES = 0x0080;
constexpr U32 F32_POS_ZERO = 0x00000000;
constexpr U32 F32_NEG_ZERO = 0x80000000;
constexpr U32 F32_POS_INF = 0x7f800000;
constexpr U32 F32_NEG_INF = 0xff800000;
constexpr U32 F32_QNAN = 0x7fc00000;

enum CompareResult {
    FPU_LESS = 0x0100,
    FPU_EQUAL = 0x4000,
    FPU_GREATER = 0x0000,
    FPU_UNORDERED = 0x4500
};

struct FloatBits {
    union {
        float f;
        U32 u;
    };
};

struct DoubleBits {
    union {
        double d;
        U64 u;
    };
};

struct F32ArithCase {
    U32 left;
    U32 right;
    U32 add;
    U32 mul;
    U32 sub;
    U32 subr;
    U32 div;
    U32 divr;
};

const F32ArithCase F32_CASES[] = {
    {0x40c00000, 0x40000000, 0x41000000, 0x41400000, 0x40800000, 0xc0800000, 0x40400000, 0x3eaaaaab},
    {0xc1100000, 0x40400000, 0xc0c00000, 0xc1d80000, 0xc1400000, 0x41400000, 0xc0400000, 0xbeaaaaab},
    {0x3f000000, 0xc0000000, 0xbfc00000, 0xbf800000, 0x40200000, 0xc0200000, 0xbe800000, 0xc0800000},
    {F32_POS_ZERO, F32_POS_ZERO, F32_POS_ZERO, F32_POS_ZERO, F32_POS_ZERO, F32_POS_ZERO, F32_QNAN, F32_QNAN},
    {F32_NEG_ZERO, F32_POS_ZERO, F32_POS_ZERO, F32_NEG_ZERO, F32_NEG_ZERO, F32_POS_ZERO, F32_QNAN, F32_QNAN},
    {F32_POS_ZERO, F32_NEG_ZERO, F32_POS_ZERO, F32_NEG_ZERO, F32_POS_ZERO, F32_NEG_ZERO, F32_QNAN, F32_QNAN},
    {F32_POS_ZERO, F32_POS_INF, F32_POS_INF, F32_QNAN, F32_NEG_INF, F32_POS_INF, F32_POS_ZERO, F32_POS_INF},
    {F32_POS_INF, F32_POS_ZERO, F32_POS_INF, F32_QNAN, F32_POS_INF, F32_NEG_INF, F32_POS_INF, F32_POS_ZERO},
    {F32_POS_ZERO, F32_NEG_INF, F32_NEG_INF, F32_QNAN, F32_POS_INF, F32_NEG_INF, F32_NEG_ZERO, F32_NEG_INF},
    {F32_NEG_INF, F32_POS_ZERO, F32_NEG_INF, F32_QNAN, F32_NEG_INF, F32_POS_INF, F32_NEG_INF, F32_NEG_ZERO},
    {F32_NEG_INF, F32_POS_INF, F32_QNAN, F32_NEG_INF, F32_NEG_INF, F32_POS_INF, F32_QNAN, F32_QNAN},
    {F32_POS_INF, F32_POS_INF, F32_POS_INF, F32_POS_INF, F32_QNAN, F32_QNAN, F32_QNAN, F32_QNAN},
    {0x3f800000, 0xbf800000, F32_POS_ZERO, 0xbf800000, 0x40000000, 0xc0000000, 0xbf800000, 0xbf800000},
    {0xbf800000, 0xbf800000, 0xc0000000, 0x3f800000, F32_POS_ZERO, F32_POS_ZERO, 0x3f800000, 0x3f800000},
    {F32_QNAN, 0x40000000, F32_QNAN, F32_QNAN, F32_QNAN, F32_QNAN, F32_QNAN, F32_QNAN},
    {0xc0000000, F32_QNAN, F32_QNAN, F32_QNAN, F32_QNAN, F32_QNAN, F32_QNAN, F32_QNAN}
};

struct F32UnaryCase {
    U32 input;
    U32 expected;
};

const F32UnaryCase FCHS_CASES[] = {
    {0x43d80ccd, 0xc3d80ccd},
    {0xc3d80ccd, 0x43d80ccd},
    {F32_POS_ZERO, F32_POS_ZERO},
    {F32_NEG_ZERO, F32_POS_ZERO},
    {F32_POS_INF, F32_NEG_INF},
    {F32_NEG_INF, F32_POS_INF},
    {F32_QNAN, F32_QNAN}
};

const F32UnaryCase FABS_CASES[] = {
    {0xbaa1be2b, 0x3aa1be2b},
    {0x3aa1be2b, 0x3aa1be2b},
    {F32_NEG_ZERO, F32_POS_ZERO},
    {F32_POS_ZERO, F32_POS_ZERO},
    {F32_NEG_INF, F32_POS_INF},
    {F32_POS_INF, F32_POS_INF},
    {F32_QNAN, F32_QNAN}
};

const U32 FST_CASES[] = {
    F32_POS_ZERO, F32_NEG_ZERO, 0x3f800000, 0xbf800000, 0x3727c5ac, 0xb727c5ac,
    0x447c80a4, 0xc47c80a4, F32_QNAN, F32_POS_INF, F32_NEG_INF
};

U8 modRM(bool memoryOperand, int group, int rm) {
    return (U8)((memoryOperand ? 0 : 0xC0) | ((group & 7) << 3) | (rm & 7));
}

U32 addressOf(U32 offset) {
    return cpu->seg[DS].address + offset;
}

void emitMemModRM(int group, U32 offset, bool big) {
    pushCode8(modRM(true, group, big ? 5 : 6));
    if (big) {
        pushCode32(offset);
    } else {
        pushCode16(offset);
    }
}

void begin(bool big) {
    newInstruction(0);
    cpu->big = big ? 1 : 0;
}

void fninit() {
    pushCode8(0xdb);
    pushCode8(0xe3);
}

void fnstswAx() {
    pushCode8(0xdf);
    pushCode8(0xe0);
}

void writeF32(U32 offset, float value) {
    FloatBits bits;
    bits.f = value;
    memory->writed(addressOf(offset), bits.u);
}

void writeF32Bits(U32 offset, U32 value) {
    memory->writed(addressOf(offset), value);
}

float readF32(U32 offset) {
    FloatBits bits;
    bits.u = memory->readd(addressOf(offset));
    return bits.f;
}

float floatFromBits(U32 value) {
    FloatBits bits;
    bits.u = value;
    return bits.f;
}

void writeF64(U32 offset, double value) {
    DoubleBits bits;
    bits.d = value;
    memory->writeq(addressOf(offset), bits.u);
}

U32 bitsOf(float value) {
    FloatBits bits;
    bits.f = value;
    return bits.u;
}

U64 bitsOf(double value) {
    DoubleBits bits;
    bits.d = value;
    return bits.u;
}

void writeExpected32(int slot, U32 value) {
    memory->writed(addressOf(EXPECT_BASE + slot * 8), value);
}

void writeExpected64(int slot, U64 value) {
    memory->writeq(addressOf(EXPECT_BASE + slot * 8), value);
}

U32 readExpected32(int slot) {
    return memory->readd(addressOf(EXPECT_BASE + slot * 8));
}

U64 readExpected64(int slot) {
    return memory->readq(addressOf(EXPECT_BASE + slot * 8));
}

U32 highestBit(U64 value) {
    for (S32 i = 63; i >= 0; --i) {
        if (value & ((U64)1 << i)) {
            return (U32)i;
        }
    }
    return 0;
}

void expectedI64Ext80(S64 value, U64* low, U16* high) {
    if (!value) {
        *low = 0;
        *high = 0;
        return;
    }
    U64 magnitude = value < 0 ? (U64)(-(value + 1)) + 1 : (U64)value;
    U32 bit = highestBit(magnitude);
    *low = magnitude << (63 - bit);
    *high = (U16)(0x3fff + bit);
    if (value < 0) {
        *high |= 0x8000;
    }
}

double readF64(U32 offset) {
    DoubleBits bits;
    bits.u = memory->readq(addressOf(offset));
    return bits.d;
}

void writeI16(U32 offset, S16 value) {
    memory->writew(addressOf(offset), (U16)value);
}

void writeI32(U32 offset, S32 value) {
    memory->writed(addressOf(offset), (U32)value);
}

void writeI64(U32 offset, S64 value) {
    memory->writeq(addressOf(offset), (U64)value);
}

U16 readI16(U32 offset) {
    return memory->readw(addressOf(offset));
}

U32 readI32(U32 offset) {
    return memory->readd(addressOf(offset));
}

U64 readI64(U32 offset) {
    return memory->readq(addressOf(offset));
}

void fldF32(U32 offset, bool big) {
    pushCode8(0xd9);
    emitMemModRM(0, offset, big);
}

void fstF32(U32 offset, bool pop, bool big) {
    pushCode8(0xd9);
    emitMemModRM(pop ? 3 : 2, offset, big);
}

void fldF64(U32 offset, bool big) {
    pushCode8(0xdd);
    emitMemModRM(0, offset, big);
}

void fstF64(U32 offset, bool pop, bool big) {
    pushCode8(0xdd);
    emitMemModRM(pop ? 3 : 2, offset, big);
}

void fild16(U32 offset, bool big) {
    pushCode8(0xdf);
    emitMemModRM(0, offset, big);
}

void fild64(U32 offset, bool big) {
    pushCode8(0xdf);
    emitMemModRM(5, offset, big);
}

void fstTopF32(U32 offset, bool big) {
    fstF32(offset, false, big);
}

void assertFloatCloseBits(float actual, U32 expectedBits, const char* name) {
    FloatBits expectedValue;
    expectedValue.u = expectedBits;
    float expected = expectedValue.f;
    if (std::isnan(expected)) {
        if (!std::isnan(actual)) {
            failed("%s float result", name);
        }
        return;
    }
    if (actual == expected) {
        return;
    }
    float diff = std::fabs(actual - expected);
    float tolerance = std::fabs(expected) / 100000.0f + 0.000001f;
    if (diff > tolerance) {
        failed("%s float result", name);
    }
}

void assertFloatCloseExpected(float actual, int slot, const char* name) {
    assertFloatCloseBits(actual, readExpected32(slot), name);
}

void assertF32MemoryBits(U32 offset, U32 expectedBits, const char* name) {
    U32 actualBits = memory->readd(addressOf(offset));
    if (std::isnan(floatFromBits(expectedBits))) {
        if (!std::isnan(floatFromBits(actualBits))) {
            failed("%s float bits", name);
        }
        return;
    }
    if (actualBits != expectedBits) {
        failed("%s float bits expected=%08x actual=%08x", name, expectedBits, actualBits);
    }
}

void assertDoubleCloseBits(double actual, U64 expectedBits, const char* name) {
    DoubleBits expectedValue;
    expectedValue.u = expectedBits;
    double expected = expectedValue.d;
    if (std::isnan(expected)) {
        if (!std::isnan(actual)) {
            failed("%s double result", name);
        }
        return;
    }
    if (actual == expected) {
        return;
    }
    double diff = std::fabs(actual - expected);
    double tolerance = std::fabs(expected) / 1000000000000.0 + 0.000000000001;
    if (diff > tolerance) {
        failed("%s double result", name);
    }
}

void assertDoubleCloseExpected(double actual, int slot, const char* name) {
    assertDoubleCloseBits(actual, readExpected64(slot), name);
}

U32 compareStatus(float left, float right) {
    if (std::isnan(left) || std::isnan(right)) {
        return FPU_UNORDERED;
    }
    if (left == right) {
        return FPU_EQUAL;
    }
    return left < right ? FPU_LESS : FPU_GREATER;
}

U32 compareStatusBits(U32 left, U32 right) {
    return compareStatus(floatFromBits(left), floatFromBits(right));
}

void runD8MemoryArith(bool big, int group, U32 expectedBits, const F32ArithCase& data, const char* name) {
    constexpr U32 SRC = MEM_BASE + 0x100;
    constexpr U32 DST = MEM_BASE + 0x120;

    begin(big);
    writeExpected32(0, expectedBits);
    fninit();
    writeF32Bits(SRC, data.right);
    memory->writed(addressOf(DST), FLOAT_GUARD);
    fldF32(MEM_BASE, big);
    pushCode8(0xd8);
    emitMemModRM(group, SRC, big);
    fstTopF32(DST, big);

    writeF32Bits(MEM_BASE, data.left);
    runTestCPU();
    assertFloatCloseExpected(readF32(DST), 0, name);
    if (cpu->fpu.GetTop() != 7 || memory->readd(addressOf(SRC)) == FLOAT_GUARD) {
        failed("%s stack/memory state", name);
    }
}

void runD8Compare(bool big, bool pop, float left, float right, const char* name) {
    constexpr U32 SRC = MEM_BASE + 0x100;
    U32 expectedStatus = compareStatus(left, right);

    begin(big);
    writeExpected32(0, expectedStatus);
    fninit();
    writeF32(MEM_BASE, left);
    writeF32(SRC, right);
    fldF32(MEM_BASE, big);
    pushCode8(0xd8);
    emitMemModRM(pop ? 3 : 2, SRC, big);
    fnstswAx();

    runTestCPU();
    if ((cpu->reg[0].u16 & STATUS_MASK) != readExpected32(0)) {
        failed("%s compare status", name);
    }
    if (cpu->fpu.GetTop() != (pop ? 0 : 7)) {
        failed("%s compare stack", name);
    }
}

void runD8CompareBits(bool big, bool pop, U32 left, U32 right, const char* name) {
    constexpr U32 SRC = MEM_BASE + 0x100;
    U32 expectedStatus = compareStatusBits(left, right);

    begin(big);
    writeExpected32(0, expectedStatus);
    fninit();
    writeF32Bits(MEM_BASE, left);
    writeF32Bits(SRC, right);
    fldF32(MEM_BASE, big);
    pushCode8(0xd8);
    emitMemModRM(pop ? 3 : 2, SRC, big);
    fnstswAx();

    runTestCPU();
    if ((cpu->reg[0].u16 & STATUS_MASK) != readExpected32(0)) {
        failed("%s compare status", name);
    }
    if (cpu->fpu.GetTop() != (pop ? 0 : 7)) {
        failed("%s compare stack", name);
    }
}

void runD8RegisterArith(bool big, int group, U32 expectedBits, const F32ArithCase& data, const char* name) {
    constexpr U32 DST = MEM_BASE + 0x140;

    begin(big);
    writeExpected32(0, expectedBits);
    fninit();
    writeF32Bits(MEM_BASE, data.right);
    writeF32Bits(MEM_BASE + 4, data.left);
    fldF32(MEM_BASE, big);
    fldF32(MEM_BASE + 4, big);
    pushCode8(0xd8);
    pushCode8(modRM(false, group, 1));
    fstTopF32(DST, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(DST), 0, name);
    if (cpu->fpu.GetTop() != 6) {
        failed("%s register stack", name);
    }
}

void runFSTFloat(bool big, bool pop, float value, const char* name) {
    constexpr U32 DST = MEM_BASE + 0x180;
    U32 expectedBits = bitsOf(value);

    begin(big);
    writeExpected32(0, expectedBits);
    fninit();
    writeF32(MEM_BASE, value);
    memory->writed(addressOf(DST), FLOAT_GUARD);
    fldF32(MEM_BASE, big);
    fstF32(DST, pop, big);
    runTestCPU();
    assertFloatCloseExpected(readF32(DST), 0, name);
    if (cpu->fpu.GetTop() != (pop ? 0 : 7)) {
        failed("%s fst stack", name);
    }
}

void runFSTFloatBits(bool big, bool pop, U32 value, const char* name) {
    constexpr U32 DST = MEM_BASE + 0x180;

    begin(big);
    writeExpected32(0, value);
    fninit();
    writeF32Bits(MEM_BASE, value);
    memory->writed(addressOf(DST), FLOAT_GUARD);
    fldF32(MEM_BASE, big);
    fstF32(DST, pop, big);
    runTestCPU();
    assertF32MemoryBits(DST, readExpected32(0), name);
    if (cpu->fpu.GetTop() != (pop ? 0 : 7)) {
        failed("%s fst stack", name);
    }
}

void runD9StackOps(bool big, const char* name) {
    constexpr U32 OUT0 = MEM_BASE + 0x200;
    constexpr U32 OUT1 = MEM_BASE + 0x204;
    constexpr U32 OUT2 = MEM_BASE + 0x208;

    begin(big);
    writeExpected32(0, bitsOf(3.0f));
    writeExpected32(1, bitsOf(1.0f));
    writeExpected32(2, bitsOf(3.0f));
    fninit();
    writeF32(MEM_BASE, 4.0f);
    writeF32(MEM_BASE + 4, 3.0f);
    writeF32(MEM_BASE + 8, 2.0f);
    writeF32(MEM_BASE + 12, 1.0f);
    fldF32(MEM_BASE, big);
    fldF32(MEM_BASE + 4, big);
    fldF32(MEM_BASE + 8, big);
    fldF32(MEM_BASE + 12, big);
    pushCode8(0xd9);
    pushCode8(0xc2); // FLD ST(2)
    fstTopF32(OUT0, big);
    pushCode8(0xd9);
    pushCode8(0xc9); // FXCH ST(1)
    fstTopF32(OUT1, big);
    pushCode8(0xd9);
    pushCode8(0xda); // FSTP ST(2)
    fstTopF32(OUT2, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT0), 0, name);
    assertFloatCloseExpected(readF32(OUT1), 1, name);
    assertFloatCloseExpected(readF32(OUT2), 2, name);
}

void runD9Unary(bool big, U8 sub, float input, float expected, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x220;
    U32 expectedBits = bitsOf(expected);

    begin(big);
    writeExpected32(0, expectedBits);
    fninit();
    writeF32(MEM_BASE, input);
    fldF32(MEM_BASE, big);
    pushCode8(0xd9);
    pushCode8(modRM(false, 4, sub));
    fstTopF32(OUT, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
}

void runD9UnaryBits(bool big, U8 sub, U32 input, U32 expected, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x220;

    begin(big);
    writeExpected32(0, expected);
    fninit();
    writeF32Bits(MEM_BASE, input);
    fldF32(MEM_BASE, big);
    pushCode8(0xd9);
    pushCode8(modRM(false, 4, sub));
    fstTopF32(OUT, big);

    runTestCPU();
    assertF32MemoryBits(OUT, readExpected32(0), name);
}

void runD9Constant(bool big, U8 sub, float expected, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x240;
    U32 expectedBits = bitsOf(expected);

    begin(big);
    writeExpected32(0, expectedBits);
    fninit();
    pushCode8(0xd9);
    pushCode8(modRM(false, 5, sub));
    fstTopF32(OUT, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
}

void runD9Control(bool big, const char* name) {
    constexpr U32 CW_IN = MEM_BASE + 0x2a0;
    constexpr U32 CW_OUT = MEM_BASE + 0x2a4;

    begin(big);
    fninit();
    pushCode8(0xd9);
    pushCode8(0xd0); // FNOP
    pushCode8(0xd9);
    emitMemModRM(7, CW_OUT, big); // FNSTCW
    writeI16(CW_IN, 0x0f7f);
    pushCode8(0xd9);
    emitMemModRM(5, CW_IN, big); // FLDCW

    runTestCPU();
    if (readI16(CW_OUT) != 0x037f || (cpu->fpu.CW() & 0xffff) != 0x0f7f || cpu->fpu.round != 3) {
        failed("%s control word", name);
    }
}

void runD9StackControl(bool big, const char* name) {
    begin(big);
    fninit();
    pushCode8(0xd9);
    pushCode8(0xf6); // FDECSTP
    pushCode8(0xd9);
    pushCode8(0xf7); // FINCSTP

    runTestCPU();
    if (cpu->fpu.GetTop() != 0) {
        failed("%s stack control", name);
    }
}

void runD9FldSt0(bool big, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x2a8;

    begin(big);
    writeExpected32(0, bitsOf(5.0f));
    fninit();
    writeF32(MEM_BASE, 5.0f);
    fldF32(MEM_BASE, big);
    pushCode8(0xd9);
    pushCode8(0xc0); // FLD ST(0)
    fstTopF32(OUT, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
    if (cpu->fpu.GetTop() != 6) {
        failed("%s stack", name);
    }
}

void runFTST(bool big, float value, U32 expectedStatus, const char* name) {
    begin(big);
    writeExpected32(0, expectedStatus);
    fninit();
    writeF32(MEM_BASE, value);
    fldF32(MEM_BASE, big);
    pushCode8(0xd9);
    pushCode8(0xe4);
    fnstswAx();

    runTestCPU();
    if ((cpu->reg[0].u16 & STATUS_MASK) != readExpected32(0)) {
        failed("%s ftst status", name);
    }
}

void runD9RoundSqrtScale(bool big, const char* name) {
    constexpr U32 OUT0 = MEM_BASE + 0x260;
    constexpr U32 OUT1 = MEM_BASE + 0x264;
    constexpr U32 OUT2 = MEM_BASE + 0x268;

    begin(big);
    writeExpected32(0, bitsOf(3.0f));
    writeExpected32(1, bitsOf(12.0f));
    fninit();
    writeF32(MEM_BASE, 9.0f);
    fldF32(MEM_BASE, big);
    pushCode8(0xd9);
    pushCode8(0xfa); // FSQRT
    fstTopF32(OUT0, big);

    writeF32(MEM_BASE + 4, 2.6f);
    fldF32(MEM_BASE + 4, big);
    pushCode8(0xd9);
    pushCode8(0xfc); // FRNDINT
    fstTopF32(OUT1, big);

    writeF32(MEM_BASE + 8, 3.0f);
    writeF32(MEM_BASE + 12, 2.0f);
    fldF32(MEM_BASE + 12, big);
    fldF32(MEM_BASE + 8, big);
    pushCode8(0xd9);
    pushCode8(0xfd); // FSCALE
    fstTopF32(OUT2, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT0), 0, name);
    assertFloatCloseExpected(readF32(OUT1), 0, name);
    assertFloatCloseExpected(readF32(OUT2), 1, name);
}

void runFSQRTBits(bool big, U32 input, U32 expected, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x280;

    begin(big);
    writeExpected32(0, expected);
    fninit();
    writeF32Bits(MEM_BASE, input);
    fldF32(MEM_BASE, big);
    pushCode8(0xd9);
    pushCode8(0xfa);
    fstTopF32(OUT, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
}

void runFRNDINTBits(bool big, U32 input, U32 expected, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x284;

    begin(big);
    writeExpected32(0, expected);
    fninit();
    writeF32Bits(MEM_BASE, input);
    fldF32(MEM_BASE, big);
    pushCode8(0xd9);
    pushCode8(0xfc);
    fstTopF32(OUT, big);

    runTestCPU();
    assertF32MemoryBits(OUT, readExpected32(0), name);
}

void runFSCALEBits(bool big, U32 value, U32 exponent, U32 expected, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x288;

    begin(big);
    writeExpected32(0, expected);
    fninit();
    writeF32Bits(MEM_BASE, value);
    writeF32Bits(MEM_BASE + 4, exponent);
    fldF32(MEM_BASE + 4, big);
    fldF32(MEM_BASE, big);
    pushCode8(0xd9);
    pushCode8(0xfd);
    fstTopF32(OUT, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
}

void runFpuCmov(bool big, U8 group, U32 flags, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x300;

    begin(big);
    writeExpected32(0, bitsOf(2.0f));
    cpu->setFlags(flags, FMASK_ALL);
    fninit();
    writeF32(MEM_BASE, 2.0f);
    writeF32(MEM_BASE + 4, 3.0f);
    fldF32(MEM_BASE, big);
    fldF32(MEM_BASE + 4, big);
    pushCode8(0xda);
    pushCode8(modRM(false, group, 1));
    fstTopF32(OUT, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
}

void runFpuCmovOpcode(bool big, U8 opcode, U8 group, U32 flags, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x304;

    begin(big);
    writeExpected32(0, bitsOf(2.0f));
    cpu->setFlags(flags, FMASK_ALL);
    fninit();
    writeF32(MEM_BASE, 2.0f);
    writeF32(MEM_BASE + 4, 3.0f);
    fldF32(MEM_BASE, big);
    fldF32(MEM_BASE + 4, big);
    pushCode8(opcode);
    pushCode8(modRM(false, group, 1));
    fstTopF32(OUT, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
}

void runFUCOMPP(bool big, float left, float right, U32 expectedStatus, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x320;

    begin(big);
    writeExpected32(0, bitsOf(100.0f));
    writeExpected32(1, expectedStatus);
    fninit();
    writeF32(MEM_BASE, 100.0f);
    writeF32(MEM_BASE + 4, left);
    writeF32(MEM_BASE + 8, right);
    fldF32(MEM_BASE, big);
    fldF32(MEM_BASE + 4, big);
    fldF32(MEM_BASE + 8, big);
    pushCode8(0xda);
    pushCode8(0xe9);
    fstTopF32(OUT, big);
    fnstswAx();

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
    if ((cpu->reg[0].u16 & STATUS_MASK) != readExpected32(1) || cpu->fpu.GetTop() != 7) {
        failed("%s fucompp state", name);
    }
}

void runDAIntegerArith(bool big, U8 group, S32 integerValue, U32 expectedBits, const char* name) {
    constexpr U32 SRC = MEM_BASE + 0x340;
    constexpr U32 OUT = MEM_BASE + 0x344;

    begin(big);
    writeExpected32(0, expectedBits);
    fninit();
    writeF32(MEM_BASE, 10.0f);
    writeI32(SRC, integerValue);
    fldF32(MEM_BASE, big);
    pushCode8(0xda);
    emitMemModRM(group, SRC, big);
    fstTopF32(OUT, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
}

void runDAIntegerCompare(bool big, bool pop, U32 left, S32 right, U32 expectedStatus, const char* name) {
    constexpr U32 SRC = MEM_BASE + 0x360;
    constexpr U32 OUT = MEM_BASE + 0x364;
    U32 expectedBits = pop ? bitsOf(77.0f) : left;

    begin(big);
    writeExpected32(0, expectedBits);
    writeExpected32(1, expectedStatus);
    fninit();
    writeF32(MEM_BASE, 77.0f);
    writeF32Bits(MEM_BASE + 4, left);
    writeI32(SRC, right);
    fldF32(MEM_BASE, big);
    fldF32(MEM_BASE + 4, big);
    pushCode8(0xda);
    emitMemModRM(pop ? 3 : 2, SRC, big);
    fstTopF32(OUT, big);
    fnstswAx();

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
    if ((cpu->reg[0].u16 & STATUS_MASK) != readExpected32(1)) {
        failed("%s integer compare status", name);
    }
}

void runDCDoubleArith(bool big, U8 group, double left, double right, double expected, const char* name) {
    constexpr U32 SRC = MEM_BASE + 0x380;
    constexpr U32 OUT = MEM_BASE + 0x388;

    begin(big);
    writeExpected64(0, bitsOf(expected));
    fninit();
    writeF64(MEM_BASE, left);
    writeF64(SRC, right);
    fldF64(MEM_BASE, big);
    pushCode8(0xdc);
    emitMemModRM(group, SRC, big);
    fstF64(OUT, false, big);

    runTestCPU();
    assertDoubleCloseExpected(readF64(OUT), 0, name);
    if (cpu->fpu.GetTop() != 7) {
        failed("%s double arithmetic stack", name);
    }
}

void runDCDoubleCompare(bool big, bool pop, double left, double right, U32 expectedStatus, const char* name) {
    constexpr U32 SRC = MEM_BASE + 0x390;

    begin(big);
    writeExpected32(0, expectedStatus);
    fninit();
    writeF64(MEM_BASE, left);
    writeF64(SRC, right);
    fldF64(MEM_BASE, big);
    pushCode8(0xdc);
    emitMemModRM(pop ? 3 : 2, SRC, big);
    fnstswAx();

    runTestCPU();
    if ((cpu->reg[0].u16 & STATUS_MASK) != readExpected32(0) || cpu->fpu.GetTop() != (pop ? 0 : 7)) {
        failed("%s double compare state", name);
    }
}

void runDCRegisterArith(bool big, U8 group, double left, double right, double expected, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x3a0;

    begin(big);
    writeExpected64(0, bitsOf(expected));
    fninit();
    writeF64(MEM_BASE, right);
    writeF64(MEM_BASE + 8, left);
    fldF64(MEM_BASE, big);
    fldF64(MEM_BASE + 8, big);
    pushCode8(0xdc);
    pushCode8(modRM(false, group, 1));
    pushCode8(0xd9);
    pushCode8(0xc9); // FXCH ST(1)
    fstF64(OUT, false, big);

    runTestCPU();
    assertDoubleCloseExpected(readF64(OUT), 0, name);
    if (cpu->fpu.GetTop() != 6) {
        failed("%s register arithmetic stack", name);
    }
}

void runDERegisterArith(bool big, U8 group, double left, double right, double expected, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x3b0;

    begin(big);
    writeExpected64(0, bitsOf(expected));
    fninit();
    writeF64(MEM_BASE, right);
    writeF64(MEM_BASE + 8, left);
    fldF64(MEM_BASE, big);
    fldF64(MEM_BASE + 8, big);
    pushCode8(0xde);
    pushCode8(modRM(false, group, 1));
    fstF64(OUT, false, big);

    runTestCPU();
    assertDoubleCloseExpected(readF64(OUT), 0, name);
    if (cpu->fpu.GetTop() != 7) {
        failed("%s register arithmetic pop stack", name);
    }
}

void runDEWordArith(bool big, U8 group, S16 integerValue, U32 expectedBits, const char* name) {
    constexpr U32 SRC = MEM_BASE + 0x3c0;
    constexpr U32 OUT = MEM_BASE + 0x3c4;

    begin(big);
    writeExpected32(0, expectedBits);
    fninit();
    writeF32(MEM_BASE, 10.0f);
    writeI16(SRC, integerValue);
    fldF32(MEM_BASE, big);
    pushCode8(0xde);
    emitMemModRM(group, SRC, big);
    fstTopF32(OUT, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
    if (cpu->fpu.GetTop() != 7) {
        failed("%s word integer arithmetic stack", name);
    }
}

void runDEWordCompare(bool big, bool pop, U32 left, S16 right, U32 expectedStatus, const char* name) {
    constexpr U32 SRC = MEM_BASE + 0x3d0;

    begin(big);
    writeExpected32(0, expectedStatus);
    fninit();
    writeF32Bits(MEM_BASE, left);
    writeI16(SRC, right);
    fldF32(MEM_BASE, big);
    pushCode8(0xde);
    emitMemModRM(pop ? 3 : 2, SRC, big);
    fnstswAx();

    runTestCPU();
    if ((cpu->reg[0].u16 & STATUS_MASK) != readExpected32(0) || cpu->fpu.GetTop() != (pop ? 0 : 7)) {
        failed("%s word integer compare state", name);
    }
}

void runDERegisterComparePop(bool big, const char* name) {
    begin(big);
    writeExpected32(0, FPU_EQUAL);
    fninit();
    writeF32(MEM_BASE, 2.0f);
    writeF32(MEM_BASE + 4, 2.0f);
    fldF32(MEM_BASE, big);
    fldF32(MEM_BASE + 4, big);
    pushCode8(0xde);
    pushCode8(modRM(false, 2, 1)); // FCOMP ST(1)
    fnstswAx();

    runTestCPU();
    if ((cpu->reg[0].u16 & STATUS_MASK) != readExpected32(0) || cpu->fpu.GetTop() != 7) {
        failed("%s register compare pop state", name);
    }
}

void runDDDoubleStore(bool big, bool pop, double value, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x400;
    U64 expectedBits = bitsOf(value);

    begin(big);
    writeExpected64(0, expectedBits);
    fninit();
    writeF64(MEM_BASE, value);
    memory->writeq(addressOf(OUT), 0);
    fldF64(MEM_BASE, big);
    fstF64(OUT, pop, big);

    runTestCPU();
    assertDoubleCloseExpected(readF64(OUT), 0, name);
    if (cpu->fpu.GetTop() != (pop ? 0 : 7)) {
        failed("%s double stack", name);
    }
}

void runFISTTP64(bool big, double value, U64 expected, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x440;

    begin(big);
    writeExpected64(0, expected);
    fninit();
    writeF64(MEM_BASE, value);
    fldF64(MEM_BASE, big);
    pushCode8(0xdd);
    emitMemModRM(1, OUT, big);

    runTestCPU();
    if (readI64(OUT) != readExpected64(0) || cpu->fpu.GetTop() != 0 || cpu->fpu.GetTag(cpu, 7) != TAG_Empty) {
        failed("%s fisttp64 result", name);
    }
}

U32 packedTags(const U8 tags[8]) {
    U32 result = 0;
    for (int i = 0; i < 8; ++i) {
        result |= (tags[i] & 3) << (i * 2);
    }
    return result;
}

void writeEnvValue(U32 base, int index, bool big, U32 value) {
    if (big) {
        memory->writed(addressOf(base + index * 4), value);
    } else {
        memory->writew(addressOf(base + index * 2), (U16)value);
    }
}

U32 readEnvValue(U32 base, int index, bool big) {
    if (big) {
        return memory->readd(addressOf(base + index * 4));
    }
    return memory->readw(addressOf(base + index * 2));
}

void emitD9MemoryGroup(U8 group, U32 offset, bool big) {
    pushCode8(0xd9);
    emitMemModRM(group, offset, big);
}

void runFPUEnvironmentLoad(bool big, const char* name) {
    constexpr U32 ENV = MEM_BASE + 0x580;
    const U32 expectedCw = 0x0b7f;
    const U32 expectedSw = 0x2800 | FPU_LESS;
    const U8 tags[8] = {TAG_Empty, TAG_Valid, TAG_Zero, TAG_Special, TAG_Empty, TAG_Valid, TAG_Empty, TAG_Zero};
    const U32 expectedTag = packedTags(tags);
    const U32 envData[4] = {0x1234, 0x5678, 0x9abc, 0xdef0};

    begin(big);
    writeEnvValue(ENV, 0, big, expectedCw);
    writeEnvValue(ENV, 1, big, expectedSw);
    writeEnvValue(ENV, 2, big, expectedTag);
    for (int i = 0; i < 4; ++i) {
        writeEnvValue(ENV, 3 + i, big, envData[i]);
    }
    emitD9MemoryGroup(4, ENV, big); // FLDENV

    runTestCPU();
    if ((cpu->fpu.CW() & 0xffff) != expectedCw ||
            (cpu->fpu.sw & 0xffff) != expectedSw ||
            cpu->fpu.GetTop() != 5) {
        failed("%s fldenv control/status", name);
    }
    for (int i = 0; i < 8; ++i) {
        if (cpu->fpu.tags[i] != tags[i]) {
            failed("%s fldenv tags", name);
        }
    }
    for (int i = 0; i < 4; ++i) {
        if ((cpu->fpu.envData[i] & 0xffff) != envData[i]) {
            failed("%s fldenv data", name);
        }
    }
}

void runFPUEnvironmentStore(bool big, const char* name) {
    constexpr U32 ENV = MEM_BASE + 0x5c0;

    begin(big);
    fninit();
    writeF32(MEM_BASE, 1.25f);
    writeF32(MEM_BASE + 4, 2.5f);
    fldF32(MEM_BASE, big);
    fldF32(MEM_BASE + 4, big);
    emitD9MemoryGroup(6, ENV, big); // FNSTENV

    runTestCPU();
    U32 actualCw = readEnvValue(ENV, 0, big) & 0xffff;
    U32 actualSw = readEnvValue(ENV, 1, big);
    if (actualCw != 0x037f || ((actualSw >> 11) & 7) != 6) {
        failed("%s fnstenv header cw=%x sw=%x", name, actualCw, actualSw);
    }
    for (int i = 3; i < 7; ++i) {
        if (readEnvValue(ENV, i, big) != 0) {
            failed("%s fnstenv data", name);
        }
    }
}

void runMaskedDivideByZeroStatus(bool big, const char* name) {
    constexpr U32 ZERO = MEM_BASE + 0x620;
    constexpr U32 ENV_MASKED = MEM_BASE + 0x640;
    constexpr U32 CW_UNMASK_ZE = MEM_BASE + 0x680;
    constexpr U32 ENV_UNMASKED = MEM_BASE + 0x6c0;
    constexpr U32 STATUS_BITS = FPU_STATUS_ZE | FPU_STATUS_ES;

    begin(big);
    fninit();

    writeF32(MEM_BASE, 1.0f);
    writeF32(ZERO, 0.0f);
    fldF32(MEM_BASE, big);
    pushCode8(0xd8);
    emitMemModRM(6, ZERO, big); // FDIV m32fp
    fnstswAx();
    emitD9MemoryGroup(6, ENV_MASKED, big); // FNSTENV

    fninit();
    writeI16(CW_UNMASK_ZE, 0x037b);
    pushCode8(0xd9);
    emitMemModRM(5, CW_UNMASK_ZE, big); // FLDCW
    fldF32(MEM_BASE, big);
    pushCode8(0xd8);
    emitMemModRM(6, ZERO, big); // FDIV m32fp
    emitD9MemoryGroup(6, ENV_UNMASKED, big); // FNSTENV

    runTestCPU();

    U32 maskedAxStatus = cpu->reg[0].u16 & STATUS_BITS;
    U32 maskedEnvStatus = readEnvValue(ENV_MASKED, 1, big) & STATUS_BITS;
    if (maskedAxStatus != FPU_STATUS_ZE || maskedEnvStatus != FPU_STATUS_ZE) {
        failed("%s masked divide-by-zero status ax=%x env=%x", name, maskedAxStatus, maskedEnvStatus);
    }

    U32 unmaskedEnvStatus = readEnvValue(ENV_UNMASKED, 1, big) & STATUS_BITS;
    if (unmaskedEnvStatus != STATUS_BITS) {
        failed("%s unmasked divide-by-zero status env=%x", name, unmaskedEnvStatus);
    }
    if (!cpu->fpu.divExceptionsUnmasked) {
        failed("%s FLDCW did not mark divide exceptions unmasked", name);
    }
}

void runMaskedDoubleDivideByZeroStatus(bool big, const char* name) {
    constexpr U32 ZERO = MEM_BASE + 0x620;
    constexpr U32 ENV_MASKED = MEM_BASE + 0x640;
    constexpr U32 CW_UNMASK_ZE = MEM_BASE + 0x680;
    constexpr U32 ENV_UNMASKED = MEM_BASE + 0x6c0;
    constexpr U32 STATUS_BITS = FPU_STATUS_ZE | FPU_STATUS_ES;

    begin(big);
    fninit();

    writeF64(MEM_BASE, 1.0);
    writeF64(ZERO, 0.0);
    fldF64(MEM_BASE, big);
    pushCode8(0xdc);
    emitMemModRM(6, ZERO, big); // FDIV m64fp
    fnstswAx();
    emitD9MemoryGroup(6, ENV_MASKED, big); // FNSTENV

    fninit();
    writeI16(CW_UNMASK_ZE, 0x037b);
    pushCode8(0xd9);
    emitMemModRM(5, CW_UNMASK_ZE, big); // FLDCW
    fldF64(MEM_BASE, big);
    pushCode8(0xdc);
    emitMemModRM(6, ZERO, big); // FDIV m64fp
    emitD9MemoryGroup(6, ENV_UNMASKED, big); // FNSTENV

    runTestCPU();

    U32 maskedAxStatus = cpu->reg[0].u16 & STATUS_BITS;
    U32 maskedEnvStatus = readEnvValue(ENV_MASKED, 1, big) & STATUS_BITS;
    if (maskedAxStatus != FPU_STATUS_ZE || maskedEnvStatus != FPU_STATUS_ZE) {
        failed("%s masked divide-by-zero status ax=%x env=%x", name, maskedAxStatus, maskedEnvStatus);
    }

    U32 unmaskedEnvStatus = readEnvValue(ENV_UNMASKED, 1, big) & STATUS_BITS;
    if (unmaskedEnvStatus != STATUS_BITS) {
        failed("%s unmasked divide-by-zero status env=%x", name, unmaskedEnvStatus);
    }
}

void runMaskedExceptionUnmaskSummary(bool big, const char* name) {
    constexpr U32 ZERO = MEM_BASE + 0x700;
    constexpr U32 CW_UNMASK_ZE = MEM_BASE + 0x704;
    constexpr U32 STATUS_BITS = FPU_STATUS_ZE | FPU_STATUS_ES;

    begin(big);
    fninit();
    writeF32(MEM_BASE, 1.0f);
    writeF32(ZERO, 0.0f);
    writeI16(CW_UNMASK_ZE, 0x037b);

    fldF32(MEM_BASE, big);
    pushCode8(0xd8);
    emitMemModRM(6, ZERO, big); // FDIV m32fp while ZE is masked
    pushCode8(0xd9);
    emitMemModRM(5, CW_UNMASK_ZE, big); // FLDCW unmasks existing ZE
    fnstswAx();

    runTestCPU();
    if ((cpu->reg[0].u16 & STATUS_BITS) != STATUS_BITS) {
        failed("%s status=%x", name, cpu->reg[0].u16);
    }
    if (!cpu->fpu.divExceptionsUnmasked) {
        failed("%s FLDENV did not mark divide exceptions unmasked", name);
    }
}

void runEnvironmentLoadExceptionSummary(bool big, const char* name) {
    constexpr U32 ENV = MEM_BASE + 0x740;
    constexpr U32 STATUS_BITS = FPU_STATUS_ZE | FPU_STATUS_ES;

    begin(big);
    writeEnvValue(ENV, 0, big, 0x037b); // ZE unmasked
    writeEnvValue(ENV, 1, big, FPU_STATUS_ZE); // pending ZE, stale ES clear
    writeEnvValue(ENV, 2, big, 0xffff); // all tags empty
    for (int i = 3; i < 7; ++i) {
        writeEnvValue(ENV, i, big, 0);
    }

    emitD9MemoryGroup(4, ENV, big); // FLDENV
    fnstswAx();

    runTestCPU();
    if ((cpu->reg[0].u16 & STATUS_BITS) != STATUS_BITS) {
        failed("%s status=%x", name, cpu->reg[0].u16);
    }
}

void runUnmaskedExceptionMaskSummary(bool big, const char* name) {
    constexpr U32 ZERO = MEM_BASE + 0x700;
    constexpr U32 CW_UNMASK_ZE = MEM_BASE + 0x704;
    constexpr U32 CW_MASK_ZE = MEM_BASE + 0x708;
    constexpr U32 STATUS_BITS = FPU_STATUS_ZE | FPU_STATUS_ES;

    begin(big);
    fninit();
    writeF32(MEM_BASE, 1.0f);
    writeF32(ZERO, 0.0f);
    writeI16(CW_UNMASK_ZE, 0x037b);
    writeI16(CW_MASK_ZE, 0x037f);

    fldF32(MEM_BASE, big);
    pushCode8(0xd8);
    emitMemModRM(6, ZERO, big); // FDIV m32fp while ZE is masked
    pushCode8(0xd9);
    emitMemModRM(5, CW_UNMASK_ZE, big); // FLDCW makes pending ZE unmasked and sets ES
    pushCode8(0xd9);
    emitMemModRM(5, CW_MASK_ZE, big); // FLDCW masks pending ZE and clears ES
    fnstswAx();

    runTestCPU();
    if ((cpu->reg[0].u16 & STATUS_BITS) != FPU_STATUS_ZE) {
        failed("%s status=%x", name, cpu->reg[0].u16);
    }
    if (cpu->fpu.divExceptionsUnmasked) {
        failed("%s FLDCW did not mark divide exceptions masked", name);
    }
}

void runEnvironmentLoadMaskedExceptionSummary(bool big, const char* name) {
    constexpr U32 ENV = MEM_BASE + 0x780;
    constexpr U32 STATUS_BITS = FPU_STATUS_ZE | FPU_STATUS_ES;

    begin(big);
    writeEnvValue(ENV, 0, big, 0x037f); // ZE masked
    writeEnvValue(ENV, 1, big, STATUS_BITS); // pending ZE, stale ES set
    writeEnvValue(ENV, 2, big, 0xffff); // all tags empty
    for (int i = 3; i < 7; ++i) {
        writeEnvValue(ENV, i, big, 0);
    }

    emitD9MemoryGroup(4, ENV, big); // FLDENV
    fnstswAx();

    runTestCPU();
    if ((cpu->reg[0].u16 & STATUS_BITS) != FPU_STATUS_ZE) {
        failed("%s status=%x", name, cpu->reg[0].u16);
    }
    if (cpu->fpu.divExceptionsUnmasked) {
        failed("%s FLDENV did not mark divide exceptions masked", name);
    }
}

void runFNSTSWMemory(bool big, float left, float right, U32 expectedStatus, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x600;

    begin(big);
    writeExpected32(0, expectedStatus);
    fninit();
    writeF32(MEM_BASE, left);
    writeF32(MEM_BASE + 4, right);
    fldF32(MEM_BASE + 4, big);
    fldF32(MEM_BASE, big);
    pushCode8(0xd8);
    pushCode8(0xd1); // FCOM ST(1)
    pushCode8(0xdd);
    emitMemModRM(7, OUT, big); // FNSTSW m16

    runTestCPU();
    U32 actualStatus = memory->readw(addressOf(OUT)) & STATUS_MASK;
    if (actualStatus != readExpected32(0)) {
        failed("%s fnstsw memory expected=%x actual=%x", name, readExpected32(0), actualStatus);
    }
}

void runFNSTSWAxAfterDirtyEax(bool big, const char* name) {
    begin(big);
    fninit();
    pushCode8(0xb8);
    if (big) {
        pushCode32(0x12345678); // mov eax, imm32
    } else {
        pushCode16(0x5678); // mov ax, imm16
    }
    fnstswAx();
    pushCode8(0x8b);
    pushCode8(0xd8); // mov ebx/eax or bx/ax

    runTestCPU();
    U32 expected = big ? 0x12340000 : 0;
    if (cpu->reg[3].u32 != expected) {
        failed("%s ebx expected=%x actual=%x", name, expected, cpu->reg[3].u32);
    }
    if (cpu->reg[0].u32 != expected) {
        failed("%s eax expected=%x actual=%x", name, expected, cpu->reg[0].u32);
    }
}

U32 expectedFCOMIFlags(float left, float right) {
    if (std::isnan(left) || std::isnan(right)) {
        return CF | PF | ZF;
    }
    if (left == right) {
        return ZF;
    }
    return left < right ? CF : 0;
}

void runFCOMI(bool big, U8 opcode, U8 group, bool pop, float left, float right, const char* name) {
    begin(big);
    writeExpected32(0, expectedFCOMIFlags(left, right));
    cpu->setFlags(CF | PF | AF | ZF | SF | OF, FMASK_ALL);
    fninit();
    writeF32(MEM_BASE, left);
    writeF32(MEM_BASE + 4, right);
    fldF32(MEM_BASE + 4, big);
    fldF32(MEM_BASE, big);
    pushCode8(opcode);
    pushCode8((U8)(0xc0 | (group << 3) | 1));

    runTestCPU();
    cpu->fillFlags();
    U32 actualFlags = cpu->flags & FMASK_TEST;
    if (actualFlags != readExpected32(0)) {
        failed("%s fcomi flags expected=%x actual=%x", name, readExpected32(0), actualFlags);
    }
    if (cpu->fpu.GetTop() != (pop ? 7 : 6)) {
        failed("%s fcomi stack", name);
    }
}

void runFILD16(bool big, S16 value, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x500;

    begin(big);
    writeExpected32(0, bitsOf((float)value));
    fninit();
    writeI16(MEM_BASE, value);
    fild16(MEM_BASE, big);
    fstTopF32(OUT, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
    if (cpu->fpu.GetTop() != 7) {
        failed("%s fild16 stack", name);
    }
}

void runFILD32(bool big, S32 value, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x620;

    begin(big);
    writeExpected32(0, bitsOf((float)value));
    fninit();
    writeI32(MEM_BASE, value);
    pushCode8(0xdb);
    emitMemModRM(0, MEM_BASE, big);
    fstTopF32(OUT, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
    if (cpu->fpu.GetTop() != 7) {
        failed("%s fild32 stack", name);
    }
}

void runFILD64(bool big, S64 value, const char* name) {
    U64 expectedLow = 0;
    U16 expectedHigh = 0;
    expectedI64Ext80(value, &expectedLow, &expectedHigh);

    begin(big);
    writeExpected64(0, expectedLow);
    writeExpected32(1, expectedHigh);
    fninit();
    writeI64(MEM_BASE, value);
    fild64(MEM_BASE, big);

    runTestCPU();
    U64 low = 0;
    U64 high = 0;
    cpu->fpu.ST80(cpu->fpu.STV(0), &low, &high);
    if (low != readExpected64(0) || (U16)high != (U16)readExpected32(1) || cpu->fpu.GetTop() != 7) {
        failed("%s fild64 value", name);
    }
}

void runFIST32(bool big, U8 group, float value, U32 expected, bool pop, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x640;

    begin(big);
    writeExpected32(0, expected);
    fninit();
    writeF32(MEM_BASE, value);
    fldF32(MEM_BASE, big);
    pushCode8(0xdb);
    emitMemModRM(group, OUT, big);

    runTestCPU();
    if (readI32(OUT) != readExpected32(0) || cpu->fpu.GetTop() != (pop ? 0 : 7)) {
        failed("%s fist32 result", name);
    }
}

void runFISTTP32(bool big, double value, U32 expected, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x660;

    begin(big);
    writeExpected32(0, expected);
    fninit();
    writeF64(MEM_BASE, value);
    fldF64(MEM_BASE, big);
    pushCode8(0xdb);
    emitMemModRM(1, OUT, big);

    runTestCPU();
    if (readI32(OUT) != readExpected32(0) || cpu->fpu.GetTop() != 0 || cpu->fpu.GetTag(cpu, 7) != TAG_Empty) {
        failed("%s fisttp32 result", name);
    }
}

void runFNCLEX(bool big, const char* name) {
    begin(big);
    cpu->fpu.sw = 0xffff;
    pushCode8(0xdb);
    pushCode8(0xe2);

    runTestCPU();
    if ((cpu->fpu.sw & 0xffff) != 0x7f00) {
        failed("%s status word", name);
    }
}

void runFIST16(bool big, U8 group, float value, U16 expected, bool pop, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x540;

    begin(big);
    writeExpected32(0, expected);
    fninit();
    writeF32(MEM_BASE, value);
    fldF32(MEM_BASE, big);
    pushCode8(0xdf);
    emitMemModRM(group, OUT, big);

    runTestCPU();
    if (readI16(OUT) != (U16)readExpected32(0) || cpu->fpu.GetTop() != (pop ? 0 : 7)) {
        failed("%s fist16 result", name);
    }
}

void runFISTP64(bool big, S64 value, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x560;

    begin(big);
    writeExpected64(0, (U64)value);
    fninit();
    writeI64(MEM_BASE, value);
    fild64(MEM_BASE, big);
    pushCode8(0xdf);
    emitMemModRM(7, OUT, big);

    runTestCPU();
    if (readI64(OUT) != readExpected64(0) || cpu->fpu.GetTop() != 0 || cpu->fpu.GetTag(cpu, 7) != TAG_Empty) {
        failed("%s fistp64 result", name);
    }
}

#ifdef BOXEDWINE_WASM_JIT
void runFISTP64FromDouble(bool big, double value, U64 expected, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x580;

    begin(big);
    writeExpected64(0, expected);
    fninit();
    writeF64(MEM_BASE, value);
    fldF64(MEM_BASE, big);
    pushCode8(0xdf);
    emitMemModRM(7, OUT, big);

    runTestCPU();
    if (readI64(OUT) != readExpected64(0) || cpu->fpu.GetTop() != 0 || cpu->fpu.GetTag(cpu, 7) != TAG_Empty) {
        failed("%s fistp64 result", name);
    }
}
#endif

void runDDRegisterOps(bool big, const char* name) {
    constexpr U32 OUT = MEM_BASE + 0x680;

    begin(big);
    writeExpected32(0, bitsOf(2.0f));
    fninit();
    writeF32(MEM_BASE, 4.0f);
    writeF32(MEM_BASE + 4, 2.0f);
    fldF32(MEM_BASE, big);
    fldF32(MEM_BASE + 4, big);
    pushCode8(0xdd);
    pushCode8(0xd1); // FST ST(1)
    pushCode8(0xdd);
    pushCode8(0xe1); // FUCOM ST(1)
    pushCode8(0xdd);
    pushCode8(0xe9); // FUCOMP ST(1)
    fstTopF32(OUT, big);

    runTestCPU();
    assertFloatCloseExpected(readF32(OUT), 0, name);
    if (cpu->fpu.GetTop() != 7) {
        failed("%s register operation stack", name);
    }
}

void runFFREE(bool big, const char* name) {
    begin(big);
    fninit();
    writeF32(MEM_BASE, 1.0f);
    writeF32(MEM_BASE + 4, 2.0f);
    fldF32(MEM_BASE, big);
    fldF32(MEM_BASE + 4, big);
    pushCode8(0xdd);
    pushCode8(0xc1); // FFREE ST(1)

    runTestCPU();
    if (cpu->fpu.tags[(cpu->fpu.GetTop() + 1) & 7] != TAG_Empty) {
        failed("%s tag", name);
    }
}

void runFFREEP(bool big, const char* name) {
    begin(big);
    fninit();
    writeF32(MEM_BASE, 1.0f);
    fldF32(MEM_BASE, big);
    pushCode8(0xdf);
    pushCode8(0xc0); // FFREEP ST(0)

    runTestCPU();
    if (cpu->fpu.GetTop() != 0 || cpu->fpu.GetTag(cpu, 7) != TAG_Empty) {
        failed("%s pop state", name);
    }
}

void runD8(bool big) {
    for (size_t i = 0; i < sizeof(F32_CASES) / sizeof(F32_CASES[0]); ++i) {
        const F32ArithCase& data = F32_CASES[i];
        runD8MemoryArith(big, 0, data.add, data, "fadd m32fp d8");
        runD8MemoryArith(big, 1, data.mul, data, "fmul m32fp d8");
        runD8MemoryArith(big, 4, data.sub, data, "fsub m32fp d8");
        runD8MemoryArith(big, 5, data.subr, data, "fsubr m32fp d8");
        runD8MemoryArith(big, 6, data.div, data, "fdiv m32fp d8");
        runD8MemoryArith(big, 7, data.divr, data, "fdivr m32fp d8");
        runD8RegisterArith(big, 0, data.add, data, "fadd st0,sti d8");
        runD8RegisterArith(big, 1, data.mul, data, "fmul st0,sti d8");
        runD8RegisterArith(big, 4, data.sub, data, "fsub st0,sti d8");
        runD8RegisterArith(big, 5, data.subr, data, "fsubr st0,sti d8");
    }
    runD8Compare(big, false, 2.0f, 2.0f, "fcom m32fp d8");
    runD8Compare(big, false, 2.0f, 1.0f, "fcom m32fp d8");
    runD8Compare(big, false, 2.0f, 3.0f, "fcom m32fp d8");
    runD8Compare(big, true, 2.0f, 2.0f, "fcomp m32fp d8");
    runD8CompareBits(big, false, F32_QNAN, 0x40000000, "fcom m32fp d8 unordered");
    runD8CompareBits(big, false, F32_POS_INF, 0x40000000, "fcom m32fp d8 infinity");
    runD8CompareBits(big, false, F32_NEG_INF, 0x40000000, "fcom m32fp d8 infinity");
    runD8CompareBits(big, true, F32_QNAN, F32_QNAN, "fcomp m32fp d8 unordered");
}

void runD9(bool big) {
    for (size_t i = 0; i < sizeof(FST_CASES) / sizeof(FST_CASES[0]); ++i) {
        runFSTFloatBits(big, false, FST_CASES[i], "fst m32fp d9");
        runFSTFloatBits(big, true, FST_CASES[i], "fstp m32fp d9");
    }
    runD9StackOps(big, "fpu stack d9");
    runD9Control(big, "fpu control d9");
    runD9StackControl(big, "fpu stack control d9");
    runD9FldSt0(big, "fld st0 d9");
    runFTST(big, 0.0f, FPU_EQUAL, "ftst d9");
    runFTST(big, -2.0f, FPU_LESS, "ftst d9");
    runFTST(big, 3.0f, FPU_GREATER, "ftst d9");
    for (size_t i = 0; i < sizeof(FCHS_CASES) / sizeof(FCHS_CASES[0]); ++i) {
        runD9UnaryBits(big, 0, FCHS_CASES[i].input, FCHS_CASES[i].expected, "fchs d9");
    }
    for (size_t i = 0; i < sizeof(FABS_CASES) / sizeof(FABS_CASES[0]); ++i) {
        runD9UnaryBits(big, 1, FABS_CASES[i].input, FABS_CASES[i].expected, "fabs d9");
    }
    runD9Constant(big, 0, 1.0f, "fld1 d9");
    runD9Constant(big, 1, 3.3219280948873623f, "fldl2t d9");
    runD9Constant(big, 2, 1.4426950408889634f, "fldl2e d9");
    runD9Constant(big, 3, 3.14159265f, "fldpi d9");
    runD9Constant(big, 4, 0.3010299956639812f, "fldlg2 d9");
    runD9Constant(big, 5, 0.6931471805599453f, "fldln2 d9");
    runD9Constant(big, 6, 0.0f, "fldz d9");
    runFPUEnvironmentLoad(big, "fldenv d9");
    runFPUEnvironmentStore(big, "fnstenv d9");
    runMaskedExceptionUnmaskSummary(big, "fldcw x87 exception summary d9");
    runEnvironmentLoadExceptionSummary(big, "fldenv x87 exception summary d9");
    runUnmaskedExceptionMaskSummary(big, "fldcw x87 exception summary clear d9");
    runEnvironmentLoadMaskedExceptionSummary(big, "fldenv x87 exception summary clear d9");
    runMaskedDivideByZeroStatus(big, "masked x87 exception d9");
    runD9RoundSqrtScale(big, "sqrt round scale d9");
    runFSQRTBits(big, 0x40800000, 0x40000000, "fsqrt d9");
    runFSQRTBits(big, 0x41800000, 0x40800000, "fsqrt d9");
    runFSQRTBits(big, F32_POS_ZERO, F32_POS_ZERO, "fsqrt d9");
    runFSQRTBits(big, F32_POS_INF, F32_POS_INF, "fsqrt d9");
    runFSQRTBits(big, 0xbf800000, F32_QNAN, "fsqrt d9");
    runFRNDINTBits(big, 0x40266666, 0x40400000, "frndint d9");
    runFRNDINTBits(big, 0xc0266666, 0xc0400000, "frndint d9");
    runFRNDINTBits(big, 0x4019999a, 0x40000000, "frndint d9");
    runFRNDINTBits(big, 0xc019999a, 0xc0000000, "frndint d9");
    runFRNDINTBits(big, F32_POS_ZERO, F32_POS_ZERO, "frndint d9");
    runFRNDINTBits(big, F32_NEG_ZERO, F32_NEG_ZERO, "frndint d9");
    runFSCALEBits(big, 0x40400000, 0x40000000, 0x41400000, "fscale d9");
    runFSCALEBits(big, 0x40800000, 0xc0000000, 0x3f800000, "fscale d9");
    runFSCALEBits(big, 0x3fc00000, 0x40400000, 0x41400000, "fscale d9");
}

void runDA(bool big) {
    runFpuCmov(big, 0, CF, "fcmovb da");
    runFpuCmov(big, 1, ZF, "fcmove da");
    runFpuCmov(big, 2, CF | ZF, "fcmovbe da");
    runFpuCmov(big, 3, PF, "fcmovu da");
    runFpuCmovOpcode(big, 0xdb, 0, 0, "fcmovnb db");
    runFpuCmovOpcode(big, 0xdb, 1, 0, "fcmovne db");
    runFpuCmovOpcode(big, 0xdb, 2, 0, "fcmovnbe db");
    runFpuCmovOpcode(big, 0xdb, 3, 0, "fcmovnu db");
    runFUCOMPP(big, 2.0f, 2.0f, FPU_EQUAL, "fucompp da");
    runFUCOMPP(big, 2.0f, 1.0f, FPU_LESS, "fucompp da");
    runFUCOMPP(big, 2.0f, 3.0f, FPU_GREATER, "fucompp da");
    runFUCOMPP(big, floatFromBits(F32_QNAN), 3.0f, FPU_UNORDERED, "fucompp da unordered");
    runDAIntegerArith(big, 0, 100000, 0x47c35500, "fiadd m32int da");
    runDAIntegerArith(big, 0, -20, 0xc1200000, "fiadd m32int da");
    runDAIntegerArith(big, 1, 100000, 0x49742400, "fimul m32int da");
    runDAIntegerArith(big, 1, -3, 0xc1f00000, "fimul m32int da");
    runDAIntegerArith(big, 4, 7, 0x40400000, "fisub m32int da");
    runDAIntegerArith(big, 4, -7, 0x41880000, "fisub m32int da");
    runDAIntegerArith(big, 5, 7, 0xc0400000, "fisubr m32int da");
    runDAIntegerArith(big, 5, -7, 0xc1880000, "fisubr m32int da");
    runDAIntegerArith(big, 6, 2, 0x40a00000, "fidiv m32int da");
    runDAIntegerArith(big, 6, -2, 0xc0a00000, "fidiv m32int da");
    runDAIntegerArith(big, 7, 20, 0x40000000, "fidivr m32int da");
    runDAIntegerArith(big, 7, -20, 0xc0000000, "fidivr m32int da");
    runDAIntegerCompare(big, false, 0x40000000, 2, FPU_EQUAL, "ficom m32int da");
    runDAIntegerCompare(big, false, 0xc0400000, 2, FPU_LESS, "ficom m32int da");
    runDAIntegerCompare(big, false, 0x40400000, -2, FPU_GREATER, "ficom m32int da");
    runDAIntegerCompare(big, true, 0x3f800000, 20000, FPU_LESS, "ficomp m32int da");
}

void runDB(bool big) {
    runFILD32(big, 0, "fild m32int db");
    runFILD32(big, 123456, "fild m32int db");
    runFILD32(big, -234567, "fild m32int db");
    runFIST32(big, 2, 12345.0f, 12345, false, "fist m32int db");
    runFIST32(big, 2, -12345.0f, (U32)(S32)-12345, false, "fist m32int db");
    runFIST32(big, 3, 98765.0f, 98765, true, "fistp m32int db");
    runFIST32(big, 3, -98765.0f, (U32)(S32)-98765, true, "fistp m32int db");
    runFISTTP32(big, 12345.9, 12345, "fisttp m32int db");
    runFISTTP32(big, -12345.9, (U32)(S32)-12345, "fisttp m32int db");
#ifdef BOXEDWINE_WASM_JIT
    runFIST32(big, 2, std::numeric_limits<float>::infinity(), 0x80000000u, false, "fist m32int db infinity");
    runFIST32(big, 3, std::numeric_limits<float>::quiet_NaN(), 0x80000000u, true, "fistp m32int db nan");
    runFISTTP32(big, 2147483647.0, 0x7fffffffu, "fisttp m32int db upper valid");
    runFISTTP32(big, 2147483648.0, 0x80000000u, "fisttp m32int db positive overflow");
    runFISTTP32(big, -2147483649.0, 0x80000000u, "fisttp m32int db negative overflow");
    runFISTTP32(big, std::numeric_limits<double>::quiet_NaN(), 0x80000000u, "fisttp m32int db nan");
#endif
    runFCOMI(big, 0xdb, 5, false, 2.0f, 2.0f, "fucomi db");
    runFCOMI(big, 0xdb, 5, false, 2.0f, 3.0f, "fucomi db");
    runFCOMI(big, 0xdb, 6, false, 3.0f, 2.0f, "fcomi db");
    runFCOMI(big, 0xdb, 6, false, floatFromBits(F32_QNAN), 2.0f, "fcomi db unordered");
    runFNCLEX(big, "fnclex db");
}

void runDC(bool big) {
    runDCDoubleArith(big, 0, 10.0, 2.0, 12.0, "fadd m64fp dc");
    runDCDoubleArith(big, 1, 10.0, 2.0, 20.0, "fmul m64fp dc");
    runDCDoubleArith(big, 4, 10.0, 2.0, 8.0, "fsub m64fp dc");
    runDCDoubleArith(big, 5, 10.0, 2.0, -8.0, "fsubr m64fp dc");
    runDCDoubleArith(big, 6, 10.0, 2.0, 5.0, "fdiv m64fp dc");
    runDCDoubleArith(big, 7, 10.0, 2.0, 0.2, "fdivr m64fp dc");
    runDCDoubleCompare(big, false, 2.0, 2.0, FPU_EQUAL, "fcom m64fp dc");
    runDCDoubleCompare(big, true, 2.0, 3.0, FPU_LESS, "fcomp m64fp dc");
    runDCRegisterArith(big, 0, 10.0, 2.0, 12.0, "fadd sti,st0 dc");
    runDCRegisterArith(big, 1, 10.0, 2.0, 20.0, "fmul sti,st0 dc");
    runDCRegisterArith(big, 4, 10.0, 2.0, 8.0, "fsubr sti,st0 dc");
    runDCRegisterArith(big, 5, 10.0, 2.0, -8.0, "fsub sti,st0 dc");
    runDCRegisterArith(big, 6, 10.0, 2.0, 5.0, "fdivr sti,st0 dc");
    runDCRegisterArith(big, 7, 10.0, 2.0, 0.2, "fdiv sti,st0 dc");
    runMaskedDoubleDivideByZeroStatus(big, "masked x87 double exception dc");
}

void runDD(bool big) {
    runDDDoubleStore(big, false, 123456.5, "fst m64fp dd");
    runDDDoubleStore(big, true, -123456.5, "fstp m64fp dd");
    runDDDoubleStore(big, false, 0.0, "fst m64fp dd");
    runDDDoubleStore(big, true, -0.0, "fstp m64fp dd");
    runDDDoubleStore(big, false, std::numeric_limits<double>::infinity(), "fst m64fp dd");
    runDDDoubleStore(big, true, -std::numeric_limits<double>::infinity(), "fstp m64fp dd");
    runFISTTP64(big, 123456789.6, 123456789, "fisttp m64int dd");
    runFISTTP64(big, -123456789.4, (U64)(S64)-123456789, "fisttp m64int dd");
    runFISTTP64(big, 0.0, 0, "fisttp m64int dd");
    runFISTTP64(big, -0.0, 0, "fisttp m64int dd");
#ifdef BOXEDWINE_WASM_JIT
    runFISTTP64(big, 0x1.fffffffffffffp+62, 0x7ffffffffffffc00ULL, "fisttp m64int dd upper valid");
    runFISTTP64(big, 0x1p+63, 0x8000000000000000ULL, "fisttp m64int dd positive overflow");
    runFISTTP64(big, std::nextafter(-0x1p+63, -std::numeric_limits<double>::infinity()), 0x8000000000000000ULL, "fisttp m64int dd negative overflow");
    runFISTTP64(big, std::numeric_limits<double>::quiet_NaN(), 0x8000000000000000ULL, "fisttp m64int dd nan");
    runFISTP64FromDouble(big, std::numeric_limits<double>::infinity(), 0x8000000000000000ULL, "fistp m64int df infinity");
    runFISTP64FromDouble(big, std::numeric_limits<double>::quiet_NaN(), 0x8000000000000000ULL, "fistp m64int df nan");
#endif
    runFNSTSWMemory(big, 2.0f, 2.0f, FPU_EQUAL, "fnstsw m16 dd");
    runFNSTSWMemory(big, 2.0f, 3.0f, FPU_LESS, "fnstsw m16 dd");
    runFNSTSWMemory(big, floatFromBits(F32_QNAN), 3.0f, FPU_UNORDERED, "fnstsw m16 dd unordered");
    runDDRegisterOps(big, "fpu register ops dd");
    runFFREE(big, "ffree dd");
}

void runDE(bool big) {
    runDEWordArith(big, 0, 2, 0x41400000, "fiadd m16int de");
    runDEWordArith(big, 1, 3, 0x41f00000, "fimul m16int de");
    runDEWordArith(big, 4, 7, 0x40400000, "fisub m16int de");
    runDEWordArith(big, 5, 7, 0xc0400000, "fisubr m16int de");
    runDEWordArith(big, 6, 2, 0x40a00000, "fidiv m16int de");
    runDEWordArith(big, 7, 20, 0x40000000, "fidivr m16int de");
    runDEWordCompare(big, false, 0x40000000, 2, FPU_EQUAL, "ficom m16int de");
    runDEWordCompare(big, true, 0x3f800000, 2, FPU_LESS, "ficomp m16int de");
    runDERegisterComparePop(big, "fcomp sti de");
    runDERegisterArith(big, 0, 10.0, 2.0, 12.0, "faddp de");
    runDERegisterArith(big, 1, 10.0, 2.0, 20.0, "fmulp de");
    runDERegisterArith(big, 4, 10.0, 2.0, 8.0, "fsubrp de");
    runDERegisterArith(big, 5, 10.0, 2.0, -8.0, "fsubp de");
    runDERegisterArith(big, 6, 10.0, 2.0, 5.0, "fdivrp de");
    runDERegisterArith(big, 7, 10.0, 2.0, 0.2, "fdivp de");
}

void runDF(bool big) {
    runFILD16(big, 0, "fild m16int df");
    runFILD16(big, 12345, "fild m16int df");
    runFILD16(big, -10235, "fild m16int df");
    runFILD64(big, 0, "fild m64int df");
    runFILD64(big, 1, "fild m64int df");
    runFILD64(big, -1, "fild m64int df");
    runFILD64(big, 0x123456789abcdef0ll, "fild m64int df");
    runFILD64(big, (S64)0x8000000000000000ull, "fild m64int df");
    runFIST16(big, 1, 0.0f, 0, true, "fisttp m16int df");
    runFIST16(big, 1, 1.9f, 1, true, "fisttp m16int df");
    runFIST16(big, 1, -1.9f, (U16)-1, true, "fisttp m16int df");
    runFIST16(big, 2, 0.0f, 0, false, "fist m16int df");
    runFIST16(big, 2, -1024.4f, (U16)-1024, false, "fist m16int df");
    runFIST16(big, 3, 1.9f, 2, true, "fistp m16int df");
    runFIST16(big, 3, -1024.9f, (U16)-1025, true, "fistp m16int df");
    runFISTP64(big, 0, "fistp m64int df");
    runFISTP64(big, 1, "fistp m64int df");
    runFISTP64(big, 0x123456789abcdef0ll, "fistp m64int df");
    runFISTP64(big, -2, "fistp m64int df");
    runFCOMI(big, 0xdf, 5, true, 2.0f, 2.0f, "fucomip df");
    runFCOMI(big, 0xdf, 5, true, 2.0f, 3.0f, "fucomip df");
    runFCOMI(big, 0xdf, 6, true, 3.0f, 2.0f, "fcomip df");
    runFCOMI(big, 0xdf, 6, true, floatFromBits(F32_QNAN), 2.0f, "fcomip df unordered");
    runFNSTSWAxAfterDirtyEax(big, "fnstsw ax after dirty eax df");
    runFFREEP(big, "ffreep df");
}

}

void testFpuD8_0x0d8() { runD8(false); }
void testFpuD8_0x2d8() { runD8(true); }
void testFpuD9_0x0d9() { runD9(false); }
void testFpuD9_0x2d9() { runD9(true); }
void testFpuDA_0x0da() { runDA(false); }
void testFpuDA_0x2da() { runDA(true); }
void testFpuDB_0x0db() { runDB(false); }
void testFpuDB_0x2db() { runDB(true); }
void testFpuDC_0x0dc() { runDC(false); }
void testFpuDC_0x2dc() { runDC(true); }
void testFpuDD_0x0dd() { runDD(false); }
void testFpuDD_0x2dd() { runDD(true); }
void testFpuDE_0x0de() { runDE(false); }
void testFpuDE_0x2de() { runDE(true); }
void testFpuDF_0x0df() { runDF(false); }
void testFpuDF_0x2df() { runDF(true); }

#endif
