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

#include "dynamicData.h"

// Implementation of JIT that is host instruction independent
class DynamicCodeGen : public DynamicData {
public:
    using OpFunction = void(DynamicData::*)(DecodedOp* op);
    using InstRegReg = void(DynamicData::*)(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg);
    using InstMemReg = void(DynamicData::*)(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg);
    using InstCPUReg = void(DynamicData::*)(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg);
    using InstCPUImm = void(DynamicData::*)(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg);
    using InstRegImm = void(DynamicData::*)(DynReg reg, DynWidth regWidth, U32 imm);
    using InstMemImm = void(DynamicData::*)(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg);

    DynamicCodeGen(CPU* cpu) : DynamicData(cpu) {}
    DecodedOp* firstOp = nullptr;

    BHashTable<U32, U32> eipToBufferPos;    
    U32 startingEip = 0;
    U32 lastOpEip = 0;
    U32 emulatedLen = 0;
    U32 blockOpCount = 0;
    
    // per instruction, not per block.  
    bool canJumpInBlock(DecodedOp* op) override {
        return currentEip < lastOpEip && currentEip + op->len + op->imm <= lastOpEip && currentEip + op->len + op->imm >= startingEip;
    }

    void loadRegStoreReg(U8 dst, U8 src, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) override;
    void loadRegStoreSrc(U8 reg, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) override;
    void loadRegStoreDst(U8 reg, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) override;
    void loadRegStoreEip(U8 reg, DynReg tmpReg, bool doneWithTmpReg) override;
    void loadSegValueStoreReg(U8 reg, U8 seg, DynReg tmpReg, bool doneWithTmpReg) override;
    DynReg loadReg(U8 reg, DynReg tmpReg, DynWidth width, bool copyIntoTmp = false) override;
    void loadSegAddress(U8 seg, DynReg reg)  override;
    void loadSegValue(U8 seg, DynReg reg)  override;
    void loadCPUFlags(DynReg reg)  override;
    void loadLazyFlagsResult(DynReg reg, DynWidth width)  override;
    void loadLazyFlagsSrc(DynReg reg, DynWidth width)  override;
    void loadLazyFlagsOldCF(DynReg reg)  override;
    void loadEip(DynReg reg)  override;
    void loadStackMask(DynReg reg)  override;
    void loadStackNotMask(DynReg reg)  override;
    void loadLazyFlags(DynReg reg)  override;
    void loadLazyFlagsDst(DynReg reg, DynWidth width)  override;
    void storeReg(U8 reg, DynReg srcReg, DynWidth width, bool doneWithSrcReg)  override;
    void storeLazyFlagsResult(DynReg srcReg, DynWidth width, bool doneWithSrcReg)  override;
    void storeLazyFlagsDst(DynReg srcReg, DynWidth width, bool doneWithSrcReg)  override;
    void storeLazyFlagsOldCF(DynReg srcReg, bool doneWithSrcReg)  override;
    void storeEip(DynReg srcReg, bool doneWithSrcReg)  override;    
    void storeReg(U8 reg, DynWidth dstWidth, U32 imm)  override;
    void storeLazyFlagsSrc(DynWidth width, U32 imm)  override;
    void storeLazyFlags(const LazyFlags* lazyFlags)  override;    

    void storeRegFromMem(U8 reg, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult)  override;
    void storeLazyFlagsDstFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult)  override;
    void storeLazyFlagsSrcFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult)  override;
        
    void xorCPUFlagsImm(U32 imm, DynReg tmpReg)  override;
    void andCPUFlagsImm(U32 imm, DynReg tmpReg)  override;
    void orCPUFlagsImm(U32 imm, DynReg tmpReg)  override;
    
    void negMem(DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg, DynReg tmpReg)  override;
    void notMem(DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg, DynReg tmpReg)  override;
    void negCPU(U8 regIndex, DynWidth regWidth, DynReg tmpReg)  override;
    void notCPU(U8 regIndex, DynWidth regWidth, DynReg tmpReg)  override;        

    void addMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) override;
    void orMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) override;
    void subMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) override;
    void andMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) override;  
    void xorMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) override;
    void shrMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) override;
    void sarMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) override;
    void shlMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) override;
  
    void addCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) override;
    void orCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) override;
    void subCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) override;
    void andCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) override;
    void xorCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) override;
    void shrCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) override;
    void sarCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) override;
    void shlCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) override;
  
    void addCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) override;
    void orCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) override;
    void subCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) override;
    void andCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) override;
    void xorCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) override;
    void shrCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) override;
    void sarCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) override;
    void shlCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) override;
  
    void addMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) override;
    void orMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) override;
    void subMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) override;
    void andMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) override;
    void xorMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) override;
    void shrMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) override;
    void sarMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) override;
    void shlMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) override;

    void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg, DynReg tmpReg) override;
    void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg, DynReg tmpReg) override;
    void setCPUReg(U8 regIndex, DynWidth regWidth, DynConditional condition) override;
    void setMem(DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg, DynReg tmpReg) override;
    void blockCall(DecodedOp* op) override;
    void blockDone(bool returnEarly) override;
    void blockDoneCall() override;
    void incrementEip(U32 inc) override;

    void blockNext1(DecodedOp* op) override;
    void blockNext2(DecodedOp* op) override;
    void blockDoneJump() override;
    void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg) override;
    
    void doJIT(U32 address, DecodedOp* op);
    void onTestEnd(DecodedOp* op) override;
protected:

    virtual void instMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg, InstRegReg pfnInstRegReg);
    virtual void instCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg, InstRegReg pfnInstRegReg);
    virtual void instCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg, InstRegImm pfnInstRegImm);
    virtual void instMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg, InstRegImm pfnInstRegImm);

    // don't use private methods in the dynamic op code, maybe in the future I can enforce this with an interface
    virtual void movToCpuFromCpu(U32 dstOffset, U32 srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg);
    virtual void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width) = 0;
    virtual void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) = 0;
    virtual void movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm) = 0;
    virtual void movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult);
    void arithMR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags, InstRegReg pfnInstRegReg, InstMemReg pfnInstMemReg);
    void arithRR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags, InstRegReg pfnInstRegReg, InstCPUReg pfnInstCpuReg);
    void arithRM(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags, InstRegReg pfnInstRegReg, InstCPUReg pfnInstCpuReg);
    void arithRI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags, InstRegReg pfnInstRegReg, InstRegImm pfnInstRegImm, InstCPUReg pfnInstCpuReg, InstCPUImm pfnInstCpuImm);
    void arithMI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags, InstRegReg pfnInstRegReg, InstRegImm pfnInstRegImm, InstMemReg pfnInstMemReg, InstMemImm pfnInstMemImm);

    virtual void movToMem(DynReg addressReg, DynWidth width, U32 value, DynCallParamType paramType, bool doneWithReg, bool doneWithAddressReg, DynReg tmp);
    virtual U32 getBufferSize() = 0;
    virtual U8* getBuffer() = 0;
    virtual U32 getIfJumpSize() = 0;                  
    virtual void IfLessThan(DynReg reg, U32 value, bool doneWithReg) = 0;
    virtual void IfBitSet(DynReg reg, U32 value, bool doneWithReg) = 0;

    U32 cpuOffsetResult(DynWidth width);
    U32 cpuOffsetDst(DynWidth width);
    U32 cpuOffsetSrc(DynWidth width);
    U32 cpuOffset(U32 r, DynWidth width);

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
    
    void dynamic_andMR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithMR(op, width, cf, store, flags, &DynamicData::andRegReg, &DynamicData::andMemReg);
    }
    void dynamic_subMR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithMR(op, width, cf, store, flags, &DynamicData::subRegReg, &DynamicData::subMemReg);
    }
    void dynamic_addMR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithMR(op, width, cf, store, flags, &DynamicData::addRegReg, &DynamicData::addMemReg);
    }
    void dynamic_orMR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithMR(op, width, cf, store, flags, &DynamicData::orRegReg, &DynamicData::orMemReg);
    }
    void dynamic_xorMR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithMR(op, width, cf, store, flags, &DynamicData::xorRegReg, &DynamicData::xorMemReg);
    }    
    void dynamic_andRR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRR(op, width, cf, store, flags, &DynamicData::andRegReg, &DynamicData::andCPUReg);
    }
    void dynamic_subRR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRR(op, width, cf, store, flags, &DynamicData::subRegReg, &DynamicData::subCPUReg);
    }
    void dynamic_addRR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRR(op, width, cf, store, flags, &DynamicData::addRegReg, &DynamicData::addCPUReg);
    }
    void dynamic_orRR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRR(op, width, cf, store, flags, &DynamicData::orRegReg, &DynamicData::orCPUReg);
    }
    void dynamic_xorRR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRR(op, width, cf, store, flags, &DynamicData::xorRegReg, &DynamicData::xorCPUReg);
    }    
    void dynamic_andRM(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRM(op, width, cf, store, flags, &DynamicData::andRegReg, &DynamicData::andCPUReg);
    }
    void dynamic_subRM(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRM(op, width, cf, store, flags, &DynamicData::subRegReg, &DynamicData::subCPUReg);
    }
    void dynamic_addRM(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRM(op, width, cf, store, flags, &DynamicData::addRegReg, &DynamicData::addCPUReg);
    }
    void dynamic_orRM(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRM(op, width, cf, store, flags, &DynamicData::orRegReg, &DynamicData::orCPUReg);
    }
    void dynamic_xorRM(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRM(op, width, cf, store, flags, &DynamicData::xorRegReg, &DynamicData::xorCPUReg);
    }    
    void dynamic_andRI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRI(op, width, cf, store, flags, &DynamicData::andRegReg, &DynamicData::andRegImm, &DynamicData::andCPUReg, &DynamicData::andCPUImm);
    }
    void dynamic_subRI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRI(op, width, cf, store, flags, &DynamicData::subRegReg, &DynamicData::subRegImm, &DynamicData::subCPUReg, &DynamicData::subCPUImm);
    }
    void dynamic_addRI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRI(op, width, cf, store, flags, &DynamicData::addRegReg, &DynamicData::addRegImm, &DynamicData::addCPUReg, &DynamicData::addCPUImm);
    }
    void dynamic_orRI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRI(op, width, cf, store, flags, &DynamicData::orRegReg, &DynamicData::orRegImm, &DynamicData::orCPUReg, &DynamicData::orCPUImm);
    }
    void dynamic_xorRI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithRI(op, width, cf, store, flags, &DynamicData::xorRegReg, &DynamicData::xorRegImm, &DynamicData::xorCPUReg, &DynamicData::xorCPUImm);
    }    
    void dynamic_andMI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithMI(op, width, cf, store, flags, &DynamicData::andRegReg, &DynamicData::andRegImm, &DynamicData::andMemReg, &DynamicData::andMemImm);
    }
    void dynamic_subMI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithMI(op, width, cf, store, flags, &DynamicData::subRegReg, &DynamicData::subRegImm, &DynamicData::subMemReg, &DynamicData::subMemImm);
    }
    void dynamic_addMI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithMI(op, width, cf, store, flags, &DynamicData::addRegReg, &DynamicData::addRegImm, &DynamicData::addMemReg, &DynamicData::addMemImm);
    }
    void dynamic_orMI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithMI(op, width, cf, store, flags, &DynamicData::orRegReg, &DynamicData::orRegImm, &DynamicData::orMemReg, &DynamicData::orMemImm);
    }
    void dynamic_xorMI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags) override {
        arithMI(op, width, cf, store, flags, &DynamicData::xorRegReg, &DynamicData::xorRegImm, &DynamicData::xorMemReg, &DynamicData::xorMemImm);
    }    

public:
    
};

void startNewJIT(CPU* cpu, U32 address, DecodedOp* op);

#endif