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

#include "testAdc.h"
#include "testX86Binary.h"

namespace {

constexpr U32 ADC_FLAGS_CF0 = OF | SF | ZF | AF | PF;
constexpr U32 ADC_FLAGS_CF1 = CF | OF | SF | ZF | AF | PF;

const TestBinaryCase ADC8_CASES[] = {
    {0x00, 0x00, ADC_FLAGS_CF0}, {0x00, 0x00, ADC_FLAGS_CF1},
    {0x7f, 0x00, ADC_FLAGS_CF0}, {0x7f, 0x00, ADC_FLAGS_CF1},
    {0x7f, 0x01, ADC_FLAGS_CF0}, {0x7f, 0x01, ADC_FLAGS_CF1},
    {0x80, 0x80, ADC_FLAGS_CF0}, {0x80, 0x80, ADC_FLAGS_CF1},
    {0xff, 0x00, ADC_FLAGS_CF0}, {0xff, 0x00, ADC_FLAGS_CF1},
    {0xff, 0x01, ADC_FLAGS_CF0}, {0xff, 0x01, ADC_FLAGS_CF1},
    {0x0f, 0x00, ADC_FLAGS_CF0}, {0x0f, 0x00, ADC_FLAGS_CF1},
    {0x12, 0xed, ADC_FLAGS_CF0}, {0x12, 0xed, ADC_FLAGS_CF1}
};

const TestBinaryCase ADC16_CASES[] = {
    {0x0000, 0x0000, ADC_FLAGS_CF0}, {0x0000, 0x0000, ADC_FLAGS_CF1},
    {0x7fff, 0x0000, ADC_FLAGS_CF0}, {0x7fff, 0x0000, ADC_FLAGS_CF1},
    {0x7fff, 0x0001, ADC_FLAGS_CF0}, {0x7fff, 0x0001, ADC_FLAGS_CF1},
    {0x8000, 0x8000, ADC_FLAGS_CF0}, {0x8000, 0x8000, ADC_FLAGS_CF1},
    {0xffff, 0x0000, ADC_FLAGS_CF0}, {0xffff, 0x0000, ADC_FLAGS_CF1},
    {0xffff, 0x0001, ADC_FLAGS_CF0}, {0xffff, 0x0001, ADC_FLAGS_CF1},
    {0x0fff, 0x0000, ADC_FLAGS_CF0}, {0x0fff, 0x0000, ADC_FLAGS_CF1},
    {0x1234, 0xedcb, ADC_FLAGS_CF0}, {0x1234, 0xedcb, ADC_FLAGS_CF1}
};

const TestBinaryCase ADC32_CASES[] = {
    {0x00000000, 0x00000000, ADC_FLAGS_CF0}, {0x00000000, 0x00000000, ADC_FLAGS_CF1},
    {0x7fffffff, 0x00000000, ADC_FLAGS_CF0}, {0x7fffffff, 0x00000000, ADC_FLAGS_CF1},
    {0x7fffffff, 0x00000001, ADC_FLAGS_CF0}, {0x7fffffff, 0x00000001, ADC_FLAGS_CF1},
    {0x80000000, 0x80000000, ADC_FLAGS_CF0}, {0x80000000, 0x80000000, ADC_FLAGS_CF1},
    {0xffffffff, 0x00000000, ADC_FLAGS_CF0}, {0xffffffff, 0x00000000, ADC_FLAGS_CF1},
    {0xffffffff, 0x00000001, ADC_FLAGS_CF0}, {0xffffffff, 0x00000001, ADC_FLAGS_CF1},
    {0x00000fff, 0x00000000, ADC_FLAGS_CF0}, {0x00000fff, 0x00000000, ADC_FLAGS_CF1},
    {0x12345678, 0xedcba987, ADC_FLAGS_CF0}, {0x12345678, 0xedcba987, ADC_FLAGS_CF1}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

} // namespace

void testAdcR8R8_0x010() {
    testRunBinaryRegister(TEST_BINARY_ADC, 8, ADC8_CASES, caseCount(ADC8_CASES), "adc r8,r8", false);
}

void testAdcE8R8_0x010() {
    testRunBinaryMemoryDestination(TEST_BINARY_ADC, 8, ADC8_CASES, caseCount(ADC8_CASES), "adc m8,r8");
}

void testAdcR16R16_0x011() {
    testRunBinaryRegister(TEST_BINARY_ADC, 16, ADC16_CASES, caseCount(ADC16_CASES), "adc r16,r16", false);
}

void testAdcE16R16_0x011() {
    testRunBinaryMemoryDestination(TEST_BINARY_ADC, 16, ADC16_CASES, caseCount(ADC16_CASES), "adc m16,r16");
}

void testAdcR32R32_0x011() {
    testRunBinaryRegister(TEST_BINARY_ADC, 32, ADC32_CASES, caseCount(ADC32_CASES), "adc r32,r32", false);
}

void testAdcE32R32_0x011() {
    testRunBinaryMemoryDestination(TEST_BINARY_ADC, 32, ADC32_CASES, caseCount(ADC32_CASES), "adc m32,r32");
}

void testAdcR8R8_0x012() {
    testRunBinaryRegister(TEST_BINARY_ADC, 8, ADC8_CASES, caseCount(ADC8_CASES), "adc r8,r8 12", true);
}

void testAdcR8E8_0x012() {
    testRunBinaryMemorySource(TEST_BINARY_ADC, 8, ADC8_CASES, caseCount(ADC8_CASES), "adc r8,m8");
}

void testAdcR16R16_0x013() {
    testRunBinaryRegister(TEST_BINARY_ADC, 16, ADC16_CASES, caseCount(ADC16_CASES), "adc r16,r16 13", true);
}

void testAdcR16E16_0x013() {
    testRunBinaryMemorySource(TEST_BINARY_ADC, 16, ADC16_CASES, caseCount(ADC16_CASES), "adc r16,m16");
}

void testAdcR32R32_0x013() {
    testRunBinaryRegister(TEST_BINARY_ADC, 32, ADC32_CASES, caseCount(ADC32_CASES), "adc r32,r32 13", true);
}

void testAdcR32E32_0x013() {
    testRunBinaryMemorySource(TEST_BINARY_ADC, 32, ADC32_CASES, caseCount(ADC32_CASES), "adc r32,m32");
}

void testAdcAlIb_0x014() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_ADC, 8, ADC8_CASES, caseCount(ADC8_CASES), "adc al,ib");
}

void testAdcAxIw_0x015() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_ADC, 16, ADC16_CASES, caseCount(ADC16_CASES), "adc ax,iw");
}

void testAdcEaxId_0x015() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_ADC, 32, ADC32_CASES, caseCount(ADC32_CASES), "adc eax,id");
}

void testAdcE8Ib_0x080() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADC, 8, ADC8_CASES, caseCount(ADC8_CASES), "adc e8,ib 80", 0x80, false);
}

void testAdcE8Ib_0x280() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADC, 8, ADC8_CASES, caseCount(ADC8_CASES), "adc e8,ib 280", 0x80, false);
}

void testAdcE16Iw_0x081() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADC, 16, ADC16_CASES, caseCount(ADC16_CASES), "adc e16,iw 81", 0x81, false);
}

void testAdcE32Id_0x281() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADC, 32, ADC32_CASES, caseCount(ADC32_CASES), "adc e32,id 281", 0x81, false);
}

void testAdcE8Ib_0x082() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADC, 8, ADC8_CASES, caseCount(ADC8_CASES), "adc e8,ib 82", 0x82, false);
}

void testAdcE8Ib_0x282() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADC, 8, ADC8_CASES, caseCount(ADC8_CASES), "adc e8,ib 282", 0x82, false);
}

void testAdcE16Ib_0x083() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADC, 16, ADC16_CASES, caseCount(ADC16_CASES), "adc e16,ib 83", 0x83, true);
}

void testAdcE32Ib_0x283() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADC, 32, ADC32_CASES, caseCount(ADC32_CASES), "adc e32,ib 283", 0x83, true);
}

#endif
