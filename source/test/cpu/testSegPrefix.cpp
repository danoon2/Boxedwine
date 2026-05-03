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

#include "testSegPrefix.h"
#include "testCPU.h"
#include "testAsmJit.h"

#define cpu (testContext().cpu)
#define memory (testContext().memory)
#define process (testContext().process.get())
#define pushCode8 testPushCode8
#define pushCode16 testPushCode16
#define pushCode32 testPushCode32
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

constexpr U32 REG_GUARD = 0xA55A0000;
constexpr U32 INITIAL_FLAGS = CF | PF | AF | ZF | SF | OF | DF;
constexpr U32 OFFSET = 0x380;
constexpr U8 TARGET_VALUE = 0xbf;
constexpr U8 DEFAULT_DS_VALUE = 0x4a;

struct SegPrefixCase {
    U8 opcode;
    U8 seg;
};

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit segment prefix code init failed");
    }
}

void pushGeneratedCode(const asmjit::CodeHolder& code) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        pushCode8(buffer.data()[i]);
    }
}

void emitByte(U8 value) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.db(value) != asmjit::Error::kOk) {
        failed("asmjit segment prefix byte emit failed");
    }
    pushGeneratedCode(code);
}

U32 segmentBase(U8 seg) {
    if (seg == CS) {
        return TEST_CODE_ADDRESS;
    }
    if (seg == SS) {
        return TEST_STACK_ADDRESS - K_PAGE_SIZE * 32;
    }
    if (seg == DS) {
        return TEST_HEAP_ADDRESS;
    }
    if (seg == ES) {
        return TEST_HEAP_ADDRESS + 0x2000;
    }
    if (seg == FS) {
        return TEST_HEAP_ADDRESS + 0x3000;
    }
    return TEST_HEAP_ADDRESS + 0x4000;
}

void setupSegment(U8 seg, U32* expectedValue, U32* expectedAddress) {
    U32 base = segmentBase(seg);
    cpu->seg[seg].address = base;
    cpu->seg[seg].value = 0x300 + seg * 8 + 3;
    process->hasSetSeg[seg] = true;
    expectedAddress[seg] = base;
    expectedValue[seg] = cpu->seg[seg].value;
}

void initRegisters(U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    expectedRegs[0] = (expectedRegs[0] & 0xffffff00) | TARGET_VALUE;
}

void saveSegments(U32* value, U32* address) {
    for (int i = 0; i < 6; ++i) {
        value[i] = cpu->seg[i].value;
        address[i] = cpu->seg[i].address;
    }
}

void verifyRegisters(const U32* expectedRegs, const char* name) {
    for (int i = 0; i < 8; ++i) {
        if (cpu->reg[i].u32 != expectedRegs[i]) {
            failed("%s register value", name);
        }
    }
}

void verifySegments(const U32* expectedValue, const U32* expectedAddress, const char* name) {
    for (int i = 0; i < 6; ++i) {
        if (cpu->seg[i].value != expectedValue[i] || cpu->seg[i].address != expectedAddress[i]) {
            failed("%s segment value", name);
        }
    }
}

U32 actualFlags() {
    U32 flags = 0;
    if (cpu->getCF()) flags |= CF;
    if (cpu->getPF()) flags |= PF;
    if (cpu->getAF()) flags |= AF;
    if (cpu->getZF()) flags |= ZF;
    if (cpu->getSF()) flags |= SF;
    if (cpu->getOF()) flags |= OF;
    if (cpu->flags & DF) flags |= DF;
    return flags;
}

void emitLoadAlAbsolute(const SegPrefixCase& data, bool big) {
    emitByte(data.opcode);
    emitByte(0xa0); // mov al, moffs8
    if (big) {
        pushCode32(OFFSET);
    } else {
        pushCode16(OFFSET);
    }
}

void runSegPrefixCase(const SegPrefixCase& data, bool big, const char* name) {
    U32 expectedRegs[8];
    U32 expectedSegValue[6];
    U32 expectedSegAddress[6];

    newInstruction(INITIAL_FLAGS);
    cpu->big = big ? 1 : 0;
    saveSegments(expectedSegValue, expectedSegAddress);
    setupSegment(data.seg, expectedSegValue, expectedSegAddress);
    initRegisters(expectedRegs);

    memory->writeb(cpu->seg[DS].address + OFFSET, DEFAULT_DS_VALUE);
    memory->writeb(cpu->seg[data.seg].address + OFFSET, TARGET_VALUE);
    emitLoadAlAbsolute(data, big);

    runTestCPU();

    verifyRegisters(expectedRegs, name);
    verifySegments(expectedSegValue, expectedSegAddress, name);
    if ((actualFlags() & INITIAL_FLAGS) != INITIAL_FLAGS) {
        failed("%s flags changed", name);
    }
}

} // namespace

void testSegEs_0x026() {
    runSegPrefixCase({0x26, ES}, false, "seg es 026");
}

void testSegEs_0x226() {
    runSegPrefixCase({0x26, ES}, true, "seg es 226");
}

void testSegCs_0x02e() {
    runSegPrefixCase({0x2e, CS}, false, "seg cs 02e");
}

void testSegCs_0x22e() {
    runSegPrefixCase({0x2e, CS}, true, "seg cs 22e");
}

void testSegSs_0x036() {
    runSegPrefixCase({0x36, SS}, false, "seg ss 036");
}

void testSegSs_0x236() {
    runSegPrefixCase({0x36, SS}, true, "seg ss 236");
}

void testSegDs_0x03e() {
    runSegPrefixCase({0x3e, DS}, false, "seg ds 03e");
}

void testSegDs_0x23e() {
    runSegPrefixCase({0x3e, DS}, true, "seg ds 23e");
}

void testSegFs_0x064() {
    runSegPrefixCase({0x64, FS}, false, "seg fs 064");
}

void testSegFs_0x264() {
    runSegPrefixCase({0x64, FS}, true, "seg fs 264");
}

void testSegGs_0x065() {
    runSegPrefixCase({0x65, GS}, false, "seg gs 065");
}

void testSegGs_0x265() {
    runSegPrefixCase({0x65, GS}, true, "seg gs 265");
}

#endif
