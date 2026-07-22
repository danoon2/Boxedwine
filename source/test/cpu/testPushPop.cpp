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
#include "ksignal.h"
#include "../../emulation/cpu/common/common_sse.h"

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

void runCliProtectionFault() {
    constexpr U32 HANDLER_OFFSET = 2;

    newInstruction(0);
    cpu->big = true;
    testContext().process->sigActions[K_SIGSEGV].handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    testContext().process->sigActions[K_SIGSEGV].flags = 0;

    emitByte(0xfd); // std
    emitByte(0xfa); // cli
    runTestCPU();

    KSigAction& action = testContext().process->sigActions[K_SIGSEGV];
    if (action.sigInfo[0] != K_SIGSEGV) {
        failed("cli did not raise SIGSEGV");
        action.reset();
        return;
    }
    if (action.sigInfo[1] != 0) {
        failed("cli SIGSEGV error");
    }
    if (action.sigInfo[4] != 13) {
        failed("cli SIGSEGV trap number");
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 13) {
        failed("cli context trap number");
    }
    if (memory->readd(context + 0x48) != 0) {
        failed("cli context error");
    }
    if (memory->readd(context + 0x4c) != 1) {
        failed("cli context eip");
    }
    if (!(memory->readd(context + 0x54) & DF)) {
        failed("cli context lost DF");
    }
    if (cpu->flags & DF) {
        failed("cli signal handler kept DF");
    }

    action.reset();
}

void runInt2dRaisesInterruptProtectionFault() {
    constexpr U32 INTERRUPT_VECTOR = 0x2d;
    constexpr U32 INTERRUPT_ERROR = (INTERRUPT_VECTOR << 3) | 2;
    constexpr U32 HANDLER_OFFSET = 2;

    newInstruction(0);
    cpu->big = true;
    KSigAction& action = testContext().process->sigActions[K_SIGSEGV];
    action.handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    action.flags = 0;
    KSigAction& illAction = testContext().process->sigActions[K_SIGILL];
    illAction.handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    illAction.flags = 0;

    emitByte(0xcd);
    emitByte(INTERRUPT_VECTOR);
    runTestCPU();

    if (action.sigInfo[0] != K_SIGSEGV) {
        failed("int 2d did not raise SIGSEGV");
        action.reset();
        illAction.reset();
        return;
    }
    if (action.sigInfo[1] != INTERRUPT_ERROR) {
        failed("int 2d SIGSEGV error");
    }
    if (action.sigInfo[4] != 13) {
        failed("int 2d SIGSEGV trap number");
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 13) {
        failed("int 2d context trap number");
    }
    if (memory->readd(context + 0x48) != INTERRUPT_ERROR) {
        failed("int 2d context error");
    }
    if (memory->readd(context + 0x4c) != 0) {
        failed("int 2d context eip");
    }

    action.reset();
    illAction.reset();
}

void runInt3ImmediateRaisesBreakpoint() {
    constexpr U32 HANDLER_OFFSET = 2;

    newInstruction(0);
    cpu->big = true;
    KSigAction& trapAction = testContext().process->sigActions[K_SIGTRAP];
    KSigAction& segvAction = testContext().process->sigActions[K_SIGSEGV];
    trapAction.reset();
    segvAction.reset();
    trapAction.handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    segvAction.handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    trapAction.flags = 0;
    segvAction.flags = 0;

    emitByte(0xcd);
    emitByte(0x03);
    runTestCPU();

    if (trapAction.sigInfo[0] != K_SIGTRAP) {
        if (segvAction.sigInfo[0] == K_SIGSEGV) {
            failed("int 3 immediate raised SIGSEGV");
        } else {
            failed("int 3 immediate did not raise SIGTRAP");
        }
        trapAction.reset();
        segvAction.reset();
        return;
    }
    if (trapAction.sigInfo[2] != 1) {
        failed("int 3 immediate SIGTRAP code");
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 3) {
        failed("int 3 immediate context trap number");
    }
    if (memory->readd(context + 0x48) != 0) {
        failed("int 3 immediate context error");
    }
    if (memory->readd(context + 0x4c) != HANDLER_OFFSET) {
        failed("int 3 immediate context eip");
    }

    trapAction.reset();
    segvAction.reset();
}

void runProtectionFaultBytes(const char* name, const U8* bytes, U32 byteCount, U32 expectedError = 0) {
    newInstruction(0);
    cpu->big = true;
    KSigAction& action = testContext().process->sigActions[K_SIGSEGV];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + byteCount;
    action.flags = 0;

    for (U32 i = 0; i < byteCount; ++i) {
        emitByte(bytes[i]);
    }
    runTestCPU();

    if (action.sigInfo[0] != K_SIGSEGV) {
        failed("%s did not raise SIGSEGV", name);
        action.reset();
        return;
    }
    if (action.sigInfo[1] != expectedError) {
        failed("%s SIGSEGV error", name);
    }
    if (action.sigInfo[4] != 13) {
        failed("%s SIGSEGV trap number", name);
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 13) {
        failed("%s context trap number", name);
    }
    if (memory->readd(context + 0x48) != expectedError) {
        failed("%s context error", name);
    }
    if (memory->readd(context + 0x4c) != 0) {
        failed("%s context eip", name);
    }

    action.reset();
}

void runPrefixedCliRaisesProtectionFault() {
    const U8 prefixedCli[] = { 0x64, 0x64, 0x64, 0x64, 0xfa };

    runProtectionFaultBytes("prefixed cli", prefixedCli, sizeof(prefixedCli));
}

void runOverlongPrefixedCliDoesNotPoisonCodeCache() {
    const U8 overlongPrefixedCli[] = {
        0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
        0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0xfa,
        0xc3
    };
    const U8 prefixedCli[] = { 0x64, 0x64, 0x64, 0x64, 0xfa };

    newInstruction(0);
    cpu->big = true;
    for (U32 i = 0; i < sizeof(overlongPrefixedCli); ++i) {
        emitByte(overlongPrefixedCli[i]);
    }

    DecodedOp* op = cpu->getNextOp();
    if (!op) {
        failed("overlong prefixed cli did not decode");
        return;
    }
    if (!op->len) {
        failed("overlong prefixed cli decoded with zero length");
        return;
    }

    for (U32 i = 0; i < sizeof(prefixedCli); ++i) {
        memory->writeb(TEST_CODE_ADDRESS + i, prefixedCli[i]);
    }

    if (memory->getDecodedOp(TEST_CODE_ADDRESS)) {
        failed("overlong prefixed cli remained cached after overwrite");
        return;
    }

    KSigAction& action = testContext().process->sigActions[K_SIGSEGV];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + sizeof(prefixedCli);
    action.flags = 0;
    cpu->eip.u32 = 0;
    cpu->nextOp = nullptr;
    testContext().codeIp = TEST_CODE_ADDRESS + sizeof(prefixedCli);
    runTestCPU();

    if (action.sigInfo[0] != K_SIGSEGV) {
        failed("overwritten prefixed cli did not raise SIGSEGV");
    }

    action.reset();
}

void runPortIoRaisesProtectionFault() {
    const U8 inAlIb[] = { 0xe4, 0x11 };
    const U8 inEaxIb[] = { 0xe5, 0x11 };
    const U8 outIbAl[] = { 0xe6, 0x11 };
    const U8 outIbEax[] = { 0xe7, 0x11 };
    const U8 inEaxDx[] = { 0xed };
    const U8 outDxAl[] = { 0xee };
    const U8 outDxEax[] = { 0xef };

    runProtectionFaultBytes("in al,ib", inAlIb, sizeof(inAlIb));
    runProtectionFaultBytes("in eax,ib", inEaxIb, sizeof(inEaxIb));
    runProtectionFaultBytes("out ib,al", outIbAl, sizeof(outIbAl));
    runProtectionFaultBytes("out ib,eax", outIbEax, sizeof(outIbEax));
    runProtectionFaultBytes("in eax,dx", inEaxDx, sizeof(inEaxDx));
    runProtectionFaultBytes("out dx,al", outDxAl, sizeof(outDxAl));
    runProtectionFaultBytes("out dx,eax", outDxEax, sizeof(outDxEax));
}

void runHltRaisesProtectionFault() {
    const U8 hlt[] = { 0xf4 };

    runProtectionFaultBytes("hlt", hlt, sizeof(hlt));
}

void runInvalidInterruptRaisesProtectionFault() {
    constexpr U32 INTERRUPT_VECTOR = 0xff;
    constexpr U32 INTERRUPT_ERROR = (INTERRUPT_VECTOR << 3) | 2;
    const U8 intFF[] = { 0xcd, INTERRUPT_VECTOR };

    runProtectionFaultBytes("int ff", intFF, sizeof(intFF), INTERRUPT_ERROR);
}

void runControlRegisterAccessRaisesProtectionFault() {
    const U8 movEaxCr0[] = { 0x0f, 0x20, 0xc0 };
    const U8 movEaxCr4[] = { 0x0f, 0x20, 0xe0 };
    const U8 movCr0Eax[] = { 0x0f, 0x22, 0xc0 };
    const U8 movCr4Eax[] = { 0x0f, 0x22, 0xe0 };

    runProtectionFaultBytes("mov eax,cr0", movEaxCr0, sizeof(movEaxCr0));
    runProtectionFaultBytes("mov eax,cr4", movEaxCr4, sizeof(movEaxCr4));
    runProtectionFaultBytes("mov cr0,eax", movCr0Eax, sizeof(movCr0Eax));
    runProtectionFaultBytes("mov cr4,eax", movCr4Eax, sizeof(movCr4Eax));
}

void runNullSegmentMoffsRaisesProtectionFault() {
    const U8 esMoffs[] = { 0x06, 0x31, 0xc0, 0x8e, 0xc0, 0x26, 0xa1, 0, 0, 0, 0xf0 };
    const U8 gsMoffs[] = { 0x0f, 0xa8, 0x31, 0xc0, 0x8e, 0xe8, 0x65, 0xa1, 0, 0, 0, 0xf0 };

    struct NullSegCase {
        const char* name;
        const U8* bytes;
        U32 byteCount;
        U32 expectedEip;
    };
    const NullSegCase cases[] = {
        { "null es moffs", esMoffs, sizeof(esMoffs), 5 },
        { "null gs moffs", gsMoffs, sizeof(gsMoffs), 6 },
    };

    for (const NullSegCase& data : cases) {
        newInstruction(0);
        cpu->big = true;
        KSigAction& action = testContext().process->sigActions[K_SIGSEGV];
        action.reset();
        action.handlerAndSigAction = TEST_CODE_ADDRESS + data.byteCount;
        action.flags = 0;

        for (U32 i = 0; i < data.byteCount; ++i) {
            emitByte(data.bytes[i]);
        }
        runTestCPU();

        if (action.sigInfo[0] != K_SIGSEGV) {
            failed("%s did not raise SIGSEGV", data.name);
            action.reset();
            continue;
        }
        if (action.sigInfo[1] != 0) {
            failed("%s SIGSEGV error", data.name);
        }
        if (action.sigInfo[4] != 13) {
            failed("%s SIGSEGV trap number", data.name);
        }

        U32 context = memory->readd(cpu->reg[4].u32 + 12);
        if (memory->readd(context + 0x44) != 13) {
            failed("%s context trap number", data.name);
        }
        if (memory->readd(context + 0x48) != 0) {
            failed("%s context error", data.name);
        }
        if (memory->readd(context + 0x4c) != data.expectedEip) {
            failed("%s context eip", data.name);
        }

        action.reset();
    }
}

void runPopSsFromCodeSelectorRaisesProtectionFault() {
    constexpr U32 HANDLER_OFFSET = 2;
    constexpr U32 MODIFY_LDT_CONTENTS_CODE = 2;

    newInstruction(0);
    cpu->big = true;
    testContext().process->getLDT(TEST_CODE_SEG >> 3)->contents = MODIFY_LDT_CONTENTS_CODE;

    KSigAction& action = testContext().process->sigActions[K_SIGSEGV];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    action.flags = 0;

    emitByte(0x0e); // push cs
    emitByte(0x17); // pop ss
    runTestCPU();

    if (action.sigInfo[0] != K_SIGSEGV) {
        failed("pop ss from code selector did not raise SIGSEGV");
        action.reset();
        return;
    }
    if (action.sigInfo[1] != 0) {
        failed("pop ss from code selector SIGSEGV error");
    }
    if (action.sigInfo[4] != 13) {
        failed("pop ss from code selector SIGSEGV trap number");
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 13) {
        failed("pop ss from code selector context trap number");
    }
    if (memory->readd(context + 0x48) != 0) {
        failed("pop ss from code selector context error");
    }
    if (memory->readd(context + 0x4c) != 1) {
        failed("pop ss from code selector context eip");
    }

    action.reset();
}

void runInstructionFetchFaultSetsExecuteBit() {
    newInstruction(0);
    cpu->big = true;
    memory->writeb(TEST_HEAP_ADDRESS, 0xc3); // ret, but the heap page is not executable.

    KSigAction& action = testContext().process->sigActions[K_SIGSEGV];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS;
    action.flags = 0;

    cpu->seg[CS].address = 0;
    cpu->seg[CS].value = 0xf;
    cpu->eip.u32 = TEST_HEAP_ADDRESS;
    runTestCPU();

    if (action.sigInfo[0] != K_SIGSEGV) {
        failed("instruction fetch fault did not raise SIGSEGV");
        action.reset();
        return;
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != EXCEPTION_PAGE_FAULT) {
        failed("instruction fetch fault context trap number");
    }
    if (!(memory->readd(context + 0x48) & 0x10)) {
        failed("instruction fetch fault context missing execute bit");
    }

    action.reset();
}

void clearDebugRegisters() {
    for (U32 i = 0; i < 8; ++i) {
        testContext().thread->debugRegs[i] = 0;
    }
    testContext().thread->updateDebugTrapActive();
}

void runSingleStepTrap() {
    constexpr U32 HANDLER_OFFSET = 7;

    newInstruction(0);
    cpu->big = true;
    KSigAction& action = testContext().process->sigActions[K_SIGTRAP];
    action.handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    action.flags = 0;

    emitByte(0x68); // push imm32
    emitDword(TF | 2);
    emitByte(0x9d); // popf
    emitByte(0x90); // nop, should raise #DB after it executes
    runTestCPU();

    if (action.sigInfo[0] != K_SIGTRAP) {
        failed("single-step did not raise SIGTRAP");
        action.reset();
        return;
    }
    if (action.sigInfo[2] != 2) {
        failed("single-step SIGTRAP code");
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 1) {
        failed("single-step context trap number");
    }
    if (memory->readd(context + 0x4c) != HANDLER_OFFSET) {
        failed("single-step context eip");
    }
    if (memory->readd(context + 0x54) & TF) {
        failed("single-step context kept TF");
    }

    action.reset();
}

void runSingleStepTrapAfterRet() {
    constexpr U32 TARGET_OFFSET = 8;

    newInstruction(TF);
    cpu->big = true;
    clearDebugRegisters();
    cpu->push32(TARGET_OFFSET);

    KSigAction& action = testContext().process->sigActions[K_SIGTRAP];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + TARGET_OFFSET;
    action.flags = 0;

    emitByte(0xc3); // ret, should raise #DB at the return target
    while (testContext().codeIp < TEST_CODE_ADDRESS + TARGET_OFFSET) {
        emitByte(0x90);
    }
    emitByte(0x90);
    runTestCPU();

    if (action.sigInfo[0] != K_SIGTRAP) {
        failed("single-step ret did not raise SIGTRAP");
        action.reset();
        return;
    }
    if (action.sigInfo[2] != 2) {
        failed("single-step ret SIGTRAP code");
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 1) {
        failed("single-step ret context trap number");
    }
    if (memory->readd(context + 0x4c) != TARGET_OFFSET) {
        failed("single-step ret context eip");
    }
    if (memory->readd(context + 0x54) & TF) {
        failed("single-step ret context kept TF");
    }

    action.reset();
}

void runIcebpRaisesSingleStepTrap() {
    constexpr U32 HANDLER_OFFSET = 1;

    newInstruction(0);
    cpu->big = true;
    KSigAction& trapAction = testContext().process->sigActions[K_SIGTRAP];
    KSigAction& illAction = testContext().process->sigActions[K_SIGILL];
    KSigAction& segvAction = testContext().process->sigActions[K_SIGSEGV];
    trapAction.reset();
    illAction.reset();
    segvAction.reset();
    trapAction.handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    illAction.handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    segvAction.handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    trapAction.flags = 0;
    illAction.flags = 0;
    segvAction.flags = 0;

    emitByte(0xf1); // icebp/int1, should raise #DB after the instruction
    emitByte(0x90);
    runTestCPU();

    if (trapAction.sigInfo[0] != K_SIGTRAP) {
        if (illAction.sigInfo[0] == K_SIGILL) {
            failed("icebp raised SIGILL");
        } else if (segvAction.sigInfo[0] == K_SIGSEGV) {
            failed("icebp raised SIGSEGV");
        } else {
            failed("icebp did not raise SIGTRAP");
        }
        trapAction.reset();
        illAction.reset();
        segvAction.reset();
        return;
    }
    if (trapAction.sigInfo[2] != 2) {
        failed("icebp SIGTRAP code");
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 1) {
        failed("icebp context trap number");
    }
    if (memory->readd(context + 0x4c) != HANDLER_OFFSET) {
        failed("icebp context eip");
    }

    trapAction.reset();
    illAction.reset();
    segvAction.reset();
}

void runDivpsRaisesSimdException(bool invalidOperation) {
    constexpr U32 HANDLER_OFFSET = 3;
    constexpr U32 MXCSR_INVALID_OPERATION_MASK = 1u << 7;
    constexpr U32 MXCSR_DIVIDE_BY_ZERO_MASK = 1u << 9;

    newInstruction(0);
    cpu->big = true;
    cpu->mxcsr = invalidOperation ? (0x1f80 & ~MXCSR_INVALID_OPERATION_MASK) : (0x1f80 & ~MXCSR_DIVIDE_BY_ZERO_MASK);
    for (U32 i = 0; i < 4; ++i) {
        cpu->xmm[0].ps.u32[i] = 0;
        cpu->xmm[1].ps.u32[i] = invalidOperation ? 0 : 0x3f800000;
    }

    KSigAction& action = testContext().process->sigActions[K_SIGFPE];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    action.flags = 0;

    emitByte(0x0f);
    emitByte(0x5e);
    emitByte(0xc8); // divps xmm0,xmm1
    emitByte(0x90);
    runTestCPU();

    if (action.sigInfo[0] != K_SIGFPE) {
        failed("%s divps did not raise SIGFPE", invalidOperation ? "invalid" : "divide-by-zero");
        action.reset();
        return;
    }
    if (action.sigInfo[2] != (invalidOperation ? K_FPE_FLTINV : K_FPE_FLTDIV)) {
        failed("%s divps SIGFPE code", invalidOperation ? "invalid" : "divide-by-zero");
    }
    if (action.sigInfo[4] != 19) {
        failed("%s divps trap number", invalidOperation ? "invalid" : "divide-by-zero");
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 19) {
        failed("%s divps context trap number", invalidOperation ? "invalid" : "divide-by-zero");
    }
    if (memory->readd(context + 0x4c) != 0) {
        failed("%s divps context eip", invalidOperation ? "invalid" : "divide-by-zero");
    }

    action.reset();
}

void runDivssRaisesSimdException(bool invalidOperation, bool memoryOperand) {
    constexpr U32 MXCSR_INVALID_OPERATION_MASK = 1u << 7;
    constexpr U32 MXCSR_DIVIDE_BY_ZERO_MASK = 1u << 9;
    constexpr U32 SRC_ADDRESS = MEM_BASE;
    U32 handlerOffset = memoryOperand ? 8 : 4;

    newInstruction(0);
    cpu->big = true;
    cpu->mxcsr = invalidOperation ? (0x1f80 & ~MXCSR_INVALID_OPERATION_MASK) : (0x1f80 & ~MXCSR_DIVIDE_BY_ZERO_MASK);
    cpu->xmm[0].ps.u32[0] = invalidOperation ? 0 : 0x3f800000;
    cpu->xmm[1].ps.u32[0] = 0;
    cpu->xmm[0].ps.u32[1] = 0x11111111;
    cpu->xmm[0].ps.u32[2] = 0x22222222;
    cpu->xmm[0].ps.u32[3] = 0x33333333;
    memory->writed(TEST_HEAP_ADDRESS + SRC_ADDRESS, 0);

    KSigAction& action = testContext().process->sigActions[K_SIGFPE];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + handlerOffset;
    action.flags = 0;

    emitByte(0xf3);
    emitByte(0x0f);
    emitByte(0x5e);
    if (memoryOperand) {
        emitByte(0x05); // divss xmm0,m32
        emitDword(SRC_ADDRESS);
    } else {
        emitByte(0xc1); // divss xmm0,xmm1
    }
    emitByte(0x90);
    runTestCPU();

    const char* kind = invalidOperation ? "invalid" : "divide-by-zero";
    const char* operand = memoryOperand ? "memory" : "register";
    if (action.sigInfo[0] != K_SIGFPE) {
        failed("%s divss %s did not raise SIGFPE", kind, operand);
        action.reset();
        return;
    }
    if (action.sigInfo[2] != (invalidOperation ? K_FPE_FLTINV : K_FPE_FLTDIV)) {
        failed("%s divss %s SIGFPE code", kind, operand);
    }
    if (action.sigInfo[4] != 19) {
        failed("%s divss %s trap number", kind, operand);
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 19) {
        failed("%s divss %s context trap number", kind, operand);
    }
    if (memory->readd(context + 0x4c) != 0) {
        failed("%s divss %s context eip", kind, operand);
    }

    action.reset();
}

void runDivpsDoesNotRaiseDivideByZeroForNonFiniteDividend() {
    constexpr U32 MXCSR_DIVIDE_BY_ZERO_FLAG = 1u << 2;
    constexpr U32 MXCSR_DIVIDE_BY_ZERO_MASK = 1u << 9;

    newInstruction(0);
    cpu->big = true;
    cpu->mxcsr = 0x1f80 & ~MXCSR_DIVIDE_BY_ZERO_MASK;
    cpu->xmm[0].ps.u32[0] = 0x7fc00000; // qNaN
    cpu->xmm[0].ps.u32[1] = 0x7f800000; // +Inf
    cpu->xmm[0].ps.u32[2] = 0xff800000; // -Inf
    cpu->xmm[0].ps.u32[3] = 0x7fc00001; // qNaN payload
    for (U32 i = 0; i < 4; ++i) {
        cpu->xmm[1].ps.u32[i] = 0;
    }

    KSigAction& action = testContext().process->sigActions[K_SIGFPE];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + 4;
    action.flags = 0;

    emitByte(0x0f);
    emitByte(0x5e);
    emitByte(0xc8); // divps xmm0,xmm1
    emitByte(0x90);
    runTestCPU();

    if (action.sigInfo[0] == K_SIGFPE) {
        failed("non-finite divps dividend raised SIGFPE");
    }
    if (cpu->mxcsr & MXCSR_DIVIDE_BY_ZERO_FLAG) {
        failed("non-finite divps dividend set divide-by-zero flag");
    }
    action.reset();
}

void runDivpsInfInfRaisesInvalidOperation() {
    constexpr U32 MXCSR_INVALID_OPERATION_MASK = 1u << 7;

    newInstruction(0);
    cpu->big = true;
    cpu->mxcsr = 0x1f80 & ~MXCSR_INVALID_OPERATION_MASK;
    for (U32 i = 0; i < 4; ++i) {
        cpu->xmm[0].ps.u32[i] = 0x7f800000;
        cpu->xmm[1].ps.u32[i] = 0x7f800000;
    }

    KSigAction& action = testContext().process->sigActions[K_SIGFPE];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + 4;
    action.flags = 0;

    emitByte(0x0f);
    emitByte(0x5e);
    emitByte(0xc8); // divps xmm0,xmm1
    emitByte(0x90);
    runTestCPU();

    if (action.sigInfo[0] != K_SIGFPE) {
        failed("inf/inf divps did not raise SIGFPE");
        action.reset();
        return;
    }
    if (action.sigInfo[2] != K_FPE_FLTINV) {
        failed("inf/inf divps SIGFPE code");
    }
    action.reset();
}

void runDivssDoesNotRaiseDivideByZeroForNonFiniteDividend(bool memoryOperand) {
    constexpr U32 MXCSR_DIVIDE_BY_ZERO_FLAG = 1u << 2;
    constexpr U32 MXCSR_DIVIDE_BY_ZERO_MASK = 1u << 9;
    constexpr U32 SRC_ADDRESS = MEM_BASE;

    newInstruction(0);
    cpu->big = true;
    cpu->mxcsr = 0x1f80 & ~MXCSR_DIVIDE_BY_ZERO_MASK;
    cpu->xmm[0].ps.u32[0] = 0x7fc00000;
    cpu->xmm[0].ps.u32[1] = 0x11111111;
    cpu->xmm[0].ps.u32[2] = 0x22222222;
    cpu->xmm[0].ps.u32[3] = 0x33333333;
    cpu->xmm[1].ps.u32[0] = 0;
    memory->writed(TEST_HEAP_ADDRESS + SRC_ADDRESS, 0);

    KSigAction& action = testContext().process->sigActions[K_SIGFPE];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + (memoryOperand ? 8 : 4);
    action.flags = 0;

    emitByte(0xf3);
    emitByte(0x0f);
    emitByte(0x5e);
    if (memoryOperand) {
        emitByte(0x05); // divss xmm0,m32
        emitDword(SRC_ADDRESS);
    } else {
        emitByte(0xc1); // divss xmm0,xmm1
    }
    emitByte(0x90);
    runTestCPU();

    if (action.sigInfo[0] == K_SIGFPE) {
        failed("non-finite divss %s dividend raised SIGFPE", memoryOperand ? "memory" : "register");
    }
    if (cpu->mxcsr & MXCSR_DIVIDE_BY_ZERO_FLAG) {
        failed("non-finite divss %s dividend set divide-by-zero flag", memoryOperand ? "memory" : "register");
    }
    action.reset();
}

void runSseDivFastPathDecision() {
    constexpr U32 DEFAULT_MXCSR = 0x1f80;
    constexpr U32 MXCSR_INVALID_OPERATION_MASK = 1u << 7;
    constexpr U32 MXCSR_DIVIDE_BY_ZERO_MASK = 1u << 9;

    if (common_sse_div_control_requires_slow_path(DEFAULT_MXCSR)) {
        failed("default SSE divide control marked slow");
    }
    if (!common_sse_div_control_requires_slow_path(DEFAULT_MXCSR & ~MXCSR_INVALID_OPERATION_MASK)) {
        failed("unmasked invalid operation SSE divide control marked fast");
    }
    if (!common_sse_div_control_requires_slow_path(DEFAULT_MXCSR & ~MXCSR_DIVIDE_BY_ZERO_MASK)) {
        failed("unmasked divide-by-zero SSE divide control marked fast");
    }
    if (common_sse_div_control_requires_slow_path(DEFAULT_MXCSR ^ 0x6000)) {
        failed("rounding-only SSE divide control marked slow");
    }
}

void runX87DivFastPathDecision() {
    constexpr U32 FPU_INVALID_OPERATION_MASK = 0x0001;
    constexpr U32 FPU_DIVIDE_BY_ZERO_MASK = 0x0004;

    FPU fpu;
    fpu.FINIT();
    fpu.tags[0] = TAG_Valid;
    fpu.tags[1] = TAG_Valid;
    if (fpu_div_control_or_tag_requires_slow_path(&fpu, 0, 1, false)) {
        failed("default x87 divide control and valid tags marked slow");
    }

    fpu.cw &= ~FPU_DIVIDE_BY_ZERO_MASK;
    if (!fpu_div_control_or_tag_requires_slow_path(&fpu, 0, 1, false)) {
        failed("unmasked x87 divide-by-zero marked fast");
    }

    fpu.FINIT();
    fpu.tags[0] = TAG_Valid;
    fpu.tags[1] = TAG_Valid;
    fpu.cw &= ~FPU_INVALID_OPERATION_MASK;
    if (!fpu_div_control_or_tag_requires_slow_path(&fpu, 0, 1, false)) {
        failed("unmasked x87 invalid operation marked fast");
    }

    fpu.FINIT();
    fpu.tags[0] = TAG_Valid;
    fpu.tags[1] = TAG_Zero;
    if (!fpu_div_control_or_tag_requires_slow_path(&fpu, 0, 1, false)) {
        failed("x87 FDIV zero divisor tag marked fast");
    }

    fpu.FINIT();
    fpu.tags[0] = TAG_Zero;
    fpu.tags[1] = TAG_Valid;
    if (!fpu_div_control_or_tag_requires_slow_path(&fpu, 0, 1, true)) {
        failed("x87 FDIVR zero divisor tag marked fast");
    }

    fpu.FINIT();
    fpu.tags[0] = TAG_Empty;
    fpu.tags[1] = TAG_Valid;
    if (!fpu_div_control_or_tag_requires_slow_path(&fpu, 0, 1, false)) {
        failed("x87 empty divide operand tag marked fast");
    }
}

void emitBytes(const U8* bytes, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        emitByte(bytes[i]);
    }
}

void runX87FwaitRaisesPendingException(bool stackCheck) {
    static const U8 stackCheckCode[] = {
        0x83, 0xec, 0x04,                   // sub $0x4,%esp
        0x66, 0xc7, 0x04, 0x24, 0xfe, 0x03, // movw $0x3fe,(%esp)
        0x9b, 0xd9, 0x7c, 0x24, 0x02,       // fstcw 0x2(%esp)
        0xd9, 0x2c, 0x24,                   // fldcw (%esp)
        0xd9, 0xee,                         // fldz
        0xd9, 0xe8,                         // fld1
        0xde, 0xf1,                         // fdivp
        0xdd, 0xd8,                         // fstp %st(0)
        0xdd, 0xd8,                         // fstp %st(0)
        0x9b,                               // fwait
        0xdb, 0xe2,                         // fnclex
    };
    static const U8 divZeroCode[] = {
        0x83, 0xec, 0x04,                   // sub $0x4,%esp
        0x66, 0xc7, 0x04, 0x24, 0xfb, 0x03, // movw $0x3fb,(%esp)
        0x9b, 0xd9, 0x7c, 0x24, 0x02,       // fstcw 0x2(%esp)
        0xd9, 0x2c, 0x24,                   // fldcw (%esp)
        0xdd, 0xd8,                         // fstp %st(0)
        0xd9, 0xee,                         // fldz
        0xd9, 0xe8,                         // fld1
        0xde, 0xf1,                         // fdivp
        0x9b,                               // fwait
        0xdb, 0xe2,                         // fnclex
    };
    constexpr U32 STACK_CHECK_FWAIT_OFFSET = 0x1b;
    constexpr U32 DIV_ZERO_FWAIT_OFFSET = 0x19;
    constexpr U32 STACK_CHECK_STATUS = 0x0041;
    constexpr U32 DIV_ZERO_STATUS = 0x0004;

    const U8* bytes = stackCheck ? stackCheckCode : divZeroCode;
    size_t count = stackCheck ? sizeof(stackCheckCode) : sizeof(divZeroCode);
    U32 expectedEip = stackCheck ? STACK_CHECK_FWAIT_OFFSET : DIV_ZERO_FWAIT_OFFSET;
    U32 expectedStatusBits = stackCheck ? STACK_CHECK_STATUS : DIV_ZERO_STATUS;
    U32 expectedCode = stackCheck ? K_FPE_FLTINV : K_FPE_FLTDIV;
    const char* name = stackCheck ? "stack-check" : "divide-by-zero";

    newInstruction(0);
    cpu->big = true;
    cpu->fpu.FINIT();

    KSigAction& action = testContext().process->sigActions[K_SIGFPE];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + expectedEip + 1;
    action.flags = 0;

    emitBytes(bytes, count);
    runTestCPU();

    if (action.sigInfo[0] != K_SIGFPE) {
        failed("x87 %s fwait did not raise SIGFPE", name);
        action.reset();
        return;
    }
    if (action.sigInfo[2] != expectedCode) {
        failed("x87 %s SIGFPE code", name);
    }
    if (action.sigInfo[4] != 16) {
        failed("x87 %s trap number", name);
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 16) {
        failed("x87 %s context trap number", name);
    }
    if (memory->readd(context + 0x4c) != expectedEip) {
        failed("x87 %s context eip", name);
    }
    U32 fpregs = memory->readd(context + 0x60);
    if (fpregs != context + 0x70) {
        failed("x87 %s context fpregs pointer", name);
        action.reset();
        return;
    }
    if (memory->readd(fpregs + 0x0c) != expectedEip) {
        failed("x87 %s fpu error offset", name);
    }
    if ((memory->readd(fpregs + 0x04) & expectedStatusBits) != expectedStatusBits) {
        failed("x87 %s context status word", name);
    }

    action.reset();
}

void runHardwareBreakpointTrap() {
    constexpr U32 HANDLER_OFFSET = 1;

    newInstruction(0);
    cpu->big = true;
    clearDebugRegisters();
    KSigAction& action = testContext().process->sigActions[K_SIGTRAP];
    action.handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    action.flags = 0;
    testContext().thread->debugRegs[0] = TEST_CODE_ADDRESS;
    testContext().thread->debugRegs[7] = 3;
    testContext().thread->updateDebugTrapActive();

    emitByte(0x90); // nop, should not execute before the #DB
    runTestCPU();

    if (action.sigInfo[0] != K_SIGTRAP) {
        failed("hardware breakpoint did not raise SIGTRAP");
        action.reset();
        clearDebugRegisters();
        return;
    }
    if (action.sigInfo[2] != 4) {
        failed("hardware breakpoint SIGTRAP code");
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 1) {
        failed("hardware breakpoint context trap number");
    }
    if (memory->readd(context + 0x4c) != 0) {
        failed("hardware breakpoint context eip");
    }
    if ((testContext().thread->debugRegs[6] & 0xf) != 1) {
        failed("hardware breakpoint Dr6 B0");
    }
    if (testContext().thread->debugRegs[6] & 0x4000) {
        failed("hardware breakpoint Dr6 BS");
    }

    action.reset();
    clearDebugRegisters();
}

void runHardwareBreakpointIgnoresNonExecutableAddress() {
    newInstruction(0);
    cpu->big = true;
    clearDebugRegisters();

    U32 oldEip = cpu->eip.u32;
    U32 oldCsAddress = cpu->seg[CS].address;
    U32 oldCsValue = cpu->seg[CS].value;
    KSigAction& action = testContext().process->sigActions[K_SIGTRAP];
    action.reset();

    cpu->eip.u32 = 0;
    cpu->seg[CS].address = 0;
    cpu->seg[CS].value = 0xf;
    testContext().thread->debugRegs[1] = 0;
    testContext().thread->debugRegs[7] = 1u << 2;
    testContext().thread->updateDebugTrapActive();

    if (testContext().thread->debugTrapBeforeInstruction()) {
        failed("hardware breakpoint fired on non-executable address");
    }
    if (action.sigInfo[0] == K_SIGTRAP) {
        failed("hardware breakpoint populated SIGTRAP on non-executable address");
    }

    cpu->eip.u32 = oldEip;
    cpu->seg[CS].address = oldCsAddress;
    cpu->seg[CS].value = oldCsValue;
    action.reset();
    clearDebugRegisters();
}

void runDataHardwareBreakpointTrap() {
    constexpr U32 DATA_OFFSET = 0x80;
    constexpr U32 DATA_ADDRESS = TEST_HEAP_ADDRESS + DATA_OFFSET;
    constexpr U32 HANDLER_OFFSET = 5;
    constexpr U32 VALUE = 0x12345678;

    newInstruction(0);
    cpu->big = true;
    clearDebugRegisters();
    cpu->reg[0].u32 = VALUE;
    memory->writed(DATA_ADDRESS, 0);

    KSigAction& action = testContext().process->sigActions[K_SIGTRAP];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + HANDLER_OFFSET;
    action.flags = 0;

    emitByte(0xa3); // mov moffs32,eax
    emitDword(DATA_OFFSET);

    testContext().thread->debugRegs[0] = DATA_ADDRESS;
    testContext().thread->debugRegs[7] = 3 | (1u << 16) | (3u << 18);
    testContext().thread->updateDebugTrapActive();

    runTestCPU();

    if (memory->readd(DATA_ADDRESS) != VALUE) {
        failed("data hardware breakpoint did not complete write");
    }
    if (action.sigInfo[0] != K_SIGTRAP) {
        failed("data hardware breakpoint did not raise SIGTRAP");
        action.reset();
        clearDebugRegisters();
        return;
    }
    if (action.sigInfo[2] != 4) {
        failed("data hardware breakpoint SIGTRAP code");
    }

    U32 context = memory->readd(cpu->reg[4].u32 + 12);
    if (memory->readd(context + 0x44) != 1) {
        failed("data hardware breakpoint context trap number");
    }
    if (memory->readd(context + 0x4c) != HANDLER_OFFSET) {
        failed("data hardware breakpoint context eip");
    }
    if ((testContext().thread->debugRegs[6] & 0xf) != 1) {
        failed("data hardware breakpoint Dr6 B0");
    }
    if (testContext().thread->debugRegs[6] & 0x4000) {
        failed("data hardware breakpoint Dr6 BS");
    }
    if (!testContext().thread->inSignal) {
        failed("data hardware breakpoint did not enter SIGTRAP handler");
    }
    if (testContext().thread->isDebugTrapActive()) {
        failed("data hardware breakpoint stayed active inside SIGTRAP handler");
    }

    U32 returnAddress = cpu->pop32();
    if (returnAddress != SIG_RETURN_ADDRESS) {
        failed("data hardware breakpoint signal return address");
    } else {
        onExitSignal(cpu, nullptr);
    }

    action.reset();
    clearDebugRegisters();
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

void testCliRaisesProtectionFault() {
    runCliProtectionFault();
}

void testPrefixedCliRaisesProtectionFault() {
    runPrefixedCliRaisesProtectionFault();
}

void testOverlongPrefixedCliDoesNotPoisonCodeCache() {
    runOverlongPrefixedCliDoesNotPoisonCodeCache();
}

void testInt2dRaisesInterruptProtectionFault() {
    runInt2dRaisesInterruptProtectionFault();
}

void testInt3ImmediateRaisesBreakpoint() {
    runInt3ImmediateRaisesBreakpoint();
}

void testPortIoRaisesProtectionFault() {
    runPortIoRaisesProtectionFault();
}

void testHltRaisesProtectionFault() {
    runHltRaisesProtectionFault();
}

void testInvalidInterruptRaisesProtectionFault() {
    runInvalidInterruptRaisesProtectionFault();
}

void testControlRegisterAccessRaisesProtectionFault() {
    runControlRegisterAccessRaisesProtectionFault();
}

void testNullSegmentMoffsRaisesProtectionFault() {
    runNullSegmentMoffsRaisesProtectionFault();
}

void testPopSsFromCodeSelectorRaisesProtectionFault() {
    runPopSsFromCodeSelectorRaisesProtectionFault();
}

void testInstructionFetchFaultSetsExecuteBit() {
    runInstructionFetchFaultSetsExecuteBit();
}

void testSingleStepRaisesTrap() {
    runSingleStepTrap();
}

void testSingleStepRaisesTrapAfterRet() {
    runSingleStepTrapAfterRet();
}

void testIcebpRaisesSingleStepTrap() {
    runIcebpRaisesSingleStepTrap();
}

void testDivpsRaisesSimdException() {
    runDivpsRaisesSimdException(false);
    runDivpsRaisesSimdException(true);
    runDivpsDoesNotRaiseDivideByZeroForNonFiniteDividend();
    runDivpsInfInfRaisesInvalidOperation();
}

void testDivssRaisesSimdException() {
    runDivssRaisesSimdException(false, false);
    runDivssRaisesSimdException(true, false);
    runDivssRaisesSimdException(false, true);
    runDivssRaisesSimdException(true, true);
    runDivssDoesNotRaiseDivideByZeroForNonFiniteDividend(false);
    runDivssDoesNotRaiseDivideByZeroForNonFiniteDividend(true);
}

void testSseDivFastPathDecision() {
    runSseDivFastPathDecision();
}

void testX87DivFastPathDecision() {
    runX87DivFastPathDecision();
}

void testX87FwaitRaisesPendingException() {
    runX87FwaitRaisesPendingException(true);
    runX87FwaitRaisesPendingException(false);
}

void testHardwareBreakpointRaisesTrap() {
    runHardwareBreakpointTrap();
}

void testHardwareBreakpointIgnoresNonExecutableAddress() {
    runHardwareBreakpointIgnoresNonExecutableAddress();
}

void testDataHardwareBreakpointRaisesTrap() {
    runDataHardwareBreakpointTrap();
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
