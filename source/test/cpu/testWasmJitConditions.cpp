/*
 * Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "boxedwine.h"
#include "testCPU.h"
#include "ksignal.h"

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


void testForwardedFlagResults() {
    // Exercise byte (including AH), word, and dword results with upper
    // register bits set. Check both the immediate consumer and saved EFLAGS.
    for (U32 width : {8u, 16u, 32u}) for (bool high : {false, true}) {
        if (high && width != 8) continue;
        const U32 mask = width == 32 ? 0xffffffffu : (1u << width) - 1;
        const U32 sign = 1u << (width - 1);
        for (U32 kind = 0; kind < 6; ++kind) for (U32 a : {0u, 1u, mask, sign, sign - 1}) {
            // ADD, SUB, TEST, INC, DEC, XOR, all with an operand of one.
            bool sub = kind == 1 || kind == 4;
            bool logic = kind == 2 || kind == 5;
            U32 value = logic ? (kind == 2 ? a & 1 : a ^ 1) : (sub ? a - 1 : a + 1);
            value &= mask;
            U32 flags = DF;
            if (!value) flags |= ZF;
            if (value & sign) flags |= SF;
            U32 parity = value & 0xff;
            parity ^= parity >> 4; parity ^= parity >> 2; parity ^= parity >> 1;
            if (!(parity & 1)) flags |= PF;
            if (!logic) {
                if ((a ^ 1 ^ value) & 0x10) flags |= AF;
                if (sub ? ((a ^ 1) & (a ^ value) & sign) : (~(a ^ 1) & (a ^ value) & sign)) flags |= OF;
                if (kind == 3 || kind == 4 || (sub ? a < 1 : a == mask)) flags |= CF;
            }
            const U32 initialEax = (0xa5b6c7d8u & ~(mask << (high ? 8 : 0))) | (a << (high ? 8 : 0));
            const U32 expectedEax = kind == 2 ? initialEax :
                (initialEax & ~(mask << (high ? 8 : 0))) | (value << (high ? 8 : 0));
            for (U32 cond : {4u, 5u, 8u, 9u, 6u, 7u, 12u, 13u, 14u, 15u}) for (U32 consumer = 0; consumer < 3; ++consumer) {
                testNewInstruction(CF | DF);
                CPU* cpu = testContext().cpu;
                cpu->reg[0].u32 = initialEax;
                cpu->reg[2].u32 = 0xabcdef00;
                cpu->reg[6].u32 = 1;
                if (width == 16) testPushCode8(0x66);
                if (kind == 3 || kind == 4) {
                    testPushCode8(width == 8 ? 0xfe : 0xff);
                    testPushCode8(0xc0 | (kind == 4 ? 8 : 0) | (high ? 4 : 0));
                } else {
                    testPushCode8(kind == 2 ? (width == 8 ? 0xf6 : 0xf7) : (width == 8 ? 0x80 : 0x81));
                    testPushCode8(0xc0 | (kind == 1 ? 5 << 3 : kind == 5 ? 6 << 3 : 0) | (high ? 4 : 0));
                    if (width == 8) testPushCode8(1);
                    else if (width == 16) testPushCode16(1);
                    else testPushCode32(1);
                }
                if (consumer == 0) {
                    testPushCode8(0x0f); testPushCode8(0x90 + cond); testPushCode8(0xc2); // setcc dl
                } else if (consumer == 1) {
                    testPushCode8(0x0f); testPushCode8(0x40 + cond); testPushCode8(0xfe); // cmovcc edi,esi
                } else {
                    testPushCode8(0x70 + cond); testPushCode8(5);
                    emitMov(7, 1);
                }
                testPushCode8(0x9c); testPushCode8(0x5d); // preserve all flags after the consumer
                testRunCPU();
                U32 taken = conditionExpected(cond, flags);
                U32 checkedFlags = CF | PF | ZF | SF | OF | DF | (logic ? 0 : AF);
                if (cpu->reg[0].u32 != expectedEax ||
                    cpu->reg[2].u32 != (0xabcdef00u | (consumer == 0 ? taken : 0)) ||
                    cpu->reg[7].u32 != (consumer == 0 ? 0 : consumer == 1 ? taken : !taken) ||
                    ((cpu->reg[5].u32 ^ flags) & checkedFlags)) {
                    testFail("WASM forwarded flags width=%u high=%u kind=%u a=%x cond=%u consumer=%u", width, high, kind, a, cond, consumer);
                }
            }
        }
    }
    // A join can bypass the immediately preceding producer. A zero-count
    // shift and an intervening helper must also keep using incoming flags.
    for (U32 mode = 0; mode < 4; ++mode) for (U32 take = 0; take < 2; ++take) {
        testNewInstruction(0);
        CPU* cpu = testContext().cpu;
        cpu->reg[0].u32 = take;
        testPushCode8(0x85); testPushCode8(0xc0); // test eax,eax
        if (mode == 0) {
            testPushCode8(0x74); testPushCode8(3); // bypass ADD when ZF=1
            testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(1);
        } else if (mode == 1) {
            testPushCode8(0xd3); testPushCode8(0xe0); // shl eax,cl (cl=0)
        } else if (mode == 2) {
            testPushCode8(0x9c); testPushCode8(0x5d); // materialize flags
        } else {
            emitIndirectBoundary();
        }
        testPushCode8(0x0f); testPushCode8(0x94); testPushCode8(0xc3); // setz bl
        testRunCPU();
        if (cpu->reg[3].u32 != !take) testFail("WASM flag forwarding boundary mode=%u take=%u", mode, take);
    }
}


void testInvariantLoopCarry() {
    for (U32 incoming = 0; incoming < 4; ++incoming) for (U32 carry : {0u, (U32)CF}) {
        for (U32 count : {1u, 2u, 63u, 64u, 65u, 130u}) for (U32 exitAt : {0u, 1u, count}) {
            testNewInstruction(carry | DF);
            auto& ctx = testContext(); CPU* cpu = ctx.cpu;
            if (incoming) {
                // Materialized CF deliberately disagrees with the lazy state.
                cpu->flags ^= CF;
                cpu->src.u32 = 1;
                if (incoming == 1) {
                    cpu->lazyFlagType = FLAGS_ADD32;
                    cpu->dst.u32 = carry ? 0xffffffffu : 1;
                    cpu->result.u32 = carry ? 0 : 2;
                } else if (incoming == 2) {
                    cpu->lazyFlagType = FLAGS_SUB32;
                    cpu->dst.u32 = carry ? 0 : 2;
                    cpu->result.u32 = carry ? 0xffffffffu : 1;
                } else {
                    cpu->lazyFlagType = FLAGS_DEC32;
                    cpu->oldCF = carry;
                    cpu->dst.u32 = 2; cpu->result.u32 = 1;
                }
            }
            cpu->reg[1].u32 = count;
            cpu->reg[2].u32 = exitAt;
            cpu->reg[6].u32 = 4093; // cross-page read helper preserves carry
            ctx.memory->writed(TEST_HEAP_ADDRESS + 4093, 0x12345678);
            for (U32 lane = 0; lane < 4; ++lane) {
                cpu->xmm[0].pi.u32[lane] = 0;
                cpu->xmm[1].pi.u32[lane] = lane + 1;
            }
            const U32 loop = ctx.codeIp;
            testPushCode8(0x8b); testPushCode8(0x1e); // mov ebx,[esi]
            testPushCode8(0x40); testPushCode8(0x4a); // inc eax; dec edx
            testPushCode8(0x74); U32 exit = ctx.codeIp; testPushCode8(0);
            testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0xfe); testPushCode8(0xc1);
            testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop - ctx.codeIp - 1));
            ctx.memory->writeb(exit, (U8)(ctx.codeIp - exit - 1));
            testPushCode8(0x9c); testPushCode8(0x5d); // pushfd; pop ebp
            testRunCPU();
            U32 executed = exitAt ? exitAt : count;
            U32 vectorIterations = exitAt ? executed - 1 : executed;
            if (cpu->reg[0].u32 != executed || cpu->reg[1].u32 != (exitAt ? count - executed + 1 : 0) ||
                cpu->reg[2].u32 != (exitAt ? 0 : 0u - count) || cpu->reg[3].u32 != 0x12345678 ||
                (cpu->reg[5].u32 & (CF | PF | AF | ZF | SF | OF | DF)) != (carry | PF | ZF | DF)) {
                testFail("WASM invariant loop carry incoming=%u carry=%u count=%u exit=%u", incoming, carry, count, exitAt);
            }
            for (U32 lane = 0; lane < 4; ++lane)
                if (cpu->xmm[0].pi.u32[lane] != vectorIterations * (lane + 1)) testFail("WASM invariant carry XMM state");
        }
    }
    // Consecutive loops must each initialize carry, including budget re-entry.
    for (U32 carry : {0u, (U32)CF}) for (U32 count : {1u, 65u, 130u}) {
        testNewInstruction(carry);
        auto& ctx = testContext(); CPU* cpu = ctx.cpu;
        emitMov(1, count);
        U32 loop = ctx.codeIp;
        testPushCode8(0x40); testPushCode8(0x49);
        testPushCode8(0x75); testPushCode8((U8)(loop - ctx.codeIp - 1));
        testPushCode8(carry ? 0xf8 : 0xf9); // CLC/STC changes the second loop's input
        emitMov(2, count);
        loop = ctx.codeIp;
        testPushCode8(0x40); testPushCode8(0x4a);
        testPushCode8(0x75); testPushCode8((U8)(loop - ctx.codeIp - 1));
        testRunCPU(); cpu->fillFlags();
        if (cpu->reg[0].u32 != 2 * count || cpu->reg[1].u32 || cpu->reg[2].u32 ||
            (cpu->flags & (CF | PF | AF | ZF | SF | OF)) != ((carry ^ CF) | PF | ZF))
            testFail("WASM disjoint loops reused stale carry");
    }
    // These instructions are allowed by register caching, but change CF.
    // Their loops must retain the regular carry calculation.
    for (U32 kind = 0; kind < 4; ++kind) for (U32 count : {1u, 65u, 130u}) {
        const U32 expectedCarry = kind < 2 ? CF : 0;
        testNewInstruction(expectedCarry ^ CF);
        auto& ctx = testContext(); CPU* cpu = ctx.cpu;
        cpu->reg[0].u32 = kind == 0 ? count - 1 : 0;
        cpu->reg[1].u32 = count;
        U32 loop = ctx.codeIp;
        if (kind == 3) {
            testPushCode8(0x85); testPushCode8(0xc0); // test eax,eax
        } else {
            testPushCode8(0x83);
            testPushCode8(kind == 0 ? 0xe8 : kind == 1 ? 0xf8 : 0xe0); // sub/cmp/and eax,1
            testPushCode8(1);
        }
        testPushCode8(0x47); testPushCode8(0x49);
        testPushCode8(0x75); testPushCode8((U8)(loop - ctx.codeIp - 1));
        testRunCPU(); cpu->fillFlags();
        if (cpu->reg[1].u32 || cpu->reg[7].u32 != count || (cpu->flags & CF) != expectedCarry)
            testFail("WASM carry-changing loop was treated as invariant kind=%u count=%u", kind, count);
    }
}

} // namespace

void testWasmJitMaterializedConditions() {
    testForwardedFlagResults();
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

void testWasmJitLoopRegisterState() {
    testInvariantLoopCarry();
    for (U32 count : {1u, 2u, 63u, 64u, 65u, 130u}) {
        for (U32 exitAt : {0u, count, count / 2}) {
            testNewInstruction(CF);
            auto& ctx = testContext();
            CPU* cpu = ctx.cpu;
            emitMov(1, count); emitMov(3, 5);
            testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc3); // movd xmm0,ebx
            U32 loop = ctx.codeIp;
            testPushCode8(0x81); testPushCode8(0xf9); testPushCode32(exitAt); // cmp ecx,exitAt
            testPushCode8(0x0f); testPushCode8(0x84); U32 sideExit = ctx.codeIp; testPushCode32(0);
            testPushCode8(0xf7); testPushCode8(0xc1); testPushCode32(1); // test ecx,1
            testPushCode8(0x74); U32 skip = ctx.codeIp; testPushCode8(0);
            testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(3); // add eax,3
            testPushCode8(0x89); testPushCode8(0xc3); // mov ebx,eax
            testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc3);
            ctx.memory->writeb(skip, (U8)(ctx.codeIp - skip - 1));
            testPushCode8(0x01); testPushCode8(0xda); // add edx,ebx
            testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x7e); testPushCode8(0xc7); // movd edi,xmm0
            testPushCode8(0x01); testPushCode8(0xfe); // add esi,edi
            testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop - ctx.codeIp - 1));
            ctx.memory->writed(sideExit, ctx.codeIp - sideExit - 4);
            testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x7e); testPushCode8(0xc5); // movd ebp,xmm0
            testRunCPU();
            cpu->fillFlags();
            if ((cpu->flags & (CF | PF | AF | ZF | SF | OF)) != (ZF | PF)) testFail("WASM loop exit flags");
            U32 eax = 0, ebx = 5, sum = 0, edi = 0;
            for (U32 ecx = count; ecx > exitAt; --ecx) {
                if (ecx & 1) ebx = (eax += 3);
                sum += ebx; edi = ebx;
            }
            if (cpu->reg[0].u32 != eax || cpu->reg[1].u32 != exitAt || cpu->reg[2].u32 != sum ||
                    cpu->reg[3].u32 != ebx || cpu->reg[4].u32 != 4096 || cpu->reg[5].u32 != ebx ||
                    cpu->reg[6].u32 != sum || cpu->reg[7].u32 != edi) {
                testFail("WASM loop carried registers count=%u exit=%u", count, exitAt);
            }
        }
    }
    // Carry all four lanes through a conditional join; untouched XMM
    // registers and loop inputs must survive repeated budget exits too.
    for (U32 count : {1u, 65u, 130u}) {
        testNewInstruction(0);
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        U32 expected[8][4];
        for (U32 reg = 0; reg < 8; ++reg) for (U32 lane = 0; lane < 4; ++lane) {
            expected[reg][lane] = 0x89abcdefu + reg * 0x1234567u + lane;
            cpu->xmm[reg].pi.u32[lane] = expected[reg][lane];
        }
        cpu->reg[1].u32 = count;
        const U32 loop = ctx.codeIp;
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6f); testPushCode8(0xd0); // movdqa xmm2,xmm0
        testPushCode8(0xf7); testPushCode8(0xc1); testPushCode32(1);
        testPushCode8(0x74); testPushCode8(8);
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0xfe); testPushCode8(0xc1); // paddd xmm0,xmm1
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0xef); testPushCode8(0xd3); // pxor xmm2,xmm3
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0xfe); testPushCode8(0xe2); // paddd xmm4,xmm2
        testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop - ctx.codeIp - 1));
        testRunCPU();
        for (U32 ecx = count; ecx; --ecx) for (U32 lane = 0; lane < 4; ++lane) {
            expected[2][lane] = expected[0][lane];
            if (ecx & 1) {
                expected[0][lane] += expected[1][lane];
                expected[2][lane] ^= expected[3][lane];
            }
            expected[4][lane] += expected[2][lane];
        }
        for (U32 reg = 0; reg < 8; ++reg) for (U32 lane = 0; lane < 4; ++lane) {
            if (cpu->xmm[reg].pi.u32[lane] != expected[reg][lane])
                testFail("WASM loop XMM state count=%u reg=%u lane=%u", count, reg, lane);
        }
        if (cpu->reg[1].u32) testFail("WASM XMM loop count");
    }

    // A conditional memory access may first execute on a later iteration.
    // Its GP inputs and segment bases must already be valid at the join.
    // Unaligned addresses exercise cross-page helpers; address-size 16 wraps
    // the effective offset while the full ESI/EDI values keep advancing.
    for (U32 mode : {0u, 1u, 2u}) for (U32 count : {1u, 2u, 65u, 130u}) {
        testNewInstruction(0);
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        bool ea16 = mode == 2;
        U32 ds = mode ? TEST_HEAP_ADDRESS : 0;
        U32 es = mode ? TEST_HEAP_ADDRESS + 0x10000 : 0;
        U32 src = ea16 ? 0xcafefffd : TEST_HEAP_ADDRESS + 0xffd - ds;
        U32 dst = ea16 ? 0xbeef0ffd : TEST_HEAP_ADDRESS + 0x10ffd - es;
        auto offset = [ea16](U32 value) { return ea16 ? value & 0xffff : value; };
        cpu->seg[DS].address = ds; cpu->seg[ES].address = es;
        ctx.process->hasSetSeg[DS] = ctx.process->hasSetSeg[ES] = mode != 0;
        cpu->reg[1].u32 = count; cpu->reg[3].u32 = 7;
        cpu->reg[6].u32 = src; cpu->reg[7].u32 = dst;
        for (U32 i = 0; i < count; ++i) {
            ctx.memory->writed(ds + offset(src + i * 4), i + 3);
            ctx.memory->writed(es + offset(dst + i * 4), 0xdeadbeef);
        }
        U32 loop = ctx.codeIp;
        testPushCode8(0xf7); testPushCode8(0xc1); testPushCode32(1); // test ecx,1
        testPushCode8(0x74); U32 skip = ctx.codeIp; testPushCode8(0);
        if (ea16) testPushCode8(0x67);
        testPushCode8(0x03); testPushCode8(ea16 ? 0x04 : 0x06); // add eax,[esi/si]
        testPushCode8(0x26); if (ea16) testPushCode8(0x67);
        testPushCode8(0x89); testPushCode8(ea16 ? 0x05 : 0x07); // mov es:[edi/di],eax
        testPushCode8(0x26); if (ea16) testPushCode8(0x67);
        testPushCode8(0x8b); testPushCode8(ea16 ? 0x1d : 0x1f); // mov ebx,es:[edi/di]
        ctx.memory->writeb(skip, (U8)(ctx.codeIp - skip - 1));
        testPushCode8(0x01); testPushCode8(0xda); // add edx,ebx
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc0);
        testPushCode8(0x83); testPushCode8(0xc6); testPushCode8(4);
        testPushCode8(0x83); testPushCode8(0xc7); testPushCode8(4);
        testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop - ctx.codeIp - 1));
        testRunCPU();
        U32 eax = 0, ebx = 7, sum = 0;
        for (U32 i = 0; i < count; ++i) {
            if ((count - i) & 1) ebx = (eax += i + 3);
            sum += ebx;
            U32 expected = ((count - i) & 1) ? eax : 0xdeadbeef;
            if (ctx.memory->readd(es + offset(dst + i * 4)) != expected)
                testFail("WASM cached memory loop data mode=%u count=%u offset=%u", mode, count, i);
        }
        cpu->fillFlags();
        if (cpu->reg[0].u32 != eax || cpu->reg[1].u32 || cpu->reg[2].u32 != sum ||
                cpu->reg[3].u32 != ebx || cpu->reg[6].u32 != src + count * 4 ||
                cpu->reg[7].u32 != dst + count * 4 || cpu->xmm[0].pi.u32[0] != eax ||
                (cpu->flags & (CF | PF | AF | ZF | SF | OF)) != (ZF | PF)) {
            testFail("WASM cached memory loop state mode=%u count=%u", mode, count);
        }
    }
    // Include both address registers when the memory operand uses a SIB.
    testNewInstruction(0);
    auto& ctx = testContext();
    ctx.cpu->reg[1].u32 = 65; ctx.cpu->reg[6].u32 = 0x100; ctx.cpu->reg[7].u32 = 3;
    for (U32 i = 0; i < 65; ++i) ctx.memory->writed(TEST_HEAP_ADDRESS + 0x10c + 4 * i, i);
    U32 loop = ctx.codeIp;
    testPushCode8(0x03); testPushCode8(0x04); testPushCode8(0xbe); // add eax,[esi+edi*4]
    testPushCode8(0x47); testPushCode8(0x49);
    testPushCode8(0x75); testPushCode8((U8)(loop - ctx.codeIp - 1));
    testRunCPU();
    if (ctx.cpu->reg[0].u32 != 2080 || ctx.cpu->reg[1].u32 || ctx.cpu->reg[7].u32 != 68)
        testFail("WASM cached memory loop SIB registers");

}

void testWasmJitLoopRegisterFaults() {
    // The fault is at the header, before this iteration has written any GP
    // or XMM register. State from the previous iteration must still be saved.
    for (bool invariant : {false, true}) for (U32 carry : {0u, (U32)CF}) for (U32 completed : {0u, 1u, 2u, 63u, 64u, 65u}) for (bool store : {false, true}) {
        testNewInstruction(carry);
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        const U32 page = TEST_HEAP_ADDRESS + 0x2000;
        cpu->reg[1].u32 = completed + 2;
        cpu->reg[6].u32 = 0x2000 - 4 * completed;
        cpu->reg[3].u32 = 0x12345678;
        cpu->xmm[0].pi.u32[0] = 0;
        for (U32 i = 0; i < completed; ++i) ctx.memory->writed(page - 4 * completed + i * 4, 0x12345678);
        const U32 loop = ctx.codeIp;
        testPushCode8(store ? 0x89 : 0x8b); testPushCode8(0x1e); // mov [esi],ebx / mov ebx,[esi]
        testPushCode8(invariant ? 0x8d : 0x83); testPushCode8(invariant ? 0x40 : 0xc0); testPushCode8(3);
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc0);
        testPushCode8(invariant ? 0x8d : 0x83); testPushCode8(invariant ? 0x76 : 0xc6); testPushCode8(4);
        testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop - ctx.codeIp - 1));
        auto oldAction = ctx.process->sigActions[K_SIGSEGV];
        auto& action = ctx.process->sigActions[K_SIGSEGV];
        action.reset(); action.flags = K_SA_SIGINFO; action.handlerAndSigAction = ctx.codeIp;
        testPushCode8(0xcd); testPushCode8(0x97);
        DecodedOp* entry = cpu->getNextOp();
        entry->runCount = JIT_RUN_COUNT + 1;
        startNewJIT(cpu, TEST_CODE_ADDRESS, entry);
        if (!entry->pfnJitCode) testFail("WASM loop fault case did not compile");
        ctx.memory->mprotect(ctx.thread, page, K_PAGE_SIZE, store ? K_PROT_READ : 0);
        testRunCPU();
        ctx.memory->mprotect(ctx.thread, page, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
        U32 uc = action.sigInfo[0] == K_SIGSEGV ? cpu->reg[2].u32 : 0;
        if (!uc || ctx.memory->readd(uc + 0x40) != completed * 3 ||
                ctx.memory->readd(uc + 0x3c) != 2 || ctx.memory->readd(uc + 0x28) != 0x2000 ||
                ctx.memory->readd(uc + 0x34) != 0x12345678 || ctx.memory->readd(uc + 0x4c) != 0 ||
                (ctx.memory->readd(uc + 0x54) & (CF | PF | AF | ZF | SF | OF)) != (invariant || !completed ? carry : 0u) ||
                cpu->xmm[0].pi.u32[0] != completed * 3) {
            testFail("WASM loop fault lost carried state completed=%u store=%u invariant=%u carry=%u", completed, store, invariant, carry);
        }
        ctx.process->sigActions[K_SIGSEGV] = oldAction;
    }
}

void testWasmJitLoopRegisterCodeWrite() {
    testNewInstruction(0);
    auto& ctx = testContext();
    CPU* cpu = ctx.cpu;
    cpu->reg[1].u32 = 5;
    const U32 loop = ctx.codeIp;
    testPushCode8(0x47); // inc edi
    testPushCode8(0x83); testPushCode8(0xff); testPushCode8(3); // cmp edi,3
    testPushCode8(0x75); U32 skip = ctx.codeIp; testPushCode8(0);
    testPushCode8(0xc7); testPushCode8(0x05); U32 address = ctx.codeIp; testPushCode32(0);
    testPushCode32(3); // mov dword [add immediate],3
    ctx.memory->writeb(skip, (U8)(ctx.codeIp - skip - 1));
    testPushCode8(0x81); testPushCode8(0xc0); U32 immediate = ctx.codeIp; testPushCode32(1);
    ctx.memory->writed(address, immediate - cpu->seg[DS].address);
    testPushCode8(0x49); testPushCode8(0x75); testPushCode8((U8)(loop - ctx.codeIp - 1));
    testRunCPU();
    if (cpu->reg[0].u32 != 11 || cpu->reg[1].u32 || cpu->reg[7].u32 != 5) {
        testFail("WASM loop cached registers survived into stale self-modified code");
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

static void testWasmReadGroups() {
    extern void startNewJIT(CPU*, U32, DecodedOp*);
    auto compile = [&]() {
        auto& ctx = testContext();
        DecodedOp* op = ctx.cpu->getOp(TEST_CODE_ADDRESS, 0);
        op->runCount = JIT_RUN_COUNT + 1;
        startNewJIT(ctx.cpu, TEST_CODE_ADDRESS, op);
        if (!op->pfnJitCode) testFail("WASM read group did not compile");
    };
    auto load = [](U32 reg, S32 displacement, bool add = false) {
        testPushCode8(add ? 0x03 : 0x8b);
        testPushCode8(0x86 + reg * 8);
        testPushCode32((U32)displacement);
    };
    const U32 regs[] = {0, 3, 2, 7};
    // Ascending, descending and repeated offsets; spans starting/ending at
    // page boundaries, unaligned operands, more than one bounded group.
    for (bool add : {false, true}) for (U32 count : {1u,2u,4u,8u,9u})
    for (U32 base : {0x100u,0x101u,0x1ffcu,0x1ffeu,0x2000u,0x2001u})
    for (S32 stride : {-4,0,4}) {
        testNewInstruction(CF | DF | OF);
        auto& ctx = testContext(); CPU* cpu = ctx.cpu;
        cpu->reg[6].u32 = base;
        for (U32 n = 0; n < count; ++n)
            ctx.memory->writed(TEST_HEAP_ADDRESS + base + n * stride, 0x80000000u + n + 1);
        U32 expected[8] = {};
        for (U32 r = 0; r < 8; ++r) expected[r] = cpu->reg[r].u32;
        U32 lhs = 0, rhs = 0;
        for (U32 n = 0; n < count; ++n) {
            U32 reg = add ? 0 : regs[n % 4];
            load(reg, n * stride, add);
            rhs = ctx.memory->readd(TEST_HEAP_ADDRESS + base + n * stride);
            lhs = expected[reg]; expected[reg] = add ? lhs + rhs : rhs;
        }
        testPushCode8(0xcd); testPushCode8(0x97);
        compile(); testRunCPU();
        for (U32 reg : {0u,2u,3u,6u,7u})
            if (cpu->reg[reg].u32 != expected[reg]) testFail("WASM read group values add=%u count=%u base=%x stride=%d reg=%u",add,count,base,stride,reg);
        cpu->fillFlags();
        if (!add) {
            if ((cpu->flags & (CF|DF|OF|SF|ZF|PF|AF)) != (CF|DF|OF)) testFail("WASM grouped MOV changed flags");
        } else {
            U32 result = lhs + rhs;
            U32 flags = DF | (result < lhs ? CF : 0) | (!result ? ZF : 0) | (result & 0x80000000 ? SF : 0) |
                (((~(lhs ^ rhs) & (lhs ^ result)) >> 31) ? OF : 0) | ((lhs ^ rhs ^ result) & AF);
            U32 parity = result & 0xff; parity ^= parity >> 4; parity ^= parity >> 2; parity ^= parity >> 1;
            if (!(parity & 1)) flags |= PF;
            if ((cpu->flags & (CF|DF|OF|SF|ZF|PF|AF)) != flags) testFail("WASM grouped ADD changed flags");
        }
    }
    // The cached pointer belongs to one straight-line execution, not to a
    // loop activation. Address changes and the 64-iteration budget must
    // refresh it before the next group of reads.
    for (U32 count : {1u,2u,64u,65u,130u}) for (U32 base : {0x100u,0x101u,0x1ffcu}) {
        testNewInstruction(CF);
        auto& ctx = testContext(); CPU* cpu = ctx.cpu;
        cpu->reg[1].u32 = count; cpu->reg[6].u32 = base;
        for (U32 bank = 0; bank < 2; ++bank) {
            U32 address = TEST_HEAP_ADDRESS + (base ^ (bank ? 0x40 : 0));
            for (U32 n = 0; n < 4; ++n) ctx.memory->writed(address + n * 4, bank ? 11 + n : 3 + n);
        }
        U32 start = ctx.codeIp;
        for (U32 n = 0; n < 4; ++n) load(0, n * 4, true);
        testPushCode8(0x83); testPushCode8(0xf6); testPushCode8(0x40); // xor esi,40h
        testPushCode8(0x49); testPushCode8(0x75);
        testPushCode8((U8)(start - ctx.codeIp - 1));
        testRunCPU();
        if (cpu->reg[0].u32 != ((count + 1) / 2) * 18 + (count / 2) * 50 ||
                cpu->reg[1].u32 || cpu->reg[6].u32 != (base ^ ((count & 1) ? 0x40 : 0)))
            testFail("WASM read group loop refresh count=%u base=%x", count, base);
    }
    // 16-bit addressing wraps before adding the segment; those operands
    // deliberately retain their existing individual-access path.
    testNewInstruction(0); {
        auto& ctx = testContext(); CPU* cpu = ctx.cpu;
        cpu->reg[6].u32 = 0x1234fffcu;
        ctx.memory->writed(TEST_HEAP_ADDRESS + 0xfffc, 111);
        ctx.memory->writed(TEST_HEAP_ADDRESS, 222);
        testPushCode8(0x67); testPushCode8(0x8b); testPushCode8(0x04); // mov eax,[si]
        testPushCode8(0x67); testPushCode8(0x8b); testPushCode8(0x5c); testPushCode8(4);
        testRunCPU();
        if (cpu->reg[0].u32 != 111 || cpu->reg[3].u32 != 222 || cpu->reg[6].u32 != 0x1234fffcu)
            testFail("WASM read group address-size fallback");
    }
    // Do not group across a write to the base or scaled index register.
    for (bool indexed : {false,true}) {
        testNewInstruction(0);auto& ctx=testContext();CPU* cpu=ctx.cpu;
        cpu->reg[6].u32=0x100;cpu->reg[1].u32=0;
        ctx.memory->writed(TEST_HEAP_ADDRESS+0x100,indexed?4:0x200);
        ctx.memory->writed(TEST_HEAP_ADDRESS+(indexed?0x114:0x204),1234);
        if(indexed){
            testPushCode8(0x8b);testPushCode8(0x8c);testPushCode8(0x8e);testPushCode32(0);
            testPushCode8(0x8b);testPushCode8(0x84);testPushCode8(0x8e);testPushCode32(4);
        }else{load(6,0);load(0,4);}
        testRunCPU();if(cpu->reg[0].u32!=1234)testFail("WASM read group reused changed address base");
    }
    // An internal edge into the second load must not reuse an uninitialized
    // pointer. Both taken and fallthrough paths feed the same join.
    for(bool taken:{false,true}){
        testNewInstruction(taken?ZF:0);auto& ctx=testContext();CPU* cpu=ctx.cpu;
        cpu->reg[6].u32=0x100;
        for (U32 n = 0; n < 5; ++n) ctx.memory->writed(TEST_HEAP_ADDRESS + 0x100 + n * 4, (n + 1) * 11);
        testPushCode8(0x74); testPushCode8(6);
        load(0,0); load(3,4); load(2,8); load(7,12); load(5,16);
        testRunCPU();
        if (cpu->reg[0].u32 != (taken ? 0u : 11u) || cpu->reg[3].u32 != 22 ||
                cpu->reg[2].u32 != 33 || cpu->reg[7].u32 != 44 || cpu->reg[5].u32 != 55)
            testFail("WASM read group interior join");
    }
    // Re-evaluate the mapping on each activation. A remap must be observed
    // even when the generated function and guest address are unchanged.
    testNewInstruction(0);{
        auto& ctx=testContext();CPU* cpu=ctx.cpu;cpu->reg[6].u32=0x100;
        for (U32 n = 0; n < 4; ++n) load(regs[n], n * 4);
        testPushCode8(0xcd);testPushCode8(0x97);compile();
        DecodedOp* entry=ctx.memory->getDecodedOp(TEST_CODE_ADDRESS);
        for(U32 value:{11u,22u}){
            ctx.memory->mmap(ctx.thread,TEST_HEAP_ADDRESS,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE,K_MAP_FIXED|K_MAP_PRIVATE,-1,0);
            for (U32 n = 0; n < 4; ++n) ctx.memory->writed(TEST_HEAP_ADDRESS + 0x100 + n * 4, value + n);
            cpu->eip.u32=0;cpu->nextOp=entry;testRunCPU();
            for (U32 n = 0; n < 4; ++n) if (cpu->reg[regs[n]].u32 != value + n) testFail("WASM read group reused stale mapping");
        }
    }
    // The guard may inspect a future inaccessible page, but must never
    // fault early or publish a later load's result. Validate exact signal
    // EIP, all preceding destinations, dirty SIMD prefix and incoming flags.
    for(U32 faultAt:{0u,1u,2u,3u})for(bool cross:{false,true})for(bool descending:{false,true}){
        testNewInstruction(CF|DF|OF);auto& ctx=testContext();CPU* cpu=ctx.cpu;
        U32 end=TEST_CODE_ADDRESS+0x1000;
        U32 first=cross?0x1ffe - faultAt*4:0x2000-faultAt*4;
        if(descending)first=0x2000+faultAt*4;
        cpu->reg[6].u32=first;
        for(U32 n=0;n<4;++n){cpu->reg[regs[n]].u32=0x11220000+n;ctx.memory->writed(TEST_HEAP_ADDRESS+first+(descending?-4:4)*n,n+101);}
        emitMov(0,0x11220000);testPushCode8(0x66);testPushCode8(0x0f);testPushCode8(0x6e);testPushCode8(0xc0);
        U32 fault=0;
        for(U32 n=0;n<4;++n){if(n==faultAt)fault=ctx.codeIp;load(regs[n],(descending?-4:4)*n);}
        testPushCode8(0xe9);testPushCode32(end-ctx.codeIp-4);ctx.codeIp=end;
        auto old=ctx.process->sigActions[K_SIGSEGV];auto& action=ctx.process->sigActions[K_SIGSEGV];
        action.reset();action.flags=K_SA_SIGINFO;action.handlerAndSigAction=end;
        compile();
        U32 denied=descending?0x2000:0x2000;
        // Descending cases fault on the final page below the earlier reads.
        if(descending)denied=0x1000;
        if(descending){ // arrange the faulting address just below 0x2000
            cpu->reg[6].u32=0x1ffc+faultAt*4;
            for(U32 n=0;n<4;++n)ctx.memory->writed(TEST_HEAP_ADDRESS+cpu->reg[6].u32-4*n,n+101);
        }
        ctx.memory->mprotect(ctx.thread,TEST_HEAP_ADDRESS+denied,K_PAGE_SIZE,0);
        testRunCPU();ctx.memory->mprotect(ctx.thread,TEST_HEAP_ADDRESS+denied,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE);
        U32 uc=action.sigInfo[0]==K_SIGSEGV?cpu->reg[2].u32:0;
        const U32 offsets[]={0x40,0x34,0x38,0x24};
        if(!uc||ctx.memory->readd(uc+0x4c)!=fault-TEST_CODE_ADDRESS||cpu->xmm[0].pi.u32[0]!=0x11220000)testFail("WASM read group fault location at=%u cross=%u descending=%u",faultAt,cross,descending);
        for(U32 n=0;n<4;++n)if(ctx.memory->readd(uc+offsets[n])!=(n<faultAt?n+101:0x11220000+n))testFail("WASM read group fault register at=%u reg=%u",faultAt,n);
        if((ctx.memory->readd(uc+0x54)&(CF|DF|OF|SF|ZF|PF|AF))!=(CF|DF|OF))testFail("WASM read group fault flags");
        ctx.process->sigActions[K_SIGSEGV]=old;
    }
}

static void testWasmForwardRegionState() {
    auto nearJump = [](U32 target) {
        testPushCode8(0xe9); testPushCode32(target - testContext().codeIp - 4);
    };
    auto branch = [](U8 condition) {
        testPushCode8(0x0f); testPushCode8(condition);
        U32 patch = testContext().codeIp; testPushCode32(0); return patch;
    };
    auto patch = [](U32 at, U32 target) { testContext().memory->writed(at, target - at - 4); };
    auto testMask = [](U32 mask) { testPushCode8(0xf7); testPushCode8(0xc3); testPushCode32(mask); };
    auto movXmm = []() { testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x6e); testPushCode8(0xc0); };
    auto compileAt = [](U32 address) {
        auto& ctx = testContext(); DecodedOp* op = ctx.cpu->getOp(address, 0);
        op->runCount = JIT_RUN_COUNT + 1; startNewJIT(ctx.cpu, address, op);
        if (!op->pfnJitCode) testFail("WASM forward region did not compile");
    };
    // Preserve flags and register values while adding a distinct join, so
    // memory/fault cases meet the four-forward-branch region threshold.
    auto extraJoin = [&]() {
        U32 target = branch(0x84);
        testPushCode8(0x90);
        patch(target, testContext().codeIp);
    };
    const U32 end = TEST_CODE_ADDRESS + 0x10000;
    // Four distinct forward joins exercise the retained register-only path.
    // All branch combinations include a conditional external exit. Values
    // unchanged on one incoming path must survive other paths' dirty masks.
    for (U32 mask = 0; mask < 32; ++mask) {
        testNewInstruction(CF);
        auto& ctx = testContext();
        CPU* cpu = ctx.cpu;
        auto seed = [&](U32 bits, U32 eax, U32 xmm) {
            cpu->reg[0].u32 = eax;
            cpu->reg[3].u32 = bits;
            cpu->reg[5].u32 = 100;
            cpu->reg[6].u32 = 20;
            cpu->reg[7].u32 = 30;
            for (U32 lane = 0; lane < 4; ++lane) {
                cpu->xmm[0].pi.u32[lane] = xmm + lane;
                cpu->xmm[1].pi.u32[lane] = 2 + lane;
            }
        };
        seed(mask, 10, 11);
        testMask(1); U32 first = branch(0x84);
        testPushCode8(0x83); testPushCode8(0xc0);
        U32 increment = ctx.codeIp; testPushCode8(3); // add eax,3
        U32 interior = ctx.codeIp; patch(first, interior);
        testMask(2); U32 second = branch(0x84);
        movXmm(); // movd xmm0,eax
        patch(second, ctx.codeIp);
        testMask(4); U32 third = branch(0x84);
        testPushCode8(0x01); testPushCode8(0xc6); // add esi,eax
        patch(third, ctx.codeIp);
        testMask(8); U32 fourth = branch(0x84);
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0xfe); testPushCode8(0xc1);
        patch(fourth, ctx.codeIp);
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x7e); testPushCode8(0xc2);
        testPushCode8(0x01); testPushCode8(0xf7); // add edi,esi
        testMask(16); U32 sideExit = branch(0x85); patch(sideExit, end);
        testPushCode8(0x45); // inc ebp only on the continuing path
        nearJump(end); ctx.codeIp = end;
        compileAt(TEST_CODE_ADDRESS); testRunCPU(); cpu->fillFlags();
        const U32 eax = 10 + ((mask & 1) ? 3 : 0);
        const U32 esi = 20 + ((mask & 4) ? eax : 0);
        const U32 xmm = ((mask & 2) ? eax : 11) + ((mask & 8) ? 2 : 0);
        if (cpu->reg[0].u32 != eax || cpu->reg[2].u32 != xmm || cpu->reg[3].u32 != mask ||
            cpu->reg[5].u32 != ((mask & 16) ? 100u : 101u) ||
            cpu->reg[6].u32 != esi || cpu->reg[7].u32 != 30 + esi ||
            (cpu->flags & (CF | OF | SF | ZF | PF)) != ((mask & 16) ? 0u : PF))
            testFail("WASM forward register-only join/exit state mask=%u", mask);
        for (U32 lane = 0; lane < 4; ++lane) {
            U32 expected = (mask & 2) ? (lane ? 0 : eax) : 11 + lane;
            if (mask & 8) expected += 2 + lane;
            if (cpu->xmm[0].pi.u32[lane] != expected)
                testFail("WASM forward register-only SIMD state mask=%u lane=%u", mask, lane);
        }
        seed(14, 100, 50);
        cpu->eip.u32 = interior - TEST_CODE_ADDRESS; cpu->nextOp = nullptr;
        compileAt(interior); testRunCPU();
        if (cpu->reg[0].u32 != 100 || cpu->reg[2].u32 != 102 || cpu->reg[7].u32 != 150)
            testFail("WASM forward register-only interior entry state");
        ctx.memory->writeb(increment, 5);
        seed(31, 10, 11); cpu->eip.u32 = 0; cpu->nextOp = nullptr;
        compileAt(TEST_CODE_ADDRESS); testRunCPU();
        if (cpu->reg[0].u32 != 15 || cpu->reg[2].u32 != 17 || cpu->reg[7].u32 != 65)
            testFail("WASM forward register-only invalidation state");
    }

    // Memory regions retain locals but publish at guest branches and joins.
    // The root is acyclic. Both paths consume GP/XMM values at each join,
    // including inputs that the untaken path never writes. The memory case
    // crosses page boundaries and uses the nonzero test DS segment.
    for (U32 span : {1u, 2u, 3u}) for (U32 mask = 0; mask < 8; ++mask) for (bool crossPage : {false, true}) {
        testNewInstruction(CF);
        auto& ctx = testContext(); CPU* cpu = ctx.cpu;
        const U32 src = crossPage ? 0xffe : 0x100, dst = crossPage ? 0x2ffe : 0x200;
        auto seed = [&](U32 bits, U32 eax, U32 xmm) {
            cpu->reg[0].u32 = eax; cpu->reg[3].u32 = bits;
            cpu->reg[6].u32 = src; cpu->reg[7].u32 = dst;
            for (U32 lane = 0; lane < 4; ++lane) {
                cpu->xmm[0].pi.u32[lane] = xmm + lane;
                cpu->xmm[1].pi.u32[lane] = 2 + lane;
            }
        };
        seed(mask, 10, 11);
        ctx.memory->writed(TEST_HEAP_ADDRESS + src, 7);
        ctx.memory->writed(TEST_HEAP_ADDRESS + dst, 0xdeadbeef);
        testMask(1); U32 first = branch(0x84);
        testPushCode8(0x83); testPushCode8(0xc0); U32 increment = ctx.codeIp; testPushCode8(3);
        movXmm();
        U32 interior = ctx.codeIp; patch(first, interior);
        testMask(2); U32 second = branch(0x84);
        testPushCode8(0x03); testPushCode8(0x06); // add eax,[esi]
        if (span == 1) extraJoin(); // Original one-operation spans.
        if (span == 3) { testPushCode8(0x03); testPushCode8(0x06); } // a second load before the store
        testPushCode8(0x89); testPushCode8(0x07); // mov [edi],eax
        patch(second, ctx.codeIp);
        testMask(4); U32 third = branch(0x84);
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0xfe); testPushCode8(0xc1);
        patch(third, ctx.codeIp);
        extraJoin();
        testPushCode8(0x66); testPushCode8(0x0f); testPushCode8(0x7e); testPushCode8(0xc2);
        nearJump(end); ctx.codeIp = end;
        compileAt(TEST_CODE_ADDRESS); testRunCPU(); cpu->fillFlags();
        U32 eax = 10 + ((mask & 1) ? 3 : 0) + ((mask & 2) ? (span == 3 ? 14 : 7) : 0);
        U32 xmm = ((mask & 1) ? 13 : 11) + ((mask & 4) ? 2 : 0);
        if (cpu->reg[0].u32 != eax || cpu->reg[2].u32 != xmm ||
            cpu->reg[6].u32 != src || cpu->reg[7].u32 != dst ||
            ctx.memory->readd(TEST_HEAP_ADDRESS + dst) != ((mask & 2) ? eax : 0xdeadbeef) ||
            (cpu->flags & (CF | OF | SF | ZF | PF)) != ((mask & 4) ? 0u : (ZF | PF)))
            testFail("WASM forward region join state mask=%u cross=%u", mask, crossPage);
        for (U32 lane = 0; lane < 4; ++lane) {
            U32 expected = (mask & 1) ? (lane ? 0 : 13) : 11 + lane;
            if (mask & 4) expected += 2 + lane;
            if (cpu->xmm[0].pi.u32[lane] != expected) testFail("WASM forward region SIMD lanes");
        }
        // An independently entered interior must initialize from current CPU
        // state rather than inheriting locals from the original function.
        seed(6, 100, 50); cpu->eip.u32 = interior - TEST_CODE_ADDRESS; cpu->nextOp = nullptr;
        compileAt(interior); testRunCPU();
        if (cpu->reg[0].u32 != (span == 3 ? 114u : 107u) || cpu->reg[2].u32 != 52)
            testFail("WASM forward region interior entry reused stale locals");
        // Changing the parent retires overlapping interior publications too.
        ctx.memory->writeb(increment, 5);
        seed(1, 10, 11); cpu->eip.u32 = 0; cpu->nextOp = nullptr;
        compileAt(TEST_CODE_ADDRESS); testRunCPU();
        if (cpu->reg[0].u32 != 15 || cpu->reg[2].u32 != 15)
            testFail("WASM forward region invalidation reused stale code");
    }
    // A conditional external exit must publish values written before it,
    // without applying writes that occur only on the continuing path.
    for (U32 mask = 0; mask < 4; ++mask) {
        testNewInstruction(0); auto& ctx = testContext(); CPU* cpu = ctx.cpu;
        cpu->reg[0].u32 = 10; cpu->reg[3].u32 = mask; cpu->reg[6].u32 = 20;
        cpu->xmm[0].pi.u32[0] = 11;
        testMask(1); U32 skip = branch(0x84);
        testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(3); movXmm();
        patch(skip, ctx.codeIp);
        testMask(2); U32 sideExit = branch(0x85); patch(sideExit, end);
        testPushCode8(0x46); nearJump(end); ctx.codeIp = end;
        compileAt(TEST_CODE_ADDRESS); testRunCPU();
        if (cpu->reg[0].u32 != ((mask & 1) ? 13u : 10u) ||
            cpu->reg[6].u32 != ((mask & 2) ? 20u : 21u) ||
            cpu->xmm[0].pi.u32[0] != ((mask & 1) ? 13u : 11u))
            testFail("WASM forward region conditional exit state");
    }
    // Fault before a potential write to EBP, with dirty or untouched values
    // arriving at the preceding join. Validate the saved guest signal frame.
    for (U32 prefix : {0u,1u,2u}) for (bool prefixStore : {false,true})
    for (U32 mask : {2u,3u}) for (bool store : {false,true}) for (bool crossPage : {false,true}) {
        testNewInstruction(0); auto& ctx = testContext(); CPU* cpu = ctx.cpu;
        cpu->reg[0].u32 = 10; cpu->reg[3].u32 = mask; cpu->reg[5].u32 = 0x12345678;
        cpu->reg[6].u32 = crossPage ? 0x1ffe : 0x2000;
        cpu->reg[7].u32 = crossPage ? 0x4ffe : 0x4000;
        ctx.memory->writed(TEST_HEAP_ADDRESS + cpu->reg[7].u32, 3);
        cpu->xmm[0].pi.u32[0] = 11;
        extraJoin(); extraJoin();
        testMask(1); U32 first = branch(0x84);
        testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(3); movXmm();
        patch(first,ctx.codeIp);
        testMask(2); U32 second=branch(0x84);
        // These writes happen after the last guest-edge publication. The
        // faulting helper must still publish them, with EBP unmodified.
        testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(5); movXmm();
        // Successful memory operations precede the fault in the same span.
        // Cross-page accesses also take a helper before the later fault;
        // dirty GP/SIMD state must still be published at that later point.
        for (U32 i=0; i<prefix; ++i) {
            if (prefixStore) {
                testPushCode8(0x83); testPushCode8(0xc0); testPushCode8(3);
                testPushCode8(0x89); testPushCode8(0x07); // mov [edi],eax
            } else { testPushCode8(0x03); testPushCode8(0x07); } // add eax,[edi]
            movXmm();
        }
        const U32 expected = ((mask&1) ? 18u : 15u) + 3*prefix;
        U32 fault=ctx.codeIp;
        testPushCode8(store ? 0x89 : 0x8b); testPushCode8(store ? 0x06 : 0x2e);
        patch(second,ctx.codeIp); nearJump(end); ctx.codeIp=end;
        auto oldAction=ctx.process->sigActions[K_SIGSEGV]; auto& action=ctx.process->sigActions[K_SIGSEGV];
        action.reset();action.flags=K_SA_SIGINFO;action.handlerAndSigAction=end;
        compileAt(TEST_CODE_ADDRESS);
        ctx.memory->mprotect(ctx.thread,TEST_HEAP_ADDRESS+0x2000,K_PAGE_SIZE,store?K_PROT_READ:0);
        testRunCPU();
        ctx.memory->mprotect(ctx.thread,TEST_HEAP_ADDRESS+0x2000,K_PAGE_SIZE,K_PROT_READ|K_PROT_WRITE);
        U32 uc=action.sigInfo[0]==K_SIGSEGV?cpu->reg[2].u32:0;
        if (!uc || ctx.memory->readd(uc+0x40)!=expected ||
            ctx.memory->readd(uc+0x2c)!=0x12345678 || ctx.memory->readd(uc+0x4c)!=fault-TEST_CODE_ADDRESS ||
            cpu->xmm[0].pi.u32[0]!=expected ||
            ctx.memory->readd(TEST_HEAP_ADDRESS + (crossPage ? 0x4ffe : 0x4000)) !=
                ((prefixStore && prefix) ? expected : 3u))
            testFail("WASM forward region fault state mask=%u store=%u cross=%u prefix=%u writes=%u",mask,store,crossPage,prefix,prefixStore);
        ctx.process->sigActions[K_SIGSEGV]=oldAction;
    }
    // A store to this region's own code must exit before using its old
    // decoded immediate, preserving registers already changed in locals.
    testNewInstruction(0); auto& ctx=testContext(); CPU* cpu=ctx.cpu;
    cpu->reg[0].u32=3;cpu->reg[3].u32=1;
    extraJoin(); extraJoin(); extraJoin();
    testMask(1);U32 skip=branch(0x84); movXmm();
    testPushCode8(0xa3);U32 address=ctx.codeIp;testPushCode32(0);
    patch(skip,ctx.codeIp);
    testPushCode8(0x81);testPushCode8(0xc2);U32 immediate=ctx.codeIp;testPushCode32(1);
    ctx.memory->writed(address,immediate-cpu->seg[DS].address);
    nearJump(end);ctx.codeIp=end;
    compileAt(TEST_CODE_ADDRESS);testRunCPU();
    if(cpu->reg[2].u32!=3||cpu->xmm[0].pi.u32[0]!=3)testFail("WASM forward region self-modifying code state");
}


void testWasmJitForwardBranches() {
    testWasmReadGroups();
    testWasmForwardRegionState();
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
