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

#ifndef __JIT_CODE_GEN_H__
#define __JIT_CODE_GEN_H__

#include "jit.h"

// Implementation of JIT that is host instruction independent
class JitCodeGen : public Jit {
public:    
    JitCodeGen(CPU* cpu) : Jit(cpu) {}
    virtual ~JitCodeGen() {}
    DecodedOp* firstOp = nullptr;

    const U32 SKIPPED_OP = 0xffffffff;
    BHashTable<U32, U32> eipToBufferPos;    
    U32 startingEip = 0;
    U32 lastOpEip = 0;
    U32 emulatedLen = 0;
    U32 blockOpCount = 0;
    
    // per instruction, not per block.  
    bool canJumpInBlock(DecodedOp* op) override {
        return currentEip < lastOpEip && currentEip + op->len + op->imm <= lastOpEip && currentEip + op->len + op->imm >= startingEip;
    }

    bool canJumpInBlock(U32 opEip, DecodedOp* op) override {
        return opEip < lastOpEip && opEip + op->len + op->imm <= lastOpEip && opEip + op->len + op->imm >= startingEip;
    }

    void preCompile(DecodedOp* op, bool skippedOp = false) override;
    void compile(DecodedOp* op) override;
    void postCompile(DecodedOp* op) override;
    void tryDirect(DecodedOp* op, std::function<void()> callback, std::function<void()> fallback) override;
    virtual bool supportsDirectCondition(JitConditional condition) { return true; }

    JitConditional getJumpConditionFromOp(DecodedOp* op);
    JitConditional getCmovConditionFromOp(DecodedOp* op);
    JitConditional getSetConditionFromOp(DecodedOp* op);

    virtual void preOp(DecodedOp* op) {}
    virtual void onBlockPreCommit(DecodedOp* op) {}
    virtual void readMMU(RegPtr dest, RegPtr index) = 0;
    virtual void readMMU(RegPtr dest, U32 index) = 0;

    virtual RegPtr readCPU(JitWidth width, U32 offset, RegPtr resultReg = nullptr) = 0;
    virtual RegPtr readCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr resultReg = nullptr) = 0;
    virtual void writeCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr src) = 0;
    virtual void writeCPU(JitWidth width, U32 offset, RegPtr src) = 0;
    virtual void writeCPUValue(JitWidth width, RegPtr sib, U8 lsl, U32 offset, DYN_PTR_SIZE src) = 0;
    virtual void writeCPUValue(JitWidth width, U32 offset, DYN_PTR_SIZE src) = 0;

    RegPtr readWriteMem(JitWidth width, RegPtr addressReg, std::function<void(RegPtr value)> prepareWrite, S8 hint = -1) override;
    RegPtr read(JitWidth width, RegPtr addressReg, std::function<void(RegPtr address, RegPtr offset)> customMemoryOp = nullptr, std::function<void()> failedMemoryOp = nullptr, RegPtr tmp = nullptr, bool checkAlignment = true) override;
    void write(JitWidth width, RegPtr addressReg, RegPtr src, std::function<void(RegPtr address, RegPtr offset)> customMemoryOp = nullptr, std::function<void()> failedMemoryOp = nullptr, bool checkAlignment = true) override;

    RegPtr read(JitWidth width, MemPtr mem, RegPtr result = nullptr) override;
    void write(JitWidth width, MemPtr mem, RegPtr src) override;
    void write(JitWidth width, MemPtr mem, U32 imm) override;

#ifdef BOXEDWINE_MEM_CACHE
    virtual void writeMemCache(JitWidth width, RegPtr addressReg, RegPtr src) = 0;
    virtual void writeMemCache(JitWidth width, RegPtr addressReg, U32 value) = 0;
    virtual void writeMemCache(JitWidth width, U32 mem, RegPtr src) = 0;
    virtual void writeMemCache(JitWidth width, U32 mem, U32 imm) = 0;
    virtual RegPtr readMemCache(JitWidth width, RegPtr addressReg, RegPtr tmp = nullptr) = 0;
    virtual RegPtr readMemCache(JitWidth width, U32 mem, RegPtr result = nullptr) = 0;    
    virtual RegPtr readWriteMemCache(JitWidth width, RegPtr addressReg, std::function<void(RegPtr value)> prepareWrite, RegPtr tmp) = 0;
#endif

    virtual void readHost(JitWidth width, MemPtr mem, RegPtr result, bool emlulatedMemory = true) = 0;
    virtual void writeHost(JitWidth width, MemPtr mem, RegPtr src, bool emlulatedMemory = true) = 0;
    virtual void writeHost(JitWidth width, MemPtr mem, U32 imm, bool emlulatedMemory = true) = 0;
    virtual RegPtr calculateAddress(MemPtr mem, JitWidth width = JitWidth::b32);

    virtual void clearMMUPermissionIfSpansPage(JitWidth width, RegPtr offset, RegPtr reg) = 0;

    void addRegWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, RegPtr rm) override;
    void andValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) override;
    void shlValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) override;
    void shrValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) override;
    void sarValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 immm) override;
    void subValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 imm) override;
    void addValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 imm) override;

    void genCF(LazyFlagType flags, RegPtr result); // guaranteed to return 0 or 1
    void genOF(LazyFlagType flags, RegPtr result); // guaranteed to return 0 or 1
    void genPF(RegPtr result); // guaranteed to return 0 or 1
    JitWidth getWidthOfFlags(LazyFlagType flags);

    RegPtr getZF() override;
    RegPtr getCF() override;
    void fillFlags(U32 flags = PF | SF | AF | CF | OF | ZF) override;
    virtual RegPtr getFlagDestReadOnly(RegPtr result = nullptr); // passed in result might be returned, but not guaranteed, its available just to help minimize use of temp registers
    virtual RegPtr getFlagSrcReadOnly(RegPtr result = nullptr); // passed in result might be returned, but not guaranteed, its available just to help minimize use of temp registers
    virtual RegPtr getFlagResultReadOnly(RegPtr result = nullptr); // passed in result might be returned, but not guaranteed, its available just to help minimize use of temp registers
    virtual RegPtr getFlagDestTmp(RegPtr result = nullptr); // guaranteed to return result in result
    virtual RegPtr getFlagSrcTmp(RegPtr result = nullptr); // guaranteed to return result in result
    virtual RegPtr getFlagResultTmp(RegPtr result = nullptr); // guaranteed to return result in result
    virtual RegPtr getFlagsInTmp(RegPtr reg = nullptr) override;
    virtual RegPtr getFlagCF(RegPtr result = nullptr);
    virtual RegPtr getLazyFlagType();
    virtual RegPtr getLazyFlagTypeInTmp();
    void storeLazyFlagType(LazyFlagType flags) override;
    void storeLazyFlagsDest(RegPtr reg) override;
    void storeLazyFlagsSrc(RegPtr reg) override;
    void storeLazyFlagsSrc(U32 value) override;
    void storeLazyFlagsResult(RegPtr reg) override;
    void storeLazyFlagsOldCF(RegPtr reg) override;
    void writeFlags(RegPtr flags) override;
    RegPtr getCondition(JitConditional condition, RegPtr resultReg = nullptr) override; // guaranteed to return 0 or 1
    virtual RegPtr getConditionCalculationReg(U32 index = 0) = 0;
    void IfCondition(JitConditional condition) override;
    void preIfCondition(JitConditional condition, bool& negative, RegPtr& reg);
    void IfDF() override;
    void IfSmallStack() override;

    void orCPUFlags(RegPtr reg) override;
    void xorCPUFlagsImmV2(U32 imm) override;
    void andCPUFlagsImmV2(U32 imm) override;
    void orCPUFlagsImmV2(U32 imm) override;

    void blockNext1(U32 eip, DecodedOp* op) override;
    void blockNext2(U32 eip, DecodedOp* op) override;
    void jumpEip(RegPtr reg) override;

    void doJIT(U32 address, DecodedOp* op);
    void onTestEnd(DecodedOp* op) override;
    void jumpToEipIfCached(RegPtr eip);
    U8* createEmulateSingleOp();    
    U8* createCalculationCF(LazyFlagType lazyFlagType);
    void getCF(LazyFlagType lazyFlagType, RegPtr result);
    RegPtr getStringRegEcx() override;
    RegPtr getStringRegEsi() override;
    RegPtr getStringRegEdi() override;

    void incReg(JitWidth regWidth, RegPtr dest) override;
    void decReg(JitWidth regWidth, RegPtr dest) override;
    void xaddReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
protected:

    virtual U32 getBufferSize() = 0;
    virtual U32 markBufferLocation() = 0;
    virtual U32 getBufferLocation(U32 id) = 0;
    virtual void copyBuffer(U8* dst, U32 size) = 0;
    virtual U32 getIfJumpSize() = 0;                     
    virtual U8* createSyncToHost() = 0;
    virtual U8* createSyncFromHost() = 0;
    virtual U8* createBlockExit(bool syncRegs) = 0;
#if defined(BOXEDWINE_POSIX) && defined(BOXEDWINE_HOST_EXCEPTIONS)
    virtual U8* createSignalHandler();
#endif
    bool isParamTypeReg(JitCallParamType paramType);
    bool calculateLongestBlock(DecodedOp* op);
    void removeJIT(DecodedOp* op, U32 count);

    virtual void nakedCall(RegPtr reg) = 0;
    virtual void nakedReturn() = 0;

    virtual void commitJIT(DecodedOp* op);
    virtual U8* createStartJITCode() = 0;
    virtual bool compileOps(DecodedOp* op);
    virtual U8* createDynamicExecutableMemory(U32* pSize = nullptr);
    virtual void createHelpers() {}
    RegPtr callGetCondition(JitConditional condition, RegPtr resultReg = nullptr);
    RegPtr calculateCondition(JitConditional condition, RegPtr resultReg = nullptr);

    bool disableTmps = false;
};

void startNewJIT(CPU* cpu, U32 address, DecodedOp* op);
JitCodeGen* startNewJIT(CPU* cpu);
void writeBlockExitForJIT(U32 eip, U8* buffer);

#endif
