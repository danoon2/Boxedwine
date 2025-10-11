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

void dynamic_getCF(DynamicData* data);

void dynamic_arithRR(DynamicData* data, DecodedOp* op, DynWidth width, char inst, bool cf, bool store, const LazyFlags* flags) {
    bool needsToSetFlags = op->needsToSetFlags(data->cpu);

    if (cf) {
        dynamic_getCF(data);
    }
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        data->incrementEip(op->len);
        return;
    }
    if (!needsToSetFlags) {
        data->loadReg(op->rm, DYN_SRC, width, true);
        if (cf) {
            if (width != DYN_32bit) {
                data->movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
            }
            data->instRegReg('+', DYN_SRC, DYN_CALL_RESULT, width, true);
        }
        data->instCPUReg(inst, op->reg, DYN_SRC, width, true, DYN_DEST);
    } else {
        if (cf) {
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, false);
        }
        data->loadRegStoreSrc(op->rm, width, DYN_SRC, false);
        data->loadRegStoreDst(op->reg, width, DYN_DEST, false);

        if (cf) {
            if (width != DYN_32bit) {
                data->movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
            }
            data->instRegReg('+', DYN_SRC, DYN_CALL_RESULT, width, true);
        }
        data->instRegReg(inst, DYN_DEST, DYN_SRC, width, true);
        data->storeLazyFlagsResult(DYN_DEST, width, !store);
        if (store) {
            data->storeReg(op->reg, DYN_DEST, width, true);
        }
        data->storeLazyFlags(flags);
        data->currentLazyFlags = flags;
    }
    data->incrementEip(op->len);
}

void dynamic_arithRM(DynamicData* data, DecodedOp* op, DynWidth width, char inst, bool cf, bool store, const LazyFlags* flags) {
    bool needsToSetFlags = op->needsToSetFlags(data->cpu);

    if (cf) {
        dynamic_getCF(data);
    }
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        data->incrementEip(op->len);
        return;
    }
    if (!needsToSetFlags) {
        data->calculateEaa(op, DYN_ADDRESS);
        if (cf) {
            data->movToRegFromReg(DYN_DEST, width, DYN_CALL_RESULT, DYN_32bit, false);
            data->movFromMem(width, DYN_ADDRESS, true);
            data->instRegReg('+', DYN_CALL_RESULT, DYN_DEST, width, true);
        } else {
            data->movFromMem(width, DYN_ADDRESS, true);
        }
        data->instCPUReg(inst, op->reg, DYN_CALL_RESULT, width, true, DYN_DEST);
    } else {
        if (cf) {
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, false);
        }
        if (cf) {
            data->movToRegFromReg(DYN_SRC, width, DYN_CALL_RESULT, DYN_32bit, true);
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->storeLazyFlagsSrcFromMem(width, DYN_ADDRESS, true, false);
        data->loadRegStoreDst(op->reg, width, DYN_DEST, false);
        if (cf) {
            data->instRegReg('+', DYN_CALL_RESULT, DYN_SRC, width, true);
        }
        data->instRegReg(inst, DYN_DEST, DYN_CALL_RESULT, width, true);
        data->storeLazyFlagsResult(DYN_DEST, width, !store);
        if (store) {
            data->storeReg(op->reg, DYN_DEST, width, true);
        }
        data->storeLazyFlags(flags);
        data->currentLazyFlags = flags;
    }
    data->incrementEip(op->len);
}

void dynamic_arithRI(DynamicData* data, DecodedOp* op, DynWidth width, char inst, bool cf, bool store, const LazyFlags* flags) {
    bool needsToSetFlags = op->needsToSetFlags(data->cpu);

    if (cf) {
        dynamic_getCF(data);
    }
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        data->incrementEip(op->len);
        return;
    }
    if (!needsToSetFlags) {
        if (cf) {
            if (width != DYN_32bit) {
                data->movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
            }
            data->instRegImm('+', DYN_CALL_RESULT, width, op->imm);
            data->instCPUReg(inst, op->reg, DYN_CALL_RESULT, width, true, DYN_DEST);
        } else {
            data->instCPUImm(inst, data->cpuOffset(op->reg, width), width, op->imm, DYN_DEST);
        }
    } else {
        if (cf) {
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, false);
        }
        data->storeLazyFlagsSrc(width, op->imm);
        data->loadRegStoreDst(op->reg, width, DYN_DEST, false);
        data->instRegImm(inst, DYN_DEST, width, op->imm);
        if (cf) {
            if (width != DYN_32bit) {
                data->movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
            }
            data->instRegReg(inst, DYN_DEST, DYN_CALL_RESULT, width, true);
        }
        data->storeLazyFlagsResult(DYN_DEST, width, !store);
        if (store) {
            data->storeReg(op->reg, DYN_DEST, width, true);
        }
        data->storeLazyFlags(flags);
        data->currentLazyFlags = flags;
    }
    data->incrementEip(op->len);
}

void dynamic_arithMR(DynamicData* data, DecodedOp* op, DynWidth width, char inst, bool cf, bool store, const LazyFlags* flags) {
    bool needsToSetFlags = op->needsToSetFlags(data->cpu);

    if (cf) {
        dynamic_getCF(data);
    }
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        data->incrementEip(op->len);
        return;
    }
    if (!needsToSetFlags) {
        data->calculateEaa(op, DYN_ADDRESS);
        data->loadReg(op->reg, DYN_SRC, width, true);
        if (cf) {
            if (width != DYN_32bit) {
                data->movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
            }
            data->instRegReg('+', DYN_SRC, DYN_CALL_RESULT, width, true);
        }
        data->instMemReg(inst, DYN_ADDRESS, DYN_SRC, width, true, true, DYN_DEST);
    } else {
        if (cf) {
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, false);
        }
        data->loadRegStoreSrc(op->reg, width, DYN_SRC, false);
        if (cf) {
            if (width != DYN_32bit) {
                data->movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, true);
            }
            data->instRegReg('+', DYN_SRC, DYN_CALL_RESULT, width, true);
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->storeLazyFlagsDstFromMem(width, DYN_ADDRESS, !store, false);
        data->instRegReg(inst, DYN_CALL_RESULT, DYN_SRC, width, true);
        data->storeLazyFlagsResult(DYN_CALL_RESULT, width, !store);
        if (store) {
            data->movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, width, true, true, DYN_DEST);
        }
        data->storeLazyFlags(flags);
        data->currentLazyFlags = flags;
    }
    data->incrementEip(op->len);
}

void dynamic_arithMI(DynamicData* data, DecodedOp* op, DynWidth width, char inst, bool cf, bool store, const LazyFlags* flags) {
    bool needsToSetFlags = op->needsToSetFlags(data->cpu);

    if (cf) {
        dynamic_getCF(data);
    }
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        data->incrementEip(op->len);
        return;
    }
    if (!needsToSetFlags) {
        data->calculateEaa(op, DYN_ADDRESS);
        if (cf) {
            data->movToRegFromReg(DYN_SRC, width, DYN_CALL_RESULT, DYN_32bit, true);
            data->instRegImm('+', DYN_SRC, width, op->imm);
            data->instMemReg(inst, DYN_ADDRESS, DYN_SRC, width, true, true, DYN_DEST);
        } else {
            data->instMemImm(inst, DYN_ADDRESS, width, op->imm, true, DYN_DEST);
        }
    } else {
        if (cf) {
            data->storeLazyFlagsOldCF(DYN_CALL_RESULT, false);
        }
        data->calculateEaa(op, DYN_ADDRESS);
        data->storeLazyFlagsSrc(width, op->imm);
        if (cf) {
            data->movToRegFromReg(DYN_SRC, width, DYN_CALL_RESULT, DYN_32bit, true);
            data->storeLazyFlagsDstFromMem(width, DYN_ADDRESS, !store, false);
            data->instRegImm('+', DYN_SRC, width, op->imm);
            data->instRegReg(inst, DYN_CALL_RESULT, DYN_SRC, width, true);
        } else {
            data->storeLazyFlagsDstFromMem(width, DYN_ADDRESS, !store, false);
            data->instRegImm(inst, DYN_CALL_RESULT, width, op->imm);
        }
        data->storeLazyFlagsResult(DYN_CALL_RESULT, width, !store);
        if (store) {
            data->movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, width, true, true, DYN_DEST);
        }
        data->storeLazyFlags(flags);
        data->currentLazyFlags = flags;
    }
    data->incrementEip(op->len);
}

void genCF(DynamicData* data, const LazyFlags* flags, DynReg reg) {
    if (reg==DYN_SRC || reg == DYN_DEST) {
        kpanic("genCF expects reg not to be DYN_SRC or DYN_DEST");
    }
    if (data->regUsed[DYN_SRC] || data->regUsed[DYN_DEST]) {
        kpanic("genCF expects DYN_SRC and DYN_DEST to be available");
    }
    if (flags == FLAGS_NONE) {
        data->loadCPUFlags(reg);
        data->instRegImm('&', reg, DYN_32bit, CF);
    } else if (flags == FLAGS_ADD8) {
        // cpu->result.u8<cpu->dst.u8;
        data->loadLazyFlagsResult(DYN_SRC, DYN_8bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_8bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_ADD16) {
        // cpu->result.u16<cpu->dst.u16;
        data->loadLazyFlagsResult(DYN_SRC, DYN_16bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_16bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_ADD32) {
        // cpu->result.u32<cpu->dst.u32;
        data->loadLazyFlagsResult(DYN_SRC, DYN_32bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_32bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_OR8) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_OR16) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_OR32) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_ADC8) {
        // (cpu->result.u8 < cpu->dst.u8) || (cpu->oldCF && (cpu->result.u8 == cpu->dst.u8));
        data->loadLazyFlagsResult(DYN_SRC, DYN_8bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_8bit, DYN_EQUALS, false, false);
        data->evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_8bit, DYN_LESS_THAN_UNSIGNED, false, false);
        data->loadLazyFlagsOldCF(DYN_DEST);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        data->instRegReg('&', reg, DYN_DEST, DYN_32bit, true);
        data->instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_ADC16) {
        // (cpu->result.u16 < cpu->dst.u16) || (cpu->oldCF && (cpu->result.u16 == cpu->dst.u16));
        data->loadLazyFlagsResult(DYN_SRC, DYN_16bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_16bit, DYN_EQUALS, false, false);
        data->evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_16bit, DYN_LESS_THAN_UNSIGNED, false, false);
        data->loadLazyFlagsOldCF(DYN_DEST);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        data->instRegReg('&', reg, DYN_DEST, DYN_32bit, true);
        data->instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_ADC32) {
        // (cpu->result.u32 < cpu->dst.u32) || (cpu->oldCF && (cpu->result.u32 == cpu->dst.u32));
        data->loadLazyFlagsResult(DYN_SRC, DYN_32bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_32bit, DYN_EQUALS, false, false);
        data->evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_32bit, DYN_LESS_THAN_UNSIGNED, false, false);
        data->loadLazyFlagsOldCF(DYN_DEST);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        data->instRegReg('&', reg, DYN_DEST, DYN_32bit, true);
        data->instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_SBB8) {
        // (cpu->dst.u8 < cpu->result.u8) || (cpu->oldCF && (cpu->src.u8==0xff));
        data->loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0xff, DYN_8bit, DYN_EQUALS, true, false);
        data->loadLazyFlagsOldCF(DYN_SRC);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        data->instRegReg('&', reg, DYN_SRC, DYN_32bit, true);

        data->loadLazyFlagsResult(DYN_SRC, DYN_8bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        data->evaluateToReg(DYN_SRC, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_UNSIGNED, true, false);

        data->instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_SBB16) {
        // (cpu->dst.u16 < cpu->result.u16) || (cpu->oldCF && (cpu->src.u16==0xffff));
        data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0xffff, DYN_16bit, DYN_EQUALS, true, false);
        data->loadLazyFlagsOldCF(DYN_SRC);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        data->instRegReg('&', reg, DYN_SRC, DYN_32bit, true);

        data->loadLazyFlagsResult(DYN_SRC, DYN_16bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        data->evaluateToReg(DYN_SRC, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_UNSIGNED, true, false);

        data->instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_SBB32) {
        // (cpu->dst.u32 < cpu->result.u32) || (cpu->oldCF && (cpu->src.u32==0xffffffff));
        data->loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0xffffffff, DYN_32bit, DYN_EQUALS, true, false);
        data->loadLazyFlagsOldCF(DYN_SRC);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        data->instRegReg('&', reg, DYN_SRC, DYN_32bit, true);

        data->loadLazyFlagsResult(DYN_SRC, DYN_32bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        data->evaluateToReg(DYN_SRC, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_UNSIGNED, true, false);

        data->instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_AND8) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_AND16) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_AND32) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SUB8) {
        // cpu->dst.u8<cpu->src.u8;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_SUB16) {
        // cpu->dst.u16<cpu->src.u16;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_SUB32) {
        // cpu->dst.u32<cpu->src.u32;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_XOR8) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_XOR16) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_XOR32) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_INC8) {
        // cpu->oldCF;
        data->loadLazyFlagsOldCF(reg);
    } else if (flags == FLAGS_INC16) {
        // cpu->oldCF;
        data->loadLazyFlagsOldCF(reg);
    } else if (flags == FLAGS_INC32) {
        // cpu->oldCF;
        data->loadLazyFlagsOldCF(reg);
    } else if (flags == FLAGS_DEC8) {
        // cpu->oldCF;
        data->loadLazyFlagsOldCF(reg);
    } else if (flags == FLAGS_DEC16) {
        // cpu->oldCF;
        data->loadLazyFlagsOldCF(reg);
    } else if (flags == FLAGS_DEC32) {
        // cpu->oldCF;
        data->loadLazyFlagsOldCF(reg);
    } else if (flags == FLAGS_SHL8) {
        // ((cpu->dst.u8 << (cpu->src.u8-1)) & 0x80) >> 7
        data->loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        data->loadLazyFlagsDst(reg, DYN_8bit);
        data->instRegImm('-', DYN_SRC, DYN_8bit, 1);
        data->instRegReg('<', reg, DYN_SRC, DYN_8bit, true);
        data->instRegImm('&', reg, DYN_8bit, 0x80);
        data->instRegImm('>', reg, DYN_8bit, 7);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHL16) {
        // ((cpu->dst.u16 << (cpu->src.u8-1)) & 0x8000)>>15
        data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        data->loadLazyFlagsDst(reg, DYN_16bit);
        data->instRegImm('-', DYN_SRC, DYN_16bit, 1);
        data->instRegReg('<', reg, DYN_SRC, DYN_16bit, true);
        data->instRegImm('&', reg, DYN_16bit, 0x8000);
        data->instRegImm('>', reg, DYN_16bit, 15);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHL32) {
        // (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;
        data->loadLazyFlagsSrc(DYN_DEST, DYN_32bit); // ok to use src.u32 instead of src.u8
        data->loadLazyFlagsDst(reg, DYN_32bit);
        data->movToReg(DYN_SRC, DYN_32bit, 32);
        data->instRegReg('-', DYN_SRC, DYN_DEST, DYN_32bit, true);
        data->instRegReg('>', reg, DYN_SRC, DYN_32bit, true); // on x86, shift by reg, reg must be cl
        data->instRegImm('&', reg, DYN_32bit, 1);
    } else if (flags == FLAGS_SHR8 || flags == FLAGS_SHR8_N1) {
        // (cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        data->loadLazyFlagsDst(reg, DYN_8bit);
        data->instRegImm('-', DYN_SRC, DYN_8bit, 1);
        data->instRegReg('>', reg, DYN_SRC, DYN_8bit, true);
        data->instRegImm('&', reg, DYN_8bit, 0x1);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHR16 || flags == FLAGS_SHR16_N1) {
        // (cpu->dst.u16 >> (cpu->src.u8 - 1)) & 1;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        data->loadLazyFlagsDst(reg, DYN_16bit);
        data->instRegImm('-', DYN_SRC, DYN_16bit, 1);
        data->instRegReg('>', reg, DYN_SRC, DYN_16bit, true);
        data->instRegImm('&', reg, DYN_16bit, 1);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHR32 || flags == FLAGS_SHR32_N1) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_32bit); // ok to use src.u32 instead of src.u8
        data->loadLazyFlagsDst(reg, DYN_32bit);
        data->instRegImm('-', DYN_SRC, DYN_32bit, 1);
        data->instRegReg('>', reg, DYN_SRC, DYN_32bit, true);
        data->instRegImm('&', reg, DYN_32bit, 1);
    } else if (flags == FLAGS_SHR8_1) {
        // cpu->dst.u8 & 1;
        data->loadLazyFlagsDst(reg, DYN_8bit);
        data->instRegImm('&', reg, DYN_8bit, 0x1);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHR16_1) {
        // cpu->dst.u16 & 1;
        data->loadLazyFlagsDst(reg, DYN_16bit);
        data->instRegImm('&', reg, DYN_16bit, 1);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHR32_1) {
        // cpu->dst.u32  & 1;
        data->loadLazyFlagsDst(reg, DYN_32bit);
        data->instRegImm('&', reg, DYN_32bit, 1);
    } else if (flags == FLAGS_SAR8) {
        // (((S8) cpu->dst.u8) >> (cpu->src.u8 - 1)) & 1;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        data->loadLazyFlagsDst(reg, DYN_8bit);
        data->instRegImm('-', DYN_SRC, DYN_8bit, 1);
        data->instRegReg(')', reg, DYN_SRC, DYN_8bit, true);
        data->instRegImm('&', reg, DYN_8bit, 0x1);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SAR16) {
        // (((S16) cpu->dst.u16) >> (cpu->src.u8 - 1)) & 1;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        data->loadLazyFlagsDst(reg, DYN_16bit);
        data->instRegImm('-', DYN_SRC, DYN_16bit, 1);
        data->instRegReg(')', reg, DYN_SRC, DYN_16bit, true);
        data->instRegImm('&', reg, DYN_16bit, 1);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SAR32) {
        // (((S32) cpu->dst.u32) >> (cpu->src.u8 - 1)) & 1;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_32bit); // ok to use src.u32 instead of src.u8
        data->loadLazyFlagsDst(reg, DYN_32bit);
        data->instRegImm('-', DYN_SRC, DYN_32bit, 1);
        data->instRegReg(')', reg, DYN_SRC, DYN_32bit, true);
        data->instRegImm('&', reg, DYN_32bit, 1);
    } else if (flags == FLAGS_TEST8) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_TEST16) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_TEST32) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_DSHL16) {
        // ((cpu->dst.u16 << (cpu->src.u8-1)) & 0x8000)>>15;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        data->loadLazyFlagsDst(reg, DYN_16bit);
        data->instRegImm('-', DYN_SRC, DYN_16bit, 1);
        data->instRegReg('<', reg, DYN_SRC, DYN_16bit, true);
        data->instRegImm('&', reg, DYN_16bit, 0x8000);
        data->instRegImm('>', reg, DYN_16bit, 15);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_DSHL32) {
        // (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;
        data->loadLazyFlagsSrc(DYN_DEST, DYN_32bit); // ok to use src.u32 instead of src.u8
        data->loadLazyFlagsDst(reg, DYN_32bit);
        data->movToReg(DYN_SRC, DYN_32bit, 32);
        data->instRegReg('-', DYN_SRC, DYN_DEST, DYN_32bit, true); // on x86, shift by reg, reg must be cl
        data->instRegReg('>', reg, DYN_SRC, DYN_32bit, true);
        data->instRegImm('&', reg, DYN_32bit, 1);
    } else if (flags == FLAGS_DSHR16) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        data->loadLazyFlagsDst(reg, DYN_16bit);
        data->instRegImm('-', DYN_SRC, DYN_16bit, 1);
        data->instRegReg('>', reg, DYN_SRC, DYN_16bit, true);
        data->instRegImm('&', reg, DYN_16bit, 1);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_DSHR32) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_32bit); // ok to use src.u32 instead of src.u8
        data->loadLazyFlagsDst(reg, DYN_32bit);
        data->instRegImm('-', DYN_SRC, DYN_32bit, 1);
        data->instRegReg('>', reg, DYN_SRC, DYN_32bit, true);
        data->instRegImm('&', reg, DYN_32bit, 1);
    } else if (flags == FLAGS_NEG8) {
        // cpu->src.u8!=0;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_8bit, DYN_NOT_EQUALS, true, false);
    } else if (flags == FLAGS_NEG16) {
        // cpu->src.u16!=0;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_16bit, DYN_NOT_EQUALS, true, false);
    } else if (flags == FLAGS_NEG32) {
        // cpu->src.u32!=0;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_NOT_EQUALS, true, false);
    } else {
        kpanic("genCF unknown flags");
    }
}

void genOF(DynamicData* data, const LazyFlags* flags, DynReg reg) {
    if (reg == DYN_SRC || reg == DYN_DEST) {
        kpanic("genOF expects reg not to be DYN_SRC or DYN_DEST");
    }
    if (data->regUsed[DYN_SRC] || data->regUsed[DYN_DEST]) {
        kpanic("genOF expects DYN_SRC and DYN_DEST to be available");
    }
    if (flags == FLAGS_NONE) {
        data->loadCPUFlags(reg);
        data->instRegImm('&', reg, DYN_32bit, OF);
    } else if (flags == FLAGS_ADD8 || flags == FLAGS_ADC8) {
        // ((cpu->dst.u8 ^ cpu->src.u8 ^ 0x80) & (cpu->result.u8 ^ cpu->src.u8)) & 0x80;        
        data->loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        data->loadLazyFlagsResult(reg, DYN_8bit);
        data->instRegReg('^', DYN_DEST, DYN_SRC, DYN_8bit, false);
        data->instRegImm('^', DYN_DEST, DYN_8bit, 0x80);
        data->instRegReg('^', reg, DYN_SRC, DYN_8bit, true);
        data->instRegReg('&', reg, DYN_DEST, DYN_8bit, true);
        data->instRegImm('&', reg, DYN_8bit, 0x80);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_ADD16 || flags == FLAGS_ADC16) {
        // ((cpu->dst.u16 ^ cpu->src.u16 ^ 0x8000) & (cpu->result.u16 ^ cpu->src.u16)) & 0x8000;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        data->loadLazyFlagsResult(reg, DYN_16bit);
        data->instRegReg('^', DYN_DEST, DYN_SRC, DYN_16bit, false);
        data->instRegImm('^', DYN_DEST, DYN_16bit, 0x8000);
        data->instRegReg('^', reg, DYN_SRC, DYN_16bit, true);
        data->instRegReg('&', reg, DYN_DEST, DYN_16bit, true);
        data->instRegImm('&', reg, DYN_16bit, 0x8000);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_ADD32 || flags == FLAGS_ADC32) {
        // ((cpu->dst.u32 ^ cpu->src.u32 ^ 0x80000000) & (cpu->result.u32 ^ cpu->src.u32)) & 0x80000000;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        data->loadLazyFlagsResult(reg, DYN_32bit);
        data->instRegReg('^', DYN_DEST, DYN_SRC, DYN_32bit, false);
        data->instRegImm('^', DYN_DEST, DYN_32bit, 0x80000000);
        data->instRegReg('^', reg, DYN_SRC, DYN_32bit, true);
        data->instRegReg('&', reg, DYN_DEST, DYN_32bit, true);
        data->instRegImm('&', reg, DYN_32bit, 0x80000000);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_32bit, false);
    } else if (flags == FLAGS_OR8) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_OR16) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_OR32) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_AND8) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_AND16) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_AND32) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SUB8 || flags == FLAGS_SBB8 || flags == FLAGS_CMP8) {
        // ((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        data->loadLazyFlagsResult(reg, DYN_8bit);
        data->instRegReg('^', DYN_SRC, DYN_DEST, DYN_8bit, false);
        data->instRegReg('^', reg, DYN_DEST, DYN_8bit, true);
        data->instRegReg('&', reg, DYN_SRC, DYN_8bit, true);
        data->instRegImm('&', reg, DYN_8bit, 0x80);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SUB16 || flags == FLAGS_SBB16 || flags == FLAGS_CMP16) {
        // ((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        data->loadLazyFlagsResult(reg, DYN_16bit);
        data->instRegReg('^', DYN_SRC, DYN_DEST, DYN_16bit, false);
        data->instRegReg('^', reg, DYN_DEST, DYN_16bit, true);
        data->instRegReg('&', reg, DYN_SRC, DYN_16bit, true);
        data->instRegImm('&', reg, DYN_16bit, 0x8000);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SUB32 || flags == FLAGS_SBB32 || flags == FLAGS_CMP32) {
        // ((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        data->loadLazyFlagsResult(reg, DYN_32bit);
        data->instRegReg('^', DYN_SRC, DYN_DEST, DYN_32bit, false);
        data->instRegReg('^', reg, DYN_DEST, DYN_32bit, true);
        data->instRegReg('&', reg, DYN_SRC, DYN_32bit, true);
        data->instRegImm('&', reg, DYN_32bit, 0x80000000);
    } else if (flags == FLAGS_XOR8) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_XOR16) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_XOR32) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_INC8) {
        // cpu->result.u8 == 0x80;
        data->loadLazyFlagsResult(reg, DYN_8bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80, DYN_8bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_INC16) {
        // cpu->result.u16 == 0x8000;
        data->loadLazyFlagsResult(reg, DYN_16bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x8000, DYN_16bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_INC32) {
        // cpu->result.u32 == 0x80000000;
        data->loadLazyFlagsResult(reg, DYN_32bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80000000, DYN_32bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_DEC8) {
        // cpu->result.u8 == 0x7f;
        data->loadLazyFlagsResult(reg, DYN_8bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x7f, DYN_8bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_DEC16) {
        // cpu->result.u16 == 0x7fff;
        data->loadLazyFlagsResult(reg, DYN_16bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x7fff, DYN_16bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_DEC32) {
        // cpu->result.u32 == 0x7fffffff;
        data->loadLazyFlagsResult(reg, DYN_32bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x7fffffff, DYN_32bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_SHL8) {
        // (cpu->result.u8 ^ cpu->dst.u8) & 0x80;
        data->loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        data->loadLazyFlagsResult(reg, DYN_8bit);
        data->instRegReg('^', reg, DYN_DEST, DYN_8bit, true);
        data->instRegImm('&', reg, DYN_8bit, 0x80);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHL16) {
        // (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;
        data->loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        data->loadLazyFlagsResult(reg, DYN_16bit);
        data->instRegReg('^', reg, DYN_DEST, DYN_16bit, true);
        data->instRegImm('&', reg, DYN_16bit, 0x8000);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHL32) {
        // (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;
        data->loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        data->loadLazyFlagsResult(reg, DYN_32bit);
        data->instRegReg('^', reg, DYN_DEST, DYN_32bit, true);
        data->instRegImm('&', reg, DYN_32bit, 0x80000000);
    } else if (flags == FLAGS_SHR8) {
        // if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u8 >= 0x80); else return 0;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        data->instRegImm('&', DYN_SRC, DYN_8bit, 0x1f);
        data->evaluateToReg(reg, DYN_8bit, DYN_SRC, true, DYN_NOT_SET, 1, DYN_8bit, DYN_EQUALS, true, false);
        data->instRegImm('>', DYN_DEST, DYN_8bit, 7); // mov top bit to bottom
        data->instRegReg('&', reg, DYN_DEST, DYN_8bit, true);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHR16) {
        // if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u16 >= 0x8000); else return 0;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        data->instRegImm('&', DYN_SRC, DYN_16bit, 0x1f);
        data->evaluateToReg(reg, DYN_16bit, DYN_SRC, true, DYN_NOT_SET, 1, DYN_16bit, DYN_EQUALS, true, false);
        data->instRegImm('>', DYN_DEST, DYN_16bit, 15); // mov top bit to bottom
        data->instRegReg('&', reg, DYN_DEST, DYN_16bit, true);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHR32) {
        // if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u32 >= 0x80000000); else return 0;
        data->loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        data->instRegImm('&', DYN_SRC, DYN_32bit, 0x1f);
        data->evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 1, DYN_32bit, DYN_EQUALS, true, false);
        data->instRegImm('>', DYN_DEST, DYN_32bit, 31); // mov top bit to bottom
        data->instRegReg('&', reg, DYN_DEST, DYN_32bit, true);
    } else if (flags == FLAGS_SHR8_1) {
        // (cpu->dst.u8 >= 0x80);
        data->loadLazyFlagsDst(reg, DYN_8bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80, DYN_8bit, DYN_GREATER_THAN_EQUAL_UNSIGNED, false, false);
    } else if (flags == FLAGS_SHR16_1) {
        // (cpu->dst.u16 >= 0x8000);
        data->loadLazyFlagsDst(reg, DYN_16bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x8000, DYN_16bit, DYN_GREATER_THAN_EQUAL_UNSIGNED, false, false);
    } else if (flags == FLAGS_SHR32_1) {
        // (cpu->dst.u32 >= 0x80000000);
        data->loadLazyFlagsDst(reg, DYN_32bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80000000, DYN_32bit, DYN_GREATER_THAN_EQUAL_UNSIGNED, false, false);
    } else if (flags == FLAGS_SHR8_N1) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SHR16_N1) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SHR32_N1) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SAR8) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SAR16) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SAR32) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_TEST8) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_TEST16) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_TEST32) {
        // 0
        data->movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_DSHL16 || flags == FLAGS_DSHR16) {
        // (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;
        data->loadLazyFlagsResult(reg, DYN_16bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        data->instRegReg('^', reg, DYN_DEST, DYN_16bit, true);
        data->instRegImm('&', reg, DYN_16bit, 0x8000);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_DSHL32 || flags == FLAGS_DSHR32) {
        // (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;
        data->loadLazyFlagsResult(reg, DYN_32bit);
        data->loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        data->instRegReg('^', reg, DYN_DEST, DYN_32bit, true);
        data->instRegImm('&', reg, DYN_32bit, 0x80000000);
    } else if (flags == FLAGS_NEG8) {
        // cpu->src.u8 == 0x80;
        data->loadLazyFlagsSrc(reg, DYN_8bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80, DYN_8bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_NEG16) {
        // return cpu->src.u16 == 0x8000;
        data->loadLazyFlagsSrc(reg, DYN_16bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x8000, DYN_16bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_NEG32) {
        // cpu->src.u32 == 0x80000000;
        data->loadLazyFlagsSrc(reg, DYN_32bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80000000, DYN_32bit, DYN_EQUALS, false, false);
    }
}

DynWidth getWidthOfCondition(const LazyFlags* flags) {
    if (flags->width==32)
        return DYN_32bit;
    if (flags->width==16)
        return DYN_16bit;
    if (flags->width==8)
        return DYN_8bit;
    kpanic_fmt("getWidthOfCondition: invalid flag width: %d", flags->width);
    return DYN_32bit;
}

void genNZ(DynamicData* data, const LazyFlags* flags, DynReg reg) {
    DynWidth width = getWidthOfCondition(flags);
    if (width==DYN_32bit) {
        data->loadLazyFlagsResult(reg, DYN_32bit);
    } else if (width==DYN_16bit) {
        data->loadLazyFlagsResult(reg, DYN_16bit);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (width==DYN_8bit) {
        data->loadLazyFlagsResult(reg, DYN_8bit);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else {
        kpanic_fmt("setConditionInReg: unknown condition width: %d", width);
    }
}

void genZ(DynamicData* data, const LazyFlags* flags, DynReg reg) {
    DynWidth width = getWidthOfCondition(flags);
    if (width==DYN_32bit) {
        data->loadLazyFlagsResult(reg, DYN_32bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
    } else if (width==DYN_16bit) {
        data->loadLazyFlagsResult(reg, DYN_16bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_16bit, DYN_EQUALS, false, false);
    } else if (width==DYN_8bit) {
        data->loadLazyFlagsResult(reg, DYN_8bit);
        data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_8bit, DYN_EQUALS, false, false);
    } else {
        kpanic_fmt("setConditionInReg: unknown condition width: %d", width);
    }
}

void genS(DynamicData* data, const LazyFlags* flags, DynReg reg) {
    DynWidth width = getWidthOfCondition(flags);
    if (width==DYN_32bit) {
        data->loadLazyFlagsResult(reg, DYN_32bit);
        data->instRegImm('&', reg, DYN_32bit, 0x80000000);
    } else if (width==DYN_16bit) {
        data->loadLazyFlagsResult(reg, DYN_16bit);
        data->instRegImm('&', reg, DYN_16bit, 0x8000);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);            
    } else if (width==DYN_8bit) {
        data->loadLazyFlagsResult(reg, DYN_8bit);
        data->instRegImm('&', reg, DYN_8bit, 0x80);
        data->movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);            
    } else {
        kpanic_fmt("setConditionInReg: unknown condition width: %d", width);
    }
}

bool getFlagInReg(DynamicData* data, DynConditional condition, DynReg reg) {
    U32 flag = 0;
    bool notFlag = false;

    switch (condition) {
    case O: flag = OF; break;
    case NO: flag = OF; notFlag = true; break;
    case B: flag = CF; break;
    case NB: flag = CF; notFlag = true; break;
    case Z: flag = ZF; break;
    case NZ: flag = ZF; notFlag = true; break;
    case S: flag = SF; break;
    case NS: flag = SF; notFlag = true; break;
    case P: flag = PF; break;
    case NP: flag = PF; notFlag = true; break;
    default: 
        return false;
    }

    data->loadCPUFlags(reg);
    if (notFlag)
        data->instReg('~', reg, DYN_32bit);
    data->instRegImm('&', reg, DYN_32bit, flag);
    return true;
}

void getCondition(DynamicData* data, DynConditional condition, DynReg reg) {
    if (data->regUsed[DYN_CALL_RESULT] && reg != DYN_CALL_RESULT) {
        kpanic("getCondition expects DYN_CALL_RESULT to be available");
    }
    switch (condition) {
    case O: data->callHostFunction((void*)common_condition_o, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NO: data->callHostFunction((void*)common_condition_no, true, 1, 0, DYN_PARAM_CPU, false); break;
    case B: data->callHostFunction((void*)common_condition_b, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NB: data->callHostFunction((void*)common_condition_nb, true, 1, 0, DYN_PARAM_CPU, false); break;
    case Z: data->callHostFunction((void*)common_condition_z, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NZ: data->callHostFunction((void*)common_condition_nz, true, 1, 0, DYN_PARAM_CPU, false); break;
    case BE: data->callHostFunction((void*)common_condition_be, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NBE: data->callHostFunction((void*)common_condition_nbe, true, 1, 0, DYN_PARAM_CPU, false); break;
    case S: data->callHostFunction((void*)common_condition_s, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NS: data->callHostFunction((void*)common_condition_ns, true, 1, 0, DYN_PARAM_CPU, false); break;
    case P: data->callHostFunction((void*)common_condition_p, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NP: data->callHostFunction((void*)common_condition_np, true, 1, 0, DYN_PARAM_CPU, false); break;
    case L: data->callHostFunction((void*)common_condition_l, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NL: data->callHostFunction((void*)common_condition_nl, true, 1, 0, DYN_PARAM_CPU, false); break;
    case LE: data->callHostFunction((void*)common_condition_le, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NLE: data->callHostFunction((void*)common_condition_nle, true, 1, 0, DYN_PARAM_CPU, false); break;
    default:
        kpanic_fmt("setConditionInReg: unknown condition %d", condition);
    }
    if (reg != DYN_CALL_RESULT) {
        data->movToRegFromReg(reg, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    }
}

// by inlining flag calculations, we save a virtual function call or 2.  This really helps performance for Quake 2, around 10%.
// The if statement to check if the lazyFlags are still what we thought they would be are for when code in another location
// jumps here with a different lazyFlags, its rare so the hardware branch prediction can handle this check for free.  I have
// seen this happen with iexplore.
void setConditionInReg(DynamicData* data, DynConditional condition, DynReg reg) {
    if ((data->regUsed[DYN_DEST] && reg != DYN_DEST) || (data->regUsed[DYN_SRC] && reg != DYN_SRC)) {
        kpanic("setConditionInReg expects DYN_DEST to be available");
    }
    if (!data->currentLazyFlags || condition == P || condition == NP) {
        getCondition(data, condition, reg);
        return;
    }    
    data->loadLazyFlags(reg);
    data->IfPtrEqual(reg, (DYN_PTR_SIZE)data->currentLazyFlags, false);
    if (data->currentLazyFlags==FLAGS_NONE) {
        if (!getFlagInReg(data, condition, reg)) {
            getCondition(data, condition, reg);
        }
    } else if (condition==NZ) {
        genNZ(data, data->currentLazyFlags, reg);
    } else if (condition==Z) {
        genZ(data, data->currentLazyFlags, reg);
    } else if (condition==S) {
        genS(data, data->currentLazyFlags, reg);
    } else if (condition==B) {
        genCF(data, data->currentLazyFlags, reg);
    } else if (condition==O) {
        genOF(data, data->currentLazyFlags, reg);
    } else if (condition==BE) {
        if (data->currentLazyFlags==FLAGS_SUB8) {
            data->loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
            data->loadLazyFlagsDst(DYN_DEST, DYN_8bit);
            data->evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_EQUAL_UNSIGNED, true, true);
        } else if (data->currentLazyFlags==FLAGS_SUB16) {
            data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
            data->loadLazyFlagsDst(DYN_DEST, DYN_16bit);
            data->evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_EQUAL_UNSIGNED, true, true);
        } else if (data->currentLazyFlags==FLAGS_SUB32) {
            data->loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
            data->loadLazyFlagsDst(DYN_DEST, DYN_32bit);
            data->evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_EQUAL_UNSIGNED, true, true);
        }  else {
            // cpu->getZF() || cpu->getCF()
            genCF(data, data->currentLazyFlags, reg);
            // must come after genCF, since genCF can clobber DYN_SRC
            genZ(data, data->currentLazyFlags, DYN_SRC);
            data->instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
        }
    } else if (condition==L) {
        if (data->currentLazyFlags==FLAGS_SUB8) {
            data->loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
            data->loadLazyFlagsDst(DYN_DEST, DYN_8bit);
            data->evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_SIGNED, true, true);
        } else if (data->currentLazyFlags==FLAGS_SUB16) {
            data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
            data->loadLazyFlagsDst(DYN_DEST, DYN_16bit);
            data->evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_SIGNED, true, true);
        } else if (data->currentLazyFlags==FLAGS_SUB32) {
            data->loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
            data->loadLazyFlagsDst(DYN_DEST, DYN_32bit);
            data->evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_SIGNED, true, true);
        } else {
            // cpu->getSF()!=cpu->getOF()        
            genOF(data, data->currentLazyFlags, reg);
            genS(data, data->currentLazyFlags, DYN_SRC);
            data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
            data->evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
            data->evaluateToReg(reg, DYN_32bit, reg, false, DYN_SRC, 0, DYN_32bit, DYN_NOT_EQUALS, false, true);
        }
    } else if (condition==LE) {
        if (data->currentLazyFlags==FLAGS_SUB8) {
            data->loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
            data->loadLazyFlagsDst(DYN_DEST, DYN_8bit);
            data->evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_EQUAL_SIGNED, true, true);
        } else if (data->currentLazyFlags==FLAGS_SUB16) {
            data->loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
            data->loadLazyFlagsDst(DYN_DEST, DYN_16bit);
            data->evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_EQUAL_SIGNED, true, true);
        } else if (data->currentLazyFlags==FLAGS_SUB32) {
            data->loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
            data->loadLazyFlagsDst(DYN_DEST, DYN_32bit);
            data->evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_EQUAL_SIGNED, true, true);
        } else {
            // cpu->getZF() || cpu->getSF()!=cpu->getOF()       
            genOF(data, data->currentLazyFlags, reg);
            genS(data, data->currentLazyFlags, DYN_SRC);
            data->evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
            data->evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
            data->evaluateToReg(reg, DYN_32bit, reg, false, DYN_SRC, 0, DYN_32bit, DYN_NOT_EQUALS, false, true);
            genZ(data, data->currentLazyFlags, DYN_SRC);
            data->instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
        }
    } else {
        kpanic("setConditionInReg unhandled condition");
    }
    data->StartElse();
    getCondition(data, condition, reg);
    data->EndIf();
}

void dynamic_pushReg32(DynamicData* data, DynReg reg, bool doneWithReg) {    
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        data->loadReg(4, DYN_ADDRESS, DYN_32bit, true); // need ESP in tmp reg so that we don't commit it until after write
        data->instRegImm('-', DYN_ADDRESS, DYN_32bit, 4);
        data->movToMemFromReg(DYN_ADDRESS, reg, DYN_32bit, true, doneWithReg, DYN_DEST); // need to discard DYN_ADDRESS, otherwise will be out of regs
        data->instCPUImm('-', CPU::offsetofReg32(4), DYN_32bit, 4, DYN_ADDRESS);
    } else {
        data->callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, doneWithReg);
    }
}

void dynamic_pop32(DynamicData* data) {
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        DynReg reg = data->loadReg(4, DYN_ADDRESS, DYN_32bit);
        data->movFromMem(DYN_32bit, reg, true);
        data->instCPUImm('+', CPU::offsetofReg32(4), DYN_32bit, 4, DYN_DEST);
    } else {
        data->callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
    }    
}

void dynamic_fillFlags(DynamicData* data) {
    data->callHostFunction((void*)common_fillFlags, false, 1, 0, DYN_PARAM_CPU, false);
    data->currentLazyFlags=FLAGS_NONE;
}

void dynamic_getCF(DynamicData* data) {
    if (data->currentLazyFlags) {
        data->loadLazyFlags(DYN_DEST);
        data->IfPtrEqual(DYN_DEST, (DYN_PTR_SIZE)data->currentLazyFlags, true);
        genCF(data, data->currentLazyFlags, DYN_CALL_RESULT);
        data->StartElse();
        data->callHostFunction((void*)common_getCF, true, 1, 0, DYN_PARAM_CPU, false);
        data->EndIf();
    } else {
        data->callHostFunction((void*)common_getCF, true, 1, 0, DYN_PARAM_CPU, false);
    }
}
