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

#include "testSbb.h"
#include "testX86Binary.h"

namespace {

constexpr U32 SBB_FLAGS_CF0 = OF | SF | ZF | AF | PF;
constexpr U32 SBB_FLAGS_CF1 = CF | OF | SF | ZF | AF | PF;

const TestBinaryCase SBB8_CASES[] = {
    {0x00, 0x00, SBB_FLAGS_CF0}, {0x00, 0x00, SBB_FLAGS_CF1},
    {0x80, 0x00, SBB_FLAGS_CF0}, {0x80, 0x00, SBB_FLAGS_CF1},
    {0x80, 0x01, SBB_FLAGS_CF0}, {0x80, 0x01, SBB_FLAGS_CF1},
    {0x7f, 0xff, SBB_FLAGS_CF0}, {0x7f, 0xff, SBB_FLAGS_CF1},
    {0x10, 0x0f, SBB_FLAGS_CF0}, {0x10, 0x0f, SBB_FLAGS_CF1},
    {0x00, 0xff, SBB_FLAGS_CF0}, {0x00, 0xff, SBB_FLAGS_CF1},
    {0x12, 0xee, SBB_FLAGS_CF0}, {0x12, 0xee, SBB_FLAGS_CF1}
};

const TestBinaryCase SBB16_CASES[] = {
    {0x0000, 0x0000, SBB_FLAGS_CF0}, {0x0000, 0x0000, SBB_FLAGS_CF1},
    {0x8000, 0x0000, SBB_FLAGS_CF0}, {0x8000, 0x0000, SBB_FLAGS_CF1},
    {0x8000, 0x0001, SBB_FLAGS_CF0}, {0x8000, 0x0001, SBB_FLAGS_CF1},
    {0x7fff, 0xffff, SBB_FLAGS_CF0}, {0x7fff, 0xffff, SBB_FLAGS_CF1},
    {0x1000, 0x0fff, SBB_FLAGS_CF0}, {0x1000, 0x0fff, SBB_FLAGS_CF1},
    {0x0000, 0xffff, SBB_FLAGS_CF0}, {0x0000, 0xffff, SBB_FLAGS_CF1},
    {0x1234, 0xedcc, SBB_FLAGS_CF0}, {0x1234, 0xedcc, SBB_FLAGS_CF1}
};

const TestBinaryCase SBB32_CASES[] = {
    {0x00000000, 0x00000000, SBB_FLAGS_CF0}, {0x00000000, 0x00000000, SBB_FLAGS_CF1},
    {0x80000000, 0x00000000, SBB_FLAGS_CF0}, {0x80000000, 0x00000000, SBB_FLAGS_CF1},
    {0x80000000, 0x00000001, SBB_FLAGS_CF0}, {0x80000000, 0x00000001, SBB_FLAGS_CF1},
    {0x7fffffff, 0xffffffff, SBB_FLAGS_CF0}, {0x7fffffff, 0xffffffff, SBB_FLAGS_CF1},
    {0x00001000, 0x00000fff, SBB_FLAGS_CF0}, {0x00001000, 0x00000fff, SBB_FLAGS_CF1},
    {0x00000000, 0xffffffff, SBB_FLAGS_CF0}, {0x00000000, 0xffffffff, SBB_FLAGS_CF1},
    {0x12345678, 0xedcba988, SBB_FLAGS_CF0}, {0x12345678, 0xedcba988, SBB_FLAGS_CF1}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

} // namespace

void testSbbR8R8_0x018() {
    testRunBinaryRegister(TEST_BINARY_SBB, 8, SBB8_CASES, caseCount(SBB8_CASES), "sbb r8,r8", false);
}

void testSbbE8R8_0x018() {
    testRunBinaryMemoryDestination(TEST_BINARY_SBB, 8, SBB8_CASES, caseCount(SBB8_CASES), "sbb m8,r8");
}

void testSbbR16R16_0x019() {
    testRunBinaryRegister(TEST_BINARY_SBB, 16, SBB16_CASES, caseCount(SBB16_CASES), "sbb r16,r16", false);
}

void testSbbE16R16_0x019() {
    testRunBinaryMemoryDestination(TEST_BINARY_SBB, 16, SBB16_CASES, caseCount(SBB16_CASES), "sbb m16,r16");
}

void testSbbR32R32_0x019() {
    testRunBinaryRegister(TEST_BINARY_SBB, 32, SBB32_CASES, caseCount(SBB32_CASES), "sbb r32,r32", false);
}

void testSbbE32R32_0x019() {
    testRunBinaryMemoryDestination(TEST_BINARY_SBB, 32, SBB32_CASES, caseCount(SBB32_CASES), "sbb m32,r32");
}

void testSbbR8R8_0x01a() {
    testRunBinaryRegister(TEST_BINARY_SBB, 8, SBB8_CASES, caseCount(SBB8_CASES), "sbb r8,r8 1a", true);
}

void testSbbR8E8_0x01a() {
    testRunBinaryMemorySource(TEST_BINARY_SBB, 8, SBB8_CASES, caseCount(SBB8_CASES), "sbb r8,m8");
}

void testSbbR16R16_0x01b() {
    testRunBinaryRegister(TEST_BINARY_SBB, 16, SBB16_CASES, caseCount(SBB16_CASES), "sbb r16,r16 1b", true);
}

void testSbbR16E16_0x01b() {
    testRunBinaryMemorySource(TEST_BINARY_SBB, 16, SBB16_CASES, caseCount(SBB16_CASES), "sbb r16,m16");
}

void testSbbR32R32_0x01b() {
    testRunBinaryRegister(TEST_BINARY_SBB, 32, SBB32_CASES, caseCount(SBB32_CASES), "sbb r32,r32 1b", true);
}

void testSbbR32E32_0x01b() {
    testRunBinaryMemorySource(TEST_BINARY_SBB, 32, SBB32_CASES, caseCount(SBB32_CASES), "sbb r32,m32");
}

void testSbbAlIb_0x01c() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_SBB, 8, SBB8_CASES, caseCount(SBB8_CASES), "sbb al,ib");
}

void testSbbAxIw_0x01d() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_SBB, 16, SBB16_CASES, caseCount(SBB16_CASES), "sbb ax,iw");
}

void testSbbEaxId_0x01d() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_SBB, 32, SBB32_CASES, caseCount(SBB32_CASES), "sbb eax,id");
}

void testSbbE8Ib_0x080() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SBB, 8, SBB8_CASES, caseCount(SBB8_CASES), "sbb e8,ib 80", 0x80, false);
}

void testSbbE8Ib_0x280() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SBB, 8, SBB8_CASES, caseCount(SBB8_CASES), "sbb e8,ib 280", 0x80, false);
}

void testSbbE16Iw_0x081() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SBB, 16, SBB16_CASES, caseCount(SBB16_CASES), "sbb e16,iw 81", 0x81, false);
}

void testSbbE32Id_0x281() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SBB, 32, SBB32_CASES, caseCount(SBB32_CASES), "sbb e32,id 281", 0x81, false);
}

void testSbbE8Ib_0x082() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SBB, 8, SBB8_CASES, caseCount(SBB8_CASES), "sbb e8,ib 82", 0x82, false);
}

void testSbbE8Ib_0x282() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SBB, 8, SBB8_CASES, caseCount(SBB8_CASES), "sbb e8,ib 282", 0x82, false);
}

void testSbbE16Ib_0x083() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SBB, 16, SBB16_CASES, caseCount(SBB16_CASES), "sbb e16,ib 83", 0x83, true);
}

void testSbbE32Ib_0x283() {
    testRunBinaryGroup1Immediate(TEST_BINARY_SBB, 32, SBB32_CASES, caseCount(SBB32_CASES), "sbb e32,ib 283", 0x83, true);
}

#endif
