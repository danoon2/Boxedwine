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

#include "testSub.h"
#include "testX86Binary.h"

namespace {

const TestBinaryCase SUB8_CASES[] = {
    {0x01, 0x01, 0},
    {0x00, 0x01, 0},
    {0x80, 0x01, 0},
    {0x7f, 0xff, 0},
    {0x10, 0x01, 0},
    {0x00, 0x80, 0},
    {0x12, 0xee, CF | OF | SF | ZF | AF | PF}
};

const TestBinaryCase SUB16_CASES[] = {
    {0x0001, 0x0001, 0},
    {0x0000, 0x0001, 0},
    {0x8000, 0x0001, 0},
    {0x7fff, 0xffff, 0},
    {0x0100, 0x0001, 0},
    {0x1000, 0x0001, 0},
    {0x0000, 0x8000, 0},
    {0x1234, 0xedcc, CF | OF | SF | ZF | AF | PF}
};

const TestBinaryCase SUB32_CASES[] = {
    {0x00000001, 0x00000001, 0},
    {0x00000000, 0x00000001, 0},
    {0x80000000, 0x00000001, 0},
    {0x7fffffff, 0xffffffff, 0},
    {0x00000100, 0x00000001, 0},
    {0x00001000, 0x00000001, 0},
    {0x00000000, 0x80000000, 0},
    {0x12345678, 0xedcba988, CF | OF | SF | ZF | AF | PF}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

} // namespace

void testSubR8R8_0x028() {
    testRunBinaryRegister(TEST_BINARY_SUB, 8, SUB8_CASES, caseCount(SUB8_CASES), "sub r8,r8", false);
}

void testSubE8R8_0x028() {
    testRunBinaryMemoryDestination(TEST_BINARY_SUB, 8, SUB8_CASES, caseCount(SUB8_CASES), "sub m8,r8");
}

void testSubR16R16_0x029() {
    testRunBinaryRegister(TEST_BINARY_SUB, 16, SUB16_CASES, caseCount(SUB16_CASES), "sub r16,r16", false);
}

void testSubE16R16_0x029() {
    testRunBinaryMemoryDestination(TEST_BINARY_SUB, 16, SUB16_CASES, caseCount(SUB16_CASES), "sub m16,r16");
}

void testSubR32R32_0x029() {
    testRunBinaryRegister(TEST_BINARY_SUB, 32, SUB32_CASES, caseCount(SUB32_CASES), "sub r32,r32", false);
}

void testSubE32R32_0x029() {
    testRunBinaryMemoryDestination(TEST_BINARY_SUB, 32, SUB32_CASES, caseCount(SUB32_CASES), "sub m32,r32");
}

void testSubR8R8_0x02a() {
    testRunBinaryRegister(TEST_BINARY_SUB, 8, SUB8_CASES, caseCount(SUB8_CASES), "sub r8,r8 2a", true);
}

void testSubR8E8_0x02a() {
    testRunBinaryMemorySource(TEST_BINARY_SUB, 8, SUB8_CASES, caseCount(SUB8_CASES), "sub r8,m8");
}

void testSubR16R16_0x02b() {
    testRunBinaryRegister(TEST_BINARY_SUB, 16, SUB16_CASES, caseCount(SUB16_CASES), "sub r16,r16 2b", true);
}

void testSubR16E16_0x02b() {
    testRunBinaryMemorySource(TEST_BINARY_SUB, 16, SUB16_CASES, caseCount(SUB16_CASES), "sub r16,m16");
}

void testSubR32R32_0x02b() {
    testRunBinaryRegister(TEST_BINARY_SUB, 32, SUB32_CASES, caseCount(SUB32_CASES), "sub r32,r32 2b", true);
}

void testSubR32E32_0x02b() {
    testRunBinaryMemorySource(TEST_BINARY_SUB, 32, SUB32_CASES, caseCount(SUB32_CASES), "sub r32,m32");
}

void testSubAlIb_0x02c() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_SUB, 8, SUB8_CASES, caseCount(SUB8_CASES), "sub al,ib");
}

void testSubAxIw_0x02d() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_SUB, 16, SUB16_CASES, caseCount(SUB16_CASES), "sub ax,iw");
}

void testSubEaxId_0x02d() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_SUB, 32, SUB32_CASES, caseCount(SUB32_CASES), "sub eax,id");
}

void testSubE8Ib_0x080() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SUB, 8, SUB8_CASES, caseCount(SUB8_CASES), "sub e8,ib 80", 0x80, false);
}

void testSubE8Ib_0x280() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SUB, 8, SUB8_CASES, caseCount(SUB8_CASES), "sub e8,ib 280", 0x80, false);
}

void testSubE16Iw_0x081() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SUB, 16, SUB16_CASES, caseCount(SUB16_CASES), "sub e16,iw 81", 0x81, false);
}

void testSubE32Id_0x281() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SUB, 32, SUB32_CASES, caseCount(SUB32_CASES), "sub e32,id 281", 0x81, false);
}

void testSubE8Ib_0x082() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SUB, 8, SUB8_CASES, caseCount(SUB8_CASES), "sub e8,ib 82", 0x82, false);
}

void testSubE8Ib_0x282() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SUB, 8, SUB8_CASES, caseCount(SUB8_CASES), "sub e8,ib 282", 0x82, false);
}

void testSubE16Ib_0x083() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SUB, 16, SUB16_CASES, caseCount(SUB16_CASES), "sub e16,ib 83", 0x83, true);
}

void testSubE32Ib_0x283() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SUB, 32, SUB32_CASES, caseCount(SUB32_CASES), "sub e32,ib 283", 0x83, true);
}

#endif
