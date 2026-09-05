/*
 * Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "boxedwine.h"
#include "testCPU.h"

#ifdef BOXEDWINE_WASM_JIT
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
