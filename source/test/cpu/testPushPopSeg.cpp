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

#include "testPushPopSeg.h"
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
constexpr U32 POP_SEG_SELECTOR = 0x107;
constexpr U32 POP_SEG_INDEX = POP_SEG_SELECTOR >> 3;
constexpr U32 INITIAL_FLAGS = CF | PF | AF | ZF | SF | OF | DF;
constexpr U32 PUSH16_BEFORE_GUARD = 0x3355;
constexpr U32 PUSH16_AFTER_GUARD = 0x7799;
constexpr U32 PUSH32_BEFORE_GUARD = 0x11223344;
constexpr U32 PUSH32_AFTER_GUARD = 0x55667788;
constexpr U16 READ16_VALUE = 0x3b71;
constexpr U32 READ32_VALUE = 0x91a7c35d;

struct SegCase {
    U8 opcode;
    U8 seg;
};

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit push/pop segment code init failed");
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
        failed("asmjit push/pop segment byte emit failed");
    }
    pushGeneratedCode(code);
}

void emitReadSegValue(U8 seg, bool big) {
    if (seg == ES) {
        emitByte(0x26);
    } else if (seg == CS) {
        emitByte(0x2e);
    } else if (seg == SS) {
        emitByte(0x36);
    }

    emitByte(0xa1);
    if (big) {
        pushCode32(0);
    } else {
        pushCode16(0);
    }
}

void setupPopSelector() {
    struct user_desc* ldt = process->getLDT(POP_SEG_INDEX);
    ldt->entry_number = POP_SEG_INDEX;
    ldt->base_addr = TEST_HEAP_ADDRESS;
    ldt->seg_32bit = 1;
    ldt->contents = 0;
    ldt->read_exec_only = 0;
    ldt->seg_not_present = 0;
}

void initRegisters(U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    cpu->reg[4].u32 = 0x1000;
    expectedRegs[4] = cpu->reg[4].u32;
}

void verifyRegisters(const U32* expectedRegs, const char* name) {
    for (int i = 0; i < 8; ++i) {
        if (cpu->reg[i].u32 != expectedRegs[i]) {
            failed("%s register value", name);
        }
    }
}

void verifyOtherSegments(const U32* expectedValue, const U32* expectedAddress, U8 changedSeg, const char* name) {
    for (int i = 0; i < 6; ++i) {
        if (i == changedSeg) {
            continue;
        }
        if (cpu->seg[i].value != expectedValue[i] || cpu->seg[i].address != expectedAddress[i]) {
            failed("%s unrelated segment", name);
        }
    }
}

void saveSegments(U32* value, U32* address) {
    for (int i = 0; i < 6; ++i) {
        value[i] = cpu->seg[i].value;
        address[i] = cpu->seg[i].address;
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

U32 stackAddress(U32 sp) {
    return cpu->seg[SS].address + sp;
}

void runPushCase(const SegCase& data, bool big, const char* name) {
    U32 expectedRegs[8];
    U32 expectedSegValue[6];
    U32 expectedSegAddress[6];

    newInstruction(INITIAL_FLAGS);
    cpu->big = big ? 1 : 0;
    initRegisters(expectedRegs);
    saveSegments(expectedSegValue, expectedSegAddress);

    cpu->seg[data.seg].value = POP_SEG_SELECTOR;
    expectedSegValue[data.seg] = POP_SEG_SELECTOR;

    U32 oldSp = cpu->reg[4].u32;
    if (big) {
        U32 pushedAddress = stackAddress(oldSp - 4);
        memory->writed(pushedAddress - 4, PUSH32_BEFORE_GUARD);
        memory->writed(pushedAddress + 4, PUSH32_AFTER_GUARD);
        expectedRegs[4] = oldSp - 4;
    } else {
        U32 pushedAddress = stackAddress(oldSp - 2);
        memory->writew(pushedAddress - 2, PUSH16_BEFORE_GUARD);
        memory->writew(pushedAddress + 2, PUSH16_AFTER_GUARD);
        expectedRegs[4] = oldSp - 2;
    }

    emitByte(data.opcode);
    runTestCPU();

    verifyRegisters(expectedRegs, name);
    verifyOtherSegments(expectedSegValue, expectedSegAddress, 0xff, name);
    if ((actualFlags() & INITIAL_FLAGS) != INITIAL_FLAGS) {
        failed("%s flags changed", name);
    }

    if (big) {
        U32 pushedAddress = stackAddress(expectedRegs[4]);
        if (memory->readd(pushedAddress) != POP_SEG_SELECTOR ||
                memory->readd(pushedAddress - 4) != PUSH32_BEFORE_GUARD ||
                memory->readd(pushedAddress + 4) != PUSH32_AFTER_GUARD) {
            failed("%s stack write", name);
        }
    } else {
        U32 pushedAddress = stackAddress(expectedRegs[4]);
        if (memory->readw(pushedAddress) != POP_SEG_SELECTOR ||
                memory->readw(pushedAddress - 2) != PUSH16_BEFORE_GUARD ||
                memory->readw(pushedAddress + 2) != PUSH16_AFTER_GUARD) {
            failed("%s stack write", name);
        }
    }
}

void runPopCase(const SegCase& data, bool big, const char* name) {
    U32 expectedRegs[8];
    U32 expectedSegValue[6];
    U32 expectedSegAddress[6];

    newInstruction(INITIAL_FLAGS);
    cpu->big = big ? 1 : 0;
    setupPopSelector();
    initRegisters(expectedRegs);
    saveSegments(expectedSegValue, expectedSegAddress);

    U32 oldSp = cpu->reg[4].u32;
    if (big) {
        cpu->reg[4].u32 = oldSp - 4;
        expectedRegs[4] = oldSp;
        memory->writed(stackAddress(cpu->reg[4].u32), POP_SEG_SELECTOR);
        memory->writed(TEST_HEAP_ADDRESS, READ32_VALUE);
    } else {
        cpu->reg[4].u32 = oldSp - 2;
        expectedRegs[4] = oldSp;
        memory->writew(stackAddress(cpu->reg[4].u32), POP_SEG_SELECTOR);
        memory->writew(TEST_HEAP_ADDRESS, READ16_VALUE);
    }

    cpu->seg[data.seg].value = 0;
    if (data.seg != SS) {
        cpu->seg[data.seg].address = 0;
    }
    expectedSegValue[data.seg] = POP_SEG_SELECTOR;
    expectedSegAddress[data.seg] = TEST_HEAP_ADDRESS;

    emitByte(data.opcode);
    emitReadSegValue(data.seg, big);
    expectedRegs[0] = big ? READ32_VALUE : ((expectedRegs[0] & 0xffff0000) | READ16_VALUE);

    runTestCPU();

    verifyRegisters(expectedRegs, name);
    verifyOtherSegments(expectedSegValue, expectedSegAddress, data.seg, name);
    if (cpu->seg[data.seg].value != POP_SEG_SELECTOR || cpu->seg[data.seg].address != TEST_HEAP_ADDRESS) {
        failed("%s segment value", name);
    }
    if ((actualFlags() & INITIAL_FLAGS) != INITIAL_FLAGS) {
        failed("%s flags changed", name);
    }
}

} // namespace

void testPushEs_0x006() {
    runPushCase({0x06, ES}, false, "push es 006");
}

void testPushEs_0x206() {
    runPushCase({0x06, ES}, true, "push es 206");
}

void testPopEs_0x007() {
    runPopCase({0x07, ES}, false, "pop es 007");
}

void testPopEs_0x207() {
    runPopCase({0x07, ES}, true, "pop es 207");
}

void testPushCs_0x00e() {
    runPushCase({0x0e, CS}, false, "push cs 00e");
}

void testPushCs_0x20e() {
    runPushCase({0x0e, CS}, true, "push cs 20e");
}

void testPushSs_0x016() {
    runPushCase({0x16, SS}, false, "push ss 016");
}

void testPushSs_0x216() {
    runPushCase({0x16, SS}, true, "push ss 216");
}

void testPopSs_0x017() {
    runPopCase({0x17, SS}, false, "pop ss 017");
}

void testPopSs_0x217() {
    runPopCase({0x17, SS}, true, "pop ss 217");
}

void testPushDs_0x01e() {
    runPushCase({0x1e, DS}, false, "push ds 01e");
}

void testPushDs_0x21e() {
    runPushCase({0x1e, DS}, true, "push ds 21e");
}

void testPopDs_0x01f() {
    runPopCase({0x1f, DS}, false, "pop ds 01f");
}

void testPopDs_0x21f() {
    runPopCase({0x1f, DS}, true, "pop ds 21f");
}

#endif
