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

void dynamic_inc8_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm('+', CPU::offsetofReg8(op->reg), DYN_8bit, 1);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
        }
        movToCpuFromCpu(CPU_OFFSET_OF(dst.u8), CPU::offsetofReg8(op->reg), DYN_8bit, DYN_DEST, false);
        instRegImm('+', DYN_DEST, DYN_8bit, 1);
        movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_DEST, DYN_8bit, false);
        movToCpuFromReg(CPU::offsetofReg8(op->reg), DYN_DEST, DYN_8bit, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_INC8);
        data->currentLazyFlags=FLAGS_INC8;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_inc8_mem8(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('+', DYN_ADDRESS, DYN_8bit, 1, true);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        movToCpuFromMem(CPU_OFFSET_OF(dst.u8), DYN_8bit, DYN_ADDRESS, false, false);
        instRegImm('+', DYN_CALL_RESULT, DYN_8bit, 1);
        movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_CALL_RESULT, DYN_8bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_INC8);
        data->currentLazyFlags=FLAGS_INC8;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_inc16_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm('+', CPU::offsetofReg16(op->reg), DYN_16bit, 1);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
        }
        movToCpuFromCpu(CPU_OFFSET_OF(dst.u16), CPU::offsetofReg16(op->reg), DYN_16bit, DYN_DEST, false);
        instRegImm('+', DYN_DEST, DYN_16bit, 1);
        movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_DEST, DYN_16bit, false);
        movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_DEST, DYN_16bit, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_INC16);
        data->currentLazyFlags=FLAGS_INC16;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_inc16_mem16(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('+', DYN_ADDRESS, DYN_16bit, 1, true);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        movToCpuFromMem(CPU_OFFSET_OF(dst.u16), DYN_16bit, DYN_ADDRESS, false, false);
        instRegImm('+', DYN_CALL_RESULT, DYN_16bit, 1);
        movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_CALL_RESULT, DYN_16bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_INC16);
        data->currentLazyFlags=FLAGS_INC16;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_inc32_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm('+', CPU::offsetofReg32(op->reg), DYN_32bit, 1);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
        }
        movToCpuFromCpu(CPU_OFFSET_OF(dst.u32), CPU::offsetofReg32(op->reg), DYN_32bit, DYN_DEST, false);
        instRegImm('+', DYN_DEST, DYN_32bit, 1);
        movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_DEST, DYN_32bit, false);
        movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_DEST, DYN_32bit, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_INC32);
        data->currentLazyFlags=FLAGS_INC32;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_inc32_mem32(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('+', DYN_ADDRESS, DYN_32bit, 1, true);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        movToCpuFromMem(CPU_OFFSET_OF(dst.u32), DYN_32bit, DYN_ADDRESS, false, false);
        instRegImm('+', DYN_CALL_RESULT, DYN_32bit, 1);
        movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_CALL_RESULT, DYN_32bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_INC32);
        data->currentLazyFlags=FLAGS_INC32;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_dec8_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm('-', CPU::offsetofReg8(op->reg), DYN_8bit, 1);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
        }
        movToCpuFromCpu(CPU_OFFSET_OF(dst.u8), CPU::offsetofReg8(op->reg), DYN_8bit, DYN_DEST, false);
        instRegImm('-', DYN_DEST, DYN_8bit, 1);
        movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_DEST, DYN_8bit, false);
        movToCpuFromReg(CPU::offsetofReg8(op->reg), DYN_DEST, DYN_8bit, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_DEC8);
        data->currentLazyFlags=FLAGS_DEC8;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_dec8_mem8(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('-', DYN_ADDRESS, DYN_8bit, 1, true);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        movToCpuFromMem(CPU_OFFSET_OF(dst.u8), DYN_8bit, DYN_ADDRESS, false, false);
        instRegImm('-', DYN_CALL_RESULT, DYN_8bit, 1);
        movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_CALL_RESULT, DYN_8bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_DEC8);
        data->currentLazyFlags=FLAGS_DEC8;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_dec16_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm('-', CPU::offsetofReg16(op->reg), DYN_16bit, 1);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
        }
        movToCpuFromCpu(CPU_OFFSET_OF(dst.u16), CPU::offsetofReg16(op->reg), DYN_16bit, DYN_DEST, false);
        instRegImm('-', DYN_DEST, DYN_16bit, 1);
        movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_DEST, DYN_16bit, false);
        movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_DEST, DYN_16bit, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_DEC16);
        data->currentLazyFlags=FLAGS_DEC16;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_dec16_mem16(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('-', DYN_ADDRESS, DYN_16bit, 1, true);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        movToCpuFromMem(CPU_OFFSET_OF(dst.u16), DYN_16bit, DYN_ADDRESS, false, false);
        instRegImm('-', DYN_CALL_RESULT, DYN_16bit, 1);
        movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_CALL_RESULT, DYN_16bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_DEC16);
        data->currentLazyFlags=FLAGS_DEC16;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_dec32_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm('-', CPU::offsetofReg32(op->reg), DYN_32bit, 1);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
        }
        movToCpuFromCpu(CPU_OFFSET_OF(dst.u32), CPU::offsetofReg32(op->reg), DYN_32bit, DYN_DEST, false);
        instRegImm('-', DYN_DEST, DYN_32bit, 1);
        movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_DEST, DYN_32bit, false);
        movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_DEST, DYN_32bit, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_DEC32);
        data->currentLazyFlags=FLAGS_DEC32;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_dec32_mem32(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('-', DYN_ADDRESS, DYN_32bit, 1, true);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        movToCpuFromMem(CPU_OFFSET_OF(dst.u32), DYN_32bit, DYN_ADDRESS, false, false);
        instRegImm('-', DYN_CALL_RESULT, DYN_32bit, 1);
        movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_CALL_RESULT, DYN_32bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_DEC32);
        data->currentLazyFlags=FLAGS_DEC32;
    }
    INCREMENT_EIP(data, op);
}
