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

#include "testNegNot.h"
#include "testCPU.h"
#include "testX86Util.h"
#include "testAsmJit.h"

#define cpu (testContext().cpu)
#define memory (testContext().memory)
#define pushCode8 testPushCode8
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

using namespace TestX86;

constexpr U32 REG_GUARD = 0xA5A50000;
constexpr U32 MEM_BASE = 0x10000;
constexpr U32 INITIAL_FLAGS = CF | PF | AF | ZF | SF | OF | DF;
constexpr U32 FLAG_MASK = CF | PF | AF | ZF | SF | OF | DF;

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

enum NegNotOp {
    OP_NOT,
    OP_NEG
};

struct DataCase {
    U32 value;
};

struct AddressCase {
    U32 address;
    asmjit::x86::Mem operand;
};

const DataCase NOT_CASES[] = {
    {0x00000000},
    {0xffffffff},
    {0x0f0f0f0f},
    {0xf0f0f0f0},
    {0x80000000},
    {0x7fffffff}
};

const DataCase NEG_CASES[] = {
    {0x00000000},
    {0x00000001},
    {0xffffffff},
    {20458512},
    {(U32)-20458512},
    {0x80000000},
    {0x7fffffff}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit neg/not code init failed");
    }
}

void pushGeneratedCode(const asmjit::CodeHolder& code) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        pushCode8(buffer.data()[i]);
    }
}

asmjit::Error emitOp(asmjit::x86::Assembler& a, NegNotOp op, const asmjit::x86::Gp& dst) {
    if (op == OP_NOT) {
        return a.not_(dst);
    }
    return a.neg(dst);
}

asmjit::Error emitOp(asmjit::x86::Assembler& a, NegNotOp op, const asmjit::x86::Mem& dst) {
    if (op == OP_NOT) {
        return a.not_(dst);
    }
    return a.neg(dst);
}

void emitReg(NegNotOp op, int dstReg, int width) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (emitOp(a, op, regForWidth(dstReg, width)) != asmjit::Error::kOk) {
        failed("asmjit neg/not register emit failed");
    }
    pushGeneratedCode(code);
}

void emitMem(NegNotOp op, const AddressCase& address) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (emitOp(a, op, address.operand) != asmjit::Error::kOk) {
        failed("asmjit neg/not memory emit failed");
    }
    pushGeneratedCode(code);
}

U32 parityFlag(U32 value) {
    U8 low = (U8)value;
    low ^= low >> 4;
    low &= 0x0f;
    return ((0x6996 >> low) & 1) ? 0 : PF;
}

U32 expectedFlags(NegNotOp op, U32 value, U32 result, int width) {
    if (op == OP_NOT) {
        return INITIAL_FLAGS;
    }

    value &= widthMask(width);
    result &= widthMask(width);
    U32 flags = INITIAL_FLAGS & DF;
    if (value != 0) flags |= CF;
    if (result & signBit(width)) flags |= SF;
    if (result == 0) flags |= ZF;
    if (value == signBit(width)) flags |= OF;
    if (((0 ^ value ^ result) & 0x10) != 0) flags |= AF;
    flags |= parityFlag(result);
    return flags;
}

U32 expectedValue(NegNotOp op, U32 value, int width) {
    U32 mask = widthMask(width);
    value &= mask;
    if (op == OP_NOT) {
        return ~value & mask;
    }
    return (0 - value) & mask;
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

void makeAbsoluteCase(AddressCase& data, U32 offset, int width) {
    data.address = cpu->seg[DS].address + offset;
    data.operand = memPtr(offset, width);
}

void makeBaseCase(AddressCase& data, int base, U32 offset, S32 disp, int width) {
    data.address = segmentBaseForReg(base) + offset + disp;
    data.operand = memPtr(reg32(base), disp, width);
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

void beginInstruction() {
    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
}

void runRegCase(NegNotOp op, int width, const DataCase& data, int dstReg, const char* name) {
    U32 expectedRegs[8];
    U32 result = expectedValue(op, data.value, width);

    beginInstruction();
    emitReg(op, dstReg, width);
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    setRegValue(cpu, dstReg, width, data.value);
    applyRegValue(expectedRegs, dstReg, width, result);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (((actualFlags(cpu, true) ^ expectedFlags(op, data.value, result, width)) & FLAG_MASK) != 0) {
        failed("%s flags", name);
    }
}

void runPreparedMemCase(NegNotOp op, int width, const DataCase& data, const AddressCase& address, const char* name) {
    U32 expectedRegs[8];
    U32 result = expectedValue(op, data.value, width);

    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    prepareMemory(address, data.value, width);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    verifyMemory(address, result, width, name);
    if (((actualFlags(cpu, true) ^ expectedFlags(op, data.value, result, width)) & FLAG_MASK) != 0) {
        failed("%s flags", name);
    }
}

void runMemBaseCases(NegNotOp op, int width, const DataCase& data, const char* name) {
    for (int base = 0; base < 8; ++base) {
        if (!testRunMemoryBase(base)) {
            continue;
        }
        U32 regs[8];
        AddressCase address;

        if (testRunMemoryBaseDisplacement(base, 0)) {
            beginInstruction();
            initRegisters(regs);
            makeBaseCase(address, base, regs[base], 0, width);
            emitMem(op, address);
            runPreparedMemCase(op, width, data, address, name);
        }

        if (testRunMemoryBaseDisplacement(base, 1)) {
            beginInstruction();
            initRegisters(regs);
            makeBaseCase(address, base, regs[base], 0x11, width);
            emitMem(op, address);
            runPreparedMemCase(op, width, data, address, name);
        }

        if (testRunMemoryBaseDisplacement(base, 2)) {
            beginInstruction();
            initRegisters(regs);
            makeBaseCase(address, base, regs[base], 0x123, width);
            emitMem(op, address);
            runPreparedMemCase(op, width, data, address, name);
        }
    }
}

void runMemAbsoluteCase(NegNotOp op, int width, const DataCase& data, const char* name) {
    U32 regs[8];
    AddressCase address;

    beginInstruction();
    initRegisters(regs);
    makeAbsoluteCase(address, MEM_BASE + 0x1800, width);
    emitMem(op, address);
    runPreparedMemCase(op, width, data, address, name);
}

void runMemSibCases(NegNotOp op, int width, const DataCase& data, const char* name) {
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

                beginInstruction();
                initRegisters(regs);
                makeSibCase(address, base, index, shift, MEM_BASE + 0x2000 + base * 0x100 + index * 0x10 + shift * 4, regs, width);
                emitMem(op, address);
                runPreparedMemCase(op, width, data, address, name);
            }
        }
    }
}

void runOp(NegNotOp op, int width, const DataCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int dstReg = 0; dstReg < 8; ++dstReg) {
            if (!testRunRegister(dstReg)) {
                continue;
            }
            runRegCase(op, width, cases[i], dstReg, name);
        }
        runMemBaseCases(op, width, cases[i], name);
        runMemAbsoluteCase(op, width, cases[i], name);
        runMemSibCases(op, width, cases[i], name);
    }
}

} // namespace

void testNotE8_0x0f6() { runOp(OP_NOT, 8, NOT_CASES, caseCount(NOT_CASES), "not e8 f6"); }
void testNegE8_0x0f6() { runOp(OP_NEG, 8, NEG_CASES, caseCount(NEG_CASES), "neg e8 f6"); }
void testNotE8_0x2f6() { runOp(OP_NOT, 8, NOT_CASES, caseCount(NOT_CASES), "not e8 2f6"); }
void testNegE8_0x2f6() { runOp(OP_NEG, 8, NEG_CASES, caseCount(NEG_CASES), "neg e8 2f6"); }
void testNotE16_0x0f7() { runOp(OP_NOT, 16, NOT_CASES, caseCount(NOT_CASES), "not e16 f7"); }
void testNegE16_0x0f7() { runOp(OP_NEG, 16, NEG_CASES, caseCount(NEG_CASES), "neg e16 f7"); }
void testNotE32_0x2f7() { runOp(OP_NOT, 32, NOT_CASES, caseCount(NOT_CASES), "not e32 f7"); }
void testNegE32_0x2f7() { runOp(OP_NEG, 32, NEG_CASES, caseCount(NEG_CASES), "neg e32 f7"); }

#endif
