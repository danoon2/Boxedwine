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

#include "testSSE.h"
#include "testCPU.h"
#include "testX86Util.h"
#include <bit>

#define cpu (testContext().cpu)
#define memory (testContext().memory)
#define pushCode8 testPushCode8
#define pushCode32 testPushCode32
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

constexpr U32 MEM_BASE = 0x10000;
constexpr U32 MEM_SRC = MEM_BASE + 0x100;
constexpr U32 MEM_DST = MEM_BASE + 0x180;
constexpr U64 XMM_DEFAULT_LOW = 0x1234567890abcdefULL;
constexpr U64 XMM_DEFAULT_HIGH = 0x24680bdf13579aceULL;
constexpr U64 XMM_SRC_LOW = 0xaabbccddeeff2468ULL;
constexpr U64 XMM_SRC_HIGH = 0x1122334455667788ULL;
constexpr U64 XMM_ALT_LOW = 0x0f0e0d0c0b0a0908ULL;
constexpr U64 XMM_ALT_HIGH = 0x8070605040302010ULL;
constexpr U64 MEM_GUARD_LOW = 0x9999999999999999ULL;
constexpr U64 MEM_GUARD_HIGH = 0x7777777777777777ULL;
constexpr U32 REG_GUARD = 0xa55a0000;
constexpr U64 MMX_DEFAULT = 0x0123456789abcdefULL;
constexpr U32 SSE_FLAG_MASK = CF | PF | AF | ZF | SF | OF;

void emitDirectAddressModRM(int regField, U32 address) {
    pushCode8(0x04 | (regField << 3));
    pushCode8(0x25);
    pushCode32(address);
}

void initSse() {
    newInstruction(0);
    cpu->big = 1;
    for (int i = 0; i < 8; ++i) {
        cpu->xmm[i].pi.u64[0] = XMM_DEFAULT_LOW;
        cpu->xmm[i].pi.u64[1] = XMM_DEFAULT_HIGH;
    }
    memory->writeq(TEST_HEAP_ADDRESS + MEM_SRC, XMM_SRC_LOW);
    memory->writeq(TEST_HEAP_ADDRESS + MEM_SRC + 8, XMM_SRC_HIGH);
    memory->writeq(TEST_HEAP_ADDRESS + MEM_DST, MEM_GUARD_LOW);
    memory->writeq(TEST_HEAP_ADDRESS + MEM_DST + 8, MEM_GUARD_HIGH);
}

void setXmm(int reg, U64 low, U64 high) {
    cpu->xmm[reg].pi.u64[0] = low;
    cpu->xmm[reg].pi.u64[1] = high;
}

void verifyOnlyXmmChanged(int changed, U64 low, U64 high, const char* name) {
    for (int i = 0; i < 8; ++i) {
        if (i == changed) {
            if (cpu->xmm[i].pi.u64[0] != low || cpu->xmm[i].pi.u64[1] != high) {
                failed("%s xmm value", name);
            }
        } else if (cpu->xmm[i].pi.u64[0] != XMM_DEFAULT_LOW || cpu->xmm[i].pi.u64[1] != XMM_DEFAULT_HIGH) {
            failed("%s xmm unchanged", name);
        }
    }
}

void verifyXmmMove(int dst, U64 dstLow, U64 dstHigh, int src, U64 srcLow, U64 srcHigh, const char* name) {
    for (int i = 0; i < 8; ++i) {
        U64 expectedLow = XMM_DEFAULT_LOW;
        U64 expectedHigh = XMM_DEFAULT_HIGH;
        if (i == dst) {
            expectedLow = dstLow;
            expectedHigh = dstHigh;
        } else if (i == src) {
            continue;
        }
        if (cpu->xmm[i].pi.u64[0] != expectedLow || cpu->xmm[i].pi.u64[1] != expectedHigh) {
            failed("%s xmm move", name);
        }
    }
}

void emitSseRegReg(U8 prefix, U8 opcode, int dst, int src) {
    if (prefix) {
        pushCode8(prefix);
    }
    pushCode8(0x0f);
    pushCode8(opcode);
    pushCode8(0xc0 | (dst << 3) | src);
}

void emitSseRegReg(U8 prefix1, U8 prefix2, U8 opcode, int dst, int src) {
    if (prefix1) {
        pushCode8(prefix1);
    }
    emitSseRegReg(prefix2, opcode, dst, src);
}

void emitSseRegRegRmDst(U8 prefix, U8 opcode, int dstRm, int srcReg) {
    if (prefix) {
        pushCode8(prefix);
    }
    pushCode8(0x0f);
    pushCode8(opcode);
    pushCode8(0xc0 | (srcReg << 3) | dstRm);
}

void emitSseRegMem(U8 prefix, U8 opcode, int dst, U32 address) {
    if (prefix) {
        pushCode8(prefix);
    }
    pushCode8(0x0f);
    pushCode8(opcode);
    emitDirectAddressModRM(dst, address);
}

void emitSseRegMem(U8 prefix1, U8 prefix2, U8 opcode, int dst, U32 address) {
    if (prefix1) {
        pushCode8(prefix1);
    }
    emitSseRegMem(prefix2, opcode, dst, address);
}

void emitSseMemReg(U8 prefix, U8 opcode, int src, U32 address) {
    if (prefix) {
        pushCode8(prefix);
    }
    pushCode8(0x0f);
    pushCode8(opcode);
    emitDirectAddressModRM(src, address);
}

void emitSseMemReg(U8 prefix1, U8 prefix2, U8 opcode, int src, U32 address) {
    if (prefix1) {
        pushCode8(prefix1);
    }
    emitSseMemReg(prefix2, opcode, src, address);
}

void emitSseRegRegImm(U8 prefix, U8 opcode, int dst, int src, U8 imm) {
    emitSseRegReg(prefix, opcode, dst, src);
    pushCode8(imm);
}

void emitSseRegMemImm(U8 prefix, U8 opcode, int dst, U32 address, U8 imm) {
    emitSseRegMem(prefix, opcode, dst, address);
    pushCode8(imm);
}

void emitMmxRegReg(U8 prefix, U8 opcode, int dst, int src) {
    if (prefix) {
        pushCode8(prefix);
    }
    pushCode8(0x0f);
    pushCode8(opcode);
    pushCode8(0xc0 | (dst << 3) | src);
}

void emitMmxRegMem(U8 prefix, U8 opcode, int dst, U32 address) {
    if (prefix) {
        pushCode8(prefix);
    }
    pushCode8(0x0f);
    pushCode8(opcode);
    emitDirectAddressModRM(dst, address);
}

void emitMmxRegRegImm(U8 prefix, U8 opcode, int dst, int src, U8 imm) {
    emitMmxRegReg(prefix, opcode, dst, src);
    pushCode8(imm);
}

void emitMmxRegMemImm(U8 prefix, U8 opcode, int dst, U32 address, U8 imm) {
    emitMmxRegMem(prefix, opcode, dst, address);
    pushCode8(imm);
}

void emitStoreReg32ToMem(int reg, U32 address) {
    pushCode8(0x89);
    emitDirectAddressModRM(reg, address);
}

void emitRestoreEsp() {
    pushCode8(0xbc);
    pushCode32(4096);
}

U32 bitsToFloat(U64 value, int lane) {
    return (U32)(value >> (lane * 32));
}

U64 packFloatBits(U32 lane0, U32 lane1) {
    return ((U64)lane1 << 32) | lane0;
}

float floatFromBits(U32 value) {
    return std::bit_cast<float>(value);
}

U32 bitsFromFloat(float value) {
    return std::bit_cast<U32>(value);
}

U64 bitsFromDouble(double value) {
    return std::bit_cast<U64>(value);
}

void addpsExpected(U64 aLow, U64 aHigh, U64 bLow, U64 bHigh, U64& resultLow, U64& resultHigh) {
    U32 lanes[4];
    lanes[0] = bitsFromFloat(floatFromBits(bitsToFloat(aLow, 0)) + floatFromBits(bitsToFloat(bLow, 0)));
    lanes[1] = bitsFromFloat(floatFromBits(bitsToFloat(aLow, 1)) + floatFromBits(bitsToFloat(bLow, 1)));
    lanes[2] = bitsFromFloat(floatFromBits(bitsToFloat(aHigh, 0)) + floatFromBits(bitsToFloat(bHigh, 0)));
    lanes[3] = bitsFromFloat(floatFromBits(bitsToFloat(aHigh, 1)) + floatFromBits(bitsToFloat(bHigh, 1)));
    resultLow = packFloatBits(lanes[0], lanes[1]);
    resultHigh = packFloatBits(lanes[2], lanes[3]);
}

void writeFloatVector(U32 address, float a, float b, float c, float d) {
    memory->writed(TEST_HEAP_ADDRESS + address, bitsFromFloat(a));
    memory->writed(TEST_HEAP_ADDRESS + address + 4, bitsFromFloat(b));
    memory->writed(TEST_HEAP_ADDRESS + address + 8, bitsFromFloat(c));
    memory->writed(TEST_HEAP_ADDRESS + address + 12, bitsFromFloat(d));
}

void setXmmFloatVector(int reg, float a, float b, float c, float d) {
    U64 low = packFloatBits(bitsFromFloat(a), bitsFromFloat(b));
    U64 high = packFloatBits(bitsFromFloat(c), bitsFromFloat(d));
    setXmm(reg, low, high);
}

void initSseMmx() {
    initSse();
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x100 + i);
        cpu->fpu.getMMX(i)->q = MMX_DEFAULT;
    }
}

void writeXmmMem(U32 address, U64 low, U64 high) {
    memory->writeq(TEST_HEAP_ADDRESS + address, low);
    memory->writeq(TEST_HEAP_ADDRESS + address + 8, high);
}

void runSse128(U8 prefix1, U8 prefix2, U8 opcode, U64 dstLow, U64 dstHigh, U64 srcLow, U64 srcHigh, U64 expectedLow, U64 expectedHigh, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            if (dst == src) {
                continue;
            }
            initSse();
            setXmm(dst, dstLow, dstHigh);
            setXmm(src, srcLow, srcHigh);
            emitSseRegReg(prefix1, prefix2, opcode, dst, src);
            runTestCPU();
            verifyXmmMove(dst, expectedLow, expectedHigh, src, srcLow, srcHigh, name);
        }

        initSse();
        setXmm(dst, dstLow, dstHigh);
        writeXmmMem(MEM_SRC, srcLow, srcHigh);
        emitSseRegMem(prefix1, prefix2, opcode, dst, MEM_SRC);
        runTestCPU();
        verifyOnlyXmmChanged(dst, expectedLow, expectedHigh, name);
    }
}

void runSse128RegOnly(U8 prefix1, U8 prefix2, U8 opcode, U64 dstLow, U64 dstHigh, U64 srcLow, U64 srcHigh, U64 expectedLow, U64 expectedHigh, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            if (dst == src) {
                continue;
            }
            initSse();
            setXmm(dst, dstLow, dstHigh);
            setXmm(src, srcLow, srcHigh);
            emitSseRegReg(prefix1, prefix2, opcode, dst, src);
            runTestCPU();
            verifyXmmMove(dst, expectedLow, expectedHigh, src, srcLow, srcHigh, name);
        }
    }
}

void runSse128Store(U8 prefix1, U8 prefix2, U8 opcode, U64 memLow, U64 memHigh, U64 srcLow, U64 srcHigh, U64 expectedLow, U64 expectedHigh, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            if (dst == src) {
                continue;
            }
            initSse();
            setXmm(dst, memLow, memHigh);
            setXmm(src, srcLow, srcHigh);
            emitSseRegRegRmDst(prefix2, opcode, dst, src);
            runTestCPU();
            verifyXmmMove(dst, expectedLow, expectedHigh, src, srcLow, srcHigh, name);
        }

        initSse();
        writeXmmMem(MEM_DST, memLow, memHigh);
        setXmm(dst, srcLow, srcHigh);
        emitSseMemReg(prefix1, prefix2, opcode, dst, MEM_DST);
        runTestCPU();
        if (memory->readq(TEST_HEAP_ADDRESS + MEM_DST) != expectedLow ||
                memory->readq(TEST_HEAP_ADDRESS + MEM_DST + 8) != expectedHigh) {
            failed("%s mem", name);
        }
    }
}

void runSseCompare(U8 opcode, U32 lhsBits, U32 rhsBits, U32 expectedFlags, const char* name) {
    U64 lhsLow = ((U64)0x11111111 << 32) | lhsBits;
    U64 rhsLow = ((U64)0x22222222 << 32) | rhsBits;
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            if (dst == src) {
                continue;
            }
            initSse();
            setXmm(dst, lhsLow, 0x3333333344444444ULL);
            setXmm(src, rhsLow, 0x5555555566666666ULL);
            cpu->flags = SSE_FLAG_MASK | DF;
            emitSseRegReg(0, opcode, dst, src);
            runTestCPU();
            if (((TestX86::actualFlags(cpu, true) ^ expectedFlags) & (SSE_FLAG_MASK | DF)) != 0) {
                failed("%s reg flags", name);
            }
            verifyXmmMove(dst, lhsLow, 0x3333333344444444ULL, src, rhsLow, 0x5555555566666666ULL, name);
        }

        initSse();
        setXmm(dst, lhsLow, 0x3333333344444444ULL);
        writeXmmMem(MEM_SRC, rhsLow, 0x5555555566666666ULL);
        cpu->flags = SSE_FLAG_MASK | DF;
        emitSseRegMem(0, opcode, dst, MEM_SRC);
        runTestCPU();
        if (((TestX86::actualFlags(cpu, true) ^ expectedFlags) & (SSE_FLAG_MASK | DF)) != 0) {
            failed("%s mem flags", name);
        }
        verifyOnlyXmmChanged(dst, lhsLow, 0x3333333344444444ULL, name);
    }
}

void runCvtsi2ss(U32 value, const char* name) {
    U32 expected = bitsFromFloat((float)(S32)value);
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            initSseMmx();
            setXmm(dst, XMM_SRC_LOW, XMM_SRC_HIGH);
            cpu->reg[src].u32 = value;
            emitSseRegReg(0xf3, 0x2a, dst, src);
            runTestCPU();
            verifyOnlyXmmChanged(dst, (XMM_SRC_LOW & 0xffffffff00000000ULL) | expected, XMM_SRC_HIGH, name);
            if (cpu->reg[src].u32 != value) {
                failed("%s gpr unchanged", name);
            }
        }

        initSseMmx();
        setXmm(dst, XMM_SRC_LOW, XMM_SRC_HIGH);
        memory->writed(TEST_HEAP_ADDRESS + MEM_SRC, value);
        emitSseRegMem(0, 0xf3, 0x2a, dst, MEM_SRC);
        runTestCPU();
        verifyOnlyXmmChanged(dst, (XMM_SRC_LOW & 0xffffffff00000000ULL) | expected, XMM_SRC_HIGH, name);
    }
}

void runCvtpi2ps(U64 value, const char* name) {
    U64 expectedLow = packFloatBits(bitsFromFloat((float)(S32)(U32)value), bitsFromFloat((float)(S32)(U32)(value >> 32)));
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            initSseMmx();
            setXmm(dst, XMM_SRC_LOW, XMM_SRC_HIGH);
            cpu->fpu.getMMX(src)->q = value;
            emitMmxRegReg(0, 0x2a, dst, src);
            runTestCPU();
            verifyOnlyXmmChanged(dst, expectedLow, XMM_SRC_HIGH, name);
        }

        initSseMmx();
        setXmm(dst, XMM_SRC_LOW, XMM_SRC_HIGH);
        memory->writeq(TEST_HEAP_ADDRESS + MEM_SRC, value);
        emitSseRegMem(0, 0x2a, dst, MEM_SRC);
        runTestCPU();
        verifyOnlyXmmChanged(dst, expectedLow, XMM_SRC_HIGH, name);
    }
}

void runSseToMmx(U8 opcode, U64 low, U64 high, U64 expected, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            initSseMmx();
            setXmm(src, low, high);
            emitMmxRegReg(0, opcode, dst, src);
            runTestCPU();
            if (cpu->fpu.getMMX(dst)->q != expected) {
                failed("%s reg", name);
            }
        }

        initSseMmx();
        writeXmmMem(MEM_SRC, low, high);
        emitMmxRegMem(0, opcode, dst, MEM_SRC);
        runTestCPU();
        if (cpu->fpu.getMMX(dst)->q != expected) {
            failed("%s mem", name);
        }
    }
}

void runSseToReg(U8 prefix, U8 opcode, U64 low, U64 high, U32 expected, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            initSseMmx();
            setXmm(src, low, high);
            emitSseRegReg(prefix, opcode, dst, src);
            if (dst == 4) {
                emitStoreReg32ToMem(4, MEM_DST);
                emitRestoreEsp();
            }
            runTestCPU();
            U32 actual = dst == 4 ? memory->readd(TEST_HEAP_ADDRESS + MEM_DST) : cpu->reg[dst].u32;
            if (actual != expected) {
                failed("%s reg", name);
            }
        }

        initSseMmx();
        writeXmmMem(MEM_SRC, low, high);
        emitSseRegMem(prefix, opcode, dst, MEM_SRC);
        if (dst == 4) {
            emitStoreReg32ToMem(4, MEM_DST);
            emitRestoreEsp();
        }
        runTestCPU();
        U32 actual = dst == 4 ? memory->readd(TEST_HEAP_ADDRESS + MEM_DST) : cpu->reg[dst].u32;
        if (actual != expected) {
            failed("%s mem", name);
        }
    }
}

void runMovmskps(U64 low, U64 high, U32 expected, const char* name) {
    for (int src = 0; src < 8; ++src) {
        initSseMmx();
        setXmm(src, low, high);
        emitSseRegReg(0, 0x50, 0, src);
        runTestCPU();
        if (cpu->reg[0].u32 != expected) {
            failed("%s reg src=%d expected=%x actual=%x", name, src, expected, cpu->reg[0].u32);
        }
    }
}

void testSse128Op(U8 prefix1, U8 prefix2, U8 opcode, U64 dstLow, U64 dstHigh, U64 srcLow, U64 srcHigh, U64 regLow, U64 regHigh, U64 memLow, U64 memHigh, const char* name) {
    if (memLow == 0 && memHigh == 0) {
        memLow = regLow;
        memHigh = regHigh;
    }
    if (regLow != 0xffffffffffffffffULL || regHigh != 0xffffffffffffffffULL) {
        runSse128RegOnly(prefix1, prefix2, opcode, dstLow, dstHigh, srcLow, srcHigh, regLow, regHigh, name);
    }
    if (memLow != 0xffffffffffffffffULL || memHigh != 0xffffffffffffffffULL) {
        for (int dst = 0; dst < 8; ++dst) {
            initSse();
            setXmm(dst, dstLow, dstHigh);
            writeXmmMem(MEM_SRC, srcLow, srcHigh);
            emitSseRegMem(prefix1, prefix2, opcode, dst, MEM_SRC);
            runTestCPU();
            verifyOnlyXmmChanged(dst, memLow, memHigh, name);
        }
    }
}

void testSse128ROp(U8 prefix1, U8 prefix2, U8 opcode, U64 memInitialLow, U64 memInitialHigh, U64 srcLow, U64 srcHigh, U64 regLow, U64 regHigh, U64 memLow, U64 memHigh, const char* name) {
    if (memLow == 0 && memHigh == 0) {
        memLow = regLow;
        memHigh = regHigh;
    }
    if (regLow != 0xffffffffffffffffULL || regHigh != 0xffffffffffffffffULL) {
        for (int dst = 0; dst < 8; ++dst) {
            for (int src = 0; src < 8; ++src) {
                if (dst == src) {
                    continue;
                }
                initSse();
                setXmm(dst, memInitialLow, memInitialHigh);
                setXmm(src, srcLow, srcHigh);
                emitSseRegRegRmDst(prefix2, opcode, dst, src);
                runTestCPU();
                verifyXmmMove(dst, regLow, regHigh, src, srcLow, srcHigh, name);
            }
        }
    }
    if (memLow != 0xffffffffffffffffULL || memHigh != 0xffffffffffffffffULL) {
        for (int src = 0; src < 8; ++src) {
            initSse();
            writeXmmMem(MEM_DST, memInitialLow, memInitialHigh);
            setXmm(src, srcLow, srcHigh);
            emitSseMemReg(prefix1, prefix2, opcode, src, MEM_DST);
            runTestCPU();
            if (memory->readq(TEST_HEAP_ADDRESS + MEM_DST) != memLow ||
                    memory->readq(TEST_HEAP_ADDRESS + MEM_DST + 8) != memHigh) {
                failed("%s mem", name);
            }
        }
    }
}

void testSse128ImmOp(U8 prefix1, U8 prefix2, U8 opcode, U8 imm, U64 dstLow, U64 dstHigh, U64 srcLow, U64 srcHigh, U64 expectedLow, U64 expectedHigh, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            if (dst == src) {
                continue;
            }
            initSse();
            setXmm(dst, dstLow, dstHigh);
            setXmm(src, srcLow, srcHigh);
            emitSseRegReg(prefix1, prefix2, opcode, dst, src);
            pushCode8(imm);
            runTestCPU();
            verifyXmmMove(dst, expectedLow, expectedHigh, src, srcLow, srcHigh, name);
        }

        initSse();
        setXmm(dst, dstLow, dstHigh);
        writeXmmMem(MEM_SRC, srcLow, srcHigh);
        emitSseRegMem(prefix1, prefix2, opcode, dst, MEM_SRC);
        pushCode8(imm);
        runTestCPU();
        verifyOnlyXmmChanged(dst, expectedLow, expectedHigh, name);
    }
}

void runPinsrwXmm(U64 dstLow, U64 dstHigh, U32 srcValue, U8 imm, U64 expectedLow, U64 expectedHigh, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            initSseMmx();
            setXmm(dst, dstLow, dstHigh);
            cpu->reg[src].u32 = srcValue;
            emitSseRegRegImm(0x66, 0xc4, dst, src, imm);
            runTestCPU();
            verifyOnlyXmmChanged(dst, expectedLow, expectedHigh, name);
        }

        initSseMmx();
        setXmm(dst, dstLow, dstHigh);
        memory->writed(TEST_HEAP_ADDRESS + MEM_SRC, srcValue);
        emitSseRegMemImm(0x66, 0xc4, dst, MEM_SRC, imm);
        runTestCPU();
        verifyOnlyXmmChanged(dst, expectedLow, expectedHigh, name);
    }
}

void runPinsrwMmx(U64 dstValue, U32 srcValue, U8 imm, U64 expected, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            initSseMmx();
            cpu->fpu.getMMX(dst)->q = dstValue;
            cpu->reg[src].u32 = srcValue;
            emitMmxRegRegImm(0, 0xc4, dst, src, imm);
            runTestCPU();
            if (cpu->fpu.getMMX(dst)->q != expected) {
                failed("%s reg", name);
            }
        }

        initSseMmx();
        cpu->fpu.getMMX(dst)->q = dstValue;
        memory->writed(TEST_HEAP_ADDRESS + MEM_SRC, srcValue);
        emitMmxRegMemImm(0, 0xc4, dst, MEM_SRC, imm);
        runTestCPU();
        if (cpu->fpu.getMMX(dst)->q != expected) {
            failed("%s mem", name);
        }
    }
}

void runPextrwXmm(U64 srcLow, U64 srcHigh, U8 imm, U32 expected, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            initSseMmx();
            setXmm(src, srcLow, srcHigh);
            cpu->reg[dst].u32 = REG_GUARD | dst;
            emitSseRegRegImm(0x66, 0xc5, dst, src, imm);
            if (dst == 4) {
                emitStoreReg32ToMem(4, MEM_DST);
                emitRestoreEsp();
            }
            runTestCPU();
            U32 actual = dst == 4 ? memory->readd(TEST_HEAP_ADDRESS + MEM_DST) : cpu->reg[dst].u32;
            if (actual != expected) {
                failed("%s reg", name);
            }
        }
    }
}

void runPextrwXmmMemory(U64 srcLow, U64 srcHigh, U8 imm, U16 expected, const char* name) {
    for (int src = 0; src < 8; ++src) {
        initSseMmx();
        setXmm(src, srcLow, srcHigh);
        memory->writew(TEST_HEAP_ADDRESS + MEM_DST, 0xcdcd);
        memory->writew(TEST_HEAP_ADDRESS + MEM_DST + 2, 0x7777);
        pushCode8(0x66);
        pushCode8(0x0f);
        pushCode8(0xc5);
        emitDirectAddressModRM(src, MEM_DST);
        pushCode8(imm);
        runTestCPU();
        if (memory->readw(TEST_HEAP_ADDRESS + MEM_DST) != expected ||
                memory->readw(TEST_HEAP_ADDRESS + MEM_DST + 2) != 0x7777) {
            failed("%s mem", name);
        }
        verifyOnlyXmmChanged(src, srcLow, srcHigh, name);
    }
}

void runPextrwMmx(U64 srcValue, U8 imm, U32 expected, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            initSseMmx();
            cpu->fpu.getMMX(src)->q = srcValue;
            cpu->reg[dst].u32 = REG_GUARD | dst;
            emitMmxRegRegImm(0, 0xc5, dst, src, imm);
            if (dst == 4) {
                emitStoreReg32ToMem(4, MEM_DST);
                emitRestoreEsp();
            }
            runTestCPU();
            U32 actual = dst == 4 ? memory->readd(TEST_HEAP_ADDRESS + MEM_DST) : cpu->reg[dst].u32;
            if (actual != expected) {
                failed("%s reg", name);
            }
        }
    }
}

void testSse128SubImmOp(U8 prefix1, U8 prefix2, U8 opcode, U8 group, U8 imm, U64 valueLow, U64 valueHigh, U64 expectedLow, U64 expectedHigh, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        initSse();
        setXmm(dst, valueLow, valueHigh);
        if (prefix1) {
            pushCode8(prefix1);
        }
        if (prefix2) {
            pushCode8(prefix2);
        }
        pushCode8(0x0f);
        pushCode8(opcode);
        pushCode8(0xc0 | (group << 3) | dst);
        pushCode8(imm);
        runTestCPU();
        verifyOnlyXmmChanged(dst, expectedLow, expectedHigh, name);
    }
}

void testSseReg32Op(U8 prefix1, U8 prefix2, U8 opcode, U64 dstLow, U64 dstHigh, U32 srcValue, U64 expectedLow, U64 expectedHigh, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            initSseMmx();
            setXmm(dst, dstLow, dstHigh);
            cpu->reg[src].u32 = srcValue;
            emitSseRegReg(prefix1, prefix2, opcode, dst, src);
            runTestCPU();
            verifyOnlyXmmChanged(dst, expectedLow, expectedHigh, name);
        }

        initSseMmx();
        setXmm(dst, dstLow, dstHigh);
        memory->writed(TEST_HEAP_ADDRESS + MEM_SRC, srcValue);
        emitSseRegMem(prefix1, prefix2, opcode, dst, MEM_SRC);
        runTestCPU();
        verifyOnlyXmmChanged(dst, expectedLow, expectedHigh, name);
    }
}

void testSseReg32ROp(U8 prefix1, U8 prefix2, U8 opcode, U32 dstValue, U64 srcLow, U64 srcHigh, U32 regExpected, U32 memExpected, const char* name) {
    if (memExpected == 0) {
        memExpected = regExpected;
    }
    for (int dst = 0; dst < 8; ++dst) {
        if (regExpected != 0xffffffffU) {
            for (int src = 0; src < 8; ++src) {
                initSseMmx();
                setXmm(src, srcLow, srcHigh);
                cpu->reg[dst].u32 = dstValue;
                emitSseRegReg(prefix1, prefix2, opcode, dst, src);
                if (dst == 4) {
                    emitStoreReg32ToMem(4, MEM_DST);
                    emitRestoreEsp();
                }
                runTestCPU();
                U32 actual = dst == 4 ? memory->readd(TEST_HEAP_ADDRESS + MEM_DST) : cpu->reg[dst].u32;
                if (actual != regExpected) {
                    failed("%s reg expected=%x actual=%x", name, regExpected, actual);
                }
            }
        }
        if (memExpected != 0xffffffffU) {
            initSseMmx();
            writeXmmMem(MEM_SRC, srcLow, srcHigh);
            cpu->reg[dst].u32 = dstValue;
            emitSseRegMem(prefix1, prefix2, opcode, dst, MEM_SRC);
            if (dst == 4) {
                emitStoreReg32ToMem(4, MEM_DST);
                emitRestoreEsp();
            }
            runTestCPU();
            U32 actual = dst == 4 ? memory->readd(TEST_HEAP_ADDRESS + MEM_DST) : cpu->reg[dst].u32;
            if (actual != memExpected) {
                failed("%s mem expected=%x actual=%x", name, memExpected, actual);
            }
        }
    }
}

void testSseE32ROp(U8 prefix1, U8 prefix2, U8 opcode, U32 dstValue, U64 srcLow, U64 srcHigh, U32 regExpected, U32 memExpected, const char* name) {
    if (memExpected == 0) {
        memExpected = regExpected;
    }
    for (int src = 0; src < 8; ++src) {
        if (regExpected != 0xffffffffU) {
            for (int dst = 0; dst < 8; ++dst) {
                initSseMmx();
                setXmm(src, srcLow, srcHigh);
                cpu->reg[dst].u32 = dstValue;
                emitSseRegRegRmDst(prefix2, opcode, dst, src);
                if (dst == 4) {
                    emitStoreReg32ToMem(4, MEM_DST);
                    emitRestoreEsp();
                }
                runTestCPU();
                U32 actual = dst == 4 ? memory->readd(TEST_HEAP_ADDRESS + MEM_DST) : cpu->reg[dst].u32;
                if (actual != regExpected) {
                    failed("%s reg", name);
                }
            }
        }
        if (memExpected != 0xffffffffU) {
            initSseMmx();
            setXmm(src, srcLow, srcHigh);
            memory->writed(TEST_HEAP_ADDRESS + MEM_DST, dstValue);
            emitSseMemReg(prefix1, prefix2, opcode, src, MEM_DST);
            runTestCPU();
            if (memory->readd(TEST_HEAP_ADDRESS + MEM_DST) != memExpected) {
                failed("%s mem", name);
            }
        }
    }
}

void testSseMmx64Op(U8 prefix1, U8 prefix2, U8 opcode, U64 dstLow, U64 dstHigh, U64 srcValue, U64 expectedLow, U64 expectedHigh, U64 memExpectedLow, U64 memExpectedHigh, const char* name) {
    if (memExpectedLow == 0 && memExpectedHigh == 0) {
        memExpectedLow = expectedLow;
        memExpectedHigh = expectedHigh;
    }
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            initSseMmx();
            setXmm(dst, dstLow, dstHigh);
            cpu->fpu.getMMX(src)->q = srcValue;
            emitMmxRegReg(prefix2, opcode, dst, src);
            runTestCPU();
            verifyOnlyXmmChanged(dst, expectedLow, expectedHigh, name);
        }
        if (memExpectedLow != 0xffffffffffffffffULL || memExpectedHigh != 0xffffffffffffffffULL) {
            initSseMmx();
            setXmm(dst, dstLow, dstHigh);
            memory->writeq(TEST_HEAP_ADDRESS + MEM_SRC, srcValue);
            emitSseRegMem(prefix1, prefix2, opcode, dst, MEM_SRC);
            runTestCPU();
            verifyOnlyXmmChanged(dst, memExpectedLow, memExpectedHigh, name);
        }
    }
}

void verifyXmm(int reg, U64 low, U64 high, const char* name) {
    if (cpu->xmm[reg].pi.u64[0] != low || cpu->xmm[reg].pi.u64[1] != high) {
        failed("%s xmm%d", name, reg);
    }
}

void runCachedCvtpi2pdMmx(const char* name) {
    U64 first = ((U64)(U32)(S32)-2 << 32) | 3;
    U64 second = ((U64)(U32)(S32)-4 << 32) | 5;
    initSseMmx();
    cpu->fpu.getMMX(1)->q = first;
    cpu->fpu.getMMX(2)->q = second;
    emitMmxRegReg(0x66, 0x2a, 0, 1);
    emitMmxRegReg(0x66, 0x2a, 0, 2);
    runTestCPU();
    verifyXmm(0, bitsFromDouble(5.0), bitsFromDouble(-4.0), name);
    if (cpu->fpu.getMMX(1)->q != first || cpu->fpu.getMMX(2)->q != second) {
        failed("%s mmx unchanged", name);
    }
}

void runCachedCvtpi2pdMem(const char* name) {
    U64 first = ((U64)(U32)(S32)-6 << 32) | 7;
    U64 second = ((U64)(U32)(S32)-8 << 32) | 9;
    initSseMmx();
    memory->writeq(TEST_HEAP_ADDRESS + MEM_SRC, first);
    memory->writeq(TEST_HEAP_ADDRESS + MEM_SRC + 8, second);
    emitSseRegMem(0x66, 0x2a, 0, MEM_SRC);
    emitSseRegMem(0x66, 0x2a, 0, MEM_SRC + 8);
    runTestCPU();
    verifyXmm(0, bitsFromDouble(9.0), bitsFromDouble(-8.0), name);
}

void runCachedMovdXmmR32(const char* name) {
    initSseMmx();
    cpu->reg[0].u32 = 0x11112222;
    cpu->reg[1].u32 = 0x33334444;
    emitSseRegReg(0x66, 0x6e, 0, 0);
    emitSseRegReg(0x66, 0x6e, 0, 1);
    runTestCPU();
    verifyOnlyXmmChanged(0, 0x0000000033334444ULL, 0, name);
}

void runCachedMovqXmmXmm(const char* name) {
    initSseMmx();
    setXmm(1, 0x1111222233334444ULL, 0x5555666677778888ULL);
    setXmm(2, 0x9999aaaabbbbccccULL, 0xddddeeeeffff0000ULL);
    emitSseRegReg(0xf3, 0x7e, 0, 1);
    emitSseRegReg(0xf3, 0x7e, 0, 2);
    runTestCPU();
    verifyXmm(0, 0x9999aaaabbbbccccULL, 0, name);
    verifyXmm(1, 0x1111222233334444ULL, 0x5555666677778888ULL, name);
    verifyXmm(2, 0x9999aaaabbbbccccULL, 0xddddeeeeffff0000ULL, name);
}

void runCachedMovq2dq(const char* name) {
    initSseMmx();
    cpu->fpu.getMMX(1)->q = 0x1111222233334444ULL;
    cpu->fpu.getMMX(2)->q = 0x9999aaaabbbbccccULL;
    emitMmxRegReg(0xf3, 0xd6, 0, 1);
    emitMmxRegReg(0xf3, 0xd6, 0, 2);
    runTestCPU();
    verifyXmm(0, 0x9999aaaabbbbccccULL, 0, name);
    if (cpu->fpu.getMMX(1)->q != 0x1111222233334444ULL || cpu->fpu.getMMX(2)->q != 0x9999aaaabbbbccccULL) {
        failed("%s mmx unchanged", name);
    }
}

void runSegmentedMaskmovdqu(const char* name) {
    initSse();
    cpu->seg[DS].address = TEST_HEAP_ADDRESS;
    cpu->seg[DS].value = TEST_HEAP_SEG;
    cpu->thread->process->hasSetSeg[DS] = true;
    setXmm(0, 0x1122334455667788ULL, 0x99aabbccddeeff00ULL);
    setXmm(1, 0x8000800080008000ULL, 0x0080808000008080ULL);
    cpu->reg[7].u32 = MEM_DST;
    writeXmmMem(MEM_DST, 0x9999999999999999ULL, 0x9999999999999999ULL);
    emitSseRegReg(0x66, 0xf7, 0, 1);
    runTestCPU();
    if (memory->readq(TEST_HEAP_ADDRESS + MEM_DST) != 0x1199339955997799ULL ||
            memory->readq(TEST_HEAP_ADDRESS + MEM_DST + 8) != 0x99aabbcc9999ff00ULL) {
        failed("%s mem", name);
    }
}

void testSseMmx64ROp(U8 prefix1, U8 prefix2, U8 opcode, U64 dstValue, U64 srcLow, U64 srcHigh, U64 regExpected, U64 memExpected, const char* name) {
    if (memExpected == 0) {
        memExpected = regExpected;
    }
    for (int dst = 0; dst < 8; ++dst) {
        if (regExpected != 0xffffffffffffffffULL) {
            for (int src = 0; src < 8; ++src) {
                initSseMmx();
                cpu->fpu.getMMX(dst)->q = dstValue;
                setXmm(src, srcLow, srcHigh);
                emitMmxRegReg(prefix2, opcode, dst, src);
                runTestCPU();
                if (cpu->fpu.getMMX(dst)->q != regExpected) {
                    failed("%s reg expected=%llx actual=%llx", name, regExpected, cpu->fpu.getMMX(dst)->q);
                }
            }
        }
        if (memExpected != 0xffffffffffffffffULL) {
            initSseMmx();
            cpu->fpu.getMMX(dst)->q = dstValue;
            writeXmmMem(MEM_SRC, srcLow, srcHigh);
            emitMmxRegMem(prefix2, opcode, dst, MEM_SRC);
            runTestCPU();
            if (cpu->fpu.getMMX(dst)->q != memExpected) {
                failed("%s mem expected=%llx actual=%llx", name, memExpected, cpu->fpu.getMMX(dst)->q);
            }
        }
    }
}

U64 pshufwExpected(U64 value, U8 imm) {
    U64 result = 0;
    for (int lane = 0; lane < 4; ++lane) {
        U64 word = (value >> (((imm >> (lane * 2)) & 3) * 16)) & 0xffff;
        result |= word << (lane * 16);
    }
    return result;
}

} // namespace

void testSseMovups_0x310_0x311() {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            if (dst == src) {
                continue;
            }
            initSse();
            setXmm(src, XMM_SRC_LOW, XMM_SRC_HIGH);
            emitSseRegReg(0, 0x10, dst, src);
            runTestCPU();
            verifyXmmMove(dst, XMM_SRC_LOW, XMM_SRC_HIGH, src, XMM_SRC_LOW, XMM_SRC_HIGH, "sse movups reg");
        }

        initSse();
        emitSseRegMem(0, 0x10, dst, MEM_SRC);
        runTestCPU();
        verifyOnlyXmmChanged(dst, XMM_SRC_LOW, XMM_SRC_HIGH, "sse movups load");

        initSse();
        setXmm(dst, XMM_SRC_LOW, XMM_SRC_HIGH);
        emitSseMemReg(0, 0x11, dst, MEM_DST);
        runTestCPU();
        if (memory->readq(TEST_HEAP_ADDRESS + MEM_DST) != XMM_SRC_LOW ||
                memory->readq(TEST_HEAP_ADDRESS + MEM_DST + 8) != XMM_SRC_HIGH) {
            failed("sse movups store");
        }
    }
}

void testSseMovss_0xf310_0xf311() {
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            if (dst == src) {
                continue;
            }
            initSse();
            setXmm(dst, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH);
            setXmm(src, XMM_SRC_LOW, XMM_SRC_HIGH);
            emitSseRegReg(0xf3, 0x10, dst, src);
            runTestCPU();
            U64 expectedLow = (XMM_DEFAULT_LOW & 0xffffffff00000000ULL) | (XMM_SRC_LOW & 0xffffffffULL);
            verifyXmmMove(dst, expectedLow, XMM_DEFAULT_HIGH, src, XMM_SRC_LOW, XMM_SRC_HIGH, "sse movss reg");
        }

        initSse();
        emitSseRegMem(0xf3, 0x10, dst, MEM_SRC);
        runTestCPU();
        verifyOnlyXmmChanged(dst, XMM_SRC_LOW & 0xffffffffULL, 0, "sse movss load");

        initSse();
        setXmm(dst, XMM_SRC_LOW, XMM_SRC_HIGH);
        emitSseMemReg(0xf3, 0x11, dst, MEM_DST);
        runTestCPU();
        if (memory->readd(TEST_HEAP_ADDRESS + MEM_DST) != (U32)XMM_SRC_LOW ||
                memory->readd(TEST_HEAP_ADDRESS + MEM_DST + 4) != (U32)(MEM_GUARD_LOW >> 32)) {
            failed("sse movss store");
        }
    }
}

void testSseLogic_0x354_0x355_0x356_0x357() {
    struct LogicCase {
        U8 opcode;
        U64 low;
        U64 high;
        const char* name;
    };
    const LogicCase cases[] = {
        {0x54, XMM_SRC_LOW & XMM_ALT_LOW, XMM_SRC_HIGH & XMM_ALT_HIGH, "sse andps"},
        {0x55, (~XMM_SRC_LOW) & XMM_ALT_LOW, (~XMM_SRC_HIGH) & XMM_ALT_HIGH, "sse andnps"},
        {0x56, XMM_SRC_LOW | XMM_ALT_LOW, XMM_SRC_HIGH | XMM_ALT_HIGH, "sse orps"},
        {0x57, XMM_SRC_LOW ^ XMM_ALT_LOW, XMM_SRC_HIGH ^ XMM_ALT_HIGH, "sse xorps"}
    };

    for (int i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); ++i) {
        initSse();
        setXmm(0, XMM_SRC_LOW, XMM_SRC_HIGH);
        setXmm(1, XMM_ALT_LOW, XMM_ALT_HIGH);
        emitSseRegReg(0, cases[i].opcode, 0, 1);
        runTestCPU();
        verifyXmmMove(0, cases[i].low, cases[i].high, 1, XMM_ALT_LOW, XMM_ALT_HIGH, cases[i].name);

        initSse();
        setXmm(0, XMM_SRC_LOW, XMM_SRC_HIGH);
        memory->writeq(TEST_HEAP_ADDRESS + MEM_SRC, XMM_ALT_LOW);
        memory->writeq(TEST_HEAP_ADDRESS + MEM_SRC + 8, XMM_ALT_HIGH);
        emitSseRegMem(0, cases[i].opcode, 0, MEM_SRC);
        runTestCPU();
        verifyOnlyXmmChanged(0, cases[i].low, cases[i].high, cases[i].name);
    }
}

void testSseAddps_0x358() {
    U64 aLow;
    U64 aHigh;
    U64 bLow;
    U64 bHigh;
    U64 expectedLow;
    U64 expectedHigh;

    initSse();
    setXmmFloatVector(0, 1.0f, -2.5f, 100.25f, -0.0f);
    setXmmFloatVector(1, 3.5f, 2.25f, -0.25f, 7.0f);
    aLow = cpu->xmm[0].pi.u64[0];
    aHigh = cpu->xmm[0].pi.u64[1];
    bLow = cpu->xmm[1].pi.u64[0];
    bHigh = cpu->xmm[1].pi.u64[1];
    addpsExpected(aLow, aHigh, bLow, bHigh, expectedLow, expectedHigh);
    emitSseRegReg(0, 0x58, 0, 1);
    runTestCPU();
    verifyXmmMove(0, expectedLow, expectedHigh, 1, bLow, bHigh, "sse addps reg");

    initSse();
    setXmmFloatVector(0, -8.0f, 1.5f, 0.25f, 1024.0f);
    writeFloatVector(MEM_SRC, 8.0f, -1.0f, 0.75f, -24.0f);
    aLow = cpu->xmm[0].pi.u64[0];
    aHigh = cpu->xmm[0].pi.u64[1];
    bLow = memory->readq(TEST_HEAP_ADDRESS + MEM_SRC);
    bHigh = memory->readq(TEST_HEAP_ADDRESS + MEM_SRC + 8);
    addpsExpected(aLow, aHigh, bLow, bHigh, expectedLow, expectedHigh);
    emitSseRegMem(0, 0x58, 0, MEM_SRC);
    runTestCPU();
    verifyOnlyXmmChanged(0, expectedLow, expectedHigh, "sse addps mem");
}

void testSseMoveUnpack_0x312_0x313_0x316_0x317_0x328_0x329() {
    runSse128RegOnly(0, 0, 0x12, 0xaabbccddeeff2468ULL, 0x1122334455667788ULL, 0x1234567890abcdefULL, 0x24680bdf13579aceULL, 0x24680bdf13579aceULL, 0x1122334455667788ULL, "sse movhlps");
    runSse128(0, 0, 0x14, 0xaabbccddeeff2468ULL, 0x1122334455667788ULL, 0x1234567890abcdefULL, 0x24680bdf13579aceULL, 0x90abcdefeeff2468ULL, 0x12345678aabbccddULL, "sse unpcklps");
    runSse128(0, 0, 0x15, 0xaabbccddeeff2468ULL, 0x1122334455667788ULL, 0x1234567890abcdefULL, 0x24680bdf13579aceULL, 0x13579ace55667788ULL, 0x24680bdf11223344ULL, "sse unpckhps");
    runSse128RegOnly(0, 0, 0x16, 0xaabbccddeeff2468ULL, 0x1122334455667788ULL, 0x1234567890abcdefULL, 0x24680bdf13579aceULL, 0xaabbccddeeff2468ULL, 0x1234567890abcdefULL, "sse movlhps");

    for (int dst = 0; dst < 8; ++dst) {
        initSse();
        setXmm(dst, 0xaabbccddeeff2468ULL, 0x1122334455667788ULL);
        writeXmmMem(MEM_SRC, 0x1234567890abcdefULL, 0x24680bdf13579aceULL);
        emitSseRegMem(0, 0x12, dst, MEM_SRC);
        runTestCPU();
        verifyOnlyXmmChanged(dst, 0x1234567890abcdefULL, 0x1122334455667788ULL, "sse movlps load");

        initSse();
        setXmm(dst, 0xaabbccddeeff2468ULL, 0x1122334455667788ULL);
        writeXmmMem(MEM_SRC, 0x1234567890abcdefULL, 0x24680bdf13579aceULL);
        emitSseRegMem(0, 0x16, dst, MEM_SRC);
        runTestCPU();
        verifyOnlyXmmChanged(dst, 0xaabbccddeeff2468ULL, 0x1234567890abcdefULL, "sse movhps load");

        initSse();
        setXmm(dst, 0x1234567890abcdefULL, 0x24680bdf13579aceULL);
        writeXmmMem(MEM_DST, 0xaabbccddeeff2468ULL, 0x1122334455667788ULL);
        emitSseMemReg(0, 0x13, dst, MEM_DST);
        runTestCPU();
        if (memory->readq(TEST_HEAP_ADDRESS + MEM_DST) != 0x1234567890abcdefULL ||
                memory->readq(TEST_HEAP_ADDRESS + MEM_DST + 8) != 0x1122334455667788ULL) {
            failed("sse movlps store");
        }

        initSse();
        setXmm(dst, 0x1234567890abcdefULL, 0x24680bdf13579aceULL);
        writeXmmMem(MEM_DST, 0xaabbccddeeff2468ULL, 0x1122334455667788ULL);
        emitSseMemReg(0, 0x17, dst, MEM_DST);
        runTestCPU();
        if (memory->readq(TEST_HEAP_ADDRESS + MEM_DST) != 0x24680bdf13579aceULL ||
                memory->readq(TEST_HEAP_ADDRESS + MEM_DST + 8) != 0x1122334455667788ULL) {
            failed("sse movhps store");
        }
    }

    runSse128(0, 0, 0x28, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, "sse movaps load");
    runSse128Store(0, 0, 0x29, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, "sse movaps store");
}

void testSseMovntps_0x32b() {
    for (int src = 0; src < 8; ++src) {
        initSse();
        setXmm(src, XMM_SRC_LOW, XMM_SRC_HIGH);
        memory->writeq(TEST_HEAP_ADDRESS + MEM_DST - 8, MEM_GUARD_HIGH);
        memory->writeq(TEST_HEAP_ADDRESS + MEM_DST + 16, MEM_GUARD_LOW);
        emitSseMemReg(0, 0x2b, src, MEM_DST);
        runTestCPU();
        if (memory->readq(TEST_HEAP_ADDRESS + MEM_DST) != XMM_SRC_LOW ||
                memory->readq(TEST_HEAP_ADDRESS + MEM_DST + 8) != XMM_SRC_HIGH ||
                memory->readq(TEST_HEAP_ADDRESS + MEM_DST - 8) != MEM_GUARD_HIGH ||
                memory->readq(TEST_HEAP_ADDRESS + MEM_DST + 16) != MEM_GUARD_LOW) {
            failed("sse movntps store");
        }
        verifyOnlyXmmChanged(src, XMM_SRC_LOW, XMM_SRC_HIGH, "sse movntps xmm unchanged");
    }

    initSse();
    cpu->setMxcsr(0x1f80);
    memory->writed(TEST_HEAP_ADDRESS + MEM_DST, 0xcdcdcdcd);
    pushCode8(0x0f);
    pushCode8(0xae);
    emitDirectAddressModRM(3, MEM_DST);
    runTestCPU();
    if (memory->readd(TEST_HEAP_ADDRESS + MEM_DST) != 0x1f80) {
        failed("sse stmxcsr");
    }

    initSse();
    constexpr U32 LDMXCSR_VALUE = 0x1f80 & ~(1u << 9);
    memory->writed(TEST_HEAP_ADDRESS + MEM_SRC, LDMXCSR_VALUE);
    pushCode8(0x0f);
    pushCode8(0xae);
    emitDirectAddressModRM(2, MEM_SRC);
    runTestCPU();
    if (cpu->mxcsr != LDMXCSR_VALUE) {
        failed("sse ldmxcsr");
    }
    if (!cpu->sseDivExceptionsUnmasked) {
        failed("sse ldmxcsr did not update divide exception state");
    }

    initSse();
    memory->writeb(TEST_HEAP_ADDRESS + MEM_SRC, 0x5a);
    pushCode8(0x0f);
    pushCode8(0xae);
    emitDirectAddressModRM(7, MEM_SRC);
    runTestCPU();
    if (memory->readb(TEST_HEAP_ADDRESS + MEM_SRC) != 0x5a) {
        failed("sse clflush");
    }

    initSse();
    pushCode8(0x0f);
    pushCode8(0xae);
    pushCode8(0xf8);
    runTestCPU();
}

void testSseConvert_0x32a_0x32c_0x32d() {
    runCvtpi2ps(((U64)(U32)(S32)-5000 << 32) | (U32)5000, "sse cvtpi2ps mixed");
    runCvtpi2ps(((U64)(U32)0x80000000 << 32) | 0x7fffffff, "sse cvtpi2ps limits");
    runCvtsi2ss((U32)(S32)-5000, "sse cvtsi2ss neg");
    runCvtsi2ss(0, "sse cvtsi2ss zero");
    runCvtsi2ss(0x7fffffff, "sse cvtsi2ss intmax");
    runCvtsi2ss(0x80000000, "sse cvtsi2ss intmin");

    U32 hugePos = bitsFromFloat(12345678900.0f);
    U32 hugeNeg = bitsFromFloat(-12345678900.0f);
    U32 negFrac = bitsFromFloat(-5000.6f);
    U32 posFrac = bitsFromFloat(5000.6f);
    U32 negWhole = bitsFromFloat(-5000.0f);
    U32 posTie = bitsFromFloat(2.5f);
    U32 negTie = bitsFromFloat(-2.5f);

    runSseToMmx(0x2c, packFloatBits(hugePos, negFrac), packFloatBits(1, 2), 0xffffec7880000000ULL, "sse cvttps2pi neg frac");
    runSseToMmx(0x2c, packFloatBits(hugeNeg, posFrac), packFloatBits(1, 2), 0x0000138880000000ULL, "sse cvttps2pi pos frac");
    runSseToReg(0xf3, 0x2c, ((U64)3 << 32) | hugePos, ((U64)2 << 32) | 1, 0x80000000, "sse cvttss2si huge pos");
    runSseToReg(0xf3, 0x2c, ((U64)3 << 32) | negFrac, ((U64)2 << 32) | 1, 0xffffec78, "sse cvttss2si trunc neg");
    runSseToReg(0xf3, 0x2c, ((U64)3 << 32) | hugeNeg, ((U64)2 << 32) | 1, 0x80000000, "sse cvttss2si huge neg");

    runSseToMmx(0x2d, packFloatBits(hugePos, negWhole), packFloatBits(1, 2), 0xffffec7880000000ULL, "sse cvtps2pi huge");
    runSseToMmx(0x2d, packFloatBits(posTie, negTie), packFloatBits(1, 2), 0xfffffffe00000002ULL, "sse cvtps2pi nearest even");
    runSseToReg(0xf3, 0x2d, ((U64)3 << 32) | hugePos, ((U64)2 << 32) | 1, 0x80000000, "sse cvtss2si huge");
    runSseToReg(0xf3, 0x2d, ((U64)3 << 32) | negWhole, ((U64)2 << 32) | 1, 0xffffec78, "sse cvtss2si neg");
    runSseToReg(0xf3, 0x2d, ((U64)3 << 32) | posTie, ((U64)2 << 32) | 1, 2, "sse cvtss2si nearest even");
}

void testSseCompareFlags_0x32e_0x32f() {
    struct CompareCase {
        U32 lhs;
        U32 rhs;
        U32 flags;
        const char* name;
    };
    const CompareCase cases[] = {
        {bitsFromFloat(1.0f), bitsFromFloat(2.0f), CF | DF, "less"},
        {bitsFromFloat(-1.0f), bitsFromFloat(-1.0f), ZF | DF, "equal"},
        {bitsFromFloat(1.0f), bitsFromFloat(-1.0f), DF, "greater"},
        {bitsFromFloat(-0.0f), bitsFromFloat(0.0f), ZF | DF, "signed zero equal"},
        {0x7fc00000, bitsFromFloat(-1.0f), ZF | PF | CF | DF, "quiet nan unordered"},
        {bitsFromFloat(1.0f), 0x7fc00000, ZF | PF | CF | DF, "rhs quiet nan unordered"}
    };
    for (int i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); ++i) {
        runSseCompare(0x2e, cases[i].lhs, cases[i].rhs, cases[i].flags, cases[i].name);
        runSseCompare(0x2f, cases[i].lhs, cases[i].rhs, cases[i].flags, cases[i].name);
    }
}

void testSseMovmskAndApprox_0x350_0x351_0x352_0x353() {
    runMovmskps(0xc59c40004eff8000ULL, 0x00000000459c4000ULL, 2, "sse movmskps");
    runMovmskps(0x8000000000000000ULL, 0xffffffff7fffffffULL, 10, "sse movmskps signs");

    runSse128(0, 0, 0x51, 0, 0, 0x4080000000000000ULL, 0x4000000043800000ULL, 0x4000000000000000ULL, 0x3fb504f341800000ULL, "sse sqrtps");
    runSse128(0, 0xf3, 0x51, 0x4000000043800000ULL, 0x4080000000000000ULL, 0x4000000043800000ULL, 0x4080000000000000ULL, 0x4000000041800000ULL, 0x4080000000000000ULL, "sse sqrtss");

#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)
    runSse128(0, 0, 0x52, 0x4080000000000000ULL, 0x4000000043800000ULL, 0x4080000000000000ULL, 0x4000000043800000ULL, 0x3efff0007f800000ULL, 0x3f34f8003d7ff000ULL, "sse rsqrtps");
    runSse128(0, 0xf3, 0x52, 0x4110000040800000ULL, 0x4000000043800000ULL, 0x4110000040800000ULL, 0x4000000043800000ULL, 0x411000003efff000ULL, 0x4000000043800000ULL, "sse rsqrtss");
    runSse128(0, 0, 0x53, 0x4110000040800000ULL, 0x3dcccccd43800000ULL, 0x4110000040800000ULL, 0x3dcccccd43800000ULL, 0x3de380003e7ff000ULL, 0x412000003b7ff000ULL, "sse rcpps");
    runSse128(0, 0xf3, 0x53, 0x4110000040800000ULL, 0x3dcccccd43800000ULL, 0x4110000040800000ULL, 0x3dcccccd43800000ULL, 0x411000003e7ff000ULL, 0x3dcccccd43800000ULL, "sse rcpss");
#elif defined(BOXEDWINE_JIT_ARMV8)
    runSse128(0, 0xf3, 0x52, 0x4110000040800000ULL, 0x4000000043800000ULL, 0x4110000040800000ULL, 0x4000000043800000ULL, 0x411000003eff8000ULL, 0x4000000043800000ULL, "sse rsqrtss");
    runSse128(0, 0, 0x53, 0x4110000040800000ULL, 0x3dcccccd43800000ULL, 0x4110000040800000ULL, 0x3dcccccd43800000ULL, 0x3de300003e7f8000ULL, 0x412000003b7f8000ULL, "sse rcpps");
    runSse128(0, 0xf3, 0x53, 0x4110000040800000ULL, 0x3dcccccd43800000ULL, 0x4110000040800000ULL, 0x3dcccccd43800000ULL, 0x411000003e7f8000ULL, 0x3dcccccd43800000ULL, "sse rcpss");
#else
    runSse128(0, 0xf3, 0x52, 0x4110000040800000ULL, 0x4000000043800000ULL, 0x4110000040800000ULL, 0x4000000043800000ULL, 0x411000003effc988ULL, 0x4000000043800000ULL, "sse rsqrtss");
    runSse128(0, 0, 0x53, 0x4110000040800000ULL, 0x3dcccccd43800000ULL, 0x4110000040800000ULL, 0x3dcccccd43800000ULL, 0x3de38df43e7f58ccULL, 0x411fc11d3b7f58ccULL, "sse rcpps");
    runSse128(0, 0xf3, 0x53, 0x4110000040800000ULL, 0x3dcccccd43800000ULL, 0x4110000040800000ULL, 0x3dcccccd43800000ULL, 0x411000003e800000ULL, 0x3dcccccd43800000ULL, "sse rcpss");
#endif
}

void testSseArithmetic_0xf358_0x359_0x35c_0x35d_0x35e_0x35f() {
    runSse128(0, 0xf3, 0x58, 0x4110000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41200000c0a00000ULL, 0x3f800000c0b00000ULL, 0x41100000bf800000ULL, 0xbdcccccdc0a00000ULL, "sse addss");
    runSse128(0, 0, 0x59, 0x4110000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41200000c0a00000ULL, 0x40000000c0b00000ULL, 0x42b40000c1a00000ULL, 0xbe4ccccd41dc0000ULL, "sse mulps");
    runSse128(0, 0xf3, 0x59, 0x4110000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41200000c0a00000ULL, 0x40000000c0b00000ULL, 0x41100000c1a00000ULL, 0xbdcccccdc0a00000ULL, "sse mulss");
    runSse128(0, 0, 0x5c, 0x4110000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41200000c0a00000ULL, 0x40000000c0b00000ULL, 0xbf80000041100000ULL, 0xc00666663f000000ULL, "sse subps");
    runSse128(0, 0xf3, 0x5c, 0x4110000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41200000c0a00000ULL, 0x40000000c0b00000ULL, 0x4110000041100000ULL, 0xbdcccccdc0a00000ULL, "sse subss");
    runSse128(0, 0, 0x5d, 0x4110000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41200000c0a00000ULL, 0x40000000c0b00000ULL, 0x41100000c0a00000ULL, 0xbdcccccdc0b00000ULL, "sse minps");
    runSse128(0, 0, 0x5d, 0xc0e000007fc00000ULL, 0x0000000080000000ULL, 0x7fc0000041880000ULL, 0x8000000000000000ULL, 0x7fc0000041880000ULL, 0x8000000000000000ULL, "sse minps nan zero");
    runSse128(0, 0xf3, 0x5d, 0x4110000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41200000c0a00000ULL, 0x40000000c0b00000ULL, 0x41100000c0a00000ULL, 0xbdcccccdc0a00000ULL, "sse minss");
    runSse128(0, 0xf3, 0x5d, 0x411000007fc00000ULL, 0xbdcccccdc0a00000ULL, 0x41200000c0a00000ULL, 0x40000000c0b00000ULL, 0x41100000c0a00000ULL, 0xbdcccccdc0a00000ULL, "sse minss lhs nan");
    runSse128(0, 0xf3, 0x5d, 0x4110000040800000ULL, 0xbdcccccdc0a00000ULL, 0x412000007fc00000ULL, 0x40000000c0b00000ULL, 0x411000007fc00000ULL, 0xbdcccccdc0a00000ULL, "sse minss rhs nan");
    runSse128(0, 0xf3, 0x5d, 0x4110000000000000ULL, 0xbdcccccdc0a00000ULL, 0x4120000080000000ULL, 0x40000000c0b00000ULL, 0x4110000080000000ULL, 0xbdcccccdc0a00000ULL, "sse minss zero");
    runSse128(0, 0xf3, 0x5d, 0x4110000080000000ULL, 0xbdcccccdc0a00000ULL, 0x4120000000000000ULL, 0x40000000c0b00000ULL, 0x4110000000000000ULL, 0xbdcccccdc0a00000ULL, "sse minss neg zero");
    runSse128(0, 0, 0x5e, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x3e800000bf000000ULL, 0xbd4ccccd3f800000ULL, "sse divps");
    runSse128(0, 0xf3, 0x5e, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x40a00000bf000000ULL, 0xbdcccccdc0a00000ULL, "sse divss");
    runSse128(0, 0, 0x5f, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x41a0000040800000ULL, 0x40000000c0a00000ULL, "sse maxps");
    runSse128(0, 0xf3, 0x5f, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, "sse maxss");
}

void testSsePshufw_0x370() {
    const U64 dstValue = 0x1111222233334444ULL;
    const U64 srcValues[] = {
        0x5555666677778888ULL,
        0x80007fff0000ffffULL
    };
    const U8 immediates[] = {0x1e, 0x00, 0xff, 0x39};
    for (int i = 0; i < (int)(sizeof(srcValues) / sizeof(srcValues[0])); ++i) {
        for (int j = 0; j < (int)(sizeof(immediates) / sizeof(immediates[0])); ++j) {
            U64 expected = pshufwExpected(srcValues[i], immediates[j]);
            for (int dst = 0; dst < 8; ++dst) {
                for (int src = 0; src < 8; ++src) {
                    initSseMmx();
                    cpu->fpu.getMMX(dst)->q = dstValue;
                    cpu->fpu.getMMX(src)->q = srcValues[i];
                    emitMmxRegReg(0, 0x70, dst, src);
                    pushCode8(immediates[j]);
                    runTestCPU();
                    if (cpu->fpu.getMMX(dst)->q != expected) {
                        failed("sse pshufw reg");
                    }
                }
            }
        }
    }
}

void testSseCompareImmediate_0x3c2() {
    testSse128ImmOp(0, 0, 0xc2, 0, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x0000000000000000ULL, 0x00000000ffffffffULL, "sse cmpeqps");
    testSse128ImmOp(0, 0, 0xc2, 1, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0xffffffff00000000ULL, 0xffffffff00000000ULL, "sse cmpltps");
    testSse128ImmOp(0, 0, 0xc2, 2, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0xffffffff00000000ULL, 0xffffffffffffffffULL, "sse cmpleps");
    testSse128ImmOp(0, 0, 0xc2, 3, 0x7fd0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0xffffffff00000000ULL, 0x0000000000000000ULL, "sse cmpunordps");
    testSse128ImmOp(0, 0, 0xc2, 4, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0xffffffffffffffffULL, 0xffffffff00000000ULL, "sse cmpneqps");
    testSse128ImmOp(0, 0, 0xc2, 5, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x00000000ffffffffULL, 0x00000000ffffffffULL, "sse cmpnltps");
    testSse128ImmOp(0, 0, 0xc2, 6, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x00000000ffffffffULL, 0x0000000000000000ULL, "sse cmpnleps");
    testSse128ImmOp(0, 0, 0xc2, 7, 0x7fd0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x00000000ffffffffULL, 0xffffffffffffffffULL, "sse cmpordps");

    testSse128ImmOp(0, 0xf3, 0xc2, 0, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x40a0000000000000ULL, 0xbdcccccdc0a00000ULL, "sse cmpeqss");
    testSse128ImmOp(0, 0xf3, 0xc2, 1, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x40a0000000000000ULL, 0xbdcccccdc0a00000ULL, "sse cmpltss");
    testSse128ImmOp(0, 0xf3, 0xc2, 2, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x40a0000000000000ULL, 0xbdcccccdc0a00000ULL, "sse cmpless");
    testSse128ImmOp(0, 0xf3, 0xc2, 3, 0x7fd0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x7fd0000000000000ULL, 0xbdcccccdc0a00000ULL, "sse cmpunordss");
    testSse128ImmOp(0, 0xf3, 0xc2, 4, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x40a00000ffffffffULL, 0xbdcccccdc0a00000ULL, "sse cmpneqss");
    testSse128ImmOp(0, 0xf3, 0xc2, 5, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x40a00000ffffffffULL, 0xbdcccccdc0a00000ULL, "sse cmpnltss");
    testSse128ImmOp(0, 0xf3, 0xc2, 6, 0x40a0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x40a00000ffffffffULL, 0xbdcccccdc0a00000ULL, "sse cmpnless");
    testSse128ImmOp(0, 0xf3, 0xc2, 7, 0x7fd0000040800000ULL, 0xbdcccccdc0a00000ULL, 0x41a00000c1000000ULL, 0x40000000c0a00000ULL, 0x7fd00000ffffffffULL, 0xbdcccccdc0a00000ULL, "sse cmpordss");
}

void testSseInsertExtractShuffle_0x1c4_0x3c4_0x1c5_0x3c5_0x3c6() {
    runPinsrwXmm(0x1111222233334444ULL, 0x5555666677778888ULL, 0x11229900, 5, 0x1111222233334444ULL, 0x5555666699008888ULL, "sse2 pinsrw xmm");
    runPinsrwMmx(0x1111222233334444ULL, 0x5555, 2, 0x1111555533334444ULL, "sse pinsrw mmx");
    runPextrwXmm(0x1111222233334444ULL, 0x5555666677778888ULL, 2, 0x2222, "sse2 pextrw xmm");
    runPextrwXmmMemory(0x1111222233334444ULL, 0x5555666677778888ULL, 6, 0x6666, "sse2 pextrw xmm");
    runPextrwMmx(0x1111222233334444ULL, 2, 0x2222, "sse pextrw mmx");
    testSse128ImmOp(0, 0, 0xc6, 0x1e, 0x2222222211111111ULL, 0x4444444433333333ULL, 0x6666666655555555ULL, 0x8888888877777777ULL, 0x4444444433333333ULL, 0x5555555566666666ULL, "sse shufps");
    testSse128ImmOp(0, 0, 0xc6, (U8)(1 | (1 << 4) | (3 << 6)), 0x2222222211111111ULL, 0x4444444433333333ULL, 0x6666666655555555ULL, 0x8888888866666666ULL, 0x1111111122222222ULL, 0x8888888866666666ULL, "sse shufps mixed");
}

void testSse2Move_0x110_0x310_0x111_0x311_0x112_0x113_0x116_0x117_0x128_0x129() {
    testSse128Op(0, 0x66, 0x10, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, 0, 0, "sse2 movupd");
    testSse128Op(0, 0xf2, 0x10, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_SRC_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, 0, "sse2 movsd load");
    testSse128ROp(0, 0x66, 0x11, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, 0, 0, "sse2 movupd store");
    testSse128ROp(0, 0xf2, 0x11, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_SRC_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_DEFAULT_HIGH, "sse2 movsd store");
    testSse128Op(0, 0x66, 0x12, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, 0xffffffffffffffffULL, 0xffffffffffffffffULL, XMM_DEFAULT_LOW, XMM_SRC_HIGH, "sse2 movlpd load");
    testSse128ROp(0, 0x66, 0x13, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, 0xffffffffffffffffULL, 0xffffffffffffffffULL, XMM_DEFAULT_LOW, XMM_SRC_HIGH, "sse2 movlpd store");
    testSse128Op(0, 0x66, 0x16, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, 0xffffffffffffffffULL, 0xffffffffffffffffULL, XMM_SRC_LOW, XMM_DEFAULT_LOW, "sse2 movhpd load");
    testSse128ROp(0, 0x66, 0x17, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, 0xffffffffffffffffULL, 0xffffffffffffffffULL, XMM_DEFAULT_HIGH, XMM_SRC_HIGH, "sse2 movhpd store");
    testSse128Op(0, 0x66, 0x28, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, 0, 0, "sse2 movapd load");
    testSse128ROp(0, 0x66, 0x29, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, 0, 0, "sse2 movapd store");
    testSse128ROp(0, 0x66, 0x2b, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, 0xffffffffffffffffULL, 0xffffffffffffffffULL, XMM_SRC_LOW, XMM_SRC_HIGH, "sse2 movntpd");
    testSse128Op(0, 0x66, 0x6f, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0, 0, "sse2 movdqa load");
    testSse128Op(0, 0xf3, 0x6f, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0, 0, "sse2 movdqu load");
    testSse128ROp(0, 0x66, 0x7f, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0, 0, "sse2 movdqa store");
    testSse128ROp(0, 0xf3, 0x7f, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0, 0, "sse2 movdqu store");
}

void testSse2UnpackShuffle_0x114_0x115_0x160_0x161_0x162_0x168_0x169_0x16a_0x16c_0x16d_0x170_0x370_0x1c6() {
    testSse128Op(0, 0x66, 0x14, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_DEFAULT_LOW, 0, 0, "sse2 unpcklpd");
    testSse128Op(0, 0x66, 0x15, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_HIGH, XMM_DEFAULT_HIGH, 0, 0, "sse2 unpckhpd");
    testSse128Op(0, 0x66, 0x60, 0x1122334455667788ULL, 0xffffffffffffffffULL, 0x99aabbccddeeff00ULL, 0xffffffffffffffffULL, 0xdd55ee66ff770088ULL, 0x9911aa22bb33cc44ULL, 0, 0, "sse2 punpcklbw");
    testSse128Op(0, 0x66, 0x61, 0x1122334455667788ULL, 0xffffffffffffffffULL, 0x99aabbccddeeff00ULL, 0xffffffffffffffffULL, 0xddee5566ff007788ULL, 0x99aa1122bbcc3344ULL, 0, 0, "sse2 punpcklwd");
    testSse128Op(0, 0x66, 0x62, 0x1122334455667788ULL, 0xffffffffffffffffULL, 0x99aabbccddeeff00ULL, 0xffffffffffffffffULL, 0xddeeff0055667788ULL, 0x99aabbcc11223344ULL, 0, 0, "sse2 punpckldq");
    testSse128Op(0, 0x66, 0x68, 0xffffffffffffffffULL, 0x1122334455667788ULL, 0xffffffffffffffffULL, 0x99aabbccddeeff00ULL, 0xdd55ee66ff770088ULL, 0x9911aa22bb33cc44ULL, 0, 0, "sse2 punpckhbw");
    testSse128Op(0, 0x66, 0x69, 0xffffffffffffffffULL, 0x1122334455667788ULL, 0xffffffffffffffffULL, 0x99aabbccddeeff00ULL, 0xddee5566ff007788ULL, 0x99aa1122bbcc3344ULL, 0, 0, "sse2 punpckhwd");
    testSse128Op(0, 0x66, 0x6a, 0xffffffffffffffffULL, 0x1122334455667788ULL, 0xffffffffffffffffULL, 0x99aabbccddeeff00ULL, 0xddeeff0055667788ULL, 0x99aabbcc11223344ULL, 0, 0, "sse2 punpckhdq");
    testSse128Op(0, 0x66, 0x6c, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0x1111111122222222ULL, 0x5555555566666666ULL, 0, 0, "sse2 punpcklqdq");
    testSse128Op(0, 0x66, 0x6d, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0x3333333344444444ULL, 0x7777777788888888ULL, 0, 0, "sse2 punpckhqdq");
    testSse128ImmOp(0, 0x66, 0x70, 0x1e, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0x7777777788888888ULL, 0x6666666655555555ULL, "sse2 pshufd");
    testSse128ImmOp(0, 0xf2, 0x70, 0x1e, 0x1111222233334444ULL, 0x1234567890abcdefULL, 0x5555666677778888ULL, 0xfedcba0987654321ULL, 0x8888777755556666ULL, 0xfedcba0987654321ULL, "sse2 pshuflw");
    testSse128ImmOp(0, 0xf3, 0x70, 0x1e, 0x1111222233334444ULL, 0x1234567890abcdefULL, 0x5555666677778888ULL, 0xfedcba0987654321ULL, 0x5555666677778888ULL, 0x43218765fedcba09ULL, "sse2 pshufhw");
    testSse128ImmOp(0, 0x66, 0xc6, 0x1e, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0x1111111122222222ULL, 0x7777777788888888ULL, "sse2 shufpd");
}

void testSse2PackCompare_0x163_0x164_0x165_0x166_0x167_0x16b_0x174_0x175_0x176() {
    testSse128Op(0, 0x66, 0x63, 0x007f008001230000ULL, 0xf830ff81ff80ffceULL, 0xabcd7fffffff0005ULL, 0x0001fffbfd12ff6aULL, 0x808180ce7f7f7f00ULL, 0x01fb8080807fff05ULL, 0, 0, "sse2 packsswb");
    testSse128Op(0, 0x66, 0x64, 0x1122334455667788ULL, 0x99aabbccddeeff00ULL, 0x122232ff807f0011ULL, 0x98abbb0080110000ULL, 0x0000ffffff00ff00ULL, 0xff000000ff000000ULL, 0, 0, "sse2 pcmpgtb");
    testSse128Op(0, 0x66, 0x65, 0x1122334455667788ULL, 0x99aabbccddeeff00ULL, 0x112133445567ffffULL, 0x99abbbccdded0000ULL, 0xffff00000000ffffULL, 0x00000000ffff0000ULL, 0, 0, "sse2 pcmpgtw");
    testSse128Op(0, 0x66, 0x66, 0x1122334455667788ULL, 0x99aabbccddeeff00ULL, 0x1122334355667789ULL, 0x99aabbccddeefeffULL, 0xffffffff00000000ULL, 0x00000000ffffffffULL, 0, 0, "sse2 pcmpgtd");
    testSse128Op(0, 0x66, 0x67, 0x007f008001230000ULL, 0xf830ff81ff80ffceULL, 0xabcd7fffffff0005ULL, 0x0001fffbfd12ff6aULL, 0x000000007f80ff00ULL, 0x0100000000ff0005ULL, 0, 0, "sse2 packuswb");
    testSse128Op(0, 0x66, 0x6b, 0x1234567800000000ULL, 0x0000007f00000080ULL, 0xffffffff00000005ULL, 0x0000abcd00007fffULL, 0x007f00807fff0000ULL, 0x7fff7fffffff0005ULL, 0, 0, "sse2 packssdw");
    testSse128Op(0, 0x66, 0x74, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x1100001100222200ULL, 0x0033000000444444ULL, 0xff0000ff00ffff00ULL, 0x00ff000000ffffffULL, 0, 0, "sse2 pcmpeqb");
    testSse128Op(0, 0x66, 0x75, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x1111001100222222ULL, 0x0033000000444444ULL, 0xffff00000000ffffULL, 0x000000000000ffffULL, 0, 0, "sse2 pcmpeqw");
    testSse128Op(0, 0x66, 0x76, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x1111111102222222ULL, 0x3333033344444444ULL, 0xffffffff00000000ULL, 0x00000000ffffffffULL, 0, 0, "sse2 pcmpeqd");
}

void testSse2Shift_0x171_0x172_0x173_0x1d1_0x1d2_0x1d3_0x1e1_0x1e2_0x1f1_0x1f2_0x1f3() {
    testSse128SubImmOp(0, 0x66, 0x71, 2, 4, 0x1111222233334444ULL, 0x1234567890abcdefULL, 0x0111022203330444ULL, 0x01230567090a0cdeULL, "sse2 psrlw imm");
    testSse128SubImmOp(0, 0x66, 0x71, 4, 4, 0x1111222233334444ULL, 0x1234567890abcdefULL, 0x0111022203330444ULL, 0x01230567f90afcdeULL, "sse2 psraw imm");
    testSse128SubImmOp(0, 0x66, 0x71, 6, 4, 0x1111222233334444ULL, 0x1234567890abcdefULL, 0x1110222033304440ULL, 0x234067800ab0def0ULL, "sse2 psllw imm");
    testSse128SubImmOp(0, 0x66, 0x72, 2, 4, 0x1111222233334444ULL, 0x1234567890abcdefULL, 0x0111122203333444ULL, 0x01234567090abcdeULL, "sse2 psrld imm");
    testSse128SubImmOp(0, 0x66, 0x72, 4, 4, 0x1111222233334444ULL, 0x1234567890abcdefULL, 0x0111122203333444ULL, 0x01234567f90abcdeULL, "sse2 psrad imm");
    testSse128SubImmOp(0, 0x66, 0x72, 6, 4, 0x1111222233334444ULL, 0x1234567890abcdefULL, 0x1112222033344440ULL, 0x234567800abcdef0ULL, "sse2 pslld imm");
    testSse128SubImmOp(0, 0x66, 0x73, 2, 4, 0x1111222233334444ULL, 0x8234567890abcdefULL, 0x0111122223333444ULL, 0x08234567890abcdeULL, "sse2 psrlq imm");
    testSse128SubImmOp(0, 0x66, 0x73, 3, 4, 0x1111222233334444ULL, 0x8234567890abcdefULL, 0x90abcdef11112222ULL, 0x0000000082345678ULL, "sse2 psrldq imm");
    testSse128SubImmOp(0, 0x66, 0x73, 6, 4, 0x1111222233334444ULL, 0x8234567890abcdefULL, 0x1112222333344440ULL, 0x234567890abcdef0ULL, "sse2 psllq imm");
    testSse128SubImmOp(0, 0x66, 0x73, 7, 4, 0x1111222233334444ULL, 0x8234567890abcdefULL, 0x3333444400000000ULL, 0x90abcdef11112222ULL, "sse2 pslldq imm");
    testSse128Op(0, 0x66, 0xd1, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 2, 0x7777777788888888ULL, 0x00100f000f230567ULL, 0x0ccc0ccc11111111ULL, 0, 0, "sse2 psrlw xmm");
    testSse128Op(0, 0x66, 0xd2, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 2, 0x7777777788888888ULL, 0x00100f000f234567ULL, 0x0ccccccc11111111ULL, 0, 0, "sse2 psrld xmm");
    testSse128Op(0, 0x66, 0xd3, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 2, 0x7777777788888888ULL, 0x00100f004f234567ULL, 0x0cccccccd1111111ULL, 0, 0, "sse2 psrlq xmm");
    testSse128Op(0, 0x66, 0xe1, 0xf2340178f0abcdefULL, 0x1234567887654321ULL, 2, 0x1122334455667788ULL, 0xfc8d005efc2af37bULL, 0x048d159ee1d910c8ULL, 0, 0, "sse2 psraw xmm");
    testSse128Op(0, 0x66, 0xe2, 0xf2340178f0abcdefULL, 0x1234567887654321ULL, 17, 0x1122334455667788ULL, 0xfffff91afffff855ULL, 0x0000091affffc3b2ULL, 0, 0, "sse2 psrad xmm");
    testSse128Op(0, 0x66, 0xf1, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 2, 0x7777777788888888ULL, 0x0100f004f2345678ULL, 0xcccccccc11101110ULL, 0, 0, "sse2 psllw xmm");
    testSse128Op(0, 0x66, 0xf2, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 2, 0x7777777788888888ULL, 0x0100f004f2345678ULL, 0xcccccccc11111110ULL, 0, 0, "sse2 pslld xmm");
    testSse128Op(0, 0x66, 0xf3, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 17, 0x7777777788888888ULL, 0x7802791a2b3c0000ULL, 0x6666888888880000ULL, 0, 0, "sse2 psllq xmm");
}

void testSse2Convert_0x12a_0x32a_0x12c_0x32c_0x12d_0x32d_0x15a_0x35a_0x15b_0x35b_0x1e6_0x3e6() {
    U64 i64Mixed = ((U64)(U32)(S32)-5000 << 32) | 5000;
    testSseMmx64Op(0, 0x66, 0x2a, XMM_SRC_LOW, XMM_SRC_HIGH, i64Mixed, bitsFromDouble(5000.0), bitsFromDouble(-5000.0), 0, 0, "sse2 cvtpi2pd");
    runCachedCvtpi2pdMmx("sse2 cvtpi2pd cached mmx");
    runCachedCvtpi2pdMem("sse2 cvtpi2pd cached mem");
    testSseReg32Op(0, 0xf2, 0x2a, XMM_SRC_LOW, XMM_SRC_HIGH, (U32)(S32)-5000, bitsFromDouble(-5000.0), XMM_SRC_HIGH, "sse2 cvtsi2sd neg");
    testSseReg32Op(0, 0xf2, 0x2a, XMM_SRC_LOW, XMM_SRC_HIGH, 0x7fffffff, bitsFromDouble((double)0x7fffffff), XMM_SRC_HIGH, "sse2 cvtsi2sd intmax");
    testSseReg32Op(0, 0xf2, 0x2a, XMM_SRC_LOW, XMM_SRC_HIGH, 0x80000000, bitsFromDouble((double)(S32)0x80000000), XMM_SRC_HIGH, "sse2 cvtsi2sd intmin");

    U64 hugePos = bitsFromDouble(12345678900.0);
    U64 hugeNeg = bitsFromDouble(-12345678900.0);
    U64 negFrac = bitsFromDouble(-5000.6);
    U64 posFrac = bitsFromDouble(5000.6);
    U64 negWhole = bitsFromDouble(-5000.0);
    U64 posTie = bitsFromDouble(2.5);
    U64 negTie = bitsFromDouble(-2.5);
    testSseMmx64ROp(0, 0x66, 0x2c, 0, hugePos, negFrac, 0xffffec7880000000ULL, 0, "sse2 cvttpd2pi neg frac");
    testSseMmx64ROp(0, 0x66, 0x2c, 0, hugeNeg, posFrac, 0x0000138880000000ULL, 0, "sse2 cvttpd2pi pos frac");
    testSseReg32ROp(0, 0xf2, 0x2c, 0, hugePos, ((U64)2 << 32) | 1, 0x80000000, 0, "sse2 cvttsd2si huge pos");
    testSseReg32ROp(0, 0xf2, 0x2c, 0, negFrac, ((U64)2 << 32) | 1, 0xffffec78, 0, "sse2 cvttsd2si trunc neg");
    testSseReg32ROp(0, 0xf2, 0x2c, 0, hugeNeg, ((U64)2 << 32) | 1, 0x80000000, 0, "sse2 cvttsd2si huge neg");
    testSseMmx64ROp(0, 0x66, 0x2d, 0, hugePos, negWhole, 0xffffec7880000000ULL, 0, "sse2 cvtpd2pi huge");
    testSseMmx64ROp(0, 0x66, 0x2d, 0, posTie, negTie, 0xfffffffe00000002ULL, 0, "sse2 cvtpd2pi nearest even");
    testSseReg32ROp(0, 0xf2, 0x2d, 0, hugePos, ((U64)2 << 32) | 1, 0x80000000, 0, "sse2 cvtsd2si huge");
    testSseReg32ROp(0, 0xf2, 0x2d, 0, negWhole, ((U64)2 << 32) | 1, 0xffffec78, 0, "sse2 cvtsd2si neg");
    testSseReg32ROp(0, 0xf2, 0x2d, 0, posTie, ((U64)2 << 32) | 1, 2, 0, "sse2 cvtsd2si nearest even");

    testSse128Op(0, 0x66, 0x5a, 0, 0, bitsFromDouble(-50.0), bitsFromDouble(175.0), 0x432f0000c2480000ULL, 0, 0, 0, "sse2 cvtpd2ps");
    testSse128Op(0, 0, 0x5a, 0, 0, ((U64)bitsFromFloat(175.0f) << 32) | bitsFromFloat(-50.0f), 0, bitsFromDouble(-50.0), bitsFromDouble(175.0), 0, 0, "sse2 cvtps2pd");
    testSse128Op(0, 0xf2, 0x5a, 0x1122334455667788ULL, 0x99aabbccddeeff00ULL, bitsFromDouble(-50.0), bitsFromDouble(175.0), (0x1122334400000000ULL | bitsFromFloat(-50.0f)), 0x99aabbccddeeff00ULL, 0, 0, "sse2 cvtsd2ss");
    testSse128Op(0, 0xf3, 0x5a, 0, 0x1122334455667788ULL, ((U64)bitsFromFloat(175.0f) << 32) | bitsFromFloat(-50.0f), 0, bitsFromDouble(-50.0), 0x1122334455667788ULL, 0, 0, "sse2 cvtss2sd");
    testSse128Op(0, 0x66, 0x5b, 0, 0, ((U64)bitsFromFloat(-50.0f) << 32) | bitsFromFloat(12345678900.0f), ((U64)bitsFromFloat(-12345678900.0f) << 32) | bitsFromFloat(1000000.0f), 0xffffffce80000000ULL, 0x80000000000f4240ULL, 0, 0, "sse2 cvtps2dq");
    testSse128Op(0, 0, 0x5b, 0, 0, 0x000000afffffffceULL, 0x000f424000000000ULL, ((U64)bitsFromFloat(175.0f) << 32) | bitsFromFloat(-50.0f), ((U64)bitsFromFloat(1000000.0f) << 32) | bitsFromFloat(0.0f), 0, 0, "sse2 cvtdq2ps");
    testSse128Op(0, 0xf3, 0x5b, 0, 0, ((U64)bitsFromFloat(-50.6f) << 32) | bitsFromFloat(12345678900.0f), ((U64)bitsFromFloat(1000000.6f) << 32) | bitsFromFloat(-12345678900.0f), 0xffffffce80000000ULL, 0x000f424080000000ULL, 0, 0, "sse2 cvttps2dq");
    testSse128Op(0, 0x66, 0xe6, 0x1111222233334444ULL, 0x5555666677778888ULL, hugePos, negFrac, 0xffffec7880000000ULL, 0, 0, 0, "sse2 cvttpd2dq neg frac");
    testSse128Op(0, 0x66, 0xe6, 0x1111222233334444ULL, 0x5555666677778888ULL, hugeNeg, posFrac, 0x0000138880000000ULL, 0, 0, 0, "sse2 cvttpd2dq pos frac");
    testSse128Op(0, 0xf2, 0xe6, 0x1111222233334444ULL, 0x5555666677778888ULL, hugePos, negWhole, 0xffffec7880000000ULL, 0, 0, 0, "sse2 cvtpd2dq");
    testSse128Op(0, 0xf3, 0xe6, 0x1111222233334444ULL, 0x5555666677778888ULL, 0xffffec78075bcd15ULL, 0x1234567890abcdefULL, bitsFromDouble(123456789.0), bitsFromDouble(-5000.0), 0, 0, "sse2 cvtdq2pd");
}

void testSse2CompareFlags_0x12e_0x12f() {
    struct CompareCase {
        U64 lhs;
        U64 rhs;
        U32 flags;
        const char* name;
    };
    const CompareCase cases[] = {
        {bitsFromDouble(1.0), bitsFromDouble(2.0), CF | DF, "less"},
        {bitsFromDouble(-1.0), bitsFromDouble(-1.0), ZF | DF, "equal"},
        {bitsFromDouble(1.0), bitsFromDouble(-1.0), DF, "greater"},
        {bitsFromDouble(-0.0), bitsFromDouble(0.0), ZF | DF, "signed zero equal"},
        {0x7ff8000000000000ULL, bitsFromDouble(-1.0), ZF | PF | CF | DF, "quiet nan unordered"},
        {bitsFromDouble(1.0), 0x7ff8000000000000ULL, ZF | PF | CF | DF, "rhs quiet nan unordered"}
    };
    for (int i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); ++i) {
        for (int opcode : {0x2e, 0x2f}) {
            for (int dst = 0; dst < 8; ++dst) {
                for (int src = 0; src < 8; ++src) {
                    if (dst == src) {
                        continue;
                    }
                    initSse();
                    setXmm(dst, cases[i].lhs, 0x3333333344444444ULL);
                    setXmm(src, cases[i].rhs, 0x5555555566666666ULL);
                    cpu->flags = SSE_FLAG_MASK | DF;
                    emitSseRegReg(0x66, opcode, dst, src);
                    runTestCPU();
                    if (((TestX86::actualFlags(cpu, true) ^ cases[i].flags) & (SSE_FLAG_MASK | DF)) != 0) {
                        failed("%s sse2 compare reg", cases[i].name);
                    }
                    verifyXmmMove(dst, cases[i].lhs, 0x3333333344444444ULL, src, cases[i].rhs, 0x5555555566666666ULL, cases[i].name);
                }

                initSse();
                setXmm(dst, cases[i].lhs, 0x3333333344444444ULL);
                memory->writeq(TEST_HEAP_ADDRESS + MEM_SRC, cases[i].rhs);
                memory->writeq(TEST_HEAP_ADDRESS + MEM_SRC + 8, 0x5555555566666666ULL);
                cpu->flags = SSE_FLAG_MASK | DF;
                emitSseRegMem(0x66, (U8)opcode, dst, MEM_SRC);
                runTestCPU();
                if (((TestX86::actualFlags(cpu, true) ^ cases[i].flags) & (SSE_FLAG_MASK | DF)) != 0) {
                    failed("%s sse2 compare mem", cases[i].name);
                }
                verifyOnlyXmmChanged(dst, cases[i].lhs, 0x3333333344444444ULL, cases[i].name);
            }
        }
    }

    testSse128ImmOp(0, 0xf2, 0xc2, 1, bitsFromDouble(1.0), 0x3333333344444444ULL, bitsFromDouble(2.0), 0x5555555566666666ULL, 0xffffffffffffffffULL, 0x3333333344444444ULL, "sse2 cmpltsd");
    testSse128ImmOp(0, 0xf2, 0xc2, 3, 0x7ff8000000000000ULL, 0x3333333344444444ULL, bitsFromDouble(2.0), 0x5555555566666666ULL, 0xffffffffffffffffULL, 0x3333333344444444ULL, "sse2 cmpunordsd");
}

void testSse2Arithmetic_0x151_0x351_0x154_0x155_0x156_0x157_0x158_0x358_0x159_0x359_0x15c_0x35c_0x15d_0x35d_0x15e_0x35e_0x15f_0x35f() {
    testSseReg32ROp(0, 0x66, 0x50, 0, bitsFromDouble(-5000.0), bitsFromDouble(5000.0), 1, 0xffffffffU, "sse2 movmskpd");
    testSseReg32ROp(0, 0x66, 0x50, 0, 0x8000000000000000ULL, 0xffffffffffffffffULL, 3, 0xffffffffU, "sse2 movmskpd signs");
    testSse128Op(0, 0x66, 0x51, 0, 0, bitsFromDouble(4.0), bitsFromDouble(9.0), bitsFromDouble(2.0), bitsFromDouble(3.0), 0, 0, "sse2 sqrtpd");
    testSse128Op(0, 0xf2, 0x51, bitsFromDouble(9.0), bitsFromDouble(9.0), bitsFromDouble(256.0), bitsFromDouble(9.0), bitsFromDouble(16.0), bitsFromDouble(9.0), 0, 0, "sse2 sqrtsd");
    testSse128Op(0, 0x66, 0x54, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_DEFAULT_LOW & XMM_SRC_LOW, XMM_DEFAULT_HIGH & XMM_SRC_HIGH, 0, 0, "sse2 andpd");
    testSse128Op(0, 0x66, 0x55, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, (~XMM_DEFAULT_LOW) & XMM_SRC_LOW, (~XMM_DEFAULT_HIGH) & XMM_SRC_HIGH, 0, 0, "sse2 andnpd");
    testSse128Op(0, 0x66, 0x56, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_DEFAULT_LOW | XMM_SRC_LOW, XMM_DEFAULT_HIGH | XMM_SRC_HIGH, 0, 0, "sse2 orpd");
    testSse128Op(0, 0x66, 0x57, XMM_DEFAULT_LOW, XMM_DEFAULT_HIGH, XMM_SRC_LOW, XMM_SRC_HIGH, XMM_DEFAULT_LOW ^ XMM_SRC_LOW, XMM_DEFAULT_HIGH ^ XMM_SRC_HIGH, 0, 0, "sse2 xorpd");
    testSse128Op(0, 0x66, 0x58, 0x4010000000000000ULL, 0x4022000000000000ULL, 0xc010000000000000ULL, 0x4024000000000000ULL, 0, 0x4033000000000000ULL, 0, 0, "sse2 addpd");
    testSse128Op(0, 0xf2, 0x58, bitsFromDouble(4.0), bitsFromDouble(9.0), bitsFromDouble(-4.0), bitsFromDouble(10.0), bitsFromDouble(0.0), bitsFromDouble(9.0), 0, 0, "sse2 addsd");
    testSse128Op(0, 0x66, 0x59, bitsFromDouble(4.0), bitsFromDouble(9.0), bitsFromDouble(-4.0), bitsFromDouble(10.0), bitsFromDouble(-16.0), bitsFromDouble(90.0), 0, 0, "sse2 mulpd");
    testSse128Op(0, 0xf2, 0x59, bitsFromDouble(4.0), bitsFromDouble(9.0), bitsFromDouble(-4.0), bitsFromDouble(10.0), bitsFromDouble(-16.0), bitsFromDouble(9.0), 0, 0, "sse2 mulsd");
    testSse128Op(0, 0x66, 0x5c, bitsFromDouble(4.0), bitsFromDouble(-0.1), bitsFromDouble(-5.0), bitsFromDouble(2.0), bitsFromDouble(9.0), bitsFromDouble(-2.1), 0, 0, "sse2 subpd");
    testSse128Op(0, 0xf2, 0x5c, bitsFromDouble(4.0), bitsFromDouble(-0.1), bitsFromDouble(-5.0), bitsFromDouble(2.0), bitsFromDouble(9.0), bitsFromDouble(-0.1), 0, 0, "sse2 subsd");
    testSse128Op(0, 0x66, 0x5d, bitsFromDouble(4.0), bitsFromDouble(0.1), bitsFromDouble(-5.0), bitsFromDouble(2.0), bitsFromDouble(-5.0), bitsFromDouble(0.1), 0, 0, "sse2 minpd");
    testSse128Op(0, 0x66, 0x5d, 0x7ff8000000000000ULL, bitsFromDouble(7.0), bitsFromDouble(7.0), 0x7ff8000000000000ULL, bitsFromDouble(7.0), 0x7ff8000000000000ULL, 0, 0, "sse2 minpd nan");
    testSse128Op(0, 0x66, 0x5d, bitsFromDouble(0.0), bitsFromDouble(-0.0), bitsFromDouble(-0.0), bitsFromDouble(0.0), bitsFromDouble(-0.0), bitsFromDouble(0.0), 0, 0, "sse2 minpd zero");
    testSse128Op(0, 0xf2, 0x5d, bitsFromDouble(4.0), bitsFromDouble(0.1), bitsFromDouble(-5.0), bitsFromDouble(2.0), bitsFromDouble(-5.0), bitsFromDouble(0.1), 0, 0, "sse2 minsd");
    testSse128Op(0, 0xf2, 0x5d, 0x7ff8000000000000ULL, bitsFromDouble(0.1), bitsFromDouble(-5.0), bitsFromDouble(2.0), bitsFromDouble(-5.0), bitsFromDouble(0.1), 0, 0, "sse2 minsd nan");
    testSse128Op(0, 0xf2, 0x5d, bitsFromDouble(0.0), bitsFromDouble(0.1), bitsFromDouble(-0.0), bitsFromDouble(2.0), bitsFromDouble(-0.0), bitsFromDouble(0.1), 0, 0, "sse2 minsd zero");
    testSse128Op(0, 0x66, 0x5e, bitsFromDouble(4.0), bitsFromDouble(-0.1), bitsFromDouble(8.0), bitsFromDouble(2.0), bitsFromDouble(0.5), bitsFromDouble(-0.05), 0, 0, "sse2 divpd");
    testSse128Op(0, 0xf2, 0x5e, bitsFromDouble(4.0), bitsFromDouble(-0.1), bitsFromDouble(8.0), bitsFromDouble(2.0), bitsFromDouble(0.5), bitsFromDouble(-0.1), 0, 0, "sse2 divsd");
    testSse128Op(0, 0x66, 0x5f, bitsFromDouble(4.0), 0x7ff8000000000000ULL, bitsFromDouble(-5.5), bitsFromDouble(2.0), bitsFromDouble(4.0), bitsFromDouble(2.0), 0, 0, "sse2 maxpd nan");
    testSse128Op(0, 0xf2, 0x5f, bitsFromDouble(4.0), bitsFromDouble(-2.5), bitsFromDouble(-5.5), bitsFromDouble(2.0), bitsFromDouble(4.0), bitsFromDouble(-2.5), 0, 0, "sse2 maxsd");
}

void testSse2Transfers_0x16e_0x17e_0x37e_0x1d6_0x3d6_0x3c3_0x1e7_0x1f7() {
    testSseReg32Op(0, 0x66, 0x6e, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x12345678, 0x0000000012345678ULL, 0, "sse2 movd xmm,r32");
    testSseE32ROp(0, 0x66, 0x7e, 0x12345678, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x22222222, 0, "sse2 movd r32,xmm");
    testSse128Op(0, 0xf3, 0x7e, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0x5555555566666666ULL, 0, 0, 0, "sse2 movq xmm,e64");
    testSse128ROp(0, 0x66, 0xd6, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0x5555555566666666ULL, 0, 0x5555555566666666ULL, 0x3333333344444444ULL, "sse2 movq e64,xmm");
    testSseMmx64ROp(0, 0xf2, 0xd6, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x3333333344444444ULL, 0xffffffffffffffffULL, "sse2 movdq2q");
    testSseMmx64Op(0, 0xf3, 0xd6, 0x1111111122222222ULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x5555555566666666ULL, 0, 0xffffffffffffffffULL, 0xffffffffffffffffULL, "sse2 movq2dq");
    runCachedMovdXmmR32("sse2 movd xmm,r32 cached");
    runCachedMovqXmmXmm("sse2 movq xmm,xmm cached");
    runCachedMovq2dq("sse2 movq2dq cached");
    for (int reg = 0; reg < 8; ++reg) {
        initSseMmx();
        cpu->reg[reg].u32 = 0x12348765;
        memory->writed(TEST_HEAP_ADDRESS + MEM_DST, 0xcdcdcdcd);
        pushCode8(0x0f);
        pushCode8(0xc3);
        emitDirectAddressModRM(reg, MEM_DST);
        runTestCPU();
        if (memory->readd(TEST_HEAP_ADDRESS + MEM_DST) != 0x12348765) {
            failed("sse2 movnti");
        }
    }
    testSse128ROp(0, 0x66, 0xe7, XMM_DEFAULT_LOW, XMM_DEFAULT_LOW, 0x1111222233334444ULL, 0x5555666677778888ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1111222233334444ULL, 0x5555666677778888ULL, "sse2 movntdq");
    for (int mask = 0; mask < 8; ++mask) {
        for (int src = 0; src < 8; ++src) {
            if (mask == src) {
                continue;
            }
            initSse();
            setXmm(mask, 0x8000800080008000ULL, 0x0080808000008080ULL);
            setXmm(src, 0x1122334455667788ULL, 0x99aabbccddeeff00ULL);
            cpu->reg[7].u32 = MEM_DST;
            writeXmmMem(MEM_DST, 0x9999999999999999ULL, 0x9999999999999999ULL);
            pushCode8(0x66);
            pushCode8(0x0f);
            pushCode8(0xf7);
            pushCode8(0xc0 | (src << 3) | mask);
            runTestCPU();
            if (memory->readq(TEST_HEAP_ADDRESS + MEM_DST) != 0x1199339955997799ULL ||
                    memory->readq(TEST_HEAP_ADDRESS + MEM_DST + 8) != 0x99aabbcc9999ff00ULL) {
                failed("sse2 maskmovdqu");
            }
        }
    }
    runSegmentedMaskmovdqu("sse2 maskmovdqu segmented");
}

void testSse2PackedArithmetic_0x1d4_0x1d5_0x1d7_0x1d8_0x1d9_0x1da_0x1db_0x1dc_0x1dd_0x1de_0x1df_0x1e0_0x1e3_0x1e4_0x1e5_0x1e8_0x1e9_0x1ea_0x1eb_0x1ec_0x1ed_0x1ee_0x1ef_0x1f4_0x1f5_0x1f6_0x1f8_0x1f9_0x1fa_0x1fb_0x1fc_0x1fd_0x1fe() {
    testSse128Op(0, 0x66, 0xd4, 0xf2345678f0abcdefULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0x4789abce57123455ULL, 0xaaaaaaaaccccccccULL, 0, 0, "sse2 paddq");
    testSse128Op(0, 0x66, 0xd5, 0xf2345678f0abcdefULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x7777777788888888ULL, 0x0af448dd80622473aULL, 0x81b581b564206420ULL, 0, 0, "sse2 pmullw");
    testSseReg32ROp(0, 0x66, 0xd7, 0xdeadbeef, 0xF055990011803344ULL, 0xA080112277C01177ULL, 0x0000c4a4, 0xffffffffU, "sse2 pmovmskb");
    testSse128Op(0, 0x66, 0xd8, 0xf2345678f0abcdefULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x1122334455667788ULL, 0x9d0001238a456789ULL, 0x2211000000000000ULL, 0, 0, "sse2 psubusb");
    testSse128Op(0, 0x66, 0xd9, 0xf2345678f0abcdefULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x1122334455667788ULL, 0x9cdf01238a456789ULL, 0x2211000000000000ULL, 0, 0, "sse2 psubusw");
    testSse128Op(0, 0x66, 0xda, 0xf2345678f0abcdefULL, 0x1234567887654321ULL, 0x55555555666666ffULL, 0x1122334455667788ULL, 0x55345555666666efULL, 0x1122334455654321ULL, 0, 0, "sse2 pminub");
    testSse128Op(0, 0x66, 0xdb, 0xf2340178f0abcdefULL, 0x1234567887654321ULL, 0x55550155666666ffULL, 0x1122334455667788ULL, 0x50140150602244efULL, 0x1020124005644300ULL, 0, 0, "sse2 pand");
    testSse128Op(0, 0x66, 0xdc, 0xf2340178f0abcdefULL, 0x1234567887654321ULL, 0x55550155666666ffULL, 0x1122334455667788ULL, 0xff8902cdffffffffULL, 0x235689bcdccbbaa9ULL, 0, 0, "sse2 paddusb");
    testSse128Op(0, 0x66, 0xdd, 0xf2340178f0abcdefULL, 0x1234567887654321ULL, 0x55550155666666ffULL, 0x1122334455667788ULL, 0xffff02cdffffffffULL, 0x235689bcdccbbaa9ULL, 0, 0, "sse2 paddusw");
    testSse128Op(0, 0x66, 0xde, 0xf2340178f0abcdefULL, 0x1234567887654321ULL, 0x55550155666666ffULL, 0x1122334455667788ULL, 0xf2550178f0abcdffULL, 0x1234567887667788ULL, 0, 0, "sse2 pmaxub");
    testSse128Op(0, 0x66, 0xdf, 0xf2340178f0abcdefULL, 0x1234567887654321ULL, 0x55550155666666ffULL, 0x1122334455667788ULL, 0x0541000506442210ULL, 0x0102210450023488ULL, 0, 0, "sse2 pandn");
    testSse128Op(0, 0x66, 0xe0, 0xf2340178f0abcdefULL, 0x1234567887654321ULL, 0x55550155666666ffULL, 0x1122334455667788ULL, 0xa4450167ab899af7ULL, 0x122b455e6e665d55ULL, 0, 0, "sse2 pavgb");
    testSse128Op(0, 0x66, 0xe3, 0xf2340178f0abcdefULL, 0x1234567887654321ULL, 0x55550155666666ffULL, 0x1122334455667788ULL, 0x0a3c50167ab899a77ULL, 0x11ab44de6e665d55ULL, 0, 0, "sse2 pavgw");
    testSse128Op(0, 0x66, 0xe4, 0xf2340178f0abcdefULL, 0x1234567887654321ULL, 0x55550155666666ffULL, 0x1122334455667788ULL, 0x50bb0001604452daULL, 0x013711502d2a1f58ULL, 0, 0, "sse2 pmulhuw");
    testSse128Op(0, 0x66, 0xe5, 0xf2340178f0abcdefULL, 0x1234567887654321ULL, 0x55550155666666ffULL, 0x1122334455667788ULL, 0xfb660001f9deebdbULL, 0x01371150d7c41f58ULL, 0, 0, "sse2 pmulhw");
    testSse128Op(0, 0x66, 0xe8, 0xf2345678f0abcdefULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x1122334455667788ULL, 0x9ddf01238a808089ULL, 0x221100efefdecd7fULL, 0, 0, "sse2 psubsb");
    testSse128Op(0, 0x66, 0xe9, 0xf2345678f0abcdefULL, 0x3333333344444444ULL, 0x5555555566666666ULL, 0x1122334455667788ULL, 0x9cdf01238a458000ULL, 0x2211ffefeedeccbcULL, 0, 0, "sse2 psubsw");
    testSse128Op(0, 0x66, 0xea, 0xf2345678f0abcdefULL, 0x3333003344004444ULL, 0x5555555566666666ULL, 0x1122334455667788ULL, 0xf2345555f0abcdefULL, 0x1122003344004444ULL, 0, 0, "sse2 pminsw");
    testSse128Op(0, 0x66, 0xeb, 0xf2345678f0abcdefULL, 0x3333003344004444ULL, 0x5555555566666666ULL, 0x1122334455667788ULL, 0xf775577df6efefefULL, 0x33333377556677ccULL, 0, 0, "sse2 por");
    testSse128Op(0, 0x66, 0xec, 0xf2345678f0abcdefULL, 0x3333003344004444ULL, 0x5555555566666666ULL, 0x1122334455667788ULL, 0x477f7f7f56113355ULL, 0x445533777f667fccULL, 0, 0, "sse2 paddsb");
    testSse128Op(0, 0x66, 0xed, 0xf2345678f0abcdefULL, 0x3333003344004444ULL, 0x5555555566666666ULL, 0x1122334455667788ULL, 0x47897fff57113455ULL, 0x445533777fff7fffULL, 0, 0, "sse2 paddsw");
    testSse128Op(0, 0x66, 0xee, 0xf2345678f0abcdefULL, 0x3333003344004444ULL, 0x5555555566666666ULL, 0x1122334455667788ULL, 0x5555567866666666ULL, 0x3333334455667788ULL, 0, 0, "sse2 pmaxsw");
    testSse128Op(0, 0x66, 0xef, 0xf2345678f0abcdefULL, 0x3333003344004444ULL, 0x5555555566666666ULL, 0x1122334455667788ULL, 0xa761032d96cdab89ULL, 0x22113377116633ccULL, 0, 0, "sse2 pxor");
    testSse128Op(0, 0x66, 0xf4, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 17, 0x7777777788888888ULL, 0x00000004055e6f7eULL, 0x2468acf0eca86420ULL, 0, 0, "sse2 pmuludq");
    testSse128Op(0, 0x66, 0xf5, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 17, 0x7777777788888888ULL, 0x0000000000016f7eULL, 0x2fc9036ac048c840ULL, 0, 0, "sse2 pmaddwd");
    testSse128Op(0, 0x66, 0xf6, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 0x2fc9036ac048c840ULL, 0x7802791a2b3c0000ULL, 0x0000000000000334ULL, 0x000000000000017eULL, 0, 0, "sse2 psadbw");
    testSse128Op(0, 0x66, 0xf8, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 0x2fc9036ac048c840ULL, 0x7802791a2b3c0000ULL, 0xd17739977c454d5eULL, 0xbb31ba1919084444ULL, 0, 0, "sse2 psubb");
    testSse128Op(0, 0x66, 0xf9, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 0x2fc9036ac048c840ULL, 0x7802791a2b3c0000ULL, 0xd07738977c454d5eULL, 0xbb31ba1919084444ULL, 0, 0, "sse2 psubw");
    testSse128Op(0, 0x66, 0xfa, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 0x2fc9036ac048c840ULL, 0x7802791a2b3c0000ULL, 0xd07738977c444d5eULL, 0xbb30ba1919084444ULL, 0, 0, "sse2 psubd");
    testSse128Op(0, 0x66, 0xfb, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 0x2fc9036ac048c840ULL, 0x7802791a2b3c0000ULL, 0xd07738967c444d5eULL, 0xbb30ba1919084444ULL, 0, 0, "sse2 psubq");
    testSse128Op(0, 0x66, 0xfc, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 0x2fc9036ac048c840ULL, 0x7802791a2b3c0000ULL, 0x2f093f6bfcd5dddeULL, 0xab35ac4d6f804444ULL, 0, 0, "sse2 paddb");
    testSse128Op(0, 0x66, 0xfd, 0x00403c013c8d159eULL, 0x3333333344444444ULL, 0x2fc9036ac048c840ULL, 0x7802791a2b3c0000ULL, 0x30093f6bfcd5dddeULL, 0xab35ac4d6f804444ULL, 0, 0, "sse2 paddw");
    testSse128Op(0, 0x66, 0xfe, 0x00403c013c8d159eULL, 0x33333333f4444444ULL, 0x2fc9036ac048c840ULL, 0x7802791a2b3c0000ULL, 0x30093f6bfcd5dddeULL, 0xab35ac4d1f804444ULL, 0, 0, "sse2 paddd");
    for (int dst = 0; dst < 8; ++dst) {
        for (int src = 0; src < 8; ++src) {
            if (dst == src) {
                continue;
            }
            initSseMmx();
            cpu->fpu.getMMX(dst)->q = 0x0102f007f2345678ULL;
            cpu->fpu.getMMX(src)->q = 0x5555555512345678ULL;
            emitMmxRegReg(0, 0xf4, dst, src);
            runTestCPU();
            if (cpu->fpu.getMMX(dst)->q != 0x113932851df4d840ULL) {
                failed("sse2 pmuludq mmx");
            }

            initSseMmx();
            cpu->fpu.getMMX(dst)->q = 0x33445566778899aaULL;
            cpu->fpu.getMMX(src)->q = 0x1188226699abcdefULL;
            emitMmxRegReg(0, 0xfb, dst, src);
            runTestCPU();
            if (cpu->fpu.getMMX(dst)->q != 0x21bc32ffdddccbbbULL) {
                failed("sse2 psubq mmx");
            }
        }
    }
}

#endif
