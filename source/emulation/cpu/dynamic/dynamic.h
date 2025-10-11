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

#ifndef __DYNAMIC_H__
#define __DYNAMIC_H__

#include "../common/cpu.h"

enum DynReg {
    DYN_CALL_RESULT = 0,
    DYN_SRC = 1,
    DYN_DEST = 2,
    DYN_ADDRESS = 3,
    DYN_NOT_SET = 0xff
};

enum DynWidth {
    DYN_8bit = 0,
    DYN_16bit,
    DYN_32bit,
};

enum DynCondition {
    DYN_EQUALS_ZERO,
    DYN_NOT_EQUALS_ZERO
};

enum DynConditionEvaluate {
    DYN_EQUALS,
    DYN_NOT_EQUALS,
    DYN_LESS_THAN_UNSIGNED,
    DYN_LESS_THAN_EQUAL_UNSIGNED,
    DYN_GREATER_THAN_EQUAL_UNSIGNED,
    DYN_LESS_THAN_SIGNED,
    DYN_LESS_THAN_EQUAL_SIGNED,
};

enum DynCallParamType {
    DYN_PARAM_REG_8,
    DYN_PARAM_REG_16,
    DYN_PARAM_REG_32,
    DYN_PARAM_CONST_8,
    DYN_PARAM_CONST_16,
    DYN_PARAM_CONST_32,
    DYN_PARAM_CONST_PTR,
    DYN_PARAM_CPU_ADDRESS_8,
    DYN_PARAM_CPU_ADDRESS_16,
    DYN_PARAM_CPU_ADDRESS_32,
    DYN_PARAM_CPU,
};

enum DynConditional {
    O,
    NO,
    B,
    NB,
    Z,
    NZ,
    BE,
    NBE,
    S,
    NS,
    P,
    NP,
    L,
    NL,
    LE,
    NLE
};

#define DYN_PTR_SIZE U32

class DynamicData {
public:
    DynamicData(CPU* cpu) : cpu(cpu) {}
    CPU* cpu = nullptr;
    DecodedOp* firstOp = nullptr;

    const LazyFlags* currentLazyFlags = nullptr;;
    BHashTable<U32, U32> eipToBufferPos;
    U32 currentEip = 0;
    U32 startingEip = 0;
    U32 lastOpEip = 0;
    U32 emulatedLen = 0;
    U32 blockOpCount = 0;
    
    // per instruction, not per block.  
    bool regUsed[4];
    bool canJumpInBlock(DecodedOp* op) {
        return currentEip < lastOpEip && currentEip + op->len + op->imm <= lastOpEip && currentEip + op->len + op->imm >= startingEip;
    }

    virtual void loadRegStoreReg(U8 dst, U8 src, DynWidth width, DynReg tmpReg, bool doneWithTmpReg);
    virtual void loadRegStoreSrc(U8 reg, DynWidth width, DynReg tmpReg, bool doneWithTmpReg);
    virtual void loadRegStoreDst(U8 reg, DynWidth width, DynReg tmpReg, bool doneWithTmpReg);
    virtual void loadRegStoreEip(U8 reg, DynReg tmpReg, bool doneWithTmpReg);
    virtual void loadSegValueStoreReg(U8 reg, U8 seg, DynReg tmpReg, bool doneWithTmpReg);
    virtual DynReg loadReg(U8 reg, DynReg tmpReg, DynWidth width, bool copyIntoTmp = false);
    virtual void loadSegAddress(U8 seg, DynReg reg);
    virtual void loadSegValue(U8 seg, DynReg reg);
    virtual void loadCPUFlags(DynReg reg);
    virtual void loadLazyFlagsResult(DynReg reg, DynWidth width);
    virtual void loadLazyFlagsSrc(DynReg reg, DynWidth width);
    virtual void loadLazyFlagsOldCF(DynReg reg);
    virtual void loadEip(DynReg reg);
    virtual void loadStackMask(DynReg reg);
    virtual void loadStackNotMask(DynReg reg);
    virtual void loadLazyFlags(DynReg reg);
    virtual void loadLazyFlagsDst(DynReg reg, DynWidth width);
    virtual void storeReg(U8 reg, DynReg srcReg, DynWidth width, bool doneWithSrcReg);
    virtual void storeLazyFlagsResult(DynReg srcReg, DynWidth width, bool doneWithSrcReg);
    virtual void storeLazyFlagsDst(DynReg srcReg, DynWidth width, bool doneWithSrcReg);
    virtual void storeLazyFlagsOldCF(DynReg srcReg, bool doneWithSrcReg);
    virtual void storeEip(DynReg srcReg, bool doneWithSrcReg);    
    virtual void storeReg(U8 reg, DynWidth dstWidth, U32 imm);
    virtual void storeLazyFlagsSrc(DynWidth width, U32 imm);
    virtual void storeLazyFlags(const LazyFlags* lazyFlags);    

    virtual void storeRegFromMem(U8 reg, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult);
    virtual void storeLazyFlagsDstFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult);
    virtual void storeLazyFlagsSrcFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult);
    
    virtual void instCPUReg(char inst, U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg);
    virtual void instCPUImm(char inst, U32 dstOffset, DynWidth regWidth, U32 imm, DynReg tmpReg);
    virtual void instMemImm(char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg);
    virtual void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg);
    virtual void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg, DynReg tmpReg);
    virtual void instCPU(char inst, U32 dstOffset, DynWidth regWidth, DynReg tmpReg);

    virtual void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg, DynReg tmpReg);
    virtual void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg, DynReg tmpReg);
    virtual void setCPU(U32 offset, DynWidth regWidth, DynConditional condition);
    virtual void setMem(DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg, DynReg tmpReg);
    virtual void blockCall(DecodedOp* op);
    virtual void blockDone(bool returnEarly);
    virtual void blockDoneCall();
    virtual void incrementEip(U32 inc);

    virtual void blockNext1(DecodedOp* op);
    virtual void blockNext2(DecodedOp* op);
    virtual void blockDoneJump();
    virtual void blockExit() = 0;
    virtual void setConditional(DynConditional condition) = 0;
    virtual void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) = 0;
    virtual void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) = 0;
    virtual void instReg(char inst, DynReg reg, DynWidth regWidth) = 0;
    virtual void callHostFunction(void* address, bool hasReturn = false, U32 argCount = 0, U32 arg1 = 0, DynCallParamType arg1Type = DYN_PARAM_CONST_32, bool doneWithArg1 = true, U32 arg2 = 0, DynCallParamType arg2Type = DYN_PARAM_CONST_32, bool doneWithArg2 = true, U32 arg3 = 0, DynCallParamType arg3Type = DYN_PARAM_CONST_32, bool doneWithArg3 = true, U32 arg4 = 0, DynCallParamType arg4Type = DYN_PARAM_CONST_32, bool doneWithArg4 = true, U32 arg5 = 0, DynCallParamType arg5Type = DYN_PARAM_CONST_32, bool doneWithArg5 = true) = 0;
    virtual void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) = 0;
    virtual void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) = 0;
    virtual void movToReg(DynReg reg, DynWidth width, U32 imm) = 0;
    virtual void calculateEaa(DecodedOp* op, DynReg reg) = 0;
    virtual void pushValue(U32 arg, DynCallParamType argType) = 0;
    virtual void byteSwapReg32(DynReg reg) = 0;
    virtual void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg);
    virtual void zeroExtendReg16To32(DynReg dest, DynReg src) = 0;    
    virtual void JumpIf(DynReg reg, bool doneWithReg, U32 address) = 0;
    virtual void JumpIfNot(DynReg reg, bool doneWithReg, U32 address) = 0;
    virtual void JumpInBlock(U32 address) = 0;
    virtual void IfNot(DynReg reg, bool doneWithReg) = 0;
    virtual void If(DynReg reg, bool doneWithReg) = 0;
    virtual void IfPtrEqual(DynReg reg, DYN_PTR_SIZE value, bool doneWithReg) = 0;
    virtual void StartElse() = 0;
    virtual void EndIf() = 0;
    
    virtual void doJIT(U32 address, DecodedOp* op);

    // :TODO: make protected
    U32 cpuOffset(U32 r, DynWidth width);
protected:
    virtual void movToCpuFromCpu(U32 dstOffset, U32 srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg);
    virtual void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width) = 0;
    virtual void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) = 0;
    virtual void movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm) = 0;
    virtual void movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult);    
    virtual void movToMem(DynReg addressReg, DynWidth width, U32 value, DynCallParamType paramType, bool doneWithReg, bool doneWithAddressReg, DynReg tmp);
    virtual U32 getBufferSize() = 0;
    virtual U8* getBuffer() = 0;
    virtual U32 getIfJumpSize() = 0;                  
    virtual void IfLessThan(DynReg reg, U32 value, bool doneWithReg) = 0;
    virtual void IfBitSet(DynReg reg, U32 value, bool doneWithReg) = 0;

    U32 cpuOffsetResult(DynWidth width);
    U32 cpuOffsetDst(DynWidth width);
    U32 cpuOffsetSrc(DynWidth width);

    bool isParamTypeReg(DynCallParamType paramType);
    bool calculateLongestBlock(DecodedOp* op);
    void removeJIT(DecodedOp* op, U32 count);

    virtual void jmp(DynReg reg) = 0;
    virtual void readMem(DynReg reg, DynWidth width, DynReg address, U8 lsl , U32 disp ) = 0;    
    virtual void readMem(DynReg reg, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) = 0;
    virtual void writeMem(DynReg reg, DynWidth width, DynReg address, U32 disp) = 0;
    virtual void writeMem(U32 value, DynWidth width, DynReg address, U32 disp) = 0;
    virtual void writeMem(DynReg reg, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) = 0;
    virtual void writeMem(U32 value, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) = 0;
    virtual void and32(DynReg reg, U32 imm) = 0;
    virtual void shr32(DynReg reg, U32 imm) = 0;

    virtual void commitJIT(DecodedOp* op);
    virtual U8* createStartJITCode() = 0;
    virtual bool compileOps(DecodedOp* op);
    virtual void preCommitJIT() {}
    virtual U8* createDynamicExecutableMemory();
    virtual void patch(U8* begin) {}
};

void startNewJIT(CPU* cpu, U32 address, DecodedOp* op);

typedef void (*pfnDynamicOp)(DynamicData* data, DecodedOp* op);

#endif