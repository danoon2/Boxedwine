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

// -: 31.1, 31.0, 31.2 (esi saved)
// -: 31.1, 31.2, 31.2 (esi not saved)
// with the 2 scores above, it looks like pushing/popping esi everytime going in and out of the JIT code does not have much overhead
// 
// Quake scores with different regs cached, seems like cachiing eax has the most impact
// 0: 32.4, 32.4, 32.6
// 1: 31.2, 31.1, 31.3
// 2: 31.5, 31.4, 31.4
// 3: 31.2, 31.0, 31.1
// 4: 31.4, 31.2, 31.3
// 5: 31.2, 31.4, 31.2
// 6: 31.5, 31.6, 31.6
// 7: 30.9, 31.0, 31.1
// 0+6: 32.2, 32.3, 32.1
static U8 regCache[] = { 5, 0, 0, 0, 0, 0, 6, 0 };

class X86DynamicCodeGen : DynamicCodeGen {
public:
    X86DynamicCodeGen(CPU* cpu) : DynamicCodeGen(cpu) {}

    void incrementEip(U32 inc) override;
    void setConditional(DynConditional condition) override;
    void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) override;
    void addRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void orRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void subRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void andRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void xorRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void shrRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void sarRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void shlRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void addRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void orRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void subRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void andRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void xorRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void shrRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void sarRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void shlRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;

    void negReg(DynReg reg, DynWidth regWidth) override;
    void notReg(DynReg reg, DynWidth regWidth) override;
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

    void loadReg(U8 reg, DynReg tmpReg, DynWidth width) override;
    void storeReg(U8 reg, DynReg srcReg, DynWidth width, bool doneWithSrcReg) override;
    void storeReg(U8 reg, DynWidth dstWidth, U32 imm) override;
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

    void loadCache();
    void writeCache();

    X86Asm x86;
};

void X86DynamicCodeGen::jmp(DynReg reg) {
    x86.jmp(X86Asm::Reg32(reg));
}

void X86DynamicCodeGen::readMem(DynReg reg, DynWidth width, DynReg address, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(reg), X86Asm::Reg32(address), lsl, disp);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(reg), X86Asm::Reg32(address), lsl, disp);
    } else {
        x86.readMem(X86Asm::Reg8(reg), X86Asm::Reg32(address), lsl, disp);
    }
}

void X86DynamicCodeGen::readMem(DynReg reg, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(reg), X86Asm::Reg32(address), offset, lsl, disp);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(reg), X86Asm::Reg32(address), offset, lsl, disp);
    } else {
        x86.readMem(X86Asm::Reg8(reg), X86Asm::Reg32(address), offset, lsl, disp);
    }
}

void X86DynamicCodeGen::writeMem(DynReg reg, DynWidth width, DynReg address, U32 disp) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, X86Asm::Reg32(reg));
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, X86Asm::Reg16(reg));
    } else {
        x86.writeMem(X86Asm::Reg32(address), disp, X86Asm::Reg8(reg));
    }
}

void X86DynamicCodeGen::writeMem(U32 value, DynWidth width, DynReg address, U32 disp) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, value);
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, (U16)value);
    } else {
        x86.writeMem(X86Asm::Reg32(address), disp, (U8)value);
    }
}

void X86DynamicCodeGen::writeMem(DynReg reg, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, X86Asm::Reg32(reg));
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, X86Asm::Reg16(reg));
    } else {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, X86Asm::Reg8(reg));
    }
}

void X86DynamicCodeGen::writeMem(U32 value, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, value);
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, (U16)value);
    } else {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, (U8)value);
    }
}

void X86DynamicCodeGen::and32(DynReg dst, U32 imm) {
    x86.and_(X86Asm::Reg32(dst), imm);
}

void X86DynamicCodeGen::shr32(DynReg dst, U32 imm) {
    x86.shr(X86Asm::Reg32(dst), imm);
}

U32 X86DynamicCodeGen::getBufferSize() {
    return (U32)x86.buffer.size();
}

U8* X86DynamicCodeGen::getBuffer() {
    return x86.buffer.data();
}


U32 X86DynamicCodeGen::getIfJumpSize() {
    return (U32)x86.ifJump.size();
}

void writeBlockExitForJIT(U8* buffer) {
    X86Asm x86;

    // writeCache
    for (int i = 0; i < 8; i++) {
        if (regCache[i]) {
            x86.writeMem(x86.edi, CPU::offsetofReg32(i), X86Asm::Reg32(regCache[i]));
        }
    }

    x86.pop(x86.ebp);
    x86.pop(x86.esi);
    x86.pop(x86.edi);
    x86.pop(x86.ebx);

#ifdef _DEBUG
    x86.mov(x86.esp, x86.ebp);
    x86.pop(x86.ebp);
#endif

    x86.ret();
    memcpy(buffer, x86.buffer.data(), x86.buffer.size());
}

void X86DynamicCodeGen::blockExit() {

    writeCache();

    x86.pop(x86.ebp);
    x86.pop(x86.esi);
    x86.pop(x86.edi);
    x86.pop(x86.ebx);

#ifdef _DEBUG
    x86.mov(x86.esp, x86.ebp);
    x86.pop(x86.ebp);
#endif

    x86.ret();
}

void X86DynamicCodeGen::incrementEip(U32 inc) {
    x86.addMem(x86.edi, offsetof(CPU, eip.u32), inc);
}

void setConditionInReg(DynamicData* data, DynConditional condition, DynReg reg);

void X86DynamicCodeGen::setConditional(DynConditional condition) {
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

void X86DynamicCodeGen::setCC(X86Asm::Reg32 reg, DynConditionEvaluate condition) {

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

void X86DynamicCodeGen::evaluateToReg(X86Asm::Reg32 reg, X86Asm::Reg32 left, X86Asm::Reg32 right, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
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

void X86DynamicCodeGen::evaluateToReg(X86Asm::Reg32 reg, X86Asm::Reg32 left, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg) {
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

void X86DynamicCodeGen::evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    if (reg >= 4) {
        kpanic_fmt("x32CPU::evaluateToRegFromRegs doesn't support reg %d", reg);
    }
    if (isRightConst) {
        evaluateToReg(X86Asm::Reg32(reg), X86Asm::Reg32(left), rightConst, regWidth, condition, doneWithLeftReg);
    } else {
        evaluateToReg(X86Asm::Reg32(reg), X86Asm::Reg32(left), X86Asm::Reg32(right), regWidth, condition, doneWithLeftReg, doneWithRightReg);
    }
}

void X86DynamicCodeGen::callHostFunction(void* address, bool hasReturn, U32 argCount, U32 arg1, DynCallParamType arg1Type, bool doneWithArg1, U32 arg2, DynCallParamType arg2Type, bool doneWithArg2, U32 arg3, DynCallParamType arg3Type, bool doneWithArg3, U32 arg4, DynCallParamType arg4Type, bool doneWithArg4, U32 arg5, DynCallParamType arg5Type, bool doneWithArg5) {
    bool regDone[4] = { false, false, false, false };

    if (argCount >= 5) {
        if (isParamTypeReg(arg5Type) && doneWithArg5) {
            if (arg5 < 4) {
                regDone[arg5] = true;
            }
        }
    }
    if (argCount >= 4) {
        if (isParamTypeReg(arg4Type) && doneWithArg4) {
            if (arg4 < 4) {
                regDone[arg4] = true;
            }
        }
    }
    if (argCount >= 3) {
        if (isParamTypeReg(arg3Type) && doneWithArg3) {
            if (arg3 < 4) {
                regDone[arg3] = true;
            }
        }
    }
    if (argCount >= 2) {
        if (isParamTypeReg(arg2Type) && doneWithArg2) {
            if (arg2 < 4) {
                regDone[arg2] = true;
            }
        }
    }
    if (argCount >= 1) {
        if (isParamTypeReg(arg1Type) && doneWithArg1) {
            if (arg1 < 4) {
                regDone[arg1] = true;
            }
        }
    }

    writeCache();

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
    loadCache();
    for (int i = 0; i < 4; i++) {
        if (regDone[i]) {
            regUsed[i] = false;
        }
    }
}

void X86DynamicCodeGen::addRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.add(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.add(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.add(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("instRegReg ADD");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::orRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.or_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.or_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.or_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("instRegReg OR");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::subRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.sub(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.sub(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.sub(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("instRegReg SUB");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::andRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.and_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.and_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.and_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("instRegReg AND");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::xorRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.xor_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.xor_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("instRegReg XOR");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::shrRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.shr(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.shr(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.shr(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("instRegReg SHR");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::sarRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.sar(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.sar(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.sar(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("instRegReg SAR");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::shlRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.shl(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.shl(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.shl(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("instRegReg SHL");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::negReg(DynReg reg, DynWidth regWidth) {
    if (regWidth == DYN_32bit) {
        x86.neg(X86Asm::Reg32(reg));
    } else if (regWidth == DYN_16bit) {
        x86.neg(X86Asm::Reg16(reg));
    } else if (regWidth == DYN_8bit) {
        x86.neg(X86Asm::Reg8(reg));
    } else {
        kpanic("X86DynamicCodeGen::negReg");
    }
}

void X86DynamicCodeGen::notReg(DynReg reg, DynWidth regWidth) {
    if (regWidth == DYN_32bit) {
        x86.not_(X86Asm::Reg32(reg));
    } else if (regWidth == DYN_16bit) {
        x86.not_(X86Asm::Reg16(reg));
    } else if (regWidth == DYN_8bit) {
        x86.not_(X86Asm::Reg8(reg));
    } else {
        kpanic("X86DynamicCodeGen::notReg");
    }
}

void X86DynamicCodeGen::addRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.add(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.add(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.add(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("instRegImm ADD");
    }
}

void X86DynamicCodeGen::orRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.or_(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.or_(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.or_(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("instRegImm OR");
    }
}

void X86DynamicCodeGen::subRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.sub(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.sub(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.sub(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("instRegImm SUB");
    }
}

void X86DynamicCodeGen::andRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.and_(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.and_(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.and_(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("instRegImm AND");
    }
}

void X86DynamicCodeGen::xorRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.xor_(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.xor_(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.xor_(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("instRegImm XOR");
    }
}

void X86DynamicCodeGen::shrRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.shr(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.shr(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.shr(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("instRegImm SHR");
    }
}

void X86DynamicCodeGen::sarRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.sar(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.sar(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.sar(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("instRegImm SAR");
    }
}

void X86DynamicCodeGen::shlRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.shl(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.shl(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.shl(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("instRegImm SHL");
    }
}

// :TODO: move to base
void X86DynamicCodeGen::calculateEaa(DecodedOp* op, DynReg reg) {
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
            if (regCache[op->rm]) {
                x86.add(reg16, X86Asm::Reg16(regCache[op->rm]));
            } else {
                x86.addMem(reg16, x86.edi, CPU::offsetofReg16(op->rm));
            }
        }

        if (op->sibIndex != 8) {
            // intentional 16-bit add
            if (regCache[op->sibIndex]) {
                x86.add(reg16, X86Asm::Reg16(regCache[op->sibIndex]));
            } else {
                x86.addMem(reg16, x86.edi, CPU::offsetofReg16(op->sibIndex));
            }
        }

        // seg[6] is always 0
        if (op->base < 6) {
            // intentional 32-bit add
            x86.addMem(X86Asm::Reg32(reg), x86.edi, CPU::offsetofSegAddress(op->base));
        }
    } else {
        if (op->sibIndex != 8) {
            loadReg(op->sibIndex, reg, DYN_32bit);
            if (op->sibScale) {
                x86.shl(X86Asm::Reg32(reg), op->sibScale);
            }
            if (op->rm != 8) {
                if (regCache[op->rm]) {
                    x86.add(X86Asm::Reg32(reg), X86Asm::Reg32(regCache[op->rm]));
                } else {
                    x86.addMem(X86Asm::Reg32(reg), x86.edi, CPU::offsetofReg32(op->rm));
                }
            }
            if (op->data.disp) {
                x86.add(X86Asm::Reg32(reg), op->data.disp);
            }
        } else if (op->rm != 8) {
            loadReg(op->rm, reg, DYN_32bit);
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

void X86DynamicCodeGen::byteSwapReg32(DynReg reg) {
    x86.bswap(reg);
}

void X86DynamicCodeGen::movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
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

void X86DynamicCodeGen::movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
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

void X86DynamicCodeGen::movToReg(DynReg reg, DynWidth width, U32 imm) {
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

void X86DynamicCodeGen::pushValue(U32 arg, DynCallParamType argType) {
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
    case DYN_PARAM_CPU_REG_8:
    {
        loadReg(arg, DYN_CALL_RESULT, DYN_8bit);
        x86.movzx(x86.eax, X86Asm::Reg8(DYN_CALL_RESULT));
        x86.push(x86.eax);
        regUsed[DYN_CALL_RESULT] = false;
        break;
    }
    case DYN_PARAM_CPU_REG_16:
    {
        loadReg(arg, DYN_CALL_RESULT, DYN_16bit);
        x86.movzx(x86.eax, X86Asm::Reg16(DYN_CALL_RESULT));
        x86.push(x86.eax);
        regUsed[DYN_CALL_RESULT] = false;
        break;
    }
    case DYN_PARAM_CPU_REG_32:
    {
        loadReg(arg, DYN_CALL_RESULT, DYN_32bit);
        x86.push(x86.eax);
        regUsed[DYN_CALL_RESULT] = false;
        break;
    }
    default:
        kpanic_fmt("x32CPU: unknown argType: %d", argType);
        break;
    }
}

void X86DynamicCodeGen::zeroExtendReg16To32(DynReg dest, DynReg src) {
    x86.movzx(X86Asm::Reg32(dest), X86Asm::Reg16(src));
    regUsed[dest] = true;
}

void X86DynamicCodeGen::movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width) {
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

void X86DynamicCodeGen::movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) {
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

void X86DynamicCodeGen::movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm) {
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

void X86DynamicCodeGen::JumpIf(DynReg reg, bool doneWithReg, U32 address) {
    x86.test(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
    x86.jnz(address);
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::JumpIfNot(DynReg reg, bool doneWithReg, U32 address) {
    x86.test(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
    x86.jz(address);
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::JumpInBlock(U32 address) {
    x86.jmp(address);
}

void X86DynamicCodeGen::IfPtrEqual(DynReg reg, DYN_PTR_SIZE value, bool doneWithReg) {
    IfEqual(X86Asm::Reg32(reg), value, doneWithReg);
}

void X86DynamicCodeGen::IfLessThan(DynReg reg, U32 value, bool doneWithReg) {
    x86.IfLessThan(X86Asm::Reg32(reg), value);

    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::IfEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    x86.IfEqual(reg, value);
    if (doneWithReg) {
        regUsed[reg.reg] = false;
    }
}

void X86DynamicCodeGen::IfNotEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    x86.IfNotEqual(reg, value);
    if (doneWithReg) {
        regUsed[reg.reg] = false;
    }
}


void X86DynamicCodeGen::IfBitSet(DynReg reg, U32 value, bool doneWithReg) {
    x86.IfBitSet(X86Asm::Reg32(reg), value);
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::If(DynReg reg, bool doneWithReg) {
    x86.IfNotZero(X86Asm::Reg32(reg));
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::IfNot(DynReg reg, bool doneWithReg) {
    x86.IfZero(X86Asm::Reg32(reg));
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::StartElse() {
    x86.Else();
}

void X86DynamicCodeGen::EndIf() {
    x86.EndIf();
}

U8* X86DynamicCodeGen::createStartJITCode() {
#ifdef _DEBUG
    x86.push(x86.ebp);
    x86.mov(x86.ebp, x86.esp);
#endif

    x86.push(x86.ebx);
    x86.push(x86.edi);
    x86.push(x86.esi);
    x86.push(x86.ebp);
    // on win32 ecx contains cpu
    x86.mov(x86.edi, x86.ecx);

    loadCache();

    // :TODO: what about other x86 platforms that use a different calling convention
    // 
    // jmp ((DecodedOp*)edx)->pfn
    x86.readMem(x86.eax, x86.edx, offsetof(DecodedOp, pfnJitCode));
    x86.jmp(x86.eax);
    return createDynamicExecutableMemory();
}

void X86DynamicCodeGen::preCommitJIT() {
    for (DynamicJump& jmp : x86.jumps) {
        U32 bufferIndex = 0;

        if (!eipToBufferPos.get(jmp.eip, bufferIndex)) {
            return;
        }
        *(U32*)&x86.buffer.data()[jmp.bufferPos] = bufferIndex - jmp.bufferPos - 4;
    }
}

void X86DynamicCodeGen::patch(U8* begin) {
    for (U32 i = 0; i < x86.patch.size(); i++) {
        U32 pos = x86.patch[i];
        U32* value = (U32*)(&begin[pos]);
        *value = *value - (U32)(begin + pos + 4);
    }
}

// :TODO: it would be nice to return the mapped register rather than always copying to tmpReg, but currently the dynamic code assumes in many places that it can clobber the register, like in callHostFunction when it zero extends it
void X86DynamicCodeGen::loadReg(U8 reg, DynReg tmpReg, DynWidth width) {
    bool isHigh8bit = (width == DYN_8bit) && (reg >= 4);

    if (!isHigh8bit && regCache[reg]) {
        // tmpReg can only be 0-3 (see enum DynReg), on x86 those reg can do 8-bit math, the other can't, so for 8-bit, we need to move it into the tmpReg
        if (width == DYN_8bit) {
            x86.mov(X86Asm::Reg32(tmpReg), X86Asm::Reg32(regCache[reg]));
            regUsed[tmpReg] = true;
        } else {
            movToRegFromReg(tmpReg, width, (DynReg)regCache[reg], width, false);
            regUsed[tmpReg] = true;
        }
    } else if (isHigh8bit && regCache[reg - 4]) {
        x86.mov(X86Asm::Reg32(tmpReg), X86Asm::Reg32(regCache[reg - 4]));
        x86.mov(X86Asm::Reg8(tmpReg), X86Asm::Reg8(tmpReg + 4)); // copy AH to AL since this is where the caller expects it
        regUsed[tmpReg] = true;
    } else {
        DynamicCodeGen::loadReg(reg, tmpReg, width);
    }
}

void X86DynamicCodeGen::storeReg(U8 reg, DynReg srcReg, DynWidth width, bool doneWithSrcReg) {
    if (doneWithSrcReg) {
        regUsed[srcReg] = false;
    }
    bool isHigh8bit = (width == DYN_8bit) && (reg >= 4);

    if (!isHigh8bit && regCache[reg]) {
        if (width == DYN_8bit) {
            X86Asm::Reg32 tmpReg(0);

            if (srcReg == 0) {
                tmpReg = X86Asm::Reg32(1);
            }
            if (regUsed[tmpReg.reg]) {
                x86.push(tmpReg);
            }
            x86.mov(tmpReg, X86Asm::Reg32(regCache[reg]));
            x86.mov(X86Asm::Reg8(tmpReg.reg), X86Asm::Reg8(srcReg));
            x86.mov(X86Asm::Reg32(regCache[reg]), tmpReg);
            if (regUsed[tmpReg.reg]) {
                x86.pop(tmpReg);
            }
        } else if (width == DYN_16bit) {
            x86.mov(X86Asm::Reg16(regCache[reg]), X86Asm::Reg16(srcReg));
        } else {
            x86.mov(X86Asm::Reg32(regCache[reg]), X86Asm::Reg32(srcReg));
        }
    } else if (isHigh8bit && regCache[reg - 4]) {
        X86Asm::Reg32 tmpReg(0);

        if (srcReg == 0) {
            tmpReg = X86Asm::Reg32(1);
        }
        if (regUsed[tmpReg.reg]) {
            x86.push(tmpReg);
        }
        x86.mov(tmpReg, X86Asm::Reg32(regCache[reg - 4]));
        x86.mov(X86Asm::Reg8(tmpReg.reg + 4), X86Asm::Reg8(srcReg)); // +4 for high byte (like AH)
        x86.mov(X86Asm::Reg32(regCache[reg - 4]), tmpReg);
        if (regUsed[tmpReg.reg]) {
            x86.pop(tmpReg);
        }
    } 
    else {
        DynamicCodeGen::storeReg(reg, srcReg, width, doneWithSrcReg);
    }
}

void X86DynamicCodeGen::storeReg(U8 reg, DynWidth dstWidth, U32 imm) {
    bool isHigh8bit = (dstWidth == DYN_8bit) && (reg >= 4);

    if (!isHigh8bit && regCache[reg]) {
        if (dstWidth == DYN_8bit) {
            if (regUsed[0]) {
                x86.push(x86.eax);
            }
            x86.mov(x86.eax, X86Asm::Reg32(regCache[reg]));
            x86.mov(x86.al, (U8)imm);
            x86.mov(X86Asm::Reg32(regCache[reg]), x86.eax);
            if (regUsed[0]) {
                x86.pop(x86.eax);
            }
        } else if (dstWidth == DYN_16bit) {
            x86.mov(X86Asm::Reg16(regCache[reg]), (U16)imm);
        } else {
            x86.mov(X86Asm::Reg32(regCache[reg]), imm);
        }
    } else if (isHigh8bit && regCache[reg - 4]) {
        if (regUsed[0]) {
            x86.push(x86.eax);
        }
        x86.mov(x86.eax, X86Asm::Reg32(regCache[reg - 4]));
        x86.mov(x86.ah, (U8)imm);
        x86.mov(X86Asm::Reg32(regCache[reg - 4]), x86.eax);
        if (regUsed[0]) {
            x86.pop(x86.eax);
        }
    } 
    else {
        DynamicCodeGen::storeReg(reg, dstWidth, imm);
    }
}

void X86DynamicCodeGen::loadCache() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i]) {
            DynamicCodeGen::loadReg(i, (DynReg)regCache[i], DYN_32bit);
        }
    }
}

void X86DynamicCodeGen::writeCache() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i]) {
            DynamicCodeGen::storeReg(i, (DynReg)regCache[i], DYN_32bit, false);
        }
    }    
}

void startNewJIT(CPU* cpu, U32 address, DecodedOp* op) {
    X86DynamicCodeGen data(cpu);
    data.doJIT(address, op);
}

#endif