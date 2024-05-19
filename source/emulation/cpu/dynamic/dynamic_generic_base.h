#ifndef __DYNAMIC_GENERIC_BASE_H__
#define __DYNAMIC_GENERIC_BASE_H__

#include "dynamic_memory.h"

// Functions that need to be implemented in the platform/chip specific file
/*
void clearTop16(U8 reg);
void loadConst32(U8 reg, U32 value);
void readMem8(U8 dst, U8 src, U32 offset);
void readMem16(U8 dst, U8 src, U32 offset);
void readMem32(U8 dst, U8 src, U32 offset);
void loadFromCpuOffset8(U8 reg, U32 offset);
void loadFromCpuOffset16(U8 reg, U32 offset);
void loadFromCpuOffset32(U8 reg, U32 offset);
void saveRegToCpuOffset8(U8 reg, U32 offset);
void saveRegToCpuOffset16(U8 reg, U32 offset);
void saveRegToCpuOffset32(U8 reg, U32 offset);
void saveValueToCpuOffset8(U8 value, U32 offset);
void saveValueToCpuOffset16(U16 value, U32 offset);
void saveValueToCpuOffset32(U32 value, U32 offset);
void addRegs32(U8 reg, U8 reg2);
void addValue32(U8 reg, U32 value);
void subRegs32(U8 reg, U8 reg2);
void subValue32(U8 reg, U32 value);
void orRegs32(U8 reg, U8 reg2);
void orValue32(U8 reg, U32 value);
void xorRegs32(U8 reg, U8 reg2);
void xorValue32(U8 reg, U32 value);
void andRegs32(U8 reg, U8 reg2);
void andValue32(U8 reg, U32 value);
void negReg32(U8 reg);
void notReg32(U8 reg);
void zeroReg32(U8 reg);
void shiftLeft32(U8 reg, U8 amount);
void shiftLeft32WithReg(U8 reg, U8 amount);
void shiftRight32(U8 reg, U8 amount);
void shiftRight32WithReg(U8 reg, U8 amount);
void shiftRightSigned32(U8 reg, U8 amount);
void shiftRightSigned32WithReg(U8 reg, U8 amount);
void mov32(U8 dst, U8 src);
void mov32zx16(U8 dst, U8 src);
void mov32zx8(U8 dst, U8 src);
void mov32sx16(U8 dst, U8 src);
void mov32sx8(U8 dst, U8 src);
void cmpRegs32(U8 r1, U8 r2);
void cmpRegValue32(U8 reg, U32 value);
U32 jumpIfEqual();
U32 jumpIfNotEqual();
U32 unconditionalJump();
void writeJumpAmount(U32 pos, U32 toLocation);
void evaluateCondition(U8 reg, DynConditionEvaluate condition);
void callHostFunction(void* address, bool hasReturn, U32 argCount, U32 arg1, DynCallParamType arg1Type, bool doneWithArg1, U32 arg2, DynCallParamType arg2Type, bool doneWithArg2, U32 arg3, DynCallParamType arg3Type, bool doneWithArg3, U32 arg4, DynCallParamType arg4Type, bool doneWithArg4, U32 arg5, DynCallParamType arg5Type, bool doneWithArg5);
void startBlock();
void endBlock();
void codeCreate(U8* start, U8* end);

#ifdef DYN_NEEDS_PUSH_POP_REG_FOR_FUNCTION_PARAMS
void pushRegs(U16 bitMask);
void popRegs(U16 bitMask);
#endif
*/

bool isRegSetFFF;
bool isRegSetFFD;

void initDyn() {
    isRegSetFFF = false;
    isRegSetFFD = false;
}

void calculateEaa(DecodedOp* op, DynReg reg) {
    setRegUsed(reg);
    if (op->ea16) {
        // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)

        zeroReg32(reg);

        if (op->disp) {
            addValue32(reg, op->disp);
        }
        // add ax, [DYN_CPU_REG+cpu->reg[op->rm].u16]
        if (op->rm != 8) {
            DynReg tmp = getUnsavedTmpReg();
            loadFromCpuOffset16(tmp, CPU::offsetofReg16(op->rm));
            addRegs32(reg, tmp);
            clearRegUsed(tmp);
        }

        // add ax, [cpu->reg[op->sibIndex].u16]
        if (op->sibIndex != 8) {
            DynReg tmp = getUnsavedTmpReg();
            loadFromCpuOffset16(tmp, CPU::offsetofReg16(op->sibIndex));
            addRegs32(reg, tmp);
            clearRegUsed(tmp);
        }

        // don't allow the above adds to roll over past 16-bits
        clearTop16(reg);

        // seg[6] is always 0
        if (op->base < 6) {
            // add eax, [cpu->seg[op->base].address]
            DynReg tmp = getUnsavedTmpReg();
            loadFromCpuOffset32(tmp, CPU::offsetofSegAddress(op->base));
            addRegs32(reg, tmp);
            clearRegUsed(tmp);
        }
    } else {
        // cpu->seg[op->base].address + cpu->reg[op->rm].u32 + (cpu->reg[op->sibIndex].u32 << + op->sibScale) + op->disp
        bool initiallized = false;

        if (op->sibIndex != 8) {
            initiallized = true;
            loadFromCpuOffset32(reg, CPU::offsetofReg32(op->sibIndex));
            if (op->sibScale) {
                shiftLeft32(reg, op->sibScale);
            }

            // seg[6] is always 0
            if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
                // add eax, [cpu->seg[op->base].address]
                DynReg tmp = getUnsavedTmpReg();
                loadFromCpuOffset32(tmp, CPU::offsetofSegAddress(op->base));
                addRegs32(reg, tmp);
                clearRegUsed(tmp);
            }
        } else {
            // seg[6] is always 0
            if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
                initiallized = true;
                loadFromCpuOffset32(reg, CPU::offsetofSegAddress(op->base));
            }
        }
        // add eax, [cpu->reg[op->rm].u32]
        if (op->rm != 8) {
            if (!initiallized) {
                initiallized = true;
                loadFromCpuOffset32(reg, CPU::offsetofReg32(op->rm));
            } else {
                DynReg tmp = getUnsavedTmpReg();
                loadFromCpuOffset32(tmp, CPU::offsetofReg32(op->rm));
                addRegs32(reg, tmp);
                clearRegUsed(tmp);
            }
        }

        // add eax, op->disp 
        if (op->disp) {
            if (!initiallized) {
                initiallized = true;
                loadConst32(reg, op->disp);
            } else {
                addValue32(reg, op->disp);
            }
        }
        if (!initiallized) {
            zeroReg32(reg);
        }
    }
}

void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    setRegUsed(dst);
    if (dstWidth <= srcWidth) {
        movToRegFromReg(dst, dstWidth, src, srcWidth, doneWithSrcReg);
    } else {
        if (srcWidth == DYN_16bit) {
            mov32sx16(dst, src);
        } else if (srcWidth == DYN_8bit) {
            mov32sx8(dst, src);
        }
    }
    if (doneWithSrcReg) {
        clearRegUsed(src);
    }
}

void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    setRegUsed(dst);
    if (dstWidth <= srcWidth) {
        if (dst == src) // downsizing doesn't need anything
            return;
        mov32(dst, src);
    } else {
        if (srcWidth == DYN_16bit) {
            mov32zx16(dst, src);
        } else if (srcWidth == DYN_8bit) {
            mov32zx8(dst, src);
        }
    }
    if (doneWithSrcReg) {
        clearRegUsed(src);
    }
}

void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width) {
    setRegUsed(reg);
    // mov reg, [edi+srcOffset]    
    if (width == DYN_32bit) {
        loadFromCpuOffset32(reg, srcOffset);
    } else if (width == DYN_16bit) {
        loadFromCpuOffset16(reg, srcOffset);
    } else if (width == DYN_8bit) {
        loadFromCpuOffset8(reg, srcOffset);
    } else {
        kpanic("unknown dstWidth in dyn::movToRegFromCpu %d", width);
    }
}

void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) {
    // mov [edi+dstOffset], reg
    if (width == DYN_32bit) {
        saveRegToCpuOffset32(reg, dstOffset);
    } else if (width == DYN_16bit) {
        saveRegToCpuOffset16(reg, dstOffset);
    } else if (width == DYN_8bit) {
        saveRegToCpuOffset8(reg, dstOffset);
    } else {
        kpanic("unknown dstWidth in dyn::movToCpuFromReg %d", width);
    }
    if (doneWithReg) {
        clearRegUsed(reg);
    }
}

void movToCpuFromRegPtr(U32 dstOffset, DynReg reg, bool doneWithReg) {
    // mov [edi+dstOffset], reg
    saveRegToCpuOffsetPtr(reg, dstOffset);
    if (doneWithReg) {
        clearRegUsed(reg);
    }
}

void movToCpuFromCpu(U32 dstOffset, U32 srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) {
    // mov tmpReg, [cpu+srcOffset]
    movToRegFromCpu(tmpReg, srcOffset, width);

    // mov [cpu+dstOffset], tmpReg
    movToCpuFromReg(dstOffset, tmpReg, width, doneWithTmpReg);
}

void movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm) {
    // mov [cpu+dstOffset], imm
    if (dstWidth == DYN_32bit) {
        saveValueToCpuOffset32((U32)imm, dstOffset);
    } else if (dstWidth == DYN_16bit) {
        saveValueToCpuOffset16((U16)imm, dstOffset);
    } else if (dstWidth == DYN_8bit) {
        saveValueToCpuOffset8((U8)imm, dstOffset);
    } else {
        kpanic("unknown dstWidth in movToCpu %d", dstWidth);
    }
}

void movToCpuPtr(U32 dstOffset, DYN_PTR_SIZE imm) {
    // mov [cpu+dstOffset], imm
    saveValueToCpuOffsetPtr(imm, dstOffset);
}

void movToReg(DynReg reg, DynWidth width, U32 imm) {
    setRegUsed(reg);
    loadConst32(reg, imm);
}

void movToRegPtr(DynReg reg, U64 imm) {
    setRegUsed(reg);
    loadConstPtr(reg, imm);
}

static U32 readd(U32 address) {
    return KThread::currentThread()->memory->readd(address);
}

static U32 readw(U32 address) {
    return KThread::currentThread()->memory->readw(address);
}

static U32 readb(U32 address) {
    return KThread::currentThread()->memory->readb(address);
}

static void writed(U32 address, U32 value) {
    KThread::currentThread()->memory->writed(address, value);
}

static void writew(U32 address, U16 value) {
    KThread::currentThread()->memory->writew(address, value);
}

static void writeb(U32 address, U8 value) {
    KThread::currentThread()->memory->writeb(address, value);
}

void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg) {
    // :TODO: inline
    if (width == DYN_16bit) {
        callHostFunction((void*)readw, true, 1, addressReg, DYN_PARAM_REG_32, doneWithAddressReg);
    } else if (width == DYN_32bit) {
        callHostFunction((void*)readd, true, 1, addressReg, DYN_PARAM_REG_32, doneWithAddressReg);
    } else {
        callHostFunction((void*)readb, true, 1, addressReg, DYN_PARAM_REG_32, doneWithAddressReg);
    }
    if (doneWithAddressReg) {
        clearRegUsed(addressReg);
    }
}

void movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) {
    movFromMem(dstWidth, addressReg, doneWithAddressReg);
    // mov [cpu+srcOffset], eax
    movToCpuFromReg(dstOffset, DYN_CALL_RESULT, dstWidth, doneWithCallResult);
}

#ifdef DYN_NEEDS_PUSH_POP_REG_FOR_FUNCTION_PARAMS
void pushValue(U32 arg, DynCallParamType argType) {
    switch (argType) {
    case DYN_PARAM_REG_8:
    case DYN_PARAM_REG_16:
    case DYN_PARAM_REG_32:
        pushRegs(1 << arg);
        break;
    case DYN_PARAM_CPU:
        pushRegs(1 << REG_CPU);
        break;
    case DYN_PARAM_CONST_8:
    case DYN_PARAM_CONST_16:
    case DYN_PARAM_CONST_32:
    {
        DynReg tmp = getUnsavedTmpReg();
        loadConst32(tmp, arg);
        pushRegs(1 << tmp);
        clearRegUsed(tmp);
        break;
    }
    case DYN_PARAM_CONST_PTR:
    {
        DynReg tmp = getUnsavedTmpReg();
        loadConstPtr(tmp, arg);
        pushRegs(1 << tmp);
        clearRegUsed(tmp);
        break;
    }
    case DYN_PARAM_ABSOLUTE_ADDRESS_8:
    {
        DynReg tmp = getUnsavedTmpReg();
        readMem8(tmp, arg, 0);
        pushRegs(1 << tmp);
        clearRegUsed(tmp);
        break;
    }
    case DYN_PARAM_ABSOLUTE_ADDRESS_16:
    {
        DynReg tmp = getUnsavedTmpReg();
        readMem16(tmp, arg, 0);
        pushRegs(1 << tmp);
        clearRegUsed(tmp);
        break;
    }
    case DYN_PARAM_ABSOLUTE_ADDRESS_32:
    {
        DynReg tmp = getUnsavedTmpReg();
        readMem32(tmp, arg, 0);
        pushRegs(1 << tmp);
        clearRegUsed(tmp);
        break;
    }
    case DYN_PARAM_CPU_ADDRESS_8:
    {
        DynReg tmp = getUnsavedTmpReg();
        readMem8(tmp, REG_CPU, arg);
        pushRegs(1 << tmp);
        clearRegUsed(tmp);
        break;
    }
    case DYN_PARAM_CPU_ADDRESS_16:
    {
        DynReg tmp = getUnsavedTmpReg();
        readMem16(tmp, REG_CPU, arg);
        pushRegs(1 << tmp);
        clearRegUsed(tmp);
        break;
    }
    case DYN_PARAM_CPU_ADDRESS_32:
    {
        DynReg tmp = getUnsavedTmpReg();
        readMem32(tmp, REG_CPU, arg);
        pushRegs(1 << tmp);
        clearRegUsed(tmp);
        break;
    }
    default:
        kpanic("dynCPU: unknown argType: %d", argType);
        break;
    }
}
#endif

void setValue(DYN_PTR_SIZE arg, DynCallParamType argType, U8 reg) {
    switch (argType) {
    case DYN_PARAM_REG_8:
    case DYN_PARAM_REG_16:
    case DYN_PARAM_REG_32:
        if (reg != arg) {
            mov32(reg, arg);
        }
        break;
    case DYN_PARAM_CPU:
        movPtr(reg, REG_CPU);
        break;
    case DYN_PARAM_CONST_8:
    case DYN_PARAM_CONST_16:
    case DYN_PARAM_CONST_32:
        loadConst32(reg, arg);
        break;
    case DYN_PARAM_CONST_PTR:
        loadConstPtr(reg, arg);
        break;
    case DYN_PARAM_ABSOLUTE_ADDRESS_8:
        readMem8(reg, arg, 0);
        break;
    case DYN_PARAM_ABSOLUTE_ADDRESS_16:
        readMem16(reg, arg, 0);
        break;
    case DYN_PARAM_ABSOLUTE_ADDRESS_32:
        readMem32(reg, arg, 0);
        break;
    case DYN_PARAM_CPU_ADDRESS_8:
        readMem8(reg, REG_CPU, arg);
        break;
    case DYN_PARAM_CPU_ADDRESS_16:
        readMem16(reg, REG_CPU, arg);
        break;
    case DYN_PARAM_CPU_ADDRESS_32:
        readMem32(reg, REG_CPU, arg);
        break;
    default:
        kpanic("dynCPU: unknown argType: %d", argType);
        break;
    }
}

bool isParamTypeReg(DynCallParamType paramType) {
    return paramType == DYN_PARAM_REG_8 || paramType == DYN_PARAM_REG_16 || paramType == DYN_PARAM_REG_32;
}

// will inline the following code snippet (this code is for 32-bit, but it will do a similiar thing for 16-bit and 8-bit)
//
// if ((address & 0xFFF) < 0xFFD) {
//      int index = address >> 12;
//      if (Memory::currentMMUWritePtr[index])
//          *(U32*)(&Memory::currentMMUWritePtr[index][address & 0xFFF]) = value;
//      else
//          Memory::currentMMU[index]->writed(address, value);		
//  } else {
//      Memory::currentMMU[index]->writed(address, value);		
//  }
void movToMem(DynReg addressReg, DynWidth width, U32 value, DynCallParamType paramType, bool doneWithValueReg) {
    // :TODO: inline?
    if (width == DYN_16bit) {
        callHostFunction((void*)writew, false, 2, addressReg, DYN_PARAM_REG_32, false, value, paramType, doneWithValueReg);
    } else if (width == DYN_32bit) {
        callHostFunction((void*)writed, false, 2, addressReg, DYN_PARAM_REG_32, false, value, paramType, doneWithValueReg);
    } else {
        callHostFunction((void*)writeb, false, 2, addressReg, DYN_PARAM_REG_32, false, value, paramType, doneWithValueReg);
    }
}

void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg) {
    // push reg
    DynCallParamType paramType;

    if (width == DYN_8bit) {
        paramType = DYN_PARAM_REG_8;
    } else if (width == DYN_16bit) {
        paramType = DYN_PARAM_REG_16;
    } else if (width == DYN_32bit) {
        paramType = DYN_PARAM_REG_32;
    } else {
        kpanic("unknown width %d in dyn::movToMemFromReg", width);
        paramType = DYN_PARAM_REG_32; // makes warning go away
    }

    movToMem(addressReg, width, reg, paramType, doneWithReg);
    if (doneWithAddressReg) {
        clearRegUsed(addressReg);
    }    
}

void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg) {
    DynCallParamType paramType;

    if (width == DYN_8bit) {
        paramType = DYN_PARAM_CONST_8;
    } else if (width == DYN_16bit) {
        paramType = DYN_PARAM_CONST_16;
    } else if (width == DYN_32bit) {
        paramType = DYN_PARAM_CONST_32;
    } else {
        kpanic("unknown width %d in dyn::movToMemFromImm", width);
        paramType = DYN_PARAM_CONST_32;
    }
    movToMem(addressReg, width, imm, paramType, false);
    if (doneWithAddressReg) {
        clearRegUsed(addressReg);
    }
}

// when called with 16-bit or 8-bit width, the upper bits do not need to be preserved or zero'd out, but watch out for signed operations
// inst can be +, |, - , &, ^, <, >, ) right parens is for signed right shift
void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm) {
    switch (inst) {
    case '<':
        shiftLeft32(reg, imm);
        break;
    case '>':
        shiftRight32(reg, imm);
        break;
    case ')':
        if (regWidth == DYN_16bit) {
            mov32sx16(reg, reg);
        } else if (regWidth == DYN_8bit) {
            mov32sx8(reg, reg);
        }
        shiftRightSigned32(reg, imm);
        break;
    case '+':
        if (regWidth == DYN_16bit) {
            imm = (U32)(S32)(((S16)imm));
        } else if (regWidth == DYN_8bit) {
            imm = (U32)(S32)(((S8)imm));
        }
        addValue32(reg, imm);
        break;
    case '-':
        if (regWidth == DYN_16bit) {
            imm = (U32)(S32)(((S16)imm));
        } else if (regWidth == DYN_8bit) {
            imm = (U32)(S32)(((S8)imm));
        }
        subValue32(reg, imm);
        break;
    case '|':
        if (regWidth == DYN_16bit) {
            imm = (U32)(S32)(((S16)imm));
        } else if (regWidth == DYN_8bit) {
            imm = (U32)(S32)(((S8)imm));
        }
        orValue32(reg, imm);
        break;
    case '&':
        if (regWidth == DYN_16bit) {
            imm = (U32)(S32)(((S16)imm));
        } else if (regWidth == DYN_8bit) {
            imm = (U32)(S32)(((S8)imm));
        }
        andValue32(reg, imm);
        break;
    case '^':
        if (regWidth == DYN_16bit) {
            imm = (U32)(S32)(((S16)imm));
        } else if (regWidth == DYN_8bit) {
            imm = (U32)(S32)(((S8)imm));
        }
        xorValue32(reg, imm);
        break;
    default:
        kpanic("unhandled op in dyn::instRegIMM %c", inst);
        break;
    }
}

// when called with 16-bit or 8-bit width, the upper bits do not need to be preserved or zero'd out, but watch out for signed operations
// inst can be +, |, -, &, ^, <, >, ) right parens is for signed right shift
void instRegReg(char inst, DynReg dst, DynReg src, DynWidth regWidth, bool doneWithRmReg) {
    switch (inst) {
    case '<':
    {
        int tmpReg = getUnsavedTmpReg();
        andValue32(tmpReg, src, 0x1f);
        shiftLeft32WithReg(dst, tmpReg);
        clearRegUsed(tmpReg);
        break;
    }
    case '>':
    {
        int tmpReg = getUnsavedTmpReg();
        andValue32(tmpReg, src, 0x1f);
        shiftRight32WithReg(dst, tmpReg);
        clearRegUsed(tmpReg);
        break;
    }
    case ')':
    {
        if (regWidth == DYN_16bit) {
            mov32sx16(dst, dst);
        } else if (regWidth == DYN_8bit) {
            mov32sx8(dst, dst);
        }
        int tmpReg = getUnsavedTmpReg();
        andValue32(tmpReg, src, 0x1f);
        shiftRightSigned32WithReg(dst, tmpReg);
        clearRegUsed(tmpReg);
        break;
    }
    case '+':
        addRegs32(dst, src);
        break;
    case '-':
        subRegs32(dst, src);
        break;
    case '|':
        orRegs32(dst, src);
        break;
    case '&':
        andRegs32(dst, src);
        break;
    case '^':
        xorRegs32(dst, src);
        break;
    default:
        kpanic("unhandled op in dyn::instRegIMM %c", inst);
        break;
    }
    if (doneWithRmReg) {
        clearRegUsed(src);
    }
}

void instCPUReg(char inst, U32 dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    DynReg tmp = getUnsavedTmpReg();
    movToRegFromCpu(tmp, dstOffset, regWidth);
    instRegReg(inst, tmp, rm, regWidth, doneWithRmReg);
    movToCpuFromReg(dstOffset, tmp, regWidth, true);
}

void instCPUImm(char inst, U32 dstOffset, DynWidth regWidth, U32 imm) {
    DynReg tmp = getUnsavedTmpReg();
    movToRegFromCpu(tmp, dstOffset, regWidth);
    instRegImm(inst, tmp, regWidth, imm);
    movToCpuFromReg(dstOffset, tmp, regWidth, true);
}

void instMemImm(char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg) {
    movFromMem(regWidth, addressReg, false);
    instRegImm(inst, DYN_CALL_RESULT, regWidth, imm);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, doneWithAddressReg, true);
}

void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg) {
    movFromMem(regWidth, addressReg, false);
    instRegReg(inst, DYN_CALL_RESULT, rm, regWidth, doneWithRmReg);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, doneWithAddressReg, true);
}

// when called with 16-bit or 8-bit width, the upper bits do not need to be preserved or zero'd out, but watch out for signed operations
// inst can be: ~ or -
void instReg(char inst, DynReg reg, DynWidth regWidth) {
    switch (inst) {
    case '~':
        notReg32(reg);
        break;
    case '-':
        if (regWidth == DYN_16bit) {
            mov32sx16(reg, reg);
        } else if (regWidth == DYN_8bit) {
            mov32sx8(reg, reg);
        }
        negReg32(reg);
        break;
    default:
        kpanic("unhandled op in dyn::instReg %c", inst);
        break;
    }
}

void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg) {
    movFromMem(regWidth, addressReg, false);
    instReg(inst, DYN_CALL_RESULT, regWidth);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, doneWithAddressReg, true);
}

void instCPU(char inst, U32 dstOffset, DynWidth regWidth) {
    DynReg tmp = getUnsavedTmpReg();
    movToRegFromCpu(tmp, dstOffset, regWidth);
    instReg(inst, tmp, regWidth);
    movToCpuFromReg(dstOffset, tmp, regWidth, true);
}

void startIf(DynReg reg, DynCondition condition, bool doneWithReg) {
    cmpRegValue32(reg, 0);

    if (condition == DYN_NOT_EQUALS_ZERO) {
        ifJump.push_back(jumpIfEqual());
    } else if (condition == DYN_EQUALS_ZERO) {
        ifJump.push_back(jumpIfNotEqual());
    } else {
        kpanic("dyn::startIf unknown condition %d", condition);
    }
    if (doneWithReg) {
        clearRegUsed(reg);
    }
}

void startElse() {
    // previous block should jump over else statement
    U32 pos = unconditionalJump();

    // if statement will jump here if it wasn't true
    endIf();

    ifJump.push_back(pos);
}

void endIf() {
    U32 pos = ifJump.back();
    ifJump.pop_back();
    writeJumpAmount(pos, outBufferPos);
}

void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    bool signedCompare = (condition == DYN_LESS_THAN_SIGNED) || (condition == DYN_LESS_THAN_EQUAL_SIGNED);
    if (signedCompare) {
        if (regWidth == DYN_16bit) {
            mov32sx16(left, left);
        } else if (regWidth == DYN_8bit) {
            mov32sx8(left, left);
        }
    }
    if (isRightConst) {
        if (signedCompare) {
            if (regWidth == DYN_16bit) {
                rightConst = (U32)(S32)(((S16)rightConst));
            } else if (regWidth == DYN_8bit) {
                rightConst = (U32)(S32)(((S8)rightConst));
            }
        }
        cmpRegValue32(left, rightConst);
    } else {
        if (signedCompare) {
            if (regWidth == DYN_16bit) {
                mov32sx16(right, right);
            } else if (regWidth == DYN_8bit) {
                mov32sx8(right, right);
            }
        }
        cmpRegs32(left, right);
    }
    evaluateCondition(reg, condition);
    if (doneWithLeftReg) {
        clearRegUsed(left);
    }
    if (doneWithRightReg) {
        clearRegUsed(right);
    }
    setRegUsed(reg);
}

void setCC(DynamicData* data, DynConditional condition) {
    bool notCondition = false;
    // changing conditions to ones that are optimized
    if (condition == Z) {
        condition = NZ;
        notCondition = true;
    } else if (condition == NS) {
        condition = S;
        notCondition = true;
    } else if (condition == NB) {
        condition = B;
        notCondition = true;
    } else if (condition == NO) {
        condition = O;
        notCondition = true;
    } else if (condition == NBE) {
        condition = BE;
        notCondition = true;
    } else if (condition == NLE) {
        condition = LE;
        notCondition = true;
    } else if (condition == NL) {
        condition = L;
        notCondition = true;
    }
    setConditionInReg(data, condition, DYN_CALL_RESULT);
    cmpRegValue32(DYN_CALL_RESULT, 0);

    if (notCondition) {
        evaluateCondition(DYN_CALL_RESULT, DYN_EQUALS);
    } else {
        evaluateCondition(DYN_CALL_RESULT, DYN_NOT_EQUALS);
    }
}

void setCPU(DynamicData* data, U32 offset, DynWidth regWidth, DynConditional condition) {
    setCC(data, condition);
    movToCpuFromReg(offset, DYN_CALL_RESULT, regWidth, true);
}

void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg) {
    setCC(data, condition);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, doneWithAddressReg, true);
}

void incrementEip(DynamicData* data, U32 inc) {
    if (data->skipEipUpdateLen) {
        kpanic("incrementEip had an unexpected update");
    }
    instCPUImm('+', offsetof(CPU, eip.u32), DYN_32bit, inc);
}

void incrementEip(DynamicData* data, DecodedOp* op) {
    if (op->next) {
        const InstructionInfo& info = instructionInfo[op->next->inst];
        if (!info.branch && !info.readMemWidth && !info.writeMemWidth && !info.throwsException) {
            data->skipEipUpdateLen += op->len;
            return;
        }
    }
    U32 len = op->len + data->skipEipUpdateLen;
    data->skipEipUpdateLen = 0;
    instCPUImm('+', offsetof(CPU, eip.u32), DYN_32bit, len);
}

void blockDone() {
    // cpu->nextBlock = cpu->getNextBlock();
    callHostFunction((void*)common_getNextBlock, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromRegPtr(offsetof(CPU, nextBlock), DYN_CALL_RESULT, true);
    endBlock();
}

static DecodedBlock* updateNext1(CPU* cpu) {
    DecodedBlock::currentBlock->next1 = cpu->getNextBlock();
    DecodedBlock::currentBlock->next1->addReferenceFrom(DecodedBlock::currentBlock);
    return DecodedBlock::currentBlock->next1;
}

// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
void blockNext1() {
    // if (!DecodedBlock::currentBlock->next1) {
    //    DecodedBlock::currentBlock->next1 = cpu->getNextBlock(); 
    //    DecodedBlock::currentBlock->next1->addReferenceFrom(DecodedBlock::currentBlock);
    // } 
    // cpu->nextBlock = DecodedBlock::currentBlock->next1

    movToRegPtr(DYN_CALL_RESULT, (DYN_PTR_SIZE)DecodedBlock::currentBlock);
    readMemPtr(DYN_CALL_RESULT, DYN_CALL_RESULT, offsetof(DecodedBlock, next1));

    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, false);
    callHostFunction((void*)updateNext1, true, 1, 0, DYN_PARAM_CPU);
    endIf();

    saveRegToCpuOffsetPtr(DYN_CALL_RESULT, offsetof(CPU, nextBlock));
    clearRegUsed(DYN_CALL_RESULT);
}

static DecodedBlock* updateNext2(CPU* cpu) {
    DecodedBlock::currentBlock->next2 = cpu->getNextBlock();
    DecodedBlock::currentBlock->next2->addReferenceFrom(DecodedBlock::currentBlock);
    return DecodedBlock::currentBlock->next2;
}

void blockNext2() {
    // if (!DecodedBlock::currentBlock->next2) {
    //    DecodedBlock::currentBlock->next2 = cpu->getNextBlock(); 
    //    DecodedBlock::currentBlock->next2->addReferenceFrom(DecodedBlock::currentBlock);
    // } 
    // cpu->nextBlock = DecodedBlock::currentBlock->next2

    movToRegPtr(DYN_CALL_RESULT, (DYN_PTR_SIZE)DecodedBlock::currentBlock);
    readMemPtr(DYN_CALL_RESULT, DYN_CALL_RESULT, offsetof(DecodedBlock, next2));

    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, false);
    callHostFunction((void*)updateNext2, true, 1, 0, DYN_PARAM_CPU);
    endIf();

    saveRegToCpuOffsetPtr(DYN_CALL_RESULT, offsetof(CPU, nextBlock));
    clearRegUsed(DYN_CALL_RESULT);
}

void dyn_sidt(DynamicData* data, DecodedOp* op) {
}

void dyn_onExitSignal(CPU* cpu) {
    onExitSignal(cpu, NULL);
}

void dyn_callback(DynamicData* data, DecodedOp* op) {
    if (op->pfn == onExitSignal) {
        callHostFunction((void*)dyn_onExitSignal, false, 1, 0, DYN_PARAM_CPU);
    } else {
        kpanic("dyn_callback unhandled callback");
    }
}

void dyn_invalid_op(DynamicData* data, DecodedOp* op) {
    kpanic("Invalid instruction %x\n", op->inst);
}

static pfnDynamicOp dynOps[NUMBER_OF_OPS];
static U32 dynOpsInitialized;

static void initDynOps() {
    if (dynOpsInitialized)
        return;

    dynOpsInitialized = 1;
    for (int i = 0; i < InstructionCount; i++) {
        dynOps[i] = dyn_invalid_op;
    }
#define INIT_CPU(e, f) dynOps[e] = dynamic_##f;
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_sse.h"
#include "../common/cpu_init_sse2.h"
#include "../common/cpu_init_fpu.h"
#undef INIT_CPU    

    dynOps[SLDTReg] = 0;
    dynOps[SLDTE16] = 0;
    dynOps[STRReg] = 0;
    dynOps[STRE16] = 0;
    dynOps[LLDTR16] = 0;
    dynOps[LLDTE16] = 0;
    dynOps[LTRR16] = 0;
    dynOps[LTRE16] = 0;
    dynOps[VERRR16] = 0;
    dynOps[VERWR16] = 0;
    dynOps[SGDT] = 0;
    dynOps[SIDT] = dyn_sidt;
    dynOps[LGDT] = 0;
    dynOps[LIDT] = 0;
    dynOps[SMSWRreg] = 0;
    dynOps[SMSW] = 0;
    dynOps[LMSWRreg] = 0;
    dynOps[LMSW] = 0;
    dynOps[INVLPG] = 0;
    dynOps[Callback] = dyn_callback;
}

void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op) {
#ifdef __TEST
    if (DecodedBlock::currentBlock->runCount == 0) {
#else
    if (DecodedBlock::currentBlock->runCount == 50) {
#endif
        DynamicData data;
        data.cpu = cpu;
        data.block = DecodedBlock::currentBlock;

        initDynOps();
        DecodedOp* o = op->next;
        outBufferPos = 0;
        startBlock();

        while (o) {
            resetRegsUsed();
#ifndef __TEST
#ifdef _DEBUG
            //callHostFunction((void*)common_log, false, 2, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)o, DYN_PARAM_CONST_PTR, false);
#endif
#endif
            dynOps[o->inst](&data, o);
#ifdef _DEBUG
            for (int i = 0; i < sizeof(regUsed) / sizeof(regUsed[0]); i++) {
                if (regUsed[i]) {
                    kpanic("dyn: reg %d was not released", i);
                }
            }
#endif
            if (ifJump.size()) {
                kpanic("dyn::firstDynamicOp if statement was not closed in instruction: %d", op->inst);
            }
            if (data.skipToOp) {
                o = data.skipToOp;
                data.skipToOp = NULL;
            } else if (data.done) {
#ifndef __TEST
#ifdef _DEBUG
                //f (o->next)
                //    callHostFunction((void*)common_log, false, 2, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)o->next, DYN_PARAM_CONST_PTR, false);
#endif
#endif
                break;
            } else {
                o = o->next;
            }
        }
        endBlock();

        DynamicMemory* memory = (DynamicMemory*)cpu->memory->dynamicMemory;
        if (!memory) {
            memory = new DynamicMemory();
            cpu->memory->dynamicMemory = memory;
        }
        void* mem = NULL;

        if (memory->dynamicExecutableMemory.size() == 0) {
            int blocks = (outBufferPos + 0xffff) / 0x10000;
            memory->dynamicExecutableMemoryLen = blocks * 0x10000;
            mem = Platform::alloc64kBlock(blocks, true);
            memory->dynamicExecutableMemoryPos = 0;
            memory->dynamicExecutableMemory.push_back(DynamicMemoryData(mem, blocks * 0x10000));
        } else {
            mem = memory->dynamicExecutableMemory[memory->dynamicExecutableMemory.size() - 1].p;
            if (memory->dynamicExecutableMemoryPos + outBufferPos >= memory->dynamicExecutableMemoryLen) {
                int blocks = (outBufferPos + 0xffff) / 0x10000;
                memory->dynamicExecutableMemoryLen = blocks * 0x10000;
                mem = Platform::alloc64kBlock(blocks, true);
                memory->dynamicExecutableMemoryPos = 0;
                memory->dynamicExecutableMemory.push_back(DynamicMemoryData(mem, blocks * 0x10000));
            }
        }
        U8* begin = (U8*)mem + memory->dynamicExecutableMemoryPos;

        Platform::writeCodeToMemory(begin, outBufferPos, [begin] {
            memcpy(begin, outBuffer, outBufferPos);
            });

        memory->dynamicExecutableMemoryPos += outBufferPos;

        bool b = false;
        if (b) {
            printf("\n");
            for (U32 i = 0; i < outBufferPos; i++) {
                printf("%.2X ", outBuffer[i]);
            }
            printf("\n");
        }
#ifndef _DEBUG
        //op->next->dealloc(true);
        //op->next = NULL;
#endif
        op->pfn = (OpCallback)begin; // :TODO: if function is expected to pop stack because of the two passed params, then this will not work        
        op->pfn(cpu, op);
    } else {
        op->next->pfn(cpu, op->next);
    }
}
#endif
