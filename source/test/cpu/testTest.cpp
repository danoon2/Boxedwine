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

#include "testTest.h"
#include "testCPU.h"
#include "testX86Binary.h"
#include "testX86Util.h"
#include "testAsmJit.h"

#define cpu (testContext().cpu)
#define memory (testContext().memory)
#define pushCode8 testPushCode8
#define pushCode16 testPushCode16
#define pushCode32 testPushCode32
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

using namespace TestX86;

constexpr U32 REG_GUARD = 0x5A5A0000;
constexpr U32 MEM_BASE = 0x10000;
constexpr U32 INITIAL_FLAGS = CF | PF | AF | ZF | SF | OF | DF;
constexpr U32 TEST_FLAG_MASK = CF | PF | AF | ZF | SF | OF | DF;

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

struct AddressCase {
    U32 address;
    asmjit::x86::Mem operand;
};

const TestBinaryCase TEST8_CASES[] = {
    {0x00, 0x00, CF | OF | AF | SF | PF},
    {0xff, 0x0f, CF | OF | AF},
    {0x80, 0xff, CF | OF | AF | ZF | PF},
    {0x55, 0xaa, CF | OF | AF | SF},
    {0xf0, 0x0f, CF | OF | AF | SF},
    {0x12, 0x22, CF | OF | AF | SF | ZF}
};

const TestBinaryCase TEST16_CASES[] = {
    {0x0000, 0x0000, CF | OF | AF | SF | PF},
    {0xffff, 0x000f, CF | OF | AF},
    {0x8000, 0xffff, CF | OF | AF | ZF | PF},
    {0x00ff, 0xff00, CF | OF | AF | SF},
    {0xf0f0, 0x0f0f, CF | OF | AF | SF},
    {0x1234, 0x2222, CF | OF | AF | SF | ZF}
};

const TestBinaryCase TEST32_CASES[] = {
    {0x00000000, 0x00000000, CF | OF | AF | SF | PF},
    {0xffffffff, 0x0000000f, CF | OF | AF},
    {0x80000000, 0xffffffff, CF | OF | AF | ZF | PF},
    {0x0000ffff, 0xffff0000, CF | OF | AF | SF},
    {0xf0f0f0f0, 0x0f0f0f0f, CF | OF | AF | SF},
    {0x12345678, 0x22222222, CF | OF | AF | SF | ZF}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit group3 test code init failed");
    }
}

void pushGeneratedCode(const asmjit::CodeHolder& code, int extension, U8 opcode) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    size_t opcodeIndex = 0;
    if (buffer.size() > 0 && buffer.data()[0] == 0x66) {
        pushCode8(0x66);
        opcodeIndex = 1;
    }
    if (buffer.size() < opcodeIndex + 2 || buffer.data()[opcodeIndex] != opcode) {
        failed("asmjit group3 test unexpected encoding");
        return;
    }
    pushCode8(buffer.data()[opcodeIndex]);
    pushCode8((buffer.data()[opcodeIndex + 1] & 0xc7) | (extension << 3));
    for (size_t i = opcodeIndex + 2; i < buffer.size(); ++i) {
        pushCode8(buffer.data()[i]);
    }
}

void emitTestReg(int dstReg, U32 imm, int extension, int width) {
    if (width == 16) {
        pushCode8(0x66);
    }
    pushCode8(width == 8 ? 0xf6 : 0xf7);
    pushCode8(0xc0 | (extension << 3) | dstReg);
    if (width == 8) {
        pushCode8((U8)imm);
    } else if (width == 16) {
        pushCode16((U16)imm);
    } else {
        pushCode32(imm);
    }
}

void emitTestMem(const AddressCase& address, U32 imm, int extension, int width) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.test(address.operand, asmjit::Imm(imm)) != asmjit::Error::kOk) {
        failed("asmjit group3 test memory emit failed");
    }
    pushGeneratedCode(code, extension, width == 8 ? 0xf6 : 0xf7);
}

U32 expectedTestFlags(U32 dst, U32 src, int width, U32 initialFlags) {
    U32 result = (dst & src) & widthMask(width);
    U32 flags = initialFlags & DF;
    if (result & signBit(width)) flags |= SF;
    if (result == 0) flags |= ZF;
    U8 low = (U8)result;
    low ^= low >> 4;
    low &= 0x0f;
    if (((0x6996 >> low) & 1) == 0) flags |= PF;
    return flags;
}

U32 segmentBaseForReg(int reg) {
    return (reg == R_BP || reg == R_SP) ? cpu->seg[SS].address : cpu->seg[DS].address;
}

void initRegisters(U32* regs) {
    regs[R_AX] = MEM_BASE + 0x0100;
    regs[R_CX] = MEM_BASE + 0x0200;
    regs[R_DX] = MEM_BASE + 0x0300;
    regs[R_BX] = MEM_BASE + 0x0400;
    regs[R_SP] = MEM_BASE + 0x0500;
    regs[R_BP] = MEM_BASE + 0x0600;
    regs[R_SI] = MEM_BASE + 0x0700;
    regs[R_DI] = MEM_BASE + 0x0800;
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = regs[i];
    }
}

void makeAbsoluteCase(AddressCase& data, U32 offset) {
    data.address = cpu->seg[DS].address + offset;
    data.operand = memPtr(offset, 32);
}

void makeBaseCase(AddressCase& data, int base, U32 offset, S32 disp, int width) {
    data.address = segmentBaseForReg(base) + offset + disp;
    data.operand = memPtr(reg32(base), disp, width);
}

void makeAbsoluteCase(AddressCase& data, U32 offset, int width) {
    data.address = cpu->seg[DS].address + offset;
    data.operand = memPtr(offset, width);
}

void makeSibCase(AddressCase& data, int base, int index, int shift, U32 targetOffset, const U32* regs, int width) {
    data.address = segmentBaseForReg(base) + targetOffset;
    data.operand = memPtr(reg32(base), reg32(index), shift, (S32)(targetOffset - regs[base] - (regs[index] << shift)), width);
}

void prepareMemory(const AddressCase& address, U32 value, int width) {
    if (width == 8) {
        memory->writeb(address.address - 1, 0x11);
        memory->writeb(address.address, value);
        memory->writeb(address.address + 1, 0x33);
        memory->writeb(address.address + 2, 0x44);
        memory->writeb(address.address + 3, 0x55);
    } else if (width == 16) {
        memory->writew(address.address - 2, 0x1111);
        memory->writew(address.address, value);
        memory->writew(address.address + 2, 0x3333);
    } else {
        memory->writew(address.address - 2, 0x1111);
        memory->writed(address.address, value);
        memory->writew(address.address + 4, 0x3333);
    }
}

void verifyMemory(const AddressCase& address, U32 value, int width, const char* name) {
    if (width == 8) {
        if (memory->readb(address.address - 1) != 0x11 ||
                memory->readb(address.address) != (value & 0xff) ||
                memory->readb(address.address + 1) != 0x33 ||
                memory->readb(address.address + 2) != 0x44 ||
                memory->readb(address.address + 3) != 0x55) {
            failed("%s memory", name);
        }
    } else if (width == 16) {
        if (memory->readw(address.address - 2) != 0x1111 ||
                memory->readw(address.address) != (value & 0xffff) ||
                memory->readw(address.address + 2) != 0x3333) {
            failed("%s memory", name);
        }
    } else if (memory->readw(address.address - 2) != 0x1111 ||
            memory->readd(address.address) != value ||
            memory->readw(address.address + 4) != 0x3333) {
        failed("%s memory", name);
    }
}

void beginInstruction(U32 flags) {
    newInstruction(flags);
    cpu->big = true;
}

void runTestRegCase(const TestBinaryCase& data, int dstReg, int extension, int width, const char* name) {
    U32 expectedRegs[8];

    beginInstruction(data.initialFlags);
    emitTestReg(dstReg, data.src, extension, width);
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    setRegValue(cpu, dstReg, width, data.dst);
    applyRegValue(expectedRegs, dstReg, width, data.dst);

    U32 expectedFlags = expectedTestFlags(getRegValue(cpu, dstReg, width), data.src, width, data.initialFlags);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (((actualFlags(cpu, true) ^ expectedFlags) & TEST_FLAG_MASK) != 0) {
        failed("%s flags", name);
    }
}

void runTestPreparedMemCase(const TestBinaryCase& data, const AddressCase& address, int width, const char* name) {
    U32 expectedRegs[8];

    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    prepareMemory(address, data.dst, width);

    U32 expectedFlags = expectedTestFlags(data.dst, data.src, width, data.initialFlags);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    verifyMemory(address, data.dst, width, name);
    if (((actualFlags(cpu, true) ^ expectedFlags) & TEST_FLAG_MASK) != 0) {
        failed("%s flags", name);
    }
}

void runTestMemBaseCases(const TestBinaryCase& data, int extension, int width, const char* name) {
    for (int base = 0; base < 8; ++base) {
        if (!testRunMemoryBase(base)) {
            continue;
        }
        U32 regs[8];
        AddressCase address;

        if (testRunMemoryBaseDisplacement(base, 0)) {
            beginInstruction(data.initialFlags);
            initRegisters(regs);
            makeBaseCase(address, base, regs[base], 0, width);
            emitTestMem(address, data.src, extension, width);
            runTestPreparedMemCase(data, address, width, name);
        }

        if (testRunMemoryBaseDisplacement(base, 1)) {
            beginInstruction(data.initialFlags);
            initRegisters(regs);
            makeBaseCase(address, base, regs[base], 0x11, width);
            emitTestMem(address, data.src, extension, width);
            runTestPreparedMemCase(data, address, width, name);
        }

        if (testRunMemoryBaseDisplacement(base, 2)) {
            beginInstruction(data.initialFlags);
            initRegisters(regs);
            makeBaseCase(address, base, regs[base], 0x123, width);
            emitTestMem(address, data.src, extension, width);
            runTestPreparedMemCase(data, address, width, name);
        }
    }
}

void runTestMemAbsoluteCase(const TestBinaryCase& data, int extension, int width, const char* name) {
    U32 regs[8];
    AddressCase address;

    beginInstruction(data.initialFlags);
    initRegisters(regs);
    makeAbsoluteCase(address, MEM_BASE + 0x1800, width);
    emitTestMem(address, data.src, extension, width);
    runTestPreparedMemCase(data, address, width, name);
}

void runTestMemSibCases(const TestBinaryCase& data, int extension, int width, const char* name) {
    for (int base = 0; base < 8; ++base) {
        for (int index = 0; index < 8; ++index) {
            if (index == R_SP) {
                continue;
            }
            for (int shift = 0; shift < 4; ++shift) {
                if (!testRunMemorySib(base, index, shift)) {
                    continue;
                }
                U32 regs[8];
                AddressCase address;

                beginInstruction(data.initialFlags);
                initRegisters(regs);
                makeSibCase(address, base, index, shift, MEM_BASE + 0x2000 + base * 0x100 + index * 0x10 + shift * 4, regs, width);
                emitTestMem(address, data.src, extension, width);
                runTestPreparedMemCase(data, address, width, name);
            }
        }
    }
}

void runGroup3Test(int extension, int width, const TestBinaryCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int dstReg = 0; dstReg < 8; ++dstReg) {
            if (!testRunRegister(dstReg)) {
                continue;
            }
            runTestRegCase(cases[i], dstReg, extension, width, name);
        }
        runTestMemBaseCases(cases[i], extension, width, name);
        runTestMemAbsoluteCase(cases[i], extension, width, name);
        runTestMemSibCases(cases[i], extension, width, name);
    }
}

} // namespace

void testTestR8R8_0x084() {
    testRunBinaryRegister(TEST_BINARY_TEST, 8, TEST8_CASES, caseCount(TEST8_CASES), "test r8,r8 84", false);
}

void testTestE8R8_0x084() {
    testRunBinaryMemoryDestination(TEST_BINARY_TEST, 8, TEST8_CASES, caseCount(TEST8_CASES), "test m8,r8");
}

void testTestR8R8_0x284() {
    testRunBinaryRegister(TEST_BINARY_TEST, 8, TEST8_CASES, caseCount(TEST8_CASES), "test r8,r8 284", false);
}

void testTestE8R8_0x284() {
    testRunBinaryMemoryDestination(TEST_BINARY_TEST, 8, TEST8_CASES, caseCount(TEST8_CASES), "test m8,r8 284");
}

void testTestR16R16_0x085() {
    testRunBinaryRegister(TEST_BINARY_TEST, 16, TEST16_CASES, caseCount(TEST16_CASES), "test r16,r16 85", false);
}

void testTestE16R16_0x085() {
    testRunBinaryMemoryDestination(TEST_BINARY_TEST, 16, TEST16_CASES, caseCount(TEST16_CASES), "test m16,r16");
}

void testTestR32R32_0x285() {
    testRunBinaryRegister(TEST_BINARY_TEST, 32, TEST32_CASES, caseCount(TEST32_CASES), "test r32,r32 285", false);
}

void testTestE32R32_0x285() {
    testRunBinaryMemoryDestination(TEST_BINARY_TEST, 32, TEST32_CASES, caseCount(TEST32_CASES), "test m32,r32");
}

void testTestAlIb_0x0a8() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_TEST, 8, TEST8_CASES, caseCount(TEST8_CASES), "test al,ib a8");
}

void testTestAlIb_0x2a8() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_TEST, 8, TEST8_CASES, caseCount(TEST8_CASES), "test al,ib 2a8");
}

void testTestAxIw_0x0a9() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_TEST, 16, TEST16_CASES, caseCount(TEST16_CASES), "test ax,iw a9");
}

void testTestEaxId_0x2a9() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_TEST, 32, TEST32_CASES, caseCount(TEST32_CASES), "test eax,id 2a9");
}

void testTestE8Ib_0x0f6() {
    runGroup3Test(0, 8, TEST8_CASES, caseCount(TEST8_CASES), "test e8,ib f6 /0");
}

void testTestE8IbAlias_0x0f6() {
    runGroup3Test(1, 8, TEST8_CASES, caseCount(TEST8_CASES), "test e8,ib f6 /1");
}

void testTestE8Ib_0x2f6() {
    runGroup3Test(0, 8, TEST8_CASES, caseCount(TEST8_CASES), "test e8,ib 2f6 /0");
}

void testTestE8IbAlias_0x2f6() {
    runGroup3Test(1, 8, TEST8_CASES, caseCount(TEST8_CASES), "test e8,ib 2f6 /1");
}

void testTestE16Iw_0x0f7() {
    runGroup3Test(0, 16, TEST16_CASES, caseCount(TEST16_CASES), "test e16,iw f7 /0");
}

void testTestE16IwAlias_0x0f7() {
    runGroup3Test(1, 16, TEST16_CASES, caseCount(TEST16_CASES), "test e16,iw f7 /1");
}

void testTestE32Id_0x2f7() {
    runGroup3Test(0, 32, TEST32_CASES, caseCount(TEST32_CASES), "test e32,id f7 /0");
}

void testTestE32IdAlias_0x2f7() {
    runGroup3Test(1, 32, TEST32_CASES, caseCount(TEST32_CASES), "test e32,id f7 /1");
}

#endif
