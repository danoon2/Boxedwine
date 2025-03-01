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

#ifdef __TEST
void assertTrue(int b);
extern KMemory* memory;

#if defined(BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
#include <nmmintrin.h>
#endif
#include "testCPU.h"
#include "testFPU.h"
#include "testMMX.h"

#include <math.h>
const float fPI =         3.14159265f;
const float squareRoot3 = 1.73205081f;

const U32 FLOAT_POSITIVE_INFINITY_BITS = 0x7f800000;
const U32 FLOAT_NEGATIVE_INFINITY_BITS = 0xff800000;
const U32 FLOAT_QUIET_NAN_BITS = 0x7fc00000;

const U64 DOUBLE_QUIET_NAN_BITS = 0x7FF8000000000000;

static struct Test_Float fInf = { FLOAT_POSITIVE_INFINITY_BITS };
static struct Test_Float fNegInf = { FLOAT_NEGATIVE_INFINITY_BITS };
static struct Test_Float fNan = { FLOAT_QUIET_NAN_BITS };
static struct TestDouble dNan = { DOUBLE_QUIET_NAN_BITS };

const float POSITIVE_INFINITY = fInf.f;
const float NEGATIVE_INFINITY = fNegInf.f;
const float TEST_NAN = fNan.f;
const double TEST_NAN_DOUBLE = dNan.d;

#ifdef BOXEDWINE_MSVC
#include <float.h>
//#define isnan(x) _isnan(x)
//#define isinf(x) (!_finite(x))
#endif

PACKED(
    struct F80Register {
    U64 low;
    U16 high;
}
);

PACKED(
    struct X87Register {
    union {
        U64 q;
        U8 b[10];
        F80Register f;
    };
}
);

struct FSaveState {
    U16 fcw;
    U16 reserved_1;
    U16 fsw;
    U16 reserved_2;
    U16 ftw;
    U16 reserved_3;
    U32 fip;
    U32 fop;
    U32 fd;
    U32 fds;
    X87Register st[8];
};

struct FSaveState16 {
    U16 fcw;
    U16 fsw;
    U16 ftw;
    U16 fpu_ip;
    U16 fop;
    U16 fpu_dp;
    U16 fpu_ds;
    X87Register st[8];
};

struct FPU_Float {
    union {
        float f;
        U32   i;
    };
};

struct FPU_Double {
    union {
        double d;
        U64   l;
    };
};

void assertTrueF(float f1, float f2) {
    FPU_Float bits1;
    FPU_Float bits2;
    bits1.f = f1;
    bits2.f = f2;
    if (bits1.i == bits2.i) {
        return;
    }
    if (f1 == f2) {
        return;
    }
    float diff = f1 > f2 ? f1 - f2 : f2 - f1;
    if (diff < .000001f) {
        return;
    }
    float e = f1 > f2 ? f2 / 100000.0f : f1 / 100000.0f;
    if (diff < e) {
        return;
    }
    assertTrue(0);
}

void assertTrueD(double d1, double d2) {
    FPU_Double bits1;
    FPU_Double bits2;
    bits1.d = d1;
    bits2.d = d2;
    if (bits1.l == bits2.l) {
        return;
    }
    if (d1 == d2) {
        return;
    }
    double diff = d1 > d2 ? d1 - d2 : d2 - d1;
    if (diff < .0000001) {
        return;
    }
    double e = d1 > d2 ? d2 / 100000.0 : d1 / 100000.0;
    if (e < 0) {
        e = -e;
    }
    if (diff < e) {
        return;
    }
    assertTrue(0);
}

static U8 rm(int ea, int group, int sub) {
    int result = (group & 7) << 3 | (sub & 7);
    if (!ea)
        result |= 0xC0;
    return (U8)result;
}

void writeFPUStatusToAX() {
    pushCode8(0xdf);
    pushCode8(rm(false, 4, 0));
}

int getFPUStackPosFromAX() {
    return (AX & 0x3800) >> 11;
}

void writeTopFloat(int index, bool pop = false) {
    pushCode8(0xd9);
    pushCode8(rm(true, pop?3:2, cpu->big ? 5 : 6));
    if (cpu->big)
        pushCode32(4 * index);
    else
        pushCode16(4 * index);
}

void writeTopDouble(int index, bool pop = false) {
    pushCode8(0xdd);
    pushCode8(rm(true, pop ? 3 : 2, cpu->big ? 5 : 6));
    if (cpu->big)
        pushCode32(4 * index);
    else
        pushCode16(4 * index);
}

void writeF(float f, int index) {
    struct FPU_Float value;
    value.f = f;
    memory->writed(HEAP_ADDRESS + 4 * index, value.i);
}

void writeD(double d, int index) {
    struct FPU_Double value;
    value.d = d;
    memory->writeq(HEAP_ADDRESS + 4 * index, value.l);
}

void fldf32(float f, int index) {
    int rm = 0;
    if (cpu->big)
        rm += 5;
    else
        rm += 6;
    pushCode8(0xd9);
    pushCode8(rm);
    if (cpu->big)
        pushCode32(4 * index);
    else
        pushCode16(4 * index);
    writeF(f, index);
}

void fld64(double d, int index) {
    int rm = 0;
    if (cpu->big)
        rm += 5;
    else
        rm += 6;
    pushCode8(0xdd);
    pushCode8(rm);
    if (cpu->big)
        pushCode32(4 * index);
    else
        pushCode16(4 * index);
    writeD(d, index);
}

void fild64(U64 value, int index) {
    int rm = 0;
    if (cpu->big)
        rm += 0x2d;
    else
        rm += 0x2e;
    pushCode8(0xdf);
    pushCode8(rm);
    if (cpu->big)
        pushCode32(4 * index);
    else
        pushCode16(4 * index);
    memory->writeq(HEAP_ADDRESS + 4 * index, value);
}

void fpu_init() {
    pushCode8(0xdb);
    pushCode8(rm(false, 4, 3));
}

void doF32Instruction(int op1, int group1, int op2, int group2, float x, float y, float r) {
    newInstruction(0);

    fpu_init();

    fldf32(x, 1);
    writeF(y, 2);

    if (op1 > 0xFF)
        pushCode8(0x0F);
    pushCode8(op1 & 0xFF);

    pushCode8(rm(true, group1, cpu->big ? 5 : 6));
    if (cpu->big)
        pushCode32(4 * 2);
    else
        pushCode16(4 * 2);
    writeTopFloat(3);
    writeFPUStatusToAX();
    runTestCPU();
    struct FPU_Float result;
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);

    assertTrue((isnan(result.f) && isnan(r)) || result.f == r);
    assertTrue(getFPUStackPosFromAX() == 7); // nothing was popped

    newInstruction(0);
    fpu_init();
    fldf32(y, 1);
    fldf32(x, 2);
    if (op2 > 0xFF)
        pushCode8(0x0F);
    pushCode8(op2 & 0xFF);
    pushCode8(rm(false, group2, 1));
    writeTopFloat(3);
    writeFPUStatusToAX();
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue((isnan(result.f) && isnan(r)) || result.f == r);
    assertTrue(getFPUStackPosFromAX() == 6); // nothing was popped
}

void F32Add(float x, float y, float r) {
    doF32Instruction(0xd8, 0, 0xd8, 0, x, y, r);
}

void doF32Add() {
    F32Add(0.0f, 0.0f, 0.0f);
    F32Add(-0.0f, 0.0f, 0.0f);
    F32Add(0.0f, -0.0f, 0.0f);

    F32Add(0.0f, POSITIVE_INFINITY, POSITIVE_INFINITY);
    F32Add(POSITIVE_INFINITY, 0.0f, POSITIVE_INFINITY);
    F32Add(0.0f, NEGATIVE_INFINITY, NEGATIVE_INFINITY);
    F32Add(NEGATIVE_INFINITY, 0.0f, NEGATIVE_INFINITY);
    F32Add(NEGATIVE_INFINITY, POSITIVE_INFINITY, TEST_NAN);
    F32Add(POSITIVE_INFINITY, 1.0f, POSITIVE_INFINITY);
    F32Add(POSITIVE_INFINITY, 2.0f, POSITIVE_INFINITY);
    F32Add(POSITIVE_INFINITY, POSITIVE_INFINITY, POSITIVE_INFINITY);

    F32Add(TEST_NAN, 2.0f, TEST_NAN);
    F32Add(TEST_NAN, TEST_NAN, TEST_NAN);
    F32Add(-2.0f, TEST_NAN, TEST_NAN);

    F32Add(0.0f, 1.0f, 1.0f);
    F32Add(1.0f, 0.0f, 1.0f);
    F32Add(0.0f, -1.0f, -1.0f);
    F32Add(-1.0f, 0.0f, -1.0f);
    F32Add(-1.0f, 1.0f, 0.0f);
    F32Add(1.0f, -1.0f, 0.0f);
    F32Add(-1.0f, -1.0f, -2.0f);
    F32Add(1.0f, 1.0f, 2.0f);

    F32Add(100.01f, 0.001f, 100.011f);
}

void testF32Add() {
    doF32Add();
}

void F32Sub(float x, float y, float r) {
    doF32Instruction(0xd8, 4, 0xd8, 4, x, y, r);
}

void doF32Sub() {
    F32Sub(0.0f, 0.0f, 0.0f);
    F32Sub(-0.0f, 0.0f, 0.0f);
    F32Sub(0.0f, -0.0f, 0.0f);

    F32Sub(0.0f, POSITIVE_INFINITY, NEGATIVE_INFINITY);
    F32Sub(POSITIVE_INFINITY, 0.0f, POSITIVE_INFINITY);
    F32Sub(0.0f, NEGATIVE_INFINITY, POSITIVE_INFINITY);
    F32Sub(NEGATIVE_INFINITY, 0.0f, NEGATIVE_INFINITY);
    F32Sub(NEGATIVE_INFINITY, POSITIVE_INFINITY, NEGATIVE_INFINITY);
    F32Sub(POSITIVE_INFINITY, 1.0f, POSITIVE_INFINITY);
    F32Sub(POSITIVE_INFINITY, 2.0f, POSITIVE_INFINITY);
    F32Sub(POSITIVE_INFINITY, POSITIVE_INFINITY, TEST_NAN);

    F32Sub(TEST_NAN, 2.0f, TEST_NAN);
    F32Sub(TEST_NAN, TEST_NAN, TEST_NAN);
    F32Sub(-2.0f, TEST_NAN, TEST_NAN);

    F32Sub(0.0f, 1.0f, -1.0f);
    F32Sub(1.0f, 0.0f, 1.0f);
    F32Sub(0.0f, -1.0f, 1.0f);
    F32Sub(-1.0f, 0.0f, -1.0f);
    F32Sub(-1.0f, 1.0f, -2.0f);
    F32Sub(1.0f, -1.0f, 2.0f);
    F32Sub(-1.0f, -1.0f, 0.0f);
    F32Sub(1.0f, 1.0f, 0.0f);

    F32Sub(100.01f, 0.001f, 100.009f);
}

void testF32Sub() {
    doF32Sub();
}

void F32SubR(float x, float y, float r) {
    doF32Instruction(0xd8, 5, 0xd8, 5, x, y, r);
}

void doF32SubR() {
    F32SubR(0.0f, 0.0f, 0.0f);
    F32SubR(-0.0f, 0.0f, 0.0f);
    F32SubR(0.0f, -0.0f, 0.0f);

    F32SubR(0.0f, POSITIVE_INFINITY, POSITIVE_INFINITY);
    F32SubR(POSITIVE_INFINITY, 0.0f, NEGATIVE_INFINITY);
    F32SubR(0.0f, NEGATIVE_INFINITY, NEGATIVE_INFINITY);
    F32SubR(NEGATIVE_INFINITY, 0.0f, POSITIVE_INFINITY);
    F32SubR(NEGATIVE_INFINITY, POSITIVE_INFINITY, POSITIVE_INFINITY);
    F32SubR(POSITIVE_INFINITY, 1.0f, NEGATIVE_INFINITY);
    F32SubR(POSITIVE_INFINITY, 2.0f, NEGATIVE_INFINITY);
    F32SubR(POSITIVE_INFINITY, POSITIVE_INFINITY, TEST_NAN);

    F32SubR(TEST_NAN, 2.0f, TEST_NAN);
    F32SubR(TEST_NAN, TEST_NAN, TEST_NAN);
    F32SubR(-2.0f, TEST_NAN, TEST_NAN);

    F32SubR(0.0f, 1.0f, 1.0f);
    F32SubR(1.0f, 0.0f, -1.0f);
    F32SubR(0.0f, -1.0f, -1.0f);
    F32SubR(-1.0f, 0.0f, 1.0f);
    F32SubR(-1.0f, 1.0f, 2.0f);
    F32SubR(1.0f, -1.0f, -2.0f);
    F32SubR(-1.0f, -1.0f, 0.0f);
    F32SubR(1.0f, 1.0f, 0.0f);

    F32SubR(100.01f, 0.001f, -100.009f);
}

void testF32SubR() {
    doF32SubR();
}

void F32Mul(float x, float y, float r) {
    doF32Instruction(0xd8, 1, 0xd8, 1, x, y, r);
}

void doF32Mul() {
    F32Mul(0.0f, 0.0f, 0.0f);
    F32Mul(-0.0f, 0.0f, 0.0f);
    F32Mul(0.0f, -0.0f, 0.0f);

    F32Mul(0.0f, POSITIVE_INFINITY, TEST_NAN);
    F32Mul(POSITIVE_INFINITY, 0.0f, TEST_NAN);
    F32Mul(0.0f, NEGATIVE_INFINITY, TEST_NAN);
    F32Mul(NEGATIVE_INFINITY, 0.0f, TEST_NAN);
    F32Mul(NEGATIVE_INFINITY, POSITIVE_INFINITY, NEGATIVE_INFINITY);
    F32Mul(POSITIVE_INFINITY, 1.0f, POSITIVE_INFINITY);
    F32Mul(POSITIVE_INFINITY, 2.0f, POSITIVE_INFINITY);
    F32Mul(POSITIVE_INFINITY, POSITIVE_INFINITY, POSITIVE_INFINITY);

    F32Mul(TEST_NAN, 2.0f, TEST_NAN);
    F32Mul(TEST_NAN, TEST_NAN, TEST_NAN);
    F32Mul(-2.0f, TEST_NAN, TEST_NAN);

    F32Mul(0.0f, 1.0f, 0.0f);
    F32Mul(1.0f, 0.0f, 0.0f);
    F32Mul(0.0f, -1.0f, 0.0f);
    F32Mul(-1.0f, 0.0f, 0.0f);
    F32Mul(-1.0f, 1.0f, -1.0f);
    F32Mul(1.0f, -1.0f, -1.0f);
    F32Mul(-1.0f, -1.0f, 1.0f);
    F32Mul(1.0f, 1.0f, 1.0f);

    F32Mul(100.01f, 0.001f, .10001001f);
}

void testF32Mul() {
    doF32Mul();
}

void F32Div(float x, float y, float r) {
    doF32Instruction(0xd8, 6, 0xd8, 6, x, y, r);
}

void doF32Div() {
    F32Div(0.0f, 0.0f, TEST_NAN);
    F32Div(-0.0f, 0.0f, TEST_NAN);
    F32Div(0.0f, -0.0f, TEST_NAN);

    F32Div(0.0f, POSITIVE_INFINITY, 0.0f);
    F32Div(POSITIVE_INFINITY, 0.0f, POSITIVE_INFINITY);
    F32Div(0.0f, NEGATIVE_INFINITY, -0.0f);
    F32Div(NEGATIVE_INFINITY, 0.0f, NEGATIVE_INFINITY);
    F32Div(NEGATIVE_INFINITY, POSITIVE_INFINITY, TEST_NAN);
    F32Div(POSITIVE_INFINITY, 1.0f, POSITIVE_INFINITY);
    F32Div(POSITIVE_INFINITY, 2.0f, POSITIVE_INFINITY);
    F32Div(POSITIVE_INFINITY, POSITIVE_INFINITY, TEST_NAN);

    F32Div(TEST_NAN, 2.0f, TEST_NAN);
    F32Div(TEST_NAN, TEST_NAN, TEST_NAN);
    F32Div(-2.0f, TEST_NAN, TEST_NAN);

    F32Div(0.0f, 1.0f, 0.0f);
    F32Div(1.0f, 0.0f, POSITIVE_INFINITY);
    F32Div(0.0f, -1.0f, 0.0f);
    F32Div(-1.0f, 0.0f, NEGATIVE_INFINITY);
    F32Div(-1.0f, 1.0f, -1.0f);
    F32Div(1.0f, -1.0f, -1.0f);
    F32Div(-1.0f, -1.0f, 1.0f);
    F32Div(1.0f, 1.0f, 1.0f);

    F32Div(100.01f, 0.001f, 100010.0f);
}

void testF32Div() {
    doF32Div();
}

void F32DivR(float x, float y, float r) {
    doF32Instruction(0xd8, 7, 0xd8, 7, x, y, r);
}

void doF32DivR() {
    F32DivR(0.0f, 0.0f, TEST_NAN);
    F32DivR(-0.0f, 0.0f, TEST_NAN);
    F32DivR(0.0f, -0.0f, TEST_NAN);

    F32DivR(0.0f, POSITIVE_INFINITY, POSITIVE_INFINITY);
    F32DivR(POSITIVE_INFINITY, 0.0f, 0.0f);
    F32DivR(0.0f, NEGATIVE_INFINITY, NEGATIVE_INFINITY);
    F32DivR(NEGATIVE_INFINITY, 0.0f, -0.0f);
    F32DivR(NEGATIVE_INFINITY, POSITIVE_INFINITY, TEST_NAN);
    F32DivR(POSITIVE_INFINITY, 1.0f, 0.0f);
    F32DivR(POSITIVE_INFINITY, 2.0f, 0.0f);
    F32DivR(POSITIVE_INFINITY, POSITIVE_INFINITY, TEST_NAN);

    F32DivR(TEST_NAN, 2.0f, TEST_NAN);
    F32DivR(TEST_NAN, TEST_NAN, TEST_NAN);
    F32DivR(-2.0f, TEST_NAN, TEST_NAN);

    F32DivR(0.0f, 1.0f, POSITIVE_INFINITY);
    F32DivR(1.0f, 0.0f, 0.0f);
    F32DivR(0.0f, -1.0f, NEGATIVE_INFINITY);
    F32DivR(-1.0f, 0.0f, 0.0f);
    F32DivR(-1.0f, 1.0f, -1.0f);
    F32DivR(1.0f, -1.0f, -1.0f);
    F32DivR(-1.0f, -1.0f, 1.0f);
    F32DivR(1.0f, 1.0f, 1.0f);

    F32DivR(100.01f, 0.001f, .000009999f);
}

void testF32DivR() {
    doF32DivR();
}

int UNORDERED = 0x100 | 0x400 | 0x4000;
int LESS = 0x100;
int GREATER = 0x0;
int EQUAL = 0x4000;
int MASK = 0x100 | 0x200 | 0x400 | 0x4000;

void assertTest(int r) {
    assertTrue((AX & MASK) == r);
}

void F32ComBase(int op, int group, float x, float y, int r, int popCount) {
    newInstruction(0);
    fpu_init();

    fldf32(x, 1);
    writeF(y, 2);

    if (op > 0xFF)
        pushCode8(0x0F);
    pushCode8(op & 0xFF);

    pushCode8(rm(true, group, cpu->big ? 5 : 6));
    if (cpu->big)
        pushCode32(4 * 2);
    else
        pushCode16(4 * 2);
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(r);
    assertTrue(getFPUStackPosFromAX() == ((7 + popCount) & 7));

    newInstruction(0);
    fpu_init();

    fldf32(y, 1);
    fldf32(x, 2);
    if (op > 0xFF)
        pushCode8(0x0F);
    pushCode8(op & 0xFF);
    pushCode8(rm(false, group, 1));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(r);
    assertTrue(getFPUStackPosFromAX() == ((6 + popCount) & 7));
}
void F32Com(float x, float y, int r) {
    F32ComBase(0xd8, 2, x, y, r, 0);
}

void doF32Com() {
    F32Com(0.0f, 0.0f, EQUAL);
    F32Com(-0.0f, 0.0f, EQUAL);
    F32Com(0.0f, -0.0f, EQUAL);

    F32Com(0.0f, POSITIVE_INFINITY, LESS);
    F32Com(POSITIVE_INFINITY, 0.0f, GREATER);
    F32Com(0.0f, NEGATIVE_INFINITY, GREATER);
    F32Com(NEGATIVE_INFINITY, 0.0f, LESS);
    F32Com(NEGATIVE_INFINITY, POSITIVE_INFINITY, LESS);
    F32Com(POSITIVE_INFINITY, 1.0f, GREATER);
    F32Com(POSITIVE_INFINITY, 2.0f, GREATER);
    F32Com(POSITIVE_INFINITY, POSITIVE_INFINITY, EQUAL);

    F32Com(TEST_NAN, 2.0f, UNORDERED);
    F32Com(TEST_NAN, TEST_NAN, UNORDERED);
    F32Com(-2.0f, TEST_NAN, UNORDERED);

    F32Com(0.0f, 1.0f, LESS);
    F32Com(1.0f, 0.0f, GREATER);
    F32Com(0.0f, -1.0f, GREATER);
    F32Com(-1.0f, 0.0f, LESS);
    F32Com(-1.0f, 1.0f, LESS);
    F32Com(1.0f, -1.0f, GREATER);
    F32Com(-1.0f, -1.0f, EQUAL);
    F32Com(1.0f, 1.0f, EQUAL);

    F32Com(100.01f, 0.001f, GREATER);
}

void testF32Com() {
    doF32Com();
}

void F32ComP(float x, float y, int r) {
    F32ComBase(0xd8, 3, x, y, r, 1);
}

void doF32ComP() {
    F32ComP(0.0f, 0.0f, EQUAL);
    F32ComP(-0.0f, 0.0f, EQUAL);
    F32ComP(0.0f, -0.0f, EQUAL);

    F32ComP(0.0f, POSITIVE_INFINITY, LESS);
    F32ComP(POSITIVE_INFINITY, 0.0f, GREATER);
    F32ComP(0.0f, NEGATIVE_INFINITY, GREATER);
    F32ComP(NEGATIVE_INFINITY, 0.0f, LESS);
    F32ComP(NEGATIVE_INFINITY, POSITIVE_INFINITY, LESS);
    F32ComP(POSITIVE_INFINITY, 1.0f, GREATER);
    F32ComP(POSITIVE_INFINITY, 2.0f, GREATER);
    F32ComP(POSITIVE_INFINITY, POSITIVE_INFINITY, EQUAL);

    F32ComP(TEST_NAN, 2.0f, UNORDERED);
    F32ComP(TEST_NAN, TEST_NAN, UNORDERED);
    F32ComP(-2.0f, TEST_NAN, UNORDERED);

    F32ComP(0.0f, 1.0f, LESS);
    F32ComP(1.0f, 0.0f, GREATER);
    F32ComP(0.0f, -1.0f, GREATER);
    F32ComP(-1.0f, 0.0f, LESS);
    F32ComP(-1.0f, 1.0f, LESS);
    F32ComP(1.0f, -1.0f, GREATER);
    F32ComP(-1.0f, -1.0f, EQUAL);
    F32ComP(1.0f, 1.0f, EQUAL);

    F32ComP(100.01f, 0.001f, GREATER);
}

void testF32ComP() {
    doF32ComP();
}

void testFPUD8() {
    testF32Add();
    testF32Sub();
    testF32SubR();
    testF32Mul();
    testF32Div();
    testF32DivR();
    testF32Com();
    testF32ComP();
}

void FSTFloat(int op, int group, float f, int pop) {
    struct FPU_Float result;
    newInstruction(0);
    fpu_init();

    fldf32(f, 1);
    memory->writed(HEAP_ADDRESS + 4 * 2, 0xCDCDCDCD);

    pushCode8(op);
    pushCode8(rm(true, group, cpu->big ? 5 : 6));
    if (cpu->big)
        pushCode32(4 * 2);
    else
        pushCode16(4 * 2);
    writeFPUStatusToAX();
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == f || (isnan(result.f) && isnan(f)));
    assertTrue(getFPUStackPosFromAX() == (pop ? 0 : 7));
}

void doFSTFloat(int op, int group, int pop) {
    FSTFloat(op, group, 0.0f, pop);
    FSTFloat(op, group, 1.0f, pop);
    FSTFloat(op, group, -1.0f, pop);
    FSTFloat(op, group, 0.00001f, pop);
    FSTFloat(op, group, -0.00001f, pop);
    FSTFloat(op, group, 1010.01f, pop);
    FSTFloat(op, group, -1010.01f, pop);
    FSTFloat(op, group, TEST_NAN, pop);
    FSTFloat(op, group, POSITIVE_INFINITY, pop);
    FSTFloat(op, group, NEGATIVE_INFINITY, pop);
}

void testFSTFloat() {
    doFSTFloat(0xd9, 2, false);
}

void testFSTPFloat() {
    doFSTFloat(0xd9, 3, true);
}

void doFLDSti() {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();

    fldf32(4.0f, 1);
    fldf32(3.0f, 2);
    fldf32(2.0f, 3);
    fldf32(1.0f, 4);

    pushCode8(0xd9);
    pushCode8(rm(false, 0, 0));
    writeTopFloat(5);

    pushCode8(0xd9);
    pushCode8(rm(false, 0, 2));
    writeTopFloat(6);

    pushCode8(0xd9);
    pushCode8(rm(false, 0, 4));
    writeTopFloat(7);

    pushCode8(0xd9);
    pushCode8(rm(false, 0, 6));
    writeTopFloat(8);

    runTestCPU();

    result.i = memory->readd(HEAP_ADDRESS + 4 * 5);
    assertTrue(result.f == 1.0f);
    result.i = memory->readd(HEAP_ADDRESS + 4 * 6);
    assertTrue(result.f == 2.0f);
    result.i = memory->readd(HEAP_ADDRESS + 4 * 7);
    assertTrue(result.f == 3.0f);
    result.i = memory->readd(HEAP_ADDRESS + 4 * 8);
    assertTrue(result.f == 4.0f);
}

void testFLDSTi() {
    doFLDSti();
}

void doFXCHSTi() {
    newInstruction(0);
    fpu_init();

    fldf32(4.0f, 1);
    fldf32(3.0f, 2);
    fldf32(2.0f, 3);
    fldf32(1.0f, 4);

    pushCode8(0xd9);
    pushCode8(rm(false, 1, 3));
    writeTopFloat(5);
    runTestCPU();
    struct FPU_Float result;
    result.i = memory->readd(HEAP_ADDRESS + 4 * 5);
    assertTrue(result.f == 4.0f);
}

void testFXCHSTi() {
    doFXCHSTi();
}

void doFSTPSTi() {
    newInstruction(0);
    fpu_init();

    fldf32(4.0f, 1);
    fldf32(3.0f, 2);
    fldf32(2.0f, 3);
    fldf32(1.0f, 4);

    pushCode8(0xd9);
    pushCode8(rm(false, 3, 2));
    writeTopFloat(5);
    pushCode8(0xd9);
    pushCode8(rm(false, 3, 2));
    writeTopFloat(6);
    runTestCPU();
    struct FPU_Float result;
    result.i = memory->readd(HEAP_ADDRESS + 4 * 5);
    assertTrue(result.f == 2.0f);
    result.i = memory->readd(HEAP_ADDRESS + 4 * 6);
    assertTrue(result.f == 1.0f);
}

void testFSTPSTi() {
    doFSTPSTi();
}

void doFCHS() {
    struct FPU_Float result;
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);

    // trigger fpu.isRegCached is true
    newInstruction(0);
    fpu_init();
    fild64(100, 0);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == -100.0f);

    newInstruction(0);
    fpu_init();
    fldf32(432.1f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == -432.1f);

    newInstruction(0);
    fpu_init();
    fldf32(-0.001234f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 0.001234f);

    newInstruction(0);
    fpu_init();
    fldf32(TEST_NAN, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(isnan(result.f));

    newInstruction(0);
    fpu_init();
    fldf32(POSITIVE_INFINITY, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == NEGATIVE_INFINITY);

    newInstruction(0);
    fpu_init();
    fldf32(NEGATIVE_INFINITY, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == POSITIVE_INFINITY);
}

void testFCHS() {
    doFCHS();
}

void doFABS() {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    fldf32(432.1f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 1));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 432.1f);

    newInstruction(0);
    fpu_init();
    fldf32(-0.001234f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 1));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 0.001234f);

    newInstruction(0);
    fpu_init();
    fldf32(TEST_NAN, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 1));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(isnan(result.f));

    newInstruction(0);
    fpu_init();
    fldf32(POSITIVE_INFINITY, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 1));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == POSITIVE_INFINITY);

    newInstruction(0);
    fpu_init();
    fldf32(NEGATIVE_INFINITY, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 1));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == POSITIVE_INFINITY);
}

void testFABS() {
    doFABS();
}

void doFTST() {
    newInstruction(0);
    fpu_init();
    fldf32(432.1f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 4));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(GREATER);

    newInstruction(0);
    fpu_init();
    fldf32(-0.00001f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 4));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(LESS);

    newInstruction(0);
    fpu_init();
    fldf32(0.0f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 4));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(EQUAL);

    newInstruction(0);
    fpu_init();
    fldf32(POSITIVE_INFINITY, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 4));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(GREATER);

    newInstruction(0);
    fpu_init();
    fldf32(NEGATIVE_INFINITY, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 4));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(LESS);

    newInstruction(0);
    fpu_init();
    fldf32(TEST_NAN, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 4));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(UNORDERED);
}

void testFTST() {
    doFTST();
}

void doFXAM() {
    newInstruction(0);
    fpu_init();
    fldf32(0.0f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 5));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(0x4000);

    newInstruction(0);
    fpu_init();
    fldf32(TEST_NAN, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 5));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(0x100);

    newInstruction(0);
    fpu_init();
    fldf32(POSITIVE_INFINITY, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 5));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(0x100 | 0x400);

    newInstruction(0);
    fpu_init();
    fldf32(NEGATIVE_INFINITY, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 5));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(0x100 | 0x200 | 0x400);

    newInstruction(0);
    fpu_init();
    fldf32(1.0f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 5));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(0x400);

    newInstruction(0);
    fpu_init();
    fldf32(-2.0f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 4, 5));
    writeFPUStatusToAX();
    runTestCPU();
    assertTest(0x200 | 0x400);
}

void testFXAM() {
    doFXAM();
}

void doFLD1() {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    pushCode8(0xd9);
    pushCode8(rm(false, 5, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 1.0f);
}

void testFLD1() {
    doFLD1();
}

void doFLDL2T() {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    pushCode8(0xd9);
    pushCode8(rm(false, 5, 1));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 3.32192802f);
}

void testFLDL2T() {
    doFLDL2T();
}

void doFLDL2E() {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    pushCode8(0xd9);
    pushCode8(rm(false, 5, 2));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 1.44269502f);
}

void testFLDL2E() {
    doFLDL2E();
}

void doFLDPI() {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    pushCode8(0xd9);
    pushCode8(rm(false, 5, 3));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 3.14159274f);
}

void testFLDPI() {
    doFLDPI();
}

void doFLDLG2() {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    pushCode8(0xd9);
    pushCode8(rm(false, 5, 4));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == .30103001f);
}

void testFLDLG2() {
    doFLDLG2();
}

void doFLDLN2() {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    pushCode8(0xd9);
    pushCode8(rm(false, 5, 5));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 0.693147182f);
}

void testFLDLN2() {
    doFLDLN2();
}

void doFLDZ() {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    pushCode8(0xd9);
    pushCode8(rm(false, 5, 6));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 0.0f);
}

void testFLDZ() {
    doFLDZ();
}

void doF2XM1() {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    fldf32(0.0f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 0.0f);

    newInstruction(0);
    fpu_init();
    fldf32(TEST_NAN, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(isnan(result.f));

    newInstruction(0);
    fpu_init();
    fldf32(POSITIVE_INFINITY, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == POSITIVE_INFINITY);

    newInstruction(0);
    fpu_init();
    fldf32(NEGATIVE_INFINITY, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == -1.0f);

    newInstruction(0);
    fpu_init();
    fldf32(-1.0f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == -0.5f);

    newInstruction(0);
    fpu_init();
    fldf32(1.0f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 1.0f);

    newInstruction(0);
    fpu_init();
    fldf32(-0.5f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 0));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == -0.29289323f);
}

void testF2XM1() {
    doF2XM1();
}

void doFYL2X() {
    struct FPU_Float result;
    newInstruction(0);
    fpu_init();
    fldf32(1.0f, 1);
    fldf32(0.0f, 2);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 1));
    writeTopFloat(3);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue(result.f == NEGATIVE_INFINITY);

    newInstruction(0);
    fpu_init();
    fldf32(2.0f, 1);
    fldf32(1.0f, 2);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 1));
    writeTopFloat(3);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue(result.f == 0.0f);

    newInstruction(0);
    fpu_init();
    fldf32(8.0f, 1);
    fldf32(2.5f, 2);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 1));
    writeTopFloat(3);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue(result.f == 10.575425f);

    newInstruction(0);
    fpu_init();
    fldf32(8.0f, 1);
    fldf32(2.0f, 2);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 1));
    writeTopFloat(3);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue(result.f == 8.0f);

    newInstruction(0);
    fpu_init();
    fldf32(8.0f, 1);
    fldf32(-2.0f, 2);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 1));
    writeTopFloat(3);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue(isnan(result.f));

    newInstruction(0);
    fpu_init();
    fldf32(10.0f, 1);
    fldf32(8.0f, 2);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 1));
    writeTopFloat(3);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue(result.f == 30.0f);

    newInstruction(0);
    fpu_init();
    fldf32(10.0f, 1);
    fldf32(TEST_NAN, 2);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 1));
    writeTopFloat(3);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue(isnan(result.f));

    newInstruction(0);
    fpu_init();
    fldf32(10.0f, 1);
    fldf32(POSITIVE_INFINITY, 2);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 1));
    writeTopFloat(3);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue(result.f == POSITIVE_INFINITY);

    newInstruction(0);
    fpu_init();
    fldf32(10.0f, 1);
    fldf32(NEGATIVE_INFINITY, 2);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 1));
    writeTopFloat(3);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue(isnan(result.f));
}

void testFYL2X() {
    doFYL2X();
}

void doFPTAN(float val, float out) {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    {
        struct FPU_Float result;
        struct FPU_Float top;

        __asm {
            finit;
            fld val;
            fptan
            fstp top.f;
            fstp result.f;
        }
        assertTrue(top.f == 1.0f);
        assertTrueF(result.f, out);
    }
#endif
    newInstruction(0);
    fpu_init();
    fldf32(val, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 2));
    writeTopFloat(2, true);
    writeTopFloat(3);
    runTestCPU();
    struct FPU_Float result;
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrueF(1.0f, result.f);
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrueF(out, result.f);
}

void testFPTAN() {
    doFPTAN(0.0f, 0.0f);
    doFPTAN(fPI / 6, 1 / squareRoot3);
    doFPTAN(fPI / 4, 1.0f);
    doFPTAN(fPI / 3, squareRoot3);
    doFPTAN(fPI, 0.0f);
    doFPTAN(2 * fPI, 0.0f);
}

void doFATAN(float val1, float val2, float out) {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    {
        struct FPU_Float result;

        __asm {
            finit;
            fld val1;
            fld val2;
            fpatan;
            fstp result.f;
        }
        assertTrueF(result.f, out);
    }
#endif
    newInstruction(0);
    fpu_init();
    fldf32(val1, 1);
    fldf32(val2, 2);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 3));
    writeTopFloat(3);
    runTestCPU();
    struct FPU_Float result;
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrueF(out, result.f);
}

void testFATAN() {
    doFATAN(-squareRoot3, 1.0f, -fPI / 3.0f);
    doFATAN(-1.0f, 1.0f, -fPI / 4.0f);
    doFATAN(-1.0f, squareRoot3 , -fPI / 6.0f);
    doFATAN(0.0f, 1.0f, 0.0f);
    doFATAN(1.0f, squareRoot3, fPI / 6.0f);
    doFATAN(1.0f, 1.0f, fPI / 4.0f);
    doFATAN(squareRoot3, 1.0f, fPI / 3.0f);
}

void doFXTRACT(float val, float exp, float mantis) {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    {
        struct FPU_Float result1;
        struct FPU_Float result2;
        __asm {
            finit;
            fld val;
            fxtract;
            fstp result1.f;
            fstp result2.f;
        }
        assertTrueF(result1.f, exp);
        assertTrueF(result2.f, mantis);
    }
#endif
    newInstruction(0);
    fpu_init();
    fldf32(val, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 4));
    writeTopFloat(2, true);
    writeTopFloat(3);
    runTestCPU();
    struct FPU_Float result;
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrueF(exp, result.f);
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrueF(mantis, result.f);
}

void testFXTRACT() {
    FPU_Float ninf;
    ninf.i = FLOAT_NEGATIVE_INFINITY_BITS;
    doFXTRACT(0.0f, 0.0f, ninf.f);
    doFXTRACT(-0.0f, -0.0f, ninf.f);
    doFXTRACT(1.0f, 1.0f, 0.0f);
    doFXTRACT(-1.0f, -1.0f, 0.0f);
    doFXTRACT(2.0f, 1.0f, 1.0f);
    doFXTRACT(-2.0f, -1.0f, 1.0f);
    doFXTRACT(20000.0f, 1.22070312f, 14.0f);
    doFXTRACT(-20000.0f, -1.22070312f, 14.0f);
    doFXTRACT(0.1f, 1.60000002f, -4.0f);
    doFXTRACT(0.01f, 1.27999997f, -7.0f);
}

void doFPREMnearest(double val1, double val2, double rem, U32 c0, U32 c1, U32 c2, U32 c3) {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    {
        struct FPU_Double result;
        U16 status = 0;
        __asm {
            finit;
            fld val2;
            fld val1;
            fprem1;
            fnstsw[status];
            fstp result.d;
        }
        assertTrueD(result.d, rem);
        assertTrue(c2 == ((status & 0x0400) >> 10));
    }
#endif
    newInstruction(0);
    fpu_init();
    fld64(val2, 1);
    fld64(val1, 3);
    pushCode8(0xd9);
    pushCode8(rm(false, 6, 5));
    writeTopDouble(5);
    runTestCPU();
    struct FPU_Double result;
    result.l = memory->readq(HEAP_ADDRESS + 4 * 5);
    assertTrueD(rem, result.d);
    assertTrue(c2 == ((cpu->fpu.SW() & 0x0400) >> 10));
}

void testFPREMnearest() {
    doFPREMnearest(5.0, 2.0, 1.0, 0, 0, 0, 0);
    doFPREMnearest(5.0, 0.3, -0.100000203, 0, 0, 0, 0);
    doFPREMnearest(-7.0, 4.0, 1.00000000, 0, 0, 0, 0);
    doFPREMnearest(7.0, 4.0, -1.00000000, 0, 0, 0, 0);
    // softfloat implementation is close
    //doFPREMnearest(1.54334E+134, 27.3, 1.5578957778061860e+116, 0, 0, 1, 0);

    doFPREMnearest(-12.7, TEST_NAN, TEST_NAN, 0, 0, 0, 0);
    doFPREMnearest(TEST_NAN, 2.0, TEST_NAN, 0, 0, 0, 0);

    doFPREMnearest(-12.7, NEGATIVE_INFINITY, -12.7, 0, 0, 0, 0);
    doFPREMnearest(-12.7, POSITIVE_INFINITY, -12.7, 0, 0, 0, 0);
}

void doFSQRT() {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    fldf32(0.0f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 7, 2));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 0.0f);

    newInstruction(0);
    fpu_init();
    fldf32(1.0f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 7, 2));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 1.0f);

    newInstruction(0);
    fpu_init();
    fldf32(2.0f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 7, 2));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 1.4142135f);

    newInstruction(0);
    fpu_init();
    fldf32(4.0f, 1);
    pushCode8(0xd9);
    pushCode8(rm(false, 7, 2));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    assertTrue(result.f == 2.0f);
}

void testFSQRT() {
    doFSQRT();
}

void doFSCALE_inst(FPU_Float* st0, FPU_Float* st1, FPU_Float* st0Result) {
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    {
        float f1 = st0->f;
        float f2 = st1->f;
        struct FPU_Float result;

        __asm {
            finit;
            fld f2;
            fld f1;
            fscale;
            fstp result.f;
        }
        if (st0Result->i == FLOAT_QUIET_NAN_BITS) {
            assertTrue(isnan(result.f));
        } else {
            assertTrue(result.f == st0Result->f);
        }
    }
#endif
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    fldf32(st1->f, 1);
    fldf32(st0->f, 2);
    pushCode8(0xd9);
    pushCode8(rm(false, 7, 5));
    writeTopFloat(2);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 2);
    if (st0Result->i == FLOAT_QUIET_NAN_BITS) {
        assertTrue(isnan(result.f));
    } else {
        assertTrue(result.f == st0Result->f);
    }
}

// ST(0) 		ST(1)
//        -inf -F   -0   +0   +F   +inf  NaN
// -inf   NaN  -inf -inf -inf -inf -inf  NaN
// -F     -0   -F   -F   -F   -F   -inf  NaN
// -0     -0   -0   -0   -0   -0   NaN   NaN
// +0     +0   +0   +0   +0   +0   NaN   NaN
// +F     +0   +F   +F   +F   +F   +inf  NaN
// +inf   NaN +inf  +inf +inf +inf +inf  NaN
// NaN    NaN NaN   NaN  NaN  NaN  NaN   NaN
void testFSCALE() {
    FPU_Float st0;
    FPU_Float st1;
    FPU_Float result;

    // line 1 of the grid above -inf
    st0.i = FLOAT_NEGATIVE_INFINITY_BITS;

    st1.i = FLOAT_NEGATIVE_INFINITY_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -2.0;
    result.i = FLOAT_NEGATIVE_INFINITY_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -0.0;
    result.i = FLOAT_NEGATIVE_INFINITY_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +0.0;
    result.i = FLOAT_NEGATIVE_INFINITY_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +2.0;
    result.i = FLOAT_NEGATIVE_INFINITY_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_POSITIVE_INFINITY_BITS;
    result.i = FLOAT_NEGATIVE_INFINITY_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_QUIET_NAN_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    // line 2 of the grid above -F
    st0.f = -2.0;

    st1.i = FLOAT_NEGATIVE_INFINITY_BITS;
    result.f = -0.0f;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -2.0;
    result.f = -0.5;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -0.0;
    result.f = -2.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +0.0;
    result.f = -2.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +2.0;
    result.f = -8.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_POSITIVE_INFINITY_BITS;
    result.i = FLOAT_NEGATIVE_INFINITY_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_QUIET_NAN_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    // line 3 of the grid above -0
    st0.f = -0.0;

    st1.i = FLOAT_NEGATIVE_INFINITY_BITS;
    result.f = -0.0f;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -2.0;
    result.f = -0.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -0.0;
    result.f = -0.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +0.0;
    result.f = -0.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +2.0;
    result.f = -0.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_POSITIVE_INFINITY_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_QUIET_NAN_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    // line 4 of the grid above +0
    st0.f = +0.0;

    st1.i = FLOAT_NEGATIVE_INFINITY_BITS;
    result.f = +0.0f;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -2.0;
    result.f = +0.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -0.0;
    result.f = +0.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +0.0;
    result.f = +0.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +2.0;
    result.f = +0.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_POSITIVE_INFINITY_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_QUIET_NAN_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    // line 5 of the grid above +F
    st0.f = +2.0;

    st1.i = FLOAT_NEGATIVE_INFINITY_BITS;
    result.f = +0.0f;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -2.0;
    result.f = +0.5;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -0.0;
    result.f = +2.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +0.0;
    result.f = +2.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +2.0;
    result.f = +8.0;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_POSITIVE_INFINITY_BITS;
    result.i = FLOAT_POSITIVE_INFINITY_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_QUIET_NAN_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    // line 6 of the grid above +inf
    st0.i = FLOAT_POSITIVE_INFINITY_BITS;

    st1.i = FLOAT_NEGATIVE_INFINITY_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -2.0;
    result.i = FLOAT_POSITIVE_INFINITY_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -0.0;
    result.i = FLOAT_POSITIVE_INFINITY_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +0.0;
    result.i = FLOAT_POSITIVE_INFINITY_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +2.0;
    result.i = FLOAT_POSITIVE_INFINITY_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_POSITIVE_INFINITY_BITS;
    result.i = FLOAT_POSITIVE_INFINITY_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_QUIET_NAN_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    // line 7 of the grid above nan
    st0.i = FLOAT_QUIET_NAN_BITS;

    st1.i = FLOAT_NEGATIVE_INFINITY_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -2.0;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = -0.0;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +0.0;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.f = +2.0;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_POSITIVE_INFINITY_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);

    st1.i = FLOAT_QUIET_NAN_BITS;
    result.i = FLOAT_QUIET_NAN_BITS;
    doFSCALE_inst(&st0, &st1, &result);
}

void testFPUD9() {
    // 0
    testFLDSTi();
    // 1
    testFXCHSTi();
    // 2 NOP

    // 3
    testFSTPSTi();

    // 4
    testFCHS();
    testFABS();
    testFTST();
    testFXAM();

    // 5
    testFLD1();
    testFLDL2T();
    testFLDL2E();
    testFLDPI();
    testFLDLG2();
    testFLDLN2();
    testFLDZ();

    // 6
    testF2XM1();
    testFYL2X();
    testFPTAN();
    testFATAN();
    testFXTRACT();
    testFPREMnearest();
    //testFDECSTP();
    //testFINCSTP();

    // 7
    testFSQRT();
    testFSCALE();

    // ea
    testFSTFloat();
    testFSTPFloat();
}

void testFpuCmov(U8 group, U8 flags) {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    fldf32(2.0f, 1);
    fldf32(3.0f, 2);
    pushCode8(0xda);
    pushCode8(rm(false, group, 1)); // cmov top-1 to top
    writeTopFloat(3);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue(result.f == 3.0f); // top didn't change

    newInstruction(flags);
    fpu_init();
    fldf32(2.0f, 1);
    fldf32(3.0f, 2);
    pushCode8(0xda);
    pushCode8(rm(false, group, 1)); // cmov top-1 to top
    writeTopFloat(3);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue(result.f == 2.0f); // top changed
}

static void doFUCOMPPTest(float val1, float val2, U32 swResult) {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    fldf32(100.0f, 1);
    fldf32(val1, 2);
    fldf32(val2, 3);
    pushCode8(0xda);
    pushCode8(rm(false, 5, 1));
    writeTopFloat(4);

    // FNSTSW AX
    pushCode8(0xdf);
    pushCode8(0xe0);

    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 4);
    assertTrue(result.f == 100.0f); // stack popped twice, val1 and val2 were popped off and 100.0 is now on top
    assertTrue((AX & 0x4700) == swResult);
}

void testFUCOMPP() {
    doFUCOMPPTest(2.0, 2.0, 0x4000);
    doFUCOMPPTest(2.0, 1.0, 0x0100);
    doFUCOMPPTest(2.0, 3.0, 0x0000);
    doFUCOMPPTest(2.0, INFINITY, 0x0000);
    doFUCOMPPTest(2.0, NAN, 0x4500);
}

static void doCOMIntTest(float val1, U32 val2, U32 swResult, bool pop, bool checkValue = true) {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    fldf32(100.0f, 1);
    fldf32(val1, 2);

    memory->writed(HEAP_ADDRESS + 4 * 3, val2);

    pushCode8(0xda);
    pushCode8(rm(true, pop ? 3 : 2, cpu->big ? 5 : 6));
    if (cpu->big) {
        pushCode32(4 * 3);
    } else {
        pushCode16(4 * 3);
    }

    writeTopFloat(4);

    // FNSTSW AX
    pushCode8(0xdf);
    pushCode8(0xe0);

    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 4);
    if (pop) {
        assertTrue(result.f == 100.0f);
    } else if (checkValue) {
        assertTrue(result.f == val1);
    }
    assertTrue((AX & 0x4700) == swResult);
}

void testFCOMInt(bool pop) {
    doCOMIntTest(2.0, 2, 0x4000, pop);
    doCOMIntTest(1.0, 20000, 0x0100, pop);
    doCOMIntTest(3.0, 2, 0x0000, pop);
    doCOMIntTest(INFINITY, 2, 0x0000, pop);
    doCOMIntTest(NAN, 2, 0x4500, pop, false);
}

void testFMemInt(U8 inst, U8 group, float val1, U32 val2, float fresult) {
    struct FPU_Float result;

    newInstruction(0);
    fpu_init();
    fldf32(val1, 1);
    memory->writed(HEAP_ADDRESS + 4 * 2, val2);
    pushCode8(inst);
    pushCode8(rm(true, group, cpu->big ? 5 : 6));
    if (cpu->big) {
        pushCode32(4 * 2);
    } else {
        pushCode16(4 * 2);
    }
    writeTopFloat(3);
    runTestCPU();
    result.i = memory->readd(HEAP_ADDRESS + 4 * 3);
    assertTrue(result.f == fresult);
}

void testFPUDA() {
    // REGS
    // 0 FCMOV_ST0_STj_CF
    testFpuCmov(0, CF);

    // 1 FCMOV_ST0_STj_ZF
    testFpuCmov(1, ZF);

    // 2 FCMOV_ST0_STj_CF_OR_ZF
    testFpuCmov(2, CF);
    testFpuCmov(2, ZF);
    testFpuCmov(2, CF | ZF);

    // 3 FCMOV_ST0_STj_PF
    testFpuCmov(3, PF);

    // 5 FUCOMPP
    testFUCOMPP();

    // MEMORY
    // 0 FIADD_DWORD_INTEGER
    testFMemInt(0xda, 0, 100.0f, 100000, 100100.0f);

    // 1 FIMUL_DWORD_INTEGER
    testFMemInt(0xda, 1, 2.0f, 100000, 200000.0f);

    // 2 FICOM_DWORD_INTEGER
    testFCOMInt(false);

    // 3 FICOM_DWORD_INTEGER_Pop
    testFCOMInt(true);

    // 4 FISUB_DWORD_INTEGER
    testFMemInt(0xda, 4, 202020.0f, 100000, 102020.0f);

    // 5 FISUBR_DWORD_INTEGER
    testFMemInt(0xda, 5, 202020.0f, 100000, -102020.0f);

    // 6 FIDIV_DWORD_INTEGER
    testFMemInt(0xda, 6, 202020.0f, 2, 101010.0f);

    // 7 FIDIVR_DWORD_INTEGER
    testFMemInt(0xda, 7, 2.0f, 202020, 101010.0f);
}

#define FPU_GET_TOP(sw) ((sw & 0x3800) >> 11)

void testFSAVE() {
    float f1 = 1;
    float f2 = 0;
    float f3 = -1;
    float f4 = .001f;
    struct FSaveState state;
    struct FSaveState16 state16;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    {
        __asm {
            finit;
            fld f1;
            fld f2;
            fld f3;
            fld f4;
            fsave[state];
        }
        U32 top = FPU_GET_TOP(state.fsw);
        assertTrue(top == 4);
        assertTrue(state.ftw == 0x10ff); // TAG_Valid << 14 | TAG_Zero << 12 || TAG_Valid << 10 |  || TAG_Valid << 8 (0xff for 4 TAG_Empty)
    }
#endif
    newInstruction(0);
    fpu_init();
    fldf32(f1, 1);
    fldf32(f2, 2);
    fldf32(f3, 3);
    fldf32(f4, 4);
    pushCode8(0xdd);
    pushCode8(rm(true, 6, cpu->big ? 5 : 6));
    if (cpu->big) {
        pushCode32(0);
    } else {
        pushCode16(0);
    }
    runTestCPU();
    if (cpu->isBig()) {
        cpu->memory->memcpy(&state, HEAP_ADDRESS, sizeof(state));
        U32 top = FPU_GET_TOP(state.fsw);
        assertTrue(top == 4);
        assertTrue(state.ftw == 0x10ff);
    } else {
        cpu->memory->memcpy(&state16, HEAP_ADDRESS, sizeof(state16));
        U32 top = FPU_GET_TOP(state16.fsw);
        assertTrue(top == 4);
        assertTrue(state16.ftw == 0x10ff);
    }
}

void testFSAVEMmx() {
    float f1 = 1;
    float f2 = 0;
    float f3 = -1;
    float f4 = .001f;
    struct FSaveState state;
    U64 data = 0x123456789abcdef0l;

    if (!cpu->isBig()) {
        return;
    }
#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    {
        __asm {
            finit;
            fld f1;
            fld f2;
            fld f3;
            fld f4;
            movq mm0, data
                fsave[state];
            emms;
        }
        assertTrue(state.fsw == 0);
        assertTrue(state.st[0].q == data);
    }
#endif
    newInstruction(0);
    fpu_init();
    fldf32(f1, 1);
    fldf32(f2, 2);
    fldf32(f3, 3);
    fldf32(f4, 4);
    loadMMX(0, 0, data);
    pushCode8(0xdd);
    pushCode8(rm(true, 6, cpu->big ? 5 : 6));
    pushCode32(0);
    runTestCPU();
    cpu->memory->memcpy(&state, HEAP_ADDRESS, sizeof(state));
    assertTrue(state.fsw == 0);
    assertTrue(state.st[0].q == data);
}

// this is a very tricky test for emulators
// x87 fpu registers are 80-bit with a 64 bit mantissa, this means a 64-bit integer can be push in and read out without any loss
// 
// if an emulator represents this register as a normal 64-bit double, it only has a 52-bits mantissa, thus for large 64-bit integer
// reads and writes out of the fpu there can be loss.
//
// for some games, they may count on reading/writing 64-bit ints in/out of the fpu without loss, like a memcpy would
void doFISTTP64(U64 data) {

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    {
        struct FSaveState state;
        U64 result = 0;

        __asm {
            finit;
            fild data;
            fisttp result;
            fsave[state];
        }
        assertTrue(state.fsw == 0);
        assertTrue(result == data);
    }
#endif
    newInstruction(0);
    fpu_init();
    fild64(data, 0);
    pushCode8(0xdd);
    pushCode8(rm(true, 1, cpu->big ? 5 : 6));
    if (cpu->big) {
        pushCode32(4 * 2);
    }
    else {
        pushCode16(4 * 2);
    }
    runTestCPU();
    assertTrue(cpu->fpu.GetTop() == 0);
    assertTrue(cpu->fpu.GetTag(cpu, 7) == TAG_Empty);
    assertTrue(memory->readq(HEAP_ADDRESS + 8) == data);
}

void testFISTTP64() {
    doFISTTP64(0);
    doFISTTP64(0x123456789abcdef0l);
    doFISTTP64(0x723456789abcdef0l);
    doFISTTP64(0x0000000800000000l); // gog installer uses this and it exposed a bug where only the bottom 32-bits where checked for a 0 case
    doFISTTP64((U64)(S64)-2);
}

void doFST_DOUBLE_REAL_f64(double data, bool pop) {
#if defined(BOXEDWINE_MSVC) && !defined(BOXEDWINE_64)
    {
        struct FSaveState state;
        double result = 0;

        __asm {
            finit;
            fld data;
            fst[result];
            fsave[state];
        }
        assertTrue(state.fsw == 0x3800);
        assertTrue((isnan(result) && isnan(data)) || result == data);
    }
#endif
    newInstruction(0);
    fpu_init();
    fld64(data, 0);
    pushCode8(0xdd);
    pushCode8(rm(true, 2, cpu->big ? 5 : 6));
    if (cpu->big) {
        pushCode32(4 * 2);
    } else {
        pushCode16(4 * 2);
    }
    runTestCPU();
    assertTrue(cpu->fpu.GetTop() == 7);
    assertTrue(cpu->fpu.GetTag(cpu, 7) != TAG_Empty);
    long2Double l2d;
    l2d.l = memory->readq(HEAP_ADDRESS + 8);
    assertTrue((isnan(l2d.d) && isnan(data)) || l2d.d == data);
}

void doFST_DOUBLE_REAL_i64(U64 data, bool pop) {
#if defined(BOXEDWINE_MSVC) && !defined(BOXEDWINE_64)
    {
        struct FSaveState state;
        double result = 0;

        __asm {
            finit;
            fild data;
            fst [result];
            fsave[state];
        }
        assertTrue(state.fsw == 0x3800);
        assertTrue(result == (S64)data);
    }
#endif
    newInstruction(0);
    fpu_init();
    fild64(data, 0);
    pushCode8(0xdd);
    pushCode8(rm(true, pop ? 3 : 2, cpu->big ? 5 : 6));
    if (cpu->big) {
        pushCode32(4 * 2);
    } else {
        pushCode16(4 * 2);
    }
    runTestCPU();
    assertTrue(cpu->fpu.GetTop() == (pop ? 0 : 7));
    if (pop) {
        assertTrue(cpu->fpu.GetTag(cpu, 7) == TAG_Empty);
    } else {
        assertTrue(cpu->fpu.GetTag(cpu, 7) != TAG_Empty);
    }
    long2Double l2d;
    l2d.l = memory->readq(HEAP_ADDRESS + 8);
    assertTrue(l2d.d == (S64)data);
}

void testFST_DOUBLE_REAL(bool pop) {
    // i64 case fpu.isRegCached = false
    doFST_DOUBLE_REAL_i64(0, pop);
    doFST_DOUBLE_REAL_i64(123456, pop);
    doFST_DOUBLE_REAL_i64((U64)(-123456), pop);
    // f64 case fpu.isRegCached = true
    doFST_DOUBLE_REAL_f64(0, pop);
    doFST_DOUBLE_REAL_f64(123456.5, pop);
    doFST_DOUBLE_REAL_f64(-123456.5, pop);
    doFST_DOUBLE_REAL_f64(TEST_NAN_DOUBLE, pop);
}

void testFPUDD() {
    // MEMORY

    // 1 FISTTP 64-bit
    testFISTTP64();

    // 3 FST_DOUBLE_REAL
    testFST_DOUBLE_REAL(false);

    // 4 FST_DOUBLE_REAL_Pop
    testFST_DOUBLE_REAL(true);

    // 6 FSAVE
    testFSAVE();
    testFSAVEMmx();
}

void testFILD() {
    U64 data = 0x123456789abcdef0l;

#if defined (BOXEDWINE_MSVC) && !defined (BOXEDWINE_64)
    {
        struct FSaveState state;
        __asm {
            finit;
            fild data;
            fsave[state];
        }
        assertTrue(state.fsw == 0x3800);
        assertTrue(state.st[1].q == data);
    }
#endif
    newInstruction(0);
    fpu_init();
    fild64(data, 0);
    runTestCPU();
    assertTrue(cpu->fpu.GetTop() == 7);
    U64 low = 0;
    U64 high = 0;
    cpu->fpu.ST80(cpu->fpu.STV(1), &low, &high);
    assertTrue(low == data);
    assertTrue(cpu->fpu.SW() == 0x3800);
}

void testFPUDF() {
    // MEMORY

    // 5 FILD
    testFILD();
}


void testFPU0x0d8() { cpu->big = false; testFPUD8(); }
void testFPU0x2d8() { cpu->big = true; testFPUD8(); }
void testFPU0x0d9() { cpu->big = false; testFPUD9(); }
void testFPU0x2d9() { cpu->big = true; testFPUD9(); }
void testFPU0x0da() { cpu->big = false; testFPUDA(); }
void testFPU0x2da() { cpu->big = true; testFPUDA(); }
void testFPU0x0dd() { cpu->big = false; testFPUDD(); }
void testFPU0x2dd() { cpu->big = true; testFPUDD(); }
void testFPU0x0df() { cpu->big = false; testFPUDF(); }
void testFPU0x2df() { cpu->big = true; testFPUDF(); }

#endif