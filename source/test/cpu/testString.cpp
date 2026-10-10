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
#include "ksignal.h"
#include "../../emulation/softmmu/kmemory_soft.h"
#include "../../emulation/cpu/normal/normal_strings.h"
#include "../../emulation/cpu/normal/normalCPU.h"
#include "testCPU.h"
#include "testX86Util.h"
#include "testAsmJit.h"
#ifdef BOXEDWINE_JIT
#include "../../emulation/cpu/jit/jitCodeGen.h"
#endif

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
constexpr size_t OVERLAP_SIZE = 256;
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

void runHotMovsCase(int width, bool address32, U32 count, bool backward, const char* name,
    bool checkedMemory = false) {
    newInstruction(backward ? DF : 0);
    cpu->big = address32 ? 1 : 0;
    Seg savedDs = cpu->seg[DS];
    Seg savedEs = cpu->seg[ES];
    bool savedHasDs = cpu->thread->process->hasSetSeg[DS];
    bool savedHasEs = cpu->thread->process->hasSetSeg[ES];
    if (address32) {
        cpu->seg[DS].address = 0;
        cpu->seg[DS].value = 0;
        cpu->seg[ES].address = 0;
        cpu->seg[ES].value = 0;
        cpu->thread->process->hasSetSeg[DS] = false;
        cpu->thread->process->hasSetSeg[ES] = false;
    } else {
        cpu->seg[ES].address = TEST_HEAP_ADDRESS;
        cpu->seg[ES].value = TEST_HEAP_SEG;
        cpu->thread->process->hasSetSeg[ES] = true;
    }

    U32 startOffset = backward ? (count - 1) * width : 0;
    U32 srcBase = address32 ? TEST_HEAP_ADDRESS + SRC_BASE : SRC_BASE;
    U32 dstBase = address32 ? TEST_HEAP_ADDRESS + DST_BASE : DST_BASE;
    U32 srcStart = srcBase + startOffset;
    U32 dstStart = dstBase + startOffset;
    size_t dataSize = count * width;
    U8 initialSrc[256];
    U8 initialDst[256];
    U8 expectedDst[256];
    if (dataSize > sizeof(initialSrc)) {
        failed("%s hot data size", name);
        cpu->seg[DS] = savedDs;
        cpu->seg[ES] = savedEs;
        cpu->thread->process->hasSetSeg[DS] = savedHasDs;
        cpu->thread->process->hasSetSeg[ES] = savedHasEs;
        return;
    }
    for (size_t i = 0; i < dataSize; ++i) {
        initialSrc[i] = (U8)(0x20 + ((i * 13 + 7) & 0x7f));
        initialDst[i] = (U8)(0x90 + ((i * 13 + 7) & 0x3f));
    }
    copyBytes(expectedDst, initialSrc, dataSize);
    emitCode(STRING_MOVS, width, PREFIX_REPE);
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
    if (checkedMemory) {
        DecodedOp* op = cpu->getNextOp();
        if (!op) {
            failed("%s decode", name);
        } else {
            op->exceptionCount = LINEAR_MEMORY_RECOMPILE_FAULTS;
        }
    }
#else
    (void)checkedMemory;
#endif

    for (int pass = 0; pass < 2; ++pass) {
        cpu->eip.u32 = 0;
        cpu->big = address32 ? 1 : 0;
        cpu->setFlags(backward ? DF : 0, FMASK_ALL);
        cpu->reg[R_SI].u32 = srcStart;
        cpu->reg[R_DI].u32 = dstStart;
        cpu->reg[R_CX].u32 = count;
        writeBytes(TEST_HEAP_ADDRESS + SRC_BASE, initialSrc, dataSize);
        writeBytes(TEST_HEAP_ADDRESS + DST_BASE, initialDst, dataSize);
        runTestCPU();
    }

    verifyBytes(TEST_HEAP_ADDRESS + SRC_BASE, initialSrc, dataSize, name);
    verifyBytes(TEST_HEAP_ADDRESS + DST_BASE, expectedDst, dataSize, name);
    S32 totalDelta = backward ? -(S32)(count * width) : (S32)(count * width);
    if (cpu->reg[R_CX].u32 != 0 ||
            cpu->reg[R_SI].u32 != updateIndex(srcStart, totalDelta, address32) ||
            cpu->reg[R_DI].u32 != updateIndex(dstStart, totalDelta, address32)) {
        failed("%s hot registers", name);
    }
    if ((actualFlags(cpu, true) & FLAG_MASK) != (backward ? DF : 0)) {
        failed("%s hot flags", name);
    }
    cpu->seg[DS] = savedDs;
    cpu->seg[ES] = savedEs;
    cpu->thread->process->hasSetSeg[DS] = savedHasDs;
    cpu->thread->process->hasSetSeg[ES] = savedHasEs;
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

void runHotMovsCases(int width, bool address32) {
    runHotMovsCase(width, address32, 48, false, "string move hot forward");
    runHotMovsCase(width, address32, 48, true, "string move hot backward");
#if defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
    if (width == 1 && address32) {
        runHotMovsCase(width, address32, 48, true, "string move checked hot backward", true);
    }
#endif
}

// Segment-based overlap cases use the interpreter fallback. Exercise the
// compiled flat-address copy too, including the small-overlap scalar loop.
void runHotFlatOverlapMovsCases(int width, U32 profileCount, U32 profileElements = 12, bool checkedMemory = false) {
#ifdef BOXEDWINE_JIT
    for (U8 prefix : {PREFIX_REPE, PREFIX_REPNE}) {
        for (bool backward : {false, true}) {
            for (U32 separation : {0u, (U32)width, 7u, 8u, 24u, 128u}) {
                newInstruction(backward ? DF : 0);
                Seg savedDs = cpu->seg[DS];
                Seg savedEs = cpu->seg[ES];
                bool savedHasDs = cpu->thread->process->hasSetSeg[DS];
                bool savedHasEs = cpu->thread->process->hasSetSeg[ES];
                cpu->seg[DS].address = cpu->seg[ES].address = 0;
                cpu->thread->process->hasSetSeg[DS] = false;
                cpu->thread->process->hasSetSeg[ES] = false;
                if (width == 2) pushCode8(0x66);
                emitCode(STRING_MOVS, width, prefix);
                pushCode8(0xcd);
                pushCode8(0x97);
                DecodedOp* op = cpu->getNextOp();
                // The test dispatcher compiles immediately, which otherwise
                // leaves an untraced MOVS as an interpreter stub. Model the
                // recorded profile of previously executed short copies.
                op->runCount = JIT_RUN_COUNT + 1;
                // These profiles previously selected scalar or 64-bit loops.
                // Later calls must work for all sizes, including overlap,
                // when the same compiled instruction retains a vector path.
                if (width == 4) {
                    for (U32 i = 0; i < profileCount; ++i) profileMovsdCount(op, profileElements, !backward);
                } else {
                    op->STR_COUNT = profileCount;
                    op->STR_TOTAL = profileElements * profileCount;
                }
                op->DF0 = !backward;
                op->DF1 = backward;
#ifdef BOXEDWINE_HOST_EXCEPTIONS
                if (checkedMemory) {
                    op->exceptionCount = LINEAR_MEMORY_RECOMPILE_FAULTS;
                }
#endif
                // Reuse compiled code with nonzero and zero counts.
                // Motorhead copies twelve dwords
                // with EDI = ESI + 4 to propagate the first value.
                const U32 vectorElements = 16 / width;
                for (U32 count : {12u, 0u, 1u, vectorElements - 1, vectorElements, vectorElements + 1, 17u}) {
                    U8 expected[OVERLAP_SIZE];
                    initOverlapBytes(expected, sizeof(expected));
                    writeOverlapBytes(expected, sizeof(expected));
                    U32 last = backward && count ? (count - 1) * width : 0;
                    U32 src = last + (backward ? separation : 0);
                    U32 dst = last + (backward ? 0 : separation);
                    cpu->eip.u32 = 0;
                    const U32 flags = ARITH_FLAG_MASK | (backward ? DF : 0);
                    cpu->setFlags(flags, FMASK_ALL);
                    cpu->reg[R_SI].u32 = TEST_HEAP_ADDRESS + OVERLAP_BASE + src;
                    cpu->reg[R_DI].u32 = TEST_HEAP_ADDRESS + OVERLAP_BASE + dst;
                    cpu->reg[R_CX].u32 = count;
                    for (U32 i = 0; i < count; ++i) {
                        U32 value = readCaseValue(expected, src, width);
                        writeCaseValue(expected, dst, width, value);
                        src += backward ? -width : width;
                        dst += backward ? -width : width;
                    }
                    // ARM alignment faults can discard a compiled block. Keep
                    // its fault profile, but compile again before the next case
                    // so every count exercises a JIT entry, including retries.
                    op = cpu->getNextOp();
                    if (!op->pfnJitCode) {
                        op->runCount = JIT_RUN_COUNT + 1;
                        startNewJIT(cpu, TEST_CODE_ADDRESS, op);
                    }
                    if (!op->pfnJitCode || (op->flags2 & OP_FLAG2_TRACED_STUB)) {
                        failed("hot flat MOVS overlap missing compiled entry: width=%d separation=%u count=%u DF=%d",
                            width, separation, count, backward);
                        break;
                    }
                    runTestCPU();
                    verifyOverlapBytes(expected, sizeof(expected), "hot flat MOVS overlap");
                    if (cpu->reg[R_CX].u32 != 0 ||
                            cpu->reg[R_SI].u32 != TEST_HEAP_ADDRESS + OVERLAP_BASE + src ||
                            cpu->reg[R_DI].u32 != TEST_HEAP_ADDRESS + OVERLAP_BASE + dst ||
                            (actualFlags(cpu, true) & FLAG_MASK) != flags) {
                        failed("hot flat MOVS overlap width=%d separation=%u count=%u DF=%d",
                            width, separation, count, backward);
                    }
                }
                cpu->seg[DS] = savedDs;
                cpu->seg[ES] = savedEs;
                cpu->thread->process->hasSetSeg[DS] = savedHasDs;
                cpu->thread->process->hasSetSeg[ES] = savedHasEs;
            }
        }
    }
#endif
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

// Compare the shared helper against instruction-by-instruction copying. The
// deliberately overlapping cases must not become memmove semantics.
void runSharedMovsdCases() {
    newInstruction(0);
    const U32 arena = TEST_HEAP_ADDRESS + 0x10000;
    const U32 length = 0x9000;
    std::vector<U8> initial(length), expected(length), actual(length);
    for (U32 i = 0; i < length; ++i) initial[i] = (U8)(i * 37 + (i >> 8));
    for (bool backward : {false, true}) for (U32 offset : {0u, 1u, 3u})
    for (S32 separation : {-9, -4, -1, 0, 1, 4, 9, 0x4000})
    for (U32 count : {0u, 1u, 12u, 1025u}) {
        expected = initial;
        memory->memcpy(arena, initial.data(), length);
        const U32 last = backward && count ? (count - 1) * 4 : 0;
        U32 src = 0x2000 + offset + last, dst = src + separation;
        const U32 step = backward ? (U32)-4 : 4;
        cpu->seg[DS].address = arena;
        cpu->seg[ES].address = arena + 0x1000;
        cpu->reg[R_SI].u32 = count ? src : 0xfffffff0;
        cpu->reg[R_DI].u32 = count ? dst - 0x1000 : 0xfffffff0;
        cpu->reg[R_CX].u32 = count;
        const U32 flags = ARITH_FLAG_MASK | (backward ? DF : 0);
        cpu->setFlags(flags, FMASK_ALL);
        for (U32 i = 0; i < count; ++i) {
            U32 value;
            ::memcpy(&value, expected.data() + src, 4);
            ::memcpy(expected.data() + dst, &value, 4);
            src += step;
            dst += step;
        }
        movsd32r(cpu, DS);
        memory->memcpy(actual.data(), arena, length);
        if (actual != expected || cpu->reg[R_CX].u32 ||
                cpu->reg[R_SI].u32 != (count ? src : 0xfffffff0) ||
                cpu->reg[R_DI].u32 != (count ? dst - 0x1000 : 0xfffffff0) ||
                (actualFlags(cpu, true) & FLAG_MASK) != flags) {
            failed("shared REP MOVSD count=%u offset=%u separation=%d DF=%u", count, offset, separation, backward);
        }
    }

    // Distinct guest addresses can reference overlapping host RAM.
    const U32 alias = 0x30000000;
    const RamPage ram = getMemData(memory)->mmu[arena >> K_PAGE_SHIFT].getRamPageIndex();
    if (memory->mapPages(testContext().thread, alias >> K_PAGE_SHIFT, {ram}, PAGE_READ | PAGE_WRITE) != alias) {
        failed("shared REP MOVSD alias mapping");
        return;
    }
    for (bool backward : {false, true}) for (U32 separation : {1u, 3u, 4u}) {
        memory->memcpy(arena, initial.data(), K_PAGE_SIZE);
        expected = initial;
        U32 src = 0x100 + (backward ? 31 * 4 + separation : 0);
        U32 dst = src + (backward ? -separation : separation);
        cpu->seg[DS].address = arena;
        cpu->seg[ES].address = alias;
        cpu->reg[R_SI].u32 = src;
        cpu->reg[R_DI].u32 = dst;
        cpu->reg[R_CX].u32 = 32;
        cpu->setFlags(backward ? DF : 0, FMASK_ALL);
        for (U32 i = 0; i < 32; ++i) {
            U32 value;
            ::memcpy(&value, expected.data() + src, 4);
            ::memcpy(expected.data() + dst, &value, 4);
            src += backward ? -4 : 4;
            dst += backward ? -4 : 4;
        }
        movsd32r(cpu, DS);
        memory->memcpy(actual.data(), arena, K_PAGE_SIZE);
        if (::memcmp(actual.data(), expected.data(), K_PAGE_SIZE) || cpu->reg[R_CX].u32 ||
                cpu->reg[R_SI].u32 != src || cpu->reg[R_DI].u32 != dst) {
            failed("shared REP MOVSD aliased overlap separation=%u DF=%u", separation, backward);
        }
    }
    memory->unmap(alias, K_PAGE_SIZE);
}

#ifdef BOXEDWINE_JIT
bool movsCodeChangedWithoutHostFault(DecodedOp* op, void* compiledCode) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
    // A host fault can replace linear/unaligned accesses with checked ones.
    if (op->exceptionCount) return false;
#endif
    return op->pfnJitCode != compiledCode;
}
#endif

void runHotFlatMovsdMixedCounts() {
#ifdef BOXEDWINE_JIT
    // A memcpy site can receive very different lengths on successive calls.
    // Count changes must reuse the same compiled code, including zero,
    // short and multi-page copies with overlap.
    for (U32 average : {1u, 6u, 7u, 2048u})
    for (bool backward : {false, true}) for (bool overlap : {false, true}) {
        newInstruction(CF | ZF | (backward ? DF : 0));
        cpu->seg[DS].address = cpu->seg[ES].address = 0;
        cpu->thread->process->hasSetSeg[DS] = false;
        cpu->thread->process->hasSetSeg[ES] = false;
        pushCode8(0xf3); pushCode8(0xa5);
        pushCode8(0xb8); testPushCode32(0x12345678);
        pushCode8(0xcd); pushCode8(0x97);
        DecodedOp* op = cpu->getNextOp();
        op->runCount = JIT_RUN_COUNT + 1;
        for (U32 i = 0; i < 20; ++i) profileMovsdCount(op, average, true);
        op->DF0 = !backward;
        op->DF1 = backward;
        startNewJIT(cpu, TEST_CODE_ADDRESS, op);
        if (!op->pfnJitCode || (op->flags2 & OP_FLAG2_TRACED_STUB)) {
            failed("flat REP MOVSD profile case did not compile");
            return;
        }
        const U32 arena = TEST_HEAP_ADDRESS + 0x10000, length = 0x6000;
        std::vector<U8> initial(length), expected(length), actual(length);
        for (U32 i = 0; i < length; ++i) initial[i] = (U8)(i * 37 + (i >> 8));
        for (U32 count : {0u, 1u, 6u, 7u, 8u, 15u, 16u, 17u, 511u, 512u, 513u, 1025u, 0u, 1u}) {
            expected = initial;
            memory->memcpy(arena, initial.data(), length);
            const U32 last = backward && count ? (count - 1) * 4 : 0;
            U32 src = 0x1003 + last + (overlap && backward ? 1 : 0);
            U32 dst = overlap ? src + (backward ? -1 : 1) : 0x3001 + last;
            cpu->eip.u32 = 0;
            const U32 flags = CF | ZF | (backward ? DF : 0);
            cpu->setFlags(flags, FMASK_ALL);
            cpu->reg[R_SI].u32 = arena + src;
            cpu->reg[R_DI].u32 = arena + dst;
            cpu->reg[R_CX].u32 = count;
            for (U32 i = 0; i < count; ++i) {
                U32 value;
                ::memcpy(&value, expected.data() + src, 4);
                ::memcpy(expected.data() + dst, &value, 4);
                src += backward ? -4 : 4;
                dst += backward ? -4 : 4;
            }
            void* previousCode = op->pfnJitCode;
            runTestCPU();
            // Native alignment/aperture faults may legitimately replace the
            // block with checked accesses; changing the count alone may not.
            if (movsCodeChangedWithoutHostFault(op, previousCode)) {
                failed("flat REP MOVSD invalidated its block after a count change");
            }
            memory->memcpy(actual.data(), arena, length);
            if (actual != expected || cpu->reg[R_CX].u32 ||
                    cpu->reg[R_SI].u32 != arena + src || cpu->reg[R_DI].u32 != arena + dst ||
                    cpu->reg[R_AX].u32 != 0x12345678 || (actualFlags(cpu, true) & FLAG_MASK) != flags) {
                failed("flat REP MOVSD profile=%u count=%u DF=%u overlap=%u", average, count, backward, overlap);
            }
        }
    }
#endif
}


void runHotFlatMovsAliasedOverlap(U32 width, bool helper = false) {
#ifdef BOXEDWINE_JIT
    // Guest mappings may look disjoint while their RAM overlaps. Cover both
    // overlap directions, vector tails, and copies spanning aliased pages.
    const U32 vectorElements = 16 / width;
    for (U32 count : {0u, 1u, vectorElements - 1, vectorElements, vectorElements + 1, 512u})
    for (bool backward : {false, true}) for (U32 offset : {0x100u, K_PAGE_SIZE - 3u})
    for (S32 separation : {-17, -16, -15, -4, -1, 0, 1, 4, 15, 16, 17}) {
        newInstruction(CF | ZF | (backward ? DF : 0));
        cpu->seg[DS].address = cpu->seg[ES].address = 0;
        cpu->thread->process->hasSetSeg[DS] = false;
        cpu->thread->process->hasSetSeg[ES] = false;
        const U32 arena = TEST_HEAP_ADDRESS + 0x10000, alias = 0x30000000;
        const U32 length = 3 * K_PAGE_SIZE;
        std::vector<U8> initial(length), expected(length), actual(length);
        for (U32 i = 0; i < length; ++i) initial[i] = (U8)(i * 37 + (i >> 8));
        memory->memcpy(arena, initial.data(), length);
        std::vector<RamPage> pages;
        for (U32 page = 0; page < 3; ++page) {
            pages.push_back(getMemData(memory)->mmu[(arena >> K_PAGE_SHIFT) + page].getRamPageIndex());
        }
        if (memory->mapPages(testContext().thread, alias >> K_PAGE_SHIFT, pages, PAGE_READ | PAGE_WRITE) != alias) {
            failed("flat REP MOVS alias mapping");
            return;
        }
        U32 src = offset + (backward && count ? (count - 1) * width : 0);
        U32 dst = src + separation;
        cpu->reg[R_SI].u32 = arena + src;
        cpu->reg[R_DI].u32 = alias + dst;
        cpu->reg[R_CX].u32 = count;
        expected = initial;
        for (U32 i = 0; i < count; ++i) {
            U32 value = 0;
            ::memcpy(&value, expected.data() + src, width);
            ::memcpy(expected.data() + dst, &value, width);
            src += backward ? -width : width;
            dst += backward ? -width : width;
        }
        if (width == 2) pushCode8(0x66);
        emitCode(STRING_MOVS, width, PREFIX_REPE);
        pushCode8(0xb8); testPushCode32(0x12345678);
        pushCode8(0xcd); pushCode8(0x97);
        DecodedOp* op = cpu->getNextOp();
        op->runCount = JIT_RUN_COUNT + 1;
        if (width == 4) {
            for (U32 i = 0; i < 20; ++i) profileMovsdCount(op, helper ? 2048 : 1, !backward);
        } else {
            op->STR_COUNT = 20;
            op->STR_TOTAL = 20 * (helper ? 2048 : 1);
        }
        op->DF0 = !backward;
        op->DF1 = backward;
        startNewJIT(cpu, TEST_CODE_ADDRESS, op);
        if (!op->pfnJitCode) failed("flat REP MOVS alias case did not compile");
        runTestCPU();
        memory->memcpy(actual.data(), arena, length);
        if (actual != expected || cpu->reg[R_CX].u32 ||
                cpu->reg[R_SI].u32 != arena + src || cpu->reg[R_DI].u32 != alias + dst ||
                cpu->reg[R_AX].u32 != 0x12345678 ||
                (actualFlags(cpu, true) & FLAG_MASK) != (CF | ZF | (backward ? DF : 0))) {
            failed("flat REP MOVS aliased overlap width=%u count=%u DF=%u offset=%u separation=%d",
                width, count, backward, offset, separation);
        }
        memory->unmap(alias, length);
    }
#endif
}


void runSharedMovsFaultCases(U32 width, bool flat = false, bool helper = false) {
    for (bool backward : {false, true}) for (bool sourceFault : {false, true})
    for (U32 unaligned : {0u, 1u, 2u, 3u}) {
        newInstruction(CF | ZF | (backward ? DF : 0));
        auto& ctx = testContext();
        cpu->seg[ES].address = TEST_HEAP_ADDRESS;
        ctx.process->hasSetSeg[ES] = true;
        const U32 flatBase = flat ? TEST_HEAP_ADDRESS : 0;
        const U32 count = flat ? 1025 : 64;
        if (flat) {
            cpu->seg[DS].address = cpu->seg[ES].address = 0;
            ctx.process->hasSetSeg[DS] = ctx.process->hasSetSeg[ES] = false;
        }
        const U32 src = (backward ? 0x200cu : 0x1ff0u) + unaligned;
        const U32 dst = (backward ? 0x600cu : 0x5ff0u) + unaligned;
        for (U32 i = 0; i < 0x2000; ++i) {
            memory->writeb(TEST_HEAP_ADDRESS + 0x1000 + i, (U8)(i * 37 + 9));
            memory->writeb(TEST_HEAP_ADDRESS + 0x5000 + i, 0x55);
        }
        cpu->reg[R_SI].u32 = src + flatBase;
        cpu->reg[R_DI].u32 = dst + flatBase;
        cpu->reg[R_CX].u32 = count;
        cpu->reg[R_BX].u32 = 0x12345678;
        const U32 repAddress = ctx.codeIp;
        if (width == 2) pushCode8(0x66);
        pushCode8(0xf3); pushCode8(width == 1 ? 0xa4 : 0xa5);
        pushCode8(0xbb); testPushCode32(0xbad);
        auto oldAction = ctx.process->sigActions[K_SIGSEGV];
        auto& action = ctx.process->sigActions[K_SIGSEGV];
        action.reset();
        action.flags = K_SA_SIGINFO;
        action.handlerAndSigAction = ctx.codeIp;
        pushCode8(0xcd); pushCode8(0x97);
#ifdef BOXEDWINE_JIT
        {
            DecodedOp* op = cpu->getNextOp();
            op->runCount = JIT_RUN_COUNT + 1;
            // Exercise the inline loop for flat copies and the RAM helper
            // followed by interpreter fallback for segmented copies.
            if (width == 4) {
                for (U32 i = 0; i < 20; ++i) profileMovsdCount(op, flat && !helper ? 1 : 2048, !backward);
            } else {
                op->STR_COUNT = 20;
                op->STR_TOTAL = 20 * (flat && !helper ? 1 : 2048);
            }
            op->DF0 = !backward;
            op->DF1 = backward;
            startNewJIT(cpu, TEST_CODE_ADDRESS, op);
            if (!op->pfnJitCode) failed("REP MOVS fault case did not compile");
        }
#endif
        const U32 page = TEST_HEAP_ADDRESS + (sourceFault
            ? (backward ? 0x1000 : 0x2000) : (backward ? 0x5000 : 0x6000));
        memory->mprotect(ctx.thread, page, K_PAGE_SIZE, sourceFault ? 0 : K_PROT_READ);
        runTestCPU();
        memory->mprotect(ctx.thread, page, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
        const U32 completed = backward ? (src - 0x2000) / width + 1 : (0x2000 - src) / width;
        const U32 advance = backward ? -completed * width : completed * width;
        const U32 uc = cpu->reg[R_DX].u32;
        if (action.sigInfo[0] != K_SIGSEGV || !uc || memory->readd(uc + 0x28) != src + flatBase + advance ||
                memory->readd(uc + 0x24) != dst + flatBase + advance || memory->readd(uc + 0x3c) != count - completed ||
                memory->readd(uc + 0x4c) != repAddress - TEST_CODE_ADDRESS ||
                memory->readd(uc + 0x34) != 0x12345678 ||
                (memory->readd(uc + 0x54) & (CF | ZF | DF)) != (CF | ZF | (backward ? DF : 0))) {
            failed("shared REP MOVS partial fault width=%u source=%u DF=%u offset=%u", width, sourceFault, backward, unaligned);
        }
        std::vector<U8> expected(0x2000, 0x55);
        for (U32 i = 0; i < completed; ++i) for (U32 j = 0; j < width; ++j) {
            const U32 delta = backward ? -i * width : i * width;
            expected[dst + delta + j - 0x5000] = memory->readb(TEST_HEAP_ADDRESS + src + delta + j);
        }
        std::vector<U8> actual(0x2000);
        memory->memcpy(actual.data(), TEST_HEAP_ADDRESS + 0x5000, 0x2000);
        if (actual != expected) failed("shared REP MOVS fault changed uncommitted bytes");
        ctx.process->sigActions[K_SIGSEGV] = oldAction;
    }
}

#ifdef BOXEDWINE_JIT
#undef cpu
#undef memory

DecodedOp* compileMovsdSpanCase(CPU* cpu, U32 expected) {
    DecodedOp* entry = cpu->getNextOp();
    for (DecodedOp* op = entry; op; op = op->next) {
        op->runCount = JIT_RUN_COUNT + 1;
        if (op->inst == Movsd) {
            for (U32 i = 0; i < 20; ++i) profileMovsdCount(op, expected, true);
            op->DF0 = op->DF1 = 1;
        }
        if (op->isBranch()) break;
    }
    startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
    if (!entry->pfnJitCode || !(entry->STR_FLAGS & STR_UNROLLED_COPY)) {
        testFail("REP MOVSD span case did not specialize");
    }
    return entry;
}

void setMovsdFlatSegments() {
    auto& ctx = testContext();
    ctx.cpu->seg[DS].address = ctx.cpu->seg[ES].address = 0;
    ctx.process->hasSetSeg[DS] = ctx.process->hasSetSeg[ES] = true;
}

void runMovsdCountSelection() {
    // Real interpreter warmup: fixed counts, a dominant size with occasional
    // large copies, a polymorphic site, and backward-only copies.
    for (U32 expected : {6u, 7u}) for (U32 mode : {0u, 1u, 2u, 3u}) {
        testNewInstruction(mode == 3 ? DF : 0);
        setMovsdFlatSegments();
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        testPushCode8(0xf3); testPushCode8(0xa5);
        testPushCode8(0xcd); testPushCode8(0x97);
        DecodedOp* op = cpu->getNextOp();
        for (U32 i = 0; i < 100; ++i) {
            cpu->eip.u32 = 0;
            cpu->reg[R_SI].u32 = TEST_HEAP_ADDRESS + 0x1000;
            cpu->reg[R_DI].u32 = TEST_HEAP_ADDRESS + 0x3000;
            cpu->reg[R_CX].u32 = mode == 1 && i == 0 ? 1025 :
                mode == 2 && (i & 1) ? 13 - expected : expected;
            NormalCPU::getFunctionForOp(op)(cpu, op);
        }
        if (op->STR_COUNT != 100 || movsdExpectedCount(op) != (MOVSD_UNROLL_SUPPORTED && mode < 2 ? expected : 0)) {
            testFail("REP MOVSD interpreter count profile expected=%u mode=%u", expected, mode);
        }
        op->runCount = JIT_RUN_COUNT + 1;
        startNewJIT(cpu, TEST_CODE_ADDRESS, op);
        if (!op->pfnJitCode || bool(op->STR_FLAGS & STR_UNROLLED_COPY) != (MOVSD_UNROLL_SUPPORTED && mode < 2)) {
            testFail("REP MOVSD profile selected wrong JIT body");
        }
    }
    // Saturation cannot carry the six-dword hit count into the seven counter.
    DecodedOp op;
    for (U32 i = 0; i < 0x10010; ++i) profileMovsdCount(&op, 6, true);
    if (op.STR_COUNT != 0xffff || op.MOVSD_SIZE_HITS != 0xffff || movsdExpectedCount(&op) != (MOVSD_UNROLL_SUPPORTED ? 6u : 0u)) {
        testFail("REP MOVSD profile counter saturation");
    }
}

void runRepSpanSafety() {
    for (U32 count : {6u, 7u}) for (bool backward : {false, true})
    for (bool sourceFault : {false, true}) for (bool firstPage : {false, true})
    for (U32 unaligned : {0u, 1u, 2u, 3u}) {
        testNewInstruction(CF | ZF | (backward ? DF : 0));
        setMovsdFlatSegments();
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        KMemory* memory = ctx.memory;
        const U32 src = (firstPage ? 0x1800 : backward ? 0x200c : 0x1ff0) + unaligned;
        const U32 dst = (firstPage ? 0x5800 : backward ? 0x600c : 0x5ff0) + unaligned;
        for (U32 i = 0; i < 0x2000; ++i) {
            memory->writeb(TEST_HEAP_ADDRESS + 0x1000 + i, (U8)(i * 37 + 9));
            memory->writeb(TEST_HEAP_ADDRESS + 0x5000 + i, 0x55);
        }
        cpu->reg[R_SI].u32 = TEST_HEAP_ADDRESS + src;
        cpu->reg[R_DI].u32 = TEST_HEAP_ADDRESS + dst;
        cpu->reg[R_CX].u32 = count;
        cpu->reg[R_BX].u32 = 0x12345678;
        const U32 repAddress = ctx.codeIp;
        testPushCode8(0xf3); testPushCode8(0xa5);
        testPushCode8(0xbb); testPushCode32(0xbad);
        auto oldAction = ctx.process->sigActions[K_SIGSEGV];
        auto& action = ctx.process->sigActions[K_SIGSEGV];
        action.reset();
        action.flags = K_SA_SIGINFO;
        action.handlerAndSigAction = ctx.codeIp;
        testPushCode8(0xcd); testPushCode8(0x97);
        compileMovsdSpanCase(cpu, count);
        const U32 page = TEST_HEAP_ADDRESS + (firstPage ? ((sourceFault ? src : dst) & ~K_PAGE_MASK) :
            sourceFault ? (backward ? 0x1000 : 0x2000) : (backward ? 0x5000 : 0x6000));
        memory->mprotect(ctx.thread, page, K_PAGE_SIZE, sourceFault ? 0 : K_PROT_READ);
        testRunCPU();
        memory->mprotect(ctx.thread, page, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
        const U32 completed = firstPage ? 0 : backward ? 4 : unaligned ? 3 : 4;
        const U32 advance = backward ? -completed * 4 : completed * 4;
        const U32 uc = cpu->reg[R_DX].u32;
        if (action.sigInfo[0] != K_SIGSEGV || !uc ||
                memory->readd(uc + 0x28) != TEST_HEAP_ADDRESS + src + advance ||
                memory->readd(uc + 0x24) != TEST_HEAP_ADDRESS + dst + advance ||
                memory->readd(uc + 0x3c) != count - completed ||
                memory->readd(uc + 0x4c) != repAddress - TEST_CODE_ADDRESS ||
                memory->readd(uc + 0x34) != 0x12345678 ||
                (memory->readd(uc + 0x54) & (CF | ZF | DF)) != (CF | ZF | (backward ? DF : 0))) {
            testFail("REP MOVSD span fault count=%u DF=%u source=%u first=%u offset=%u",
                count, backward, sourceFault, firstPage, unaligned);
        }
        std::vector<U8> expected(0x2000, 0x55), actual(0x2000);
        for (U32 i = 0; i < completed; ++i) for (U32 j = 0; j < 4; ++j) {
            const U32 delta = backward ? -i * 4 : i * 4;
            expected[dst + delta + j - 0x5000] = memory->readb(TEST_HEAP_ADDRESS + src + delta + j);
        }
        memory->memcpy(actual.data(), TEST_HEAP_ADDRESS + 0x5000, 0x2000);
        if (actual != expected) testFail("REP MOVSD span fault wrote uncommitted bytes");
        ctx.process->sigActions[K_SIGSEGV] = oldAction;
    }
    for (U32 count : {6u, 7u}) for (S32 separation : {-4, -1, 0, 1, 4})
    for (bool backward : {false, true}) {
        testNewInstruction(CF | ZF | (backward ? DF : 0));
        setMovsdFlatSegments();
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        KMemory* memory = ctx.memory;
        const U32 page = TEST_HEAP_ADDRESS + 0x10000, alias = 0x30000000;
        std::vector<U8> initial(K_PAGE_SIZE), actual(K_PAGE_SIZE);
        for (U32 i = 0; i < K_PAGE_SIZE; ++i) initial[i] = (U8)(i * 37 + (i >> 8));
        auto expected = initial;
        memory->memcpy(page, initial.data(), K_PAGE_SIZE);
        RamPage ram = getMemData(memory)->mmu[page >> K_PAGE_SHIFT].getRamPageIndex();
        if (memory->mapPages(ctx.thread, alias >> K_PAGE_SHIFT, {ram}, PAGE_READ | PAGE_WRITE) != alias) {
            testFail("REP MOVSD span alias mapping failed");
            return;
        }
        const U32 src = 0x803 + (backward ? (count - 1) * 4 : 0), dst = src + separation;
        cpu->reg[R_SI].u32 = page + src;
        cpu->reg[R_DI].u32 = alias + dst;
        cpu->reg[R_CX].u32 = count;
        for (U32 i = 0; i < count; ++i) {
            const U32 delta = backward ? -i * 4 : i * 4;
            U32 value;
            ::memcpy(&value, expected.data() + (src + delta), 4);
            ::memcpy(expected.data() + (dst + delta), &value, 4);
        }
        testPushCode8(0xf3); testPushCode8(0xa5);
        testPushCode8(0xcd); testPushCode8(0x97);
        compileMovsdSpanCase(cpu, count);
        testRunCPU();
        memory->memcpy(actual.data(), page, K_PAGE_SIZE);
        const U32 advance = backward ? -count * 4 : count * 4;
        if (actual != expected || cpu->reg[R_CX].u32 || cpu->reg[R_SI].u32 != page + src + advance ||
                cpu->reg[R_DI].u32 != alias + dst + advance ||
                (actualFlags(cpu, true) & FLAG_MASK) != (CF | ZF | (backward ? DF : 0))) {
            testFail("REP MOVSD aliased span count=%u separation=%d DF=%u", count, separation, backward);
        }
        memory->unmap(alias, K_PAGE_SIZE);
    }
    // Last fitting byte and first crossing byte, independently for each range.
    for (U32 count : {6u, 7u}) for (U32 srcExtra : {0u, 1u, 3u}) for (U32 dstExtra : {0u, 1u, 3u}) {
        testNewInstruction(CF | ZF);
        setMovsdFlatSegments();
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        const U32 src = 0x2000 - count * 4 + srcExtra, dst = 0x6000 - count * 4 + dstExtra;
        std::vector<U8> data(0x8000), actual(0x8000);
        for (U32 i = 0; i < data.size(); ++i) data[i] = (U8)(i * 19 + (i >> 8));
        auto expected = data;
        ::memcpy(expected.data() + dst, data.data() + src, count * 4);
        ctx.memory->memcpy(TEST_HEAP_ADDRESS, data.data(), data.size());
        cpu->reg[R_SI].u32 = TEST_HEAP_ADDRESS + src;
        cpu->reg[R_DI].u32 = TEST_HEAP_ADDRESS + dst;
        cpu->reg[R_CX].u32 = count;
        testPushCode8(0xf3); testPushCode8(0xa5);
        testPushCode8(0xb8); testPushCode32(0x12345678);
        testPushCode8(0xcd); testPushCode8(0x97);
        compileMovsdSpanCase(cpu, count);
        testRunCPU();
        ctx.memory->memcpy(actual.data(), TEST_HEAP_ADDRESS, data.size());
        if (actual != expected || cpu->reg[R_CX].u32 || cpu->reg[R_SI].u32 != TEST_HEAP_ADDRESS + src + count * 4 ||
                cpu->reg[R_DI].u32 != TEST_HEAP_ADDRESS + dst + count * 4 || cpu->reg[R_AX].u32 != 0x12345678 ||
                (actualFlags(cpu, true) & FLAG_MASK) != (CF | ZF)) {
            testFail("REP MOVSD span boundary count=%u offsets=%u,%u", count, srcExtra, dstExtra);
        }
    }
    for (U32 count : {6u, 7u}) {
        testNewInstruction(CF | ZF);
        setMovsdFlatSegments();
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        std::vector<U8> replacement(count * 4, 0x90);
        const U32 value = 0x12345678;
        ::memcpy(replacement.data(), &value, 4);
        ctx.memory->memcpy(TEST_HEAP_ADDRESS + 0x500, replacement.data(), replacement.size());
        cpu->reg[R_SI].u32 = TEST_HEAP_ADDRESS + 0x500;
        cpu->reg[R_DI].u32 = TEST_CODE_ADDRESS + 3;
        cpu->reg[R_CX].u32 = count;
        testPushCode8(0xf3); testPushCode8(0xa5);
        testPushCode8(0xb8); testPushCode32(0xbad);
        for (U32 i = 4; i < count * 4; ++i) testPushCode8(0x90);
        testPushCode8(0xcd); testPushCode8(0x97);
        compileMovsdSpanCase(cpu, count);
        testRunCPU();
        if (cpu->reg[R_AX].u32 != value || cpu->reg[R_CX].u32 ||
                cpu->reg[R_SI].u32 != TEST_HEAP_ADDRESS + 0x500 + count * 4 ||
                cpu->reg[R_DI].u32 != TEST_CODE_ADDRESS + 3 + count * 4 ||
                (actualFlags(cpu, true) & FLAG_MASK) != (CF | ZF)) {
            testFail("REP MOVSD span self-modifying code count=%u", count);
        }
        testNewInstruction(CF | ZF);
        setMovsdFlatSegments();
        cpu = ctx.cpu;
        cpu->reg[R_CX].u32 = 0;
        cpu->reg[R_SI].u32 = cpu->reg[R_DI].u32 = 0xfffffffd;
        testPushCode8(0xf3); testPushCode8(0xa5);
        testPushCode8(0xcd); testPushCode8(0x97);
        compileMovsdSpanCase(cpu, count);
        testRunCPU();
        if (cpu->reg[R_CX].u32 || cpu->reg[R_SI].u32 != 0xfffffffd || cpu->reg[R_DI].u32 != 0xfffffffd ||
                (actualFlags(cpu, true) & FLAG_MASK) != (CF | ZF)) {
            testFail("REP MOVSD zero count touched state");
        }
    }
    // A writable alias retained as COW must detach before the batched store.
    for (U32 count : {6u, 7u}) {
        testNewInstruction(CF | ZF);
        setMovsdFlatSegments();
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        KMemory* memory = ctx.memory;
        const U32 src = TEST_HEAP_ADDRESS + 0x10000, dst = TEST_HEAP_ADDRESS + 0x12000;
        const U32 alias = 0x30000000;
        for (U32 i = 0; i < count; ++i) {
            memory->writed(src + i * 4, 0x12340000 + i);
            memory->writed(dst + i * 4, 0x55555555);
        }
        auto* data = getMemData(memory);
        RamPage ram = data->mmu[dst >> K_PAGE_SHIFT].getRamPageIndex();
        memory->mapPages(ctx.thread, alias >> K_PAGE_SHIFT, {ram}, PAGE_READ | PAGE_WRITE);
        data->mmu[dst >> K_PAGE_SHIFT].setPageType(memory, dst >> K_PAGE_SHIFT, PageType::CopyOnWrite);
        data->onPageChanged(dst >> K_PAGE_SHIFT);
        cpu->reg[R_SI].u32 = src;
        cpu->reg[R_DI].u32 = dst;
        cpu->reg[R_CX].u32 = count;
        testPushCode8(0xf3); testPushCode8(0xa5);
        testPushCode8(0xcd); testPushCode8(0x97);
        compileMovsdSpanCase(cpu, count);
        testRunCPU();
        for (U32 i = 0; i < count; ++i) {
            if (memory->readd(dst + i * 4) != 0x12340000 + i || memory->readd(alias + i * 4) != 0x55555555) {
                testFail("REP MOVSD span failed to detach COW page");
            }
        }
        memory->unmap(alias, K_PAGE_SIZE);

        // Enable a data breakpoint after compilation. Preserve the existing
        // interpreter behavior: report the trap at the instruction boundary.
        testNewInstruction(CF | ZF);
        setMovsdFlatSegments();
        cpu = ctx.cpu;
        memory = ctx.memory;
        memory->writed(src, 0x87654321);
        memory->writed(dst, 0);
        cpu->reg[R_SI].u32 = src;
        cpu->reg[R_DI].u32 = dst;
        cpu->reg[R_CX].u32 = count;
        testPushCode8(0xf3); testPushCode8(0xa5);
        testPushCode8(0xcd); testPushCode8(0x97);
        auto oldAction = ctx.process->sigActions[K_SIGTRAP];
        auto& action = ctx.process->sigActions[K_SIGTRAP];
        action.reset();
        action.flags = K_SA_SIGINFO;
        action.handlerAndSigAction = ctx.codeIp;
        testPushCode8(0xcd); testPushCode8(0x97);
        compileMovsdSpanCase(cpu, count);
        ctx.thread->debugRegs[0] = dst;
        ctx.thread->debugRegs[7] = 3 | (1u << 16) | (3u << 18);
        ctx.thread->updateDebugTrapActive();
        testRunCPU();
        const U32 uc = cpu->reg[R_DX].u32;
        if (action.sigInfo[0] != K_SIGTRAP || !uc || memory->readd(dst) != 0x87654321 ||
                memory->readd(uc + 0x28) != src + count * 4 || memory->readd(uc + 0x24) != dst + count * 4 ||
                memory->readd(uc + 0x3c) != 0 || memory->readd(uc + 0x4c) != 2) {
            testFail("REP MOVSD span bypassed data breakpoint progress");
        }
        for (U32& reg : ctx.thread->debugRegs) reg = 0;
        ctx.thread->updateDebugTrapActive();
        ctx.process->sigActions[K_SIGTRAP] = oldAction;
    }

}

#define cpu (testContext().cpu)
#define memory (testContext().memory)
#endif

void runHotMovsLiveState(U32 width) {
#ifdef BOXEDWINE_JIT
    // Exercise both the inline vector loop and the shared helper with live
    // GP/XMM/x87 state and lazy arithmetic flags on entry and continuation.
    for (U32 profile : {1u, 2048u}) for (bool backward : {false, true})
    for (bool segments : {false, true}) for (U32 offset : {0u, 4093u})
    for (U32 count : {0u, 1u, 7u, 32u, 1025u}) {
        newInstruction(backward ? DF : 0);
        cpu->fpu.FINIT();
        const U32 arena = TEST_HEAP_ADDRESS;
        const U32 segBase = segments ? arena : 0;
        cpu->seg[DS].address = cpu->seg[ES].address = segBase;
        cpu->thread->process->hasSetSeg[DS] = segments;
        cpu->thread->process->hasSetSeg[ES] = segments;
        const U32 last = backward && count ? (count - 1) * width : 0;
        const U32 source = count ? arena + 0x2000 + offset + last - segBase : 0xffffefffu;
        const U32 dest = count ? arena + 0xa000 + offset + last - segBase : 0xffffdfffu;
        for (U32 i = 0; i < count; ++i)
            writeMemoryValue(arena + 0x2000 + offset + i * width, width, 0x87654321 + i);
        auto imm = [](U32 reg, U32 value) { pushCode8(0xb8 + reg); testPushCode32(value); };
        imm(0, 0x12345678);
        pushCode8(0x66); pushCode8(0x0f); pushCode8(0x6e); pushCode8(0xc0); // movd xmm0,eax
        pushCode8(0xd9); pushCode8(0xe8); // fld1
        imm(0, 5); pushCode8(0x83); pushCode8(0xe8); pushCode8(6); // sub eax,6
        imm(3, 0xabcdef99); imm(6, source); imm(7, dest); imm(1, count);
        if (width == 2) pushCode8(0x66);
        pushCode8(0xf3); pushCode8(width == 1 ? 0xa4 : 0xa5);
        pushCode8(0x9c); pushCode8(0x5d); // pushfd; pop ebp
        pushCode8(0x66); pushCode8(0x0f); pushCode8(0x7e); pushCode8(0xc2); // movd edx,xmm0
        pushCode8(0xd9); pushCode8(0x1d); testPushCode32(arena + 0x100 - segBase); // fstp float
        pushCode8(0xcd); pushCode8(0x97);
        DecodedOp* entry = cpu->getNextOp();
        for (DecodedOp* op = entry; op; op = op->next) {
            op->runCount = JIT_RUN_COUNT + 1;
            if (op->inst == Movsb || op->inst == Movsw || op->inst == Movsd) {
                if (width == 4) {
                    for (U32 i = 0; i < 20; ++i) profileMovsdCount(op, profile, true);
                } else {
                    op->STR_COUNT = 20; op->STR_TOTAL = profile * 20;
                }
                op->DF0 = op->DF1 = 1;
            }
            if (op->isBranch()) break;
        }
        startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
        runTestCPU();
        const U32 delta = backward ? -count * width : count * width;
        if (cpu->reg[R_AX].u32 != 0xffffffff || cpu->reg[R_CX].u32 ||
                cpu->reg[R_DX].u32 != 0x12345678 || cpu->reg[R_BX].u32 != 0xabcdef99 ||
                (cpu->reg[R_BP].u32 & (ARITH_FLAG_MASK | DF)) != (CF | PF | AF | SF | (backward ? DF : 0)) ||
                cpu->reg[R_SI].u32 != source + delta || cpu->reg[R_DI].u32 != dest + delta ||
                memory->readd(arena + 0x100) != 0x3f800000) {
            failed("REP live state profile=%u count=%u offset=%u DF=%u segments=%u", profile, count, offset, backward, segments);
            return;
        }
        for (U32 i = 0; i < count; ++i) {
            if (readMemoryValue(arena + 0xa000 + offset + i * width, width) != ((0x87654321 + i) & widthMask(width * 8))) {
                failed("REP live state copy result"); return;
            }
        }
    }
#endif
}

void runSharedMovsCodeWrite(U32 width) {
    newInstruction(0);
    memory->writed(TEST_HEAP_ADDRESS + 0x500, 0x12345678);
    cpu->reg[R_SI].u32 = 0x500;
    cpu->reg[R_DI].u32 = TEST_CODE_ADDRESS + (width == 2 ? 4 : 3);
    cpu->reg[R_CX].u32 = 4 / width;
    if (width == 2) pushCode8(0x66);
    pushCode8(0xf3); pushCode8(width == 1 ? 0xa4 : 0xa5);
    pushCode8(0xb8); testPushCode32(0xbad); // REP replaces this immediate.
    pushCode8(0xcd); pushCode8(0x97);
#ifdef BOXEDWINE_JIT
    DecodedOp* entry = cpu->getNextOp();
    entry->runCount = JIT_RUN_COUNT + 1;
    if (width == 4) {
        for (U32 i = 0; i < 20; ++i) profileMovsdCount(entry, 2048, true);
    } else {
        entry->STR_COUNT = 20; entry->STR_TOTAL = 2048 * 20;
    }
    startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
    if (!entry->pfnJitCode) failed("shared REP MOVS code-write case did not compile");
#endif
    runTestCPU();
    if (cpu->reg[R_AX].u32 != 0x12345678 || cpu->reg[R_CX].u32 ||
            cpu->reg[R_SI].u32 != 0x504 || cpu->reg[R_DI].u32 != TEST_CODE_ADDRESS + (width == 2 ? 8 : 7)) {
        failed("shared REP MOVS executed stale code after writing the next instruction");
    }
}

void runHotFillLoadLiveState(U32 width, bool store) {
#ifdef BOXEDWINE_JIT
    // Exercise the inline small loop and the RAM helper with live
    // GP/XMM/x87 state and lazy arithmetic flags on entry and continuation.
    for (bool backward : {false, true})
    for (bool segments : {false, true}) for (U32 offset : {0u, 4093u})
    for (U32 count : {0u, 1u, 7u, 32u, 1025u}) {
        newInstruction(backward ? DF : 0);
        cpu->fpu.FINIT();
        const U32 arena = TEST_HEAP_ADDRESS;
        const U32 segBase = segments ? arena : 0;
        cpu->seg[DS].address = cpu->seg[ES].address = segBase;
        cpu->thread->process->hasSetSeg[DS] = segments;
        cpu->thread->process->hasSetSeg[ES] = segments;
        const U32 last = backward && count ? (count - 1) * width : 0;
        const U32 source = count ? arena + 0x2000 + offset + last - segBase : 0xffffefffu;
        const U32 dest = count ? arena + 0xa000 + offset + last - segBase : 0xffffdfffu;
        for (U32 i = 0; i < count; ++i)
            writeMemoryValue(arena + 0x2000 + offset + i * width, width, 0x87654321 + i);
        auto imm = [](U32 reg, U32 value) { pushCode8(0xb8 + reg); testPushCode32(value); };
        imm(0, 0x12345678);
        pushCode8(0x66); pushCode8(0x0f); pushCode8(0x6e); pushCode8(0xc0); // movd xmm0,eax
        pushCode8(0xd9); pushCode8(0xe8); // fld1
        imm(0, 5); pushCode8(0x83); pushCode8(0xe8); pushCode8(6); // sub eax,6
        imm(0, 0x88776655); imm(3, 0xabcdef99); imm(6, source); imm(7, dest); imm(1, count);
        if (width == 2) pushCode8(0x66);
        pushCode8(0xf3); pushCode8((store ? 0xaa : 0xac) + (width != 1));
        pushCode8(0x9c); pushCode8(0x5d); // pushfd; pop ebp
        pushCode8(0x66); pushCode8(0x0f); pushCode8(0x7e); pushCode8(0xc2); // movd edx,xmm0
        pushCode8(0xd9); pushCode8(0x1d); testPushCode32(arena + 0x100 - segBase); // fstp float
        pushCode8(0xcd); pushCode8(0x97);
        DecodedOp* entry = cpu->getNextOp();
        for (DecodedOp* op = entry; op; op = op->next) {
            op->runCount = JIT_RUN_COUNT + 1;
            if (op->repZero || op->repNotZero) op->DF0 = op->DF1 = 1;
            if (op->isBranch()) break;
        }
        startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
        runTestCPU();
        const U32 delta = backward ? -count * width : count * width;
        U32 expectedEax = 0x88776655;
        if (!store && count) setAccumulator(expectedEax, width, 0x87654321 + (backward ? 0 : count - 1));
        if (cpu->reg[R_AX].u32 != expectedEax || cpu->reg[R_CX].u32 ||
                cpu->reg[R_DX].u32 != 0x12345678 || cpu->reg[R_BX].u32 != 0xabcdef99 ||
                (cpu->reg[R_BP].u32 & (ARITH_FLAG_MASK | DF)) != (CF | PF | AF | SF | (backward ? DF : 0)) ||
                cpu->reg[R_SI].u32 != source + (store ? 0 : delta) || cpu->reg[R_DI].u32 != dest + (store ? delta : 0) ||
                memory->readd(arena + 0x100) != 0x3f800000) {
            failed("REP STOS/LODS live state store=%u count=%u offset=%u DF=%u segments=%u", store, count, offset, backward, segments);
            return;
        }
        if (store) for (U32 i = 0; i < count; ++i) {
            if (readMemoryValue(arena + 0xa000 + offset + i * width, width) != (0x88776655 & widthMask(width * 8))) {
                failed("REP STOS/LODS live state copy result"); return;
            }
        }
    }
#endif
}

void runSharedFillLoadFaultCases(U32 width, bool store, bool flat) {
    for (bool backward : {false, true}) for (U32 count : {3u, 64u, 1025u})
    for (U32 unaligned = 0; unaligned < width; ++unaligned) {
        newInstruction(CF | ZF | (backward ? DF : 0));
        auto& ctx = testContext();
        cpu->seg[ES].address = TEST_HEAP_ADDRESS;
        ctx.process->hasSetSeg[ES] = true;
        const U32 flatBase = flat ? TEST_HEAP_ADDRESS : 0;
        const bool sourceFault = !store;
        if (flat) {
            cpu->seg[DS].address = cpu->seg[ES].address = 0;
            ctx.process->hasSetSeg[DS] = ctx.process->hasSetSeg[ES] = false;
        }
        const U32 src = (backward ? 0x2000u + width : 0x2000u - width * 2) + unaligned;
        const U32 dst = (backward ? 0x6000u + width : 0x6000u - width * 2) + unaligned;
        for (U32 i = 0; i < 0x2000; ++i) {
            memory->writeb(TEST_HEAP_ADDRESS + 0x1000 + i, (U8)(i * 37 + 9));
            memory->writeb(TEST_HEAP_ADDRESS + 0x5000 + i, 0x55);
        }
        cpu->reg[R_SI].u32 = src + flatBase;
        cpu->reg[R_DI].u32 = dst + flatBase;
        cpu->reg[R_CX].u32 = count;
        cpu->reg[R_AX].u32 = 0x88776655;
        cpu->reg[R_BX].u32 = 0x12345678;
        const U32 repAddress = ctx.codeIp;
        if (width == 2) pushCode8(0x66);
        pushCode8(0xf3); pushCode8((store ? 0xaa : 0xac) + (width != 1));
        pushCode8(0xbb); testPushCode32(0xbad);
        auto oldAction = ctx.process->sigActions[K_SIGSEGV];
        auto& action = ctx.process->sigActions[K_SIGSEGV];
        action.reset();
        action.flags = K_SA_SIGINFO;
        action.handlerAndSigAction = ctx.codeIp;
        pushCode8(0xcd); pushCode8(0x97);
#ifdef BOXEDWINE_JIT
        {
            DecodedOp* op = cpu->getNextOp();
            op->runCount = JIT_RUN_COUNT + 1;
            op->DF0 = !backward;
            op->DF1 = backward;
            startNewJIT(cpu, TEST_CODE_ADDRESS, op);
            if (!op->pfnJitCode) failed("REP STOS/LODS fault case did not compile");
        }
#endif
        const U32 page = TEST_HEAP_ADDRESS + (sourceFault
            ? (backward ? 0x1000 : 0x2000) : (backward ? 0x5000 : 0x6000));
        memory->mprotect(ctx.thread, page, K_PAGE_SIZE, sourceFault ? 0 : K_PROT_READ);
        runTestCPU();
        memory->mprotect(ctx.thread, page, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
        const U32 completed = backward ? (src - 0x2000) / width + 1 : (0x2000 - src) / width;
        const U32 advance = backward ? -completed * width : completed * width;
        U32 expectedEax = 0x88776655;
        if (!store && completed) setAccumulator(expectedEax, width, readMemoryValue(TEST_HEAP_ADDRESS + src + (backward ? -(completed - 1) * width : (completed - 1) * width), width));
        const U32 uc = cpu->reg[R_DX].u32;
        if (action.sigInfo[0] != K_SIGSEGV || !uc || memory->readd(uc + 0x28) != src + flatBase + (store ? 0 : advance) ||
                memory->readd(uc + 0x24) != dst + flatBase + (store ? advance : 0) || memory->readd(uc + 0x3c) != count - completed ||
                memory->readd(uc + 0x4c) != repAddress - TEST_CODE_ADDRESS ||
                memory->readd(uc + 0x34) != 0x12345678 || memory->readd(uc + 0x40) != expectedEax ||
                (memory->readd(uc + 0x54) & (CF | ZF | DF)) != (CF | ZF | (backward ? DF : 0))) {
            failed("shared REP STOS/LODS partial fault width=%u source=%u DF=%u offset=%u", width, sourceFault, backward, unaligned);
        }
        std::vector<U8> expected(0x2000, 0x55);
        if (store) for (U32 i = 0; i < completed; ++i) for (U32 j = 0; j < width; ++j) {
            const U32 delta = backward ? -i * width : i * width;
            expected[dst + delta + j - 0x5000] = (U8)(0x88776655 >> (j * 8));
        }
        std::vector<U8> actual(0x2000);
        memory->memcpy(actual.data(), TEST_HEAP_ADDRESS + 0x5000, 0x2000);
        if (actual != expected) failed("shared REP STOS/LODS fault changed uncommitted bytes");
        ctx.process->sigActions[K_SIGSEGV] = oldAction;
    }
}

void runSharedStosCodeWrite(U32 width) {
    newInstruction(0);
    cpu->reg[R_AX].u32 = 0x12345678;
    cpu->reg[R_SI].u32 = 0x500;
    cpu->reg[R_DI].u32 = TEST_CODE_ADDRESS + (width == 2 ? 4 : 3);
    cpu->reg[R_CX].u32 = 4 / width;
    if (width == 2) pushCode8(0x66);
    pushCode8(0xf3); pushCode8(width == 1 ? 0xaa : 0xab);
    pushCode8(0xb8); testPushCode32(0xbad); // REP replaces this immediate.
    pushCode8(0xcd); pushCode8(0x97);
#ifdef BOXEDWINE_JIT
    DecodedOp* entry = cpu->getNextOp();
    entry->runCount = JIT_RUN_COUNT + 1;
    startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
    if (!entry->pfnJitCode) failed("shared REP STOS code-write case did not compile");
#endif
    runTestCPU();
    const U32 expected = width == 1 ? 0x78787878 : width == 2 ? 0x56785678 : 0x12345678;
    if (cpu->reg[R_AX].u32 != expected || cpu->reg[R_CX].u32 ||
            cpu->reg[R_SI].u32 != 0x500 || cpu->reg[R_DI].u32 != TEST_CODE_ADDRESS + (width == 2 ? 8 : 7)) {
        failed("shared REP STOS code-write width=%u eax=%x expected=%x ecx=%x esi=%x edi=%x", width, cpu->reg[R_AX].u32, expected, cpu->reg[R_CX].u32, cpu->reg[R_SI].u32, cpu->reg[R_DI].u32);
    }
}

void runFillLoadSpecialPages(U32 width, bool store) {
#ifdef BOXEDWINE_JIT
    const U32 count = 64;
    const U32 buffer = TEST_HEAP_ADDRESS + 0x10000;
    auto compile = [&]() {
        DecodedOp* entry = cpu->getNextOp();
        for (DecodedOp* op = entry; op; op = op->next) {
            op->runCount = JIT_RUN_COUNT + 1;
            if (op->repZero || op->repNotZero) op->DF0 = op->DF1 = 1;
            if (op->isBranch()) break;
        }
        startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
        if (!entry->pfnJitCode) failed("STOS/LODS special-page test did not compile");
    };
    auto init = [&]() {
        newInstruction(CF | ZF);
        cpu->seg[DS].address = cpu->seg[ES].address = 0;
        cpu->thread->process->hasSetSeg[DS] = cpu->thread->process->hasSetSeg[ES] = false;
        cpu->reg[R_AX].u32 = 0x12345678;
        cpu->reg[R_CX].u32 = count;
        cpu->reg[R_SI].u32 = cpu->reg[R_DI].u32 = buffer;
        for (U32 i = 0; i < count; ++i) writeMemoryValue(buffer + i * width, width, 0x88776600 + i);
        if (width == 2) pushCode8(0x66);
        pushCode8(0xf3); pushCode8((store ? 0xaa : 0xac) + (width != 1));
    };
    if (store) {
        // A large helper call must not return to a block that REP just rewrote.
        init();
        const U32 dest = testContext().codeIp;
        cpu->reg[R_AX].u32 = 0x90909090;
        cpu->reg[R_DI].u32 = dest;
        pushCode8(0xb8); testPushCode32(0xbad);
        for (U32 i = 5; i < count * width; ++i) pushCode8(0x90);
        pushCode8(0xcd); pushCode8(0x97);
        compile();
        runTestCPU();
        if (cpu->reg[R_AX].u32 != 0x90909090 || cpu->reg[R_CX].u32 ||
                cpu->reg[R_DI].u32 != dest + count * width || cpu->reg[R_SI].u32 != buffer) {
            failed("REP STOS helper code-write width=%u", width);
        }

        init();
        auto* pages = getMemData(memory);
        const U32 alias = 0x30000000;
        RamPage ram = pages->mmu[buffer >> K_PAGE_SHIFT].getRamPageIndex();
        memory->mapPages(testContext().thread, alias >> K_PAGE_SHIFT, {ram}, PAGE_READ | PAGE_WRITE);
        pages->mmu[buffer >> K_PAGE_SHIFT].setPageType(memory, buffer >> K_PAGE_SHIFT, PageType::CopyOnWrite);
        pages->onPageChanged(buffer >> K_PAGE_SHIFT);
        pushCode8(0xcd); pushCode8(0x97);
        compile();
        runTestCPU();
        for (U32 i = 0; i < count; ++i) {
            if (readMemoryValue(buffer + i * width, width) != (0x12345678 & widthMask(width * 8)) ||
                    readMemoryValue(alias + i * width, width) != ((0x88776600 + i) & widthMask(width * 8))) {
                failed("REP STOS COW width=%u", width);
            }
        }
        memory->unmap(alias, K_PAGE_SIZE);
    }

    // Turn on a watchpoint after compilation, in the middle of the RAM span.
    // The optimized helper must leave it to the ordinary memory accessors.
    init();
    pushCode8(0xcd); pushCode8(0x97);
    auto& ctx = testContext();
    auto oldAction = ctx.process->sigActions[K_SIGTRAP];
    auto& action = ctx.process->sigActions[K_SIGTRAP];
    action.reset();
    action.flags = K_SA_SIGINFO;
    action.handlerAndSigAction = ctx.codeIp;
    pushCode8(0xcd); pushCode8(0x97);
    compile();
    ctx.thread->debugRegs[0] = buffer + 8 * width;
    ctx.thread->debugRegs[7] = 3 | ((store ? 1u : 3u) << 16);
    ctx.thread->updateDebugTrapActive();
    runTestCPU();
    const U32 uc = cpu->reg[R_DX].u32;
    U32 expectedEax = 0x12345678;
    if (!store) setAccumulator(expectedEax, width, 0x88776600 + count - 1);
    if (store && (action.sigInfo[0] != K_SIGTRAP || !uc ||
            memory->readd(uc + 0x28) != buffer + (store ? 0 : count * width) ||
            memory->readd(uc + 0x24) != buffer + (store ? count * width : 0) ||
            memory->readd(uc + 0x3c) != 0 || memory->readd(uc + 0x40) != expectedEax ||
            memory->readd(uc + 0x4c) != (width == 2 ? 3 : 2))) {
        failed("REP STOS/LODS watchpoint width=%u store=%u", width, store);
    }
    // Ordinary memory accessors currently report write watchpoints only.
    // LODS must still preserve scalar behavior with debugTrapActive set.
    if (!store && (cpu->reg[R_AX].u32 != expectedEax || cpu->reg[R_CX].u32 ||
            cpu->reg[R_SI].u32 != buffer + count * width || cpu->reg[R_DI].u32 != buffer ||
            (actualFlags(cpu, true) & (CF | ZF | DF)) != (CF | ZF))) {
        failed("REP LODS with debug watchpoint width=%u", width);
    }
    for (U32& reg : ctx.thread->debugRegs) reg = 0;
    ctx.thread->updateDebugTrapActive();
    ctx.process->sigActions[K_SIGTRAP] = oldAction;
#endif
}

void runHotCompareLiveState(U32 width, bool scan) {
#ifdef BOXEDWINE_JIT
    for (bool backward : {false, true}) for (bool segments : {false, true})
    for (bool keepEqual : {false, true}) for (U32 offset : {0u, 4093u})
    for (U32 count : {0u, 1u, 7u, 32u, 1025u}) for (U32 stopMode : {0u, 1u, 2u, 3u}) {
        newInstruction(backward ? DF : 0);
        cpu->fpu.FINIT();
        const U32 arena = TEST_HEAP_ADDRESS, segBase = segments ? arena : 0;
        cpu->seg[DS].address = cpu->seg[ES].address = segBase;
        cpu->thread->process->hasSetSeg[DS] = cpu->thread->process->hasSetSeg[ES] = segments;
        const U32 last = backward && count ? (count - 1) * width : 0;
        const U32 source = count ? arena + 0x2000 + offset + last - segBase : 0xffffefffu;
        const U32 dest = count ? arena + 0xa000 + offset + last - segBase : 0xffffdfffu;
        const U32 stop = stopMode == 0 ? count : stopMode == 1 ? 0 : stopMode == 2 ? count / 2 : count - 1;
        const U32 lhs = 0x88776655 & widthMask(width * 8);
        for (U32 i = 0; i < count; ++i) {
            const U32 delta = backward ? 0u - i * width : i * width;
            writeMemoryValue(segBase + source + delta, width, lhs);
            writeMemoryValue(segBase + dest + delta, width, lhs ^ ((keepEqual != (i == stop)) ? 0 : 0x81));
        }
        auto imm = [](U32 reg, U32 value) { pushCode8(0xb8 + reg); testPushCode32(value); };
        imm(0, 0x12345678);
        pushCode8(0x66); pushCode8(0x0f); pushCode8(0x6e); pushCode8(0xc0);
        pushCode8(0xd9); pushCode8(0xe8); // fld1
        imm(0, 5); pushCode8(0x83); pushCode8(0xe8); pushCode8(6);
        imm(0, 0x88776655); imm(3, 0xabcdef99); imm(6, source); imm(7, dest); imm(1, count);
        if (width == 2) pushCode8(0x66);
        pushCode8(keepEqual ? 0xf3 : 0xf2); pushCode8((scan ? 0xae : 0xa6) + (width != 1));
        pushCode8(0x9c); pushCode8(0x5d); // pushfd; pop ebp
        pushCode8(0x66); pushCode8(0x0f); pushCode8(0x7e); pushCode8(0xc2);
        pushCode8(0xd9); pushCode8(0x1d); testPushCode32(arena + 0x100 - segBase);
        pushCode8(0xcd); pushCode8(0x97);
        DecodedOp* entry = cpu->getNextOp();
        for (DecodedOp* op = entry; op; op = op->next) {
            op->runCount = JIT_RUN_COUNT + 1;
            if (op->repZero || op->repNotZero) op->DF0 = op->DF1 = 1;
            if (op->isBranch()) break;
        }
        startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
        if (!entry->pfnJitCode) failed("REP compare live-state case did not compile");
        runTestCPU();
        const U32 done = std::min(count, stop + 1), delta = backward ? 0u - done * width : done * width;
        U32 flags = CF | PF | AF | SF;
        if (done) {
            const U32 rhs = lhs ^ ((keepEqual != (done - 1 == stop)) ? 0 : 0x81);
            flags = subFlags(lhs, rhs, lhs - rhs, width * 8);
        }
        flags |= backward ? DF : 0;
        if (cpu->reg[R_AX].u32 != 0x88776655 || cpu->reg[R_CX].u32 != count - done ||
                cpu->reg[R_DX].u32 != 0x12345678 || cpu->reg[R_BX].u32 != 0xabcdef99 ||
                (cpu->reg[R_BP].u32 & (ARITH_FLAG_MASK | DF)) != flags ||
                cpu->reg[R_SI].u32 != source + (scan ? 0 : delta) || cpu->reg[R_DI].u32 != dest + delta ||
                memory->readd(arena + 0x100) != 0x3f800000) {
            failed("REP compare live state width=%u scan=%u count=%u stop=%u offset=%u DF=%u segments=%u equal=%u ecx=%x flags=%x expected=%x",
                width, scan, count, stopMode, offset, backward, segments, keepEqual,
                cpu->reg[R_CX].u32, cpu->reg[R_BP].u32 & FLAG_MASK, flags);
            return;
        }
    }
#endif
}

void runCompareFaultCases(U32 width, bool scan) {
    for (bool backward : {false, true}) for (bool flat : {false, true})
    for (bool keepEqual : {false, true}) for (bool sourceFault : {false, true})
    for (bool stopBeforeFault : {false, true})
    for (U32 count : {3u, 64u, 1025u}) for (U32 unaligned = 0; unaligned < width; ++unaligned) {
        if (scan && sourceFault) continue;
        newInstruction(backward ? DF : 0);
        auto& ctx = testContext();
        const U32 segBase = flat ? 0 : TEST_HEAP_ADDRESS;
        cpu->seg[DS].address = cpu->seg[ES].address = segBase;
        ctx.process->hasSetSeg[DS] = ctx.process->hasSetSeg[ES] = !flat;
        const U32 src = (backward ? 0x2000u + width : 0x2000u - width * 2) + unaligned;
        const U32 dst = src + 0x4000;
        // Byte patterns stay equal/unequal even for unaligned words/dwords.
        for (U32 i = 0; i < 0x2000; ++i) {
            memory->writeb(TEST_HEAP_ADDRESS + 0x1000 + i, 0x55);
            memory->writeb(TEST_HEAP_ADDRESS + 0x5000 + i, keepEqual ? 0x55 : 0x66);
        }
        const U32 source = src + TEST_HEAP_ADDRESS - segBase, dest = dst + TEST_HEAP_ADDRESS - segBase;
        const U32 accessible = backward ? (src - 0x2000) / width + 1 : (0x2000 - src) / width;
        const U32 lhs = 0x55555555 & widthMask(width * 8);
        const U32 rhs = keepEqual ? lhs ^ 1u : lhs;
        if (stopBeforeFault) {
            // A SIMD read crosses the protected page, but the last accessible
            // element stops REP. The speculative wide access must not fault.
            const U32 delta = (accessible - 1) * width;
            writeMemoryValue(segBase + dest + (backward ? 0u - delta : delta), width, rhs);
        }
        auto imm = [](U32 reg, U32 value) { pushCode8(0xb8 + reg); testPushCode32(value); };
        imm(0, 5); pushCode8(0x83); pushCode8(0xe8); pushCode8(6); // lazy entry flags
        imm(0, 0x55555555); imm(3, 0x12345678); imm(6, source); imm(7, dest); imm(1, count);
        const U32 repAddress = ctx.codeIp;
        if (width == 2) pushCode8(0x66);
        pushCode8(keepEqual ? 0xf3 : 0xf2); pushCode8((scan ? 0xae : 0xa6) + (width != 1));
        imm(3, 0xbad);
        auto oldAction = ctx.process->sigActions[K_SIGSEGV];
        auto& action = ctx.process->sigActions[K_SIGSEGV];
        action.reset(); action.flags = K_SA_SIGINFO; action.handlerAndSigAction = ctx.codeIp;
        pushCode8(0xcd); pushCode8(0x97);
#ifdef BOXEDWINE_JIT
        DecodedOp* entry = cpu->getNextOp();
        for (DecodedOp* op = entry; op; op = op->next) {
            op->runCount = JIT_RUN_COUNT + 1;
            if (op->repZero || op->repNotZero) op->DF0 = op->DF1 = 1;
            if (op->isBranch()) break;
        }
        startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
#endif
        const U32 page = TEST_HEAP_ADDRESS + (sourceFault ? (backward ? 0x1000 : 0x2000) : (backward ? 0x5000 : 0x6000));
        memory->mprotect(ctx.thread, page, K_PAGE_SIZE, 0);
        runTestCPU();
        memory->mprotect(ctx.thread, page, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
        const U32 completed = backward ? (src - 0x2000) / width + 1 : (0x2000 - src) / width;
        const U32 advance = backward ? 0u - completed * width : completed * width;
        const U32 uc = action.sigInfo[0] == K_SIGSEGV ? cpu->reg[R_DX].u32 : 0;
        const bool valid = action.sigInfo[0] == K_SIGSEGV && uc &&
            memory->readd(uc + 0x28) == source + (scan ? 0 : advance) &&
            memory->readd(uc + 0x24) == dest + advance && memory->readd(uc + 0x3c) == count - completed &&
            memory->readd(uc + 0x4c) == repAddress - TEST_CODE_ADDRESS &&
            memory->readd(uc + 0x34) == 0x12345678 && memory->readd(uc + 0x40) == 0x55555555 &&
            (memory->readd(uc + 0x54) & FLAG_MASK) == (CF | PF | AF | SF | (backward ? DF : 0));
        const bool stopped = action.sigInfo[0] != K_SIGSEGV && cpu->reg[R_BX].u32 == 0xbad &&
            cpu->reg[R_SI].u32 == source + (scan ? 0 : advance) && cpu->reg[R_DI].u32 == dest + advance &&
            cpu->reg[R_CX].u32 == count - completed && cpu->reg[R_AX].u32 == 0x55555555 &&
            (actualFlags(cpu, true) & FLAG_MASK) == (subFlags(lhs, rhs, lhs - rhs, width * 8) | (backward ? DF : 0));
        ctx.process->sigActions[K_SIGSEGV] = oldAction;
        if (stopBeforeFault ? !stopped : !valid) {
            failed("REP compare fault width=%u scan=%u DF=%u flat=%u equal=%u source=%u count=%u offset=%u stopBeforeFault=%u ecx=%x expected=%x esi=%x edi=%x flags=%x eip=%x expectedEip=%x",
                width, scan, backward, flat, keepEqual, sourceFault, count, unaligned, stopBeforeFault,
                uc ? memory->readd(uc + 0x3c) : 0, count - completed,
                uc ? memory->readd(uc + 0x28) : 0, uc ? memory->readd(uc + 0x24) : 0,
                uc ? memory->readd(uc + 0x54) : 0, uc ? memory->readd(uc + 0x4c) : 0, repAddress - TEST_CODE_ADDRESS);
            return;
        }
    }
}

void runHotMovsDirectionSwitch(int width) {
#ifdef BOXEDWINE_JIT
    for (bool profileBackward : {false, true}) {
        for (U32 separation : {0u, 1u, 7u, 80u}) {
            newInstruction(0);
            Seg savedDs = cpu->seg[DS], savedEs = cpu->seg[ES];
            bool savedHasDs = cpu->thread->process->hasSetSeg[DS];
            bool savedHasEs = cpu->thread->process->hasSetSeg[ES];
            cpu->seg[DS].address = cpu->seg[ES].address = 0;
            cpu->thread->process->hasSetSeg[DS] = cpu->thread->process->hasSetSeg[ES] = false;
            if (width == 2) pushCode8(0x66);
            emitCode(STRING_MOVS, width, PREFIX_REPE);
            pushCode8(0xcd); pushCode8(0x97);
            DecodedOp* op = cpu->getNextOp();
            op->runCount = JIT_RUN_COUNT + 1;
            op->DF0 = !profileBackward; op->DF1 = profileBackward;
            op->STR_COUNT = 20; op->STR_TOTAL = 20;
            if (width == 4) { op->STR_FLAGS = STR_WIDE_COPY; op->MOVSD_SIZE_HITS = 0; }
            startNewJIT(cpu, TEST_CODE_ADDRESS, op);
            auto entry = op->pfnJitCode;
            if (!entry || (op->flags2 & OP_FLAG2_TRACED_STUB)) failed("MOVS direction switch missing JIT");
            for (bool backward : {profileBackward, !profileBackward, profileBackward}) {
                for (U32 count : {0u, 1u, 3u, 17u}) {
                    U8 expected[OVERLAP_SIZE];
                    initOverlapBytes(expected, sizeof(expected));
                    writeOverlapBytes(expected, sizeof(expected));
                    U32 src = 80 + (backward ? separation : 0);
                    U32 dst = 80 + (backward ? 0 : separation);
                    cpu->eip.u32 = 0;
                    U32 flags = ARITH_FLAG_MASK | (backward ? DF : 0);
                    cpu->setFlags(flags, FMASK_ALL);
                    cpu->reg[R_SI].u32 = TEST_HEAP_ADDRESS + OVERLAP_BASE + src;
                    cpu->reg[R_DI].u32 = TEST_HEAP_ADDRESS + OVERLAP_BASE + dst;
                    cpu->reg[R_CX].u32 = count;
                    for (U32 i = 0; i < count; ++i) {
                        writeCaseValue(expected, dst, width, readCaseValue(expected, src, width));
                        src += backward ? 0u - width : width;
                        dst += backward ? 0u - width : width;
                    }
                    runTestCPU();
                    verifyOverlapBytes(expected, sizeof(expected), "MOVS changed direction at same site");
                    if (cpu->reg[R_CX].u32 || cpu->reg[R_SI].u32 != TEST_HEAP_ADDRESS + OVERLAP_BASE + src ||
                            cpu->reg[R_DI].u32 != TEST_HEAP_ADDRESS + OVERLAP_BASE + dst ||
                            (actualFlags(cpu, true) & FLAG_MASK) != flags ||
                            movsCodeChangedWithoutHostFault(op, entry)) {
                        failed("MOVS direction switch width=%d count=%u profileDF=%d actualDF=%d separation=%u ecx=%x esi=%x/%x edi=%x/%x flags=%x/%x sameEntry=%u",
                            width, count, profileBackward, backward, separation, cpu->reg[R_CX].u32,
                            cpu->reg[R_SI].u32, TEST_HEAP_ADDRESS + OVERLAP_BASE + src,
                            cpu->reg[R_DI].u32, TEST_HEAP_ADDRESS + OVERLAP_BASE + dst,
                            actualFlags(cpu, true) & FLAG_MASK, flags, op->pfnJitCode == entry);
                    }
                }
            }
            cpu->seg[DS] = savedDs; cpu->seg[ES] = savedEs;
            cpu->thread->process->hasSetSeg[DS] = savedHasDs;
            cpu->thread->process->hasSetSeg[ES] = savedHasEs;
        }
    }
#endif
}

} // namespace

void testMovsb_0x0a4() {
    runStringCases(STRING_MOVS, 1, false, MOVE_CASES, caseCount(MOVE_CASES));
    runOverlapMovsCases(1, false);
    runHotMovsCases(1, false);
    runPageBoundaryCases(STRING_MOVS, 1, false);
}

void testMovsb_0x2a4() {
    runHotMovsDirectionSwitch(1);
    runHotMovsLiveState(1);
    runSharedMovsFaultCases(1);
    runSharedMovsFaultCases(1, true);
    runSharedMovsFaultCases(1, true, true);
    runSharedMovsCodeWrite(1);
    runHotFlatMovsAliasedOverlap(1);
    runHotFlatMovsAliasedOverlap(1, true);
    runStringCases(STRING_MOVS, 1, true, MOVE_CASES, caseCount(MOVE_CASES));
    runOverlapMovsCases(1, true);
    runHotMovsCases(1, true);
    runHotFlatOverlapMovsCases(1, 1);
    runHotFlatOverlapMovsCases(1, 20);
    runHotFlatOverlapMovsCases(1, 20, 1);
    runHotFlatOverlapMovsCases(1, 20, 1, true);
    runPageBoundaryCases(STRING_MOVS, 1, true);
}

void testMovsw_0x0a5() {
    runHotMovsDirectionSwitch(2);
    runHotMovsLiveState(2);
    runSharedMovsFaultCases(2);
    runSharedMovsFaultCases(2, true);
    runSharedMovsFaultCases(2, true, true);
    runSharedMovsCodeWrite(2);
    runHotFlatMovsAliasedOverlap(2);
    runHotFlatMovsAliasedOverlap(2, true);
    runStringCases(STRING_MOVS, 2, false, MOVE_CASES, caseCount(MOVE_CASES));
    runOverlapMovsCases(2, false);
    runHotMovsCases(2, false);
    runHotFlatOverlapMovsCases(2, 1);
    runHotFlatOverlapMovsCases(2, 20);
    runHotFlatOverlapMovsCases(2, 20, 1);
    runPageBoundaryCases(STRING_MOVS, 2, false);
}

void testMovsd_0x2a5() {
    runHotMovsDirectionSwitch(4);
    runHotMovsLiveState(4);
    runSharedMovsdCases();
    runHotFlatMovsdMixedCounts();
    runHotFlatMovsAliasedOverlap(4);
    runHotFlatMovsAliasedOverlap(4, true);
    runSharedMovsFaultCases(4);
    runSharedMovsFaultCases(4, true);
    runSharedMovsFaultCases(4, true, true);
    runSharedMovsCodeWrite(4);
#ifdef BOXEDWINE_JIT
    runMovsdCountSelection();
    if (MOVSD_UNROLL_SUPPORTED) runRepSpanSafety();
#endif
    runStringCases(STRING_MOVS, 4, true, MOVE_CASES, caseCount(MOVE_CASES));
    runOverlapMovsCases(4, true);
    runHotMovsCases(4, true);
    runHotFlatOverlapMovsCases(4, 1);
    runHotFlatOverlapMovsCases(4, 20);
    runHotFlatOverlapMovsCases(4, 20, 1);
    runPageBoundaryCases(STRING_MOVS, 4, true);
}

void testCmpsb_0x0a6() {
    runStringCases(STRING_CMPS, 1, false, COMPARE_CASES, caseCount(COMPARE_CASES));
    runPageBoundaryCases(STRING_CMPS, 1, false);
}

void testCmpsb_0x2a6() {
    runHotCompareLiveState(1, false);
    runCompareFaultCases(1, false);
    runStringCases(STRING_CMPS, 1, true, COMPARE_CASES, caseCount(COMPARE_CASES));
    runPageBoundaryCases(STRING_CMPS, 1, true);
}

void testCmpsw_0x0a7() {
    runHotCompareLiveState(2, false);
    runCompareFaultCases(2, false);
    runStringCases(STRING_CMPS, 2, false, COMPARE_CASES, caseCount(COMPARE_CASES));
    runPageBoundaryCases(STRING_CMPS, 2, false);
}

void testCmpsd_0x2a7() {
    runHotCompareLiveState(4, false);
    runCompareFaultCases(4, false);
    runStringCases(STRING_CMPS, 4, true, COMPARE_CASES, caseCount(COMPARE_CASES));
    runPageBoundaryCases(STRING_CMPS, 4, true);
}

void testStosb_0x0aa() {
    runStringCases(STRING_STOS, 1, false, STORE_CASES, caseCount(STORE_CASES));
    runPageBoundaryCases(STRING_STOS, 1, false);
}

void testStosb_0x2aa() {
    runHotFillLoadLiveState(1, true);
    runSharedFillLoadFaultCases(1, true, false);
    runSharedFillLoadFaultCases(1, true, true);
    runFillLoadSpecialPages(1, true);
    runSharedStosCodeWrite(1);
    runStringCases(STRING_STOS, 1, true, STORE_CASES, caseCount(STORE_CASES));
    runPageBoundaryCases(STRING_STOS, 1, true);
}

void testStosw_0x0ab() {
    runHotFillLoadLiveState(2, true);
    runSharedFillLoadFaultCases(2, true, false);
    runSharedFillLoadFaultCases(2, true, true);
    runFillLoadSpecialPages(2, true);
    runSharedStosCodeWrite(2);
    runStringCases(STRING_STOS, 2, false, STORE_CASES, caseCount(STORE_CASES));
    runPageBoundaryCases(STRING_STOS, 2, false);
}

void testStosd_0x2ab() {
    runHotFillLoadLiveState(4, true);
    runSharedFillLoadFaultCases(4, true, false);
    runSharedFillLoadFaultCases(4, true, true);
    runFillLoadSpecialPages(4, true);
    runSharedStosCodeWrite(4);
    runStringCases(STRING_STOS, 4, true, STORE_CASES, caseCount(STORE_CASES));
    runPageBoundaryCases(STRING_STOS, 4, true);
}

void testLodsb_0x0ac() {
    runStringCases(STRING_LODS, 1, false, LOAD_CASES, caseCount(LOAD_CASES));
    runPageBoundaryCases(STRING_LODS, 1, false);
}

void testLodsb_0x2ac() {
    runHotFillLoadLiveState(1, false);
    runSharedFillLoadFaultCases(1, false, false);
    runSharedFillLoadFaultCases(1, false, true);
    runFillLoadSpecialPages(1, false);
    runStringCases(STRING_LODS, 1, true, LOAD_CASES, caseCount(LOAD_CASES));
    runPageBoundaryCases(STRING_LODS, 1, true);
}

void testLodsw_0x0ad() {
    runHotFillLoadLiveState(2, false);
    runSharedFillLoadFaultCases(2, false, false);
    runSharedFillLoadFaultCases(2, false, true);
    runFillLoadSpecialPages(2, false);
    runStringCases(STRING_LODS, 2, false, LOAD_CASES, caseCount(LOAD_CASES));
    runPageBoundaryCases(STRING_LODS, 2, false);
}

void testLodsd_0x2ad() {
    runHotFillLoadLiveState(4, false);
    runSharedFillLoadFaultCases(4, false, false);
    runSharedFillLoadFaultCases(4, false, true);
    runFillLoadSpecialPages(4, false);
    runStringCases(STRING_LODS, 4, true, LOAD_CASES, caseCount(LOAD_CASES));
    runPageBoundaryCases(STRING_LODS, 4, true);
}

void testScasb_0x0ae() {
    runStringCases(STRING_SCAS, 1, false, SCAN_CASES, caseCount(SCAN_CASES));
    runPageBoundaryCases(STRING_SCAS, 1, false);
}

void testScasb_0x2ae() {
    runHotCompareLiveState(1, true);
    runCompareFaultCases(1, true);
    runStringCases(STRING_SCAS, 1, true, SCAN_CASES, caseCount(SCAN_CASES));
    runPageBoundaryCases(STRING_SCAS, 1, true);
}

void testScasw_0x0af() {
    runHotCompareLiveState(2, true);
    runCompareFaultCases(2, true);
    runStringCases(STRING_SCAS, 2, false, SCAN_CASES, caseCount(SCAN_CASES));
    runPageBoundaryCases(STRING_SCAS, 2, false);
}

void testScasd_0x2af() {
    runHotCompareLiveState(4, true);
    runCompareFaultCases(4, true);
    runStringCases(STRING_SCAS, 4, true, SCAN_CASES, caseCount(SCAN_CASES));
    runPageBoundaryCases(STRING_SCAS, 4, true);
}

#endif
