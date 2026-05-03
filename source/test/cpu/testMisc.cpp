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

#include "testMisc.h"
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
constexpr U32 ADD_FLAG_MASK = CF | PF | AF | ZF | SF | OF;
constexpr U32 SAHF_FLAG_MASK = CF | PF | AF | ZF | SF;
constexpr U32 GUARD_BEFORE = 0x11223344;
constexpr U32 GUARD_AFTER = 0x55667788;
constexpr U32 BSWAP_RESULT_OFFSET = MEM_BASE + 0x9000;

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

struct AddressCase {
    U32 address;
    asmjit::x86::Mem operand;
};

struct BoundCase {
    S32 value;
    S32 lower;
    S32 upper;
};

struct AddCase {
    U32 dst;
    U32 src;
};

struct EnterCase {
    U32 sp;
    U32 bp;
    U16 bytes;
    U8 level;
};

struct LeaveCase {
    U32 sp;
    U32 bp;
    U32 savedBp;
};

struct Lea16Case {
    U8 modrmBase;
    bool hasDisp8;
    bool hasDisp16;
    S32 disp;
    U32 expected;
};

struct Lea32Case {
    U8 rm;
    bool hasSib;
    U8 sib;
    bool hasDisp8;
    bool hasDisp32;
    S32 disp;
    U32 expected;
};

struct ConvertCase {
    U32 eax;
    U32 edx;
};

struct SahfCase {
    U32 eax;
    U32 initialFlags;
};

struct LahfCase {
    U32 eax;
    U32 initialFlags;
};

struct SalcCase {
    U32 eax;
    U32 initialFlags;
};

struct XlatCase {
    U32 eax;
    U32 ebx;
    U8 value;
};

struct XlatExpected {
    U32 eax;
    U32 flags;
};

struct FlagCase {
    U32 flags;
};

struct StackWrite {
    U32 offset;
    U32 value;
};

const BoundCase BOUND16_CASES[] = {
    {15, 10, 20},
    {10, 10, 20},
    {20, 10, 20},
    {-15, -20, -10}
};

const BoundCase BOUND32_CASES[] = {
    {15000, 10000, 20000},
    {10000, 10000, 20000},
    {20000, 10000, 20000},
    {-15000, -20000, -10000}
};

const AddCase OP_SIZE16_CASES[] = {
    {0x1000, 0x0003},
    {0xffff, 0x0001},
    {0x7fff, 0x0001},
    {0x8000, 0x8000}
};

const AddCase OP_SIZE32_CASES[] = {
    {0x10000000, 0x00000003},
    {0xffffffff, 0x00000001},
    {0x7fffffff, 0x00000001},
    {0x80000000, 0x80000000}
};

const AddCase ADDRESS_SIZE8_CASES[] = {
    {0x10, 0x03},
    {0xff, 0x01},
    {0x7f, 0x01},
    {0x80, 0x80}
};

const EnterCase ENTER16_CASES[] = {
    {0x00001200, 0x00001100, 0x0000, 0},
    {0xabcd1200, 0xabcd1100, 0x0020, 0},
    {0xabcd1200, 0xabcd1100, 0x0006, 1},
    {0xabcd1200, 0xabcd1100, 0x0010, 2},
    {0xabcd1200, 0xabcd1100, 0x0100, 4},
    {0xabcd0010, 0xabcdfff0, 0x0020, 2}
};

const EnterCase ENTER32_CASES[] = {
    {0x00001200, 0x00001100, 0x0000, 0},
    {0x00001200, 0x00001100, 0x0020, 0},
    {0x00001200, 0x00001100, 0x0006, 1},
    {0x00001200, 0x00001100, 0x0010, 2},
    {0x00001200, 0x00001100, 0x0100, 4}
};

const LeaveCase LEAVE16_CASES[] = {
    {0xabcd2222, 0xabcd1200, 0x5678},
    {0xffff0100, 0xffff00f0, 0xabcd},
    {0x76540010, 0x7654fffc, 0x1357}
};

const LeaveCase LEAVE32_CASES[] = {
    {0x00002222, 0x00001200, 0x12345678},
    {0x00000100, 0x00000ff0, 0x87654321},
    {0x00000010, 0x0000fffc, 0x0badcafe}
};

const Lea16Case LEA16_CASES[] = {
    {0x00, false, false, 0, 0x3000},
    {0x01, false, false, 0, 0x3100},
    {0x02, false, false, 0, 0x3200},
    {0x03, false, false, 0, 0x3300},
    {0x04, false, false, 0, 0x1000},
    {0x05, false, false, 0, 0x1100},
    {0x06, false, true, 0x7000, 0x7000},
    {0x07, false, false, 0, 0x2000},
    {0x40, true, false, 0x7f, 0x307f},
    {0x41, true, false, -0x10, 0x30f0},
    {0x42, true, false, 0x20, 0x3220},
    {0x43, true, false, -0x20, 0x32e0},
    {0x44, true, false, 0x11, 0x1011},
    {0x45, true, false, -0x11, 0x10ef},
    {0x46, true, false, 0x30, 0x2230},
    {0x47, true, false, -0x30, 0x1fd0},
    {0x80, false, true, 0x1234, 0x4234},
    {0x81, false, true, -0x100, 0x3000},
    {0x82, false, true, 0x4000, 0x7200},
    {0x83, false, true, -0x4000, 0xf300},
    {0x84, false, true, 0x2000, 0x3000},
    {0x85, false, true, -0x1000, 0x0100},
    {0x86, false, true, 0x0100, 0x2300},
    {0x87, false, true, 0xf000, 0x1000}
};

const Lea32Case LEA32_CASES[] = {
    {0x00, false, 0, false, false, 0, 0x00100000},
    {0x01, false, 0, false, false, 0, 0x00200000},
    {0x02, false, 0, false, false, 0, 0x00300000},
    {0x03, false, 0, false, false, 0, 0x00400000},
    {0x04, true, 0x24, false, false, 0, 0x00500000},
    {0x05, false, 0, false, true, 0x12345678, 0x12345678},
    {0x06, false, 0, false, false, 0, 0x00600000},
    {0x07, false, 0, false, false, 0, 0x00700000},
    {0x40, false, 0, true, false, 0x7f, 0x0010007f},
    {0x41, false, 0, true, false, -0x10, 0x001ffff0},
    {0x45, false, 0, true, false, 0x20, 0x00800020},
    {0x80, false, 0, false, true, 0x1234, 0x00101234},
    {0x81, false, 0, false, true, -0x1000, 0x001ff000},
    {0x85, false, 0, false, true, 0x0100, 0x00800100},
    {0x04, true, 0x88, false, false, 0, 0x00900000},
    {0x44, true, 0x53, true, false, 0x11, 0x00a00011},
    {0x84, true, 0x9b, false, true, -0x20, 0x013fffe0},
    {0x04, true, 0x25, false, true, 0x70000000, 0x70000000}
};

const ConvertCase CONVERT_CASES[] = {
    {0x12340000, 0x87650000},
    {0x12340001, 0x8765ffff},
    {0x1234007f, 0x87655555},
    {0x12340080, 0x8765aaaa},
    {0x123400ff, 0x87651234},
    {0x12347fff, 0x80000000},
    {0x12348000, 0x7fffffff},
    {0x1234ffff, 0xffffffff},
    {0x80000000, 0x13572468},
    {0xffffffff, 0x24681357}
};

const SahfCase SAHF_CASES[] = {
    {0x12340000, OF | DF},
    {0x1234ff00, 0},
    {0x1234d500, CF | OF | DF},
    {0x12342a00, PF | AF | ZF | SF | OF | DF},
    {0xffff0102, CF | PF | AF | ZF | SF | OF | DF},
    {0x0000807f, OF}
};

const LahfCase LAHF_CASES[] = {
    {0x123456ff, 0},
    {0xffffffff, CF | PF | AF | ZF | SF | OF | DF},
    {0x00000000, CF | AF | OF},
    {0xa5a55a5a, PF | ZF | DF},
    {0x87654321, SF | OF | DF}
};

const SalcCase SALC_CASES[] = {
    {0x12345678, 0},
    {0x87654321, CF},
    {0x000000ff, PF | AF | ZF | SF | OF | DF},
    {0xffffffff, CF | PF | AF | ZF | SF | OF | DF},
    {0x80000000, CF | OF},
    {0x7fffff80, SF | DF}
};

const XlatCase XLAT16_CASES[] = {
    {0x12340000, 0xabcd1200, 0x42},
    {0x12340001, 0xabcd1200, 0x80},
    {0x1234007f, 0xabcd1280, 0x00},
    {0x123400ff, 0xabcd2000, 0xff},
    {0x12340020, 0xabcdfff0, 0x19}
};

const XlatCase XLAT32_CASES[] = {
    {0x12340000, MEM_BASE + 0x1200, 0x42},
    {0x12340001, MEM_BASE + 0x1200, 0x80},
    {0x1234007f, MEM_BASE + 0x1280, 0x00},
    {0x123400ff, MEM_BASE + 0x2000, 0xff},
    {0x12340020, MEM_BASE + 0xf000, 0x19}
};

const FlagCase CMC_CASES[] = {
    {0},
    {CF},
    {PF | AF | ZF | SF | OF | DF},
    {CF | PF | AF | ZF | SF | OF | DF}
};

const U32 BSWAP_CASES[] = {
    0x00000000,
    0xffffffff,
    0x12345678,
    0x80000001,
    0x00ff0080,
    0x01020304
};

#if defined(_MSC_VER) && defined(_M_IX86)
#define TEST_MISC_HAS_HARDWARE_XLAT_ORACLE 1

XlatExpected hardwareXlat32(U32 eaxValue, const U8* table, U32 initialFlags) {
    U32 result = 0;
    U32 flags = 0;
    U32 savedFlags = 0;

    __asm {
        pushfd
        pop savedFlags
        push initialFlags
        popfd
        push ebx
        mov eax, eaxValue
        mov ebx, table
        xlat
        pop ebx
        pushfd
        pop flags
        push savedFlags
        popfd
        mov result, eax
    }

    return {result, flags};
}
#else
#define TEST_MISC_HAS_HARDWARE_XLAT_ORACLE 0
#endif

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit misc code init failed");
    }
}

void pushGeneratedCode(const asmjit::CodeHolder& code) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        pushCode8(buffer.data()[i]);
    }
}

void emitBound(const AddressCase& address, int reg, int width) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.bound(regForWidth(reg, width), address.operand) != asmjit::Error::kOk) {
        failed("asmjit bound failed");
    }
    pushGeneratedCode(code);
}

void emitEnter(U16 bytes, U8 level) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.enter(bytes, level) != asmjit::Error::kOk) {
        failed("asmjit enter failed");
    }
    pushGeneratedCode(code);
}

void emitLeave() {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.leave() != asmjit::Error::kOk) {
        failed("asmjit leave failed");
    }
    pushGeneratedCode(code);
}

void emitCwdeOpcode() {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.cwde() != asmjit::Error::kOk) {
        failed("asmjit cwde failed");
    }
    pushGeneratedCode(code);
}

void emitCdqOpcode() {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.cdq() != asmjit::Error::kOk) {
        failed("asmjit cdq failed");
    }
    pushGeneratedCode(code);
}

void emitBswap(int reg) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.bswap(reg32(reg)) != asmjit::Error::kOk) {
        failed("asmjit bswap failed");
    }
    pushGeneratedCode(code);
}

void emitStoreEspAndRestoreStack(U32 offset) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.mov(asmjit::x86::dword_ptr(offset), asmjit::x86::esp) != asmjit::Error::kOk ||
            a.mov(asmjit::x86::esp, 4096) != asmjit::Error::kOk) {
        failed("asmjit bswap esp restore failed");
    }
    pushGeneratedCode(code);
}

void emitSahf() {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.sahf() != asmjit::Error::kOk) {
        failed("asmjit sahf failed");
    }
    pushGeneratedCode(code);
}

void emitLahf() {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.lahf() != asmjit::Error::kOk) {
        failed("asmjit lahf failed");
    }
    pushGeneratedCode(code);
}

void emitSalc() {
    pushCode8(0xd6);
}

void emitXlat() {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (a.xlatb() != asmjit::Error::kOk) {
        failed("asmjit xlatb failed");
    }
    pushGeneratedCode(code);
}

void emitLea(U8 modrm, bool hasSib, U8 sib, bool hasDisp8, bool hasDisp16, bool hasDisp32, S32 disp) {
    pushCode8(0x8d);
    pushCode8(modrm);
    if (hasSib) {
        pushCode8(sib);
    }
    if (hasDisp8) {
        pushCode8((U8)disp);
    }
    if (hasDisp16) {
        pushCode16((U16)disp);
    }
    if (hasDisp32) {
        pushCode32((U32)disp);
    }
}

void emitMovFromMemory(U8 modrm, bool hasSib, U8 sib, bool hasDisp8, bool hasDisp16, bool hasDisp32, S32 disp) {
    pushCode8(0x8b);
    pushCode8(modrm);
    if (hasSib) {
        pushCode8(sib);
    }
    if (hasDisp8) {
        pushCode8((U8)disp);
    }
    if (hasDisp16) {
        pushCode16((U16)disp);
    }
    if (hasDisp32) {
        pushCode32((U32)disp);
    }
}

U32 opSizeOffset(bool defaultBig) {
    return defaultBig ? MEM_BASE + 0x3000 : 0x3000;
}

void emitOpSizeAddMemReg(bool defaultBig, int reg) {
    U32 offset = opSizeOffset(defaultBig);

    pushCode8(0x66);
    pushCode8(0x01);
    pushCode8((reg << 3) | (defaultBig ? 5 : 6));
    if (defaultBig) {
        pushCode32(offset);
    } else {
        pushCode16(offset);
    }
}

void emitAddressSizeAddMemImm(bool address32, U32 offset, U8 imm) {
    pushCode8(0x67);
    pushCode8(0x80);
    pushCode8(address32 ? 0x05 : 0x06);
    if (address32) {
        pushCode32(offset);
    } else {
        pushCode16(offset);
    }
    pushCode8(imm);
}

U32 flagsForAdd(U32 lhs, U32 rhs, U32 result, int width) {
    U32 mask = widthMask(width);
    U32 sign = signBit(width);
    U64 fullResult = (U64)(lhs & mask) + (U64)(rhs & mask);
    U32 flags = 0;

    result &= mask;
    if (fullResult > mask) flags |= CF;
    if (((lhs ^ result) & (rhs ^ result) & sign) != 0) flags |= OF;
    if (((lhs ^ rhs ^ result) & 0x10) != 0) flags |= AF;
    if (result == 0) flags |= ZF;
    if ((result & sign) != 0) flags |= SF;

    U8 low = (U8)result;
    low ^= low >> 4;
    low &= 0x0f;
    if (((0x6996 >> low) & 1) == 0) flags |= PF;
    return flags;
}

void verifyFlagsUnchanged(const char* name) {
    if ((actualFlags(cpu, true) & INITIAL_FLAGS) != INITIAL_FLAGS) {
        failed("%s flags changed", name);
    }
}

void verifyAddFlags(U32 expectedFlags, const char* name) {
    if (((actualFlags(cpu, true) ^ expectedFlags) & ADD_FLAG_MASK) != 0) {
        failed("%s flags", name);
    }
}

void verifyLinearMemoryUnchanged(U32 linear, const char* name) {
    memory->writed(linear - 4, GUARD_BEFORE);
    memory->writed(linear, 0x89abcdef);
    memory->writed(linear + 4, GUARD_AFTER);
    if (memory->readd(linear - 4) != GUARD_BEFORE ||
            memory->readd(linear) != 0x89abcdef ||
            memory->readd(linear + 4) != GUARD_AFTER) {
        failed("%s memory setup", name);
    }
}

void checkLinearMemoryUnchanged(U32 linear, const char* name) {
    if (memory->readd(linear - 4) != GUARD_BEFORE ||
            memory->readd(linear) != 0x89abcdef ||
            memory->readd(linear + 4) != GUARD_AFTER) {
        failed("%s memory changed", name);
    }
}

U32 stackItemMask(int width) {
    return width == 16 ? 0xffff : 0xffffffff;
}

U32 stackOffset(U32 value, int width) {
    return value & stackItemMask(width);
}

U32 stackLinear(U32 offset) {
    return cpu->seg[SS].address + offset;
}

U32 readStackValue(U32 offset, int width) {
    return width == 16 ? memory->readw(stackLinear(offset)) : memory->readd(stackLinear(offset));
}

void writeStackValue(U32 offset, U32 value, int width) {
    if (width == 16) {
        memory->writew(stackLinear(offset), value);
    } else {
        memory->writed(stackLinear(offset), value);
    }
}

void configureStackMode(int width) {
    if (width == 16) {
        cpu->stackMask = 0xffff;
        cpu->stackNotMask = 0xffff0000;
    } else {
        cpu->stackMask = 0xffffffff;
        cpu->stackNotMask = 0;
    }
}

U32 segmentBaseForReg(int reg) {
    return (reg == R_BP || reg == R_SP) ? cpu->seg[SS].address : cpu->seg[DS].address;
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

void setBoundRegValue(U32* regs, int reg, int width, S32 value) {
    if (width == 16) {
        regs[reg] = (regs[reg] & 0xffff0000) | ((U32)value & 0xffff);
    } else {
        regs[reg] = (U32)value;
    }
}

void writeBounds(const AddressCase& address, int width, S32 lower, S32 upper) {
    memory->writed(address.address - 4, GUARD_BEFORE);
    if (width == 16) {
        memory->writew(address.address, (U16)lower);
        memory->writew(address.address + 2, (U16)upper);
        memory->writed(address.address + 4, GUARD_AFTER);
    } else {
        memory->writed(address.address, (U32)lower);
        memory->writed(address.address + 4, (U32)upper);
        memory->writed(address.address + 8, GUARD_AFTER);
    }
}

void verifyBounds(const AddressCase& address, int width, S32 lower, S32 upper, const char* name) {
    if (memory->readd(address.address - 4) != GUARD_BEFORE) {
        failed("%s lower memory guard", name);
    }
    if (width == 16) {
        if (memory->readw(address.address) != ((U32)lower & 0xffff) ||
                memory->readw(address.address + 2) != ((U32)upper & 0xffff) ||
                memory->readd(address.address + 4) != GUARD_AFTER) {
            failed("%s memory bounds", name);
        }
    } else if (memory->readd(address.address) != (U32)lower ||
            memory->readd(address.address + 4) != (U32)upper ||
            memory->readd(address.address + 8) != GUARD_AFTER) {
        failed("%s memory bounds", name);
    }
}

void makeAbsoluteCase(AddressCase& data, U32 offset, int width) {
    data.address = cpu->seg[DS].address + offset;
    data.operand = memPtr(offset, width);
}

void makeBaseCase(AddressCase& data, int base, U32 offset, int disp, int width) {
    data.address = segmentBaseForReg(base) + offset + disp;
    data.operand = memPtr(reg32(base), disp, width);
}

void makeSibCase(AddressCase& data, int base, int index, int shift, U32 targetOffset, const U32* regs, int width) {
    data.address = segmentBaseForReg(base) + targetOffset;
    data.operand = memPtr(reg32(base), reg32(index), shift, targetOffset - regs[base] - (regs[index] << shift), width);
}

void beginInstruction() {
    newInstruction(INITIAL_FLAGS);
    cpu->big = true;
}

void beginInstruction(bool big) {
    newInstruction(INITIAL_FLAGS);
    cpu->big = big;
}

void beginInstruction(U32 flags, bool big) {
    newInstruction(flags);
    cpu->big = big;
}

void emitCmc() {
    pushCode8(0xf5);
}

void emitClc() {
    pushCode8(0xf8);
}

void emitStc() {
    pushCode8(0xf9);
}

void runFlagCase(const FlagCase& data, bool big, U32 expectedFlags, void (*emitInstruction)(), const char* name) {
    U32 expectedRegs[8];

    beginInstruction(data.flags, big);
    emitInstruction();
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (((actualFlags(cpu, true) ^ expectedFlags) & INITIAL_FLAGS) != 0) {
        failed("%s flags", name);
    }
}

void runCmcCases(bool big, const char* name) {
    for (size_t i = 0; i < caseCount(CMC_CASES); ++i) {
        runFlagCase(CMC_CASES[i], big, CMC_CASES[i].flags ^ CF, emitCmc, name);
    }
}

void runClcCases(bool big, const char* name) {
    for (size_t i = 0; i < caseCount(CMC_CASES); ++i) {
        runFlagCase(CMC_CASES[i], big, CMC_CASES[i].flags & ~CF, emitClc, name);
    }
}

void runStcCases(bool big, const char* name) {
    for (size_t i = 0; i < caseCount(CMC_CASES); ++i) {
        runFlagCase(CMC_CASES[i], big, CMC_CASES[i].flags | CF, emitStc, name);
    }
}

void runPreparedBoundCase(int width, const BoundCase& data, int reg, const AddressCase& address, const U32* regs, const char* name) {
    U32 expectedRegs[8];

    writeRegs(cpu, regs);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }
    writeBounds(address, width, data.lower, data.upper);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    verifyBounds(address, width, data.lower, data.upper, name);
    verifyFlagsUnchanged(name);
}

void runAbsoluteBoundCase(int width, const BoundCase& data, int reg, const char* name) {
    U32 regs[8];
    AddressCase address;

    initAddressRegisters(regs);
    setBoundRegValue(regs, reg, width, data.value);
    makeAbsoluteCase(address, MEM_BASE + 0x3000 + reg * 0x20, width);
    beginInstruction();
    emitBound(address, reg, width);
    runPreparedBoundCase(width, data, reg, address, regs, name);
}

void runBaseBoundCases(int width, const BoundCase& data, int reg, const char* name) {
    for (int base = 0; base < 8; ++base) {
        U32 regs[8];
        initAddressRegisters(regs);
        regs[base] = MEM_BASE + 0x1000 + base * 0x80;
        setBoundRegValue(regs, reg, width, data.value);

        if (base != R_BP) {
            AddressCase noDisp;
            makeBaseCase(noDisp, base, regs[base], 0, width);
            beginInstruction();
            emitBound(noDisp, reg, width);
            runPreparedBoundCase(width, data, reg, noDisp, regs, name);
        }

        AddressCase disp8;
        makeBaseCase(disp8, base, regs[base], 0x11, width);
        beginInstruction();
        emitBound(disp8, reg, width);
        runPreparedBoundCase(width, data, reg, disp8, regs, name);

        AddressCase disp32;
        makeBaseCase(disp32, base, regs[base], 0x123, width);
        beginInstruction();
        emitBound(disp32, reg, width);
        runPreparedBoundCase(width, data, reg, disp32, regs, name);
    }
}

void runSibBoundCases(int width, const BoundCase& data, int reg, const char* name) {
    for (int base = 0; base < 8; ++base) {
        for (int index = 0; index < 8; ++index) {
            if (index == R_SP) {
                continue;
            }
            for (int shift = 0; shift < 4; ++shift) {
                U32 regs[8];
                AddressCase address;
                U32 targetOffset = MEM_BASE + 0x7000 + base * 0x200 + index * 0x20 + shift * 4;
                initAddressRegisters(regs);
                regs[base] = MEM_BASE + 0x0100 + base * 0x40;
                regs[index] = 3;
                setBoundRegValue(regs, reg, width, data.value);
                makeSibCase(address, base, index, shift, targetOffset, regs, width);
                beginInstruction();
                emitBound(address, reg, width);
                runPreparedBoundCase(width, data, reg, address, regs, name);
            }
        }
    }
}

void runBoundCases(int width, const BoundCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int reg = 0; reg < 8; ++reg) {
            runAbsoluteBoundCase(width, cases[i], reg, name);

            if (cases[i].value < 0) {
                continue;
            }
            runBaseBoundCases(width, cases[i], reg, name);
            runSibBoundCases(width, cases[i], reg, name);
        }
    }
}

void runOpSizePrefixCase(bool defaultBig, int width, const AddCase& data, const char* name) {
    U32 address = cpu->seg[DS].address + opSizeOffset(defaultBig);
    U32 initialMemory = width == 16 ? (0xa5a50000 | (data.dst & 0xffff)) : data.dst;
    U32 expectedMemory = width == 16 ? ((initialMemory & 0xffff0000) | ((data.dst + data.src) & 0xffff)) : data.dst + data.src;
    U32 expectedFlags = flagsForAdd(data.dst, data.src, expectedMemory, width);
    U32 expectedRegs[8];

    beginInstruction(defaultBig);
    emitOpSizeAddMemReg(defaultBig, R_CX);
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    if (width == 16) {
        cpu->reg[R_CX].u16 = (U16)data.src;
        expectedRegs[R_CX] = cpu->reg[R_CX].u32;
    } else {
        cpu->reg[R_CX].u32 = data.src;
        expectedRegs[R_CX] = cpu->reg[R_CX].u32;
    }
    memory->writed(address - 4, GUARD_BEFORE);
    memory->writed(address, initialMemory);
    memory->writed(address + 4, GUARD_AFTER);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (memory->readd(address - 4) != GUARD_BEFORE ||
            memory->readd(address) != expectedMemory ||
            memory->readd(address + 4) != GUARD_AFTER) {
        failed("%s memory", name);
    }
    verifyAddFlags(expectedFlags, name);
}

void runOpSizePrefixCases(bool defaultBig, int width, const AddCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        runOpSizePrefixCase(defaultBig, width, cases[i], name);
    }
}

void runAddressPrefixCase(bool defaultBig, bool address32, U32 offset, U32 wrongOffset, const AddCase& data, const char* name) {
    U32 address = cpu->seg[DS].address + offset;
    U32 wrongAddress = cpu->seg[DS].address + wrongOffset;
    U8 expected = (U8)(data.dst + data.src);
    U32 expectedFlags = flagsForAdd(data.dst, data.src, expected, 8);
    U32 expectedRegs[8];

    beginInstruction(defaultBig);
    emitAddressSizeAddMemImm(address32, offset, (U8)data.src);
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    memory->writeb(address - 1, 0x11);
    memory->writeb(address, data.dst);
    memory->writeb(address + 1, 0x33);
    memory->writeb(address + 2, 0x44);
    memory->writeb(wrongAddress, 0x5a);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (memory->readb(address - 1) != 0x11 ||
            memory->readb(address) != expected ||
            memory->readb(address + 1) != 0x33 ||
            memory->readb(address + 2) != 0x44 ||
            memory->readb(wrongAddress) != 0x5a) {
        failed("%s memory", name);
    }
    verifyAddFlags(expectedFlags, name);
}

void runAddressPrefixCases(bool defaultBig, bool address32, const AddCase* cases, size_t count, const char* name) {
    U32 offset = address32 ? MEM_BASE + 0x2345 : 0x3456;
    U32 wrongOffset = address32 ? (offset & 0xffff) : MEM_BASE + 0x3456;

    for (size_t i = 0; i < count; ++i) {
        runAddressPrefixCase(defaultBig, address32, offset + (U32)i * 0x10, wrongOffset + (U32)i * 0x10, cases[i], name);
    }
}

void initLea16Regs(U32* regs) {
    regs[R_AX] = REG_GUARD | 0x0100;
    regs[R_CX] = REG_GUARD | 0x0200;
    regs[R_DX] = REG_GUARD | 0x0300;
    regs[R_BX] = REG_GUARD | 0x2000;
    regs[R_SP] = REG_GUARD | 0x0400;
    regs[R_BP] = REG_GUARD | 0x2200;
    regs[R_SI] = REG_GUARD | 0x1000;
    regs[R_DI] = REG_GUARD | 0x1100;
}

void initLea32Regs(U32* regs) {
    regs[R_AX] = 0x00100000;
    regs[R_CX] = 0x00200000;
    regs[R_DX] = 0x00300000;
    regs[R_BX] = 0x00400000;
    regs[R_SP] = 0x00500000;
    regs[R_BP] = 0x00800000;
    regs[R_SI] = 0x00600000;
    regs[R_DI] = 0x00700000;
}

void runLea16Case(const Lea16Case& data, int dstReg, const char* name) {
    char caseName[96];
    U32 regs[8];
    U32 expectedRegs[8];
    U32 linear = cpu->seg[DS].address + 0x7000;

    snprintf(caseName, sizeof(caseName), "%s dst=%d modrm=%02x", name, dstReg, data.modrmBase);
    initLea16Regs(regs);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }
    expectedRegs[dstReg] = (expectedRegs[dstReg] & 0xffff0000) | (data.expected & 0xffff);

    beginInstruction(false);
    emitLea(data.modrmBase | (dstReg << 3), false, 0, data.hasDisp8, data.hasDisp16, false, data.disp);
    writeRegs(cpu, regs);
    verifyLinearMemoryUnchanged(linear, caseName);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    checkLinearMemoryUnchanged(linear, caseName);
    verifyFlagsUnchanged(caseName);
}

void runLea32Case(const Lea32Case& data, int dstReg, const char* name) {
    char caseName[96];
    U32 regs[8];
    U32 expectedRegs[8];
    U32 linear = cpu->seg[DS].address + 0x7000;

    snprintf(caseName, sizeof(caseName), "%s dst=%d rm=%02x", name, dstReg, data.rm);
    initLea32Regs(regs);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }
    expectedRegs[dstReg] = data.expected;

    beginInstruction(true);
    emitLea(data.rm | (dstReg << 3), data.hasSib, data.sib, data.hasDisp8, false, data.hasDisp32, data.disp);
    writeRegs(cpu, regs);
    verifyLinearMemoryUnchanged(linear, caseName);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    checkLinearMemoryUnchanged(linear, caseName);
    verifyFlagsUnchanged(caseName);
}

void runLea16Cases(const char* name) {
    for (size_t i = 0; i < caseCount(LEA16_CASES); ++i) {
        for (int dstReg = 0; dstReg < 8; ++dstReg) {
            runLea16Case(LEA16_CASES[i], dstReg, name);
        }
    }
}

void runLea32Cases(const char* name) {
    for (size_t i = 0; i < caseCount(LEA32_CASES); ++i) {
        for (int dstReg = 0; dstReg < 8; ++dstReg) {
            runLea32Case(LEA32_CASES[i], dstReg, name);
        }
    }
}

U16 effectiveAddress16(int rm, const U32* regs, S32 disp, bool directAddress) {
    if (directAddress) {
        return (U16)disp;
    }
    switch (rm) {
    case 0: return (U16)(regs[R_BX] + regs[R_SI] + disp);
    case 1: return (U16)(regs[R_BX] + regs[R_DI] + disp);
    case 2: return (U16)(regs[R_BP] + regs[R_SI] + disp);
    case 3: return (U16)(regs[R_BP] + regs[R_DI] + disp);
    case 4: return (U16)(regs[R_SI] + disp);
    case 5: return (U16)(regs[R_DI] + disp);
    case 6: return (U16)(regs[R_BP] + disp);
    default: return (U16)(regs[R_BX] + disp);
    }
}

U32 segmentForAddress16(int rm, bool directAddress) {
    if (directAddress) {
        return cpu->seg[DS].address;
    }
    return (rm == 2 || rm == 3 || rm == 6) ? cpu->seg[SS].address : cpu->seg[DS].address;
}

void writeMemoryAccess16Regs(U32* regs, int rm, U16 first, U16 second) {
    initLea16Regs(regs);
    switch (rm) {
    case 0:
        regs[R_BX] = (regs[R_BX] & 0xffff0000) | first;
        regs[R_SI] = (regs[R_SI] & 0xffff0000) | second;
        break;
    case 1:
        regs[R_BX] = (regs[R_BX] & 0xffff0000) | first;
        regs[R_DI] = (regs[R_DI] & 0xffff0000) | second;
        break;
    case 2:
        regs[R_BP] = (regs[R_BP] & 0xffff0000) | first;
        regs[R_SI] = (regs[R_SI] & 0xffff0000) | second;
        break;
    case 3:
        regs[R_BP] = (regs[R_BP] & 0xffff0000) | first;
        regs[R_DI] = (regs[R_DI] & 0xffff0000) | second;
        break;
    case 4:
        regs[R_SI] = (regs[R_SI] & 0xffff0000) | first;
        break;
    case 5:
        regs[R_DI] = (regs[R_DI] & 0xffff0000) | first;
        break;
    case 6:
        regs[R_BP] = (regs[R_BP] & 0xffff0000) | first;
        break;
    case 7:
        regs[R_BX] = (regs[R_BX] & 0xffff0000) | first;
        break;
    }
}

void runMemoryAccess16Encoding(U8 modrmBase, bool hasDisp8, bool hasDisp16, S32 disp, const U32* regs, U16 expectedOffset, U32 segment, int dstReg, const char* name) {
    char caseName[128];
    U32 expectedRegs[8];
    U32 linear = segment + expectedOffset;
    U16 value = (U16)(0x3500 | dstReg);

    snprintf(caseName, sizeof(caseName), "%s dst=%d modrm=%02x", name, dstReg, modrmBase);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }
    expectedRegs[dstReg] = (expectedRegs[dstReg] & 0xffff0000) | expectedOffset;

    beginInstruction(false);
    emitLea(modrmBase | (dstReg << 3), false, 0, hasDisp8, hasDisp16, false, disp);
    writeRegs(cpu, regs);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    verifyFlagsUnchanged(caseName);

    beginInstruction(false);
    emitMovFromMemory(modrmBase | (dstReg << 3), false, 0, hasDisp8, hasDisp16, false, disp);
    writeRegs(cpu, regs);
    if (expectedOffset >= 2) {
        memory->writew(linear - 2, (U16)GUARD_BEFORE);
    }
    memory->writew(linear, value);
    memory->writew(linear + 2, (U16)GUARD_AFTER);
    expectedRegs[dstReg] = (expectedRegs[dstReg] & 0xffff0000) | value;

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    if ((expectedOffset >= 2 && memory->readw(linear - 2) != (GUARD_BEFORE & 0xffff)) ||
            memory->readw(linear) != value ||
            memory->readw(linear + 2) != (GUARD_AFTER & 0xffff)) {
        failed("%s memory", caseName);
    }
    verifyFlagsUnchanged(caseName);
}

void runMemoryAccess16Cases(const char* name) {
    static const U16 values[][2] = {
        {0, 0},
        {1, 0},
        {0, 1},
        {0xfffe, 0},
        {0, 0xfffe},
        {0xfffe, 2},
        {2, 0xfffe},
        {0xfffe, 0xfffe}
    };
    static const S32 disp8Values[] = {0x10, -0x10};
    static const S32 disp16Values[] = {0x1000, -0x1000, 0x10010};

    for (int rm = 0; rm < 8; ++rm) {
        for (size_t valueIndex = 0; valueIndex < caseCount(values); ++valueIndex) {
            U32 regs[8];
            bool directAddress = rm == 6;
            S32 disp = directAddress ? 0x7000 : 0;
            writeMemoryAccess16Regs(regs, rm, values[valueIndex][0], values[valueIndex][1]);
            U16 expected = effectiveAddress16(rm, regs, disp, directAddress);
            U32 segment = segmentForAddress16(rm, directAddress);
            for (int dst = 0; dst < 8; ++dst) {
                runMemoryAccess16Encoding((U8)rm, false, directAddress, disp, regs, expected, segment, dst, name);
            }
        }
    }

    for (int rm = 0; rm < 8; ++rm) {
        for (size_t i = 0; i < caseCount(disp8Values); ++i) {
            U32 regs[8];
            writeMemoryAccess16Regs(regs, rm, 0x1000, 0x0100);
            U16 expected = effectiveAddress16(rm, regs, disp8Values[i], false);
            U32 segment = segmentForAddress16(rm, false);
            for (int dst = 0; dst < 8; ++dst) {
                runMemoryAccess16Encoding((U8)(0x40 | rm), true, false, disp8Values[i], regs, expected, segment, dst, name);
            }
        }

        for (size_t i = 0; i < caseCount(disp16Values); ++i) {
            U32 regs[8];
            writeMemoryAccess16Regs(regs, rm, 0x1000, 0x0100);
            U16 expected = effectiveAddress16(rm, regs, disp16Values[i], false);
            U32 segment = segmentForAddress16(rm, false);
            for (int dst = 0; dst < 8; ++dst) {
                runMemoryAccess16Encoding((U8)(0x80 | rm), false, true, disp16Values[i], regs, expected, segment, dst, name);
            }
        }
    }
}

bool sibHasBase(int mod, int base) {
    return mod != 0 || base != R_BP;
}

bool sibHasIndex(int index) {
    return index != R_SP;
}

U32 segmentForAddress32(int mod, int rm, bool hasSib, U8 sib) {
    if (!hasSib) {
        if (mod == 0 && rm == R_BP) {
            return cpu->seg[DS].address;
        }
        return segmentBaseForReg(rm);
    }

    int base = sib & 7;
    if (!sibHasBase(mod, base)) {
        return cpu->seg[DS].address;
    }
    return segmentBaseForReg(base);
}

U32 effectiveAddress32(int mod, int rm, bool hasSib, U8 sib, const U32* regs, S32 disp) {
    if (!hasSib) {
        if (mod == 0 && rm == R_BP) {
            return (U32)disp;
        }
        return regs[rm] + (U32)disp;
    }

    int base = sib & 7;
    int index = (sib >> 3) & 7;
    int scale = sib >> 6;
    U32 result = (U32)disp;
    if (sibHasBase(mod, base)) {
        result += regs[base];
    }
    if (sibHasIndex(index)) {
        result += regs[index] << scale;
    }
    return result;
}

void runMemoryAccess32Encoding(U8 modrmBase, bool hasSib, U8 sib, bool hasDisp8, bool hasDisp32, S32 disp, const U32* regs, U32 expectedOffset, U32 segment, int dstReg, const char* name) {
    char caseName[128];
    U32 expectedRegs[8];
    U32 linear = segment + expectedOffset;
    U32 value = 0x35790000 | (U32)dstReg;

    snprintf(caseName, sizeof(caseName), "%s dst=%d modrm=%02x sib=%02x", name, dstReg, modrmBase, sib);
    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = regs[i];
    }
    expectedRegs[dstReg] = expectedOffset;

    beginInstruction(true);
    emitLea(modrmBase | (dstReg << 3), hasSib, sib, hasDisp8, false, hasDisp32, disp);
    writeRegs(cpu, regs);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    verifyFlagsUnchanged(caseName);

    beginInstruction(true);
    emitMovFromMemory(modrmBase | (dstReg << 3), hasSib, sib, hasDisp8, false, hasDisp32, disp);
    writeRegs(cpu, regs);
    if (expectedOffset >= 4) {
        memory->writed(linear - 4, GUARD_BEFORE);
    }
    memory->writed(linear, value);
    memory->writed(linear + 4, GUARD_AFTER);
    expectedRegs[dstReg] = value;

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    if ((expectedOffset >= 4 && memory->readd(linear - 4) != GUARD_BEFORE) ||
            memory->readd(linear) != value ||
            memory->readd(linear + 4) != GUARD_AFTER) {
        failed("%s memory", caseName);
    }
    verifyFlagsUnchanged(caseName);
}

void initMemoryAccess32Regs(U32* regs) {
    initAddressRegisters(regs);
    regs[R_AX] += 0x0000;
    regs[R_CX] += 0x1000;
    regs[R_DX] += 0x2000;
    regs[R_BX] += 0x3000;
    regs[R_SP] += 0x4000;
    regs[R_BP] += 0x5000;
    regs[R_SI] += 0x6000;
    regs[R_DI] += 0x7000;
}

void runMemoryAccess32Cases(const char* name) {
    static const S32 disp8Values[] = {0x10, -0x10};
    static const S32 disp32Values[] = {0x1234, -0x1000, 0x10};

    for (int mod = 0; mod < 3; ++mod) {
        for (int rm = 0; rm < 8; ++rm) {
            bool hasSib = rm == R_SP;
            if (hasSib) {
                continue;
            }
            int dispCount = mod == 0 ? 1 : (mod == 1 ? (int)caseCount(disp8Values) : (int)caseCount(disp32Values));
            for (int dispIndex = 0; dispIndex < dispCount; ++dispIndex) {
                U32 regs[8];
                initMemoryAccess32Regs(regs);
                S32 disp = 0;
                bool hasDisp8 = false;
                bool hasDisp32 = false;
                if (mod == 0 && rm == R_BP) {
                    disp = MEM_BASE + 0x3000;
                    hasDisp32 = true;
                } else if (mod == 1) {
                    disp = disp8Values[dispIndex];
                    hasDisp8 = true;
                } else if (mod == 2) {
                    disp = disp32Values[dispIndex];
                    hasDisp32 = true;
                }

                U32 expected = effectiveAddress32(mod, rm, false, 0, regs, disp);
                U32 segment = segmentForAddress32(mod, rm, false, 0);
                for (int dst = 0; dst < 8; ++dst) {
                    runMemoryAccess32Encoding((U8)((mod << 6) | rm), false, 0, hasDisp8, hasDisp32, disp, regs, expected, segment, dst, name);
                }
            }
        }
    }

    for (int mod = 0; mod < 3; ++mod) {
        int dispCount = mod == 0 ? 1 : (mod == 1 ? (int)caseCount(disp8Values) : (int)caseCount(disp32Values));
        for (int base = 0; base < 8; ++base) {
            for (int index = 0; index < 8; ++index) {
                for (int scale = 0; scale < 4; ++scale) {
                    U8 sib = (U8)((scale << 6) | (index << 3) | base);
                    for (int dispIndex = 0; dispIndex < dispCount; ++dispIndex) {
                        U32 regs[8];
                        initMemoryAccess32Regs(regs);
                        S32 disp = 0;
                        bool hasDisp8 = false;
                        bool hasDisp32 = false;
                        bool hasBase = sibHasBase(mod, base);
                        bool hasIndex = sibHasIndex(index);
                        U32 targetOffset = MEM_BASE + 0x3000 + (U32)mod * 0x4000 + (U32)base * 0x400 + (U32)index * 0x40 + (U32)scale * 4;

                        if (!hasBase) {
                            if (hasIndex) {
                                regs[index] = 3;
                            }
                            disp = (S32)(targetOffset - (hasIndex ? regs[index] << scale : 0));
                            hasDisp32 = true;
                        } else {
                            if (mod == 1) {
                                disp = disp8Values[dispIndex];
                                hasDisp8 = true;
                            } else if (mod == 2) {
                                disp = disp32Values[dispIndex];
                                hasDisp32 = true;
                            }

                            if (hasIndex && base == index) {
                                U32 factor = 1u + (1u << scale);
                                regs[base] = (targetOffset - (U32)disp) / factor;
                            } else {
                                if (hasIndex) {
                                    regs[index] = 3;
                                }
                                regs[base] = targetOffset - (hasIndex ? regs[index] << scale : 0) - (U32)disp;
                            }
                        }

                        U32 expected = effectiveAddress32(mod, R_SP, true, sib, regs, disp);
                        U32 segment = segmentForAddress32(mod, R_SP, true, sib);
                        for (int dst = 0; dst < 8; ++dst) {
                            runMemoryAccess32Encoding((U8)((mod << 6) | R_SP), true, sib, hasDisp8, hasDisp32, disp, regs, expected, segment, dst, name);
                        }
                    }
                }
            }
        }
    }
}

void initConvertRegs(const ConvertCase& data, U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    cpu->reg[R_AX].u32 = data.eax;
    cpu->reg[R_DX].u32 = data.edx;
    expectedRegs[R_AX] = data.eax;
    expectedRegs[R_DX] = data.edx;
}

void runCbwCase(const ConvertCase& data, const char* name) {
    U32 expectedRegs[8];
    U32 linear = cpu->seg[DS].address + 0x7000;

    beginInstruction(false);
    emitCwdeOpcode();
    initConvertRegs(data, expectedRegs);
    expectedRegs[R_AX] = (data.eax & 0xffff0000) | (U16)(S16)(S8)data.eax;
    verifyLinearMemoryUnchanged(linear, name);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    checkLinearMemoryUnchanged(linear, name);
    verifyFlagsUnchanged(name);
}

void runCwdeCase(const ConvertCase& data, const char* name) {
    U32 expectedRegs[8];
    U32 linear = cpu->seg[DS].address + 0x7000;

    beginInstruction(true);
    emitCwdeOpcode();
    initConvertRegs(data, expectedRegs);
    expectedRegs[R_AX] = (U32)(S32)(S16)data.eax;
    verifyLinearMemoryUnchanged(linear, name);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    checkLinearMemoryUnchanged(linear, name);
    verifyFlagsUnchanged(name);
}

void runCwdCase(const ConvertCase& data, const char* name) {
    U32 expectedRegs[8];
    U32 linear = cpu->seg[DS].address + 0x7000;
    U16 expectedDx = (data.eax & 0x8000) ? 0xffff : 0x0000;

    beginInstruction(false);
    emitCdqOpcode();
    initConvertRegs(data, expectedRegs);
    expectedRegs[R_DX] = (data.edx & 0xffff0000) | expectedDx;
    verifyLinearMemoryUnchanged(linear, name);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    checkLinearMemoryUnchanged(linear, name);
    verifyFlagsUnchanged(name);
}

void runCdqCase(const ConvertCase& data, const char* name) {
    U32 expectedRegs[8];
    U32 linear = cpu->seg[DS].address + 0x7000;

    beginInstruction(true);
    emitCdqOpcode();
    initConvertRegs(data, expectedRegs);
    expectedRegs[R_DX] = (data.eax & 0x80000000) ? 0xffffffff : 0x00000000;
    verifyLinearMemoryUnchanged(linear, name);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    checkLinearMemoryUnchanged(linear, name);
    verifyFlagsUnchanged(name);
}

void runConvertCases(void (*runCase)(const ConvertCase&, const char*), const char* name) {
    for (size_t i = 0; i < caseCount(CONVERT_CASES); ++i) {
        runCase(CONVERT_CASES[i], name);
    }
}

void initOneByteRegs(U32 eax, U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0200 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    cpu->reg[R_AX].u32 = eax;
    expectedRegs[R_AX] = eax;
}

void runSahfCase(const SahfCase& data, bool big, const char* name) {
    U32 expectedRegs[8];
    U32 linear = cpu->seg[DS].address + 0x7000;
    U32 expectedFlags = (data.initialFlags & ~SAHF_FLAG_MASK) | ((data.eax >> 8) & SAHF_FLAG_MASK);

    newInstruction(data.initialFlags);
    cpu->big = big;
    emitSahf();
    initOneByteRegs(data.eax, expectedRegs);
    verifyLinearMemoryUnchanged(linear, name);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    checkLinearMemoryUnchanged(linear, name);
    if (((actualFlags(cpu, true) ^ expectedFlags) & (SAHF_FLAG_MASK | OF | DF)) != 0) {
        failed("%s flags", name);
    }
}

void runLahfCase(const LahfCase& data, bool big, const char* name) {
    U32 expectedRegs[8];
    U32 linear = cpu->seg[DS].address + 0x7000;
    U32 expectedAh = (data.initialFlags & SAHF_FLAG_MASK) | 2;

    newInstruction(data.initialFlags);
    cpu->big = big;
    emitLahf();
    initOneByteRegs(data.eax, expectedRegs);
    expectedRegs[R_AX] = (data.eax & 0xffff00ff) | (expectedAh << 8);
    verifyLinearMemoryUnchanged(linear, name);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    checkLinearMemoryUnchanged(linear, name);
    if (((actualFlags(cpu, true) ^ data.initialFlags) & (SAHF_FLAG_MASK | OF | DF)) != 0) {
        failed("%s flags changed", name);
    }
}

void runSahfCases(bool big, const char* name) {
    for (size_t i = 0; i < caseCount(SAHF_CASES); ++i) {
        runSahfCase(SAHF_CASES[i], big, name);
    }
}

void runLahfCases(bool big, const char* name) {
    for (size_t i = 0; i < caseCount(LAHF_CASES); ++i) {
        runLahfCase(LAHF_CASES[i], big, name);
    }
}

void runSalcCase(const SalcCase& data, bool big, const char* name) {
    U32 expectedRegs[8];
    U32 linear = cpu->seg[DS].address + 0x7000;
    U32 expectedAl = (data.initialFlags & CF) ? 0xff : 0x00;

    newInstruction(data.initialFlags);
    cpu->big = big;
    emitSalc();
    initOneByteRegs(data.eax, expectedRegs);
    expectedRegs[R_AX] = (data.eax & 0xffffff00) | expectedAl;
    verifyLinearMemoryUnchanged(linear, name);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    checkLinearMemoryUnchanged(linear, name);
    if (((actualFlags(cpu, true) ^ data.initialFlags) & (CF | PF | AF | ZF | SF | OF | DF)) != 0) {
        failed("%s flags changed", name);
    }
}

void runSalcCases(bool big, const char* name) {
    for (size_t i = 0; i < caseCount(SALC_CASES); ++i) {
        runSalcCase(SALC_CASES[i], big, name);
    }
}

U32 xlatOffset(const XlatCase& data, bool big) {
    if (big) {
        return data.ebx + (data.eax & 0xff);
    }
    return (U16)((U16)data.ebx + (U8)data.eax);
}

XlatExpected calculatedXlat(const XlatCase& data, U32 initialFlags) {
    return {(data.eax & 0xffffff00) | data.value, initialFlags};
}

XlatExpected expectedXlat(const XlatCase& data, bool big, U32 initialFlags) {
#if TEST_MISC_HAS_HARDWARE_XLAT_ORACLE
    if (big) {
        U8 table[256];
        XlatExpected calculated = calculatedXlat(data, initialFlags);
        memset(table, 0xcc, sizeof(table));
        table[data.eax & 0xff] = data.value;
        XlatExpected hardware = hardwareXlat32(data.eax, table, initialFlags);
        if (hardware.eax != calculated.eax || ((hardware.flags ^ calculated.flags) & INITIAL_FLAGS) != 0) {
            failed("hardware xlat oracle mismatch");
        }
        return hardware;
    }
#endif
    return calculatedXlat(data, initialFlags);
}

void writeXlatTableByte(U32 linear, U8 value) {
    memory->writed(linear - 4, GUARD_BEFORE);
    memory->writeb(linear - 1, 0x11);
    memory->writeb(linear, value);
    memory->writeb(linear + 1, 0x22);
    memory->writed(linear + 2, GUARD_AFTER);
}

void verifyXlatTableByte(U32 linear, U8 value, const char* name) {
    if (memory->readd(linear - 4) != GUARD_BEFORE ||
            memory->readb(linear - 1) != 0x11 ||
            memory->readb(linear) != value ||
            memory->readb(linear + 1) != 0x22 ||
            memory->readd(linear + 2) != GUARD_AFTER) {
        failed("%s memory", name);
    }
}

void runXlatCase(const XlatCase& data, bool big, const char* name) {
    U32 expectedRegs[8];
    XlatExpected expected;
    U32 linear;

    beginInstruction(big);
    emitXlat();
    expected = expectedXlat(data, big, INITIAL_FLAGS);
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    cpu->reg[R_AX].u32 = data.eax;
    cpu->reg[R_BX].u32 = data.ebx;
    expectedRegs[R_AX] = expected.eax;
    expectedRegs[R_BX] = data.ebx;

    linear = cpu->seg[DS].address + xlatOffset(data, big);
    writeXlatTableByte(linear, data.value);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    verifyXlatTableByte(linear, data.value, name);
    verifyFlagsUnchanged(name);
}

void runXlatCases(bool big, const char* name) {
    if (big) {
        for (size_t i = 0; i < caseCount(XLAT32_CASES); ++i) {
            runXlatCase(XLAT32_CASES[i], true, name);
        }
    } else {
        for (size_t i = 0; i < caseCount(XLAT16_CASES); ++i) {
            runXlatCase(XLAT16_CASES[i], false, name);
        }
    }
}

void prepareEnterPreviousFrame(const EnterCase& data, int width) {
    U32 itemSize = width / 8;
    U32 bpIndex = stackOffset(data.bp, width);
    U8 level = data.level & 0x1f;

    for (U32 i = 1; i < level; ++i) {
        writeStackValue((bpIndex - itemSize * i) & stackItemMask(width), 0x51000000 | (width << 8) | i, width);
    }
}

void expectedEnter(const EnterCase& data, int width, U32& expectedSp, U32& expectedBp, StackWrite* writes, size_t& writeCount) {
    U32 itemSize = width / 8;
    U32 mask = stackItemMask(width);
    U32 spIndex = stackOffset(data.sp, width);
    U32 bpIndex = stackOffset(data.bp, width);
    U8 level = data.level & 0x1f;

    writeCount = 0;
    spIndex = (spIndex - itemSize) & mask;
    writes[writeCount++] = {spIndex, data.bp};
    expectedBp = (data.sp & ~mask) | spIndex;

    if (level != 0) {
        for (U32 i = 1; i < level; ++i) {
            spIndex = (spIndex - itemSize) & mask;
            bpIndex = (bpIndex - itemSize) & mask;
            writes[writeCount++] = {spIndex, readStackValue(bpIndex, width)};
        }
        spIndex = (spIndex - itemSize) & mask;
        writes[writeCount++] = {spIndex, expectedBp};
    }

    spIndex = (spIndex - data.bytes) & mask;
    expectedSp = (data.sp & ~mask) | spIndex;
}

void runEnterCase(const EnterCase& data, int width, const char* name) {
    U32 expectedRegs[8];
    StackWrite writes[40];
    size_t writeCount;
    U32 expectedSp;
    U32 expectedBp;
    U32 lowerGuardOffset;
    U32 upperGuardOffset = stackOffset(data.sp, width);

    beginInstruction(width == 32);
    emitEnter(data.bytes, data.level);
    configureStackMode(width);
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    cpu->reg[R_SP].u32 = data.sp;
    cpu->reg[R_BP].u32 = data.bp;
    prepareEnterPreviousFrame(data, width);
    expectedEnter(data, width, expectedSp, expectedBp, writes, writeCount);
    expectedRegs[R_SP] = expectedSp;
    expectedRegs[R_BP] = expectedBp;

    lowerGuardOffset = (stackOffset(expectedSp, width) - 4) & stackItemMask(width);
    memory->writed(stackLinear(lowerGuardOffset), GUARD_BEFORE);
    memory->writed(stackLinear(upperGuardOffset), GUARD_AFTER);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (memory->readd(stackLinear(lowerGuardOffset)) != GUARD_BEFORE ||
            memory->readd(stackLinear(upperGuardOffset)) != GUARD_AFTER) {
        failed("%s stack guard", name);
    }
    for (size_t i = 0; i < writeCount; ++i) {
        U32 expected = width == 16 ? (writes[i].value & 0xffff) : writes[i].value;
        if (readStackValue(writes[i].offset, width) != expected) {
            failed("%s frame write", name);
        }
    }
    verifyFlagsUnchanged(name);
}

void runEnterCases(int width, const EnterCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        runEnterCase(cases[i], width, name);
    }
}

void runLeaveCase(const LeaveCase& data, int width, const char* name) {
    U32 expectedRegs[8];
    U32 itemSize = width / 8;
    U32 bpOffset = stackOffset(data.bp, width);
    U32 expectedSp = width == 16
        ? ((data.bp & 0xffff0000) | ((bpOffset + itemSize) & 0xffff))
        : data.bp + itemSize;
    U32 expectedBp = width == 16
        ? ((data.bp & 0xffff0000) | (data.savedBp & 0xffff))
        : data.savedBp;

    beginInstruction(width == 32);
    emitLeave();
    configureStackMode(width);
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
    cpu->reg[R_SP].u32 = data.sp;
    cpu->reg[R_BP].u32 = data.bp;
    expectedRegs[R_SP] = expectedSp;
    expectedRegs[R_BP] = expectedBp;

    memory->writed(stackLinear((bpOffset - 4) & stackItemMask(width)), GUARD_BEFORE);
    writeStackValue(bpOffset, data.savedBp, width);
    memory->writed(stackLinear((bpOffset + itemSize) & stackItemMask(width)), GUARD_AFTER);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (memory->readd(stackLinear((bpOffset - 4) & stackItemMask(width))) != GUARD_BEFORE ||
            memory->readd(stackLinear((bpOffset + itemSize) & stackItemMask(width))) != GUARD_AFTER) {
        failed("%s stack guard", name);
    }
    verifyFlagsUnchanged(name);
}

void runLeaveCases(int width, const LeaveCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        runLeaveCase(cases[i], width, name);
    }
}

U32 bswapExpected(U32 value) {
    return ((value & 0x000000ff) << 24) |
        ((value & 0x0000ff00) << 8) |
        ((value & 0x00ff0000) >> 8) |
        ((value & 0xff000000) >> 24);
}

void runBswapCase(int reg, U32 value, const char* name) {
    U32 regs[8];
    U32 expectedRegs[8];
    char caseName[64];
    U32 expected = bswapExpected(value);
    U32 resultLinear = cpu->seg[DS].address + BSWAP_RESULT_OFFSET;

    snprintf(caseName, sizeof(caseName), "%s r%d %08x", name, reg, value);
    newInstruction(INITIAL_FLAGS);
    for (int i = 0; i < 8; ++i) {
        regs[i] = REG_GUARD | (0x100 + i);
        expectedRegs[i] = regs[i];
    }
    regs[reg] = value;
    expectedRegs[reg] = expected;

    emitBswap(reg);
    if (reg == R_SP) {
        expectedRegs[R_SP] = 4096;
        memory->writed(resultLinear - 4, GUARD_BEFORE);
        memory->writed(resultLinear, 0);
        memory->writed(resultLinear + 4, GUARD_AFTER);
        emitStoreEspAndRestoreStack(BSWAP_RESULT_OFFSET);
    }

    writeRegs(cpu, regs);
    runTestCPU();
    verifyRegisters(cpu, expectedRegs, caseName);
    verifyFlagsUnchanged(caseName);
    if (reg == R_SP) {
        if (memory->readd(resultLinear - 4) != GUARD_BEFORE ||
                memory->readd(resultLinear) != expected ||
                memory->readd(resultLinear + 4) != GUARD_AFTER) {
            failed("%s memory result", caseName);
        }
    }
}

void runBswapCases(const char* name) {
    for (int reg = R_AX; reg <= R_DI; ++reg) {
        for (size_t i = 0; i < caseCount(BSWAP_CASES); ++i) {
            runBswapCase(reg, BSWAP_CASES[i], name);
        }
    }
}

} // namespace

void testBoundR16M16_0x062() {
    runBoundCases(16, BOUND16_CASES, caseCount(BOUND16_CASES), "bound r16,m16&16");
}

void testBoundR32M32_0x262() {
    runBoundCases(32, BOUND32_CASES, caseCount(BOUND32_CASES), "bound r32,m32&32");
}

void testOpSizePrefix_0x066() {
    runOpSizePrefixCases(false, 32, OP_SIZE32_CASES, caseCount(OP_SIZE32_CASES), "operand size prefix 066");
}

void testOpSizePrefix_0x266() {
    runOpSizePrefixCases(true, 16, OP_SIZE16_CASES, caseCount(OP_SIZE16_CASES), "operand size prefix 266");
}

void testAddressPrefix_0x067() {
    runAddressPrefixCases(false, true, ADDRESS_SIZE8_CASES, caseCount(ADDRESS_SIZE8_CASES), "address prefix 067");
}

void testAddressPrefix_0x267() {
    runAddressPrefixCases(true, false, ADDRESS_SIZE8_CASES, caseCount(ADDRESS_SIZE8_CASES), "address prefix 267");
}

void testMemoryAccess16() {
    runMemoryAccess16Cases("16-bit memory access");
}

void testMemoryAccess32() {
    runMemoryAccess32Cases("32-bit memory access");
}

void testLeaR16M_0x08d() {
    runLea16Cases("lea r16,m 08d");
}

void testLeaR32M_0x28d() {
    runLea32Cases("lea r32,m 28d");
}

void testCbw_0x098() {
    runConvertCases(runCbwCase, "cbw 098");
}

void testCwde_0x298() {
    runConvertCases(runCwdeCase, "cwde 298");
}

void testCwd_0x099() {
    runConvertCases(runCwdCase, "cwd 099");
}

void testCdq_0x299() {
    runConvertCases(runCdqCase, "cdq 299");
}

void testSahf_0x09e() {
    runSahfCases(false, "sahf 09e");
}

void testSahf_0x29e() {
    runSahfCases(true, "sahf 29e");
}

void testLahf_0x09f() {
    runLahfCases(false, "lahf 09f");
}

void testLahf_0x29f() {
    runLahfCases(true, "lahf 29f");
}

void testEnter16_0x0c8() {
    runEnterCases(16, ENTER16_CASES, caseCount(ENTER16_CASES), "enter16 c8");
}

void testEnter32_0x2c8() {
    runEnterCases(32, ENTER32_CASES, caseCount(ENTER32_CASES), "enter32 2c8");
}

void testLeave16_0x0c9() {
    runLeaveCases(16, LEAVE16_CASES, caseCount(LEAVE16_CASES), "leave16 c9");
}

void testLeave32_0x2c9() {
    runLeaveCases(32, LEAVE32_CASES, caseCount(LEAVE32_CASES), "leave32 2c9");
}

void testSalc_0x0d6() {
    runSalcCases(false, "salc 0d6");
}

void testSalc_0x2d6() {
    runSalcCases(true, "salc 2d6");
}

void testXlat_0x0d7() {
    runXlatCases(false, "xlat 0d7");
}

void testXlat_0x2d7() {
    runXlatCases(true, "xlat 2d7");
}

void testCmc_0x0f5() {
    runCmcCases(false, "cmc 0f5");
}

void testCmc_0x2f5() {
    runCmcCases(true, "cmc 2f5");
}

void testClc_0x0f8() {
    runClcCases(false, "clc 0f8");
}

void testClc_0x2f8() {
    runClcCases(true, "clc 2f8");
}

void testStc_0x0f9() {
    runStcCases(false, "stc 0f9");
}

void testStc_0x2f9() {
    runStcCases(true, "stc 2f9");
}

void testBswap_0x3c8_0x3cf() {
    runBswapCases("bswap 3c8-3cf");
}

#endif
