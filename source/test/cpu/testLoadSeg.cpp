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

#include "testLoadSeg.h"
#include "testCPU.h"

#define cpu (testContext().cpu)
#define memory (testContext().memory)
#define pushCode8 testPushCode8
#define pushCode16 testPushCode16
#define pushCode32 testPushCode32
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

constexpr U32 REG_GUARD = 0xA55A0000;
constexpr U32 INITIAL_FLAGS = CF | PF | AF | ZF | SF | OF | DF;
constexpr U32 MEM_BASE = 0x3000;
constexpr U32 VALUE16 = 0xabcd;
constexpr U32 VALUE32 = 0x89abcdef;
constexpr U32 GUARD_BEFORE = 0x11223344;
constexpr U32 GUARD_AFTER = 0x55667788;

enum RegId {
    R_AX,
    R_CX,
    R_DX,
    R_BX,
    R_SP,
    R_BP,
    R_SI,
    R_DI
};

struct LoadSegOp {
    U8 firstOpcode;
    U8 secondOpcode;
    bool twoByte;
    int segment;
    U16 selector;
};

struct AddressCase {
    U8 modrmRm;
    U8 sib;
    bool hasSib;
    S32 disp;
    int dispSize;
    U32 linearAddress;
};

const LoadSegOp LOAD_SEG_OPS[] = {
    {0xc4, 0, false, ES, TEST_HEAP_SEG},
    {0xc5, 0, false, DS, TEST_CODE_SEG},
    {0x0f, 0xb2, true, SS, TEST_STACK_SEG},
    {0x0f, 0xb4, true, FS, TEST_HEAP_SEG},
    {0x0f, 0xb5, true, GS, TEST_HEAP_SEG}
};

U32 heapAddress(U32 offset) {
    return TEST_HEAP_ADDRESS + offset;
}

U32 stackAddress(U32 offset) {
    return cpu->seg[SS].address + offset;
}

U32 segmentBase32(int baseReg) {
    return (baseReg == R_SP || baseReg == R_BP) ? cpu->seg[SS].address : TEST_HEAP_ADDRESS;
}

U32 address32RegValue(int reg) {
    return MEM_BASE + 0x0200 + reg * 0x0200;
}

U32 selectorBase(U16 selector) {
    if (selector == TEST_CODE_SEG) {
        return TEST_CODE_ADDRESS;
    }
    if (selector == TEST_STACK_SEG) {
        return cpu->seg[SS].address;
    }
    return TEST_HEAP_ADDRESS;
}

void beginInstruction(bool big) {
    newInstruction(INITIAL_FLAGS);
    cpu->big = big;
}

void initRegisters(U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
}

void verifyRegisters(const U32* expectedRegs, const char* name) {
    for (int i = 0; i < 8; ++i) {
        if (cpu->reg[i].u32 != expectedRegs[i]) {
            failed("%s register value", name);
        }
    }
}

void captureSegments(U32* values, U32* addresses) {
    for (int i = 0; i < 6; ++i) {
        values[i] = cpu->seg[i].value;
        addresses[i] = cpu->seg[i].address;
    }
}

void verifySegments(const U32* values, const U32* addresses, int changedSeg, U16 selector, const char* name) {
    for (int i = 0; i < 6; ++i) {
        U32 expectedValue = i == changedSeg ? selector : values[i];
        U32 expectedAddress = i == changedSeg ? selectorBase(selector) : addresses[i];
        if (cpu->seg[i].value != expectedValue || cpu->seg[i].address != expectedAddress) {
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

void verifyFlagsUnchanged(const char* name) {
    if ((actualFlags() & INITIAL_FLAGS) != INITIAL_FLAGS) {
        failed("%s flags changed", name);
    }
}

void writePointer(const AddressCase& address, bool big, U16 selector) {
    memory->writed(address.linearAddress - 4, GUARD_BEFORE);
    if (big) {
        memory->writed(address.linearAddress, VALUE32);
        memory->writew(address.linearAddress + 4, selector);
        memory->writew(address.linearAddress + 6, 0xcccc);
    } else {
        memory->writew(address.linearAddress, VALUE16);
        memory->writew(address.linearAddress + 2, selector);
        memory->writew(address.linearAddress + 4, 0xcccc);
    }
    memory->writed(address.linearAddress + (big ? 8 : 6), GUARD_AFTER);
}

void verifyPointerGuards(const AddressCase& address, bool big, const char* name) {
    if (memory->readd(address.linearAddress - 4) != GUARD_BEFORE ||
            memory->readd(address.linearAddress + (big ? 8 : 6)) != GUARD_AFTER) {
        failed("%s memory guards", name);
    }
}

void setAddressRegisters16() {
    cpu->reg[R_BX].u32 = REG_GUARD | 0x1000;
    cpu->reg[R_BP].u32 = REG_GUARD | 0x1100;
    cpu->reg[R_SI].u32 = REG_GUARD | 0x0200;
    cpu->reg[R_DI].u32 = REG_GUARD | 0x0300;
}

void setAddressRegisters32() {
    cpu->reg[R_AX].u32 = MEM_BASE + 0x0200;
    cpu->reg[R_CX].u32 = MEM_BASE + 0x0400;
    cpu->reg[R_DX].u32 = MEM_BASE + 0x0600;
    cpu->reg[R_BX].u32 = MEM_BASE + 0x0800;
    cpu->reg[R_SP].u32 = MEM_BASE + 0x0a00;
    cpu->reg[R_BP].u32 = MEM_BASE + 0x0c00;
    cpu->reg[R_SI].u32 = MEM_BASE + 0x0e00;
    cpu->reg[R_DI].u32 = MEM_BASE + 0x1000;
}

void emitLoadSeg(const LoadSegOp& op, int dstReg, const AddressCase& address) {
    if (op.twoByte) {
        pushCode8(op.firstOpcode);
        pushCode8(op.secondOpcode);
    } else {
        pushCode8(op.firstOpcode);
    }
    pushCode8((dstReg << 3) | address.modrmRm);
    if (address.hasSib) {
        pushCode8(address.sib);
    }
    if (address.dispSize == 1) {
        pushCode8((U8)address.disp);
    } else if (address.dispSize == 2) {
        pushCode16((U16)address.disp);
    } else if (address.dispSize == 4) {
        pushCode32((U32)address.disp);
    }
}

void applyExpectedReg(U32* expectedRegs, int reg, bool big) {
    if (big) {
        expectedRegs[reg] = VALUE32;
    } else {
        expectedRegs[reg] = (expectedRegs[reg] & 0xffff0000) | VALUE16;
    }
}

AddressCase address16Case(int index) {
    switch (index) {
    case 0: return {0x00, 0, false, 0, 0, heapAddress(0x1200)};
    case 1: return {0x41, 0, false, 0x20, 1, heapAddress(0x1320)};
    case 2: return {0x82, 0, false, 0x40, 2, stackAddress(0x1340)};
    case 3: return {0x43, 0, false, -0x10, 1, stackAddress(0x13f0)};
    case 4: return {0x84, 0, false, MEM_BASE + 0x40, 2, heapAddress(0x3240)};
    case 5: return {0x45, 0, false, -0x20, 1, heapAddress(0x02e0)};
    case 6: return {0x06, 0, false, MEM_BASE + 0x600, 2, heapAddress(0x3600)};
    default: return {0x47, 0, false, 0x30, 1, heapAddress(0x1030)};
    }
}

AddressCase address32Case(int index) {
    if (index < 8) {
        U32 targetOffset = MEM_BASE + 0x5000 + index * 0x0200;
        S32 disp = (S32)(targetOffset - address32RegValue(index));
        if (index == R_SP) {
            return {0x84, 0x24, true, disp, 4, segmentBase32(index) + targetOffset};
        }
        return {(U8)(0x80 | index), 0, false, disp, 4, segmentBase32(index) + targetOffset};
    }
    if (index == 8) {
        return {0x05, 0, false, MEM_BASE + 0x1600, 4, heapAddress(MEM_BASE + 0x1600)};
    }
    return {0x84, 0x8b, true, 0x200, 4, heapAddress(0x10a00)};
}

void runCase(const LoadSegOp& op, int dstReg, bool big, const AddressCase& address, const char* name) {
    U32 expectedRegs[8];
    U32 segValues[6];
    U32 segAddresses[6];

    beginInstruction(big);
    emitLoadSeg(op, dstReg, address);
    initRegisters(expectedRegs);
    if (big) {
        setAddressRegisters32();
    } else {
        setAddressRegisters16();
    }
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    captureSegments(segValues, segAddresses);
    writePointer(address, big, op.selector);
    applyExpectedReg(expectedRegs, dstReg, big);

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    verifySegments(segValues, segAddresses, op.segment, op.selector, name);
    verifyPointerGuards(address, big, name);
    verifyFlagsUnchanged(name);
}

void runLoadSegCases(const LoadSegOp& op, bool big, const char* name) {
    int addressCount = big ? 10 : 8;
    for (int dstReg = 0; dstReg < 8; ++dstReg) {
        for (int i = 0; i < addressCount; ++i) {
            AddressCase address = big ? address32Case(i) : address16Case(i);
            runCase(op, dstReg, big, address, name);
        }
    }
}

} // namespace

void testLesR16M16_0x0c4() { runLoadSegCases(LOAD_SEG_OPS[0], false, "les r16,m16 c4"); }
void testLesR32M32_0x2c4() { runLoadSegCases(LOAD_SEG_OPS[0], true, "les r32,m32 2c4"); }
void testLdsR16M16_0x0c5() { runLoadSegCases(LOAD_SEG_OPS[1], false, "lds r16,m16 c5"); }
void testLdsR32M32_0x2c5() { runLoadSegCases(LOAD_SEG_OPS[1], true, "lds r32,m32 2c5"); }
void testLssR16M16_0x1b2() { runLoadSegCases(LOAD_SEG_OPS[2], false, "lss r16,m16 1b2"); }
void testLssR32M32_0x3b2() { runLoadSegCases(LOAD_SEG_OPS[2], true, "lss r32,m32 3b2"); }
void testLfsR16M16_0x1b4() { runLoadSegCases(LOAD_SEG_OPS[3], false, "lfs r16,m16 1b4"); }
void testLfsR32M32_0x3b4() { runLoadSegCases(LOAD_SEG_OPS[3], true, "lfs r32,m32 3b4"); }
void testLgsR16M16_0x1b5() { runLoadSegCases(LOAD_SEG_OPS[4], false, "lgs r16,m16 1b5"); }
void testLgsR32M32_0x3b5() { runLoadSegCases(LOAD_SEG_OPS[4], true, "lgs r32,m32 3b5"); }

#endif
