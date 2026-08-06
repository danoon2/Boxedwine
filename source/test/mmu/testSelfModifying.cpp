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

#ifdef BOXEDWINE_MULTI_THREADED
#undef cpu
#undef memory
void testDecodedOpInvalidationDefersCrossThreadReuse() {
    TestContext& context = testContext();
    CPU* cpu = context.cpu;
    KMemory* memory = context.memory;
    newInstruction(0);

    // Primary block: nop; test-end.
    pushCode8(0x90);
    pushCode8(0xcd);
    pushCode8(0x97);

    // Leave a second block uncached so decoding it can trigger reclamation
    // without retiring another op into the first batch.
    constexpr U32 secondBlockOffset = 0x100;
    memory->writeb(TEST_CODE_ADDRESS + secondBlockOffset, 0x90);
    memory->writeb(TEST_CODE_ADDRESS + secondBlockOffset + 1, 0xcd);
    memory->writeb(TEST_CODE_ADDRESS + secondBlockOffset + 2, 0x97);

    auto crossDispatchBoundary = [](CPU* boundaryCpu) {
        boundaryCpu->nextOp = nullptr;
        U32 observedEpoch = boundaryCpu->decodedOpCacheGlobalEpoch->load(std::memory_order_acquire);
        boundaryCpu->memory->synchronizeDecodedOpCache(boundaryCpu, observedEpoch);
    };

    crossDispatchBoundary(cpu);
    DecodedOp* oldOp = cpu->getNextOp();
    DecodedOp* oldNext = oldOp->next;
    OpCallback oldPfn = oldOp->pfn;

    KThread* holdingThread = context.process->createThread();
    crossDispatchBoundary(holdingThread->cpu);
    holdingThread->cpu->nextOp = oldOp;

    // Turn the nop into inc eax. Both CPUs still announce the generation in
    // which oldOp was reachable, so it must remain intact.
    memory->writeb(TEST_CODE_ADDRESS, 0x40);
    crossDispatchBoundary(cpu);
    DecodedOp* replacement = cpu->getNextOp();
    if (replacement == oldOp) {
        failed("invalidated DecodedOp was reused while another CPU held it");
    }
    if (oldOp->next != oldNext || oldOp->pfn != oldPfn) {
        failed("invalidated DecodedOp was reset while another CPU held it");
    }

    crossDispatchBoundary(holdingThread->cpu);

    // Adding an unrelated block runs reclamation. Now that both CPUs crossed
    // the boundary, oldOp should be reset and returned to the pool.
    cpu->eip.u32 = secondBlockOffset;
    cpu->getNextOp();
    if (oldOp->next || oldOp->pfn) {
        failed("invalidated DecodedOp was not reclaimed after all CPUs advanced");
    }

    cpu->eip.u32 = 0;
    cpu->nextOp = replacement;
    context.process->deleteThread(holdingThread);
}
#endif

#endif
