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

#include "boxedwine.h"

#ifdef BOXEDWINE_DYNAMIC
#include "jit.h"

#include "../normal/instructions.h"
#include "../common/common_arith.h"
#include "../common/common_pushpop.h"
#include "jitArith.h"
#include "jitMov.h"
#include "jitIncDec.h"
#include "jitJump.h"
#include "jitPushPop.h"
#include "jitStrings.h"
#include "jitShift.h"
#include "jitConditions.h"
#include "jitSetcc.h"
#include "jitXchg.h"
#include "jitBit.h"
#include "jitOther.h"
#include "jitCallMmx.h"
#include "jitCallSse.h"
#include "jitCallSse2.h"
#include "jitCallFPU.h"
#include "jitLock.h"

U8 JitReg::hardwareReg() {
    if (reg != 0xff) {
        return reg;
    }
    reg = delayedLoading();
    return reg;
}

RegPtr Jit::calculateEaaV2(DecodedOp* op, U32 popEspAmount) {
    if (op->ea16) {
        // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)
        RegPtr result = getTmpReg();

        xorReg(JitWidth::b32, result, result); // clear top bits

        if (popEspAmount && op->rm != 4 && op->sibIndex != 4) {
            popEspAmount = 0;
        } else if (popEspAmount && op->rm == 4 && op->sibIndex == 4) {
            popEspAmount *= 2;
        } else if (popEspAmount) {
            kpanic("found sp pop");
        }
        if (op->data.disp) {
            movValue(JitWidth::b16, result, op->data.disp + popEspAmount);
        } else if (popEspAmount) {
            movValue(JitWidth::b16, result, popEspAmount);
        }
        if (op->rm == op->sibIndex && op->rm != 8) {
            RegPtr reg = getReadOnlyReg(op->rm);
            addReg(JitWidth::b16, result, reg);
            addReg(JitWidth::b16, result, reg);
        } else {
            if (op->rm != 8) {
                addReg(JitWidth::b16, result, getReadOnlyReg(op->rm));
            }
            if (op->sibIndex != 8) {
                addReg(JitWidth::b16, result, getReadOnlyReg(op->sibIndex));
            }
        }

        // seg[6] is always 0
        if (op->base < 6) {
            // intentional 32-bit add
            addReg(JitWidth::b32, result, getReadOnlySegAddress(op->base));
        }
        return result;
    } else {
        RegPtr result;

        if (op->sibIndex != 8) {
            result = getTmpReg(op->sibIndex);
            if (op->sibScale) {
                shlValue(JitWidth::b32, result, op->sibScale);
            }

            if (op->rm != 8) {
                addReg(JitWidth::b32, result, getReadOnlyReg(op->rm));
            }
            if (op->data.disp) {
                addValue(JitWidth::b32, result, op->data.disp);
            }
        } else if (op->rm != 8) {
            result = getTmpReg(op->rm);
            if (op->data.disp) {
                addValue(JitWidth::b32, result, op->data.disp);
            }
        } else if (op->data.disp) {
            result = getTmpReg();
            movValue(JitWidth::b32, result, op->data.disp);
        } else {
            result = getTmpReg();
            xorReg(JitWidth::b32, result, result);
        }

        // seg[6] is always 0
        if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
            addReg(JitWidth::b32, result, getReadOnlySegAddress(op->base));
        }
        return result;
    }
}

void Jit::dynamic_sidt(DecodedOp* op) {
}

static void dynamic_onExitSignal(CPU* cpu) {
    onExitSignal(cpu, NULL);
}

void Jit::dynamic_callback(DecodedOp* op) {
    if (op->pfn == onExitSignal) {
        call(dynamic_onExitSignal);
    } else {
        kpanic("x32CPU::x32_callback unhandled callback");
    }
}

void Jit::dynamic_invalid_op(DecodedOp* op) {
    //kpanic_fmt("Invalid instruction %x\n", op->inst);
    call(common_ud2);
    blockDone(true);
}

void Jit::dynamic_onTestEnd(DecodedOp* op) {
    onTestEnd(op);
}

void Jit::pushParam(std::vector<DynParam>& params, JitWidth width, RegPtr reg) {
    if (width == JitWidth::b32) {
        params.push_back(DynParam(JitCallParamType::REG_32, reg));
    } else if (width == JitWidth::b16) {
        params.push_back(DynParam(JitCallParamType::REG_16, reg));
    } else if (width == JitWidth::b8) {
        params.push_back(DynParam(JitCallParamType::REG_8, reg));
    } else {
        kpanic("Jit::callAndReturn");
    }
}

void Jit::call(CallNoArgs address) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    callHostFunction(address, params);
}

void Jit::call_I(CallI address, U32 value) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value));
    callHostFunction(address, params);
}

void Jit::call_R(CallR address, JitWidth width, RegPtr reg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    pushParam(params, width, reg);
    callHostFunction(address, params);
}

void Jit::call_II(CallII address, U32 value1, U32 value2) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value1));
    params.push_back(DynParam(JitCallParamType::CONST_32, value2));
    callHostFunction(address, params);
}

void Jit::call_II8(CallII8 address, U32 value1, U32 value2) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value1));
    params.push_back(DynParam(JitCallParamType::CONST_32, value2));
    callHostFunction(address, params);
}

void Jit::call_IR(CallIR address, U32 value, JitWidth width, RegPtr reg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value));
    pushParam(params, width, reg);
    callHostFunction(address, params);
}

void Jit::call_RI(CallRI address, JitWidth width, RegPtr reg, U32 value) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    pushParam(params, width, reg);
    params.push_back(DynParam(JitCallParamType::CONST_32, value));
    callHostFunction(address, params);
}

void Jit::call_RR(CallRR address, JitWidth width, RegPtr reg, JitWidth width2, RegPtr reg2) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    pushParam(params, width, reg);
    pushParam(params, width2, reg2);
    callHostFunction(address, params);
}

void Jit::call_III(CallIII address, U32 value1, U32 value2, U32 value3) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value1));
    params.push_back(DynParam(JitCallParamType::CONST_32, value2));
    params.push_back(DynParam(JitCallParamType::CONST_32, value3));
    callHostFunction(address, params);
}

void Jit::call_III8(CallIII8 address, U32 value1, U32 value2, U32 value3) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value1));
    params.push_back(DynParam(JitCallParamType::CONST_32, value2));
    params.push_back(DynParam(JitCallParamType::CONST_32, value3));
    callHostFunction(address, params);
}

void Jit::call_IIR(CallIIR address, U32 value1, U32 value2, JitWidth width3, RegPtr reg3) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value1));
    params.push_back(DynParam(JitCallParamType::CONST_32, value2));
    pushParam(params, width3, reg3);
    callHostFunction(address, params);
}

void Jit::call_IRI8(CallIRI8 address, U32 value1, JitWidth width2, RegPtr reg2, U32 value3) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value1));
    pushParam(params, width2, reg2);
    params.push_back(DynParam(JitCallParamType::CONST_32, value3));
    callHostFunction(address, params);
}

void Jit::call_RII(CallRII address, JitWidth width1, RegPtr reg1, U32 value2, U32 value3) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    pushParam(params, width1, reg1);
    params.push_back(DynParam(JitCallParamType::CONST_32, value2));
    params.push_back(DynParam(JitCallParamType::CONST_32, value3));
    callHostFunction(address, params);
}

void Jit::call_RRI(CallRRI address, JitWidth width1, RegPtr reg1, JitWidth width2, RegPtr reg2, U32 value3) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    pushParam(params, width1, reg1);
    pushParam(params, width2, reg2);
    params.push_back(DynParam(JitCallParamType::CONST_32, value3));
    callHostFunction(address, params);
}

void Jit::call_IIIR(CallIIIR address, U32 value1, U32 value2, U32 value3, JitWidth width, RegPtr reg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value1));
    params.push_back(DynParam(JitCallParamType::CONST_32, value2));
    params.push_back(DynParam(JitCallParamType::CONST_32, value3));
    pushParam(params, width, reg);
    callHostFunction(address, params);
}

void Jit::call_IRRR(CallIIIR address, U32 value1, JitWidth width1, RegPtr reg1, JitWidth width2, RegPtr reg2, JitWidth width3, RegPtr reg3) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value1));
    pushParam(params, width1, reg1);
    pushParam(params, width2, reg2);
    pushParam(params, width3, reg3);
    callHostFunction(address, params);
}

RegPtr Jit::callAndReturn(CallReturn address, RegPtr resultReg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    if (!resultReg) {
        resultReg = getTmpRegForCallResult();
    }
    callHostFunctionWithResult(resultReg, address, params);
    return resultReg;
}

RegPtr Jit::callAndReturnOp(CallReturnOp address, RegPtr resultReg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    if (!resultReg) {
        resultReg = getTmpRegForCallResult();
    }
    callHostFunctionWithResult(resultReg, address, params);
    return resultReg;
}

RegPtr Jit::callAndReturn_II(CallReturnII address, U32 value1, U32 value2) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value1));
    params.push_back(DynParam(JitCallParamType::CONST_32, value2));
    RegPtr result = getTmpRegForCallResult();
    callHostFunctionWithResult(result, address, params);
    return result;
}

RegPtr Jit::callAndReturn_R(CallReturnR address, JitWidth width, RegPtr reg, RegPtr resultReg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    pushParam(params, width, reg);
    if (!resultReg) {
        resultReg = getTmpRegForCallResult();
    }
    callHostFunctionWithResult(resultReg, address, params);
    return resultReg;
}

RegPtr Jit::callAndReturn_RS(CallReturnRS address, JitWidth width, RegPtr reg, RegPtr resultReg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    pushParam(params, width, reg);
    if (!resultReg) {
        resultReg = getTmpRegForCallResult();
    }
    callHostFunctionWithResult(resultReg, address, params);
    return resultReg;
}

RegPtr Jit::callAndReturn_IR(CallReturnIR address, U32 value, JitWidth width, RegPtr reg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value));
    pushParam(params, width, reg);
    RegPtr result = getTmpRegForCallResult();
    callHostFunctionWithResult(result, address, params);
    return result;
}

RegPtr Jit::callAndReturn_I(CallReturnI address, U32 value, RegPtr resultReg) {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    params.push_back(DynParam(JitCallParamType::CONST_32, value));
    if (!resultReg) {
        resultReg = getTmpRegForCallResult();
    }
    callHostFunctionWithResult(resultReg, address, params);
    return resultReg;
}

void Jit::arithSetup(DecodedOp* op, U32& needsToSetFlags, RegPtr& cf, const LazyFlags* flags, bool addCF) {
    if (flags) {
        needsToSetFlags = op->needsToSetFlags(cpu);
    }
    if (addCF) {
        cf = getCF();
    }
    if (needsToSetFlags) {
        if (flags && flags->usesOldCF(needsToSetFlags)) {
            storeLazyFlagsOldCF(cf);
        }
        storeLazyFlags(flags);
        currentLazyFlags = flags;
    }
}

void Jit::arithSetup(DecodedOp* op, U32& needsToSetFlags, const LazyFlags* flags) {
    if (flags) {
        needsToSetFlags = op->needsToSetFlags(cpu);
    }
    if (needsToSetFlags) {
        if (flags && !(instructionInfo[op->inst].flagsSets & CF) && op->getNeededFlagsAfter(CF)) {
            storeLazyFlagsOldCF(getCF());
        }
        storeLazyFlags(flags);
        currentLazyFlags = flags;
    }
}

void Jit::dynamic_MI(DecodedOp* op, JitWidth width, InstRegImm2 callback, const LazyFlags* flags, bool writeback, bool addCF, InstRegReg2 cfCallback) {
    U32 needsToSetFlags = 0;
    RegPtr cf;
    arithSetup(op, needsToSetFlags, cf, flags, addCF);

    if (flags && flags->usesSrc(needsToSetFlags)) {
        storeLazyFlagsSrc(op->imm);
    }

    if (writeback) {
        readWriteMem(width, calculateEaaV2(op), [cf, needsToSetFlags, flags, op, width, callback, cfCallback, this](RegPtr value) {
            if (flags && flags->usesDst(needsToSetFlags)) {
                storeLazyFlagsDest(value);
            }
            if (cf) {
                addValue(width, cf, op->imm);
                (this->*cfCallback)(width, value, cf);
            } else {
                (this->*callback)(width, value, op->imm);
            }
            if (flags && flags->usesResult(needsToSetFlags)) {
                storeLazyFlagsResult(value);
            }
            });
    } else {
        RegPtr dest = read(width, calculateEaaV2(op));

        if (flags && flags->usesDst(needsToSetFlags)) {
            storeLazyFlagsDest(dest);
        }
        if (cf) {
            addValue(width, cf, op->imm);
            (this->*cfCallback)(width, dest, cf);
        } else {
            (this->*callback)(width, dest, op->imm);
        }
        if (flags && flags->usesResult(needsToSetFlags)) {
            storeLazyFlagsResult(dest);
        }
    }
    incrementEip(op->len);
}

void Jit::dynamic_RI(DecodedOp* op, JitWidth width, InstRegImm2 callback, const LazyFlags* flags, bool writeback, bool addCF, InstRegReg2 cfCallback) {
    U32 needsToSetFlags = 0;
    RegPtr cf;
    arithSetup(op, needsToSetFlags, cf, flags, addCF);

    if (flags && flags->usesSrc(needsToSetFlags)) {
        storeLazyFlagsSrc(op->imm);
    }
    RegPtr dest;

    if (writeback) {
        if (width == JitWidth::b8) {
            dest = getReg8(op->reg);
        } else {
            dest = getReg(op->reg);
        }
    } else {
        if (width == JitWidth::b8) {
            dest = getTmpReg8(op->reg);
        } else {
            dest = getTmpReg(op->reg);
        }
    }
    if (flags && flags->usesDst(needsToSetFlags)) {
        storeLazyFlagsDest(dest);
    }
    if (cf) {
        addValue(width, cf, op->imm);
        (this->*cfCallback)(width, dest, cf);
    } else {
        (this->*callback)(width, dest, op->imm);
    }
    if (flags && flags->usesResult(needsToSetFlags)) {
        storeLazyFlagsResult(dest);
    }
    incrementEip(op->len);
}

void Jit::dynamic_MR(DecodedOp* op, JitWidth width, InstRegReg2 callback, const LazyFlags* flags, bool writeback, bool addCF) {
    U32 needsToSetFlags = 0;
    RegPtr cf;
    arithSetup(op, needsToSetFlags, cf, flags, addCF);

    if (writeback) {
        readWriteMem(width, calculateEaaV2(op), [needsToSetFlags, flags, cf, op, width, callback, this](RegPtr value) {
            RegPtr src;

            if (flags && flags->usesDst(needsToSetFlags)) {
                storeLazyFlagsDest(value);
            }
            if (width == JitWidth::b8) {
                if (cf) {
                    src = getTmpReg8(op->reg);
                } else {
                    src = getReadOnlyReg8(op->reg);
                }
            } else {
                if (cf) {
                    src = getTmpReg(op->reg);
                } else {
                    src = getReadOnlyReg(op->reg);
                }
            }
            if (flags && flags->usesSrc(needsToSetFlags)) {
                storeLazyFlagsSrc(src);
            }
            if (cf) {
                // after we stored src
                addReg(width, src, cf);
            }
            (this->*callback)(width, value, src);
            if (flags && flags->usesResult(needsToSetFlags)) {
                storeLazyFlagsResult(value);
            }
            });
    } else {
        if (addCF) {
            kpanic("Jit::dynamic_MR wasn't expecting addCF");
        }
        RegPtr dest = read(width, calculateEaaV2(op));

        if (flags && flags->usesDst(needsToSetFlags)) {
            storeLazyFlagsDest(dest);
        }

        RegPtr src;
        if (width == JitWidth::b8) {
            src = getReadOnlyReg8(op->reg);
        } else {
            src = getReadOnlyReg(op->reg);
        }
        if (flags && flags->usesSrc(needsToSetFlags)) {
            storeLazyFlagsSrc(src);
        }
        (this->*callback)(width, dest, src);
        if (flags && flags->usesResult(needsToSetFlags)) {
            storeLazyFlagsResult(dest);
        }
    }
    incrementEip(op->len);
}

void Jit::dynamic_RM(DecodedOp* op, JitWidth width, InstRegReg2 callback, const LazyFlags* flags, bool writeback, bool addCF) {
    U32 needsToSetFlags = 0;
    RegPtr cf;
    arithSetup(op, needsToSetFlags, cf, flags, addCF);

    RegPtr dest;
    RegPtr src = read(width, calculateEaaV2(op));

    if (flags && flags->usesSrc(needsToSetFlags)) {
        storeLazyFlagsSrc(src);
    }
    if (cf) {
        // after we stored src
        addReg(width, src, cf);
    }
    if (writeback) {
        if (width == JitWidth::b8) {
            dest = getReg8(op->reg);

        } else {
            dest = getReg(op->reg);
        }
    } else {
        if (width == JitWidth::b8) {
            dest = getTmpReg8(op->reg);
        } else {
            dest = getTmpReg(op->reg);
        }
    }
    if (flags && flags->usesDst(needsToSetFlags)) {
        storeLazyFlagsDest(dest);
    }
    (this->*callback)(width, dest, src);
    if (flags && flags->usesResult(needsToSetFlags)) {
        storeLazyFlagsResult(dest);
    }
    incrementEip(op->len);
}

void Jit::dynamic_RR(DecodedOp* op, JitWidth width, InstRegReg2 callback, const LazyFlags* flags, bool writeback, bool addCF) {
    U32 needsToSetFlags = 0;
    RegPtr cf;
    arithSetup(op, needsToSetFlags, cf, flags, addCF);

    if (writeback) {
        if (width == JitWidth::b8) {
            RegPtr reg = getReg8(op->reg);

            if (flags && flags->usesDst(needsToSetFlags)) {
                storeLazyFlagsDest(reg);
            }
            if (op->reg == op->rm && !cf) {
                if (flags && flags->usesSrc(needsToSetFlags)) {
                    storeLazyFlagsSrc(reg);
                }
                (this->*callback)(width, reg, reg);
            } else {
                RegPtr rm;

                if (cf) {
                    if (op->rm == op->reg) {
                        rm = getTmpReg8();
                        mov(width, rm, reg);
                    } else {
                        rm = getTmpReg8(op->rm);
                    }
                } else {
                    rm = getReadOnlyReg8(op->rm);
                }
                if (flags && flags->usesSrc(needsToSetFlags)) {
                    storeLazyFlagsSrc(rm);
                }
                if (cf) {
                    // after we stored src
                    addReg(width, rm, cf);
                }
                (this->*callback)(width, reg, rm);
            }
            if (flags && flags->usesResult(needsToSetFlags)) {
                storeLazyFlagsResult(reg);
            }
        } else {
            if (op->reg == op->rm && !cf) {
                RegPtr reg = getReg(op->reg);

                if (flags && flags->usesDst(needsToSetFlags)) {
                    storeLazyFlagsDest(reg);
                }
                if (flags && flags->usesSrc(needsToSetFlags)) {
                    storeLazyFlagsSrc(reg);
                }
                (this->*callback)(width, reg, reg);
                if (flags && flags->usesResult(needsToSetFlags)) {
                    storeLazyFlagsResult(reg);
                }
            } else {
                RegPtr reg = getReg(op->reg);
                RegPtr rm;

                if (cf) {
                    if (op->reg == op->rm) {
                        rm = getTmpReg();
                        mov(width, rm, reg);
                    } else {
                        rm = getTmpReg(op->rm);
                    }
                } else {
                    rm = getReadOnlyReg(op->rm);
                }
                if (flags && flags->usesDst(needsToSetFlags)) {
                    storeLazyFlagsDest(reg);
                }
                if (flags && flags->usesSrc(needsToSetFlags)) {
                    storeLazyFlagsSrc(rm);
                }
                if (cf) {
                    // after we stored src
                    addReg(width, rm, cf);
                }
                (this->*callback)(width, reg, rm);
                if (flags && flags->usesResult(needsToSetFlags)) {
                    storeLazyFlagsResult(reg);
                }
            }
        }
    } else {
        if (addCF) {
            kpanic("Jit::dynamic_RR wasn't expecting addCF");
        }
        if (width == JitWidth::b8) {
            RegPtr reg = getTmpReg8(op->reg);

            if (flags && flags->usesDst(needsToSetFlags)) {
                storeLazyFlagsDest(reg);
            }
            if (op->reg == op->rm) {
                if (flags && flags->usesSrc(needsToSetFlags)) {
                    storeLazyFlagsSrc(reg);
                }
                (this->*callback)(width, reg, reg);
            } else {
                RegPtr rm = getReadOnlyReg8(op->rm);

                if (flags && flags->usesSrc(needsToSetFlags)) {
                    storeLazyFlagsSrc(rm);
                }
                (this->*callback)(width, reg, rm);
            }
            if (flags && flags->usesResult(needsToSetFlags)) {
                storeLazyFlagsResult(reg);
            }
        } else {
            RegPtr reg = getTmpReg(op->reg);

            if (flags && flags->usesDst(needsToSetFlags)) {
                storeLazyFlagsDest(reg);
            }
            if (op->reg == op->rm) {
                if (flags && flags->usesSrc(needsToSetFlags)) {
                    storeLazyFlagsSrc(reg);
                }
                (this->*callback)(width, reg, reg);
            } else {
                RegPtr rm = getReadOnlyReg(op->rm);

                if (flags && flags->usesSrc(needsToSetFlags)) {
                    storeLazyFlagsSrc(rm);
                }
                (this->*callback)(width, reg, rm);
            }
            if (flags && flags->usesResult(needsToSetFlags)) {
                storeLazyFlagsResult(reg);
            }
        }
    }
    incrementEip(op->len);
}

void Jit::dynamic_R(DecodedOp* op, JitWidth width, InstReg2 callback, const LazyFlags* flags, bool writeback) {
    U32 needsToSetFlags = 0;
    arithSetup(op, needsToSetFlags, flags);

    RegPtr dest;

    if (writeback) {
        if (width == JitWidth::b8) {
            dest = getReg8(op->reg);
        } else {
            dest = getReg(op->reg);
        }
    } else {
        // mul/imul don't write back, they use EDX:EAX
        if (width == JitWidth::b8) {
            dest = getReadOnlyReg8(op->reg);
        } else {
            dest = getReadOnlyReg(op->reg);
        }
    }
    if (flags && flags->usesDst(needsToSetFlags)) {
        storeLazyFlagsDest(dest);
    }
    if (flags && flags->usesSrc(needsToSetFlags)) {
        storeLazyFlagsSrc(dest);
    }
    (this->*callback)(width, dest);
    if (flags && flags->usesResult(needsToSetFlags)) {
        storeLazyFlagsResult(dest);
    }
    incrementEip(op->len);
}

void Jit::dynamic_M(DecodedOp* op, JitWidth width, InstReg2 callback, const LazyFlags* flags, bool writeback, RegPtr tmp) {
    U32 needsToSetFlags = 0;
    arithSetup(op, needsToSetFlags, flags);

    if (writeback) {
        readWriteMem(width, calculateEaaV2(op), [needsToSetFlags, flags, op, width, callback, this](RegPtr value) {
            if (flags && flags->usesDst(needsToSetFlags)) {
                storeLazyFlagsDest(value);
            }
            if (flags && flags->usesSrc(needsToSetFlags)) {
                storeLazyFlagsSrc(value);
            }
            (this->*callback)(width, value);
            if (flags && flags->usesResult(needsToSetFlags)) {
                storeLazyFlagsResult(value);
            }
            });
    } else {
        RegPtr dest = read(width, calculateEaaV2(op), nullptr, nullptr, false, tmp);
        if (flags && flags->usesDst(needsToSetFlags)) {
            storeLazyFlagsDest(dest);
        }
        if (flags && flags->usesSrc(needsToSetFlags)) {
            storeLazyFlagsSrc(dest);
        }
        (this->*callback)(width, dest);
        if (flags && flags->usesResult(needsToSetFlags)) {
            kpanic("Jit::dynamic_M");
        }
    }
    incrementEip(op->len);
}

// SHL/SHR/SAR use this
// The CF flag contains the value of the last bit shifted out of the destination operand; it is undefined for SHL and SHR instructions where the count is greater than 
// or equal to the size(in bits) of the destination operand.The OF flag is affected only for 1 - bit shifts(see "Description" above); otherwise, it is undefined.The SF, 
// ZF, and PF flags are set according to the result.If the count is 0, the flags are not affected.For a non - zero count, the AF flag is undefined.
void Jit::dynamic_R_Cl(DecodedOp* op, JitWidth width, InstRegReg2 callback, const LazyFlags* flags) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);

    RegPtr dest;

    if (width == JitWidth::b8) {
        dest = getReg8(op->reg);
    } else {
        dest = getReg(op->reg);
    }

    if (!needsToSetFlags) {
        RegPtr src;
        if (width == JitWidth::b8) {
            src = getReadOnlyReg8(1, false, 1);
        } else {
            src = getReadOnlyReg(1, false, 1);
        }
        (this->*callback)(width, dest, src);
    } else {
        RegPtr src;
        if (width == JitWidth::b8) {
            src = getTmpReg8(1, false, 1);
        } else {
            src = getTmpReg(1, false, 1);
        }
        andValue(JitWidth::b8, src, 0x1f);
        If(JitWidth::b8, src); {
            storeLazyFlags(flags);
            if (flags && flags->usesDst(needsToSetFlags)) {
                storeLazyFlagsDest(dest);
            }
            if (flags && flags->usesSrc(needsToSetFlags)) {
                storeLazyFlagsSrc(src);
            }
            (this->*callback)(width, dest, src);
            if (flags && flags->usesResult(needsToSetFlags)) {
                storeLazyFlagsResult(dest);
            }
        } EndIf();
        currentLazyFlags = nullptr;
    }
    incrementEip(op->len);
}

void Jit::dynamic_M_Cl(DecodedOp* op, JitWidth width, InstRegReg2 callback, const LazyFlags* flags) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);

    if (!needsToSetFlags) {
        readWriteMem(width, calculateEaaV2(op), [op, width, callback, this](RegPtr value) {
            if (width == JitWidth::b8) {
                (this->*callback)(width, value, getReadOnlyReg8(1, true, 1));
            } else {
                (this->*callback)(width, value, getReadOnlyReg(1, true, 1));
            }
            });
    } else {
        RegPtr src;
        if (width == JitWidth::b8) {
            src = getTmpReg8(1, false, 1);
        } else {
            src = getTmpReg(1, false, 1);
        }
        andValue(JitWidth::b8, src, 0x1f);
        If(JitWidth::b8, src, true); {
            readWriteMem(width, calculateEaaV2(op), [needsToSetFlags, flags, src, op, width, callback, this](RegPtr value) {
                storeLazyFlags(flags);
                if (flags && flags->usesDst(needsToSetFlags)) {
                    storeLazyFlagsDest(value);
                }
                if (flags && flags->usesSrc(needsToSetFlags)) {
                    storeLazyFlagsSrc(src);
                }
                (this->*callback)(width, value, src);
                if (flags && flags->usesResult(needsToSetFlags)) {
                    storeLazyFlagsResult(value);
                }
                });
        } EndIf(true);
        currentLazyFlags = nullptr;
    }
    incrementEip(op->len);
}

void Jit::dynamic_RM_WriteM(DecodedOp* op, JitWidth width, InstRegReg2 callback, const LazyFlags* flags) {
    U32 needsToSetFlags = 0;
    arithSetup(op, needsToSetFlags, flags);

    if (!needsToSetFlags) {
        readWriteMem(width, calculateEaaV2(op), [op, width, callback, this](RegPtr value) {
            if (width == JitWidth::b8) {
                (this->*callback)(width, getReg8(op->reg), value);
            } else {
                (this->*callback)(width, getReg(op->reg), value);
            }
            });
    } else {
        RegPtr src;
        if (width == JitWidth::b8) {
            src = getReg8(op->reg);
        } else {
            src = getReg(op->reg);
        }

        if (flags && flags->usesSrc(needsToSetFlags)) {
            storeLazyFlagsSrc(src);
        }

        readWriteMem(width, calculateEaaV2(op), [needsToSetFlags, flags, src, op, width, callback, this](RegPtr value) {
            if (flags && flags->usesDst(needsToSetFlags)) {
                storeLazyFlagsDest(value);
            }
            (this->*callback)(width, src, value);
            if (flags && flags->usesResult(needsToSetFlags)) {
                storeLazyFlagsResult(value);
            }
            });
    }
    incrementEip(op->len);
}

void Jit::dynamic_RR_WriteBoth(DecodedOp* op, JitWidth width, InstRegReg2 callback, const LazyFlags* flags) {
    U32 needsToSetFlags = 0;
    arithSetup(op, needsToSetFlags, flags);

    RegPtr src;
    RegPtr dest;

    if (width == JitWidth::b8) {
        // dynamic_xchgr8r8 has the same issue, the API doesn't allow for the write of 2 registers that map to the same emulated register (AL/AH etc)
        if (op->rm == op->reg + 4) {
            src = getReg8(op->reg);
            dest = std::make_shared<JitReg>(src->hardwareReg(), 0xff);
            dest->isHigh = true;
        } else if (op->reg == op->rm + 4) {
            dest = getReg8(op->rm);
            src = std::make_shared<JitReg>(dest->hardwareReg(), 0xff);
            src->isHigh = true;
        } else if (op->reg == op->rm) {
            src = getReg8(op->reg);
            dest = src;
        } else {
            src = getReg8(op->reg);
            dest = getReg8(op->rm);
        }
    } else {
        src = getReg(op->reg);
        dest = getReg(op->rm);
    }
    if (flags && flags->usesSrc(needsToSetFlags)) {
        storeLazyFlagsSrc(src);
    }
    if (flags && flags->usesDst(needsToSetFlags)) {
        storeLazyFlagsDest(dest);
    }
    (this->*callback)(width, src, dest);
    if (flags && flags->usesResult(needsToSetFlags)) {
        storeLazyFlagsResult(dest);
    }
    incrementEip(op->len);
}

#endif