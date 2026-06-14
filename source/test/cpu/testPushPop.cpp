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

#include "testPushPop.h"
#include "testCPU.h"
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

constexpr U32 REG_GUARD = 0xA55A0000;
constexpr U32 INITIAL_FLAGS = CF | PF | AF | ZF | SF | OF | DF;
constexpr U32 STACK_START = 0x1000;
constexpr U32 MEM_BASE = 0x3000;
constexpr U32 PUSH16_VALUE = 0x1234;
constexpr U32 PUSH32_VALUE = 0x56781234;
constexpr U32 GUARD16_A = 0xaaaa;
constexpr U32 GUARD16_B = 0xbbbb;
constexpr U32 GUARD32_A = 0xaaaaaaaa;
constexpr U32 GUARD32_B = 0xbbbbbbbb;
constexpr U32 DEFAULT32 = 0xccccdddd;

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
    U8 modrm;
    U8 sib;
    bool hasSib;
    U32 disp;
    int dispSize;
    bool addressPrefix;
};

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit push/pop code init failed");
    }
}

void emitByte(U8 value) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.db(value) != asmjit::Error::kOk) {
        failed("asmjit push/pop byte emit failed");
    }
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        pushCode8(buffer.data()[i]);
    }
}

void emitWord(U16 value) {
    pushCode16(value);
}

void emitDword(U32 value) {
    pushCode32(value);
}

void emitOperandPrefixIfNeeded(int width) {
    if (width == 16) {
        emitByte(0x66);
    }
}

void emitModRMInstruction(U8 opcode, U8 regOpcode, const AddressCase& data, int width) {
    emitOperandPrefixIfNeeded(width);
    if (data.addressPrefix) {
        emitByte(0x67);
    }
    emitByte(opcode);
    emitByte((data.modrm & 0xc7) | (regOpcode << 3));
    if (data.hasSib) {
        emitByte(data.sib);
    }
    if (data.dispSize == 1) {
        emitByte((U8)data.disp);
    } else if (data.dispSize == 2) {
        emitWord((U16)data.disp);
    } else if (data.dispSize == 4) {
        emitDword(data.disp);
    }
}

U32 stackAddress(U32 sp) {
    return cpu->seg[SS].address + sp;
}

U32 segmentBaseForReg(int reg) {
    return (reg == R_BP || reg == R_SP) ? cpu->seg[SS].address : cpu->seg[DS].address;
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

void initRegisters(U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    cpu->reg[R_SP].u32 = STACK_START;
    expectedRegs[R_SP] = STACK_START;
}

void verifyRegisters(const U32* expectedRegs, const char* name) {
    for (int i = 0; i < 8; ++i) {
        if (cpu->reg[i].u32 != expectedRegs[i]) {
            failed("%s register value", name);
        }
    }
}

void setReg16(int reg, U32 value) {
    cpu->reg[reg].u32 = (cpu->reg[reg].u32 & 0xffff0000) | (value & 0xffff);
}

void setExpected16(U32* expectedRegs, int reg, U32 value) {
    expectedRegs[reg] = (expectedRegs[reg] & 0xffff0000) | (value & 0xffff);
}

void writeStackGuard(int width, U32 sp) {
    if (width == 16) {
        memory->writew(stackAddress(sp + 2), GUARD16_A);
        memory->writew(stackAddress(sp), 0xcccc);
        memory->writew(stackAddress(sp - 2), GUARD16_B);
    } else {
        memory->writed(stackAddress(sp + 4), GUARD32_A);
        memory->writed(stackAddress(sp), 0xcccccccc);
        memory->writed(stackAddress(sp - 4), GUARD32_B);
    }
}

void verifyPushStack(int width, U32 newSp, U32 expectedValue, const char* name) {
    if (width == 16) {
        if (cpu->reg[R_SP].u32 != newSp ||
                memory->readw(stackAddress(newSp)) != (expectedValue & 0xffff) ||
                memory->readw(stackAddress(newSp + 2)) != GUARD16_A ||
                memory->readw(stackAddress(newSp - 2)) != GUARD16_B) {
            failed("%s stack write", name);
        }
    } else if (cpu->reg[R_SP].u32 != newSp ||
            memory->readd(stackAddress(newSp)) != expectedValue ||
            memory->readd(stackAddress(newSp + 4)) != GUARD32_A ||
            memory->readd(stackAddress(newSp - 4)) != GUARD32_B) {
        failed("%s stack write", name);
    }
}

void writePopStack(int width, U32 oldSp, U32 value) {
    if (width == 16) {
        memory->writew(stackAddress(oldSp), GUARD16_A);
        memory->writew(stackAddress(oldSp - 2), value);
        memory->writew(stackAddress(oldSp - 4), GUARD16_B);
        cpu->reg[R_SP].u32 = oldSp - 2;
    } else {
        memory->writed(stackAddress(oldSp), GUARD32_A);
        memory->writed(stackAddress(oldSp - 4), value);
        memory->writed(stackAddress(oldSp - 8), GUARD32_B);
        cpu->reg[R_SP].u32 = oldSp - 4;
    }
}

void verifyPopStack(int width, U32 oldSp, const char* name) {
    if (width == 16) {
        if (memory->readw(stackAddress(oldSp)) != GUARD16_A || memory->readw(stackAddress(oldSp - 4)) != GUARD16_B) {
            failed("%s stack guards", name);
        }
    } else if (memory->readd(stackAddress(oldSp)) != GUARD32_A || memory->readd(stackAddress(oldSp - 8)) != GUARD32_B) {
        failed("%s stack guards", name);
    }
}

void runPushRegCase(int reg, int width, const char* name) {
    U32 expectedRegs[8];

    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
    initRegisters(expectedRegs);

    U32 pushedValue = width == 16 ? PUSH16_VALUE : PUSH32_VALUE;
    if (reg == R_SP) {
        pushedValue = cpu->reg[R_SP].u32;
    } else if (width == 16) {
        setReg16(reg, pushedValue);
        setExpected16(expectedRegs, reg, pushedValue);
    } else {
        cpu->reg[reg].u32 = pushedValue;
        expectedRegs[reg] = pushedValue;
    }

    writeStackGuard(width, STACK_START - (width / 8));
    expectedRegs[R_SP] = STACK_START - (width / 8);
    emitOperandPrefixIfNeeded(width);
    emitByte(0x50 + reg);
    runTestCPU();

    verifyRegisters(expectedRegs, name);
    verifyPushStack(width, expectedRegs[R_SP], pushedValue, name);
    verifyFlagsUnchanged(name);
}

void runPopRegCase(int reg, int width, const char* name) {
    U32 expectedRegs[8];
    U32 value = width == 16 ? PUSH16_VALUE : PUSH32_VALUE;

    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
    initRegisters(expectedRegs);
    writePopStack(width, STACK_START, value);

    if (reg == R_SP) {
        expectedRegs[R_SP] = width == 16 ? ((STACK_START & 0xffff0000) | value) : value;
    } else {
        expectedRegs[R_SP] = STACK_START;
        if (width == 16) {
            setExpected16(expectedRegs, reg, value);
        } else {
            expectedRegs[reg] = value;
        }
    }

    emitOperandPrefixIfNeeded(width);
    emitByte(0x58 + reg);
    runTestCPU();

    verifyRegisters(expectedRegs, name);
    verifyPopStack(width, STACK_START, name);
    verifyFlagsUnchanged(name);
}

void runPushGroupRegCase(int reg, int width, const char* name) {
    U32 expectedRegs[8];

    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
    initRegisters(expectedRegs);

    U32 pushedValue = width == 16 ? PUSH16_VALUE : PUSH32_VALUE;
    if (reg == R_SP) {
        pushedValue = cpu->reg[R_SP].u32;
    } else if (width == 16) {
        setReg16(reg, pushedValue);
        setExpected16(expectedRegs, reg, pushedValue);
    } else {
        cpu->reg[reg].u32 = pushedValue;
        expectedRegs[reg] = pushedValue;
    }

    writeStackGuard(width, STACK_START - (width / 8));
    expectedRegs[R_SP] = STACK_START - (width / 8);
    emitOperandPrefixIfNeeded(width);
    emitByte(0xff);
    emitByte(0xc0 | (6 << 3) | reg);
    runTestCPU();

    verifyRegisters(expectedRegs, name);
    verifyPushStack(width, expectedRegs[R_SP], pushedValue, name);
    verifyFlagsUnchanged(name);
}

void runPopGroupRegCase(int reg, int width, const char* name) {
    U32 expectedRegs[8];
    U32 value = width == 16 ? PUSH16_VALUE : PUSH32_VALUE;

    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
    initRegisters(expectedRegs);
    writePopStack(width, STACK_START, value);

    expectedRegs[R_SP] = STACK_START;
    if (width == 16) {
        setExpected16(expectedRegs, reg, value);
    } else {
        expectedRegs[reg] = value;
    }

    emitOperandPrefixIfNeeded(width);
    emitByte(0x8f);
    emitByte(0xc0 | reg);
    runTestCPU();

    verifyRegisters(expectedRegs, name);
    verifyPopStack(width, STACK_START, name);
    verifyFlagsUnchanged(name);
}

void writeAddressRegs(const U32* regs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = regs[i];
    }
}

void makeAbsoluteCase(AddressCase& data, bool big, U32 offset) {
    data.address = cpu->seg[DS].address + offset;
    data.hasSib = false;
    data.sib = 0;
    data.disp = offset;
    data.addressPrefix = false;
    if (big) {
        data.modrm = 0x05;
        data.dispSize = 4;
    } else {
        data.modrm = 0x06;
        data.dispSize = 2;
    }
}

void makeBaseCase(AddressCase& data, int base, U32 offset, int disp, bool addressPrefix) {
    data.address = segmentBaseForReg(base) + offset + disp;
    data.addressPrefix = addressPrefix;
    data.hasSib = base == R_SP;
    data.sib = (0 << 6) | (R_SP << 3) | R_SP;
    data.disp = disp;
    if (disp == 0 && base != R_BP) {
        data.modrm = base;
        data.dispSize = 0;
    } else if (disp >= -128 && disp <= 127) {
        data.modrm = 0x40 | base;
        data.dispSize = 1;
    } else {
        data.modrm = 0x80 | base;
        data.dispSize = 4;
    }
}

void makeSibCase(AddressCase& data, int base, int index, int shift, U32 targetOffset, const U32* regs, bool addressPrefix) {
    data.address = segmentBaseForReg(base) + targetOffset;
    data.addressPrefix = addressPrefix;
    data.hasSib = true;
    data.modrm = 0x84;
    data.sib = (shift << 6) | (index << 3) | base;
    data.disp = targetOffset - regs[base] - (regs[index] << shift);
    data.dispSize = 4;
}

void prepareMemoryValue(const AddressCase& address, int width, U32 value) {
    if (width == 16) {
        memory->writed(address.address - 4, DEFAULT32);
        memory->writew(address.address, value);
        memory->writed(address.address + 2, DEFAULT32);
    } else {
        memory->writed(address.address - 4, DEFAULT32);
        memory->writed(address.address, value);
        memory->writed(address.address + 4, DEFAULT32);
    }
}

void verifyMemoryValue(const AddressCase& address, int width, U32 value, const char* name) {
    if (width == 16) {
        if (memory->readw(address.address) != (value & 0xffff) ||
                memory->readd(address.address - 4) != DEFAULT32 ||
                memory->readd(address.address + 2) != DEFAULT32) {
            failed("%s memory write", name);
        }
    } else if (memory->readd(address.address) != value ||
            memory->readd(address.address - 4) != DEFAULT32 ||
            memory->readd(address.address + 4) != DEFAULT32) {
        failed("%s memory write", name);
    }
}

void runPushMemoryPrepared(const AddressCase& address, const U32* regs, int width, const char* name) {
    U32 expectedRegs[8];
    U32 value = width == 16 ? PUSH16_VALUE : PUSH32_VALUE;

    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
    initRegisters(expectedRegs);
    writeAddressRegs(regs);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    expectedRegs[R_SP] = cpu->reg[R_SP].u32 - (width / 8);
    prepareMemoryValue(address, width, value);
    writeStackGuard(width, expectedRegs[R_SP]);

    emitModRMInstruction(0xff, 6, address, width);
    runTestCPU();

    verifyRegisters(expectedRegs, name);
    verifyPushStack(width, expectedRegs[R_SP], value, name);
    verifyMemoryValue(address, width, value, name);
    verifyFlagsUnchanged(name);
}

void runPopMemoryPrepared(const AddressCase& address, const U32* regs, int width, const char* name) {
    U32 expectedRegs[8];
    U32 value = width == 16 ? PUSH16_VALUE : PUSH32_VALUE;

    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
    initRegisters(expectedRegs);
    writeAddressRegs(regs);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    writePopStack(width, cpu->reg[R_SP].u32, value);
    expectedRegs[R_SP] = cpu->reg[R_SP].u32 + (width / 8);
    prepareMemoryValue(address, width, 0);

    emitModRMInstruction(0x8f, 0, address, width);
    runTestCPU();

    verifyRegisters(expectedRegs, name);
    verifyPopStack(width, expectedRegs[R_SP], name);
    verifyMemoryValue(address, width, value, name);
    verifyFlagsUnchanged(name);
}

void runAbsoluteMemoryCases(int width, bool push, const char* name) {
    U32 regs[8];
    AddressCase address;
    makeAbsoluteCase(address, true, MEM_BASE + 0x100);
    for (int i = 0; i < 8; ++i) {
        regs[i] = REG_GUARD | (0x0100 + i);
    }
    regs[R_SP] = STACK_START;
    if (push) {
        runPushMemoryPrepared(address, regs, width, name);
    } else {
        runPopMemoryPrepared(address, regs, width, name);
    }
}

void runBaseMemoryCases(int width, bool push, const char* name) {
    for (int base = 0; base < 8; ++base) {
        if (!testRunMemoryBase(base)) {
            continue;
        }
        for (int dispIndex = 0; dispIndex < 3; ++dispIndex) {
            if (base == R_SP && dispIndex == 0) {
                continue;
            }
            if (!testRunMemoryBaseDisplacement(base, dispIndex)) {
                continue;
            }
            U32 regs[8];
            AddressCase address;
            int disp = dispIndex == 0 ? 0 : (dispIndex == 1 ? 0x11 : 0x123);
            for (int i = 0; i < 8; ++i) {
                regs[i] = REG_GUARD | (0x0100 + i);
            }
            regs[R_SP] = STACK_START;
            regs[base] = MEM_BASE + 0x200 + base * 0x100;
            if (base == R_SP) {
                regs[base] = STACK_START;
            }
            makeBaseCase(address, base, regs[base], disp, false);
            if (push) {
                runPushMemoryPrepared(address, regs, width, name);
            } else {
                runPopMemoryPrepared(address, regs, width, name);
            }
        }
    }
}

void runSibMemoryCases(int width, bool push, const char* name) {
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
                U32 targetOffset = MEM_BASE + 0x3000 + base * 0x200 + index * 0x20 + shift * 4;
                for (int i = 0; i < 8; ++i) {
                    regs[i] = REG_GUARD | (0x0100 + i);
                }
                regs[R_SP] = STACK_START;
                regs[base] = MEM_BASE + 0x20 + base * 0x40;
                regs[index] = 3;
                if (base == R_SP) {
                    regs[base] = STACK_START;
                    targetOffset = STACK_START + 0x400 + index * 0x20 + shift * 4;
                }
                makeSibCase(address, base, index, shift, targetOffset, regs, false);
                if (push) {
                    runPushMemoryPrepared(address, regs, width, name);
                } else {
                    runPopMemoryPrepared(address, regs, width, name);
                }
            }
        }
    }
}

void runPushMemoryCases(int width, const char* name) {
    runAbsoluteMemoryCases(width, true, name);
    runBaseMemoryCases(width, true, name);
    runSibMemoryCases(width, true, name);
}

void runPopMemoryCases(int width, const char* name) {
    runAbsoluteMemoryCases(width, false, name);
    runBaseMemoryCases(width, false, name);
    runSibMemoryCases(width, false, name);
}

void runPopEspMemoryCase(const char* name) {
    U32 expectedRegs[8];

    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
    initRegisters(expectedRegs);
    cpu->reg[R_SP].u32 = STACK_START - 12;
    expectedRegs[R_SP] = STACK_START - 8;
    memory->writed(stackAddress(cpu->reg[R_SP].u32), PUSH32_VALUE);
    memory->writed(stackAddress(cpu->reg[R_SP].u32 + 4), DEFAULT32);
    memory->writed(stackAddress(cpu->reg[R_SP].u32 + 8), DEFAULT32);

    emitByte(0x8f);
    emitByte(0x44);
    emitByte(0x24);
    emitByte(4);
    runTestCPU();

    verifyRegisters(expectedRegs, name);
    if (memory->readd(stackAddress(STACK_START - 8)) != DEFAULT32 || memory->readd(stackAddress(STACK_START - 4)) != PUSH32_VALUE) {
        failed("%s pop esp memory addressing", name);
    }
    verifyFlagsUnchanged(name);
}

void runPushImmediate(int width, U32 value, bool signExtend8, const char* name) {
    U32 expectedRegs[8];
    U32 pushedValue = value;

    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
    initRegisters(expectedRegs);
    writeStackGuard(width, STACK_START - (width / 8));
    expectedRegs[R_SP] = STACK_START - (width / 8);

    emitOperandPrefixIfNeeded(width);
    emitByte(signExtend8 ? 0x6a : 0x68);
    if (signExtend8) {
        emitByte((U8)value);
        if (width == 16) {
            pushedValue = (U16)(S16)(S8)value;
        } else {
            pushedValue = (U32)(S32)(S8)value;
        }
    } else if (width == 16) {
        emitWord((U16)value);
        pushedValue &= 0xffff;
    } else {
        emitDword(value);
    }
    runTestCPU();

    verifyRegisters(expectedRegs, name);
    verifyPushStack(width, expectedRegs[R_SP], pushedValue, name);
    verifyFlagsUnchanged(name);
}

void runPushA(int width, const char* name) {
    U32 expectedRegs[8];
    U32 values[8] = {
        0xffff1111,
        0xeeee2222,
        0xdddd3333,
        0xcccc4444,
        STACK_START,
        0xbbbb6666,
        0xaaaa7777,
        0x99998888
    };

    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
    initRegisters(expectedRegs);
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = values[i];
        expectedRegs[i] = values[i];
    }
    expectedRegs[R_SP] = STACK_START - (width == 16 ? 16 : 32);

    emitOperandPrefixIfNeeded(width);
    emitByte(0x60);
    runTestCPU();

    verifyRegisters(expectedRegs, name);
    U32 sp = expectedRegs[R_SP];
    int step = width / 8;
    U32 expectedStack[8] = {
        values[R_DI],
        values[R_SI],
        values[R_BP],
        STACK_START,
        values[R_BX],
        values[R_DX],
        values[R_CX],
        values[R_AX]
    };
    for (int i = 0; i < 8; ++i) {
        if (width == 16) {
            if (memory->readw(stackAddress(sp + i * step)) != (expectedStack[i] & 0xffff)) {
                failed("%s stack value", name);
            }
        } else if (memory->readd(stackAddress(sp + i * step)) != expectedStack[i]) {
            failed("%s stack value", name);
        }
    }
    verifyFlagsUnchanged(name);
}

void runPopA(int width, const char* name) {
    U32 expectedRegs[8];
    U32 values[8] = {
        0xffff1111,
        0xeeee2222,
        0xdddd3333,
        0xcccc4444,
        STACK_START,
        0xbbbb6666,
        0xaaaa7777,
        0x99998888
    };

    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
    initRegisters(expectedRegs);
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | i;
        expectedRegs[i] = cpu->reg[i].u32;
    }
    cpu->reg[R_SP].u32 = STACK_START - (width == 16 ? 16 : 32);
    expectedRegs[R_SP] = STACK_START;
    int step = width / 8;
    U32 sp = cpu->reg[R_SP].u32;
    U32 stackValues[8] = {
        values[R_DI],
        values[R_SI],
        values[R_BP],
        0x55555555,
        values[R_BX],
        values[R_DX],
        values[R_CX],
        values[R_AX]
    };
    for (int i = 0; i < 8; ++i) {
        if (width == 16) {
            memory->writew(stackAddress(sp + i * step), stackValues[i]);
        } else {
            memory->writed(stackAddress(sp + i * step), stackValues[i]);
        }
    }
    for (int i = 0; i < 8; ++i) {
        if (i == R_SP) {
            continue;
        }
        if (width == 16) {
            expectedRegs[i] = (expectedRegs[i] & 0xffff0000) | (values[i] & 0xffff);
        } else {
            expectedRegs[i] = values[i];
        }
    }

    emitOperandPrefixIfNeeded(width);
    emitByte(0x61);
    runTestCPU();

    verifyRegisters(expectedRegs, name);
    verifyFlagsUnchanged(name);
}

void runPushF(int width, const char* name) {
    U32 expectedRegs[8];
    U32 flags = FMASK_TEST | 2;

    newInstruction(0);
    cpu->big = true;
    initRegisters(expectedRegs);
    cpu->setFlags(flags, FMASK_ALL);
    writeStackGuard(width, STACK_START - (width / 8));
    expectedRegs[R_SP] = STACK_START - (width / 8);

    emitOperandPrefixIfNeeded(width);
    emitByte(0x9c);
    runTestCPU();

    verifyRegisters(expectedRegs, name);
    if (width == 16) {
        if ((memory->readw(stackAddress(expectedRegs[R_SP])) & (FMASK_TEST | 2)) != (FMASK_TEST | 2) ||
                memory->readw(stackAddress(expectedRegs[R_SP] + 2)) != GUARD16_A ||
                memory->readw(stackAddress(expectedRegs[R_SP] - 2)) != GUARD16_B) {
            failed("%s stack value", name);
        }
    } else if ((memory->readd(stackAddress(expectedRegs[R_SP])) & (FMASK_TEST | 2)) != (FMASK_TEST | 2) ||
            memory->readd(stackAddress(expectedRegs[R_SP] + 4)) != GUARD32_A ||
            memory->readd(stackAddress(expectedRegs[R_SP] - 4)) != GUARD32_B) {
        failed("%s stack value", name);
    }
}

} // namespace

void testPushR16_0x050() {
    for (int reg = 0; reg < 8; ++reg) {
        if (!testRunRegister(reg)) {
            continue;
        }
        runPushRegCase(reg, 16, "push r16");
    }
}

void testPushR32_0x250() {
    for (int reg = 0; reg < 8; ++reg) {
        if (!testRunRegister(reg)) {
            continue;
        }
        runPushRegCase(reg, 32, "push r32");
    }
}

void testPopR16_0x058() {
    for (int reg = 0; reg < 8; ++reg) {
        if (!testRunRegister(reg)) {
            continue;
        }
        runPopRegCase(reg, 16, "pop r16");
    }
}

void testPopR32_0x258() {
    for (int reg = 0; reg < 8; ++reg) {
        if (!testRunRegister(reg)) {
            continue;
        }
        runPopRegCase(reg, 32, "pop r32");
    }
}

void testPushA16_0x060() {
    runPushA(16, "pusha 16");
}

void testPushA32_0x260() {
    runPushA(32, "pusha 32");
}

void testPopA16_0x061() {
    runPopA(16, "popa 16");
}

void testPopA32_0x261() {
    runPopA(32, "popa 32");
}

void testPushIw_0x068() {
    runPushImmediate(16, PUSH16_VALUE, false, "push iw");
}

void testPushId_0x268() {
    runPushImmediate(32, PUSH32_VALUE, false, "push id");
}

void testPushIb16_0x06a() {
    runPushImmediate(16, 0xfc, true, "push ib 16 negative");
    runPushImmediate(16, 0x7f, true, "push ib 16 positive");
}

void testPushIb32_0x26a() {
    runPushImmediate(32, 0xfc, true, "push ib 32 negative");
    runPushImmediate(32, 0x7f, true, "push ib 32 positive");
}

void testPopE16_0x08f() {
    for (int reg = 0; reg < 8; ++reg) {
        if (!testRunRegister(reg)) {
            continue;
        }
        if (reg != R_SP) {
            runPopGroupRegCase(reg, 16, "pop e16 reg");
        }
    }
    runPopMemoryCases(16, "pop e16 memory");
}

void testPopE32_0x28f() {
    for (int reg = 0; reg < 8; ++reg) {
        if (!testRunRegister(reg)) {
            continue;
        }
        if (reg != R_SP) {
            runPopGroupRegCase(reg, 32, "pop e32 reg");
        }
    }
    runPopMemoryCases(32, "pop e32 memory");
    runPopEspMemoryCase("pop e32 esp memory");
}

void testPushF16_0x09c() {
    runPushF(16, "pushf");
}

void testPushF32_0x29c() {
    runPushF(32, "pushfd");
}

void testPushE16_0x0ff() {
    for (int reg = 0; reg < 8; ++reg) {
        if (!testRunRegister(reg)) {
            continue;
        }
        runPushGroupRegCase(reg, 16, "push e16 reg");
    }
    runPushMemoryCases(16, "push e16 memory");
}

void testPushE32_0x2ff() {
    for (int reg = 0; reg < 8; ++reg) {
        if (!testRunRegister(reg)) {
            continue;
        }
        runPushGroupRegCase(reg, 32, "push e32 reg");
    }
    runPushMemoryCases(32, "push e32 memory");
}

#endif
