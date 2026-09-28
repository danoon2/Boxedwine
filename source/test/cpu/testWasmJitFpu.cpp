/*
 * Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "boxedwine.h"
#include "testCPU.h"
#include "ksignal.h"
#include <limits>
#include <cmath>

#if defined(__TEST) && (defined(BOXEDWINE_WASM_JIT) || defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8))
namespace {
void fp(U8 opcode, U8 modrm) {
    testPushCode8(opcode);
    testPushCode8(modrm);
}
void fpMem(U8 opcode, U8 group, U32 address) {
    fp(opcode, (group << 3) | 5);
    testPushCode32(address);
}
void beginFp(U32 top) {
    testNewInstruction(0);
    auto& fpu = testContext().cpu->fpu;
    fpu.FINIT();
    fpu.top = top;
}
void expectFp(U32 index, double expected) {
    auto& fpu = testContext().cpu->fpu;
    double actual = fpu.getF64(fpu.STV(index));
    if (actual != expected) testFail("x87 cache ST(%u): %.17g, expected %.17g", index, actual, expected);
}
void runFpWithHostRegisterCheck() {
#if defined(BOXEDWINE_JIT_ARMV8) && defined(__GNUC__)
    // Keep independent caller values live in every callee-saved D register.
    // Empty asm barriers make their contents observable on both sides of JIT.
    register double d8 asm("d8") = 8.125;
    register double d9 asm("d9") = 9.25;
    register double d10 asm("d10") = 10.375;
    register double d11 asm("d11") = 11.5;
    register double d12 asm("d12") = 12.625;
    register double d13 asm("d13") = 13.75;
    register double d14 asm("d14") = 14.875;
    register double d15 asm("d15") = 15.125;
    asm volatile("" : "+w"(d8), "+w"(d9), "+w"(d10), "+w"(d11),
        "+w"(d12), "+w"(d13), "+w"(d14), "+w"(d15));
    testRunCPU();
    asm volatile("" : "+w"(d8), "+w"(d9), "+w"(d10), "+w"(d11),
        "+w"(d12), "+w"(d13), "+w"(d14), "+w"(d15));
    if (d8 != 8.125 || d9 != 9.25 || d10 != 10.375 || d11 != 11.5 ||
        d12 != 12.625 || d13 != 13.75 || d14 != 14.875 || d15 != 15.125)
        testFail("ARM JIT clobbered callee-saved floating-point registers");
#else
    testRunCPU();
#endif
}
void requireFpJit() {
    auto* entry = testContext().memory->getDecodedOp(TEST_CODE_ADDRESS);
    if (!entry || !entry->pfnJitCode) testFail("x87 cache test was not compiled");
}
}

void testWasmJitFpuCache() {
    // Incoming extended-format values, every possible physical TOP, lazy
    // conversion after dirty pushes, register renaming, and duplicate aliases.
    for (U32 top = 0; top < 8; ++top) {
        beginFp(top);
        auto& fpu = testContext().cpu->fpu;
        fpu.FLD_I64(3, top);
        fpu.tags[top] = TAG_Valid;
        fp(0xd9, 0xe8); // fld1: 1,3
        fp(0xd9, 0xe8); // fld1: 1,1,3
        fp(0xd8, 0xc2); // fadd st0,st2: 4,1,3 (imports old st0)
        fp(0xd9, 0xca); // fxch st2: 3,1,4
        fp(0xd9, 0xc0); // fld st0: 3,3,1,4
        fp(0xd8, 0xc0); // fadd st0,st0: 6,3,1,4
        fp(0xde, 0xc1); // faddp st1,st0: 9,1,4
        fp(0xd9, 0xc9); // fxch st1: 1,9,4
        testRunCPU();
        expectFp(0, 1); expectFp(1, 9); expectFp(2, 4);
        if (fpu.top != ((top + 6) & 7)) testFail("x87 virtual TOP wraparound");
        for (U32 i = 0; i < 8; ++i) {
            if (fpu.tags[fpu.STV(i)] != (i < 3 ? TAG_Valid : TAG_Empty))
                testFail("x87 cached tag mismatch at ST(%u)", i);
        }
        requireFpJit();
    }

    // A single-precision store must not round the retained double, and the
    // subsequent unary instruction must preserve its remaining precision.
    beginFp(5);
    const U64 precise = 0x3ff0000000000001ull;
    auto* mem = testContext().memory;
    mem->writeq(TEST_HEAP_ADDRESS + 0x100, precise);
    fpMem(0xdd, 0, 0x100); // fld double
    fpMem(0xd9, 2, 0x200); // fst float, no pop
    fpMem(0xdd, 2, 0x208); // fst double, no pop
    fp(0xd9, 0xe0); // fchs with the cached double
    fpMem(0xdd, 3, 0x210);
    testRunCPU();
    if (mem->readq(TEST_HEAP_ADDRESS + 0x208) != precise ||
        mem->readq(TEST_HEAP_ADDRESS + 0x210) != (precise | (1ull << 63)))
        testFail("x87 non-pop float store changed cached precision");

    // Integer registers, byte/word aliases, carry flags, and x87 values must
    // remain independent while their caches are live at the same time.
    beginFp(5);
    testContext().cpu->fpu.FLD_I64(3, 5);
    testContext().cpu->fpu.tags[5] = TAG_Valid;
    fp(0xd9, 0xe8); // 1,3
    testPushCode8(0xb8); testPushCode32(0x123400ff); // mov eax,...
    testPushCode8(0x04); testPushCode8(1); // add al,1: CF=1
    fp(0xd8, 0xc1); // 4,3; lazy import and precision check preserve CF
    testPushCode8(0x80); testPushCode8(0xd4); testPushCode8(0); // adc ah,0
    testPushCode8(0x66); testPushCode8(0x05); testPushCode16(0x20); // add ax,32
    testPushCode8(0x0f); testPushCode8(0xb7); testPushCode8(0xc8); // movzx ecx,ax
    testPushCode8(0x8d); testPushCode8(0x54); testPushCode8(0x89); testPushCode8(7); // lea edx,[ecx*5+7]
    fp(0xd9, 0xc9); // 3,4
    testPushCode8(0x41); // inc ecx
    testPushCode8(0x49); // dec ecx
    testPushCode8(0x81); testPushCode8(0xe2); testPushCode32(0xffff); // and edx,65535
    testPushCode8(0x83); testPushCode8(0xea); testPushCode8(7); // sub edx,7
    testPushCode8(0x83); testPushCode8(0xca); testPushCode8(1); // or edx,1
    fp(0xde, 0xc1); // 7
    testPushCode8(0x83); testPushCode8(0xf2); testPushCode8(1); // xor edx,1
    testRunCPU();
    expectFp(0, 7);
    if (testContext().cpu->reg[0].u32 != 0x12340120 ||
        testContext().cpu->reg[1].u32 != 0x120 || testContext().cpu->reg[2].u32 != 0x5a0)
        testFail("integer registers/flags corrupted while x87 cache was live");
    requireFpJit();

    // A fused compare/branch can compile intervening instructions via
    // preCompile(skippedOp=true). Both branch paths must flush the same state,
    // and the join must not reuse the fall-through arm's virtual stack mapping.
    for (U32 taken : {0u, 1u}) {
        beginFp(6);
        testContext().cpu->reg[0].u32 = taken;
        fp(0xd9, 0xe8); // 1
        testPushCode8(0x83); testPushCode8(0xf8); testPushCode8(1); // cmp eax,1
        fp(0xd9, 0xe8); // 1,1; skipped by fused producer's outer compile loop
        testPushCode8(0x74); testPushCode8(2); // je join
        fp(0xd8, 0xc0); // 2,1
        fp(0xde, 0xc1); // join: 2 or 3
        testPushCode8(0x31); testPushCode8(0xdb); // xor ebx,ebx: comparison flags dead
        testRunCPU();
        expectFp(0, taken ? 2 : 3);
        if (testContext().cpu->fpu.top != 5) testFail("x87 cache fused branch TOP");
        requireFpJit();
    }

    // Loop backedges and dispatcher exits must reload the right physical stack.
    beginFp(7);
    fp(0xd9, 0xe8);
    testPushCode8(0xb9); testPushCode32(1000); // mov ecx,1000
    fp(0xd9, 0xe8); // loop: fld1
    fp(0xde, 0xc1); // faddp
    testPushCode8(0x49); // dec ecx
    testPushCode8(0x75); testPushCode8(0xf9); // jnz loop
    testRunCPU();
    expectFp(0, 1001);
    requireFpJit();

    // Every register division encoding, both destination directions, aliases,
    // pop/no-pop, entry TOP, and 24/64-bit precision. Force the cold arm with
    // unmasked exceptions or an incoming special tag as well as the fast arm.
    const struct {
        U8 opcode;
        U8 modrm;
        bool dstIsTop;
        bool reverse;
        bool pop;
    } divisions[] = {
        {0xd8, 0xf0, true, false, false},
        {0xd8, 0xf8, true, true, false},
        {0xdc, 0xf8, false, false, false},
        {0xdc, 0xf0, false, true, false},
        {0xde, 0xf8, false, false, true},
        {0xde, 0xf0, false, true, true},
    };
    for (const auto& div : divisions) {
        for (U32 top = 0; top < 8; ++top) {
            for (U32 mode : {0u, 1u, 2u}) {
                for (bool single : {false, true}) {
                    for (U8 index : {0, 1}) {
                        beginFp(top);
                        auto& fpu = testContext().cpu->fpu;
                        fpu.SetCW((single ? 0x007f : 0x037f) & (mode == 1 ? ~FPU_SW_ZE : 0xffff));
                        fpu.FLD_I64(3, top);
                        fpu.tags[top] = mode == 2 ? TAG_Special : TAG_Valid;
                        fp(0xd9, 0xe8);
                        fp(0xd8, 0xc0); // 2,3
                        fp(0xd9, 0xc9); // 3,2, dirty renamed values
                        fp(div.opcode, div.modrm + index);
                        fp(0xd9, 0xe8);
                        fp(0xde, 0xc1); // add 1 after either execution path
                        testRunCPU();
                        double values[] = {3, 2};
                        U32 dst = div.dstIsTop ? 0 : index;
                        U32 src = div.dstIsTop ? index : 0;
                        double result = div.reverse ? values[src] / values[dst] : values[dst] / values[src];
                        values[dst] = single ? (double)(float)result : result;
                        U32 finalTop = div.pop ? 1 : 0;
                        double sum = values[finalTop] + 1;
                        expectFp(0, single ? (double)(float)sum : sum);
                        if (!div.pop) expectFp(1, values[1]);
                        if (fpu.top != ((top + (div.pop ? 0 : 7)) & 7)) testFail("cached divide TOP/pop mismatch");
                        requireFpJit();
                    }
                }
            }
        }
    }

    // Division's cold exit must not lose flags from an integer producer which
    // would otherwise fuse across it with a following conditional branch.
    for (bool cold : {false, true}) {
        for (U32 taken : {0u, 1u}) {
            beginFp(4);
            if (cold) testContext().cpu->fpu.SetCW(0x037b);
            testContext().cpu->reg[0].u32 = taken;
            fp(0xd9, 0xe8);
            fp(0xd8, 0xc0); // 2
            fp(0xd9, 0xe8); // 1,2
            testPushCode8(0x83); testPushCode8(0xf8); testPushCode8(1); // cmp eax,1
            fp(0xde, 0xf9); // fdivp: 2
            testPushCode8(0x74); testPushCode8(2); // je join
            fp(0xd8, 0xc0); // fall through: 4
            fp(0xd9, 0xe8);
            fp(0xde, 0xc1); // join: 3 or 5
            testPushCode8(0x31); testPushCode8(0xdb); // comparison flags dead
            testRunCPU();
            expectFp(0, taken ? 3 : 5);
            requireFpJit();
        }
    }

    // Special/empty tags still invoke the interpreter and record exceptions.
    // Preserve an unrelated dirty stack value across that exit too.
    for (bool empty : {false, true}) {
        for (bool unmasked : {false, true}) {
            beginFp(2);
            auto& fpu = testContext().cpu->fpu;
            fpu.FLD_I64(0, 2);
            fpu.tags[2] = empty ? TAG_Empty : TAG_Zero;
            fpu.SetCW(unmasked ? 0x037a : 0x037f);
            fp(0xd9, 0xe8);
            fp(0xd8, 0xc0); // 2,0
            fp(0xd9, 0xe8); // 1,2,0
            fp(0xd8, 0xf2); // divide 1 by the incoming zero/empty slot
            testRunCPU();
            expectFp(1, 2);
            U32 bits = empty ? FPU_SW_IE | FPU_SW_SF : FPU_SW_ZE;
            if ((fpu.sw & bits) != bits || (unmasked && !(fpu.sw & FPU_SW_ES)))
                testFail("cached divide lost exception status");
            if (fpu.top != 0) testFail("cached divide exception changed TOP");
            requireFpJit();
        }
    }
}

void testWasmJitFpuMemory() {
    // Execute both the direct memory path and the complete-instruction
    // interpreter path, including a stack-changing operation in the cold arm.
    for (U32 address : {0x100u, K_PAGE_SIZE - 2u}) {
        beginFp(0);
        auto* mem = testContext().memory;
        mem->writed(TEST_HEAP_ADDRESS + address, 0x40400000); // 3
        fp(0xd9, 0xe8); // 1
        fp(0xd9, 0xe8); // 1,1
        fp(0xd8, 0xc1); // 2,1
        fpMem(0xd9, 0, address); // 3,2,1
        fp(0xde, 0xc1); // 5,1
        fpMem(0xd9, 3, address + 0x2000); // store 5, pop
        fp(0xd8, 0xc0); // 2
        testRunCPU();
        expectFp(0, 2);
        if (mem->readd(TEST_HEAP_ADDRESS + address + 0x2000) != 0x40a00000)
            testFail("x87 memory slow path lost cached state");
        if (testContext().cpu->fpu.top != 7) testFail("x87 memory slow path TOP");
        requireFpJit();
    }

    // Ordinary MOV transfers do not change x87 state. Exercise each width on
    // direct pages and across a page boundary, with dirty values on both sides
    // of the helper call. Include accumulator-offset and immediate stores.
    for (U32 width : {1u, 2u, 4u}) {
        for (U32 address : {0x100u, K_PAGE_SIZE - 1u}) {
            for (U32 form : {0u, 1u, 2u}) {
                beginFp(5);
                auto& context = testContext();
                context.cpu->reg[0].u32 = 0x89abcdef;
                fp(0xd9, 0xe8);
                fp(0xd8, 0xc0); // 2, dirty before the store
                if (width == 2) testPushCode8(0x66);
                if (form == 1) {
                    testPushCode8(width == 1 ? 0xa2 : 0xa3);
                    testPushCode32(address); // mov [moffs],al/ax/eax
                } else {
                    fpMem(form == 2 ? (width == 1 ? 0xc6 : 0xc7) :
                        (width == 1 ? 0x88 : 0x89), 0, address);
                    if (form == 2) {
                        if (width == 1) testPushCode8(0xef);
                        else if (width == 2) testPushCode16(0xcdef);
                        else testPushCode32(0x89abcdef);
                    }
                }
                fp(0xd8, 0xc0); // 4, dirty again before the load
                testPushCode8(0xb8); testPushCode32(0x76543210);
                if (width == 2) testPushCode8(0x66);
                if (form == 1) {
                    testPushCode8(width == 1 ? 0xa0 : 0xa1);
                    testPushCode32(address); // mov al/ax/eax,[moffs]
                } else {
                    fpMem(width == 1 ? 0x8a : 0x8b, 0, address);
                }
                fp(0xd9, 0xe8);
                fp(0xde, 0xc1); // 5
                testRunCPU();
                expectFp(0, 5);
                U32 mask = width == 1 ? 0xff : (width == 2 ? 0xffff : 0xffffffff);
                U32 expected = (0x76543210 & ~mask) | (0x89abcdef & mask);
                if (context.cpu->reg[0].u32 != expected ||
                    (context.memory->readd(TEST_HEAP_ADDRESS + address) & mask) != (0x89abcdef & mask))
                    testFail("x87 cache across MOV: width=%u, form=%u, address=%x", width, form, address);
                if (context.cpu->fpu.top != 4) testFail("MOV changed cached x87 TOP");
                requireFpJit();
            }
        }
    }

    // Sign/zero extension and partial-register writes must also survive a
    // helper return without replacing the cached FPU values or their tags.
    for (U32 address : {0x100u, K_PAGE_SIZE - 1u}) {
        for (U32 width : {1u, 2u}) {
            for (bool sign : {false, true}) {
                for (bool wordDest : {false, true}) {
                    if (width == 2 && wordDest) continue;
                    beginFp(0);
                    auto& context = testContext();
                    context.cpu->reg[0].u32 = 0x12345678;
                    context.memory->writew(TEST_HEAP_ADDRESS + address, 0x8181);
                    fp(0xd9, 0xe8);
                    fp(0xd8, 0xc0); // 2
                    if (wordDest) testPushCode8(0x66);
                    testPushCode8(0x0f);
                    fpMem((sign ? 0xbe : 0xb6) + (width == 2 ? 1 : 0), 0, address);
                    fp(0xd8, 0xc0); // 4
                    testRunCPU();
                    expectFp(0, 4);
                    U32 expected = width == 1 ? (sign ? 0xffffff81 : 0x81) : (sign ? 0xffff8181 : 0x8181);
                    if (wordDest) expected = 0x12340000 | (expected & 0xffff);
                    if (context.cpu->reg[0].u32 != expected) testFail("MOVSX/MOVZX corrupted live x87 cache or integer destination");
                    requireFpJit();
                }
            }
        }
    }

    // Change the next cached instruction from FADD to FMUL. The store must
    // exit the invalidated block with the current TOP/tags/value written back.
    beginFp(3);
    fp(0xd9, 0xe8);
    fp(0xd8, 0xc0);
    fp(0xd9, 0xe8);
    fp(0xde, 0xc1); // 3
    U32 modifiedByte = testContext().codeIp - TEST_CODE_ADDRESS + 8 + 1;
    testPushCode8(0x2e); // CS override: write to this code block
    fpMem(0xc6, 0, modifiedByte);
    testPushCode8(0xc8); // replace fadd st0,st0 with fmul st0,st0
    fp(0xd8, 0xc0);
    testRunCPU();
    expectFp(0, 9);
    if (testContext().cpu->fpu.top != 2) testFail("self-modifying MOV lost x87 TOP");
}

void testWasmJitFpuIntegerStores() {
    const struct {
        double value;
        S64 rounded[4]; // nearest-even, down, up, toward zero
    } cases[] = {
        {1.5, {2, 1, 2, 1}},
        {2.5, {2, 2, 3, 2}},
        {-1.5, {-2, -2, -1, -1}},
        {-2.5, {-2, -3, -2, -2}},
        {123.75, {124, 123, 124, 123}},
        {-0.0, {0, 0, 0, 0}},
        {2147483647.0, {2147483647, 2147483647, 2147483647, 2147483647}},
        {2147483647.5, {0x80000000, 2147483647, 0x80000000, 2147483647}},
        {2147483648.0, {0x80000000, 0x80000000, 0x80000000, 0x80000000}},
        {-2147483648.0, {-2147483648ll, -2147483648ll, -2147483648ll, -2147483648ll}},
        {-2147483649.0, {0x80000000, 0x80000000, 0x80000000, 0x80000000}},
        {std::numeric_limits<double>::infinity(), {0x80000000, 0x80000000, 0x80000000, 0x80000000}},
        {-std::numeric_limits<double>::infinity(), {0x80000000, 0x80000000, 0x80000000, 0x80000000}},
        {std::numeric_limits<double>::quiet_NaN(), {0x80000000, 0x80000000, 0x80000000, 0x80000000}},
    };
    for (bool word : {false, true}) {
        for (U32 top = 0; top < 8; ++top) {
            for (U32 rounding = 0; rounding < 4; ++rounding) {
                for (U8 group : {1, 2, 3}) { // FISTTP, FIST, FISTP
                    for (U32 address : {0x100u, K_PAGE_SIZE - (word ? 1u : 2u)}) {
                        for (const auto& item : cases) {
                            beginFp(top);
                            auto& context = testContext();
                            context.cpu->fpu.SetCW(0x037f | (rounding << 10));
                            U64 bits;
                            memcpy(&bits, &item.value, sizeof(bits));
                            context.memory->writeq(TEST_HEAP_ADDRESS + 0x600, bits);
                            fp(0xd9, 0xe8);
                            fp(0xd8, 0xc0); // unrelated dirty 2
                            fpMem(0xdd, 0, 0x600);
                            fp(0xd9, 0xc0); // duplicate the value; conversion must not mutate either copy
                            testPushCode8(0xb8); testPushCode32(0xffffffff);
                            testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(1); // CF=1
                            fpMem(word ? 0xdf : 0xdb, group, address);
                            testPushCode8(0x83); testPushCode8(0xd0); testPushCode8(0); // adc eax,0
                            fpMem(0xdd, 3, 0x800); // retain the exact surviving double, then pop
                            if (group == 2) fpMem(0xdd, 3, 0x808); // FIST did not pop its source
                            fp(0xd8, 0xc0); // unrelated value is still 2: now 4
                            testRunCPU();
                            U32 expected = (U32)item.rounded[group == 1 ? 3 : rounding];
                            if (word) expected &= 0xffff;
                            U32 actual = word ? context.memory->readw(TEST_HEAP_ADDRESS + address) : context.memory->readd(TEST_HEAP_ADDRESS + address);
                            if (actual != expected)
                                testFail("cached integer store: group=%u, rounding=%u, value=%.17g, address=%x, got=%x, expected=%x", group, rounding, item.value, address, actual, expected);
                            if (context.memory->readq(TEST_HEAP_ADDRESS + 0x800) != bits ||
                                (group == 2 && context.memory->readq(TEST_HEAP_ADDRESS + 0x808) != bits))
                                testFail("cached integer store modified retained floating-point value");
                            expectFp(0, 4);
                            if (context.cpu->reg[0].u32 != 1) testFail("cached integer store lost carry flag");
                            auto& fpu = context.cpu->fpu;
                            if (fpu.top != ((top + 7) & 7)) testFail("cached integer store pop/TOP mismatch");
                            for (U32 i = 0; i < 8; ++i) {
                                if (fpu.tags[fpu.STV(i)] != (i == 0 ? TAG_Valid : TAG_Empty))
                                    testFail("cached integer store tag mismatch");
                            }
                            requireFpJit();
                        }
                    }
                }
            }
        }
    }

    // Store four NOP bytes over the next two FADDs. A code-page FISTP must
    // commit its pop exactly once, invalidate the code, and preserve ST(0).
    beginFp(3);
    testContext().memory->writed(TEST_HEAP_ADDRESS + 0x600, 0x90909090);
    fp(0xd9, 0xe8);
    fp(0xd8, 0xc0); // 2
    fpMem(0xdb, 0, 0x600); // fild the signed integer encoding four NOPs
    U32 target = testContext().codeIp - TEST_CODE_ADDRESS + 7;
    testPushCode8(0x2e); // CS override
    fpMem(0xdb, 3, target); // fistp dword [next instruction]
    fp(0xd8, 0xc0); // overwritten with two NOPs
    fp(0xd8, 0xc0); // overwritten with two NOPs
    fp(0xd8, 0xc0); // surviving FADD: 4
    testRunCPU();
    expectFp(0, 4);
    if (testContext().cpu->fpu.top != 2) testFail("code-page integer store popped incorrectly");
}

#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64) || defined(BOXEDWINE_JIT_ARMV8)
void testNativeJitFpuCachePressure() {
    // Fresh Wine prefixes can enter a block at a memory divide with only raw
    // incoming x87 state. Unlike a preceding FLD, this leaves entry TOP live
    // while the guarded MMU path and the division check need GP temporaries.
    const struct { U8 opcode; U8 group; } memoryDivisions[] = {
        {0xdc, 7}, {0xdc, 6}, {0xd8, 7}, {0xd8, 6},
        {0xda, 7}, {0xda, 6}, {0xde, 7}, {0xde, 6}
    };
    for (U32 top = 0; top < 8; ++top) {
        for (const auto& op : memoryDivisions) {
            for (bool cached : {false, true}) {
                for (U32 address : {0x100u, 0x1fffu}) {
                    beginFp(top);
                    auto& context = testContext();
                    context.cpu->fpu.FLD_I64(2, top);
                    context.cpu->fpu.tags[top] = TAG_Valid;
                    if (cached) context.cpu->fpu.getF64(top);
                    if (op.opcode == 0xdc) context.memory->writeq(TEST_HEAP_ADDRESS + address, 0x4020000000000000ull);
                    else if (op.opcode == 0xd8) context.memory->writed(TEST_HEAP_ADDRESS + address, 0x41000000);
                    else context.memory->writed(TEST_HEAP_ADDRESS + address, 8);
                    fpMem(op.opcode, op.group, address);
                    testRunCPU();
                    requireFpJit();
                    expectFp(0, op.group == 7 ? 4.0 : 0.25);
                    if (context.cpu->fpu.top != top) testFail("native memory divide entry changed TOP");
                }
            }
        }
    }

    // Memory compares must release their address before importing a tag or
    // allocating the byte temporaries used to update the condition codes.
    for (U32 top = 0; top < 8; ++top) {
        for (U8 opcode : {0xdc, 0xd8, 0xda, 0xde}) {
            for (bool pop : {false, true}) for (bool cached : {false, true}) {
                for (U32 address : {0x100u, K_PAGE_SIZE - 1u}) {
                    for (U32 left : {2u, 8u, 16u}) {
                        beginFp(top);
                        auto& context = testContext();
                        auto& fpu = context.cpu->fpu;
                        fpu.FLD_I64(left, top);
                        fpu.tags[top] = TAG_Valid;
                        fpu.FLD_I64(17, (top + 1) & 7);
                        fpu.tags[(top + 1) & 7] = TAG_Valid;
                        if (cached) fpu.getF64(top);
                        if (opcode == 0xdc) context.memory->writeq(TEST_HEAP_ADDRESS + address, 0x4020000000000000ull);
                        else if (opcode == 0xd8) context.memory->writed(TEST_HEAP_ADDRESS + address, 0x41000000);
                        else context.memory->writed(TEST_HEAP_ADDRESS + address, 8);
                        fpMem(opcode, pop ? 3 : 2, address);
                        testRunCPU();
                        requireFpJit();
                        U32 expected = left < 8 ? 0x0100 : (left == 8 ? 0x4000 : 0);
                        if ((fpu.sw & 0x4500) != expected || fpu.top != ((top + pop) & 7) ||
                            fpu.tags[top] != (pop ? TAG_Empty : TAG_Valid))
                            testFail("native memory compare entry lost status or stack state");
                        expectFp(0, pop ? 17 : left);
                    }
                }
            }
        }
    }

    for (U32 top = 0; top < 8; ++top) {
        beginFp(top);
        double values[8] = {};
        for (U32 i = 0; i < 8; ++i) {
            fp(0xd9, 0xe8);
            for (U32 j = 0; j < i; ++j) fp(0xd8, 0xc0);
            values[7-i] = (double)(1u << i);
        }
        for (U32 pass = 0; pass < 3; ++pass) for (U32 i = 1; i < 8; ++i) {
            fp(0xd9, 0xc8 + i);
            std::swap(values[0], values[i]);
            fp(0xd8, 0xc0 + i);
            values[0] += values[i];
        }
        fp(0xd9, 0xe4); // ftst: positive, with all eight slots occupied
        fpMem(0xdd, 7, 0x100); // fnstsw
        fp(0xd8, 0xd0); // fcom st0: equal
        fpMem(0xdd, 7, 0x102);
        runFpWithHostRegisterCheck();
        requireFpJit();
        for (U32 i = 0; i < 8; ++i) expectFp(i, values[i]);
        if (testContext().cpu->fpu.top != top) testFail("native x87 spill TOP");
        if ((testContext().memory->readw(TEST_HEAP_ADDRESS + 0x100) & 0x4500) != 0 ||
            (testContext().memory->readw(TEST_HEAP_ADDRESS + 0x102) & 0x4500) != 0x4000)
            testFail("native cached comparison under register pressure");

        // Overwrite a deep spilled slot, then reuse the popped slot. Neither
        // destination needs its previous value reloaded, but every untouched
        // value and the copy made before the pop must survive allocation.
        beginFp(top);
        for (U32 i = 0; i < 8; ++i) {
            fp(0xd9, 0xe8);
            for (U32 j = 0; j < i; ++j) fp(0xd8, 0xc0);
        }
        fp(0xdd, 0xdf); // fstp st(7): copy 128 to the deepest slot, then pop
        fp(0xd9, 0xe8); // fld1: overwrite the vacated slot
        testRunCPU();
        requireFpJit();
        expectFp(0, 1);
        for (U32 i = 1; i < 7; ++i) expectFp(i, (double)(1u << (7-i)));
        expectFp(7, 128);
        if (testContext().cpu->fpu.top != top) testFail("native x87 overwrite TOP");
        for (U32 i = 0; i < 8; ++i)
            if (testContext().cpu->fpu.tags[i] != TAG_Valid) testFail("native x87 overwrite tag");

        // Converting later raw inputs calls C++ while earlier sums and guest
        // XMM values are live. This exercises both host ABIs and cache eviction.
        beginFp(top);
        auto& context = testContext();
        for (U32 i = 0; i < 8; ++i) {
            context.cpu->fpu.FLD_I64(i+2, (top+i)&7);
            context.cpu->fpu.tags[(top+i)&7] = TAG_Valid;
            context.cpu->xmm[i].pi.u64[0] = 0x0123456789abcdefull + i;
            context.cpu->xmm[i].pi.u64[1] = 0xfedcba9876543210ull - i;
        }
        fp(0xd9, 0xe8);
        for (U32 i = 1; i < 8; ++i) fp(0xd8, 0xc0+i);
        testRunCPU();
        expectFp(0, 36);
        for (U32 i = 0; i < 8; ++i) {
            if (context.cpu->xmm[i].pi.u64[0] != 0x0123456789abcdefull+i ||
                context.cpu->xmm[i].pi.u64[1] != 0xfedcba9876543210ull-i)
                testFail("native x87 helper clobbered guest XMM%u", i);
        }
    }

    beginFp(0);
    fp(0xd9, 0xe8);
    fp(0xd8, 0xc0);
    fp(0xd8, 0xc0);
    testRunCPU();
    expectFp(0, 4);
    auto& context = testContext();
    DecodedOp* interior = context.memory->getDecodedOp(TEST_CODE_ADDRESS+2);
    if (!interior || interior->pfnJitCode || (interior->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE))
        testFail("native cached interior must allow independent interpreted entry");
    context.cpu->fpu.FINIT();
    context.cpu->fpu.FLD_I64(3, 0);
    context.cpu->fpu.tags[0] = TAG_Valid;
    context.cpu->eip.u32 = TEST_CODE_ADDRESS + 2 - context.cpu->seg[CS].address;
    context.cpu->nextOp = nullptr;
    testRunCPU();
    expectFp(0, 12);
    if (!interior->pfnJitCode || !(interior->flags2 & OP_FLAG2_JUMP_TARGET))
        testFail("native cached interior did not become an independent JIT entry");
    DecodedOp* head = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
    if (!head || !head->pfnJitCode || interior->blockStart != head)
        testFail("native interior promotion lost the compiled prefix");

    // Discover another interior after the first recompile. All entry points
    // must read the incoming architectural stack, regardless of its TOP.
    for (U32 top = 0; top < 8; ++top) {
        for (U32 offset : {4u, 2u, 0u}) {
            context.cpu->fpu.FINIT();
            context.cpu->fpu.top = top;
            context.cpu->fpu.FLD_I64(5, top);
            context.cpu->fpu.tags[top] = TAG_Valid;
            context.cpu->eip.u32 = TEST_CODE_ADDRESS + offset - context.cpu->seg[CS].address;
            context.cpu->nextOp = nullptr;
            testRunCPU();
            expectFp(0, offset == 0 ? 4 : offset == 2 ? 20 : 10);
            DecodedOp* entry = context.memory->getDecodedOp(TEST_CODE_ADDRESS + offset);
            if (!entry->pfnJitCode || entry->blockStart != head)
                testFail("native promoted entry or owning block was lost");
        }
    }

    // A write into the rebuilt block must invalidate all its promoted entries.
    context.memory->writeb(TEST_CODE_ADDRESS + 5, 0xc1); // last add uses ST(1)
    context.cpu->fpu.FINIT();
    context.cpu->fpu.FLD_I64(5, 0);
    context.cpu->fpu.tags[0] = TAG_Valid;
    context.cpu->eip.u32 = TEST_CODE_ADDRESS - context.cpu->seg[CS].address;
    context.cpu->nextOp = nullptr;
    testRunCPU();
    expectFp(0, 7);

    // Recompiling an existing block must not mistake its old JIT flag for a
    // successful recursive compilation of an earlier, uncompiled jump target.
    beginFp(0);
    fp(0xcd, 0x97); // earlier TestEnd, deliberately left interpreted
    fp(0xd9, 0xe8); // owning block starts at +2
    fp(0xd8, 0xc0); // hidden entry at +4
    fp(0xd8, 0xc0);
    fp(0xeb, 0xf6); // jump back to TestEnd at +0
    context.cpu->getOp(TEST_CODE_ADDRESS, 0)->flags |= OP_FLAG_NO_JIT;
    context.cpu->eip.u32 = TEST_CODE_ADDRESS + 2 - context.cpu->seg[CS].address;
    context.cpu->nextOp = nullptr;
    testRunCPU();
    expectFp(0, 4);
    interior = context.memory->getDecodedOp(TEST_CODE_ADDRESS + 4);
    if (!interior || interior->pfnJitCode)
        testFail("backward-branch test did not create a hidden FPU entry");
    context.cpu->fpu.FINIT();
    context.cpu->fpu.FLD_I64(3, 0);
    context.cpu->fpu.tags[0] = TAG_Valid;
    context.cpu->eip.u32 = TEST_CODE_ADDRESS + 4 - context.cpu->seg[CS].address;
    context.cpu->nextOp = nullptr;
    testRunCPU();
    expectFp(0, 12);
    if (!interior->pfnJitCode || !interior->blockStart->pfnJitCode)
        testFail("native interior with uncompiled backward target was not promoted");

    // Legacy operations must expose the dirty cache and discard its mapping
    // before an instruction replaces the architectural FPU state.
    beginFp(3);
    fp(0xd9, 0xe8); fp(0xd8, 0xc0); // 2
    fp(0xd9, 0xfc); // frndint: legacy native lowering
    testPushCode8(0x0f); fpMem(0xae, 0, 0x100); // fxsave 2
    fp(0xd8, 0xc0); // 4
    testPushCode8(0x0f); fpMem(0xae, 1, 0x100); // fxrstor 2
    fp(0xd8, 0xc0); // 4
    testRunCPU();
    expectFp(0, 4);
    if (context.cpu->fpu.top != 2) testFail("native legacy FPU transition lost TOP");
}
#endif

#ifdef BOXEDWINE_WASM_JIT
void testWasmJitFpuInt64() {
    const S64 exactValues[] = {0, 1, -1, 0x20000000000001ll, -0x20000000000001ll,
        0x123456789abcdefll, 0x7fffffffffffffffll, -0x7fffffffffffffffll - 1};
    for (U32 top = 0; top < 8; ++top) {
        for (U32 rounding = 0; rounding < 4; ++rounding) {
            for (bool truncate : {false, true}) {
                for (U32 address : {0x100u, 0x1fffu}) {
                    for (S64 value : exactValues) {
                        beginFp(top);
                        auto& context = testContext();
                        context.cpu->fpu.SetCW(0x037f | (rounding << 10));
                        context.memory->writeq(TEST_HEAP_ADDRESS + 0x500, value);
                        fp(0xd9, 0xe8); fp(0xd8, 0xc0); // dirty sentinel 2
                        fpMem(0xdf, 5, 0x500); // exact FILD qword
                        testPushCode8(0xb8); testPushCode32(0xffffffff);
                        fp(0x83, 0xc0); testPushCode8(1); // CF=1
                        fpMem(truncate ? 0xdd : 0xdf, truncate ? 1 : 7, address);
                        fp(0x83, 0xd0); testPushCode8(0); // adc eax,0
                        fp(0xd8, 0xc0); // sentinel remains cached, now 4
                        testRunCPU();
                        if (context.memory->readq(TEST_HEAP_ADDRESS + address) != (U64)value)
                            testFail("exact cached i64 round trip value=%llx rounding=%u truncate=%u address=%x", (U64)value, rounding, truncate, address);
                        expectFp(0, 4);
                        if (context.cpu->reg[0].u32 != 1 || context.cpu->fpu.top != ((top + 7) & 7))
                            testFail("cached i64 helper changed flags/TOP or other values");
                        requireFpJit();
                    }
                }
            }
        }
    }

    const S64 indefinite = -0x7fffffffffffffffll - 1;
    const struct { double input; S64 rounded[4]; } cases[] = {
        {1.5, {2,1,2,1}}, {-1.5, {-2,-2,-1,-1}},
        {2.5, {2,2,3,2}}, {-2.5, {-2,-3,-2,-2}},
        {2147483648.5, {2147483648ll,2147483648ll,2147483649ll,2147483648ll}},
        {-2147483649.5, {-2147483650ll,-2147483650ll,-2147483649ll,-2147483649ll}},
        {9223372036854774784.0, {9223372036854774784ll,9223372036854774784ll,9223372036854774784ll,9223372036854774784ll}},
        {-9223372036854775808.0, {indefinite,indefinite,indefinite,indefinite}},
        {9223372036854775808.0, {indefinite,indefinite,indefinite,indefinite}},
        {std::numeric_limits<double>::infinity(), {indefinite,indefinite,indefinite,indefinite}},
        {-std::numeric_limits<double>::infinity(), {indefinite,indefinite,indefinite,indefinite}},
        {std::numeric_limits<double>::quiet_NaN(), {indefinite,indefinite,indefinite,indefinite}}
    };
    for (U32 rounding = 0; rounding < 4; ++rounding) {
        for (bool truncate : {false, true}) {
            for (U32 address : {0x100u, 0x1fffu}) {
                for (const auto& item : cases) {
                    beginFp(3);
                    auto& context = testContext();
                    context.cpu->fpu.SetCW(0x037f | (rounding << 10));
                    U64 bits; memcpy(&bits, &item.input, sizeof(bits));
                    context.memory->writeq(TEST_HEAP_ADDRESS + 0x500, bits);
                    fp(0xd9, 0xe8); fp(0xd8, 0xc0); // sentinel 2
                    fpMem(0xdd, 0, 0x500); fp(0xd9, 0xc0); // duplicate
                    fpMem(truncate ? 0xdd : 0xdf, truncate ? 1 : 7, address);
                    fpMem(0xdd, 3, 0x300); // surviving value must not be rounded
                    fp(0xd8, 0xc0);
                    testRunCPU();
                    if (context.memory->readq(TEST_HEAP_ADDRESS + address) != (U64)item.rounded[truncate ? 3 : rounding])
                        testFail("cached i64 conversion input=%.17g rounding=%u truncate=%u address=%x", item.input, rounding, truncate, address);
                    if (context.memory->readq(TEST_HEAP_ADDRESS + 0x300) != bits)
                        testFail("i64 conversion rounded another cached value");
                    expectFp(0, 4);
                    requireFpJit();
                }
            }
        }
    }

    // Incoming extended/double alternatives through FCMOV must select the
    // right integer-store arm without converting an untaken destination.
    for (U32 top = 0; top < 8; ++top) {
        for (bool taken : {false, true}) {
            beginFp(top);
            auto& context = testContext();
            context.cpu->flags = taken ? CF : 0;
            context.cpu->fpu.FLD_I64(0x20000000000001ll, top);
            context.cpu->fpu.tags[top] = TAG_Valid;
            context.cpu->fpu.FLD_I64(7, (top + 1) & 7);
            context.cpu->fpu.tags[(top + 1) & 7] = TAG_Valid;
            fp(0xda, 0xc1); // fcmovb st0,st1
            fpMem(0xdf, 7, 0x100);
            testRunCPU();
            if (context.memory->readq(TEST_HEAP_ADDRESS + 0x100) != (taken ? 7ull : 0x20000000000001ull))
                testFail("conditional validity lost during i64 store");
            expectFp(0, 7);
        }

        // A header slot starts as a double, then FILD makes it extended on
        // every backedge. Its first arithmetic use must validate every time.
        beginFp(top);
        auto& context = testContext();
        context.memory->writeq(TEST_HEAP_ADDRESS + 0x500, 0x20000000000001ull);
        fp(0xd9, 0xe8); fp(0xd8, 0xc0); fp(0xd9, 0xe8); // 1,2
        testPushCode8(0xb9); testPushCode32(31);
        U32 loop = context.codeIp;
        fp(0xd8, 0xc1); // add sentinel 2
        fpMem(0xdf, 7, 0x100); // pop current value
        fpMem(0xdf, 5, 0x500); // replace it with a lazy exact integer
        testPushCode8(0x49); testPushCode8(0x75);
        testPushCode8((U8)(loop - (context.codeIp + 1)));
        fpMem(0xdf, 7, 0x108);
        testRunCPU();
        if (context.memory->readq(TEST_HEAP_ADDRESS + 0x100) != 0x20000000000002ull ||
            context.memory->readq(TEST_HEAP_ADDRESS + 0x108) != 0x20000000000001ull)
            testFail("cached i64 loop lost extended/double validity");
        expectFp(0, 2);
        requireFpJit();
    }

    // A code-page qword store must invalidate and execute its pop once.
    beginFp(3);
    auto& context = testContext();
    context.memory->writeq(TEST_HEAP_ADDRESS + 0x500, 0x9090909090909090ull);
    fp(0xd9, 0xe8); fp(0xd8, 0xc0);
    fpMem(0xdf, 5, 0x500);
    U32 target = context.codeIp - TEST_CODE_ADDRESS + 7;
    testPushCode8(0x2e); fpMem(0xdf, 7, target);
    for (U32 i = 0; i < 5; ++i) fp(0xd8, 0xc0);
    testRunCPU();
    expectFp(0, 4);
    if (context.cpu->fpu.top != 2) testFail("code-page i64 store popped incorrectly");
}

void testWasmJitFpuRawInt64() {
    const U16 exponents[] = {0,1,0x3fbd,0x3fbe,0x3ffd,0x3ffe,0x3fff,0x4000,
        0x401e,0x4033,0x4034,0x403d,0x403e,0x403f,0x7ffe,0x7fff};
    const U64 significands[] = {0,1,0x4000000000000000ull,0x7fffffffffffffffull,
        0x8000000000000000ull,0x8000000000000001ull,0xbfffffffffffffffull,
        0xc000000000000000ull,0xc000000000000001ull,0xffffffffffffffffull};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    U32 sequence = 0;
    for (U16 exponent : exponents) for (U64 significand : significands) {
        for (bool negative : {false,true}) for (U32 rounding = 0; rounding < 4; ++rounding) {
            for (bool truncate : {false,true}) for (U32 output : {0x100u,0x1fffu}) {
                U32 top = (sequence++ >> 1) & 7;
                beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
                U16 high = exponent | (negative ? 0x8000 : 0);
                fpu.LD80(top, significand, high); fpu.tags[top] = TAG_Valid;
                fpu.SetCW(0x037f | (rounding << 10));
                softfloat_exceptionFlags = softfloat_flag_infinite;
                S64 expected = fpu.toInt64(top, truncate);
                U8 flags = softfloat_exceptionFlags;
                fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xda); // dirty neighbor 2
                context.cpu->xmm[0].pi.u32[0] = 0x3f800000;
                context.cpu->xmm[0].pi.u32[1] = 0x12345678;
                testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
                testPushCode8(0xb8); testPushCode32(0xffffffff);
                fp(0x83,0xc0); testPushCode8(1);
                context.memory->writeb(TEST_HEAP_ADDRESS + output - 1, 0xa5);
                context.memory->writeb(TEST_HEAP_ADDRESS + output + 8, 0x5a);
                fpMem(truncate ? 0xdd : 0xdf, truncate ? 1 : 7, output);
                fp(0x83,0xd0); testPushCode8(0);
                testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
                fp(0xd8,0xc0);
                softfloat_exceptionFlags = softfloat_flag_infinite;
                softfloat_roundingMode = softfloat_round_max;
                testRunCPU();
                if (context.memory->readq(TEST_HEAP_ADDRESS + output) != (U64)expected ||
                    context.memory->readb(TEST_HEAP_ADDRESS + output - 1) != 0xa5 ||
                    context.memory->readb(TEST_HEAP_ADDRESS + output + 8) != 0x5a ||
                    softfloat_exceptionFlags != flags || softfloat_roundingMode != softfloat_round_max)
                    testFail("raw i64 result/flags exp=%x sig=%llx round=%u trunc=%u output=%x flags=%u expected=%u",high,significand,rounding,truncate,output,softfloat_exceptionFlags,flags);
                if (fpu.top != ((top + 1) & 7) || fpu.tags[top] != TAG_Empty || fpu.isRegCached[top] ||
                    fpu.regs[top].signif != significand || fpu.regs[top].signExp != high ||
                    context.cpu->reg[0].u32 != 1 || context.cpu->xmm[0].pi.u32[0] != 0x40800000 ||
                    context.cpu->xmm[0].pi.u32[1] != 0x12345678)
                    testFail("raw i64 store changed source/TOP/GP/XMM");
                expectFp(0,4); requireFpJit();
            }
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuIntegerFaults() {
    const struct { U8 opcode; U8 group; } ops[] = {
        {0xdf,1}, {0xdf,2}, {0xdf,3}, {0xdf,5}, {0xdf,7}, {0xdd,1}
    };
    for (const auto& op : ops) {
        for (U32 address : {0x2000u, 0x1fffu}) {
            for (bool extended : {false, true}) {
                beginFp(3);
                auto& context = testContext();
                context.memory->writeq(TEST_HEAP_ADDRESS + address, 0x1122334455667788ull);
                context.memory->writeq(TEST_HEAP_ADDRESS + 0x500, 0x20000000000001ull);
                fp(0xd9, 0xe8); fp(0xd8, 0xc0); // unrelated dirty 2
                if (extended) fpMem(0xdf, 5, 0x500);
                else fp(0xd9, 0xe8);
                fpMem(op.opcode, op.group, address);
                fp(0xd8, 0xc0); // must not execute
                auto oldAction = context.process->sigActions[K_SIGSEGV];
                auto& action = context.process->sigActions[K_SIGSEGV];
                action.reset(); action.handlerAndSigAction = context.codeIp;
                context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, 0);
                testRunCPU();
                context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
                if (action.sigInfo[0] != K_SIGSEGV || context.cpu->fpu.top != 1 ||
                    context.memory->readq(TEST_HEAP_ADDRESS + address) != 0x1122334455667788ull)
                    testFail("faulting cached integer operation committed store or stack change");
                if (extended && (context.cpu->fpu.isRegCached[1] || context.cpu->fpu.toInt64(1, true) != 0x20000000000001ll))
                    testFail("fault converted untouched extended operand");
                if (!extended) expectFp(0, 1);
                expectFp(1, 2);
                context.process->sigActions[K_SIGSEGV] = oldAction;
            }
        }
    }
}

namespace {
void makeBcd(S64 number, U8 data[10]) {
    U64 value = number < 0 ? 0 - (U64)number : (U64)number;
    for (U32 i = 0; i < 9; ++i) {
        data[i] = value % 10; value /= 10;
        data[i] |= (value % 10) << 4; value /= 10;
    }
    data[9] = number < 0 ? 0x80 : 0;
}
void writeTen(U32 address, const U8 data[10]) {
    for (U32 i = 0; i < 10; ++i) testContext().memory->writeb(TEST_HEAP_ADDRESS + address + i, data[i]);
}
void expectTen(U32 address, const U8 data[10]) {
    for (U32 i = 0; i < 10; ++i) {
        if (testContext().memory->readb(TEST_HEAP_ADDRESS + address + i) != data[i])
            testFail("x87 ten-byte transfer changed byte %u at %x", i, address);
    }
}
}

void testWasmJitFpuExtended() {
    const struct { U64 low; U16 high; } values[] = {
        {0,0}, {0,0x8000}, {0x8000000000000000ull,0x3fff},
        {0x8123456789abcdefull,0x3fff}, {0xabcdef0123456789ull,0xbfff},
        {1,0}, {0xffffffffffffffffull,0x7ffe},
        {0x8000000000000000ull,0x7fff}, {0xc123456789abcdefull,0x7fff},
        {0x8123456789abcdefull,0x7fff}
    };
    for (U32 top = 0; top < 8; ++top) {
        for (U32 input : {0x100u, 0x1ff6u, 0x1fffu}) {
            for (U32 output : {0x500u, 0x3ff6u, 0x3fffu}) {
                for (const auto& value : values) {
                    beginFp(top);
                    auto& context = testContext();
                    context.memory->writeq(TEST_HEAP_ADDRESS + input, value.low);
                    context.memory->writew(TEST_HEAP_ADDRESS + input + 8, value.high);
                    context.memory->writeb(TEST_HEAP_ADDRESS + output - 1, 0xa5);
                    context.memory->writeb(TEST_HEAP_ADDRESS + output + 10, 0x5a);
                    fp(0xd9, 0xe8); fp(0xd8, 0xc0); // unrelated dirty 2
                    fpMem(0xdb, 5, input); // fld tbyte
                    testPushCode8(0xb8); testPushCode32(0xffffffff);
                    fp(0x83, 0xc0); testPushCode8(1); // CF=1
                    fpMem(0xdb, 7, output); // fstp tbyte
                    fp(0x83, 0xd0); testPushCode8(0); // consume preserved carry
                    fp(0xd8, 0xc0);
                    testRunCPU();
                    if (context.memory->readq(TEST_HEAP_ADDRESS + output) != value.low ||
                        context.memory->readw(TEST_HEAP_ADDRESS + output + 8) != value.high ||
                        context.memory->readb(TEST_HEAP_ADDRESS + output - 1) != 0xa5 ||
                        context.memory->readb(TEST_HEAP_ADDRESS + output + 10) != 0x5a)
                        testFail("cached 80-bit transfer lost exact payload or touched neighbor bytes");
                    if (context.cpu->reg[0].u32 != 1 || context.cpu->fpu.top != ((top + 7) & 7))
                        testFail("cached 80-bit helper changed flags/TOP");
                    expectFp(0, 4);
                    requireFpJit();
                }
            }
        }

        // Dirty doubles, FXCH renaming, and untouched raw alternatives at a
        // conditional join must all feed the physical-slot store correctly.
        for (bool taken : {false, true}) {
            beginFp(top);
            auto& context = testContext();
            context.cpu->flags = taken ? CF : 0;
            context.memory->writeq(TEST_HEAP_ADDRESS + 0x100, 0x8123456789abcdefull);
            context.memory->writew(TEST_HEAP_ADDRESS + 0x108, 0x3fff);
            fp(0xd9, 0xe8); fp(0xd8, 0xc0); // 2
            fp(0xd9, 0xe8); fp(0xd9, 0xc9); // renamed 2,1
            fpMem(0xdb, 5, 0x100); // raw,2,1
            fp(0xda, 0xc1); // move 2 only if CF
            fpMem(0xdb, 7, 0x500);
            fp(0xde, 0xc1); // 3
            testRunCPU();
            if (context.memory->readq(TEST_HEAP_ADDRESS + 0x500) != (taken ? 0x8000000000000000ull : 0x8123456789abcdefull) ||
                context.memory->readw(TEST_HEAP_ADDRESS + 0x508) != (taken ? 0x4000 : 0x3fff))
                testFail("conditional/renamed extended store selected stale value");
            expectFp(0, 3);
        }

        // The loop header starts with a double, but each backedge replaces it
        // with extended 1.5. The next arithmetic use must revalidate the slot.
        beginFp(top);
        auto& context = testContext();
        context.memory->writeq(TEST_HEAP_ADDRESS + 0x100, 0xc000000000000000ull);
        context.memory->writew(TEST_HEAP_ADDRESS + 0x108, 0x3fff);
        fp(0xd9, 0xe8); fp(0xd8, 0xc0); fp(0xd9, 0xe8); // 1,2
        testPushCode8(0xb9); testPushCode32(31);
        U32 loop = context.codeIp;
        fp(0xd8, 0xc1);
        fpMem(0xdb, 7, 0x500); // 3.5 after first iteration
        fpMem(0xdb, 5, 0x100); // raw 1.5
        testPushCode8(0x49); testPushCode8(0x75);
        testPushCode8((U8)(loop - (context.codeIp + 1)));
        fpMem(0xdb, 7, 0x510);
        testRunCPU();
        if (context.memory->readq(TEST_HEAP_ADDRESS + 0x500) != 0xe000000000000000ull ||
            context.memory->readw(TEST_HEAP_ADDRESS + 0x508) != 0x4000 ||
            context.memory->readq(TEST_HEAP_ADDRESS + 0x510) != 0xc000000000000000ull ||
            context.memory->readw(TEST_HEAP_ADDRESS + 0x518) != 0x3fff)
            testFail("80-bit loop lost raw/double validity");
        expectFp(0, 2);
        requireFpJit();
    }
}

void testWasmJitFpuBcd() {
    const S64 values[] = {0,1,-1,9007199254740993ll,-9007199254740993ll,
        999999999999999999ll,-987654321098765432ll};
    for (U32 top = 0; top < 8; ++top) {
        for (U32 input : {0x100u,0x1ff6u,0x1fffu}) {
            for (U32 output : {0x500u,0x3ff6u,0x3fffu}) {
                for (S64 number : values) {
                    beginFp(top);
                    auto& context = testContext();
                    U8 data[10]; makeBcd(number,data); writeTen(input,data);
                    context.memory->writeb(TEST_HEAP_ADDRESS + output - 1, 0xa5);
                    context.memory->writeb(TEST_HEAP_ADDRESS + output + 10, 0x5a);
                    fp(0xd9, 0xe8); fp(0xd8, 0xc0);
                    fpMem(0xdf, 4, input); // fbld
                    testPushCode8(0xb8); testPushCode32(0xffffffff);
                    fp(0x83, 0xc0); testPushCode8(1);
                    fpMem(0xdf, 6, output); // fbstp
                    fp(0x83, 0xd0); testPushCode8(0);
                    fp(0xd8, 0xc0);
                    testRunCPU();
                    expectTen(output,data); expectFp(0,4);
                    if (context.cpu->reg[0].u32 != 1 || context.cpu->fpu.top != ((top + 7) & 7) ||
                        context.memory->readb(TEST_HEAP_ADDRESS + output - 1) != 0xa5 ||
                        context.memory->readb(TEST_HEAP_ADDRESS + output + 10) != 0x5a)
                        testFail("BCD helper changed flags/TOP or neighboring bytes");
                    requireFpJit();
                }
            }
        }
    }

    // Preserve the existing conversion policy for doubles and extended values
    // across both the cached helper and cross-page interpreter paths.
    for (U32 rounding = 0; rounding < 4; ++rounding) {
        for (bool negative : {false,true}) {
            for (bool extended : {false,true}) {
                for (U32 output : {0x500u,0x1fffu}) {
                    beginFp(3);
                    auto& context = testContext();
                    context.cpu->fpu.SetCW(0x037f | (rounding << 10));
                    fp(0xd9, 0xe8); fp(0xd8, 0xc0); // sentinel 2
                    if (extended) {
                        context.memory->writeq(TEST_HEAP_ADDRESS + 0x100,0xa000000000000000ull);
                        context.memory->writew(TEST_HEAP_ADDRESS + 0x108, negative ? 0xc000 : 0x4000);
                        fpMem(0xdb,5,0x100); // +/- 2.5
                    } else {
                        context.memory->writeq(TEST_HEAP_ADDRESS + 0x100,negative ? 0xc004000000000000ull : 0x4004000000000000ull);
                        fpMem(0xdd,0,0x100);
                        fp(0xd9,0xe0); fp(0xd9,0xe0); // dirty cached double
                    }
                    fpMem(0xdf,6,output);
                    fp(0xd8,0xc0);
                    testRunCPU();
                    S64 result = extended && ((negative && rounding == 1) || (!negative && rounding == 2)) ? 3 : 2;
                    U8 data[10]; makeBcd(negative ? -result : result,data); expectTen(output,data);
                    expectFp(0,4);
                }
            }
        }
    }

    // BCD loads also replace an initially-double loop slot with an exact value.
    beginFp(3);
    auto& context = testContext();
    U8 data[10]; makeBcd(7,data); writeTen(0x100,data);
    fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xd9,0xe8); // 1,2
    testPushCode8(0xb9); testPushCode32(31);
    U32 loop = context.codeIp;
    fp(0xd8,0xc1); fpMem(0xdf,6,0x500); fpMem(0xdf,4,0x100);
    testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop-(context.codeIp+1)));
    fpMem(0xdf,7,0x600);
    testRunCPU();
    makeBcd(9,data); expectTen(0x500,data);
    if (context.memory->readq(TEST_HEAP_ADDRESS+0x600)!=7) testFail("BCD loop lost exact reload");
    expectFp(0,2); requireFpJit();
}

void testWasmJitFpuExtendedStore() {
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    const U64 classes[] = {0, 1, 2, 0x0008000000000000ull, 0x000fffffffffffffull,
        0x0010000000000000ull, 0x0010000000000001ull, 0x3ff0000000000000ull,
        0x3ff0000000000001ull, 0x7fefffffffffffffull, 0x7ff0000000000000ull,
        0x7ff0000000000001ull, 0x7ff7ffffffffffffull, 0x7ff8000000000000ull,
        0x7ff8123456789abcull, 0x7fffffffffffffffull};
    U64 random = 0xe7037ed1a0b428dbull;
    for (U32 sequence = 0; sequence < 128; ++sequence) {
        random ^= random << 13; random ^= random >> 7; random ^= random << 17;
        U64 bits = sequence < 64 ? classes[sequence & 15] | ((sequence & 16) ? 1ull << 63 : 0) : random;
        for (U32 output : {0x500u, 0x1ff6u, 0x1fffu}) {
            U32 top = sequence & 7;
            beginFp(top);
            auto& context = testContext(); auto& fpu = context.cpu->fpu;
            fpu.regCache[top].l = bits; fpu.isRegCached[top] = true; fpu.tags[top] = TAG_Valid;
            fpu.SetCW(0x037f | ((sequence & 3) << 10));
            FPU expected = fpu;
            softfloat_exceptionFlags = softfloat_flag_infinite;
            U64 low, high; expected.ST80(top, &low, &high);
            U8 flags = softfloat_exceptionFlags;
            fp(0xd9, 0xe8); fp(0xd8, 0xc0); fp(0xdd, 0xda); // dirty neighbor 2
            if (sequence & 32) {
                context.memory->writeq(TEST_HEAP_ADDRESS + 0x100, bits);
                fpMem(0xdd, 0, 0x100); fp(0xdd, 0xd9); // dirty local source
            }
            context.cpu->xmm[0].pi.u32[0] = 0x3f800000;
            context.cpu->xmm[0].pi.u32[1] = 0x12345678;
            testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58, 0xc0);
            testPushCode8(0xb8); testPushCode32(0xffffffff);
            fp(0x83, 0xc0); testPushCode8(1);
            context.memory->writeb(TEST_HEAP_ADDRESS + output - 1, 0xa5);
            context.memory->writeb(TEST_HEAP_ADDRESS + output + 10, 0x5a);
            fpMem(0xdb, 7, output);
            fp(0x83, 0xd0); testPushCode8(0);
            testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58, 0xc0);
            fp(0xd8, 0xc0); // dirty neighbor survives the store
            softfloat_exceptionFlags = softfloat_flag_infinite;
            softfloat_roundingMode = softfloat_round_max;
            testRunCPU();
            if (context.memory->readq(TEST_HEAP_ADDRESS + output) != low ||
                context.memory->readw(TEST_HEAP_ADDRESS + output + 8) != high ||
                context.memory->readb(TEST_HEAP_ADDRESS + output - 1) != 0xa5 ||
                context.memory->readb(TEST_HEAP_ADDRESS + output + 10) != 0x5a ||
                fpu.regs[top].signif != low || fpu.regs[top].signExp != high || fpu.isRegCached[top] ||
                fpu.top != ((top + 1) & 7) || fpu.tags[top] != TAG_Empty)
                testFail("inline extended store bits/retired slot sequence=%u output=%x", sequence, output);
            if (softfloat_exceptionFlags != flags || softfloat_roundingMode != softfloat_round_max ||
                context.cpu->reg[0].u32 != 1 || context.cpu->xmm[0].pi.u32[0] != 0x40800000 ||
                context.cpu->xmm[0].pi.u32[1] != 0x12345678)
                testFail("inline extended store changed flags/mode/GP/XMM sequence=%u", sequence);
            expectFp(0, 4); requireFpJit();
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuExtendedStoreLoops() {
    for (U32 top = 0; top < 8; ++top) {
        for (bool raw : {false, true}) {
            for (U32 count : {1u, 2u, 35u}) {
                beginFp(top);
                auto& context = testContext(); auto& fpu = context.cpu->fpu;
                fpu.tags[top] = TAG_Valid;
                if (raw) fpu.LD80(top, 0xa000000000000000ull, 0x3fff);
                else { fpu.regCache[top].d = 1.25; fpu.isRegCached[top] = true; }
                context.cpu->reg[1].u32 = count;
                U32 loop = context.codeIp;
                fp(0xd8, 0xc0);
                fpMem(0xdb, 7, 0x500);
                fp(0xd9, 0xf6); // revisit the raw value left in the popped slot
                testPushCode8(0x49); testPushCode8(0x75);
                testPushCode8((U8)(loop - context.codeIp - 1));
                // FSTP is the only instruction in this loop that changes a
                // cached double to raw storage. TOP has no net change.
                testRunCPU();
                if (fpu.top != top || fpu.isRegCached[top] || fpu.tags[top] != TAG_Empty ||
                    fpu.regs[top].signif != 0xa000000000000000ull || fpu.regs[top].signExp != 0x3fff + count ||
                    context.memory->readq(TEST_HEAP_ADDRESS + 0x500) != fpu.regs[top].signif ||
                    context.memory->readw(TEST_HEAP_ADDRESS + 0x508) != fpu.regs[top].signExp || context.cpu->reg[1].u32)
                    testFail("inline extended store loop lost raw validity top=%u raw=%u count=%u", top, raw, count);
                requireFpJit();
            }
        }
    }
}

void testWasmJitFpuBcdDecode() {
    U64 random = 0xa0761d6478bd642full;
    for (U32 sequence = 0; sequence < 256; ++sequence) {
        U8 data[10];
        for (U32 byte = 0; byte < 9; ++byte) {
            random ^= random << 13; random ^= random >> 7; random ^= random << 17;
            data[byte] = sequence < 16 ? 0 : sequence < 32 ? 0x99 : (U8)random;
        }
        data[9] = (U8)sequence;
        // Use unsigned accumulation to define modulo-64 overflow, avoiding
        // signed overflow in the oracle for the interpreter's permissive BCD.
        U64 magnitude = data[9] & 15;
        for (S32 byte = 8; byte >= 0; --byte)
            magnitude = magnitude * 100 + (data[byte] >> 4) * 10 + (data[byte] & 15);
        U64 bits = (data[9] & 0x80) ? 0 - magnitude : magnitude;
        S64 number; memcpy(&number, &bits, sizeof(number));
        for (U32 input : {0x100u, 0x1ff6u, 0x1fffu}) {
            beginFp(sequence & 7);
            auto& context = testContext(); auto& fpu = context.cpu->fpu;
            U32 destination = (fpu.top + 7) & 7;
            FPU expected = fpu; expected.FLD_I64(number, destination);
            writeTen(input, data);
            fp(0xd9, 0xe8); fp(0xd8, 0xc0); fp(0xdd, 0xda); // dirty neighbor 2
            testPushCode8(0xb8); testPushCode32(0xffffffff);
            fp(0x83, 0xc0); testPushCode8(1);
            fpMem(0xdf, 4, input);
            fp(0x83, 0xd0); testPushCode8(0);
            testRunCPU();
            if (fpu.top != destination || fpu.tags[destination] != TAG_Valid || fpu.isRegCached[destination] ||
                fpu.regs[destination].signif != expected.regs[destination].signif ||
                fpu.regs[destination].signExp != expected.regs[destination].signExp || context.cpu->reg[0].u32 != 1)
                testFail("inline BCD decode lost exact integer sequence=%u input=%x", sequence, input);
            expectFp(2, 2); expectTen(input, data); requireFpJit();
        }
    }
}

void testWasmJitFpuBcdStore() {
    const U64 doubles[] = {0,1,0x000fffffffffffffull,0x3fe0000000000000ull,
        0x4004000000000000ull,0x400c000000000000ull,0x4340000000000000ull,
        0x43abc16d674ec800ull,0x43dfffffffffffffull,0x43e0000000000000ull,
        0x7fefffffffffffffull,0x7ff0000000000000ull,0x7ff0000000000001ull,
        0x7ff8000000000000ull,0x7ff8123456789abcull,0x7fffffffffffffffull};
    const U16 exponents[] = {0,1,0x3fbe,0x3ffd,0x3ffe,0x3fff,0x4000,0x4001,
        0x401e,0x4033,0x4034,0x403c,0x403d,0x403e,0x403f,0x7fff};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    U64 random = 0xe7037ed1a0b428dbull;
    for (U32 sequence = 0; sequence < 96; ++sequence) {
        random ^= random << 13; random ^= random >> 7; random ^= random << 17;
        for (bool raw : {false,true}) for (U32 rounding = 0; rounding < 4; ++rounding) {
            for (U32 output : {0x500u,0x1ff6u,0x1fffu}) {
                U32 top = sequence & 7;
                beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
                fpu.SetCW(0x037f | (rounding << 10)); fpu.tags[top] = TAG_Valid;
                U64 bits = sequence < 32 ? doubles[sequence & 15] | ((sequence & 16) ? 1ull << 63 : 0) : random;
                U64 sig = sequence < 32 ? 0xa000000000000000ull : random;
                U16 high = exponents[sequence & 15] | ((sequence & 16) ? 0x8000 : 0);
                if (raw) fpu.LD80(top, sig, high);
                else { fpu.regCache[top].l = bits; fpu.isRegCached[top] = true; }
                softfloat_exceptionFlags = softfloat_flag_infinite;
                U8 expected[10]; fpu.FBST(top, expected);
                U8 flags = softfloat_exceptionFlags;
                fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xda);
                if (!raw) {
                    context.memory->writeq(TEST_HEAP_ADDRESS + 0x100, bits);
                    fpMem(0xdd,0,0x100); fp(0xdd,0xd9); // dirty, unrounded local
                }
                testPushCode8(0xb8); testPushCode32(0xffffffff);
                fp(0x83,0xc0); testPushCode8(1);
                context.memory->writeb(TEST_HEAP_ADDRESS + output - 1, 0xa5);
                context.memory->writeb(TEST_HEAP_ADDRESS + output + 10, 0x5a);
                fpMem(0xdf,6,output);
                fp(0x83,0xd0); testPushCode8(0);
                fp(0xd8,0xc0);
                softfloat_exceptionFlags = softfloat_flag_infinite;
                softfloat_roundingMode = softfloat_round_max;
                testRunCPU();
                expectTen(output, expected); expectFp(0,4); requireFpJit();
                if (fpu.top != ((top + 1) & 7) || fpu.tags[top] != TAG_Empty ||
                    (raw ? fpu.isRegCached[top] || fpu.regs[top].signif != sig || fpu.regs[top].signExp != high :
                        !fpu.isRegCached[top] || fpu.regCache[top].l != bits) ||
                    context.memory->readb(TEST_HEAP_ADDRESS + output - 1) != 0xa5 ||
                    context.memory->readb(TEST_HEAP_ADDRESS + output + 10) != 0x5a || context.cpu->reg[0].u32 != 1 ||
                    softfloat_exceptionFlags != flags || softfloat_roundingMode != softfloat_round_max)
                    testFail("inline BCD store source/flags/top sequence=%u raw=%u rounding=%u output=%x",sequence,raw,rounding,output);
            }
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuBcdStoreLoops() {
    for (U32 top = 0; top < 8; ++top) for (bool taken : {false,true}) {
        for (U32 count : {1u,2u,31u}) {
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            fpu.SetCW(0x077f); // floor: raw -2.5 -> -3, cached +3.5 -> +3
            fpu.LD80(top,0xa000000000000000ull,0xc000); fpu.tags[top] = TAG_Valid;
            context.cpu->flags = taken ? CF : 0;
            context.cpu->reg[1].u32 = count;
            context.memory->writeq(TEST_HEAP_ADDRESS + 0x100,0x400c000000000000ull);
            fpMem(0xdd,0,0x100); fp(0xdd,0xda); // dirty +3.5 in ST1
            U32 loop = context.codeIp;
            fp(0xda,0xc1); fpMem(0xdf,6,0x500); fp(0xd9,0xf6);
            testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop-context.codeIp-1));
            testRunCPU();
            U8 expected[10]; makeBcd(taken ? 3 : -3,expected); expectTen(0x500,expected);
            if (fpu.top != top || fpu.tags[top] != TAG_Empty || context.cpu->reg[1].u32 ||
                (taken ? !fpu.isRegCached[top] || fpu.regCache[top].l != 0x400c000000000000ull :
                    fpu.isRegCached[top] || fpu.regs[top].signif != 0xa000000000000000ull || fpu.regs[top].signExp != 0xc000))
                testFail("BCD store loop lost conditional source top=%u taken=%u count=%u",top,taken,count);
            expectFp(1,3.5); requireFpJit();
        }
    }
}

void testWasmJitFpu80Faults() {
    for (bool bcd : {false,true}) {
        for (bool store : {false,true}) {
            for (U32 address : {0x2000u,0x1fffu,0x1ff8u,0x1ff6u}) {
                beginFp(3);
                auto& context = testContext();
                U8 initial[10]; makeBcd(123456,initial); writeTen(address,initial);
                U8 source[10]; makeBcd(9007199254740993ll,source); writeTen(0x500,source);
                fp(0xd9,0xe8); fp(0xd8,0xc0); // dirty 2
                if (store) fpMem(0xdf,4,0x500); // exact source
                fpMem(bcd ? 0xdf : 0xdb, bcd ? (store ? 6 : 4) : (store ? 7 : 5),address);
                auto oldAction = context.process->sigActions[K_SIGSEGV];
                auto& action = context.process->sigActions[K_SIGSEGV];
                action.reset(); action.handlerAndSigAction = context.codeIp;
                context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS+0x2000,K_PAGE_SIZE,0);
                testRunCPU();
                context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS+0x2000,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE);
                bool fault = address != 0x1ff6;
                if ((action.sigInfo[0] == K_SIGSEGV) != fault || context.cpu->fpu.top != (fault ? (store ? 1u : 2u) : (store ? 2u : 1u)))
                    testFail("ten-byte access fault/TOP mismatch bcd=%u store=%u address=%x",bcd,store,address);
                if (fault) expectTen(address,initial);
                if (fault && store && (context.cpu->fpu.isRegCached[1] || context.cpu->fpu.toInt64(1,true)!=9007199254740993ll))
                    testFail("faulting ten-byte store converted exact source");
                expectFp((store == fault) ? 1 : 0,2);
                context.process->sigActions[K_SIGSEGV] = oldAction;
            }
        }
    }

    // Write earlier bytes in the active code page, then execute the following
    // add. The interpreter must commit the store/pop once and exit stale code.
    for (bool bcd : {false,true}) {
        beginFp(3);
        auto& context = testContext();
        U8 source[10]; makeBcd(909090909090909090ll,source); writeTen(0x500,source);
        fp(0xd9,0xe8); fp(0xd8,0xc0); fpMem(0xdf,4,0x500);
        testPushCode8(0x2e); fpMem(bcd ? 0xdf : 0xdb,bcd ? 6 : 7,0);
        fp(0xd8,0xc0);
        testRunCPU();
        expectFp(0,4);
        if(context.cpu->fpu.top!=2) testFail("code-page ten-byte store popped twice");
    }
}

namespace {
void seedMathValue(FPU& fpu, U32 index, double value, bool extended) {
    union { double d; U64 bits; } v = {value};
    if (extended) {
        float64_sf f; f.v = v.bits;
        fpu.regs[index] = f64_to_extF80(f);
        fpu.isRegCached[index] = false;
    } else {
        fpu.regCache[index].l = v.bits;
        fpu.isRegCached[index] = true;
    }
    fpu.tags[index] = TAG_Valid;
}
void expectMathValue(U32 index, double expected) {
    auto& fpu = testContext().cpu->fpu;
    double actual = fpu.getF64(fpu.STV(index));
    if (std::isnan(expected) ? !std::isnan(actual) : actual != expected ||
        (actual == 0 && std::signbit(actual) != std::signbit(expected)))
        testFail("cached math ST(%u): %.17g, expected %.17g", index, actual, expected);
}
const U8 cachedMathOps[] = {0xf0,0xf1,0xf2,0xf3,0xf9,0xfb,0xfe,0xff};
bool mathPops(U8 op) { return op == 0xf1 || op == 0xf3 || op == 0xf9; }
bool mathPushes(U8 op) { return op == 0xf2 || op == 0xfb; }
void referenceMath(FPU& fpu, U8 op) {
    switch (op) {
    case 0xf0: fpu.F2XM1(); break;
    case 0xf1: fpu.FYL2X(); break;
    case 0xf2: fpu.FPTAN(); break;
    case 0xf3: fpu.FPATAN(); break;
    case 0xf9: fpu.FYL2XP1(); break;
    case 0xfb: fpu.FSINCOS(); break;
    case 0xfe: fpu.FSIN(); break;
    case 0xff: fpu.FCOS(); break;
    }
}
}

void testWasmJitFpuExamine() {
    const U64 doubles[] = {0,0x8000000000000000ull,0x3ff0000000000000ull,
        0xbff0000000000000ull,1,0x8000000000000001ull,0x7ff0000000000000ull,
        0xfff0000000000000ull,0x7ff8123456789abcull,0xfff0123456789abcull};
    const struct { U64 low; U16 high; } raw[] = {{0,0},{0,0x8000},
        {0x8000000000000000ull,0x3fff},{0x8000000000000000ull,0xbfff},
        {1,0},{1,0x8000},{0x8000000000000000ull,0x7ffe},
        {0x8000000000000000ull,0x7fff},{0,0x7fff},
        {0xc123456789abcdefull,0x7fff},{0x8123456789abcdefull,0xffff},
        {0x0123456789abcdefull,0x3fff}};
    for (U32 top = 0; top < 8; ++top) {
        for (U32 tag = 0; tag < 4; ++tag) {
            for (bool extended : {false,true}) {
                U32 count = extended ? sizeof(raw)/sizeof(raw[0]) : sizeof(doubles)/sizeof(doubles[0]);
                for (U32 i = 0; i < count; ++i) {
                    beginFp(top);
                    auto& context = testContext(); auto& fpu = context.cpu->fpu;
                    fpu.tags[top] = tag; fpu.sw = 0x8755;
                    if (extended) fpu.LD80(top,raw[i].low,raw[i].high);
                    else { fpu.regCache[top].l = doubles[i]; fpu.isRegCached[top] = true; }
                    FPU expected = fpu; expected.FXAM();
                    // Dirty neighbor without reading or converting the operand.
                    fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xda); // fstp st2
                    testPushCode8(0xb8); testPushCode32(0xffffffff);
                    fp(0x83,0xc0); testPushCode8(1);
                    fp(0xd9,0xe5); // fxam
                    fp(0x83,0xd0); testPushCode8(0);
                    testRunCPU();
                    if ((fpu.sw & 0x4700) != (expected.sw & 0x4700) || fpu.top != top ||
                        fpu.tags[top] != tag || context.cpu->reg[0].u32 != 1)
                        testFail("FXAM classification/tag/TOP/flags mismatch extended=%u i=%u tag=%u",extended,i,tag);
                    if (extended ? fpu.isRegCached[top] || fpu.regs[top].signif != raw[i].low ||
                        fpu.regs[top].signExp != raw[i].high : !fpu.isRegCached[top] || fpu.regCache[top].l != doubles[i])
                        testFail("FXAM changed the classified representation");
                    expectFp(1,2); requireFpJit();
                }
            }
        }
        // A conditional move selects a dirty negative double or an exact raw
        // NaN. The classifier must follow runtime validity without converting.
        for (bool taken : {false,true}) {
            beginFp(top); auto& context=testContext();
            context.cpu->flags=taken ? CF : 0;
            context.memory->writeq(TEST_HEAP_ADDRESS+0x100,0x8123456789abcdefull);
            context.memory->writew(TEST_HEAP_ADDRESS+0x108,0x7fff);
            fp(0xd9,0xe8); fp(0xd9,0xe0); // -1
            fpMem(0xdb,5,0x100); fp(0xda,0xc1); fp(0xd9,0xe5);
            testRunCPU();
            if ((context.cpu->fpu.sw & 0x4700) != (taken ? 0x0600u : 0x0100u))
                testFail("FXAM ignored conditional raw/double validity");
            if (!taken && context.cpu->fpu.isRegCached[context.cpu->fpu.top])
                testFail("FXAM converted untouched conditional NaN");
        }
    }
}

void testWasmJitFpuMath() {
    const double inputs[] = {0.0,-0.0,0.25,-0.5,1.0,-1.0,2.0,-2.0,1000.0,0x1p70,
        0x1p-1074,std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()};
    for (U32 top=0; top<8; ++top) {
        for (U8 op : cachedMathOps) {
            for (bool extended : {false,true}) {
                for (double x : inputs) {
                    for (double y : {0.0,-0.0,0.75,-3.0}) {
                        beginFp(top); auto& context=testContext(); auto& fpu=context.cpu->fpu;
                        seedMathValue(fpu,top,x,extended);
                        seedMathValue(fpu,fpu.STV(1),y,extended);
                        fpu.sw=0x8755;
                        FPU expected=fpu; referenceMath(expected,op);
                        U32 values=mathPushes(op) ? 3 : mathPops(op) ? 1 : 2;
                        // Keep an unrelated dirty x87 slot and XMM register live.
                        fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
                        context.cpu->xmm[0].pi.u32[0]=0x3f800000;
                        context.cpu->xmm[0].pi.u32[1]=0x12345678;
                        testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
                        testPushCode8(0xb8); testPushCode32(0xffffffff);
                        fp(0x83,0xc0); testPushCode8(1);
                        fp(0xd9,op);
                        fp(0x83,0xd0); testPushCode8(0);
                        testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
                        testRunCPU();
                        if (fpu.top!=expected.top || (fpu.sw & ~0x3800u)!=(expected.sw & ~0x3800u) ||
                            context.cpu->reg[0].u32!=1 || context.cpu->xmm[0].pi.u32[0]!=0x40800000 ||
                            context.cpu->xmm[0].pi.u32[1]!=0x12345678)
                            testFail("math opcode %x changed TOP/status/GP/XMM",op);
                        for (U32 i=0;i<values;++i) expectMathValue(i,expected.getF64(expected.STV(i)));
                        expectFp(values,2);
                        requireFpJit();
                    }
                }
            }
        }
    }
}

void testWasmJitFpuMathSequences() {
    for (U32 top = 0; top < 8; ++top) {
        beginFp(top); auto& context = testContext();
        context.memory->writeq(TEST_HEAP_ADDRESS + 0x100,0x8000000000000000ull);
        context.memory->writew(TEST_HEAP_ADDRESS + 0x108,0x3fff); // extended 1
        fpMem(0xdb,5,0x100); fp(0xd9,0xfe); fpMem(0xdf,6,0x200); // FLD80; FSIN; FBSTP
        testRunCPU(); requireFpJit();
        if (context.memory->readq(TEST_HEAP_ADDRESS + 0x200) != 1 ||
            context.memory->readw(TEST_HEAP_ADDRESS + 0x208) || context.cpu->fpu.top != top)
            testFail("raw FSIN followed by FBSTP must round sin(1) to BCD 1");
    }
    for (U32 top = 0; top < 8; ++top) for (U8 op : {0xf8,0xf5}) {
        beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
        context.memory->writeq(TEST_HEAP_ADDRESS + 0x100,0x8000000000000000ull);
        context.memory->writew(TEST_HEAP_ADDRESS + 0x108,0x3fff - 100); // extended 2^-100
        fpMem(0xdb,5,0x100); fp(0xd9,0xf2); fp(0xd9,op); // FLD80; FPTAN; FPREM/FPREM1
        testRunCPU(); requireFpJit();
        if (fpu.top != ((top - 2) & 7) || !(fpu.sw & 0x400) || fpu.isRegCached[fpu.top] ||
            fpu.regs[fpu.top].signif || fpu.regs[fpu.top].signExp || fpu.isRegCached[fpu.STV(1)])
            testFail("raw FPTAN remainder must retain extended partial reduction with C2 set");
    }
}

void testWasmJitFpuMathLoops() {
    for (U32 top=0;top<8;++top) {
        for (U8 op : cachedMathOps) {
            beginFp(top); auto& context=testContext();
            context.memory->writeq(TEST_HEAP_ADDRESS+0x100,0x3fd0000000000000ull); // .25
            context.memory->writeq(TEST_HEAP_ADDRESS+0x110,0x3fe8000000000000ull); // .75
            FPU expected; expected.FINIT();
            seedMathValue(expected,0,.25,false); seedMathValue(expected,1,.75,false);
            referenceMath(expected,op);
            U32 values=mathPushes(op) ? 3 : mathPops(op) ? 1 : 2;
            fp(0xd9,0xe8); fp(0xd8,0xc0); // dirty sentinel
            testPushCode8(0xb9); testPushCode32(31);
            U32 loop=context.codeIp;
            fpMem(0xdd,0,0x110); fpMem(0xdd,0,0x100);
            fp(0xd9,op); fp(0xd9,0xe5); // classify a dirty result on each backedge
            for (U32 i=0;i<values;++i) fpMem(0xdd,3,0x500+i*8);
            testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop-(context.codeIp+1)));
            testRunCPU();
            for (U32 i=0;i<values;++i) {
                union { U64 bits; double d; } actual={context.memory->readq(TEST_HEAP_ADDRESS+0x500+i*8)};
                if (actual.d!=expected.getF64(expected.STV(i))) testFail("math loop output mismatch %x",op);
            }
            expectFp(0,2); requireFpJit();
        }
    }
}

void testWasmJitFpuMathFaults() {
    for (U8 op : cachedMathOps) {
        beginFp(3); auto& context=testContext(); auto& fpu=context.cpu->fpu;
        seedMathValue(fpu,3,.25,true); seedMathValue(fpu,4,.75,true);
        FPU expected=fpu; referenceMath(expected,op);
        fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
        fp(0xd9,op);
        fpMem(0xdd,3,0x2000); // fault before storing or popping the math result
        auto oldAction=context.process->sigActions[K_SIGSEGV];
        auto& action=context.process->sigActions[K_SIGSEGV];
        action.reset(); action.handlerAndSigAction=context.codeIp;
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS+0x2000,K_PAGE_SIZE,0);
        testRunCPU();
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS+0x2000,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE);
        if(action.sigInfo[0]!=K_SIGSEGV || fpu.top!=expected.top) testFail("math fault lost TOP");
        U32 values=mathPushes(op) ? 3 : mathPops(op) ? 1 : 2;
        for(U32 i=0;i<values;++i) expectMathValue(i,expected.getF64(expected.STV(i)));
        expectFp(values,2);
        context.process->sigActions[K_SIGSEGV]=oldAction;
    }
}

namespace {
void referenceSpecial(FPU& fpu, U8 op) {
    if(op==0xf8) fpu.FPREM();
    else if(op==0xf5) fpu.FPREM1();
    else if(op==0xfd) fpu.FSCALE();
    else fpu.FXTRACT();
}
void expectSpecialSlot(U32 relative, const FPU& expected, U32 expectedIndex) {
    auto& fpu=testContext().cpu->fpu;
    U32 index=fpu.STV(relative);
    if(fpu.isRegCached[index]!=expected.isRegCached[expectedIndex])
        testFail("special arithmetic changed representation at ST(%u)",relative);
    if(expected.isRegCached[expectedIndex]) {
        if(fpu.regCache[index].l!=expected.regCache[expectedIndex].l)
            testFail("special arithmetic double mismatch ST(%u): %llx expected %llx",relative,
                (unsigned long long)fpu.regCache[index].l,(unsigned long long)expected.regCache[expectedIndex].l);
    } else if(fpu.regs[index].signif!=expected.regs[expectedIndex].signif ||
              fpu.regs[index].signExp!=expected.regs[expectedIndex].signExp) {
        testFail("special arithmetic raw mismatch ST(%u): %x:%llx expected %x:%llx",relative,
            fpu.regs[index].signExp,(unsigned long long)fpu.regs[index].signif,
            expected.regs[expectedIndex].signExp,(unsigned long long)expected.regs[expectedIndex].signif);
    }
}
void runSpecialPair(U32 top, U8 op, double x, double y, bool rawX, bool rawY) {
    beginFp(top); auto& context=testContext(); auto& fpu=context.cpu->fpu;
    seedMathValue(fpu,top,x,rawX); seedMathValue(fpu,fpu.STV(1),y,rawY);
    fpu.sw=0x8755;
    FPU expected=fpu; referenceSpecial(expected,op);
    fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb); // unrelated dirty 2
    context.cpu->xmm[0].pi.u32[0]=0x3f800000;
    context.cpu->xmm[0].pi.u32[1]=0x12345678;
    testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
    testPushCode8(0xb8); testPushCode32(0xffffffff);
    fp(0x83,0xc0); testPushCode8(1);
    fp(0xd9,op);
    fp(0x83,0xd0); testPushCode8(0);
    testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
    testRunCPU();
    if(fpu.top!=expected.top || (fpu.sw & ~0x3800u)!=(expected.sw & ~0x3800u) ||
       context.cpu->reg[0].u32!=1 || context.cpu->xmm[0].pi.u32[0]!=0x40800000 ||
       context.cpu->xmm[0].pi.u32[1]!=0x12345678)
        testFail("special opcode %x changed TOP/status/GP/XMM x=%.17g y=%.17g raw=%u/%u",op,x,y,rawX,rawY);
    U32 values=op==0xf4 ? 3 : 2;
    for(U32 i=0;i<values;++i) expectSpecialSlot(i,expected,expected.STV(i));
    expectFp(values,2); requireFpJit();
}
}

void testWasmJitFpuMathRepresentations() {
    const struct { U64 low; U16 high; U64 cached; } inputs[] = {
        {0,0,0},{0,0x8000,0x8000000000000000ull},{1,0,1},
        {0x8000000000000000ull,0x3bcd,1},
        {0x8000000000000400ull,0x3ffe,0x3fe0000000000000ull},
        {0x8000000000000401ull,0x3fff,0x3ff0000000000001ull},
        {0xffffffffffffffffull,0x3fff,0x4000000000000000ull},
        {0xa000000000000000ull,0xbffe,0xbfe4000000000000ull},
        {0x8000000000000000ull,0x7ffe,0x7fefffffffffffffull},
        {0x8000000000000000ull,0x7fff,0x7ff0000000000000ull},
        {0x8000000000000001ull,0x7fff,0x7ff0000000000001ull},
        {0xc123456789abcdefull,0xffff,0xfff8123456789abcull}};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    auto oldTiny = softfloat_detectTininess;
    U32 sequence = 0;
    for (U8 op : cachedMathOps) for (const auto& input : inputs) {
        for (U32 representation = 0; representation < 4; ++representation) for (U8 mode : {0,1,2,3,4,6}) {
            for (bool dirty : {false,true}) {
                bool rawX = !(representation & 1), rawY = !(representation & 2);
                U32 top = sequence++ & 7;
                beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
                fpu.SetCW(0x037f | ((sequence & 3) << 10)); fpu.sw = 0x8755;
                if (rawX) fpu.LD80(top,input.low,input.high);
                else { fpu.regCache[top].l = input.cached; fpu.isRegCached[top] = true; }
                U32 other = fpu.STV(1);
                if (rawY) fpu.LD80(other,0xc000000000000401ull,0x3ffe);
                else { fpu.regCache[other].d = 0.75; fpu.isRegCached[other] = true; }
                fpu.tags[top] = fpu.tags[other] = TAG_Valid;
                FPU expected = fpu;
                softfloat_roundingMode = mode; softfloat_detectTininess = (sequence >> 3) & 1;
                softfloat_exceptionFlags = softfloat_flag_infinite;
                referenceMath(expected,op);
                U8 flags = softfloat_exceptionFlags, expectedMode = softfloat_roundingMode;
                fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb); // unrelated dirty ST2
                if (dirty) {
                    context.memory->writeq(TEST_HEAP_ADDRESS + 0x100,rawX ? input.low : input.cached);
                    context.memory->writew(TEST_HEAP_ADDRESS + 0x108,input.high);
                    if (rawX) { fp(0xd9,0xf7); fpMem(0xdb,5,0x100); }
                    else { fpMem(0xdd,0,0x100); fp(0xdd,0xd9); }
                    context.memory->writeq(TEST_HEAP_ADDRESS + 0x110,rawY ? 0xc000000000000401ull : 0x3fe8000000000000ull);
                    context.memory->writew(TEST_HEAP_ADDRESS + 0x118,0x3ffe);
                    if (rawY) { fp(0xd9,0xf7); fp(0xd9,0xf7); fpMem(0xdb,5,0x110); fp(0xd9,0xf6); }
                    else { fpMem(0xdd,0,0x110); fp(0xdd,0xda); }
                }
                context.cpu->xmm[0].pi.u32[0] = 0x3f800000; context.cpu->xmm[0].pi.u32[1] = 0x12345678;
                testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
                testPushCode8(0xb8); testPushCode32(0xffffffff); fp(0x83,0xc0); testPushCode8(1);
                fp(0xd9,op); fp(0x83,0xd0); testPushCode8(0);
                testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
                softfloat_roundingMode = mode; softfloat_exceptionFlags = softfloat_flag_infinite;
                testRunCPU();
                U32 values = mathPushes(op) ? 3 : mathPops(op) ? 1 : 2;
                for (U32 i = 0; i < values; ++i) expectSpecialSlot(i,expected,expected.STV(i));
                // A raw popped operand is still observable after a TOP rotation.
                if (mathPops(op)) expectSpecialSlot(7,expected,top);
                if (fpu.top != expected.top || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                    softfloat_exceptionFlags != flags || softfloat_roundingMode != expectedMode ||
                    context.cpu->reg[0].u32 != 1 || context.cpu->xmm[0].pi.u32[0] != 0x40800000 ||
                    context.cpu->xmm[0].pi.u32[1] != 0x12345678)
                    testFail("math representation seq=%u op=%x rep=%u dirty=%u flags=%u expected=%u mode=%u expected=%u",
                        sequence,op,representation,dirty,softfloat_exceptionFlags,flags,softfloat_roundingMode,expectedMode);
                expectFp(values,2); requireFpJit();
            }
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode; softfloat_detectTininess = oldTiny;
}

void testWasmJitFpuMathRepresentationLoops() {
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    for (U32 top = 0; top < 8; ++top) for (U8 op : cachedMathOps) {
        for (bool raw : {false,true}) for (U32 count : {1u,2u,19u}) {
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            seedMathValue(fpu,top,0.75,raw); seedMathValue(fpu,fpu.STV(1),0.75,raw);
            FPU expected = fpu;
            bool binary = mathPops(op);
            softfloat_roundingMode = softfloat_round_max; softfloat_exceptionFlags = softfloat_flag_infinite;
            U8 bcd[10] = {};
            for (U32 i = 0; i < count; ++i) {
                referenceMath(expected,op);
                if (mathPushes(op)) expected.FPOP();
                if (binary) { expected.PREP_PUSH(); seedMathValue(expected,expected.top,0.75,raw); }
            }
            if (binary) expected.top = (expected.top + 1) & 7;
            expected.FBST(expected.top,bcd); expected.FPOP();
            U8 flags = softfloat_exceptionFlags, mode = softfloat_roundingMode;
            context.memory->writeq(TEST_HEAP_ADDRESS + 0x100,raw ? 0xc000000000000000ull : 0x3fe8000000000000ull);
            context.memory->writew(TEST_HEAP_ADDRESS + 0x108,0x3ffe);
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
            testPushCode8(0xb9); testPushCode32(count);
            U32 loop = context.codeIp;
            fp(0xd9,op);
            // FFREEP discards the extra result without converting it. Binary
            // operations reload their popped input; the result stays live.
            if (mathPushes(op)) fp(0xdf,0xc0);
            if (binary) fpMem(raw ? 0xdb : 0xdd,raw ? 5 : 0,0x100);
            testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop - (context.codeIp + 1)));
            if (binary) fp(0xd9,0xf7);
            fpMem(0xdf,6,0x200);
            softfloat_roundingMode = softfloat_round_max; softfloat_exceptionFlags = softfloat_flag_infinite;
            testRunCPU();
            expectTen(0x200,bcd);
            if (fpu.top != expected.top || context.cpu->reg[1].u32 || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                softfloat_exceptionFlags != flags || softfloat_roundingMode != mode)
                testFail("math representation loop op=%x top=%u raw=%u count=%u",op,top,raw,count);
            expectSpecialSlot((top - fpu.top) & 7,expected,top);
            expectSpecialSlot((top + 1 - fpu.top) & 7,expected,(top + 1) & 7);
            expectFp(binary ? 0 : 1,2); requireFpJit();
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuMathRepresentationFaults() {
    for (U32 top = 0; top < 8; ++top) for (U8 op : cachedMathOps) for (U32 representation = 0; representation < 4; ++representation) {
        beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
        seedMathValue(fpu,top,0.75,!(representation & 1));
        seedMathValue(fpu,fpu.STV(1),0.75,!(representation & 2));
        FPU expected = fpu; referenceMath(expected,op);
        fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
        fp(0xd9,op); fpMem(0xdf,6,0x2000);
        auto oldAction = context.process->sigActions[K_SIGSEGV];
        auto& action = context.process->sigActions[K_SIGSEGV];
        action.reset(); action.handlerAndSigAction = context.codeIp;
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS + 0x2000,K_PAGE_SIZE,0);
        testRunCPU();
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS + 0x2000,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE);
        if (action.sigInfo[0] != K_SIGSEGV || fpu.top != expected.top)
            testFail("math representation fault lost TOP op=%x rep=%u",op,representation);
        U32 values = mathPushes(op) ? 3 : mathPops(op) ? 1 : 2;
        for (U32 i = 0; i < values; ++i) { expected.getReg(expected.STV(i)); expectSpecialSlot(i,expected,expected.STV(i)); }
        expectFp(values,2);
        context.process->sigActions[K_SIGSEGV] = oldAction;
    }
}

void testWasmJitFpuRemainder() {
    const double dividends[]={0.0,-0.0,1.0,-1.0,7.0,-7.0,9.0,-9.0,0x1p-1074,
        0x1p52+1,0x1p63,-0x1p63,0x1p100,
        std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()};
    const double divisors[]={0.0,-0.0,2.0,-2.0,0.75,-3.0,0x1p-1074,
        std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN()};
    for(U32 top=0;top<8;++top) {
        for(U8 op : {0xf8,0xf5}) {
            for(bool rawX : {false,true}) for(bool rawY : {false,true}) {
                for(double x : dividends) for(double y : divisors)
                    runSpecialPair(top,op,x,y,rawX,rawY);
            }
        }
    }
}

void testWasmJitFpuRemainderInputs() {
    const struct { U64 low; U16 high; } raw[] = {
        {0,0},{1,0},{0xffffffffffffffffull,0},
        {0x8000000000000000ull,0x3fff},{0x8000000000000400ull,0x3fff},
        {0x8000000000000401ull,0x3fff},{0xe000000000000001ull,0x4001},
        {0xffffffffffffffffull,0x403d},{0x8000000000000000ull,0x403e},
        {0x8000000000000000ull,0x3bcd},{0x8000000000000001ull,0x3bcd},
        {0xffffffffffffffffull,0x3c00},{0x8000000000000000ull,0x7ffe},
        {0x8000000000000000ull,0x7fff},{0x8000000000000001ull,0x7fff},
        {0xc123456789abcdefull,0x7fff},{0x0123456789abcdefull,0x3fff},
        {0,0x7fff},{0x8123456789abcdefull,0x0064},{0xffffffffffffffffull,0x43fe}};
    const U64 cached[] = {0,1,0x000fffffffffffffull,0x0010000000000000ull,
        0x3ff0000000000000ull,0x401c000000000000ull,0x4022000000000000ull,
        0x43dfffffffffffffull,0x43e0000000000000ull,0x7fefffffffffffffull,
        0x7ff0000000000000ull,0x7ff0000000000001ull,0x7ff7ffffffffffffull,
        0x7ff8000000000000ull,0x7ff8123456789abcull,0x7fffffffffffffffull};
    const U64 divisors[] = {0,0x8000000000000000ull,0x4000000000000000ull,
        0xc000000000000000ull,0x4008000000000000ull,0x3fe8000000000000ull,1,
        0x7ff0000000000000ull,0xfff0000000000000ull,0x7ff8000000000000ull,
        0x7ff0000000000001ull,0xfff8123456789abcull};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    auto oldTiny = softfloat_detectTininess;
    U32 sequence = 0;
    for (U32 input = 0; input < 36; ++input) for (bool negative : {false,true}) {
        bool extended = input < 20;
        U64 low = extended ? raw[input].low : cached[input - 20] | (negative ? 1ull << 63 : 0);
        U16 high = extended ? raw[input].high | (negative ? 0x8000 : 0) : 0;
        for (U64 divisor : divisors) for (U8 op : {0xf8,0xf5}) for (bool dirty : {false,true}) {
            U32 top = sequence++ & 7;
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            fpu.SetCW(0x037f | ((sequence & 3) << 10)); fpu.sw = 0x8755;
            if (extended) fpu.LD80(top,low,high);
            else { fpu.regCache[top].l = low; fpu.isRegCached[top] = true; }
            fpu.tags[top] = TAG_Valid;
            U32 other = fpu.STV(1);
            fpu.regCache[other].l = divisor; fpu.isRegCached[other] = true; fpu.tags[other] = TAG_Valid;
            FPU expected = fpu;
            U8 initialMode = (sequence >> 3) & 3;
            softfloat_roundingMode = initialMode; softfloat_exceptionFlags = softfloat_flag_infinite;
            softfloat_detectTininess = (sequence >> 5) & 1;
            referenceSpecial(expected,op);
            U8 flags = softfloat_exceptionFlags, mode = softfloat_roundingMode;
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb); // dirty ST2
            if (dirty) {
                context.memory->writeq(TEST_HEAP_ADDRESS + 0x100, low);
                context.memory->writew(TEST_HEAP_ADDRESS + 0x108, high);
                if (extended) { fp(0xd9,0xf7); fpMem(0xdb,5,0x100); } // replace ST0 with raw FLD80
                else { fpMem(0xdd,0,0x100); fp(0xdd,0xd9); }
                context.memory->writeq(TEST_HEAP_ADDRESS + 0x110, divisor);
                fpMem(0xdd,0,0x110); fp(0xdd,0xda); // dirty cached ST1
            }
            context.cpu->xmm[0].pi.u32[0] = 0x3f800000; context.cpu->xmm[0].pi.u32[1] = 0x12345678;
            testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
            testPushCode8(0xb8); testPushCode32(0xffffffff); fp(0x83,0xc0); testPushCode8(1);
            fp(0xd9,op); fp(0x83,0xd0); testPushCode8(0);
            testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
            softfloat_roundingMode = initialMode; softfloat_exceptionFlags = softfloat_flag_infinite;
            testRunCPU();
            expectSpecialSlot(0,expected,top); expectSpecialSlot(1,expected,other);
            expectFp(2,2); requireFpJit();
            if (fpu.top != top || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                softfloat_exceptionFlags != flags || softfloat_roundingMode != mode ||
                context.cpu->reg[0].u32 != 1 || context.cpu->xmm[0].pi.u32[0] != 0x40800000 ||
                context.cpu->xmm[0].pi.u32[1] != 0x12345678)
                testFail("remainder input state seq=%u op=%x raw=%u dirty=%u flags=%u expected=%u mode=%u expected=%u",
                    sequence,op,extended,dirty,softfloat_exceptionFlags,flags,softfloat_roundingMode,mode);
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode; softfloat_detectTininess = oldTiny;
}

void testWasmJitFpuRemainderConversionLoops() {
    const struct { U64 low; U16 high; double divisor; } cases[] = {
        {0xe000000000000001ull,0x4001,2}, {0x8000000000000400ull,0x3fff,0.75},
        {0x8000000000000001ull,0x3bcd,2}, {0x8000000000000000ull,0x403e,0},
        {0xffffffffffffffffull,0x403d,0.75}, {0x8000000000000000ull,0x7ffe,2},
        {0x8000000000000001ull,0x7fff,2}, {0xe000000000000000ull,0x4001,std::numeric_limits<double>::infinity()}};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    for (U32 top = 0; top < 8; ++top) for (const auto& item : cases) {
        for (U8 op : {0xf8,0xf5}) for (U32 count : {1u,2u,19u}) {
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            seedMathValue(fpu,top,1,false); seedMathValue(fpu,fpu.STV(1),item.divisor,false);
            fpu.sw = 0x8755; FPU expected = fpu;
            softfloat_roundingMode = softfloat_round_max; softfloat_exceptionFlags = softfloat_flag_infinite;
            for (U32 i = 0; i < count; ++i) { expected.LD80(top,item.low,item.high); referenceSpecial(expected,op); }
            U8 flags = softfloat_exceptionFlags, mode = softfloat_roundingMode;
            context.memory->writeq(TEST_HEAP_ADDRESS + 0x100,item.low);
            context.memory->writew(TEST_HEAP_ADDRESS + 0x108,item.high);
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
            testPushCode8(0xb9); testPushCode32(count);
            U32 loop = context.codeIp;
            fp(0xd9,0xf7); fpMem(0xdb,5,0x100); fp(0xd9,op);
            testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop - (context.codeIp + 1)));
            softfloat_roundingMode = softfloat_round_max; softfloat_exceptionFlags = softfloat_flag_infinite;
            testRunCPU();
            expectSpecialSlot(0,expected,top); expectSpecialSlot(1,expected,expected.STV(1));
            expectFp(2,2); requireFpJit();
            if (fpu.top != top || context.cpu->reg[1].u32 || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                softfloat_exceptionFlags != flags || softfloat_roundingMode != mode)
                testFail("remainder conversion loop top=%u op=%x count=%u",top,op,count);
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuRemainderConversionFaults() {
    for (U32 top = 0; top < 8; ++top) for (U8 op : {0xf8,0xf5}) {
        for (double divisor : {2.0,0.0,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}) {
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            fpu.LD80(top,0xe000000000000001ull,0x4001); fpu.tags[top] = TAG_Valid;
            seedMathValue(fpu,fpu.STV(1),divisor,false);
            FPU expected = fpu; referenceSpecial(expected,op);
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
            fp(0xd9,op); fpMem(0xdd,3,0x2000);
            auto oldAction = context.process->sigActions[K_SIGSEGV];
            auto& action = context.process->sigActions[K_SIGSEGV];
            action.reset(); action.handlerAndSigAction = context.codeIp;
            context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS + 0x2000,K_PAGE_SIZE,0);
            testRunCPU();
            context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS + 0x2000,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE);
            if (action.sigInfo[0] != K_SIGSEGV || fpu.top != top) testFail("remainder conversion fault lost TOP");
            for (U32 i = 0; i < 2; ++i) { expected.getReg(expected.STV(i)); expectSpecialSlot(i,expected,expected.STV(i)); }
            expectFp(2,2);
            context.process->sigActions[K_SIGSEGV] = oldAction;
        }
    }
}

void testWasmJitFpuScaleExtract() {
    const double inputs[]={0.0,-0.0,1.0,-1.5,0x1p-1022,0x1p-1074,0x1p1023,
        std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN()};
    const double scales[]={0.0,-0.0,2.75,-2.75,1023.0,-1022.0,1024.0,-1075.0,0x1p-1074,
        std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN()};
    for(U32 top=0;top<8;++top) {
        for(bool rawX : {false,true}) for(bool rawY : {false,true}) {
            for(double x : inputs) for(double y : scales)
                runSpecialPair(top,0xfd,x,y,rawX,rawY);
        }
        for(bool raw : {false,true}) for(double x : inputs)
            runSpecialPair(top,0xf4,x,3.0,raw,raw);
        // Raw extraction must preserve all 64 significand bits and the sign,
        // including encodings outside the finite-double range.
        const struct { U64 low; U16 high; } values[]={
            {0x8123456789abcdefull,0x3fff},{0x8123456789abcdefull,0xbfff},
            {1,0},{1,0x8000},{0x8000000000000000ull,0x7ffe},
            {0xc123456789abcdefull,0x7fff},{0x8123456789abcdefull,0xffff},
            {0x0123456789abcdefull,0x3fff},{0,0x7fff}};
        for(const auto& value : values) {
            beginFp(top); auto& fpu=testContext().cpu->fpu;
            fpu.LD80(top,value.low,value.high); fpu.tags[top]=TAG_Valid;
            FPU expected=fpu; expected.FXTRACT();
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xda); // dirty neighbor
            fp(0xd9,0xf4); testRunCPU();
            expectSpecialSlot(0,expected,expected.top);
            expectSpecialSlot(1,expected,expected.STV(1)); expectFp(2,2);
        }
    }
}

void testWasmJitFpuExtractAll() {
    const U64 classes[] = {0,1,2,0x0008000000000000ull,0x000fffffffffffffull,
        0x0010000000000000ull,0x0010000000000001ull,0x3ff0000000000000ull,
        0x3ff0000000000001ull,0x7fefffffffffffffull,0x7ff0000000000000ull,
        0x7ff0000000000001ull,0x7ff7ffffffffffffull,0x7ff8000000000000ull,
        0x7ff8123456789abcull,0x7fffffffffffffffull};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    U64 random = 0xa0761d6478bd642full;
    for (U32 sequence = 0; sequence < 128; ++sequence) {
        random ^= random << 13; random ^= random >> 7; random ^= random << 17;
        U64 bits = sequence < 32 ? classes[sequence & 15] | ((sequence & 16) ? 1ull << 63 : 0) : random;
        for (bool dirty : {false,true}) {
            U32 top = sequence & 7;
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            fpu.SetCW(0x037f | ((sequence & 3) << 10)); fpu.sw = 0x8755;
            fpu.regCache[top].l = bits; fpu.isRegCached[top] = true; fpu.tags[top] = TAG_Valid;
            FPU expected = fpu;
            softfloat_exceptionFlags = softfloat_flag_infinite;
            softfloat_roundingMode = softfloat_round_max;
            expected.FXTRACT(); U8 flags = softfloat_exceptionFlags;
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xda); // dirty neighbor 2
            if (dirty) {
                context.memory->writeq(TEST_HEAP_ADDRESS + 0x100, bits);
                fpMem(0xdd,0,0x100); fp(0xdd,0xd9);
            }
            context.cpu->xmm[0].pi.u32[0] = 0x3f800000;
            context.cpu->xmm[0].pi.u32[1] = 0x12345678;
            testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
            testPushCode8(0xb8); testPushCode32(0xffffffff); fp(0x83,0xc0); testPushCode8(1);
            fp(0xd9,0xf4);
            fp(0x83,0xd0); testPushCode8(0);
            testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
            softfloat_exceptionFlags = softfloat_flag_infinite;
            testRunCPU();
            expectSpecialSlot(0,expected,expected.top); expectSpecialSlot(1,expected,expected.STV(1));
            expectFp(2,2); requireFpJit();
            if (fpu.top != expected.top || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                softfloat_exceptionFlags != flags || softfloat_roundingMode != softfloat_round_max ||
                fpu.tags[fpu.top] != TAG_Valid || fpu.tags[fpu.STV(1)] != TAG_Valid ||
                context.cpu->reg[0].u32 != 1 || context.cpu->xmm[0].pi.u32[0] != 0x40800000 ||
                context.cpu->xmm[0].pi.u32[1] != 0x12345678)
                testFail("full inline FXTRACT state sequence=%u dirty=%u",sequence,dirty);
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuScaleRange() {
    const U64 classes[] = {0,1,2,0x0008000000000000ull,0x000fffffffffffffull,
        0x0010000000000000ull,0x0010000000000001ull,0x3ff8000000000000ull,
        0x4008000000000000ull,0x7fdfffffffffffffull,0x7fefffffffffffffull,
        0x7ff0000000000000ull,0x7ff0000000000001ull,0x7ff7ffffffffffffull,
        0x7ff8000000000000ull,0x7ff8123456789abcull};
    const double scales[] = {0.0,-0.0,0x1p-1074,-0x1p-1074,0.99,-0.99,1,-1,2.75,-2.75,
        1023,1023.999,1024,1024.5,1074,-1022,-1022.99,-1023,-1073,-1074,-1074.9,-1075,
        0x1p63,-0x1p63,0x1p100,-0x1p100,0x1.fffffffffffffp1023,-0x1.fffffffffffffp1023};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    U32 sequence = 0;
    for (U64 value : classes) for (bool negative : {false,true}) {
        U64 bits = value | (negative ? 1ull << 63 : 0);
        for (double scale : scales) for (bool dirty : {false,true}) {
            U32 top = sequence++ & 7;
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            fpu.SetCW(0x037f | ((sequence & 3) << 10)); fpu.sw = 0x8755;
            fpu.regCache[top].l = bits; fpu.isRegCached[top] = true; fpu.tags[top] = TAG_Valid;
            seedMathValue(fpu,fpu.STV(1),scale,false);
            FPU expected = fpu;
            U8 mode = (sequence >> 3) & 3;
            softfloat_roundingMode = mode; softfloat_exceptionFlags = softfloat_flag_infinite;
            expected.FSCALE(); U8 flags = softfloat_exceptionFlags;
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb); // dirty ST2
            if (dirty) {
                context.memory->writeq(TEST_HEAP_ADDRESS + 0x100, bits);
                context.memory->writeq(TEST_HEAP_ADDRESS + 0x108, fpu.regCache[fpu.STV(1)].l);
                fpMem(0xdd,0,0x100); fp(0xdd,0xd9);
                fpMem(0xdd,0,0x108); fp(0xdd,0xda);
            }
            context.cpu->xmm[0].pi.u32[0] = 0x3f800000;
            context.cpu->xmm[0].pi.u32[1] = 0x12345678;
            testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
            testPushCode8(0xb8); testPushCode32(0xffffffff); fp(0x83,0xc0); testPushCode8(1);
            fp(0xd9,0xfd); fp(0x83,0xd0); testPushCode8(0);
            testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
            softfloat_exceptionFlags = softfloat_flag_infinite;
            testRunCPU();
            expectSpecialSlot(0,expected,expected.top); expectSpecialSlot(1,expected,expected.STV(1));
            expectFp(2,2); requireFpJit();
            if (fpu.top != top || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                softfloat_exceptionFlags != flags || softfloat_roundingMode != mode ||
                context.cpu->reg[0].u32 != 1 || context.cpu->xmm[0].pi.u32[0] != 0x40800000 ||
                context.cpu->xmm[0].pi.u32[1] != 0x12345678)
                testFail("inline FSCALE range state bits=%llx scale=%.17g dirty=%u flags=%u expected=%u",bits,scale,dirty,softfloat_exceptionFlags,flags);
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuScaleNonfinite() {
    const U64 classes[] = {0,1,0x0008000000000000ull,0x000fffffffffffffull,
        0x0010000000000000ull,0x3ff0000000000000ull,0x3ff8000000000000ull,
        0x7fefffffffffffffull,0x7ff0000000000000ull,0x7ff0000000000001ull,
        0x7ff0000000000400ull,0x7ff7ffffffffffffull,0x7ff8000000000000ull,
        0x7ff8000000000001ull,0x7ff8123456789abcull,0x7fffffffffffffffull};
    const U64 scales[] = {0,0x8000000000000000ull,1,0x3fe8000000000000ull,
        0xbfe8000000000000ull,0x4090000000000000ull,0xc090cc0000000000ull,
        0x7fefffffffffffffull,0xffefffffffffffffull,0x7ff0000000000000ull,
        0xfff0000000000000ull,0x7ff0000000000001ull,0xfff0000000000001ull,
        0x7ff7ffffffffffffull,0x7ff8123456789abcull,0xfff8123456789abcull};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    U32 sequence = 0;
    for (U64 value : classes) for (bool negative : {false,true}) {
        U64 bits = value | (negative ? 1ull << 63 : 0);
        for (U64 scale : scales) for (bool dirty : {false,true}) {
            U32 top = sequence++ & 7;
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            fpu.SetCW(0x037f | ((sequence & 3) << 10)); fpu.sw = 0x8755;
            fpu.regCache[top].l = bits; fpu.isRegCached[top] = true; fpu.tags[top] = TAG_Valid;
            U32 other = fpu.STV(1);
            fpu.regCache[other].l = scale; fpu.isRegCached[other] = true; fpu.tags[other] = TAG_Valid;
            FPU expected = fpu;
            U8 mode = (sequence >> 3) & 3;
            softfloat_roundingMode = mode; softfloat_exceptionFlags = softfloat_flag_infinite;
            expected.FSCALE(); U8 flags = softfloat_exceptionFlags;
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb); // dirty ST2
            if (dirty) {
                context.memory->writeq(TEST_HEAP_ADDRESS + 0x100,bits);
                context.memory->writeq(TEST_HEAP_ADDRESS + 0x108,scale);
                fpMem(0xdd,0,0x100); fp(0xdd,0xd9);
                fpMem(0xdd,0,0x108); fp(0xdd,0xda);
            }
            context.cpu->xmm[0].pi.u32[0] = 0x3f800000; context.cpu->xmm[0].pi.u32[1] = 0x12345678;
            testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
            testPushCode8(0xb8); testPushCode32(0xffffffff); fp(0x83,0xc0); testPushCode8(1);
            fp(0xd9,0xfd); fp(0x83,0xd0); testPushCode8(0);
            testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
            softfloat_exceptionFlags = softfloat_flag_infinite;
            testRunCPU();
            expectSpecialSlot(0,expected,top); expectSpecialSlot(1,expected,other);
            expectFp(2,2); requireFpJit();
            if (fpu.top != top || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                softfloat_exceptionFlags != flags || softfloat_roundingMode != mode ||
                context.cpu->reg[0].u32 != 1 || context.cpu->xmm[0].pi.u32[0] != 0x40800000 ||
                context.cpu->xmm[0].pi.u32[1] != 0x12345678)
                testFail("nonfinite FSCALE seq=%u x=%llx y=%llx dirty=%u flags=%u expected=%u",
                    sequence,bits,scale,dirty,softfloat_exceptionFlags,flags);
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

namespace {
const struct { U64 x, y; } scaleNonfiniteCases[] = {
    {0x7ff8123456789abcull,0x4000000000000000ull},
    {0x7ff0000000000001ull,0x4000000000000000ull},
    {0xfff8123456789abcull,0x4000000000000000ull},
    {0x4000000000000000ull,0x7ff0000000000001ull},
    {0,0x7ff0000000000000ull}, {0x8000000000000000ull,0xfff0000000000000ull},
    {0xbff0000000000000ull,0xfff0000000000000ull},
    {0x7ff0000000000000ull,0x7ff0000000000000ull},
    {0x7ff0000000000000ull,0xfff0000000000000ull},
    {0xfff0000000000000ull,0xc090cc0000000000ull},
    {0x7ff0000000000001ull,0xfff0000000000000ull},
    {0xfff0000000000001ull,0x7ff0000000000000ull}};
void seedScaleNonfinite(U64 x, U64 y) {
    auto& fpu = testContext().cpu->fpu;
    fpu.regCache[fpu.top].l = x; fpu.isRegCached[fpu.top] = true; fpu.tags[fpu.top] = TAG_Valid;
    U32 other = fpu.STV(1);
    fpu.regCache[other].l = y; fpu.isRegCached[other] = true; fpu.tags[other] = TAG_Valid;
    fpu.sw = 0x8755;
}
}

void testWasmJitFpuScaleNonfiniteLoops() {
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    for (U32 top = 0; top < 8; ++top) for (const auto& item : scaleNonfiniteCases) {
        for (bool validate : {false,true}) for (U32 count : {1u,2u,19u}) {
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            seedScaleNonfinite(item.x,item.y); FPU expected = fpu;
            softfloat_roundingMode = softfloat_round_max; softfloat_exceptionFlags = softfloat_flag_infinite;
            for (U32 i = 0; i < count; ++i) {
                if (validate) { expected.getF64(top); expected.getF64(expected.STV(1)); }
                expected.FSCALE();
            }
            U8 flags = softfloat_exceptionFlags, mode = softfloat_roundingMode;
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
            testPushCode8(0xb9); testPushCode32(count);
            U32 loop = context.codeIp;
            if (validate) { fp(0xd9,0xc0); fp(0xdd,0xd8); fp(0xd9,0xc1); fp(0xdd,0xd8); }
            fp(0xd9,0xfd);
            testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop - (context.codeIp + 1)));
            softfloat_roundingMode = softfloat_round_max; softfloat_exceptionFlags = softfloat_flag_infinite;
            testRunCPU();
            expectSpecialSlot(0,expected,top); expectSpecialSlot(1,expected,expected.STV(1));
            expectFp(2,2); requireFpJit();
            if (fpu.top != top || context.cpu->reg[1].u32 || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                softfloat_exceptionFlags != flags || softfloat_roundingMode != mode)
                testFail("nonfinite scaling loop top=%u count=%u validate=%u",top,count,validate);
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuScaleNonfiniteFaults() {
    for (U32 top = 0; top < 8; ++top) for (const auto& item : scaleNonfiniteCases) {
        beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
        seedScaleNonfinite(item.x,item.y); FPU expected = fpu; expected.FSCALE();
        fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
        fp(0xd9,0xfd); fpMem(0xdd,3,0x2000);
        auto oldAction = context.process->sigActions[K_SIGSEGV];
        auto& action = context.process->sigActions[K_SIGSEGV];
        action.reset(); action.handlerAndSigAction = context.codeIp;
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS + 0x2000,K_PAGE_SIZE,0);
        testRunCPU();
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS + 0x2000,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE);
        if (action.sigInfo[0] != K_SIGSEGV || fpu.top != top) testFail("nonfinite scaling fault lost TOP");
        expected.getF64(top); // conversion precedes the faulting store
        for (U32 i = 0; i < 2; ++i) { expected.getReg(expected.STV(i)); expectSpecialSlot(i,expected,expected.STV(i)); }
        expectFp(2,2);
        context.process->sigActions[K_SIGSEGV] = oldAction;
    }
}

namespace {
void emitScaleOperand(U32 slot, bool raw, U64 low, U16 high) {
    U32 address = 0x100 + slot * 16;
    testContext().memory->writeq(TEST_HEAP_ADDRESS + address,low);
    testContext().memory->writew(TEST_HEAP_ADDRESS + address + 8,high);
    if (raw) {
        for (U32 i = 0; i <= slot; ++i) fp(0xd9,0xf7);
        fpMem(0xdb,5,address);
        if (slot) fp(0xd9,0xf6);
    } else {
        fpMem(0xdd,0,address); fp(0xdd,0xd9 + slot);
    }
}
}

void testWasmJitFpuRemainderQuotientLimits() {
    const double dividends[] = {0.0,-0.0,0.5,1.5,2.5,3.5,7.5,
        0x1p52-0.5,0x1p52+1,0x1p53-1,0x1p63-0x1p10,0x1p63,0x1p63+0x1p11,
        0x1p100,std::numeric_limits<double>::max(),0x1p-1074};
    const double divisors[] = {0.0,-0.0,1.0,-1.0,0.5,0x1p-1074};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    U32 sequence = 0;
    for (double x : dividends) for (double y : divisors) for (bool negative : {false,true}) {
        for (U8 op : {0xf8,0xf5}) for (bool dirty : {false,true}) {
            U32 top = sequence++ & 7;
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            seedMathValue(fpu,top,negative ? -x : x,false);
            seedMathValue(fpu,fpu.STV(1),y,false); fpu.sw = 0x8755;
            FPU expected = fpu; U8 mode = (sequence & 1) ? softfloat_round_odd : softfloat_round_max;
            softfloat_roundingMode = mode; softfloat_exceptionFlags = softfloat_flag_infinite;
            referenceSpecial(expected,op); U8 flags = softfloat_exceptionFlags;
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
            if (dirty) {
                emitScaleOperand(0,false,fpu.regCache[top].l,0);
                emitScaleOperand(1,false,fpu.regCache[fpu.STV(1)].l,0);
            }
            fp(0xd9,op); softfloat_exceptionFlags = softfloat_flag_infinite;
            testRunCPU();
            expectSpecialSlot(0,expected,top); expectSpecialSlot(1,expected,expected.STV(1));
            expectFp(2,2); requireFpJit();
            if (fpu.top != top || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                softfloat_exceptionFlags != flags || softfloat_roundingMode != mode)
                testFail("remainder quotient limit seq=%u op=%x dirty=%u",sequence,op,dirty);
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuRemainderExtendedSpecial() {
    const struct { U64 low; U16 high; U64 cached; } values[] = {
        {0,0,0},{1,0,1},{0,0x3fff,0x3ff0000000000000ull},
        {0xe000000000000001ull,0x4001,0x401c000000000000ull},
        {0x8000000000000000ull,0x7ffe,0x7fefffffffffffffull},
        {0x0123456789abcdefull,0x3fff,0x3ff0000000000001ull},
        {0x8000000000000000ull,0x7fff,0x7ff0000000000000ull},
        {0,0x7fff,0x7ff0000000000000ull},
        {0x8000000000000001ull,0x7fff,0x7ff0000000000001ull},
        {0xc123456789abcdefull,0x7fff,0x7ff8123456789abcull},
        {0x0123456789abcdefull,0x7fff,0x7ff0000000000400ull}};
    const struct { U64 low; U16 high; } divisors[] = {
        {0x8000000000000000ull,0x7fff},{0,0x7fff},
        {0x8000000000000001ull,0x7fff},{0xc123456789abcdefull,0x7fff},
        {0,0},{0x8000000000000000ull,0x4000}};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    U32 sequence = 0;
    for (const auto& value : values) for (bool negative : {false,true}) {
        for (const auto& divisor : divisors) for (bool divisorNegative : {false,true}) {
            for (bool rawX : {false,true}) for (U8 op : {0xf8,0xf5}) for (bool dirty : {false,true}) {
                U32 top = sequence++ & 7;
                beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
                U64 x = rawX ? value.low : value.cached | (negative ? 1ull << 63 : 0);
                U16 xHigh = value.high | (negative ? 0x8000 : 0);
                U16 yHigh = divisor.high | (divisorNegative ? 0x8000 : 0);
                if (rawX) fpu.LD80(top,x,xHigh);
                else { fpu.regCache[top].l = x; fpu.isRegCached[top] = true; }
                U32 other = fpu.STV(1); fpu.LD80(other,divisor.low,yHigh);
                fpu.tags[top] = fpu.tags[other] = TAG_Valid; fpu.sw = 0x8755;
                FPU expected = fpu; U8 mode = (sequence & 1) ? softfloat_round_odd : softfloat_round_max;
                softfloat_roundingMode = mode; softfloat_exceptionFlags = softfloat_flag_infinite;
                referenceSpecial(expected,op); U8 flags = softfloat_exceptionFlags, expectedMode = softfloat_roundingMode;
                fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
                if (dirty) { emitScaleOperand(0,rawX,x,xHigh); emitScaleOperand(1,true,divisor.low,yHigh); }
                context.cpu->xmm[0].pi.u32[0] = 0x3f800000; context.cpu->xmm[0].pi.u32[1] = 0x12345678;
                testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
                testPushCode8(0xb8); testPushCode32(0xffffffff); fp(0x83,0xc0); testPushCode8(1);
                fp(0xd9,op); fp(0x83,0xd0); testPushCode8(0);
                testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
                softfloat_roundingMode = mode; softfloat_exceptionFlags = softfloat_flag_infinite;
                testRunCPU();
                expectSpecialSlot(0,expected,top); expectSpecialSlot(1,expected,other);
                expectFp(2,2); requireFpJit();
                if (fpu.top != top || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                    softfloat_exceptionFlags != flags || softfloat_roundingMode != expectedMode ||
                    context.cpu->reg[0].u32 != 1 || context.cpu->xmm[0].pi.u32[0] != 0x40800000 ||
                    context.cpu->xmm[0].pi.u32[1] != 0x12345678)
                    testFail("raw remainder classification seq=%u op=%x rawX=%u dirty=%u flags=%u expected=%u",
                        sequence,op,rawX,dirty,softfloat_exceptionFlags,flags);
            }
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

namespace {
const struct { U64 x; U16 xHigh; U64 y; U16 yHigh; bool rawX, rawY; } remainderSpecialCases[] = {
    {0xe000000000000001ull,0x4001,0x8000000000000000ull,0x7fff,true,true},
    {1,0x8000,0,0xffff,true,true},
    {0x8000000000000001ull,0x7fff,0x8000000000000000ull,0x4000,true,true},
    {0x8000000000000000ull,0x7ffe,0x8000000000000001ull,0x7fff,true,true},
    {0x7ff0000000000001ull,0,0x8000000000000000ull,0x7fff,false,true},
    {0x401c000000000000ull,0,0xc123456789abcdefull,0xffff,false,true},
    {0x43e0000000000000ull,0,0x3ff0000000000000ull,0,false,false},
    {0xc3e0000000000001ull,0,0,0,false,false},
    {0,0,0,0,false,false},
    {0x7fefffffffffffffull,0,1,0,false,false},
    {0x401c000000000000ull,0,0x8000000000000000ull,0x4000,false,true},
    {0x8000000000000401ull,0x403f,0xc000000000000000ull,0x3fff,true,true}};
void seedRemainderSpecial(U32 index) {
    const auto& item = remainderSpecialCases[index]; auto& fpu = testContext().cpu->fpu;
    if (item.rawX) fpu.LD80(fpu.top,item.x,item.xHigh);
    else { fpu.regCache[fpu.top].l = item.x; fpu.isRegCached[fpu.top] = true; }
    U32 other = fpu.STV(1);
    if (item.rawY) fpu.LD80(other,item.y,item.yHigh);
    else { fpu.regCache[other].l = item.y; fpu.isRegCached[other] = true; }
    fpu.tags[fpu.top] = fpu.tags[other] = TAG_Valid; fpu.sw = 0x8755;
}
}

void testWasmJitFpuRemainderSpecialLoops() {
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    for (U32 top = 0; top < 8; ++top) for (U32 index = 0; index < 12; ++index) {
        for (U8 op : {0xf8,0xf5}) for (bool validate : {false,true}) for (U32 count : {1u,2u,19u}) {
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            seedRemainderSpecial(index); FPU expected = fpu;
            U8 initialMode = top & 3;
            softfloat_roundingMode = initialMode; softfloat_exceptionFlags = softfloat_flag_infinite;
            for (U32 i = 0; i < count; ++i) {
                referenceSpecial(expected,op);
                if (validate) { expected.getF64(top); expected.getF64(expected.STV(1)); }
            }
            U8 flags = softfloat_exceptionFlags, mode = softfloat_roundingMode;
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
            testPushCode8(0xb9); testPushCode32(count);
            U32 loop = context.codeIp; fp(0xd9,op);
            if (validate) { fp(0xd9,0xc0); fp(0xdd,0xd8); fp(0xd9,0xc1); fp(0xdd,0xd8); }
            testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop - (context.codeIp + 1)));
            softfloat_roundingMode = initialMode; softfloat_exceptionFlags = softfloat_flag_infinite;
            testRunCPU();
            expectSpecialSlot(0,expected,top); expectSpecialSlot(1,expected,expected.STV(1));
            expectFp(2,2); requireFpJit();
            if (fpu.top != top || context.cpu->reg[1].u32 || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                softfloat_exceptionFlags != flags || softfloat_roundingMode != mode)
                testFail("remainder special loop top=%u index=%u op=%x count=%u validate=%u",top,index,op,count,validate);
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuRemainderSpecialFaults() {
    for (U32 top = 0; top < 8; ++top) for (U32 index = 0; index < 12; ++index) for (U8 op : {0xf8,0xf5}) {
        beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
        seedRemainderSpecial(index); FPU expected = fpu; referenceSpecial(expected,op);
        fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
        fp(0xd9,op); fpMem(0xdd,3,0x2000);
        auto oldAction = context.process->sigActions[K_SIGSEGV];
        auto& action = context.process->sigActions[K_SIGSEGV];
        action.reset(); action.handlerAndSigAction = context.codeIp;
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS + 0x2000,K_PAGE_SIZE,0);
        testRunCPU();
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS + 0x2000,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE);
        if (action.sigInfo[0] != K_SIGSEGV || fpu.top != top) testFail("remainder special fault lost TOP");
        // A faulting cold FST_F64 leaves raw operands untouched before signal serialization.
        for (U32 i = 0; i < 2; ++i) { expected.getReg(expected.STV(i)); expectSpecialSlot(i,expected,expected.STV(i)); }
        expectFp(2,2);
        context.process->sigActions[K_SIGSEGV] = oldAction;
    }
}

namespace {
struct RemainderExactInput {
    U64 x; U16 xHigh; U64 y; U16 yHigh; bool cachedX = false;
};
struct RemainderSoftFloatState {
    U8 mode = softfloat_roundingMode, flags = softfloat_exceptionFlags;
    U8 tiny = softfloat_detectTininess, precision = extF80_roundingPrecision;
    ~RemainderSoftFloatState() {
        softfloat_roundingMode = mode; softfloat_exceptionFlags = flags;
        softfloat_detectTininess = tiny; extF80_roundingPrecision = precision;
    }
};
void runRemainderExact(const RemainderExactInput& input, U32 top, U8 op, U8 precision,
        U8 mode, bool dirty, U32 count = 1, bool fault = false, bool requireInline = false) {
    RemainderSoftFloatState restore;
    beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
    if (input.cachedX) { fpu.regCache[top].l = input.x; fpu.isRegCached[top] = true; }
    else fpu.LD80(top,input.x,input.xHigh);
    U32 other = fpu.STV(1); fpu.LD80(other,input.y,input.yHigh);
    fpu.tags[top] = fpu.tags[other] = TAG_Valid; fpu.sw = 0xc755;
    FPU expected = fpu;
    U8 tiny = top & 1;
    auto resetSoftFloat = [&] {
        softfloat_roundingMode = mode; extF80_roundingPrecision = precision;
        softfloat_detectTininess = tiny; softfloat_exceptionFlags = softfloat_flag_infinite;
    };
    resetSoftFloat();
    for (U32 i = 0; i < count; ++i) referenceSpecial(expected,op);
    // A cold faulting FST_F64 converts using the guest control word before
    // attempting the write. Its rounding-mode side effect precedes the fault.
    if (fault) expected.FST_F64(context.cpu,TEST_HEAP_ADDRESS + 0x3000);
    U8 flags = softfloat_exceptionFlags, expectedMode = softfloat_roundingMode;
    if (dirty) {
        emitScaleOperand(0,!input.cachedX,input.x,input.xHigh);
        emitScaleOperand(1,true,input.y,input.yHigh);
    }
    context.cpu->xmm[0].pi.u32[0] = 0x3f800000; context.cpu->xmm[0].pi.u32[1] = 0x12345678;
    testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
    testPushCode8(0xb8); testPushCode32(0xffffffff); fp(0x83,0xc0); testPushCode8(1);
    testPushCode8(0xb9); testPushCode32(count);
    U32 loop = context.codeIp; fp(0xd9,op);
    testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop - (context.codeIp + 1)));
    fp(0x83,0xd0); testPushCode8(0);
    testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
    auto oldAction = context.process->sigActions[K_SIGSEGV];
    if (fault) {
        fpMem(0xdd,3,0x2000);
        auto& action = context.process->sigActions[K_SIGSEGV];
        action.reset(); action.handlerAndSigAction = context.codeIp;
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS + 0x2000,K_PAGE_SIZE,0);
    }
    resetSoftFloat(); context.cpu->memHelperValue = 0xdeadbeef;
    testRunCPU();
    U32 savedEax = context.cpu->reg[0].u32, savedEcx = context.cpu->reg[1].u32;
    if (fault) {
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS + 0x2000,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE);
        if (context.process->sigActions[K_SIGSEGV].sigInfo[0] != K_SIGSEGV)
            testFail("exact remainder fault not delivered");
        // Signal entry replaces EAX/ECX with handler arguments. Check the
        // interrupted values saved in ucontext instead.
        U32 signalContext = context.memory->readd(context.cpu->reg[4].u32 + 12);
        savedEax = context.memory->readd(signalContext + 0x40);
        savedEcx = context.memory->readd(signalContext + 0x3c);
        context.process->sigActions[K_SIGSEGV] = oldAction;
    } else {
        requireFpJit();
        // The slot arithmetic helper always writes its arguments here. A pure
        // register case must leave the sentinel alone when reduced inline.
        if (requireInline && context.cpu->memHelperValue != 0xdeadbeef)
            testFail("exact remainder unexpectedly called slot helper op=%x top=%u",op,top);
    }
    expectSpecialSlot(0,expected,top); expectSpecialSlot(1,expected,other);
    if (fpu.top != top || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
        softfloat_exceptionFlags != flags || softfloat_roundingMode != expectedMode ||
        softfloat_detectTininess != tiny || extF80_roundingPrecision != precision ||
        savedEax != 1 || savedEcx ||
        context.cpu->xmm[0].pi.u32[0] != 0x40800000 || context.cpu->xmm[0].pi.u32[1] != 0x12345678)
        testFail("exact remainder state top=%u op=%x precision=%u/%u mode=%u/%u dirty=%u count=%u fault=%u sw=%x/%x flags=%u/%u tiny=%u/%u eax=%x ecx=%x xmm=%x/%x",
            top,op,extF80_roundingPrecision,precision,softfloat_roundingMode,expectedMode,dirty,count,fault,fpu.sw,expected.sw,
            softfloat_exceptionFlags,flags,softfloat_detectTininess,tiny,savedEax,savedEcx,
            context.cpu->xmm[0].pi.u32[0],context.cpu->xmm[0].pi.u32[1]);
}
const RemainderExactInput remainderExactInputs[] = {
    {0,0,0x8000000000000000ull,0x3fff},
    {0,0x8000,0xffffffffffffffffull,0xbfff},
    {0,0,0x8000000000000000ull,0x0001},
    {0,0,0x8000000000000000ull,0x7ffe},
    {0,0,1,0}, // subnormal divisor
    {0,0,0,0}, // zero divisor
    {0,0x3fff,0x8000000000000000ull,0x3fff}, // unnormal zero dividend
    {0,0,0,0x3fff}, // unnormal zero divisor
    {0x8123456789abcdefull,0x3fff,0x8123456789abcdeeull,0x3fff}, // unequal significands
    {0x123456789abcdefull,0x3fff,0x123456789abcdefull,0x3fff}, // unnormal
    {0x8000000000000000ull,0,0x8000000000000000ull,0}, // pseudo-denormal
    {1,0,1,0},
    {0,0,0x8000000000000000ull,0x3fff,true}, // cached +0
    {0x8000000000000000ull,0,0x8000000000000000ull,0x3fff,true}, // cached -0
    {0x3ff0000000000000ull,0,0x8000000000000000ull,0x3f9b,true}, // 1 / 2^-100
    {0x3ff0000000000001ull,0,0x8000000000000800ull,0x3fff,true},
    {0x8000000000000000ull,0x7ffe,0x8000000000000000ull,0x7ffe},
    {0x8000000000000000ull,1,0x8000000000000000ull,1}
};
}

void testWasmJitFpuRemainderExact() {
    U32 sequence = 0;
    for (S32 difference : {-1,0,1,2,3,31,32,62,63,64,100,1081,1082}) {
        for (U64 sig : {0x8000000000000000ull,0x8123450000000000ull,0x8123450000000001ull,
                0x8123456789abc800ull,0x8123456789abc801ull,0xffffffffffffffffull}) {
            for (U8 precision : {32,64,80}) for (U8 mode : {0,1,2,3,4,6}) {
                for (U32 signs = 0; signs < 4; ++signs) for (U8 op : {0xf8,0xf5}) for (bool dirty : {false,true}) {
                    RemainderExactInput input = {sig,(U16)((0x4000+difference) | ((signs & 1) << 15)),
                        sig,(U16)(0x4000 | ((signs & 2) << 14))};
                    runRemainderExact(input,sequence++ & 7,op,precision,mode,dirty);
                }
            }
        }
    }
    // Explicit helper-avoidance checks, independent of the differential oracle.
    for (U32 top = 0; top < 8; ++top) for (U8 op : {0xf8,0xf5}) for (S32 difference : {0,1,2,62,64,100,1081}) {
        RemainderExactInput input = {0x8123456789abcdefull,(U16)(0x4000+difference),0x8123456789abcdefull,0xc000};
        runRemainderExact(input,top,op,80,softfloat_round_min,false,1,false,true);
    }
}

void testWasmJitFpuRemainderExactInputs() {
    U32 sequence = 0;
    for (const auto& input : remainderExactInputs) for (U8 precision : {32,64,80}) {
        for (U8 mode : {0,1,2,3,4,6}) for (U8 op : {0xf8,0xf5}) for (bool dirty : {false,true}) {
            runRemainderExact(input,sequence++ & 7,op,precision,mode,dirty);
        }
    }
    for (U32 signs = 0; signs < 4; ++signs) for (U8 mode : {0,1,2,3,4,6}) for (U8 op : {0xf8,0xf5}) {
        RemainderExactInput input = {0,(U16)((signs & 1) << 15),0x8123456789abcdefull,(U16)(0x3fff | ((signs & 2) << 14))};
        runRemainderExact(input,sequence++ & 7,op,32,mode,false,1,false,true);
    }
}

void testWasmJitFpuRemainderExactLoops() {
    for (U32 top = 0; top < 8; ++top) for (U8 op : {0xf8,0xf5}) for (U32 count : {2u,19u}) {
        for (bool dirty : {false,true}) for (U8 mode : {0,2,6}) {
            for (U32 index : {0u,1u,14u,15u})
                runRemainderExact(remainderExactInputs[index],top,op,80,mode,dirty,count,false,!dirty);
        }
    }
}

void testWasmJitFpuRemainderExactFaults() {
    for (U32 top = 0; top < 8; ++top) for (U8 op : {0xf8,0xf5}) for (U32 count : {1u,2u}) {
        for (U8 mode : {0,2}) for (U32 index : {0u,1u,14u,15u})
            runRemainderExact(remainderExactInputs[index],top,op,80,mode,true,count,true);
    }
}

void testWasmJitFpuRemainderSmallQuotients() {
    U32 sequence = 0;
    for (S32 difference : {-1,-2,-63,-16382,-16383,-32765}) {
        for (U64 sig : {0x8000000000000000ull,0xc000000000000000ull,0x8123450000000000ull,
                0x8123450000000001ull,0x8123456789abc800ull,0xffffffffffffffffull}) {
            for (U8 precision : {32,64,80}) for (U8 mode : {0,1,2,3,4,6}) {
                for (U32 signs = 0; signs < 4; ++signs) for (U8 op : {0xf8,0xf5}) for (bool dirty : {false,true}) {
                    RemainderExactInput input = {sig,(U16)((0x7ffe + difference) | ((signs & 1) << 15)),
                        sig,(U16)(0x7ffe | ((signs & 2) << 14))};
                    runRemainderExact(input,sequence++ & 7,op,precision,mode,dirty);
                }
            }
        }
    }
    // Equal full significands make the division exact even when the dividend
    // is far below binary64's range. Both quotient-rounding rules return zero.
    for (U32 top = 0; top < 8; ++top) for (U8 op : {0xf8,0xf5}) for (U8 precision : {32,64,80}) {
        for (S32 difference : {-1,-2,-63,-16382}) {
            RemainderExactInput input = {0xc000000000000000ull,(U16)(0x7ffe + difference),0xc000000000000000ull,0xfffe};
            runRemainderExact(input,top,op,precision,softfloat_round_min,false,1,false,true);
        }
    }
}

namespace {
const RemainderExactInput remainderPowerInputs[] = {
    {0xc000000000000000ull,0x3fff,0x8000000000000000ull,0x4000}, // 1.5/2, nearest rounds up
    {0xe000000000000000ull,0x4001,0x8000000000000000ull,0x4000}, // 7/2, odd halfway quotient
    {0xa000000000000000ull,0x4001,0x8000000000000000ull,0x4000}, // 5/2, even halfway quotient
    {0x8123456789abcdefull,0x4063,0x8000000000000000ull,0x3fff}, // partial then complete
    {0x8000000000000001ull,1,0x8000000000000000ull,1}, // smallest subnormal remainder
    {0xffffffffffffffffull,1,0x8000000000000000ull,1}, // negative subnormal remainder, nearest
    {0x8000000000000001ull,2,0x8000000000000000ull,1},
    {0xc000000000000000ull,0x3fff,0xc000000000000000ull,0x4001}, // equal-significand zero quotient
    {0x401c000000000000ull,0,0x8000000000000000ull,0x4000,true}, // cached 7 / raw 2
    {0x8000000000000001ull,0x7ffe,0x8000000000000000ull,0x7ffe}, // product-overflow guard
    {0x8000000000000001ull,0,0x8000000000000000ull,1}, // subnormal dividend fallback
    {0x8000000000000001ull,1,0x8000000000000000ull,0}, // pseudo-denormal divisor fallback
    {0x0123456789abcdefull,0x3fff,0x8000000000000000ull,0x3fff}, // unnormal dividend fallback
    {0x8000000000000001ull,0x3fff,0x8000000000000001ull,0x4000}, // precision guard on zero quotient
    {0xffffffffffffffffull,0x403d,0x8000000000000000ull,0x3fff}, // nearest signed quotient overflow
    {0xffffffffffffffffull,0x7ffe,0x8000000000000000ull,0x7ffe} // rounded product would overflow
};
}

void testWasmJitFpuRemainderPowerDivisors() {
    U32 sequence = 0;
    for (S32 difference : {-16383,-16382,-2,-1,0,1,2,30,31,32,52,53,61,62,63,64,65,100,1081,1082}) {
        U16 yExp = difference < 0 ? 0x6000 : 0x4000;
        for (U64 sig : {0x8000000000000001ull,0x8000000000000800ull,0x8123450000000000ull,
                0x8123456789abc801ull,0xa000000000000000ull,0xc000000000000000ull,
                0xe000000000000000ull,0xfffffffffffffffeull,0xffffffffffffffffull}) {
            for (U8 precision : {32,64,80}) for (U8 mode : {0,1,2,3,4,6}) {
                for (U32 signs = 0; signs < 4; ++signs) for (U8 op : {0xf8,0xf5}) for (bool dirty : {false,true}) {
                    RemainderExactInput input = {sig,(U16)((yExp+difference) | ((signs & 1) << 15)),
                        0x8000000000000000ull,(U16)(yExp | ((signs & 2) << 14))};
                    runRemainderExact(input,sequence++ & 7,op,precision,mode,dirty);
                }
            }
        }
    }
    for (const auto& input : remainderPowerInputs) for (U8 precision : {32,64,80}) {
        for (U8 mode : {0,1,2,3,4,6}) for (U8 op : {0xf8,0xf5}) for (bool dirty : {false,true})
            runRemainderExact(input,sequence++ & 7,op,precision,mode,dirty);
    }
    for (U32 top = 0; top < 8; ++top) for (U8 op : {0xf8,0xf5}) {
        for (U32 index = 0; index < 9; ++index)
            runRemainderExact(remainderPowerInputs[index],top,op,80,softfloat_round_min,false,1,false,true);
    }
}

void testWasmJitFpuRemainderPowerLoops() {
    for (U32 top = 0; top < 8; ++top) for (U8 op : {0xf8,0xf5}) for (U32 count : {2u,19u}) {
        for (bool dirty : {false,true}) for (U8 mode : {0,2,6}) for (U32 index = 0; index < 9; ++index) {
            // A subnormal remainder on the next iteration intentionally falls
            // back; ordinary/partial reductions must stay inline throughout.
            bool requireInline = !dirty && (index < 4 || index >= 7);
            runRemainderExact(remainderPowerInputs[index],top,op,80,mode,dirty,count,false,requireInline);
        }
    }
}

void testWasmJitFpuRemainderPowerFaults() {
    for (U32 top = 0; top < 8; ++top) for (U8 op : {0xf8,0xf5}) for (U32 count : {1u,2u}) {
        for (U8 mode : {0,2}) for (U32 index = 0; index < 9; ++index)
            runRemainderExact(remainderPowerInputs[index],top,op,80,mode,true,count,true);
    }
}

namespace {
U64 normalizedIntegerSignificand(U64 value) {
    while (!(value & (1ull << 63))) value <<= 1;
    return value;
}
const RemainderExactInput remainderDyadicInputs[] = {
    {0xf000000000000000ull,0x4002,0xc000000000000000ull,0x4001}, // 15/6, even halfway quotient
    {0xa800000000000000ull,0x4003,0xc000000000000000ull,0x4001}, // 21/6, odd halfway quotient
    {0x9000000000000000ull,0x4002,0xc000000000000000ull,0x4002}, // 9/12, zero or one quotient
    {0xa800000000000015ull,0x4063,0xc000000000000000ull,0x3fff}, // partial reduction with remainder
    {0xc000000000000003ull,1,0xc000000000000000ull,1}, // exact positive subnormal
    {0xbffffffffffffffdull,1,0xc000000000000000ull,1}, // exact negative subnormal, nearest
    {0xffffffffffffffffull,0x403c,0xa000000000000000ull,0x3fff}, // integral reduced numerator, no shift
    {0xc0000000c0000000ull,0x401f,0x8000000080000000ull,0x3fff}, // left-shifted reduced numerator
    {0x4035000000000000ull,0,0xc000000000000000ull,0x4001,true}, // cached 21 / raw 6
    {0xa800000000000016ull,0x4063,0xc000000000000000ull,0x3fff}, // odd factor does not divide
    {0xa800000000000000ull,0,0xc000000000000000ull,1}, // subnormal input fallback
    {0xa800000000000000ull,1,0xc000000000000000ull,0}, // pseudo-denormal input fallback
    {0x2800000000000000ull,0x4003,0xc000000000000000ull,0x4001}, // unnormal input fallback
    {0xa800000000000000ull,0x7ffe,0xc000000000000000ull,0x7ffc}, // product-boundary guard
    {0xa800000000000000ull,0x403d,0xc000000000000000ull,0x3fff}, // quotient-boundary guard
    {0xa800000000000000ull,0x3ffd,0xc000000000000000ull,0x7fff} // infinite divisor, earlier path
};
}

void testWasmJitFpuRemainderDyadic() {
    const struct { U64 odd, numerator; } pairs[] = {
        {3,3},{3,5},{3,7},{5,3},{5,7},{7,5},{9,7},{17,31},{65537,127},
        {0x100000001ull,0x1fffff},{0x10000000001ull,0x1fffff},
        {3,0x5555555555555555ull},{5,0x3333333333333333ull}};
    U32 sequence = 0;
    for (const auto& pair : pairs) {
        U64 x = normalizedIntegerSignificand(pair.odd * pair.numerator);
        U64 y = normalizedIntegerSignificand(pair.odd);
        for (S32 difference : {-16382,-16381,-3,-1,0,1,2,31,32,53,61,62,63,64,100,1081,1082}) {
            U16 yExp = difference < 0 ? 0x6000 : 0x4000;
            for (U8 precision : {32,64,80}) for (U8 mode : {0,1,2,3,4,6}) {
                for (U32 signs = 0; signs < 4; ++signs) for (U8 op : {0xf8,0xf5}) for (bool dirty : {false,true}) {
                    RemainderExactInput input = {x,(U16)((yExp + difference) | ((signs & 1) << 15)),
                        y,(U16)(yExp | ((signs & 2) << 14))};
                    runRemainderExact(input,sequence++ & 7,op,precision,mode,dirty);
                }
            }
        }
    }
}

void testWasmJitFpuRemainderDyadicInputs() {
    U32 sequence = 0;
    for (const auto& input : remainderDyadicInputs) for (U8 precision : {32,64,80}) {
        for (U8 mode : {0,1,2,3,4,6}) for (U8 op : {0xf8,0xf5}) for (bool dirty : {false,true})
            runRemainderExact(input,sequence++ & 7,op,precision,mode,dirty);
    }
    // Fixed-seed operand variation also perturbs exact significands by one bit
    // to verify that inexact division retains the SoftFloat flag behavior.
    U32 random = 0x835a71d9;
    const U8 modes[] = {0,1,2,3,4,6};
    for (U32 i = 0; i < 256; ++i) {
        random = random * 1664525u + 1013904223u; U64 odd = (random & 0xffffff) | 3;
        random = random * 1664525u + 1013904223u; U64 n = (random & 0xffffff) | 1;
        U64 x = normalizedIntegerSignificand(odd*n), y = normalizedIntegerSignificand(odd);
        if (i & 1) x ^= 1;
        S32 difference = (i % 7 == 0) ? 100 : (S32)(i % 65) - 3;
        RemainderExactInput input = {x,(U16)((0x4000 + difference) | ((i & 2) << 14)),
            y,(U16)(0x4000 | ((i & 4) << 13))};
        for (U8 precision : {32,64,80}) for (U8 op : {0xf8,0xf5})
            runRemainderExact(input,sequence++ & 7,op,precision,modes[i % 6],i & 8);
    }
    for (U32 top = 0; top < 8; ++top) for (U8 op : {0xf8,0xf5}) for (U32 index = 0; index < 9; ++index)
        runRemainderExact(remainderDyadicInputs[index],top,op,80,softfloat_round_min,false,1,false,true);
}

void testWasmJitFpuRemainderDyadicLoops() {
    for (U32 top = 0; top < 8; ++top) for (U8 op : {0xf8,0xf5}) for (U32 count : {2u,19u}) {
        for (bool dirty : {false,true}) for (U8 mode : {0,2,6}) for (U32 index = 0; index < 9; ++index) {
            bool requireInline = !dirty && index != 4 && index != 5; // subsequent subnormal operands use the helper
            runRemainderExact(remainderDyadicInputs[index],top,op,80,mode,dirty,count,false,requireInline);
        }
    }
}

void testWasmJitFpuRemainderDyadicFaults() {
    for (U32 top = 0; top < 8; ++top) for (U8 op : {0xf8,0xf5}) for (U32 count : {1u,2u}) {
        for (U8 mode : {0,2}) for (U32 index = 0; index < 9; ++index)
            runRemainderExact(remainderDyadicInputs[index],top,op,80,mode,true,count,true);
    }
}

void testWasmJitFpuScaleExtendedRounding() {
    const struct { U64 low; U16 high; } values[] = {
        {0,0},{1,0},{0xffffffffffffffffull,0},{0,0x3fff},{1,0x3fff},
        {0x8000000000000000ull,0x3fff},{0x80000000000003ffull,0x3fff},
        {0x8000000000000400ull,0x3fff},{0x8000000000000401ull,0x3fff},
        {0x8000000000000c00ull,0x3fff},{0xffffffffffffffffull,0x3fff},
        {0x8000000000000000ull,0x3c01},{0xffffffffffffffffull,0x3c00},
        {0x8000000000000000ull,0x3bcd},{0x8000000000000001ull,0x3bcd},
        {0x8000000000000000ull,0x3bcc},{0xffffffffffffffffull,0x3bcc},
        {0xfffffffffffff800ull,0x43fe},{0xfffffffffffffbffull,0x43fe},
        {0xfffffffffffffc00ull,0x43fe},{0x8000000000000000ull,0x43ff},
        {0x8000000000000000ull,0x7ffe},{0x8000000000000000ull,0x7fff},
        {0,0x7fff},{0x8000000000000001ull,0x7fff},{0xc123456789abcdefull,0x7fff},
        {0x0123456789abcdefull,0x3fff},{0x8123456789abcdefull,0x0064},
        {0xffffffffffffffffull,0x4008},{0x8000000000000000ull,0x4009},
        {0x8660000000000000ull,0x4009},{0x865fffffffffffffull,0x4009}};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    auto oldTiny = softfloat_detectTininess;
    U32 sequence = 0;
    for (const auto& value : values) for (bool negative : {false,true}) {
        U16 high = value.high | (negative ? 0x8000 : 0);
        for (U8 mode : {0,1,2,3,4,6}) for (U8 tiny : {0,1}) {
            for (U32 slot : {0u,1u}) for (bool dirty : {false,true}) {
                U32 top = sequence++ & 7;
                beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
                fpu.SetCW(0x037f | ((sequence & 3) << 10)); fpu.sw = 0x8755;
                seedMathValue(fpu,top,1,false); seedMathValue(fpu,fpu.STV(1),0,false);
                fpu.LD80(fpu.STV(slot),value.low,high);
                FPU expected = fpu;
                softfloat_roundingMode = mode; softfloat_detectTininess = tiny;
                softfloat_exceptionFlags = softfloat_flag_infinite;
                expected.FSCALE(); U8 flags = softfloat_exceptionFlags;
                fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
                if (dirty) emitScaleOperand(slot,true,value.low,high);
                context.cpu->xmm[0].pi.u32[0] = 0x3f800000; context.cpu->xmm[0].pi.u32[1] = 0x12345678;
                testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
                testPushCode8(0xb8); testPushCode32(0xffffffff); fp(0x83,0xc0); testPushCode8(1);
                fp(0xd9,0xfd); fp(0x83,0xd0); testPushCode8(0);
                testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
                softfloat_exceptionFlags = softfloat_flag_infinite;
                testRunCPU();
                expectSpecialSlot(0,expected,top); expectSpecialSlot(1,expected,expected.STV(1));
                expectFp(2,2); requireFpJit();
                if (fpu.top != top || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                    softfloat_exceptionFlags != flags || softfloat_roundingMode != mode ||
                    context.cpu->reg[0].u32 != 1 || context.cpu->xmm[0].pi.u32[0] != 0x40800000 ||
                    context.cpu->xmm[0].pi.u32[1] != 0x12345678)
                    testFail("raw FSCALE rounding seq=%u slot=%u mode=%u tiny=%u flags=%u expected=%u",
                        sequence,slot,mode,tiny,softfloat_exceptionFlags,flags);
            }
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode; softfloat_detectTininess = oldTiny;
}

void testWasmJitFpuScaleExtendedSpecial() {
    const struct { U64 low; U16 high; U64 cached; } values[] = {
        {0,0,0},{1,0,1},{0,0x3fff,0x3ff0000000000000ull},
        {0x8000000000000000ull,0x3fff,0x3ff0000000000000ull},
        {0xe000000000000001ull,0x4001,0x401c000000000000ull},
        {0x8000000000000000ull,0x7ffe,0x7fefffffffffffffull},
        {0x0123456789abcdefull,0x3fff,0x3ff0000000000001ull},
        {0x8000000000000000ull,0x7fff,0x7ff0000000000000ull},
        {0,0x7fff,0x7ff0000000000000ull},
        {0x8000000000000001ull,0x7fff,0x7ff0000000000001ull},
        {0xc123456789abcdefull,0x7fff,0x7ff8123456789abcull},
        {0x0123456789abcdefull,0x7fff,0x7ff0000000000400ull}};
    const struct { U64 low, cached; } scales[] = {
        {0x8000000000000000ull,0x7ff0000000000000ull},{0,0x7ff0000000000000ull},
        {0x8000000000000001ull,0x7ff0000000000001ull},{0xc123456789abcdefull,0x7ff8123456789abcull}};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    U32 sequence = 0;
    for (const auto& value : values) for (bool negative : {false,true}) {
        for (const auto& scale : scales) for (bool scaleNegative : {false,true}) for (U32 representation = 0; representation < 3; ++representation) {
            U32 top = sequence++ & 7;
            beginFp(top); auto& fpu = testContext().cpu->fpu;
            bool rawX = representation != 1, rawY = representation != 2;
            U64 x = rawX ? value.low : value.cached | (negative ? 1ull << 63 : 0);
            U16 xHigh = value.high | (negative ? 0x8000 : 0);
            U64 y = rawY ? scale.low : scale.cached | (scaleNegative ? 1ull << 63 : 0);
            U16 yHigh = scaleNegative ? 0xffff : 0x7fff;
            if (rawX) fpu.LD80(top,x,xHigh); else { fpu.regCache[top].l = x; fpu.isRegCached[top] = true; }
            U32 other = fpu.STV(1);
            if (rawY) fpu.LD80(other,y,yHigh); else { fpu.regCache[other].l = y; fpu.isRegCached[other] = true; }
            fpu.tags[top] = fpu.tags[other] = TAG_Valid; fpu.sw = 0x8755;
            FPU expected = fpu; U8 mode = (sequence & 1) ? softfloat_round_odd : softfloat_round_max;
            softfloat_roundingMode = mode; softfloat_exceptionFlags = softfloat_flag_infinite;
            expected.FSCALE(); U8 flags = softfloat_exceptionFlags;
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
            emitScaleOperand(0,rawX,x,xHigh); emitScaleOperand(1,rawY,y,yHigh);
            fp(0xd9,0xfd); softfloat_exceptionFlags = softfloat_flag_infinite;
            testRunCPU();
            expectSpecialSlot(0,expected,top); expectSpecialSlot(1,expected,other);
            expectFp(2,2); requireFpJit();
            if (fpu.top != top || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                softfloat_exceptionFlags != flags || softfloat_roundingMode != mode)
                testFail("raw FSCALE classification seq=%u rep=%u flags=%u expected=%u",sequence,representation,softfloat_exceptionFlags,flags);
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

namespace {
const struct { U64 x; U16 xHigh; U64 y; U16 yHigh; } scaleExtendedCases[] = {
    {0x8000000000000400ull,0x3fff,0,0},
    {0x8000000000000001ull,0x3bcd,0x8000000000000000ull,0xbfff},
    {0xfffffffffffffc00ull,0xc3fe,0,0},
    {0x8000000000000000ull,0x7ffe,0x8000000000000000ull,0xffff},
    {1,0x8000,0x8000000000000000ull,0x7fff},
    {0,0xffff,0,0x7fff},
    {0x8000000000000001ull,0x7fff,0x8000000000000001ull,0xffff},
    {0x8000000000000001ull,0xffff,0x8000000000000000ull,0x7fff}};
void seedScaleExtended(U32 index) {
    const auto& item = scaleExtendedCases[index]; auto& fpu = testContext().cpu->fpu;
    fpu.LD80(fpu.top,item.x,item.xHigh); fpu.LD80(fpu.STV(1),item.y,item.yHigh);
    fpu.tags[fpu.top] = fpu.tags[fpu.STV(1)] = TAG_Valid; fpu.sw = 0x8755;
}
}

void testWasmJitFpuScaleExtendedLoops() {
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    for (U32 top = 0; top < 8; ++top) for (U32 index = 0; index < 8; ++index) {
        for (bool validate : {false,true}) for (U32 count : {1u,2u,19u}) {
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            seedScaleExtended(index); FPU expected = fpu;
            U8 initialMode = top & 3;
            softfloat_roundingMode = initialMode; softfloat_exceptionFlags = softfloat_flag_infinite;
            for (U32 i = 0; i < count; ++i) {
                expected.FSCALE();
                if (validate) { expected.getF64(top); expected.getF64(expected.STV(1)); }
            }
            U8 flags = softfloat_exceptionFlags, mode = softfloat_roundingMode;
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
            testPushCode8(0xb9); testPushCode32(count);
            U32 loop = context.codeIp; fp(0xd9,0xfd);
            if (validate) { fp(0xd9,0xc0); fp(0xdd,0xd8); fp(0xd9,0xc1); fp(0xdd,0xd8); }
            testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop - (context.codeIp + 1)));
            softfloat_roundingMode = initialMode; softfloat_exceptionFlags = softfloat_flag_infinite;
            testRunCPU();
            expectSpecialSlot(0,expected,top); expectSpecialSlot(1,expected,expected.STV(1));
            expectFp(2,2); requireFpJit();
            if (fpu.top != top || context.cpu->reg[1].u32 || (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u) ||
                softfloat_exceptionFlags != flags || softfloat_roundingMode != mode)
                testFail("extended scale loop top=%u index=%u count=%u validate=%u",top,index,count,validate);
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuScaleExtendedFaults() {
    for (U32 top = 0; top < 8; ++top) for (U32 index = 0; index < 8; ++index) {
        beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
        seedScaleExtended(index); FPU expected = fpu; expected.FSCALE();
        fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
        fp(0xd9,0xfd); fpMem(0xdd,3,0x2000);
        auto oldAction = context.process->sigActions[K_SIGSEGV];
        auto& action = context.process->sigActions[K_SIGSEGV];
        action.reset(); action.handlerAndSigAction = context.codeIp;
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS + 0x2000,K_PAGE_SIZE,0);
        testRunCPU();
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS + 0x2000,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE);
        if (action.sigInfo[0] != K_SIGSEGV || fpu.top != top) testFail("extended scaling fault lost TOP");
        // The memory guard takes the interpreter path. FST_F64 converts its
        // output without caching it, so a failed store preserves the original
        // raw slot (including noncanonical infinity) for signal serialization.
        for (U32 i = 0; i < 2; ++i) { expected.getReg(expected.STV(i)); expectSpecialSlot(i,expected,expected.STV(i)); }
        expectFp(2,2);
        context.process->sigActions[K_SIGSEGV] = oldAction;
    }
}

void testWasmJitFpuExtractScaleRangeLoops() {
    const struct { U64 bits; double scale; } cases[] = {
        {1,-1},{0x000fffffffffffffull,1},{0x0010000000000000ull,-1},
        {0x7fefffffffffffffull,1024},{0,1024},{0x7ff0000000000001ull,-1075}};
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    for (U32 top = 0; top < 8; ++top) for (const auto& item : cases) {
        for (bool extract : {false,true}) for (U32 count : {1u,2u,19u}) {
            beginFp(top); auto& context = testContext(); auto& fpu = context.cpu->fpu;
            fpu.regCache[top].l = item.bits; fpu.isRegCached[top] = true; fpu.tags[top] = TAG_Valid;
            seedMathValue(fpu,fpu.STV(1),item.scale,false);
            FPU expected = fpu;
            softfloat_exceptionFlags = softfloat_flag_infinite;
            softfloat_roundingMode = softfloat_round_max;
            for (U32 i = 0; i < count; ++i) {
                if (extract) {
                    expected.FXTRACT(); expected.getF64(expected.top); expected.FPOP();
                } else {
                    expected.getF64(expected.top); expected.getF64(expected.STV(1)); expected.FSCALE();
                }
            }
            U8 flags = softfloat_exceptionFlags, mode = softfloat_roundingMode;
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb); // dirty ST2
            context.cpu->reg[1].u32 = count;
            U32 loop = context.codeIp;
            if (extract) {
                fp(0xd9,0xf4); fp(0xdd,0xd8);
            } else {
                // Revalidate raw results into locals before each scaling.
                fp(0xd9,0xc0); fp(0xdd,0xd8); fp(0xd9,0xc1); fp(0xdd,0xd8);
                fp(0xd9,0xfd);
            }
            testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop-context.codeIp-1));
            softfloat_exceptionFlags = softfloat_flag_infinite;
            softfloat_roundingMode = softfloat_round_max;
            testRunCPU();
            expectSpecialSlot(0,expected,expected.top); expectSpecialSlot(1,expected,expected.STV(1));
            expectFp(2,2); requireFpJit();
            if (fpu.top != top || context.cpu->reg[1].u32 || softfloat_exceptionFlags != flags || softfloat_roundingMode != mode)
                testFail("extract/scale range loop top=%u extract=%u count=%u bits=%llx flags=%u expected=%u mode=%u expected=%u",top,extract,count,item.bits,softfloat_exceptionFlags,flags,softfloat_roundingMode,mode);
        }
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuSpecialLoops() {
    for(U32 top=0;top<8;++top) {
        for(U8 op : {0xf8,0xf5,0xfd,0xf4}) for(bool raw : {false,true}) {
            beginFp(top); auto& context=testContext();
            seedMathValue(context.cpu->fpu,top,7.0,raw);
            seedMathValue(context.cpu->fpu,context.cpu->fpu.STV(1),2.0,false);
            FPU expected=context.cpu->fpu;
            for(U32 i=0;i<11;++i) {
                referenceSpecial(expected,op);
                if(op==0xf4) expected.FPOP(); // consume mantissa; next input is exponent
            }
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
            testPushCode8(0xb9); testPushCode32(11);
            U32 loop=context.codeIp;
            fp(0xd9,op);
            if(op==0xf4) fp(0xdd,0xd8); // fstp st0
            testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop-(context.codeIp+1)));
            testRunCPU();
            expectSpecialSlot(0,expected,expected.top);
            expectSpecialSlot(1,expected,expected.STV(1)); expectFp(2,2);
            requireFpJit();
        }
        // A raw divisor selects partial extended reduction (C2=1). Repeating
        // until C2 clears must retain the exact remainder and condition bits.
        beginFp(top); auto& context=testContext(); auto& fpu=context.cpu->fpu;
        seedMathValue(fpu,top,0x1p100,true); seedMathValue(fpu,fpu.STV(1),3.0,true);
        FPU expected=fpu; U32 count=0;
        do {expected.FPREM(); ++count;} while((expected.sw & 0x400) && count<16);
        if(count>=16) testFail("reference partial remainder did not converge");
        fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
        U32 loop=context.codeIp;
        fp(0xd9,0xf8); fp(0xdf,0xe0); // fnstsw ax
        testPushCode8(0xf6); testPushCode8(0xc4); testPushCode8(4); // test ah,4
        testPushCode8(0x75); testPushCode8((U8)(loop-(context.codeIp+1)));
        testRunCPU();
        expectSpecialSlot(0,expected,expected.top); expectFp(2,2);
        if(fpu.sw & 0x400) testFail("partial remainder loop did not clear C2");
        requireFpJit();
    }
}

void testWasmJitFpuSpecialFaults() {
    for(U8 op : {0xf8,0xf5,0xfd,0xf4}) {
        for(bool raw : {false,true}) {
            beginFp(3); auto& context=testContext(); auto& fpu=context.cpu->fpu;
            seedMathValue(fpu,3,7.0,raw); seedMathValue(fpu,4,2.0,raw);
            FPU expected=fpu; referenceSpecial(expected,op);
            fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb);
            fp(0xd9,op); fpMem(0xdd,3,0x2000);
            auto oldAction=context.process->sigActions[K_SIGSEGV];
            auto& action=context.process->sigActions[K_SIGSEGV];
            action.reset(); action.handlerAndSigAction=context.codeIp;
            context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS+0x2000,K_PAGE_SIZE,0);
            testRunCPU();
            context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS+0x2000,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE);
            if(action.sigInfo[0]!=K_SIGSEGV || fpu.top!=expected.top) testFail("special arithmetic fault lost TOP");
            // The double store may convert ST(0) before faulting. Signal-frame
            // serialization then converts every slot to extended precision.
            expected.getF64(expected.top);
            U32 values=op==0xf4 ? 3 : 2;
            for(U32 i=0;i<values;++i) {
                expected.getReg(expected.STV(i));
                expectSpecialSlot(i,expected,expected.STV(i));
            }
            expectFp(values,2);
            context.process->sigActions[K_SIGSEGV]=oldAction;
        }
    }
}

namespace {
void checkExtendedConversion(U64 low, U16 high, U32 sequence, U8 tininess) {
    beginFp(sequence & 7);
    auto& context = testContext(); auto& fpu = context.cpu->fpu;
    U32 top = fpu.top, source = fpu.STV(1);
    const U64 neighbor = 0x8123456789abcdefull;
    fpu.LD80(top, neighbor, 0x4000); fpu.tags[top] = TAG_Valid;
    fpu.LD80(source, low, high); fpu.tags[source] = TAG_Valid;
    fpu.SetCW(0x037f | ((sequence & 3) << 10));
    FPU expected = fpu;
    softfloat_exceptionFlags = softfloat_flag_infinite;
    softfloat_detectTininess = tininess;
    expected.getF64(source);
    U64 bits = expected.regCache[source].l;
    U8 flags = softfloat_exceptionFlags;
    fp(0xd9,0xe8); fp(0xd8,0xc0); fp(0xdd,0xdb); // unrelated dirty ST(2) = 2
    context.cpu->xmm[0].pi.u32[0] = 0x3f800000;
    context.cpu->xmm[0].pi.u32[1] = 0x12345678;
    testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
    testPushCode8(0xb8); testPushCode32(0xffffffff);
    fp(0x83,0xc0); testPushCode8(1); // carry across conversion
    fp(0xd9,0xc1); fpMem(0xdd,3,0x200); // duplicate ST(1), then store/pop
    fp(0x83,0xd0); testPushCode8(0);
    testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58,0xc0);
    softfloat_exceptionFlags = softfloat_flag_infinite;
    softfloat_roundingMode = softfloat_round_max;
    testRunCPU();
    if (context.memory->readq(TEST_HEAP_ADDRESS+0x200) != bits ||
        !fpu.isRegCached[source] || fpu.regCache[source].l != bits ||
        softfloat_exceptionFlags != flags || softfloat_roundingMode != softfloat_round_near_even)
        testFail("extended conversion %04x:%016llx bits=%016llx expected=%016llx flags=%u expected=%u mode=%u",
            high,(unsigned long long)low,(unsigned long long)context.memory->readq(TEST_HEAP_ADDRESS+0x200),
            (unsigned long long)bits,softfloat_exceptionFlags,flags,softfloat_roundingMode);
    if (fpu.top != top || fpu.isRegCached[top] || fpu.regs[top].signif != neighbor || fpu.regs[top].signExp != 0x4000)
        testFail("converting ST(1) changed unrelated raw ST(0)");
    if (context.cpu->reg[0].u32 != 1 || context.cpu->xmm[0].pi.u32[0] != 0x40800000 ||
        context.cpu->xmm[0].pi.u32[1] != 0x12345678)
        testFail("extended conversion lost GP/XMM locals");
    expectFp(2,2); requireFpJit();
}
}

void testWasmJitFpuConvert() {
    auto oldMode=softfloat_roundingMode, oldFlags=softfloat_exceptionFlags, oldTiny=softfloat_detectTininess;
    const U16 exponents[]={0,1,0x3bbf,0x3bc0,0x3bc1,0x3bcb,0x3bcc,0x3bcd,
        0x3bce,0x3bfe,0x3bff,0x3c00,0x3c01,0x3c02,0x3ffe,0x3fff,0x4000,
        0x43fc,0x43fd,0x43fe,0x43ff,0x4400,0x7ffe,0x7fff};
    const U64 significands[]={0,1,0x3ff,0x400,0x401,0x7fffffffffffffffull,
        0x8000000000000000ull,0x80000000000003ffull,0x8000000000000400ull,
        0x8000000000000401ull,0x8000000000000bffull,0x8000000000000c00ull,
        0x8000000000000c01ull,0x8123456789abcdefull,0xbfffffffffffffffull,
        0xc000000000000000ull,0xc123456789abcdefull,0xfffffffffffff7ffull,
        0xfffffffffffff800ull,0xfffffffffffffbffull,0xfffffffffffffc00ull,
        0xfffffffffffffc01ull,0xffffffffffffffffull};
    U32 sequence=0;
    for (U8 tininess : {softfloat_tininess_beforeRounding,softfloat_tininess_afterRounding})
        for (U16 exponent : exponents) for (U64 significand : significands) for (U16 sign : {0,0x8000})
            checkExtendedConversion(significand,exponent|sign,sequence++,tininess);
    // Deterministic noncanonical payloads across the entire extended exponent range.
    U64 state=0xa0761d6478bd642full;
    for (U32 i=0;i<768;++i) {
        state ^= state<<13; state ^= state>>7; state ^= state<<17;
        U64 low=state;
        state ^= state<<13; state ^= state>>7; state ^= state<<17;
        checkExtendedConversion(low,(U16)state,sequence++,i&1);
    }
    softfloat_roundingMode=oldMode; softfloat_exceptionFlags=oldFlags; softfloat_detectTininess=oldTiny;
}

void testWasmJitFpuConvertLoops() {
    // Each iteration overwrites a previously cached position with raw data.
    // The conversion must see the current operand, including after TOP rotates.
    const struct { U64 low; U16 high; } values[]={
        {0x8000000000000400ull,0x3fff},{0x8000000000000c00ull,0xbfff},
        {0xfffffffffffff800ull,0x3c00},{0x8000000000000000ull,0x3bcd},
        {0xffffffffffffffffull,0x43fe},{0x8000000000000001ull,0x7fff},
        {0xc123456789abcdefull,0xffff},{0,0x8000}};
    for(U32 top=0;top<8;++top) for(bool rotate : {false,true}) {
        beginFp(top); auto& context=testContext(); auto& fpu=context.cpu->fpu;
        U64 expected[8];
        for(U32 i=0;i<8;++i) {
            context.memory->writeq(TEST_HEAP_ADDRESS+0x100+i*16,values[i].low);
            context.memory->writew(TEST_HEAP_ADDRESS+0x108+i*16,values[i].high);
            FPU reference; reference.FINIT(); reference.LD80(0,values[i].low,values[i].high);
            reference.getF64(0); expected[i]=reference.regCache[0].l;
        }
        testPushCode8(0xbe); testPushCode32(0x100); // esi source
        testPushCode8(0xbf); testPushCode32(0x300); // edi output
        testPushCode8(0xb9); testPushCode32(8);
        U32 loop=context.codeIp;
        fp(0xdb,0x2e); // fld extended [esi]
        fp(0xdd,0x1f); // fstp double [edi]
        if(rotate) fp(0xd9,0xf6); // fdecstp: rotate header slots
        fp(0x83,0xc6); testPushCode8(16);
        fp(0x83,0xc7); testPushCode8(8);
        testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop-(context.codeIp+1)));
        testRunCPU();
        for(U32 i=0;i<8;++i)
            if(context.memory->readq(TEST_HEAP_ADDRESS+0x300+i*8)!=expected[i])
                testFail("extended conversion loop %u rotate=%u",i,rotate);
        if(fpu.top!=top) testFail("extended conversion loop TOP");
        requireFpJit();
    }
}

void testWasmJitFpuConvertFaults() {
    for(U32 top=0;top<8;++top) {
        beginFp(top); auto& context=testContext(); auto& fpu=context.cpu->fpu;
        fpu.LD80(top,0x8123456789abcdefull,0x4000); fpu.tags[top]=TAG_Valid;
        fpu.LD80(fpu.STV(1),0x8000000000000c00ull,0x3fff); fpu.tags[fpu.STV(1)]=TAG_Valid;
        FPU expected=fpu; expected.getF64(expected.STV(1));
        expected.PREP_PUSH(); expected.regCache[expected.top]=expected.regCache[expected.STV(2)];
        expected.isRegCached[expected.top]=true;
        fp(0xd9,0xc1); // duplicate/convert ST(1), leaving original ST(0) raw
        testPushCode8(0xa1); testPushCode32(0x2000); // integer fault with all values live
        auto oldAction=context.process->sigActions[K_SIGSEGV];
        auto& action=context.process->sigActions[K_SIGSEGV];
        action.reset(); action.handlerAndSigAction=context.codeIp;
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS+0x2000,K_PAGE_SIZE,0);
        testRunCPU();
        context.memory->mprotect(context.thread,TEST_HEAP_ADDRESS+0x2000,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE);
        if(action.sigInfo[0]!=K_SIGSEGV || fpu.top!=expected.top) testFail("conversion fault lost TOP");
        for(U32 i=0;i<3;++i) {
            expected.getReg(expected.STV(i)); // signal frame stores raw extended registers
            expectSpecialSlot(i,expected,expected.STV(i));
        }
        context.process->sigActions[K_SIGSEGV]=oldAction;
    }
}

namespace {
void envMem(U8 group, U32 address, bool big) {
    fp(0xd9, (group << 3) | (big ? 5 : 6));
    if (big) testPushCode32(address);
    else testPushCode16((U16)address);
}
void seedEnvironment(U32 top) {
    beginFp(top);
    auto& fpu = testContext().cpu->fpu;
    for (U32 i = 0; i < 8; ++i) {
        fpu.FLD_I64(11 + i, i);
        fpu.tags[i] = TAG_Valid;
        if (i & 1) fpu.getF64(i);
    }
    for (U32 i = 0; i < 4; ++i) fpu.envData[i] = 0x12340000 + i * 17;
}
void dirtyEnvironment(FPU& expected) {
    fp(0xd9, 0xe8); expected.FLD1();
    fp(0xd8, 0xc0); expected.FADD(expected.top, expected.top);
    expected.getF64(expected.top); expected.getF64(expected.STV(1));
    fp(0xd9, 0xc9); expected.FXCH(expected.top, expected.STV(1));
}
void expectEnvironment(const FPU& expected) {
    auto& fpu = testContext().cpu->fpu;
    if (fpu.top != expected.top || fpu.cw != expected.cw || fpu.round != expected.round ||
        fpu.divExceptionsUnmasked != expected.divExceptionsUnmasked || fpu.isMMXInUse != expected.isMMXInUse ||
        (fpu.sw & ~0x3800u) != (expected.sw & ~0x3800u))
        testFail("cached environment control/status mismatch top=%u/%u cw=%x/%x sw=%x/%x",
            fpu.top, expected.top, fpu.cw, expected.cw, fpu.sw, expected.sw);
    for (U32 i = 0; i < 4; ++i)
        if (fpu.envData[i] != expected.envData[i]) testFail("cached environment field %u", i);
    for (U32 i = 0; i < 8; ++i) {
        if (fpu.tags[i] != expected.tags[i]) testFail("cached environment physical tag %u", i);
        expectSpecialSlot(i, expected, (expected.top + i) & 7);
    }
}
void writeEnvironment(U32 address, U32 newTop, bool big) {
    auto* memory = testContext().memory;
    const U32 words[] = {0xdead0f7a, 0xbeef0045u | (newTop << 11), 0xabcd2490,
        0x12345678, 0xabcdef09, 0x87654321, 0xcafed00d};
    for (U32 i = 0; i < 7; ++i) {
        if (big) memory->writed(TEST_HEAP_ADDRESS + address + i * 4, words[i]);
        else memory->writew(TEST_HEAP_ADDRESS + address + i * 2, (U16)words[i]);
    }
}
}

void testWasmJitFpuEnvironment() {
    // Changing TOP retains physical values, including dirty FXCH renames,
    // lazy raw values, and all three independent runtime rotation bits.
    for (bool big : {false, true}) {
        for (U32 top = 0; top < 8; ++top) {
            for (U32 newTop = 0; newTop < 8; ++newTop) {
                for (U32 address : {0x100u, K_PAGE_SIZE - 6u}) {
                    seedEnvironment(top);
                    auto& context = testContext(); context.cpu->big = big;
                    FPU expected = context.cpu->fpu;
                    dirtyEnvironment(expected);
                    writeEnvironment(address, newTop, big);
                    expected.FLDENV(context.cpu, TEST_HEAP_ADDRESS + address);
                    envMem(4, address, big);
                    expected.getF64(expected.top); expected.getF64(expected.STV(1));
                    fp(0xd9, 0xc9); expected.FXCH(expected.top, expected.STV(1));
                    fp(0xd9, 0xe0); expected.FCHS();
                    testRunCPU();
                    expectEnvironment(expected);
                    requireFpJit();
                }
            }
        }
    }

    // FNINIT discards cached doubles and resets metadata, including the
    // scratch slot's validity. A subsequent FLDENV can expose retained raw
    // register bits; stale pre-init locals must never take their place.
    for (U32 top = 0; top < 8; ++top) {
        seedEnvironment(top);
        auto& context = testContext(); auto& fpu = context.cpu->fpu;
        fpu.isRegCached[8] = true; fpu.SetCW(0x0f7a); fpu.sw = 0xffff;
        FPU expected = fpu;
        dirtyEnvironment(expected);
        fp(0xdb, 0xe3); expected.FINIT();
        writeEnvironment(0x100, top, true);
        envMem(4, 0x100, true); expected.FLDENV(context.cpu, TEST_HEAP_ADDRESS + 0x100);
        expected.getF64(expected.top); // cached unary lowering uses a double
        fp(0xd9, 0xe0); expected.FCHS();
        testRunCPU();
        expectEnvironment(expected);
        if (fpu.isRegCached[8]) testFail("FNINIT did not clear scratch validity");
        requireFpJit();
    }

    // Store exact tag classifications without converting raw values. Compare
    // the entire 14/28-byte image, including MMX's override of stored tags.
    for (bool big : {false, true}) {
        for (bool mmx : {false, true}) {
            for (U32 top = 0; top < 8; ++top) {
                for (U32 address : {0x100u, K_PAGE_SIZE - 6u}) {
                    seedEnvironment(top);
                    auto& context = testContext(); auto& fpu = context.cpu->fpu;
                    context.cpu->big = big; fpu.isMMXInUse = mmx;
                    fpu.LD80(0, 0, 0x8000);
                    fpu.LD80(1, 0xc000000000000123ull, 0xffff);
                    fpu.LD80(2, 0x8000000000000000ull, 0x7fff);
                    fpu.LD80(3, 1, 0);
                    fpu.regCache[4].l = 0x7ff0000000000000ull; fpu.isRegCached[4] = true;
                    fpu.regCache[5].l = 0x8000000000000000ull; fpu.isRegCached[5] = true;
                    fpu.tags[6] = TAG_Empty; fpu.tags[7] = TAG_Special;
                    FPU expected = fpu;
                    // Change TOP without altering any operand's representation.
                    fp(0xd9, 0xf6); expected.top = (expected.top - 1) & 7;
                    expected.FSTENV(context.cpu, TEST_HEAP_ADDRESS + 0x500);
                    envMem(6, address, big);
                    testRunCPU();
                    expectEnvironment(expected);
                    for (U32 i = 0; i < (big ? 28u : 14u); ++i)
                        if (context.memory->readb(TEST_HEAP_ADDRESS + address + i) != context.memory->readb(TEST_HEAP_ADDRESS + 0x500 + i))
                            testFail("FNSTENV byte %u, top %u, big %u, mmx %u", i, top, big, mmx);
                    requireFpJit();
                }
            }
        }
    }
}

void testWasmJitFpuEnvironmentLoops() {
    for (bool init : {false, true}) {
        for (U32 top = 0; top < 8; ++top) {
            for (U32 count : {1u, 2u, 35u}) {
                seedEnvironment(top);
                auto& context = testContext();
                FPU expected = context.cpu->fpu;
                writeEnvironment(0x100, (top + 3) & 7, true);
                // Keep all tags ordinary and rounding at nearest for arithmetic.
                context.memory->writed(TEST_HEAP_ADDRESS + 0x100, 0x37f);
                context.memory->writed(TEST_HEAP_ADDRESS + 0x108, 0);
                context.cpu->reg[1].u32 = count;
                U32 start = context.codeIp;
                if (init) fp(0xdb, 0xe3);
                envMem(4, 0x100, true);
                fp(0xd9, 0xc9); fp(0xd9, 0xe0);
                envMem(6, 0x200, true);
                fp(0xd9, 0xf6); // changing TOP after runtime rebasing
                testPushCode8(0x49);
                testPushCode8(0x75); testPushCode8((U8)(start - context.codeIp - 1));
                for (U32 i = 0; i < count; ++i) {
                    if (init) expected.FINIT();
                    expected.FLDENV(context.cpu, TEST_HEAP_ADDRESS + 0x100);
                    expected.getF64(expected.top); expected.getF64(expected.STV(1));
                    expected.FXCH(expected.top, expected.STV(1)); expected.FCHS();
                    expected.FSTENV(context.cpu, TEST_HEAP_ADDRESS + 0x500);
                    expected.top = (expected.top - 1) & 7;
                }
                testRunCPU();
                expectEnvironment(expected);
                if (context.cpu->reg[1].u32) testFail("environment loop counter");
                for (U32 i = 0; i < 28; ++i)
                    if (context.memory->readb(TEST_HEAP_ADDRESS + 0x200 + i) != context.memory->readb(TEST_HEAP_ADDRESS + 0x500 + i))
                        testFail("environment loop saved byte %u", i);
                requireFpJit();
            }
        }
    }
}

void testWasmJitFpuEnvironmentFaults() {
    // Cold memory exits preserve the interpreter's partial-transfer behavior.
    // A store may have written earlier fields; a load changes SW/envData before
    // installing CW/tags/TOP. Signal delivery must see that exact state.
    for (bool store : {false, true}) {
        for (bool big : {false, true}) {
            for (U32 fields : {0u, 2u, 5u}) {
                seedEnvironment(5);
                auto& context = testContext(); context.cpu->big = big;
                FPU expected = context.cpu->fpu;
                dirtyEnvironment(expected);
                U32 stride = big ? 4 : 2;
                U32 address = 0x2000 - fields * stride;
                writeEnvironment(address, 1, big);
                if (store) expected.FSTENV(context.cpu, TEST_HEAP_ADDRESS + 0x500);
                else {
                    if (fields >= 2) expected.sw = big ? context.memory->readd(TEST_HEAP_ADDRESS + address + stride) : context.memory->readw(TEST_HEAP_ADDRESS + address + stride);
                    for (U32 i = 3; i < fields; ++i)
                        expected.envData[i - 3] = big ? context.memory->readd(TEST_HEAP_ADDRESS + address + stride * i) : context.memory->readw(TEST_HEAP_ADDRESS + address + stride * i);
                }
                envMem(store ? 6 : 4, address, big);
                fp(0xdb, 0xe3); // must not execute
                auto oldAction = context.process->sigActions[K_SIGSEGV];
                auto& action = context.process->sigActions[K_SIGSEGV];
                action.reset(); action.handlerAndSigAction = context.codeIp;
                context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, 0);
                testRunCPU();
                context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
                if (action.sigInfo[0] != K_SIGSEGV) testFail("environment transfer did not fault");
                for (U32 i = 0; i < 8; ++i) expected.getReg(i); // signal frame's raw representation
                expectEnvironment(expected);
                if (store) {
                    for (U32 i = 0; i < fields * stride; ++i)
                        if (context.memory->readb(TEST_HEAP_ADDRESS + address + i) != context.memory->readb(TEST_HEAP_ADDRESS + 0x500 + i))
                            testFail("faulting FNSTENV partial byte %u", i);
                }
                context.process->sigActions[K_SIGSEGV] = oldAction;
            }
        }
    }
}

namespace {
void fullStateMem(bool store, U32 address, bool big) {
    fp(0xdd, ((store ? 6 : 4) << 3) | (big ? 5 : 6));
    if (big) testPushCode32(address);
    else testPushCode16((U16)address);
}
void writeFullState(U32 address, U32 top, bool big) {
    writeEnvironment(address, top, big);
    auto* memory = testContext().memory;
    U32 start = TEST_HEAP_ADDRESS + address + (big ? 28 : 14);
    const U64 low[] = {0x8123456789abcdefull, 0, 1, 0xffffffffffffffffull,
        0x8000000000000001ull, 0xc123456789abcdefull, 0x8000000000000000ull, 0x123456789abcdef0ull};
    const U16 high[] = {0x4000, 0x8000, 0, 0x7ffe, 0x7fff, 0xffff, 0x3fff, 0x1234};
    for (U32 i = 0; i < 8; ++i) {
        memory->writeq(start + i * 10, low[i]);
        memory->writew(start + i * 10 + 8, high[i]);
    }
}
void expectSavedImage(U32 actual, U32 expected, U32 size) {
    auto* memory = testContext().memory;
    for (U32 i = 0; i < size; ++i)
        if (memory->readb(TEST_HEAP_ADDRESS + actual + i) != memory->readb(TEST_HEAP_ADDRESS + expected + i))
            testFail("cached full FPU image byte %u at %x", i, actual);
}
}

void testWasmJitFpuSaveRestore() {
    for (bool big : {false, true}) {
        U32 size = big ? 108 : 94;
        for (U32 top = 0; top < 8; ++top) {
            for (U32 address : {0x100u, K_PAGE_SIZE - size, K_PAGE_SIZE - 32u, K_PAGE_SIZE - 6u}) {
                seedEnvironment(top);
                auto& context = testContext(); context.cpu->big = big;
                context.cpu->fpuDirtyFlags = 0x12345678;
                context.cpu->fpu.isMMXInUse = (top & 1) != 0;
                FPU expected = context.cpu->fpu;
                dirtyEnvironment(expected);
                context.memory->writeb(TEST_HEAP_ADDRESS + address - 1, 0xa5);
                context.memory->writeb(TEST_HEAP_ADDRESS + address + size, 0x5a);
                expected.FSAVE(context.cpu, TEST_HEAP_ADDRESS + 0x500);
                fullStateMem(true, address, big);
                testRunCPU();
                expectSavedImage(address, 0x500, size);
                expectEnvironment(expected);
                if (context.cpu->fpuDirtyFlags != 0x12345678 ||
                    context.memory->readb(TEST_HEAP_ADDRESS + address - 1) != 0xa5 ||
                    context.memory->readb(TEST_HEAP_ADDRESS + address + size) != 0x5a)
                    testFail("FNSAVE clobbered dirty flags or adjacent memory");
                requireFpJit();
            }
            for (U32 newTop = 0; newTop < 8; ++newTop) {
                for (U32 address : {0x100u, K_PAGE_SIZE - size, K_PAGE_SIZE - 32u}) {
                    seedEnvironment(top);
                    auto& context = testContext(); context.cpu->big = big;
                    context.cpu->fpuDirtyFlags = 0x12345678;
                    context.cpu->fpu.isMMXInUse = (top & 1) != 0;
                    FPU expected = context.cpu->fpu;
                    dirtyEnvironment(expected);
                    writeFullState(address, newTop, big);
                    expected.FRSTOR(context.cpu, TEST_HEAP_ADDRESS + address);
                    fullStateMem(false, address, big);
                    testRunCPU();
                    expectEnvironment(expected);
                    if (context.cpu->fpuDirtyFlags != 1) testFail("FRSTOR did not set full dirty-flags field");
                    requireFpJit();
                }
            }
        }
    }
}

void testWasmJitFpuSaveConversion() {
    auto oldFlags = softfloat_exceptionFlags, oldMode = softfloat_roundingMode;
    const U64 cases[] = {0, 1, 2, 0x0008000000000000ull, 0x000fffffffffffffull,
        0x0010000000000000ull, 0x0010000000000001ull, 0x3ff0000000000000ull,
        0x3ff0000000000001ull, 0x7fefffffffffffffull, 0x7ff0000000000000ull,
        0x7ff0000000000001ull, 0x7ff7ffffffffffffull, 0x7ff8000000000000ull,
        0x7ff8123456789abcull, 0x7fffffffffffffffull};
    U64 random = 0xe7037ed1a0b428dbull;
    // Every sign/class plus deterministic random payloads; all eight slots
    // are saved on each case. The last source is pushed from guest memory,
    // ensuring the save observes a dirty local that CPU state never received.
    for (U32 sequence = 0; sequence < 160; ++sequence) {
        beginFp(sequence & 7);
        auto& context = testContext(); auto& fpu = context.cpu->fpu;
        fpu.SetCW(0x037f | ((sequence & 3) << 10));
        for (U32 i = 0; i < 8; ++i) {
            random ^= random << 13; random ^= random >> 7; random ^= random << 17;
            U64 bits = sequence < 32 ? cases[(sequence + i) & 15] | (sequence & 16 ? 1ull << 63 : 0) : random;
            fpu.regCache[i].l = bits; fpu.isRegCached[i] = true;
            fpu.tags[i] = (sequence + i) & 3;
        }
        FPU expected = fpu;
        U64 pushed = fpu.regCache[expected.STV(7)].l;
        context.memory->writeq(TEST_HEAP_ADDRESS + 0x900, pushed);
        fpMem(0xdd, 0, 0x900); expected.PREP_PUSH(); expected.FLD_F64(pushed, expected.top);
        context.cpu->xmm[0].pi.u32[0] = 0x3f800000;
        context.cpu->xmm[0].pi.u32[1] = 0x12345678;
        testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58, 0xc0);
        testPushCode8(0xb8); testPushCode32(0xffffffff);
        fp(0x83, 0xc0); testPushCode8(1);
        softfloat_exceptionFlags = softfloat_flag_infinite;
        expected.FSAVE(context.cpu, TEST_HEAP_ADDRESS + 0x500);
        U8 flags = softfloat_exceptionFlags;
        fullStateMem(true, 0x100, true);
        fp(0x83, 0xd0); testPushCode8(0); // carry crosses full save/reset
        testPushCode8(0xf3); testPushCode8(0x0f); fp(0x58, 0xc0);
        softfloat_exceptionFlags = softfloat_flag_infinite;
        softfloat_roundingMode = softfloat_round_max;
        testRunCPU();
        expectSavedImage(0x100, 0x500, 108);
        expectEnvironment(expected);
        if (softfloat_exceptionFlags != flags || softfloat_roundingMode != softfloat_round_max)
            testFail("FNSAVE conversion flags/mode sequence=%u flags=%u expected=%u", sequence, softfloat_exceptionFlags, flags);
        if (context.cpu->reg[0].u32 != 1 || context.cpu->xmm[0].pi.u32[0] != 0x40800000 ||
            context.cpu->xmm[0].pi.u32[1] != 0x12345678)
            testFail("FNSAVE conversion lost GP/XMM state");
        requireFpJit();
    }
    softfloat_exceptionFlags = oldFlags; softfloat_roundingMode = oldMode;
}

void testWasmJitFpuSaveRestoreLoops() {
    for (U32 mode : {0u, 1u, 2u}) {
        for (U32 top = 0; top < 8; ++top) {
            for (U32 count : {1u, 2u, 35u}) {
                seedEnvironment(top);
                auto& context = testContext();
                FPU image = context.cpu->fpu;
                image.FSAVE(context.cpu, TEST_HEAP_ADDRESS + 0x100);
                FPU expected = context.cpu->fpu;
                context.cpu->reg[1].u32 = count;
                U32 start = context.codeIp;
                if (mode == 1) fullStateMem(false, 0x100, true);
                fp(0xd9, 0xe8); fp(0xde, 0xc1); // add 1 to ST0
                fp(0xd9, 0xc9); // rename the two live values
                if (mode != 1) fullStateMem(true, 0x100, true);
                if (mode == 0) fullStateMem(false, 0x100, true);
                fp(0xd9, 0xf7); // rotating TOP after any save/restore
                testPushCode8(0x49);
                testPushCode8(0x75); testPushCode8((U8)(start - context.codeIp - 1));
                // Reference image must not overwrite the JIT's initial input.
                for (U32 i = 0; i < 108; ++i)
                    context.memory->writeb(TEST_HEAP_ADDRESS + 0x500 + i, context.memory->readb(TEST_HEAP_ADDRESS + 0x100 + i));
                for (U32 i = 0; i < count; ++i) {
                    if (mode == 1) expected.FRSTOR(context.cpu, TEST_HEAP_ADDRESS + 0x500);
                    expected.FLD1(); expected.FADD(expected.STV(1), expected.top); expected.FPOP();
                    expected.getF64(expected.top); expected.getF64(expected.STV(1));
                    expected.FXCH(expected.top, expected.STV(1));
                    if (mode != 1) expected.FSAVE(context.cpu, TEST_HEAP_ADDRESS + 0x500);
                    if (mode == 0) expected.FRSTOR(context.cpu, TEST_HEAP_ADDRESS + 0x500);
                    expected.top = (expected.top + 1) & 7;
                }
                testRunCPU();
                expectEnvironment(expected);
                expectSavedImage(0x100, 0x500, 108);
                if (context.cpu->reg[1].u32) testFail("full FPU state loop counter");
                requireFpJit();
            }
        }
    }
}

void testWasmJitFpuSaveRestoreFaults() {
    for (bool store : {false, true}) {
        for (bool big : {false, true}) {
            U32 header = big ? 28 : 14, stride = big ? 4 : 2;
            for (U32 bytes : {0u, 2 * stride, header, header + 8, header + 10, header + 38, header + 79}) {
                seedEnvironment(5);
                auto& context = testContext(); context.cpu->big = big;
                FPU expected = context.cpu->fpu;
                dirtyEnvironment(expected);
                U32 address = 0x2000 - bytes;
                U32 size = header + 80;
                for (U32 i = 0; i < size; ++i) {
                    context.memory->writeb(TEST_HEAP_ADDRESS + address + i, 0xa5);
                    context.memory->writeb(TEST_HEAP_ADDRESS + 0x500 + i, 0xa5);
                }
                if (store) {
                    expected.FSTENV(context.cpu, TEST_HEAP_ADDRESS + 0x500);
                    for (U32 i = 0; header + (i + 1) * 10 <= bytes; ++i)
                        expected.ST80(context.cpu, TEST_HEAP_ADDRESS + 0x500 + header + i * 10, expected.STV(i));
                } else {
                    writeFullState(address, 2, big);
                    if (bytes >= header) {
                        expected.FLDENV(context.cpu, TEST_HEAP_ADDRESS + address);
                        for (U32 i = 0; i < 8; ++i) {
                            U32 offset = header + i * 10, index = expected.STV(i);
                            if (bytes < offset + 8) break;
                            expected.regs[index].signif = context.memory->readq(TEST_HEAP_ADDRESS + address + offset);
                            if (bytes < offset + 10) break;
                            expected.regs[index].signExp = context.memory->readw(TEST_HEAP_ADDRESS + address + offset + 8);
                            expected.isRegCached[index] = false;
                        }
                    } else {
                        if (bytes >= 2 * stride)
                            expected.sw = big ? context.memory->readd(TEST_HEAP_ADDRESS + address + stride) : context.memory->readw(TEST_HEAP_ADDRESS + address + stride);
                    }
                }
                fullStateMem(store, address, big);
                fp(0xdb, 0xe3); // must not execute
                auto oldAction = context.process->sigActions[K_SIGSEGV];
                auto& action = context.process->sigActions[K_SIGSEGV];
                action.reset(); action.handlerAndSigAction = context.codeIp;
                context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, 0);
                testRunCPU();
                context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
                if (action.sigInfo[0] != K_SIGSEGV) testFail("full FPU transfer did not fault");
                for (U32 i = 0; i < 8; ++i) expected.getReg(i); // signal-frame serialization
                expectEnvironment(expected);
                if (store) {
                    expectSavedImage(address, 0x500, bytes);
                    for (U32 i = bytes; i < size; ++i)
                        if (context.memory->readb(TEST_HEAP_ADDRESS + address + i) != 0xa5)
                            testFail("FNSAVE wrote protected byte %u", i);
                }
                context.process->sigActions[K_SIGSEGV] = oldAction;
            }
        }
    }
}

void testWasmJitFpuBoundaries() {
    // Mixed integer/RMW/SSE paths, including page-crossing whole-instruction
    // fallback, must preserve dirty and renamed x87 registers.
    for (U32 top = 0; top < 8; ++top) {
        for (U32 address : {0x100u, K_PAGE_SIZE - 2u}) {
            beginFp(top);
            auto& context = testContext();
            context.cpu->xmm[0].pi.u32[0] = 0x3f800000; // 1.0f
            context.cpu->xmm[0].pi.u32[1] = 0x12345678;
            context.memory->writed(TEST_HEAP_ADDRESS + address, 0x40000000); // 2.0f
            fp(0xd9, 0xe8); fp(0xd8, 0xc0); // 2
            fp(0xd9, 0xe8); fp(0xd9, 0xc9); // renamed 2,1
            testPushCode8(0xb8); testPushCode32(3);
            testPushCode8(0xc1); testPushCode8(0xe0); testPushCode8(2); // shl eax,2
            testPushCode8(0x6b); testPushCode8(0xc0); testPushCode8(5); // imul eax,5:60
            testPushCode8(0x50); testPushCode8(0x5b); // push eax; pop ebx
            testPushCode8(0x83); testPushCode8(0x05); testPushCode32(0x300); testPushCode8(1); // add [mem],1
            testPushCode8(0xf3); testPushCode8(0x0f); fpMem(0x58, 0, address); // addss xmm0,[mem]:3
            testPushCode8(0x9b); // fwait: cold helper must not discard hot cache metadata
            fp(0xd8, 0xc1); // 3,1
            testPushCode8(0xba); testPushCode32(0); // edx=0
            testPushCode8(0xb9); testPushCode32(7); // ecx=7
            fp(0xf7, 0xf1); // div ecx:8 remainder4; conditional fallback not taken
            fp(0xde, 0xc1); // 4
            testRunCPU();
            expectFp(0, 4);
            if (context.cpu->reg[0].u32 != 8 || context.cpu->reg[2].u32 != 4 || context.cpu->reg[3].u32 != 60)
                testFail("integer arithmetic lost state with active x87 cache");
            if (context.cpu->xmm[0].pi.u32[0] != 0x40400000 || context.cpu->xmm[0].pi.u32[1] != 0x12345678)
                testFail("SSE memory path clobbered x87/XMM state");
            if (context.cpu->fpu.top != ((top + 7) & 7)) testFail("mixed cache boundary TOP");
            requireFpJit();
        }
    }

    // Carry cached values across a bounded integer-only loop, including early
    // taken exits and an untaken forward branch. The target labels themselves
    // are not entry points into this Wasm activation.
    for (U32 early : {0u, 1u}) {
        beginFp(4);
        auto& context = testContext();
        context.cpu->reg[0].u32 = early;
        fp(0xd9, 0xe8); fp(0xd8, 0xc0); // 2
        testPushCode8(0xb9); testPushCode32(1000);
        testPushCode8(0x83); testPushCode8(0xf8); testPushCode8(1); // loop: cmp eax,1
        testPushCode8(0x74); testPushCode8(3); // je after loop
        testPushCode8(0x49); // dec ecx
        testPushCode8(0x75); testPushCode8(0xf8); // jnz loop
        fp(0xd8, 0xc0); // 4
        testRunCPU();
        expectFp(0, 4);
        if (context.cpu->reg[1].u32 != (early ? 1000 : 0)) testFail("x87 cache integer loop control");
        requireFpJit();
    }

    // Balanced loops carry a three-register FXCH permutation. The backedge
    // must restore the header's local mapping without converting untouched
    // extended values or losing results from earlier iterations.
    for (U32 top = 0; top < 8; ++top) {
        for (U32 count : {1u, 2u, 3u, 35u}) {
            beginFp(top);
            auto& fpu = testContext().cpu->fpu;
            for (U32 i = 0; i < 4; ++i) {
                fpu.FLD_I64(i == 3 ? 0x20000000000001ll : (i == 0 ? 1 : i == 1 ? 3 : 7), fpu.STV(i));
                fpu.tags[fpu.STV(i)] = TAG_Valid;
            }
            auto untouched = fpu.regs[fpu.STV(3)];
            fp(0xd8, 0xc0); // 2,3,7
            fp(0xd9, 0xc9); // 3,2,7: noncanonical mapping before loop entry
            testPushCode8(0xb9); testPushCode32(count);
            fp(0xd9, 0xc9); // loop: b,a,c
            fp(0xd9, 0xca); // c,a,b
            fp(0xd9, 0xe8); fp(0xde, 0xc1); // c+1,a,b
            testPushCode8(0x49);
            testPushCode8(0x75); testPushCode8(0xf5); // jnz loop (-11)
            testRunCPU();
            double a=3, c=7, d=2;
            for (U32 i = 0; i < count; ++i) { double next=c+1; c=d; d=a; a=next; }
            expectFp(0,a); expectFp(1,d); expectFp(2,c);
            if (fpu.top != top || fpu.isRegCached[fpu.STV(3)] ||
                fpu.regs[fpu.STV(3)].signif != untouched.signif || fpu.regs[fpu.STV(3)].signExp != untouched.signExp)
                testFail("cached loop changed unused extended register or TOP");
            requireFpJit();
        }
    }

    // MDK builds a reciprocal table in a balanced x87 loop. Conditional
    // writeback in a cold memory arm must not make the fast arm reload stale
    // integer registers. Use a separate bounded counter so the regression
    // fails an assertion instead of repeating the stale table index forever.
    for (U32 top = 0; top < 8; ++top) {
        beginFp(top);
        auto& context = testContext();
        context.memory->writed(TEST_HEAP_ADDRESS + 0x100, 1);
        context.cpu->reg[2].u32 = 8;
        fpMem(0xdb, 0, 0x100); // loop: fild dword
        fp(0xd9, 0xe8); fp(0xde, 0xf1); // 1 / counter
        fpMem(0x8b, 1, 0x100); // mov ecx,[counter]
        testPushCode8(0x41); // inc ecx
        fpMem(0x89, 1, 0x100); // mov [counter],ecx
        fpMem(0xd9, 3, 0x200); // fstp float
        testPushCode8(0x4a); // dec edx
        testPushCode8(0x75); testPushCode8(0xe0); // jnz loop (-32)
        testRunCPU();
        if (context.memory->readd(TEST_HEAP_ADDRESS + 0x100) != 9 ||
            context.cpu->reg[1].u32 != 9 || context.cpu->reg[2].u32 != 0)
            testFail("cached x87 loop stored stale integer counter");
        if (context.memory->readd(TEST_HEAP_ADDRESS + 0x200) != 0x3e000000 || context.cpu->fpu.top != top)
            testFail("cached x87 reciprocal loop result");
        requireFpJit();
    }

    // Unbalanced loops rotate TOP between iterations. An initially cached
    // slot can become an uncached extended slot at the same header position.
    for (U32 top = 0; top < 8; ++top) {
        for (bool pop : {false, true}) {
            beginFp(top);
            auto& context = testContext();
            auto& fpu = context.cpu->fpu;
            for (U32 i = 0; i < 4; ++i) {
                fpu.FLD_I64(i + 1, fpu.STV(i));
                fpu.tags[fpu.STV(i)] = TAG_Valid;
            }
            fp(0xd8, 0xc0); // dirty initial ST(0)=2
            testPushCode8(0xb9); testPushCode32(3);
            if (pop) fpMem(0xd9, 3, 0x300); // loop: fstp float
            else fp(0xd9, 0xe8); // loop: fld1
            testPushCode8(0x49);
            testPushCode8(0x75); testPushCode8(pop ? 0xf7 : 0xfb);
            if (pop) fp(0xd8, 0xc0); // initial ST(3)=4 ->8
            else { fp(0xde, 0xc1); fp(0xde, 0xc1); fp(0xde, 0xc1); } // 2+1+1+1=5
            testRunCPU();
            expectFp(0, pop ? 8 : 5);
            if (fpu.top != ((top + (pop ? 3 : 0)) & 7)) testFail("rotating x87 loop TOP");
            if (pop && context.memory->readd(TEST_HEAP_ADDRESS + 0x300) != 0x40400000)
                testFail("rotating x87 loop stored stale value");
            requireFpJit();
        }
    }

    // On iteration two, fault before this iteration's first FPU operation.
    // The exception must see the dirty result carried from iteration one.
    beginFp(3);
    {
        auto& context = testContext();
        context.cpu->reg[6].u32 = 0x1000;
        fp(0xd9, 0xe8); fp(0xd8, 0xc0); // 2
        testPushCode8(0xb9); testPushCode32(2);
        fp(0x8b, 0x06); // loop: mov eax,[esi]
        fp(0xd8, 0xc0); // 4 on first iteration
        testPushCode8(0x81); testPushCode8(0xc6); testPushCode32(0x1000);
        testPushCode8(0x49);
        testPushCode8(0x75); testPushCode8(0xf3); // -13
        auto oldAction = context.process->sigActions[K_SIGSEGV];
        auto& action = context.process->sigActions[K_SIGSEGV];
        action.reset(); action.handlerAndSigAction = context.codeIp;
        context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, 0);
        testRunCPU();
        context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
        if (action.sigInfo[0] != K_SIGSEGV) testFail("carried x87 loop did not fault");
        expectFp(0,4);
        context.process->sigActions[K_SIGSEGV] = oldAction;
    }

    // An unconditional interpreter callout may continue through dispatch, but
    // must not overwrite the helper's CPU state or lose preceding dirty values.
    for (bool carry : {false, true}) {
        beginFp(2);
        fp(0xd9, 0xe8); fp(0xd8, 0xc0); // 2
        testPushCode8(0xb8); testPushCode32(0x80000000);
        testPushCode8(carry ? 0xf9 : 0xf8); // stc/clc
        testPushCode8(0xb1); testPushCode8(1); // cl=1
        fp(0xd3, 0xd0); // rcl eax,cl, interpreter-backed
        fp(0xd8, 0xc0); // 4
        testRunCPU();
        expectFp(0, 4);
        if (testContext().cpu->reg[0].u32 != (carry ? 1u : 0u)) testFail("callout lost rotate result");
    }

    // Lookahead starts with no x87 cache; a skipped FLD activates it before
    // CPUID exits through the interpreter. The following branch still needs
    // the architectural comparison flags from before FLD.
    for (U32 taken : {0u, 1u}) {
        beginFp(3);
        testContext().cpu->reg[6].u32 = taken;
        testPushCode8(0x83); testPushCode8(0xfe); testPushCode8(1); // cmp esi,1
        fp(0xd9, 0xe8);
        fp(0x0f, 0xa2); // cpuid
        testPushCode8(0x74); testPushCode8(2);
        fp(0xd8, 0xc0);
        testRunCPU();
        expectFp(0, taken ? 1 : 2);
    }

    // Save/restore and MMX aliasing are explicit architectural-state consumers.
    beginFp(3);
    fp(0xd9, 0xe8); fp(0xd8, 0xc0); // 2
    testPushCode8(0x0f); fpMem(0xae, 0, 0x400); // fxsave
    fp(0xd8, 0xc0); // 4
    testPushCode8(0x0f); fpMem(0xae, 1, 0x400); // fxrstor:2
    fp(0xd8, 0xc0); // 4
    testRunCPU();
    expectFp(0, 4);
    beginFp(3);
    fp(0xd9, 0xe8); fp(0xd8, 0xc0);
    fp(0x0f, 0x77); // emms
    fp(0xd9, 0xe8);
    testRunCPU();
    expectFp(0, 1);
    for (U32 i = 1; i < 8; ++i) {
        if (testContext().cpu->fpu.tags[testContext().cpu->fpu.STV(i)] != TAG_Empty)
            testFail("EMMS resurrected old cached tags");
    }

    // A taken integer divide exception must expose the pre-fault x87 state.
    beginFp(3);
    auto& context = testContext();
    fp(0xd9, 0xe8); fp(0xd8, 0xc0); // 2
    testPushCode8(0xb9); testPushCode32(0);
    fp(0xf7, 0xf1); // divide by zero
    fp(0xd8, 0xc0); // must not execute
    auto oldAction = context.process->sigActions[K_SIGFPE];
    auto& action = context.process->sigActions[K_SIGFPE];
    action.reset();
    action.handlerAndSigAction = context.codeIp;
    testRunCPU();
    if (action.sigInfo[0] != K_SIGFPE) testFail("cached x87 integer divide did not fault");
    expectFp(0, 2);
    context.process->sigActions[K_SIGFPE] = oldAction;
}

void testWasmJitFpuCompare() {
    // Repeated comparisons must preserve the cached tags and values after
    // renaming. Cover all TOPs, ordered results, NaN, and a freed operand.
    for (U32 top = 0; top < 8; ++top) {
        for (U32 kind = 0; kind < 5; ++kind) {
            beginFp(top);
            auto& context = testContext();
            const U64 bits[] = {0x3ff0000000000000ull, 0x4008000000000000ull,
                0x4000000000000000ull, 0x7ff8000000000001ull, 0x4000000000000000ull};
            const U32 condition[] = {0, 0x100, 0x4000, 0x4500, 0x4500};
            context.memory->writeq(TEST_HEAP_ADDRESS + 0x100, bits[kind]);
            fpMem(0xdd, 0, 0x100); // other
            fp(0xd9, 0xe8); fp(0xd8, 0xc0); // 2,other
            fp(0xd9, 0xc9); fp(0xd9, 0xc9); // rename twice
            if (kind == 4) fp(0xdd, 0xc1); // ffree st1
            fp(0xd8, 0xd1); // fcom st1
            fp(0xdd, 0xe1); // fucom st1, tags must survive previous comparison
            testPushCode8(0xb8); testPushCode32(0x12345678);
            fp(0xdf, 0xe0); // fnstsw ax, preserving upper EAX
            fpMem(0xdd, 7, 0x200); // fnstsw word
            fp(0xd8, 0xc0); // keep using the cached 2
            testRunCPU();
            U32 sw = (((top + 6) & 7) << 11) | condition[kind];
            if (context.cpu->reg[0].u32 != (0x12340000 | sw) ||
                context.memory->readw(TEST_HEAP_ADDRESS + 0x200) != sw)
                testFail("cached comparison/status mismatch top=%u kind=%u", top, kind);
            expectFp(0, 4);
            if (context.cpu->fpu.tags[context.cpu->fpu.STV(0)] != TAG_Valid ||
                context.cpu->fpu.tags[context.cpu->fpu.STV(1)] != (kind == 4 ? TAG_Empty : TAG_Valid))
                testFail("comparison corrupted persistent tags");
            requireFpJit();
        }

        // Memory comparisons read the dirty ST0 and apply their pop only after
        // the access. Signed integers and real operands share the same result.
        for (U32 type = 0; type < 4; ++type) {
            for (bool pop : {false, true}) {
                beginFp(top);
                auto& context = testContext();
                const U8 opcode[] = {0xd8, 0xdc, 0xda, 0xde};
                const U64 bits[] = {0xbf800000ull, 0xbff0000000000000ull, 0xffffffffull, 0xffffull};
                context.memory->writeq(TEST_HEAP_ADDRESS + 0x100, bits[type]);
                fp(0xd9, 0xe8); // sentinel 1
                fp(0xd9, 0xe8); fp(0xd8, 0xc0); // dirty 2,1
                fpMem(opcode[type], pop ? 3 : 2, 0x100); // 2 > -1
                fp(0xdf, 0xe0);
                fp(0xd8, 0xc0);
                testRunCPU();
                U32 endTop = (top + (pop ? 7 : 6)) & 7;
                if ((context.cpu->reg[0].u32 & 0xffff) != (endTop << 11) || context.cpu->fpu.top != endTop)
                    testFail("cached memory comparison TOP/status");
                expectFp(0, pop ? 2 : 4);
                if (pop && context.cpu->fpu.tags[(endTop + 7) & 7] != TAG_Empty)
                    testFail("cached memory comparison did not empty popped tag");
            }
        }

        for (U32 variant = 0; variant < 4; ++variant) {
            beginFp(top);
            auto& context = testContext();
            fp(0xd9, 0xe8); // sentinel
            fp(0xd9, 0xe8); fp(0xd8, 0xc0); // 2,1
            fp(0xd9, 0xe8); // 1,2,1
            if (variant == 0) fp(0xd8, 0xd9); // fcomp st1
            if (variant == 1) fp(0xdd, 0xe9); // fucomp st1
            if (variant == 2) fp(0xde, 0xd9); // fcompp
            if (variant == 3) fp(0xda, 0xe9); // fucompp
            fp(0xdf, 0xe0);
            testRunCPU();
            U32 pops = variant < 2 ? 1 : 2;
            U32 endTop = (top + 5 + pops) & 7;
            if ((context.cpu->reg[0].u32 & 0xffff) != ((endTop << 11) | 0x100))
                testFail("cached register comparison pop/status");
            expectFp(0, pops == 1 ? 2 : 1);
            for (U32 i = 1; i <= pops; ++i) {
                if (context.cpu->fpu.tags[(endTop + 8 - i) & 7] != TAG_Empty)
                    testFail("cached double comparison did not empty both tags");
            }
        }

        // FFREE only changes a tag, including for untouched extended values.
        for (bool pop : {false, true}) {
            beginFp(top);
            auto& fpu = testContext().cpu->fpu;
            fpu.FLD_I64(0x123456789abcdefll, top); fpu.tags[top] = TAG_Valid;
            fp(0xd9, 0xe8); fp(0xd8, 0xc0);
            fp(pop ? 0xdf : 0xdd, 0xc1); // free old extended ST1; optionally pop dirty ST0
            testRunCPU();
            if (fpu.isRegCached[top] || fpu.tags[top] != TAG_Empty || fpu.top != ((top + (pop ? 0 : 7)) & 7))
                testFail("cached FFREE converted unused extended value or lost tag/TOP");
            if (pop && fpu.tags[(top + 7) & 7] != TAG_Empty) testFail("FFREEP did not pop ST0");
        }

        // The comparison/status/free sequence must survive a cached backedge.
        beginFp(top);
        fp(0xd9, 0xe8);
        testPushCode8(0xb9); testPushCode32(100);
        U32 loop = testContext().codeIp;
        fp(0xd9, 0xe8); fp(0xde, 0xc1); // accumulator += 1
        fp(0xd9, 0xc0); fp(0xd8, 0xd9); // compare duplicate and pop: equal
        fp(0xdf, 0xe0);
        fp(0xdd, 0xc7); // free unused slot
        testPushCode8(0x49);
        testPushCode8(0x75); testPushCode8((U8)(loop - (testContext().codeIp + 1)));
        testRunCPU();
        expectFp(0, 101);
        if ((testContext().cpu->reg[0].u32 & 0xffff) != ((((top + 7) & 7) << 11) | 0x4000))
            testFail("cached comparison loop status");
        requireFpJit();
    }

    // FNSTSW may be skipped by integer flag fusion. It overwrites AX, while
    // the branch must use the comparison result captured before that write.
    for (U32 taken : {0u, 1u}) {
        beginFp(3);
        fp(0xd9, 0xe8);
        testPushCode8(0xb8); testPushCode32(taken);
        fp(0x83, 0xf8); testPushCode8(1);
        fp(0xdf, 0xe0);
        testPushCode8(0x74); testPushCode8(2);
        fp(0xd8, 0xc0);
        fp(0x31, 0xdb); // integer flags dead after branch
        testRunCPU();
        expectFp(0, taken ? 1 : 2);
    }

    beginFp(3);
    testContext().cpu->fpu.sw = 0xffff;
    fp(0xd9, 0xe8); fp(0xdb, 0xe2); fp(0xd9, 0xd0); // fld1,fnclex,fnop
    fp(0xdf, 0xe0); fp(0xd8, 0xc0);
    testRunCPU();
    expectFp(0, 2);
    if ((testContext().cpu->reg[0].u32 & 0xffff) != 0x5700) testFail("cached FNCLEX/status");
}

void testWasmJitFpuRegisters() {
    for (U32 top = 0; top < 8; ++top) {
        // Register stores copy rather than alias locals, including after FXCH.
        for (bool pop : {false, true}) {
            for (U8 dst = 0; dst < 8; ++dst) {
                beginFp(top);
                fp(0xd9, 0xe8); fp(0xd8, 0xc0); // 2
                fp(0xd9, 0xe8); // 1,2
                fp(0xd9, 0xc9); // 2,1 (renamed locals)
                fp(0xdd, (pop ? 0xd8 : 0xd0) + dst);
                if (!pop) fp(0xd8, 0xc0); // copied destination stays 2
                testRunCPU();
                auto& fpu = testContext().cpu->fpu;
                if (pop) {
                    if (dst) expectFp(dst - 1, 2);
                    if (fpu.tags[(top + 6) & 7] != TAG_Empty) testFail("FSTP did not empty old TOP");
                } else {
                    expectFp(0, 4);
                    if (dst) expectFp(dst, 2);
                }
                if (fpu.top != ((top + (pop ? 7 : 6)) & 7)) testFail("cached FST TOP");
                requireFpJit();
            }
        }

        beginFp(top);
        fp(0xd9, 0xe8); fp(0xd8, 0xc0); fp(0xd8, 0xc0); // 4
        fp(0xd9, 0xe0); fp(0xd9, 0xe1); fp(0xd9, 0xfa); // -4, abs, sqrt
        fp(0xd9, 0xe4); // FTST: positive
        fp(0xdf, 0xe0);
        fp(0xd9, 0xe0); fp(0xd9, 0xe4); // -2, FTST: negative
        fpMem(0xdd, 7, 0x100);
        testRunCPU();
        expectFp(0, -2);
        if ((testContext().cpu->reg[0].u32 & 0x4500) != 0 ||
            (testContext().memory->readw(TEST_HEAP_ADDRESS + 0x100) & 0x4500) != 0x100)
            testFail("cached unary comparison clobbered ST0 or tag");

        // Rotate TOP through unused extended values without converting them.
        for (U8 rotate : {0xf6, 0xf7}) {
            beginFp(top);
            auto& context = testContext();
            for (U32 i = 0; i < 8; ++i) {
                context.cpu->fpu.FLD_I64(0x123456789abcdefll + i, i);
                context.cpu->fpu.tags[i] = TAG_Valid;
            }
            auto original = context.cpu->fpu;
            testPushCode8(0xb9); testPushCode32(19);
            U32 loop = context.codeIp;
            fp(0xd9, rotate);
            testPushCode8(0x49); testPushCode8(0x75);
            testPushCode8((U8)(loop - (context.codeIp + 1)));
            testRunCPU();
            if (context.cpu->fpu.top != ((top + (rotate == 0xf6 ? 5 : 3)) & 7)) testFail("cached TOP rotation loop");
            for (U32 i = 0; i < 8; ++i) {
                if (context.cpu->fpu.isRegCached[i] || context.cpu->fpu.tags[i] != TAG_Valid ||
                    context.cpu->fpu.regs[i].signif != original.regs[i].signif ||
                    context.cpu->fpu.regs[i].signExp != original.regs[i].signExp)
                    testFail("TOP rotation converted or corrupted an untouched extended value");
            }
            requireFpJit();
        }

        beginFp(top);
        auto& context = testContext();
        const U64 expected[] = {context.cpu->fL2T, context.cpu->fL2E, context.cpu->fPi, context.cpu->fLG2, context.cpu->fLN2};
        for (U8 i = 0; i < 5; ++i) fp(0xd9, 0xe9 + i);
        for (U32 i = 0; i < 5; ++i) fpMem(0xdd, 3, 0x100 + 8 * i);
        testRunCPU();
        for (U32 i = 0; i < 5; ++i) {
            if (context.memory->readq(TEST_HEAP_ADDRESS + 0x100 + 8 * i) != expected[4 - i])
                testFail("cached constant load");
        }
        if (context.cpu->fpu.top != top) testFail("cached constant TOP");
    }

    // An empty FSTP must execute the interpreter's exception and pop exactly
    // once; the JIT must not subsequently overwrite its result with old locals.
    beginFp(3);
    fp(0xd9, 0xe8); fp(0xd8, 0xc0); // sentinel 2
    fp(0xd9, 0xe8); fp(0xdd, 0xc0); // empty ST0
    fp(0xdd, 0xda); // fstp st2
    testRunCPU();
    expectFp(0, 2);
    auto& fpu = testContext().cpu->fpu;
    if (fpu.top != 2 || (fpu.sw & (FPU_SW_IE | FPU_SW_SF)) != (FPU_SW_IE | FPU_SW_SF))
        testFail("cached empty FSTP lost stack fault or popped twice");
}

void testWasmJitFpuConditional() {
    const U32 masks[] = {CF, ZF, CF | ZF, PF};
    for (U32 top = 0; top < 8; ++top) {
        for (U32 variant = 0; variant < 8; ++variant) {
            for (U32 flags : {0u, (U32)CF, (U32)ZF, (U32)PF, (U32)(CF | ZF | PF)}) {
                for (bool consume : {false, true}) {
                    beginFp(top);
                    auto& context = testContext();
                    auto& fpu = context.cpu->fpu;
                    context.cpu->flags = flags;
                    fpu.FLD_I64(0x123456789abcdefll, top); fpu.tags[top] = TAG_Valid;
                    U32 other = (top + 1) & 7;
                    fpu.FLD_I64(7, other); fpu.tags[other] = TAG_Valid;
                    auto exact = fpu.regs[top];
                    fp(variant < 4 ? 0xda : 0xdb, 0xc1 + (variant % 4) * 8);
                    if (consume) {
                        // Source and destination must load correctly after the join.
                        fp(0xd8, 0xc1);
                        fp(0xd9, 0xc9); fp(0xd9, 0xc9);
                    }
                    testRunCPU();
                    bool taken = !!(flags & masks[variant % 4]) != (variant >= 4);
                    if (consume) expectFp(0, (taken ? 7.0 : (double)0x123456789abcdefll) + 7);
                    else if (taken) expectFp(0, 7);
                    else if (fpu.isRegCached[top] || fpu.isRegCached[other] ||
                        fpu.regs[top].signif != exact.signif || fpu.regs[top].signExp != exact.signExp)
                        testFail("untaken FCMOV converted extended source/destination");
                    if (context.cpu->flags != flags) testFail("cached FCMOV changed flags");
                    requireFpJit();
                }
            }
        }

        // A conditionally imported destination precedes a loop header. The
        // header must retain its runtime validity instead of assuming double.
        for (bool taken : {false, true}) {
            beginFp(top);
            auto& context = testContext();
            context.cpu->flags = taken ? CF : 0;
            context.cpu->fpu.FLD_I64(0x123456789abcdefll, top); context.cpu->fpu.tags[top] = TAG_Valid;
            context.cpu->fpu.FLD_I64(7, (top + 1) & 7); context.cpu->fpu.tags[(top + 1) & 7] = TAG_Valid;
            auto exact = context.cpu->fpu.regs[top];
            fp(0xda, 0xc1);
            testPushCode8(0xb9); testPushCode32(31);
            U32 loop = context.codeIp;
            fp(0xd9, 0xe8); fp(0xdd, 0xd8); // push/pop only
            testPushCode8(0x49); testPushCode8(0x75);
            testPushCode8((U8)(loop - (context.codeIp + 1)));
            testRunCPU();
            if (taken) expectFp(0, 7);
            else if (context.cpu->fpu.isRegCached[top] || context.cpu->fpu.regs[top].signif != exact.signif ||
                context.cpu->fpu.regs[top].signExp != exact.signExp) testFail("FCMOV validity lost at loop entry");
            requireFpJit();
        }
    }

    // Integer compare/branch fusion must not hide the flags that FCMOV reads.
    for (U32 value : {0u, 1u, 2u}) {
        beginFp(3);
        fp(0xd9, 0xe8); fp(0xd8, 0xc0); fp(0xd9, 0xe8); // 1,2
        testPushCode8(0xb8); testPushCode32(value);
        fp(0x83, 0xf8); testPushCode8(1); // cmp eax,1
        fp(0xda, 0xc1); // fcmovb: 2 if eax < 1
        testPushCode8(0x72); testPushCode8(2); // jb over add
        fp(0xd8, 0xc0);
        fp(0x31, 0xdb); // old integer flags dead
        testRunCPU();
        expectFp(0, 2);
    }

    // All integer-flag comparisons, both pop forms, repeated after a rename.
    for (U32 variant = 0; variant < 4; ++variant) {
        for (U32 kind = 0; kind < 5; ++kind) {
            beginFp(6);
            auto& context = testContext();
            const U64 bits[] = {0x3ff0000000000000ull, 0x4008000000000000ull,
                0x4000000000000000ull, 0x7ff8000000000001ull, 0x4000000000000000ull};
            const U32 flags[] = {0, CF, ZF, CF | PF | ZF, CF | PF | ZF};
            context.memory->writeq(TEST_HEAP_ADDRESS + 0x100, bits[kind]);
            fpMem(0xdd, 0, 0x100);
            fp(0xd9, 0xe8); fp(0xd8, 0xc0); // 2,other
            fp(0xd9, 0xc9); fp(0xd9, 0xc9);
            if (kind == 4) fp(0xdd, 0xc1);
            fp(0xdb, 0xf1); // fcomi must not clobber persistent tags
            fp(variant < 2 ? 0xdb : 0xdf, variant % 2 ? 0xe9 : 0xf1);
            testRunCPU();
            if ((context.cpu->flags & FMASK_TEST) != flags[kind] || context.cpu->fpu.top != (variant < 2 ? 4 : 5))
                testFail("cached FCOMI flags/pop variant=%u kind=%u", variant, kind);
            if (context.cpu->fpu.tags[5] != (kind == 4 ? TAG_Empty : TAG_Valid))
                testFail("cached FCOMI changed operand tag");
            requireFpJit();
        }
    }
}

void testWasmJitFpuDivRound() {
    // Every memory division encoding, all TOPs, signed operands, precision,
    // cached tags, and direct/cross-page accesses. Continue with cached math
    // after both the inline division and its whole-instruction fallback.
    const U8 opcodes[] = {0xd8, 0xdc, 0xda, 0xde};
    const U64 operands[] = {0xc0400000ull, 0xc008000000000000ull, 0xfffffffdull, 0xfffdull};
    for (U32 type = 0; type < 4; ++type) {
        for (bool reverse : {false, true}) {
            for (U32 top = 0; top < 8; ++top) {
                for (U32 mode : {0u, 1u, 2u}) {
                    for (bool single : {false, true}) {
                        for (U32 address : {0x100u, 0x1fffu}) {
                            beginFp(top);
                            auto& context = testContext();
                            auto& fpu = context.cpu->fpu;
                            fpu.SetCW((single ? 0x007f : 0x037f) & (mode == 1 ? ~FPU_SW_ZE : 0xffff));
                            fpu.FLD_I64(2, top);
                            fpu.tags[top] = mode == 2 ? TAG_Special : TAG_Valid;
                            context.memory->writeq(TEST_HEAP_ADDRESS + address, operands[type]);
                            fp(0xd9, 0xe8); // 1,2
                            fp(0xd9, 0xc9); // 2,1; dirty renamed values/tags
                            fpMem(opcodes[type], reverse ? 7 : 6, address);
                            fp(0xd9, 0xe8); fp(0xde, 0xc1); // add 1 after either path
                            testRunCPU();
                            double result = reverse ? -3.0 / 2.0 : 2.0 / -3.0;
                            if (single) result = (double)(float)result;
                            result += 1;
                            expectFp(0, single ? (double)(float)result : result);
                            expectFp(1, 1);
                            if (fpu.top != ((top + 7) & 7)) testFail("cached memory divide changed TOP");
                            requireFpJit();
                        }
                    }
                }
            }
        }
    }

    // Both signs of a real zero divisor must use the exception path. Control
    // word changes while values are cached must update the division guard and
    // pending-exception summary. Integer zero and empty ST0 use the same path.
    for (U32 type = 0; type < 4; ++type) {
        for (bool negativeZero : {false, true}) {
            if (type >= 2 && negativeZero) continue;
            for (bool unmasked : {false, true}) {
                for (bool empty : {false, true}) {
                    beginFp(3);
                    auto& context = testContext();
                    U64 zero = !negativeZero ? 0 : (type == 0 ? 0x80000000ull : 0x8000000000000000ull);
                    context.memory->writeq(TEST_HEAP_ADDRESS + 0x100, zero);
                    context.memory->writew(TEST_HEAP_ADDRESS + 0x200, unmasked ? 0x037a : 0x037f);
                    context.memory->writew(TEST_HEAP_ADDRESS + 0x202, 0x037f);
                    fp(0xd9, 0xe8); fp(0xd8, 0xc0); // unrelated dirty 2
                    fp(0xd9, 0xe8); // divisor's numerator 1,2
                    if (empty) fp(0xdd, 0xc0); // free ST0 without materializing it
                    fpMem(0xd9, 5, 0x200); // fldcw after cache activation
                    fpMem(opcodes[type], 6, 0x100);
                    fpMem(0xdd, 7, 0x208); // snapshot status before masking
                    fpMem(0xd9, 5, 0x202);
                    testRunCPU();
                    U32 bits = empty ? FPU_SW_IE | FPU_SW_SF : FPU_SW_ZE;
                    U32 sw = context.memory->readw(TEST_HEAP_ADDRESS + 0x208);
                    if ((sw & bits) != bits || !!(sw & FPU_SW_ES) != unmasked ||
                        (context.cpu->fpu.sw & FPU_SW_ES) || context.cpu->fpu.divExceptionsUnmasked)
                        testFail("cached memory divide/control word lost exception state");
                    expectFp(1, 2);
                    if (context.cpu->fpu.top != 1) testFail("cached memory divide exception TOP");
                }
            }
        }
    }

    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const struct { double input; double expected[4]; } rounded[] = {
        {2.5, {2, 2, 3, 2}}, {-2.5, {-2, -3, -2, -2}},
        {3.5, {4, 3, 4, 3}}, {-3.5, {-4, -4, -3, -3}},
        {-0.25, {-0.0, -1, -0.0, -0.0}}, {-0.0, {-0.0, -0.0, -0.0, -0.0}},
        {1e30, {1e30, 1e30, 1e30, 1e30}},
        {infinity, {infinity, infinity, infinity, infinity}},
        {-infinity, {-infinity, -infinity, -infinity, -infinity}},
        {nan, {nan, nan, nan, nan}}
    };
    for (U32 top = 0; top < 8; ++top) {
        for (U32 rounding = 0; rounding < 4; ++rounding) {
            for (const auto& item : rounded) {
                beginFp(top);
                auto& context = testContext();
                U64 bits; memcpy(&bits, &item.input, sizeof(bits));
                context.memory->writeq(TEST_HEAP_ADDRESS + 0x100, bits);
                context.memory->writew(TEST_HEAP_ADDRESS + 0x200, 0x037f | (rounding << 10));
                context.memory->writew(TEST_HEAP_ADDRESS + 0x202, 0x037f);
                fp(0xd9, 0xe8); fpMem(0xdd, 0, 0x100); // input,1
                fp(0xd9, 0xc9); fp(0xd9, 0xc9); // dirty renamed value
                testPushCode8(0xb8); testPushCode32(0x12345678); testPushCode8(0x40);
                fpMem(0xd9, 5, 0x200); // fldcw
                fpMem(0xd9, 7, 0x204); // fnstcw
                fp(0xd9, 0xfc); // frndint
                fpMem(0xdd, 2, 0x300); // store retained rounded value
                fpMem(0xd9, 5, 0x202); // restore nearest without flushing
                fpMem(0xd9, 7, 0x206);
                testRunCPU();
                U64 actualBits = context.memory->readq(TEST_HEAP_ADDRESS + 0x300);
                double actual; memcpy(&actual, &actualBits, sizeof(actual));
                double expected = item.expected[rounding];
                U64 expectedBits; memcpy(&expectedBits, &expected, sizeof(expected));
                if (expected != expected ? actual == actual : actualBits != expectedBits)
                    testFail("cached FRNDINT rounding mode %u: %.17g -> %.17g, expected %.17g", rounding, item.input, actual, expected);
                if (context.cpu->reg[0].u32 != 0x12345679 || context.cpu->fpu.cw != 0x037f || context.cpu->fpu.round != 0 ||
                    context.memory->readw(TEST_HEAP_ADDRESS + 0x204) != (0x037f | (rounding << 10)) ||
                    context.memory->readw(TEST_HEAP_ADDRESS + 0x206) != 0x037f)
                    testFail("cached rounding/control word corrupted GP or control state");
                expectFp(1, 1);
                requireFpJit();
            }
        }
    }

    // Fused integer branches must retain their condition across FRNDINT's
    // rounding-mode branches, which do not change guest integer flags.
    for (U32 rounding = 0; rounding < 4; ++rounding) {
        for (U32 taken : {0u, 1u}) {
            beginFp(3);
            testContext().cpu->fpu.SetCW(0x037f | (rounding << 10));
            fp(0xd9, 0xe8);
            testPushCode8(0xb8); testPushCode32(taken);
            fp(0x83, 0xf8); testPushCode8(1);
            fp(0xd9, 0xfc);
            testPushCode8(0x74); testPushCode8(2);
            fp(0xd8, 0xc0);
            fp(0x31, 0xdb);
            testRunCPU();
            expectFp(0, taken ? 1 : 2);
        }
    }

    // Control-word memory helpers return into the live cache. Verify both
    // a cross-page load and store preserve dirty values and GP registers.
    beginFp(3);
    auto& cross = testContext();
    cross.memory->writeq(TEST_HEAP_ADDRESS + 0x100, 0x4004000000000000ull); // 2.5
    cross.memory->writew(TEST_HEAP_ADDRESS + 0x1fff, 0x077f); // round down
    fpMem(0xdd, 0, 0x100); fp(0xd9, 0xe8); fp(0xde, 0xc1); // dirty 3.5
    testPushCode8(0xb8); testPushCode32(123); testPushCode8(0x40);
    fpMem(0xd9, 5, 0x1fff); fp(0xd9, 0xfc); // 3
    fpMem(0xd9, 7, 0x3fff);
    fp(0xd8, 0xc0); // retained 6
    testRunCPU();
    expectFp(0, 6);
    if (cross.cpu->reg[0].u32 != 124 || cross.cpu->fpu.cw != 0x077f ||
        cross.memory->readw(TEST_HEAP_ADDRESS + 0x3fff) != 0x077f)
        testFail("cross-page control-word helper lost cached state");

    // A complete round/divide/control-word loop keeps the accumulator cached
    // across its backedge, and alternates rounding modes at runtime.
    beginFp(4);
    auto& context = testContext();
    context.memory->writed(TEST_HEAP_ADDRESS + 0x100, 0x40000000); // 2
    context.memory->writew(TEST_HEAP_ADDRESS + 0x200, 0x0b7f); // up
    context.memory->writew(TEST_HEAP_ADDRESS + 0x202, 0x037f); // nearest
    fp(0xd9, 0xe8);
    testPushCode8(0xb9); testPushCode32(100);
    U32 loop = context.codeIp;
    fp(0xd9, 0xe8); fp(0xde, 0xc1); // add 1
    fpMem(0xd8, 6, 0x100); // divide by 2
    fpMem(0xd9, 5, 0x200); fp(0xd9, 0xfc); // ceil
    fpMem(0xd9, 5, 0x202);
    fp(0xd8, 0xc0); // multiply by 2, accumulator += 2
    testPushCode8(0x49);
    testPushCode8(0x75); testPushCode8((U8)(loop - (context.codeIp + 1)));
    testRunCPU();
    expectFp(0, 200);
    requireFpJit();
}

#endif // BOXEDWINE_WASM_JIT

void testWasmJitFpuDivRoundFaults() {
    const struct { U8 opcode; U8 group; } memoryOps[] = {
        {0xd8, 6}, {0xd8, 7}, {0xdc, 6}, {0xdc, 7},
        {0xda, 6}, {0xda, 7}, {0xde, 6}, {0xde, 7},
        {0xd9, 5}, {0xd9, 7} // fldcw / fnstcw
    };
    for (const auto& op : memoryOps) {
        for (U32 address : {0x2000u, 0x1fffu}) {
            beginFp(3);
            auto& context = testContext();
            context.memory->writeq(TEST_HEAP_ADDRESS + address, 0x1122334455667788ull);
            fp(0xd9, 0xe8); fp(0xd8, 0xc0); // dirty 2
            fpMem(op.opcode, op.group, address);
            fp(0xd8, 0xc0); // must not run
            auto oldAction = context.process->sigActions[K_SIGSEGV];
            auto& action = context.process->sigActions[K_SIGSEGV];
            action.reset(); action.handlerAndSigAction = context.codeIp;
            context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, 0);
            testRunCPU();
            context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
            if (action.sigInfo[0] != K_SIGSEGV) testFail("cached divide/control word did not fault");
            expectFp(0, 2);
            if (context.cpu->fpu.top != 2 || context.cpu->fpu.cw != 0x037f || context.cpu->fpu.round != 0 ||
                context.memory->readq(TEST_HEAP_ADDRESS + address) != 0x1122334455667788ull)
                testFail("faulting divide/control word changed architectural state");
            context.process->sigActions[K_SIGSEGV] = oldAction;
        }
    }
}

void testWasmJitFpuFaultState() {
    // FNSTSW writes exactly two bytes; the next page need not be accessible.
    beginFp(3);
    auto& wordContext = testContext();
    fp(0xd9, 0xe8); fp(0xd8, 0xc0);
    fpMem(0xdd, 7, 0x1ffe);
    fp(0xd8, 0xc0);
    auto oldWordAction = wordContext.process->sigActions[K_SIGSEGV];
    auto& wordAction = wordContext.process->sigActions[K_SIGSEGV];
    wordAction.reset(); wordAction.handlerAndSigAction = wordContext.codeIp;
    wordContext.memory->mprotect(wordContext.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, 0);
    testRunCPU();
    wordContext.memory->mprotect(wordContext.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
    if (wordAction.sigInfo[0] == K_SIGSEGV || wordContext.memory->readw(TEST_HEAP_ADDRESS + 0x1ffe) != 0x1000)
        testFail("cached FNSTSW accessed beyond its word operand");
    expectFp(0, 4);
    wordContext.process->sigActions[K_SIGSEGV] = oldWordAction;

    // Both x87 whole-instruction fallback and integer memory-only helpers
    // must expose the same pre-fault FPU state to the signal handler. Include
    // a store straddling a writable page and a protected page.
    for (U32 memoryOp : {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u}) {
        for (bool store : {false, true}) {
            if (memoryOp >= 3 && memoryOp <= 6 && !store) continue;
            if ((memoryOp == 7 || memoryOp == 8) && store) continue;
            if (memoryOp == 9 && !store) continue;
            for (U32 address : {0x2000u, memoryOp == 9 ? 0x1fffu : 0x1ffeu}) {
                beginFp(3);
                auto& context = testContext();
                context.memory->writed(TEST_HEAP_ADDRESS + address, 0x11223344);
                fp(0xd9, 0xe8);
                fp(0xd8, 0xc0); // dirty 2
                testPushCode8(0xb8); testPushCode32(0x13579bdf);
                testPushCode8(0x40); // inc eax with live FPU cache
                if (memoryOp == 0) fpMem(0xd9, store ? 3 : 0, address); // fstp/fld
                else if (memoryOp == 1) fpMem(store ? 0x89 : 0x8b, 0, address); // MOV ModR/M
                else if (memoryOp == 2) {
                    testPushCode8(store ? 0xa3 : 0xa1); testPushCode32(address); // MOV moffs
                } else if (memoryOp <= 5) {
                    fpMem(0xdb, memoryOp - 2, address); // FISTTP/FIST/FISTP
                } else if (memoryOp == 6) {
                    testPushCode8(0x83); testPushCode8(0x05); testPushCode32(address); testPushCode8(1); // add [mem],1
                } else {
                    if (memoryOp == 8) fpMem(0xd8, 3, address); // fcomp float: fault must not compare/pop
                    else if (memoryOp == 9) fpMem(0xdd, 7, address); // fnstsw word
                    else {
                        testPushCode8(0xf3); testPushCode8(0x0f); fpMem(0x58, 0, address); // addss xmm0,[mem]
                    }
                }
                fp(0xd8, 0xc0); // must not run
                auto oldAction = context.process->sigActions[K_SIGSEGV];
                auto& action = context.process->sigActions[K_SIGSEGV];
                action.reset();
                action.handlerAndSigAction = context.codeIp;
                context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, 0);
                testRunCPU();
                context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
                if (action.sigInfo[0] != K_SIGSEGV) testFail("x87 cache test did not fault");
                expectFp(0, 2);
                if (context.cpu->fpu.top != 2) testFail("faulting memory op changed x87 TOP");
                if (context.memory->readd(TEST_HEAP_ADDRESS + address) != 0x11223344)
                    testFail("faulting store changed memory before completing");
                context.process->sigActions[K_SIGSEGV] = oldAction;
            }
        }
    }
}
#endif
