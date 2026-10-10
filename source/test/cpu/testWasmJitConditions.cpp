/*
 * Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "boxedwine.h"
#include "testCPU.h"

#ifdef BOXEDWINE_WASM_JIT
#include "../../emulation/cpu/jit/jitCodeGen.h"
#include "../../emulation/cpu/normal/normal_strings.h"
namespace {

bool conditionExpected(U32 condition, U32 flags) {
    bool of = (flags & OF) != 0, cf = (flags & CF) != 0;
    bool zf = (flags & ZF) != 0, sf = (flags & SF) != 0;
    bool pf = (flags & PF) != 0;
    const bool values[] = {of, !of, cf, !cf, zf, !zf, cf || zf,
        !cf && !zf, sf, !sf, pf, !pf, sf != of, sf == of,
        zf || sf != of, !zf && sf == of};
    return values[condition];
}

void emitMov(U32 reg, U32 value) {
    testPushCode8(0xb8 + reg);
    testPushCode32(value);
}

void emitIndirectBoundary() {
    emitMov(2, 0x100); // mov edx,target; jmp edx
    testPushCode8(0xff);
    testPushCode8(0xe2);
    while (testContext().codeIp < TEST_CODE_ADDRESS + 0x100) {
        testPushCode8(0x90);
    }
}

void runConsumers(U32 condition, U32 expectedFlags, bool crossBlock) {
    CPU* cpu = testContext().cpu;
    U32 consumer = testContext().codeIp;
    // These values must survive both inline predicates and helper fallbacks
    // while dirty in the GP local cache. SETcc must preserve the other bytes.
    emitMov(0, 0x12345678);
    emitMov(3, 0x89abcdef);
    emitMov(1, 0x90abcde0);
    emitMov(6, 0x55667788);
    emitMov(7, 0xaabbccdd);
    testPushCode8(0x0f); testPushCode8(0x90 + condition); testPushCode8(0xc0); // setcc al
    testPushCode8(0x0f); testPushCode8(0x90 + condition); testPushCode8(0xc7); // setcc bh
    testPushCode8(0x0f); testPushCode8(0x40 + condition); testPushCode8(0xf1); // cmovcc esi,ecx
    testPushCode8(0x66);
    testPushCode8(0x0f); testPushCode8(0x40 + condition); testPushCode8(0xf9); // cmovcc di,cx
    emitMov(2, 0);
    testPushCode8(0x70 + condition); testPushCode8(5); // jcc skips mov edx,1
    emitMov(2, 1);
    testPushCode8(0x9c); testPushCode8(0x5d); // pushfd; pop ebp
    testRunCPU();
    bool taken = conditionExpected(condition, expectedFlags);
    if (cpu->reg[0].u32 != (0x12345600u | taken) ||
        cpu->reg[3].u32 != (0x89ab00efu | ((U32)taken << 8)) ||
        cpu->reg[1].u32 != 0x90abcde0 ||
        cpu->reg[6].u32 != (taken ? 0x90abcde0u : 0x55667788u) ||
        cpu->reg[7].u32 != (taken ? 0xaabbcde0u : 0xaabbccddu) ||
        cpu->reg[2].u32 != (taken ? 0u : 1u) || cpu->reg[4].u32 != 4096 ||
        ((cpu->reg[5].u32 ^ expectedFlags) & (CF | PF | AF | ZF | SF | OF | DF))) {
        testFail("WASM condition %u flags %x cross-block %u: SETcc/CMOV/Jcc/register preservation",
                 condition, expectedFlags, crossBlock);
    }
    DecodedOp* entry = testContext().memory->getDecodedOp(TEST_CODE_ADDRESS);
    if (!entry || !entry->pfnJitCode || entry->pfn != cpu->thread->process->startJITOp) {
        testFail("WASM materialized condition producer was not JIT compiled");
    }
    if (crossBlock) {
        DecodedOp* target = testContext().memory->getDecodedOp(consumer);
        if (!entry || !target || !target->pfnJitCode || target->pfnJitCode == entry->pfnJitCode) {
            testFail("WASM condition consumer must use a distinct compiled block");
        }
    }
}

} // namespace

void testWasmJitMaterializedConditions() {
    const U32 bits[] = {CF, PF, ZF, SF, OF};
    for (U32 pattern = 0; pattern < 32; ++pattern) {
        U32 flags = AF | DF;
        for (U32 bit = 0; bit < 5; ++bit) {
            if (pattern & (1u << bit)) flags |= bits[bit];
        }
        for (U32 condition = 0; condition < 16; ++condition) {
            for (bool boundary : {false, true}) {
                testNewInstruction(flags);
                // PUSHFD materializes the incoming flags in the same block.
                testPushCode8(0x9c); testPushCode8(0x5a);
                if (boundary) emitIndirectBoundary();
                runConsumers(condition, flags, boundary);
            }
        }
    }
    // Unknown incoming lazy state must still use its formula, not stale flags.
    const U32 comparisons[][3] = {
        {0, 1, CF | PF | AF | SF},
        {0x80000000, 1, OF | PF | AF},
        {5, 5, ZF | PF}
    };
    for (const auto& comparison : comparisons) {
        for (U32 condition = 0; condition < 16; ++condition) {
            testNewInstruction(0);
            emitMov(6, comparison[0]); emitMov(7, comparison[1]);
            testPushCode8(0x39); testPushCode8(0xfe); // cmp esi,edi
            emitIndirectBoundary();
            runConsumers(condition, comparison[2], true);
        }
    }
}

void testWasmJitForwardLoopBranches() {
    // The harness uses a nonzero CS base. Cover taken/fall-through joins,
    // an unconditional forward edge, dirty GP/XMM locals, lazy flags, and
    // more than one 64-iteration activation of the bounded Wasm loop.
    for (U32 iterations : {2u, 65u, 130u}) {
        testNewInstruction(CF);
        auto& context = testContext();
        CPU* cpu = context.cpu;
        cpu->fpu.FINIT();
        testPushCode8(0xd9); testPushCode8(0xe8); // fld1: dirty x87 state before the integer loop
        emitMov(1, iterations);
        U32 loop = context.codeIp;
        testPushCode8(0xf7); testPushCode8(0xc1); testPushCode32(1); // test ecx,1
        testPushCode8(0x74); U32 toEven = context.codeIp; testPushCode8(0);
        testPushCode8(0x74); U32 toEvenAgain = context.codeIp; testPushCode8(0); // shared forward label
        testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(3); // add eax,3
        emitMov(3, 0x11223344);
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc3); // movd xmm0,ebx
        testPushCode8(0xeb); U32 toJoin = context.codeIp; testPushCode8(0);
        U32 even = context.codeIp;
        testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(5); // add eax,5
        emitMov(3, 0x55667788);
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc3);
        U32 join = context.codeIp;
        testPushCode8(0x89); testPushCode8(0xda); // mov edx,ebx
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x7e); testPushCode8(0xc6); // movd esi,xmm0
        testPushCode8(0x9c); testPushCode8(0x5d); // pushfd; pop ebp
        testPushCode8(0x49); // dec ecx
        testPushCode8(0x75); testPushCode8((U8)(loop - (context.codeIp + 1)));
        testPushCode8(0xd9); testPushCode8(0x1d); testPushCode32(0x200); // fstp dword [0x200]
        context.memory->writeb(toEven, (U8)(even - toEven - 1));
        context.memory->writeb(toEvenAgain, (U8)(even - toEvenAgain - 1));
        context.memory->writeb(toJoin, (U8)(join - toJoin - 1));
        testRunCPU();
        U32 expected = ((iterations + 1) / 2) * 3 + (iterations / 2) * 5;
        U32 bits = expected & 0xff, parity = 0;
        for (; bits; bits >>= 1) parity ^= bits & 1;
        U32 flags = parity ? 0 : PF;
        if (((expected - 3) & 15) + 3 > 15) flags |= AF;
        if (cpu->reg[0].u32 != expected || cpu->reg[1].u32 ||
            cpu->reg[2].u32 != 0x11223344 || cpu->reg[6].u32 != 0x11223344 ||
            ((cpu->reg[5].u32 ^ flags) & (CF | PF | AF | ZF | SF | OF)) ||
            cpu->reg[4].u32 != 4096 || context.memory->readd(TEST_HEAP_ADDRESS + 0x200) != 0x3f800000) {
            testFail("WASM forward loop join lost register/flag state (%u iterations)", iterations);
        }
        for (U32 target : {even, join}) {
            DecodedOp* op = context.memory->getDecodedOp(target);
            if (!op || op->pfnJitCode) {
                testFail("WASM forward loop target returned to dispatcher (%u iterations)", iterations);
            }
        }
    }
}

void testWasmJitForwardBranches() {
    auto patch = [](U32 displacement, U32 target) {
        testContext().memory->writeb(displacement, (U8)(target - displacement - 1));
    };
    auto requireInternal = [](U32 target) {
        DecodedOp* op = testContext().memory->getDecodedOp(target);
        if (!op || op->pfnJitCode) testFail("WASM forward target %x returned to dispatcher", target);
    };
    const U32 flagBits[] = {CF, PF, ZF, SF, OF};
    // All predicates, two incoming flag states, shared labels, an unconditional
    // edge, and dirty GP/XMM values at a join outside any backward loop.
    for (U32 condition = 0; condition < 16; ++condition) {
        for (bool taken : {false, true}) {
            U32 flags = 0;
            for (U32 pattern = 0; pattern < 32; ++pattern) {
                flags = AF | DF;
                for (U32 bit = 0; bit < 5; ++bit) {
                    if (pattern & (1u << bit)) flags |= flagBits[bit];
                }
                if (conditionExpected(condition, flags) == taken) break;
            }
            testNewInstruction(flags);
            auto& context = testContext();
            CPU* cpu = context.cpu;
            emitMov(0, 0x12345678);
            testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc0);
            testPushCode8(0x70 + condition); U32 toTaken = context.codeIp; testPushCode8(0);
            testPushCode8(0x70 + condition); U32 toTakenAgain = context.codeIp; testPushCode8(0);
            emitMov(3, 0x1111);
            testPushCode8(0xeb); U32 toJoin = context.codeIp; testPushCode8(0);
            U32 takenTarget = context.codeIp;
            emitMov(3, 0x2222);
            U32 join = context.codeIp;
            testPushCode8(0x89); testPushCode8(0xda); // mov edx,ebx
            testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x7e); testPushCode8(0xc6);
            testPushCode8(0x9c); testPushCode8(0x5d);
            patch(toTaken, takenTarget); patch(toTakenAgain, takenTarget); patch(toJoin, join);
            testRunCPU();
            if (cpu->reg[0].u32 != 0x12345678 || cpu->reg[6].u32 != 0x12345678 ||
                    cpu->reg[2].u32 != (taken ? 0x2222u : 0x1111u) || cpu->reg[4].u32 != 4096 ||
                    ((cpu->reg[5].u32 ^ flags) & (CF | PF | AF | ZF | SF | OF | DF))) {
                testFail("WASM forward diamond condition %u taken %u lost state", condition, taken);
            }
            requireInternal(takenTarget); requireInternal(join);
        }
    }
    // Distinct lazy-flag producers and XMM writes on each incoming path.
    for (bool taken : {false, true}) {
        testNewInstruction(taken ? ZF : 0);
        auto& context = testContext();
        CPU* cpu = context.cpu;
        emitMov(0, 0x7ffffffe);
        testPushCode8(0x74); U32 toSub = context.codeIp; testPushCode8(0);
        testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(3); // add eax,3
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc0);
        testPushCode8(0xeb); U32 toJoin = context.codeIp; testPushCode8(0);
        U32 sub = context.codeIp;
        testPushCode8(0x83); testPushCode8(0xe8); testPushCode8(5); // sub eax,5
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc0);
        U32 join = context.codeIp;
        testPushCode8(0x9c); testPushCode8(0x5d);
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x7e); testPushCode8(0xc6);
        patch(toSub, sub); patch(toJoin, join);
        testRunCPU();
        U32 expected = taken ? 0x7ffffff9u : 0x80000001u;
        U32 flags = taken ? PF : (SF | AF | OF);
        if (cpu->reg[0].u32 != expected || cpu->reg[6].u32 != expected ||
                ((cpu->reg[5].u32 ^ flags) & (CF | PF | AF | ZF | SF | OF))) {
            testFail("WASM forward lazy-flag/XMM join taken %u lost state", taken);
        }
        requireInternal(join);
    }
    // Enter at a loop header, skip a whole loop, or leave it early. A second
    // disjoint loop must always initialize its own budget, including after
    // a branch has skipped the first loop's Wasm end and its remaining body.
    for (U32 mode : {0u, 1u, 2u}) {
        for (U32 iterations : {2u, 65u, 130u}) {
            testNewInstruction(0);
            auto& context = testContext();
            CPU* cpu = context.cpu;
            emitMov(1, iterations);
            emitMov(3, mode);
            testPushCode8(0x83); testPushCode8(0xfb); testPushCode8(1); // cmp ebx,1
            testPushCode8(0x74); U32 skipLoop = context.codeIp; testPushCode8(0);
            testPushCode8(0x83); testPushCode8(0xfb); testPushCode8(0); // cmp ebx,0
            testPushCode8(0x74); U32 toHeader = context.codeIp; testPushCode8(0);
            testPushCode8(0x42); // inc edx, only on the early-exit path
            U32 firstLoop = context.codeIp;
            testPushCode8(0x40); // inc eax
            testPushCode8(0x83); testPushCode8(0xfb); testPushCode8(2); // cmp ebx,2
            testPushCode8(0x74); U32 leaveLoop = context.codeIp; testPushCode8(0);
            testPushCode8(0x49); // dec ecx
            testPushCode8(0x75); testPushCode8((U8)(firstLoop - context.codeIp - 1));
            U32 afterLoop = context.codeIp;
            emitMov(1, iterations + 1);
            U32 secondLoop = context.codeIp;
            testPushCode8(0x47); // inc edi
            testPushCode8(0x49);
            testPushCode8(0x75); testPushCode8((U8)(secondLoop - context.codeIp - 1));
            patch(skipLoop, afterLoop); patch(toHeader, firstLoop); patch(leaveLoop, afterLoop);
            testRunCPU();
            if (cpu->reg[0].u32 != (mode == 0 ? iterations : mode == 2 ? 1u : 0u) ||
                    cpu->reg[2].u32 != (mode == 2 ? 1u : 0u) || cpu->reg[1].u32 ||
                    cpu->reg[7].u32 != iterations + 1) {
                testFail("WASM forward loop boundary mode %u iterations %u lost state", mode, iterations);
            }
            requireInternal(afterLoop);
        }
    }
    // Do not enter the middle of a Wasm loop: that would bypass its budget
    // initialization. The dispatcher must give this target its own entry.
    testNewInstruction(ZF);
    auto& context = testContext();
    CPU* cpu = context.cpu;
    emitMov(1, 3);
    testPushCode8(0x74); U32 intoLoop = context.codeIp; testPushCode8(0);
    U32 header = context.codeIp;
    testPushCode8(0x40);
    U32 interior = context.codeIp;
    testPushCode8(0x43);
    testPushCode8(0x49);
    testPushCode8(0x75); testPushCode8((U8)(header - context.codeIp - 1));
    patch(intoLoop, interior);
    testRunCPU();
    DecodedOp* interiorOp = context.memory->getDecodedOp(interior);
    if (cpu->reg[0].u32 != 2 || cpu->reg[3].u32 != 3 || cpu->reg[1].u32 ||
            !interiorOp || !interiorOp->pfnJitCode) {
        testFail("WASM loop interior entry must retain dispatcher fallback");
    }
}

void testWasmJitRepMovsdState() {
    // Exercise both the adaptive loop and each expected-count body, with
    // flat, zero-base-but-set, and genuinely nonzero segment bases.
    for (U32 profile : {1u, 6u, 7u}) for (U32 mode : {0u, 1u, 2u}) for (bool backward : {false, true})
    for (U32 count : {0u, 1u, 6u, 7u, 8u, 12u, 15u, 16u, 17u, 32u, 511u, 512u, 513u, 1025u}) {
        testNewInstruction(backward ? DF : 0);
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        KMemory* mem = ctx.memory;
        const U32 sourceBase = mode == 2 ? TEST_HEAP_ADDRESS : 0;
        const U32 destBase = mode == 2 ? TEST_HEAP_ADDRESS + 0x1000 : 0;
        cpu->seg[DS].address = sourceBase;
        cpu->seg[ES].address = destBase;
        ctx.process->hasSetSeg[DS] = ctx.process->hasSetSeg[ES] = mode != 0;
        cpu->fpu.FINIT();
        for (U32 i = 0; i < 0x1800; ++i) {
            mem->writeb(TEST_HEAP_ADDRESS + 0x800 + i, (U8)(i * 37 + 9));
            mem->writeb(TEST_HEAP_ADDRESS + 0x3800 + i, 0x55);
        }
        const U32 offset = count & 3;
        const U32 last = backward && count ? (count - 1) * 4 : 0;
        // Zero-count REP must not access either invalid address.
        const U32 src = count ? TEST_HEAP_ADDRESS + 0x800 + offset + last - sourceBase : 0xfffffff0;
        const U32 dst = count ? TEST_HEAP_ADDRESS + 0x3800 + offset + last - destBase : 0xfffffff0;
        testPushCode8(0xd9); testPushCode8(0xe8); // fld1 (live x87 cache)
        emitMov(0, 0x12345678);
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc0); // movd xmm0,eax
        emitMov(0, 0xffffffff);
        testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(1); // add eax,1 (live lazy flags)
        emitMov(3, 0x87654321); emitMov(6, src); emitMov(7, dst); emitMov(1, count);
        testPushCode8(0xf3); testPushCode8(0xa5);
        testPushCode8(0x9c); testPushCode8(0x5d); // pushfd; pop ebp
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x7e); testPushCode8(0xc2); // movd edx,xmm0
        testPushCode8(0xd9); testPushCode8(0x1d); testPushCode32(TEST_HEAP_ADDRESS + 0x200 - sourceBase);
        testPushCode8(0xcd); testPushCode8(0x97);
        DecodedOp* entry = cpu->getNextOp();
        for (DecodedOp* op = entry; op; op = op->next) {
            op->runCount = JIT_RUN_COUNT + 1;
            if (op->inst == Movsd) {
                for (U32 i = 0; i < 20; ++i) profileMovsdCount(op, profile, true);
            }
            op->DF0 = !backward;
            op->DF1 = backward;
            if (op->isBranch()) break;
        }
        startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
        if (!entry->pfnJitCode) testFail("REP cached-state case not compiled");
        testRunCPU();
        const U32 delta = backward ? -count * 4 : count * 4;
        if (cpu->reg[0].u32 || cpu->reg[1].u32 || cpu->reg[2].u32 != 0x12345678 || cpu->reg[3].u32 != 0x87654321 ||
                cpu->reg[6].u32 != src + delta || cpu->reg[7].u32 != dst + delta ||
                (cpu->reg[5].u32 & (CF | ZF | AF | PF | SF | OF | DF)) != (CF | ZF | AF | PF | (backward ? DF : 0)) ||
                mem->readd(TEST_HEAP_ADDRESS + 0x200) != 0x3f800000) {
            testFail("REP cache/register/flag state mode=%u count=%u DF=%u", mode, count, backward);
        }
        for (U32 i = 0; i < 0x1800; ++i) {
            const U8 expected = i >= offset && i < offset + count * 4 ? (U8)(i * 37 + 9) : 0x55;
            if (mem->readb(TEST_HEAP_ADDRESS + 0x3800 + i) != expected) {
                testFail("REP data/guard mismatch");
                break;
            }
        }
    }
}

void testWasmJitLoopReachability() {
    auto patch = [](U32 displacement, U32 target) {
        testContext().memory->writeb(displacement, (U8)(target - displacement - 1));
    };
    // Out-of-line code jumps backward to cleanup, which returns rather than
    // looping. Cover both RET forms, a jump into cleanup, and both paths of
    // the forward branch with dirty registers/XMM and a nonzero CS base.
    for (bool taken : {false, true}) {
        for (bool popArguments : {false, true}) {
            for (bool trampoline : {false, true}) {
                U32 flags = CF | DF | (taken ? ZF : 0);
                testNewInstruction(flags);
                auto& context = testContext();
                CPU* cpu = context.cpu;
                if (popArguments) {
                    testPushCode8(0x68); testPushCode32(0x1234);
                    testPushCode8(0x68); testPushCode32(0x5678);
                }
                testPushCode8(0x68); U32 returnAddress = context.codeIp; testPushCode32(0);
                emitMov(0, 0x12345678);
                testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc0);
                testPushCode8(0x74); U32 toTail = context.codeIp; testPushCode8(0);
                U32 cleanup = context.codeIp;
                if (trampoline) { testPushCode8(0xeb); testPushCode8(0); }
                testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x7e); testPushCode8(0xc6);
                testPushCode8(0x9c); testPushCode8(0x5d); // pushfd; pop ebp
                testPushCode8(popArguments ? 0xc2 : 0xc3);
                if (popArguments) testPushCode16(8);
                U32 tail = context.codeIp;
                emitMov(0, 0x89abcdef);
                testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc0);
                // Also cover a conditional backward edge (known taken here).
                testPushCode8(popArguments ? 0x74 : 0xeb);
                U32 backward = context.codeIp; testPushCode8(0);
                patch(backward, cleanup); patch(toTail, tail);
                context.memory->writed(returnAddress, context.codeIp - TEST_CODE_ADDRESS);
                testPushCode8(0xcd); testPushCode8(0x97);
                testRunCPU();
                U32 expected = taken ? 0x89abcdef : 0x12345678;
                if (cpu->reg[0].u32 != expected || cpu->reg[6].u32 != expected ||
                        cpu->reg[4].u32 != 4096 ||
                        ((cpu->reg[5].u32 ^ flags) & (CF | PF | AF | ZF | SF | OF | DF))) {
                    testFail("WASM cleanup backward jump lost state (taken %u, arguments %u, trampoline %u)",
                        taken, popArguments, trampoline);
                }
                DecodedOp* target = context.memory->getDecodedOp(tail);
                if (!target || target->pfnJitCode) {
                    testFail("WASM false cleanup loop forced forward target through dispatcher (taken %u, arguments %u, trampoline %u)",
                        taken, popArguments, trampoline);
                }
            }
        }
    }
    // A return inside the address interval does not rule out a real loop:
    // its continuing path can jump over the return. Preserve the internal
    // backedge and bounded execution, as well as the optional early return.
    for (bool earlyExit : {false, true}) {
        for (U32 iterations : {2u, 65u, 130u}) {
            testNewInstruction(CF);
            auto& context = testContext();
            CPU* cpu = context.cpu;
            testPushCode8(0x68); U32 returnAddress = context.codeIp; testPushCode32(0);
            emitMov(1, iterations);
            emitMov(3, earlyExit ? 1 : 0);
            U32 header = context.codeIp;
            testPushCode8(0x85); testPushCode8(0xdb); // test ebx,ebx
            testPushCode8(0x74);
            U32 skipReturn = context.codeIp; testPushCode8(0);
            testPushCode8(0xc3);
            U32 body = context.codeIp;
            testPushCode8(0x40); // inc eax
            testPushCode8(0x49); // dec ecx
            testPushCode8(0x75); U32 backward = context.codeIp; testPushCode8(0);
            testPushCode8(0xc3);
            patch(skipReturn, body); patch(backward, header);
            context.memory->writed(returnAddress, context.codeIp - TEST_CODE_ADDRESS);
            testPushCode8(0xcd); testPushCode8(0x97);
            testRunCPU();
            if (cpu->reg[0].u32 != (earlyExit ? 0 : iterations) ||
                    cpu->reg[1].u32 != (earlyExit ? iterations : 0) || cpu->reg[4].u32 != 4096) {
                testFail("WASM real loop across return lost state (early exit %u, iterations %u)", earlyExit, iterations);
            }
            DecodedOp* loop = context.memory->getDecodedOp(header);
            if (iterations == 2 && (!loop || loop->pfnJitCode)) {
                testFail("WASM real loop across return lost internal backedge (early exit %u)", earlyExit);
            }
        }
    }
}

void testWasmJitMultipleLoops() {
    for (U32 iterations : {2u, 65u, 130u}) {
        testNewInstruction(CF);
        auto& context = testContext();
        CPU* cpu = context.cpu;
        cpu->fpu.FINIT();
        U32 joins[3];
        U32 expected[3];
        U32 sum = 0;
        for (U32 i = 0; i < 3; ++i) {
            emitMov(1, iterations + i);
            U32 loop = context.codeIp;
            testPushCode8(0xf7); testPushCode8(0xc1); testPushCode32(1); // test ecx,1
            testPushCode8(0x74); U32 skip = context.codeIp; testPushCode8(0);
            testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(i + 1); // add eax,i+1
            testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc8); // movd xmm1,eax
            joins[i] = context.codeIp;
            testPushCode8(0x47); // inc edi
            testPushCode8(0x49); // dec ecx
            testPushCode8(0x75); testPushCode8((U8)(loop - (context.codeIp + 1)));
            context.memory->writeb(skip, (U8)(joins[i] - skip - 1));
            testPushCode8(0xa3); testPushCode32(0x210 + i * 4); // save each loop's result
            // Dirty x87 state between two integer loops must survive entry
            // into a new structured loop and its forward-branch joins.
            if (!i) { testPushCode8(0xd9); testPushCode8(0xe8); } // fld1
            sum += ((iterations + i + 1) / 2) * (i + 1);
            expected[i] = sum;
        }
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x7e); testPushCode8(0xce); // movd esi,xmm1
        testPushCode8(0x9c); testPushCode8(0x5d); // pushfd; pop ebp
        testPushCode8(0xd9); testPushCode8(0x1d); testPushCode32(0x200); // fstp dword [0x200]
        testRunCPU();
        if (cpu->reg[0].u32 != sum || cpu->reg[6].u32 != sum ||
            cpu->reg[7].u32 != 3 * iterations + 3 || cpu->reg[1].u32 ||
            ((cpu->reg[5].u32 ^ (ZF | PF)) & (CF | PF | AF | ZF | SF | OF)) ||
            cpu->reg[4].u32 != 4096 || context.memory->readd(TEST_HEAP_ADDRESS + 0x200) != 0x3f800000) {
            testFail("WASM separate loops lost register/flag/FPU state (%u iterations)", iterations);
        }
        for (U32 i = 0; i < 3; ++i) {
            if (context.memory->readd(TEST_HEAP_ADDRESS + 0x210 + i * 4) != expected[i]) {
                testFail("WASM separate loop %u produced the wrong result (%u iterations)", i, iterations);
            }
            DecodedOp* target = context.memory->getDecodedOp(joins[i]);
            if (!target || target->pfnJitCode) {
                testFail("WASM separate loop %u returned to its forward target through the dispatcher (%u iterations)", i, iterations);
            }
        }
    }
}

void testWasmJitConditionalSideExits() {
    // A taken side exit and the continuing fall-through must both retain
    // dirty GP/XMM/x87 state and incoming flags, with a nonzero CS base.
    const U32 flagBits[] = {CF, PF, ZF, SF, OF};
    for (U32 condition = 0; condition < 16; ++condition) {
        for (bool taken : {false, true}) {
            U32 flags = 0;
            for (U32 pattern = 0; pattern < 32; ++pattern) {
                flags = AF | DF;
                for (U32 bit = 0; bit < 5; ++bit) {
                    if (pattern & (1u << bit)) flags |= flagBits[bit];
                }
                if (conditionExpected(condition, flags) == taken) break;
            }
            testNewInstruction(flags);
            auto& context = testContext();
            CPU* cpu = context.cpu;
            cpu->fpu.FINIT();
            emitMov(0, 0x12345678);
            testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc0); // movd xmm0,eax
            testPushCode8(0xd9); testPushCode8(0xe8); // fld1
            testPushCode8(0x0f); testPushCode8(0x80 + condition);
            testPushCode32(TEST_CODE_ADDRESS + 0x200 - context.codeIp - 4);
            auto finish = [&]() {
                testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x7e); testPushCode8(0xc6); // movd esi,xmm0
                testPushCode8(0xd9); testPushCode8(0x1d); testPushCode32(0x200); // fstp [200h]
                testPushCode8(0x9c); testPushCode8(0x5d); // pushfd; pop ebp
            };
            emitMov(3, 0x1111);
            finish();
            U32 fallthroughEnd = context.codeIp;
            testPushCode8(0xcd); testPushCode8(0x97);
            while (context.codeIp < TEST_CODE_ADDRESS + 0x200) testPushCode8(0x90);
            emitMov(3, 0x2222);
            finish();
            testPushCode8(0xcd); testPushCode8(0x97);
            // Cover both a cached target and a target first decoded on exit.
            if (condition & 1) cpu->getOp(TEST_CODE_ADDRESS + 0x200, 0);
            testRunCPU();
            if (cpu->reg[0].u32 != 0x12345678 || cpu->reg[6].u32 != 0x12345678 ||
                    cpu->reg[3].u32 != (taken ? 0x2222u : 0x1111u) ||
                    ((cpu->reg[5].u32 ^ flags) & (CF | PF | AF | ZF | SF | OF | DF)) ||
                    context.memory->readd(TEST_HEAP_ADDRESS + 0x200) != 0x3f800000 ||
                    cpu->reg[4].u32 != 4096) {
                testFail("WASM side exit condition %u taken %u lost guest state", condition, taken);
            }
            DecodedOp* entry = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
            if (!entry || !entry->pfnJitCode || entry->blockLen < fallthroughEnd - TEST_CODE_ADDRESS) {
                testFail("WASM side exit condition %u truncated its fall-through block", condition);
            }
        }
    }

    // Several cold exits used to split this loop into a chain of tiny blocks.
    // Exercise no exit, an immediate exit and exits across the loop budget.
    for (U32 exitAt : {0u, 130u, 65u, 1u}) {
        testNewInstruction(0);
        auto& context = testContext();
        CPU* cpu = context.cpu;
        cpu->reg[1].u32 = 130;
        testPushCode8(0x81); testPushCode8(0xf9); testPushCode32(exitAt); // cmp ecx,exitAt
        testPushCode8(0x0f); testPushCode8(0x84);
        testPushCode32(TEST_CODE_ADDRESS + 0x200 - context.codeIp - 4);
        testPushCode8(0x85); testPushCode8(0xc9); // test ecx,ecx
        testPushCode8(0x0f); testPushCode8(0x84);
        testPushCode32(TEST_CODE_ADDRESS + 0x200 - context.codeIp - 4);
        U32 body = context.codeIp;
        testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(3); // add eax,3
        testPushCode8(0x49); // dec ecx
        testPushCode8(0x75); testPushCode8((U8)(TEST_CODE_ADDRESS - context.codeIp - 1));
        U32 loopEnd = context.codeIp;
        testPushCode8(0xcd); testPushCode8(0x97);
        while (context.codeIp < TEST_CODE_ADDRESS + 0x200) testPushCode8(0x90);
        testPushCode8(0x9c); testPushCode8(0x5d); // side exit observes CMP/TEST flags
        testRunCPU();
        if (cpu->reg[0].u32 != (130 - exitAt) * 3 || cpu->reg[1].u32 != exitAt ||
                (exitAt && ((cpu->reg[5].u32 ^ (ZF | PF)) & (CF | PF | AF | ZF | SF | OF)))) {
            testFail("WASM conditional loop side exit at %u lost state", exitAt);
        }
        DecodedOp* entry = context.memory->getDecodedOp(TEST_CODE_ADDRESS);
        DecodedOp* bodyOp = context.memory->getDecodedOp(body);
        if (!entry || !entry->pfnJitCode || entry->blockLen < loopEnd - TEST_CODE_ADDRESS ||
                !bodyOp || bodyOp->pfnJitCode) {
            testFail("WASM conditional side exits fragmented the loop at %u", exitAt);
        }
    }

    for (bool taken : {false, true}) {
        testNewInstruction(0);
        auto& context = testContext();
        CPU* cpu = context.cpu;
        // An out-of-block backward Jcc must return to an independently
        // compiled entry, while its fall-through stays in the later block.
        emitMov(7, 0x2222);
        testPushCode8(0xcd); testPushCode8(0x97);
        cpu->getOp(TEST_CODE_ADDRESS, 0);
        while (context.codeIp < TEST_CODE_ADDRESS + 0x100) testPushCode8(0x90);
        cpu->eip.u32 = 0x100;
        cpu->reg[0].u32 = taken ? 0 : 1;
        testPushCode8(0x43); // inc ebx
        testPushCode8(0x85); testPushCode8(0xc0); // test eax,eax
        testPushCode8(0x0f); testPushCode8(0x84);
        testPushCode32(TEST_CODE_ADDRESS - context.codeIp - 4);
        emitMov(7, 0x1111);
        U32 fallthroughEnd = context.codeIp;
        testRunCPU();
        DecodedOp* entry = context.memory->getDecodedOp(TEST_CODE_ADDRESS + 0x100);
        if (cpu->reg[3].u32 != 1 || cpu->reg[7].u32 != (taken ? 0x2222u : 0x1111u) ||
                !entry || !entry->pfnJitCode || entry->blockLen < fallthroughEnd - TEST_CODE_ADDRESS - 0x100) {
            testFail("WASM backward side exit taken %u lost state or split fall-through", taken);
        }
    }
}

void testWasmJitSseCompareConditions() {
    struct CompareCase { U64 lhs, rhs; U32 flags; };
    const CompareCase doubles[] = {
        {0, 0, ZF}, {0x8000000000000000ULL, 0, ZF},
        {0x3ff0000000000000ULL, 0, 0}, {0, 0x3ff0000000000000ULL, CF},
        {0x7ff0000000000000ULL, 0x3ff0000000000000ULL, 0},
        {0x7ff8000000000000ULL, 0, CF | PF | ZF},
        {0, 0x7ff8000000000000ULL, CF | PF | ZF}
    };
    const CompareCase singles[] = {
        {0, 0, ZF}, {0x80000000, 0, ZF},
        {0x3f800000, 0, 0}, {0, 0x3f800000, CF},
        {0x7f800000, 0x3f800000, 0},
        {0x7fc00000, 0, CF | PF | ZF}, {0, 0x7fc00000, CF | PF | ZF}
    };
    for (bool isDouble : {false, true}) {
        for (U32 opcode : {0x2eu, 0x2fu}) {
            for (const auto& comparison : (isDouble ? doubles : singles)) {
                for (U32 condition = 0; condition < 16; ++condition) {
                    testNewInstruction(CF | PF | AF | ZF | SF | OF);
                    CPU* cpu = testContext().cpu;
                    cpu->xmm[0].pi.u64[0] = comparison.lhs;
                    cpu->xmm[1].pi.u64[0] = comparison.rhs;
                    if (isDouble) testPushCode8(0x66);
                    testPushCode8(0x0f); testPushCode8(opcode); testPushCode8(0xc1);
                    runConsumers(condition, comparison.flags, false);
                }
            }
        }
    }
}
#endif
