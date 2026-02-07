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

    IfSmallStack(); {
        andValue(JitWidth::b32, address, 0xFFFF);
        subValue(JitWidth::b16, address, 2);
        addReg(JitWidth::b32, address, getReadOnlySegAddress(SS));
        write(JitWidth::b16, address, reg, nullptr, nullptr, false);
        subValue(JitWidth::b16, getReg(4), 2);
    } StartElse(); {
        subValue(JitWidth::b32, address, 2);
        addReg(JitWidth::b32, address, getReadOnlySegAddress(SS));
        write(JitWidth::b16, address, reg, nullptr, nullptr, false);
        subValue(JitWidth::b32, getReg(4), 2);
    } EndIf();
}

void Jit::push32(RegPtr reg) {
    if (!cpu->thread->process->hasSetStackMask && !cpu->thread->process->hasSetSeg[SS]) {
        RegPtr esp = getReg(4);
        RegPtr address = getTmpReg();
        mov(JitWidth::b32, address, esp);
        subValue(JitWidth::b32, address, 4);
        write(JitWidth::b32, address, reg, nullptr, nullptr, false);
        subValue(JitWidth::b32, esp, 4);
    } else {
        RegPtr address = getTmpReg(4);

        IfSmallStack(); {
            andValue(JitWidth::b32, address, 0xFFFF);
            subValue(JitWidth::b16, address, 4);
            addReg(JitWidth::b32, address, getReadOnlySegAddress(SS));
            write(JitWidth::b32, address, reg); // don't skip alignment check, firefight installer will crash
            subValue(JitWidth::b16, getReg(4), 4);
        } StartElse(); {
            subValue(JitWidth::b32, address, 4);
            addReg(JitWidth::b32, address, getReadOnlySegAddress(SS));
            write(JitWidth::b32, address, reg, nullptr, nullptr, false);
            subValue(JitWidth::b32, getReg(4), 4);
        } EndIf();
    }
}

RegPtr Jit::pop32(RegPtr reg, U32 amount) {
    if (!reg) {
        reg = getTmpReg();
    }
    mov(JitWidth::b32, reg, peek32());
    if (reg->emulatedReg != 4) {
        if (cpu->thread->process->hasSetSeg[SS]) {
            IfSmallStack(); {
                addValue(JitWidth::b16, getReg(4), amount);
            } StartElse(); {
                addValue(JitWidth::b32, getReg(4), amount);
            } EndIf();
        } else {
            addValue(JitWidth::b32, getReg(4), amount);
        }
    }
    return reg;
}

RegPtr Jit::pop16(RegPtr reg, U32 amount) {
    if (!reg) {
        reg = getTmpReg();
    }
    mov(JitWidth::b16, reg, peek16());
    if (reg->emulatedReg != 4) {
        IfSmallStack(); {
            addValue(JitWidth::b16, getReg(4), amount);
        } StartElse(); {
            addValue(JitWidth::b32, getReg(4), amount);
        } EndIf();
    }
    return reg;
}

RegPtr Jit::peek16(RegPtr resultReg) {
    RegPtr address = getTmpReg(4);

    IfSmallStack(); {
        andValue(JitWidth::b32, address, 0xFFFF);
    } EndIf();
    addReg(JitWidth::b32, address, getReadOnlySegAddress(SS));
    return read(JitWidth::b16, address, nullptr, nullptr, resultReg, false);
}
RegPtr Jit::peek32(RegPtr resultReg) {
    RegPtr address = getTmpReg(4);

    if (cpu->thread->process->hasSetSeg[SS]) {
        IfSmallStack(); {
            andValue(JitWidth::b32, address, 0xFFFF);
        } EndIf();
    }
    addReg(JitWidth::b32, address, getReadOnlySegAddress(SS));
    return read(JitWidth::b32, address, nullptr, nullptr, resultReg, cpu->seg[SS].address != 0);
}
void Jit::dynamic_pushEw_reg(DecodedOp* op) {
    push16(getReadOnlyReg(op->reg));
}
void Jit::dynamic_popEw_reg(DecodedOp* op) {
    pop16(getReg(op->reg));
}
void Jit::dynamic_pushEw_mem(DecodedOp* op) {
    push16(read(JitWidth::b16, calculateEaa(op)));
}
void Jit::dynamic_popEw_mem(DecodedOp* op) {
    write(JitWidth::b16, calculateEaa(op, 2), peek16()); // eaa must be calculated after esp is incremented which is why we pass 2 here

    IfSmallStack(); {
        addValue(JitWidth::b16, getReg(4), 2);
    } StartElse(); {
        addValue(JitWidth::b32, getReg(4), 2);
    } EndIf();
}
void Jit::dynamic_pushEd_reg(DecodedOp* op) {
    push32(getReadOnlyReg(op->reg));
}
void Jit::dynamic_popEd_reg(DecodedOp* op) {
    pop32(getReg(op->reg));
}
void Jit::dynamic_pushEd_mem(DecodedOp* op) {
    push32(read(JitWidth::b32, calculateEaa(op)));
}
void Jit::dynamic_popEd_mem(DecodedOp* op) {
    write(JitWidth::b32, calculateEaa(op, 4), peek32()); // eaa must be calculated after esp is incremented which is why we pass 4 here

    if (cpu->thread->process->hasSetSeg[SS]) {
        IfSmallStack(); {
            addValue(JitWidth::b16, getReg(4), 4);
        } StartElse(); {
            addValue(JitWidth::b32, getReg(4), 4);
        } EndIf();
    } else {
        addValue(JitWidth::b32, getReg(4), 4);
    }
}
void Jit::dynamic_pushSeg16(DecodedOp* op) {
    push16(getReadOnlySegValue(op->reg));
}
void Jit::dynamic_popSeg16(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_pushSeg32(DecodedOp* op) {
    push32(getReadOnlySegValue(op->reg));
}
void Jit::dynamic_popSeg32(DecodedOp* op) {
    emulateSingleOp();
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
            writeHost(JitWidth::b16, address, offset, 0, 2 * i, getReadOnlyReg(7 - i));
        }
        IfSmallStack(); {
            subValue(JitWidth::b16, getReg(4), 16);
        } StartElse(); {
            subValue(JitWidth::b32, getReg(4), 16);
        }EndIf();
    });
}
void Jit::dynamic_pushA32(DecodedOp* op) {
    RegPtr esp = getTmpReg(4);

    if (cpu->thread->process->hasSetSeg[SS]) {
        IfSmallStack(); {
            andValue(JitWidth::b32, esp, 0xFFFF);
        } EndIf();
    }
    addReg(JitWidth::b32, esp, getReadOnlySegAddress(SS));
    subValue(JitWidth::b32, esp, 32);

    // 8x 4 byte pushes is 256 bits, if we have permission and space on one page to write this, then we only need to do the memory checks once
    write(JitWidth::b256, esp, nullptr, [this](RegPtr address, RegPtr offset) {
        for (int i = 0; i < 8; i++) {
            writeHost(JitWidth::b32, address, offset, 0, 4 * i, getReadOnlyReg(7 - i));
        }
        if (cpu->thread->process->hasSetSeg[SS]) {
            IfSmallStack(); {
                subValue(JitWidth::b16, getReg(4), 32);
            } StartElse(); {
                subValue(JitWidth::b32, getReg(4), 32);
            } EndIf();
        } else {
            subValue(JitWidth::b32, getReg(4), 32);
        }
    });
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
                readHost(JitWidth::b16, getReg(7 - i), address, offset, 0, 2 * i);
            }
        }
        IfSmallStack(); {
            addValue(JitWidth::b16, getReg(4), 16);
        } StartElse(); {
            addValue(JitWidth::b32, getReg(4), 16);
        } EndIf();
    });
}
void Jit::dynamic_popA32(DecodedOp* op) {
    RegPtr esp = getTmpReg(4);

    if (cpu->thread->process->hasSetSeg[SS]) {
        IfSmallStack(); {
            andValue(JitWidth::b32, esp, 0xFFFF);
        } EndIf();
    }
    addReg(JitWidth::b32, esp, getReadOnlySegAddress(SS));

    // 8x 4 byte is 256 bits, if we have permission and space on one page to read this, then we only need to do the memory checks once
    read(JitWidth::b256, esp, [this](RegPtr address, RegPtr offset) {
        for (int i = 0; i < 8; i++) {
            if (i != 3) {
                readHost(JitWidth::b32, getReg(7 - i), address, offset, 0, 4 * i);
            }
        }
        if (cpu->thread->process->hasSetSeg[SS]) {
            IfSmallStack(); {
                addValue(JitWidth::b16, getReg(4), 32);
            } StartElse(); {
                addValue(JitWidth::b32, getReg(4), 32);
            } EndIf();
        } else {
            addValue(JitWidth::b32, getReg(4), 32);
        }
    });
}
void Jit::dynamic_push16imm(DecodedOp* op) {
    RegPtr reg = getTmpReg();
    movValue(JitWidth::b16, reg, op->imm);
    push16(reg);
}
void Jit::dynamic_push32imm(DecodedOp* op) {
    RegPtr reg = getTmpReg();
    movValue(JitWidth::b32, reg, op->imm);
    push32(reg);
}
void Jit::dynamic_popf16(DecodedOp* op) {
    setFlags(pop16(), FMASK_ALL & 0xFFFF);
}
void Jit::dynamic_popf32(DecodedOp* op) {
    setFlags(pop32(), FMASK_ALL);
}
void Jit::dynamic_pushf16(DecodedOp* op) {
    fillFlags();
    push16(getReadOnlyFlags());
}
void Jit::dynamic_pushf32(DecodedOp* op) {
    fillFlags();
    push32(getReadOnlyFlags());
}
