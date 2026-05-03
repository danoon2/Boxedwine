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

#include "testXchg.h"
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
constexpr U32 CMPXCHG_MEM_BASE = 0x3000;
constexpr U32 CMPXCHG_SETCC_RESULT = 0x5000;
constexpr U32 INITIAL_FLAGS = CF | PF | AF | ZF | SF | OF | DF;
constexpr U32 FLAG_MASK = CF | PF | AF | ZF | SF | OF | DF;
constexpr U32 CMPXCHG_FLAG_MASK = CF | PF | AF | ZF | SF | OF | DF;

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

struct XchgCase {
    U32 dst;
    U32 src;
};

struct CmpXchgCase {
    U32 dst;
    U32 src;
    U32 acc;
};

struct CmpXchg8bCase {
    U64 dst;
    U64 acc;
    U64 src;
};

const XchgCase XCHG8_CASES[] = {
    {0x02, 0x01},
    {0x00, 0x00},
    {0xab, 0xfc},
    {0x80, 0x7f},
    {0xff, 0x11}
};

const XchgCase XCHG16_CASES[] = {
    {0x0002, 0x0001},
    {0x0000, 0x0000},
    {0xab38, 0xfc15},
    {0x8000, 0x7fff},
    {0xffff, 0x1111}
};

const XchgCase XCHG32_CASES[] = {
    {0x00000002, 0x00000001},
    {0x00000000, 0x00000000},
    {0x0000ab38, 0xfc150146},
    {0x80000000, 0x7fffffff},
    {0xffffffff, 0x11111111}
};

const CmpXchgCase CMPXCHG8_CASES[] = {
    {0x01, 0x02, 0x01},
    {0x01, 0x02, 0x09},
    {0x09, 0x02, 0x01},
    {0xff, 0x80, 0xff},
    {0x80, 0x7f, 0x7f},
    {0x00, 0xff, 0x00},
};

const CmpXchgCase CMPXCHG16_CASES[] = {
    {0x0001, 0x0002, 0x0001},
    {0x0001, 0x0002, 0x0009},
    {0x0009, 0x0002, 0x0001},
    {0xffff, 0x8000, 0xffff},
    {0x8000, 0x7fff, 0x7fff},
    {0x0000, 0xffff, 0x0000},
};

const CmpXchgCase CMPXCHG32_CASES[] = {
    {0x00000001, 0x00000002, 0x00000001},
    {0x00000001, 0x00000002, 0x00000009},
    {0x00000009, 0x00000002, 0x00000001},
    {0xffffffff, 0x80000000, 0xffffffff},
    {0x80000000, 0x7fffffff, 0x7fffffff},
    {0x00000000, 0xffffffff, 0x00000000},
};

const CmpXchg8bCase CMPXCHG8B_CASES[] = {
    {0x5555555544444444ULL, 0x5555555544444444ULL, 0x7777777766666666ULL},
    {0x5555555044444440ULL, 0x5555555544444444ULL, 0x7777777766666666ULL},
    {0x0000000000000000ULL, 0x0000000000000000ULL, 0xffffffff80000000ULL},
    {0xffffffffffffffffULL, 0x7fffffff80000000ULL, 0x123456789abcdef0ULL},
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit xchg code init failed");
    }
}

void pushGeneratedCode(const asmjit::CodeHolder& code) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        pushCode8(buffer.data()[i]);
    }
}

void emitXchgRegister(int dst, int src, int width) {
    if (width == 16) {
        pushCode8(0x66);
    }
    pushCode8(width == 8 ? 0x86 : 0x87);
    pushCode8(0xc0 | (src << 3) | dst);
}

void emitXchg(const asmjit::x86::Mem& dst, asmjit::x86::Gp src) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.xchg(dst, src) != asmjit::Error::kOk) {
        failed("asmjit xchg mem, reg failed");
    }
    pushGeneratedCode(code);
}

void emitAccumulatorXchg(int reg, int width) {
    if (width == 16) {
        pushCode8(0x66);
    }
    pushCode8(0x90 + reg);
}

void emitDirectAddressModRM(int regField, U32 address, bool big) {
    if (big) {
        pushCode8(0x05 | (regField << 3));
        pushCode32(address);
    } else {
        pushCode8(0x06 | (regField << 3));
        pushCode16((U16)address);
    }
}

void emitCmpXchgPrefix(int width, bool big, bool lockPrefix) {
    if (lockPrefix) {
        pushCode8(0xf0);
    }
    if ((width == 16 && big) || (width == 32 && !big)) {
        pushCode8(0x66);
    }
    pushCode8(0x0f);
    pushCode8(width == 8 ? 0xb0 : 0xb1);
}

void emitCmpXchgRegReg(int dst, int src, int width, bool big) {
    emitCmpXchgPrefix(width, big, false);
    pushCode8(0xc0 | (src << 3) | dst);
}

void emitCmpXchgMemReg(int src, int width, U32 address, bool big, bool lockPrefix) {
    emitCmpXchgPrefix(width, big, lockPrefix);
    emitDirectAddressModRM(src, address, big);
}

void emitCmpXchg8bMem(U32 address, bool lockPrefix) {
    if (lockPrefix) {
        pushCode8(0xf0);
    }
    pushCode8(0x0f);
    pushCode8(0xc7);
    emitDirectAddressModRM(1, address, true);
}

void emitSetzMem(U32 address, bool big) {
    pushCode8(0x0f);
    pushCode8(0x94);
    emitDirectAddressModRM(0, address, big);
}

void emitCmpEaxEax() {
    pushCode8(0x39);
    pushCode8(0xc0);
}

bool registerOverlapsAddressReg(int reg, int width, int addressReg) {
    if (width == 8) {
        return physicalReg8(reg) == addressReg;
    }
    return reg == addressReg;
}

void initRegisters(U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
}

U32 regValueFromArray(const U32* regs, int reg, int width) {
    if (width == 8) {
        int physical = physicalReg8(reg);
        return reg < 4 ? regs[physical] & 0xff : (regs[physical] >> 8) & 0xff;
    }
    return regs[reg] & widthMask(width);
}

void verifyFlagsUnchanged(const char* name) {
    if ((actualFlags(cpu, true) & FLAG_MASK) != INITIAL_FLAGS) {
        failed("%s flags changed", name);
    }
}

U32 flagsForCmp(U32 lhs, U32 rhs, int width, U32 initialFlags) {
    U32 mask = widthMask(width);
    U32 sign = signBit(width);
    lhs &= mask;
    rhs &= mask;
    U32 result = (lhs - rhs) & mask;
    U32 flags = initialFlags & DF;

    if (lhs < rhs) {
        flags |= CF;
    }
    if (((lhs ^ rhs) & (lhs ^ result) & sign) != 0) {
        flags |= OF;
    }
    if (((lhs ^ rhs ^ result) & 0x10) != 0) {
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

U32 expectedCmpXchgMemory(const CmpXchgCase& data, int width, U32 actualSrc, U32 actualAcc) {
    U32 mask = widthMask(width);
    return ((actualAcc & mask) == (data.dst & mask)) ? (actualSrc & mask) : (data.dst & mask);
}

void verifyCmpXchgFlags(U32 expectedFlags, const char* name) {
    if (((actualFlags(cpu, true) ^ expectedFlags) & CMPXCHG_FLAG_MASK) != 0) {
        failed("%s flags", name);
    }
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

U32 baseMemoryOffset(int base, int width, int valueReg, U32 value) {
    if (width == 8) {
        return 0x2022 + base * 0x80;
    }
    if (width == 16 && valueReg == base && (value & 0xffff) >= 0xfe00) {
        return 0x2000 + base * 0x80;
    }
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
        if (memory->readb(address) != expected) {
            failed("%s memory value", name);
        }
        if (memory->readb(address - 1) != 0x11 || memory->readb(address + 1) != 0x33 || memory->readb(address + 2) != 0x44 || memory->readb(address + 3) != 0x55) {
            failed("%s memory guard", name);
        }
    } else if (width == 16) {
        if (memory->readw(address) != expected) {
            failed("%s memory value", name);
        }
        if (memory->readw(address - 2) != 0x1111 || memory->readw(address + 2) != 0x3333) {
            failed("%s memory guard", name);
        }
    } else {
        if (memory->readd(address) != expected) {
            failed("%s memory value", name);
        }
        if (memory->readw(address - 2) != 0x1111 || memory->readw(address + 4) != 0x3333) {
            failed("%s memory guard", name);
        }
    }
}

void runPreparedMemoryCase(int reg, int width, U32 linearAddress, const XchgCase& data, const char* name) {
    U32 expectedRegs[8];
    U32 actualReg = getRegValue(cpu, reg, width);

    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    applyRegValue(expectedRegs, reg, width, data.dst);

    prepareMemTarget(linearAddress, data.dst, width);
    runTestCPU();
    verifyMemTarget(linearAddress, actualReg, width, name);
    verifyRegisters(cpu, expectedRegs, name);
    verifyFlagsUnchanged(name);
}

void runBaseMemoryCases(int reg, int width, const XchgCase& data, const char* name) {
    for (int base = 0; base < 8; ++base) {
        U32 regs[8];
        U32 baseOffset = baseMemoryOffset(base, width, reg, data.src);
        initAddressRegisters(regs);
        regs[base] = baseOffset;
        if (!registerOverlapsAddressReg(reg, width, base)) {
            applyRegValue(regs, reg, width, data.src);
        }

        if (base != R_BP) {
            newInstruction(INITIAL_FLAGS);
            emitXchg(memPtr(reg32(base), 0, width), regForWidth(reg, width));
            writeRegs(cpu, regs);
            runPreparedMemoryCase(reg, width, segmentBaseForAddressReg(base) + regs[base], data, name);
        }

        newInstruction(INITIAL_FLAGS);
        emitXchg(memPtr(reg32(base), 0x11, width), regForWidth(reg, width));
        writeRegs(cpu, regs);
        runPreparedMemoryCase(reg, width, segmentBaseForAddressReg(base) + regs[base] + 0x11, data, name);

        newInstruction(INITIAL_FLAGS);
        emitXchg(memPtr(reg32(base), 0x123, width), regForWidth(reg, width));
        writeRegs(cpu, regs);
        runPreparedMemoryCase(reg, width, segmentBaseForAddressReg(base) + regs[base] + 0x123, data, name);
    }
}

void runAbsoluteMemoryCases(int reg, int width, const XchgCase& data, const char* name) {
    U32 regs[8];
    U32 offset = MEM_BASE + 0x3000;
    initAddressRegisters(regs);
    applyRegValue(regs, reg, width, data.src);

    newInstruction(INITIAL_FLAGS);
    emitXchg(memPtr(offset, width), regForWidth(reg, width));
    writeRegs(cpu, regs);
    runPreparedMemoryCase(reg, width, cpu->seg[DS].address + offset, data, name);
}

void runSibMemoryCases(int reg, int width, const XchgCase& data, const char* name) {
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
                if (!registerOverlapsAddressReg(reg, width, base) && !registerOverlapsAddressReg(reg, width, index)) {
                    applyRegValue(regs, reg, width, data.src);
                }

                S32 disp = (S32)(targetOffset - regs[base] - (regs[index] << shift));
                newInstruction(INITIAL_FLAGS);
                emitXchg(memPtr(reg32(base), reg32(index), shift, disp, width), regForWidth(reg, width));
                writeRegs(cpu, regs);
                runPreparedMemoryCase(reg, width, segmentBaseForAddressReg(base) + targetOffset, data, name);
            }
        }
    }
}

void runRegisterCases(int width, const XchgCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int dst = 0; dst < 8; ++dst) {
            for (int src = 0; src < 8; ++src) {
                U32 expectedRegs[8];

                newInstruction(INITIAL_FLAGS);
                emitXchgRegister(dst, src, width);
                initRegisters(expectedRegs);
                applyRegValue(expectedRegs, dst, width, cases[i].dst);
                setRegValue(cpu, dst, width, cases[i].dst);
                if (dst != src) {
                    applyRegValue(expectedRegs, src, width, cases[i].src);
                    setRegValue(cpu, src, width, cases[i].src);
                    U32 actualDst = getRegValue(cpu, dst, width);
                    U32 actualSrc = getRegValue(cpu, src, width);
                    applyRegValue(expectedRegs, dst, width, actualSrc);
                    applyRegValue(expectedRegs, src, width, actualDst);
                }

                runTestCPU();
                verifyRegisters(cpu, expectedRegs, name);
                verifyFlagsUnchanged(name);
            }
        }
    }
}

void runMemoryCases(int width, const XchgCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int reg = 0; reg < 8; ++reg) {
            runBaseMemoryCases(reg, width, cases[i], name);
            runAbsoluteMemoryCases(reg, width, cases[i], name);
            runSibMemoryCases(reg, width, cases[i], name);
        }
    }
}

void runAccumulatorCase(int reg, int width, const XchgCase& data, const char* name) {
    U32 expectedRegs[8];
    newInstruction(INITIAL_FLAGS);
    emitAccumulatorXchg(reg, width);
    initRegisters(expectedRegs);
    applyRegValue(expectedRegs, R_AX, width, data.dst);
    setRegValue(cpu, R_AX, width, data.dst);
    if (reg != R_AX) {
        applyRegValue(expectedRegs, reg, width, data.src);
        setRegValue(cpu, reg, width, data.src);
        applyRegValue(expectedRegs, R_AX, width, getRegValue(cpu, reg, width));
        applyRegValue(expectedRegs, reg, width, getRegValue(cpu, R_AX, width));
    }
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    verifyFlagsUnchanged(name);
}

void runAccumulatorCases(int reg, int width, const char* name) {
    const XchgCase* cases = width == 16 ? XCHG16_CASES : XCHG32_CASES;
    size_t count = width == 16 ? caseCount(XCHG16_CASES) : caseCount(XCHG32_CASES);
    for (size_t i = 0; i < count; ++i) {
        runAccumulatorCase(reg, width, cases[i], name);
    }
}

void runCmpXchgRegCase(int width, bool big, int dst, int src, const CmpXchgCase& data, int flagMode, const char* name) {
    char caseName[160];
    snprintf(caseName, sizeof(caseName), "%s reg dst=%d src=%d dstValue=%x srcValue=%x acc=%x mode=%d", name, dst, src, data.dst, data.src, data.acc, flagMode);

    U32 regs[8];
    U32 expectedRegs[8];
    initRegisters(regs);
    applyRegValue(regs, dst, width, data.dst);
    applyRegValue(regs, src, width, data.src);
    applyRegValue(regs, R_AX, width, data.acc);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }

    U32 actualDst = regValueFromArray(regs, dst, width);
    U32 actualSrc = regValueFromArray(regs, src, width);
    U32 actualAcc = regValueFromArray(regs, R_AX, width);
    U32 expectedFlags = flagsForCmp(actualAcc, actualDst, width, INITIAL_FLAGS);
    bool equal = (actualAcc & widthMask(width)) == (actualDst & widthMask(width));
    if (equal) {
        applyRegValue(expectedRegs, dst, width, actualSrc);
    } else {
        applyRegValue(expectedRegs, R_AX, width, actualDst);
    }

    newInstruction(INITIAL_FLAGS);
    cpu->big = big;
    emitCmpXchgRegReg(dst, src, width, big);
    if (flagMode == 1) {
        emitCmpEaxEax();
    } else if (flagMode == 2) {
        emitSetzMem(CMPXCHG_SETCC_RESULT, big);
    }
    writeRegs(cpu, regs);
    memory->writeb(cpu->seg[DS].address + CMPXCHG_SETCC_RESULT, 0xcc);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    if (flagMode == 0) {
        verifyCmpXchgFlags(expectedFlags, caseName);
    } else if (flagMode == 2 && memory->readb(cpu->seg[DS].address + CMPXCHG_SETCC_RESULT) != (U8)(equal ? 1 : 0)) {
        failed("%s setz", caseName);
    }
}

void runCmpXchgMemCase(int width, bool big, int src, const CmpXchgCase& data, int flagMode, bool lockPrefix, bool unaligned, bool crossPage, const char* name) {
    if (crossPage && width == 8) {
        return;
    }

    char caseName[200];
    snprintf(caseName, sizeof(caseName), "%s mem src=%d dstValue=%x srcValue=%x acc=%x mode=%d lock=%d unaligned=%d crossPage=%d", name, src, data.dst, data.src, data.acc, flagMode, lockPrefix ? 1 : 0, unaligned ? 1 : 0, crossPage ? 1 : 0);

    U32 address = crossPage ? (K_PAGE_SIZE * 2 - width / 8 + 1) : CMPXCHG_MEM_BASE + (unaligned ? 1 : 0);
    U32 linear = cpu->seg[DS].address + address;
    U32 regs[8];
    U32 expectedRegs[8];
    initRegisters(regs);
    applyRegValue(regs, src, width, data.src);
    applyRegValue(regs, R_AX, width, data.acc);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }

    U32 actualSrc = regValueFromArray(regs, src, width);
    U32 actualAcc = regValueFromArray(regs, R_AX, width);
    U32 expectedFlags = flagsForCmp(actualAcc, data.dst, width, INITIAL_FLAGS);
    U32 expectedMemory = expectedCmpXchgMemory(data, width, actualSrc, actualAcc);
    bool equal = (actualAcc & widthMask(width)) == (data.dst & widthMask(width));
    if (!equal) {
        applyRegValue(expectedRegs, R_AX, width, data.dst);
    }

    newInstruction(INITIAL_FLAGS);
    cpu->big = big;
    emitCmpXchgMemReg(src, width, address, big, lockPrefix);
    if (flagMode == 1) {
        emitCmpEaxEax();
    } else if (flagMode == 2) {
        emitSetzMem(CMPXCHG_SETCC_RESULT, big);
    }
    writeRegs(cpu, regs);
    prepareMemTarget(linear, data.dst, width);
    memory->writeb(cpu->seg[DS].address + CMPXCHG_SETCC_RESULT, 0xcc);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    verifyMemTarget(linear, expectedMemory, width, caseName);
    if (flagMode == 0) {
        verifyCmpXchgFlags(expectedFlags, caseName);
    } else if (flagMode == 2 && memory->readb(cpu->seg[DS].address + CMPXCHG_SETCC_RESULT) != (U8)(equal ? 1 : 0)) {
        failed("%s setz", caseName);
    }
}

void runCmpXchgCases(int width, bool big, const CmpXchgCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int flagMode = 0; flagMode < 3; ++flagMode) {
            for (int dst = 0; dst < 8; ++dst) {
                for (int src = 0; src < 8; ++src) {
                    runCmpXchgRegCase(width, big, dst, src, cases[i], flagMode, name);
                }
            }
            for (int src = 0; src < 8; ++src) {
                runCmpXchgMemCase(width, big, src, cases[i], flagMode, false, false, false, name);
                runCmpXchgMemCase(width, big, src, cases[i], flagMode, false, true, false, name);
                runCmpXchgMemCase(width, big, src, cases[i], flagMode, false, false, true, name);
                runCmpXchgMemCase(width, big, src, cases[i], flagMode, true, false, false, name);
                runCmpXchgMemCase(width, big, src, cases[i], flagMode, true, true, false, name);
                runCmpXchgMemCase(width, big, src, cases[i], flagMode, true, false, true, name);
            }
        }
    }
}

void prepareMem64Target(U32 address, U64 value) {
    memory->writew(address - 2, 0x1111);
    memory->writeq(address, value);
    memory->writew(address + 8, 0x3333);
}

void verifyMem64Target(U32 address, U64 expected, const char* name) {
    if (memory->readq(address) != expected) {
        failed("%s memory value", name);
    }
    if (memory->readw(address - 2) != 0x1111 || memory->readw(address + 8) != 0x3333) {
        failed("%s memory guard", name);
    }
}

void runCmpXchg8bCase(const CmpXchg8bCase& data, bool lockPrefix, bool unaligned, bool crossPage, int flagMode, const char* name) {
    char caseName[200];
    snprintf(caseName, sizeof(caseName), "%s dst=%llx acc=%llx src=%llx lock=%d unaligned=%d crossPage=%d mode=%d",
        name,
        (unsigned long long)data.dst,
        (unsigned long long)data.acc,
        (unsigned long long)data.src,
        lockPrefix ? 1 : 0,
        unaligned ? 1 : 0,
        crossPage ? 1 : 0,
        flagMode);

    U32 address = crossPage ? (K_PAGE_SIZE * 2 - 7) : CMPXCHG_MEM_BASE + (unaligned ? 1 : 0);
    U32 linear = cpu->seg[DS].address + address;
    bool equal = data.acc == data.dst;
    U64 expectedMemory = equal ? data.src : data.dst;
    U32 expectedFlags = (INITIAL_FLAGS & ~ZF) | (equal ? ZF : 0);
    U32 expectedRegs[8];

    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
    emitCmpXchg8bMem(address, lockPrefix);
    if (flagMode == 1) {
        emitCmpEaxEax();
    } else if (flagMode == 2) {
        emitSetzMem(CMPXCHG_SETCC_RESULT, true);
    }

    initRegisters(expectedRegs);
    EAX = (U32)data.acc;
    EDX = (U32)(data.acc >> 32);
    EBX = (U32)data.src;
    ECX = (U32)(data.src >> 32);
    expectedRegs[R_AX] = EAX;
    expectedRegs[R_DX] = EDX;
    expectedRegs[R_BX] = EBX;
    expectedRegs[R_CX] = ECX;
    if (!equal) {
        expectedRegs[R_AX] = (U32)data.dst;
        expectedRegs[R_DX] = (U32)(data.dst >> 32);
    }

    prepareMem64Target(linear, data.dst);
    memory->writeb(cpu->seg[DS].address + CMPXCHG_SETCC_RESULT, 0xcc);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    verifyMem64Target(linear, expectedMemory, caseName);
    if (flagMode == 0) {
        verifyCmpXchgFlags(expectedFlags, caseName);
    } else if (flagMode == 2 && memory->readb(cpu->seg[DS].address + CMPXCHG_SETCC_RESULT) != (U8)(equal ? 1 : 0)) {
        failed("%s setz", caseName);
    }
}

void runCmpXchg8bCases(const char* name) {
    for (size_t i = 0; i < caseCount(CMPXCHG8B_CASES); ++i) {
        for (int flagMode = 0; flagMode < 3; ++flagMode) {
            for (int lockPrefix = 0; lockPrefix < 2; ++lockPrefix) {
                runCmpXchg8bCase(CMPXCHG8B_CASES[i], lockPrefix != 0, false, false, flagMode, name);
                runCmpXchg8bCase(CMPXCHG8B_CASES[i], lockPrefix != 0, true, false, flagMode, name);
                runCmpXchg8bCase(CMPXCHG8B_CASES[i], lockPrefix != 0, false, true, flagMode, name);
            }
        }
    }
}

} // namespace

void testXchgR8R8_0x086() {
    runRegisterCases(8, XCHG8_CASES, caseCount(XCHG8_CASES), "xchg r8,r8 86");
}

void testXchgE8R8_0x086() {
    runMemoryCases(8, XCHG8_CASES, caseCount(XCHG8_CASES), "xchg m8,r8 86");
}

void testXchgR8R8_0x286() {
    runRegisterCases(8, XCHG8_CASES, caseCount(XCHG8_CASES), "xchg r8,r8 286");
}

void testXchgE8R8_0x286() {
    runMemoryCases(8, XCHG8_CASES, caseCount(XCHG8_CASES), "xchg m8,r8 286");
}

void testXchgR16R16_0x087() {
    runRegisterCases(16, XCHG16_CASES, caseCount(XCHG16_CASES), "xchg r16,r16 87");
}

void testXchgE16R16_0x087() {
    runMemoryCases(16, XCHG16_CASES, caseCount(XCHG16_CASES), "xchg m16,r16 87");
}

void testXchgR32R32_0x287() {
    runRegisterCases(32, XCHG32_CASES, caseCount(XCHG32_CASES), "xchg r32,r32 287");
}

void testXchgE32R32_0x287() {
    runMemoryCases(32, XCHG32_CASES, caseCount(XCHG32_CASES), "xchg m32,r32 287");
}

void testXchgR16Ax_0x090() { runAccumulatorCases(R_AX, 16, "xchg ax,ax 90"); }
void testXchgR32Eax_0x290() { runAccumulatorCases(R_AX, 32, "xchg eax,eax 290"); }
void testXchgR16Ax_0x091() { runAccumulatorCases(R_CX, 16, "xchg cx,ax 91"); }
void testXchgR32Eax_0x291() { runAccumulatorCases(R_CX, 32, "xchg ecx,eax 291"); }
void testXchgR16Ax_0x092() { runAccumulatorCases(R_DX, 16, "xchg dx,ax 92"); }
void testXchgR32Eax_0x292() { runAccumulatorCases(R_DX, 32, "xchg edx,eax 292"); }
void testXchgR16Ax_0x093() { runAccumulatorCases(R_BX, 16, "xchg bx,ax 93"); }
void testXchgR32Eax_0x293() { runAccumulatorCases(R_BX, 32, "xchg ebx,eax 293"); }
void testXchgR16Ax_0x094() { runAccumulatorCases(R_SP, 16, "xchg sp,ax 94"); }
void testXchgR32Eax_0x294() { runAccumulatorCases(R_SP, 32, "xchg esp,eax 294"); }
void testXchgR16Ax_0x095() { runAccumulatorCases(R_BP, 16, "xchg bp,ax 95"); }
void testXchgR32Eax_0x295() { runAccumulatorCases(R_BP, 32, "xchg ebp,eax 295"); }
void testXchgR16Ax_0x096() { runAccumulatorCases(R_SI, 16, "xchg si,ax 96"); }
void testXchgR32Eax_0x296() { runAccumulatorCases(R_SI, 32, "xchg esi,eax 296"); }
void testXchgR16Ax_0x097() { runAccumulatorCases(R_DI, 16, "xchg di,ax 97"); }
void testXchgR32Eax_0x297() { runAccumulatorCases(R_DI, 32, "xchg edi,eax 297"); }

void testCmpXchgE8R8_0x1b0() { runCmpXchgCases(8, false, CMPXCHG8_CASES, caseCount(CMPXCHG8_CASES), "cmpxchg e8,r8 1b0"); }
void testCmpXchgE8R8_0x3b0() { runCmpXchgCases(8, true, CMPXCHG8_CASES, caseCount(CMPXCHG8_CASES), "cmpxchg e8,r8 3b0"); }
void testCmpXchgE16R16_0x1b1() { runCmpXchgCases(16, false, CMPXCHG16_CASES, caseCount(CMPXCHG16_CASES), "cmpxchg e16,r16 1b1"); }
void testCmpXchgE32R32_0x3b1() { runCmpXchgCases(32, true, CMPXCHG32_CASES, caseCount(CMPXCHG32_CASES), "cmpxchg e32,r32 3b1"); }
void testCmpXchg8b_0x3c7() { runCmpXchg8bCases("cmpxchg8b 3c7"); }

#endif
