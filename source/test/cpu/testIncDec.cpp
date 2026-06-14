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

#include "testIncDec.h"
#include "testCPU.h"
#include "testX86Util.h"
#include "testAsmJit.h"

#include <vector>

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
constexpr U32 INC_DEC_FLAG_MASK = CF | PF | AF | ZF | SF | OF;
#ifdef BOXEDWINE_MULTI_THREADED
constexpr U32 LOCKED_INC_THREADS = 10;
constexpr U32 LOCKED_INC_ITERATIONS = 1000000;
#endif

enum IncDecOp {
    INC_DEC_INC,
    INC_DEC_DEC
};

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

struct IncDecCase {
    U32 value;
    U32 initialFlags;
};

struct IncDecExpected {
    U32 result;
    U32 flags;
};

enum FlagMode {
    FLAGS_CHECKED,
    FLAGS_OVERWRITTEN
};

const IncDecCase INC8_CASES[] = {
    {0x00, 0},
    {0x00, CF | ZF | SF | OF | AF | PF},
    {0x7f, CF},
    {0x80, 0},
    {0x0f, CF},
    {0xff, CF | OF | AF | SF}
};

const IncDecCase INC16_CASES[] = {
    {0x0000, 0},
    {0x0000, CF | ZF | SF | OF | AF | PF},
    {0x7fff, CF},
    {0x8000, 0},
    {0x00ff, CF},
    {0xffff, CF | OF | AF | SF}
};

const IncDecCase INC32_CASES[] = {
    {0x00000000, 0},
    {0x00000000, CF | ZF | SF | OF | AF | PF},
    {0x7fffffff, CF},
    {0x80000000, 0},
    {0x00000fff, CF},
    {0xffffffff, CF | OF | AF | SF}
};

const IncDecCase DEC8_CASES[] = {
    {0x02, 0},
    {0x01, CF | ZF | SF | OF | AF | PF},
    {0x80, CF},
    {0x00, 0},
    {0x10, CF},
    {0x7f, CF | OF | AF | SF}
};

const IncDecCase DEC16_CASES[] = {
    {0x0002, 0},
    {0x0001, CF | ZF | SF | OF | AF | PF},
    {0x8000, CF},
    {0x0000, 0},
    {0x0100, CF},
    {0x7fff, CF | OF | AF | SF}
};

const IncDecCase DEC32_CASES[] = {
    {0x00000002, 0},
    {0x00000001, CF | ZF | SF | OF | AF | PF},
    {0x80000000, CF},
    {0x00000000, 0},
    {0x00001000, CF},
    {0x7fffffff, CF | OF | AF | SF}
};

template <typename T, size_t N>
size_t caseCount(const T(&)[N]) {
    return N;
}

U32 flagsForResult(U32 result, int width) {
    U32 flags = 0;
    U32 sign = signBit(width);
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

IncDecExpected calculatedExpected(IncDecOp op, U32 value, int width, U32 initialFlags) {
    U32 mask = widthMask(width);
    U32 sign = signBit(width);
    value &= mask;
    U32 result = op == INC_DEC_INC ? (value + 1) & mask : (value - 1) & mask;
    U32 flags = initialFlags & CF;

    if (op == INC_DEC_INC) {
        if (value == sign - 1) {
            flags |= OF;
        }
        if ((value & 0x0f) == 0x0f) {
            flags |= AF;
        }
    } else {
        if (value == sign) {
            flags |= OF;
        }
        if ((value & 0x0f) == 0) {
            flags |= AF;
        }
    }
    flags |= flagsForResult(result, width);
    return {result, flags};
}

const char* opName(IncDecOp op) {
    return op == INC_DEC_INC ? "inc" : "dec";
}

#if defined(_MSC_VER) && defined(_M_IX86)
#define TEST_BINARY_HAS_HARDWARE_ORACLE 1

IncDecExpected hardwareExpected(IncDecOp op, U32 value, int width, U32 initialFlags) {
    U32 result = 0;
    U32 flags = 0;

    if (op == INC_DEC_INC) {
        if (width == 8) {
            if (initialFlags & CF) {
                __asm {
                    mov eax, value
                    stc
                    inc al
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            } else {
                __asm {
                    mov eax, value
                    clc
                    inc al
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            }
        } else if (width == 16) {
            if (initialFlags & CF) {
                __asm {
                    mov eax, value
                    stc
                    inc ax
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            } else {
                __asm {
                    mov eax, value
                    clc
                    inc ax
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            }
        } else {
            if (initialFlags & CF) {
                __asm {
                    mov eax, value
                    stc
                    inc eax
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            } else {
                __asm {
                    mov eax, value
                    clc
                    inc eax
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            }
        }
    } else {
        if (width == 8) {
            if (initialFlags & CF) {
                __asm {
                    mov eax, value
                    stc
                    dec al
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            } else {
                __asm {
                    mov eax, value
                    clc
                    dec al
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            }
        } else if (width == 16) {
            if (initialFlags & CF) {
                __asm {
                    mov eax, value
                    stc
                    dec ax
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            } else {
                __asm {
                    mov eax, value
                    clc
                    dec ax
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            }
        } else {
            if (initialFlags & CF) {
                __asm {
                    mov eax, value
                    stc
                    dec eax
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            } else {
                __asm {
                    mov eax, value
                    clc
                    dec eax
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            }
        }
    }

    return {result & widthMask(width), flags & INC_DEC_FLAG_MASK};
}
#else
#define TEST_BINARY_HAS_HARDWARE_ORACLE 0
#endif

IncDecExpected expected(IncDecOp op, U32 value, int width, U32 initialFlags) {
#if TEST_BINARY_HAS_HARDWARE_ORACLE
    IncDecExpected hardware = hardwareExpected(op, value, width, initialFlags);
    IncDecExpected calculated = calculatedExpected(op, value, width, initialFlags);
    if (hardware.result != calculated.result || (hardware.flags & INC_DEC_FLAG_MASK) != (calculated.flags & INC_DEC_FLAG_MASK)) {
        failed("hardware %s oracle mismatch", opName(op));
    }
    return hardware;
#else
    return calculatedExpected(op, value, width, initialFlags);
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
        failed("asmjit inc/dec code init failed");
    }
}

void emitReg(IncDecOp op, asmjit::x86::Gp reg) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Error err = op == INC_DEC_INC ? a.inc(reg) : a.dec(reg);
    if (err != asmjit::Error::kOk) {
        failed("asmjit %s reg failed", opName(op));
    }
    pushGeneratedCode(code);
}

void emitMem(IncDecOp op, const asmjit::x86::Mem& mem, bool lockPrefix) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (lockPrefix) {
        a.lock();
    }
    asmjit::Error err = op == INC_DEC_INC ? a.inc(mem) : a.dec(mem);
    if (err != asmjit::Error::kOk) {
        failed("asmjit %s mem failed", opName(op));
    }
    pushGeneratedCode(code);
}

void overwriteFlagsIfNeeded(FlagMode flagMode) {
    if (flagMode == FLAGS_OVERWRITTEN) {
        pushCode8(0x39);
        pushCode8(0xc0); // cmp eax, eax
    }
}

void initExpectedRegs(U32* expectedRegs) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }
}

void verifyFlags(IncDecOp op, U32 value, int width, U32 initialFlags, const char* name) {
    IncDecExpected e = expected(op, value, width, initialFlags);
    if (((actualFlags(cpu) ^ e.flags) & INC_DEC_FLAG_MASK) != 0) {
        failed("%s flags", name);
    }
}

void beginInstruction(U32 flags) {
    newInstruction(flags);
    cpu->big = true;
}

void runRegisterCase(IncDecOp op, int reg, int width, const IncDecCase& data, FlagMode flagMode, const char* name) {
    U32 expectedRegs[8];
    U32 value;

    beginInstruction(data.initialFlags);
    emitReg(op, regForWidth(reg, width));
    overwriteFlagsIfNeeded(flagMode);
    initExpectedRegs(expectedRegs);
    setRegValue(cpu, reg, width, data.value);
    applyRegValue(expectedRegs, reg, width, data.value);
    value = getRegValue(cpu, reg, width);
    applyRegValue(expectedRegs, reg, width, expected(op, value, width, data.initialFlags).result);

    runTestCPU();
    verifyRegisters(cpu, expectedRegs, name);
    if (flagMode == FLAGS_CHECKED) {
        verifyFlags(op, value, width, data.initialFlags, name);
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

void runPreparedMemoryCase(IncDecOp op, int width, U32 linearAddress, const IncDecCase& data, FlagMode flagMode, const char* name) {
    U32 expectedRegs[8];
    IncDecExpected e = expected(op, data.value, width, data.initialFlags);

    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }

    prepareMemTarget(linearAddress, data.value, width);
    runTestCPU();
    verifyMemTarget(linearAddress, e.result, width, name);
    verifyRegisters(cpu, expectedRegs, name);
    if (flagMode == FLAGS_CHECKED) {
        verifyFlags(op, data.value, width, data.initialFlags, name);
    }
}

void runBaseMemoryCases(IncDecOp op, int width, const IncDecCase& data, bool lockPrefix, FlagMode flagMode, const char* name) {
    for (int base = 0; base < 8; ++base) {
        if (!testRunMemoryBase(base)) {
            continue;
        }
        U32 regs[8];
        initAddressRegisters(regs);
        regs[base] = MEM_BASE + 0x1000 + base * 0x80;

        if (base != R_BP && testRunMemoryBaseDisplacement(base, 0)) {
            beginInstruction(data.initialFlags);
            emitMem(op, memPtr(reg32(base), 0, width), lockPrefix);
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedMemoryCase(op, width, segmentBaseForAddressReg(base) + regs[base], data, flagMode, name);
        }

        if (testRunMemoryBaseDisplacement(base, 1)) {
            beginInstruction(data.initialFlags);
            emitMem(op, memPtr(reg32(base), 0x11, width), lockPrefix);
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedMemoryCase(op, width, segmentBaseForAddressReg(base) + regs[base] + 0x11, data, flagMode, name);
        }

        if (testRunMemoryBaseDisplacement(base, 2)) {
            beginInstruction(data.initialFlags);
            emitMem(op, memPtr(reg32(base), 0x123, width), lockPrefix);
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedMemoryCase(op, width, segmentBaseForAddressReg(base) + regs[base] + 0x123, data, flagMode, name);
        }
    }
}

void runAbsoluteMemoryCases(IncDecOp op, int width, const IncDecCase& data, bool lockPrefix, FlagMode flagMode, const char* name) {
    U32 regs[8];
    U32 offset = MEM_BASE + 0x3000;
    initAddressRegisters(regs);

    beginInstruction(data.initialFlags);
    emitMem(op, memPtr(offset, width), lockPrefix);
    overwriteFlagsIfNeeded(flagMode);
    writeRegs(cpu, regs);
    runPreparedMemoryCase(op, width, cpu->seg[DS].address + offset, data, flagMode, name);
}

void runSibMemoryCases(IncDecOp op, int width, const IncDecCase& data, bool lockPrefix, FlagMode flagMode, const char* name) {
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

                S32 disp = (S32)(targetOffset - regs[base] - (regs[index] << shift));
                beginInstruction(data.initialFlags);
                emitMem(op, memPtr(reg32(base), reg32(index), shift, disp, width), lockPrefix);
                overwriteFlagsIfNeeded(flagMode);
                writeRegs(cpu, regs);
                runPreparedMemoryCase(op, width, segmentBaseForAddressReg(base) + targetOffset, data, flagMode, name);
            }
        }
    }
}

void runRegisterCases(IncDecOp op, int width, const IncDecCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int flagMode = FLAGS_CHECKED; flagMode <= FLAGS_OVERWRITTEN; ++flagMode) {
            for (int reg = 0; reg < 8; ++reg) {
                if (!testRunRegister(reg)) {
                    continue;
                }
                runRegisterCase(op, reg, width, cases[i], (FlagMode)flagMode, name);
            }
        }
    }
}

void runMemoryCases(IncDecOp op, int width, const IncDecCase* cases, size_t count, const char* name) {
    for (size_t i = 0; i < count; ++i) {
        for (int flagMode = FLAGS_CHECKED; flagMode <= FLAGS_OVERWRITTEN; ++flagMode) {
            for (int lockPrefix = 0; lockPrefix < 2; ++lockPrefix) {
                runBaseMemoryCases(op, width, cases[i], lockPrefix != 0, (FlagMode)flagMode, name);
                runAbsoluteMemoryCases(op, width, cases[i], lockPrefix != 0, (FlagMode)flagMode, name);
                runSibMemoryCases(op, width, cases[i], lockPrefix != 0, (FlagMode)flagMode, name);
            }
        }
    }
}

#ifdef BOXEDWINE_MULTI_THREADED
#undef cpu
#undef memory
void emitLockedIncLoop(U32 address) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Label loop = a.new_label();

    if (a.bind(loop) != asmjit::Error::kOk) {
        failed("asmjit locked inc loop bind failed");
    }
    a.lock();
    if (a.inc(asmjit::x86::dword_ptr(address)) != asmjit::Error::kOk) {
        failed("asmjit locked inc failed");
    }
    if (a.loop(loop) != asmjit::Error::kOk) {
        failed("asmjit locked inc loop failed");
    }
    pushGeneratedCode(code);
    pushCode8(0xcd);
    pushCode8(0x97);
}

KThread* startLockedIncThread(U32 iterations) {
    KThread* thread = testContext().process->createThread();
    thread->cpu->clone(testContext().cpu);
    thread->cpu->reg[R_CX].u32 = iterations;
    scheduleThread(thread);
    return thread;
}

void runLockedIncCase(U32 address, const char* name) {
    newInstruction(0);
    CPU* baseCpu = testContext().cpu;
    KMemory* baseMemory = testContext().memory;
    baseMemory->writed(baseCpu->seg[DS].address + address, 0);
    emitLockedIncLoop(address);

    std::vector<KThread*> threads;
    for (U32 i = 0; i < LOCKED_INC_THREADS; ++i) {
        threads.push_back(startLockedIncThread(LOCKED_INC_ITERATIONS));
    }
    for (U32 i = 0; i < LOCKED_INC_THREADS; ++i) {
        joinThread(threads[i]);
    }
    KThread::setCurrentThread(testContext().thread);

    U32 value = baseMemory->readd(baseCpu->seg[DS].address + address);
    U32 expectedValue = LOCKED_INC_THREADS * LOCKED_INC_ITERATIONS;
    if (value != expectedValue) {
        failed("%s expected %u, got %u", name, expectedValue, value);
    }
}
#define cpu (testContext().cpu)
#define memory (testContext().memory)
#endif

} // namespace

void testIncR16_0x040() {
    runRegisterCases(INC_DEC_INC, 16, INC16_CASES, caseCount(INC16_CASES), "inc r16");
}

void testIncR32_0x240() {
    runRegisterCases(INC_DEC_INC, 32, INC32_CASES, caseCount(INC32_CASES), "inc r32");
}

void testDecR16_0x048() {
    runRegisterCases(INC_DEC_DEC, 16, DEC16_CASES, caseCount(DEC16_CASES), "dec r16");
}

void testDecR32_0x248() {
    runRegisterCases(INC_DEC_DEC, 32, DEC32_CASES, caseCount(DEC32_CASES), "dec r32");
}

void testIncR8_0x0fe() {
    runRegisterCases(INC_DEC_INC, 8, INC8_CASES, caseCount(INC8_CASES), "inc r8");
}

void testIncE8_0x0fe() {
    runMemoryCases(INC_DEC_INC, 8, INC8_CASES, caseCount(INC8_CASES), "inc m8");
}

void testDecR8_0x0fe() {
    runRegisterCases(INC_DEC_DEC, 8, DEC8_CASES, caseCount(DEC8_CASES), "dec r8");
}

void testDecE8_0x0fe() {
    runMemoryCases(INC_DEC_DEC, 8, DEC8_CASES, caseCount(DEC8_CASES), "dec m8");
}

void testIncE16_0x0ff() {
    runMemoryCases(INC_DEC_INC, 16, INC16_CASES, caseCount(INC16_CASES), "inc m16");
}

void testIncE32_0x2ff() {
    runMemoryCases(INC_DEC_INC, 32, INC32_CASES, caseCount(INC32_CASES), "inc m32");
}

void testDecE16_0x0ff() {
    runMemoryCases(INC_DEC_DEC, 16, DEC16_CASES, caseCount(DEC16_CASES), "dec m16");
}

void testDecE32_0x2ff() {
    runMemoryCases(INC_DEC_DEC, 32, DEC32_CASES, caseCount(DEC32_CASES), "dec m32");
}

#ifdef BOXEDWINE_MULTI_THREADED
void testLockedInc() {
    runLockedIncCase(0xc8, "locked inc aligned 8");
    runLockedIncCase(0xc4, "locked inc aligned 4");
    runLockedIncCase(0xc9, "locked inc aligned 1");
}
#endif

#endif
