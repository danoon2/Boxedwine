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

void DynamicData::movs(U32 base, DynWidth valueWidth, U32 size, DynWidth regWidth) {
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

        if (regWidth == DYN_32bit) {
            addReg(DYN_32bit, readAddress, esi, false);
        } else {
            RegPtr si = getTmpReg();
            xorReg(DYN_32bit, si, si, false);
            mov(regWidth, si, esi);
            addReg(DYN_32bit, readAddress, si, false);
        }
        RegPtr value = read(valueWidth, std::move(readAddress));

        RegPtr writeAddress = getTmpSegAddress(ES);
        if (regWidth == DYN_32bit) {
            addReg(DYN_32bit, writeAddress, edi, false);
        } else {
            RegPtr di = getTmpReg();
            xorReg(DYN_32bit, di, di, false);
            mov(regWidth, di, edi);
            addReg(DYN_32bit, writeAddress, std::move(di), false);
        }
        write(valueWidth, writeAddress, value);
    } else {
        write(valueWidth, edi, read(valueWidth, esi));
    }
    IfFlagSet(DF); {
        subValue(regWidth, esi, size, false);
        subValue(regWidth, edi, size, false);
    } StartElse(); {
        addValue(regWidth, esi, size, false);
        addValue(regWidth, edi, size, false);
    } EndIf();
}

void DynamicData::movsr(DynWidth valueWidth, U32 size, DynWidth regWidth) {
    RegPtr esi = getReg(6);
    RegPtr edi = getReg(7);
    RegPtr ecx = getReg(1);

    IfFlagSet(DF, true); {
        U32 label = MarkJumpLocation();
        If(regWidth, ecx, true); {
            write(valueWidth, edi, read(valueWidth, esi));
            subValue(regWidth, esi, size, false);
            subValue(regWidth, edi, size, false);
            decReg(regWidth, ecx, false);
            Goto(label);
        } EndIf(true);
    } StartElse(true); {
        U32 label = MarkJumpLocation();
        If(regWidth, ecx, true); {
            write(valueWidth, edi, read(valueWidth, esi));
            addValue(regWidth, esi, size, false);
            addValue(regWidth, edi, size, false);
            decReg(regWidth, ecx, false);
            Goto(label);
        } EndIf(true);
    }
    EndIf(true);
}

void DynamicData::dynamic_movsb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call_I(movsb16r, op->base);
        } else {
            movs(op->base, DYN_8bit, 1, DYN_16bit);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                call_I(movsb32r, op->base);
            } else {
                movsr(DYN_8bit, 1, DYN_32bit);
            }
        } else { 
            movs(op->base, DYN_8bit, 1, DYN_32bit);
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_movsw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call_I(movsw16r, op->base);
        } else {
            movs(op->base, DYN_16bit, 2, DYN_16bit);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                call_I(movsw32r, op->base);
            } else {
                movsr(DYN_16bit, 2, DYN_32bit);
            }
        } else {
            movs(op->base, DYN_16bit, 2, DYN_32bit);
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_movsd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call_I(movsd16r, op->base);
        } else {
            movs(op->base, DYN_32bit, 4, DYN_16bit);
        }
    } else {
        if (op->repZero || op->repNotZero) {
            if (cpu->thread->process->hasSetSeg[ES] || cpu->thread->process->hasSetSeg[op->base]) {
                call_I(movsd32r, op->base);
            } else {
                movsr(DYN_32bit, 4, DYN_32bit);
            }
        } else {
            movs(op->base, DYN_32bit, 4, DYN_32bit);
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpsb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call_II(cmpsb16r, op->repZero, op->base);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else { 
            call_II(cmpsb16, op->repZero, op->base);
            currentLazyFlags=FLAGS_SUB8;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            call_II(cmpsb32r, op->repZero, op->base);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else { 
            call_II(cmpsb32, op->repZero, op->base);
            currentLazyFlags=FLAGS_SUB8;
        }
    }    
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpsw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call_II(cmpsw16r, op->repZero, op->base);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else { 
            call_II(cmpsw16, op->repZero, op->base);
            currentLazyFlags=FLAGS_SUB16;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            call_II(cmpsw32r, op->repZero, op->base);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else { 
            call_II(cmpsw32, op->repZero, op->base);
            currentLazyFlags=FLAGS_SUB16;
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpsd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call_II(cmpsd16r, op->repZero, op->base);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else { 
            call_II(cmpsd16, op->repZero, op->base);
            currentLazyFlags=FLAGS_SUB32;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            call_II(cmpsd32r, op->repZero, op->base);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else { 
            call_II(cmpsd32, op->repZero, op->base);
            currentLazyFlags=FLAGS_SUB32;
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_stosb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call(stosb16r);
        } else { 
            call(stosb16);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            call(stosb32r);
        } else { 
            call(stosb32);
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_stosw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call(stosw16r);
        } else { 
            call(stosw16);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            call(stosw32r);
        } else { 
            call(stosw32);
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_stosd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call(stosd16r);
        } else { 
            call(stosd16);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            call(stosd32r);
        } else { 
            call(stosd32);
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_lodsb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call_I(lodsb16r, op->base);
        } else { 
            call_I(lodsb16, op->base);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            call_I(lodsb32r, op->base);
        } else { 
            call_I(lodsb32, op->base);
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_lodsw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call_I(lodsw16r, op->base);
        } else { 
            call_I(lodsw16, op->base);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            call_I(lodsw32r, op->base);
        } else { 
            call_I(lodsw32, op->base);
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_lodsd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call_I(lodsd16r, op->base);
        } else { 
            call_I(lodsd16, op->base);
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            call_I(lodsd32r, op->base);
        } else { 
            call_I(lodsd32, op->base);
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_scasb_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call_I(scasb16r, op->repZero);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else { 
            call_I(scasb16, op->repZero);
            currentLazyFlags=FLAGS_SUB8;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            call_I(scasb32r, op->repZero);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB8 if (e)cx is 0
        } else { 
            call_I(scasb32, op->repZero);
            currentLazyFlags=FLAGS_SUB8;
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_scasw_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call_I(scasw16r, op->repZero);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else { 
            call_I(scasw16, op->repZero);
            currentLazyFlags=FLAGS_SUB16;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            call_I(scasw32r, op->repZero);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB16 if (e)cx is 0
        } else { 
            call_I(scasw32, op->repZero);
            currentLazyFlags=FLAGS_SUB16;
        }
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_scasd_op(DecodedOp* op) {
    if (op->ea16) {
        if (op->repZero || op->repNotZero) {
            call_I(scasd16r, op->repZero);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else { 
            call_I(scasd16, op->repZero);
            currentLazyFlags=FLAGS_SUB32;
        }
    } else { 
        if (op->repZero || op->repNotZero) {
            call_I(scasd32r, op->repZero);
            currentLazyFlags = nullptr; // not set to FLAGS_SUB32 if (e)cx is 0
        } else { 
            call_I(scasd32, op->repZero);
            currentLazyFlags=FLAGS_SUB32;
        }
    }
    incrementEip(op->len);
}
