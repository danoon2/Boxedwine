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

void DynamicData::dynamic_pushEw_reg(DecodedOp* op) {
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_popEw_reg(DecodedOp* op) {
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    storeReg(op->reg, DYN_CALL_RESULT, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_pushEw_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_popEw_mem(DecodedOp* op) {    
    // save current ESP
    loadReg(4, DYN_SRC, DYN_32bit, true);
    // pop stack
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    // calculate write address
    calculateEaa(op, DYN_ADDRESS);
    // set ESP back to original ESP before it was incremented in case the write throws an exception
    loadReg(4, DYN_DEST, DYN_32bit, true);
    storeReg(4, DYN_SRC, DYN_32bit, true);
    // do write
    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_SRC);
    // restore to calculated ESP from common_pop16 after write succeeds
    storeReg(4, DYN_DEST, DYN_32bit, true);

    incrementEip(op->len);
}
void DynamicData::dynamic_pushEd_reg(DecodedOp* op) {
    if (!cpu->thread->process->hasSetStackMask && !cpu->thread->process->hasSetSeg[SS]) {
        loadReg(4, DYN_ADDRESS, DYN_32bit, true);
        subRegImm(DYN_ADDRESS, DYN_32bit, 4);
        DynReg reg = loadReg(op->reg, DYN_SRC, DYN_32bit);
        movToMemFromReg(DYN_ADDRESS, reg, DYN_32bit, true, true, DYN_DEST); // need to discard DYN_ADDRESS, otherwise will be out of regs
        subCPUImm(4, DYN_32bit, 4, DYN_ADDRESS);
    } else {
        callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_popEd_reg(DecodedOp* op) {
    if (!cpu->thread->process->hasSetStackMask && !cpu->thread->process->hasSetSeg[SS]) {
        loadReg(4, DYN_ADDRESS, DYN_32bit, true);
        addCPUImm(4, DYN_32bit, 4, DYN_DEST); // increment before assign, in case esp=pop32()
        storeRegFromMem(op->reg, DYN_32bit, DYN_ADDRESS, true, true);
    } else {
        callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
        storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_pushEd_mem(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    if (!cpu->thread->process->hasSetStackMask && !cpu->thread->process->hasSetSeg[SS]) {
        loadReg(4, DYN_ADDRESS, DYN_32bit, true);
        subRegImm(DYN_ADDRESS, DYN_32bit, 4);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST); // need to discard DYN_ADDRESS, otherwise will be out of regs
        subCPUImm(4, DYN_32bit, 4, DYN_ADDRESS);
    } else {
        callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_popEd_mem(DecodedOp* op) {    
    if (!cpu->thread->process->hasSetStackMask && !cpu->thread->process->hasSetSeg[SS]) {
        loadReg(4, DYN_ADDRESS, DYN_32bit, true);
        movFromMem(DYN_32bit, DYN_ADDRESS, true);
        // address calculation happens after ESP is incremented, but we don't want it committed
        // before the write happens in case the write throws an exception, iexplorer.exe will exercise 
        // that esp needs to be increated before calculateEaa
        if (op->rm == 4 || op->sibIndex == 4) {
            addCPUImm(4, DYN_32bit, 4, DYN_DEST);
            calculateEaa(op, DYN_ADDRESS);
            subCPUImm(4, DYN_32bit, 4, DYN_DEST);
        } else {
            calculateEaa(op, DYN_ADDRESS);
        }
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
        addCPUImm(4, DYN_32bit, 4, DYN_DEST);
    } else {
        // save current esp
        loadReg(4, DYN_SRC, DYN_32bit, true);
        // pop stack
        callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);        
        // calculate write address
        calculateEaa(op, DYN_ADDRESS);
        // set ESP back to original ESP before it was incremented in case the write throws an exception
        DynReg reg = loadReg(4, DYN_DEST, DYN_32bit);
        storeReg(4, DYN_SRC, DYN_32bit, true);
        // do write
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_SRC);
        // restore to calculated ESP from common_pop32 after write succeeds
        storeReg(4, reg, DYN_32bit, true);
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_pushSeg16(DecodedOp* op) {
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofSegValue(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_popSeg16(DecodedOp* op) {
    callHostFunction((void*)common_peek16, true, 2, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false);
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    loadStackMask(DYN_DEST);
    loadReg(4, DYN_SRC, DYN_32bit, true);
    movToRegFromReg(DYN_ADDRESS, DYN_32bit, DYN_SRC, DYN_32bit, false);
    addRegImm(DYN_SRC, DYN_32bit, 2);
    andRegReg(DYN_SRC, DYN_DEST, DYN_32bit, true);
    loadStackNotMask(DYN_DEST);
    andRegReg(DYN_ADDRESS, DYN_DEST, DYN_32bit, true);
    orRegReg(DYN_SRC, DYN_ADDRESS, DYN_32bit, true);
    storeReg(4, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_pushSeg32(DecodedOp* op) {
    callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofSegValue(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_popSeg32(DecodedOp* op) {
    callHostFunction((void*)common_peek32, true, 2, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false);
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    loadStackMask(DYN_DEST);
    loadReg(4, DYN_SRC, DYN_32bit, true);
    movToRegFromReg(DYN_ADDRESS, DYN_32bit, DYN_SRC, DYN_32bit, false);
    addRegImm(DYN_SRC, DYN_32bit, 4);
    andRegReg(DYN_SRC, DYN_DEST, DYN_32bit, true);
    loadStackNotMask(DYN_DEST);
    andRegReg(DYN_ADDRESS, DYN_DEST, DYN_32bit, true);
    orRegReg(DYN_SRC, DYN_ADDRESS, DYN_32bit, true);
    storeReg(4, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_pushA16(DecodedOp* op) {
    callHostFunction((void*)common_pushA16, false, 1, 0, DYN_PARAM_CPU, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_pushA32(DecodedOp* op) {
    callHostFunction((void*)common_pushA32, false, 1, 0, DYN_PARAM_CPU, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_popA16(DecodedOp* op) {
    callHostFunction((void*)common_popA16, false, 1, 0, DYN_PARAM_CPU, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_popA32(DecodedOp* op) {
    callHostFunction((void*)common_popA32, false, 1, 0, DYN_PARAM_CPU, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_push16imm(DecodedOp* op) {
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_push32imm(DecodedOp* op) {
     if (!cpu->thread->process->hasSetStackMask && !cpu->thread->process->hasSetSeg[SS]) {
         loadReg(4, DYN_ADDRESS, DYN_32bit, true);
         subRegImm(DYN_ADDRESS, DYN_32bit, 4);
         movToMemFromImm(DYN_ADDRESS, DYN_32bit, op->imm, true, DYN_DEST); // need to discard DYN_ADDRESS, otherwise will be out of regs
         subCPUImm(4, DYN_32bit, 4, DYN_ADDRESS);
     } else {
         callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
     }
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
    dynamic_fillFlags();
    loadCPUFlags(DYN_SRC);
    orRegImm(DYN_SRC, DYN_32bit, 2);
    andRegImm(DYN_SRC, DYN_32bit, 0xFCFFFF);
    dynamic_pushReg32(DYN_SRC, true);
    incrementEip(op->len);
}
