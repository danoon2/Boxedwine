/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_X86_BINARY_H__
#define __TEST_X86_BINARY_H__

enum TestBinaryOp {
    TEST_BINARY_ADD,
    TEST_BINARY_ADC,
    TEST_BINARY_SBB,
    TEST_BINARY_SUB,
    TEST_BINARY_CMP,
    TEST_BINARY_AND,
    TEST_BINARY_XOR,
    TEST_BINARY_OR,
    TEST_BINARY_TEST
};

struct TestBinaryCase {
    U32 dst;
    U32 src;
    U32 initialFlags;
};

void testRunBinaryRegister(TestBinaryOp op, int width, const TestBinaryCase* cases, size_t caseCount, const char* name, bool reverseEncoding);
void testRunBinaryMemoryDestination(TestBinaryOp op, int width, const TestBinaryCase* cases, size_t caseCount, const char* name);
void testRunBinaryMemorySource(TestBinaryOp op, int width, const TestBinaryCase* cases, size_t caseCount, const char* name);
void testRunBinaryAccumulatorImmediate(TestBinaryOp op, int width, const TestBinaryCase* cases, size_t caseCount, const char* name);
void testRunBinaryGroup1Immediate(TestBinaryOp op, int width, const TestBinaryCase* cases, size_t caseCount, const char* name, U8 opcode, bool signExtend8);

#endif
