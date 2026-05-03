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

#include "testOr.h"
#include "testX86Binary.h"

namespace {

const TestBinaryCase OR8_CASES[] = {
    {0x00, 0x00, CF | OF | AF | SF | PF},
    {0x01, 0x02, CF | OF | AF},
    {0x80, 0x00, CF | OF | AF | ZF | PF},
    {0x55, 0xaa, CF | OF | AF | ZF},
    {0xf0, 0x0f, CF | OF | AF | ZF},
    {0x12, 0x20, CF | OF | AF | SF | ZF}
};

const TestBinaryCase OR16_CASES[] = {
    {0x0000, 0x0000, CF | OF | AF | SF | PF},
    {0x0001, 0x0002, CF | OF | AF},
    {0x8000, 0x0000, CF | OF | AF | ZF | PF},
    {0x00ff, 0xff00, CF | OF | AF | ZF},
    {0x0f0f, 0xf0f0, CF | OF | AF | ZF},
    {0x1234, 0x2000, CF | OF | AF | SF | ZF}
};

const TestBinaryCase OR32_CASES[] = {
    {0x00000000, 0x00000000, CF | OF | AF | SF | PF},
    {0x00000001, 0x00000002, CF | OF | AF},
    {0x80000000, 0x00000000, CF | OF | AF | ZF | PF},
    {0x0000ffff, 0xffff0000, CF | OF | AF | ZF},
    {0x0f0f0f0f, 0xf0f0f0f0, CF | OF | AF | ZF},
    {0x12345678, 0x20000000, CF | OF | AF | SF | ZF}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

} // namespace

void testOrR8R8_0x008() {
    testRunBinaryRegister(TEST_BINARY_OR, 8, OR8_CASES, caseCount(OR8_CASES), "or r8,r8", false);
}

void testOrE8R8_0x008() {
    testRunBinaryMemoryDestination(TEST_BINARY_OR, 8, OR8_CASES, caseCount(OR8_CASES), "or m8,r8");
}

void testOrR16R16_0x009() {
    testRunBinaryRegister(TEST_BINARY_OR, 16, OR16_CASES, caseCount(OR16_CASES), "or r16,r16", false);
}

void testOrE16R16_0x009() {
    testRunBinaryMemoryDestination(TEST_BINARY_OR, 16, OR16_CASES, caseCount(OR16_CASES), "or m16,r16");
}

void testOrR32R32_0x009() {
    testRunBinaryRegister(TEST_BINARY_OR, 32, OR32_CASES, caseCount(OR32_CASES), "or r32,r32", false);
}

void testOrE32R32_0x009() {
    testRunBinaryMemoryDestination(TEST_BINARY_OR, 32, OR32_CASES, caseCount(OR32_CASES), "or m32,r32");
}

void testOrR8R8_0x00a() {
    testRunBinaryRegister(TEST_BINARY_OR, 8, OR8_CASES, caseCount(OR8_CASES), "or r8,r8 0a", true);
}

void testOrR8E8_0x00a() {
    testRunBinaryMemorySource(TEST_BINARY_OR, 8, OR8_CASES, caseCount(OR8_CASES), "or r8,m8");
}

void testOrR16R16_0x00b() {
    testRunBinaryRegister(TEST_BINARY_OR, 16, OR16_CASES, caseCount(OR16_CASES), "or r16,r16 0b", true);
}

void testOrR16E16_0x00b() {
    testRunBinaryMemorySource(TEST_BINARY_OR, 16, OR16_CASES, caseCount(OR16_CASES), "or r16,m16");
}

void testOrR32R32_0x00b() {
    testRunBinaryRegister(TEST_BINARY_OR, 32, OR32_CASES, caseCount(OR32_CASES), "or r32,r32 0b", true);
}

void testOrR32E32_0x00b() {
    testRunBinaryMemorySource(TEST_BINARY_OR, 32, OR32_CASES, caseCount(OR32_CASES), "or r32,m32");
}

void testOrAlIb_0x00c() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_OR, 8, OR8_CASES, caseCount(OR8_CASES), "or al,ib");
}

void testOrAxIw_0x00d() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_OR, 16, OR16_CASES, caseCount(OR16_CASES), "or ax,iw");
}

void testOrEaxId_0x00d() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_OR, 32, OR32_CASES, caseCount(OR32_CASES), "or eax,id");
}

void testOrE8Ib_0x080() {
    testRunBinaryGroup1Immediate(TEST_BINARY_OR, 8, OR8_CASES, caseCount(OR8_CASES), "or e8,ib 80", 0x80, false);
}

void testOrE8Ib_0x280() {
    testRunBinaryGroup1Immediate(TEST_BINARY_OR, 8, OR8_CASES, caseCount(OR8_CASES), "or e8,ib 280", 0x80, false);
}

void testOrE16Iw_0x081() {
    testRunBinaryGroup1Immediate(TEST_BINARY_OR, 16, OR16_CASES, caseCount(OR16_CASES), "or e16,iw 81", 0x81, false);
}

void testOrE32Id_0x281() {
    testRunBinaryGroup1Immediate(TEST_BINARY_OR, 32, OR32_CASES, caseCount(OR32_CASES), "or e32,id 281", 0x81, false);
}

void testOrE8Ib_0x082() {
    testRunBinaryGroup1Immediate(TEST_BINARY_OR, 8, OR8_CASES, caseCount(OR8_CASES), "or e8,ib 82", 0x82, false);
}

void testOrE8Ib_0x282() {
    testRunBinaryGroup1Immediate(TEST_BINARY_OR, 8, OR8_CASES, caseCount(OR8_CASES), "or e8,ib 282", 0x82, false);
}

void testOrE16Ib_0x083() {
    testRunBinaryGroup1Immediate(TEST_BINARY_OR, 16, OR16_CASES, caseCount(OR16_CASES), "or e16,ib 83", 0x83, true);
}

void testOrE32Ib_0x283() {
    testRunBinaryGroup1Immediate(TEST_BINARY_OR, 32, OR32_CASES, caseCount(OR32_CASES), "or e32,ib 283", 0x83, true);
}

#endif
