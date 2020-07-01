#ifndef __DYNAMIC_GENERIC_BASE_H__
#define __DYNAMIC_GENERIC_BASE_H__

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
void pushRegs(U16 bitMask);
void popRegs(U16 bitMask);
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
*/

bool isRegSetFFF;
bool isRegSetFFD;

void initDyn() {
    isRegSetFFF = false;
    isRegSetFFD = false;
}

U8 getRegFFF() {
    if (!isRegSetFFF) {
        loadConst32(REG_TMP_FFF_FFD, 0xFFF);
        isRegSetFFF = true;
        isRegSetFFD = false;
    }
    return REG_TMP_FFF_FFD;
}

U8 getRegFFD() {
    if (!isRegSetFFD) {
        loadConst32(REG_TMP_FFF_FFD, 0xFFD);
        isRegSetFFD = true;
        isRegSetFFF = false;
    }
    return REG_TMP_FFF_FFD;
}

void calculateEaa(DecodedOp* op, DynReg reg) {
    if (op->ea16) {
        // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)

        zeroReg32(reg);

        if (op->disp) {
            addValue32(reg, op->disp);
        }
        // add ax, [DYN_CPU_REG+cpu->reg[op->rm].u16]
        if (op->rm != 8) {
            loadFromCpuOffset16(REG_LOAD_TMP, offsetof(CPU, reg[op->rm].u16));
            addRegs32(reg, REG_LOAD_TMP);
        }

        // add ax, [cpu->reg[op->sibIndex].u16]
        if (op->sibIndex != 8) {
            loadFromCpuOffset16(REG_LOAD_TMP, offsetof(CPU, reg[op->sibIndex].u16));
            addRegs32(reg, REG_LOAD_TMP);
        }

        // don't allow the above adds to roll over past 16-bits
        clearTop16(reg);

        // seg[6] is always 0
        if (op->base < 6) {
            // add eax, [cpu->seg[op->base].address]
            loadFromCpuOffset32(REG_LOAD_TMP, offsetof(CPU, seg[op->base].address));
            addRegs32(reg, REG_LOAD_TMP);
        }
    } else {
        // cpu->seg[op->base].address + cpu->reg[op->rm].u32 + (cpu->reg[op->sibIndex].u32 << + op->sibScale) + op->disp
        bool initiallized = false;

        if (op->sibIndex != 8) {
            initiallized = true;
            loadFromCpuOffset32(reg, offsetof(CPU, reg[op->sibIndex].u32));
            if (op->sibScale) {
                shiftLeft32(reg, op->sibScale);
            }

            // seg[6] is always 0
            if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
                // add eax, [cpu->seg[op->base].address]
                loadFromCpuOffset32(REG_LOAD_TMP, offsetof(CPU, seg[op->base].address));
                addRegs32(reg, REG_LOAD_TMP);
            }
        } else {
            // seg[6] is always 0
            if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
                initiallized = true;
                loadFromCpuOffset32(reg, offsetof(CPU, seg[op->base].address));
            }
        }
        // add eax, [cpu->reg[op->rm].u32]
        if (op->rm != 8) {
            if (!initiallized) {
                initiallized = true;
                loadFromCpuOffset32(reg, offsetof(CPU, reg[op->rm].u32));
            } else {
                loadFromCpuOffset32(REG_LOAD_TMP, offsetof(CPU, reg[op->rm].u32));
                addRegs32(reg, REG_LOAD_TMP);
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
    if (dstWidth <= srcWidth) {
        movToRegFromReg(dst, dstWidth, src, srcWidth, doneWithSrcReg);
    } else {
        if (srcWidth == DYN_16bit) {
            mov32sx16(dst, src);
        } else if (srcWidth == DYN_8bit) {
            mov32sx8(dst, src);
        }
    }
}

void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
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
}

void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width) {
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
        saveValueToCpuOffset32(imm, dstOffset);
    } else if (dstWidth == DYN_16bit) {
        saveValueToCpuOffset16((U16)imm, dstOffset);
    } else if (dstWidth == DYN_8bit) {
        saveValueToCpuOffset8((U8)imm, dstOffset);
    } else {
        kpanic("unknown dstWidth in movToCpu %d", dstWidth);
    }
}

void movToReg(DynReg reg, DynWidth width, U32 imm) {
    loadConst32(reg, imm);
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
}

void movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) {
    movFromMem(dstWidth, addressReg, doneWithAddressReg);
    // mov [cpu+srcOffset], eax
    movToCpuFromReg(dstOffset, DYN_CALL_RESULT, dstWidth, doneWithCallResult);
}

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
    case DYN_PARAM_CONST_PTR:
        loadConst32(REG_LOAD_TMP, arg);
        pushRegs(1 << REG_LOAD_TMP);
        break;
    case DYN_PARAM_ABSOLUTE_ADDRESS_8:
        readMem8(REG_LOAD_TMP, arg, 0);
        pushRegs(1 << REG_LOAD_TMP);
        break;
    case DYN_PARAM_ABSOLUTE_ADDRESS_16:
        readMem16(REG_LOAD_TMP, arg, 0);
        pushRegs(1 << REG_LOAD_TMP);
        break;
    case DYN_PARAM_ABSOLUTE_ADDRESS_32:
        readMem32(REG_LOAD_TMP, arg, 0);
        pushRegs(1 << REG_LOAD_TMP);
        break;
    case DYN_PARAM_CPU_ADDRESS_8:
        readMem8(REG_LOAD_TMP, REG_CPU, arg);
        pushRegs(1 << REG_LOAD_TMP);
        break;
    case DYN_PARAM_CPU_ADDRESS_16:
        readMem16(REG_LOAD_TMP, REG_CPU, arg);
        pushRegs(1 << REG_LOAD_TMP);
        break;
    case DYN_PARAM_CPU_ADDRESS_32:
        readMem32(REG_LOAD_TMP, REG_CPU, arg);
        pushRegs(1 << REG_LOAD_TMP);
        break;
    default:
        kpanic("dynCPU: unknown argType: %d", argType);
        break;
    }
}

void setValue(U32 arg, DynCallParamType argType, U8 reg) {
    switch (argType) {
    case DYN_PARAM_REG_8:
    case DYN_PARAM_REG_16:
    case DYN_PARAM_REG_32:
        if (reg != arg) {
            mov32(reg, arg);
        }
        break;
    case DYN_PARAM_CPU:
        mov32(reg, REG_CPU);
        break;
    case DYN_PARAM_CONST_8:
    case DYN_PARAM_CONST_16:
    case DYN_PARAM_CONST_32:
    case DYN_PARAM_CONST_PTR:
        loadConst32(reg, arg);
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
        callHostFunction((void*)writew, true, 2, addressReg, DYN_PARAM_REG_32, false, value, paramType, doneWithValueReg);
    } else if (width == DYN_32bit) {
        callHostFunction((void*)writed, true, 2, addressReg, DYN_PARAM_REG_32, false, value, paramType, doneWithValueReg);
    } else {
        callHostFunction((void*)writeb, true, 2, addressReg, DYN_PARAM_REG_32, false, value, paramType, doneWithValueReg);
    }
}

void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg) {
    // push reg
    DynCallParamType paramType;

    if (width == DYN_8bit)
        paramType = DYN_PARAM_REG_8;
    else if (width == DYN_16bit)
        paramType = DYN_PARAM_REG_16;
    else if (width == DYN_32bit)
        paramType = DYN_PARAM_REG_32;
    else
        kpanic("unknown width %d in dyn::movToMemFromReg", width);

    movToMem(addressReg, width, reg, paramType, doneWithReg);
}

void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg) {
    DynCallParamType paramType;

    if (width == DYN_8bit)
        paramType = DYN_PARAM_CONST_8;
    else if (width == DYN_16bit)
        paramType = DYN_PARAM_CONST_16;
    else if (width == DYN_32bit)
        paramType = DYN_PARAM_CONST_32;
    else
        kpanic("unknown width %d in dyn::movToMemFromImm", width);

    movToMem(addressReg, width, imm, paramType, false);
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
        shiftLeft32WithReg(dst, src);
        break;
    case '>':
        shiftRight32WithReg(dst, src);
        break;
    case ')':
        if (regWidth == DYN_16bit) {
            mov32sx16(dst, dst);
        } else if (regWidth == DYN_8bit) {
            mov32sx8(dst, dst);
        }
        shiftRightSigned32(dst, src);
        break;
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
}

void instCPUReg(char inst, U32 dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    movToRegFromCpu(REG_TMP_1, dstOffset, regWidth);
    instRegReg(inst, REG_TMP_1, rm, regWidth, doneWithRmReg);
    movToCpuFromReg(dstOffset, REG_TMP_1, regWidth, true);
}

void instCPUImm(char inst, U32 dstOffset, DynWidth regWidth, U32 imm) {
    movToRegFromCpu(REG_TMP_1, dstOffset, regWidth);
    instRegImm(inst, REG_TMP_1, regWidth, imm);
    movToCpuFromReg(dstOffset, REG_TMP_1, regWidth, true);
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
    movToRegFromCpu(REG_TMP_1, dstOffset, regWidth);
    instReg(inst, REG_TMP_1, regWidth);
    movToCpuFromReg(dstOffset, REG_TMP_1, regWidth, true);
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
    // cmp left, right
    if (isRightConst) {
        cmpRegValue32(left, rightConst);
    } else {
        cmpRegs32(left, right);
    }
    evaluateCondition(reg, condition);
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
        evaluateCondition(DYN_CALL_RESULT, DYN_NOT_EQUALS);
    } else {
        evaluateCondition(DYN_CALL_RESULT, DYN_EQUALS);
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

void incrementEip(U32 inc) {
    instCPUImm('+', offsetof(CPU, eip.u32), DYN_32bit, inc);
}

void blockDone() {
    // cpu->nextBlock = cpu->getNextBlock();
    callHostFunction((void*)common_getNextBlock, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(offsetof(CPU, nextBlock), DYN_CALL_RESULT, DYN_32bit, true);
    popRegs(0x20F0);
    rtn();
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

    movToReg(DYN_CALL_RESULT, DYN_32bit, (U32)DecodedBlock::currentBlock);
    readMem32(DYN_CALL_RESULT, DYN_CALL_RESULT, offsetof(DecodedBlock, next1));

    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, false);
    callHostFunction((void*)updateNext1, true, 1, 0, DYN_PARAM_CPU);
    endIf();

    saveRegToCpuOffset32(DYN_CALL_RESULT, offsetof(CPU, nextBlock));
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

    movToReg(DYN_CALL_RESULT, DYN_32bit, (U32)DecodedBlock::currentBlock);
    readMem32(DYN_CALL_RESULT, DYN_CALL_RESULT, offsetof(DecodedBlock, next2));

    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, false);
    callHostFunction((void*)updateNext2, true, 1, 0, DYN_PARAM_CPU);
    endIf();

    saveRegToCpuOffset32(DYN_CALL_RESULT, offsetof(CPU, nextBlock));
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
        patch.clear();        
        startBlock();

        while (o) {
#ifndef __TEST
#ifdef _DEBUG
            callHostFunction((void*)common_log, false, 2, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)o, DYN_PARAM_CONST_PTR, false);
#endif
#endif
            dynOps[o->inst](&data, o);
            if (ifJump.size()) {
                kpanic("dyn::firstDynamicOp if statement was not closed in instruction: %d", op->inst);
            }
            if (data.skipToOp) {
                o = data.skipToOp;
                data.skipToOp = NULL;
            } else if (data.done) {
#ifndef __TEST
#ifdef _DEBUG
                if (o->next)
                    callHostFunction((void*)common_log, false, 2, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)o->next, DYN_PARAM_CONST_PTR, false);
#endif
#endif
                break;
            } else {
                o = o->next;
            }
        }
        endBlock();

        Memory* memory = cpu->thread->process->memory;
        void* mem = NULL;

        if (memory->dynamicExecutableMemory.size() == 0) {
            int blocks = (outBufferPos + 0xffff) / 0x10000;
            memory->dynamicExecutableMemoryLen = blocks * 0x10000;
            mem = allocExecutable64kBlock(blocks);
            memory->dynamicExecutableMemoryPos = 0;
            memory->dynamicExecutableMemory.push_back(mem);
        } else {
            mem = memory->dynamicExecutableMemory[memory->dynamicExecutableMemory.size() - 1];
            if (memory->dynamicExecutableMemoryPos + outBufferPos >= memory->dynamicExecutableMemoryLen) {
                int blocks = (outBufferPos + 0xffff) / 0x10000;
                memory->dynamicExecutableMemoryLen = blocks * 0x10000;
                mem = allocExecutable64kBlock(blocks);
                memory->dynamicExecutableMemoryPos = 0;
                memory->dynamicExecutableMemory.push_back(mem);
            }
        }
        U8* begin = (U8*)mem + memory->dynamicExecutableMemoryPos;
        memcpy(begin, outBuffer, outBufferPos);
        codeCreated(begin, begin + outBufferPos);

        memory->dynamicExecutableMemoryPos += outBufferPos;

        for (U32 i = 0; i < patch.size(); i++) {
            U32 pos = patch[i];
            U32* value = (U32*)(&begin[pos]);
            *value = *value - (U32)(begin + pos + 4);
        }
        bool b = false;
        if (b) {
            printf("\n");
            for (U32 i = 0; i < outBufferPos; i++) {
                printf("%0.2X ", outBuffer[i]);
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