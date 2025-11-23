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
	using XmmImmCallback = void(DynamicCodeGenSSE::*)(DynXMMReg dst, U32 imm);

	DynamicCodeGenSSE(CPU* cpu) : DynamicCodeGenMMX(cpu) {}

	virtual DynXMMReg getTmpXMM(U8 inUse) = 0;
	virtual void storeCpuXMMReg(DynXMMReg reg, U32 index) = 0;
	virtual void loadCpuXMMReg(DynXMMReg reg, U32 index) = 0;
	virtual void loadCpuXMMReg64ZeroExtend(DynXMMReg reg, U32 index) = 0;
	virtual void loadXMMFromMem128(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
	virtual void loadXMMFromMem32(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
	virtual void loadXMMFromMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
	virtual void loadLowXMMFromMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
	virtual void loadHighXMMFromMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
	virtual void storeXMMToMem128(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
	virtual void storeXMMToMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
	virtual void storeXMMToMem32(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
	virtual void storeHighXMMToMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;

	void opXmmXmm(DecodedOp* op, XmmXmmCallback callback, bool loadDest = true);
	void opXmmXmmImm(DecodedOp* op, XmmXmmImmCallback callback);
	void opXmmImm(DecodedOp* op, XmmImmCallback callback);
	void opXmmE128(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback, bool loadDest = true);
	void opXmmE128Imm(DecodedOp* op, XmmXmmImmCallback callback, std::function<void()> fallback);
	void opXmmE64(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback, bool loadDest = true);
	void opXmmE64Imm(DecodedOp* op, XmmXmmImmCallback callback, std::function<void()> fallback);
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
	virtual void cvtsi2ssXmmR32(DynXMMReg dst, RegPtr src) = 0;
	virtual void cvtss2siR32Xmm(RegPtr dst, DynXMMReg src) = 0;
	virtual void cvttps2piMmxXmm(DynMMXReg dst, DynXMMReg src) = 0;
	virtual void cvttss2siR32Xmm(RegPtr dst, DynXMMReg src) = 0;
	virtual void movhlpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void movlhpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void movmskpsR32Xmm(RegPtr dst, DynXMMReg src) = 0;
	virtual void movssXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void shufpsXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) = 0;
	virtual void unpckhpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void unpcklpsXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cmppsXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) = 0;
	virtual void cmpssXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) = 0;
	virtual void comissXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void ucomissXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void sfence() = 0;
	virtual void stmxcsr(RegPtr address) = 0;
	virtual void ldmxcsr(RegPtr address) = 0;

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

	// SSE2
	virtual void addpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void addsdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void subpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void subsdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void mulpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void mulsdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void divpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void divsdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void maxpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void maxsdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void minpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void minsdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void paddbXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void paddwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void padddXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void paddqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void paddsbXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void paddswXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void paddusbXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void padduswXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psubbXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psubwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psubdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psubqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psubsbXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psubswXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psubusbXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psubuswXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pmaddwdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pmulhwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pmullwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pmuludqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void sqrtpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void sqrtsdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void andnpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void andpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pandXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pandnXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void porXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pslldqXmm(DynXMMReg dst, U32 imm) = 0;
	virtual void psllqXmm(DynXMMReg dst, U32 imm) = 0;
	virtual void pslldXmm(DynXMMReg dst, U32 imm) = 0;
	virtual void psllwXmm(DynXMMReg dst, U32 imm) = 0;
	virtual void psradXmm(DynXMMReg dst, U32 imm) = 0;
	virtual void psrawXmm(DynXMMReg dst, U32 imm) = 0;
	virtual void psrldqXmm(DynXMMReg dst, U32 imm) = 0;
	virtual void psrlqXmm(DynXMMReg dst, U32 imm) = 0;
	virtual void psrldXmm(DynXMMReg dst, U32 imm) = 0;
	virtual void psrlwXmm(DynXMMReg dst, U32 imm) = 0;
	virtual void psllqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pslldXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psllwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psradXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psrawXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psrlqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psrldXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psrlwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pxorXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void orpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void xorpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cmppdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) = 0;
	virtual void cmpsdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) = 0;
	virtual void comisdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void ucomisdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pcmpgtbXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pcmpgtwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pcmpgtdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pcmpeqbXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pcmpeqwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pcmpeqdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cvtdq2pdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cvtdq2psXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cvtpd2piMmxXmm(DynMMXReg dst, DynXMMReg src) = 0;
	virtual void cvtpi2pdXmmMmx(DynXMMReg dst, DynMMXReg src) = 0;
	virtual void cvtpd2dqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cvtpd2psXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cvttpd2piMmxXmm(DynMMXReg dst, DynXMMReg src) = 0;
	virtual void cvtps2dqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cvtps2pdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cvtsd2siR32Xmm(RegPtr dst, DynXMMReg src) = 0;
	virtual void cvtsd2ssXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cvtsi2sdXmmR32(DynXMMReg dst, RegPtr src) = 0;
	virtual void cvtss2sdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cvttpd2dqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cvttps2dqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void cvttsd2siR32Xmm(RegPtr dst, DynXMMReg src) = 0;
	virtual void movsdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void movupdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void movmskpd(RegPtr dst, DynXMMReg src) = 0;
	virtual void movd(RegPtr dst, DynXMMReg src) = 0;
	virtual void movd(DynXMMReg dst, RegPtr src) = 0;
	virtual void movdq2q(DynMMXReg dst, DynXMMReg src) = 0;
	virtual void movq2dq(DynXMMReg dst, DynMMXReg src) = 0;

	virtual void maskmovdqu(DynXMMReg dst, DynXMMReg src, RegPtr address) = 0;
	virtual void pshufdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) = 0;
	virtual void pshufhwXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) = 0;
	virtual void pshuflwXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) = 0;
	virtual void shufpdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) = 0;
	virtual void unpckhpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void unpcklpdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void punpckhbwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void punpckhwdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void punpckhdqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void punpckhqdqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void punpcklbwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void punpcklwdXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void punpckldqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void punpcklqdqXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void packssdwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void packsswbXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void packuswbXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pavgbXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pavgwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void psadbwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pmaxswXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pmaxubXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pminswXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pminubXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void pmulhuwXmmXmm(DynXMMReg dst, DynXMMReg src) = 0;
	virtual void lfence() = 0;
	virtual void mfence() = 0;
	virtual void clflush(RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
	virtual void pause() = 0;
	virtual void pextrwR32Xmm(RegPtr dst, DynXMMReg src, U32 imm) = 0;
	virtual void pinsrwXmmR32(DynXMMReg dst, RegPtr src, U32 imm) = 0;
	virtual void pmovmskbR32Xmm(RegPtr dst, DynXMMReg src) = 0;

	void dynamic_addpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::addpdXmmXmm); }
	void dynamic_addpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::addpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_addpdXmmE128(op); }); }
	void dynamic_addsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::addsdXmmXmm); }
	void dynamic_addsdXmmE64(DecodedOp* op) override { opXmmE64(op, &DynamicCodeGenSSE::addsdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_addsdXmmE64(op); }); }
	void dynamic_subpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::subpdXmmXmm); }
	void dynamic_subpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::subpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_subpdXmmE128(op); }); }
	void dynamic_subsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::subsdXmmXmm); }
	void dynamic_subsdXmmE64(DecodedOp* op) override { opXmmE64(op, &DynamicCodeGenSSE::subsdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_subsdXmmE64(op); }); }
	void dynamic_mulpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::mulpdXmmXmm); }
	void dynamic_mulpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::mulpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_mulpdXmmE128(op); }); }
	void dynamic_mulsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::mulsdXmmXmm); }
	void dynamic_mulsdXmmE64(DecodedOp* op) override { opXmmE64(op, &DynamicCodeGenSSE::mulsdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_mulsdXmmE64(op); }); }
	void dynamic_divpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::divpdXmmXmm); }
	void dynamic_divpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::divpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_divpdXmmE128(op); }); }
	void dynamic_divsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::divsdXmmXmm); }
	void dynamic_divsdXmmE64(DecodedOp* op) override { opXmmE64(op, &DynamicCodeGenSSE::divsdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_divsdXmmE64(op); }); }
	void dynamic_maxpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::maxpdXmmXmm); }
	void dynamic_maxpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::maxpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_maxpdXmmE128(op); }); }
	void dynamic_maxsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::maxsdXmmXmm); }
	void dynamic_maxsdXmmE64(DecodedOp* op) override { opXmmE64(op, &DynamicCodeGenSSE::maxsdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_maxsdXmmE64(op); }); }
	void dynamic_minpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::minpdXmmXmm); }
	void dynamic_minpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::minpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_minpdXmmE128(op); }); }
	void dynamic_minsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::minsdXmmXmm); }
	void dynamic_minsdXmmE64(DecodedOp* op) override { opXmmE64(op, &DynamicCodeGenSSE::minsdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_minsdXmmE64(op); }); }
	void dynamic_sqrtpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::sqrtpdXmmXmm); }
	void dynamic_sqrtpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::sqrtpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_sqrtpdXmmE128(op); }); }
	void dynamic_sqrtsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::sqrtsdXmmXmm); }
	void dynamic_sqrtsdXmmE64(DecodedOp* op) override { opXmmE64(op, &DynamicCodeGenSSE::sqrtsdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_sqrtsdXmmE64(op); }); }
	void dynamic_paddbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::paddbXmmXmm); }
	void dynamic_paddbXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::paddbXmmXmm, [op, this]() {DynamicCodeGen::dynamic_paddbXmmE128(op); }); }
	void dynamic_paddwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::paddwXmmXmm); }
	void dynamic_paddwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::paddwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_paddwXmmE128(op); }); }
	void dynamic_padddXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::padddXmmXmm); }
	void dynamic_padddXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::padddXmmXmm, [op, this]() {DynamicCodeGen::dynamic_padddXmmE128(op); }); }
	void dynamic_paddqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::paddqXmmXmm); }
	void dynamic_paddqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::paddqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_paddqXmmE128(op); }); }
	void dynamic_paddsbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::paddsbXmmXmm); }
	void dynamic_paddsbXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::paddsbXmmXmm, [op, this]() {DynamicCodeGen::dynamic_paddsbXmmE128(op); }); }
	void dynamic_paddswXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::paddswXmmXmm); }
	void dynamic_paddswXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::paddswXmmXmm, [op, this]() {DynamicCodeGen::dynamic_paddswXmmE128(op); }); }
	void dynamic_paddusbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::paddusbXmmXmm); }
	void dynamic_paddusbXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::paddusbXmmXmm, [op, this]() {DynamicCodeGen::dynamic_paddusbXmmE128(op); }); }
	void dynamic_padduswXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::padduswXmmXmm); }
	void dynamic_padduswXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::padduswXmmXmm, [op, this]() {DynamicCodeGen::dynamic_padduswXmmE128(op); }); }
	void dynamic_psubbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psubbXmmXmm); }
	void dynamic_psubbXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psubbXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psubbXmmE128(op); }); }
	void dynamic_psubwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psubwXmmXmm); }
	void dynamic_psubwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psubwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psubwXmmE128(op); }); }
	void dynamic_psubdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psubdXmmXmm); }
	void dynamic_psubdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psubdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psubdXmmE128(op); }); }
	void dynamic_psubqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psubqXmmXmm); }
	void dynamic_psubqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psubqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psubqXmmE128(op); }); }
	void dynamic_psubsbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psubsbXmmXmm); }
	void dynamic_psubsbXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psubsbXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psubsbXmmE128(op); }); }
	void dynamic_psubswXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psubswXmmXmm); }
	void dynamic_psubswXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psubswXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psubswXmmE128(op); }); }
	void dynamic_psubusbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psubusbXmmXmm); }
	void dynamic_psubusbXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psubusbXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psubusbXmmE128(op); }); }
	void dynamic_psubuswXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psubuswXmmXmm); }
	void dynamic_psubuswXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psubuswXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psubuswXmmE128(op); }); }
	void dynamic_pmaddwdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pmaddwdXmmXmm); }
	void dynamic_pmaddwdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pmaddwdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pmaddwdXmmE128(op); }); }
	void dynamic_pmulhwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pmulhwXmmXmm); }
	void dynamic_pmulhwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pmulhwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pmulhwXmmE128(op); }); }
	void dynamic_pmullwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pmullwXmmXmm); }
	void dynamic_pmullwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pmullwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pmullwXmmE128(op); }); }
	void dynamic_pmuludqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pmuludqXmmXmm); }
	void dynamic_pmuludqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pmuludqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pmuludqXmmE128(op); }); }
	void dynamic_andnpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::andnpdXmmXmm); }
	void dynamic_andnpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::andnpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_andnpdXmmE128(op); }); }
	void dynamic_andpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::andpdXmmXmm); }
	void dynamic_andpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::andpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_andpdXmmE128(op); }); }
	void dynamic_pandXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pandXmmXmm); }
	void dynamic_pandXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pandXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pandXmmE128(op); }); }
	void dynamic_pandnXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pandnXmmXmm); }
	void dynamic_pandnXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pandnXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pandnXmmE128(op); }); }
	void dynamic_porXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::porXmmXmm); }
	void dynamic_porXmmXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::porXmmXmm, [op, this]() {DynamicCodeGen::dynamic_porXmmXmmE128(op); }); }
	void dynamic_pslldqXmm(DecodedOp* op) override { opXmmImm(op, &DynamicCodeGenSSE::pslldqXmm); }
	void dynamic_psllqXmm(DecodedOp* op) override { opXmmImm(op, &DynamicCodeGenSSE::psllqXmm); }
	void dynamic_pslldXmm(DecodedOp* op) override { opXmmImm(op, &DynamicCodeGenSSE::pslldXmm); }
	void dynamic_psllwXmm(DecodedOp* op) override { opXmmImm(op, &DynamicCodeGenSSE::psllwXmm); }
	void dynamic_psradXmm(DecodedOp* op) override { opXmmImm(op, &DynamicCodeGenSSE::psradXmm); }
	void dynamic_psrawXmm(DecodedOp* op) override { opXmmImm(op, &DynamicCodeGenSSE::psrawXmm); }
	void dynamic_psrldqXmm(DecodedOp* op) override { opXmmImm(op, &DynamicCodeGenSSE::psrldqXmm); }
	void dynamic_psrlqXmm(DecodedOp* op) override { opXmmImm(op, &DynamicCodeGenSSE::psrlqXmm); }
	void dynamic_psrldXmm(DecodedOp* op) override { opXmmImm(op, &DynamicCodeGenSSE::psrldXmm); }
	void dynamic_psrlwXmm(DecodedOp* op) override { opXmmImm(op, &DynamicCodeGenSSE::psrlwXmm); }
	void dynamic_psllqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psllqXmmXmm); }
	void dynamic_psllqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psllqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psllqXmmE128(op); }); }
	void dynamic_pslldXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pslldXmmXmm); }
	void dynamic_pslldXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pslldXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pslldXmmE128(op); }); }
	void dynamic_psllwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psllwXmmXmm); }
	void dynamic_psllwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psllwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psllwXmmE128(op); }); }
	void dynamic_psradXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psradXmmXmm); }
	void dynamic_psradXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psradXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psradXmmE128(op); }); }
	void dynamic_psrawXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psrawXmmXmm); }
	void dynamic_psrawXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psrawXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psrawXmmE128(op); }); }
	void dynamic_psrlqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psrlqXmmXmm); }
	void dynamic_psrlqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psrlqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psrlqXmmE128(op); }); }
	void dynamic_psrldXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psrldXmmXmm); }
	void dynamic_psrldXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psrldXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psrldXmmE128(op); }); }
	void dynamic_psrlwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psrlwXmmXmm); }
	void dynamic_psrlwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psrlwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psrlwXmmE128(op); }); }
	void dynamic_pxorXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pxorXmmXmm); }
	void dynamic_pxorXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pxorXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pxorXmmE128(op); }); }
	void dynamic_orpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::orpdXmmXmm); }
	void dynamic_orpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::orpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_orpdXmmE128(op); }); }
	void dynamic_xorpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::xorpdXmmXmm); }
	void dynamic_xorpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::xorpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_xorpdXmmE128(op); }); }
	void dynamic_cmppdXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &DynamicCodeGenSSE::cmppdXmmXmm); }
	void dynamic_cmppdXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &DynamicCodeGenSSE::cmppdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cmppdXmmE128(op); }); }
	void dynamic_cmpsdXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &DynamicCodeGenSSE::cmpsdXmmXmm); }
	void dynamic_cmpsdXmmE64(DecodedOp* op) override { opXmmE64Imm(op, &DynamicCodeGenSSE::cmpsdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cmpsdXmmE64(op); }); }
	void dynamic_comisdXmmXmm(DecodedOp* op) override;
	void dynamic_comisdXmmE64(DecodedOp* op) override;
	void dynamic_ucomisdXmmXmm(DecodedOp* op) override;
	void dynamic_ucomisdXmmE64(DecodedOp* op) override;
	void dynamic_pcmpgtbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pcmpgtbXmmXmm); }
	void dynamic_pcmpgtbXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pcmpgtbXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pcmpgtbXmmE128(op); }); }
	void dynamic_pcmpgtwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pcmpgtwXmmXmm); }
	void dynamic_pcmpgtwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pcmpgtwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pcmpgtwXmmE128(op); }); }
	void dynamic_pcmpgtdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pcmpgtdXmmXmm); }
	void dynamic_pcmpgtdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pcmpgtdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pcmpgtdXmmE128(op); }); }
	void dynamic_pcmpeqbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pcmpeqbXmmXmm); }
	void dynamic_pcmpeqbXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pcmpeqbXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pcmpeqbXmmE128(op); }); }
	void dynamic_pcmpeqwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pcmpeqwXmmXmm); }
	void dynamic_pcmpeqwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pcmpeqwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pcmpeqwXmmE128(op); }); }
	void dynamic_pcmpeqdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pcmpeqdXmmXmm); }
	void dynamic_pcmpeqdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pcmpeqdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pcmpeqdXmmE128(op); }); }

	void dynamic_cvtdq2pdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::cvtdq2pdXmmXmm, false); }
	void dynamic_cvtdq2pdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::cvtdq2pdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cvtdq2pdXmmE128(op); }, false); }
	void dynamic_cvtdq2psXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::cvtdq2psXmmXmm, false); }
	void dynamic_cvtdq2psXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::cvtdq2psXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cvtdq2psXmmE128(op); }, false); }
	void dynamic_cvtpd2dqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::cvtpd2dqXmmXmm, false); } // top bits in dest are zero'd
	void dynamic_cvtpd2dqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::cvtpd2dqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cvtpd2dqXmmE128(op); }, false); }
	void dynamic_cvtpd2psXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::cvtpd2psXmmXmm, false); } // top bits in dest are zero'd
	void dynamic_cvtpd2psXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::cvtpd2psXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cvtpd2psXmmE128(op); }, false); }
	void dynamic_cvtpd2piMmxXmm(DecodedOp* op) override;
	void dynamic_cvtpd2piMmxE128(DecodedOp* op) override;
	void dynamic_cvtpi2pdXmmMmx(DecodedOp* op) override;
	void dynamic_cvtpi2pdXmmE64(DecodedOp* op) override;
	void dynamic_cvttpd2piMmxXmm(DecodedOp* op) override;
	void dynamic_cvttpd2piMmE128(DecodedOp* op) override;

	void dynamic_cvtsd2siR32Xmm(DecodedOp* op) override;
	void dynamic_cvtsd2siR32E64(DecodedOp* op) override;
	void dynamic_cvtsi2sdXmmR32(DecodedOp* op) override;
	void dynamic_cvtsi2sdXmmE32(DecodedOp* op) override;
	void dynamic_cvttsd2siR32Xmm(DecodedOp* op) override;
	void dynamic_cvttsd2siR32E64(DecodedOp* op) override;
	void dynamic_cvtps2dqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::cvtps2dqXmmXmm, false); }
	void dynamic_cvtps2dqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::cvtps2dqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cvtpd2psXmmE128(op); }, false); }
	void dynamic_cvtps2pdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::cvtps2pdXmmXmm, false); }
	void dynamic_cvtps2pdXmmE64(DecodedOp* op) override { opXmmE64(op, &DynamicCodeGenSSE::cvtps2pdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cvtps2pdXmmE64(op); }, false); }
	void dynamic_cvtsd2ssXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::cvtsd2ssXmmXmm); }
	void dynamic_cvtsd2ssXmmE64(DecodedOp* op) override { opXmmE64(op, &DynamicCodeGenSSE::cvtsd2ssXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cvtsd2ssXmmE64(op); }); }
	void dynamic_cvtss2sdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::cvtss2sdXmmXmm); }
	void dynamic_cvtss2sdXmmE32(DecodedOp* op) override { opXmmE32(op, &DynamicCodeGenSSE::cvtss2sdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cvtss2sdXmmE32(op); }); }
	void dynamic_cvttpd2dqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::cvttpd2dqXmmXmm); }
	void dynamic_cvttpd2dqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::cvttpd2dqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cvttpd2dqXmmE128(op); }); }
	void dynamic_cvttps2dqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::cvttps2dqXmmXmm, false); }
	void dynamic_cvttps2dqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::cvttps2dqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_cvttps2dqXmmE128(op); }, false); }

	void dynamic_movqXmmXmm(DecodedOp* op) override;
	void dynamic_movqE64Xmm(DecodedOp* op) override;
	void dynamic_movqXmmE64(DecodedOp* op) override;
	void dynamic_movsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::movsdXmmXmm); } // top bits are preserved
	void dynamic_movsdXmmE64(DecodedOp* op) override; // // top bits are zero'd
	void dynamic_movsdE64Xmm(DecodedOp* op) override;
	void dynamic_movapdXmmXmm(DecodedOp* op) override { dynamic_movupdXmmXmm(op); }
	void dynamic_movapdXmmE128(DecodedOp* op) override { dynamic_movupdXmmE128(op); }
	void dynamic_movapdE128Xmm(DecodedOp* op) override { dynamic_movupdE128Xmm(op); }
	void dynamic_movupdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::movupdXmmXmm, false); }
	void dynamic_movupdXmmE128(DecodedOp* op) override;
	void dynamic_movupdE128Xmm(DecodedOp* op) override;
	void dynamic_movhpdXmmE64(DecodedOp* op) override;
	void dynamic_movhpdE64Xmm(DecodedOp* op) override;
	void dynamic_movlpdXmmE64(DecodedOp* op) override;
	void dynamic_movlpdE64Xmm(DecodedOp* op) override;
	void dynamic_movmskpdR32Xmm(DecodedOp* op) override;
	void dynamic_movdXmmR32(DecodedOp* op) override;
	void dynamic_movdXmmE32(DecodedOp* op) override;
	void dynamic_movdR32Xmm(DecodedOp* op) override;
	void dynamic_movdE32Xmm(DecodedOp* op) override;

	void dynamic_movdqaXmmXmm(DecodedOp* op) override { dynamic_movdquXmmXmm(op); }
	void dynamic_movdqaXmmE128(DecodedOp* op) override { dynamic_movdquXmmE128(op); }
	void dynamic_movdqaE128Xmm(DecodedOp* op) override { dynamic_movdquE128Xmm(op); }
	void dynamic_movdquXmmXmm(DecodedOp* op) override { dynamic_movupdXmmXmm(op); }
	void dynamic_movdquXmmE128(DecodedOp* op) override { dynamic_movupdXmmE128(op); }
	void dynamic_movdquE128Xmm(DecodedOp* op) override { dynamic_movupdE128Xmm(op); }

	void dynamic_movdq2qMmxXmm(DecodedOp* op) override;
	void dynamic_movq2dqXmmMmx(DecodedOp* op) override;

	void dynamic_movntpdE128Xmm(DecodedOp* op) override { dynamic_movupdE128Xmm(op); }
	void dynamic_movntdqE128Xmm(DecodedOp* op) override { dynamic_movupdE128Xmm(op); }
	void dynamic_movntiE32R32(DecodedOp* op) override { dynamic_move32r32(op); }


	void dynamic_pshufdXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &DynamicCodeGenSSE::pshufdXmmXmm); }
	void dynamic_pshufdXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &DynamicCodeGenSSE::pshufdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pshufdXmmE128(op); }); }
	void dynamic_pshufhwXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &DynamicCodeGenSSE::pshufhwXmmXmm); }
	void dynamic_pshufhwXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &DynamicCodeGenSSE::pshufhwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pshufhwXmmE128(op); }); }
	void dynamic_pshuflwXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &DynamicCodeGenSSE::pshuflwXmmXmm); }
	void dynamic_pshuflwXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &DynamicCodeGenSSE::pshuflwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pshuflwXmmE128(op); }); }

	void dynamic_shufpdXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &DynamicCodeGenSSE::shufpdXmmXmm); }
	void dynamic_shufpdXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &DynamicCodeGenSSE::shufpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_shufpdXmmE128(op); }); }
	void dynamic_unpckhpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::unpckhpdXmmXmm); }
	void dynamic_unpckhpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::unpckhpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_unpckhpdXmmE128(op); }); }
	void dynamic_unpcklpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::unpcklpdXmmXmm); }

	void dynamic_unpcklpdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::unpcklpdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_unpcklpdXmmE128(op); }); }
	void dynamic_punpckhbwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::punpckhbwXmmXmm); }
	void dynamic_punpckhbwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::punpckhbwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_punpckhbwXmmE128(op); }); }
	void dynamic_punpckhwdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::punpckhwdXmmXmm); }
	void dynamic_punpckhwdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::punpckhwdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_punpckhwdXmmE128(op); }); }
	void dynamic_punpckhdqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::punpckhdqXmmXmm); }
	void dynamic_punpckhdqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::punpckhdqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_punpckhdqXmmE128(op); }); }
	void dynamic_punpckhqdqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::punpckhqdqXmmXmm); }
	void dynamic_punpckhqdqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::punpckhqdqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_punpckhqdqXmmE128(op); }); }
	void dynamic_punpcklbwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::punpcklbwXmmXmm); }
	void dynamic_punpcklbwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::punpcklbwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_punpcklbwXmmE128(op); }); }
	void dynamic_punpcklwdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::punpcklwdXmmXmm); }
	void dynamic_punpcklwdXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::punpcklwdXmmXmm, [op, this]() {DynamicCodeGen::dynamic_punpcklwdXmmE128(op); }); }
	void dynamic_punpckldqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::punpckldqXmmXmm); }
	void dynamic_punpckldqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::punpckldqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_punpckldqXmmE128(op); }); }
	void dynamic_punpcklqdqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::punpcklqdqXmmXmm); }
	void dynamic_punpcklqdqXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::punpcklqdqXmmXmm, [op, this]() {DynamicCodeGen::dynamic_punpcklqdqXmmE128(op); }); }
	void dynamic_packssdwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::packssdwXmmXmm); }
	void dynamic_packssdwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::packssdwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_packssdwXmmE128(op); }); }
	void dynamic_packsswbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::packsswbXmmXmm); }
	void dynamic_packsswbXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::packsswbXmmXmm, [op, this]() {DynamicCodeGen::dynamic_packsswbXmmE128(op); }); }
	void dynamic_packuswbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::packuswbXmmXmm); }
	void dynamic_packuswbXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::packuswbXmmXmm, [op, this]() {DynamicCodeGen::dynamic_packuswbXmmE128(op); }); }
	void dynamic_pause(DecodedOp* op) override { pause(); incrementEip(op->len); }
	void dynamic_pavgbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pavgbXmmXmm); }
	void dynamic_pavgbXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pavgbXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pavgbXmmE128(op); }); }
	void dynamic_pavgwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pavgwXmmXmm); }
	void dynamic_pavgwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pavgwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pavgwXmmE128(op); }); }
	void dynamic_psadbwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::psadbwXmmXmm); }
	void dynamic_psadbwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::psadbwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_psadbwXmmE128(op); }); }
	void dynamic_pmaxswXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pmaxswXmmXmm); }
	void dynamic_pmaxswXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pmaxswXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pmaxswXmmE128(op); }); }
	void dynamic_pmaxubXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pmaxubXmmXmm); }
	void dynamic_pmaxubXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pmaxubXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pmaxubXmmE128(op); }); }
	void dynamic_pminswXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pminswXmmXmm); }
	void dynamic_pminswXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pminswXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pminswXmmE128(op); }); }
	void dynamic_pminubXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pminubXmmXmm); }
	void dynamic_pminubXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pminubXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pminubXmmE128(op); }); }
	void dynamic_pmulhuwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &DynamicCodeGenSSE::pmulhuwXmmXmm); }
	void dynamic_pmulhuwXmmE128(DecodedOp* op) override { opXmmE128(op, &DynamicCodeGenSSE::pmulhuwXmmXmm, [op, this]() {DynamicCodeGen::dynamic_pmulhuwXmmE128(op); }); }
	void dynamic_pextrwR32Xmm(DecodedOp* op) override;
	void dynamic_pextrwE16Xmm(DecodedOp* op) override;
	void dynamic_pinsrwXmmR32(DecodedOp* op) override;
	void dynamic_pinsrwXmmE16(DecodedOp* op) override;
	void dynamic_pmovmskbR32Xmm(DecodedOp* op) override;
	void dynamic_maskmovdquE128XmmXmm(DecodedOp* op) override;
	void dynamic_lfence(DecodedOp* op) override { lfence(); incrementEip(op->len); }
	void dynamic_mfence(DecodedOp* op) override { mfence(); incrementEip(op->len); }
	void dynamic_clflush(DecodedOp* op) override;
};

#endif