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

void Jit::push16(RegPtr reg) {
    // U32 new_esp = (THIS_ESP & this->stackNotMask) | ((THIS_ESP - 2) & this->stackMask);
    // memory->writew(this->seg[SS].address + (new_esp & this->stackMask), value);
    // THIS_ESP = new_esp;

    RegPtr address = getTmpReg(4);

    IfSmallStack(true); {
        andValue(JitWidth::b32, address, 0xFFFF);
        subValue(JitWidth::b16, address, 2);
        addReg(JitWidth::b32, address, getReadOnlySegAddress(SS));
        write(JitWidth::b16, address, reg);
        subValue(JitWidth::b16, getReg(4), 2);
    } StartElse(true); {
        subValue(JitWidth::b32, address, 2);
        addReg(JitWidth::b32, address, getReadOnlySegAddress(SS));
        write(JitWidth::b16, address, reg);
        subValue(JitWidth::b32, getReg(4), 2);
    } EndIf(true);
}

void Jit::push32(RegPtr reg) {
    if (!cpu->thread->process->hasSetStackMask && !cpu->thread->process->hasSetSeg[SS]) {
        RegPtr address = getTmpReg(4);
        subValue(JitWidth::b32, address, 4);
        write(JitWidth::b32, address, reg);
        subValue(JitWidth::b32, getReg(4), 4);
    } else {
        RegPtr address = getTmpReg(4);

        IfSmallStack(true); {
            andValue(JitWidth::b32, address, 0xFFFF);
            subValue(JitWidth::b16, address, 4);
            addReg(JitWidth::b32, address, getReadOnlySegAddress(SS));
            write(JitWidth::b32, address, reg);
            subValue(JitWidth::b16, getReg(4), 4);
        } StartElse(true); {
            subValue(JitWidth::b32, address, 4);
            addReg(JitWidth::b32, address, getReadOnlySegAddress(SS));
            write(JitWidth::b32, address, reg);
            subValue(JitWidth::b32, getReg(4), 4);
        } EndIf(true);
    }
}

RegPtr Jit::pop32(RegPtr reg, U32 amount) {
    if (!reg) {
        reg = getTmpReg();
    }
    mov(JitWidth::b32, reg, peek32());
    IfSmallStack(); {
        addValue(JitWidth::b16, getReg(4), amount);
    } StartElse(); {
        addValue(JitWidth::b32, getReg(4), amount);
    } EndIf();
    return reg;
}

RegPtr Jit::pop16(RegPtr reg, U32 amount) {
    if (!reg) {
        reg = getTmpReg();
    }
    mov(JitWidth::b16, reg, peek16());
    IfSmallStack(); {
        addValue(JitWidth::b16, getReg(4), amount);
    } StartElse(); {
        addValue(JitWidth::b32, getReg(4), amount);
    } EndIf();
    return reg;
}

RegPtr Jit::peek16(RegPtr resultReg) {
    RegPtr address = getTmpReg(4);

    IfSmallStack(); {
        andValue(JitWidth::b32, address, 0xFFFF);
    } EndIf();
    addReg(JitWidth::b32, address, getReadOnlySegAddress(SS));
    return read(JitWidth::b16, address, nullptr, nullptr, false, resultReg);
}
RegPtr Jit::peek32(RegPtr resultReg) {
    RegPtr address = getTmpReg(4);

    IfSmallStack(); {
        andValue(JitWidth::b32, address, 0xFFFF);
    } EndIf();
    addReg(JitWidth::b32, address, getReadOnlySegAddress(SS));
    return read(JitWidth::b32, address, nullptr, nullptr, false, resultReg);
}
void Jit::dynamic_pushEw_reg(DecodedOp* op) {
    push16(getReadOnlyReg(op->reg));
    incrementEip(op->len);
}
void Jit::dynamic_popEw_reg(DecodedOp* op) {
    pop16(getReg(op->reg));
    incrementEip(op->len);
}
void Jit::dynamic_pushEw_mem(DecodedOp* op) {
    push16(read(JitWidth::b16, calculateEaa(op)));
    incrementEip(op->len);
}
void Jit::dynamic_popEw_mem(DecodedOp* op) {
    write(JitWidth::b16, calculateEaa(op, 2), peek16()); // eaa must be calculated after esp is incremented which is why we pass 2 here

    IfSmallStack(); {
        addValue(JitWidth::b16, getReg(4), 2);
    } StartElse(); {
        addValue(JitWidth::b32, getReg(4), 2);
    } EndIf();
    incrementEip(op->len);
}
void Jit::dynamic_pushEd_reg(DecodedOp* op) {
    push32(getReadOnlyReg(op->reg));
    incrementEip(op->len);
}
void Jit::dynamic_popEd_reg(DecodedOp* op) {
    pop32(getReg(op->reg));
    incrementEip(op->len);
}
void Jit::dynamic_pushEd_mem(DecodedOp* op) {
    push32(read(JitWidth::b32, calculateEaa(op)));
    incrementEip(op->len);
}
void Jit::dynamic_popEd_mem(DecodedOp* op) {
    write(JitWidth::b32, calculateEaa(op, 4), peek32()); // eaa must be calculated after esp is incremented which is why we pass 2 here

    IfSmallStack();
    addValue(JitWidth::b16, getReg(4), 4);
    StartElse();
    addValue(JitWidth::b32, getReg(4), 4);
    EndIf();
    incrementEip(op->len);
}
void Jit::dynamic_pushSeg16(DecodedOp* op) {
    push16(getReadOnlySegValue(op->reg));
    incrementEip(op->len);
}
void Jit::dynamic_popSeg16(DecodedOp* op) {
    RegPtr result = callAndReturn_IR(common_setSegment, op->reg, JitWidth::b16, peek16(getTmpReg())); // getTmpReg so that peek16 won't use hardware EAX on x86, callAndReturn needs that. :TODO: maybe callAndReturn can be enhanced to allow a param using EAX but that doesn't need it after its used as a param.

    IfNot(JitWidth::b32, result);
    blockDone(true);
    EndIf();

    IfSmallStack(); {
        addValue(JitWidth::b16, getReg(4), 2);
    } StartElse(); {
        addValue(JitWidth::b32, getReg(4), 2);
    } EndIf();

    incrementEip(op->len);
}
void Jit::dynamic_pushSeg32(DecodedOp* op) {
    push32(getReadOnlySegValue(op->reg));
    incrementEip(op->len);
}
void Jit::dynamic_popSeg32(DecodedOp* op) {
    RegPtr result = callAndReturn_IR(common_setSegment, op->reg, JitWidth::b32, peek32(getTmpReg())); // getTmpReg so that peek16 won't use hardware EAX on x86, callAndReturn needs that. :TODO: maybe callAndReturn can be enhanced to allow a param using EAX but that doesn't need it after its used as a param.

    IfNot(JitWidth::b32, result); {
        blockDone(true);
    } EndIf();

    IfSmallStack(); {
        addValue(JitWidth::b16, getReg(4), 4);
    } StartElse(); {
        addValue(JitWidth::b32, getReg(4), 4);
    } EndIf();

    incrementEip(op->len);
}
void Jit::dynamic_pushA16(DecodedOp* op) {
    RegPtr esp = getTmpReg(4);

    IfSmallStack(); {
        andValue(JitWidth::b32, esp, 0xFFFF);
    } EndIf();
    addReg(JitWidth::b32, esp, getReadOnlySegAddress(SS));
    subValue(JitWidth::b32, esp, 16);

    // 8x 2 byte pushes is 128 bits, if we have permission and space on one page to write this, then we only need to do the memory checks once
    write(JitWidth::b128, esp, nullptr, [this](RegPtr address, RegPtr offset) {
        for (int i = 0; i < 8; i++) {
            write(JitWidth::b16, address, offset, 0, 2 * i, getReadOnlyReg(7 - i));
        }
        IfSmallStack(); {
            subValue(JitWidth::b16, getReg(4), 16);
        } StartElse(); {
            subValue(JitWidth::b32, getReg(4), 16);
        }EndIf();
    }, [this]() {
        call(common_pushA16);
    });
    incrementEip(op->len);
}
void Jit::dynamic_pushA32(DecodedOp* op) {
    RegPtr esp = getTmpReg(4);

    IfSmallStack(); {
        andValue(JitWidth::b32, esp, 0xFFFF);
    }EndIf();
    addReg(JitWidth::b32, esp, getReadOnlySegAddress(SS));
    subValue(JitWidth::b32, esp, 32);

    // 8x 4 byte pushes is 256 bits, if we have permission and space on one page to write this, then we only need to do the memory checks once
    write(JitWidth::b256, esp, nullptr, [this](RegPtr address, RegPtr offset) {
        for (int i = 0; i < 8; i++) {
            write(JitWidth::b32, address, offset, 0, 4 * i, getReadOnlyReg(7 - i));
        }
        IfSmallStack(); {
            subValue(JitWidth::b16, getReg(4), 32);
        } StartElse(); {
            subValue(JitWidth::b32, getReg(4), 32);
        } EndIf();
    }, [this]() {
        call(common_pushA32);
    });
    incrementEip(op->len);
}
void Jit::dynamic_popA16(DecodedOp* op) {
    RegPtr esp = getTmpReg(4);

    IfSmallStack(); {
        andValue(JitWidth::b32, esp, 0xFFFF);
    } EndIf();
    addReg(JitWidth::b32, esp, getReadOnlySegAddress(SS));

    // 8x 2 byte is 128 bits, if we have permission and space on one page to read this, then we only need to do the memory checks once
    read(JitWidth::b128, esp, [this](RegPtr address, RegPtr offset) {
        for (int i = 0; i < 8; i++) {
            if (i != 3) {
                read(JitWidth::b16, getReg(7 - i), address, offset, 0, 2 * i);
            }
        }
        IfSmallStack(); {
            addValue(JitWidth::b16, getReg(4), 16);
        } StartElse(); {
            addValue(JitWidth::b32, getReg(4), 16);
        } EndIf();
    }, [this]() {
        call(common_popA16);
    });
    incrementEip(op->len);
}
void Jit::dynamic_popA32(DecodedOp* op) {
    RegPtr esp = getTmpReg(4);

    IfSmallStack(); {
        andValue(JitWidth::b32, esp, 0xFFFF);
    } EndIf();
    addReg(JitWidth::b32, esp, getReadOnlySegAddress(SS));

    // 8x 4 byte is 256 bits, if we have permission and space on one page to read this, then we only need to do the memory checks once
    read(JitWidth::b256, esp, [this](RegPtr address, RegPtr offset) {
        for (int i = 0; i < 8; i++) {
            if (i != 3) {
                read(JitWidth::b32, getReg(7 - i), address, offset, 0, 4 * i);
            }
        }
        IfSmallStack(); {
            addValue(JitWidth::b16, getReg(4), 32);
        } StartElse(); {
            addValue(JitWidth::b32, getReg(4), 32);
        } EndIf();
    }, [this]() {
        call(common_popA32);
    });
    incrementEip(op->len);
}
void Jit::dynamic_push16imm(DecodedOp* op) {
    RegPtr reg = getTmpReg();
    movValue(JitWidth::b16, reg, op->imm);
    push16(reg);
    incrementEip(op->len);
}
void Jit::dynamic_push32imm(DecodedOp* op) {
    RegPtr reg = getTmpReg();
    movValue(JitWidth::b32, reg, op->imm);
    push32(reg);
    incrementEip(op->len);
}
void Jit::dynamic_popf16(DecodedOp* op) {
    setFlags(pop16(), FMASK_ALL & 0xFFFF);
    incrementEip(op->len);
}
void Jit::dynamic_popf32(DecodedOp* op) {
    setFlags(pop32(), FMASK_ALL);
    incrementEip(op->len);
}
void Jit::dynamic_pushf16(DecodedOp* op) {
    push16(getReadOnlyFlags());
    incrementEip(op->len);
}
void Jit::dynamic_pushf32(DecodedOp* op) {
    push32(getReadOnlyFlags());
    incrementEip(op->len);
}
