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

#include "testMulDiv.h"
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
constexpr U32 INITIAL_FLAGS = CF | PF | AF | ZF | SF | OF | DF;
constexpr U32 MUL_FLAG_MASK = CF | OF;

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

enum MulDivOp {
    OP_MUL,
    OP_IMUL,
    OP_DIV,
    OP_IDIV
};

enum ImulForm {
    IMUL_RM,
    IMUL_IMM,
    IMUL_IMM8
};

enum FlagMode {
    FLAGS_CHECKED,
    FLAGS_OVERWRITTEN
};

struct OneOpCase {
    U32 lo;
    U32 hi;
    U32 src;
};

struct ImulCase {
    U32 dst;
    U32 src;
    U32 imm;
};

struct Expected {
    U32 lo;
    U32 hi;
    U32 flags;
};

struct AddressCase {
    U32 address;
    asmjit::x86::Mem operand;
};

const OneOpCase MUL8_CASES[] = {
    {2, 0, 2},
    {0, 0, 0},
    {0x20, 0, 0x10},
    {0xfa, 0, 2}
};

const OneOpCase MUL16_CASES[] = {
    {2, 0, 2},
    {0, 0, 0},
    {0x2001, 0, 0x10},
    {0x2001, 0, 0x1000},
    {0xfffa, 0, 2}
};

const OneOpCase MUL32_CASES[] = {
    {2, 0, 2},
    {0, 0, 0},
    {0x20000001, 0, 0x10},
    {0x20000001, 0, 0x00010000},
    {0xfffffffa, 0, 2}
};

const OneOpCase IMUL8_CASES[] = {
    {2, 0, 2},
    {0xfa, 0, 2},
    {0xfa, 0, 0x9c},
    {0, 0, 0xff},
    {0x40, 0, 2}
};

const OneOpCase ONE_IMUL16_CASES[] = {
    {2, 0, 2},
    {0xfffa, 0, 2},
    {(U32)(S16)-600, 0, 30000},
    {0xfffa, 0, 0xff9c},
    {0x4000, 0, 2}
};

const OneOpCase ONE_IMUL32_CASES[] = {
    {2, 0, 2},
    {0xfffffffa, 0, 2},
    {(U32)-60000, 0, 3000000},
    {0x40000000, 0, 2}
};

const OneOpCase DIV8_CASES[] = {
    {10, 0, 3},
    {1003, 0, 200}
};

const OneOpCase DIV16_CASES[] = {
    {10, 0, 3},
    {0x8512, 0x00cb, 3000}
};

const OneOpCase DIV32_CASES[] = {
    {10, 0, 3},
    {0x85121234, 0x000000cb, 0x12345678}
};

const OneOpCase IDIV8_CASES[] = {
    {10, 0, 0xfd},
    {10, 0, 3},
    {10, 0, (U8)(S8)-3},
    {(U16)(S16)-1003, 0, (U8)(S8)-100}
};

const OneOpCase IDIV16_CASES[] = {
    {10, 0, 0xfffd},
    {10, 0, 3},
    {0x8512, 0x00cb, 3000},
    {0x7aee, 0xff34, 3000}
};

const OneOpCase IDIV32_CASES[] = {
    {10, 0, 3},
    {10, 0, 0xfffffffd},
    {0x85121234, 0x000000cb, 0x12345678},
    {0x7aededcc, 0xffffff34, 0x12345678}
};

const ImulCase TWO_IMUL16_CASES[] = {
    {0, 2, 2},
    {0, 0xfffe, 2},
    {0, 0xfffe, 0xfffe},
    {0, 300, 300},
    {0, (U16)(S16)-300, 300},
    {0, 0x4000, 2}
};

const ImulCase TWO_IMUL32_CASES[] = {
    {0, 2, 2},
    {0, 0xfffffffe, 2},
    {0, 0xfffffffe, 0xfffffffe},
    {0, 300000, 400000},
    {0, (U32)-300000, 400000},
    {0, 0x40000000, 2}
};

const ImulCase IMUL16_IMM8_CASES[] = {
    {0, 2, 2},
    {0, 0xfffe, 2},
    {0, 0xfffe, 0xfe},
    {0, 3000, 127},
    {0, (U16)(S16)-3000, 127},
    {0, 0x4000, 2}
};

const ImulCase IMUL32_IMM8_CASES[] = {
    {0, 2, 2},
    {0, 0xfffffffe, 2},
    {0, 0xfffffffe, 0xfe},
    {0, 300000000, 127},
    {0, (U32)-300000000, 127},
    {0, 0x40000000, 2}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit mul/div code init failed");
    }
}

void pushGeneratedCode(const asmjit::CodeHolder& code) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        pushCode8(buffer.data()[i]);
    }
}

asmjit::Error emitOneOperand(asmjit::x86::Assembler& a, MulDivOp op, const asmjit::x86::Gp& src) {
    if (op == OP_MUL) return a.mul(src);
    if (op == OP_IMUL) return a.imul(src);
    if (op == OP_DIV) return a.div(src);
    return a.idiv(src);
}

asmjit::Error emitOneOperand(asmjit::x86::Assembler& a, MulDivOp op, const asmjit::x86::Mem& src) {
    if (op == OP_MUL) return a.mul(src);
    if (op == OP_IMUL) return a.imul(src);
    if (op == OP_DIV) return a.div(src);
    return a.idiv(src);
}

void emitOneOperandReg(MulDivOp op, int width, int srcReg) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (emitOneOperand(a, op, regForWidth(srcReg, width)) != asmjit::Error::kOk) {
        failed("asmjit mul/div register emit failed");
    }
    pushGeneratedCode(code);
}

void emitOneOperandMem(MulDivOp op, const AddressCase& address) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (emitOneOperand(a, op, address.operand) != asmjit::Error::kOk) {
        failed("asmjit mul/div memory emit failed");
    }
    pushGeneratedCode(code);
}

asmjit::Error emitImul(asmjit::x86::Assembler& a, ImulForm form, const asmjit::x86::Gp& dst, const asmjit::x86::Gp& src, U32 imm) {
    if (form == IMUL_RM) {
        return a.imul(dst, src);
    }
    if (form == IMUL_IMM) {
        return a.long_().imul(dst, src, asmjit::Imm(imm));
    }
    return a.short_().imul(dst, src, asmjit::Imm((S8)imm));
}

asmjit::Error emitImul(asmjit::x86::Assembler& a, ImulForm form, const asmjit::x86::Gp& dst, const asmjit::x86::Mem& src, U32 imm) {
    if (form == IMUL_RM) {
        return a.imul(dst, src);
    }
    if (form == IMUL_IMM) {
        return a.long_().imul(dst, src, asmjit::Imm(imm));
    }
    return a.short_().imul(dst, src, asmjit::Imm((S8)imm));
}

void emitImulReg(ImulForm form, int width, int dstReg, int srcReg, U32 imm) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (emitImul(a, form, regForWidth(dstReg, width), regForWidth(srcReg, width), imm) != asmjit::Error::kOk) {
        failed("asmjit imul register emit failed");
    }
    pushGeneratedCode(code);
}

void emitImulMem(ImulForm form, int width, int dstReg, const AddressCase& address, U32 imm) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (emitImul(a, form, regForWidth(dstReg, width), address.operand, imm) != asmjit::Error::kOk) {
        failed("asmjit imul memory emit failed");
    }
    pushGeneratedCode(code);
}

void overwriteFlagsIfNeeded(FlagMode flagMode) {
    if (flagMode == FLAGS_OVERWRITTEN) {
        asmjit::CodeHolder code;
        initCode(code);
        asmjit::x86::Assembler a(&code);
        if (a.cmp(asmjit::x86::eax, asmjit::x86::eax) != asmjit::Error::kOk) {
            failed("asmjit cmp emit failed");
        }
        pushGeneratedCode(code);
    }
}

U32 actualMulFlags() {
    U32 flags = 0;
    if (cpu->getCF()) flags |= CF;
    if (cpu->getOF()) flags |= OF;
    return flags;
}

void initRegisters(U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    cpu->reg[R_SP].u32 = MEM_BASE + 0x500;
    expectedRegs[R_SP] = cpu->reg[R_SP].u32;
}

void setAccumulatorDividend(U32 lo, U32 hi, int width) {
    if (width == 8) {
        cpu->reg[R_AX].u16 = (U16)lo;
    } else if (width == 16) {
        cpu->reg[R_AX].u16 = (U16)lo;
        cpu->reg[R_DX].u16 = (U16)hi;
    } else {
        cpu->reg[R_AX].u32 = lo;
        cpu->reg[R_DX].u32 = hi;
    }
}

void applyAccumulatorExpected(U32* expectedRegs, const Expected& e, int width) {
    if (width == 8) {
        U32 ax = ((e.hi & 0xff) << 8) | (e.lo & 0xff);
        applyRegValue(expectedRegs, R_AX, 16, ax);
    } else if (width == 16) {
        applyRegValue(expectedRegs, R_AX, 16, e.lo);
        applyRegValue(expectedRegs, R_DX, 16, e.hi);
    } else {
        applyRegValue(expectedRegs, R_AX, 32, e.lo);
        applyRegValue(expectedRegs, R_DX, 32, e.hi);
    }
}

const char* opName(MulDivOp op) {
    if (op == OP_MUL) return "mul";
    if (op == OP_IMUL) return "imul";
    if (op == OP_DIV) return "div";
    return "idiv";
}

Expected calculatedOneOperand(MulDivOp op, U32 lo, U32 hi, U32 src, int width) {
    U64 mask = widthMask64(width);
    src &= (U32)mask;
    hi &= (U32)mask;

    if (op == OP_MUL) {
        lo &= (U32)mask;
        U64 product = (U64)lo * (U64)src;
        U32 resultLo = (U32)(product & mask);
        U32 resultHi = (U32)((product >> width) & mask);
        U32 flags = resultHi ? (CF | OF) : 0;
        return {resultLo, resultHi, flags};
    }
    if (op == OP_IMUL) {
        lo &= (U32)mask;
        S64 product = signExtend(lo, width) * signExtend(src, width);
        U32 resultLo = (U32)((U64)product & mask);
        U32 resultHi = (U32)(((U64)product >> width) & mask);
        S64 truncated = signExtend(resultLo, width);
        U32 flags = truncated != product ? (CF | OF) : 0;
        return {resultLo, resultHi, flags};
    }
    if (op == OP_DIV) {
        U64 dividend = width == 8 ? lo & 0xffff : ((U64)hi << width) | (lo & (U32)mask);
        U64 quotient = dividend / src;
        U64 remainder = dividend % src;
        return {(U32)quotient, (U32)remainder, 0};
    }

    S64 dividend;
    if (width == 8) {
        dividend = (S16)(U16)lo;
    } else if (width == 16) {
        dividend = (S32)((hi << 16) | (lo & (U32)mask));
    } else {
        dividend = ((S64)(S32)hi << 32) | (lo & (U32)mask);
    }
    S64 divisor = signExtend(src, width);
    S64 quotient = dividend / divisor;
    S64 remainder = dividend % divisor;
    return {(U32)quotient & (U32)mask, (U32)remainder & (U32)mask, 0};
}

Expected calculatedImul(U32 src, U32 imm, int width, bool imm8) {
    S64 signedImm = imm8 ? (S8)imm : signExtend(imm, width);
    S64 product = signExtend(src, width) * signedImm;
    U32 result = (U32)((U64)product & widthMask64(width));
    U32 flags = signExtend(result, width) != product ? (CF | OF) : 0;
    return {result, 0, flags};
}

#if defined(_MSC_VER) && defined(_M_IX86)
#define TEST_BINARY_HAS_HARDWARE_ORACLE 1

Expected hardwareOneOperand(MulDivOp op, U32 lo, U32 hi, U32 src, int width) {
    U32 resultLo = 0;
    U32 resultHi = 0;
    U32 flags = 0;

    if (op == OP_MUL) {
        if (width == 8) {
            __asm {
                mov eax, lo
                mov ecx, src
                mul cl
                pushfd
                pop ebx
                mov resultLo, eax
                mov resultHi, eax
                mov flags, ebx
            }
        } else if (width == 16) {
            __asm {
                mov eax, lo
                mov ecx, src
                mul cx
                pushfd
                pop ebx
                mov resultLo, eax
                mov resultHi, edx
                mov flags, ebx
            }
        } else {
            __asm {
                mov eax, lo
                mov ecx, src
                mul ecx
                pushfd
                pop ebx
                mov resultLo, eax
                mov resultHi, edx
                mov flags, ebx
            }
        }
    } else if (op == OP_IMUL) {
        if (width == 8) {
            __asm {
                mov eax, lo
                mov ecx, src
                imul cl
                pushfd
                pop ebx
                mov resultLo, eax
                mov resultHi, eax
                mov flags, ebx
            }
        } else if (width == 16) {
            __asm {
                mov eax, lo
                mov ecx, src
                imul cx
                pushfd
                pop ebx
                mov resultLo, eax
                mov resultHi, edx
                mov flags, ebx
            }
        } else {
            __asm {
                mov eax, lo
                mov ecx, src
                imul ecx
                pushfd
                pop ebx
                mov resultLo, eax
                mov resultHi, edx
                mov flags, ebx
            }
        }
    } else if (op == OP_DIV) {
        if (width == 8) {
            __asm {
                mov eax, lo
                mov ecx, src
                div cl
                mov resultLo, eax
                mov resultHi, eax
            }
        } else if (width == 16) {
            __asm {
                mov eax, lo
                mov edx, hi
                mov ecx, src
                div cx
                mov resultLo, eax
                mov resultHi, edx
            }
        } else {
            __asm {
                mov eax, lo
                mov edx, hi
                mov ecx, src
                div ecx
                mov resultLo, eax
                mov resultHi, edx
            }
        }
    } else {
        if (width == 8) {
            __asm {
                mov eax, lo
                mov ecx, src
                idiv cl
                mov resultLo, eax
                mov resultHi, eax
            }
        } else if (width == 16) {
            __asm {
                mov eax, lo
                mov edx, hi
                mov ecx, src
                idiv cx
                mov resultLo, eax
                mov resultHi, edx
            }
        } else {
            __asm {
                mov eax, lo
                mov edx, hi
                mov ecx, src
                idiv ecx
                mov resultLo, eax
                mov resultHi, edx
            }
        }
    }

    if (width == 8) {
        return {resultLo & 0xff, (resultHi >> 8) & 0xff, flags & MUL_FLAG_MASK};
    }
    return {resultLo & widthMask(width), resultHi & widthMask(width), flags & MUL_FLAG_MASK};
}

Expected hardwareImul(U32 dst, U32 src, int width) {
    U32 result = 0;
    U32 flags = 0;

    if (width == 16) {
        __asm {
            mov eax, dst
            mov ecx, src
            imul ax, cx
            pushfd
            pop edx
            mov result, eax
            mov flags, edx
        }
    } else {
        __asm {
            mov eax, dst
            mov ecx, src
            imul eax, ecx
            pushfd
            pop edx
            mov result, eax
            mov flags, edx
        }
    }

    return {result & widthMask(width), 0, flags & MUL_FLAG_MASK};
}
#else
#define TEST_BINARY_HAS_HARDWARE_ORACLE 0
#endif

Expected expectedOneOperand(MulDivOp op, U32 lo, U32 hi, U32 src, int width) {
    Expected calculated = calculatedOneOperand(op, lo, hi, src, width);
#if TEST_BINARY_HAS_HARDWARE_ORACLE
    Expected hardware = hardwareOneOperand(op, lo, hi, src, width);
    if (hardware.lo != calculated.lo ||
            hardware.hi != calculated.hi ||
            ((op == OP_MUL || op == OP_IMUL) && ((hardware.flags ^ calculated.flags) & MUL_FLAG_MASK) != 0)) {
        failed("hardware %s oracle mismatch", opName(op));
    }
    return hardware;
#else
    return calculated;
#endif
}

Expected expectedImul(ImulForm form, U32 dst, U32 src, U32 imm, int width, bool imm8) {
    Expected calculated = form == IMUL_RM ? calculatedImul(dst, src, width, false) : calculatedImul(src, imm, width, imm8);
#if TEST_BINARY_HAS_HARDWARE_ORACLE
    if (form == IMUL_RM) {
        Expected hardware = hardwareImul(dst, src, width);
        if (hardware.lo != calculated.lo || ((hardware.flags ^ calculated.flags) & MUL_FLAG_MASK) != 0) {
            failed("hardware imul oracle mismatch");
        }
        return hardware;
    }
#endif
    return calculated;
}

void verifyMulFlags(U32 expectedFlags, const char* name) {
    if (((actualMulFlags() ^ expectedFlags) & MUL_FLAG_MASK) != 0) {
        failed("%s flags", name);
    }
}

U32 segmentBaseForReg(int reg) {
    return (reg == R_BP || reg == R_SP) ? cpu->seg[SS].address : cpu->seg[DS].address;
}

void makeAbsoluteCase(AddressCase& data, U32 offset, int width) {
    data.address = cpu->seg[DS].address + offset;
    data.operand = memPtr(offset, width);
}

void makeBaseCase(AddressCase& data, int base, U32 offset, S32 disp, int width) {
    data.address = segmentBaseForReg(base) + offset + disp;
    data.operand = memPtr(reg32(base), disp, width);
}

void makeSibCase(AddressCase& data, int base, int index, int shift, U32 targetOffset, const U32* regs, int width) {
    data.address = segmentBaseForReg(base) + targetOffset;
    data.operand = memPtr(reg32(base), reg32(index), shift, (S32)(targetOffset - regs[base] - (regs[index] << shift)), width);
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

void prepareMemSource(const AddressCase& address, U32 value, int width) {
    if (width == 8) {
        memory->writeb(address.address - 1, 0x11);
        memory->writeb(address.address, value);
        memory->writeb(address.address + 1, 0x33);
        memory->writeb(address.address + 2, 0x44);
        memory->writeb(address.address + 3, 0x55);
    } else if (width == 16) {
        memory->writew(address.address - 2, 0x1111);
        memory->writew(address.address, value);
        memory->writew(address.address + 2, 0x3333);
    } else {
        memory->writew(address.address - 2, 0x1111);
        memory->writed(address.address, value);
        memory->writew(address.address + 4, 0x3333);
    }
}

void verifyMemSource(const AddressCase& address, U32 value, int width, const char* name) {
    if (width == 8) {
        if (memory->readb(address.address) != (value & 0xff) ||
                memory->readb(address.address - 1) != 0x11 ||
                memory->readb(address.address + 1) != 0x33 ||
                memory->readb(address.address + 2) != 0x44 ||
                memory->readb(address.address + 3) != 0x55) {
            failed("%s memory source", name);
        }
    } else if (width == 16) {
        if (memory->readw(address.address) != (value & 0xffff) ||
                memory->readw(address.address - 2) != 0x1111 ||
                memory->readw(address.address + 2) != 0x3333) {
            failed("%s memory source", name);
        }
    } else if (memory->readd(address.address) != value ||
            memory->readw(address.address - 2) != 0x1111 ||
            memory->readw(address.address + 4) != 0x3333) {
        failed("%s memory source", name);
    }
}

void beginInstruction(U32 flags) {
    newInstruction(flags);
    cpu->big = true;
}

void runOneOperandRegCase(MulDivOp op, int width, const OneOpCase& data, int srcReg, FlagMode flagMode, const char* name) {
    U32 expectedRegs[8];

    beginInstruction(INITIAL_FLAGS);
    emitOneOperandReg(op, width, srcReg);
    overwriteFlagsIfNeeded(flagMode);
    initRegisters(expectedRegs);
    setAccumulatorDividend(data.lo, data.hi, width);
    setRegValue(cpu, srcReg, width, data.src);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }

    U32 actualLo = width == 8 ? data.lo : getRegValue(cpu, R_AX, width);
    U32 actualHi = width == 8 ? 0 : getRegValue(cpu, R_DX, width);
    U32 actualSrc = getRegValue(cpu, srcReg, width);
    Expected e = expectedOneOperand(op, actualLo, actualHi, actualSrc, width);
    applyAccumulatorExpected(expectedRegs, e, width);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (flagMode == FLAGS_CHECKED && (op == OP_MUL || op == OP_IMUL)) {
        verifyMulFlags(e.flags, name);
    }
}

void runOneOperandPreparedMemCase(MulDivOp op, int width, const OneOpCase& data, const AddressCase& address, FlagMode flagMode, const char* name) {
    U32 expectedRegs[8];

    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    setAccumulatorDividend(data.lo, data.hi, width);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    prepareMemSource(address, data.src, width);
    Expected e = expectedOneOperand(op, width == 8 ? data.lo : getRegValue(cpu, R_AX, width), width == 8 ? 0 : getRegValue(cpu, R_DX, width), data.src, width);
    applyAccumulatorExpected(expectedRegs, e, width);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    verifyMemSource(address, data.src, width, name);
    if (flagMode == FLAGS_CHECKED && (op == OP_MUL || op == OP_IMUL)) {
        verifyMulFlags(e.flags, name);
    }
}

void runImulRegCase(ImulForm form, int width, const ImulCase& data, int dstReg, int srcReg, FlagMode flagMode, const char* name) {
    U32 expectedRegs[8];

    beginInstruction(INITIAL_FLAGS);
    emitImulReg(form, width, dstReg, srcReg, data.imm);
    overwriteFlagsIfNeeded(flagMode);
    initRegisters(expectedRegs);
    setRegValue(cpu, dstReg, width, data.dst);
    setRegValue(cpu, srcReg, width, data.src);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }

    Expected e = expectedImul(form, getRegValue(cpu, dstReg, width), getRegValue(cpu, srcReg, width), data.imm, width, form == IMUL_IMM8);
    applyRegValue(expectedRegs, dstReg, width, e.lo);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (flagMode == FLAGS_CHECKED) {
        verifyMulFlags(e.flags, name);
    }
}

void runImulPreparedMemCase(ImulForm form, int width, const ImulCase& data, int dstReg, const AddressCase& address, FlagMode flagMode, const char* name) {
    U32 expectedRegs[8];

    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    setRegValue(cpu, dstReg, width, data.dst);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    prepareMemSource(address, data.src, width);

    Expected e = expectedImul(form, getRegValue(cpu, dstReg, width), data.src, data.imm, width, form == IMUL_IMM8);
    applyRegValue(expectedRegs, dstReg, width, e.lo);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    verifyMemSource(address, data.src, width, name);
    if (flagMode == FLAGS_CHECKED) {
        verifyMulFlags(e.flags, name);
    }
}

void runBaseMemoryCases(MulDivOp op, int width, const OneOpCase& data, FlagMode flagMode, const char* name) {
    for (int base = 0; base < 8; ++base) {
        if (base == R_AX || base == R_DX) {
            continue;
        }
        U32 regs[8];
        initAddressRegisters(regs);
        regs[base] = MEM_BASE + 0x1000 + base * 0x80;

        if (base != R_BP) {
            AddressCase address;
            makeBaseCase(address, base, regs[base], 0, width);
            beginInstruction(INITIAL_FLAGS);
            emitOneOperandMem(op, address);
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runOneOperandPreparedMemCase(op, width, data, address, flagMode, name);
        }

        AddressCase disp8;
        makeBaseCase(disp8, base, regs[base], 0x11, width);
        beginInstruction(INITIAL_FLAGS);
        emitOneOperandMem(op, disp8);
        overwriteFlagsIfNeeded(flagMode);
        writeRegs(cpu, regs);
        runOneOperandPreparedMemCase(op, width, data, disp8, flagMode, name);

        AddressCase disp32;
        makeBaseCase(disp32, base, regs[base], 0x123, width);
        beginInstruction(INITIAL_FLAGS);
        emitOneOperandMem(op, disp32);
        overwriteFlagsIfNeeded(flagMode);
        writeRegs(cpu, regs);
        runOneOperandPreparedMemCase(op, width, data, disp32, flagMode, name);
    }
}

void runAbsoluteMemoryCase(MulDivOp op, int width, const OneOpCase& data, FlagMode flagMode, const char* name) {
    U32 regs[8];
    AddressCase address;
    initAddressRegisters(regs);
    makeAbsoluteCase(address, MEM_BASE + 0x3000, width);
    beginInstruction(INITIAL_FLAGS);
    emitOneOperandMem(op, address);
    overwriteFlagsIfNeeded(flagMode);
    writeRegs(cpu, regs);
    runOneOperandPreparedMemCase(op, width, data, address, flagMode, name);
}

void runSibMemoryCases(MulDivOp op, int width, const OneOpCase& data, FlagMode flagMode, const char* name) {
    for (int base = 0; base < 8; ++base) {
        if (base == R_AX || base == R_DX) {
            continue;
        }
        for (int index = 0; index < 8; ++index) {
            if (index == R_SP || index == R_AX || index == R_DX) {
                continue;
            }
            for (int shift = 0; shift < 4; ++shift) {
                U32 regs[8];
                AddressCase address;
                U32 targetOffset = MEM_BASE + 0x7000 + base * 0x200 + index * 0x20 + shift * 4;
                initAddressRegisters(regs);
                regs[base] = MEM_BASE + 0x0100 + base * 0x40;
                regs[index] = 3;
                makeSibCase(address, base, index, shift, targetOffset, regs, width);
                beginInstruction(INITIAL_FLAGS);
                emitOneOperandMem(op, address);
                overwriteFlagsIfNeeded(flagMode);
                writeRegs(cpu, regs);
                runOneOperandPreparedMemCase(op, width, data, address, flagMode, name);
            }
        }
    }
}

void runImulBaseMemoryCases(ImulForm form, int width, const ImulCase& data, FlagMode flagMode, const char* name) {
    for (int base = 0; base < 8; ++base) {
        for (int dst = 0; dst < 8; ++dst) {
            if (dst == base) {
                continue;
            }
            U32 regs[8];
            initAddressRegisters(regs);
            regs[base] = MEM_BASE + 0x1000 + base * 0x80;

            AddressCase address;
            makeBaseCase(address, base, regs[base], base == R_BP ? 0x11 : 0, width);
            beginInstruction(INITIAL_FLAGS);
            emitImulMem(form, width, dst, address, data.imm);
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runImulPreparedMemCase(form, width, data, dst, address, flagMode, name);
        }
    }
}

void runImulAbsoluteMemoryCases(ImulForm form, int width, const ImulCase& data, FlagMode flagMode, const char* name) {
    for (int dst = 0; dst < 8; ++dst) {
        U32 regs[8];
        AddressCase address;
        initAddressRegisters(regs);
        makeAbsoluteCase(address, MEM_BASE + 0x3000 + dst * 0x10, width);
        beginInstruction(INITIAL_FLAGS);
        emitImulMem(form, width, dst, address, data.imm);
        overwriteFlagsIfNeeded(flagMode);
        writeRegs(cpu, regs);
        runImulPreparedMemCase(form, width, data, dst, address, flagMode, name);
    }
}

void runOneOperandCases(MulDivOp op, int width, const OneOpCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int flagMode = FLAGS_CHECKED; flagMode <= FLAGS_OVERWRITTEN; ++flagMode) {
            for (int reg = 0; reg < 8; ++reg) {
                if (width == 8 && (reg == 0 || reg == 4)) {
                    continue;
                }
                if (width != 8 && reg == R_AX) {
                    continue;
                }
                if ((op == OP_DIV || op == OP_IDIV) && width != 8 && reg == R_DX) {
                    continue;
                }
                runOneOperandRegCase(op, width, cases[i], reg, (FlagMode)flagMode, name);
            }
            runBaseMemoryCases(op, width, cases[i], (FlagMode)flagMode, name);
            runAbsoluteMemoryCase(op, width, cases[i], (FlagMode)flagMode, name);
            runSibMemoryCases(op, width, cases[i], (FlagMode)flagMode, name);
        }
    }
}

void runImulCases(ImulForm form, int width, const ImulCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int flagMode = FLAGS_CHECKED; flagMode <= FLAGS_OVERWRITTEN; ++flagMode) {
            for (int dst = 0; dst < 8; ++dst) {
                for (int src = 0; src < 8; ++src) {
                    runImulRegCase(form, width, cases[i], dst, src, (FlagMode)flagMode, name);
                }
            }
            runImulBaseMemoryCases(form, width, cases[i], (FlagMode)flagMode, name);
            runImulAbsoluteMemoryCases(form, width, cases[i], (FlagMode)flagMode, name);
        }
    }
}

} // namespace

void testIMulR16E16Iw_0x069() {
    runImulCases(IMUL_IMM, 16, TWO_IMUL16_CASES, caseCount(TWO_IMUL16_CASES), "imul r16,e16,iw");
}

void testIMulR32E32Id_0x269() {
    runImulCases(IMUL_IMM, 32, TWO_IMUL32_CASES, caseCount(TWO_IMUL32_CASES), "imul r32,e32,id");
}

void testIMulR16E16Ib_0x06b() {
    runImulCases(IMUL_IMM8, 16, IMUL16_IMM8_CASES, caseCount(IMUL16_IMM8_CASES), "imul r16,e16,ib");
}

void testIMulR32E32Ib_0x26b() {
    runImulCases(IMUL_IMM8, 32, IMUL32_IMM8_CASES, caseCount(IMUL32_IMM8_CASES), "imul r32,e32,ib");
}

void testMulR8E8_0x0f6() {
    runOneOperandCases(OP_MUL, 8, MUL8_CASES, caseCount(MUL8_CASES), "mul r/m8");
}

void testIMulR8E8_0x0f6() {
    runOneOperandCases(OP_IMUL, 8, IMUL8_CASES, caseCount(IMUL8_CASES), "imul r/m8");
}

void testDivR8E8_0x0f6() {
    runOneOperandCases(OP_DIV, 8, DIV8_CASES, caseCount(DIV8_CASES), "div r/m8");
}

void testIDivR8E8_0x0f6() {
    runOneOperandCases(OP_IDIV, 8, IDIV8_CASES, caseCount(IDIV8_CASES), "idiv r/m8");
}

void testMulR8E8_0x2f6() {
    runOneOperandCases(OP_MUL, 8, MUL8_CASES, caseCount(MUL8_CASES), "mul r/m8 2f6");
}

void testIMulR8E8_0x2f6() {
    runOneOperandCases(OP_IMUL, 8, IMUL8_CASES, caseCount(IMUL8_CASES), "imul r/m8 2f6");
}

void testDivR8E8_0x2f6() {
    runOneOperandCases(OP_DIV, 8, DIV8_CASES, caseCount(DIV8_CASES), "div r/m8 2f6");
}

void testIDivR8E8_0x2f6() {
    runOneOperandCases(OP_IDIV, 8, IDIV8_CASES, caseCount(IDIV8_CASES), "idiv r/m8 2f6");
}

void testMulR16E16_0x0f7() {
    runOneOperandCases(OP_MUL, 16, MUL16_CASES, caseCount(MUL16_CASES), "mul r/m16");
}

void testIMulR16E16_0x0f7() {
    runOneOperandCases(OP_IMUL, 16, ONE_IMUL16_CASES, caseCount(ONE_IMUL16_CASES), "imul r/m16");
}

void testDivR16E16_0x0f7() {
    runOneOperandCases(OP_DIV, 16, DIV16_CASES, caseCount(DIV16_CASES), "div r/m16");
}

void testIDivR16E16_0x0f7() {
    runOneOperandCases(OP_IDIV, 16, IDIV16_CASES, caseCount(IDIV16_CASES), "idiv r/m16");
}

void testMulR32E32_0x2f7() {
    runOneOperandCases(OP_MUL, 32, MUL32_CASES, caseCount(MUL32_CASES), "mul r/m32");
}

void testIMulR32E32_0x2f7() {
    runOneOperandCases(OP_IMUL, 32, ONE_IMUL32_CASES, caseCount(ONE_IMUL32_CASES), "imul r/m32");
}

void testDivR32E32_0x2f7() {
    runOneOperandCases(OP_DIV, 32, DIV32_CASES, caseCount(DIV32_CASES), "div r/m32");
}

void testIDivR32E32_0x2f7() {
    runOneOperandCases(OP_IDIV, 32, IDIV32_CASES, caseCount(IDIV32_CASES), "idiv r/m32");
}

void testIMulR16E16_0x1af() {
    runImulCases(IMUL_RM, 16, TWO_IMUL16_CASES, caseCount(TWO_IMUL16_CASES), "imul r16,e16");
}

void testIMulR32E32_0x3af() {
    runImulCases(IMUL_RM, 32, TWO_IMUL32_CASES, caseCount(TWO_IMUL32_CASES), "imul r32,e32");
}

#endif
