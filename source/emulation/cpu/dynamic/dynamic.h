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
    virtual RegPtr readCPU(DynWidth width, RegPtr sib, U8 lsl, U32 offset) = 0;
    virtual void writeCPU(DynWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr src) = 0;
    virtual void writeCPU(DynWidth width, U32 offset, RegPtr src) = 0;
    virtual void writeCPUValue(DynWidth width, RegPtr sib, U8 lsl, U32 offset, U32 src) = 0;
    virtual void writeCPUValue(DynWidth width, U32 offset, U32 src) = 0;
    RegPtr getDF() override;

    RegPtr calculateEaa(DecodedOp* op, U32 popEspAmount = 0) override; // :TODO: V2

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

    void storeLazyFlags(const LazyFlags* lazyFlags) override;
    void xorCPUFlagsImm(U32 imm) override;
    void andCPUFlagsImm(U32 imm) override;
    void orCPUFlagsImm(U32 imm) override;
    void orCPUFlagsReg(RegPtr reg);
  
    void blockCall(DecodedOp* op) override;
    void blockDone(bool returnEarly) override;
    void blockDoneCall() override;
    void incrementEip(U32 inc) override;

    void blockNext1(DecodedOp* op) override;
    void blockNext2(DecodedOp* op) override;
    void blockDoneJump() override;
    
    void doJIT(U32 address, DecodedOp* op);
    void onTestEnd(DecodedOp* op) override;
    U8* createJumpEip();
    void jumpToEipIfCached();
protected:
    virtual U32 getBufferSize() = 0;
    virtual U8* getBuffer() = 0;
    virtual U32 getIfJumpSize() = 0;                  

    bool isParamTypeReg(DynCallParamType paramType);
    bool calculateLongestBlock(DecodedOp* op);
    void removeJIT(DecodedOp* op, U32 count);
    void jumpEip();

    virtual void jmp(RegPtr reg) = 0;
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