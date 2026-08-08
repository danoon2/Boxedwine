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
#include "ksignal.h"
#include "../../emulation/softmmu/kmemory_soft.h"
#include "../../emulation/softmmu/soft_ram.h"

#define cpu (testContext().cpu)
#define testMemory (testContext().memory)
#define pushCode8 testPushCode8
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

U8* getLinearAliasAddress(KMemoryData* data, U32 address) {
    return data->linearMemoryBase + address;
}

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
        testMemory->writeb(linearAddress + (U32)i, buffer.data()[i]);
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

void verifyLinearAlias(KMemory* targetMemory, U32 address, U32 expected, const char* name) {
    KMemoryData* data = getMemData(targetMemory);
    if (!data->linearMemoryBase) {
        failed("%s missing linear memory aperture", name);
        return;
    }
    if (!data->linearMemoryMappings[address >> K_PAGE_SHIFT]) {
        if (ramPageLinearMemoryPageCount() == 1) {
            failed("%s missing linear memory mapping", name);
        }
        return;
    }
    U32 actual = *(volatile U32*)getLinearAliasAddress(data, address);
    if (actual != expected) {
        failed("%s alias value expected %x, got %x", name, expected, actual);
    }
}

void runLinearMemoryFaultCase(bool writeFault) {
    constexpr U32 writableOffset = 0x1000;
    constexpr U32 unmappedOffset = 0x01000000;
    constexpr U32 initialValue = 0x12345678;

    newInstruction(0);
    U32 offset = writeFault ? writableOffset : unmappedOffset;
    U32 targetAddress = TEST_HEAP_ADDRESS + offset;
    if (writeFault) {
        testMemory->writed(targetAddress, initialValue);
        if (testMemory->mprotect(testContext().thread, targetAddress & ~K_PAGE_MASK,
                K_PAGE_SIZE, K_PROT_READ)) {
            failed("linear memory could not protect fault destination");
            return;
        }
        cpu->reg[0].u32 = 0x89abcdef;
        pushCode8(0xa3); // mov [moffs32], eax
    } else {
        testMemory->unmap(targetAddress & ~K_PAGE_MASK, K_PAGE_SIZE);
        pushCode8(0xa1); // mov eax, [moffs32]
    }
    testPushCode32(offset);

    KSigAction& action = testContext().process->sigActions[K_SIGSEGV];
    action.reset();
    action.handlerAndSigAction = TEST_CODE_ADDRESS + 5;
    action.flags = 0;

    runTestCPU();

    if (writeFault) {
        testMemory->mprotect(testContext().thread, targetAddress & ~K_PAGE_MASK,
            K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
    }
    if (action.sigInfo[0] != K_SIGSEGV) {
        failed("linear memory %s did not raise SIGSEGV", writeFault ? "write fault" : "unmapped read");
    }
    if (action.sigInfo[3] != targetAddress) {
        failed("linear memory %s reported address %x instead of %x",
            writeFault ? "write fault" : "unmapped read", action.sigInfo[3], targetAddress);
    }
    if (writeFault && testMemory->readd(targetAddress) != initialValue) {
        failed("linear memory write fault changed protected memory");
    }
    action.reset();
}

void runLinearMemoryRepeatedFirstTouchCase() {
    if (ramPageLinearMemoryPageCount() != 1) {
        return;
    }
    constexpr U32 targetAddress = 0x20000000;
    constexpr U32 pageCount = JIT_RUN_COUNT + LINEAR_MEMORY_RECOMPILE_FAULTS + 4;
    constexpr U32 value = 0x12345678;

    newInstruction(0);
    U32 mapped = testMemory->mmap(testContext().thread, targetAddress,
        pageCount * K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_FIXED | K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0);
    if (mapped != targetAddress) {
        failed("linear memory repeated-first-touch mmap failed: %x", mapped);
        return;
    }

    cpu->reg[0].u32 = targetAddress - cpu->seg[DS].address;
    cpu->reg[1].u32 = pageCount;
    cpu->reg[2].u32 = value;

    CodeHolder code;
    initCode(code);
    Assembler a(&code);
    Label loop = a.new_label();
    check(a.bind(loop), "bind repeated first-touch loop");
    check(a.mov(dword_ptr(eax), edx), "mov repeated first-touch page");
    check(a.add(eax, K_PAGE_SIZE), "advance repeated first-touch page");
    check(a.dec(ecx), "count repeated first-touch pages");
    check(a.short_().jnz(loop), "repeat first-touch page loop");
    pushGeneratedCode(code);

    runTestCPU();

#ifdef BOXEDWINE_HOST_EXCEPTIONS
    DecodedOp* store = testMemory->getDecodedOp(TEST_CODE_ADDRESS);
    if (!store || store->inst != MovE32R32) {
        failed("linear memory repeated-first-touch store was not decoded");
    } else if (store->exceptionCount != MAX_OP_EXCEPTION_COUNT) {
        failed("linear memory repeated-first-touch store retained exception path: %u",
            (U32)store->exceptionCount);
    }
#endif
    if (testMemory->readd(targetAddress) != value ||
            testMemory->readd(targetAddress + (pageCount - 1) * K_PAGE_SIZE) != value) {
        failed("linear memory repeated-first-touch loop did not write all pages");
    }
    testMemory->unmap(targetAddress, pageCount * K_PAGE_SIZE);
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

void testLinearMemoryAliasAndFaults() {
    if (!ramPageUseLinearMemory()) {
        return;
    }

    constexpr U32 address = TEST_HEAP_ADDRESS + 0x100;
    constexpr U32 firstValue = 0x12345678;
    constexpr U32 secondValue = 0x89abcdef;

    newInstruction(0);
    testMemory->writed(address, firstValue);
    verifyLinearAlias(testMemory, address, firstValue, "linear memory normal write");

    KMemoryData* data = getMemData(testMemory);
    if (data->linearMemoryMappings[address >> K_PAGE_SHIFT]) {
        *(volatile U32*)getLinearAliasAddress(data, address) = secondValue;
    } else {
        failed("linear memory grouped allocation was not installed in the aperture");
        return;
    }
    if (testMemory->readd(address) != secondValue) {
        failed("linear memory alias write was not visible through the soft MMU");
    }

    runLinearMemoryFaultCase(true);
    runLinearMemoryFaultCase(false);
    runLinearMemoryRepeatedFirstTouchCase();

    U32 groupPageCount = ramPageLinearMemoryPageCount();
    if (groupPageCount > 1) {
        U32 firstMapping = testMemory->mmap(testContext().thread, 0, K_PAGE_SIZE,
            K_PROT_READ | K_PROT_WRITE, K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0);
        U32 secondMapping = testMemory->mmap(testContext().thread, 0, K_PAGE_SIZE,
            K_PROT_READ | K_PROT_WRITE, K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0);
        U32 alignmentMask = groupPageCount * K_PAGE_SIZE - 1;
        bool firstFailed = firstMapping >= (U32)-4095;
        bool secondFailed = secondMapping >= (U32)-4095;
        if (firstFailed || secondFailed) {
            failed("linear memory aligned mmap placement failed: %x %x", firstMapping, secondMapping);
        } else if ((firstMapping & alignmentMask) || (secondMapping & alignmentMask)) {
            failed("linear memory mmap placement was not %u-byte aligned: %x %x",
                groupPageCount * K_PAGE_SIZE, firstMapping, secondMapping);
        }
        if (!firstFailed) {
            testMemory->unmap(firstMapping, K_PAGE_SIZE);
        }
        if (!secondFailed) {
            testMemory->unmap(secondMapping, K_PAGE_SIZE);
        }
    }
}

void testLinearMemoryCloneMappings() {
    if (!ramPageUseLinearMemory()) {
        return;
    }

    constexpr U32 privateAddress = TEST_HEAP_ADDRESS + 0x2000;
    constexpr U32 sharedAddress = 0xc1000000;
    constexpr U32 generationAddress = 0xc2000000;
    constexpr U32 parentPrivateValue = 0x11112222;
    constexpr U32 childPrivateValue = 0x33334444;
    constexpr U32 parentSharedValue = 0x55556666;
    constexpr U32 childSharedValue = 0x77778888;
    constexpr U32 generationOldValue = 0x13572468;
    constexpr U32 generationNewValue = 0x24681357;
    U32 sharedLength = ramPageLinearMemoryPageCount() * K_PAGE_SIZE;

    newInstruction(0);
    U32 mapped = testMemory->mmap(testContext().thread, sharedAddress, sharedLength,
        K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_SHARED | K_MAP_ANONYMOUS, -1, 0);
    if (mapped != sharedAddress) {
        failed("linear memory shared mapping failed: %x", mapped);
        return;
    }
    testMemory->writed(privateAddress, parentPrivateValue);
    testMemory->writed(sharedAddress, parentSharedValue);
    if (ramPageUseLinearMemoryBacking()) {
        mapped = testMemory->mmap(testContext().thread, generationAddress, sharedLength,
            K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0);
        if (mapped != generationAddress) {
            failed("linear memory backing-generation mapping failed: %x", mapped);
            return;
        }
        testMemory->writed(generationAddress, generationOldValue);
    }

    KProcessPtr cloneProcess = KProcess::create();
    KMemory* cloneMemory = KMemory::create(cloneProcess.get());
    cloneProcess->memory = cloneMemory;
    cloneMemory->clone(testMemory, false);
    KThread* cloneThread = cloneProcess->createThread();

    verifyLinearAlias(testMemory, privateAddress, parentPrivateValue, "linear memory parent COW mapping");
    verifyLinearAlias(cloneMemory, privateAddress, parentPrivateValue, "linear memory child COW mapping");
    verifyLinearAlias(cloneMemory, sharedAddress, parentSharedValue, "linear memory child shared mapping");

    if (ramPageUseLinearMemoryBacking()) {
        if (testMemory->unmap(generationAddress, sharedLength)) {
            failed("linear memory backing-generation unmap failed");
        }
        mapped = testMemory->mmap(testContext().thread, generationAddress, sharedLength,
            K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE | K_MAP_ANONYMOUS, -1, 0);
        if (mapped != generationAddress) {
            failed("linear memory backing-generation remap failed: %x", mapped);
        } else {
            testMemory->writed(generationAddress, generationNewValue);
            if (testMemory->readd(generationAddress) != generationNewValue ||
                    cloneMemory->readd(generationAddress) != generationOldValue) {
                failed("linear memory backing generation did not preserve fork isolation");
            }
            verifyLinearAlias(testMemory, generationAddress, generationNewValue,
                "linear memory parent remapped backing generation");
            verifyLinearAlias(cloneMemory, generationAddress, generationOldValue,
                "linear memory child old backing generation");
        }
    }

    KThread* parentThread = testContext().thread;
    KThread::setCurrentThread(cloneThread);
    cloneMemory->writed(privateAddress, childPrivateValue);
    cloneMemory->writed(sharedAddress, childSharedValue);
    KThread::setCurrentThread(parentThread);

    if (ramPageLinearMemoryPageCount() > 1) {
        KMemoryData* cloneData = getMemData(cloneMemory);
        U32 firstPage = (privateAddress >> K_PAGE_SHIFT) & ~(ramPageLinearMemoryPageCount() - 1);
        RamPage firstBacking = cloneData->mmu[firstPage].getRamPageIndex();
        for (U32 i = 0; i < ramPageLinearMemoryPageCount(); i++) {
            MMU& entry = cloneData->mmu[firstPage + i];
            if (entry.getPageType() != PageType::Ram ||
                    ramPageGet(entry.getRamPageIndex()) != ramPageGet(firstBacking) + ((U64)i << K_PAGE_SHIFT)) {
                failed("linear memory child COW group did not retain contiguous backing at page %x", firstPage + i);
                break;
            }
        }
        if (!cloneData->linearMemoryMappings[firstPage]) {
            failed("linear memory child COW group was not installed in the aperture");
        }
    }

    if (testMemory->readd(privateAddress) != parentPrivateValue ||
            cloneMemory->readd(privateAddress) != childPrivateValue) {
        failed("linear memory clone did not preserve private COW isolation");
    }
    if (testMemory->readd(sharedAddress) != childSharedValue ||
            cloneMemory->readd(sharedAddress) != childSharedValue) {
        failed("linear memory clone did not preserve shared-page visibility");
    }
    verifyLinearAlias(testMemory, privateAddress, parentPrivateValue, "linear memory parent after child COW");
    verifyLinearAlias(cloneMemory, privateAddress, childPrivateValue, "linear memory child after COW");
    verifyLinearAlias(testMemory, sharedAddress, childSharedValue, "linear memory parent shared update");

    U32 cloneId = cloneProcess->id;
    if (KSystem::getProcess(cloneId)) {
        KSystem::eraseProcess(cloneId);
    }
    cloneProcess.reset();
    KThread::setCurrentThread(parentThread);
    if (testMemory->unmap(sharedAddress, sharedLength)) {
        failed("linear memory shared mapping cleanup failed");
    }
    if (ramPageUseLinearMemoryBacking() && testMemory->unmap(generationAddress, sharedLength)) {
        failed("linear memory backing-generation cleanup failed");
    }
}

void testLinearMemoryCodeInvalidation() {
    if (!ramPageUseLinearMemory()) {
        return;
    }

    testSelfModifying();
    if (testMemory->readb(TEST_CODE_ADDRESS + 2) != 0x40) {
        failed("linear memory self-modifying write did not update code");
    }
    KMemoryData* data = getMemData(testMemory);
    if (!data->linearMemoryMappings[TEST_CODE_ADDRESS >> K_PAGE_SHIFT]) {
        if (ramPageLinearMemoryPageCount() > 1) {
            return;
        }
        failed("linear memory code page was not installed in the aperture");
    } else if (*getLinearAliasAddress(data, TEST_CODE_ADDRESS + 2) != 0x40) {
        failed("linear memory code alias was stale after JIT invalidation");
    }
}

#endif
