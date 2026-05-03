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

#include "testShift.h"
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

constexpr U32 MEM_BASE = 0x10000;
constexpr U32 MEM_GUARD = 0xcdcdcdcd;
constexpr U32 REG_GUARD = 0x71000000;
constexpr U32 SHIFT_FLAG_MASK = CF | PF | ZF | SF | OF;

enum ShiftOp {
    SHIFT_ROL,
    SHIFT_ROR,
    SHIFT_RCL,
    SHIFT_RCR,
    SHIFT_SHL,
    SHIFT_SHR,
    SHIFT_SHL6,
    SHIFT_SAR
};

enum CountMode {
    COUNT_IMM,
    COUNT_ONE,
    COUNT_CL
};

enum DoubleShiftOp {
    DOUBLE_SHLD,
    DOUBLE_SHRD
};

enum RegIndex {
    R_AX = 0,
    R_CX = 1,
    R_DX = 2,
    R_BX = 3,
    R_SP = 4,
    R_BP = 5,
    R_SI = 6,
    R_DI = 7
};

struct ShiftCase {
    U32 value;
    U8 count;
    U32 initialFlags;
};

struct ShiftExpected {
    U32 result;
    U32 flags;
    U32 flagMask;
};

struct DoubleShiftCase {
    U32 dst;
    U32 src;
    U8 count;
    U32 initialFlags;
};

const ShiftOp SHIFT_OPS[] = {
    SHIFT_ROL, SHIFT_ROR, SHIFT_RCL, SHIFT_RCR, SHIFT_SHL, SHIFT_SHR, SHIFT_SHL6, SHIFT_SAR
};

const ShiftCase SHIFT_CASES[] = {
    {0x00000000, 0, CF | PF | ZF | SF | OF},
    {0x00000001, 1, 0},
    {0x00000080, 1, CF},
    {0x00000081, 1, CF},
    {0x0000007f, 4, 0},
    {0x00000080, 7, 0},
    {0x000000ff, 8, 0},
    {0x00000001, 9, 0},
    {0x00008000, 15, 0},
    {0x00008001, 16, 0},
    {0x80000000, 31, 0},
    {0xffffffff, 32, CF | PF | ZF | SF | OF},
    {0x12345678, 33, 0},
    {0x87654321, 12, 0}
};

const DoubleShiftCase DOUBLE_SHIFT_CASES[] = {
    {0x00000000, 0xffffffff, 0, CF | PF | ZF | SF | OF},
    {0x12345678, 0x90abcdef, 4, 0},
    {0x12345678, 0x90abcdef, 12, 0},
    {0x80808080, 0x80000000, 1, 0},
    {0x40808080, 0x80000000, 1, 0},
    {0x20808080, 0x80000000, 1, 0},
    {0x40808080, 0x80000000, 2, 0},
    {0x12345678, 0x90abcdef, 40, 0},
    {0xffffffff, 0x00000000, 15, CF | OF},
    {0x80000001, 0x7ffffffe, 31, 0}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

const char* opName(ShiftOp op) {
    switch (op) {
    case SHIFT_ROL: return "rol";
    case SHIFT_ROR: return "ror";
    case SHIFT_RCL: return "rcl";
    case SHIFT_RCR: return "rcr";
    case SHIFT_SHL: return "shl";
    case SHIFT_SHR: return "shr";
    case SHIFT_SHL6: return "shl6";
    default: return "sar";
    }
}

const char* doubleShiftName(DoubleShiftOp op) {
    return op == DOUBLE_SHLD ? "shld" : "shrd";
}

U32 parityFlag(U32 value) {
    U8 low = (U8)value;
    low ^= low >> 4;
    low &= 0x0f;
    return ((0x6996 >> low) & 1) ? 0 : PF;
}

U32 resultFlags(U32 result, int width) {
    U32 flags = parityFlag(result);
    if ((result & widthMask(width)) == 0) {
        flags |= ZF;
    }
    if (result & signBit(width)) {
        flags |= SF;
    }
    return flags;
}

U32 lowMask(int width) {
    return width == 32 ? 0xffffffffu : ((1u << width) - 1);
}

U32 countMask(U8 count) {
    return count & 0x1f;
}

U32 rotateThroughCarryCount(ShiftOp op, int width, U8 count) {
    U32 masked = countMask(count);
    if (op != SHIFT_RCL && op != SHIFT_RCR) {
        return masked % width;
    }
    if (width == 32) {
        return masked;
    }
    return masked % (width + 1);
}

ShiftExpected calculateShift(ShiftOp op, int width, U32 value, U8 count, U32 initialFlags) {
    U32 mask = lowMask(width);
    U32 result = value & mask;
    U32 flags = initialFlags & SHIFT_FLAG_MASK;
    U32 maskedCount = countMask(count);
    U32 effectiveCount = rotateThroughCarryCount(op, width, count);
    U32 cf = initialFlags & CF ? 1 : 0;
    U32 flagMask = 0;

    if (maskedCount == 0) {
        return {result, flags, SHIFT_FLAG_MASK};
    }
    if (effectiveCount == 0) {
        if (op == SHIFT_ROL || op == SHIFT_ROR) {
            effectiveCount = width;
        } else if (op == SHIFT_RCL || op == SHIFT_RCR) {
            return {result, flags, SHIFT_FLAG_MASK};
        }
    }

    if (op == SHIFT_ROL) {
        for (U32 i = 0; i < effectiveCount; ++i) {
            U32 msb = (result >> (width - 1)) & 1;
            result = ((result << 1) | msb) & mask;
            cf = msb;
        }
        flags = (initialFlags & ~(CF | OF)) | (cf ? CF : 0);
        if (effectiveCount == 1 && (((result >> (width - 1)) & 1) ^ cf)) {
            flags |= OF;
        }
        flagMask = CF | (effectiveCount == 1 ? OF : 0);
    } else if (op == SHIFT_ROR) {
        for (U32 i = 0; i < effectiveCount; ++i) {
            U32 lsb = result & 1;
            result = (result >> 1) | (lsb << (width - 1));
            cf = lsb;
        }
        flags = (initialFlags & ~(CF | OF)) | (cf ? CF : 0);
        if (effectiveCount == 1 && (((result >> (width - 1)) ^ (result >> (width - 2))) & 1)) {
            flags |= OF;
        }
        flagMask = CF | (effectiveCount == 1 ? OF : 0);
    } else if (op == SHIFT_RCL) {
        for (U32 i = 0; i < effectiveCount; ++i) {
            U32 msb = (result >> (width - 1)) & 1;
            result = ((result << 1) | cf) & mask;
            cf = msb;
        }
        flags = (initialFlags & ~(CF | OF)) | (cf ? CF : 0);
        if (effectiveCount == 1 && (((result >> (width - 1)) & 1) ^ cf)) {
            flags |= OF;
        }
        flagMask = CF | (effectiveCount == 1 ? OF : 0);
    } else if (op == SHIFT_RCR) {
        for (U32 i = 0; i < effectiveCount; ++i) {
            U32 lsb = result & 1;
            result = (result >> 1) | (cf << (width - 1));
            cf = lsb;
        }
        flags = (initialFlags & ~(CF | OF)) | (cf ? CF : 0);
        if (effectiveCount == 1 && (((result >> (width - 1)) ^ (result >> (width - 2))) & 1)) {
            flags |= OF;
        }
        flagMask = CF | (effectiveCount == 1 ? OF : 0);
    } else {
        for (U32 i = 0; i < maskedCount; ++i) {
            if (op == SHIFT_SHR) {
                cf = result & 1;
                result >>= 1;
            } else if (op == SHIFT_SAR) {
                cf = result & 1;
                result = (result >> 1) | (result & signBit(width));
            } else {
                cf = (result >> (width - 1)) & 1;
                result = (result << 1) & mask;
            }
        }
        flags = resultFlags(result, width) | (cf ? CF : 0);
        if (maskedCount == 1) {
            if (op == SHIFT_SHR && (value & signBit(width))) {
                flags |= OF;
            } else if ((op == SHIFT_SHL || op == SHIFT_SHL6) && ((((result >> (width - 1)) & 1) ^ cf) != 0)) {
                flags |= OF;
            }
        }
        flagMask = maskedCount > (U32)width ? 0 : (CF | PF | ZF | SF | (maskedCount == 1 ? OF : 0));
    }

    return {result & mask, flags & SHIFT_FLAG_MASK, flagMask};
}

ShiftExpected calculateDoubleShift(DoubleShiftOp op, int width, U32 dst, U32 src, U8 count, U32 initialFlags) {
    U32 mask = lowMask(width);
    U32 result = dst & mask;
    U32 flags = initialFlags & SHIFT_FLAG_MASK;
    U32 maskedCount = countMask(count);
    U32 flagMask = 0;

    if (maskedCount == 0) {
        return {result, flags, SHIFT_FLAG_MASK};
    }
    if (maskedCount > (U32)width) {
        return {result, flags, 0};
    }

    dst &= mask;
    src &= mask;
    if (op == DOUBLE_SHLD) {
        U32 fromDst = (dst << maskedCount) & mask;
        U32 fromSrc = maskedCount == (U32)width ? src : (src >> (width - maskedCount));
        result = (fromDst | fromSrc) & mask;
        flags = resultFlags(result, width) | (((dst >> (width - maskedCount)) & 1) ? CF : 0);
        if (maskedCount == 1 && ((((result >> (width - 1)) & 1) ^ ((flags & CF) ? 1 : 0)) != 0)) {
            flags |= OF;
        }
    } else {
        U32 fromDst = dst >> maskedCount;
        U32 fromSrc = maskedCount == (U32)width ? src : ((src << (width - maskedCount)) & mask);
        result = (fromDst | fromSrc) & mask;
        flags = resultFlags(result, width) | (((dst >> (maskedCount - 1)) & 1) ? CF : 0);
        if (maskedCount == 1 && ((((dst >> (width - 1)) ^ (result >> (width - 1))) & 1) != 0)) {
            flags |= OF;
        }
    }

    flagMask = CF | PF | ZF | SF | (maskedCount == 1 ? OF : 0);
    return {result, flags & SHIFT_FLAG_MASK, flagMask};
}

#if defined(_MSC_VER) && defined(_M_IX86)
#define TEST_SHIFT_HAS_HARDWARE_ORACLE 1

ShiftExpected hardwareShift(ShiftOp op, int width, U32 value, U8 count, U32 initialFlags) {
    U32 result = value;
    U32 flags = 0;
    U32 flagIn = 0x202 | (initialFlags & SHIFT_FLAG_MASK);

#define HW_SHIFT8(inst) __asm { push flagIn __asm popfd __asm mov eax, value __asm mov cl, count __asm inst al, cl __asm pushfd __asm pop edx __asm mov result, eax __asm mov flags, edx }
#define HW_SHIFT16(inst) __asm { push flagIn __asm popfd __asm mov eax, value __asm mov cl, count __asm inst ax, cl __asm pushfd __asm pop edx __asm mov result, eax __asm mov flags, edx }
#define HW_SHIFT32(inst) __asm { push flagIn __asm popfd __asm mov eax, value __asm mov cl, count __asm inst eax, cl __asm pushfd __asm pop edx __asm mov result, eax __asm mov flags, edx }

    if (width == 8) {
        if (op == SHIFT_ROL) { HW_SHIFT8(rol); }
        else if (op == SHIFT_ROR) { HW_SHIFT8(ror); }
        else if (op == SHIFT_RCL) { HW_SHIFT8(rcl); }
        else if (op == SHIFT_RCR) { HW_SHIFT8(rcr); }
        else if (op == SHIFT_SHR) { HW_SHIFT8(shr); }
        else if (op == SHIFT_SAR) { HW_SHIFT8(sar); }
        else { HW_SHIFT8(shl); }
    } else if (width == 16) {
        if (op == SHIFT_ROL) { HW_SHIFT16(rol); }
        else if (op == SHIFT_ROR) { HW_SHIFT16(ror); }
        else if (op == SHIFT_RCL) { HW_SHIFT16(rcl); }
        else if (op == SHIFT_RCR) { HW_SHIFT16(rcr); }
        else if (op == SHIFT_SHR) { HW_SHIFT16(shr); }
        else if (op == SHIFT_SAR) { HW_SHIFT16(sar); }
        else { HW_SHIFT16(shl); }
    } else {
        if (op == SHIFT_ROL) { HW_SHIFT32(rol); }
        else if (op == SHIFT_ROR) { HW_SHIFT32(ror); }
        else if (op == SHIFT_RCL) { HW_SHIFT32(rcl); }
        else if (op == SHIFT_RCR) { HW_SHIFT32(rcr); }
        else if (op == SHIFT_SHR) { HW_SHIFT32(shr); }
        else if (op == SHIFT_SAR) { HW_SHIFT32(sar); }
        else { HW_SHIFT32(shl); }
    }

#undef HW_SHIFT8
#undef HW_SHIFT16
#undef HW_SHIFT32

    ShiftExpected expected = calculateShift(op, width, value, count, initialFlags);
    expected.result = result & widthMask(width);
    expected.flags = flags & SHIFT_FLAG_MASK;
    return expected;
}

ShiftExpected hardwareDoubleShift(DoubleShiftOp op, int width, U32 dst, U32 src, U8 count, U32 initialFlags) {
    U32 result = dst;
    U32 flags = 0;
    U32 flagIn = 0x202 | (initialFlags & SHIFT_FLAG_MASK);

#define HW_DOUBLE_SHIFT16(inst) __asm { push flagIn __asm popfd __asm mov eax, dst __asm mov edx, src __asm mov cl, count __asm inst ax, dx, cl __asm pushfd __asm pop ebx __asm mov result, eax __asm mov flags, ebx }
#define HW_DOUBLE_SHIFT32(inst) __asm { push flagIn __asm popfd __asm mov eax, dst __asm mov edx, src __asm mov cl, count __asm inst eax, edx, cl __asm pushfd __asm pop ebx __asm mov result, eax __asm mov flags, ebx }

    if (width == 16) {
        if (op == DOUBLE_SHLD) { HW_DOUBLE_SHIFT16(shld); }
        else { HW_DOUBLE_SHIFT16(shrd); }
    } else {
        if (op == DOUBLE_SHLD) { HW_DOUBLE_SHIFT32(shld); }
        else { HW_DOUBLE_SHIFT32(shrd); }
    }

#undef HW_DOUBLE_SHIFT16
#undef HW_DOUBLE_SHIFT32

    ShiftExpected expected = calculateDoubleShift(op, width, dst, src, count, initialFlags);
    expected.result = result & widthMask(width);
    expected.flags = flags & SHIFT_FLAG_MASK;
    return expected;
}

#else
#define TEST_SHIFT_HAS_HARDWARE_ORACLE 0
#endif

ShiftExpected expectedShift(ShiftOp op, int width, U32 value, U8 count, U32 initialFlags) {
    ShiftExpected calculated = calculateShift(op, width, value, count, initialFlags);
#if TEST_SHIFT_HAS_HARDWARE_ORACLE
    ShiftExpected hardware = hardwareShift(op, width, value, count, initialFlags);
    if (hardware.result != calculated.result || (hardware.flags & calculated.flagMask) != (calculated.flags & calculated.flagMask)) {
        failed("hardware %s oracle mismatch", opName(op));
    }
    return hardware;
#else
    return calculated;
#endif
}

ShiftExpected expectedDoubleShift(DoubleShiftOp op, int width, U32 dst, U32 src, U8 count, U32 initialFlags) {
    ShiftExpected calculated = calculateDoubleShift(op, width, dst, src, count, initialFlags);
#if TEST_SHIFT_HAS_HARDWARE_ORACLE
    ShiftExpected hardware = hardwareDoubleShift(op, width, dst, src, count, initialFlags);
    if (hardware.result != calculated.result || (hardware.flags & calculated.flagMask) != (calculated.flags & calculated.flagMask)) {
        failed("hardware %s oracle mismatch", doubleShiftName(op));
    }
    hardware.flagMask = calculated.flagMask;
    return hardware;
#else
    return calculated;
#endif
}

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit code init failed");
    }
}

void pushGeneratedCode(const asmjit::CodeHolder& code, ShiftOp op) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    bool patchedShl6 = false;
    for (size_t i = 0; i < buffer.size(); ++i) {
        U8 value = buffer.data()[i];
        if (op == SHIFT_SHL6 && !patchedShl6 && i > 0) {
            U8 opcode = buffer.data()[i - 1];
            if (opcode == 0xc0 || opcode == 0xc1 || opcode == 0xd0 || opcode == 0xd1 || opcode == 0xd2 || opcode == 0xd3) {
                value = (value & 0xc7) | (6 << 3);
                patchedShl6 = true;
            }
        }
        pushCode8(value);
    }
}

template <typename Dst>
void emitShift(ShiftOp op, const Dst& dst, CountMode mode, U8 count) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Error err = asmjit::Error::kOk;
    if (mode == COUNT_CL) {
        if (op == SHIFT_ROL) err = a.rol(dst, asmjit::x86::cl);
        else if (op == SHIFT_ROR) err = a.ror(dst, asmjit::x86::cl);
        else if (op == SHIFT_RCL) err = a.rcl(dst, asmjit::x86::cl);
        else if (op == SHIFT_RCR) err = a.rcr(dst, asmjit::x86::cl);
        else if (op == SHIFT_SHR) err = a.shr(dst, asmjit::x86::cl);
        else if (op == SHIFT_SAR) err = a.sar(dst, asmjit::x86::cl);
        else err = a.shl(dst, asmjit::x86::cl);
    } else {
        asmjit::Imm imm(mode == COUNT_ONE ? 1 : count);
        if (op == SHIFT_ROL) err = a.rol(dst, imm);
        else if (op == SHIFT_ROR) err = a.ror(dst, imm);
        else if (op == SHIFT_RCL) err = a.rcl(dst, imm);
        else if (op == SHIFT_RCR) err = a.rcr(dst, imm);
        else if (op == SHIFT_SHR) err = a.shr(dst, imm);
        else if (op == SHIFT_SAR) err = a.sar(dst, imm);
        else err = a.shl(dst, imm);
    }

    if (err != asmjit::Error::kOk) {
        failed("asmjit %s failed", opName(op));
    }
    pushGeneratedCode(code, op);
}

template <typename Dst>
void emitDoubleShift(DoubleShiftOp op, const Dst& dst, const asmjit::x86::Gp& src, CountMode mode, U8 count) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Error err;
    if (mode == COUNT_CL) {
        err = op == DOUBLE_SHLD ? a.shld(dst, src, asmjit::x86::cl) : a.shrd(dst, src, asmjit::x86::cl);
    } else {
        err = op == DOUBLE_SHLD ? a.shld(dst, src, asmjit::Imm(count)) : a.shrd(dst, src, asmjit::Imm(count));
    }
    if (err != asmjit::Error::kOk) {
        failed("asmjit %s failed", doubleShiftName(op));
    }
    pushGeneratedCode(code, SHIFT_SHL);
}

void beginShift(U32 flags) {
    newInstruction(flags);
    cpu->big = true;
}

void initRegs(U32* regs) {
    for (int i = 0; i < 8; ++i) {
        regs[i] = REG_GUARD | (0x100 + i);
    }
    regs[R_AX] = MEM_BASE + 0x100;
    regs[R_CX] = MEM_BASE + 0x200;
    regs[R_DX] = MEM_BASE + 0x300;
    regs[R_BX] = MEM_BASE + 0x400;
    regs[R_SP] = MEM_BASE + 0x500;
    regs[R_BP] = MEM_BASE + 0x600;
    regs[R_SI] = MEM_BASE + 0x700;
    regs[R_DI] = MEM_BASE + 0x800;
}

void writeRegsLocal(const U32* regs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = regs[i];
    }
}

void verifyFlags(const ShiftExpected& expected, const char* name) {
    U32 actual = actualFlags(cpu);
    if ((actual & expected.flagMask) != (expected.flags & expected.flagMask)) {
        failed("%s flags", name);
    }
}

void runRegisterCase(ShiftOp op, int width, CountMode mode, int dstReg, const ShiftCase& data, const char* name) {
    char caseName[160];
    snprintf(caseName, sizeof(caseName), "%s %s reg=%d value=%x count=%u flags=%x", name, opName(op), dstReg, data.value, data.count, data.initialFlags);
    U32 regs[8];
    initRegs(regs);
    U8 count = mode == COUNT_ONE ? 1 : data.count;
    if (mode == COUNT_CL) {
        regs[R_CX] = count;
    }
    applyRegValue(regs, dstReg, width, data.value);
    if (mode == COUNT_CL) {
        count = (U8)(regs[R_CX] & 0xff);
    }

    U32 actualInput = width == 8
        ? ((dstReg >= 4 ? (regs[physicalReg8(dstReg)] >> 8) : regs[physicalReg8(dstReg)]) & 0xff)
        : (regs[dstReg] & widthMask(width));
    ShiftExpected expected = expectedShift(op, width, actualInput, count, data.initialFlags);
    if (mode == COUNT_CL && width == 32 && dstReg == R_CX) {
        expected.flagMask = 0;
    }
    U32 expectedRegs[8];
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }
    applyRegValue(expectedRegs, dstReg, width, expected.result);

    beginShift(data.initialFlags);
    emitShift(op, regForWidth(dstReg, width), mode, data.count);
    writeRegsLocal(regs);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    verifyFlags(expected, caseName);
}

U32 linearForBase(int base, U32 offset) {
    return ((base == R_SP || base == R_BP) ? cpu->seg[SS].address : cpu->seg[DS].address) + offset;
}

void prepareMem(U32 address, U32 value, int width) {
    memory->writed(address - 4, MEM_GUARD);
    memory->writed(address + width / 8, MEM_GUARD);
    if (width == 8) memory->writeb(address, value);
    else if (width == 16) memory->writew(address, value);
    else memory->writed(address, value);
}

U32 readMem(U32 address, int width) {
    if (width == 8) return memory->readb(address);
    if (width == 16) return memory->readw(address);
    return memory->readd(address);
}

void verifyMem(U32 address, U32 expected, int width, const char* name) {
    if (readMem(address, width) != (expected & widthMask(width))) {
        failed("%s memory result", name);
    }
    if (memory->readd(address - 4) != MEM_GUARD || memory->readd(address + width / 8) != MEM_GUARD) {
        failed("%s memory guard", name);
    }
}

void runMemoryCase(ShiftOp op, int width, CountMode mode, const ShiftCase& data, const char* name) {
    for (int form = 0; form < 3; ++form) {
        char caseName[160];
        snprintf(caseName, sizeof(caseName), "%s %s mem=%d value=%x count=%u flags=%x", name, opName(op), form, data.value, data.count, data.initialFlags);
        U32 regs[8];
        initRegs(regs);
        U8 count = mode == COUNT_ONE ? 1 : data.count;
        if (mode == COUNT_CL) {
            regs[R_CX] = count;
        }

        U32 offset = MEM_BASE + 0x2000 + form * 0x100;
        asmjit::x86::Mem mem;
        int base = R_BX;
        int index = R_SI;
        if (form == 0) {
            mem = memPtr(offset, width);
        } else if (form == 1) {
            regs[base] = offset - 0x20;
            mem = memPtr(reg32(base), 0x20, width);
        } else {
            regs[base] = offset - 0x40;
            regs[index] = 0x10;
            mem = memPtr(reg32(base), reg32(index), 1, 0x20, width);
        }

        U32 linear = form == 0 ? cpu->seg[DS].address + offset : linearForBase(base, offset);
        ShiftExpected expected = expectedShift(op, width, data.value, count, data.initialFlags);

        beginShift(data.initialFlags);
        emitShift(op, mem, mode, data.count);
        writeRegsLocal(regs);
        prepareMem(linear, data.value, width);
        runTestCPU();
        verifyMem(linear, expected.result, width, caseName);
        verifyRegisters(cpu, regs, caseName);
        verifyFlags(expected, caseName);
    }
}

void runShiftWidth(CountMode mode, int width, const char* name) {
    for (size_t opIndex = 0; opIndex < caseCount(SHIFT_OPS); ++opIndex) {
        ShiftOp op = SHIFT_OPS[opIndex];
        for (size_t caseIndex = 0; caseIndex < caseCount(SHIFT_CASES); ++caseIndex) {
            const ShiftCase& data = SHIFT_CASES[caseIndex];
            if (mode == COUNT_ONE && data.count != 1) {
                continue;
            }
            for (int reg = 0; reg < 8; ++reg) {
                runRegisterCase(op, width, mode, reg, data, name);
            }
            runMemoryCase(op, width, mode, data, name);
        }
    }
}

void runDoubleShiftRegisterCase(DoubleShiftOp op, int width, CountMode mode, int dstReg, int srcReg, const DoubleShiftCase& data, const char* name) {
    char caseName[180];
    snprintf(caseName, sizeof(caseName), "%s %s dst=%d src=%d dstValue=%x srcValue=%x count=%u flags=%x",
        name, doubleShiftName(op), dstReg, srcReg, data.dst, data.src, data.count, data.initialFlags);
    U32 regs[8];
    initRegs(regs);
    applyRegValue(regs, dstReg, width, data.dst);
    applyRegValue(regs, srcReg, width, data.src);
    if (mode == COUNT_CL) {
        applyRegValue(regs, R_CX, 8, data.count);
    }

    U8 count = mode == COUNT_CL ? (U8)(regs[R_CX] & 0xff) : data.count;
    U32 actualDst = getRegValue(cpu, dstReg, width);
    U32 actualSrc = getRegValue(cpu, srcReg, width);
    U32 savedRegs[8];
    for (int i = 0; i < 8; ++i) {
        savedRegs[i] = cpu->reg[i].u32;
        cpu->reg[i].u32 = regs[i];
    }
    actualDst = getRegValue(cpu, dstReg, width);
    actualSrc = getRegValue(cpu, srcReg, width);
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = savedRegs[i];
    }

    ShiftExpected expected = expectedDoubleShift(op, width, actualDst, actualSrc, count, data.initialFlags);
    U32 expectedRegs[8];
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }
    applyRegValue(expectedRegs, dstReg, width, expected.result);

    beginShift(data.initialFlags);
    emitDoubleShift(op, regForWidth(dstReg, width), regForWidth(srcReg, width), mode, data.count);
    writeRegsLocal(regs);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    verifyFlags(expected, caseName);
}

void runDoubleShiftMemoryCase(DoubleShiftOp op, int width, CountMode mode, int srcReg, const DoubleShiftCase& data, const char* name) {
    for (int form = 0; form < 3; ++form) {
        char caseName[180];
        snprintf(caseName, sizeof(caseName), "%s %s mem=%d src=%d dstValue=%x srcValue=%x count=%u flags=%x",
            name, doubleShiftName(op), form, srcReg, data.dst, data.src, data.count, data.initialFlags);
        U32 regs[8];
        initRegs(regs);
        applyRegValue(regs, srcReg, width, data.src);
        if (mode == COUNT_CL) {
            applyRegValue(regs, R_CX, 8, data.count);
        }

        U32 offset = MEM_BASE + 0x3000 + form * 0x100 + srcReg * 0x10;
        asmjit::x86::Mem mem;
        int base = R_BX;
        int index = R_SI;
        if (form == 0) {
            mem = memPtr(offset, width);
        } else if (form == 1) {
            regs[base] = offset - 0x20;
            mem = memPtr(reg32(base), 0x20, width);
        } else {
            regs[base] = offset - 0x40;
            regs[index] = 0x10;
            mem = memPtr(reg32(base), reg32(index), 1, 0x20, width);
        }

        U32 savedRegs[8];
        for (int i = 0; i < 8; ++i) {
            savedRegs[i] = cpu->reg[i].u32;
            cpu->reg[i].u32 = regs[i];
        }
        U8 count = mode == COUNT_CL ? (U8)(cpu->reg[R_CX].u32 & 0xff) : data.count;
        U32 actualSrc = getRegValue(cpu, srcReg, width);
        for (int i = 0; i < 8; ++i) {
            cpu->reg[i].u32 = savedRegs[i];
        }

        U32 linear = form == 0 ? cpu->seg[DS].address + offset : linearForBase(base, offset);
        ShiftExpected expected = expectedDoubleShift(op, width, data.dst, actualSrc, count, data.initialFlags);

        beginShift(data.initialFlags);
        emitDoubleShift(op, mem, regForWidth(srcReg, width), mode, data.count);
        writeRegsLocal(regs);
        prepareMem(linear, data.dst, width);
        runTestCPU();
        verifyMem(linear, expected.result, width, caseName);
        verifyRegisters(cpu, regs, caseName);
        verifyFlags(expected, caseName);
    }
}

void runDoubleShiftWidth(DoubleShiftOp op, CountMode mode, int width, const char* name) {
    for (size_t caseIndex = 0; caseIndex < caseCount(DOUBLE_SHIFT_CASES); ++caseIndex) {
        const DoubleShiftCase& data = DOUBLE_SHIFT_CASES[caseIndex];
        U32 maskedCount = countMask(data.count);
        if (maskedCount > (U32)width) {
            continue;
        }
        for (int dst = 0; dst < 8; ++dst) {
            for (int src = 0; src < 8; ++src) {
                runDoubleShiftRegisterCase(op, width, mode, dst, src, data, name);
            }
        }
        for (int src = 0; src < 8; ++src) {
            runDoubleShiftMemoryCase(op, width, mode, src, data, name);
        }
    }
}

} // namespace

void testShiftE8Ib_0x0c0() { runShiftWidth(COUNT_IMM, 8, "shift e8,ib c0"); }
void testShiftE8Ib_0x2c0() { runShiftWidth(COUNT_IMM, 8, "shift e8,ib 2c0"); }
void testShiftE16Ib_0x0c1() { runShiftWidth(COUNT_IMM, 16, "shift e16,ib c1"); }
void testShiftE32Ib_0x2c1() { runShiftWidth(COUNT_IMM, 32, "shift e32,ib 2c1"); }
void testShiftE8_0x0d0() { runShiftWidth(COUNT_ONE, 8, "shift e8,1 d0"); }
void testShiftE8_0x2d0() { runShiftWidth(COUNT_ONE, 8, "shift e8,1 2d0"); }
void testShiftE16_0x0d1() { runShiftWidth(COUNT_ONE, 16, "shift e16,1 d1"); }
void testShiftE32_0x2d1() { runShiftWidth(COUNT_ONE, 32, "shift e32,1 2d1"); }
void testShiftE8Cl_0x0d2() { runShiftWidth(COUNT_CL, 8, "shift e8,cl d2"); }
void testShiftE8Cl_0x2d2() { runShiftWidth(COUNT_CL, 8, "shift e8,cl 2d2"); }
void testShiftE16Cl_0x0d3() { runShiftWidth(COUNT_CL, 16, "shift e16,cl d3"); }
void testShiftE32Cl_0x2d3() { runShiftWidth(COUNT_CL, 32, "shift e32,cl 2d3"); }
void testShldE16R16Ib_0x1a4() { runDoubleShiftWidth(DOUBLE_SHLD, COUNT_IMM, 16, "shld e16,r16,ib 1a4"); }
void testShldE32R32Ib_0x3a4() { runDoubleShiftWidth(DOUBLE_SHLD, COUNT_IMM, 32, "shld e32,r32,ib 3a4"); }
void testShldE16R16Cl_0x1a5() { runDoubleShiftWidth(DOUBLE_SHLD, COUNT_CL, 16, "shld e16,r16,cl 1a5"); }
void testShldE32R32Cl_0x3a5() { runDoubleShiftWidth(DOUBLE_SHLD, COUNT_CL, 32, "shld e32,r32,cl 3a5"); }
void testShrdE16R16Ib_0x1ac() { runDoubleShiftWidth(DOUBLE_SHRD, COUNT_IMM, 16, "shrd e16,r16,ib 1ac"); }
void testShrdE32R32Ib_0x3ac() { runDoubleShiftWidth(DOUBLE_SHRD, COUNT_IMM, 32, "shrd e32,r32,ib 3ac"); }
void testShrdE16R16Cl_0x1ad() { runDoubleShiftWidth(DOUBLE_SHRD, COUNT_CL, 16, "shrd e16,r16,cl 1ad"); }
void testShrdE32R32Cl_0x3ad() { runDoubleShiftWidth(DOUBLE_SHRD, COUNT_CL, 32, "shrd e32,r32,cl 3ad"); }

#endif
