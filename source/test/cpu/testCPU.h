/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_CPU_H__
#define __TEST_CPU_H__

#include <string>
#include <vector>

#define TEST_STACK_ADDRESS 0xE0000000
#define TEST_HEAP_ADDRESS 0xF0000000
#define TEST_CODE_ADDRESS 0xD0000000
#define TEST_HEAP_SEG 0x213
#define TEST_CODE_SEG 0x223
#define TEST_STACK_SEG 0x233
#define TEST_CODE_SEG_16 0x243

struct TestContext {
    KProcessPtr process;
    KMemory* memory = nullptr;
    KThread* thread = nullptr;
    CPU* cpu = nullptr;
    U32 codeIp = TEST_CODE_ADDRESS;
    bool failed = false;
    std::vector<std::string> failures;
};

struct TestEntry {
    void (*function)();
    const char* name;
    U32 flags = 0;
};

constexpr U32 TEST_ENTRY_SERIAL = 1u << 0;

TestContext& testContext();
void testNewInstruction(int flags);
void testPushCode8(int value);
void testPushCode16(int value);
void testPushCode32(int value);
void testRunCPU();
void testFail(const char* msg, ...);
void testRunParallel(const TestEntry* entries, size_t entryCount, U32 workerCount = 0);
void testX87ExceptionSummaryState();
void testDefaultUserSegmentsUseGdtSelectors();
void testSignalHandlerSegmentsUseGdtSelectors();
void testSignalReturnPreservesLoadedInvalidTlsSelector();
bool testIsFastMode();
void testSetFastMode(bool fast);
void testWasmJitOnlyBlockEntryIsCallable();
void testWasmJitOomRetryAfterRelease();
void testFlagsAcrossIndirectJitBlockBoundary();
void testJitOverlappingDirectJumpTarget();
void testNativeJitRunCountWraps();
bool testShouldRunRegister(bool fast, int reg);
bool testShouldRunRegisterPair(bool fast, int dst, int src);
bool testShouldRunMemoryBase(bool fast, int base);
bool testShouldRunMemoryBaseDisplacement(bool fast, int base, int displacementIndex);
bool testShouldRunMemorySib(bool fast, int base, int index, int shift);
bool testRunRegister(int reg);
bool testRunRegisterPair(int dst, int src);
bool testRunMemoryBase(int base);
bool testRunMemoryBaseDisplacement(int base, int displacementIndex);
bool testRunMemorySib(int base, int index, int shift);
void testFastModeSelectionHelpers();

#endif
