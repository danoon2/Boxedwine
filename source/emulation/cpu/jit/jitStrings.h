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

// Note that this is only used when there are no segments involved
void Jit::movsr(JitWidth valueWidth, U32 size, JitWidth regWidth) {
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
    if (currentOp->STR_COUNT > 10 && currentOp->STR_TOTAL / currentOp->STR_COUNT < 8) {
#endif
    bool doDF1 = currentOp->DF1 || (currentOp->DF1 == 0 && currentOp->DF0 == 0);
    bool doDF0 = currentOp->DF0 || (currentOp->DF1 == 0 && currentOp->DF0 == 0);

    if (doDF0 && doDF1) {
        IfDF();
    }
    if (doDF1) {
        U32 label = LoopBegin();
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
            U32 label = MarkJumpLocation();
            If(regWidth, delta); {
                copyOneBackward();
                Goto(label);
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
            U32 label = MarkJumpLocation();
            If(regWidth, delta); {
                copyOneForward();
                Goto(label);
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
                emulateSingleOp();
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
                emulateSingleOp();
            } else {
                movsr(JitWidth::b16, 2, JitWidth::b32);
            }
        } else {
            movs(op->base, JitWidth::b16, 2, JitWidth::b32);
        }
    }
}
void Jit::dynamic_movsd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp();
        } else {
            movs(op->base, JitWidth::b32, 4, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp();
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

void Jit::cmpsr(JitWidth valueWidth, U32 size, JitWidth regWidth, U32 rep_zero, LazyFlagType lazyFlags) {    
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
    };

    IfDF(); {
        U32 label = LoopBegin();
        If(regWidth, ecx); {
            write(valueWidth, edi, al, nullptr, onFailure);
            subValue(regWidth, edi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf();
        LoopEnd();
    } StartElse(); {
        U32 label = LoopBegin();
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
            if (cpu->thread->process->hasSetSeg[ES]) {
                emulateSingleOp();
            } else {
                stosr(JitWidth::b8, 1, JitWidth::b32);
            }
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
            if (cpu->thread->process->hasSetSeg[ES]) {
                emulateSingleOp();
            } else {
                stosr(JitWidth::b16, 2, JitWidth::b32);
            }
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
            if (cpu->thread->process->hasSetSeg[ES]) {
                emulateSingleOp();
            } else {
                stosr(JitWidth::b32, 4, JitWidth::b32);
            }
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
    };

    IfDF(); {
        U32 label = LoopBegin();
        If(regWidth, ecx); {
            mov(valueWidth, al, read(valueWidth, esi, nullptr, onFailure, getTmpReg8()));
            subValue(regWidth, esi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf();
        LoopEnd();
    } StartElse(); {
        U32 label = LoopBegin();
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
            if (cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp();
            } else {
                lodsr(JitWidth::b8, 1, JitWidth::b32);
            }
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
            if (cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp();
            } else {
                lodsr(JitWidth::b16, 2, JitWidth::b32);
            }
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
            if (cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp();
            } else {
                lodsr(JitWidth::b32, 4, JitWidth::b32);
            }
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
