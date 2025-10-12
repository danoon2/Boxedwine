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

#ifndef __DYNAMIC_DATA_H__
#define __DYNAMIC_DATA_H__

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

// API available to dynamic ops
class DynamicData {
public:
    DynamicData(CPU* cpu) : cpu(cpu) {}
    CPU* cpu = nullptr;
    const LazyFlags* currentLazyFlags = nullptr;;
    U32 currentEip = 0;

    // per instruction, not per block.  
    bool regUsed[4];
    virtual bool canJumpInBlock(DecodedOp* op) = 0;

    virtual void loadRegStoreReg(U8 dst, U8 src, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) = 0;
    virtual void loadRegStoreSrc(U8 reg, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) = 0;
    virtual void loadRegStoreDst(U8 reg, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) = 0;
    virtual void loadRegStoreEip(U8 reg, DynReg tmpReg, bool doneWithTmpReg) = 0;
    virtual void loadSegValueStoreReg(U8 reg, U8 seg, DynReg tmpReg, bool doneWithTmpReg) = 0;
    virtual DynReg loadReg(U8 reg, DynReg tmpReg, DynWidth width, bool copyIntoTmp = false) = 0;
    virtual void loadSegAddress(U8 seg, DynReg reg) = 0;
    virtual void loadSegValue(U8 seg, DynReg reg) = 0;
    virtual void loadCPUFlags(DynReg reg) = 0;
    virtual void loadLazyFlagsResult(DynReg reg, DynWidth width) = 0;
    virtual void loadLazyFlagsSrc(DynReg reg, DynWidth width) = 0;
    virtual void loadLazyFlagsOldCF(DynReg reg) = 0;
    virtual void loadEip(DynReg reg) = 0;
    virtual void loadStackMask(DynReg reg) = 0;
    virtual void loadStackNotMask(DynReg reg) = 0;
    virtual void loadLazyFlags(DynReg reg) = 0;
    virtual void loadLazyFlagsDst(DynReg reg, DynWidth width) = 0;
    virtual void storeReg(U8 reg, DynReg srcReg, DynWidth width, bool doneWithSrcReg) = 0;
    virtual void storeLazyFlagsResult(DynReg srcReg, DynWidth width, bool doneWithSrcReg) = 0;
    virtual void storeLazyFlagsDst(DynReg srcReg, DynWidth width, bool doneWithSrcReg) = 0;
    virtual void storeLazyFlagsOldCF(DynReg srcReg, bool doneWithSrcReg) = 0;
    virtual void storeEip(DynReg srcReg, bool doneWithSrcReg) = 0;
    virtual void storeReg(U8 reg, DynWidth dstWidth, U32 imm) = 0;
    virtual void storeLazyFlagsSrc(DynWidth width, U32 imm) = 0;
    virtual void storeLazyFlags(const LazyFlags* lazyFlags) = 0;

    virtual void storeRegFromMem(U8 reg, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) = 0;
    virtual void storeLazyFlagsDstFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) = 0;
    virtual void storeLazyFlagsSrcFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) = 0;

    virtual void xorCPUFlagsImm(U32 imm, DynReg tmpReg) = 0;
    virtual void andCPUFlagsImm(U32 imm, DynReg tmpReg) = 0;
    virtual void orCPUFlagsImm(U32 imm, DynReg tmpReg) = 0;

    virtual void negMem(DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg, DynReg tmpReg) = 0;
    virtual void notMem(DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg, DynReg tmpReg) = 0;
    virtual void negCPU(U8 regIndex, DynWidth regWidth, DynReg tmpReg) = 0;
    virtual void notCPU(U8 regIndex, DynWidth regWidth, DynReg tmpReg) = 0;
    virtual void negReg(DynReg reg, DynWidth regWidth) = 0;
    virtual void notReg(DynReg reg, DynWidth regWidth) = 0;

    virtual void addRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) = 0;
    virtual void orRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) = 0;
    virtual void subRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) = 0;
    virtual void andRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) = 0;
    virtual void xorRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) = 0;
    virtual void shrRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) = 0;
    virtual void sarRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) = 0;
    virtual void shlRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) = 0;

    virtual void addRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void orRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void subRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void andRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void xorRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void shrRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void sarRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void shlRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;

    virtual void addMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void orMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void subMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void andMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void xorMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void shrMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void sarMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void shlMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) = 0;

    virtual void addCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void orCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void subCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void andCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void xorCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void shrCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void sarCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) = 0;
    virtual void shlCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) = 0;

    virtual void addCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) = 0;
    virtual void orCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) = 0;
    virtual void subCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) = 0;
    virtual void andCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) = 0;
    virtual void xorCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) = 0;
    virtual void shrCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) = 0;
    virtual void sarCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) = 0;
    virtual void shlCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) = 0;

    virtual void addMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) = 0;
    virtual void orMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) = 0;
    virtual void subMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) = 0;
    virtual void andMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) = 0;
    virtual void xorMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) = 0;
    virtual void shrMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) = 0;
    virtual void sarMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) = 0;
    virtual void shlMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) = 0;

    virtual void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg, DynReg tmpReg) = 0;
    virtual void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg, DynReg tmpReg) = 0;
    virtual void setCPUReg(U8 regIndex, DynWidth regWidth, DynConditional condition) = 0;
    virtual void setMem(DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg, DynReg tmpReg) = 0;
    virtual void blockCall(DecodedOp* op) = 0;
    virtual void blockDone(bool returnEarly) = 0;
    virtual void blockDoneCall() = 0;
    virtual void incrementEip(U32 inc) = 0;

    virtual void blockNext1(DecodedOp* op) = 0;
    virtual void blockNext2(DecodedOp* op) = 0;
    virtual void blockDoneJump() = 0;
    virtual void blockExit() = 0;
    virtual void setConditional(DynConditional condition) = 0;
    virtual void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) = 0;
    virtual void callHostFunction(void* address, bool hasReturn = false, U32 argCount = 0, U32 arg1 = 0, DynCallParamType arg1Type = DYN_PARAM_CONST_32, bool doneWithArg1 = true, U32 arg2 = 0, DynCallParamType arg2Type = DYN_PARAM_CONST_32, bool doneWithArg2 = true, U32 arg3 = 0, DynCallParamType arg3Type = DYN_PARAM_CONST_32, bool doneWithArg3 = true, U32 arg4 = 0, DynCallParamType arg4Type = DYN_PARAM_CONST_32, bool doneWithArg4 = true, U32 arg5 = 0, DynCallParamType arg5Type = DYN_PARAM_CONST_32, bool doneWithArg5 = true) = 0;
    virtual void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) = 0;
    virtual void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) = 0;
    virtual void movToReg(DynReg reg, DynWidth width, U32 imm) = 0;
    virtual void calculateEaa(DecodedOp* op, DynReg reg) = 0;
    virtual void pushValue(U32 arg, DynCallParamType argType) = 0;
    virtual void byteSwapReg32(DynReg reg) = 0;
    virtual void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg) = 0;
    virtual void zeroExtendReg16To32(DynReg dest, DynReg src) = 0;
    virtual void JumpIf(DynReg reg, bool doneWithReg, U32 address) = 0;
    virtual void JumpIfNot(DynReg reg, bool doneWithReg, U32 address) = 0;
    virtual void JumpInBlock(U32 address) = 0;
    virtual void IfNot(DynReg reg, bool doneWithReg) = 0;
    virtual void If(DynReg reg, bool doneWithReg) = 0;
    virtual void IfPtrEqual(DynReg reg, DYN_PTR_SIZE value, bool doneWithReg) = 0;
    virtual void StartElse() = 0;
    virtual void EndIf() = 0;

    void genCF(const LazyFlags* flags, DynReg reg);
    void genOF(const LazyFlags* flags, DynReg reg);
    void genNZ(const LazyFlags* flags, DynReg reg);
    void genZ(const LazyFlags* flags, DynReg reg);
    void genS(const LazyFlags* flags, DynReg reg);
    bool getFlagInReg(DynConditional condition, DynReg reg);
    void getCondition(DynConditional condition, DynReg reg);
    void setConditionInReg(DynConditional condition, DynReg reg);
    void dynamic_pushReg32(DynReg reg, bool doneWithReg);
    void dynamic_pop32();
    void dynamic_fillFlags();
    void dynamic_getCF();
    void dynamic_jumpIfRegSet(DecodedOp* op, DynReg reg, bool doneWithReg);
    void dynamic_jumpIfRegNotSet(DecodedOp* op, DynReg reg, bool doneWithReg);
    void calculateMask16InDest(DecodedOp* op);
    void calculateMask32InDest(DecodedOp* op);
    void calculateEffectiveEaa16(DecodedOp* op);
    void calculateEffectiveEaa32(DecodedOp* op);

    virtual void dynamic_andMR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_subMR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_addMR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_orMR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_xorMR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_andRR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_subRR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_addRR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_orRR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_xorRR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_andRM(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_subRM(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_addRM(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_orRM(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_xorRM(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_andRI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_subRI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_addRI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_orRI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_xorRI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_andMI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_subMI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_addMI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_orMI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;
    virtual void dynamic_xorMI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) = 0;

    void dynamic_sidt(DecodedOp* op);
    void dynamic_callback(DecodedOp* op);
    void dynamic_invalid_op(DecodedOp* op);    
    void dynamic_onTestEnd(DecodedOp* op);
    virtual void onTestEnd(DecodedOp* op) = 0;

#define INIT_CPU(e, f) void dynamic_##f(DecodedOp* op);
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_sse.h"
#include "../common/cpu_init_sse2.h"
#include "../common/cpu_init_fpu.h"
#ifdef BOXEDWINE_MULTI_THREADED
#define INIT_CPU_LOCK(e, f) void dynamic_##f##_lock(DecodedOp* op);
#include "../common/cpu_init_lock.h"
#undef INIT_CPU_LOCK
#endif
#undef INIT_CPU
};

#endif