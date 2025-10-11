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
        data->instCPUImm('+', op->reg, DYN_8bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        data->loadRegStoreDst(op->reg, DYN_8bit, DYN_DEST, false);
        data->instRegImm('+', DYN_DEST, DYN_8bit, 1);
        data->storeLazyFlagsResult(DYN_DEST, DYN_8bit, false);
        data->storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        data->storeLazyFlags(FLAGS_INC8);
        data->currentLazyFlags=FLAGS_INC8;
    }
    data->incrementEip(op->len);
}
void dynamic_inc8_mem8(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm('+', DYN_ADDRESS, DYN_8bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->storeLazyFlagsDstFromMem(DYN_8bit, DYN_ADDRESS, false, false);
        data->instRegImm('+', DYN_CALL_RESULT, DYN_8bit, 1);
        data->storeLazyFlagsResult(DYN_CALL_RESULT, DYN_8bit, false);
        data->movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true, DYN_DEST);
        data->storeLazyFlags(FLAGS_INC8);
        data->currentLazyFlags=FLAGS_INC8;
    }
    data->incrementEip(op->len);
}
void dynamic_inc16_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        data->instCPUImm('+', op->reg, DYN_16bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        data->loadRegStoreDst(op->reg, DYN_16bit, DYN_DEST, false);
        data->instRegImm('+', DYN_DEST, DYN_16bit, 1);
        data->storeLazyFlagsResult(DYN_DEST, DYN_16bit, false);
        data->storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        data->storeLazyFlags(FLAGS_INC16);
        data->currentLazyFlags=FLAGS_INC16;
    }
    data->incrementEip(op->len);
}
void dynamic_inc16_mem16(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm('+', DYN_ADDRESS, DYN_16bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->storeLazyFlagsDstFromMem(DYN_16bit, DYN_ADDRESS, false, false);
        data->instRegImm('+', DYN_CALL_RESULT, DYN_16bit, 1);
        data->storeLazyFlagsResult(DYN_CALL_RESULT, DYN_16bit, false);
        data->movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_DEST);
        data->storeLazyFlags(FLAGS_INC16);
        data->currentLazyFlags=FLAGS_INC16;
    }
    data->incrementEip(op->len);
}
void dynamic_inc32_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        data->instCPUImm('+', op->reg, DYN_32bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        data->loadRegStoreDst(op->reg, DYN_32bit, DYN_DEST, false);
        data->instRegImm('+', DYN_DEST, DYN_32bit, 1);
        data->storeLazyFlagsResult(DYN_DEST, DYN_32bit, false);
        data->storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        data->storeLazyFlags(FLAGS_INC32);
        data->currentLazyFlags=FLAGS_INC32;
    }
    data->incrementEip(op->len);
}
void dynamic_inc32_mem32(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm('+', DYN_ADDRESS, DYN_32bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->storeLazyFlagsDstFromMem(DYN_32bit, DYN_ADDRESS, false, false);
        data->instRegImm('+', DYN_CALL_RESULT, DYN_32bit, 1);
        data->storeLazyFlagsResult(DYN_CALL_RESULT, DYN_32bit, false);
        data->movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
        data->storeLazyFlags(FLAGS_INC32);
        data->currentLazyFlags=FLAGS_INC32;
    }
    data->incrementEip(op->len);
}
void dynamic_dec8_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        data->instCPUImm('-', op->reg, DYN_8bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        data->loadRegStoreDst(op->reg, DYN_8bit, DYN_DEST, false);
        data->instRegImm('-', DYN_DEST, DYN_8bit, 1);
        data->storeLazyFlagsResult(DYN_DEST, DYN_8bit, false);
        data->storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        data->storeLazyFlags(FLAGS_DEC8);
        data->currentLazyFlags=FLAGS_DEC8;
    }
    data->incrementEip(op->len);
}
void dynamic_dec8_mem8(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm('-', DYN_ADDRESS, DYN_8bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->storeLazyFlagsDstFromMem(DYN_8bit, DYN_ADDRESS, false, false);
        data->instRegImm('-', DYN_CALL_RESULT, DYN_8bit, 1);
        data->storeLazyFlagsResult(DYN_CALL_RESULT, DYN_8bit, false);
        data->movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_8bit, true, true, DYN_DEST);
        data->storeLazyFlags(FLAGS_DEC8);
        data->currentLazyFlags=FLAGS_DEC8;
    }
    data->incrementEip(op->len);
}
void dynamic_dec16_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        data->instCPUImm('-', op->reg, DYN_16bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        data->loadRegStoreDst(op->reg, DYN_16bit, DYN_DEST, false);
        data->instRegImm('-', DYN_DEST, DYN_16bit, 1);
        data->storeLazyFlagsResult(DYN_DEST, DYN_16bit, false);
        data->storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        data->storeLazyFlags(FLAGS_DEC16);
        data->currentLazyFlags=FLAGS_DEC16;
    }
    data->incrementEip(op->len);
}
void dynamic_dec16_mem16(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm('-', DYN_ADDRESS, DYN_16bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->storeLazyFlagsDstFromMem(DYN_16bit, DYN_ADDRESS, false, false);
        data->instRegImm('-', DYN_CALL_RESULT, DYN_16bit, 1);
        data->storeLazyFlagsResult(DYN_CALL_RESULT, DYN_16bit, false);
        data->movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_16bit, true, true, DYN_DEST);
        data->storeLazyFlags(FLAGS_DEC16);
        data->currentLazyFlags=FLAGS_DEC16;
    }
    data->incrementEip(op->len);
}
void dynamic_dec32_reg(DynamicData* data, DecodedOp* op) {
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        data->instCPUImm('-', op->reg, DYN_32bit, 1, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        data->loadRegStoreDst(op->reg, DYN_32bit, DYN_DEST, false);
        data->instRegImm('-', DYN_DEST, DYN_32bit, 1);
        data->storeLazyFlagsResult(DYN_DEST, DYN_32bit, false);
        data->storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        data->storeLazyFlags(FLAGS_DEC32);
        data->currentLazyFlags=FLAGS_DEC32;
    }
    data->incrementEip(op->len);
}
void dynamic_dec32_mem32(DynamicData* data, DecodedOp* op) {    
    U32 neededFlags = op->next->getNeededFlags(ARITH_FLAGS);
    if (!neededFlags) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->instMemImm('-', DYN_ADDRESS, DYN_32bit, 1, true, DYN_DEST);
    } else {
        if (neededFlags & CF) {
            dynamic_getCF(data);
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, true);
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->storeLazyFlagsDstFromMem(DYN_32bit, DYN_ADDRESS, false, false);
        data->instRegImm('-', DYN_CALL_RESULT, DYN_32bit, 1);
        data->storeLazyFlagsResult(DYN_CALL_RESULT, DYN_32bit, false);
        data->movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, DYN_32bit, true, true, DYN_DEST);
        data->storeLazyFlags(FLAGS_DEC32);
        data->currentLazyFlags=FLAGS_DEC32;
    }
    data->incrementEip(op->len);
}
