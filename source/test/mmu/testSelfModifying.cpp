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
#include <bit>

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

#ifdef BOXEDWINE_HOST_EXCEPTIONS
void runLinearMemoryFileFirstTouchCase(bool write, bool preload) {
    constexpr U32 targetAddress = 0x20000000;
    constexpr U32 pageCount = LINEAR_MEMORY_RECOMPILE_FAULTS;
    constexpr U32 initialValue = 0x11223344;
    constexpr U32 addend = 0x01020304;
    const char* name = !write ? "file read" : preload ? "COW write" : "file write";

    newInstruction(0);
    KProcessPtr process = testContext().process;
    U32 fd = process->memfd_create(B("linear-first-touch"), 0);
    if ((S32)fd < 0) {
        failed("linear memory %s memfd failed", name);
        return;
    }
    std::shared_ptr<KFile> file = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject);
    if (process->ftruncate64(fd, pageCount * K_PAGE_SIZE)) {
        failed("linear memory %s truncate failed", name);
        process->close(fd);
        return;
    }
    for (U32 page = 0; page < pageCount; page++) {
        U32 value = initialValue;
        if (file->pwriteNative((U8*)&value, page * K_PAGE_SIZE, sizeof(value)) != sizeof(value)) {
            failed("linear memory %s file initialization failed", name);
        }
    }
    if (testMemory->mmap(testContext().thread, targetAddress, pageCount * K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_FIXED | K_MAP_PRIVATE, fd, 0) != targetAddress) {
        failed("linear memory %s mmap failed", name);
        process->close(fd);
        return;
    }

    // Compile against ordinary RAM, then run the same instruction against
    // fresh file/COW pages without rewriting or invalidating its code.
    cpu->reg[0].u32 = 0x1000;
    cpu->reg[2].u32 = addend;
    pushCode8(write ? 0x01 : 0x8b);
    pushCode8(0x10); // add [eax], edx / mov edx, [eax]
    runTestCPU();
    DecodedOp* op = testMemory->getDecodedOp(TEST_CODE_ADDRESS);
    const auto jitCode = op->pfnJitCode;
    if (!jitCode || op->exceptionCount) {
        failed("linear memory %s did not start with fault-free JIT code", name);
    }

    for (U32 page = 0; page < pageCount; page++) {
        U32 address = targetAddress + page * K_PAGE_SIZE;
        if (preload) {
            testMemory->readd(address);
        }
        PageType expectedType = preload ? PageType::CopyOnWrite : PageType::File;
        if (getMemData(testMemory)->mmu[address >> K_PAGE_SHIFT].getPageType() != expectedType) {
            failed("linear memory %s did not start with the expected page type", name);
        }
        cpu->reg[0].u32 = address - cpu->seg[DS].address;
        cpu->reg[2].u32 = addend;
        cpu->eip.u32 = 0;
        cpu->nextOp = cpu->getNextOp();
        do {
            cpu->run();
        } while (!cpu->nextOp || cpu->nextOp->inst != TestEnd);

        U32 actual = write ? testMemory->readd(address) : cpu->reg[2].u32;
        if (actual != (write ? initialValue + addend : initialValue)) {
            failed("linear memory %s was not executed exactly once", name);
        }
        U32 fileValue = 0;
        file->preadNativeUncached((U8*)&fileValue, page * K_PAGE_SIZE, sizeof(fileValue));
        if (fileValue != initialValue) {
            failed("linear memory %s changed private file backing", name);
        }
        op = testMemory->getDecodedOp(TEST_CODE_ADDRESS);
        if (page + 1 < pageCount) {
            if (!op || op->pfnJitCode != jitCode || op->exceptionCount != page + 1) {
                failed("linear memory %s retired JIT code on a resolved first touch", name);
            }
        } else if (!op || op->pfnJitCode || op->exceptionCount != MAX_OP_EXCEPTION_COUNT) {
            failed("linear memory %s did not retire JIT code after repeated first touches", name);
        }
    }
    testMemory->unmap(targetAddress, pageCount * K_PAGE_SIZE);
    process->close(fd);
}
#endif

void runLinearMemoryCrossPageCowRmwCase(bool checkedMemory = false) {
    if (ramPageLinearMemoryPageCount() != 1) {
        return;
    }

    constexpr U32 targetOffset = K_PAGE_SIZE * 2 - 1;
    constexpr U32 initialValue = 0x11223344;
    constexpr U32 addend = 0x01020304;
    U32 targetAddress = TEST_HEAP_ADDRESS + targetOffset;

    newInstruction(0);
    testMemory->writed(targetAddress, initialValue);

    KMemoryData* data = getMemData(testMemory);
    U32 firstPage = targetAddress >> K_PAGE_SHIFT;
    MMU& firstEntry = data->mmu[firstPage];
    RamPage retainedBacking = firstEntry.getRamPageIndex();
    ramPageRetain(retainedBacking);
    firstEntry.setPageType(testMemory, firstPage, PageType::CopyOnWrite);
    data->onPageChanged(firstPage);

    cpu->reg[0].u32 = targetOffset;
    cpu->reg[7].u32 = addend;
    pushCode8(0x01);
    pushCode8(0x38); // add dword ptr [eax], edi
#ifdef BOXEDWINE_HOST_EXCEPTIONS
    if (checkedMemory) {
        cpu->getNextOp()->exceptionCount = LINEAR_MEMORY_RECOMPILE_FAULTS;
    }
#endif
    runTestCPU();

    ramPageRelease(retainedBacking);
    U32 expected = initialValue + addend;
    U32 actual = testMemory->readd(targetAddress);
    if (actual != expected) {
        failed("linear memory cross-page COW add expected %x, got %x", expected, actual);
    }
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
    runLinearMemoryCrossPageCowRmwCase();
    runLinearMemoryCrossPageCowRmwCase(true);

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

void testLinearMemoryFileFirstTouches() {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
    if (!ramPageUseLinearMemory() || ramPageLinearMemoryPageCount() != 1) {
        return;
    }
    runLinearMemoryFileFirstTouchCase(false, false);
    runLinearMemoryFileFirstTouchCase(true, false);
    runLinearMemoryFileFirstTouchCase(true, true);
#endif
}

void testLinearMemoryWraparound() {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
    const U32 groupBytes = ramPageLinearMemoryPageCount() * K_PAGE_SIZE;
    const U32 highAddress = 0U - groupBytes;
    constexpr U32 iterations = 1000;

    // Exercise scalar loads/stores, read-modify-write, and the widest native
    // loads/stores. Map complete host-page groups so the fault comes from the
    // end of the aperture, not an unmapped guest page in a partial group.
    for (U32 variant = 0; variant < 5; variant++) {
        newInstruction(0);
        U32 flags = K_MAP_FIXED | K_MAP_PRIVATE | K_MAP_ANONYMOUS;
        // The unrounded length must not overflow mmap's 32-bit address check.
        if (testMemory->mmap(testContext().thread, highAddress, groupBytes - 1,
                K_PROT_READ | K_PROT_WRITE, flags, -1, 0) != highAddress ||
            testMemory->mmap(testContext().thread, 0, groupBytes,
                K_PROT_READ | K_PROT_WRITE, flags, -1, 0) != 0) {
            failed("linear memory wraparound mmap failed");
            return;
        }
        testMemory->memset(highAddress, 0, groupBytes);
        testMemory->memset(0, 0, groupBytes);
        const U32 width = variant < 3 ? 4 : 16;
        const U32 address = 0U - width / 2;
        for (U32 i = 0; i < width; i++) {
            testMemory->writeb(address + i, 0x20 + i);
            reinterpret_cast<U8*>(&cpu->xmm[0])[i] = 0x80 + i;
        }
        cpu->reg[0].u32 = 0x83828180;
        cpu->reg[1].u32 = iterations;

        CodeHolder code;
        initCode(code);
        Assembler a(&code);
        Label loop = a.new_label();
        check(a.bind(loop), "bind wraparound loop");
        U32 offset = address - TEST_HEAP_ADDRESS;
        switch (variant) {
        case 0: check(a.mov(eax, dword_ptr(offset)), "wraparound load"); break;
        case 1: check(a.mov(dword_ptr(offset), eax), "wraparound store"); break;
        case 2: check(a.add(dword_ptr(offset), 1), "wraparound add"); break;
        case 3: check(a.movdqu(xmm0, xmmword_ptr(offset)), "wraparound vector load"); break;
        case 4: check(a.movdqu(xmmword_ptr(offset), xmm0), "wraparound vector store"); break;
        }
        check(a.dec(ecx), "wraparound loop count");
        check(a.jnz(loop), "wraparound loop branch");
        pushGeneratedCode(code);
        runTestCPU();

        if (variant == 0) {
            verifyReg32(0, 0x23222120, "linear memory wraparound load");
        } else if (variant == 2) {
            if (testMemory->readd(address) != 0x23222120 + iterations) {
                failed("linear memory wraparound add was not applied exactly once per iteration");
            }
        } else {
            for (U32 i = 0; i < width; i++) {
                U8 actual = variant == 3 ? reinterpret_cast<U8*>(&cpu->xmm[0])[i] : testMemory->readb(address + i);
                U8 expected = (variant == 3 ? 0x20 : 0x80) + i;
                if (actual != expected) {
                    failed("linear memory wraparound variant %u byte %u: expected %x, got %x",
                        variant, i, expected, actual);
                }
            }
        }
        testMemory->unmap(highAddress, groupBytes);
        testMemory->unmap(0, groupBytes);
    }
#endif
}

void testJitMemoryReadOperands() {
#if defined(BOXEDWINE_WASM_JIT) || ((defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)) && defined(BOXEDWINE_HOST_EXCEPTIONS))
    constexpr U32 targetAddress = 0x20000000;
    constexpr U32 savedFlags = CF | PF | AF | ZF | SF | OF | DF;
    constexpr U64 xmmHigh = 0x123456789abcdef0ULL;
    constexpr U64 singleUpper = 0xa55a123400000000ULL;
    const U8 scalarOpcodes[] = {0x58, 0x5c, 0x59, 0x5e, 0x5d, 0x5f, 0x51};
    const double scalarResults[] = {12.0, 4.0, 32.0, 2.0, 4.0, 8.0, 2.0};
#ifdef BOXEDWINE_WASM_JIT
    const U32 faultCounts[] = {0}; // WASM uses explicit TLB checks instead of host-fault tiers.
#else
    const U32 faultCounts[] = {0, LINEAR_MEMORY_RECOMPILE_FAULTS, MAX_OP_EXCEPTION_COUNT};
#endif
    const U32 groupBytes = ramPageLinearMemoryPageCount() * K_PAGE_SIZE;
    const U32 mappingBytes = groupBytes * 2;
    const U32 offsets[] = {0, K_PAGE_SIZE - 1, groupBytes - 1};
    const U32 offsetCount = groupBytes > K_PAGE_SIZE ? 3 : 2;

    // Compile each load on resident RAM, then reuse its JIT code on unread
    // file pages. Cross both guest pages and larger host-page groups, and
    // exercise each memory fallback tier.
    for (U32 faultCount : faultCounts) {
        for (U32 boundary = 0; boundary < offsetCount; boundary++) {
            for (U32 kind = 0; kind < 24; kind++) {
                newInstruction(savedFlags);
                const bool scalar = kind >= 10;
                const bool single = kind >= 10 && kind < 17;
                const U32 offset = offsets[boundary];
                U64 fileBits = !scalar ? 0x88776655800180ffULL :
                    single ? (U64)std::bit_cast<U32>(4.0f) : std::bit_cast<U64>(4.0);
                const U64 initialXmm = single ? singleUpper | std::bit_cast<U32>(8.0f) : std::bit_cast<U64>(8.0);
                testMemory->writeq(TEST_HEAP_ADDRESS + 0x1000, fileBits);
                cpu->reg[0].u32 = 0x1000;
                cpu->xmm[0].pi.u64[0] = initialXmm;
                cpu->xmm[0].pi.u64[1] = xmmHigh;

                if (scalar) {
                    pushCode8(single ? 0xf3 : 0xf2);
                    pushCode8(0x0f);
                    pushCode8(scalarOpcodes[(kind - 10) % 7]);
                } else if (kind < 4) {
                    if (kind == 1) pushCode8(0x66);
                    pushCode8(kind < 2 ? 0x8b : 0x8a);
                } else {
                    if (kind < 6) pushCode8(0x66);
                    pushCode8(0x0f);
                    pushCode8((kind & 1 ? 0xbe : 0xb6) + (kind >= 8 ? 1 : 0));
                }
                pushCode8(kind == 3 ? 0x20 : 0x00); // destination aliases EAX for integer loads
#ifdef BOXEDWINE_HOST_EXCEPTIONS
                cpu->getNextOp()->exceptionCount = faultCount;
#endif
                runTestCPU();
                DecodedOp* op = testMemory->getDecodedOp(TEST_CODE_ADDRESS);
                if (!op || !op->pfnJitCode) {
                    failed("memory operand case %u did not compile", kind);
                    return;
                }

                KProcessPtr process = testContext().process;
                U32 fd = process->memfd_create(B("jit-memory-operands"), 0);
                if ((S32)fd < 0) {
                    failed("memory operand memfd failed");
                    return;
                }
                std::shared_ptr<KFile> file = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject);
                if (process->ftruncate64(fd, mappingBytes) ||
                    file->pwriteNative((U8*)&fileBits, offset, sizeof(fileBits)) != sizeof(fileBits) ||
                    testMemory->mmap(testContext().thread, targetAddress, mappingBytes,
                        K_PROT_READ, K_MAP_FIXED | K_MAP_PRIVATE, fd, 0) != targetAddress) {
                    failed("memory operand file setup failed");
                    process->close(fd);
                    return;
                }
                const U32 initialEax = targetAddress + offset - cpu->seg[DS].address;
                cpu->reg[0].u32 = initialEax;
                cpu->xmm[0].pi.u64[0] = initialXmm;
                cpu->xmm[0].pi.u64[1] = xmmHigh;
                cpu->setFlags(savedFlags, FMASK_ALL);
                cpu->eip.u32 = 0;
                cpu->nextOp = cpu->getNextOp();
                do { cpu->run(); } while (!cpu->nextOp || cpu->nextOp->inst != TestEnd);

                U32 expectedEax = initialEax;
                const U32 expectedIntegers[] = {
                    0x800180ff, (initialEax & 0xffff0000) | 0x80ff,
                    (initialEax & 0xffffff00) | 0xff, (initialEax & 0xffff00ff) | 0xff00,
                    (initialEax & 0xffff0000) | 0xff, (initialEax & 0xffff0000) | 0xffff,
                    0xff, 0xffffffff, 0x80ff, 0xffff80ff
                };
                U64 expectedXmm = initialXmm;
                if (scalar) {
                    double value = scalarResults[(kind - 10) % 7];
                    expectedXmm = single ? singleUpper | std::bit_cast<U32>((float)value) : std::bit_cast<U64>(value);
                } else {
                    expectedEax = expectedIntegers[kind];
                }
                if (cpu->reg[0].u32 != expectedEax || cpu->xmm[0].pi.u64[0] != expectedXmm ||
                    cpu->xmm[0].pi.u64[1] != xmmHigh || (cpu->flags & savedFlags) != savedFlags) {
                    failed("memory operand case %u tier %u offset %u corrupted destination or flags",
                        kind, faultCount, offset);
                }
                testMemory->unmap(targetAddress, mappingBytes);
                process->close(fd);
            }
        }
    }
#endif
}

void testJitMemoryReadFaultState() {
#ifdef BOXEDWINE_WASM_JIT
    // A direct load must not mark an uninitialized destination local dirty,
    // nor discard an earlier write when the MMU helper raises a guest fault.
    constexpr U32 initialEdi = 0x12345678;
    constexpr U32 writtenEdi = 0x89abcdef;
    constexpr U32 faultOffset = 0x01000000;
    const U8 loadOpcodes[] = {0, 0xb6, 0xbe, 0xb7, 0xbf};
    for (bool dirty : {false, true}) {
        for (U8 opcode : loadOpcodes) {
            newInstruction(0);
            cpu->reg[7].u32 = initialEdi;
            cpu->reg[1].u32 = faultOffset;
            U32 length = 0;
            if (dirty) {
                pushCode8(0xbf); // mov edi, immediate
                testPushCode32(writtenEdi);
                length += 5;
            }
            if (opcode) {
                pushCode8(0x0f);
                pushCode8(opcode);
                length += 3;
            } else {
                pushCode8(0x8b);
                length += 2;
            }
            pushCode8(0x39); // mov/movzx/movsx edi,[ecx]
            U32 faultAddress = cpu->seg[DS].address + faultOffset;
            testMemory->unmap(faultAddress, K_PAGE_SIZE);
            KSigAction& action = testContext().process->sigActions[K_SIGSEGV];
            action.reset();
            action.handlerAndSigAction = TEST_CODE_ADDRESS + length;
            action.flags = 0;
            runTestCPU();
            if (action.sigInfo[0] != K_SIGSEGV || action.sigInfo[3] != faultAddress ||
                cpu->reg[7].u32 != (dirty ? writtenEdi : initialEdi)) {
                failed("WASM memory load %x dirty %u did not preserve fault state", opcode, (U32)dirty);
            }
            action.reset();
        }
    }
#endif
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
