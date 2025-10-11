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
#ifdef BOXEDWINE_DYNAMIC32

#include "x32CPU.h"
#include "../dynamic/dynamic.h"
#include "x86Asm.h"

// cdecl calling convention states EAX, ECX, and EDX are caller saved

class X86DynamicData : DynamicData {
public:
    X86DynamicData(CPU* cpu) : DynamicData(cpu) {}

    void incrementEip(U32 inc) override;
    void setConditional(DynConditional condition) override;
    void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) override;
    void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void negReg(DynReg reg, DynWidth regWidth) override;
    void notReg(DynReg reg, DynWidth regWidth) override;
    void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm) override;
    void callHostFunction(void* address, bool hasReturn = false, U32 argCount = 0, U32 arg1 = 0, DynCallParamType arg1Type = DYN_PARAM_CONST_32, bool doneWithArg1 = true, U32 arg2 = 0, DynCallParamType arg2Type = DYN_PARAM_CONST_32, bool doneWithArg2 = true, U32 arg3 = 0, DynCallParamType arg3Type = DYN_PARAM_CONST_32, bool doneWithArg3 = true, U32 arg4 = 0, DynCallParamType arg4Type = DYN_PARAM_CONST_32, bool doneWithArg4 = true, U32 arg5 = 0, DynCallParamType arg5Type = DYN_PARAM_CONST_32, bool doneWithArg5 = true) override;
    void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) override;
    void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) override;
    void movToReg(DynReg reg, DynWidth width, U32 imm) override;
    void calculateEaa(DecodedOp* op, DynReg reg) override;
    void pushValue(U32 arg, DynCallParamType argType) override;
    void byteSwapReg32(DynReg reg) override;
    void zeroExtendReg16To32(DynReg dest, DynReg src) override;
    void JumpIf(DynReg reg, bool doneWithReg, U32 address) override;
    void JumpIfNot(DynReg reg, bool doneWithReg, U32 address) override;
    void JumpInBlock(U32 address) override;
    void IfNot(DynReg reg, bool doneWithReg) override;
    void If(DynReg reg, bool doneWithReg) override;
    void IfPtrEqual(DynReg reg, DYN_PTR_SIZE value, bool doneWithReg) override;
    void StartElse() override;
    void EndIf() override;
    void blockExit() override;

protected:
    friend void startNewJIT(CPU* cpu, U32 address, DecodedOp* op);

    void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) override;
    void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width) override;
    void movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm) override;    
    void IfLessThan(DynReg reg, U32 value, bool doneWithReg) override;
    void IfBitSet(DynReg reg, U32 value, bool doneWithReg) override;

    U32 getBufferSize() override;
    U32 getIfJumpSize() override;    
    U8* getBuffer() override;

    void IfEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg);
    void IfNotEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg);    
    void setCC(X86Asm::Reg32 reg, DynConditionEvaluate condition);
    void evaluateToReg(X86Asm::Reg32 reg, X86Asm::Reg32 left, X86Asm::Reg32 right, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg);
    void evaluateToReg(X86Asm::Reg32 reg, X86Asm::Reg32 left, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg);    

    void jmp(DynReg reg) override;
    void readMem(DynReg reg, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) override;    
    void readMem(DynReg reg, DynWidth width, DynReg address, U8 lsl, U32 disp) override;
    void writeMem(DynReg reg, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) override;
    void writeMem(DynReg reg, DynWidth width, DynReg address, U32 disp) override;
    void writeMem(U32 value, DynWidth width, DynReg address, U32 disp) override;
    void writeMem(U32 value, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) override;
    void and32(DynReg dst, U32 imm) override;
    void shr32(DynReg reg, U32 imm) override;

    void preCommitJIT() override;
    void patch(U8* begin) override;
    U8* createStartJITCode() override;

    X86Asm x86;
};

void X86DynamicData::jmp(DynReg reg) {
    x86.jmp(X86Asm::Reg32(reg));
}

void X86DynamicData::readMem(DynReg reg, DynWidth width, DynReg address, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(reg), X86Asm::Reg32(address), lsl, disp);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(reg), X86Asm::Reg32(address), lsl, disp);
    } else {
        x86.readMem(X86Asm::Reg8(reg), X86Asm::Reg32(address), lsl, disp);
    }
}

void X86DynamicData::readMem(DynReg reg, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(reg), X86Asm::Reg32(address), offset, lsl, disp);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(reg), X86Asm::Reg32(address), offset, lsl, disp);
    } else {
        x86.readMem(X86Asm::Reg8(reg), X86Asm::Reg32(address), offset, lsl, disp);
    }
}

void X86DynamicData::writeMem(DynReg reg, DynWidth width, DynReg address, U32 disp) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, X86Asm::Reg32(reg));
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, X86Asm::Reg16(reg));
    } else {
        x86.writeMem(X86Asm::Reg32(address), disp, X86Asm::Reg8(reg));
    }
}

void X86DynamicData::writeMem(U32 value, DynWidth width, DynReg address, U32 disp) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, value);
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, (U16)value);
    } else {
        x86.writeMem(X86Asm::Reg32(address), disp, (U8)value);
    }
}

void X86DynamicData::writeMem(DynReg reg, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, X86Asm::Reg32(reg));
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, X86Asm::Reg16(reg));
    } else {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, X86Asm::Reg8(reg));
    }
}

void X86DynamicData::writeMem(U32 value, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, value);
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, (U16)value);
    } else {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, (U8)value);
    }
}

void X86DynamicData::and32(DynReg dst, U32 imm) {
    x86.and_(X86Asm::Reg32(dst), imm);
}

void X86DynamicData::shr32(DynReg dst, U32 imm) {
    x86.shr(X86Asm::Reg32(dst), imm);
}

U32 X86DynamicData::getBufferSize() {
    return (U32)x86.buffer.size();
}

U8* X86DynamicData::getBuffer() {
    return x86.buffer.data();
}


U32 X86DynamicData::getIfJumpSize() {
    return (U32)x86.ifJump.size();
}

void X86DynamicData::blockExit() {
    x86.pop(x86.edi);
    x86.pop(x86.ebx);

#ifdef _DEBUG
    x86.mov(x86.esp, x86.ebp);
    x86.pop(x86.ebp);
#endif

    x86.ret();
}

void X86DynamicData::incrementEip(U32 inc) {
    x86.addMem(x86.edi, offsetof(CPU, eip.u32), inc);
}

void setConditionInReg(DynamicData* data, DynConditional condition, DynReg reg);

void X86DynamicData::setConditional(DynConditional condition) {
    bool setnz = true;
    // changing conditions to ones that are optimized
    if (condition == Z) {
        condition = NZ;
        setnz = false;
    } else if (condition == NS) {
        condition = S;
        setnz = false;
    } else if (condition == NB) {
        condition = B;
        setnz = false;
    } else if (condition == NO) {
        condition = O;
        setnz = false;
    } else if (condition == NBE) {
        condition = BE;
        setnz = false;
    } else if (condition == NLE) {
        condition = LE;
        setnz = false;
    } else if (condition == NL) {
        condition = L;
        setnz = false;
    }
    setConditionInReg(condition, DYN_CALL_RESULT);
    x86.test(x86.eax, x86.eax);
    if (setnz) {
        x86.setnz(X86Asm::Reg8(x86.eax.reg));
    } else {
        x86.setz(X86Asm::Reg8(x86.eax.reg));
    }
}

void X86DynamicData::setCC(X86Asm::Reg32 reg, DynConditionEvaluate condition) {

    switch (condition) {
    case DYN_EQUALS:
        x86.setz(X86Asm::Reg8(reg.reg));
        break;
    case DYN_NOT_EQUALS:
        x86.setnz(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_UNSIGNED:
        x86.setb(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_EQUAL_UNSIGNED:
        x86.setbe(X86Asm::Reg8(reg.reg));
        break;
    case DYN_GREATER_THAN_EQUAL_UNSIGNED:
        x86.setnb(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_SIGNED:
        x86.setl(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_EQUAL_SIGNED:
        x86.setle(X86Asm::Reg8(reg.reg));
        break;
    default:
        kpanic_fmt("x32CPU::evaluateToRegFromRegs unknown condition %d", condition);
    }
}

void X86DynamicData::evaluateToReg(X86Asm::Reg32 reg, X86Asm::Reg32 left, X86Asm::Reg32 right, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    if (regWidth == DYN_32bit) {
        x86.cmp(left, right);
    } else if (regWidth == DYN_16bit) {
        x86.cmp(X86Asm::Reg16(left.reg), X86Asm::Reg16(right.reg));
    } else if (regWidth == DYN_8bit) {
        x86.cmp(X86Asm::Reg8(left.reg), X86Asm::Reg8(right.reg));
    } else {
        kpanic_fmt("x32CPU::evaluateToReg reg width %d", regWidth);
    }

    regUsed[reg.reg] = true;
    setCC(reg, condition);
    x86.movzx(reg, X86Asm::Reg8(reg.reg));

    if (doneWithLeftReg) {
        regUsed[left.reg] = false;
    }
    if (doneWithRightReg) {
        regUsed[right.reg] = false;
    }
}

void X86DynamicData::evaluateToReg(X86Asm::Reg32 reg, X86Asm::Reg32 left, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg) {
    if (regWidth == DYN_32bit) {
        x86.cmp(left, rightConst);
    } else if (regWidth == DYN_16bit) {
        x86.cmp(X86Asm::Reg16(left.reg), (U16)rightConst);
    } else if (regWidth == DYN_8bit) {
        x86.cmp(X86Asm::Reg8(left.reg), (U8)rightConst);
    } else {
        kpanic_fmt("x32CPU::evaluateToReg reg width %d", regWidth);
    }

    regUsed[reg.reg] = true;
    setCC(reg, condition);
    x86.movzx(reg, X86Asm::Reg8(reg.reg));

    if (doneWithLeftReg) {
        regUsed[left.reg] = false;
    }
}

void X86DynamicData::evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    if (reg >= 4) {
        kpanic_fmt("x32CPU::evaluateToRegFromRegs doesn't support reg %d", reg);
    }
    if (isRightConst) {
        evaluateToReg(X86Asm::Reg32(reg), X86Asm::Reg32(left), rightConst, regWidth, condition, doneWithLeftReg);
    } else {
        evaluateToReg(X86Asm::Reg32(reg), X86Asm::Reg32(left), X86Asm::Reg32(right), regWidth, condition, doneWithLeftReg, doneWithRightReg);
    }
}

void X86DynamicData::callHostFunction(void* address, bool hasReturn, U32 argCount, U32 arg1, DynCallParamType arg1Type, bool doneWithArg1, U32 arg2, DynCallParamType arg2Type, bool doneWithArg2, U32 arg3, DynCallParamType arg3Type, bool doneWithArg3, U32 arg4, DynCallParamType arg4Type, bool doneWithArg4, U32 arg5, DynCallParamType arg5Type, bool doneWithArg5) {
    bool regDone[4] = { false, false, false, false };

    if (argCount >= 5) {
        if (isParamTypeReg(arg5Type) && doneWithArg5) {
            if (arg5 >= 4) {
                kpanic_fmt("x32CPU::callHostFunction bad param 5: arg=%d argType=%d", arg5, arg5Type);
            }
            regDone[arg5] = true;
        }
    }
    if (argCount >= 4) {
        if (isParamTypeReg(arg4Type) && doneWithArg4) {
            if (arg4 >= 4) {
                kpanic_fmt("x32CPU::callHostFunction bad param 4: arg=%d argType=%d", arg4, arg4Type);
            }
            regDone[arg4] = true;
        }
    }
    if (argCount >= 3) {
        if (isParamTypeReg(arg3Type) && doneWithArg3) {
            if (arg3 >= 4) {
                kpanic_fmt("x32CPU::callHostFunction bad param 3: arg=%d argType=%d", arg3, arg3Type);
            }
            regDone[arg3] = true;
        }
    }
    if (argCount >= 2) {
        if (isParamTypeReg(arg2Type) && doneWithArg2) {
            if (arg2 >= 4) {
                kpanic_fmt("x32CPU::callHostFunction bad param 5: arg=%d argType=%d", arg2, arg2Type);
            }
            regDone[arg2] = true;
        }
    }
    if (argCount >= 1) {
        if (isParamTypeReg(arg1Type) && doneWithArg1) {
            if (arg1 >= 4) {
                kpanic_fmt("x32CPU::callHostFunction bad param 5: arg=%d argType=%d", arg1, arg1Type);
            }
            regDone[arg1] = true;
        }
    }

    if (hasReturn) {
        regUsed[x86.eax.reg] = true;
    } else if (regUsed[x86.eax.reg] && !hasReturn && !regDone[x86.eax.reg]) {
        x86.push(x86.eax);
    }
    if (regUsed[x86.ecx.reg] && !regDone[x86.ecx.reg]) {
        x86.push(x86.ecx);
    }
    if (regUsed[x86.edx.reg] && !regDone[x86.edx.reg]) {
        x86.push(x86.edx);
    }
    if (argCount >= 5) {
        pushValue(arg5, arg5Type);
    }
    if (argCount >= 4) {
        pushValue(arg4, arg4Type);
    }
    if (argCount >= 3) {
        pushValue(arg3, arg3Type);
    }
    if (argCount >= 2) {
        pushValue(arg2, arg2Type);
    }
    if (argCount >= 1) {
        pushValue(arg1, arg1Type);
    }

    x86.call(address);

    if (argCount) {
        x86.add(x86.esp, 4 * argCount);
    }
    if (regUsed[x86.edx.reg] && !regDone[x86.edx.reg]) {
        x86.pop(x86.edx);
    }
    if (regUsed[x86.ecx.reg] && !regDone[x86.ecx.reg]) {
        x86.pop(x86.ecx);
    }
    if (!hasReturn && regUsed[x86.eax.reg] && !regDone[x86.eax.reg]) {
        x86.pop(x86.eax);
    }
    for (int i = 0; i < 4; i++) {
        if (regDone[i]) {
            regUsed[i] = false;
        }
    }
}

// inst can be +, |, -, &, ^, <, >, ) right parens is for signed right shift
void X86DynamicData::instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    switch (inst) {
    case '+':
        if (regWidth == DYN_32bit) {
            x86.add(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.add(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.add(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg ADD");
        }
        break;
    case '-':
        if (regWidth == DYN_32bit) {
            x86.sub(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.sub(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.sub(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg SUB");
        }
        break;
    case '&':
        if (regWidth == DYN_32bit) {
            x86.and_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.and_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.and_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg AND");
        }
        break;
    case '|':
        if (regWidth == DYN_32bit) {
            x86.or_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.or_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.or_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg OR");
        }
        break;
    case '^':
        if (regWidth == DYN_32bit) {
            x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.xor_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.xor_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg XOR");
        }
        break;
    case '<':
        if (regWidth == DYN_32bit) {
            x86.shl(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.shl(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.shl(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg SHL");
        }
        break;
    case '>':
        if (regWidth == DYN_32bit) {
            x86.shr(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.shr(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.shr(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg SHR");
        }
        break;
    case ')':
        if (regWidth == DYN_32bit) {
            x86.sar(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.sar(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.sar(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg SAR");
        }
        break;
    default:
        kpanic("instRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicData::negReg(DynReg reg, DynWidth regWidth) {
    if (regWidth == DYN_32bit) {
        x86.neg(X86Asm::Reg32(reg));
    } else if (regWidth == DYN_16bit) {
        x86.neg(X86Asm::Reg16(reg));
    } else if (regWidth == DYN_8bit) {
        x86.neg(X86Asm::Reg8(reg));
    } else {
        kpanic("X86DynamicData::negReg");
    }
}

void X86DynamicData::notReg(DynReg reg, DynWidth regWidth) {
    if (regWidth == DYN_32bit) {
        x86.not_(X86Asm::Reg32(reg));
    } else if (regWidth == DYN_16bit) {
        x86.not_(X86Asm::Reg16(reg));
    } else if (regWidth == DYN_8bit) {
        x86.not_(X86Asm::Reg8(reg));
    } else {
        kpanic("X86DynamicData::notReg");
    }
}

// inst can be +, |, - , &, ^, <, >, ) right parens is for signed right shift
void X86DynamicData::instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm) {
    switch (inst) {
    case '+':
        if (regWidth == DYN_32bit) {
            x86.add(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.add(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.add(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm ADD");
        }
        break;
    case '-':
        if (regWidth == DYN_32bit) {
            x86.sub(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.sub(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.sub(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm SUB");
        }
        break;
    case '&':
        if (regWidth == DYN_32bit) {
            x86.and_(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.and_(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.and_(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm AND");
        }
        break;
    case '|':
        if (regWidth == DYN_32bit) {
            x86.or_(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.or_(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.or_(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm OR");
        }
        break;
    case '^':
        if (regWidth == DYN_32bit) {
            x86.xor_(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.xor_(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.xor_(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm XOR");
        }
        break;
    case '<':
        if (regWidth == DYN_32bit) {
            x86.shl(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.shl(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.shl(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm SHL");
        }
        break;
    case '>':
        if (regWidth == DYN_32bit) {
            x86.shr(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.shr(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.shr(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm SHR");
        }
        break;
    case ')':
        if (regWidth == DYN_32bit) {
            x86.sar(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.sar(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.sar(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm SAR");
        }
        break;
    default:
        kpanic("instRegImm");
    }
}

// :TODO: move to base
void X86DynamicData::calculateEaa(DecodedOp* op, DynReg reg) {
    regUsed[reg] = true;

    if (op->ea16) {
        // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)
        X86Asm::Reg16 reg16(reg);

        x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(reg));

        if (op->data.disp) {
            x86.add(reg16, (U16)op->data.disp);
        }
        if (op->rm != 8) {
            // intentional 16-bit add
            x86.addMem(reg16, x86.edi, CPU::offsetofReg16(op->rm));
        }

        if (op->sibIndex != 8) {
            // intentional 16-bit add
            x86.addMem(reg16, x86.edi, CPU::offsetofReg16(op->sibIndex));
        }

        // seg[6] is always 0
        if (op->base < 6) {
            // intentional 32-bit add
            x86.addMem(X86Asm::Reg32(reg), x86.edi, CPU::offsetofSegAddress(op->base));
        }
    } else {
        if (op->sibIndex != 8) {
            x86.readMem(X86Asm::Reg32(reg), x86.edi, CPU::offsetofReg32(op->sibIndex));
            if (op->sibScale) {
                x86.shl(X86Asm::Reg32(reg), op->sibScale);
            }
            if (op->rm != 8) {
                x86.addMem(X86Asm::Reg32(reg), x86.edi, CPU::offsetofReg32(op->rm));
            }
            if (op->data.disp) {
                x86.add(X86Asm::Reg32(reg), op->data.disp);
            }
        } else if (op->rm != 8) {
            x86.readMem(X86Asm::Reg32(reg), x86.edi, CPU::offsetofReg32(op->rm));
            if (op->data.disp) {
                x86.add(X86Asm::Reg32(reg), op->data.disp);
            }
        } else if (op->data.disp) {
            x86.mov(X86Asm::Reg32(reg), op->data.disp);
        } else {
            x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
        }

        // seg[6] is always 0
        if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
            x86.addMem(X86Asm::Reg32(reg), x86.edi, CPU::offsetofSegAddress(op->base));
        }
    }
}

void X86DynamicData::byteSwapReg32(DynReg reg) {
    x86.bswap(reg);
}

void X86DynamicData::movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    regUsed[dst] = true;
    if (dstWidth <= srcWidth) {
        movToRegFromReg(dst, dstWidth, src, srcWidth, doneWithSrcReg);
    } else {
        if (dstWidth == DYN_32bit) {
            if (srcWidth == DYN_16bit) {
                x86.movsx(X86Asm::Reg32(dst), X86Asm::Reg16(src));
            } else if (srcWidth == DYN_8bit) {
                x86.movsx(X86Asm::Reg32(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth == DYN_16bit) {
            if (srcWidth == DYN_8bit) {
                x86.movsx(X86Asm::Reg16(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
            }
        } else {
            kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
        }
        if (doneWithSrcReg) {
            regUsed[src] = false;
        }
    }
}

void X86DynamicData::movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    regUsed[dst] = true;
    if (dstWidth <= srcWidth) {
        if (dst == src) // downsizing doesn't need anything
            return;
        if (dstWidth == DYN_32bit) {
            x86.mov(X86Asm::Reg32(dst), X86Asm::Reg32(src));
        } else if (dstWidth == DYN_16bit) {
            x86.mov(X86Asm::Reg16(dst), X86Asm::Reg16(src));
        } else if (dstWidth == DYN_8bit) {
            x86.mov(X86Asm::Reg8(dst), X86Asm::Reg8(src));
        } else {
            kpanic_fmt("unknown dstWidth in x32CPU::movToRegFromReg %d", dstWidth);
        }
    } else {
        if (dstWidth == DYN_32bit) {
            if (srcWidth == DYN_16bit) {
                x86.movzx(X86Asm::Reg32(dst), X86Asm::Reg16(src));
            } else if (srcWidth == DYN_8bit) {
                x86.movzx(X86Asm::Reg32(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth == DYN_16bit) {
            if (srcWidth == DYN_8bit) {
                x86.movzx(X86Asm::Reg16(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }
        } else {
            kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
        }
    }
    if (doneWithSrcReg) {
        regUsed[src] = false;
    }
}

void X86DynamicData::movToReg(DynReg reg, DynWidth width, U32 imm) {
    regUsed[reg] = true;
    if (width == DYN_32bit) {
        x86.mov(X86Asm::Reg32(reg), imm);
    } else if (width == DYN_16bit) {
        x86.mov(X86Asm::Reg16(reg), (U16)imm);
    } else if (width == DYN_8bit) {
        x86.mov(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic_fmt("unknown width in x32CPU::movToReg %d", width);
    }
}

void X86DynamicData::pushValue(U32 arg, DynCallParamType argType) {
    switch (argType) {
    case DYN_PARAM_REG_8:
        x86.movzx(X86Asm::Reg32(arg), X86Asm::Reg8(arg));
        x86.push(X86Asm::Reg32(arg));
        break;
    case DYN_PARAM_REG_16:
        x86.movzx(X86Asm::Reg32(arg), X86Asm::Reg16(arg));
        x86.push(X86Asm::Reg32(arg));
        break;
    case DYN_PARAM_REG_32:
        x86.push(X86Asm::Reg32(arg));
        break;
    case DYN_PARAM_CPU:
        x86.push(x86.edi);
        break;
    case DYN_PARAM_CONST_8:
        x86.push((U32)(arg & 0xFF));
        break;
    case DYN_PARAM_CONST_16:
        x86.push((U32)(arg & 0xFFFF));
        break;
    case DYN_PARAM_CONST_32:
        x86.push(arg);
        break;
    case DYN_PARAM_CONST_PTR:
        x86.push((U32)arg);
        break;
    case DYN_PARAM_CPU_ADDRESS_8:
        x86.readMem(X86Asm::Reg8(x86.eax.reg), x86.edi, arg);
        x86.movzx(x86.eax, X86Asm::Reg8(x86.eax.reg));
        x86.push(x86.eax);
        break;
    case DYN_PARAM_CPU_ADDRESS_16:
        x86.readMem(X86Asm::Reg16(x86.eax.reg), x86.edi, arg);
        x86.movzx(x86.eax, X86Asm::Reg16(x86.eax.reg));
        x86.push(x86.eax);
        break;
    case DYN_PARAM_CPU_ADDRESS_32:
        x86.readMem(x86.eax, x86.edi, arg);
        x86.push(x86.eax);
        break;
    default:
        kpanic_fmt("x32CPU: unknown argType: %d", argType);
        break;
    }
}

void X86DynamicData::zeroExtendReg16To32(DynReg dest, DynReg src) {
    x86.movzx(X86Asm::Reg32(dest), X86Asm::Reg16(src));
    regUsed[dest] = true;
}

void X86DynamicData::movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width) {
    this->regUsed[reg] = true;
    // mov reg, [edi+srcOffset]    
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(reg), x86.edi, srcOffset);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(reg), x86.edi, srcOffset);
    } else if (width == DYN_8bit) {
        x86.readMem(X86Asm::Reg8(reg), x86.edi, srcOffset);
    } else {
        kpanic_fmt("unknown dstWidth in x32CPU::movToRegFromCpu %d", width);
    }
}

void X86DynamicData::movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) {
    if (width == DYN_32bit) {
        x86.writeMem(x86.edi, dstOffset, X86Asm::Reg32(reg));
    } else if (width == DYN_16bit) {
        x86.writeMem(x86.edi, dstOffset, X86Asm::Reg16(reg));
    } else if (width == DYN_8bit) {
        x86.writeMem(x86.edi, dstOffset, X86Asm::Reg8(reg));
    } else {
        kpanic_fmt("unknown dstWidth in x32CPU::movToCpuFromReg %d", width);
    }
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicData::movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm) {
    if (dstWidth == DYN_32bit) {
        x86.writeMem(x86.edi, dstOffset, imm);
    } else if (dstWidth == DYN_16bit) {
        x86.writeMem(x86.edi, dstOffset, (U16)imm);
    } else if (dstWidth == DYN_8bit) {
        x86.writeMem(x86.edi, dstOffset, (U8)imm);
    } else {
        kpanic_fmt("unknown dstWidth in DynamicData::movToCpu %d", dstWidth);
    }
}

void X86DynamicData::JumpIf(DynReg reg, bool doneWithReg, U32 address) {
    x86.test(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
    x86.jnz(address);
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicData::JumpIfNot(DynReg reg, bool doneWithReg, U32 address) {
    x86.test(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
    x86.jz(address);
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicData::JumpInBlock(U32 address) {
    x86.jmp(address);
}

void X86DynamicData::IfPtrEqual(DynReg reg, DYN_PTR_SIZE value, bool doneWithReg) {
    IfEqual(X86Asm::Reg32(reg), value, doneWithReg);
}

void X86DynamicData::IfLessThan(DynReg reg, U32 value, bool doneWithReg) {
    x86.IfLessThan(X86Asm::Reg32(reg), value);

    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicData::IfEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    x86.IfEqual(reg, value);
    if (doneWithReg) {
        regUsed[reg.reg] = false;
    }
}

void X86DynamicData::IfNotEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    x86.IfNotEqual(reg, value);
    if (doneWithReg) {
        regUsed[reg.reg] = false;
    }
}


void X86DynamicData::IfBitSet(DynReg reg, U32 value, bool doneWithReg) {
    x86.IfBitSet(X86Asm::Reg32(reg), value);
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicData::If(DynReg reg, bool doneWithReg) {
    x86.IfNotZero(X86Asm::Reg32(reg));
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicData::IfNot(DynReg reg, bool doneWithReg) {
    x86.IfZero(X86Asm::Reg32(reg));
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicData::StartElse() {
    x86.Else();
}

void X86DynamicData::EndIf() {
    x86.EndIf();
}

U8* X86DynamicData::createStartJITCode() {
#ifdef _DEBUG
    x86.push(x86.ebp);
    x86.mov(x86.ebp, x86.esp);
#endif

    x86.push(x86.ebx);
    x86.push(x86.edi);
    // on win32 ecx contains cpu
    x86.mov(x86.edi, x86.ecx);

    // :TODO: what about other x86 platforms that use a different calling convention
    // 
    // jmp ((DecodedOp*)edx)->pfn
    x86.readMem(x86.eax, x86.edx, offsetof(DecodedOp, pfnJitCode));
    x86.jmp(x86.eax);
    return createDynamicExecutableMemory();
}

void X86DynamicData::preCommitJIT() {
    for (DynamicJump& jmp : x86.jumps) {
        U32 bufferIndex = 0;

        if (!eipToBufferPos.get(jmp.eip, bufferIndex)) {
            return;
        }
        *(U32*)&x86.buffer.data()[jmp.bufferPos] = bufferIndex - jmp.bufferPos - 4;
    }
}

void X86DynamicData::patch(U8* begin) {
    for (U32 i = 0; i < x86.patch.size(); i++) {
        U32 pos = x86.patch[i];
        U32* value = (U32*)(&begin[pos]);
        *value = *value - (U32)(begin + pos + 4);
    }
}

void startNewJIT(CPU* cpu, U32 address, DecodedOp* op) {
    X86DynamicData data(cpu);
    data.doJIT(address, op);
}

#endif