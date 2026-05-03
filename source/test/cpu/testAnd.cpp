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

#include "testAnd.h"
#include "testX86Binary.h"

namespace {

const TestBinaryCase AND8_CASES[] = {
    {0x00, 0x00, CF | OF | AF | SF | PF},
    {0xff, 0x0f, CF | OF | AF},
    {0x80, 0xff, CF | OF | AF | ZF | PF},
    {0x55, 0xaa, CF | OF | AF | SF},
    {0xf0, 0x0f, CF | OF | AF | SF},
    {0x12, 0x22, CF | OF | AF | SF | ZF}
};

const TestBinaryCase AND16_CASES[] = {
    {0x0000, 0x0000, CF | OF | AF | SF | PF},
    {0xffff, 0x000f, CF | OF | AF},
    {0x8000, 0xffff, CF | OF | AF | ZF | PF},
    {0x00ff, 0xff00, CF | OF | AF | SF},
    {0xf0f0, 0x0f0f, CF | OF | AF | SF},
    {0x1234, 0x2222, CF | OF | AF | SF | ZF}
};

const TestBinaryCase AND32_CASES[] = {
    {0x00000000, 0x00000000, CF | OF | AF | SF | PF},
    {0xffffffff, 0x0000000f, CF | OF | AF},
    {0x80000000, 0xffffffff, CF | OF | AF | ZF | PF},
    {0x0000ffff, 0xffff0000, CF | OF | AF | SF},
    {0xf0f0f0f0, 0x0f0f0f0f, CF | OF | AF | SF},
    {0x12345678, 0x22222222, CF | OF | AF | SF | ZF}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

} // namespace

void testAndR8R8_0x020() {
    testRunBinaryRegister(TEST_BINARY_AND, 8, AND8_CASES, caseCount(AND8_CASES), "and r8,r8", false);
}

void testAndE8R8_0x020() {
    testRunBinaryMemoryDestination(TEST_BINARY_AND, 8, AND8_CASES, caseCount(AND8_CASES), "and m8,r8");
}

void testAndR16R16_0x021() {
    testRunBinaryRegister(TEST_BINARY_AND, 16, AND16_CASES, caseCount(AND16_CASES), "and r16,r16", false);
}

void testAndE16R16_0x021() {
    testRunBinaryMemoryDestination(TEST_BINARY_AND, 16, AND16_CASES, caseCount(AND16_CASES), "and m16,r16");
}

void testAndR32R32_0x021() {
    testRunBinaryRegister(TEST_BINARY_AND, 32, AND32_CASES, caseCount(AND32_CASES), "and r32,r32", false);
}

void testAndE32R32_0x021() {
    testRunBinaryMemoryDestination(TEST_BINARY_AND, 32, AND32_CASES, caseCount(AND32_CASES), "and m32,r32");
}

void testAndR8R8_0x022() {
    testRunBinaryRegister(TEST_BINARY_AND, 8, AND8_CASES, caseCount(AND8_CASES), "and r8,r8 22", true);
}

void testAndR8E8_0x022() {
    testRunBinaryMemorySource(TEST_BINARY_AND, 8, AND8_CASES, caseCount(AND8_CASES), "and r8,m8");
}

void testAndR16R16_0x023() {
    testRunBinaryRegister(TEST_BINARY_AND, 16, AND16_CASES, caseCount(AND16_CASES), "and r16,r16 23", true);
}

void testAndR16E16_0x023() {
    testRunBinaryMemorySource(TEST_BINARY_AND, 16, AND16_CASES, caseCount(AND16_CASES), "and r16,m16");
}

void testAndR32R32_0x023() {
    testRunBinaryRegister(TEST_BINARY_AND, 32, AND32_CASES, caseCount(AND32_CASES), "and r32,r32 23", true);
}

void testAndR32E32_0x023() {
    testRunBinaryMemorySource(TEST_BINARY_AND, 32, AND32_CASES, caseCount(AND32_CASES), "and r32,m32");
}

void testAndAlIb_0x024() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_AND, 8, AND8_CASES, caseCount(AND8_CASES), "and al,ib");
}

void testAndAxIw_0x025() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_AND, 16, AND16_CASES, caseCount(AND16_CASES), "and ax,iw");
}

void testAndEaxId_0x025() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_AND, 32, AND32_CASES, caseCount(AND32_CASES), "and eax,id");
}

void testAndE8Ib_0x080() {
    testRunBinaryGroup1Immediate(TEST_BINARY_AND, 8, AND8_CASES, caseCount(AND8_CASES), "and e8,ib 80", 0x80, false);
}

void testAndE8Ib_0x280() {
    testRunBinaryGroup1Immediate(TEST_BINARY_AND, 8, AND8_CASES, caseCount(AND8_CASES), "and e8,ib 280", 0x80, false);
}

void testAndE16Iw_0x081() {
    testRunBinaryGroup1Immediate(TEST_BINARY_AND, 16, AND16_CASES, caseCount(AND16_CASES), "and e16,iw 81", 0x81, false);
}

void testAndE32Id_0x281() {
    testRunBinaryGroup1Immediate(TEST_BINARY_AND, 32, AND32_CASES, caseCount(AND32_CASES), "and e32,id 281", 0x81, false);
}

void testAndE8Ib_0x082() {
    testRunBinaryGroup1Immediate(TEST_BINARY_AND, 8, AND8_CASES, caseCount(AND8_CASES), "and e8,ib 82", 0x82, false);
}

void testAndE8Ib_0x282() {
    testRunBinaryGroup1Immediate(TEST_BINARY_AND, 8, AND8_CASES, caseCount(AND8_CASES), "and e8,ib 282", 0x82, false);
}

void testAndE16Ib_0x083() {
    testRunBinaryGroup1Immediate(TEST_BINARY_AND, 16, AND16_CASES, caseCount(AND16_CASES), "and e16,ib 83", 0x83, true);
}

void testAndE32Ib_0x283() {
    testRunBinaryGroup1Immediate(TEST_BINARY_AND, 32, AND32_CASES, caseCount(AND32_CASES), "and e32,ib 283", 0x83, true);
}

#endif
