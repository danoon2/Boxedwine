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

void dynamic_pushEw_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popEw_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(data, CPU::offsetofReg16(op->reg), DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushEw_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    INCREMENT_EIP(data, op);
}
void dynamic_popEw_mem(DynamicData* data, DecodedOp* op) {    
    // save current ESP
    loadReg(data, 4, DYN_SRC, DYN_32bit, true);
    // pop stack
    callHostFunction(data, (void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    // calculate write address
    calculateEaa(data, op, DYN_ADDRESS);
    // set ESP back to original ESP before it was incremented in case the write throws an exception
    loadReg(data, 4, DYN_DEST, DYN_32bit, true);
    movToCpuFromReg(data, CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);
    // do write
    movToMemFromReg(data, DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_SRC);
    // restore to calculated ESP from common_pop16 after write succeeds
    movToCpuFromReg(data, CPU_OFFSET_OF(reg[4].u32), DYN_DEST, DYN_32bit, true);

    INCREMENT_EIP(data, op);
}
void dynamic_pushEd_reg(DynamicData* data, DecodedOp* op) {
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        loadReg(data, 4, DYN_ADDRESS, DYN_32bit, true);
        instRegImm(data, '-', DYN_ADDRESS, DYN_32bit, 4);
        DynReg reg = loadReg(data, op->reg, DYN_SRC, DYN_32bit);
        movToMemFromReg(data, DYN_ADDRESS, reg, DYN_32bit, false, true, DYN_DEST);
        movToCpuFromReg(data, CPU_OFFSET_OF(reg[4].u32), DYN_ADDRESS, DYN_32bit, true);
    } else {
        callHostFunction(data, (void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_popEd_reg(DynamicData* data, DecodedOp* op) {
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        loadReg(data, 4, DYN_ADDRESS, DYN_32bit, true);
        instCPUImm(data, '+', CPU_OFFSET_OF(reg[4].u32), DYN_32bit, 4, DYN_DEST); // increment before assign, in case esp=pop32()
        movToCpuFromMem(data, CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);        
    } else {
        callHostFunction(data, (void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
        movToCpuFromReg(data, CPU::offsetofReg32(op->reg), DYN_CALL_RESULT, DYN_32bit, true);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_pushEd_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_32bit, DYN_ADDRESS, true);
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        loadReg(data, 4, DYN_ADDRESS, DYN_32bit, true);
        instRegImm(data, '-', DYN_ADDRESS, DYN_32bit, 4);
        movToMemFromReg(data, DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, false, true, DYN_DEST);
        movToCpuFromReg(data, CPU_OFFSET_OF(reg[4].u32), DYN_ADDRESS, DYN_32bit, true);
    } else {
        callHostFunction(data, (void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_popEd_mem(DynamicData* data, DecodedOp* op) {    
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        loadReg(data, 4, DYN_ADDRESS, DYN_32bit, true);
        movFromMem(data, DYN_32bit, DYN_ADDRESS, true);
        // address calculation happens after ESP is incremented, but we don't want it committed
        // before the write happens in case the write throws an exception, iexplorer.exe will exercise 
        // that esp needs to be increated before calculateEaa
        if (op->rm == 4 || op->sibIndex == 4) {
            instCPUImm(data, '+', CPU_OFFSET_OF(reg[4].u32), DYN_32bit, 4, DYN_DEST);
            calculateEaa(data, op, DYN_ADDRESS);
            instCPUImm(data, '-', CPU_OFFSET_OF(reg[4].u32), DYN_32bit, 4, DYN_DEST);
        } else {
            calculateEaa(data, op, DYN_ADDRESS);
        }
        movToMemFromReg(data, DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
        instCPUImm(data, '+', CPU_OFFSET_OF(reg[4].u32), DYN_32bit, 4, DYN_DEST);
    } else {
        // save current esp
        loadReg(data, 4, DYN_SRC, DYN_32bit, true);
        // pop stack
        callHostFunction(data, (void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);        
        // calculate write address
        calculateEaa(data, op, DYN_ADDRESS);
        // set ESP back to original ESP before it was incremented in case the write throws an exception
        DynReg reg = loadReg(data, 4, DYN_DEST, DYN_32bit);
        movToCpuFromReg(data, CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);
        // do write
        movToMemFromReg(data, DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_SRC);
        // restore to calculated ESP from common_pop32 after write succeeds
        movToCpuFromReg(data, CPU_OFFSET_OF(reg[4].u32), reg, DYN_32bit, true);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_pushSeg16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofSegValue(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popSeg16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_peek16, true, 2, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false);
    callHostFunction(data, (void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    loadStackMask(data, DYN_DEST);
    loadReg(data, 4, DYN_SRC, DYN_32bit, true);
    movToRegFromReg(data, DYN_ADDRESS, DYN_32bit, DYN_SRC, DYN_32bit, false);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, 2);
    instRegReg(data, '&', DYN_SRC, DYN_DEST, DYN_32bit, true);
    loadStackNotMask(data, DYN_DEST);
    instRegReg(data, '&', DYN_ADDRESS, DYN_DEST, DYN_32bit, true);
    instRegReg(data, '|', DYN_SRC, DYN_ADDRESS, DYN_32bit, true);
    movToCpuFromReg(data, CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushSeg32(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofSegValue(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popSeg32(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_peek32, true, 2, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false);
    callHostFunction(data, (void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    loadStackMask(data, DYN_DEST);
    loadReg(data, 4, DYN_SRC, DYN_32bit, true);
    movToRegFromReg(data, DYN_ADDRESS, DYN_32bit, DYN_SRC, DYN_32bit, false);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, 4);
    instRegReg(data, '&', DYN_SRC, DYN_DEST, DYN_32bit, true);
    loadStackNotMask(data, DYN_DEST);
    instRegReg(data, '&', DYN_ADDRESS, DYN_DEST, DYN_32bit, true);
    instRegReg(data, '|', DYN_SRC, DYN_ADDRESS, DYN_32bit, true);
    movToCpuFromReg(data, CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushA16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_pushA16, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_pushA32(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_pushA32, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popA16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_popA16, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popA32(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_popA32, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_push16imm(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_push32imm(DynamicData* data, DecodedOp* op) {
     if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
         loadReg(data, 4, DYN_ADDRESS, DYN_32bit, true);
         instRegImm(data, '-', DYN_ADDRESS, DYN_32bit, 4);
         movToMemFromImm(data, DYN_ADDRESS, DYN_32bit, op->imm, false, DYN_DEST);
         movToCpuFromReg(data, CPU_OFFSET_OF(reg[4].u32), DYN_ADDRESS, DYN_32bit, true);
     } else {
         callHostFunction(data, (void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
     }
    INCREMENT_EIP(data, op);
}
void dynamic_popf16(DynamicData* data, DecodedOp* op) {
    movToCpuPtr(data, CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_NONE);
    callHostFunction(data, (void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    callHostFunction(data, (void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, FMASK_ALL & 0xFFFF, DYN_PARAM_CONST_16, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_popf32(DynamicData* data, DecodedOp* op) {
    movToCpuPtr(data, CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_NONE);
    callHostFunction(data, (void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
    callHostFunction(data, (void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, FMASK_ALL, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_pushf16(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    loadCPUFlags(data, DYN_SRC);
    instRegImm(data, '|', DYN_SRC, DYN_32bit, 2);
    callHostFunction(data, (void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushf32(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    loadCPUFlags(data, DYN_SRC);
    instRegImm(data, '|', DYN_SRC, DYN_32bit, 2);
    instRegImm(data, '&', DYN_SRC, DYN_32bit, 0xFCFFFF);
    dynamic_pushReg32(data, DYN_SRC, true);
    INCREMENT_EIP(data, op);
}
