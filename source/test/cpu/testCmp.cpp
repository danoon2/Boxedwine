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

#include "testCmp.h"
#include "testX86Binary.h"

namespace {

const TestBinaryCase CMP8_CASES[] = {
    {0x01, 0x01, 0},
    {0x00, 0x01, 0},
    {0x80, 0x01, 0},
    {0x7f, 0xff, 0},
    {0x10, 0x01, 0},
    {0x00, 0x80, 0},
    {0x12, 0xee, CF | OF | SF | ZF | AF | PF}
};

const TestBinaryCase CMP16_CASES[] = {
    {0x0001, 0x0001, 0},
    {0x0000, 0x0001, 0},
    {0x8000, 0x0001, 0},
    {0x7fff, 0xffff, 0},
    {0x0100, 0x0001, 0},
    {0x1000, 0x0001, 0},
    {0x0000, 0x8000, 0},
    {0x1234, 0xedcc, CF | OF | SF | ZF | AF | PF}
};

const TestBinaryCase CMP32_CASES[] = {
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

void testCmpR8R8_0x038() {
    testRunBinaryRegister(TEST_BINARY_CMP, 8, CMP8_CASES, caseCount(CMP8_CASES), "cmp r8,r8", false);
}

void testCmpE8R8_0x038() {
    testRunBinaryMemoryDestination(TEST_BINARY_CMP, 8, CMP8_CASES, caseCount(CMP8_CASES), "cmp m8,r8");
}

void testCmpR16R16_0x039() {
    testRunBinaryRegister(TEST_BINARY_CMP, 16, CMP16_CASES, caseCount(CMP16_CASES), "cmp r16,r16", false);
}

void testCmpE16R16_0x039() {
    testRunBinaryMemoryDestination(TEST_BINARY_CMP, 16, CMP16_CASES, caseCount(CMP16_CASES), "cmp m16,r16");
}

void testCmpR32R32_0x039() {
    testRunBinaryRegister(TEST_BINARY_CMP, 32, CMP32_CASES, caseCount(CMP32_CASES), "cmp r32,r32", false);
}

void testCmpE32R32_0x039() {
    testRunBinaryMemoryDestination(TEST_BINARY_CMP, 32, CMP32_CASES, caseCount(CMP32_CASES), "cmp m32,r32");
}

void testCmpR8R8_0x03a() {
    testRunBinaryRegister(TEST_BINARY_CMP, 8, CMP8_CASES, caseCount(CMP8_CASES), "cmp r8,r8 3a", true);
}

void testCmpR8E8_0x03a() {
    testRunBinaryMemorySource(TEST_BINARY_CMP, 8, CMP8_CASES, caseCount(CMP8_CASES), "cmp r8,m8");
}

void testCmpR16R16_0x03b() {
    testRunBinaryRegister(TEST_BINARY_CMP, 16, CMP16_CASES, caseCount(CMP16_CASES), "cmp r16,r16 3b", true);
}

void testCmpR16E16_0x03b() {
    testRunBinaryMemorySource(TEST_BINARY_CMP, 16, CMP16_CASES, caseCount(CMP16_CASES), "cmp r16,m16");
}

void testCmpR32R32_0x03b() {
    testRunBinaryRegister(TEST_BINARY_CMP, 32, CMP32_CASES, caseCount(CMP32_CASES), "cmp r32,r32 3b", true);
}

void testCmpR32E32_0x03b() {
    testRunBinaryMemorySource(TEST_BINARY_CMP, 32, CMP32_CASES, caseCount(CMP32_CASES), "cmp r32,m32");
}

void testCmpAlIb_0x03c() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_CMP, 8, CMP8_CASES, caseCount(CMP8_CASES), "cmp al,ib");
}

void testCmpAxIw_0x03d() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_CMP, 16, CMP16_CASES, caseCount(CMP16_CASES), "cmp ax,iw");
}

void testCmpEaxId_0x03d() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_CMP, 32, CMP32_CASES, caseCount(CMP32_CASES), "cmp eax,id");
}

void testCmpE8Ib_0x080() {
    testRunBinaryGroup1Immediate(TEST_BINARY_CMP, 8, CMP8_CASES, caseCount(CMP8_CASES), "cmp e8,ib 80", 0x80, false);
}

void testCmpE8Ib_0x280() {
    testRunBinaryGroup1Immediate(TEST_BINARY_CMP, 8, CMP8_CASES, caseCount(CMP8_CASES), "cmp e8,ib 280", 0x80, false);
}

void testCmpE16Iw_0x081() {
    testRunBinaryGroup1Immediate(TEST_BINARY_CMP, 16, CMP16_CASES, caseCount(CMP16_CASES), "cmp e16,iw 81", 0x81, false);
}

void testCmpE32Id_0x281() {
    testRunBinaryGroup1Immediate(TEST_BINARY_CMP, 32, CMP32_CASES, caseCount(CMP32_CASES), "cmp e32,id 281", 0x81, false);
}

void testCmpE8Ib_0x082() {
    testRunBinaryGroup1Immediate(TEST_BINARY_CMP, 8, CMP8_CASES, caseCount(CMP8_CASES), "cmp e8,ib 82", 0x82, false);
}

void testCmpE8Ib_0x282() {
    testRunBinaryGroup1Immediate(TEST_BINARY_CMP, 8, CMP8_CASES, caseCount(CMP8_CASES), "cmp e8,ib 282", 0x82, false);
}

void testCmpE16Ib_0x083() {
    testRunBinaryGroup1Immediate(TEST_BINARY_CMP, 16, CMP16_CASES, caseCount(CMP16_CASES), "cmp e16,ib 83", 0x83, true);
}

void testCmpE32Ib_0x283() {
    testRunBinaryGroup1Immediate(TEST_BINARY_CMP, 32, CMP32_CASES, caseCount(CMP32_CASES), "cmp e32,ib 283", 0x83, true);
}

#endif
