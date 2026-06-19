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

#include "testSelfModifying.h"
#include "../cpu/testCPU.h"
#include "../cpu/testAsmJit.h"

#define cpu (testContext().cpu)
#define memory (testContext().memory)
#define pushCode8 testPushCode8
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

using namespace asmjit;
using namespace asmjit::x86;

void initCode(CodeHolder& code) {
    Environment env(Arch::kX86);
    if (code.init(env) != Error::kOk) {
        failed("asmjit self modifying code init failed");
    }
}

void check(Error err, const char* name) {
    if (err != Error::kOk) {
        failed("asmjit self modifying emit failed: %s", name);
    }
}

void pushGeneratedCode(const CodeHolder& code) {
    const CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        pushCode8(buffer.data()[i]);
    }
}

void writeGeneratedCode(U32 linearAddress, const CodeHolder& code) {
    const CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        memory->writeb(linearAddress + (U32)i, buffer.data()[i]);
    }
}

Mem csBytePtr(U32 offset) {
    Mem result = byte_ptr(offset);
    result.set_segment(cs);
    return result;
}

void verifyReg32(int reg, U32 expected, const char* name) {
    if (cpu->reg[reg].u32 != expected) {
        failed("%s register value", name);
    }
}

void emitSubEax5At(U32 linearAddress) {
    CodeHolder code;
    initCode(code);
    Assembler a(&code);

    check(a.short_().sub(eax, 0x05), "sub eax, 5");
    writeGeneratedCode(linearAddress, code);
}

void emitSelfModifying() {
    CodeHolder code;
    initCode(code);
    Assembler a(&code);
    Label start = a.new_label();
    Label done = a.new_label();

    check(a.bind(start), "bind start");
    check(a.short_().add(eax, 0x20), "add eax, 20h");
    check(a.test(ecx, ecx), "test ecx, ecx");
    check(a.short_().jnz(done), "jnz done");
    check(a.inc(ecx), "inc ecx");
    check(a.mov(csBytePtr(0x2), 0x40), "mov cs:[2], 40h");
    check(a.short_().jmp(start), "jmp start");
    check(a.bind(done), "bind done");

    pushGeneratedCode(code);
}

void emitSelfModifyingMovsb() {
    CodeHolder code;
    initCode(code);
    Assembler a(&code);
    Label start = a.new_label();
    Label done = a.new_label();

    check(a.bind(start), "bind start");
    check(a.short_().add(eax, 0x20), "add eax, 20h");
    check(a.test(edx, edx), "test edx, edx");
    check(a.short_().jnz(done), "jnz done");
    check(a.inc(edx), "inc edx");
    check(a.db(0xf3), "rep prefix");
    check(a.db(0x2e), "cs prefix");
    check(a.movsb(), "movsb");
    check(a.short_().jmp(start), "jmp start");
    check(a.bind(done), "bind done");

    pushGeneratedCode(code);
}

void emitSelfModifyingFront() {
    CodeHolder code;
    initCode(code);
    Assembler a(&code);
    Label start = a.new_label();
    Label done = a.new_label();

    check(a.bind(start), "bind start");
    check(a.short_().add(eax, 0x20), "add eax, 20h");
    check(a.mov(csBytePtr(0x2), 0x40), "mov cs:[2], 40h");
    check(a.test(ecx, ecx), "test ecx, ecx");
    check(a.short_().jnz(done), "jnz done");
    check(a.inc(ecx), "inc ecx");
    check(a.short_().jmp(start), "jmp start");
    check(a.bind(done), "bind done");

    pushGeneratedCode(code);
}

void emitSelfModifyingBack() {
    CodeHolder code;
    initCode(code);
    Assembler a(&code);

    check(a.short_().add(eax, 0x20), "add eax, 20h");
    check(a.mov(csBytePtr(0x0d), 0x40), "mov cs:[0dh], 40h");
    check(a.short_().add(eax, 0x20), "add eax, 20h");

    pushGeneratedCode(code);
}

} // namespace

void testSelfModifying() {
    newInstruction(0);
    emitSelfModifying();

    runTestCPU();
    verifyReg32(1, 1, "self modifying code ecx");
    verifyReg32(0, 0x60, "self modifying code eax");
}

void testSelfModifyingMovsb() {
    newInstruction(0);

    cpu->reg[7].u32 = 0;
    cpu->reg[6].u32 = 512;
    cpu->reg[1].u32 = 3;

    // Code copied over the first instruction: sub eax, 0x05.
    emitSubEax5At(TEST_CODE_ADDRESS + 512);

    cpu->setSeg(ES, TEST_CODE_ADDRESS, 1);
    emitSelfModifyingMovsb();

    runTestCPU();
    verifyReg32(2, 1, "self modifying movsb edx");
    verifyReg32(0, 0x1b, "self modifying movsb eax");
}

void testSelfModifyingFront() {
    newInstruction(0);
    emitSelfModifyingFront();

    runTestCPU();
    verifyReg32(1, 1, "self modifying front ecx");
    verifyReg32(0, 0x60, "self modifying front eax");
}

void testSelfModifyingBack() {
    newInstruction(0);
    emitSelfModifyingBack();

    runTestCPU();
    verifyReg32(0, 0x60, "self modifying back eax");
}

void testSelfModifyingWriteProtectPageIsDynamic() {
#ifdef BOXEDWINE_JIT
    newInstruction(0);
    pushCode8(0x40); // inc eax
    runTestCPU();

    DecodedOp* first = memory->getDecodedOp(TEST_CODE_ADDRESS);
    if (!first) {
        failed("self modifying write-protect dynamic initial decode");
        return;
    }

    if (memory->mprotect(cpu->thread, TEST_CODE_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE) != 0) {
        failed("self modifying write-protect dynamic writable mprotect");
        return;
    }
    memory->writeb(TEST_CODE_ADDRESS, 0x43); // inc ebx
    if (memory->mprotect(cpu->thread, TEST_CODE_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_EXEC) != 0) {
        failed("self modifying write-protect dynamic executable mprotect");
        return;
    }

    cpu->eip.u32 = 0;
    cpu->nextOp = nullptr;
    DecodedOp* patched = cpu->getNextOp();
    if (!patched) {
        failed("self modifying write-protect dynamic patched decode");
        return;
    }
    if (!(patched->flags & OP_FLAG_NO_JIT)) {
        failed("self modifying write-protect dynamic page disables jit");
    }
    memory->mprotect(cpu->thread, TEST_CODE_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC);
#endif
}

void testSelfModifyingUndecodedWriteIsDynamic() {
#ifdef BOXEDWINE_JIT
    newInstruction(0);
    pushCode8(0x40); // inc eax

    memory->removeCode(cpu->thread, TEST_CODE_ADDRESS, 1, true);

    DecodedOp* op = cpu->getNextOp();
    if (!op) {
        failed("self modifying undecoded write dynamic decode");
        return;
    }
    if (!(op->flags & OP_FLAG_NO_JIT)) {
        failed("self modifying undecoded write disables jit");
    }
#endif
}

void testSelfModifyingWritableToExecutablePageIsDynamic() {
#ifdef BOXEDWINE_JIT
    newInstruction(0);

    memory->unmap(TEST_CODE_ADDRESS, K_PAGE_SIZE);
    U32 mapped = memory->mmap(cpu->thread, TEST_CODE_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
    if (mapped != TEST_CODE_ADDRESS) {
        failed("self modifying writable-to-executable mmap");
        return;
    }

    memory->writeb(TEST_CODE_ADDRESS, 0x40);     // inc eax
    memory->writeb(TEST_CODE_ADDRESS + 1, 0xcd); // test end
    memory->writeb(TEST_CODE_ADDRESS + 2, 0x97);

    if (memory->mprotect(cpu->thread, TEST_CODE_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_EXEC) != 0) {
        failed("self modifying writable-to-executable mprotect");
        return;
    }

    cpu->eip.u32 = 0;
    cpu->nextOp = nullptr;
    DecodedOp* op = cpu->getNextOp();
    if (!op) {
        failed("self modifying writable-to-executable decode");
        return;
    }
    if (!(op->flags & OP_FLAG_NO_JIT)) {
        failed("self modifying writable-to-executable page disables jit");
    }

    memory->unmap(TEST_CODE_ADDRESS, K_PAGE_SIZE);
    memory->mmap(cpu->thread, TEST_CODE_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC, K_MAP_FIXED | K_MAP_PRIVATE, -1, 0);
#endif
}

#endif
