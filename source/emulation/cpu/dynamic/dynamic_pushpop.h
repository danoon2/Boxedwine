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
    data->callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg16(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    data->incrementEip(op->len);
}
void dynamic_popEw_reg(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    data->storeReg(op->reg, DYN_CALL_RESULT, DYN_16bit, true);
    data->incrementEip(op->len);
}
void dynamic_pushEw_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    data->incrementEip(op->len);
}
void dynamic_popEw_mem(DynamicData* data, DecodedOp* op) {    
    // save current ESP
    data->loadReg(4, DYN_SRC, DYN_32bit, true);
    // pop stack
    data->callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    // calculate write address
    data->calculateEaa(op, DYN_ADDRESS);
    // set ESP back to original ESP before it was incremented in case the write throws an exception
    data->loadReg(4, DYN_DEST, DYN_32bit, true);
    data->storeReg(4, DYN_SRC, DYN_32bit, true);
    // do write
    data->movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_SRC);
    // restore to calculated ESP from common_pop16 after write succeeds
    data->storeReg(4, DYN_DEST, DYN_32bit, true);

    data->incrementEip(op->len);
}
void dynamic_pushEd_reg(DynamicData* data, DecodedOp* op) {
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        data->loadReg(4, DYN_ADDRESS, DYN_32bit, true);
        data->instRegImm('-', DYN_ADDRESS, DYN_32bit, 4);
        DynReg reg = data->loadReg(op->reg, DYN_SRC, DYN_32bit);
        data->movToMemFromReg(DYN_ADDRESS, reg, DYN_32bit, true, true, DYN_DEST); // need to discard DYN_ADDRESS, otherwise will be out of regs
        data->instCPUImm('-', CPU::offsetofReg32(4), DYN_32bit, 4, DYN_ADDRESS);
    } else {
        data->callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofReg32(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    }
    data->incrementEip(op->len);
}
void dynamic_popEd_reg(DynamicData* data, DecodedOp* op) {
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        data->loadReg(4, DYN_ADDRESS, DYN_32bit, true);
        data->instCPUImm('+', CPU::offsetofReg32(4), DYN_32bit, 4, DYN_DEST); // increment before assign, in case esp=pop32()
        data->storeRegFromMem(op->reg, DYN_32bit, DYN_ADDRESS, true, true);
    } else {
        data->callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
        data->storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
    }
    data->incrementEip(op->len);
}
void dynamic_pushEd_mem(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_32bit, DYN_ADDRESS, true);
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        data->loadReg(4, DYN_ADDRESS, DYN_32bit, true);
        data->instRegImm('-', DYN_ADDRESS, DYN_32bit, 4);
        data->movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST); // need to discard DYN_ADDRESS, otherwise will be out of regs
        data->instCPUImm('-', CPU::offsetofReg32(4), DYN_32bit, 4, DYN_ADDRESS);
    } else {
        data->callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    }
    data->incrementEip(op->len);
}
void dynamic_popEd_mem(DynamicData* data, DecodedOp* op) {    
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        data->loadReg(4, DYN_ADDRESS, DYN_32bit, true);
        data->movFromMem(DYN_32bit, DYN_ADDRESS, true);
        // address calculation happens after ESP is incremented, but we don't want it committed
        // before the write happens in case the write throws an exception, iexplorer.exe will exercise 
        // that esp needs to be increated before calculateEaa
        if (op->rm == 4 || op->sibIndex == 4) {
            data->instCPUImm('+', CPU::offsetofReg32(4), DYN_32bit, 4, DYN_DEST);
            data->calculateEaa(op, DYN_ADDRESS);
            data->instCPUImm('-', CPU::offsetofReg32(4), DYN_32bit, 4, DYN_DEST);
        } else {
            data->calculateEaa(op, DYN_ADDRESS);
        }
        data->movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
        data->instCPUImm('+', CPU::offsetofReg32(4), DYN_32bit, 4, DYN_DEST);
    } else {
        // save current esp
        data->loadReg(4, DYN_SRC, DYN_32bit, true);
        // pop stack
        data->callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);        
        // calculate write address
        data->calculateEaa(op, DYN_ADDRESS);
        // set ESP back to original ESP before it was incremented in case the write throws an exception
        DynReg reg = data->loadReg(4, DYN_DEST, DYN_32bit);
        data->storeReg(4, DYN_SRC, DYN_32bit, true);
        // do write
        data->movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_SRC);
        // restore to calculated ESP from common_pop32 after write succeeds
        data->storeReg(4, reg, DYN_32bit, true);
    }
    data->incrementEip(op->len);
}
void dynamic_pushSeg16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofSegValue(op->reg), DYN_PARAM_CPU_ADDRESS_16, false);
    data->incrementEip(op->len);
}
void dynamic_popSeg16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_peek16, true, 2, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false);
    data->callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    data->IfNot(DYN_CALL_RESULT, true);
    data->blockDone(true);
    data->EndIf();
    data->loadStackMask(DYN_DEST);
    data->loadReg(4, DYN_SRC, DYN_32bit, true);
    data->movToRegFromReg(DYN_ADDRESS, DYN_32bit, DYN_SRC, DYN_32bit, false);
    data->instRegImm('+', DYN_SRC, DYN_32bit, 2);
    data->instRegReg('&', DYN_SRC, DYN_DEST, DYN_32bit, true);
    data->loadStackNotMask(DYN_DEST);
    data->instRegReg('&', DYN_ADDRESS, DYN_DEST, DYN_32bit, true);
    data->instRegReg('|', DYN_SRC, DYN_ADDRESS, DYN_32bit, true);
    data->storeReg(4, DYN_SRC, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_pushSeg32(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, CPU::offsetofSegValue(op->reg), DYN_PARAM_CPU_ADDRESS_32, false);
    data->incrementEip(op->len);
}
void dynamic_popSeg32(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_peek32, true, 2, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false);
    data->callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);
    data->IfNot(DYN_CALL_RESULT, true);
    data->blockDone(true);
    data->EndIf();
    data->loadStackMask(DYN_DEST);
    data->loadReg(4, DYN_SRC, DYN_32bit, true);
    data->movToRegFromReg(DYN_ADDRESS, DYN_32bit, DYN_SRC, DYN_32bit, false);
    data->instRegImm('+', DYN_SRC, DYN_32bit, 4);
    data->instRegReg('&', DYN_SRC, DYN_DEST, DYN_32bit, true);
    data->loadStackNotMask(DYN_DEST);
    data->instRegReg('&', DYN_ADDRESS, DYN_DEST, DYN_32bit, true);
    data->instRegReg('|', DYN_SRC, DYN_ADDRESS, DYN_32bit, true);
    data->storeReg(4, DYN_SRC, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_pushA16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_pushA16, false, 1, 0, DYN_PARAM_CPU, false);
    data->incrementEip(op->len);
}
void dynamic_pushA32(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_pushA32, false, 1, 0, DYN_PARAM_CPU, false);
    data->incrementEip(op->len);
}
void dynamic_popA16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_popA16, false, 1, 0, DYN_PARAM_CPU, false);
    data->incrementEip(op->len);
}
void dynamic_popA32(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_popA32, false, 1, 0, DYN_PARAM_CPU, false);
    data->incrementEip(op->len);
}
void dynamic_push16imm(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_16, false);
    data->incrementEip(op->len);
}
void dynamic_push32imm(DynamicData* data, DecodedOp* op) {
     if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
         data->loadReg(4, DYN_ADDRESS, DYN_32bit, true);
         data->instRegImm('-', DYN_ADDRESS, DYN_32bit, 4);
         data->movToMemFromImm(DYN_ADDRESS, DYN_32bit, op->imm, true, DYN_DEST); // need to discard DYN_ADDRESS, otherwise will be out of regs
         data->instCPUImm('-', CPU::offsetofReg32(4), DYN_32bit, 4, DYN_ADDRESS);
     } else {
         data->callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
     }
    data->incrementEip(op->len);
}
void dynamic_popf16(DynamicData* data, DecodedOp* op) {
    data->storeLazyFlags(FLAGS_NONE);
    data->callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    data->callHostFunction((void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, FMASK_ALL & 0xFFFF, DYN_PARAM_CONST_16, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_popf32(DynamicData* data, DecodedOp* op) {
    data->storeLazyFlags(FLAGS_NONE);
    data->callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
    data->callHostFunction((void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, DYN_CALL_RESULT, DYN_PARAM_REG_32, true, FMASK_ALL, DYN_PARAM_CONST_32, false);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_pushf16(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    data->loadCPUFlags(DYN_SRC);
    data->instRegImm('|', DYN_SRC, DYN_32bit, 2);
    data->callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}
void dynamic_pushf32(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    data->loadCPUFlags(DYN_SRC);
    data->instRegImm('|', DYN_SRC, DYN_32bit, 2);
    data->instRegImm('&', DYN_SRC, DYN_32bit, 0xFCFFFF);
    dynamic_pushReg32(data, DYN_SRC, true);
    data->incrementEip(op->len);
}
