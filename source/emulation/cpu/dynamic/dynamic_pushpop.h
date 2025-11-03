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

void DynamicData::push16(RegPtr reg) {
    // U32 new_esp = (THIS_ESP & this->stackNotMask) | ((THIS_ESP - 2) & this->stackMask);
    // memory->writew(this->seg[SS].address + (new_esp & this->stackMask), value);
    // THIS_ESP = new_esp;

    RegPtr address = getTmpReg(4);

    IfSmallStack();
        andValue(DYN_32bit, address, 0xFFFF, false);
        subValue(DYN_16bit, address, 2, false);
        addReg(DYN_32bit, address, getReadOnlySegAddress(SS), false);
        write(DYN_16bit, address, reg);
        subValue(DYN_16bit, getReg(4), 2, false);
    StartElse();
        subValue(DYN_32bit, address, 2, false);
        addReg(DYN_32bit, address, getReadOnlySegAddress(SS), false);
        write(DYN_16bit, address, reg);
        subValue(DYN_32bit, getReg(4), 2, false);
    EndIf();
}

void DynamicData::push32(RegPtr reg) {
    if (!cpu->thread->process->hasSetStackMask && !cpu->thread->process->hasSetSeg[SS]) {
        RegPtr address = getTmpReg(4);
        subValue(DYN_32bit, address, 4, false);
        write(DYN_32bit, address, reg);
        subValue(DYN_32bit, getReg(4), 4, false);
    } else {
        RegPtr address = getTmpReg(4);

        IfSmallStack();
            andValue(DYN_32bit, address, 0xFFFF, false);
            subValue(DYN_16bit, address, 4, false);
            addReg(DYN_32bit, address, getReadOnlySegAddress(SS), false);
            write(DYN_32bit, address, reg);
            subValue(DYN_16bit, getReg(4), 4, false);
        StartElse();
            subValue(DYN_32bit, address, 4, false);
            addReg(DYN_32bit, address, getReadOnlySegAddress(SS), false);
            write(DYN_32bit, address, reg);
            subValue(DYN_32bit, getReg(4), 4, false);
        EndIf();
    }
}

RegPtr DynamicData::pop32(RegPtr reg, U32 amount) {
    if (!reg) {
        reg = getTmpReg();
    }
    mov(DYN_32bit, reg, peek32());
    IfSmallStack();
        addValue(DYN_16bit, getReg(4), amount, false);
    StartElse();
        addValue(DYN_32bit, getReg(4), amount, false);
    EndIf();
    return reg;
}

RegPtr DynamicData::pop16(RegPtr reg, U32 amount) {
    if (!reg) {
        reg = getTmpReg();
    }
    mov(DYN_16bit, reg, peek16());
    IfSmallStack();
        addValue(DYN_16bit, getReg(4), amount, false);
    StartElse();
        addValue(DYN_32bit, getReg(4), amount, false);
    EndIf();
    return reg;
}

RegPtr DynamicData::peek16(RegPtr resultReg) {
    RegPtr address = getTmpReg(4);

    IfSmallStack();
        andValue(DYN_32bit, address, 0xFFFF, false);
    EndIf();
    addReg(DYN_32bit, address, getReadOnlySegAddress(SS), false);
    return read(DYN_16bit, address, nullptr, nullptr, false, resultReg);
}
RegPtr DynamicData::peek32(RegPtr resultReg) {
    RegPtr address = getTmpReg(4);

    IfSmallStack();
        andValue(DYN_32bit, address, 0xFFFF, false);
    EndIf();
    addReg(DYN_32bit, address, getReadOnlySegAddress(SS), false);
    return read(DYN_32bit, address, nullptr, nullptr, false, resultReg);
}
void DynamicData::dynamic_pushEw_reg(DecodedOp* op) {    
    push16(getReadOnlyReg(op->reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_popEw_reg(DecodedOp* op) {
    pop16(getReg(op->reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_pushEw_mem(DecodedOp* op) {
    push16(read(DYN_16bit, calculateEaa2(op)));
    incrementEip(op->len);
}
void DynamicData::dynamic_popEw_mem(DecodedOp* op) {
    write(DYN_16bit, calculateEaa2(op, 2), peek16()); // eaa must be calculated after esp is incremented which is why we pass 2 here

    IfSmallStack();
        addValue(DYN_16bit, getReg(4), 2, false);        
    StartElse();
        addValue(DYN_32bit, getReg(4), 2, false);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_pushEd_reg(DecodedOp* op) {
    push32(getReadOnlyReg(op->reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_popEd_reg(DecodedOp* op) {
    pop32(getReg(op->reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_pushEd_mem(DecodedOp* op) {
    push32(read(DYN_32bit, calculateEaa2(op)));
    incrementEip(op->len);
}
void DynamicData::dynamic_popEd_mem(DecodedOp* op) {
    write(DYN_32bit, calculateEaa2(op, 4), peek32()); // eaa must be calculated after esp is incremented which is why we pass 2 here

    IfSmallStack();
        addValue(DYN_16bit, getReg(4), 4, false);
    StartElse();
        addValue(DYN_32bit, getReg(4), 4, false);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_pushSeg16(DecodedOp* op) {
    push16(getReadOnlySegValue(op->reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_popSeg16(DecodedOp* op) {
    cpu->thread->process->hasSetSeg[op->reg] = true;
    RegPtr result = callAndReturn_IR(common_setSegment, op->reg, DYN_16bit, peek16(getTmpReg())); // getTmpReg so that peek16 won't use hardware EAX on x86, callAndReturn needs that. :TODO: maybe callAndReturn can be enhanced to allow a param using EAX but that doesn't need it after its used as a param.

    IfNot(DYN_32bit, result);
        blockDone(true);
    EndIf();

    IfSmallStack();
        addValue(DYN_16bit, getReg(4), 2, false);
    StartElse();
        addValue(DYN_32bit, getReg(4), 2, false);
    EndIf();

    incrementEip(op->len);
}
void DynamicData::dynamic_pushSeg32(DecodedOp* op) {
    push32(getReadOnlySegValue(op->reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_popSeg32(DecodedOp* op) {
    cpu->thread->process->hasSetSeg[op->reg] = true;
    RegPtr result = callAndReturn_IR(common_setSegment, op->reg, DYN_32bit, peek32(getTmpReg())); // getTmpReg so that peek16 won't use hardware EAX on x86, callAndReturn needs that. :TODO: maybe callAndReturn can be enhanced to allow a param using EAX but that doesn't need it after its used as a param.

    IfNot(DYN_32bit, result);
        blockDone(true);
    EndIf();

    IfSmallStack();
        addValue(DYN_16bit, getReg(4), 4, false);
    StartElse();
        addValue(DYN_32bit, getReg(4), 4, false);
    EndIf();

    incrementEip(op->len);
}
void DynamicData::dynamic_pushA16(DecodedOp* op) {
    RegPtr esp = getTmpReg(4);

    IfSmallStack();
        andValue(DYN_32bit, esp, 0xFFFF, false);
    EndIf();        
    addReg(DYN_32bit, esp, getReadOnlySegAddress(SS), false);
    subValue(DYN_32bit, esp, 16, false);

    // 8x 2 byte pushes is 128 bits, if we have permission and space on one page to write this, then we only need to do the memory checks once
    write(DYN_128bit, esp, nullptr, [this](RegPtr address, RegPtr offset) {
        for (int i = 0; i < 8; i++) {
            write(DYN_16bit, address, offset, 0, 2 * i, getReadOnlyReg(7 - i));
        }
        IfSmallStack();
            subValue(DYN_16bit, getReg(4), 16, false);
        StartElse();
            subValue(DYN_32bit, getReg(4), 16, false);
        EndIf();
    }, [this]() {
        call(common_pushA16);
    });
    incrementEip(op->len);
}
void DynamicData::dynamic_pushA32(DecodedOp* op) {
    RegPtr esp = getTmpReg(4);

    IfSmallStack();
        andValue(DYN_32bit, esp, 0xFFFF, false);
    EndIf();
    addReg(DYN_32bit, esp, getReadOnlySegAddress(SS), false);
    subValue(DYN_32bit, esp, 32, false);

    // 8x 4 byte pushes is 256 bits, if we have permission and space on one page to write this, then we only need to do the memory checks once
    write(DYN_256bit, esp, nullptr, [this](RegPtr address, RegPtr offset) {
        for (int i = 0; i < 8; i++) {
            write(DYN_32bit, address, offset, 0, 4 * i, getReadOnlyReg(7 - i));
        }
        IfSmallStack();
            subValue(DYN_16bit, getReg(4), 32, false);
        StartElse();
            subValue(DYN_32bit, getReg(4), 32, false);
        EndIf();
    }, [this]() {
        call(common_pushA32);
    });
    incrementEip(op->len);
}
void DynamicData::dynamic_popA16(DecodedOp* op) {
    RegPtr esp = getTmpReg(4);

    IfSmallStack();
        andValue(DYN_32bit, esp, 0xFFFF, false);
    EndIf();
    addReg(DYN_32bit, esp, getReadOnlySegAddress(SS), false);

    // 8x 2 byte is 128 bits, if we have permission and space on one page to read this, then we only need to do the memory checks once
    read(DYN_128bit, esp, [this](RegPtr address, RegPtr offset) {
        for (int i = 0; i < 8; i++) {
            if (i != 3) {
                read(DYN_16bit, getReg(7 - i), address, offset, 0, 2 * i);
            }
        }
        IfSmallStack();
            addValue(DYN_16bit, getReg(4), 16, false);
        StartElse();
            addValue(DYN_32bit, getReg(4), 16, false);
        EndIf();
    }, [this]() {
        call(common_popA16);
    });
    incrementEip(op->len);
}
void DynamicData::dynamic_popA32(DecodedOp* op) {
    RegPtr esp = getTmpReg(4);

    IfSmallStack();
    andValue(DYN_32bit, esp, 0xFFFF, false);
    EndIf();
    addReg(DYN_32bit, esp, getReadOnlySegAddress(SS), false);

    // 8x 4 byte is 256 bits, if we have permission and space on one page to read this, then we only need to do the memory checks once
    read(DYN_256bit, esp, [this](RegPtr address, RegPtr offset) {
        for (int i = 0; i < 8; i++) {
            if (i != 3) {
                read(DYN_32bit, getReg(7 - i), address, offset, 0, 4 * i);
            }
        }
        IfSmallStack();
        addValue(DYN_16bit, getReg(4), 32, false);
        StartElse();
        addValue(DYN_32bit, getReg(4), 32, false);
        EndIf();
    }, [this]() {
        call(common_popA32);
    });
    incrementEip(op->len);
}
void DynamicData::dynamic_push16imm(DecodedOp* op) {
    RegPtr reg = getTmpReg();
    movValue(DYN_16bit, reg, op->imm);
    push16(reg);
    incrementEip(op->len);
}
void DynamicData::dynamic_push32imm(DecodedOp* op) {
    RegPtr reg = getTmpReg();
    movValue(DYN_32bit, reg, op->imm);
    push32(reg);
    incrementEip(op->len);
}
void DynamicData::dynamic_popf16(DecodedOp* op) {
    storeLazyFlags(FLAGS_NONE);
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    callHostFunction((void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, FMASK_ALL & 0xFFFF, DYN_PARAM_CONST_16, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_popf32(DecodedOp* op) {
    storeLazyFlags(FLAGS_NONE);
    callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
    callHostFunction((void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, FMASK_ALL, DYN_PARAM_CONST_32, false);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_pushf16(DecodedOp* op) {
    dynamic_fillFlags();
    loadCPUFlags(DYN_SRC);
    orRegImm(DYN_SRC, DYN_32bit, 2);
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_pushf32(DecodedOp* op) {
    push32(getReadOnlyFlags());
    incrementEip(op->len);
}
