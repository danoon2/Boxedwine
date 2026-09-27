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

#ifndef __JIT_FPU_H__
#define __JIT_FPU_H__

#include "jitCodeGen.h"

enum DynFpuWidth {
    DYN_FPU_32_BIT = 0,
    DYN_FPU_64_BIT = 1
};

class FPURegInternal {
public:
    FPURegInternal(U8 hardwareReg, std::function<U8()> reload = nullptr) : reg(hardwareReg), reload(reload) {}

    U8 hardwareReg() { if (reg == 0xff) reg = reload(); return reg; }
    bool isLoaded() const { return reg != 0xff; }
    void invalidateHardwareReg() { reg = 0xff; }
    void setHardwareReg(U8 hardwareReg) { reg = hardwareReg; }

private:
    U8 reg;
    std::function<U8()> reload;
};

using FPURegPtr = std::shared_ptr<FPURegInternal>;

enum class FpuMath { Sin, Cos, Tan, Log, Exp2Minus1 };

// Implementation of JIT that is host instruction independent
class JitFPU : public JitCodeGen {
public:
    using XmmXmmCallback = void(JitFPU::*)(FPURegPtr dst, FPURegPtr src);

    JitFPU(CPU* cpu) : JitCodeGen(cpu) {}

    // Backends opt in with eight persistent value/tag registers and an entry TOP.
    // The virtual stack and instruction lowering are shared across hosts.
    virtual bool supportsFpuStackCache() const { return false; }
    virtual bool supportsFpuLoopCache() const { return false; }
    virtual RegPtr getFpuCacheValidReg(U8 index) { kpanic("FPU loop cache unsupported"); return nullptr; }
    virtual FPURegPtr getFpuCacheReg(U8 index) { kpanic("FPU cache unsupported"); return nullptr; }
    virtual RegPtr getFpuCacheTagReg(U8 index) { kpanic("FPU cache unsupported"); return nullptr; }
    virtual RegPtr getFpuCacheTopReg() { kpanic("FPU cache unsupported"); return nullptr; }
    virtual void moveFpuReg(FPURegPtr dst, FPURegPtr src) { kpanic("FPU cache unsupported"); }
    // Import one physical slot, converting raw extended precision if necessary.
    // With valid supplied, dst already contains the loop-carried double value.
    // Raw transcendental inputs use the current SoftFloat rounding mode;
    // ordinary cache validation matches getF64's nearest-even conversion.
    virtual void loadFpuValue(FPURegPtr dst, RegPtr index, RegPtr valid, bool nearest = true) { kpanic("FPU conversion unsupported"); }
    virtual void storeDoubleAsExtended(FPURegPtr value, RegPtr index) { kpanic("FPU extended result unsupported"); }
    // Access one physical CPU slot without changing TOP or other cached slots.
    virtual void loadInt64ToExtended(MemPtr address, RegPtr index) { kpanic("FPU extended load unsupported"); }
    virtual void storeExtendedAsInt64(RegPtr index, MemPtr address, bool truncate) { kpanic("FPU extended store unsupported"); }

    // The callback sees exactly ten checked bytes of contiguous host memory.
    virtual void accessFpu80(RegPtr address, bool store, const std::function<void(MemPtr)>& action) { kpanic("FPU 80-bit memory unsupported"); }
    // Environment transfers check the entire range before changing cached state.
    virtual void accessFpuEnvironment(RegPtr address, U32 size, bool store, const std::function<void(MemPtr)>& action) { kpanic("FPU environment memory unsupported"); }
    // No temporary handles may escape the callback. Register-file backends
    // can reuse their per-instruction scratch locals between unrolled slots.
    virtual void withFpuScratchScope(const std::function<void()>& action) { action(); }
    virtual void transferFpu80(MemPtr address, RegPtr index, bool store, bool bcd) { kpanic("FPU 80-bit transfer unsupported"); }
    // Save one live value as raw extended precision, retaining its physical
    // raw register bits as FSAVE requires. The guest range is already checked.
    virtual void saveFpu80(MemPtr address, RegPtr index, FPURegPtr value, RegPtr valid) { kpanic("FPU cached save unsupported"); }
    virtual void storeFpuBcd(MemPtr address, RegPtr index, FPURegPtr value, RegPtr valid) { kpanic("FPU cached BCD store unsupported"); }

    // Math routines take/return floating-point registers, without CPU state.
    virtual void fpuMath(FpuMath op, FPURegPtr dst, FPURegPtr src) { kpanic("FPU math unsupported"); }
    virtual void fpuAtan2(FPURegPtr dst, FPURegPtr y, FPURegPtr x) { kpanic("FPU atan2 unsupported"); }
    virtual RegPtr classifyFpu(FPURegPtr value) { kpanic("FPU classify unsupported"); return nullptr; }
    virtual RegPtr classifyExtendedFpu(RegPtr index) { kpanic("FPU extended classify unsupported"); return nullptr; }

    // Slow arithmetic accesses only the named physical slots and never TOP.
    virtual void fpuSlotArithmetic(U8 operation, RegPtr st0, RegPtr st1) { kpanic("FPU slot arithmetic unsupported"); }
    virtual void fpuRemainder(FPURegPtr x, FPURegPtr y, RegPtr xValid, RegPtr yValid, RegPtr st0, RegPtr st1, bool nearest, const std::function<void()>& slow) { kpanic("FPU remainder unsupported"); }
    virtual void fpuScale(FPURegPtr x, FPURegPtr y, RegPtr xValid, RegPtr yValid, RegPtr st0, RegPtr st1) { kpanic("FPU scale unsupported"); }
    virtual void fpuExtract(FPURegPtr x, RegPtr valid, RegPtr st0, RegPtr result) { kpanic("FPU extract unsupported"); }

    bool canKeepFpuCache(DecodedOp* op) const;
    virtual bool canCacheFpuOp(DecodedOp* op) const;
    bool compileCachedFpuOp(DecodedOp* op);
    virtual void materializeFpuCache(U8 slotMask = 0xff);
    void flushFpuCache();

protected:
    // Native backends currently support the same subset of cached x87 lowering.
    // Keep their exclusions together until either backend implements more hooks.
    bool canCacheNativeFpuOp(DecodedOp* op) const;

    struct CachedFpuSlot {
        FPURegPtr value;
        RegPtr tag;
        bool loaded = false;
        bool dirty = false;
        bool tagLoaded = false;
        bool tagDirty = false;
        RegPtr valid; // loop-carried double validity; extended values stay lazy
        bool needsValidation = false;
    };
    struct FpuStackCache {
        CachedFpuSlot slots[8];
        RegPtr entryTop;
        U8 top = 0;
    } fpuStackCache;
    void beginFpuCache();
    void prepareFpuCacheForLoop(bool dynamicValidity);
    void reconcileFpuCacheForLoop(const FpuStackCache& entry);
    S32 cachedFpuStackChange(DecodedOp* op) const;
    CachedFpuSlot& cachedFpuSlot(U8 relativeIndex);
    FPURegPtr cachedFpuValue(U8 relativeIndex);
    RegPtr cachedFpuTag(U8 relativeIndex);
    void cachedFpuPush(FPURegPtr value, RegPtr tag = nullptr);
    void cachedFpuPop();
    void cachedFpuCompare(U8 src, U8 pops, bool integerFlags = false);
    void cachedFpuStore(U8 dst, bool pop);
    void cachedFpuConditionalMove(U8 src, JitConditional condition);
    void cachedFpuLoadConst(U32 offset);
    void cachedFpuLoadInt64(MemPtr address);
    void markCachedFpuTopExtended();
    void cachedFpuTransfer80(DecodedOp* op);
    void cachedFpuMath(DecodedOp* op);
    void cachedFpuExamine();
    void loadCachedFpuState(U8 relativeIndex);
    void cachedFpuSpecial(DecodedOp* op);
    void cachedFpuInit(DecodedOp* op);
    void cachedFpuEnvironment(DecodedOp* op);
    void rebaseFpuCache(RegPtr newTop);
    RegPtr cachedFpuTagWord();
    void cachedFpuStoreInt64(MemPtr address, bool truncate);
    void cachedFpuCompareMemory(DecodedOp* op, JitWidth width, bool integer, bool pop);
    RegPtr cachedFpuStatusWord();
    void cachedFpuBinary(U8 dst, U8 src, XmmXmmCallback callback, bool reverse, bool pop);
    void cachedFpuDiv(U8 dst, U8 src, bool reverse, bool pop);
    void guardCachedFpuFallback(RegPtr state);
    void cachedFpuMemory(DecodedOp* op, XmmXmmCallback callback, JitWidth width, bool integer, bool reverse, bool division = false);

public:

    virtual FPURegPtr getFPUTmp() = 0;
    virtual void storeCpuFpuReg(FPURegPtr reg, RegPtr index) = 0;
    virtual void loadCpuFpuReg(FPURegPtr reg, RegPtr index) = 0;
    virtual void loadCpuFpuRegConst(FPURegPtr reg, U32 offset) = 0;

    virtual void storeFpuReg(FPURegPtr reg, MemPtr address, DynFpuWidth width = DYN_FPU_64_BIT) = 0;
    virtual void loadFpuReg(FPURegPtr reg, MemPtr address, DynFpuWidth width = DYN_FPU_64_BIT) = 0;
    virtual void loadFpuRegFromInt(FPURegPtr reg, MemPtr address) = 0;
    virtual void fpuRegExtend32To64(FPURegPtr dst, FPURegPtr src) = 0;
    virtual void fpuReg64To32(FPURegPtr dst, FPURegPtr src) = 0;
    virtual RegPtr fpuRegToInt32(FPURegPtr fpuRegSrc, bool truncate) = 0;
    virtual void regToFpuReg(FPURegPtr dst, RegPtr src) = 0;
#ifdef BOXEDWINE_64
    virtual void regToFpuReg64(FPURegPtr dst, RegPtr src) = 0;
#endif
    virtual void updateFPURounding() = 0;
    virtual void restoreFPURounding() = 0;
    virtual void roundFPUToInt64(FPURegPtr src) = 0;
    virtual void storeFPUToInt64(FPURegPtr src, MemPtr address, bool truncate) = 0;

    virtual void fpuAdd(FPURegPtr dst, FPURegPtr src) = 0;
    virtual void fpuMul(FPURegPtr dst, FPURegPtr src) = 0;
    virtual void fpuSub(FPURegPtr dst, FPURegPtr src) = 0;
    virtual void fpuDiv(FPURegPtr dst, FPURegPtr src) = 0;
    virtual void fpuXor(FPURegPtr dst, FPURegPtr src) = 0;
    virtual void fpuAnd(FPURegPtr dst, FPURegPtr src) = 0;
    virtual void fpuSqrt(FPURegPtr dst, FPURegPtr src) = 0;
    virtual void fcompare(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) = 0;

    RegPtr getTopReg();
    RegPtr calculateIndexReg(RegPtr topReg, U32 index);
    void IfNotRegCached(RegPtr indexReg);
    void setRegIsCached(RegPtr indexReg, bool regIsCached);
    void syncXmmToCPU(RegPtr topReg, FPURegPtr xmm, U8 regIndex);
    void syncXmmToCPUWithIndexReg(RegPtr indexReg, FPURegPtr fpuReg);
    virtual void cacheFpuReg(U32 regIndex);
    RegPtr syncCPUToXmm(RegPtr topReg, FPURegPtr xmm, U8 regIndex);
    RegPtr readFPUTag(RegPtr indexReg);
    void writeFPUTag(RegPtr indexReg, RegPtr valueReg);
    void updateFpuDivExceptionState();
    void updateExceptionSummary();
    void guardFpuDivRegTags(RegPtr stIndex, RegPtr otherIndex, bool reverse);
    void guardFpuDivMemory(RegPtr top, RegPtr isZero);
    void roundFpuResultToPrecision(FPURegPtr result);

    void dynamic_FPU_POP(RegPtr topReg, U8 amount = 1);
    void dynamic_FPU_PREP_PUSH(RegPtr topReg, bool writeTag);
    void dynamic_doCMov(U8 regIndex);
    void dynamic_ST0_STj(DecodedOp* op, XmmXmmCallback callback, bool reverse = false);
    void dynamic_FADD_ST0_STj(DecodedOp* op) override { dynamic_ST0_STj(op, &JitFPU::fpuAdd); }
    void dynamic_FMUL_ST0_STj(DecodedOp* op) override { dynamic_ST0_STj(op, &JitFPU::fpuMul); }
    void dynamic_FSUBR_ST0_STj(DecodedOp* op) override { dynamic_ST0_STj(op, &JitFPU::fpuSub, true); }
    void dynamic_FSUB_ST0_STj(DecodedOp* op) override { dynamic_ST0_STj(op, &JitFPU::fpuSub); }
    void dynamic_FDIVR_ST0_STj(DecodedOp* op) override;
    void dynamic_FDIV_ST0_STj(DecodedOp* op) override;

    void dynamic_STi_ST0(DecodedOp* op, XmmXmmCallback callback, bool reverse = false, bool pop = false);
    void dynamic_FADD_STi_ST0(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuAdd); }
    void dynamic_FMUL_STi_ST0(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuMul); }
    void dynamic_FCOM_STi(DecodedOp* op) override;
    void dynamic_FCOM_STi_Pop(DecodedOp* op) override;
    void dynamic_FSUBR_STi_ST0(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuSub, true); }
    void dynamic_FSUB_STi_ST0(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuSub); }
    void dynamic_FDIVR_STi_ST0(DecodedOp* op) override;
    void dynamic_FDIV_STi_ST0(DecodedOp* op) override;
    void dynamic_FADD_STi_ST0_Pop(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuAdd, false, true); }
    void dynamic_FMUL_STi_ST0_Pop(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuMul, false, true); }
    void dynamic_FCOMPP(DecodedOp* op) override;
    void dynamic_FSUBR_STi_ST0_Pop(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuSub, true, true); }
    void dynamic_FSUB_STi_ST0_Pop(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuSub, false, true); }
    void dynamic_FDIVR_STi_ST0_Pop(DecodedOp* op) override;
    void dynamic_FDIV_STi_ST0_Pop(DecodedOp* op) override;

    void dynamic_SINGLE_REAL(DecodedOp* op, XmmXmmCallback callback, bool reverse = false);
    void dynamic_DOUBLE_REAL(DecodedOp* op, XmmXmmCallback callback, bool reverse = false);
    void dynamic_DWORD_INTEGER(DecodedOp* op, XmmXmmCallback callback, bool reverse = false);
    void dynamic_WORD_INTEGER(DecodedOp* op, XmmXmmCallback callback, bool reverse = false);
    void dynamic_DIV_SINGLE_REAL(DecodedOp* op, bool reverse);
    void dynamic_DIV_DOUBLE_REAL(DecodedOp* op, bool reverse);
    void dynamic_IDIV_DWORD_INTEGER(DecodedOp* op, bool reverse);
    void dynamic_IDIV_WORD_INTEGER(DecodedOp* op, bool reverse);

    void dynamic_FADD_SINGLE_REAL(DecodedOp* op) override { dynamic_SINGLE_REAL(op, &JitFPU::fpuAdd); }
    void dynamic_FMUL_SINGLE_REAL(DecodedOp* op) override { dynamic_SINGLE_REAL(op, &JitFPU::fpuMul); }
    void dynamic_FCOM_SINGLE_REAL(DecodedOp* op) override;
    void dynamic_FCOM_SINGLE_REAL_Pop(DecodedOp* op) override;
    void dynamic_FSUB_SINGLE_REAL(DecodedOp* op) override { dynamic_SINGLE_REAL(op, &JitFPU::fpuSub); }
    void dynamic_FSUBR_SINGLE_REAL(DecodedOp* op) override { dynamic_SINGLE_REAL(op, &JitFPU::fpuSub, true); }
    void dynamic_FDIV_SINGLE_REAL(DecodedOp* op) override { dynamic_DIV_SINGLE_REAL(op, false); }
    void dynamic_FDIVR_SINGLE_REAL(DecodedOp* op) override { dynamic_DIV_SINGLE_REAL(op, true); }
    void dynamic_FADD_DOUBLE_REAL(DecodedOp* op) override { dynamic_DOUBLE_REAL(op, &JitFPU::fpuAdd); }
    void dynamic_FMUL_DOUBLE_REAL(DecodedOp* op) override { dynamic_DOUBLE_REAL(op, &JitFPU::fpuMul); }
    void dynamic_FCOM_DOUBLE_REAL(DecodedOp* op) override;
    void dynamic_FCOM_DOUBLE_REAL_Pop(DecodedOp* op) override;
    void dynamic_FSUB_DOUBLE_REAL(DecodedOp* op) override { dynamic_DOUBLE_REAL(op, &JitFPU::fpuSub); }
    void dynamic_FSUBR_DOUBLE_REAL(DecodedOp* op) override { dynamic_DOUBLE_REAL(op, &JitFPU::fpuSub, true); }
    void dynamic_FDIV_DOUBLE_REAL(DecodedOp* op) override { dynamic_DIV_DOUBLE_REAL(op, false); }
    void dynamic_FDIVR_DOUBLE_REAL(DecodedOp* op) override { dynamic_DIV_DOUBLE_REAL(op, true); }
    void dynamic_FIADD_DWORD_INTEGER(DecodedOp* op) override { dynamic_DWORD_INTEGER(op, &JitFPU::fpuAdd); }
    void dynamic_FIMUL_DWORD_INTEGER(DecodedOp* op) override { dynamic_DWORD_INTEGER(op, &JitFPU::fpuMul); }
    void dynamic_FICOM_DWORD_INTEGER(DecodedOp* op) override;
    void dynamic_FICOM_DWORD_INTEGER_Pop(DecodedOp* op) override;
    void dynamic_FISUB_DWORD_INTEGER(DecodedOp* op) override { dynamic_DWORD_INTEGER(op, &JitFPU::fpuSub); }
    void dynamic_FISUBR_DWORD_INTEGER(DecodedOp* op) override { dynamic_DWORD_INTEGER(op, &JitFPU::fpuSub, true); }
    void dynamic_FIDIV_DWORD_INTEGER(DecodedOp* op) override { dynamic_IDIV_DWORD_INTEGER(op, false); }
    void dynamic_FIDIVR_DWORD_INTEGER(DecodedOp* op) override { dynamic_IDIV_DWORD_INTEGER(op, true); }
    void dynamic_FIADD_WORD_INTEGER(DecodedOp* op) override { dynamic_WORD_INTEGER(op, &JitFPU::fpuAdd); }
    void dynamic_FIMUL_WORD_INTEGER(DecodedOp* op) override { dynamic_WORD_INTEGER(op, &JitFPU::fpuMul); }
    void dynamic_FICOM_WORD_INTEGER(DecodedOp* op) override;
    void dynamic_FICOM_WORD_INTEGER_Pop(DecodedOp* op) override;
    void dynamic_FISUB_WORD_INTEGER(DecodedOp* op) override { dynamic_WORD_INTEGER(op, &JitFPU::fpuSub); }
    void dynamic_FISUBR_WORD_INTEGER(DecodedOp* op) override { dynamic_WORD_INTEGER(op, &JitFPU::fpuSub, true); }
    void dynamic_FIDIV_WORD_INTEGER(DecodedOp* op) override { dynamic_IDIV_WORD_INTEGER(op, false); }
    void dynamic_FIDIVR_WORD_INTEGER(DecodedOp* op) override { dynamic_IDIV_WORD_INTEGER(op, true); }

    void dynamic_FCMOV_ST0_STj_CF(DecodedOp* op) override;
    void dynamic_FCMOV_ST0_STj_ZF(DecodedOp* op) override;
    void dynamic_FCMOV_ST0_STj_CF_OR_ZF(DecodedOp* op) override;
    void dynamic_FCMOV_ST0_STj_PF(DecodedOp* op) override;
    void dynamic_FCMOV_ST0_STj_NCF(DecodedOp* op) override;
    void dynamic_FCMOV_ST0_STj_NZF(DecodedOp* op) override;
    void dynamic_FCMOV_ST0_STj_NCF_AND_NZF(DecodedOp* op) override;
    void dynamic_FCMOV_ST0_STj_NPF(DecodedOp* op) override;

    void dynamic_FCHS(DecodedOp* op) override;
    void dynamic_FABS(DecodedOp* op) override;
    void dynamic_FTST(DecodedOp* op) override;
    void dynamic_FLD_STi(DecodedOp* op) override;
    void dynamic_FXCH_STi(DecodedOp* op) override;
    void dynamic_FNOP(DecodedOp* op) override;
    void dynamic_FST_STi_Pop(DecodedOp* op) override;
    void dynamic_FST_STi(DecodedOp* op) override;
    void dynamic_FLD1(DecodedOp* op) override;
    void dynamic_FLDL2T(DecodedOp* op) override;
    void dynamic_FLDL2E(DecodedOp* op) override;
    void dynamic_FLDPI(DecodedOp* op) override;
    void dynamic_FLDLG2(DecodedOp* op) override;
    void dynamic_FLDLN2(DecodedOp* op) override;
    void dynamic_FLDZ(DecodedOp* op) override;
    void dynamic_FDECSTP(DecodedOp* op) override;
    void dynamic_FINCSTP(DecodedOp* op) override;
    void dynamic_FSQRT(DecodedOp* op) override;
    void dynamic_FILD_DWORD_INTEGER(DecodedOp* op) override;
    void dynamic_FLD_SINGLE_REAL(DecodedOp* op) override;
    void dynamic_FST_SINGLE_REAL(DecodedOp* op) override;
    void dynamic_FST_SINGLE_REAL_Pop(DecodedOp* op) override;
    void dynamic_FNSTCW(DecodedOp* op) override;
    void dynamic_FLDCW(DecodedOp* op) override;
    void dynamic_FRNDINT(DecodedOp* op) override;
    void dynamic_FLDENV(DecodedOp* op) override;
    void dynamic_FNSTENV(DecodedOp* op) override;
    void dynamic_FUCOMPP(DecodedOp* op) override;
    void dynamic_FNCLEX(DecodedOp* op) override;
    void dynamic_FNINIT(DecodedOp* op) override;
    void dynamic_FUCOMI_ST0_STj(DecodedOp* op) override;
    void dynamic_FCOMI_ST0_STj(DecodedOp* op) override;
    void dynamic_FISTTP32(DecodedOp* op) override;
    void dynamic_FIST_DWORD_INTEGER(DecodedOp* op) override;
    void dynamic_FIST_DWORD_INTEGER_Pop(DecodedOp* op) override;

    void dynamic_FFREE_STi(DecodedOp* op) override;
    void dynamic_FUCOM_STi(DecodedOp* op) override;
    void dynamic_FUCOM_STi_Pop(DecodedOp* op) override;
    void dynamic_FLD_DOUBLE_REAL(DecodedOp* op) override;
    void dynamic_FISTTP64(DecodedOp* op) override;
    void dynamic_FST_DOUBLE_REAL(DecodedOp* op) override;
    void dynamic_FST_DOUBLE_REAL_Pop(DecodedOp* op) override;
    void dynamic_FNSTSW(DecodedOp* op) override;
    void dynamic_FFREEP_STi(DecodedOp* op) override;
    void dynamic_FNSTSW_AX(DecodedOp* op) override;
    void dynamic_FUCOMI_ST0_STj_Pop(DecodedOp* op) override;
    void dynamic_FCOMI_ST0_STj_Pop(DecodedOp* op) override;
    void dynamic_FILD_WORD_INTEGER(DecodedOp* op) override;
    void dynamic_FISTTP16(DecodedOp* op) override;
    void dynamic_FIST_WORD_INTEGER(DecodedOp* op) override;
    void dynamic_FIST_WORD_INTEGER_Pop(DecodedOp* op) override;    
    void dynamic_FILD_QWORD_INTEGER(DecodedOp* op) override;
    void dynamic_FISTP_QWORD_INTEGER(DecodedOp* op) override;    

private:
    RegPtr getFpuDivSlowPathState(RegPtr indexReg);
    RegPtr getFpuDivMemoryFloatZero(MemPtr address, JitWidth width);
    RegPtr getFpuDivMemoryIntZero(MemPtr address, JitWidth width);
    void guardFpuDivSlowPath(RegPtr state);
    void loadFpuRegFromShort(FPURegPtr reg, MemPtr address);
    void fpuLoadConst(U32 offset);
    virtual void doFCOM(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags);
    virtual void doFCOMI(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags);
    void doFST_STi(DecodedOp* op, bool pop);
    void doFCOM_STi(DecodedOp* op, bool pop);
    void doFCOMI_ST0_STj(DecodedOp* op, bool pop);
    void doFFREE_STi(DecodedOp* op, bool pop);

    void createCOS32s();
};

#endif
