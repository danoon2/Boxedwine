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
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popEw_reg(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushEw_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    INCREMENT_EIP(data, op);
}
void dynamic_popEw_mem(DynamicData* data, DecodedOp* op) {    
    // save current ESP
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
    // pop stack
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    // calculate write address
    calculateEaa(op, DYN_ADDRESS);
    // set ESP back to original ESP before it was incremented in case the write throws an exception
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
    movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);
    // do write
    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true);
    // restore to calculated ESP from common_pop16 after write succeeds
    movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_DEST, DYN_32bit, true);

    INCREMENT_EIP(data, op);
}
void dynamic_pushEd_reg(DynamicData* data, DecodedOp* op) {
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        movToRegFromCpu(DYN_ADDRESS, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
        instRegImm('-', DYN_ADDRESS, DYN_32bit, 4);
        movToRegFromCpu(DYN_SRC, CPU::offsetofReg32(op->reg), DYN_32bit);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, false, true);
        movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_ADDRESS, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_popEd_reg(DynamicData* data, DecodedOp* op) {
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        movToRegFromCpu(DYN_ADDRESS, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
        instCPUImm('+', CPU_OFFSET_OF(reg[4].u32), DYN_32bit, 4); // increment before assign, in case esp=pop32()
        movToCpuFromMem(CPU::offsetofReg32(op->reg), DYN_32bit, DYN_ADDRESS, true, true);        
    } else {
        callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
        movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_CALL_RESULT, DYN_32bit, true);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_pushEd_mem(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        movToRegFromCpu(DYN_ADDRESS, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
        instRegImm('-', DYN_ADDRESS, DYN_32bit, 4);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, false, true);
        movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_ADDRESS, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_popEd_mem(DynamicData* data, DecodedOp* op) {    
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
        movFromMem(DYN_32bit, DYN_SRC, true);
        // address calculation happens after ESP is incremented, but we don't want it committed
        // before the write happens in case the write throws an exception, iexplorer.exe will exercise 
        // that esp needs to be increated before calculateEaa
        if (op->rm == 4 || op->sibIndex == 4) {
            instCPUImm('+', CPU_OFFSET_OF(reg[4].u32), DYN_32bit, 4);
            calculateEaa(op, DYN_ADDRESS);
            instCPUImm('-', CPU_OFFSET_OF(reg[4].u32), DYN_32bit, 4);
        } else {
            calculateEaa(op, DYN_ADDRESS);
        }
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true);                
        instCPUImm('+', CPU_OFFSET_OF(reg[4].u32), DYN_32bit, 4);
    } else {
        // save current esp
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
        // pop stack
        callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);        
        // calculate write address
        calculateEaa(op, DYN_ADDRESS);
        // set ESP back to original ESP before it was incremented in case the write throws an exception
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
        movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);
        // do write
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true);
        // restore to calculated ESP from common_pop32 after write succeeds
        movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_DEST, DYN_32bit, true);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_pushSeg16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofSegValue(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popSeg16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_peek16, true, 2, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false);
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone(data, true);
    endIf();
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(stackMask), DYN_32bit);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
    movToRegFromReg(DYN_ADDRESS, DYN_32bit, DYN_SRC, DYN_32bit, false);
    instRegImm('+', DYN_SRC, DYN_32bit, 2);
    instRegReg('&', DYN_SRC, DYN_DEST, DYN_32bit, true);
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(stackNotMask), DYN_32bit);
    instRegReg('&', DYN_ADDRESS, DYN_DEST, DYN_32bit, true);
    instRegReg('|', DYN_SRC, DYN_ADDRESS, DYN_32bit, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushSeg32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofSegValue(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popSeg32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_peek32, true, 2, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false);
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone(data, true);
    endIf();
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(stackMask), DYN_32bit);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
    movToRegFromReg(DYN_ADDRESS, DYN_32bit, DYN_SRC, DYN_32bit, false);
    instRegImm('+', DYN_SRC, DYN_32bit, 4);
    instRegReg('&', DYN_SRC, DYN_DEST, DYN_32bit, true);
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(stackNotMask), DYN_32bit);
    instRegReg('&', DYN_ADDRESS, DYN_DEST, DYN_32bit, true);
    instRegReg('|', DYN_SRC, DYN_ADDRESS, DYN_32bit, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushA16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_pushA16, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_pushA32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_pushA32, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popA16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_popA16, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popA32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_popA32, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_push16imm(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_push32imm(DynamicData* data, DecodedOp* op) {
     if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
         movToRegFromCpu(DYN_ADDRESS, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
         instRegImm('-', DYN_ADDRESS, DYN_32bit, 4);
         movToMemFromImm(DYN_ADDRESS, DYN_32bit, op->imm, false);
         movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_ADDRESS, DYN_32bit, true);
     } else {
         callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
     }
    INCREMENT_EIP(data, op);
}
void dynamic_popf16(DynamicData* data, DecodedOp* op) {
    movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_NONE);
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    callHostFunction((void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, FMASK_ALL & 0xFFFF, DYN_PARAM_CONST_16, false);
    INCREMENT_EIP(data, op);
}
void dynamic_popf32(DynamicData* data, DecodedOp* op) {
    movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_NONE);
    callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
    callHostFunction((void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, FMASK_ALL, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_pushf16(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(flags), DYN_32bit);
    instRegImm('|', DYN_SRC, DYN_32bit, 2);
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_pushf32(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(flags), DYN_32bit);
    instRegImm('|', DYN_SRC, DYN_32bit, 2);
    instRegImm('&', DYN_SRC, DYN_32bit, 0xFCFFFF);
    dynamic_pushReg32(data, DYN_SRC, true);
    INCREMENT_EIP(data, op);
}
