/*
 * Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "boxedwine.h"
#include "testCPU.h"
#include "ksignal.h"

#ifdef BOXEDWINE_WASM_JIT
namespace {
void sse(U8 prefix, U8 opcode, U8 modrm) {
    if (prefix) testPushCode8(prefix);
    testPushCode8(0x0f); testPushCode8(opcode); testPushCode8(modrm);
}

void sseMem(U8 prefix, U8 opcode, U8 reg, U32 address) {
    sse(prefix, opcode, (reg << 3) | 5);
    testPushCode32(address);
}

void checkXmm(U32 reg, U32 low, U32 upper) {
    const auto& xmm = testContext().cpu->xmm[reg].pi;
    if (xmm.u32[0] != low || xmm.u32[1] != upper ||
        xmm.u32[2] != upper || xmm.u32[3] != upper) {
        testFail("XMM%u cache result: %08x %08x %08x %08x, expected low %08x upper %08x",
            reg, xmm.u32[0], xmm.u32[1], xmm.u32[2], xmm.u32[3], low, upper);
    }
}

void initXmm() {
    for (U32 reg = 0; reg < 8; ++reg) {
        auto& xmm = testContext().cpu->xmm[reg].pi;
        xmm.u32[0] = 0x3f800000; // 1.0f
        for (U32 lane = 1; lane < 4; ++lane) xmm.u32[lane] = 0x12340000 + reg;
    }
}

void requireJit() {
    DecodedOp* entry = testContext().memory->getDecodedOp(TEST_CODE_ADDRESS);
    if (!entry || !entry->pfnJitCode) testFail("XMM cache test was not JIT compiled");
}
} // namespace

void testWasmJitXmmCache() {
    // Force both the inline memory path and the cross-page interpreter path.
    // Every register starts dirty; the loaded destination and untouched values
    // must agree after the paths rejoin. Scalar operations preserve upper lanes.
    for (U32 address : {0x100u, K_PAGE_SIZE - 2u}) {
        testNewInstruction(0);
        initXmm();
        testContext().memory->writed(TEST_HEAP_ADDRESS + address, 0x40400000); // 3.0f
        for (U32 reg = 0; reg < 8; ++reg) sse(0xf3, 0x58, 0xc0 | reg * 9); // addss xmmN,xmmN
        sseMem(0xf3, 0x59, 0, address); // mulss xmm0,[mem]: 6.0f
        sseMem(0xf3, 0x11, 0, address + 0x2000); // movss [mem],xmm0
        sseMem(0xf3, 0x10, 1, address); // movss xmm1,[mem]: zero upper lanes
        sse(0xf3, 0x58, 0xc1); // addss xmm0,xmm1: 9.0f
        sse(0xf3, 0x58, 0xd2); // addss xmm2,xmm2: 4.0f
        // A conditional interpreter fallback which is not taken must preserve
        // dirty XMM locals; this also checks branch metadata at the join.
        testPushCode8(0x9b); // fwait
        testPushCode8(0x9c); testPushCode8(0x58); // pushfd; pop eax
        testRunCPU();
        checkXmm(0, 0x41100000, 0x12340000);
        checkXmm(1, 0x40400000, 0);
        checkXmm(2, 0x40800000, 0x12340002);
        for (U32 reg = 3; reg < 8; ++reg) checkXmm(reg, 0x40000000, 0x12340000 + reg);
        if (testContext().memory->readd(TEST_HEAP_ADDRESS + address + 0x2000) != 0x40c00000)
            testFail("XMM cached store did not see preceding arithmetic");
        requireJit();
    }

    // FXSAVE consumes dirty locals; FXRSTOR replaces even clean cached values.
    testNewInstruction(0);
    initXmm();
    for (U32 reg = 0; reg < 8; ++reg) sse(0xf3, 0x58, 0xc0 | reg * 9);
    sseMem(0, 0xae, 0, 0x400); // fxsave [mem]
    for (U32 reg = 0; reg < 8; ++reg) sse(0, 0x57, 0xc0 | reg * 9); // xorps xmmN,xmmN
    sseMem(0, 0xae, 1, 0x400); // fxrstor [mem]
    for (U32 reg = 0; reg < 8; ++reg) sse(0xf3, 0x58, 0xc0 | reg * 9);
    testRunCPU();
    for (U32 reg = 0; reg < 8; ++reg) checkXmm(reg, 0x40800000, 0x12340000 + reg);
    requireJit();

    // A packed conversion with identical source/destination must read every
    // source lane before it overwrites that lane's cached register.
    testNewInstruction(0);
    for (U32 lane = 0; lane < 4; ++lane) testContext().cpu->xmm[0].pi.u32[lane] = 0x3f800000 + (lane << 23);
    sse(0xf3, 0x5b, 0xc0); // cvttps2dq xmm0,xmm0 => 1,2,4,8
    sse(0x66, 0x70, 0xc0); testPushCode8(0x1b); // pshufd xmm0,xmm0,reverse
    testRunCPU();
    for (U32 lane = 0; lane < 4; ++lane) {
        if (testContext().cpu->xmm[0].pi.u32[lane] != (8u >> lane))
            testFail("XMM cached conversion/shuffle alias lane %u", lane);
    }

    // Keep an accumulator live through direct backedges and dispatcher exits.
    testNewInstruction(0);
    initXmm();
    testPushCode8(0xb9); testPushCode32(1000); // mov ecx,1000
    sse(0xf3, 0x58, 0xc1); // addss xmm0,xmm1
    testPushCode8(0x49); // dec ecx
    testPushCode8(0x75); testPushCode8(0xf9); // jnz addss
    testRunCPU();
    checkXmm(0, 0x447a4000, 0x12340000); // 1001.0f
    requireJit();
}

void testWasmJitXmmFaultState() {
    for (bool scalar : {false, true}) {
        testNewInstruction(0);
        initXmm();
        auto& context = testContext();
        sse(0xf3, 0x58, 0xc0); // addss xmm0,xmm0: dirty 2.0f
        if (scalar) sseMem(0xf3, 0x58, 0, 0x2000); // addss xmm0,[inaccessible]
        else { testPushCode8(0xa1); testPushCode32(0x2000); } // mov eax,[inaccessible]
        sse(0xf3, 0x58, 0xc0); // must not execute after the fault
        auto oldAction = context.process->sigActions[K_SIGSEGV];
        auto& action = context.process->sigActions[K_SIGSEGV];
        action.reset();
        action.handlerAndSigAction = context.codeIp; // test-end appended by testRunCPU
        context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, 0);
        testRunCPU();
        context.memory->mprotect(context.thread, TEST_HEAP_ADDRESS + 0x2000, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE);
        if (action.sigInfo[0] != K_SIGSEGV) testFail("XMM cache test did not fault");
        checkXmm(0, 0x40000000, 0x12340000);
        context.process->sigActions[K_SIGSEGV] = oldAction;
    }
}
#endif
