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

    DYN_REG4 = 4,
    DYN_REG5 = 5,
    DYN_REG6 = 6,
    DYN_REG7 = 7,
    DYN_NOT_SET = 0xff
};

enum DynWidth {
    DYN_8bit = 0,
    DYN_16bit,
    DYN_32bit,
    DYN_64bit,
    DYN_128bit,
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
    DYN_PARAM_CPU_REG_8,
    DYN_PARAM_CPU_REG_16,
    DYN_PARAM_CPU_REG_32,
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
#define DYN_PTR DYN_32bit
// API available to dynamic ops

class DynamicData;

// the code will guarantee that for a single instruction, 2 DynReg's will never point to the same reg and both be read/write (see dynamic_xchgr8r8), that way we don't have to worry about clobbering
class DynReg2 {
public:
    DynReg2(U8 hardwareReg, U8 emulatedReg, std::function<U8()> delayedLoading = nullptr) : emulatedReg(emulatedReg), isHigh(false), reg(hardwareReg), delayedLoading(delayedLoading) {}
    DynReg2(U8 hardwareReg, U8 emulatedReg, bool isHigh, std::function<U8()> delayedLoading = nullptr) : emulatedReg(emulatedReg), isHigh(isHigh), reg(hardwareReg), delayedLoading(delayedLoading) {}
    
    U8 hardwareReg();
    bool isLoaded() { return reg != 0xff; }
    U8 emulatedReg;
    bool isHigh;

private:
    U8 reg;
    std::function<U8()> delayedLoading;
};

using RegPtr = std::shared_ptr<DynReg2>;

class DynamicData {
   
public:
    using OpFunction = void(DynamicData::*)(DecodedOp* op);
    using InstRegReg = void(DynamicData::*)(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg);
    using InstMemReg = void(DynamicData::*)(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg);
    using InstCPUReg = void(DynamicData::*)(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg);
    using InstCPUImm = void(DynamicData::*)(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg);
    using InstRegImm = void(DynamicData::*)(DynReg reg, DynWidth regWidth, U32 imm);
    using InstMemImm = void(DynamicData::*)(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg);

    DynamicData(CPU* cpu) : cpu(cpu) {}
    CPU* cpu = nullptr;
    const LazyFlags* currentLazyFlags = nullptr;;
    U32 currentEip = 0;

    // per instruction, not per block.  
    bool regUsed[32]; // host regs might index into this, only the first 4 are important
    virtual bool canJumpInBlock(DecodedOp* op) = 0;

    // V2 API
    // It would be nice to pass RegPtr around with const&, but sometimes we don't want the caller to keep a reference if its done, that way the function
    // being called can null it out if its done and the temp reg can be re-used
    using InstRegRegImm = void(DynamicData::*)(DynWidth regWidth, RegPtr reg, RegPtr rm, U32 imm, bool checkFlags);
    using InstRegRegCl = void(DynamicData::*)(DynWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl, bool checkFlags);
    using InstRegReg2 = void(DynamicData::*)(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags);
    using InstRegImm2 = void(DynamicData::*)(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags);
    using InstReg = void(DynamicData::*)(DynWidth regWidth, RegPtr reg, bool checkFlags);

    virtual RegPtr getReg(U8 reg, S8 hint = -1) = 0; // for cached regs, they will be used directly, for unchached regs, it will be read from the cpu and written back when done
    virtual RegPtr getReg8(U8 reg) = 0;
    virtual RegPtr getReadOnlyReg(U8 reg, bool delayed = false, S8 hint = -1) = 0; // for cached regs, they will be used directly, for unchached regs, it will be read from the cpu but NOT written back when done
    virtual RegPtr getReadOnlyReg8(U8 reg, bool delayed = false, S8 hint = -1) = 0;
    virtual RegPtr getTmpReg() = 0;
    virtual RegPtr getTmpRegWithHint(S8 hint) = 0;
    virtual RegPtr getTmpRegForCallResult() = 0; // just a hint to try and get the same reg used for a call result in order to prevent an extra mov
    virtual RegPtr getTmpReg(U8 reg, bool delayed = false, S8 hint = -1) = 0; // a reg that doesn't represent an emulated reg, but come pre-loaded with the emulated reg's current value
    virtual RegPtr getTmpReg8(U8 reg, bool delayed = false, S8 hint = -1) = 0;
    virtual RegPtr getSegAddress(U8 reg) = 0;
    virtual RegPtr getSegValue(U8 reg) = 0;

    virtual RegPtr calculateEaa2(DecodedOp* op, bool popEsp = false) = 0; // :TODO: V2
    virtual void readWriteMem(DynWidth width, RegPtr addressReg, std::function<void(RegPtr value)> prepareWrite, S8 hint = -1) = 0;
    virtual RegPtr read(DynWidth width, RegPtr addressReg, std::function<void(RegPtr address, RegPtr offset)> customMemoryOp = nullptr, std::function<void()> failedMemoryOp = nullptr, bool isBigJump = false, RegPtr tmp = nullptr) = 0;
    virtual void write(DynWidth width, RegPtr addressReg, RegPtr src, std::function<void(RegPtr address, RegPtr offset)> customMemoryOp = nullptr, std::function<void()> failedMemoryOp = nullptr, bool isBigJump = false) = 0;
    virtual void writeValue(DynWidth width, RegPtr addressReg, U32 imm) = 0;

    virtual void IfLessThan2(DynWidth regWidth, RegPtr reg, U32 value) = 0;
    virtual void IfNot(DynWidth regWidth, RegPtr reg) = 0;
    virtual void If(DynWidth regWidth, RegPtr reg) = 0;
    virtual void IfEqual(DynWidth regWidth, RegPtr reg, U32 value) = 0;
    virtual void IfBitSet2(DynWidth regWidth, RegPtr reg, U32 value, bool bigJump = false) = 0;
    virtual void IfCondition(DynConditional condition) = 0;
    virtual void JumpIfCondition(DynConditional condition, U32 address) = 0;
    virtual void setReg(DynConditional condition, RegPtr reg) = 0;

    virtual void mov(DynWidth regWidth, RegPtr dest, RegPtr src) = 0;
    virtual void mov8(RegPtr dest, bool isDestHigh, RegPtr src, bool isSrcHigh) = 0;
    virtual void movzx(DynWidth dstWidth, RegPtr dest, DynWidth srcWidth, RegPtr src) = 0;
    virtual void movsx(DynWidth dstWidth, RegPtr dest, DynWidth srcWidth, RegPtr src) = 0;
    virtual void movValue(DynWidth regWidth, RegPtr dst, U32 imm) = 0;
    

    virtual void addReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void addValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) = 0;
    virtual void adcReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void adcValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) = 0;
    virtual void orReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void orValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) = 0;
    virtual void subReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void subValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) = 0;
    virtual void sbbReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void sbbValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) = 0;
    virtual void andReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void andValue(DynWidth regWidth, RegPtr reg, U32 immm, bool checkFlags) = 0;
    virtual void xorReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void xorValue(DynWidth regWidth, RegPtr reg, U32 immm, bool checkFlags) = 0;
    virtual void cmpReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void cmpValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) = 0;
    virtual void testReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void testValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) = 0;
    virtual void shrReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void shrValue(DynWidth regWidth, RegPtr reg, U32 immm, bool checkFlags) = 0;
    virtual void shrdReg(DynWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl, bool checkFlags) = 0;
    virtual void shrdValue(DynWidth regWidth, RegPtr reg, RegPtr rm, U32 imm, bool checkFlags) = 0;
    virtual void shlReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void shlValue(DynWidth regWidth, RegPtr reg, U32 immm, bool checkFlags) = 0;
    virtual void shldReg(DynWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl, bool checkFlags) = 0;
    virtual void shldValue(DynWidth regWidth, RegPtr reg, RegPtr rm, U32 imm, bool checkFlags) = 0;
    virtual void sarReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void sarValue(DynWidth regWidth, RegPtr reg, U32 immm, bool checkFlags) = 0;
    virtual void negReg2(DynWidth regWidth, RegPtr reg, bool checkFlags) = 0;
    virtual void notReg2(DynWidth regWidth, RegPtr reg, bool checkFlags) = 0;
    virtual void mulReg(DynWidth regWidth, RegPtr reg, bool checkFlags) = 0;
    virtual void imulReg(DynWidth regWidth, RegPtr reg, bool checkFlags) = 0;
    virtual void imulRRI(DynWidth regWidth, RegPtr dst, RegPtr src, U32 src2, bool checkFlags) = 0;
    virtual void imulRR(DynWidth regWidth, RegPtr dst, RegPtr src, bool checkFlags) = 0;
    virtual void divRegRegWithRemainder2(DynWidth regWidth, RegPtr dest, RegPtr src, RegPtr remainder) = 0; // src should be checked for 0 before calling
    virtual void idivRegRegWithRemainder2(DynWidth regWidth, RegPtr dest, RegPtr src, RegPtr remainder) = 0; // src should be checked for 0 before calling
    virtual void incReg(DynWidth regWidth, RegPtr dest, bool checkFlags) = 0;
    virtual void decReg(DynWidth regWidth, RegPtr dest, bool checkFlags) = 0;
    virtual void xchgReg(DynWidth regWidth, RegPtr dest, RegPtr src) = 0;
    virtual void rolReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void rolValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) = 0;
    virtual void rorReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void rorValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) = 0;
    virtual void rclReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void rclValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) = 0;
    virtual void rcrReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) = 0;
    virtual void rcrValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) = 0;

    virtual void loadRegStoreReg(U8 dst, U8 src, DynWidth width, DynReg tmpReg) = 0;
    virtual void loadRegStoreSrc(U8 reg, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) = 0;
    virtual void loadRegStoreDst(U8 reg, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) = 0;
    virtual void loadRegStoreEip(U8 reg, DynReg tmpReg) = 0;
    virtual void loadSegValueStoreReg(U8 reg, U8 seg, DynReg tmpReg) = 0;
    virtual void loadReg(U8 reg, DynReg tmpReg, DynWidth width) = 0;
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
    virtual void storeLazyFlagsSrc(DynReg srcReg, DynWidth width, bool doneWithSrcReg) = 0;
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
    virtual void setCPUFlags(DynReg reg, U32 mask, DynReg tmpReg, bool doneWithReg) = 0;

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
    virtual void rolRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) = 0;
    virtual void rorRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) = 0;
    virtual void imulRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) = 0;
    virtual void imulRegReg64(DynReg high64, DynReg dst, DynReg src, bool doneWithSrcReg) = 0;
    virtual void mulRegReg64(DynReg high64, DynReg dst, DynReg src, bool doneWithSrcReg) = 0;
    virtual void divRegRegWithRemainder(DynReg dest, DynReg src, DynReg remainder, DynWidth width) = 0; // src should be checked for 0 before calling
    virtual void idivRegRegWithRemainder(DynReg dest, DynReg src, DynReg remainder, DynWidth width) = 0; // src should be checked for 0 before calling

    virtual void addRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void orRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void subRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void andRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void xorRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void shrRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void sarRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void shlRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void rolRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void rorRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;
    virtual void imulRegImm(DynReg reg, DynWidth regWidth, U32 imm) = 0;

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
    virtual void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, std::function<void(DynReg address, DynReg offset)> customMemoryOp = nullptr, std::function<void()> failedMemoryOp = nullptr, bool bigJump = false) = 0;
    virtual void readWriteMem(DynWidth width, DynReg addressReg, DynReg tmpReg, bool doneWithAddressReg, std::function<void()> prepareWrite) = 0;

    virtual void zeroExtendReg16To32(DynReg dest, DynReg src) = 0;
    virtual void JumpIf(DynReg reg, bool doneWithReg, U32 address) = 0;
    virtual void JumpIfNot(DynReg reg, bool doneWithReg, U32 address) = 0;
    virtual void JumpInBlock(U32 address) = 0;
    virtual void IfNot(DynReg reg, bool doneWithReg) = 0;
    virtual void If(DynReg reg, bool doneWithReg, bool bigJump = false) = 0;
    virtual void IfPtrEqual(DynReg reg, DYN_PTR_SIZE value, bool doneWithReg) = 0;
    virtual void StartElse(bool bigJump = false) = 0;
    virtual void EndIf(bool bigJump = false) = 0;

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
    void dynamic_jump(DecodedOp* op, DynConditional condition);
    RegPtr calculateMask16(DecodedOp* op);
    RegPtr calculateMask32(DecodedOp* op);
    RegPtr calculateEffectiveEaa16(DecodedOp* op);
    RegPtr calculateEffectiveEaa32(DecodedOp* op);

    void dynamic_RR(DecodedOp* op, DynWidth width, InstRegReg2 callback, bool writeback = true) {
        if (writeback) {
            if (width == DYN_8bit) {
                (this->*callback)(width, getReg8(op->reg), getReadOnlyReg8(op->rm), true);
            } else {
                (this->*callback)(width, getReg(op->reg), getReadOnlyReg(op->rm), true);
            }
        } else {
            if (width == DYN_8bit) {
                (this->*callback)(width, getReadOnlyReg8(op->reg), getReadOnlyReg8(op->rm), true);
            } else {
                (this->*callback)(width, getReadOnlyReg(op->reg), getReadOnlyReg(op->rm), true);
            }
        }
        incrementEip(op->len);
    }

    void dynamic_RRI(DecodedOp* op, DynWidth width, InstRegRegImm callback) {
        if (width == DYN_8bit) {
            (this->*callback)(width, getReg8(op->reg), getReadOnlyReg8(op->rm), op->imm, true);
        } else {
            (this->*callback)(width, getReg(op->reg), getReadOnlyReg(op->rm), op->imm, true);
        }
        incrementEip(op->len);
    }

    void dynamic_RR_Cl(DecodedOp* op, DynWidth width, InstRegRegCl callback) {
        if (width == DYN_8bit) {
            (this->*callback)(width, getReg8(op->reg), getReadOnlyReg(op->rm), getReadOnlyReg8(1, true, 1), true);
        } else {
            (this->*callback)(width, getReg(op->reg), getReadOnlyReg(op->rm), getReadOnlyReg(1, true, 1), true);
        }
        incrementEip(op->len);
    }

    void dynamic_R_Cl(DecodedOp* op, DynWidth width, InstRegReg2 callback) {
        if (width == DYN_8bit) {
            (this->*callback)(width, getReg8(op->reg), getReadOnlyReg8(1, false, 1), true);
        } else {
            (this->*callback)(width, getReg(op->reg), getReadOnlyReg(1, false, 1), true);
        }
        incrementEip(op->len);
    }

    void dynamic_MR(DecodedOp* op, DynWidth width, InstRegReg2 callback, bool writeback = true) {
        if (writeback) {
            readWriteMem(width, calculateEaa2(op), [op, width, callback, this](RegPtr value) {
                // don't create a local variable for getReg, we don't want to hold a reference, this
                // way the function can null out its copy when its done and reclaim a tmp reg
                //
                // delayed reg loading is so that ADC/SBB have a tmp reg available to load CF before doing calculation on x86
                if (width == DYN_8bit) {
                    (this->*callback)(width, value, getReadOnlyReg8(op->reg, true), true);
                } else {
                    (this->*callback)(width, value, getReadOnlyReg(op->reg, true), true);
                }
            });
        } else {
            if (width == DYN_8bit) {
                (this->*callback)(width, read(width, calculateEaa2(op)) , getReadOnlyReg8(op->reg, true), true);
            } else {
                (this->*callback)(width, read(width, calculateEaa2(op)), getReadOnlyReg(op->reg, true), true);
            }            
        }
        incrementEip(op->len);
    }

    void dynamic_MRI(DecodedOp* op, DynWidth width, InstRegRegImm callback) {
        readWriteMem(width, calculateEaa2(op), [op, width, callback, this](RegPtr value) {
            if (width == DYN_8bit) {
                (this->*callback)(width, value, getReadOnlyReg8(op->reg), op->imm, true);
            } else {
                (this->*callback)(width, value, getReadOnlyReg(op->reg), op->imm, true);
            }
        });
        incrementEip(op->len);
    }

    void dynamic_M_Cl(DecodedOp* op, DynWidth width, InstRegReg2 callback) {
        readWriteMem(width, calculateEaa2(op), [op, width, callback, this](RegPtr value) {
            if (width == DYN_8bit) {
                (this->*callback)(width, value, getReadOnlyReg8(1, true, 1), true);
            } else {
                (this->*callback)(width, value, getReadOnlyReg(1, true, 1), true);
            }
            });
        incrementEip(op->len);
    }

    void dynamic_MR_Cl(DecodedOp* op, DynWidth width, InstRegRegCl callback) {
        readWriteMem(width, calculateEaa2(op), [op, width, callback, this](RegPtr value) {
            // cl is delay loaded because shldReg/shrdReg on x86 JIT will check if hardware ECX is in use and if so then push/pop it
            if (width == DYN_8bit) {
                (this->*callback)(width, value, getReadOnlyReg8(op->reg), getReadOnlyReg8(1, true, 1), true);
            } else {
                (this->*callback)(width, value, getReadOnlyReg(op->reg), getReadOnlyReg(1, true, 1), true);
            }
            }, 1); // hint to use ECX for address reg so that x86 JIT can push/pop it for the delayed load of CL
        incrementEip(op->len);
    }

    void dynamic_RM(DecodedOp* op, DynWidth width, InstRegReg2 callback, bool writeback = true) {
        if (writeback) {
            if (width == DYN_8bit) {
                (this->*callback)(width, getReg8(op->reg), read(width, calculateEaa2(op)), true);
            } else {
                (this->*callback)(width, getReg(op->reg), read(width, calculateEaa2(op)), true);
            }
        } else {
            if (width == DYN_8bit) {
                (this->*callback)(width, getReadOnlyReg8(op->reg), read(width, calculateEaa2(op)), true);
            } else {
                (this->*callback)(width, getReadOnlyReg(op->reg), read(width, calculateEaa2(op)), true);
            }
        }
        incrementEip(op->len);
    }

    void dynamic_RI(DecodedOp* op, DynWidth width, InstRegImm2 callback, bool writeback = true) {
        if (writeback) {
            if (width == DYN_8bit) {
                (this->*callback)(width, getReg8(op->reg), op->imm, true);
            } else {
                (this->*callback)(width, getReg(op->reg), op->imm, true);
            }
        } else {
            if (width == DYN_8bit) {
                (this->*callback)(width, getReadOnlyReg8(op->reg), op->imm, true);
            } else {
                (this->*callback)(width, getReadOnlyReg(op->reg), op->imm, true);
            }
        }
        incrementEip(op->len);
    }

    void dynamic_MI(DecodedOp* op, DynWidth width, InstRegImm2 callback, bool writeback = true) {
        if (writeback) {
            readWriteMem(width, calculateEaa2(op), [op, width, callback, this](RegPtr value) {
                (this->*callback)(width, value, op->imm, true);
            });
        } else {
            (this->*callback)(width, read(width, calculateEaa2(op)), op->imm, true);
        }
        incrementEip(op->len);
    }

    void dynamic_R(DecodedOp* op, DynWidth width, InstReg callback, bool writeback = true) {
        if (writeback) {
            if (width == DYN_8bit) {
                (this->*callback)(width, getReg8(op->reg), true);
            } else {
                (this->*callback)(width, getReg(op->reg), true);
            }
        } else {
            if (width == DYN_8bit) {
                (this->*callback)(width, getReadOnlyReg8(op->reg), true);
            } else {
                (this->*callback)(width, getReadOnlyReg(op->reg), true);
            }
        }
        incrementEip(op->len);
    }

    void dynamic_M(DecodedOp* op, DynWidth width, InstReg callback, bool writeback = true, RegPtr tmp = nullptr) {
        if (writeback) {
            readWriteMem(width, calculateEaa2(op), [op, width, callback, this](RegPtr value) {
                (this->*callback)(width, value, true);
                });
        } else {
            (this->*callback)(width, read(width, calculateEaa2(op), nullptr, nullptr, false, tmp), true);
        }
        incrementEip(op->len);
    }

    class DynParam {
    public:
        DynParam(DynCallParamType type) : type(type), value(0) {}
        DynParam(DynCallParamType type, U32 value) : type(type), value(value) {}
        DynParam(DynCallParamType type, RegPtr reg) : type(type), value(0), reg(reg) {}

        DynCallParamType type;
        U32 value;
        const RegPtr reg;
    };
    virtual void callHostFunction(void* address, const std::vector<DynParam>& params) = 0;
    virtual void callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) = 0;

    RegPtr callAndReturn(void* address, DynWidth width, RegPtr reg, RegPtr resultReg = nullptr);
    void call(void* address, DynWidth width, RegPtr reg, DynWidth width2, RegPtr reg2);
    using CallII = void(*)(CPU* cpu, U32 value1, U32 value2);
    void call(CallII address, U32 value1, U32 value2);
    using CallRI = void(*)(CPU* cpu, U32 value1, U32 value2);
    void call(CallRI address, DynWidth width, RegPtr reg, U32 value);
    using CallReturnIR = U32(*)(CPU* cpu, U32 value1, U32 value2);
    RegPtr callAndReturn(CallReturnIR address, U32 value, DynWidth width, RegPtr reg);
    using CallReturnII = U32(*)(CPU* cpu, U32 value1, U32 value2);
    RegPtr callAndReturn(CallReturnII address, U32 value1, U32 value2);
    using CallR = void(*)(CPU* cpu, U32 value);
    void call(CallR address, DynWidth width, RegPtr reg);
    using CallNoArgs = void(*)(CPU* cpu);
    void call(CallNoArgs address);
    void pushParam(std::vector<DynParam>& params, DynWidth width, RegPtr reg);

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

#define INIT_CPU(e, f) virtual void dynamic_##f(DecodedOp* op);
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_sse.h"
#include "../common/cpu_init_sse2.h"
#include "../common/cpu_init_fpu.h"
#ifdef BOXEDWINE_MULTI_THREADED
#define INIT_CPU_LOCK(e, f) virtual void dynamic_##f##_lock(DecodedOp* op);
#include "../common/cpu_init_lock.h"
#undef INIT_CPU_LOCK
#endif
#undef INIT_CPU

protected:
    using InstDiv = void(DynamicData::*)(DynReg dest, DynReg src, DynReg remainder, DynWidth width);
    using InstDiv2 = void(DynamicData::*)(DynWidth width, RegPtr dest, RegPtr src, RegPtr remainder);
    void div8(DecodedOp* op, RegPtr src, bool isSigned, InstDiv2 callback);
    void div16(DecodedOp* op, RegPtr src, bool isSigned, InstDiv2 callback);
    void div32(DecodedOp* op, RegPtr src, InstDiv2 callback, std::function<void()> fallback);

private:
    void dynamic_cmov_R(DynWidth width, DecodedOp* op, DynConditional condition);
    void dynamic_cmov_M(DynWidth width, DecodedOp* op, DynConditional condition);
    void dynamic_set_R(DecodedOp* op, DynConditional condition);
    void dynamic_set_M(DecodedOp* op, DynConditional condition);
};

#endif