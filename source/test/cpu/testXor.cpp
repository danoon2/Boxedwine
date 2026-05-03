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

#include "testXor.h"
#include "testX86Binary.h"

namespace {

const TestBinaryCase XOR8_CASES[] = {
    {0x00, 0x00, CF | OF | AF | SF | PF},
    {0xff, 0x0f, CF | OF | AF},
    {0x80, 0x00, CF | OF | AF | ZF | PF},
    {0x55, 0xaa, CF | OF | AF | ZF},
    {0xf0, 0xf0, CF | OF | AF | SF},
    {0x12, 0x22, CF | OF | AF | SF | ZF}
};

const TestBinaryCase XOR16_CASES[] = {
    {0x0000, 0x0000, CF | OF | AF | SF | PF},
    {0xffff, 0x000f, CF | OF | AF},
    {0x8000, 0x0000, CF | OF | AF | ZF | PF},
    {0x55aa, 0xaa55, CF | OF | AF | ZF},
    {0xf0f0, 0xf0f0, CF | OF | AF | SF},
    {0x1234, 0x2222, CF | OF | AF | SF | ZF}
};

const TestBinaryCase XOR32_CASES[] = {
    {0x00000000, 0x00000000, CF | OF | AF | SF | PF},
    {0xffffffff, 0x0000000f, CF | OF | AF},
    {0x80000000, 0x00000000, CF | OF | AF | ZF | PF},
    {0x55aa55aa, 0xaa55aa55, CF | OF | AF | ZF},
    {0xf0f0f0f0, 0xf0f0f0f0, CF | OF | AF | SF},
    {0x12345678, 0x22222222, CF | OF | AF | SF | ZF}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

} // namespace

void testXorR8R8_0x030() {
    testRunBinaryRegister(TEST_BINARY_XOR, 8, XOR8_CASES, caseCount(XOR8_CASES), "xor r8,r8", false);
}

void testXorE8R8_0x030() {
    testRunBinaryMemoryDestination(TEST_BINARY_XOR, 8, XOR8_CASES, caseCount(XOR8_CASES), "xor m8,r8");
}

void testXorR16R16_0x031() {
    testRunBinaryRegister(TEST_BINARY_XOR, 16, XOR16_CASES, caseCount(XOR16_CASES), "xor r16,r16", false);
}

void testXorE16R16_0x031() {
    testRunBinaryMemoryDestination(TEST_BINARY_XOR, 16, XOR16_CASES, caseCount(XOR16_CASES), "xor m16,r16");
}

void testXorR32R32_0x031() {
    testRunBinaryRegister(TEST_BINARY_XOR, 32, XOR32_CASES, caseCount(XOR32_CASES), "xor r32,r32", false);
}

void testXorE32R32_0x031() {
    testRunBinaryMemoryDestination(TEST_BINARY_XOR, 32, XOR32_CASES, caseCount(XOR32_CASES), "xor m32,r32");
}

void testXorR8R8_0x032() {
    testRunBinaryRegister(TEST_BINARY_XOR, 8, XOR8_CASES, caseCount(XOR8_CASES), "xor r8,r8 32", true);
}

void testXorR8E8_0x032() {
    testRunBinaryMemorySource(TEST_BINARY_XOR, 8, XOR8_CASES, caseCount(XOR8_CASES), "xor r8,m8");
}

void testXorR16R16_0x033() {
    testRunBinaryRegister(TEST_BINARY_XOR, 16, XOR16_CASES, caseCount(XOR16_CASES), "xor r16,r16 33", true);
}

void testXorR16E16_0x033() {
    testRunBinaryMemorySource(TEST_BINARY_XOR, 16, XOR16_CASES, caseCount(XOR16_CASES), "xor r16,m16");
}

void testXorR32R32_0x033() {
    testRunBinaryRegister(TEST_BINARY_XOR, 32, XOR32_CASES, caseCount(XOR32_CASES), "xor r32,r32 33", true);
}

void testXorR32E32_0x033() {
    testRunBinaryMemorySource(TEST_BINARY_XOR, 32, XOR32_CASES, caseCount(XOR32_CASES), "xor r32,m32");
}

void testXorAlIb_0x034() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_XOR, 8, XOR8_CASES, caseCount(XOR8_CASES), "xor al,ib");
}

void testXorAxIw_0x035() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_XOR, 16, XOR16_CASES, caseCount(XOR16_CASES), "xor ax,iw");
}

void testXorEaxId_0x035() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_XOR, 32, XOR32_CASES, caseCount(XOR32_CASES), "xor eax,id");
}

void testXorE8Ib_0x080() {
    testRunBinaryGroup1Immediate(TEST_BINARY_XOR, 8, XOR8_CASES, caseCount(XOR8_CASES), "xor e8,ib 80", 0x80, false);
}

void testXorE8Ib_0x280() {
    testRunBinaryGroup1Immediate(TEST_BINARY_XOR, 8, XOR8_CASES, caseCount(XOR8_CASES), "xor e8,ib 280", 0x80, false);
}

void testXorE16Iw_0x081() {
    testRunBinaryGroup1Immediate(TEST_BINARY_XOR, 16, XOR16_CASES, caseCount(XOR16_CASES), "xor e16,iw 81", 0x81, false);
}

void testXorE32Id_0x281() {
    testRunBinaryGroup1Immediate(TEST_BINARY_XOR, 32, XOR32_CASES, caseCount(XOR32_CASES), "xor e32,id 281", 0x81, false);
}

void testXorE8Ib_0x082() {
    testRunBinaryGroup1Immediate(TEST_BINARY_XOR, 8, XOR8_CASES, caseCount(XOR8_CASES), "xor e8,ib 82", 0x82, false);
}

void testXorE8Ib_0x282() {
    testRunBinaryGroup1Immediate(TEST_BINARY_XOR, 8, XOR8_CASES, caseCount(XOR8_CASES), "xor e8,ib 282", 0x82, false);
}

void testXorE16Ib_0x083() {
    testRunBinaryGroup1Immediate(TEST_BINARY_XOR, 16, XOR16_CASES, caseCount(XOR16_CASES), "xor e16,ib 83", 0x83, true);
}

void testXorE32Ib_0x283() {
    testRunBinaryGroup1Immediate(TEST_BINARY_XOR, 32, XOR32_CASES, caseCount(XOR32_CASES), "xor e32,ib 283", 0x83, true);
}

#endif
