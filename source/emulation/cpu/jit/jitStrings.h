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
    RegPtr esi = getReg(6);
    RegPtr edi = getReg(7);

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
    IfFlagSet(DF); {
        subValue(regWidth, esi, size);
        subValue(regWidth, edi, size);
    } StartElse(); {
        addValue(regWidth, esi, size);
        addValue(regWidth, edi, size);
    } EndIf();
}

void Jit::movsr(JitWidth valueWidth, U32 size, JitWidth regWidth) {
    RegPtr esi = getReg(6);
    RegPtr edi = getReg(7);
    RegPtr ecx = getReg(1);

    // in case we partically completed the move before moving to a new page that doesn't have permission (code page, on demmand page, etc)
    auto onFailure = [esi, edi, ecx, this]() {
        forceSyncBackIfNotCached(esi);
        forceSyncBackIfNotCached(edi);
        forceSyncBackIfNotCached(ecx);
        emulateSingleOp(ecx);
    };

    IfFlagSet(DF, true); {
        U32 label = MarkJumpLocation();
        If(regWidth, ecx, true); {
            write(valueWidth, edi, read(valueWidth, esi, nullptr, onFailure), nullptr, onFailure);
            subValue(regWidth, esi, size);
            subValue(regWidth, edi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf(true);
    } StartElse(true); {
        U32 label = MarkJumpLocation();
        If(regWidth, ecx, true); {
            write(valueWidth, edi, read(valueWidth, esi, nullptr, onFailure), nullptr, onFailure);
            addValue(regWidth, esi, size);
            addValue(regWidth, edi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf(true);
    }
    EndIf(true);
}

void Jit::dynamic_movsb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
        } else {
            movs(op->base, JitWidth::b8, 1, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp(getTmpReg());
            } else {
                movsr(JitWidth::b8, 1, JitWidth::b32);
            }
        } else {
            movs(op->base, JitWidth::b8, 1, JitWidth::b32);
        }
    }
    incrementEip(op->len);
}
void Jit::dynamic_movsw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
        } else {
            movs(op->base, JitWidth::b16, 2, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp(getTmpReg());
            } else {
                movsr(JitWidth::b16, 2, JitWidth::b32);
            }
        } else {
            movs(op->base, JitWidth::b16, 2, JitWidth::b32);
        }
    }
    incrementEip(op->len);
}
void Jit::dynamic_movsd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
        } else {
            movs(op->base, JitWidth::b32, 4, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp(getTmpReg());
            } else {
                movsr(JitWidth::b32, 4, JitWidth::b32);
            }
        } else {
            movs(op->base, JitWidth::b32, 4, JitWidth::b32);
        }
    }
    incrementEip(op->len);
}

void Jit::cmps(U32 base, JitWidth valueWidth, U32 size, JitWidth regWidth, const LazyFlags* lazyFlags) {
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
    RegPtr esi = getReg(6);
    RegPtr edi = getReg(7);

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
        dest = read(valueWidth, std::move(srcAddress), nullptr, nullptr, false, getTmpReg8());

        RegPtr destAddress = getTmpSegAddress(ES);
        if (regWidth == JitWidth::b32) {
            addReg(JitWidth::b32, destAddress, edi);
        } else {
            RegPtr di = getTmpReg();
            xorReg(JitWidth::b32, di, di);
            mov(regWidth, di, edi);
            addReg(JitWidth::b32, destAddress, di);
        }

        src = read(valueWidth, std::move(destAddress), nullptr, nullptr, false, getTmpReg8());
    } else {
        dest = read(valueWidth, esi);
        src = read(valueWidth, edi);
    }
    storeLazyFlagsDest(dest);
    storeLazyFlagsSrc(src);
    subReg(valueWidth, dest, src);
    storeLazyFlagsResult(dest);
    storeLazyFlags(lazyFlags);

    IfFlagSet(DF); {
        subValue(regWidth, esi, size);
        subValue(regWidth, edi, size);
    } StartElse(); {
        addValue(regWidth, esi, size);
        addValue(regWidth, edi, size);
    } EndIf();
}

void Jit::cmpsr(JitWidth valueWidth, U32 size, JitWidth regWidth, U32 rep_zero, const LazyFlags* lazyFlags) {
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
    RegPtr esi = getReg(6);
    RegPtr edi = getReg(7);
    RegPtr dest = getTmpReg8();
    RegPtr src = getTmpReg8();
    // :TODO: maybe cache ecx if we have 6 or more temp regs?

    auto onFailure = [esi, edi, src, this]() {
        forceSyncBackIfNotCached(esi);
        forceSyncBackIfNotCached(edi);
        emulateSingleOp(src);
    };

    IfFlagSet(DF, true); {
        If(regWidth, getReadOnlyReg(1), true); {
            U32 label = MarkJumpLocation();
            If(regWidth, getReadOnlyReg(1), true); {
                read(valueWidth, esi, nullptr, onFailure, false, dest);
                read(valueWidth, edi, nullptr, onFailure, false, src);
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
            } EndIf(true);
            storeLazyFlagsDest(dest);
            storeLazyFlagsSrc(src);
            subReg(valueWidth, dest, src);
            storeLazyFlagsResult(dest);
            storeLazyFlags(lazyFlags);
        } EndIf(true);
    } StartElse(true); {
        If(regWidth, getReadOnlyReg(1), true); {
            U32 label = MarkJumpLocation();
            If(regWidth, getReadOnlyReg(1), true); {
                read(valueWidth, esi, nullptr, onFailure, false, dest);
                read(valueWidth, edi, nullptr, onFailure, false, src);
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
            } EndIf(true);
            storeLazyFlagsDest(dest);
            storeLazyFlagsSrc(src);
            subReg(valueWidth, dest, src);
            storeLazyFlagsResult(dest);
            storeLazyFlags(lazyFlags);
        } EndIf(true);
    }
    EndIf(true);
}

void Jit::dynamic_cmpsb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
            currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else {
            cmps(op->base, JitWidth::b8, 1, JitWidth::b16, FLAGS_SUB8);
            currentLazyFlags = FLAGS_SUB8;
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp(getTmpReg());
                currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
            } else {
                cmpsr(JitWidth::b8, 1, JitWidth::b32, op->repZero, FLAGS_SUB8);
                currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
            }
        } else {
            cmps(op->base, JitWidth::b8, 1, JitWidth::b32, FLAGS_SUB8);
            currentLazyFlags = FLAGS_SUB8;
        }
    }
    incrementEip(op->len);
}
void Jit::dynamic_cmpsw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
            currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else {
            cmps(op->base, JitWidth::b16, 2, JitWidth::b16, FLAGS_SUB16);
            currentLazyFlags = FLAGS_SUB16;
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp(getTmpReg());
                currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
            } else {
                cmpsr(JitWidth::b16, 2, JitWidth::b32, op->repZero, FLAGS_SUB16);
                currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
            }
        } else {
            cmps(op->base, JitWidth::b16, 2, JitWidth::b32, FLAGS_SUB8);
            currentLazyFlags = FLAGS_SUB16;
        }
    }
    incrementEip(op->len);
}
void Jit::dynamic_cmpsd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
            currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else {
            cmps(op->base, JitWidth::b32, 4, JitWidth::b16, FLAGS_SUB32);
            currentLazyFlags = FLAGS_SUB32;
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp(getTmpReg());
                currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
            } else {
                cmpsr(JitWidth::b32, 4, JitWidth::b32, op->repZero, FLAGS_SUB32);
                currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
            }
        } else {
            cmps(op->base, JitWidth::b32, 4, JitWidth::b32, FLAGS_SUB32);
            currentLazyFlags = FLAGS_SUB32;
        }
    }
    incrementEip(op->len);
}

void Jit::stos(JitWidth valueWidth, U32 size, JitWidth regWidth) {
    // cpu->memory->writeb(cpu->seg[ES].address + EDI, AL);
    // EDI += cpu->getDirection();    
    RegPtr edi = getReg(7);

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
        write(valueWidth, std::move(writeAddress), getReadOnlyReg8(0));
    } else {
        write(valueWidth, edi, getReadOnlyReg8(0));
    }
    IfFlagSet(DF); {
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
    RegPtr edi = getReg(7);
    RegPtr ecx = getReg(1);
    RegPtr al = getReadOnlyReg8(0);

    auto onFailure = [edi, ecx, this]() {
        forceSyncBackIfNotCached(ecx);
        forceSyncBackIfNotCached(edi);
        emulateSingleOp(ecx);
    };

    IfFlagSet(DF, true); {
        U32 label = MarkJumpLocation();
        If(regWidth, ecx, true); {
            write(valueWidth, edi, al, nullptr, onFailure);
            subValue(regWidth, edi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf(true);
    } StartElse(true); {
        U32 label = MarkJumpLocation();
        If(regWidth, ecx, true); {
            write(valueWidth, edi, al, nullptr, onFailure);
            addValue(regWidth, edi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf(true);
    }
    EndIf(true);
}

void Jit::dynamic_stosb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
        } else {
            stos(JitWidth::b8, 1, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES]) {
                emulateSingleOp(getTmpReg());
            } else {
                stosr(JitWidth::b8, 1, JitWidth::b32);
            }
        } else {
            stos(JitWidth::b8, 1, JitWidth::b32);
        }
    }
    incrementEip(op->len);
}
void Jit::dynamic_stosw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
        } else {
            stos(JitWidth::b16, 2, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES]) {
                emulateSingleOp(getTmpReg());
            } else {
                stosr(JitWidth::b16, 2, JitWidth::b32);
            }
        } else {
            stos(JitWidth::b16, 2, JitWidth::b32);
        }
    }
    incrementEip(op->len);
}
void Jit::dynamic_stosd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
        } else {
            stos(JitWidth::b32, 4, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES]) {
                emulateSingleOp(getTmpReg());
            } else {
                stosr(JitWidth::b32, 4, JitWidth::b32);
            }
        } else {
            stos(JitWidth::b32, 4, JitWidth::b32);
        }
    }
    incrementEip(op->len);
}

void Jit::lods(U32 base, JitWidth valueWidth, U32 size, JitWidth regWidth) {
    // AL = cpu->memory->readb(cpu->seg[base].address+ESI);
    // ESI += cpu->getDirection();
    RegPtr esi = getReg(6);
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
        mov(valueWidth, al, read(valueWidth, std::move(readAddress), nullptr, nullptr, false, getTmpReg8()));
    } else {
        mov(valueWidth, al, read(valueWidth, esi, nullptr, nullptr, false, getTmpReg8()));
    }
    IfFlagSet(DF); {
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
    RegPtr esi = getReg(6);
    RegPtr ecx = getReg(1);
    RegPtr al = getReg8(0);

    auto onFailure = [esi, ecx, al, this]() {
        forceSyncBackIfNotCached(ecx);
        forceSyncBackIfNotCached(esi);
        forceSyncBackIfNotCached(al);
        emulateSingleOp(ecx);
    };

    IfFlagSet(DF, true); {
        U32 label = MarkJumpLocation();
        If(regWidth, ecx, true); {
            mov(valueWidth, al, read(valueWidth, esi, nullptr, onFailure, false, getTmpReg8()));
            subValue(regWidth, esi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf(true);
    } StartElse(true); {
        U32 label = MarkJumpLocation();
        If(regWidth, ecx, true); {
            mov(valueWidth, al, read(valueWidth, esi, nullptr, onFailure, false, getTmpReg8()));
            addValue(regWidth, esi, size);
            decReg(regWidth, ecx);
            Goto(label);
        } EndIf(true);
    }
    EndIf(true);
}

void Jit::dynamic_lodsb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
        } else {
            lods(op->base, JitWidth::b8, 1, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp(getTmpReg());
            } else {
                lodsr(JitWidth::b8, 1, JitWidth::b32);
            }
        } else {
            lods(op->base, JitWidth::b8, 1, JitWidth::b32);
        }
    }
    incrementEip(op->len);
}
void Jit::dynamic_lodsw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
        } else {
            lods(op->base, JitWidth::b16, 2, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp(getTmpReg());
            } else {
                lodsr(JitWidth::b16, 2, JitWidth::b32);
            }
        } else {
            lods(op->base, JitWidth::b16, 2, JitWidth::b32);
        }
    }
    incrementEip(op->len);
}
void Jit::dynamic_lodsd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
        } else {
            lods(op->base, JitWidth::b32, 4, JitWidth::b16);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[op->base]) {
                emulateSingleOp(getTmpReg());
            } else {
                lodsr(JitWidth::b32, 4, JitWidth::b32);
            }
        } else {
            lods(op->base, JitWidth::b32, 4, JitWidth::b32);
        }
    }
    incrementEip(op->len);
}

void Jit::scas(JitWidth valueWidth, U32 size, JitWidth regWidth, const LazyFlags* lazyFlags) {
    // U32 dBase = cpu->seg[ES].address;
    // S32 inc = cpu->getDirection();
    // U8 v1 = cpu->memory->readb(dBase + EDI);
    // EDI += inc;
    // cpu->dst.u8 = AL;
    // cpu->src.u8 = v1;
    // cpu->result.u8 = AL - v1;
    // cpu->lazyFlags = FLAGS_SUB8;
    RegPtr edi = getReg(7);
    RegPtr src;
    RegPtr dest = getReadOnlyReg8(0);

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

        src = read(valueWidth, std::move(destAddress), nullptr, nullptr, false, getTmpReg8());
    } else {
        src = read(valueWidth, edi);
    }
    storeLazyFlagsDest(dest);
    storeLazyFlagsSrc(src);
    subReg(valueWidth, dest, src);
    storeLazyFlagsResult(dest);
    storeLazyFlags(lazyFlags);

    IfFlagSet(DF); {
        subValue(regWidth, edi, size);
    } StartElse(); {
        addValue(regWidth, edi, size);
    } EndIf();
}

void Jit::scasr(JitWidth valueWidth, U32 size, JitWidth regWidth, U32 rep_zero, const LazyFlags* lazyFlags) {
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
    RegPtr edi = getReg(7);
    RegPtr dest = getTmpReg8(0); // tmp because result = AL - v1, basstour/opentdd wil fail if AL is written back after math
    RegPtr src = getTmpReg8();
    RegPtr ecx = getReg(1);

    auto onFailure = [edi, ecx, this]() {
        forceSyncBackIfNotCached(ecx);
        forceSyncBackIfNotCached(edi);
        emulateSingleOp(ecx);
    };

    IfFlagSet(DF, true); {
        If(regWidth, ecx, true); {
            U32 label = MarkJumpLocation();
            If(regWidth, ecx, true); {
                read(valueWidth, edi, nullptr, onFailure, false, src);
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
            } EndIf(true);
            storeLazyFlagsDest(dest);
            storeLazyFlagsSrc(src);
            subReg(valueWidth, dest, src);
            storeLazyFlagsResult(dest);
            storeLazyFlags(lazyFlags);
        } EndIf(true);
    } StartElse(true); {
        If(regWidth, ecx, true); {
            U32 label = MarkJumpLocation();
            If(regWidth, ecx, true); {
                read(valueWidth, edi, nullptr, onFailure, false, src);
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
            } EndIf(true);
            storeLazyFlagsDest(dest);
            storeLazyFlagsSrc(src);
            subReg(valueWidth, dest, src);
            storeLazyFlagsResult(dest);
            storeLazyFlags(lazyFlags);
        } EndIf(true);
    }
    EndIf(true);
}

void Jit::dynamic_scasb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
            currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else {
            scas(JitWidth::b8, 1, JitWidth::b16, FLAGS_SUB8);
            currentLazyFlags = FLAGS_SUB8;
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES]) {
                emulateSingleOp(getTmpReg());
                currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
            } else {
                scasr(JitWidth::b8, 1, JitWidth::b32, op->repZero, FLAGS_SUB8);
                currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
            }
        } else {
            scas(JitWidth::b8, 1, JitWidth::b32, FLAGS_SUB8);
            currentLazyFlags = FLAGS_SUB8;
        }
    }    
    incrementEip(op->len);
}
void Jit::dynamic_scasw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
            currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else {
            scas(JitWidth::b16, 2, JitWidth::b16, FLAGS_SUB16);
            currentLazyFlags = FLAGS_SUB16;
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES]) {
                emulateSingleOp(getTmpReg());
                currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
            } else {
                scasr(JitWidth::b16, 2, JitWidth::b32, op->repZero, FLAGS_SUB16);
                currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
            }
        } else {
            scas(JitWidth::b16, 2, JitWidth::b32, FLAGS_SUB16);
            currentLazyFlags = FLAGS_SUB16;
        }
    }
    incrementEip(op->len);
}
void Jit::dynamic_scasd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            emulateSingleOp(getTmpReg());
            currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else {
            scas(JitWidth::b32, 4, JitWidth::b16, FLAGS_SUB32);
            currentLazyFlags = FLAGS_SUB32;
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES]) {
                emulateSingleOp(getTmpReg());
                currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
            } else {
                scasr(JitWidth::b32, 4, JitWidth::b32, op->repZero, FLAGS_SUB32);
                currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
            }
        } else {
            scas(JitWidth::b32, 4, JitWidth::b32, FLAGS_SUB32);
            currentLazyFlags = FLAGS_SUB32;
        }
    }
    incrementEip(op->len);
}
