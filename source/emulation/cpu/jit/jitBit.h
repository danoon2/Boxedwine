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

#include "../common/common_bit.h"

RegPtr Jit::calculateEffectiveEaa32(DecodedOp* op) {
    RegPtr result = calculateEaa(op);
    RegPtr reg = getTmpReg(op->reg);

    sarValue(JitWidth::b32, reg, 5);
    shlValue(JitWidth::b32, reg, 2);
    addReg(JitWidth::b32, result, reg);
    return result;
}

RegPtr Jit::calculateEffectiveEaa16(DecodedOp* op) {
    RegPtr result = calculateEaa(op);
    RegPtr reg = getTmpReg(op->reg);

    sarValue(JitWidth::b16, reg, 4);
    shlValue(JitWidth::b16, reg, 1);
    movsx(JitWidth::b32, reg, JitWidth::b16, reg);
    addReg(JitWidth::b32, result, reg);
    return result;
}

RegPtr Jit::calculateEffectiveEaa(DecodedOp* op) {
    if (op->ea16) {
        return calculateEffectiveEaa16(op);
    }
    return calculateEffectiveEaa32(op);
}
bool Jit::btStartFlags(DecodedOp* op) {
    // The CF flag contains the value of the selected bit. The ZF flag is unaffected. The OF, SF, AF, and PF flags are undefined.
    U32 flagsNeeded = op->needsToSetFlags(cpu);
    if (flagsNeeded) {        
        if (op->getNeededFlagsAfter(ZF)) {
            RegPtr zf = getZF();
            andCPUFlagsImmV2(~ZF);
            orCPUFlags(zf);
        }
        //if (currentLazyFlags != FLAGS_NONE) {
        storeLazyFlagType(FLAGS_NONE);
            currentLazyFlags = FLAGS_NONE;
        //}
    }
    return flagsNeeded != 0;
}

RegPtr Jit::btMask(U32 bitMask, U32 reg) {
    RegPtr cl = getTmpReg(reg, false, 1);
    RegPtr mask = getTmpReg();
    movValue(JitWidth::b32, mask, 1);    
    andValue(JitWidth::b32, cl, bitMask);
    shlReg(JitWidth::b32, mask, cl);
    return mask;
}

void Jit::bitModifyMem(DecodedOp* op, JitWidth width, RegPtr address, BitModifyOp operation, bool registerBitIndex) {
    bool flags = op->needsToSetFlags(cpu) != 0;
    U32 bitMask = width == JitWidth::b16 ? 0xf : 0x1f;

    auto prepareWrite = [flags, op, width, bitMask, operation, registerBitIndex, this](RegPtr value) {
        if (flags) {
            btStartFlags(op);
        }
        RegPtr mask;
        if (registerBitIndex) {
            mask = btMask(bitMask, op->reg);
        }
        if (flags) {
            if (registerBitIndex) {
                IfTest(JitWidth::b32, value, mask); {
                    orCPUFlagsImmV2(CF);
                } StartElse(); {
                    andCPUFlagsImmV2(~CF);
                } EndIf();
            } else {
                IfTest(JitWidth::b32, value, op->imm); {
                    orCPUFlagsImmV2(CF);
                } StartElse(); {
                    andCPUFlagsImmV2(~CF);
                } EndIf();
            }
        }
        if (operation == BitModifyOp::Set) {
            if (registerBitIndex) {
                orReg(width, value, mask);
            } else {
                orValue(width, value, op->imm);
            }
        } else if (operation == BitModifyOp::Reset) {
            if (registerBitIndex) {
                notReg2(width, mask);
                andReg(width, value, mask);
            } else {
                andValue(width, value, ~op->imm);
            }
        } else {
            if (registerBitIndex) {
                xorReg(width, value, mask);
            } else {
                xorValue(width, value, op->imm);
            }
        }
    };

    if (!flags) {
        readWriteMem(width, std::move(address), std::move(prepareWrite));
        return;
    }

    RegPtr originalValue;
    RegPtr linearMask;
    bool savedResetBit = operation == BitModifyOp::Reset && registerBitIndex;
    auto prepareWriteLinear = [&originalValue, &linearMask, op, width, bitMask, operation, registerBitIndex, savedResetBit, this](RegPtr value) {
        if (operation != BitModifyOp::Complement) {
            originalValue = getTmpReg();
            mov(width, originalValue, value);
        }
        if (registerBitIndex) {
            linearMask = btMask(bitMask, op->reg);
        }
        if (savedResetBit) {
            // BTR consumes the mask while clearing the bit. Save just the old
            // bit so the mask register does not have to survive the store.
            andReg(width, originalValue, linearMask);
        }
        if (operation == BitModifyOp::Set) {
            if (registerBitIndex) {
                orReg(width, value, linearMask);
            } else {
                orValue(width, value, op->imm);
            }
        } else if (operation == BitModifyOp::Reset) {
            if (registerBitIndex) {
                notReg2(width, linearMask);
                andReg(width, value, linearMask);
                linearMask = nullptr;
            } else {
                andValue(width, value, ~op->imm);
            }
        } else {
            if (registerBitIndex) {
                xorReg(width, value, linearMask);
            } else {
                xorValue(width, value, op->imm);
            }
        }
    };
    auto commitWriteLinear = [&originalValue, &linearMask, op, width, operation, registerBitIndex, savedResetBit, this](RegPtr value) {
        btStartFlags(op);
        if (operation == BitModifyOp::Complement) {
            // The selected result bit is the inverse of the original carry.
            if (registerBitIndex) {
                IfTest(JitWidth::b32, value, linearMask); {
                    andCPUFlagsImmV2(~CF);
                } StartElse(); {
                    orCPUFlagsImmV2(CF);
                } EndIf();
            } else {
                IfTest(JitWidth::b32, value, op->imm); {
                    andCPUFlagsImmV2(~CF);
                } StartElse(); {
                    orCPUFlagsImmV2(CF);
                } EndIf();
            }
        } else if (savedResetBit) {
            If(width, originalValue); {
                orCPUFlagsImmV2(CF);
            } StartElse(); {
                andCPUFlagsImmV2(~CF);
            } EndIf();
        } else if (registerBitIndex) {
            IfTest(JitWidth::b32, originalValue, linearMask); {
                orCPUFlagsImmV2(CF);
            } StartElse(); {
                andCPUFlagsImmV2(~CF);
            } EndIf();
        } else {
            IfTest(JitWidth::b32, originalValue, op->imm); {
                orCPUFlagsImmV2(CF);
            } StartElse(); {
                andCPUFlagsImmV2(~CF);
            } EndIf();
        }
    };
    readWriteMemWithLinearPostCommit(width, std::move(address), std::move(prepareWrite),
        std::move(prepareWriteLinear), std::move(commitWriteLinear));
}

void Jit::dynamic_btr16r16(DecodedOp* op) {
    btStartFlags(op);
    IfTest(JitWidth::b32, getReadOnlyReg(op->reg), btMask(0xf, op->rm)); {
        orCPUFlagsImmV2(CF);
    } StartElse(); {
        andCPUFlagsImmV2(~CF);
    } EndIf();
}
void Jit::dynamic_btr16(DecodedOp* op) {
    btStartFlags(op);
    IfTest(JitWidth::b32, getReadOnlyReg(op->reg), op->imm); {
        orCPUFlagsImmV2(CF);
    } StartElse(); {
        andCPUFlagsImmV2(~CF);
    } EndIf();
}
void Jit::dynamic_bte16r16(DecodedOp* op) {
    btStartFlags(op);
    IfTest(JitWidth::b32, read(JitWidth::b16, calculateEffectiveEaa(op)), btMask(0xf, op->reg)); {
        orCPUFlagsImmV2(CF);
    } StartElse(); {
        andCPUFlagsImmV2(~CF);
    } EndIf();
}
void Jit::dynamic_bte16(DecodedOp* op) {
    btStartFlags(op);
    IfTest(JitWidth::b32, read(JitWidth::b16, calculateEaa(op)), op->imm); {
        orCPUFlagsImmV2(CF);
    } StartElse(); {
        andCPUFlagsImmV2(~CF);
    } EndIf();
}
void Jit::dynamic_btr32r32(DecodedOp* op) {
    auto fallback = [op, this]() {
        btStartFlags(op);
        IfTest(JitWidth::b32, getReadOnlyReg(op->reg), btMask(0x1f, op->rm)); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    };
    if (!supportsDirectFlagsForBitTest()) {
        fallback();
        return;
    }
    tryDirect(op, [op, this]() {
        direct_bit_test(JitWidth::b32, getReadOnlyReg(op->reg), getReadOnlyReg(op->rm));
    }, fallback, CF);
}
void Jit::dynamic_btr32(DecodedOp* op) {
    auto fallback = [op, this]() {
        btStartFlags(op);
        IfTest(JitWidth::b32, getReadOnlyReg(op->reg), op->imm); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    };
    if (!supportsDirectFlagsForBitTest()) {
        fallback();
        return;
    }
    tryDirect(op, [op, this]() {
        direct_bit_test(JitWidth::b32, getReadOnlyReg(op->reg), op->imm);
    }, fallback, CF);
}
void Jit::dynamic_bte32r32(DecodedOp* op) {
    auto fallback = [op, this]() {
        btStartFlags(op);
        IfTest(JitWidth::b32, read(JitWidth::b32, calculateEffectiveEaa(op)), btMask(0x1f, op->reg)); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    };
    if (!supportsDirectFlagsForBitTest()) {
        fallback();
        return;
    }
    tryDirect(op, [op, this]() {
        direct_bit_test(JitWidth::b32, read(JitWidth::b32, calculateEffectiveEaa(op)), getReadOnlyReg(op->reg));
    }, fallback, CF);
}
void Jit::dynamic_bte32(DecodedOp* op) {
    auto fallback = [op, this]() {
        btStartFlags(op);
        IfTest(JitWidth::b32, read(JitWidth::b32, calculateEaa(op)), op->imm); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    };
    if (!supportsDirectFlagsForBitTest()) {
        fallback();
        return;
    }
    tryDirect(op, [op, this]() {
        direct_bit_test(JitWidth::b32, read(JitWidth::b32, calculateEaa(op)), op->imm);
    }, fallback, CF);
}
void Jit::dynamic_btsr16r16(DecodedOp* op) {
    bool flags = btStartFlags(op);
    RegPtr mask = btMask(0xf, op->rm);
    RegPtr reg = getReg(op->reg);
    if (flags) {
        IfTest(JitWidth::b32, reg, mask); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    }
    orReg(JitWidth::b16, reg, mask);
}
void Jit::dynamic_btsr16(DecodedOp* op) {
    bool flags = btStartFlags(op);
    RegPtr reg = getReg(op->reg);
    if (flags) {
        IfTest(JitWidth::b32, reg, op->imm); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    }
    orValue(JitWidth::b16, reg, op->imm);
}
void Jit::dynamic_btse16r16(DecodedOp* op) {
    bitModifyMem(op, JitWidth::b16, calculateEffectiveEaa(op), BitModifyOp::Set, true);
}
void Jit::dynamic_btse16(DecodedOp* op) {
    bitModifyMem(op, JitWidth::b16, calculateEaa(op), BitModifyOp::Set, false);
}
void Jit::dynamic_btsr32r32(DecodedOp* op) {
    bool flags = btStartFlags(op);
    RegPtr mask = btMask(0x1f, op->rm);
    RegPtr reg = getReg(op->reg);
    if (flags) {
        IfTest(JitWidth::b32, reg, mask); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    }
    orReg(JitWidth::b32, reg, mask);
}
void Jit::dynamic_btsr32(DecodedOp* op) {
    bool flags = btStartFlags(op);
    RegPtr reg = getReg(op->reg);
    if (flags) {
        IfTest(JitWidth::b32, reg, op->imm); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    }
    orValue(JitWidth::b32, reg, op->imm);
}
void Jit::dynamic_btse32r32(DecodedOp* op) {
    bitModifyMem(op, JitWidth::b32, calculateEffectiveEaa(op), BitModifyOp::Set, true);
}
void Jit::dynamic_btse32(DecodedOp* op) {
    bitModifyMem(op, JitWidth::b32, calculateEaa(op), BitModifyOp::Set, false);
}
void Jit::dynamic_btrr16r16(DecodedOp* op) {
    bool flags = btStartFlags(op);
    RegPtr mask = btMask(0xf, op->rm);
    RegPtr reg = getReg(op->reg);
    if (flags) {
        IfTest(JitWidth::b32, reg, mask); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    }
    notReg2(JitWidth::b16, mask);
    andReg(JitWidth::b16, reg, mask);
}
void Jit::dynamic_btrr16(DecodedOp* op) {
    bool flags = btStartFlags(op);
    RegPtr reg = getReg(op->reg);
    if (flags) {
        IfTest(JitWidth::b32, reg, op->imm); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    }
    andValue(JitWidth::b16, reg, ~op->imm);
}
void Jit::dynamic_btre16r16(DecodedOp* op) {
    bitModifyMem(op, JitWidth::b16, calculateEffectiveEaa(op), BitModifyOp::Reset, true);
}
void Jit::dynamic_btre16(DecodedOp* op) {
    bitModifyMem(op, JitWidth::b16, calculateEaa(op), BitModifyOp::Reset, false);
}
void Jit::dynamic_btrr32r32(DecodedOp* op) {
    bool flags = btStartFlags(op);
    RegPtr mask = btMask(0x1f, op->rm);
    RegPtr reg = getReg(op->reg);
    if (flags) {
        IfTest(JitWidth::b32, reg, mask); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    }
    notReg2(JitWidth::b32, mask);
    andReg(JitWidth::b32, reg, mask);
}
void Jit::dynamic_btrr32(DecodedOp* op) {
    bool flags = btStartFlags(op);
    RegPtr reg = getReg(op->reg);
    if (flags) {
        IfTest(JitWidth::b32, reg, op->imm); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    }
    andValue(JitWidth::b32, reg, ~op->imm);
}
void Jit::dynamic_btre32r32(DecodedOp* op) {
    bitModifyMem(op, JitWidth::b32, calculateEffectiveEaa(op), BitModifyOp::Reset, true);
}
void Jit::dynamic_btre32(DecodedOp* op) {
    bitModifyMem(op, JitWidth::b32, calculateEaa(op), BitModifyOp::Reset, false);
}

void Jit::dynamic_btcr16r16(DecodedOp* op) {
    bool flags = btStartFlags(op);
    RegPtr mask = btMask(0xf, op->rm);
    RegPtr reg = getReg(op->reg);
    if (flags) {
        IfTest(JitWidth::b32, reg, mask); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    }
    xorReg(JitWidth::b16, reg, mask);
}
void Jit::dynamic_btcr16(DecodedOp* op) {
    bool flags = btStartFlags(op);
    RegPtr reg = getReg(op->reg);
    if (flags) {
        IfTest(JitWidth::b32, reg, op->imm); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    }
    xorValue(JitWidth::b16, reg, op->imm);
}
void Jit::dynamic_btce16r16(DecodedOp* op) {
    bitModifyMem(op, JitWidth::b16, calculateEffectiveEaa(op), BitModifyOp::Complement, true);
}
void Jit::dynamic_btce16(DecodedOp* op) {
    bitModifyMem(op, JitWidth::b16, calculateEaa(op), BitModifyOp::Complement, false);
}
void Jit::dynamic_btcr32r32(DecodedOp* op) {
    bool flags = btStartFlags(op);
    RegPtr mask = btMask(0x1f, op->rm);
    RegPtr reg = getReg(op->reg);
    if (flags) {
        IfTest(JitWidth::b32, reg, mask); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    }
    xorReg(JitWidth::b32, reg, mask);
}
void Jit::dynamic_btcr32(DecodedOp* op) {
    bool flags = btStartFlags(op);
    RegPtr reg = getReg(op->reg);
    if (flags) {
        IfTest(JitWidth::b32, reg, op->imm); {
            orCPUFlagsImmV2(CF);
        } StartElse(); {
            andCPUFlagsImmV2(~CF);
        } EndIf();
    }
    xorValue(JitWidth::b32, reg, op->imm);
}
void Jit::dynamic_btce32r32(DecodedOp* op) {
    bitModifyMem(op, JitWidth::b32, calculateEffectiveEaa(op), BitModifyOp::Complement, true);
}
void Jit::dynamic_btce32(DecodedOp* op) {
    bitModifyMem(op, JitWidth::b32, calculateEaa(op), BitModifyOp::Complement, false);
}
// bsf/bsr The ZF flag is set to 1 if the source operand is 0; otherwise, the ZF flag is cleared. The CF, OF, SF, AF, and PF flags are undefined.
// If the content of the source operand is 0, the content of the destination operand is undefined, but on hardware it looks like it just leaves dst untouched.
bool Jit::bsStartFlags(DecodedOp* op) {
    U32 flagsNeeded = op->needsToSetFlags(cpu);
    if (flagsNeeded) {
        if (currentLazyFlags != FLAGS_NONE) {
            storeLazyFlagType(FLAGS_NONE);
            currentLazyFlags = FLAGS_NONE;
        }
    }
    return flagsNeeded != 0;
}
void Jit::dynamic_bsfr16r16(DecodedOp* op) {
    bool flags = bsStartFlags(op);
    RegPtr src = getReadOnlyReg(op->rm);
    if (flags) {
        If(JitWidth::b16, src); {
            andCPUFlagsImmV2(~ZF);
            bsfReg(JitWidth::b16, getReg(op->reg), src);
        } StartElse(); {
            orCPUFlagsImmV2(ZF);            
        } EndIf();
    } else {
        If(JitWidth::b16, src); {
            bsfReg(JitWidth::b16, getReg(op->reg), src);
        } EndIf();
    }
}
void Jit::dynamic_bsfr16e16(DecodedOp* op) {
    bool flags = bsStartFlags(op);
    RegPtr src = read(JitWidth::b16, calculateEaa(op));
    if (flags) {
        If(JitWidth::b16, src); {
            andCPUFlagsImmV2(~ZF);
            bsfReg(JitWidth::b16, getReg(op->reg), src);
        } StartElse(); {
            orCPUFlagsImmV2(ZF);
        } EndIf();
    } else {
        If(JitWidth::b16, src); {
            bsfReg(JitWidth::b16, getReg(op->reg), src);
        } EndIf();
    }
}
void Jit::dynamic_bsfr32r32(DecodedOp* op) {
    bool flags = bsStartFlags(op);
    RegPtr src = getReadOnlyReg(op->rm);
    if (flags) {
        If(JitWidth::b32, src); {
            andCPUFlagsImmV2(~ZF);
            bsfReg(JitWidth::b32, getReg(op->reg), src);
        } StartElse(); {
            orCPUFlagsImmV2(ZF);
        } EndIf();
    } else {
        If(JitWidth::b32, src); {
            bsfReg(JitWidth::b32, getReg(op->reg), src);
        } EndIf();
    }
}
void Jit::dynamic_bsfr32e32(DecodedOp* op) {
    bool flags = bsStartFlags(op);
    RegPtr src = read(JitWidth::b32, calculateEaa(op));
    if (flags) {
        If(JitWidth::b32, src); {
            andCPUFlagsImmV2(~ZF);
            bsfReg(JitWidth::b32, getReg(op->reg), src);
        } StartElse(); {
            orCPUFlagsImmV2(ZF);
        } EndIf();
    } else {
        If(JitWidth::b32, src); {
            bsfReg(JitWidth::b32, getReg(op->reg), src);
        } EndIf();
    }
}
void Jit::dynamic_bsrr16r16(DecodedOp* op) {
    bool flags = bsStartFlags(op);
    RegPtr src = getReadOnlyReg(op->rm);
    if (flags) {
        If(JitWidth::b16, src); {
            andCPUFlagsImmV2(~ZF);
            bsrReg(JitWidth::b16, getReg(op->reg), src);
        } StartElse(); {
            orCPUFlagsImmV2(ZF);
        } EndIf();
    } else {
        If(JitWidth::b16, src); {
            bsrReg(JitWidth::b16, getReg(op->reg), src);
        } EndIf();
    }
}
void Jit::dynamic_bsrr16e16(DecodedOp* op) {
    bool flags = bsStartFlags(op);
    RegPtr src = read(JitWidth::b16, calculateEaa(op));
    if (flags) {
        If(JitWidth::b16, src); {
            andCPUFlagsImmV2(~ZF);
            bsrReg(JitWidth::b16, getReg(op->reg), src);
        } StartElse(); {
            orCPUFlagsImmV2(ZF);
        } EndIf();
    } else {
        If(JitWidth::b16, src); {
            bsrReg(JitWidth::b16, getReg(op->reg), src);
        } EndIf();
    }
}
void Jit::dynamic_bsrr32r32(DecodedOp* op) {
    bool flags = bsStartFlags(op);
    RegPtr src = getReadOnlyReg(op->rm);
    if (flags) {
        If(JitWidth::b32, src); {
            andCPUFlagsImmV2(~ZF);
            bsrReg(JitWidth::b32, getReg(op->reg), src);
        } StartElse(); {
            orCPUFlagsImmV2(ZF);
        } EndIf();
    } else {
        If(JitWidth::b32, src); {
            bsrReg(JitWidth::b32, getReg(op->reg), src);
        } EndIf();
    }
}
void Jit::dynamic_bsrr32e32(DecodedOp* op) {
    bool flags = bsStartFlags(op);
    RegPtr src = read(JitWidth::b32, calculateEaa(op));
    if (flags) {
        If(JitWidth::b32, src); {
            andCPUFlagsImmV2(~ZF);
            bsrReg(JitWidth::b32, getReg(op->reg), src);
        } StartElse(); {
            orCPUFlagsImmV2(ZF);
        } EndIf();
    } else {
        If(JitWidth::b32, src); {
            bsrReg(JitWidth::b32, getReg(op->reg), src);
        } EndIf();
    }
}
