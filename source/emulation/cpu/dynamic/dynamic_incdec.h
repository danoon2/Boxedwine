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
        instCPUImm(data,'+', CPU::offsetofReg8(op->reg), DYN_8bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            storeLazyFlagsOldCF(data, DYN_CALL_RESULT, true);
        }
        loadRegStoreDst(data, op->reg, DYN_8bit, DYN_DEST, false);
        instRegImm(data,'+', DYN_DEST, DYN_8bit, 1);
        storeLazyFlagsResult(data, DYN_DEST, DYN_8bit, false);
        storeReg(data, op->reg, DYN_DEST, DYN_8bit, true);
        storeLazyFlags(data, FLAGS_INC8);
        data->currentLazyFlags=FLAGS_INC8;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_inc8_mem8(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(data,op, DYN_ADDRESS);
        instMemImm(data,'+', DYN_ADDRESS, DYN_8bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            storeLazyFlagsOldCF(data, DYN_CALL_RESULT, true);
        }
        calculateEaa(data,op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(data, DYN_8bit, DYN_ADDRESS, false, false);
        instRegImm(data,'+', DYN_CALL_RESULT, DYN_8bit, 1);
        storeLazyFlagsResult(data, DYN_CALL_RESULT, DYN_8bit, false);
        movToMemFromReg(data,DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true, DYN_DEST);
        storeLazyFlags(data, FLAGS_INC8);
        data->currentLazyFlags=FLAGS_INC8;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_inc16_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm(data,'+', CPU::offsetofReg16(op->reg), DYN_16bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            storeLazyFlagsOldCF(data, DYN_CALL_RESULT, true);
        }
        loadRegStoreDst(data, op->reg, DYN_16bit, DYN_DEST, false);
        instRegImm(data,'+', DYN_DEST, DYN_16bit, 1);
        storeLazyFlagsResult(data, DYN_DEST, DYN_16bit, false);
        storeReg(data, op->reg, DYN_DEST, DYN_16bit, true);
        storeLazyFlags(data, FLAGS_INC16);
        data->currentLazyFlags=FLAGS_INC16;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_inc16_mem16(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(data,op, DYN_ADDRESS);
        instMemImm(data,'+', DYN_ADDRESS, DYN_16bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            storeLazyFlagsOldCF(data, DYN_CALL_RESULT, true);
        }
        calculateEaa(data,op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(data, DYN_16bit, DYN_ADDRESS, false, false);
        instRegImm(data,'+', DYN_CALL_RESULT, DYN_16bit, 1);
        storeLazyFlagsResult(data, DYN_CALL_RESULT, DYN_16bit, false);
        movToMemFromReg(data,DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_DEST);
        storeLazyFlags(data, FLAGS_INC16);
        data->currentLazyFlags=FLAGS_INC16;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_inc32_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm(data,'+', CPU::offsetofReg32(op->reg), DYN_32bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            storeLazyFlagsOldCF(data, DYN_CALL_RESULT, true);
        }
        loadRegStoreDst(data, op->reg, DYN_32bit, DYN_DEST, false);
        instRegImm(data,'+', DYN_DEST, DYN_32bit, 1);
        storeLazyFlagsResult(data, DYN_DEST, DYN_32bit, false);
        storeReg(data, op->reg, DYN_DEST, DYN_32bit, true);
        storeLazyFlags(data, FLAGS_INC32);
        data->currentLazyFlags=FLAGS_INC32;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_inc32_mem32(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(data,op, DYN_ADDRESS);
        instMemImm(data,'+', DYN_ADDRESS, DYN_32bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            storeLazyFlagsOldCF(data, DYN_CALL_RESULT, true);
        }
        calculateEaa(data,op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(data, DYN_32bit, DYN_ADDRESS, false, false);
        instRegImm(data,'+', DYN_CALL_RESULT, DYN_32bit, 1);
        storeLazyFlagsResult(data, DYN_CALL_RESULT, DYN_32bit, false);
        movToMemFromReg(data,DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
        storeLazyFlags(data, FLAGS_INC32);
        data->currentLazyFlags=FLAGS_INC32;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_dec8_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm(data,'-', CPU::offsetofReg8(op->reg), DYN_8bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            storeLazyFlagsOldCF(data, DYN_CALL_RESULT, true);
        }
        loadRegStoreDst(data, op->reg, DYN_8bit, DYN_DEST, false);
        instRegImm(data,'-', DYN_DEST, DYN_8bit, 1);
        storeLazyFlagsResult(data, DYN_DEST, DYN_8bit, false);
        storeReg(data, op->reg, DYN_DEST, DYN_8bit, true);
        storeLazyFlags(data, FLAGS_DEC8);
        data->currentLazyFlags=FLAGS_DEC8;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_dec8_mem8(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(data,op, DYN_ADDRESS);
        instMemImm(data,'-', DYN_ADDRESS, DYN_8bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            storeLazyFlagsOldCF(data, DYN_CALL_RESULT, true);
        }
        calculateEaa(data,op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(data, DYN_8bit, DYN_ADDRESS, false, false);
        instRegImm(data,'-', DYN_CALL_RESULT, DYN_8bit, 1);
        storeLazyFlagsResult(data, DYN_CALL_RESULT, DYN_8bit, false);
        movToMemFromReg(data,DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true, DYN_DEST);
        storeLazyFlags(data, FLAGS_DEC8);
        data->currentLazyFlags=FLAGS_DEC8;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_dec16_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm(data,'-', CPU::offsetofReg16(op->reg), DYN_16bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            storeLazyFlagsOldCF(data, DYN_CALL_RESULT, true);
        }
        loadRegStoreDst(data, op->reg, DYN_16bit, DYN_DEST, false);
        instRegImm(data,'-', DYN_DEST, DYN_16bit, 1);
        storeLazyFlagsResult(data, DYN_DEST, DYN_16bit, false);
        storeReg(data, op->reg, DYN_DEST, DYN_16bit, true);
        storeLazyFlags(data, FLAGS_DEC16);
        data->currentLazyFlags=FLAGS_DEC16;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_dec16_mem16(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(data,op, DYN_ADDRESS);
        instMemImm(data,'-', DYN_ADDRESS, DYN_16bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            storeLazyFlagsOldCF(data, DYN_CALL_RESULT, true);
        }
        calculateEaa(data,op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(data, DYN_16bit, DYN_ADDRESS, false, false);
        instRegImm(data,'-', DYN_CALL_RESULT, DYN_16bit, 1);
        storeLazyFlagsResult(data, DYN_CALL_RESULT, DYN_16bit, false);
        movToMemFromReg(data,DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_DEST);
        storeLazyFlags(data, FLAGS_DEC16);
        data->currentLazyFlags=FLAGS_DEC16;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_dec32_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm(data,'-', CPU::offsetofReg32(op->reg), DYN_32bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            storeLazyFlagsOldCF(data, DYN_CALL_RESULT, true);
        }
        loadRegStoreDst(data, op->reg, DYN_32bit, DYN_DEST, false);
        instRegImm(data,'-', DYN_DEST, DYN_32bit, 1);
        storeLazyFlagsResult(data, DYN_DEST, DYN_32bit, false);
        storeReg(data, op->reg, DYN_DEST, DYN_32bit, true);
        storeLazyFlags(data, FLAGS_DEC32);
        data->currentLazyFlags=FLAGS_DEC32;
    }
    INCREMENT_EIP(data, op);
}
void dynamic_dec32_mem32(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(data,op, DYN_ADDRESS);
        instMemImm(data,'-', DYN_ADDRESS, DYN_32bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            storeLazyFlagsOldCF(data, DYN_CALL_RESULT, true);
        }
        calculateEaa(data,op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(data, DYN_32bit, DYN_ADDRESS, false, false);
        instRegImm(data,'-', DYN_CALL_RESULT, DYN_32bit, 1);
        storeLazyFlagsResult(data, DYN_CALL_RESULT, DYN_32bit, false);
        movToMemFromReg(data,DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
        storeLazyFlags(data, FLAGS_DEC32);
        data->currentLazyFlags=FLAGS_DEC32;
    }
    INCREMENT_EIP(data, op);
}
