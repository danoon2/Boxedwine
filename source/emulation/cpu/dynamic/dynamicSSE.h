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

#ifndef __DYNAMICSSE_H__
#define __DYNAMICSSE_H__

#include "dynamicMMX.h"

enum DynXMMReg {
    DYN_XMM_REG_0 = 0,
    DYN_XMM_REG_1 = 1,
    DYN_XMM_REG_2 = 2,
    DYN_XMM_REG_3 = 3,
    DYN_XMM_REG_4 = 4,
    DYN_XMM_REG_5 = 5,
    DYN_XMM_REG_6 = 6,
    DYN_XMM_REG_7 = 7,
};

// Implementation of JIT that is host instruction independent
class DynamicCodeGenSSE : public DynamicCodeGenMMX {
public:
    using XmmXmmCallback = void(DynamicCodeGenSSE::*)(DynXMMReg dst, DynXMMReg src);
    using XmmXmmImmCallback = void(DynamicCodeGenSSE::*)(DynXMMReg dst, DynXMMReg src, U32 imm);

    virtual DynXMMReg getTmpXMM(U8 inUse) = 0;
    virtual void storeCpuXMMReg(DynXMMReg reg, U32 index) = 0;
    virtual void loadCpuXMMReg(DynXMMReg reg, U32 index) = 0;
    virtual void loadXMMFromMem128(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) = 0;
    virtual void loadXMMFromMem32(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) = 0;
    virtual void loadXMMFromMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) = 0;
    virtual void loadLowXMMFromMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) = 0;
    virtual void loadHighXMMFromMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) = 0;
    virtual void storeXMMToMem128(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) = 0;
    virtual void storeXMMToMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) = 0;
    virtual void storeXMMToMem32(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) = 0;
    virtual void storeHighXMMToMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) = 0;

    void opXmmXmm(DecodedOp* op, XmmXmmCallback callback);
    void opXmmXmmImm(DecodedOp* op, XmmXmmImmCallback callback);
    void opXmmE128(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback);
    void opXmmE128Imm(DecodedOp* op, XmmXmmImmCallback callback, std::function<void()> fallback);
    void opXmmE32(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback);
    void opXmmE32Imm(DecodedOp* op, XmmXmmImmCallback callback, std::function<void()> fallback);

    virtual void addpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void addssXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void subpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void subssXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void mulpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void mulssXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void divpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void divssXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void rcppsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void rcpssXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void sqrtpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void sqrtssXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void rsqrtpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void rsqrtssXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void maxpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void maxssXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void minpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void minssXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void andnpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void andpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void orpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void xorpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void cvtpi2psXmmMmx(DynXMMReg dst, DynMMXReg src) = 0;
    virtual void cvtps2piMmxXmm(DynMMXReg dst, DynXMMReg src) = 0;
    virtual void cvtsi2ssXmmR32(DynXMMReg dst, DynReg src) = 0;
    virtual void cvtss2siR32Xmm(DynReg dst, DynXMMReg src) = 0;
    virtual void cvttps2piMmxXmm(DynMMXReg dst, DynXMMReg src) = 0;
    virtual void cvttss2siR32Xmm(DynReg dst, DynXMMReg src) = 0;
    virtual void movhlpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void movlhpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void movmskpsR32Xmm(DynReg dst, DynXMMReg src) = 0;
    virtual void movssXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void shufpsXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) = 0;
    virtual void unpckhpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void unpcklpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void cmppsXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) = 0;
    virtual void cmpssXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) = 0;
    virtual void comissXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void ucomissXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
    virtual void sfence() = 0;
    virtual void stmxcsr(DynReg address) = 0;
    virtual void ldmxcsr(DynReg address) = 0;

    DynamicCodeGenSSE(CPU* cpu) : DynamicCodeGenMMX(cpu) {}
    void dynamic_addpsXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::addpsXmmXmm); }
    void dynamic_addpsE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::addpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_addpsE128(op); }); }
    void dynamic_addssXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::addssXmmXmm); }
    void dynamic_addssE32(DecodedOp* op) override { opXmmE32(op, &DynamicCodeGenSSE::addssXmmXmm, [op, this]() {DynamicCodeGen::dynamic_addssE32(op); }); }
    void dynamic_subpsXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::subpsXmmXmm); }
    void dynamic_subpsE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::subpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_subpsE128(op); }); }
    void dynamic_subssXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::subssXmmXmm); }
    void dynamic_subssE32(DecodedOp* op) override { opXmmE32(op, &DynamicCodeGenSSE::subssXmmXmm, [op, this]() {DynamicCodeGen::dynamic_subssE32(op); }); }
    void dynamic_mulpsXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::mulpsXmmXmm); }
    void dynamic_mulpsE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::mulpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_mulpsE128(op); }); }
    void dynamic_mulssXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::mulssXmmXmm); }
    void dynamic_mulssE32(DecodedOp* op) override { opXmmE32(op, &DynamicCodeGenSSE::mulssXmmXmm, [op, this]() {DynamicCodeGen::dynamic_mulssE32(op); }); }
    void dynamic_divpsXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::divpsXmmXmm); }
    void dynamic_divpsE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::divpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_divpsE128(op); }); }
    void dynamic_divssXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::divssXmmXmm); }
    void dynamic_divssE32(DecodedOp* op) override { opXmmE32(op, &DynamicCodeGenSSE::divssXmmXmm, [op, this]() {DynamicCodeGen::dynamic_divssE32(op); }); }
    void dynamic_rcppsXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::rcppsXmmXmm); }
    void dynamic_rcppsE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::rcppsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_rcppsE128(op); }); }
    void dynamic_rcpssXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::rcpssXmmXmm); }
    void dynamic_rcpssE32(DecodedOp* op) override { opXmmE32(op, &DynamicCodeGenSSE::rcpssXmmXmm, [op, this]() {DynamicCodeGen::dynamic_rcpssE32(op); }); }
    void dynamic_sqrtpsXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::sqrtpsXmmXmm); }
    void dynamic_sqrtpsE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::sqrtpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_sqrtpsE128(op); }); }
    void dynamic_sqrtssXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::sqrtssXmmXmm); }
    void dynamic_sqrtssE32(DecodedOp* op) override { opXmmE32(op, &DynamicCodeGenSSE::sqrtssXmmXmm, [op, this]() {DynamicCodeGen::dynamic_sqrtssE32(op); }); }
    void dynamic_rsqrtpsXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::rsqrtpsXmmXmm); }
    void dynamic_rsqrtpsE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::rsqrtpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_rsqrtpsE128(op); }); }
    void dynamic_rsqrtssXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::rsqrtssXmmXmm); }
    void dynamic_rsqrtssE32(DecodedOp* op) override { opXmmE32(op, &DynamicCodeGenSSE::rsqrtssXmmXmm, [op, this]() {DynamicCodeGen::dynamic_rsqrtssE32(op); }); }
    void dynamic_maxpsXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::maxpsXmmXmm); }
    void dynamic_maxpsE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::maxpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_maxpsE128(op); }); }
    void dynamic_maxssXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::maxssXmmXmm); }
    void dynamic_maxssE32(DecodedOp* op) override { opXmmE32(op, &DynamicCodeGenSSE::maxssXmmXmm, [op, this]() {DynamicCodeGen::dynamic_maxssE32(op); }); }
    void dynamic_minpsXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::minpsXmmXmm); }
    void dynamic_minpsE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::minpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_minpsE128(op); }); }
    void dynamic_minssXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::minssXmmXmm); }
    void dynamic_minssE32(DecodedOp* op) override { opXmmE32(op, &DynamicCodeGenSSE::minssXmmXmm, [op, this]() {DynamicCodeGen::dynamic_minssE32(op); }); }
    
    void dynamic_andnpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::andnpsXmmXmm); }
    void dynamic_andnpsXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::andnpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_andnpsXmmE128(op); }); }
    void dynamic_andpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::andpsXmmXmm); }
    void dynamic_andpsXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::andpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_andpsXmmE128(op); }); }
    void dynamic_orpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::orpsXmmXmm); }
    void dynamic_orpsXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::orpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_orpsXmmE128(op); }); }
    void dynamic_xorpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::xorpsXmmXmm); }
    void dynamic_xorpsXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::xorpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_xorpsXmmE128(op); }); }

    void dynamic_cvtpi2psXmmMmx(DecodedOp* op) override;
    void dynamic_cvtpi2psXmmE64(DecodedOp* op) override;
    void dynamic_cvtps2piMmxXmm(DecodedOp* op) override;
    void dynamic_cvtps2piMmxE64(DecodedOp* op) override;
    void dynamic_cvtsi2ssXmmR32(DecodedOp* op) override;
    void dynamic_cvtsi2ssXmmE32(DecodedOp* op) override;
    void dynamic_cvtss2siR32Xmm(DecodedOp* op) override;
    void dynamic_cvtss2siR32E32(DecodedOp* op) override;
    void dynamic_cvttps2piMmxXmm(DecodedOp* op) override;
    void dynamic_cvttps2piMmxE64(DecodedOp* op) override;
    void dynamic_cvttss2siR32Xmm(DecodedOp* op) override;
    void dynamic_cvttss2siR32E32(DecodedOp* op) override;

    void dynamic_movapsXmmXmm(DecodedOp* op) override { dynamic_movupsXmmXmm(op); }
    void dynamic_movapsXmmE128(DecodedOp* op) override { dynamic_movupsXmmE128(op); }
    void dynamic_movapsE128Xmm(DecodedOp* op) override { dynamic_movupsE128Xmm(op); }
    void dynamic_movupsXmmXmm(DecodedOp* op) override;
    void dynamic_movupsXmmE128(DecodedOp* op) override;
    void dynamic_movupsE128Xmm(DecodedOp* op) override;
    void dynamic_movhlpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::movhlpsXmmXmm); }
    void dynamic_movlhpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::movlhpsXmmXmm); }
    void dynamic_movhpsXmmE64(DecodedOp* op) override;
    void dynamic_movhpsE64Xmm(DecodedOp* op) override;
    void dynamic_movlpsXmmE64(DecodedOp* op) override;
    void dynamic_movlpsE64Xmm(DecodedOp* op) override;
    void dynamic_movmskpsR32Xmm(DecodedOp* op) override;
    void dynamic_movssXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::movssXmmXmm); }
    void dynamic_movssXmmE32(DecodedOp* op) override;
    void dynamic_movssE32Xmm(DecodedOp* op) override;
    void dynamic_movntpsE128Xmm(DecodedOp* op) override { dynamic_movupsE128Xmm(op); }
    
    void dynamic_shufpsXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &DynamicCodeGenSSE::shufpsXmmXmm); }
    void dynamic_shufpsXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &DynamicCodeGenSSE::shufpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_shufpsXmmE128(op); }); }
    void dynamic_unpckhpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::unpckhpsXmmXmm); }
    void dynamic_unpckhpsXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::unpckhpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_unpckhpsXmmE128(op); }); }
    void dynamic_unpcklpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::unpcklpsXmmXmm); }
    void dynamic_unpcklpsXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::unpcklpsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_unpcklpsXmmE128(op); }); }

    void dynamic_prefetchT0(DecodedOp* op) override {}
    void dynamic_prefetchT1(DecodedOp* op) override {}
    void dynamic_prefetchT2(DecodedOp* op) override {}
    void dynamic_prefetchNTA(DecodedOp* op) override {}

    void dynamic_cmppsXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &DynamicCodeGenSSE::cmppsXmmXmm); }
    void dynamic_cmppsXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &DynamicCodeGenSSE::cmppsXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cmppsXmmE128(op); }); }

    void dynamic_cmpssXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &DynamicCodeGenSSE::cmpssXmmXmm); }
    void dynamic_cmpssXmmE32(DecodedOp* op) override { opXmmE32Imm(op, &DynamicCodeGenSSE::cmpssXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cmpssXmmE32(op); }); }

    void dynamic_comissXmmXmm(DecodedOp* op) override;
    void dynamic_comissXmmE32(DecodedOp* op) override;
    void dynamic_ucomissXmmXmm(DecodedOp* op) override;
    void dynamic_ucomissXmmE32(DecodedOp* op) override;

    void dynamic_stmxcsr(DecodedOp* op) override;
    void dynamic_ldmxcsr(DecodedOp* op) override;
    void dynamic_sfence(DecodedOp* op) override;
};

#endif