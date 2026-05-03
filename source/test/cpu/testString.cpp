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

#include "testString.h"
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
constexpr U32 SRC_BASE = 0x1200;
constexpr U32 DST_BASE = 0x2200;
constexpr U32 OVERLAP_BASE = 0x3200;
constexpr U32 PAGE_SRC_BASE = 0x3fff;
constexpr U32 PAGE_DST_BASE = 0x5fff;
constexpr size_t OVERLAP_SIZE = 96;
constexpr size_t PAGE_TEST_SIZE = 32;
constexpr U8 PREFIX_REPNE = 0xf2;
constexpr U8 PREFIX_REPE = 0xf3;
constexpr U8 GUARD_BEFORE = 0x5a;
constexpr U8 GUARD_AFTER = 0xa5;
constexpr U32 FLAG_MASK = CF | PF | AF | ZF | SF | OF | DF;
constexpr U32 ARITH_FLAG_MASK = CF | PF | AF | ZF | SF | OF;

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

enum StringOp {
    STRING_MOVS,
    STRING_CMPS,
    STRING_STOS,
    STRING_LODS,
    STRING_SCAS
};

struct StringCase {
    U8 prefix;
    U32 flags;
    U32 count;
    bool backward;
    U32 eax;
    U8 src[20];
    U8 dst[20];
    size_t dataSize;
    const char* name;
};

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit string code init failed");
    }
}

void pushGeneratedCode(const asmjit::CodeHolder& code, bool stripOperandSizePrefix) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        if (stripOperandSizePrefix && buffer.data()[i] == 0x66) {
            continue;
        }
        pushCode8(buffer.data()[i]);
    }
}

asmjit::Error emitStringInstruction(asmjit::x86::Assembler& a, StringOp op, int width) {
    if (op == STRING_MOVS) {
        if (width == 1) return a.movsb();
        if (width == 2) return a.movsw();
        return a.movsd();
    }
    if (op == STRING_CMPS) {
        if (width == 1) return a.cmpsb();
        if (width == 2) return a.cmpsw();
        return a.cmpsd();
    }
    if (op == STRING_STOS) {
        if (width == 1) return a.stosb();
        if (width == 2) return a.stosw();
        return a.stosd();
    }
    if (op == STRING_LODS) {
        if (width == 1) return a.lodsb();
        if (width == 2) return a.lodsw();
        return a.lodsd();
    }
    if (width == 1) return a.scasb();
    if (width == 2) return a.scasw();
    return a.scasd();
}

void emitCode(StringOp op, int width, U8 prefix) {
    if (prefix) {
        pushCode8(prefix);
    }
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (emitStringInstruction(a, op, width) != asmjit::Error::kOk) {
        failed("asmjit string emit failed");
    }
    pushGeneratedCode(code, width == 2);
}

void initRegisters(U32* expectedRegs, U32 eax, U32 esi, U32 edi, U32 ecx) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    cpu->reg[R_AX].u32 = eax;
    cpu->reg[R_CX].u32 = ecx;
    cpu->reg[R_SI].u32 = esi;
    cpu->reg[R_DI].u32 = edi;
    expectedRegs[R_AX] = eax;
    expectedRegs[R_CX] = ecx;
    expectedRegs[R_SI] = esi;
    expectedRegs[R_DI] = edi;
}

U32 addressMask(bool address32) {
    return address32 ? 0xffffffff : 0xffff;
}

U32 indexOffset(U32 value, bool address32) {
    return value & addressMask(address32);
}

U32 updateIndex(U32 value, S32 delta, bool address32) {
    if (address32) {
        return value + delta;
    }
    return (value & 0xffff0000) | ((value + delta) & 0xffff);
}

U32 countValue(U32 ecx, bool address32) {
    return address32 ? ecx : (ecx & 0xffff);
}

void setCountValue(U32& ecx, U32 count, bool address32) {
    if (address32) {
        ecx = count;
    } else {
        ecx = (ecx & 0xffff0000) | (count & 0xffff);
    }
}

void writeBytes(U32 base, const U8* values, size_t count) {
    memory->writeb(base - 1, GUARD_BEFORE);
    for (size_t i = 0; i < count; ++i) {
        memory->writeb(base + (U32)i, values[i]);
    }
    memory->writeb(base + (U32)count, GUARD_AFTER);
}

void verifyBytes(U32 base, const U8* expected, size_t count, const char* name) {
    if (memory->readb(base - 1) != GUARD_BEFORE || memory->readb(base + (U32)count) != GUARD_AFTER) {
        failed("%s memory guard", name);
    }
    for (size_t i = 0; i < count; ++i) {
        if (memory->readb(base + (U32)i) != expected[i]) {
            failed("%s memory byte", name);
        }
    }
}

bool evenParity(U32 value) {
    value &= 0xff;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1) == 0;
}

U32 subFlags(U32 lhs, U32 rhs, U32 result, int bits) {
    U32 mask = widthMask(bits);
    U32 sign = signBit(bits);
    lhs &= mask;
    rhs &= mask;
    result &= mask;
    U32 flags = 0;
    if (lhs < rhs) flags |= CF;
    if (((lhs ^ rhs) & (lhs ^ result) & sign) != 0) flags |= OF;
    if (((lhs ^ rhs ^ result) & 0x10) != 0) flags |= AF;
    if (result == 0) flags |= ZF;
    if ((result & sign) != 0) flags |= SF;
    if (evenParity(result)) flags |= PF;
    return flags;
}

U32 accumulatorForWidth(U32 eax, int width) {
    if (width == 1) return eax & 0xff;
    if (width == 2) return eax & 0xffff;
    return eax;
}

void setAccumulator(U32& eax, int width, U32 value) {
    if (width == 1) {
        eax = (eax & 0xffffff00) | (value & 0xff);
    } else if (width == 2) {
        eax = (eax & 0xffff0000) | (value & 0xffff);
    } else {
        eax = value;
    }
}

U32 readCaseValue(const U8* values, U32 offset, int width) {
    U32 result = values[offset];
    if (width >= 2) {
        result |= ((U32)values[offset + 1]) << 8;
    }
    if (width == 4) {
        result |= ((U32)values[offset + 2]) << 16;
        result |= ((U32)values[offset + 3]) << 24;
    }
    return result;
}

void writeCaseValue(U8* values, U32 offset, int width, U32 value) {
    values[offset] = (U8)value;
    if (width >= 2) {
        values[offset + 1] = (U8)(value >> 8);
    }
    if (width == 4) {
        values[offset + 2] = (U8)(value >> 16);
        values[offset + 3] = (U8)(value >> 24);
    }
}

U32 readMemoryValue(U32 linear, int width) {
    if (width == 1) return memory->readb(linear);
    if (width == 2) return memory->readw(linear);
    return memory->readd(linear);
}

void writeMemoryValue(U32 linear, int width, U32 value) {
    if (width == 1) {
        memory->writeb(linear, value);
    } else if (width == 2) {
        memory->writew(linear, value);
    } else {
        memory->writed(linear, value);
    }
}

void copyBytes(U8* dst, const U8* src, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

void simulate(StringOp op, int width, bool address32, U8 prefix, U32& expectedFlags, U32* expectedRegs, const U8* expectedSrc, U8* expectedDst) {
    S32 delta = (expectedFlags & DF) ? -width : width;
    bool compareOp = op == STRING_CMPS || op == STRING_SCAS;
    bool repeat = prefix == PREFIX_REPNE || prefix == PREFIX_REPE;
    U32 remaining = repeat ? countValue(expectedRegs[R_CX], address32) : 1;

    while (remaining) {
        U32 esi = expectedRegs[R_SI];
        U32 edi = expectedRegs[R_DI];
        U32 srcOffset = indexOffset(esi, address32) - SRC_BASE;
        U32 dstOffset = indexOffset(edi, address32) - DST_BASE;
        U32 srcValue = readCaseValue(expectedSrc, srcOffset, width);
        U32 dstValue = readCaseValue(expectedDst, dstOffset, width);

        if (op == STRING_MOVS) {
            writeCaseValue(expectedDst, dstOffset, width, srcValue);
        } else if (op == STRING_CMPS) {
            U32 result = (srcValue - dstValue) & widthMask(width * 8);
            expectedFlags = (expectedFlags & ~ARITH_FLAG_MASK) | subFlags(srcValue, dstValue, result, width * 8);
        } else if (op == STRING_STOS) {
            writeCaseValue(expectedDst, dstOffset, width, accumulatorForWidth(expectedRegs[R_AX], width));
        } else if (op == STRING_LODS) {
            setAccumulator(expectedRegs[R_AX], width, srcValue);
        } else {
            U32 acc = accumulatorForWidth(expectedRegs[R_AX], width);
            U32 result = (acc - dstValue) & widthMask(width * 8);
            expectedFlags = (expectedFlags & ~ARITH_FLAG_MASK) | subFlags(acc, dstValue, result, width * 8);
        }

        if (op == STRING_MOVS || op == STRING_CMPS || op == STRING_LODS) {
            expectedRegs[R_SI] = updateIndex(expectedRegs[R_SI], delta, address32);
        }
        if (op == STRING_MOVS || op == STRING_CMPS || op == STRING_STOS || op == STRING_SCAS) {
            expectedRegs[R_DI] = updateIndex(expectedRegs[R_DI], delta, address32);
        }

        if (repeat) {
            --remaining;
            setCountValue(expectedRegs[R_CX], remaining, address32);
            if (compareOp) {
                bool zf = (expectedFlags & ZF) != 0;
                if ((prefix == PREFIX_REPE && !zf) || (prefix == PREFIX_REPNE && zf)) {
                    break;
                }
            }
        } else {
            break;
        }
    }

}

void runStringCase(StringOp op, int width, bool address32, const StringCase& data) {
    newInstruction(data.flags);
    cpu->big = address32 ? 1 : 0;
    cpu->seg[ES].address = TEST_HEAP_ADDRESS;
    cpu->seg[ES].value = TEST_HEAP_SEG;
    cpu->thread->process->hasSetSeg[ES] = true;

    U32 startOffset = data.backward && data.count ? (data.count - 1) * width : 0;
    U32 esi = SRC_BASE + startOffset;
    U32 edi = DST_BASE + startOffset;
    U32 ecx = (address32 || data.prefix) ? data.count : (REG_GUARD | data.count);
    U32 expectedRegs[8];
    U8 expectedDst[20];
    U32 expectedFlags = data.flags;
    size_t dataSize = data.dataSize;
    size_t requiredSize = data.count ? data.count * width : width;
    if (requiredSize > dataSize) {
        dataSize = requiredSize;
    }
    initRegisters(expectedRegs, data.eax, esi, edi, ecx);
    writeBytes(TEST_HEAP_ADDRESS + SRC_BASE, data.src, dataSize);
    writeBytes(TEST_HEAP_ADDRESS + DST_BASE, data.dst, dataSize);
    copyBytes(expectedDst, data.dst, dataSize);

    emitCode(op, width, data.prefix);
    simulate(op, width, address32, data.prefix, expectedFlags, expectedRegs, data.src, expectedDst);
    runTestCPU();

    verifyRegisters(cpu, expectedRegs, data.name);
    verifyBytes(TEST_HEAP_ADDRESS + SRC_BASE, data.src, dataSize, data.name);
    verifyBytes(TEST_HEAP_ADDRESS + DST_BASE, expectedDst, dataSize, data.name);
    if ((actualFlags(cpu, true) & FLAG_MASK) != (expectedFlags & FLAG_MASK)) {
        if (op != STRING_CMPS && op != STRING_SCAS) {
            failed("%s flags changed", data.name);
        }
    }
    if (op == STRING_CMPS || op == STRING_SCAS) {
        if ((actualFlags(cpu, true) & FLAG_MASK) != (expectedFlags & FLAG_MASK)) {
            failed("%s flags", data.name);
        }
    }
}

void runStringCases(StringOp op, int width, bool address32, const StringCase* cases, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        runStringCase(op, width, address32, cases[i]);
    }
}

void initOverlapBytes(U8* values, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        values[i] = (U8)(0x40 + ((i * 17 + 3) & 0x3f));
    }
}

void writeOverlapBytes(const U8* values, size_t count) {
    memory->writeb(TEST_HEAP_ADDRESS + OVERLAP_BASE - 1, GUARD_BEFORE);
    for (size_t i = 0; i < count; ++i) {
        memory->writeb(TEST_HEAP_ADDRESS + OVERLAP_BASE + (U32)i, values[i]);
    }
    memory->writeb(TEST_HEAP_ADDRESS + OVERLAP_BASE + (U32)count, GUARD_AFTER);
}

void verifyOverlapBytes(const U8* expected, size_t count, const char* name) {
    if (memory->readb(TEST_HEAP_ADDRESS + OVERLAP_BASE - 1) != GUARD_BEFORE ||
            memory->readb(TEST_HEAP_ADDRESS + OVERLAP_BASE + (U32)count) != GUARD_AFTER) {
        failed("%s overlap guard", name);
    }
    for (size_t i = 0; i < count; ++i) {
        if (memory->readb(TEST_HEAP_ADDRESS + OVERLAP_BASE + (U32)i) != expected[i]) {
            failed("%s overlap byte", name);
        }
    }
}

void simulateOverlapMovs(U8* expected, int width, bool address32, U32 count, bool backward, U32& esi, U32& edi, U32& ecx) {
    S32 delta = backward ? -width : width;
    U32 remaining = countValue(ecx, address32);
    while (remaining) {
        U32 srcOffset = indexOffset(esi, address32) - OVERLAP_BASE;
        U32 dstOffset = indexOffset(edi, address32) - OVERLAP_BASE;
        U32 value = readCaseValue(expected, srcOffset, width);
        writeCaseValue(expected, dstOffset, width, value);
        esi = updateIndex(esi, delta, address32);
        edi = updateIndex(edi, delta, address32);
        --remaining;
        setCountValue(ecx, remaining, address32);
    }
}

void runOverlapMovsCase(int width, bool address32, U32 count, bool backward, const char* name) {
    newInstruction(backward ? DF : 0);
    cpu->big = address32 ? 1 : 0;
    cpu->seg[ES].address = TEST_HEAP_ADDRESS;
    cpu->seg[ES].value = TEST_HEAP_SEG;
    cpu->thread->process->hasSetSeg[ES] = true;

    U32 overlapDelta = width;
    U32 srcStart = OVERLAP_BASE + (backward ? overlapDelta + (count - 1) * width : 0);
    U32 dstStart = OVERLAP_BASE + (backward ? (count - 1) * width : overlapDelta);
    U32 ecx = count;
    size_t dataSize = count * width + overlapDelta;
    if (dataSize > OVERLAP_SIZE) {
        failed("%s overlap data size", name);
        return;
    }

    U8 initial[OVERLAP_SIZE];
    U8 expected[OVERLAP_SIZE];
    initOverlapBytes(initial, dataSize);
    copyBytes(expected, initial, dataSize);
    writeOverlapBytes(initial, dataSize);

    U32 expectedRegs[8];
    initRegisters(expectedRegs, 0x89abcdef, srcStart, dstStart, ecx);
    simulateOverlapMovs(expected, width, address32, count, backward, expectedRegs[R_SI], expectedRegs[R_DI], expectedRegs[R_CX]);

    emitCode(STRING_MOVS, width, PREFIX_REPE);
    runTestCPU();

    verifyRegisters(cpu, expectedRegs, name);
    verifyOverlapBytes(expected, dataSize, name);
    if ((actualFlags(cpu, true) & FLAG_MASK) != (backward ? DF : 0)) {
        failed("%s overlap flags", name);
    }
}

void runOverlapMovsCases(int width, bool address32) {
    static const U32 counts[] = {15, 16, 17};
    for (size_t i = 0; i < sizeof(counts) / sizeof(counts[0]); ++i) {
        runOverlapMovsCase(width, address32, counts[i], false, "string move overlap forward");
        runOverlapMovsCase(width, address32, counts[i], true, "string move overlap backward");
    }
}

void initPageBytes(U8* values, size_t count, U8 seed) {
    for (size_t i = 0; i < count; ++i) {
        values[i] = (U8)(seed + ((i * 13 + 7) & 0x7f));
    }
}

void writePageBytes(U32 offset, const U8* values, size_t count) {
    U32 base = TEST_HEAP_ADDRESS + offset;
    memory->writeb(base - 1, GUARD_BEFORE);
    for (size_t i = 0; i < count; ++i) {
        memory->writeb(base + (U32)i, values[i]);
    }
    memory->writeb(base + (U32)count, GUARD_AFTER);
}

void verifyPageBytes(U32 offset, const U8* expected, size_t count, const char* name) {
    U32 base = TEST_HEAP_ADDRESS + offset;
    if (memory->readb(base - 1) != GUARD_BEFORE || memory->readb(base + (U32)count) != GUARD_AFTER) {
        failed("%s page guard", name);
    }
    for (size_t i = 0; i < count; ++i) {
        if (memory->readb(base + (U32)i) != expected[i]) {
            failed("%s page byte", name);
        }
    }
}

U32 pageLinear(U32 value, bool address32) {
    return TEST_HEAP_ADDRESS + indexOffset(value, address32);
}

void setPageCompareData(StringOp op, int width, U8 prefix, U32 eax, U8* src, U8* dst, U32 count) {
    for (U32 i = 0; i < count; ++i) {
        U32 offset = i * width;
        if (op == STRING_SCAS) {
            writeCaseValue(dst, offset, width, accumulatorForWidth(eax, width));
            if (prefix == PREFIX_REPNE || prefix == 0) {
                dst[offset] = (U8)(dst[offset] + 1);
            }
        } else {
            for (int j = 0; j < width; ++j) {
                dst[offset + j] = src[offset + j];
            }
            if (prefix == PREFIX_REPNE || prefix == 0) {
                dst[offset] = (U8)(dst[offset] + 1);
            }
        }
    }
}

void simulatePageBoundary(StringOp op, int width, bool address32, U8 prefix, U32& expectedFlags, U32* expectedRegs) {
    bool compareOp = op == STRING_CMPS || op == STRING_SCAS;
    bool repeat = prefix == PREFIX_REPNE || prefix == PREFIX_REPE;
    U32 remaining = repeat ? countValue(expectedRegs[R_CX], address32) : 1;

    while (remaining) {
        U32 srcValue = readMemoryValue(pageLinear(expectedRegs[R_SI], address32), width);
        U32 dstValue = readMemoryValue(pageLinear(expectedRegs[R_DI], address32), width);
        if (op == STRING_MOVS) {
            writeMemoryValue(pageLinear(expectedRegs[R_DI], address32), width, srcValue);
        } else if (op == STRING_CMPS) {
            U32 result = (srcValue - dstValue) & widthMask(width * 8);
            expectedFlags = (expectedFlags & ~ARITH_FLAG_MASK) | subFlags(srcValue, dstValue, result, width * 8);
        } else if (op == STRING_STOS) {
            writeMemoryValue(pageLinear(expectedRegs[R_DI], address32), width, accumulatorForWidth(expectedRegs[R_AX], width));
        } else if (op == STRING_LODS) {
            setAccumulator(expectedRegs[R_AX], width, srcValue);
        } else {
            U32 acc = accumulatorForWidth(expectedRegs[R_AX], width);
            U32 result = (acc - dstValue) & widthMask(width * 8);
            expectedFlags = (expectedFlags & ~ARITH_FLAG_MASK) | subFlags(acc, dstValue, result, width * 8);
        }

        if (op == STRING_MOVS || op == STRING_CMPS || op == STRING_LODS) {
            expectedRegs[R_SI] = updateIndex(expectedRegs[R_SI], width, address32);
        }
        if (op == STRING_MOVS || op == STRING_CMPS || op == STRING_STOS || op == STRING_SCAS) {
            expectedRegs[R_DI] = updateIndex(expectedRegs[R_DI], width, address32);
        }

        if (repeat) {
            --remaining;
            setCountValue(expectedRegs[R_CX], remaining, address32);
            if (compareOp) {
                bool zf = (expectedFlags & ZF) != 0;
                if ((prefix == PREFIX_REPE && !zf) || (prefix == PREFIX_REPNE && zf)) {
                    break;
                }
            }
        } else {
            break;
        }
    }
}

void runPageBoundaryCase(StringOp op, int width, bool address32, U8 prefix, const char* name) {
    constexpr U32 initialFlags = CF | PF | AF | ZF | SF | OF;
    newInstruction(initialFlags);
    cpu->big = address32 ? 1 : 0;
    cpu->seg[ES].address = TEST_HEAP_ADDRESS;
    cpu->seg[ES].value = TEST_HEAP_SEG;
    cpu->thread->process->hasSetSeg[ES] = true;

    U32 count = prefix ? 3 : 1;
    size_t dataSize = count * width;
    U32 eax = 0x6d5c3b2a;
    U8 src[PAGE_TEST_SIZE];
    U8 dst[PAGE_TEST_SIZE];
    U8 expectedSrc[PAGE_TEST_SIZE];
    U8 expectedDst[PAGE_TEST_SIZE];
    initPageBytes(src, dataSize, 0x10);
    initPageBytes(dst, dataSize, 0x70);
    if (op == STRING_CMPS || op == STRING_SCAS) {
        setPageCompareData(op, width, prefix, eax, src, dst, count);
    }
    copyBytes(expectedSrc, src, dataSize);
    copyBytes(expectedDst, dst, dataSize);

    writePageBytes(PAGE_SRC_BASE, src, dataSize);
    writePageBytes(PAGE_DST_BASE, dst, dataSize);

    U32 expectedRegs[8];
    initRegisters(expectedRegs, eax, PAGE_SRC_BASE, PAGE_DST_BASE, prefix ? count : REG_GUARD);
    U32 expectedFlags = initialFlags;
    emitCode(op, width, prefix);
    simulatePageBoundary(op, width, address32, prefix, expectedFlags, expectedRegs);
    for (size_t i = 0; i < dataSize; ++i) {
        expectedDst[i] = memory->readb(TEST_HEAP_ADDRESS + PAGE_DST_BASE + (U32)i);
    }
    for (size_t i = 0; i < dataSize; ++i) {
        memory->writeb(TEST_HEAP_ADDRESS + PAGE_SRC_BASE + (U32)i, src[i]);
        memory->writeb(TEST_HEAP_ADDRESS + PAGE_DST_BASE + (U32)i, dst[i]);
    }

    runTestCPU();

    verifyRegisters(cpu, expectedRegs, name);
    verifyPageBytes(PAGE_SRC_BASE, expectedSrc, dataSize, name);
    verifyPageBytes(PAGE_DST_BASE, expectedDst, dataSize, name);
    if ((actualFlags(cpu, true) & FLAG_MASK) != (expectedFlags & FLAG_MASK)) {
        failed("%s page flags", name);
    }
}

void runPageBoundaryCases(StringOp op, int width, bool address32) {
    static const U8 prefixes[] = {0, PREFIX_REPE, PREFIX_REPNE};
    for (size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
        runPageBoundaryCase(op, width, address32, prefixes[i], "string page boundary");
    }
}

const StringCase MOVE_CASES[] = {
    {0, 0, 1, false, 0x89abcdef, {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 12, "string move once"},
    {0, DF, 1, true, 0x89abcdef, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 12, "string move backward"},
    {PREFIX_REPE, CF | OF, 4, false, 0x89abcdef, {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0, 0, 0, 0, 0, 0, 0, 0}, {0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0, 0, 0, 0, 0, 0, 0, 0}, 16, "string move rep"},
    {PREFIX_REPNE, DF | SF, 0, false, 0x89abcdef, {0x71, 0x72, 0x73, 0x74, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0xd1, 0xd2, 0xd3, 0xd4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 8, "string move zero count"}
};

const StringCase COMPARE_CASES[] = {
    {0, 0, 1, false, 0x89abcdef, {0x20, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x10, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 8, "string compare greater"},
    {0, DF, 1, true, 0x89abcdef, {0x10, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x20, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 8, "string compare less backward"},
    {PREFIX_REPE, OF, 4, false, 0x89abcdef, {0x31, 0x00, 0x32, 0x00, 0x33, 0x00, 0x34, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x31, 0x00, 0x32, 0x00, 0x30, 0x00, 0x34, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 12, "string compare repe stops"},
    {PREFIX_REPNE, CF, 4, false, 0x89abcdef, {0x01, 0x00, 0x02, 0x00, 0x44, 0x00, 0x05, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x10, 0x00, 0x20, 0x00, 0x44, 0x00, 0x50, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 12, "string compare repne stops"},
    {PREFIX_REPE, CF | PF | DF, 0, false, 0x89abcdef, {0x01, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x03, 0x04, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 8, "string compare zero count"}
};

const StringCase STORE_CASES[] = {
    {0, 0, 1, false, 0x12345678, {0x10, 0x20, 0x30, 0x40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 12, "string store once"},
    {0, DF, 1, true, 0x87654321, {0x11, 0x22, 0x33, 0x44, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 12, "string store backward"},
    {PREFIX_REPE, SF | OF, 4, false, 0xa1b2c3d4, {0x01, 0x02, 0x03, 0x04, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0, 0, 0, 0, 0, 0, 0, 0}, 16, "string store rep"},
    {PREFIX_REPNE, DF | AF, 0, false, 0xaabbccdd, {0x01, 0x02, 0x03, 0x04, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x90, 0x91, 0x92, 0x93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 8, "string store zero count"}
};

const StringCase LOAD_CASES[] = {
    {0, 0, 1, false, 0x12345678, {0xaa, 0xbb, 0xcc, 0xdd, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x10, 0x11, 0x12, 0x13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 12, "string load once"},
    {0, DF, 1, true, 0x87654321, {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x20, 0x21, 0x22, 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 12, "string load backward"},
    {PREFIX_REPE, PF | AF, 4, false, 0xaabbccdd, {0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0, 0, 0, 0, 0, 0, 0, 0}, {0x30, 0x31, 0x32, 0x33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 16, "string load rep"},
    {PREFIX_REPNE, CF | OF | DF, 0, false, 0x55667788, {0x51, 0x52, 0x53, 0x54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x60, 0x61, 0x62, 0x63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 8, "string load zero count"}
};

const StringCase SCAN_CASES[] = {
    {0, 0, 1, false, 0x00000030, {0x20, 0x21, 0x22, 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x20, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 8, "string scan greater"},
    {0, DF, 1, true, 0x00000010, {0x10, 0x11, 0x12, 0x13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x20, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 8, "string scan less backward"},
    {PREFIX_REPE, OF, 4, false, 0x00000044, {0x01, 0x02, 0x03, 0x04, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x44, 0x00, 0x44, 0x00, 0x45, 0x00, 0x44, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 12, "string scan repe stops"},
    {PREFIX_REPNE, CF, 4, false, 0x00000055, {0x01, 0x02, 0x03, 0x04, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x10, 0x00, 0x20, 0x00, 0x55, 0x00, 0x60, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 12, "string scan repne stops"},
    {PREFIX_REPE, CF | PF | DF, 0, false, 0x00000011, {0x01, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0x03, 0x04, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 8, "string scan zero count"}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

} // namespace

void testMovsb_0x0a4() {
    runStringCases(STRING_MOVS, 1, false, MOVE_CASES, caseCount(MOVE_CASES));
    runOverlapMovsCases(1, false);
    runPageBoundaryCases(STRING_MOVS, 1, false);
}

void testMovsb_0x2a4() {
    runStringCases(STRING_MOVS, 1, true, MOVE_CASES, caseCount(MOVE_CASES));
    runOverlapMovsCases(1, true);
    runPageBoundaryCases(STRING_MOVS, 1, true);
}

void testMovsw_0x0a5() {
    runStringCases(STRING_MOVS, 2, false, MOVE_CASES, caseCount(MOVE_CASES));
    runOverlapMovsCases(2, false);
    runPageBoundaryCases(STRING_MOVS, 2, false);
}

void testMovsd_0x2a5() {
    runStringCases(STRING_MOVS, 4, true, MOVE_CASES, caseCount(MOVE_CASES));
    runOverlapMovsCases(4, true);
    runPageBoundaryCases(STRING_MOVS, 4, true);
}

void testCmpsb_0x0a6() {
    runStringCases(STRING_CMPS, 1, false, COMPARE_CASES, caseCount(COMPARE_CASES));
    runPageBoundaryCases(STRING_CMPS, 1, false);
}

void testCmpsb_0x2a6() {
    runStringCases(STRING_CMPS, 1, true, COMPARE_CASES, caseCount(COMPARE_CASES));
    runPageBoundaryCases(STRING_CMPS, 1, true);
}

void testCmpsw_0x0a7() {
    runStringCases(STRING_CMPS, 2, false, COMPARE_CASES, caseCount(COMPARE_CASES));
    runPageBoundaryCases(STRING_CMPS, 2, false);
}

void testCmpsd_0x2a7() {
    runStringCases(STRING_CMPS, 4, true, COMPARE_CASES, caseCount(COMPARE_CASES));
    runPageBoundaryCases(STRING_CMPS, 4, true);
}

void testStosb_0x0aa() {
    runStringCases(STRING_STOS, 1, false, STORE_CASES, caseCount(STORE_CASES));
    runPageBoundaryCases(STRING_STOS, 1, false);
}

void testStosb_0x2aa() {
    runStringCases(STRING_STOS, 1, true, STORE_CASES, caseCount(STORE_CASES));
    runPageBoundaryCases(STRING_STOS, 1, true);
}

void testStosw_0x0ab() {
    runStringCases(STRING_STOS, 2, false, STORE_CASES, caseCount(STORE_CASES));
    runPageBoundaryCases(STRING_STOS, 2, false);
}

void testStosd_0x2ab() {
    runStringCases(STRING_STOS, 4, true, STORE_CASES, caseCount(STORE_CASES));
    runPageBoundaryCases(STRING_STOS, 4, true);
}

void testLodsb_0x0ac() {
    runStringCases(STRING_LODS, 1, false, LOAD_CASES, caseCount(LOAD_CASES));
    runPageBoundaryCases(STRING_LODS, 1, false);
}

void testLodsb_0x2ac() {
    runStringCases(STRING_LODS, 1, true, LOAD_CASES, caseCount(LOAD_CASES));
    runPageBoundaryCases(STRING_LODS, 1, true);
}

void testLodsw_0x0ad() {
    runStringCases(STRING_LODS, 2, false, LOAD_CASES, caseCount(LOAD_CASES));
    runPageBoundaryCases(STRING_LODS, 2, false);
}

void testLodsd_0x2ad() {
    runStringCases(STRING_LODS, 4, true, LOAD_CASES, caseCount(LOAD_CASES));
    runPageBoundaryCases(STRING_LODS, 4, true);
}

void testScasb_0x0ae() {
    runStringCases(STRING_SCAS, 1, false, SCAN_CASES, caseCount(SCAN_CASES));
    runPageBoundaryCases(STRING_SCAS, 1, false);
}

void testScasb_0x2ae() {
    runStringCases(STRING_SCAS, 1, true, SCAN_CASES, caseCount(SCAN_CASES));
    runPageBoundaryCases(STRING_SCAS, 1, true);
}

void testScasw_0x0af() {
    runStringCases(STRING_SCAS, 2, false, SCAN_CASES, caseCount(SCAN_CASES));
    runPageBoundaryCases(STRING_SCAS, 2, false);
}

void testScasd_0x2af() {
    runStringCases(STRING_SCAS, 4, true, SCAN_CASES, caseCount(SCAN_CASES));
    runPageBoundaryCases(STRING_SCAS, 4, true);
}

#endif
