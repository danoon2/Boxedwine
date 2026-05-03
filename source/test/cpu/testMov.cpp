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

#include "testMov.h"
#include "testCPU.h"
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

constexpr U32 REG_GUARD = 0xA55A0000;
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

enum MovDirection {
    MOV_E_FROM_G,
    MOV_G_FROM_E
};

enum ExtendMode {
    EXTEND_ZERO,
    EXTEND_SIGN
};

struct MovCase {
    U32 dst;
    U32 src;
};

struct AddressCase {
    U32 address;
    asmjit::x86::Mem operand;
};

const MovCase MOV8_CASES[] = {
    {0x02, 0x01},
    {0x00, 0xff},
    {0xab, 0xfc},
    {0x80, 0x7f}
};

const MovCase MOV16_CASES[] = {
    {0x0002, 0x0001},
    {0x0000, 0xffff},
    {0xab38, 0xfc15},
    {0x8000, 0x7fff}
};

const MovCase MOV32_CASES[] = {
    {0x00000002, 0x00000001},
    {0x00000000, 0xffffffff},
    {0x0000ab38, 0xfc150146},
    {0x80000000, 0x7fffffff}
};

const U32 EXTEND8_CASES[] = {
    0x00,
    0x01,
    0x7f,
    0x80,
    0xff,
    0xa5
};

const U32 EXTEND16_CASES[] = {
    0x0000,
    0x0001,
    0x7fff,
    0x8000,
    0xffff,
    0xa55a
};

const int VALID_SEGMENTS[] = {ES, CS, SS, DS, FS, GS};
const int LOADABLE_SEGMENTS[] = {ES, SS, DS, FS, GS};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit mov code init failed");
    }
}

void pushGeneratedCode(const asmjit::CodeHolder& code) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        pushCode8(buffer.data()[i]);
    }
}

void initRegisters(U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
}

void verifyFlagsUnchanged(const char* name) {
    if ((actualFlags(cpu, true) & FLAG_MASK) != INITIAL_FLAGS) {
        failed("%s flags changed", name);
    }
}

int physicalAddressReg(int reg, int width) {
    return width == 8 ? physicalReg8(reg) : reg;
}

bool registerOverlapsAddressReg(int reg, int width, int addressReg) {
    return physicalAddressReg(reg, width) == addressReg;
}

U32 segmentBaseForAddressReg(int baseReg) {
    return (baseReg == R_SP || baseReg == R_BP) ? cpu->seg[SS].address : cpu->seg[DS].address;
}

void initAddressRegisters(U32* regs) {
    regs[R_AX] = MEM_BASE + 0x0100;
    regs[R_CX] = MEM_BASE + 0x0200;
    regs[R_DX] = MEM_BASE + 0x0300;
    regs[R_BX] = MEM_BASE + 0x0400;
    regs[R_SP] = MEM_BASE + 0x0500;
    regs[R_BP] = MEM_BASE + 0x0600;
    regs[R_SI] = MEM_BASE + 0x0700;
    regs[R_DI] = MEM_BASE + 0x0800;
}

U32 baseMemoryOffset(int base) {
    return MEM_BASE + 0x1000 + base * 0x80;
}

void prepareMemTarget(U32 address, U32 value, int width) {
    if (width == 8) {
        memory->writeb(address - 1, 0x11);
        memory->writeb(address, value);
        memory->writeb(address + 1, 0x33);
        memory->writeb(address + 2, 0x44);
        memory->writeb(address + 3, 0x55);
    } else if (width == 16) {
        memory->writew(address - 2, 0x1111);
        memory->writew(address, value);
        memory->writew(address + 2, 0x3333);
    } else {
        memory->writew(address - 2, 0x1111);
        memory->writed(address, value);
        memory->writew(address + 4, 0x3333);
    }
}

void verifyMemTarget(U32 address, U32 expected, int width, const char* name) {
    if (width == 8) {
        if (memory->readb(address) != (expected & 0xff) ||
                memory->readb(address - 1) != 0x11 ||
                memory->readb(address + 1) != 0x33 ||
                memory->readb(address + 2) != 0x44 ||
                memory->readb(address + 3) != 0x55) {
            failed("%s memory value", name);
        }
    } else if (width == 16) {
        if (memory->readw(address) != (expected & 0xffff) ||
                memory->readw(address - 2) != 0x1111 ||
                memory->readw(address + 2) != 0x3333) {
            failed("%s memory value", name);
        }
    } else if (memory->readd(address) != expected ||
            memory->readw(address - 2) != 0x1111 ||
            memory->readw(address + 4) != 0x3333) {
        failed("%s memory value", name);
    }
}

void emitMovRegister(MovDirection direction, int dst, int src, int width) {
    if (width == 16) {
        pushCode8(0x66);
    }
    pushCode8(direction == MOV_E_FROM_G ? (width == 8 ? 0x88 : 0x89) : (width == 8 ? 0x8a : 0x8b));
    if (direction == MOV_E_FROM_G) {
        pushCode8(0xc0 | (src << 3) | dst);
    } else {
        pushCode8(0xc0 | (dst << 3) | src);
    }
}

void emitMovMemReg(MovDirection direction, const asmjit::x86::Mem& mem, asmjit::x86::Gp reg) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Error err = direction == MOV_E_FROM_G ? a.mov(mem, reg) : a.mov(reg, mem);
    if (err != asmjit::Error::kOk) {
        failed("asmjit mov memory/register failed");
    }
    pushGeneratedCode(code);
}

void emitMovExtendRegReg(ExtendMode mode, int dst, int dstWidth, int src, int srcWidth) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Error err = mode == EXTEND_ZERO ?
        a.movzx(regForWidth(dst, dstWidth), regForWidth(src, srcWidth)) :
        a.movsx(regForWidth(dst, dstWidth), regForWidth(src, srcWidth));
    if (err != asmjit::Error::kOk) {
        failed("asmjit mov extend register/register failed");
    }
    pushGeneratedCode(code);
}

void emitMovExtendRegMem(ExtendMode mode, int dst, int dstWidth, const asmjit::x86::Mem& mem) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Error err = mode == EXTEND_ZERO ?
        a.movzx(regForWidth(dst, dstWidth), mem) :
        a.movsx(regForWidth(dst, dstWidth), mem);
    if (err != asmjit::Error::kOk) {
        failed("asmjit mov extend register/memory failed");
    }
    pushGeneratedCode(code);
}

void emitMovAbsoluteReg(MovDirection direction, int reg, int width, U32 offset) {
    if (width == 16) {
        pushCode8(0x66);
    }
    pushCode8(direction == MOV_E_FROM_G ? (width == 8 ? 0x88 : 0x89) : (width == 8 ? 0x8a : 0x8b));
    pushCode8((reg << 3) | 0x05);
    pushCode32(offset);
}

void runRegisterCase(MovDirection direction, int width, int dst, int src, const MovCase& data, const char* name) {
    U32 expectedRegs[8];
    U32 actualSrc;

    newInstruction(INITIAL_FLAGS);
    emitMovRegister(direction, dst, src, width);
    initRegisters(expectedRegs);
    applyRegValue(expectedRegs, dst, width, data.dst);
    setRegValue(cpu, dst, width, data.dst);
    if (dst != src) {
        applyRegValue(expectedRegs, src, width, data.src);
        setRegValue(cpu, src, width, data.src);
    }
    actualSrc = getRegValue(cpu, src, width);
    applyRegValue(expectedRegs, dst, width, actualSrc);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    verifyFlagsUnchanged(name);
}

void runRegisterCases(MovDirection direction, int width, const MovCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int dst = 0; dst < 8; ++dst) {
            for (int src = 0; src < 8; ++src) {
                runRegisterCase(direction, width, dst, src, cases[i], name);
            }
        }
    }
}

void runPreparedMemoryCase(MovDirection direction, int reg, int width, const AddressCase& address, const MovCase& data, const char* name) {
    U32 expectedRegs[8];
    U32 actualReg = getRegValue(cpu, reg, width);

    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    prepareMemTarget(address.address, data.dst, width);
    runTestCPU();

    if (direction == MOV_E_FROM_G) {
        verifyMemTarget(address.address, actualReg, width, name);
    } else {
        applyRegValue(expectedRegs, reg, width, data.dst);
        verifyMemTarget(address.address, data.dst, width, name);
    }
    verifyRegisters(cpu, expectedRegs, name);
    verifyFlagsUnchanged(name);
}

void runBaseMemoryCases(MovDirection direction, int reg, int width, const MovCase& data, const char* name) {
    for (int base = 0; base < 8; ++base) {
        U32 regs[8];
        initAddressRegisters(regs);
        regs[base] = baseMemoryOffset(base);
        if (direction == MOV_E_FROM_G && registerOverlapsAddressReg(reg, width, base)) {
            continue;
        }
        if (direction == MOV_E_FROM_G) {
            applyRegValue(regs, reg, width, data.src);
        }

        if (base != R_BP) {
            AddressCase address = {segmentBaseForAddressReg(base) + regs[base], memPtr(reg32(base), 0, width)};
            newInstruction(INITIAL_FLAGS);
            emitMovMemReg(direction, address.operand, regForWidth(reg, width));
            writeRegs(cpu, regs);
            runPreparedMemoryCase(direction, reg, width, address, data, name);
        }

        AddressCase address = {segmentBaseForAddressReg(base) + regs[base] + 0x11, memPtr(reg32(base), 0x11, width)};
        newInstruction(INITIAL_FLAGS);
        emitMovMemReg(direction, address.operand, regForWidth(reg, width));
        writeRegs(cpu, regs);
        runPreparedMemoryCase(direction, reg, width, address, data, name);

        address = {segmentBaseForAddressReg(base) + regs[base] + 0x123, memPtr(reg32(base), 0x123, width)};
        newInstruction(INITIAL_FLAGS);
        emitMovMemReg(direction, address.operand, regForWidth(reg, width));
        writeRegs(cpu, regs);
        runPreparedMemoryCase(direction, reg, width, address, data, name);
    }
}

void runAbsoluteMemoryCase(MovDirection direction, int reg, int width, const MovCase& data, const char* name) {
    U32 regs[8];
    U32 offset = MEM_BASE + 0x3000;
    initAddressRegisters(regs);
    if (direction == MOV_E_FROM_G) {
        applyRegValue(regs, reg, width, data.src);
    }

    AddressCase address = {cpu->seg[DS].address + offset, memPtr(offset, width)};
    newInstruction(INITIAL_FLAGS);
    emitMovAbsoluteReg(direction, reg, width, offset);
    writeRegs(cpu, regs);
    runPreparedMemoryCase(direction, reg, width, address, data, name);
}

void runSibMemoryCases(MovDirection direction, int reg, int width, const MovCase& data, const char* name) {
    for (int base = 0; base < 8; ++base) {
        for (int index = 0; index < 8; ++index) {
            if (index == R_SP || (direction == MOV_E_FROM_G && (registerOverlapsAddressReg(reg, width, base) || registerOverlapsAddressReg(reg, width, index)))) {
                continue;
            }
            for (int shift = 0; shift < 4; ++shift) {
                U32 regs[8];
                U32 targetOffset = MEM_BASE + 0x7000 + base * 0x200 + index * 0x20 + shift * 4;
                initAddressRegisters(regs);
                regs[base] = MEM_BASE + 0x0100 + base * 0x40;
                regs[index] = 3;
                if (direction == MOV_E_FROM_G) {
                    applyRegValue(regs, reg, width, data.src);
                }

                S32 disp = (S32)(targetOffset - regs[base] - (regs[index] << shift));
                AddressCase address = {segmentBaseForAddressReg(base) + targetOffset, memPtr(reg32(base), reg32(index), shift, disp, width)};
                newInstruction(INITIAL_FLAGS);
                emitMovMemReg(direction, address.operand, regForWidth(reg, width));
                writeRegs(cpu, regs);
                runPreparedMemoryCase(direction, reg, width, address, data, name);
            }
        }
    }
}

void runMemoryCases(MovDirection direction, int width, const MovCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int reg = 0; reg < 8; ++reg) {
            runBaseMemoryCases(direction, reg, width, cases[i], name);
            runAbsoluteMemoryCase(direction, reg, width, cases[i], name);
            runSibMemoryCases(direction, reg, width, cases[i], name);
        }
    }
}

U32 extendedValue(U32 value, int srcWidth, int dstWidth, ExtendMode mode) {
    if (mode == EXTEND_SIGN) {
        value = (U32)signExtend(value, srcWidth);
    }
    return value & widthMask(dstWidth);
}

void runExtendRegisterCase(ExtendMode mode, int dstWidth, int srcWidth, int dst, int src, U32 value, const char* name) {
    U32 expectedRegs[8];
    U32 actualSrc;

    newInstruction(INITIAL_FLAGS);
    emitMovExtendRegReg(mode, dst, dstWidth, src, srcWidth);
    initRegisters(expectedRegs);
    setRegValue(cpu, src, srcWidth, value);
    applyRegValue(expectedRegs, src, srcWidth, value);
    actualSrc = getRegValue(cpu, src, srcWidth);
    applyRegValue(expectedRegs, dst, dstWidth, extendedValue(actualSrc, srcWidth, dstWidth, mode));

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    verifyFlagsUnchanged(name);
}

void runExtendRegisterCases(ExtendMode mode, int dstWidth, int srcWidth, const U32* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int dst = 0; dst < 8; ++dst) {
            for (int src = 0; src < 8; ++src) {
                runExtendRegisterCase(mode, dstWidth, srcWidth, dst, src, cases[i], name);
            }
        }
    }
}

void runPreparedExtendMemoryCase(ExtendMode mode, int dst, int dstWidth, int srcWidth, const AddressCase& address, U32 value, const char* name) {
    U32 expectedRegs[8];
    U32 expectedValue = extendedValue(value, srcWidth, dstWidth, mode);

    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    prepareMemTarget(address.address, value, srcWidth);
    applyRegValue(expectedRegs, dst, dstWidth, expectedValue);

    runTestCPU();
    verifyMemTarget(address.address, value, srcWidth, name);
    verifyRegisters(cpu, expectedRegs, name);
    verifyFlagsUnchanged(name);
}

void runExtendBaseMemoryCases(ExtendMode mode, int dst, int dstWidth, int srcWidth, U32 value, const char* name) {
    for (int base = 0; base < 8; ++base) {
        U32 regs[8];
        initAddressRegisters(regs);
        regs[base] = baseMemoryOffset(base);

        if (base != R_BP) {
            AddressCase address = {segmentBaseForAddressReg(base) + regs[base], memPtr(reg32(base), 0, srcWidth)};
            newInstruction(INITIAL_FLAGS);
            emitMovExtendRegMem(mode, dst, dstWidth, address.operand);
            writeRegs(cpu, regs);
            runPreparedExtendMemoryCase(mode, dst, dstWidth, srcWidth, address, value, name);
        }

        AddressCase address = {segmentBaseForAddressReg(base) + regs[base] + 0x11, memPtr(reg32(base), 0x11, srcWidth)};
        newInstruction(INITIAL_FLAGS);
        emitMovExtendRegMem(mode, dst, dstWidth, address.operand);
        writeRegs(cpu, regs);
        runPreparedExtendMemoryCase(mode, dst, dstWidth, srcWidth, address, value, name);

        address = {segmentBaseForAddressReg(base) + regs[base] + 0x123, memPtr(reg32(base), 0x123, srcWidth)};
        newInstruction(INITIAL_FLAGS);
        emitMovExtendRegMem(mode, dst, dstWidth, address.operand);
        writeRegs(cpu, regs);
        runPreparedExtendMemoryCase(mode, dst, dstWidth, srcWidth, address, value, name);
    }
}

void runExtendAbsoluteMemoryCase(ExtendMode mode, int dst, int dstWidth, int srcWidth, U32 value, const char* name) {
    U32 regs[8];
    U32 offset = MEM_BASE + 0x9000 + dst * 0x20 + srcWidth;
    AddressCase address = {cpu->seg[DS].address + offset, memPtr(offset, srcWidth)};

    initAddressRegisters(regs);
    newInstruction(INITIAL_FLAGS);
    emitMovExtendRegMem(mode, dst, dstWidth, address.operand);
    writeRegs(cpu, regs);
    runPreparedExtendMemoryCase(mode, dst, dstWidth, srcWidth, address, value, name);
}

void runExtendSibMemoryCases(ExtendMode mode, int dst, int dstWidth, int srcWidth, U32 value, const char* name) {
    for (int base = 0; base < 8; ++base) {
        for (int index = 0; index < 8; ++index) {
            if (index == R_SP) {
                continue;
            }
            for (int shift = 0; shift < 4; ++shift) {
                U32 regs[8];
                U32 targetOffset = MEM_BASE + 0xa000 + base * 0x200 + index * 0x20 + shift * 4;
                initAddressRegisters(regs);
                regs[base] = MEM_BASE + 0x0100 + base * 0x40;
                regs[index] = 3;

                S32 disp = (S32)(targetOffset - regs[base] - (regs[index] << shift));
                AddressCase address = {segmentBaseForAddressReg(base) + targetOffset, memPtr(reg32(base), reg32(index), shift, disp, srcWidth)};
                newInstruction(INITIAL_FLAGS);
                emitMovExtendRegMem(mode, dst, dstWidth, address.operand);
                writeRegs(cpu, regs);
                runPreparedExtendMemoryCase(mode, dst, dstWidth, srcWidth, address, value, name);
            }
        }
    }
}

void runExtendMemoryCases(ExtendMode mode, int dstWidth, int srcWidth, const U32* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int dst = 0; dst < 8; ++dst) {
            runExtendBaseMemoryCases(mode, dst, dstWidth, srcWidth, cases[i], name);
            runExtendAbsoluteMemoryCase(mode, dst, dstWidth, srcWidth, cases[i], name);
            runExtendSibMemoryCases(mode, dst, dstWidth, srcWidth, cases[i], name);
        }
    }
}

void runExtendCases(ExtendMode mode, int dstWidth, int srcWidth, const U32* cases, size_t count, const char* name) {
    runExtendRegisterCases(mode, dstWidth, srcWidth, cases, count, name);
    runExtendMemoryCases(mode, dstWidth, srcWidth, cases, count, name);
}

void emitMovSregToRegMem(int seg, int rm, bool regDestination, bool bigDestination) {
    if (!bigDestination) {
        pushCode8(0x66);
    }
    pushCode8(0x8c);
    pushCode8((regDestination ? 0xc0 : 0x00) | (seg << 3) | rm);
}

void emitMovRegMemToSreg(int seg, int rm, bool regSource, bool bigSource) {
    if (!bigSource) {
        pushCode8(0x66);
    }
    pushCode8(0x8e);
    pushCode8((regSource ? 0xc0 : 0x00) | (seg << 3) | rm);
}

U32 selectorForSegment(int seg) {
    return seg == SS ? TEST_STACK_SEG : TEST_HEAP_SEG;
}

U32 baseForSelector(U32 selector) {
    return selector == TEST_STACK_SEG ? cpu->seg[SS].address : TEST_HEAP_ADDRESS;
}

void verifySegmentsUnchanged(const U32* values, const U32* addresses, const char* name) {
    for (int i = 0; i < 6; ++i) {
        if (cpu->seg[i].value != values[i] || cpu->seg[i].address != addresses[i]) {
            failed("%s segment changed", name);
        }
    }
}

void captureSegments(U32* values, U32* addresses) {
    for (int i = 0; i < 6; ++i) {
        values[i] = cpu->seg[i].value;
        addresses[i] = cpu->seg[i].address;
    }
}

void runSegmentToRegCases(bool bigDestination, const char* name) {
    for (int segIndex = 0; segIndex < 6; ++segIndex) {
        int seg = VALID_SEGMENTS[segIndex];
        for (int reg = 0; reg < 8; ++reg) {
            U32 expectedRegs[8];
            U32 segValues[6];
            U32 segAddresses[6];
            newInstruction(INITIAL_FLAGS);
            emitMovSregToRegMem(seg, reg, true, bigDestination);
            initRegisters(expectedRegs);
            captureSegments(segValues, segAddresses);
            if (bigDestination) {
                expectedRegs[reg] = cpu->seg[seg].value;
            } else {
                applyRegValue(expectedRegs, reg, 16, cpu->seg[seg].value);
            }
            runTestCPU();
            verifyRegisters(cpu, expectedRegs, name);
            verifySegmentsUnchanged(segValues, segAddresses, name);
            verifyFlagsUnchanged(name);
        }
    }
}

void runSegmentToMemoryCases(bool bigDestination, const char* name) {
    for (int segIndex = 0; segIndex < 6; ++segIndex) {
        int seg = VALID_SEGMENTS[segIndex];
        U32 offset = MEM_BASE + 0x4000 + seg * 0x20;
        U32 address = cpu->seg[DS].address + offset;
        U32 regs[8];
        U32 segValues[6];
        U32 segAddresses[6];

        newInstruction(INITIAL_FLAGS);
        emitMovSregToRegMem(seg, 5, false, bigDestination);
        pushCode32(offset);
        initAddressRegisters(regs);
        writeRegs(cpu, regs);
        captureSegments(segValues, segAddresses);
        prepareMemTarget(address, 0, 16);
        runTestCPU();
        verifyMemTarget(address, cpu->seg[seg].value, 16, name);
        verifySegmentsUnchanged(segValues, segAddresses, name);
        verifyFlagsUnchanged(name);
    }
}

void runMemoryOrRegToSegmentCases(bool bigSource, const char* name) {
    for (int segIndex = 0; segIndex < 5; ++segIndex) {
        int seg = LOADABLE_SEGMENTS[segIndex];
        U32 selector = selectorForSegment(seg);

        for (int reg = 0; reg < 8; ++reg) {
            U32 expectedRegs[8];
            newInstruction(INITIAL_FLAGS);
            emitMovRegMemToSreg(seg, reg, true, bigSource);
            initRegisters(expectedRegs);
            setRegValue(cpu, reg, 16, selector);
            applyRegValue(expectedRegs, reg, 16, selector);
            runTestCPU();
            verifyRegisters(cpu, expectedRegs, name);
            if (cpu->seg[seg].value != selector || cpu->seg[seg].address != baseForSelector(selector)) {
                failed("%s segment value", name);
            }
            verifyFlagsUnchanged(name);
        }

        U32 offset = MEM_BASE + 0x5000 + seg * 0x20;
        U32 address = cpu->seg[DS].address + offset;
        U32 regs[8];
        newInstruction(INITIAL_FLAGS);
        emitMovRegMemToSreg(seg, 5, false, bigSource);
        pushCode32(offset);
        initAddressRegisters(regs);
        writeRegs(cpu, regs);
        memory->writew(address - 2, 0x1111);
        memory->writew(address, selector);
        memory->writew(address + 2, 0x3333);
        runTestCPU();
        if (cpu->seg[seg].value != selector || cpu->seg[seg].address != baseForSelector(selector) ||
                memory->readw(address - 2) != 0x1111 || memory->readw(address + 2) != 0x3333) {
            failed("%s memory segment value", name);
        }
        verifyFlagsUnchanged(name);
    }
}

void emitAbsoluteAccumulator(bool load, int width, bool address32) {
    if (width == 16 && address32) {
        pushCode8(0x66);
    }
    pushCode8(load ? (width == 8 ? 0xa0 : 0xa1) : (width == 8 ? 0xa2 : 0xa3));
}

void runAbsoluteAccumulatorCase(bool load, int width, bool address32, const MovCase& data, const char* name) {
    U32 expectedRegs[8];
    U32 offset = address32 ? MEM_BASE + 0x1234 : MEM_BASE + 0x0234;
    U32 address = cpu->seg[DS].address + (address32 ? offset : (offset & 0xffff));

    newInstruction(INITIAL_FLAGS);
    cpu->big = address32;
    emitAbsoluteAccumulator(load, width, address32);
    if (address32) {
        pushCode32(offset);
    } else {
        pushCode16(offset);
    }
    initRegisters(expectedRegs);
    setRegValue(cpu, R_AX, width, data.src);
    applyRegValue(expectedRegs, R_AX, width, data.src);
    prepareMemTarget(address, data.dst, width);
    if (load) {
        applyRegValue(expectedRegs, R_AX, width, data.dst);
    }
    runTestCPU();
    if (!load) {
        verifyMemTarget(address, data.src, width, name);
    } else {
        verifyMemTarget(address, data.dst, width, name);
    }
    verifyRegisters(cpu, expectedRegs, name);
    verifyFlagsUnchanged(name);
}

void runAbsoluteAccumulatorCases(bool load, int width, bool address32, const char* name) {
    const MovCase* cases = width == 8 ? MOV8_CASES : (width == 16 ? MOV16_CASES : MOV32_CASES);
    size_t count = width == 8 ? caseCount(MOV8_CASES) : (width == 16 ? caseCount(MOV16_CASES) : caseCount(MOV32_CASES));
    for (size_t i = 0; i < count; ++i) {
        runAbsoluteAccumulatorCase(load, width, address32, cases[i], name);
    }
}

void emitMovRegImmediate(int reg, int width, U32 value) {
    if (width == 8) {
        pushCode8(0xb0 + reg);
        pushCode8(value);
    } else {
        if (width == 16) {
            pushCode8(0x66);
        }
        pushCode8(0xb8 + reg);
        if (width == 16) {
            pushCode16(value);
        } else {
            pushCode32(value);
        }
    }
}

void runRegImmediateCases(int width, const MovCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int reg = 0; reg < 8; ++reg) {
            U32 expectedRegs[8];
            newInstruction(INITIAL_FLAGS);
            emitMovRegImmediate(reg, width, cases[i].src);
            initRegisters(expectedRegs);
            applyRegValue(expectedRegs, reg, width, cases[i].src);
            runTestCPU();
            verifyRegisters(cpu, expectedRegs, name);
            verifyFlagsUnchanged(name);
        }
    }
}

void emitMovMemImmediate(const asmjit::x86::Mem& mem, int width, U32 value) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Error err = a.mov(mem, asmjit::Imm(value & widthMask(width)));
    if (err != asmjit::Error::kOk) {
        failed("asmjit mov memory immediate failed");
    }
    pushGeneratedCode(code);
}

void emitMovRegImmediateC6C7(int reg, int width, U32 value) {
    if (width == 16) {
        pushCode8(0x66);
    }
    pushCode8(width == 8 ? 0xc6 : 0xc7);
    pushCode8(0xc0 | reg);
    if (width == 8) {
        pushCode8(value);
    } else if (width == 16) {
        pushCode16(value);
    } else {
        pushCode32(value);
    }
}

void runRegImmediateC6C7Cases(int width, const MovCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int reg = 0; reg < 8; ++reg) {
            U32 expectedRegs[8];
            newInstruction(INITIAL_FLAGS);
            emitMovRegImmediateC6C7(reg, width, cases[i].src);
            initRegisters(expectedRegs);
            applyRegValue(expectedRegs, reg, width, cases[i].src);
            runTestCPU();
            verifyRegisters(cpu, expectedRegs, name);
            verifyFlagsUnchanged(name);
        }
    }
}

void runMemoryImmediateCase(const AddressCase& address, int width, U32 value, const char* name) {
    U32 expectedRegs[8];
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    prepareMemTarget(address.address, 0, width);
    runTestCPU();
    verifyMemTarget(address.address, value, width, name);
    verifyRegisters(cpu, expectedRegs, name);
    verifyFlagsUnchanged(name);
}

void runMemoryImmediateCases(int width, const MovCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        U32 offset = MEM_BASE + 0x6000 + (U32)i * 0x20;
        AddressCase address = {cpu->seg[DS].address + offset, memPtr(offset, width)};
        newInstruction(INITIAL_FLAGS);
        emitMovMemImmediate(address.operand, width, cases[i].src);
        runMemoryImmediateCase(address, width, cases[i].src, name);

        U32 regs[8];
        initAddressRegisters(regs);
        regs[R_BX] = MEM_BASE + 0x6800 + (U32)i * 0x20;
        address = {cpu->seg[DS].address + regs[R_BX] + 0x12, memPtr(reg32(R_BX), 0x12, width)};
        newInstruction(INITIAL_FLAGS);
        emitMovMemImmediate(address.operand, width, cases[i].src);
        writeRegs(cpu, regs);
        runMemoryImmediateCase(address, width, cases[i].src, name);
    }
}

} // namespace

void testMovR8R8_0x088() { runRegisterCases(MOV_E_FROM_G, 8, MOV8_CASES, caseCount(MOV8_CASES), "mov r8,r8 88"); }
void testMovE8R8_0x088() { runMemoryCases(MOV_E_FROM_G, 8, MOV8_CASES, caseCount(MOV8_CASES), "mov m8,r8 88"); }
void testMovR8R8_0x288() { runRegisterCases(MOV_E_FROM_G, 8, MOV8_CASES, caseCount(MOV8_CASES), "mov r8,r8 288"); }
void testMovE8R8_0x288() { runMemoryCases(MOV_E_FROM_G, 8, MOV8_CASES, caseCount(MOV8_CASES), "mov m8,r8 288"); }
void testMovR16R16_0x089() { runRegisterCases(MOV_E_FROM_G, 16, MOV16_CASES, caseCount(MOV16_CASES), "mov r16,r16 89"); }
void testMovE16R16_0x089() { runMemoryCases(MOV_E_FROM_G, 16, MOV16_CASES, caseCount(MOV16_CASES), "mov m16,r16 89"); }
void testMovR32R32_0x289() { runRegisterCases(MOV_E_FROM_G, 32, MOV32_CASES, caseCount(MOV32_CASES), "mov r32,r32 289"); }
void testMovE32R32_0x289() { runMemoryCases(MOV_E_FROM_G, 32, MOV32_CASES, caseCount(MOV32_CASES), "mov m32,r32 289"); }
void testMovR8R8_0x08a() { runRegisterCases(MOV_G_FROM_E, 8, MOV8_CASES, caseCount(MOV8_CASES), "mov r8,r8 8a"); }
void testMovR8E8_0x08a() { runMemoryCases(MOV_G_FROM_E, 8, MOV8_CASES, caseCount(MOV8_CASES), "mov r8,m8 8a"); }
void testMovR8R8_0x28a() { runRegisterCases(MOV_G_FROM_E, 8, MOV8_CASES, caseCount(MOV8_CASES), "mov r8,r8 28a"); }
void testMovR8E8_0x28a() { runMemoryCases(MOV_G_FROM_E, 8, MOV8_CASES, caseCount(MOV8_CASES), "mov r8,m8 28a"); }
void testMovR16R16_0x08b() { runRegisterCases(MOV_G_FROM_E, 16, MOV16_CASES, caseCount(MOV16_CASES), "mov r16,r16 8b"); }
void testMovR16E16_0x08b() { runMemoryCases(MOV_G_FROM_E, 16, MOV16_CASES, caseCount(MOV16_CASES), "mov r16,m16 8b"); }
void testMovR32R32_0x28b() { runRegisterCases(MOV_G_FROM_E, 32, MOV32_CASES, caseCount(MOV32_CASES), "mov r32,r32 28b"); }
void testMovR32E32_0x28b() { runMemoryCases(MOV_G_FROM_E, 32, MOV32_CASES, caseCount(MOV32_CASES), "mov r32,m32 28b"); }
void testMovE16Sreg_0x08c() { runSegmentToRegCases(false, "mov r16,sreg 8c"); runSegmentToMemoryCases(false, "mov m16,sreg 8c"); }
void testMovE32Sreg_0x28c() { runSegmentToRegCases(true, "mov r32,sreg 28c"); runSegmentToMemoryCases(true, "mov m32,sreg 28c"); }
void testMovSregE16_0x08e() { runMemoryOrRegToSegmentCases(false, "mov sreg,r/m16 8e"); }
void testMovSregE32_0x28e() { runMemoryOrRegToSegmentCases(true, "mov sreg,r/m32 28e"); }
void testMovAlOb_0x0a0() { runAbsoluteAccumulatorCases(true, 8, false, "mov al,ob a0"); }
void testMovAlOb_0x2a0() { runAbsoluteAccumulatorCases(true, 8, true, "mov al,ob 2a0"); }
void testMovAxOw_0x0a1() { runAbsoluteAccumulatorCases(true, 16, false, "mov ax,ow a1"); }
void testMovEaxOd_0x2a1() { runAbsoluteAccumulatorCases(true, 32, true, "mov eax,od 2a1"); }
void testMovObAl_0x0a2() { runAbsoluteAccumulatorCases(false, 8, false, "mov ob,al a2"); }
void testMovObAl_0x2a2() { runAbsoluteAccumulatorCases(false, 8, true, "mov ob,al 2a2"); }
void testMovOwAx_0x0a3() { runAbsoluteAccumulatorCases(false, 16, false, "mov ow,ax a3"); }
void testMovOdEax_0x2a3() { runAbsoluteAccumulatorCases(false, 32, true, "mov od,eax 2a3"); }
void testMovR8Ib_0x0b0() { runRegImmediateCases(8, MOV8_CASES, caseCount(MOV8_CASES), "mov r8,ib b0"); }
void testMovR8Ib_0x2b0() { runRegImmediateCases(8, MOV8_CASES, caseCount(MOV8_CASES), "mov r8,ib 2b0"); }
void testMovR16Iw_0x0b8() { runRegImmediateCases(16, MOV16_CASES, caseCount(MOV16_CASES), "mov r16,iw b8"); }
void testMovR32Id_0x2b8() { runRegImmediateCases(32, MOV32_CASES, caseCount(MOV32_CASES), "mov r32,id 2b8"); }
void testMovE8Ib_0x0c6() { runRegImmediateC6C7Cases(8, MOV8_CASES, caseCount(MOV8_CASES), "mov r/m8,ib c6"); runMemoryImmediateCases(8, MOV8_CASES, caseCount(MOV8_CASES), "mov m8,ib c6"); }
void testMovE8Ib_0x2c6() { runRegImmediateC6C7Cases(8, MOV8_CASES, caseCount(MOV8_CASES), "mov r/m8,ib 2c6"); runMemoryImmediateCases(8, MOV8_CASES, caseCount(MOV8_CASES), "mov m8,ib 2c6"); }
void testMovE16Iw_0x0c7() { runRegImmediateC6C7Cases(16, MOV16_CASES, caseCount(MOV16_CASES), "mov r/m16,iw c7"); runMemoryImmediateCases(16, MOV16_CASES, caseCount(MOV16_CASES), "mov m16,iw c7"); }
void testMovE32Id_0x2c7() { runRegImmediateC6C7Cases(32, MOV32_CASES, caseCount(MOV32_CASES), "mov r/m32,id 2c7"); runMemoryImmediateCases(32, MOV32_CASES, caseCount(MOV32_CASES), "mov m32,id 2c7"); }
void testMovzxR16E8_0x1b6() { runExtendCases(EXTEND_ZERO, 16, 8, EXTEND8_CASES, caseCount(EXTEND8_CASES), "movzx r16,r/m8 1b6"); }
void testMovzxR32E8_0x3b6() { runExtendCases(EXTEND_ZERO, 32, 8, EXTEND8_CASES, caseCount(EXTEND8_CASES), "movzx r32,r/m8 3b6"); }
void testMovzxR32E16_0x3b7() { runExtendCases(EXTEND_ZERO, 32, 16, EXTEND16_CASES, caseCount(EXTEND16_CASES), "movzx r32,r/m16 3b7"); }
void testMovsxR16E8_0x1be() { runExtendCases(EXTEND_SIGN, 16, 8, EXTEND8_CASES, caseCount(EXTEND8_CASES), "movsx r16,r/m8 1be"); }
void testMovsxR32E8_0x3be() { runExtendCases(EXTEND_SIGN, 32, 8, EXTEND8_CASES, caseCount(EXTEND8_CASES), "movsx r32,r/m8 3be"); }
void testMovsxR32E16_0x3bf() { runExtendCases(EXTEND_SIGN, 32, 16, EXTEND16_CASES, caseCount(EXTEND16_CASES), "movsx r32,r/m16 3bf"); }

#endif
