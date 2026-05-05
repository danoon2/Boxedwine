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

#include "testAdd.h"
#include "testX86Binary.h"
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

constexpr U32 REG_GUARD = 0xA55A0000;
constexpr U32 MEM_BASE = 0x10000;
constexpr U32 XADD_FLAG_MASK = CF | PF | AF | ZF | SF | OF;

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

enum XaddFlagMode {
    XADD_FLAGS_CHECKED,
    XADD_FLAGS_OVERWRITTEN
};

const TestBinaryCase ADD8_CASES[] = {
    {0x01, 0x01, 0},
    {0x7f, 0x01, 0},
    {0x80, 0x80, 0},
    {0xff, 0x01, 0},
    {0x0f, 0x01, 0},
    {0x12, 0xee, CF | OF | SF | ZF | AF | PF}
};

const TestBinaryCase ADD16_CASES[] = {
    {0x0001, 0x0001, 0},
    {0x7fff, 0x0001, 0},
    {0x8000, 0x8000, 0},
    {0xffff, 0x0001, 0},
    {0x00ff, 0x0001, 0},
    {0x0fff, 0x0001, 0},
    {0x1234, 0xedcc, CF | OF | SF | ZF | AF | PF}
};

const TestBinaryCase ADD32_CASES[] = {
    {0x00000001, 0x00000001, 0},
    {0x7fffffff, 0x00000001, 0},
    {0x80000000, 0x80000000, 0},
    {0xffffffff, 0x00000001, 0},
    {0x000000ff, 0x00000001, 0},
    {0x00000fff, 0x00000001, 0},
    {0x12345678, 0xedcba988, CF | OF | SF | ZF | AF | PF}
};

const TestBinaryCase XADD8_CASES[] = {
    {0x01, 0x0a, 0},
    {0x7f, 0x01, 0},
    {0x80, 0x80, 0},
    {0xff, 0x01, 0},
    {0x0f, 0x01, 0},
    {0x12, 0xee, CF | OF | SF | ZF | AF | PF}
};

const TestBinaryCase XADD16_CASES[] = {
    {0x0001, 0x000a, 0},
    {0x7fff, 0x0001, 0},
    {0x8000, 0x8000, 0},
    {0xffff, 0x0001, 0},
    {0x00ff, 0x0001, 0},
    {0x0fff, 0x0001, 0},
    {0x1234, 0xedcc, CF | OF | SF | ZF | AF | PF}
};

const TestBinaryCase XADD32_CASES[] = {
    {0x00000001, 0x0000000a, 0},
    {0x7fffffff, 0x00000001, 0},
    {0x80000000, 0x80000000, 0},
    {0xffffffff, 0x00000001, 0},
    {0x000000ff, 0x00000001, 0},
    {0x00000fff, 0x00000001, 0},
    {0x12345678, 0xedcba988, CF | OF | SF | ZF | AF | PF}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit xadd code init failed");
    }
}

void pushGeneratedCode(const asmjit::CodeHolder& code) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        pushCode8(buffer.data()[i]);
    }
}

void beginGeneratedInstruction(U32 flags) {
    newInstruction(flags);
    cpu->big = true;
}

void overwriteFlagsIfNeeded(XaddFlagMode flagMode) {
    if (flagMode == XADD_FLAGS_OVERWRITTEN) {
        pushCode8(0x39);
        pushCode8(0xc0); // cmp eax, eax
    }
}

void initRegisters(U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
}

U32 xaddResult(U32 dst, U32 src, int width) {
    return (dst + src) & widthMask(width);
}

U32 xaddFlags(U32 dst, U32 src, int width) {
    U32 mask = widthMask(width);
    U32 sign = signBit(width);
    U32 result = xaddResult(dst, src, width);
    U32 flags = 0;

    dst &= mask;
    src &= mask;
    if (width == 32) {
        if ((U64)dst + (U64)src > 0xffffffffULL) {
            flags |= CF;
        }
    } else if (dst + src > mask) {
        flags |= CF;
    }
    if (((dst ^ result) & (src ^ result) & sign) != 0) {
        flags |= OF;
    }
    if (((dst ^ src ^ result) & 0x10) != 0) {
        flags |= AF;
    }
    if (result == 0) {
        flags |= ZF;
    }
    if ((result & sign) != 0) {
        flags |= SF;
    }

    U8 low = (U8)result;
    low ^= low >> 4;
    low &= 0x0f;
    if (((0x6996 >> low) & 1) == 0) {
        flags |= PF;
    }
    return flags;
}

void verifyXaddFlags(U32 dst, U32 src, int width, const char* name) {
    U32 actual = 0;
    if (cpu->getCF()) actual |= CF;
    if (cpu->getPF()) actual |= PF;
    if (cpu->getAF()) actual |= AF;
    if (cpu->getZF()) actual |= ZF;
    if (cpu->getSF()) actual |= SF;
    if (cpu->getOF()) actual |= OF;
    if ((actual & XADD_FLAG_MASK) != (xaddFlags(dst, src, width) & XADD_FLAG_MASK)) {
        failed("%s flags", name);
    }
}

void emitXadd(asmjit::x86::Gp dst, asmjit::x86::Gp src) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.xadd(dst, src) != asmjit::Error::kOk) {
        failed("asmjit xadd reg, reg failed");
    }
    pushGeneratedCode(code);
}

void emitXadd(const asmjit::x86::Mem& dst, asmjit::x86::Gp src, bool lockPrefix) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (lockPrefix) {
        a.lock();
    }
    if (a.xadd(dst, src) != asmjit::Error::kOk) {
        failed("asmjit xadd mem, reg failed");
    }
    pushGeneratedCode(code);
}

U32 segmentBaseForAddressReg(int baseReg) {
    return (baseReg == R_SP || baseReg == R_BP) ? cpu->seg[SS].address : cpu->seg[DS].address;
}

bool registerOverlapsAddressReg(int reg, int width, int addressReg) {
    return width == 8 ? physicalReg8(reg) == addressReg : reg == addressReg;
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

void runXaddRegisterCase(int width, int dst, int src, const TestBinaryCase& data, XaddFlagMode flagMode, const char* name) {
    U32 expectedRegs[8];

    beginGeneratedInstruction(data.initialFlags);
    emitXadd(regForWidth(dst, width), regForWidth(src, width));
    overwriteFlagsIfNeeded(flagMode);
    initRegisters(expectedRegs);
    applyRegValue(expectedRegs, dst, width, data.dst);
    setRegValue(cpu, dst, width, data.dst);
    if (dst != src) {
        applyRegValue(expectedRegs, src, width, data.src);
        setRegValue(cpu, src, width, data.src);
    }

    U32 actualDst = getRegValue(cpu, dst, width);
    U32 actualSrc = getRegValue(cpu, src, width);
    applyRegValue(expectedRegs, src, width, actualDst);
    applyRegValue(expectedRegs, dst, width, xaddResult(actualDst, actualSrc, width));

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (flagMode == XADD_FLAGS_CHECKED) {
        verifyXaddFlags(actualDst, actualSrc, width, name);
    }
}

void runXaddRegisterCases(int width, const TestBinaryCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int flagMode = XADD_FLAGS_CHECKED; flagMode <= XADD_FLAGS_OVERWRITTEN; ++flagMode) {
            for (int dst = 0; dst < 8; ++dst) {
                for (int src = 0; src < 8; ++src) {
                    runXaddRegisterCase(width, dst, src, cases[i], (XaddFlagMode)flagMode, name);
                }
            }
        }
    }
}

void runXaddByteRegisterAliasCases(const char* name) {
    static const TestBinaryCase cases[] = {
        {0x56, 0x78, 0},
        {0x7f, 0x01, 0},
        {0x80, 0x80, 0},
        {0xff, 0x01, 0}
    };
    static const int regPairs[][2] = {
        {4, 0}, // ah, al
        {0, 4}, // al, ah
        {5, 1}, // ch, cl
        {1, 5} // cl, ch
    };

    for (size_t i = 0; i < caseCount(cases); ++i) {
        for (size_t pair = 0; pair < caseCount(regPairs); ++pair) {
            runXaddRegisterCase(8, regPairs[pair][0], regPairs[pair][1], cases[i], XADD_FLAGS_CHECKED, name);
        }
    }
}

void runPreparedXaddMemoryCase(int srcReg, int width, U32 linearAddress, const TestBinaryCase& data, XaddFlagMode flagMode, const char* name) {
    U32 expectedRegs[8];
    U32 actualSrc = getRegValue(cpu, srcReg, width);

    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    prepareMemTarget(linearAddress, data.dst, width);
    applyRegValue(expectedRegs, srcReg, width, data.dst);

    runTestCPU();
    verifyMemTarget(linearAddress, xaddResult(data.dst, actualSrc, width), width, name);
    verifyRegisters(cpu, expectedRegs, name);
    if (flagMode == XADD_FLAGS_CHECKED) {
        verifyXaddFlags(data.dst, actualSrc, width, name);
    }
}

void runXaddBaseMemoryCases(int srcReg, int width, const TestBinaryCase& data, bool lockPrefix, XaddFlagMode flagMode, const char* name) {
    for (int base = 0; base < 8; ++base) {
        U32 regs[8];
        initAddressRegisters(regs);
        regs[base] = baseMemoryOffset(base);
        if (!registerOverlapsAddressReg(srcReg, width, base)) {
            applyRegValue(regs, srcReg, width, data.src);
        }

        if (base != R_BP) {
            beginGeneratedInstruction(data.initialFlags);
            emitXadd(memPtr(reg32(base), 0, width), regForWidth(srcReg, width), lockPrefix);
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedXaddMemoryCase(srcReg, width, segmentBaseForAddressReg(base) + regs[base], data, flagMode, name);
        }

        beginGeneratedInstruction(data.initialFlags);
        emitXadd(memPtr(reg32(base), 0x11, width), regForWidth(srcReg, width), lockPrefix);
        overwriteFlagsIfNeeded(flagMode);
        writeRegs(cpu, regs);
        runPreparedXaddMemoryCase(srcReg, width, segmentBaseForAddressReg(base) + regs[base] + 0x11, data, flagMode, name);

        beginGeneratedInstruction(data.initialFlags);
        emitXadd(memPtr(reg32(base), 0x123, width), regForWidth(srcReg, width), lockPrefix);
        overwriteFlagsIfNeeded(flagMode);
        writeRegs(cpu, regs);
        runPreparedXaddMemoryCase(srcReg, width, segmentBaseForAddressReg(base) + regs[base] + 0x123, data, flagMode, name);
    }
}

void runXaddAbsoluteMemoryCases(int srcReg, int width, const TestBinaryCase& data, bool lockPrefix, XaddFlagMode flagMode, const char* name) {
    U32 regs[8];
    U32 offset = MEM_BASE + 0x3000;
    initAddressRegisters(regs);
    applyRegValue(regs, srcReg, width, data.src);

    beginGeneratedInstruction(data.initialFlags);
    emitXadd(memPtr(offset, width), regForWidth(srcReg, width), lockPrefix);
    overwriteFlagsIfNeeded(flagMode);
    writeRegs(cpu, regs);
    runPreparedXaddMemoryCase(srcReg, width, cpu->seg[DS].address + offset, data, flagMode, name);
}

void runXaddSibMemoryCases(int srcReg, int width, const TestBinaryCase& data, bool lockPrefix, XaddFlagMode flagMode, const char* name) {
    for (int base = 0; base < 8; ++base) {
        for (int index = 0; index < 8; ++index) {
            if (index == R_SP) {
                continue;
            }
            for (int shift = 0; shift < 4; ++shift) {
                U32 regs[8];
                U32 targetOffset = MEM_BASE + 0x7000 + base * 0x200 + index * 0x20 + shift * 4;
                initAddressRegisters(regs);
                regs[base] = MEM_BASE + 0x0100 + base * 0x40;
                regs[index] = 3;
                if (!registerOverlapsAddressReg(srcReg, width, base) && !registerOverlapsAddressReg(srcReg, width, index)) {
                    applyRegValue(regs, srcReg, width, data.src);
                }

                S32 disp = (S32)(targetOffset - regs[base] - (regs[index] << shift));
                beginGeneratedInstruction(data.initialFlags);
                emitXadd(memPtr(reg32(base), reg32(index), shift, disp, width), regForWidth(srcReg, width), lockPrefix);
                overwriteFlagsIfNeeded(flagMode);
                writeRegs(cpu, regs);
                runPreparedXaddMemoryCase(srcReg, width, segmentBaseForAddressReg(base) + targetOffset, data, flagMode, name);
            }
        }
    }
}

void runXaddMemoryCases(int width, const TestBinaryCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int flagMode = XADD_FLAGS_CHECKED; flagMode <= XADD_FLAGS_OVERWRITTEN; ++flagMode) {
            for (int src = 0; src < 8; ++src) {
                for (int lockPrefix = 0; lockPrefix < 2; ++lockPrefix) {
                    runXaddBaseMemoryCases(src, width, cases[i], lockPrefix != 0, (XaddFlagMode)flagMode, name);
                    runXaddAbsoluteMemoryCases(src, width, cases[i], lockPrefix != 0, (XaddFlagMode)flagMode, name);
                    runXaddSibMemoryCases(src, width, cases[i], lockPrefix != 0, (XaddFlagMode)flagMode, name);
                }
            }
        }
    }
}

void runXaddCases(int width, const TestBinaryCase* cases, size_t count, const char* name) {
    if (width == 8) {
        runXaddByteRegisterAliasCases(name);
    }
    runXaddRegisterCases(width, cases, count, name);
    runXaddMemoryCases(width, cases, count, name);
}

} // namespace

void testAddR8R8_0x000() {
    testRunBinaryRegister(TEST_BINARY_ADD, 8, ADD8_CASES, caseCount(ADD8_CASES), "add r8,r8", false);
}

void testAddE8R8_0x000() {
    testRunBinaryMemoryDestination(TEST_BINARY_ADD, 8, ADD8_CASES, caseCount(ADD8_CASES), "add m8,r8");
}

void testAddR16R16_0x001() {
    testRunBinaryRegister(TEST_BINARY_ADD, 16, ADD16_CASES, caseCount(ADD16_CASES), "add r16,r16", false);
}

void testAddE16R16_0x001() {
    testRunBinaryMemoryDestination(TEST_BINARY_ADD, 16, ADD16_CASES, caseCount(ADD16_CASES), "add m16,r16");
}

void testAddR32R32_0x001() {
    testRunBinaryRegister(TEST_BINARY_ADD, 32, ADD32_CASES, caseCount(ADD32_CASES), "add r32,r32", false);
}

void testAddE32R32_0x001() {
    testRunBinaryMemoryDestination(TEST_BINARY_ADD, 32, ADD32_CASES, caseCount(ADD32_CASES), "add m32,r32");
}

void testAddR8R8_0x002() {
    testRunBinaryRegister(TEST_BINARY_ADD, 8, ADD8_CASES, caseCount(ADD8_CASES), "add r8,r8 02", true);
}

void testAddR8E8_0x002() {
    testRunBinaryMemorySource(TEST_BINARY_ADD, 8, ADD8_CASES, caseCount(ADD8_CASES), "add r8,m8");
}

void testAddR16R16_0x003() {
    testRunBinaryRegister(TEST_BINARY_ADD, 16, ADD16_CASES, caseCount(ADD16_CASES), "add r16,r16 03", true);
}

void testAddR16E16_0x003() {
    testRunBinaryMemorySource(TEST_BINARY_ADD, 16, ADD16_CASES, caseCount(ADD16_CASES), "add r16,m16");
}

void testAddR32R32_0x003() {
    testRunBinaryRegister(TEST_BINARY_ADD, 32, ADD32_CASES, caseCount(ADD32_CASES), "add r32,r32 03", true);
}

void testAddR32E32_0x003() {
    testRunBinaryMemorySource(TEST_BINARY_ADD, 32, ADD32_CASES, caseCount(ADD32_CASES), "add r32,m32");
}

void testAddAlIb_0x004() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_ADD, 8, ADD8_CASES, caseCount(ADD8_CASES), "add al,ib");
}

void testAddAxIw_0x005() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_ADD, 16, ADD16_CASES, caseCount(ADD16_CASES), "add ax,iw");
}

void testAddEaxId_0x005() {
    testRunBinaryAccumulatorImmediate(TEST_BINARY_ADD, 32, ADD32_CASES, caseCount(ADD32_CASES), "add eax,id");
}

void testAddE8Ib_0x080() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADD, 8, ADD8_CASES, caseCount(ADD8_CASES), "add e8,ib 80", 0x80, false);
}

void testAddE8Ib_0x280() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADD, 8, ADD8_CASES, caseCount(ADD8_CASES), "add e8,ib 280", 0x80, false);
}

void testAddE16Iw_0x081() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADD, 16, ADD16_CASES, caseCount(ADD16_CASES), "add e16,iw 81", 0x81, false);
}

void testAddE32Id_0x281() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADD, 32, ADD32_CASES, caseCount(ADD32_CASES), "add e32,id 281", 0x81, false);
}

void testAddE8Ib_0x082() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADD, 8, ADD8_CASES, caseCount(ADD8_CASES), "add e8,ib 82", 0x82, false);
}

void testAddE8Ib_0x282() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADD, 8, ADD8_CASES, caseCount(ADD8_CASES), "add e8,ib 282", 0x82, false);
}

void testAddE16Ib_0x083() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADD, 16, ADD16_CASES, caseCount(ADD16_CASES), "add e16,ib 83", 0x83, true);
}

void testAddE32Ib_0x283() {
    testRunBinaryGroup1Immediate(TEST_BINARY_ADD, 32, ADD32_CASES, caseCount(ADD32_CASES), "add e32,ib 283", 0x83, true);
}

void testXaddE8R8_0x1c0() {
    runXaddCases(8, XADD8_CASES, caseCount(XADD8_CASES), "xadd r/m8,r8 1c0");
}

void testXaddE8R8_0x3c0() {
    runXaddCases(8, XADD8_CASES, caseCount(XADD8_CASES), "xadd r/m8,r8 3c0");
}

void testXaddE16R16_0x1c1() {
    runXaddCases(16, XADD16_CASES, caseCount(XADD16_CASES), "xadd r/m16,r16 1c1");
}

void testXaddE32R32_0x3c1() {
    runXaddCases(32, XADD32_CASES, caseCount(XADD32_CASES), "xadd r/m32,r32 3c1");
}

#endif
