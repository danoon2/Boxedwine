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
    DynamicCodeGen(CPU* cpu) : DynamicData(cpu) {}

    // V2
    virtual void preOp(DecodedOp* op) {}
    virtual void postOp(DecodedOp* op) {}
    virtual void read(DynWidth width, RegPtr dest, RegPtr reg, U8 lsl, U32 disp) = 0;
    virtual void read(DynWidth width, RegPtr dest, RegPtr reg, RegPtr sib, U8 lsl, U32 disp) = 0;
    virtual void write(DynWidth width, RegPtr reg, U32 disp, RegPtr src) = 0;
    virtual void write(DynWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, RegPtr src) = 0;
    virtual void write(DynWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, U32 value) = 0;
    
    void readWriteMem(DynWidth width, RegPtr addressReg, std::function<void(RegPtr value)> prepareWrite, S8 hint = -1) override;
    RegPtr read(DynWidth width, RegPtr addressReg, std::function<void(RegPtr address, RegPtr offset)> customMemoryOp = nullptr, std::function<void()> failedMemoryOp = nullptr, bool isBigJump = false, RegPtr tmp = nullptr) override;
    void write(DynWidth width, RegPtr addressReg, RegPtr src, std::function<void(RegPtr address, RegPtr offset)> customMemoryOp = nullptr, std::function<void()> failedMemoryOp = nullptr, bool isBigJump = false) override;
    void writeValue(DynWidth width, RegPtr addressReg, U32 imm) override;
    virtual RegPtr readCPU(DynWidth width, U32 offset) = 0;

    RegPtr calculateEaa2(DecodedOp* op, U32 popEspAmount = 0) override; // :TODO: V2

    DecodedOp* firstOp = nullptr;

    BHashTable<U32, U32> eipToBufferPos;    
    U32 startingEip = 0;
    U32 lastOpEip = 0;
    U32 emulatedLen = 0;
    U32 blockOpCount = 0;
    
    // per instruction, not per block.  
    bool canJumpInBlock(DecodedOp* op) override {
        return currentEip + op->len + op->imm <= lastOpEip && currentEip + op->len + op->imm >= startingEip;
    }

    void loadReg(U8 reg, DynReg dstReg, DynWidth width) override;
    void loadSegAddress(U8 seg, DynReg reg) override;
    void loadCPUFlags(DynReg reg) override;
    void loadLazyFlagsResult(DynReg reg, DynWidth width) override;
    void loadLazyFlagsSrc(DynReg reg, DynWidth width) override;
    void loadLazyFlagsOldCF(DynReg reg) override;
    void loadEip(DynReg reg) override;
    void loadStackMask(DynReg reg) override;
    void loadStackNotMask(DynReg reg) override;
    void loadLazyFlags(DynReg reg) override;
    void loadLazyFlagsDst(DynReg reg, DynWidth width) override;
    void storeReg(U8 reg, DynReg srcReg, DynWidth width, bool doneWithSrcReg) override;
    void storeLazyFlagsResult(DynReg srcReg, DynWidth width, bool doneWithSrcReg) override;
    void storeLazyFlagsDst(DynReg srcReg, DynWidth width, bool doneWithSrcReg) override;
    void storeLazyFlagsSrc(DynReg srcReg, DynWidth width, bool doneWithSrcReg) override;
    void storeLazyFlagsOldCF(DynReg srcReg, bool doneWithSrcReg) override;
    void storeEip(DynReg srcReg, bool doneWithSrcReg) override;
    void storeReg(U8 reg, DynWidth dstWidth, U32 imm) override;
    void storeLazyFlagsSrc(DynWidth width, U32 imm) override;
    void storeLazyFlags(const LazyFlags* lazyFlags) override;

    void storeRegFromMem(U8 reg, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) override;
    void storeLazyFlagsDstFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) override;
    void storeLazyFlagsSrcFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) override;

    void xorCPUFlagsImm(U32 imm, DynReg tmpReg) override;
    void andCPUFlagsImm(U32 imm, DynReg tmpReg) override;
    void orCPUFlagsImm(U32 imm, DynReg tmpReg) override;
    void orCPUFlagsReg(DynReg reg, DynReg tmpReg, bool doneWithReg);
    void setCPUFlags(DynReg reg, U32 mask, DynReg tmpReg, bool doneWithReg) override;
  
    void blockCall(DecodedOp* op) override;
    void blockDone(bool returnEarly) override;
    void blockDoneCall() override;
    void incrementEip(U32 inc) override;

    void blockNext1(DecodedOp* op) override;
    void blockNext2(DecodedOp* op) override;
    void blockDoneJump() override;
    void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, std::function<void(DynReg address, DynReg offset)> customMemoryOp = nullptr, std::function<void()> failedMemoryOp = nullptr, bool bigJump = false) override;
    
    void doJIT(U32 address, DecodedOp* op);
    void onTestEnd(DecodedOp* op) override;
    U8* createJumpEip();
    void jumpToEipIfCached();
protected:
    virtual void movToRegFromCpu(DynReg reg, DynReg sib, U8 lsl, U32 srcOffset, DynWidth width) = 0;
    virtual void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width) = 0;
    virtual void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) = 0;    
    virtual void movToCpuFromReg(DynReg sib, U8 lsl, U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) = 0;
    virtual void movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm) = 0;
    virtual void movToCpu(DynReg sib, U8 lsl, U32 dstOffset, DynWidth dstWidth, U32 imm) = 0;
    virtual void movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult);

    virtual void movToMem(DynReg addressReg, DynWidth width, U32 value, DynCallParamType paramType, bool doneWithReg, bool doneWithAddressReg, DynReg tmp, std::function<void(DynReg address, DynReg offset)> customMemoryOp = nullptr, std::function<void()> failedMemoryOp = nullptr, bool bigJump = false);
    void readWriteMem(DynWidth width, DynReg addressReg, DynReg tmpReg, bool doneWithAddressReg, std::function<void()> prepareWrite) override;

    virtual U32 getBufferSize() = 0;
    virtual U8* getBuffer() = 0;
    virtual U32 getIfJumpSize() = 0;                  
    virtual void IfLessThan(DynReg reg, U32 value, bool doneWithReg) = 0;
    virtual void IfBitSet(DynReg reg, U32 value, bool doneWithReg, bool bigJump = false) = 0;

    U32 cpuOffsetResult(DynWidth width);
    U32 cpuOffsetDst(DynWidth width);
    U32 cpuOffsetSrc(DynWidth width);
   

    bool isParamTypeReg(DynCallParamType paramType);
    bool calculateLongestBlock(DecodedOp* op);
    void removeJIT(DecodedOp* op, U32 count);
    void jumpEip();

    virtual void jmp(DynReg reg) = 0;
    virtual void jmp2(RegPtr reg) = 0;
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
DynamicCodeGen* startNewJIT(CPU* cpu);
void writeBlockExitForJIT(U8* buffer);

#endif