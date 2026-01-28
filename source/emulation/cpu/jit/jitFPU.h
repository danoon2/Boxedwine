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
    FPURegInternal(U8 hardwareReg) : reg(hardwareReg) {}

    U8 hardwareReg() { return reg; }

private:
    U8 reg;
};

using FPURegPtr = std::shared_ptr<FPURegInternal>;

// Implementation of JIT that is host instruction independent
class JitFPU : public JitCodeGen {
public:
    using XmmXmmCallback = void(JitFPU::*)(FPURegPtr dst, FPURegPtr src);

    JitFPU(CPU* cpu) : JitCodeGen(cpu) {}
    
    virtual FPURegPtr getFPUTmp() = 0;
    virtual void storeCpuFpuReg(FPURegPtr reg, RegPtr index) = 0;
    virtual void loadCpuFpuReg(FPURegPtr reg, RegPtr index) = 0;
    virtual void loadCpuFpuRegConst(FPURegPtr reg, U32 offset) = 0;

    virtual void storeFpuReg(FPURegPtr reg, RegPtr rm, RegPtr sib, DynFpuWidth width = DYN_FPU_64_BIT) = 0;
    virtual void loadFpuReg(FPURegPtr reg, RegPtr rm, RegPtr sib, DynFpuWidth width = DYN_FPU_64_BIT) = 0;
    virtual void loadFpuRegFromInt(FPURegPtr reg, RegPtr rm, RegPtr sib) = 0;
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
    virtual void storeFPUToInt64(FPURegPtr src, RegPtr address, RegPtr offset, bool truncate) = 0;

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
    RegPtr syncCPUToXmm(RegPtr topReg, FPURegPtr xmm, U8 regIndex);
    RegPtr readFPUTag(RegPtr indexReg);
    void writeFPUTag(RegPtr indexReg, RegPtr valueReg);

    void dynamic_FPU_POP(RegPtr topReg, U8 amount = 1);
    void dynamic_FPU_PREP_PUSH(RegPtr topReg, bool writeTag);
    void dynamic_doCMov(U8 regIndex);
    void dynamic_ST0_STj(DecodedOp* op, XmmXmmCallback callback, bool reverse = false);
    void dynamic_FADD_ST0_STj(DecodedOp* op) override { dynamic_ST0_STj(op, &JitFPU::fpuAdd); }
    void dynamic_FMUL_ST0_STj(DecodedOp* op) override { dynamic_ST0_STj(op, &JitFPU::fpuMul); }
    void dynamic_FSUBR_ST0_STj(DecodedOp* op) override { dynamic_ST0_STj(op, &JitFPU::fpuSub, true); }
    void dynamic_FSUB_ST0_STj(DecodedOp* op) override { dynamic_ST0_STj(op, &JitFPU::fpuSub); }
    void dynamic_FDIVR_ST0_STj(DecodedOp* op) override { dynamic_ST0_STj(op, &JitFPU::fpuDiv, true); }
    void dynamic_FDIV_ST0_STj(DecodedOp* op) override { dynamic_ST0_STj(op, &JitFPU::fpuDiv); }

    void dynamic_STi_ST0(DecodedOp* op, XmmXmmCallback callback, bool reverse = false, bool pop = false);
    void dynamic_FADD_STi_ST0(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuAdd); }
    void dynamic_FMUL_STi_ST0(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuMul); }
    void dynamic_FCOM_STi(DecodedOp* op) override;
    void dynamic_FCOM_STi_Pop(DecodedOp* op) override;
    void dynamic_FSUBR_STi_ST0(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuSub, true); }
    void dynamic_FSUB_STi_ST0(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuSub); }
    void dynamic_FDIVR_STi_ST0(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuDiv, true); }
    void dynamic_FDIV_STi_ST0(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuDiv); }
    void dynamic_FADD_STi_ST0_Pop(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuAdd, false, true); }
    void dynamic_FMUL_STi_ST0_Pop(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuMul, false, true); }
    void dynamic_FCOMPP(DecodedOp* op) override;
    void dynamic_FSUBR_STi_ST0_Pop(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuSub, true, true); }
    void dynamic_FSUB_STi_ST0_Pop(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuSub, false, true); }
    void dynamic_FDIVR_STi_ST0_Pop(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuDiv, true, true); }
    void dynamic_FDIV_STi_ST0_Pop(DecodedOp* op) override { dynamic_STi_ST0(op, &JitFPU::fpuDiv, false, true); }

    void dynamic_SINGLE_REAL(DecodedOp* op, XmmXmmCallback callback, bool reverse = false);
    void dynamic_DOUBLE_REAL(DecodedOp* op, XmmXmmCallback callback, bool reverse = false);
    void dynamic_DWORD_INTEGER(DecodedOp* op, XmmXmmCallback callback, bool reverse = false);
    void dynamic_WORD_INTEGER(DecodedOp* op, XmmXmmCallback callback, bool reverse = false);

    void dynamic_FADD_SINGLE_REAL(DecodedOp* op) override { dynamic_SINGLE_REAL(op, &JitFPU::fpuAdd); }
    void dynamic_FMUL_SINGLE_REAL(DecodedOp* op) override { dynamic_SINGLE_REAL(op, &JitFPU::fpuMul); }
    void dynamic_FCOM_SINGLE_REAL(DecodedOp* op) override;
    void dynamic_FCOM_SINGLE_REAL_Pop(DecodedOp* op) override;
    void dynamic_FSUB_SINGLE_REAL(DecodedOp* op) override { dynamic_SINGLE_REAL(op, &JitFPU::fpuSub); }
    void dynamic_FSUBR_SINGLE_REAL(DecodedOp* op) override { dynamic_SINGLE_REAL(op, &JitFPU::fpuSub, true); }
    void dynamic_FDIV_SINGLE_REAL(DecodedOp* op) override { dynamic_SINGLE_REAL(op, &JitFPU::fpuDiv); }
    void dynamic_FDIVR_SINGLE_REAL(DecodedOp* op) override { dynamic_SINGLE_REAL(op, &JitFPU::fpuDiv, true); }
    void dynamic_FADD_DOUBLE_REAL(DecodedOp* op) override { dynamic_DOUBLE_REAL(op, &JitFPU::fpuAdd); }
    void dynamic_FMUL_DOUBLE_REAL(DecodedOp* op) override { dynamic_DOUBLE_REAL(op, &JitFPU::fpuMul); }
    void dynamic_FCOM_DOUBLE_REAL(DecodedOp* op) override;
    void dynamic_FCOM_DOUBLE_REAL_Pop(DecodedOp* op) override;
    void dynamic_FSUB_DOUBLE_REAL(DecodedOp* op) override { dynamic_DOUBLE_REAL(op, &JitFPU::fpuSub); }
    void dynamic_FSUBR_DOUBLE_REAL(DecodedOp* op) override { dynamic_DOUBLE_REAL(op, &JitFPU::fpuSub, true); }
    void dynamic_FDIV_DOUBLE_REAL(DecodedOp* op) override { dynamic_DOUBLE_REAL(op, &JitFPU::fpuDiv); }
    void dynamic_FDIVR_DOUBLE_REAL(DecodedOp* op) override { dynamic_DOUBLE_REAL(op, &JitFPU::fpuDiv, true); }
    void dynamic_FIADD_DWORD_INTEGER(DecodedOp* op) override { dynamic_DWORD_INTEGER(op, &JitFPU::fpuAdd); }
    void dynamic_FIMUL_DWORD_INTEGER(DecodedOp* op) override { dynamic_DWORD_INTEGER(op, &JitFPU::fpuMul); }
    void dynamic_FICOM_DWORD_INTEGER(DecodedOp* op) override;
    void dynamic_FICOM_DWORD_INTEGER_Pop(DecodedOp* op) override;
    void dynamic_FISUB_DWORD_INTEGER(DecodedOp* op) override { dynamic_DWORD_INTEGER(op, &JitFPU::fpuSub); }
    void dynamic_FISUBR_DWORD_INTEGER(DecodedOp* op) override { dynamic_DWORD_INTEGER(op, &JitFPU::fpuSub, true); }
    void dynamic_FIDIV_DWORD_INTEGER(DecodedOp* op) override { dynamic_DWORD_INTEGER(op, &JitFPU::fpuDiv); }
    void dynamic_FIDIVR_DWORD_INTEGER(DecodedOp* op) override { dynamic_DWORD_INTEGER(op, &JitFPU::fpuDiv, true); }
    void dynamic_FIADD_WORD_INTEGER(DecodedOp* op) override { dynamic_WORD_INTEGER(op, &JitFPU::fpuAdd); }
    void dynamic_FIMUL_WORD_INTEGER(DecodedOp* op) override { dynamic_WORD_INTEGER(op, &JitFPU::fpuMul); }
    void dynamic_FICOM_WORD_INTEGER(DecodedOp* op) override;
    void dynamic_FICOM_WORD_INTEGER_Pop(DecodedOp* op) override;
    void dynamic_FISUB_WORD_INTEGER(DecodedOp* op) override { dynamic_WORD_INTEGER(op, &JitFPU::fpuSub); }
    void dynamic_FISUBR_WORD_INTEGER(DecodedOp* op) override { dynamic_WORD_INTEGER(op, &JitFPU::fpuSub, true); }
    void dynamic_FIDIV_WORD_INTEGER(DecodedOp* op) override { dynamic_WORD_INTEGER(op, &JitFPU::fpuDiv); }
    void dynamic_FIDIVR_WORD_INTEGER(DecodedOp* op) override { dynamic_WORD_INTEGER(op, &JitFPU::fpuDiv, true); }

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
    void loadFpuRegFromShort(FPURegPtr reg, RegPtr rm, RegPtr sib);
    void fpuLoadConst(U32 offset);
    void doFCOM(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags);
    void doFCOMI(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags);
    void doFST_STi(DecodedOp* op, bool pop);
    void doFCOM_STi(DecodedOp* op, bool pop);
    void doFCOMI_ST0_STj(DecodedOp* op, bool pop);
    void doFFREE_STi(DecodedOp* op, bool pop);

    void createCOS32s();
};

#endif