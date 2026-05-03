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

#include "testBCD.h"
#include "testCPU.h"
#include "testAsmJit.h"

#define cpu (testContext().cpu)
#define pushCode8 testPushCode8
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

constexpr U32 REG_GUARD = 0xA55A0000;
constexpr U32 DEFINED_ARITH_FLAGS = CF | PF | AF | ZF | SF;
constexpr U32 DEFINED_AAA_FLAGS = CF | AF;
constexpr U32 DEFINED_AAM_FLAGS = PF | ZF | SF;

enum BcdOp {
    BCD_DAA,
    BCD_DAS,
    BCD_AAA,
    BCD_AAS,
    BCD_AAM,
    BCD_AAD
};

struct BcdCase {
    U32 ax;
    U32 initialFlags;
    U8 base;
};

struct BcdExpected {
    U16 ax;
    U32 flags;
    U32 flagMask;
};

const BcdCase DAA_CASES[] = {
    {0x0000, 0, 10},
    {0x0009, AF | ZF | SF, 10},
    {0x000a, 0, 10},
    {0x0099, 0, 10},
    {0x009a, 0, 10},
    {0x00f9, CF, 10},
    {0x0003, CF | AF, 10},
    {0x00ff, CF | AF | SF | ZF | PF, 10}
};

const BcdCase DAS_CASES[] = {
    {0x0000, 0, 10},
    {0x0003, AF, 10},
    {0x000a, 0, 10},
    {0x0060, AF, 10},
    {0x009f, AF, 10},
    {0x0003, CF, 10},
    {0x0006, CF, 10},
    {0x0003, CF | AF | SF | ZF | PF, 10}
};

const BcdCase AAA_CASES[] = {
    {0x0205, 0, 10},
    {0x0205, AF, 10},
    {0x0306, AF, 10},
    {0x040a, 0, 10},
    {0x05fa, 0, 10},
    {0x12ff, CF | AF | SF | ZF | PF, 10}
};

const BcdCase AAS_CASES[] = {
    {0x0205, 0, 10},
    {0x0205, AF, 10},
    {0x0306, AF, 10},
    {0x040a, 0, 10},
    {0x05fa, 0, 10},
    {0x1200, CF | AF | SF | ZF | PF, 10}
};

const BcdCase AAM_CASES[] = {
    {0x0000, CF | AF | SF, 10},
    {0x0009, CF | AF | ZF, 10},
    {0x000a, CF | AF, 10},
    {0x0059, CF | AF | SF | ZF | PF, 10},
    {0x00ff, 0, 10},
    {0x00ff, 0, 16}
};

const BcdCase AAD_CASES[] = {
    {0x0000, CF | AF | SF, 10},
    {0x0102, CF | AF | ZF, 10},
    {0x0407, 0, 10},
    {0x0909, CF | AF | SF | ZF | PF, 10},
    {0x0fff, 0, 10},
    {0x0fff, 0, 16}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

U32 paritySignZero(U8 value) {
    U32 flags = 0;
    if (value == 0) {
        flags |= ZF;
    }
    if ((value & 0x80) != 0) {
        flags |= SF;
    }

    U8 low = value;
    low ^= low >> 4;
    low &= 0x0f;
    if (((0x6996 >> low) & 1) == 0) {
        flags |= PF;
    }
    return flags;
}

BcdExpected expectedDaa(U16 ax, U32 initialFlags) {
    U8 oldAl = (U8)ax;
    U8 al = oldAl;
    U32 flags = 0;

    if ((al & 0x0f) > 9 || (initialFlags & AF)) {
        al = (U8)(al + 6);
        flags |= AF;
    }
    if (oldAl > 0x99 || (initialFlags & CF)) {
        al = (U8)(al + 0x60);
        flags |= CF;
    }
    flags |= paritySignZero(al);
    return {(U16)((ax & 0xff00) | al), flags, DEFINED_ARITH_FLAGS};
}

BcdExpected expectedDas(U16 ax, U32 initialFlags) {
    U8 oldAl = (U8)ax;
    U8 al = oldAl;
    U32 flags = 0;

    if ((al & 0x0f) > 9 || (initialFlags & AF)) {
        if (al < 6) {
            flags |= CF;
        }
        al = (U8)(al - 6);
        flags |= AF;
    }
    if (oldAl > 0x99 || (initialFlags & CF)) {
        al = (U8)(al - 0x60);
        flags |= CF;
    }
    flags |= paritySignZero(al);
    return {(U16)((ax & 0xff00) | al), flags, DEFINED_ARITH_FLAGS};
}

BcdExpected expectedAaa(U16 ax, U32 initialFlags) {
    U32 result = ax;
    U32 flags = 0;

    if ((result & 0x0f) > 9 || (initialFlags & AF)) {
        result = (result + 0x106) & 0xffff;
        flags |= CF | AF;
    }
    result &= 0xff0f;
    return {(U16)result, flags, DEFINED_AAA_FLAGS};
}

BcdExpected expectedAas(U16 ax, U32 initialFlags) {
    U32 result = ax;
    U32 flags = 0;

    if ((result & 0x0f) > 9 || (initialFlags & AF)) {
        result = (result - 0x106) & 0xffff;
        flags |= CF | AF;
    }
    result &= 0xff0f;
    return {(U16)result, flags, DEFINED_AAA_FLAGS};
}

BcdExpected expectedAam(U16 ax, U8 base) {
    U8 al = (U8)ax;
    U16 result = (U16)(((al / base) << 8) | (al % base));
    return {result, paritySignZero((U8)result), DEFINED_AAM_FLAGS};
}

BcdExpected expectedAad(U16 ax, U8 base) {
    U8 al = (U8)ax;
    U8 ah = (U8)(ax >> 8);
    U8 resultAl = (U8)(al + ah * base);
    return {resultAl, paritySignZero(resultAl), DEFINED_AAM_FLAGS};
}

BcdExpected expected(BcdOp op, U16 ax, U32 initialFlags, U8 base) {
    BcdExpected calculated;
    if (op == BCD_DAA) {
        calculated = expectedDaa(ax, initialFlags);
    } else if (op == BCD_DAS) {
        calculated = expectedDas(ax, initialFlags);
    } else if (op == BCD_AAA) {
        calculated = expectedAaa(ax, initialFlags);
    } else if (op == BCD_AAS) {
        calculated = expectedAas(ax, initialFlags);
    } else if (op == BCD_AAM) {
        calculated = expectedAam(ax, base);
    } else {
        calculated = expectedAad(ax, base);
    }
    return calculated;
}

const char* opName(BcdOp op) {
    if (op == BCD_DAA) return "daa";
    if (op == BCD_DAS) return "das";
    if (op == BCD_AAA) return "aaa";
    if (op == BCD_AAS) return "aas";
    if (op == BCD_AAM) return "aam";
    return "aad";
}

#if defined(_MSC_VER) && defined(_M_IX86)
#define TEST_BINARY_HAS_HARDWARE_ORACLE 1

BcdExpected hardwareExpected(BcdOp op, U16 ax, U32 initialFlags, U8 base, U32 flagMask) {
    U32 axValue = ax;
    U32 initialEflags = initialFlags | 2;
    U32 result = 0;
    U32 flags = 0;

    if (op == BCD_DAA) {
        __asm {
            mov eax, initialEflags
            push eax
            popfd
            mov eax, axValue
            _emit 0x27
            pushfd
            pop edx
            mov result, eax
            mov flags, edx
        }
    } else if (op == BCD_DAS) {
        __asm {
            mov eax, initialEflags
            push eax
            popfd
            mov eax, axValue
            _emit 0x2f
            pushfd
            pop edx
            mov result, eax
            mov flags, edx
        }
    } else if (op == BCD_AAA) {
        __asm {
            mov eax, initialEflags
            push eax
            popfd
            mov eax, axValue
            _emit 0x37
            pushfd
            pop edx
            mov result, eax
            mov flags, edx
        }
    } else if (op == BCD_AAS) {
        __asm {
            mov eax, initialEflags
            push eax
            popfd
            mov eax, axValue
            _emit 0x3f
            pushfd
            pop edx
            mov result, eax
            mov flags, edx
        }
    } else if (op == BCD_AAM && base == 16) {
        __asm {
            mov eax, initialEflags
            push eax
            popfd
            mov eax, axValue
            _emit 0xd4
            _emit 0x10
            pushfd
            pop edx
            mov result, eax
            mov flags, edx
        }
    } else if (op == BCD_AAM) {
        __asm {
            mov eax, initialEflags
            push eax
            popfd
            mov eax, axValue
            _emit 0xd4
            _emit 0x0a
            pushfd
            pop edx
            mov result, eax
            mov flags, edx
        }
    } else if (base == 16) {
        __asm {
            mov eax, initialEflags
            push eax
            popfd
            mov eax, axValue
            _emit 0xd5
            _emit 0x10
            pushfd
            pop edx
            mov result, eax
            mov flags, edx
        }
    } else {
        __asm {
            mov eax, initialEflags
            push eax
            popfd
            mov eax, axValue
            _emit 0xd5
            _emit 0x0a
            pushfd
            pop edx
            mov result, eax
            mov flags, edx
        }
    }

    return {(U16)result, flags & flagMask, flagMask};
}
#else
#define TEST_BINARY_HAS_HARDWARE_ORACLE 0
#endif

BcdExpected expectedWithOracle(BcdOp op, U16 ax, U32 initialFlags, U8 base) {
    BcdExpected calculated = expected(op, ax, initialFlags, base);
#if TEST_BINARY_HAS_HARDWARE_ORACLE
    BcdExpected hardware = hardwareExpected(op, ax, initialFlags, base, calculated.flagMask);
    if (hardware.ax != calculated.ax || ((hardware.flags ^ calculated.flags) & calculated.flagMask) != 0) {
        failed("hardware %s oracle mismatch", opName(op));
    }
    return hardware;
#else
    return calculated;
#endif
}

void pushGeneratedCode(const asmjit::CodeHolder& code) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        pushCode8(buffer.data()[i]);
    }
}

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit bcd code init failed");
    }
}

void emitBcd(BcdOp op, U8 base) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Error err = asmjit::Error::kOk;

    if (op == BCD_DAA) {
        err = a.daa();
    } else if (op == BCD_DAS) {
        err = a.das();
    } else if (op == BCD_AAA) {
        err = a.aaa();
    } else if (op == BCD_AAS) {
        err = a.aas();
    } else if (op == BCD_AAM) {
        err = a.aam(asmjit::Imm(base));
    } else {
        err = a.aad(asmjit::Imm(base));
    }
    if (err != asmjit::Error::kOk) {
        failed("asmjit %s failed", opName(op));
    }
    pushGeneratedCode(code);
}

void overwriteFlags() {
    pushCode8(0x39);
    pushCode8(0xc0); // cmp eax, eax
}

U32 actualFlags() {
    U32 flags = 0;
    if (cpu->getCF()) flags |= CF;
    if (cpu->getPF()) flags |= PF;
    if (cpu->getAF()) flags |= AF;
    if (cpu->getZF()) flags |= ZF;
    if (cpu->getSF()) flags |= SF;
    if (cpu->getOF()) flags |= OF;
    return flags;
}

void initRegisters(U16 ax, U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    cpu->reg[0].u16 = ax;
    expectedRegs[0] = (expectedRegs[0] & 0xffff0000) | ax;
}

void verifyRegisters(const U32* expectedRegs, const char* name) {
    for (int i = 0; i < 8; ++i) {
        if (cpu->reg[i].u32 != expectedRegs[i]) {
            failed("%s register value", name);
        }
    }
}

void runCase(BcdOp op, const BcdCase& data, bool checkFlags, const char* name) {
    U32 expectedRegs[8];
    BcdExpected expectedValue = expectedWithOracle(op, (U16)data.ax, data.initialFlags, data.base);

    newInstruction(data.initialFlags);
    cpu->big = true;
    emitBcd(op, data.base);
    if (!checkFlags) {
        overwriteFlags();
    }
    initRegisters((U16)data.ax, expectedRegs);
    expectedRegs[0] = (expectedRegs[0] & 0xffff0000) | expectedValue.ax;

    runTestCPU();
    verifyRegisters(expectedRegs, name);
    if (checkFlags && ((actualFlags() ^ expectedValue.flags) & expectedValue.flagMask) != 0) {
        failed("%s flags", name);
    }
}

void runCases(BcdOp op, const BcdCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        runCase(op, cases[i], true, name);
        runCase(op, cases[i], false, name);
    }
}

} // namespace

void testDaa_0x027() {
    runCases(BCD_DAA, DAA_CASES, caseCount(DAA_CASES), "daa");
}

void testDas_0x02f() {
    runCases(BCD_DAS, DAS_CASES, caseCount(DAS_CASES), "das");
}

void testAaa_0x037() {
    runCases(BCD_AAA, AAA_CASES, caseCount(AAA_CASES), "aaa");
}

void testAas_0x03f() {
    runCases(BCD_AAS, AAS_CASES, caseCount(AAS_CASES), "aas");
}

void testAam_0x0d4() {
    runCases(BCD_AAM, AAM_CASES, caseCount(AAM_CASES), "aam");
}

void testAad_0x0d5() {
    runCases(BCD_AAD, AAD_CASES, caseCount(AAD_CASES), "aad");
}

#endif
