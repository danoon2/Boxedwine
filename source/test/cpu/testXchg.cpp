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

#ifdef BOXEDWINE_MULTI_THREADED
#include "../../emulation/cpu/common/common_lock.h"
#include "ksignal.h"
#endif

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
#ifdef BOXEDWINE_MULTI_THREADED
#ifdef __EMSCRIPTEN__
constexpr U32 LOCKED_PLAIN_STORE_RACE_ITERATIONS = 1000;
constexpr U32 LOCKED_ORDERING_ITERATIONS = 1000;
#else
constexpr U32 LOCKED_PLAIN_STORE_RACE_ITERATIONS = 100000;
constexpr U32 LOCKED_ORDERING_ITERATIONS = 100000;
#endif
constexpr U32 LOCKED_PLAIN_STORE_TARGET = 0xc8;
constexpr U32 LOCKED_PLAIN_STORE_UNALIGNED_TARGET = 0xc9;
constexpr U32 LOCKED_PLAIN_STORE_PHASE = 0xd0;
constexpr U32 LOCKED_PLAIN_STORE_WRITER_DONE = 0xd4;
constexpr U32 LOCKED_PLAIN_STORE_ACK = 0xd8;
constexpr U32 LOCKED_PLAIN_STORE_ERROR = 0xdc;
constexpr U32 LOCKED_PLAIN_STORE_SENTINEL32 = 0x10000;
constexpr U32 LOCKED_PLAIN_STORE_SENTINEL16 = 0x4000;
constexpr U32 LOCKED_PLAIN_STORE_SENTINEL8 = 0x40;
constexpr U32 LOCKED_CMPXCHG_REPLACEMENT = 2;
constexpr U32 LOCKED_XADD_ADDEND = 2;
constexpr U32 LOCKED_READ_VALUE_A = 0x55aa33cc;
constexpr U32 LOCKED_READ_VALUE_B = 0xaa55cc33;
constexpr U32 LOCKED_CMPXCHG8B_TARGET = 0xe0;
constexpr U32 LOCKED_CMPXCHG8B_REPLACEMENT_LOW = 0x11112222;
constexpr U32 LOCKED_CMPXCHG8B_REPLACEMENT_HIGH = 0x33334444;
constexpr U32 LOCKED_CMPXCHG8B_SENTINEL_LOW = 0x55556666;
constexpr U32 LOCKED_CMPXCHG8B_SENTINEL_HIGH = 0x77778888;
constexpr U32 LOCKED_ORDERING_DATA = 0xc8;
constexpr U32 LOCKED_ORDERING_FENCE = 0xcc;
constexpr U32 LOCKED_ORDERING_READY = 0xd0;
constexpr U32 LOCKED_ORDERING_ACK = 0xd4;
constexpr U32 LOCKED_ORDERING_ERROR = 0xd8;
#endif

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
        if (!testRunMemoryBase(base)) {
            continue;
        }
        U32 regs[8];
        U32 baseOffset = baseMemoryOffset(base, width, reg, data.src);
        initAddressRegisters(regs);
        regs[base] = baseOffset;
        if (!registerOverlapsAddressReg(reg, width, base)) {
            applyRegValue(regs, reg, width, data.src);
        }

        if (base != R_BP && testRunMemoryBaseDisplacement(base, 0)) {
            newInstruction(INITIAL_FLAGS);
            emitXchg(memPtr(reg32(base), 0, width), regForWidth(reg, width));
            writeRegs(cpu, regs);
            runPreparedMemoryCase(reg, width, segmentBaseForAddressReg(base) + regs[base], data, name);
        }

        if (testRunMemoryBaseDisplacement(base, 1)) {
            newInstruction(INITIAL_FLAGS);
            emitXchg(memPtr(reg32(base), 0x11, width), regForWidth(reg, width));
            writeRegs(cpu, regs);
            runPreparedMemoryCase(reg, width, segmentBaseForAddressReg(base) + regs[base] + 0x11, data, name);
        }

        if (testRunMemoryBaseDisplacement(base, 2)) {
            newInstruction(INITIAL_FLAGS);
            emitXchg(memPtr(reg32(base), 0x123, width), regForWidth(reg, width));
            writeRegs(cpu, regs);
            runPreparedMemoryCase(reg, width, segmentBaseForAddressReg(base) + regs[base] + 0x123, data, name);
        }
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
                if (!testRunMemorySib(base, index, shift)) {
                    continue;
                }
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
                if (!testRunRegisterPair(dst, src)) {
                    continue;
                }
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
            if (!testRunRegister(reg)) {
                continue;
            }
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
                    if (!testRunRegisterPair(dst, src)) {
                        continue;
                    }
                    runCmpXchgRegCase(width, big, dst, src, cases[i], flagMode, name);
                }
            }
            for (int src = 0; src < 8; ++src) {
                if (!testRunRegister(src)) {
                    continue;
                }
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

#ifdef BOXEDWINE_MULTI_THREADED
#undef cpu
#undef memory
asmjit::x86::Mem targetMem(U32 address, int width) {
    if (width == 8) {
        return asmjit::x86::byte_ptr(address);
    }
    if (width == 16) {
        return asmjit::x86::word_ptr(address);
    }
    if (width == 32) {
        return asmjit::x86::dword_ptr(address);
    }
    failed("unsupported locked race width %d", width);
    return asmjit::x86::dword_ptr(address);
}

asmjit::x86::Gp bxForWidth(int width) {
    if (width == 8) {
        return asmjit::x86::bl;
    }
    if (width == 16) {
        return asmjit::x86::bx;
    }
    if (width == 32) {
        return asmjit::x86::ebx;
    }
    failed("unsupported locked race bx width %d", width);
    return asmjit::x86::ebx;
}

U32 sentinelForWidth(int width) {
    if (width == 8) {
        return LOCKED_PLAIN_STORE_SENTINEL8;
    }
    if (width == 16) {
        return LOCKED_PLAIN_STORE_SENTINEL16;
    }
    if (width == 32) {
        return LOCKED_PLAIN_STORE_SENTINEL32;
    }
    failed("unsupported locked race sentinel width %d", width);
    return LOCKED_PLAIN_STORE_SENTINEL32;
}

void writeTargetValue(KMemory* memory, U32 linearAddress, int width, U32 value) {
    if (width == 8) {
        memory->writeb(linearAddress, value);
    } else if (width == 16) {
        memory->writew(linearAddress, value);
    } else if (width == 32) {
        memory->writed(linearAddress, value);
    } else {
        failed("unsupported locked race write width %d", width);
    }
}

U32 readTargetValue(KMemory* memory, U32 linearAddress, int width) {
    if (width == 8) {
        return memory->readb(linearAddress);
    }
    if (width == 16) {
        return memory->readw(linearAddress);
    }
    if (width == 32) {
        return memory->readd(linearAddress);
    }
    failed("unsupported locked race read width %d", width);
    return 0;
}

void expectAsmOk(asmjit::Error err, const char* name) {
    if (err != asmjit::Error::kOk) {
        failed("%s failed", name);
    }
}

void emitWaitDwordEquals(asmjit::x86::Assembler& a, U32 address, asmjit::x86::Gp value, const char* name) {
    asmjit::Label wait = a.new_label();
    expectAsmOk(a.bind(wait), name);
    expectAsmOk(a.cmp(asmjit::x86::dword_ptr(address), value), name);
    expectAsmOk(a.jne(wait), name);
}

U32 emitPlainStoreWriterLoop(U32 target, int width, U32 sentinel) {
    U32 eip = testContext().codeIp - TEST_CODE_ADDRESS;
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();

    expectAsmOk(a.mov(asmjit::x86::edx, 1), "asmjit cmpxchg plain store phase init");
    expectAsmOk(a.bind(loop), "asmjit cmpxchg plain store loop bind");
    emitWaitDwordEquals(a, LOCKED_PLAIN_STORE_PHASE, asmjit::x86::edx, "asmjit cmpxchg plain store wait phase");
    expectAsmOk(a.mov(targetMem(target, width), sentinel), "asmjit cmpxchg plain store target");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_WRITER_DONE), asmjit::x86::edx), "asmjit cmpxchg plain store done");
    emitWaitDwordEquals(a, LOCKED_PLAIN_STORE_ACK, asmjit::x86::edx, "asmjit cmpxchg plain store wait ack");
    expectAsmOk(a.inc(asmjit::x86::edx), "asmjit cmpxchg plain store phase inc");
    expectAsmOk(a.loop(loop), "asmjit cmpxchg plain store loop");
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
    return eip;
}

U32 emitLockedCmpXchgAgainstPlainStoreLoop(U32 target, int width) {
    U32 eip = testContext().codeIp - TEST_CODE_ADDRESS;
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();
    asmjit::Label ok = a.new_label();

    expectAsmOk(a.mov(asmjit::x86::edx, 1), "asmjit locked cmpxchg phase init");
    expectAsmOk(a.bind(loop), "asmjit locked cmpxchg plain loop bind");
    expectAsmOk(a.mov(targetMem(target, width), 0), "asmjit locked cmpxchg target reset");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_PHASE), asmjit::x86::edx), "asmjit locked cmpxchg phase publish");
    expectAsmOk(a.mov(asmjit::x86::eax, 0), "asmjit locked cmpxchg eax");
    expectAsmOk(a.mov(asmjit::x86::ebx, LOCKED_CMPXCHG_REPLACEMENT), "asmjit locked cmpxchg ebx");
    a.lock();
    expectAsmOk(a.cmpxchg(targetMem(target, width), bxForWidth(width)), "asmjit locked cmpxchg plain target");
    emitWaitDwordEquals(a, LOCKED_PLAIN_STORE_WRITER_DONE, asmjit::x86::edx, "asmjit locked cmpxchg wait writer");
    expectAsmOk(a.cmp(targetMem(target, width), LOCKED_CMPXCHG_REPLACEMENT), "asmjit locked cmpxchg check target");
    expectAsmOk(a.jne(ok), "asmjit locked cmpxchg check branch");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_ERROR), asmjit::x86::edx), "asmjit locked cmpxchg error");
    expectAsmOk(a.bind(ok), "asmjit locked cmpxchg ok bind");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_ACK), asmjit::x86::edx), "asmjit locked cmpxchg ack");
    expectAsmOk(a.inc(asmjit::x86::edx), "asmjit locked cmpxchg phase inc");
    expectAsmOk(a.loop(loop), "asmjit locked cmpxchg plain loop");
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
    return eip;
}

U32 emitImplicitXchgAgainstPlainStoreLoop(U32 target, int width) {
    U32 eip = testContext().codeIp - TEST_CODE_ADDRESS;
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();
    asmjit::Label ok = a.new_label();

    expectAsmOk(a.mov(asmjit::x86::edx, 1), "asmjit implicit xchg phase init");
    expectAsmOk(a.bind(loop), "asmjit implicit xchg loop bind");
    expectAsmOk(a.mov(targetMem(target, width), 0), "asmjit implicit xchg target reset");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_PHASE), asmjit::x86::edx), "asmjit implicit xchg phase publish");
    expectAsmOk(a.mov(asmjit::x86::eax, LOCKED_CMPXCHG_REPLACEMENT), "asmjit implicit xchg eax");
    expectAsmOk(a.xchg(targetMem(target, width), width == 8 ? asmjit::x86::al : (width == 16 ? asmjit::x86::ax : asmjit::x86::eax)), "asmjit implicit xchg target");
    emitWaitDwordEquals(a, LOCKED_PLAIN_STORE_WRITER_DONE, asmjit::x86::edx, "asmjit implicit xchg wait writer");
    expectAsmOk(a.cmp(targetMem(target, width), LOCKED_CMPXCHG_REPLACEMENT), "asmjit implicit xchg check target");
    expectAsmOk(a.jne(ok), "asmjit implicit xchg target branch");
    expectAsmOk(a.cmp(asmjit::x86::eax, 0), "asmjit implicit xchg check old value");
    expectAsmOk(a.jne(ok), "asmjit implicit xchg old value branch");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_ERROR), asmjit::x86::edx), "asmjit implicit xchg error");
    expectAsmOk(a.bind(ok), "asmjit implicit xchg ok bind");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_ACK), asmjit::x86::edx), "asmjit implicit xchg ack");
    expectAsmOk(a.inc(asmjit::x86::edx), "asmjit implicit xchg phase inc");
    expectAsmOk(a.loop(loop), "asmjit implicit xchg loop");
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
    return eip;
}

U32 emitLockedCmpXchgFailureAgainstPlainStoreLoop(U32 target, int width, U32 sentinel) {
    U32 eip = testContext().codeIp - TEST_CODE_ADDRESS;
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();
    asmjit::Label ok = a.new_label();

    expectAsmOk(a.mov(asmjit::x86::edx, 1), "asmjit locked cmpxchg fail phase init");
    expectAsmOk(a.bind(loop), "asmjit locked cmpxchg fail loop bind");
    expectAsmOk(a.mov(targetMem(target, width), 0), "asmjit locked cmpxchg fail target reset");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_PHASE), asmjit::x86::edx), "asmjit locked cmpxchg fail phase publish");
    expectAsmOk(a.mov(asmjit::x86::eax, (sentinel + 1) & widthMask(width)), "asmjit locked cmpxchg fail eax");
    expectAsmOk(a.mov(asmjit::x86::ebx, LOCKED_CMPXCHG_REPLACEMENT), "asmjit locked cmpxchg fail ebx");
    a.lock();
    expectAsmOk(a.cmpxchg(targetMem(target, width), bxForWidth(width)), "asmjit locked cmpxchg fail target");
    emitWaitDwordEquals(a, LOCKED_PLAIN_STORE_WRITER_DONE, asmjit::x86::edx, "asmjit locked cmpxchg fail wait writer");
    expectAsmOk(a.cmp(targetMem(target, width), sentinel), "asmjit locked cmpxchg fail check target");
    expectAsmOk(a.je(ok), "asmjit locked cmpxchg fail branch");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_ERROR), asmjit::x86::edx), "asmjit locked cmpxchg fail error");
    expectAsmOk(a.bind(ok), "asmjit locked cmpxchg fail ok bind");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_ACK), asmjit::x86::edx), "asmjit locked cmpxchg fail ack");
    expectAsmOk(a.inc(asmjit::x86::edx), "asmjit locked cmpxchg fail phase inc");
    expectAsmOk(a.loop(loop), "asmjit locked cmpxchg fail loop");
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
    return eip;
}

U32 emitLockedXaddAgainstPlainStoreLoop(U32 target, int width) {
    U32 eip = testContext().codeIp - TEST_CODE_ADDRESS;
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();
    asmjit::Label ok = a.new_label();

    expectAsmOk(a.mov(asmjit::x86::edx, 1), "asmjit locked xadd phase init");
    expectAsmOk(a.bind(loop), "asmjit locked xadd loop bind");
    expectAsmOk(a.mov(targetMem(target, width), 0), "asmjit locked xadd target reset");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_PHASE), asmjit::x86::edx), "asmjit locked xadd phase publish");
    expectAsmOk(a.mov(asmjit::x86::ebx, LOCKED_XADD_ADDEND), "asmjit locked xadd ebx");
    a.lock();
    expectAsmOk(a.xadd(targetMem(target, width), bxForWidth(width)), "asmjit locked xadd target");
    emitWaitDwordEquals(a, LOCKED_PLAIN_STORE_WRITER_DONE, asmjit::x86::edx, "asmjit locked xadd wait writer");
    expectAsmOk(a.cmp(targetMem(target, width), LOCKED_XADD_ADDEND), "asmjit locked xadd check target");
    expectAsmOk(a.jne(ok), "asmjit locked xadd branch");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_ERROR), asmjit::x86::edx), "asmjit locked xadd error");
    expectAsmOk(a.bind(ok), "asmjit locked xadd ok bind");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_ACK), asmjit::x86::edx), "asmjit locked xadd ack");
    expectAsmOk(a.inc(asmjit::x86::edx), "asmjit locked xadd phase inc");
    expectAsmOk(a.loop(loop), "asmjit locked xadd loop");
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
    return eip;
}

U32 emitPlainReadAgainstLockedWriteReaderLoop(U32 target) {
    U32 eip = testContext().codeIp - TEST_CODE_ADDRESS;
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();
    asmjit::Label readAgain = a.new_label();
    asmjit::Label valueOk = a.new_label();

    expectAsmOk(a.mov(asmjit::x86::edx, 1), "asmjit plain read phase init");
    expectAsmOk(a.bind(loop), "asmjit plain read loop bind");
    emitWaitDwordEquals(a, LOCKED_PLAIN_STORE_PHASE, asmjit::x86::edx, "asmjit plain read wait phase");
    expectAsmOk(a.bind(readAgain), "asmjit plain read retry bind");
    expectAsmOk(a.mov(asmjit::x86::eax, asmjit::x86::dword_ptr(target)), "asmjit plain read target");
    expectAsmOk(a.cmp(asmjit::x86::eax, LOCKED_READ_VALUE_A), "asmjit plain read compare value a");
    expectAsmOk(a.je(valueOk), "asmjit plain read branch value a");
    expectAsmOk(a.cmp(asmjit::x86::eax, LOCKED_READ_VALUE_B), "asmjit plain read compare value b");
    expectAsmOk(a.je(valueOk), "asmjit plain read branch value b");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_ERROR), asmjit::x86::edx), "asmjit plain read error");
    expectAsmOk(a.bind(valueOk), "asmjit plain read ok bind");
    expectAsmOk(a.cmp(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_WRITER_DONE), asmjit::x86::edx), "asmjit plain read done check");
    expectAsmOk(a.jne(readAgain), "asmjit plain read retry");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_ACK), asmjit::x86::edx), "asmjit plain read ack");
    expectAsmOk(a.inc(asmjit::x86::edx), "asmjit plain read phase inc");
    expectAsmOk(a.loop(loop), "asmjit plain read loop");
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
    return eip;
}

U32 emitLockedWriteAgainstPlainReadWriterLoop(U32 target) {
    U32 eip = testContext().codeIp - TEST_CODE_ADDRESS;
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();

    expectAsmOk(a.mov(asmjit::x86::edx, 1), "asmjit locked write phase init");
    expectAsmOk(a.bind(loop), "asmjit locked write loop bind");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(target), LOCKED_READ_VALUE_A), "asmjit locked write reset");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_PHASE), asmjit::x86::edx), "asmjit locked write phase publish");
    expectAsmOk(a.mov(asmjit::x86::eax, LOCKED_READ_VALUE_B), "asmjit locked write eax");
    expectAsmOk(a.xchg(asmjit::x86::dword_ptr(target), asmjit::x86::eax), "asmjit locked write xchg");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_WRITER_DONE), asmjit::x86::edx), "asmjit locked write done");
    emitWaitDwordEquals(a, LOCKED_PLAIN_STORE_ACK, asmjit::x86::edx, "asmjit locked write wait ack");
    expectAsmOk(a.inc(asmjit::x86::edx), "asmjit locked write phase inc");
    expectAsmOk(a.loop(loop), "asmjit locked write loop");
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
    return eip;
}

U32 emitPlainStore64WriterLoop(U32 target) {
    U32 eip = testContext().codeIp - TEST_CODE_ADDRESS;
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();

    expectAsmOk(a.mov(asmjit::x86::edx, 1), "asmjit cmpxchg8b writer phase init");
    expectAsmOk(a.bind(loop), "asmjit cmpxchg8b writer loop bind");
    emitWaitDwordEquals(a, LOCKED_PLAIN_STORE_PHASE, asmjit::x86::edx, "asmjit cmpxchg8b writer wait phase");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(target), LOCKED_CMPXCHG8B_SENTINEL_LOW), "asmjit cmpxchg8b writer low");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(target + 4), LOCKED_CMPXCHG8B_SENTINEL_HIGH), "asmjit cmpxchg8b writer high");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_WRITER_DONE), asmjit::x86::edx), "asmjit cmpxchg8b writer done");
    emitWaitDwordEquals(a, LOCKED_PLAIN_STORE_ACK, asmjit::x86::edx, "asmjit cmpxchg8b writer wait ack");
    expectAsmOk(a.inc(asmjit::x86::edx), "asmjit cmpxchg8b writer phase inc");
    expectAsmOk(a.loop(loop), "asmjit cmpxchg8b writer loop");
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
    return eip;
}

U32 emitLockedCmpXchg8bAgainstPlainStoreLoop(U32 target) {
    U32 eip = testContext().codeIp - TEST_CODE_ADDRESS;
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();
    asmjit::Label ok = a.new_label();

    expectAsmOk(a.mov(asmjit::x86::edi, asmjit::x86::ecx), "asmjit cmpxchg8b iteration copy");
    expectAsmOk(a.mov(asmjit::x86::esi, 1), "asmjit cmpxchg8b phase init");
    expectAsmOk(a.bind(loop), "asmjit cmpxchg8b loop bind");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(target), 0), "asmjit cmpxchg8b target low reset");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(target + 4), 0), "asmjit cmpxchg8b target high reset");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_PHASE), asmjit::x86::esi), "asmjit cmpxchg8b phase publish");
    expectAsmOk(a.mov(asmjit::x86::eax, 0), "asmjit cmpxchg8b eax");
    expectAsmOk(a.mov(asmjit::x86::edx, 0), "asmjit cmpxchg8b edx");
    expectAsmOk(a.mov(asmjit::x86::ebx, LOCKED_CMPXCHG8B_REPLACEMENT_LOW), "asmjit cmpxchg8b ebx");
    expectAsmOk(a.mov(asmjit::x86::ecx, LOCKED_CMPXCHG8B_REPLACEMENT_HIGH), "asmjit cmpxchg8b ecx");
    a.lock();
    expectAsmOk(a.cmpxchg8b(asmjit::x86::qword_ptr(target)), "asmjit locked cmpxchg8b target");
    emitWaitDwordEquals(a, LOCKED_PLAIN_STORE_WRITER_DONE, asmjit::x86::esi, "asmjit cmpxchg8b wait writer");
    expectAsmOk(a.cmp(asmjit::x86::dword_ptr(target), LOCKED_CMPXCHG8B_REPLACEMENT_LOW), "asmjit cmpxchg8b check low");
    expectAsmOk(a.jne(ok), "asmjit cmpxchg8b low branch");
    expectAsmOk(a.cmp(asmjit::x86::dword_ptr(target + 4), LOCKED_CMPXCHG8B_REPLACEMENT_HIGH), "asmjit cmpxchg8b check high");
    expectAsmOk(a.jne(ok), "asmjit cmpxchg8b high branch");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_ERROR), asmjit::x86::esi), "asmjit cmpxchg8b error");
    expectAsmOk(a.bind(ok), "asmjit cmpxchg8b ok bind");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_PLAIN_STORE_ACK), asmjit::x86::esi), "asmjit cmpxchg8b ack");
    expectAsmOk(a.inc(asmjit::x86::esi), "asmjit cmpxchg8b phase inc");
    expectAsmOk(a.dec(asmjit::x86::edi), "asmjit cmpxchg8b iteration dec");
    expectAsmOk(a.jnz(loop), "asmjit cmpxchg8b loop");
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
    return eip;
}

U32 emitOrderingWriterLoop() {
    U32 eip = testContext().codeIp - TEST_CODE_ADDRESS;
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();

    expectAsmOk(a.mov(asmjit::x86::edx, 1), "asmjit ordering writer phase init");
    expectAsmOk(a.bind(loop), "asmjit ordering writer loop bind");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_ORDERING_DATA), asmjit::x86::edx), "asmjit ordering writer data");
    a.lock();
    expectAsmOk(a.inc(asmjit::x86::dword_ptr(LOCKED_ORDERING_FENCE)), "asmjit ordering writer fence");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_ORDERING_READY), asmjit::x86::edx), "asmjit ordering writer ready");
    emitWaitDwordEquals(a, LOCKED_ORDERING_ACK, asmjit::x86::edx, "asmjit ordering writer wait ack");
    expectAsmOk(a.inc(asmjit::x86::edx), "asmjit ordering writer phase inc");
    expectAsmOk(a.loop(loop), "asmjit ordering writer loop");
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
    return eip;
}

U32 emitPlainOrderingWriterLoop() {
    U32 eip = testContext().codeIp - TEST_CODE_ADDRESS;
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();

    expectAsmOk(a.mov(asmjit::x86::edx, 1), "asmjit plain ordering writer phase init");
    expectAsmOk(a.bind(loop), "asmjit plain ordering writer loop bind");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_ORDERING_DATA), asmjit::x86::edx), "asmjit plain ordering writer data");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_ORDERING_READY), asmjit::x86::edx), "asmjit plain ordering writer ready");
    emitWaitDwordEquals(a, LOCKED_ORDERING_ACK, asmjit::x86::edx, "asmjit plain ordering writer wait ack");
    expectAsmOk(a.inc(asmjit::x86::edx), "asmjit plain ordering writer phase inc");
    expectAsmOk(a.loop(loop), "asmjit plain ordering writer loop");
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
    return eip;
}

U32 emitOrderingReaderLoop() {
    U32 eip = testContext().codeIp - TEST_CODE_ADDRESS;
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();
    asmjit::Label ok = a.new_label();

    expectAsmOk(a.mov(asmjit::x86::edx, 1), "asmjit ordering reader phase init");
    expectAsmOk(a.bind(loop), "asmjit ordering reader loop bind");
    emitWaitDwordEquals(a, LOCKED_ORDERING_READY, asmjit::x86::edx, "asmjit ordering reader wait ready");
    expectAsmOk(a.cmp(asmjit::x86::dword_ptr(LOCKED_ORDERING_DATA), asmjit::x86::edx), "asmjit ordering reader check data");
    expectAsmOk(a.je(ok), "asmjit ordering reader branch");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_ORDERING_ERROR), asmjit::x86::edx), "asmjit ordering reader error");
    expectAsmOk(a.bind(ok), "asmjit ordering reader ok bind");
    expectAsmOk(a.mov(asmjit::x86::dword_ptr(LOCKED_ORDERING_ACK), asmjit::x86::edx), "asmjit ordering reader ack");
    expectAsmOk(a.inc(asmjit::x86::edx), "asmjit ordering reader phase inc");
    expectAsmOk(a.loop(loop), "asmjit ordering reader loop");
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
    return eip;
}

asmjit::x86::Mem csBytePtr(U32 offset) {
    asmjit::x86::Mem result = asmjit::x86::byte_ptr(offset);
    result.set_segment(asmjit::x86::cs);
    return result;
}

void emitImplicitXchgSelfModifyingCode() {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label start = a.new_label();
    asmjit::Label done = a.new_label();

    expectAsmOk(a.bind(start), "asmjit implicit xchg smc start");
    expectAsmOk(a.short_().add(asmjit::x86::eax, 0x20), "asmjit implicit xchg smc add");
    expectAsmOk(a.test(asmjit::x86::ecx, asmjit::x86::ecx), "asmjit implicit xchg smc test");
    expectAsmOk(a.short_().jnz(done), "asmjit implicit xchg smc jnz");
    expectAsmOk(a.inc(asmjit::x86::ecx), "asmjit implicit xchg smc inc");
    expectAsmOk(a.mov(asmjit::x86::al, 0x40), "asmjit implicit xchg smc al");
    expectAsmOk(a.xchg(csBytePtr(0x2), asmjit::x86::al), "asmjit implicit xchg smc xchg");
    expectAsmOk(a.short_().jmp(start), "asmjit implicit xchg smc jmp");
    expectAsmOk(a.bind(done), "asmjit implicit xchg smc done");
    pushGeneratedCode(code);
}

KThread* startGuestThreadAt(U32 eip, U32 iterations) {
    KThread* thread = testContext().process->createThread();
    thread->cpu->clone(testContext().cpu);
    thread->cpu->eip.u32 = eip;
    thread->cpu->reg[R_CX].u32 = iterations;
    scheduleThread(thread);
    return thread;
}

template <typename EmitLockedLoop>
void runPlainStoreRaceCase(const char* name, U32 target, int width, U32 sentinel, EmitLockedLoop emitLockedLoop) {
    newInstruction(0);
    CPU* baseCpu = testContext().cpu;
    KMemory* baseMemory = testContext().memory;

    writeTargetValue(baseMemory, baseCpu->seg[DS].address + target, width, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_PHASE, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_WRITER_DONE, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_ACK, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_ERROR, 0);

    U32 writerEip = emitPlainStoreWriterLoop(target, width, sentinel);
    U32 lockedEip = emitLockedLoop();

    KThread* writer = startGuestThreadAt(writerEip, LOCKED_PLAIN_STORE_RACE_ITERATIONS);
    KThread* locked = startGuestThreadAt(lockedEip, LOCKED_PLAIN_STORE_RACE_ITERATIONS);

    joinThread(writer);
    joinThread(locked);
    KThread::setCurrentThread(testContext().thread);

    U32 errorPhase = baseMemory->readd(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_ERROR);
    if (errorPhase) {
        failed("%s invalid locked/plain-store ordering observed at phase %u", name, errorPhase);
    }
}

void runLockedCmpXchgAgainstPlainStoreCase(const char* name) {
    runPlainStoreRaceCase(name, LOCKED_PLAIN_STORE_TARGET, 32, LOCKED_PLAIN_STORE_SENTINEL32, []() {
        return emitLockedCmpXchgAgainstPlainStoreLoop(LOCKED_PLAIN_STORE_TARGET, 32);
    });
}

void runImplicitXchgAgainstPlainStoreCase(const char* name) {
    runPlainStoreRaceCase(name, LOCKED_PLAIN_STORE_TARGET, 32, LOCKED_PLAIN_STORE_SENTINEL32, []() {
        return emitImplicitXchgAgainstPlainStoreLoop(LOCKED_PLAIN_STORE_TARGET, 32);
    });
}

void runLockedCmpXchgFailureAgainstPlainStoreCase(const char* name) {
    runPlainStoreRaceCase(name, LOCKED_PLAIN_STORE_TARGET, 32, LOCKED_PLAIN_STORE_SENTINEL32, []() {
        return emitLockedCmpXchgFailureAgainstPlainStoreLoop(LOCKED_PLAIN_STORE_TARGET, 32, LOCKED_PLAIN_STORE_SENTINEL32);
    });
}

void runLockedXaddAgainstPlainStoreCase(const char* name, U32 target, int width) {
    U32 sentinel = sentinelForWidth(width);
    runPlainStoreRaceCase(name, target, width, sentinel, [target, width]() {
        return emitLockedXaddAgainstPlainStoreLoop(target, width);
    });
}

void runPlainReadAgainstLockedWriteCase(const char* name) {
    newInstruction(0);
    CPU* baseCpu = testContext().cpu;
    KMemory* baseMemory = testContext().memory;

    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_TARGET, LOCKED_READ_VALUE_A);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_PHASE, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_WRITER_DONE, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_ACK, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_ERROR, 0);

    U32 writerEip = emitLockedWriteAgainstPlainReadWriterLoop(LOCKED_PLAIN_STORE_TARGET);
    U32 readerEip = emitPlainReadAgainstLockedWriteReaderLoop(LOCKED_PLAIN_STORE_TARGET);

    KThread* writer = startGuestThreadAt(writerEip, LOCKED_PLAIN_STORE_RACE_ITERATIONS);
    KThread* reader = startGuestThreadAt(readerEip, LOCKED_PLAIN_STORE_RACE_ITERATIONS);

    joinThread(writer);
    joinThread(reader);
    KThread::setCurrentThread(testContext().thread);

    U32 errorPhase = baseMemory->readd(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_ERROR);
    if (errorPhase) {
        failed("%s plain read observed torn locked write at phase %u", name, errorPhase);
    }
}

enum class LockedFamilyOp {
    Add,
    Sub,
    Adc,
    Sbb,
    And,
    Or,
    Xor,
    Inc,
    Dec,
    Neg,
    Not,
    Bts,
    Btr,
    Btc
};

U32 lockedFamilyOperand(int width) {
    if (width == 8) {
        return 0x13;
    }
    if (width == 16) {
        return 0x1357;
    }
    return 0x13572468;
}

U32 lockedFamilyInitial(LockedFamilyOp op, int width) {
    if (op == LockedFamilyOp::Bts || op == LockedFamilyOp::Btc) {
        return 0x20 & widthMask(width);
    }
    if (op == LockedFamilyOp::Btr) {
        return 0x28 & widthMask(width);
    }
    if (width == 8) {
        return 0x59;
    }
    if (width == 16) {
        return 0x59a6;
    }
    return 0x59a6c33c;
}

U32 lockedFamilyExpected(LockedFamilyOp op, int width, U32 initial) {
    U32 mask = widthMask(width);
    U32 operand = lockedFamilyOperand(width) & mask;
    switch (op) {
    case LockedFamilyOp::Add:
        return (initial + operand) & mask;
    case LockedFamilyOp::Sub:
        return (initial - operand) & mask;
    case LockedFamilyOp::Adc:
        return (initial + operand + 1) & mask;
    case LockedFamilyOp::Sbb:
        return (initial - operand - 1) & mask;
    case LockedFamilyOp::And:
        return initial & operand;
    case LockedFamilyOp::Or:
        return (initial | operand) & mask;
    case LockedFamilyOp::Xor:
        return (initial ^ operand) & mask;
    case LockedFamilyOp::Inc:
        return (initial + 1) & mask;
    case LockedFamilyOp::Dec:
        return (initial - 1) & mask;
    case LockedFamilyOp::Neg:
        return (0 - initial) & mask;
    case LockedFamilyOp::Not:
        return (~initial) & mask;
    case LockedFamilyOp::Bts:
        return (initial | 0x08) & mask;
    case LockedFamilyOp::Btr:
        return (initial & ~0x08) & mask;
    case LockedFamilyOp::Btc:
        return (initial ^ 0x08) & mask;
    }
    failed("unsupported locked family op");
    return 0;
}

void emitLockedFamilyInstruction(asmjit::x86::Assembler& a, LockedFamilyOp op, U32 target, int width) {
    asmjit::x86::Mem mem = targetMem(target, width);
    switch (op) {
    case LockedFamilyOp::Add:
        expectAsmOk(a.mov(asmjit::x86::ebx, lockedFamilyOperand(width)), "asmjit locked family add ebx");
        a.lock();
        expectAsmOk(a.add(mem, bxForWidth(width)), "asmjit locked family add");
        break;
    case LockedFamilyOp::Sub:
        expectAsmOk(a.mov(asmjit::x86::ebx, lockedFamilyOperand(width)), "asmjit locked family sub ebx");
        a.lock();
        expectAsmOk(a.sub(mem, bxForWidth(width)), "asmjit locked family sub");
        break;
    case LockedFamilyOp::Adc:
        expectAsmOk(a.mov(asmjit::x86::ebx, lockedFamilyOperand(width)), "asmjit locked family adc ebx");
        expectAsmOk(a.stc(), "asmjit locked family adc stc");
        a.lock();
        expectAsmOk(a.adc(mem, bxForWidth(width)), "asmjit locked family adc");
        break;
    case LockedFamilyOp::Sbb:
        expectAsmOk(a.mov(asmjit::x86::ebx, lockedFamilyOperand(width)), "asmjit locked family sbb ebx");
        expectAsmOk(a.stc(), "asmjit locked family sbb stc");
        a.lock();
        expectAsmOk(a.sbb(mem, bxForWidth(width)), "asmjit locked family sbb");
        break;
    case LockedFamilyOp::And:
        expectAsmOk(a.mov(asmjit::x86::ebx, lockedFamilyOperand(width)), "asmjit locked family and ebx");
        a.lock();
        expectAsmOk(a.and_(mem, bxForWidth(width)), "asmjit locked family and");
        break;
    case LockedFamilyOp::Or:
        expectAsmOk(a.mov(asmjit::x86::ebx, lockedFamilyOperand(width)), "asmjit locked family or ebx");
        a.lock();
        expectAsmOk(a.or_(mem, bxForWidth(width)), "asmjit locked family or");
        break;
    case LockedFamilyOp::Xor:
        expectAsmOk(a.mov(asmjit::x86::ebx, lockedFamilyOperand(width)), "asmjit locked family xor ebx");
        a.lock();
        expectAsmOk(a.xor_(mem, bxForWidth(width)), "asmjit locked family xor");
        break;
    case LockedFamilyOp::Inc:
        a.lock();
        expectAsmOk(a.inc(mem), "asmjit locked family inc");
        break;
    case LockedFamilyOp::Dec:
        a.lock();
        expectAsmOk(a.dec(mem), "asmjit locked family dec");
        break;
    case LockedFamilyOp::Neg:
        a.lock();
        expectAsmOk(a.neg(mem), "asmjit locked family neg");
        break;
    case LockedFamilyOp::Not:
        a.lock();
        expectAsmOk(a.not_(mem), "asmjit locked family not");
        break;
    case LockedFamilyOp::Bts:
        a.lock();
        expectAsmOk(a.bts(mem, 3), "asmjit locked family bts");
        break;
    case LockedFamilyOp::Btr:
        a.lock();
        expectAsmOk(a.btr(mem, 3), "asmjit locked family btr");
        break;
    case LockedFamilyOp::Btc:
        a.lock();
        expectAsmOk(a.btc(mem, 3), "asmjit locked family btc");
        break;
    }
}

void runLockedFamilyCase(const char* name, LockedFamilyOp op, int width) {
    newInstruction(0);
    CPU* baseCpu = testContext().cpu;
    KMemory* baseMemory = testContext().memory;
    U32 initial = lockedFamilyInitial(op, width);
    U32 expected = lockedFamilyExpected(op, width, initial);

    writeTargetValue(baseMemory, baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_TARGET, width, initial);

    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    emitLockedFamilyInstruction(a, op, LOCKED_PLAIN_STORE_TARGET, width);
    pushGeneratedCode(code);
    runTestCPU();

    U32 actual = readTargetValue(baseMemory, baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_TARGET, width) & widthMask(width);
    if (actual != expected) {
        failed("%s m%d expected %.8X got %.8X", name, width, expected, actual);
    }
}

void runLockedFamilyCoverageCase() {
    struct LockedFamilyCase {
        const char* name;
        LockedFamilyOp op;
        bool include8;
    };
    const LockedFamilyCase cases[] = {
        {"locked add family", LockedFamilyOp::Add, true},
        {"locked sub family", LockedFamilyOp::Sub, true},
        {"locked adc family", LockedFamilyOp::Adc, true},
        {"locked sbb family", LockedFamilyOp::Sbb, true},
        {"locked and family", LockedFamilyOp::And, true},
        {"locked or family", LockedFamilyOp::Or, true},
        {"locked xor family", LockedFamilyOp::Xor, true},
        {"locked inc family", LockedFamilyOp::Inc, true},
        {"locked dec family", LockedFamilyOp::Dec, true},
        {"locked neg family", LockedFamilyOp::Neg, true},
        {"locked not family", LockedFamilyOp::Not, true},
        {"locked bts family", LockedFamilyOp::Bts, false},
        {"locked btr family", LockedFamilyOp::Btr, false},
        {"locked btc family", LockedFamilyOp::Btc, false}
    };

    for (const LockedFamilyCase& testCase : cases) {
        if (testCase.include8) {
            runLockedFamilyCase(testCase.name, testCase.op, 8);
        }
        runLockedFamilyCase(testCase.name, testCase.op, 16);
        runLockedFamilyCase(testCase.name, testCase.op, 32);
    }
}

void runLockedCmpXchg8bAgainstPlainStoreCase(const char* name) {
    newInstruction(0);
    CPU* baseCpu = testContext().cpu;
    KMemory* baseMemory = testContext().memory;

    baseMemory->writeq(baseCpu->seg[DS].address + LOCKED_CMPXCHG8B_TARGET, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_PHASE, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_WRITER_DONE, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_ACK, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_ERROR, 0);

    U32 writerEip = emitPlainStore64WriterLoop(LOCKED_CMPXCHG8B_TARGET);
    U32 lockedEip = emitLockedCmpXchg8bAgainstPlainStoreLoop(LOCKED_CMPXCHG8B_TARGET);

    KThread* writer = startGuestThreadAt(writerEip, LOCKED_PLAIN_STORE_RACE_ITERATIONS);
    KThread* locked = startGuestThreadAt(lockedEip, LOCKED_PLAIN_STORE_RACE_ITERATIONS);

    joinThread(writer);
    joinThread(locked);
    KThread::setCurrentThread(testContext().thread);

    U32 errorPhase = baseMemory->readd(baseCpu->seg[DS].address + LOCKED_PLAIN_STORE_ERROR);
    if (errorPhase) {
        failed("%s invalid cmpxchg8b/plain-store ordering observed at phase %u", name, errorPhase);
    }
}

void runLockedPageBoundaryNoPartialWriteCase(const char* name) {
    newInstruction(0);
    CPU* baseCpu = testContext().cpu;
    KMemory* baseMemory = testContext().memory;
    U32 linearTarget = baseCpu->seg[DS].address + K_PAGE_SIZE * 2 - 2;
    U32 secondPage = (linearTarget + 2) & ~K_PAGE_MASK;
    constexpr U32 initial = 0x11223344;
    constexpr U32 addend = 0x01020304;

    baseMemory->writed(linearTarget, initial);
    U32 protectResult = baseMemory->mprotect(testContext().thread, secondPage, K_PAGE_SIZE, K_PROT_READ);
    if (protectResult) {
        failed("%s protect setup", name);
        return;
    }

    KSigAction oldSegv = testContext().process->sigActions[K_SIGSEGV];
    testContext().process->sigActions[K_SIGSEGV].handlerAndSigAction = TEST_CODE_ADDRESS + 0x300;
    baseCpu->reg[R_BX].u32 = addend;

    bool faulted = false;
    try {
        common_xaddr32e32_lock(baseCpu, linearTarget, R_BX);
    } catch (...) {
        faulted = true;
    }

    testContext().process->sigActions[K_SIGSEGV] = oldSegv;
    baseMemory->mprotect(testContext().thread, secondPage, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);

    if (!faulted) {
        failed("%s expected write fault", name);
    }
    U32 actual = baseMemory->readd(linearTarget);
    if (actual != initial) {
        failed("%s partial write expected %.8X got %.8X", name, initial, actual);
    }
}

void runLockedXaddSingleThreadCase(const char* name, U32 target, int width) {
    newInstruction(0);
    CPU* baseCpu = testContext().cpu;
    KMemory* baseMemory = testContext().memory;
    constexpr U32 initialValue = 5;
    U32 mask = widthMask(width);

    writeTargetValue(baseMemory, baseCpu->seg[DS].address + target, width, initialValue);

    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    expectAsmOk(a.mov(asmjit::x86::ebx, LOCKED_XADD_ADDEND), "asmjit locked xadd single ebx");
    a.lock();
    expectAsmOk(a.xadd(targetMem(target, width), bxForWidth(width)), "asmjit locked xadd single target");
    pushGeneratedCode(code);
    runTestCPU();

    U32 actualMemory = readTargetValue(baseMemory, baseCpu->seg[DS].address + target, width) & mask;
    U32 actualRegister = baseCpu->reg[R_BX].u32 & mask;
    if (actualMemory != ((initialValue + LOCKED_XADD_ADDEND) & mask)) {
        failed("%s memory", name);
    }
    if (actualRegister != initialValue) {
        failed("%s register", name);
    }
}

void runLockedOrderingCase(const char* name) {
    newInstruction(0);
    CPU* baseCpu = testContext().cpu;
    KMemory* baseMemory = testContext().memory;

    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_ORDERING_DATA, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_ORDERING_FENCE, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_ORDERING_READY, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_ORDERING_ACK, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_ORDERING_ERROR, 0);

    U32 writerEip = emitOrderingWriterLoop();
    U32 readerEip = emitOrderingReaderLoop();

    KThread* writer = startGuestThreadAt(writerEip, LOCKED_ORDERING_ITERATIONS);
    KThread* reader = startGuestThreadAt(readerEip, LOCKED_ORDERING_ITERATIONS);

    joinThread(writer);
    joinThread(reader);
    KThread::setCurrentThread(testContext().thread);

    U32 errorPhase = baseMemory->readd(baseCpu->seg[DS].address + LOCKED_ORDERING_ERROR);
    if (errorPhase) {
        failed("%s stale data observed after locked ordering point at phase %u", name, errorPhase);
    }
}

void runPlainOrderingCase(const char* name) {
    newInstruction(0);
    CPU* baseCpu = testContext().cpu;
    KMemory* baseMemory = testContext().memory;

    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_ORDERING_DATA, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_ORDERING_READY, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_ORDERING_ACK, 0);
    baseMemory->writed(baseCpu->seg[DS].address + LOCKED_ORDERING_ERROR, 0);

    U32 writerEip = emitPlainOrderingWriterLoop();
    U32 readerEip = emitOrderingReaderLoop();

    KThread* writer = startGuestThreadAt(writerEip, LOCKED_ORDERING_ITERATIONS);
    KThread* reader = startGuestThreadAt(readerEip, LOCKED_ORDERING_ITERATIONS);

    joinThread(writer);
    joinThread(reader);
    KThread::setCurrentThread(testContext().thread);

    U32 errorPhase = baseMemory->readd(baseCpu->seg[DS].address + LOCKED_ORDERING_ERROR);
    if (errorPhase) {
        failed("%s stale data observed after plain ready store at phase %u", name, errorPhase);
    }
}

void runImplicitXchgSelfModifyingCodeCase(const char* name) {
    newInstruction(0);
    emitImplicitXchgSelfModifyingCode();

    runTestCPU();
    if (testContext().cpu->reg[R_CX].u32 != 1) {
        failed("%s ecx", name);
    }
    if (testContext().cpu->reg[R_AX].u32 != 0x60) {
        failed("%s eax", name);
    }
}
#define cpu (testContext().cpu)
#define memory (testContext().memory)
#endif

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

#ifdef BOXEDWINE_MULTI_THREADED
void testLockedCmpXchgAgainstPlainStore() { runLockedCmpXchgAgainstPlainStoreCase("locked cmpxchg against plain store"); }
void testImplicitLockedXchgAgainstPlainStore() { runImplicitXchgAgainstPlainStoreCase("implicit locked xchg against plain store"); }
void testLockedCmpXchgFailureAgainstPlainStore() { runLockedCmpXchgFailureAgainstPlainStoreCase("locked cmpxchg failure against plain store"); }
void testLockedXaddAgainstPlainStore() { runLockedXaddAgainstPlainStoreCase("locked xadd against plain store", LOCKED_PLAIN_STORE_TARGET, 32); }
void testPlainReadAgainstLockedWrite() { runPlainReadAgainstLockedWriteCase("plain read against locked write"); }
void testLockedAdditionalFamilies() { runLockedFamilyCoverageCase(); }
void testLockedCmpXchg8bAgainstPlainStore() { runLockedCmpXchg8bAgainstPlainStoreCase("locked cmpxchg8b against plain store"); }
void testLockedPageBoundaryNoPartialWrite() { runLockedPageBoundaryNoPartialWriteCase("locked page-boundary no partial write"); }
void testImplicitLockedXchgSelfModifyingCode() { runImplicitXchgSelfModifyingCodeCase("implicit locked xchg self-modifying code"); }
void testPlainMemoryOrdering() { runPlainOrderingCase("plain x86 memory ordering"); }
void testLockedWidthsAndAlignmentAgainstPlainStore() {
    runLockedXaddAgainstPlainStoreCase("locked xadd m8 aligned against plain store", LOCKED_PLAIN_STORE_TARGET, 8);
    runLockedXaddAgainstPlainStoreCase("locked xadd m16 aligned against plain store", LOCKED_PLAIN_STORE_TARGET, 16);
    runLockedXaddAgainstPlainStoreCase("locked xadd m32 aligned against plain store", LOCKED_PLAIN_STORE_TARGET, 32);
    runLockedXaddSingleThreadCase("locked xadd m16 unaligned", LOCKED_PLAIN_STORE_UNALIGNED_TARGET, 16);
    runLockedXaddSingleThreadCase("locked xadd m32 unaligned", LOCKED_PLAIN_STORE_UNALIGNED_TARGET, 32);
}
void testLockedMemoryOrdering() { runLockedOrderingCase("locked operation memory ordering"); }
#endif

#endif
