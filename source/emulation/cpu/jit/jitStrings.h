/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "../normal/normal_strings.h"

void Jit::movs(U32 base, JitWidth valueWidth, U32 size, JitWidth regWidth) {
    // U32 dBase = cpu->seg[ES].address;
    // U32 sBase = cpu->seg[base].address;
    // S32 inc = cpu->getDirection() << 1;
    // cpu->memory->writew(dBase + DI, cpu->memory->readw(sBase + SI));
    // DI += inc;
    // SI += inc;
    RegPtr esi = getStringRegEsi();
    RegPtr edi = getStringRegEdi();

    if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[base]) {
        RegPtr readAddress = getTmpSegAddress(base);

        if (regWidth == JitWidth::b32) {
            addReg(JitWidth::b32, readAddress, esi);
        } else {
            RegPtr si = getTmpReg();
            xorReg(JitWidth::b32, si, si);
            mov(regWidth, si, esi);
            addReg(JitWidth::b32, readAddress, si);
        }
        RegPtr value = read(valueWidth, std::move(readAddress));

        RegPtr writeAddress = getTmpSegAddress(ES);
        if (regWidth == JitWidth::b32) {
            addReg(JitWidth::b32, writeAddress, edi);
        } else {
            RegPtr di = getTmpReg();
            xorReg(JitWidth::b32, di, di);
            mov(regWidth, di, edi);
            addReg(JitWidth::b32, writeAddress, di);
        }
        write(valueWidth, std::move(writeAddress), value);
    } else {
        write(valueWidth, edi, read(valueWidth, esi));
    }
    IfDF(); {
        subValue(regWidth, esi, size);
        subValue(regWidth, edi, size);
    } StartElse(); {
        addValue(regWidth, esi, size);
        addValue(regWidth, edi, size);
    } EndIf();
}

static constexpr U32 MOVS_REGS = (1u << 1) | (1u << 6) | (1u << 7); // ECX, ESI, EDI

// Note that this is only used when there are no segments involved
void Jit::movsr(JitWidth valueWidth, U32 size, JitWidth regWidth) {
    withCpuRegisterState(MOVS_REGS, MOVS_REGS, [&]() {
        if (size != 4 && regWidth == JitWidth::b32) {
            // Fold an unexpected direction into a large count so both guards
            // share one cold helper arm, without duplicating either copy loop.
            {
                RegPtr count = getStringRegEcx();
                if (currentOp->DF0 != currentOp->DF1) {
                    // ARM64 keeps flags in a live register; never modify the
                    // read-only view while constructing the helper guard.
                    RegPtr direction = getTmpReg();
                    mov(JitWidth::b32, direction, getReadOnlyFlags());
                    if (currentOp->DF1) xorValue(JitWidth::b32, direction, DF);
                    andValue(JitWidth::b32, direction, DF);
                    shlValue(JitWidth::b32, direction, 21);
                    orReg(JitWidth::b32, direction, count);
                    count = direction;
                }
                IfGreaterThan(JitWidth::b32, ComparisonType::Unsigned, count, MOVS_RAM_MIN_BYTES / size - 1);
            }
            {
                movs32rWithSegments(currentOp->base, size);
            } StartElse(); {
                withCpuRegisterState(MOVS_REGS, MOVS_REGS, [&]() {
                    movsrLoop(valueWidth, size, regWidth);
                });
            } EndIf();
            return;
        }
        // A direction observed during warmup is a hint, not an invariant.
        // Keep its compact loop and handle a later direction change safely.
        if (regWidth == JitWidth::b32 && currentOp->DF0 != currentOp->DF1) {
            auto specialized = [&]() {
                withCpuRegisterState(MOVS_REGS, MOVS_REGS, [&]() {
                    movsrLoop(valueWidth, size, regWidth);
                });
            };
            if (currentOp->DF1) IfDF();
            else IfNotTestBit(JitWidth::b32, getReadOnlyFlags(), 10);
            {
                specialized();
            } StartElse(); {
                movs32rWithSegments(currentOp->base, size);
            } EndIf();
        } else {
            movsrLoop(valueWidth, size, regWidth);
        }
    });
}

void Jit::movsrLoop(JitWidth valueWidth, U32 size, JitWidth regWidth) {
    if (currentOp->runCount == 0) {
        currentOp->flags2 |= OP_FLAG2_TRACED_STUB;
        emulateSingleOp(); // since this was never run, just stub it out so that we save jit code cache since its a lot of code
        return;
    }
    RegPtr esi = getStringRegEsi();
    RegPtr edi = getStringRegEdi();
    RegPtr ecx = getStringRegEcx();

    // in case we partially completed the move before moving to a new page
    // that doesn't have permission (code page, on demand page, etc)
    auto onFailure = [esi, edi, ecx, this]() {
        forceSyncBackIfNotCached(esi);
        forceSyncBackIfNotCached(edi);
        forceSyncBackIfNotCached(ecx);
        emulateSingleOp();
    };

#ifdef BOXEDWINE_64
    const bool smallCopy = currentOp->inst == Movsd ? !(currentOp->STR_FLAGS & STR_WIDE_COPY) :
        currentOp->STR_TOTAL / (currentOp->STR_COUNT ? currentOp->STR_COUNT : 1) < 8;
    if (currentOp->STR_COUNT > 10 && smallCopy) {
#endif
    bool doDF1 = currentOp->DF1 || (currentOp->DF1 == 0 && currentOp->DF0 == 0);
    bool doDF0 = currentOp->DF0 || (currentOp->DF1 == 0 && currentOp->DF0 == 0);

    if (doDF0 && doDF1) {
        IfDF();
    }
    if (doDF1) {
        U32 label = LoopBegin();
        hintLikelyStringLoopContinue();
        If(regWidth, ecx); {
            write(valueWidth, edi, read(valueWidth, esi, nullptr, onFailure), nullptr, onFailure);
            subValue(regWidth, esi, size);
            subValue(regWidth, edi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf();
        LoopEnd();
    }
    if (doDF1 && doDF0) {
        StartElse();
    }
    if (doDF0) {
        U32 label = LoopBegin();
        hintLikelyStringLoopContinue();
        If(regWidth, ecx); {
            write(valueWidth, edi, read(valueWidth, esi, nullptr, onFailure), nullptr, onFailure);
            addValue(regWidth, esi, size);
            addValue(regWidth, edi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf();
        LoopEnd();
    }
    if (doDF1 && doDF0) {
        EndIf();
    }
#ifdef BOXEDWINE_64    
    return;
    }

    // will use 64-bit reg instructions to do the copying in 8 byte chunks
    U32 bytesPerIter = 8;
    RegPtr tmp0 = getTmpReg();

    U32 mask;

    if (valueWidth == JitWidth::b32) {
        mask = 1;
    } else if (valueWidth == JitWidth::b16) {
        mask = 3;
    } else {
        mask = 7;
    }

    auto copyOneForward = [this, valueWidth, regWidth, size, esi, edi, ecx, onFailure]() {
        write(valueWidth, edi, read(valueWidth, esi, nullptr, onFailure), nullptr, onFailure);
        addValue(regWidth, esi, size);
        addValue(regWidth, edi, size);
        decReg(regWidth, ecx);
    };

    auto copyOneBackward = [this, valueWidth, regWidth, size, esi, edi, ecx, onFailure]() {
        write(valueWidth, edi, read(valueWidth, esi, nullptr, onFailure), nullptr, onFailure);
        subValue(regWidth, esi, size);
        subValue(regWidth, edi, size);
        decReg(regWidth, ecx);
    };

    bool doDF1 = currentOp->DF1 || (currentOp->DF1 == 0 && currentOp->DF0 == 0);
    bool doDF0 = currentOp->DF0 || (currentOp->DF1 == 0 && currentOp->DF0 == 0);

    if (doDF0 && doDF1) {
        IfDF();
    }
    if (doDF1) {
        // Backward direction (DF=1)
        // ESI/EDI point to the LAST element.  Pre-loop: if ECX
        // is odd, copy the highest single dword so the main loop
        // only processes aligned pairs.

        RegPtr delta = getTmpReg();
        mov(regWidth, delta, esi);
        subReg(regWidth, delta, edi);
        IfLessThan(regWidth, ComparisonType::Unsigned, delta, bytesPerIter); {
            If(regWidth, delta); {
                // Delta is fixed; only the remaining count terminates copying.
                U32 label = MarkJumpLocation();
                If(regWidth, ecx); {
                    copyOneBackward();
                    Goto(label);
                } EndIf();
            } EndIf();
        } EndIf();

        U32 label1 = MarkJumpLocation();
        IfTest(regWidth, ecx, mask); {
            copyOneBackward();
            Goto(label1);
        } EndIf();

        // Main loop: reads [ESI-4, ESI+3] via base = ESI-4
        U32 label = MarkJumpLocation();
        If(regWidth, ecx); {
            RegPtr addr = getTmpReg();            

            subValueWithDest(regWidth, addr, esi, bytesPerIter - size);
            read(JitWidth::b64, addr, nullptr, onFailure, tmp0, true);

            subValueWithDest(regWidth, addr, edi, bytesPerIter - size);
            write(JitWidth::b64, addr, tmp0, nullptr, onFailure, true);

            subValue(regWidth, esi, bytesPerIter);
            subValue(regWidth, edi, bytesPerIter);
            subValue(regWidth, ecx, bytesPerIter / size);
            Goto(label);
        } EndIf();
    }
    if (doDF1 && doDF0) {
        StartElse();
    }
    if (doDF0) {
        // Forward direction (DF=0)
        // ESI/EDI point to the FIRST element.  Same pre-loop odd
        // check, then the main loop reads [ESI, ESI+7].

        RegPtr delta = getTmpReg();
        mov(regWidth, delta, edi);
        subReg(regWidth, delta, esi);
        IfLessThan(regWidth, ComparisonType::Unsigned, delta, bytesPerIter); {
            If(regWidth, delta); {
                // Preserve sequential writes for overlap, stopping at ECX=0.
                U32 label = MarkJumpLocation();
                If(regWidth, ecx); {
                    copyOneForward();
                    Goto(label);
                } EndIf();
            } EndIf();
        } EndIf();

        U32 label1 = MarkJumpLocation();
        IfTest(regWidth, ecx, mask); {
            copyOneForward();
            Goto(label1);
        } EndIf();

        U32 label = MarkJumpLocation();
        If(regWidth, ecx); {            
            read(JitWidth::b64, esi, nullptr, onFailure, tmp0, true);
            write(JitWidth::b64, edi, tmp0, nullptr, onFailure, true);
            addValue(regWidth, esi, bytesPerIter);
            addValue(regWidth, edi, bytesPerIter);
            subValue(regWidth, ecx, bytesPerIter / size);
            Goto(label);
        } EndIf();
    }
    if (doDF1 && doDF0) {
        EndIf();
    }
#endif
}

void Jit::dynamic_movsb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
        } else {
            movs(op->base, JitWidth::b8, 1, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                movs32rWithSegments(op->base, 1);
            } else {
                movsr(JitWidth::b8, 1, JitWidth::b32);
            }
        } else {
            movs(op->base, JitWidth::b8, 1, JitWidth::b32);
        }
    }
}
void Jit::dynamic_movsw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
        } else {
            movs(op->base, JitWidth::b16, 2, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                movs32rWithSegments(op->base, 2);
            } else {
                movsr(JitWidth::b16, 2, JitWidth::b32);
            }
        } else {
            movs(op->base, JitWidth::b16, 2, JitWidth::b32);
        }
    }
}
void Jit::movs32rWithSegments(U32 base, U32 size) {
    withCpuRegisterState(MOVS_REGS, MOVS_REGS, [&]() {
        If(JitWidth::b32, getStringRegEcx()); {
            // Ordinary RAM copies return here without faulting or invalidating
            // code. Only unfinished special accesses need the interpreter.
            callNonFaultingCpuHelper(size == 1 ? movsb32rRam : size == 2 ? movsw32rRam : movsd32rRam,
                base, MOVS_REGS, MOVS_REGS);
            If(JitWidth::b32, getStringRegEcx()); {
                emulateSingleOp();
                blockExit();
            } EndIf();
        } EndIf();
    });
}

void Jit::movsd32rUnrolled(U32 base, U32 expected) {
    RegPtr esi = getStringRegEsi();
    RegPtr edi = getStringRegEdi();
    RegPtr ecx = getStringRegEcx();
    RegPtr guard = getTmpReg();
    const U32 bytes = expected * 4;
    mov(JitWidth::b32, guard, ecx);
    xorValue(JitWidth::b32, guard, expected);
    const U32 alignment = ramAccessAlignment(JitWidth::b32);
    if (alignment > 1) {
        // An unrolled overlapping copy cannot restart after committing an
        // element. Use the helper if the host may fault on its alignment.
        RegPtr misaligned = getTmpReg();
        mov(JitWidth::b32, misaligned, esi);
        orReg(JitWidth::b32, misaligned, edi);
        andValue(JitWidth::b32, misaligned, alignment - 1);
        orReg(JitWidth::b32, guard, misaligned);
    }
    {
        RegPtr direction = getTmpReg();
        mov(JitWidth::b32, direction, getReadOnlyFlags());
        andValue(JitWidth::b32, direction, DF);
        orReg(JitWidth::b32, guard, direction);
    }
    if (cpu->thread->process->hasSetSeg[base])
        orReg(JitWidth::b32, guard, getTmpSegAddress(base));
    if (cpu->thread->process->hasSetSeg[ES])
        orReg(JitWidth::b32, guard, getTmpSegAddress(ES));
    {
        RegPtr debug = readCPU(JitWidth::b8, offsetof(CPU, debugTrapActive));
        andValue(JitWidth::b32, debug, 0xff);
        orReg(JitWidth::b32, guard, debug);
    }
    {
        RegPtr srcEnd = getTmpReg(), dstEnd = getTmpReg();
        andValueWithDest(JitWidth::b32, srcEnd, esi, K_PAGE_MASK);
        andValueWithDest(JitWidth::b32, dstEnd, edi, K_PAGE_MASK);
        addValue(JitWidth::b32, srcEnd, bytes - 1);
        addValue(JitWidth::b32, dstEnd, bytes - 1);
        orReg(JitWidth::b32, srcEnd, dstEnd);
        andValue(JitWidth::b32, srcEnd, K_PAGE_SIZE);
        orReg(JitWidth::b32, guard, srcEnd);
    }
    IfNot(JitWidth::b32, guard); {
        guard = nullptr;
        auto onFailure = [esi, edi, ecx, this]() {
            forceSyncBackIfNotCached(esi);
            forceSyncBackIfNotCached(edi);
            forceSyncBackIfNotCached(ecx);
            emulateSingleOp();
            blockExit();
        };
        withRamPage(esi, false, [&](RegPtr srcHost) {
            withRamPage(edi, true, [&](RegPtr dstHost) {
                RegPtr value = getTmpReg();
                // Preserve x86 element order, including overlapping aliases.
                for (U32 i = 0; i < expected; ++i) {
                    read(JitWidth::b32, createMemPtr(srcHost, i * 4, false), value);
                    write(JitWidth::b32, createMemPtr(dstHost, i * 4, false), value);
                }
            }, onFailure);
        }, onFailure);
        // Update guest registers outside the callbacks: the Wasm callback
        // join restores its saved register-cache metadata. Failure exits
        // this activation, so only a fully successful batch reaches here.
        addValue(JitWidth::b32, esi, bytes);
        addValue(JitWidth::b32, edi, bytes);
        movValue(JitWidth::b32, ecx, 0);
    } StartElse(); {
        movs32rWithSegments(base, 4);
    } EndIf();
}

void Jit::dynamic_movsd_op(DecodedOp* op) {
    op->STR_FLAGS &= ~STR_UNROLLED_COPY;
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
        } else {
            movs(op->base, JitWidth::b32, 4, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            const U32 expected = movsdExpectedCount(op);
            if (expected) {
                op->STR_FLAGS |= STR_UNROLLED_COPY;
                movsd32rUnrolled(op->base, expected);
            } else if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base] ||
                    (op->STR_FLAGS & STR_LARGE_COPY) || op->STR_COUNT <= 10) {
                movs32rWithSegments(op->base, 4);
            } else {
                movsr(JitWidth::b32, 4, JitWidth::b32);
            }
        } else {
            movs(op->base, JitWidth::b32, 4, JitWidth::b32);
        }
    }
}

void Jit::cmps(U32 base, JitWidth valueWidth, U32 size, JitWidth regWidth, LazyFlagType lazyFlags) {
    // U32 dBase = cpu->seg[ES].address;
    // U32 sBase = cpu->seg[base].address;
    // S32 inc = cpu->getDirection();
    // U8 v1 = cpu->memory->readb(dBase + DI);
    // U8 v2 = cpu->memory->readb(sBase + SI);
    // DI += inc;
    // SI += inc;
    // cpu->dst.u8 = v2;
    // cpu->src.u8 = v1;
    // cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    // cpu->lazyFlags = FLAGS_SUB8;
    RegPtr esi = getStringRegEsi();
    RegPtr edi = getStringRegEdi();

    RegPtr src;
    RegPtr dest;

    if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[base]) {
        RegPtr srcAddress = getTmpSegAddress(base);

        if (regWidth == JitWidth::b32) {
            addReg(JitWidth::b32, srcAddress, esi);
        } else {
            RegPtr si = getTmpReg();
            xorReg(JitWidth::b32, si, si);
            mov(regWidth, si, esi);
            addReg(JitWidth::b32, srcAddress, si);
        }
        dest = read(valueWidth, std::move(srcAddress), nullptr, nullptr, getTmpReg8());

        RegPtr destAddress = getTmpSegAddress(ES);
        if (regWidth == JitWidth::b32) {
            addReg(JitWidth::b32, destAddress, edi);
        } else {
            RegPtr di = getTmpReg();
            xorReg(JitWidth::b32, di, di);
            mov(regWidth, di, edi);
            addReg(JitWidth::b32, destAddress, di);
        }

        src = read(valueWidth, std::move(destAddress), nullptr, nullptr, getTmpReg8());
    } else {
        dest = read(valueWidth, esi);
        src = read(valueWidth, edi);
    }
    storeLazyFlagsDest(dest);
    storeLazyFlagsSrc(src);
    subReg(valueWidth, dest, src);
    storeLazyFlagsResult(dest);
    storeLazyFlagType(lazyFlags);

    IfDF(); {
        subValue(regWidth, esi, size);
        subValue(regWidth, edi, size);
    } StartElse(); {
        addValue(regWidth, esi, size);
        addValue(regWidth, edi, size);
    } EndIf();
}

// The prefix leaves at least one element for scalar subtraction and flags.
// The base implementation is also usable by backends without vector emitters.
void Jit::compareStringPrefix(U32 size, bool scan, bool backward, bool repZero,
        RegPtr accumulator, RegPtr scratch, bool useHelper,
        const std::function<void()>& helper, const std::function<void()>& onFailure) {
    helper();
}

void Jit::compareStringR(JitWidth valueWidth, U32 size, U32 repZero,
        LazyFlagType lazyFlags, bool scan) {
    const U32 readRegs = (1u << 1) | (1u << 7) | (1u << (scan ? 0 : 6));
    const U32 writtenRegs = scan ? readRegs & ~(1u << 0) : readRegs;
    auto prefixHelper = scan ? (size == 1 ? scasb32rPrefix : size == 2 ? scasw32rPrefix : scasd32rPrefix)
        : (size == 1 ? cmpsb32rPrefix : size == 2 ? cmpsw32rPrefix : cmpsd32rPrefix);
    withCpuRegisterState(readRegs, writtenRegs, [&]() {
        RegPtr edi = getStringRegEdi(), ecx = getStringRegEcx();
        RegPtr esi = scan ? nullptr : getStringRegEsi();
        RegPtr dest = scan ? (size == 1 ? getTmpReg8(0) : getTmpReg(0)) : getTmpReg8();
        RegPtr src = getTmpReg8();
        auto onFailure = [&]() {
            forceSyncBackIfNotCached(ecx);
            forceSyncBackIfNotCached(edi);
            if (!scan) forceSyncBackIfNotCached(esi);
            emulateSingleOp();
            blockExit();
        };
        auto helper = [&]() {
            withCpuRegisterState(readRegs, writtenRegs, [&]() {
                callNonFaultingCpuHelper(prefixHelper, currentOp->base | (repZero ? 8 : 0), readRegs, writtenRegs);
            });
        };
        auto compare = [&](bool backward) {
            if (!scan) read(valueWidth, esi, nullptr, onFailure, dest);
            read(valueWidth, edi, nullptr, onFailure, src);
            if (backward) {
                subValue(JitWidth::b32, edi, size);
                if (!scan) subValue(JitWidth::b32, esi, size);
            } else {
                addValue(JitWidth::b32, edi, size);
                if (!scan) addValue(JitWidth::b32, esi, size);
            }
            decReg(JitWidth::b32, ecx);
        };
        auto continuing = [&]() {
            if (repZero) IfEqual(valueWidth, dest, src);
            else IfNotEqual(valueWidth, dest, src);
        };
        auto direction = [&](bool backward) {
            withCpuRegisterState(readRegs, writtenRegs, [&]() {
                If(JitWidth::b32, ecx); {
                    // Check the common immediate-stop case before any prefix.
                    compare(backward);
                    continuing(); {
                        withCpuRegisterState(readRegs, writtenRegs, [&]() {
                            // Native libc wins equal/byte scans; Wasm CMPS
                            // wins with page checks outside its inner loop.
                            // Pure vector loops also benefit at smaller counts.
                            const bool useHelper = scan ? sizeof(void*) == 8 && size == 1 && !repZero && !backward
                                : sizeof(void*) == 4 || repZero;
                            IfGreaterThan(JitWidth::b32, ComparisonType::Unsigned, ecx, useHelper ? 16 : 16 / size); {
                                auto prefix = [&]() {
                                    compareStringPrefix(size, scan, backward, repZero != 0, dest, src, useHelper, helper, onFailure);
                                };
                                if (sizeof(void*) == 4) {
                                    // A wide access near a page edge need not exit the JIT.
                                    // The RAM helper can span pages and return to the scalar tail.
                                    RegPtr guard = getTmpReg();
                                    auto span = [&](RegPtr result, RegPtr address) {
                                        andValueWithDest(JitWidth::b32, result, address, K_PAGE_MASK);
                                        if (backward) subValue(JitWidth::b32, result, 16 - size);
                                        else addValue(JitWidth::b32, result, 15);
                                        andValue(JitWidth::b32, result, K_PAGE_SIZE);
                                    };
                                    span(guard, edi);
                                    if (!scan) {
                                        RegPtr sourceGuard = getTmpReg();
                                        span(sourceGuard, esi);
                                        orReg(JitWidth::b32, guard, sourceGuard);
                                    }
                                    If(JitWidth::b32, guard); {
                                        guard = nullptr;
                                        helper();
                                    } StartElse(); {
                                        prefix();
                                    } EndIf();
                                } else {
                                    prefix();
                                }
                            } EndIf();
                        });
                        If(JitWidth::b32, ecx); {
                            U32 label = LoopBegin();
                            hintLikelyStringLoopContinue();
                            compare(backward);
                            If(JitWidth::b32, ecx); {
                                continuing(); { Goto(label); } EndIf();
                            } EndIf();
                            LoopEnd();
                        } EndIf();
                    } EndIf();
                    storeLazyFlagsDest(dest);
                    storeLazyFlagsSrc(src);
                    subReg(valueWidth, dest, src);
                    storeLazyFlagsResult(dest);
                    storeLazyFlagType(lazyFlags);
                } EndIf();
            });
        };
        IfDF(); { direction(true); } StartElse(); { direction(false); } EndIf();
    });
}

void Jit::cmpsr(JitWidth valueWidth, U32 size, JitWidth regWidth, U32 rep_zero, LazyFlagType lazyFlags) {
#ifdef BOXEDWINE_JIT_X86
    // U32 dBase = cpu->seg[ES].address;
    // U32 sBase = cpu->seg[base].address;
    // S32 inc = cpu->getDirection();
    // U32 count = ECX;
    // if (count) {
    //     U8 v1 = 0;
    //     U8 v2 = 0;
    //     for (U32 i = 0; i < count; i++) {
    //         v1 = cpu->memory->readb(dBase + EDI);
    //         v2 = cpu->memory->readb(sBase + ESI);
    //         EDI += inc;
    //         ESI += inc;
    //         ECX--;
    //         if ((v1 == v2) != rep_zero) break;
    //     }
    //     cpu->dst.u8 = v2;
    //     cpu->src.u8 = v1;
    //     cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    //     cpu->lazyFlags = FLAGS_SUB8;
    // }
    RegPtr esi = getStringRegEsi();
    RegPtr edi = getStringRegEdi();
    RegPtr dest = getTmpReg8();
    RegPtr src = getTmpReg8();
    // :TODO: maybe cache ecx if we have 6 or more temp regs?

    auto onFailure = [esi, edi, this]() {
        forceSyncBackIfNotCached(esi);
        forceSyncBackIfNotCached(edi);
        emulateSingleOp();
    };

    IfDF(); {
        If(regWidth, getReadOnlyReg(1)); {
            U32 label = LoopBegin();
            hintLikelyStringLoopContinue();
            If(regWidth, getReadOnlyReg(1)); {
                read(valueWidth, esi, nullptr, onFailure, dest);
                read(valueWidth, edi, nullptr, onFailure, src);
                subValue(regWidth, esi, size);
                subValue(regWidth, edi, size);
                decReg(regWidth, getReg(1));

                if (rep_zero) {
                    IfEqual(valueWidth, dest, src); {
                        Goto(label);
                    } EndIf();
                } else {
                    IfNotEqual(valueWidth, dest, src); {
                        Goto(label);
                    } EndIf();
                }
            } EndIf();
            LoopEnd();
            storeLazyFlagsDest(dest);
            storeLazyFlagsSrc(src);
            subReg(valueWidth, dest, src);
            storeLazyFlagsResult(dest);
            storeLazyFlagType(lazyFlags);
        } EndIf();
    } StartElse(); {
        If(regWidth, getReadOnlyReg(1)); {
            U32 label = LoopBegin();
            hintLikelyStringLoopContinue();
            If(regWidth, getReadOnlyReg(1)); {
                read(valueWidth, esi, nullptr, onFailure, dest);
                read(valueWidth, edi, nullptr, onFailure, src);
                addValue(regWidth, esi, size);
                addValue(regWidth, edi, size);
                decReg(regWidth, getReg(1));

                if (rep_zero) {
                    IfEqual(valueWidth, dest, src); {
                        Goto(label);
                    } EndIf();
                } else {
                    IfNotEqual(valueWidth, dest, src); {
                        Goto(label);
                    } EndIf();
                }
            } EndIf();
            LoopEnd();
            storeLazyFlagsDest(dest);
            storeLazyFlagsSrc(src);
            subReg(valueWidth, dest, src);
            storeLazyFlagsResult(dest);
            storeLazyFlagType(lazyFlags);
        } EndIf();
    }
    EndIf();
#else
    compareStringR(valueWidth, size, rep_zero, lazyFlags, false);
#endif
}

void Jit::dynamic_cmpsb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
            currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB8 if (e)cx is 0
        } else {
            cmps(op->base, JitWidth::b8, 1, JitWidth::b16, FLAGS_SUB8);
            currentLazyFlags = FLAGS_SUB8;
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp();
                currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB8 if (e)cx is 0
            } else {
                cmpsr(JitWidth::b8, 1, JitWidth::b32, op->repZero, FLAGS_SUB8);
                currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB8 if (e)cx is 0
            }
        } else {
            cmps(op->base, JitWidth::b8, 1, JitWidth::b32, FLAGS_SUB8);
            currentLazyFlags = FLAGS_SUB8;
        }
    }
}
void Jit::dynamic_cmpsw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
            currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB16 if (e)cx is 0
        } else {
            cmps(op->base, JitWidth::b16, 2, JitWidth::b16, FLAGS_SUB16);
            currentLazyFlags = FLAGS_SUB16;
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp();
                currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB16 if (e)cx is 0
            } else {
                cmpsr(JitWidth::b16, 2, JitWidth::b32, op->repZero, FLAGS_SUB16);
                currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB16 if (e)cx is 0
            }
        } else {
            cmps(op->base, JitWidth::b16, 2, JitWidth::b32, FLAGS_SUB16);
            currentLazyFlags = FLAGS_SUB16;
        }
    }
}
void Jit::dynamic_cmpsd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
            currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB32 if (e)cx is 0
        } else {
            cmps(op->base, JitWidth::b32, 4, JitWidth::b16, FLAGS_SUB32);
            currentLazyFlags = FLAGS_SUB32;
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp();
                currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB32 if (e)cx is 0
            } else {
                cmpsr(JitWidth::b32, 4, JitWidth::b32, op->repZero, FLAGS_SUB32);
                currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB32 if (e)cx is 0
            }
        } else {
            cmps(op->base, JitWidth::b32, 4, JitWidth::b32, FLAGS_SUB32);
            currentLazyFlags = FLAGS_SUB32;
        }
    }
}

void Jit::fillLoad32r(U32 base, U32 size, bool store) {
    const U32 readRegs = (1u << 0) | (1u << 1) | (1u << (store ? 7 : 6));
    const U32 writtenRegs = store ? readRegs & ~(1u << 0) : readRegs;
    const bool segmented = cpu->thread->process->hasSetSeg[base];
    // Measured x64/Wasm crossovers. Tiny calls keep the existing scalar loop;
    // the runtime guard also handles sites whose counts change after warmup.
    const U32 inlineCount = segmented ? 0 : sizeof(void*) == 8 ? (store ? 32 : 16) : 4;
    const JitWidth width = size == 1 ? JitWidth::b8 : size == 2 ? JitWidth::b16 : JitWidth::b32;
    auto helper = store ? (size == 1 ? stosb32rRam : size == 2 ? stosw32rRam : stosd32rRam)
        : (size == 1 ? lodsb32rRam : size == 2 ? lodsw32rRam : lodsd32rRam);
    withCpuRegisterState(readRegs, writtenRegs, [&]() {
        IfGreaterThan(JitWidth::b32, ComparisonType::Unsigned, getStringRegEcx(), inlineCount); {
            // Restore the entry cache mapping before generating the small arm.
            withCpuRegisterState(readRegs, writtenRegs, [&]() {
                callNonFaultingCpuHelper(helper, base, readRegs, writtenRegs);
                If(JitWidth::b32, getStringRegEcx()); {
                    emulateSingleOp();
                    blockExit();
                } EndIf();
            });
        } StartElse(); {
            if (!segmented) {
                if (store) stosr(width, size, JitWidth::b32);
                else lodsr(width, size, JitWidth::b32);
            }
        } EndIf();
    });
}

void Jit::stos(JitWidth valueWidth, U32 size, JitWidth regWidth) {
    // cpu->memory->writeb(cpu->seg[ES].address + EDI, AL);
    // EDI += cpu->getDirection();    
    RegPtr edi = getStringRegEdi();

    if (cpu->thread->process->hasSetSeg[ES]) {
        RegPtr writeAddress = getTmpSegAddress(ES);
        if (regWidth == JitWidth::b32) {
            addReg(JitWidth::b32, writeAddress, edi);
        } else {
            RegPtr di = getTmpReg();
            xorReg(JitWidth::b32, di, di);
            mov(regWidth, di, edi);
            addReg(JitWidth::b32, writeAddress, di);
        }
        write(valueWidth, std::move(writeAddress), valueWidth == JitWidth::b8 ? getReadOnlyReg8(0) : getReadOnlyReg(0));
    } else {
        write(valueWidth, edi, valueWidth == JitWidth::b8 ? getReadOnlyReg8(0) : getReadOnlyReg(0));
    }
    IfDF(); {
        subValue(regWidth, edi, size);
    } StartElse(); {
        addValue(regWidth, edi, size);
    } EndIf();
}

void Jit::stosr(JitWidth valueWidth, U32 size, JitWidth regWidth) {
    // U32 dBase = cpu->seg[ES].address;
    // S32 inc = cpu->getDirection();
    // U32 count = ECX;
    // 
    // for (U32 i = 0; i < count; i++) {
    // cpu->memory->writeb(dBase + EDI, AL);
    //     EDI += inc;
    //     ECX--;
    // }
    RegPtr edi = getStringRegEdi();
    RegPtr ecx = getStringRegEcx();
    RegPtr al = valueWidth == JitWidth::b8 ? getReadOnlyReg8(0) : getReadOnlyReg(0);

    auto onFailure = [edi, ecx, this]() {
        forceSyncBackIfNotCached(ecx);
        forceSyncBackIfNotCached(edi);
        emulateSingleOp();
        blockExit();
    };

    IfDF(); {
        U32 label = LoopBegin();
        hintLikelyStringLoopContinue();
        If(regWidth, ecx); {
            write(valueWidth, edi, al, nullptr, onFailure);
            subValue(regWidth, edi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf();
        LoopEnd();
    } StartElse(); {
        U32 label = LoopBegin();
        hintLikelyStringLoopContinue();
        If(regWidth, ecx); {
            write(valueWidth, edi, al, nullptr, onFailure);
            addValue(regWidth, edi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf();
        LoopEnd();
    }
    EndIf();
}

void Jit::dynamic_stosb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
        } else {
            stos(JitWidth::b8, 1, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            fillLoad32r(ES, 1, true);
        } else {
            stos(JitWidth::b8, 1, JitWidth::b32);
        }
    }
}
void Jit::dynamic_stosw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
        } else {
            stos(JitWidth::b16, 2, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            fillLoad32r(ES, 2, true);
        } else {
            stos(JitWidth::b16, 2, JitWidth::b32);
        }
    }
}
void Jit::dynamic_stosd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
        } else {
            stos(JitWidth::b32, 4, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            fillLoad32r(ES, 4, true);
        } else {
            stos(JitWidth::b32, 4, JitWidth::b32);
        }
    }
}

void Jit::lods(U32 base, JitWidth valueWidth, U32 size, JitWidth regWidth) {
    // AL = cpu->memory->readb(cpu->seg[base].address+ESI);
    // ESI += cpu->getDirection();
    RegPtr esi = getStringRegEsi();
    RegPtr al;

    if (valueWidth == JitWidth::b8) {
        al = getReg8(0);
    } else {
        al = getReg(0);
    }
    if (cpu->thread->process->hasSetSeg[base]) {
        RegPtr readAddress = getTmpSegAddress(base);

        if (regWidth == JitWidth::b32) {
            addReg(JitWidth::b32, readAddress, esi);
        } else {
            RegPtr si = getTmpReg();
            xorReg(JitWidth::b32, si, si);
            mov(regWidth, si, esi);
            addReg(JitWidth::b32, readAddress, si);
        }
        mov(valueWidth, al, read(valueWidth, std::move(readAddress), nullptr, nullptr, getTmpReg8()));
    } else {
        mov(valueWidth, al, read(valueWidth, esi, nullptr, nullptr, getTmpReg8()));
    }
    IfDF(); {
        subValue(regWidth, esi, size);
    } StartElse(); {
        addValue(regWidth, esi, size);
    } EndIf();
}

void Jit::lodsr(JitWidth valueWidth, U32 size, JitWidth regWidth) {
    // U32 sBase = cpu->seg[base].address;
    // S32 inc = cpu->getDirection();
    // U32 count = ECX;

    // for (U32 i = 0; i < count; i++) {
    //     AL = cpu->memory->readb(sBase + ESI);
    //     ESI += inc;
    //     ECX--;
    // }
    RegPtr esi = getStringRegEsi();
    RegPtr ecx = getStringRegEcx();
    RegPtr al = valueWidth == JitWidth::b8 ? getReg8(0) : getReg(0);

    auto onFailure = [esi, ecx, al, this]() {
        forceSyncBackIfNotCached(ecx);
        forceSyncBackIfNotCached(esi);
        forceSyncBackIfNotCached(al);
        emulateSingleOp();
        blockExit();
    };

    IfDF(); {
        U32 label = LoopBegin();
        hintLikelyStringLoopContinue();
        If(regWidth, ecx); {
            mov(valueWidth, al, read(valueWidth, esi, nullptr, onFailure, getTmpReg8()));
            subValue(regWidth, esi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf();
        LoopEnd();
    } StartElse(); {
        U32 label = LoopBegin();
        hintLikelyStringLoopContinue();
        If(regWidth, ecx); {
            mov(valueWidth, al, read(valueWidth, esi, nullptr, onFailure, getTmpReg8()));
            addValue(regWidth, esi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf();
        LoopEnd();
    }
    EndIf();
}

void Jit::dynamic_lodsb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
        } else {
            lods(op->base, JitWidth::b8, 1, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            fillLoad32r(op->base, 1, false);
        } else {
            lods(op->base, JitWidth::b8, 1, JitWidth::b32);
        }
    }
}
void Jit::dynamic_lodsw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
        } else {
            lods(op->base, JitWidth::b16, 2, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            fillLoad32r(op->base, 2, false);
        } else {
            lods(op->base, JitWidth::b16, 2, JitWidth::b32);
        }
    }
}
void Jit::dynamic_lodsd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
        } else {
            lods(op->base, JitWidth::b32, 4, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            fillLoad32r(op->base, 4, false);
        } else {
            lods(op->base, JitWidth::b32, 4, JitWidth::b32);
        }
    }
}

void Jit::scas(JitWidth valueWidth, U32 size, JitWidth regWidth, LazyFlagType lazyFlags) {
    // U32 dBase = cpu->seg[ES].address;
    // S32 inc = cpu->getDirection();
    // U8 v1 = cpu->memory->readb(dBase + EDI);
    // EDI += inc;
    // cpu->dst.u8 = AL;
    // cpu->src.u8 = v1;
    // cpu->result.u8 = AL - v1;
    // cpu->lazyFlags = FLAGS_SUB8;
    RegPtr edi = getStringRegEdi();
    RegPtr src;
    RegPtr dest = valueWidth == JitWidth::b8 ? getTmpReg8(0) : getTmpReg(0);

    if (cpu->thread->process->hasSetSeg[ES]) {
        RegPtr destAddress = getTmpSegAddress(ES);
        if (regWidth == JitWidth::b32) {
            addReg(JitWidth::b32, destAddress, edi);
        } else {
            RegPtr di = getTmpReg();
            xorReg(JitWidth::b32, di, di);
            mov(regWidth, di, edi);
            addReg(JitWidth::b32, destAddress, di);
        }

        src = read(valueWidth, std::move(destAddress), nullptr, nullptr, getTmpReg8());
    } else {
        src = read(valueWidth, edi);
    }
    storeLazyFlagsDest(dest);
    storeLazyFlagsSrc(src);
    subReg(valueWidth, dest, src);
    storeLazyFlagsResult(dest);
    storeLazyFlagType(lazyFlags);

    IfDF(); {
        subValue(regWidth, edi, size);
    } StartElse(); {
        addValue(regWidth, edi, size);
    } EndIf();
}

void Jit::scasr(JitWidth valueWidth, U32 size, JitWidth regWidth, U32 rep_zero, LazyFlagType lazyFlags) {
#ifdef BOXEDWINE_JIT_X86
    // U32 dBase = cpu->seg[ES].address;
    // S32 inc = cpu->getDirection();
    // U32 count = ECX;
    // if (count) {
    //     U8 v1 = 0;
    //     for (U32 i = 0; i < count; i++) {
    //         v1 = cpu->memory->readb(dBase + EDI);
    //         EDI += inc;
    //         ECX--;
    //         if ((AL == v1) != rep_zero) break;
    //    }
    //     cpu->dst.u8 = AL;
    //     cpu->src.u8 = v1;
    //     cpu->result.u8 = AL - v1;
    //     cpu->lazyFlags = FLAGS_SUB8;
    // }
    RegPtr edi = getStringRegEdi();
    RegPtr ecx = getStringRegEcx();
    RegPtr dest = valueWidth == JitWidth::b8 ? getTmpReg8(0) : getTmpReg(0); // tmp because result = AL - v1, basstour/opentdd wil fail if AL is written back after math
    RegPtr src = getTmpReg8();    

    auto onFailure = [edi, ecx, this]() {
        forceSyncBackIfNotCached(ecx);
        forceSyncBackIfNotCached(edi);
        emulateSingleOp();
    };

    IfDF(); {
        If(regWidth, ecx); {
            U32 label = LoopBegin();
            hintLikelyStringLoopContinue();
            If(regWidth, ecx); {
                read(valueWidth, edi, nullptr, onFailure, src);
                subValue(regWidth, edi, size);
                decReg(regWidth, ecx);

                if (rep_zero) {
                    IfEqual(valueWidth, dest, src); {
                        Goto(label);
                    } EndIf();
                } else {
                    IfNotEqual(valueWidth, dest, src); {
                        Goto(label);
                    } EndIf();
                }
            } EndIf();
            LoopEnd();
            storeLazyFlagsDest(dest);
            storeLazyFlagsSrc(src);
            subReg(valueWidth, dest, src);
            storeLazyFlagsResult(dest);
            storeLazyFlagType(lazyFlags);
        } EndIf();
    } StartElse(); {
        If(regWidth, ecx); {
            U32 label = LoopBegin();
            hintLikelyStringLoopContinue();
            If(regWidth, ecx); {
                read(valueWidth, edi, nullptr, onFailure, src);
                addValue(regWidth, edi, size);
                decReg(regWidth, ecx);

                if (rep_zero) {
                    IfEqual(valueWidth, dest, src); {
                        Goto(label);
                    } EndIf();
                } else {
                    IfNotEqual(valueWidth, dest, src); {
                        Goto(label);
                    } EndIf();
                }
            } EndIf();
            LoopEnd();
            storeLazyFlagsDest(dest);
            storeLazyFlagsSrc(src);
            subReg(valueWidth, dest, src);
            storeLazyFlagsResult(dest);
            storeLazyFlagType(lazyFlags);
        } EndIf();
    }
    EndIf();
#else
    compareStringR(valueWidth, size, rep_zero, lazyFlags, true);
#endif
}

void Jit::dynamic_scasb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
            currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB8 if (e)cx is 0
        } else {
            scas(JitWidth::b8, 1, JitWidth::b16, FLAGS_SUB8);
            currentLazyFlags = FLAGS_SUB8;
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES]) {
                emulateSingleOp();
                currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB8 if (e)cx is 0
            } else {
                scasr(JitWidth::b8, 1, JitWidth::b32, op->repZero, FLAGS_SUB8);
                currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB8 if (e)cx is 0
            }
        } else {
            scas(JitWidth::b8, 1, JitWidth::b32, FLAGS_SUB8);
            currentLazyFlags = FLAGS_SUB8;
        }
    }    
}
void Jit::dynamic_scasw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
            currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB16 if (e)cx is 0
        } else {
            scas(JitWidth::b16, 2, JitWidth::b16, FLAGS_SUB16);
            currentLazyFlags = FLAGS_SUB16;
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES]) {
                emulateSingleOp();
                currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB16 if (e)cx is 0
            } else {
                scasr(JitWidth::b16, 2, JitWidth::b32, op->repZero, FLAGS_SUB16);
                currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB16 if (e)cx is 0
            }
        } else {
            scas(JitWidth::b16, 2, JitWidth::b32, FLAGS_SUB16);
            currentLazyFlags = FLAGS_SUB16;
        }
    }
}
void Jit::dynamic_scasd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
            currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB32 if (e)cx is 0
        } else {
            scas(JitWidth::b32, 4, JitWidth::b16, FLAGS_SUB32);
            currentLazyFlags = FLAGS_SUB32;
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES]) {
                emulateSingleOp();
                currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB32 if (e)cx is 0
            } else {
                scasr(JitWidth::b32, 4, JitWidth::b32, op->repZero, FLAGS_SUB32);
                currentLazyFlags = FLAGS_NULL; // not set to FLAGS_SUB32 if (e)cx is 0
            }
        } else {
            scas(JitWidth::b32, 4, JitWidth::b32, FLAGS_SUB32);
            currentLazyFlags = FLAGS_SUB32;
        }
    }
}
