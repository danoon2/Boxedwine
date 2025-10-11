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

void DynamicData::dynamic_inc8_reg(DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm('+', op->reg, DYN_8bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF();
            storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        loadRegStoreDst(op->reg, DYN_8bit, DYN_DEST, false);
        instRegImm('+', DYN_DEST, DYN_8bit, 1);
        storeLazyFlagsResult(DYN_DEST, DYN_8bit, false);
        storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        storeLazyFlags(FLAGS_INC8);
        currentLazyFlags=FLAGS_INC8;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_inc8_mem8(DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('+', DYN_ADDRESS, DYN_8bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF();
            storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(DYN_8bit, DYN_ADDRESS, false, false);
        instRegImm('+', DYN_CALL_RESULT, DYN_8bit, 1);
        storeLazyFlagsResult(DYN_CALL_RESULT, DYN_8bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true, DYN_DEST);
        storeLazyFlags(FLAGS_INC8);
        currentLazyFlags=FLAGS_INC8;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_inc16_reg(DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm('+', op->reg, DYN_16bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF();
            storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        loadRegStoreDst(op->reg, DYN_16bit, DYN_DEST, false);
        instRegImm('+', DYN_DEST, DYN_16bit, 1);
        storeLazyFlagsResult(DYN_DEST, DYN_16bit, false);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        storeLazyFlags(FLAGS_INC16);
        currentLazyFlags=FLAGS_INC16;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_inc16_mem16(DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('+', DYN_ADDRESS, DYN_16bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF();
            storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(DYN_16bit, DYN_ADDRESS, false, false);
        instRegImm('+', DYN_CALL_RESULT, DYN_16bit, 1);
        storeLazyFlagsResult(DYN_CALL_RESULT, DYN_16bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_DEST);
        storeLazyFlags(FLAGS_INC16);
        currentLazyFlags=FLAGS_INC16;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_inc32_reg(DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm('+', op->reg, DYN_32bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF();
            storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        loadRegStoreDst(op->reg, DYN_32bit, DYN_DEST, false);
        instRegImm('+', DYN_DEST, DYN_32bit, 1);
        storeLazyFlagsResult(DYN_DEST, DYN_32bit, false);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        storeLazyFlags(FLAGS_INC32);
        currentLazyFlags=FLAGS_INC32;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_inc32_mem32(DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('+', DYN_ADDRESS, DYN_32bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF();
            storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(DYN_32bit, DYN_ADDRESS, false, false);
        instRegImm('+', DYN_CALL_RESULT, DYN_32bit, 1);
        storeLazyFlagsResult(DYN_CALL_RESULT, DYN_32bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
        storeLazyFlags(FLAGS_INC32);
        currentLazyFlags=FLAGS_INC32;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dec8_reg(DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm('-', op->reg, DYN_8bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF();
            storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        loadRegStoreDst(op->reg, DYN_8bit, DYN_DEST, false);
        instRegImm('-', DYN_DEST, DYN_8bit, 1);
        storeLazyFlagsResult(DYN_DEST, DYN_8bit, false);
        storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        storeLazyFlags(FLAGS_DEC8);
        currentLazyFlags=FLAGS_DEC8;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dec8_mem8(DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('-', DYN_ADDRESS, DYN_8bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF();
            storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(DYN_8bit, DYN_ADDRESS, false, false);
        instRegImm('-', DYN_CALL_RESULT, DYN_8bit, 1);
        storeLazyFlagsResult(DYN_CALL_RESULT, DYN_8bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true, DYN_DEST);
        storeLazyFlags(FLAGS_DEC8);
        currentLazyFlags=FLAGS_DEC8;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dec16_reg(DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm('-', op->reg, DYN_16bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF();
            storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        loadRegStoreDst(op->reg, DYN_16bit, DYN_DEST, false);
        instRegImm('-', DYN_DEST, DYN_16bit, 1);
        storeLazyFlagsResult(DYN_DEST, DYN_16bit, false);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        storeLazyFlags(FLAGS_DEC16);
        currentLazyFlags=FLAGS_DEC16;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dec16_mem16(DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('-', DYN_ADDRESS, DYN_16bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF();
            storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(DYN_16bit, DYN_ADDRESS, false, false);
        instRegImm('-', DYN_CALL_RESULT, DYN_16bit, 1);
        storeLazyFlagsResult(DYN_CALL_RESULT, DYN_16bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_DEST);
        storeLazyFlags(FLAGS_DEC16);
        currentLazyFlags=FLAGS_DEC16;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dec32_reg(DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        instCPUImm('-', op->reg, DYN_32bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF();
            storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        loadRegStoreDst(op->reg, DYN_32bit, DYN_DEST, false);
        instRegImm('-', DYN_DEST, DYN_32bit, 1);
        storeLazyFlagsResult(DYN_DEST, DYN_32bit, false);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        storeLazyFlags(FLAGS_DEC32);
        currentLazyFlags=FLAGS_DEC32;
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_dec32_mem32(DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        calculateEaa(op, DYN_ADDRESS);
        instMemImm('-', DYN_ADDRESS, DYN_32bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF();
            storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(DYN_32bit, DYN_ADDRESS, false, false);
        instRegImm('-', DYN_CALL_RESULT, DYN_32bit, 1);
        storeLazyFlagsResult(DYN_CALL_RESULT, DYN_32bit, false);
        movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
        storeLazyFlags(FLAGS_DEC32);
        currentLazyFlags=FLAGS_DEC32;
    }
    incrementEip(op->len);
}
