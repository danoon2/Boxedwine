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

#ifndef __JIT_SSE_H__
#define __JIT_SSE_H__

#include "jitMMX.h"

class SSERegInternal {
public:
	SSERegInternal(U8 hardwareReg, U8 emulatedReg) : reg(hardwareReg), emulatedReg(emulatedReg) {}

	U8 hardwareReg() { return reg; }

	U8 emulatedReg;
private:
	U8 reg;
};

using SSERegPtr = std::shared_ptr<SSERegInternal>;

#define SSE_TMP_INDEX 0xff

// Implementation of JIT that is host instruction independent
class JitSSE : public JitMMX {
public:
	using XmmXmmCallback = void(JitSSE::*)(SSERegPtr dst, SSERegPtr src);
	using XmmXmmImmCallback = void(JitSSE::*)(SSERegPtr dst, SSERegPtr src, U32 imm);
	using XmmImmCallback = void(JitSSE::*)(SSERegPtr dst, U32 imm);

	JitSSE(CPU* cpu) : JitMMX(cpu) {}

	virtual SSERegPtr getTmpSSE() = 0;
	virtual bool isSseRegCached(U8 reg) = 0;
	virtual void storeCpuXMMReg(SSERegPtr reg, U32 index) = 0;
	virtual SSERegPtr loadCpuXMMReg(U8 index) = 0;
	virtual SSERegPtr loadXMMFromMem128(U8 index, RegPtr rm, RegPtr sib) = 0;
	virtual SSERegPtr loadXMMFromMem32(U8 index, RegPtr rm, RegPtr sib) = 0;
	virtual SSERegPtr loadXMMFromMem64(U8 index, RegPtr rm, RegPtr sib) = 0;
	virtual SSERegPtr loadLowXMMFromMem64(U8 index, RegPtr rm, RegPtr sib) = 0;
	virtual SSERegPtr loadHighXMMFromMem64(U8 index, RegPtr rm, RegPtr sib) = 0;
	virtual void storeXMMToMem128(SSERegPtr reg, RegPtr rm, RegPtr sib) = 0;
	virtual void storeXMMToMem64(SSERegPtr reg, RegPtr rm, RegPtr sib) = 0;
	virtual void storeXMMToMem32(SSERegPtr reg, RegPtr rm, RegPtr sib) = 0;
	virtual void storeHighXMMToMem64(SSERegPtr reg, RegPtr rm, RegPtr sib) = 0;

	void opXmmXmm(DecodedOp* op, XmmXmmCallback callback, bool loadDest = true);
	void opXmmXmmImm(DecodedOp* op, XmmXmmImmCallback callback);
	void opXmmImm(DecodedOp* op, XmmImmCallback callback);
	void opXmmE128(DecodedOp* op, XmmXmmCallback callback, bool loadDest = true);
	void opXmmE128Imm(DecodedOp* op, XmmXmmImmCallback callback);
	void opXmmE64(DecodedOp* op, XmmXmmCallback callback, bool loadDest = true);
	void opXmmE64Imm(DecodedOp* op, XmmXmmImmCallback callback);
	void opXmmE32(DecodedOp* op, XmmXmmCallback callback);
	void opXmmE32Imm(DecodedOp* op, XmmXmmImmCallback callback);

	virtual void addpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void addssXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void subpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void subssXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void mulpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void mulssXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void divpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void divssXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void rcppsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void rcpssXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void sqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void sqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void rsqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void rsqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void maxpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void maxssXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void minpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void minssXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void andnpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void andpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void orpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void xorpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cvtpi2psXmmMmx(SSERegPtr dst, MMXRegPtr src) = 0;
	virtual void cvtps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) = 0;
	virtual void cvtsi2ssXmmR32(SSERegPtr dst, RegPtr src) = 0;
	virtual void cvtss2siR32Xmm(RegPtr dst, SSERegPtr src) = 0;
	virtual void cvttps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) = 0;
	virtual void cvttss2siR32Xmm(RegPtr dst, SSERegPtr src) = 0;
	virtual void movhlpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void movlhpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void movmskpsR32Xmm(RegPtr dst, SSERegPtr src) = 0;
	virtual void movssXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void shufpsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) = 0;
	virtual void unpckhpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void unpcklpsXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cmppsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) = 0;
	virtual void cmpssXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) = 0;
	virtual void comissXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void ucomissXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void sfence() = 0;
	virtual void stmxcsr(RegPtr address) = 0;
	virtual void ldmxcsr(RegPtr address) = 0;

	void dynamic_addpsXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::addpsXmmXmm); }
	void dynamic_addpsE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::addpsXmmXmm); }
	void dynamic_addssXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::addssXmmXmm); }
	void dynamic_addssE32(DecodedOp* op) override { opXmmE32(op, &JitSSE::addssXmmXmm); }
	void dynamic_subpsXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::subpsXmmXmm); }
	void dynamic_subpsE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::subpsXmmXmm); }
	void dynamic_subssXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::subssXmmXmm); }
	void dynamic_subssE32(DecodedOp* op) override { opXmmE32(op, &JitSSE::subssXmmXmm); }
	void dynamic_mulpsXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::mulpsXmmXmm); }
	void dynamic_mulpsE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::mulpsXmmXmm); }
	void dynamic_mulssXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::mulssXmmXmm); }
	void dynamic_mulssE32(DecodedOp* op) override { opXmmE32(op, &JitSSE::mulssXmmXmm); }
	void dynamic_divpsXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::divpsXmmXmm); }
	void dynamic_divpsE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::divpsXmmXmm); }
	void dynamic_divssXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::divssXmmXmm); }
	void dynamic_divssE32(DecodedOp* op) override { opXmmE32(op, &JitSSE::divssXmmXmm); }
	void dynamic_rcppsXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::rcppsXmmXmm); }
	void dynamic_rcppsE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::rcppsXmmXmm); }
	void dynamic_rcpssXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::rcpssXmmXmm); }
	void dynamic_rcpssE32(DecodedOp* op) override { opXmmE32(op, &JitSSE::rcpssXmmXmm); }
	void dynamic_sqrtpsXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::sqrtpsXmmXmm); }
	void dynamic_sqrtpsE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::sqrtpsXmmXmm); }
	void dynamic_sqrtssXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::sqrtssXmmXmm); }
	void dynamic_sqrtssE32(DecodedOp* op) override { opXmmE32(op, &JitSSE::sqrtssXmmXmm); }
	void dynamic_rsqrtpsXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::rsqrtpsXmmXmm); }
	void dynamic_rsqrtpsE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::rsqrtpsXmmXmm); }
	void dynamic_rsqrtssXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::rsqrtssXmmXmm); }
	void dynamic_rsqrtssE32(DecodedOp* op) override { opXmmE32(op, &JitSSE::rsqrtssXmmXmm); }
	void dynamic_maxpsXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::maxpsXmmXmm); }
	void dynamic_maxpsE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::maxpsXmmXmm); }
	void dynamic_maxssXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::maxssXmmXmm); }
	void dynamic_maxssE32(DecodedOp* op) override { opXmmE32(op, &JitSSE::maxssXmmXmm); }
	void dynamic_minpsXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::minpsXmmXmm); }
	void dynamic_minpsE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::minpsXmmXmm); }
	void dynamic_minssXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::minssXmmXmm); }
	void dynamic_minssE32(DecodedOp* op) override { opXmmE32(op, &JitSSE::minssXmmXmm); }

	void dynamic_andnpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::andnpsXmmXmm); }
	void dynamic_andnpsXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::andnpsXmmXmm); }
	void dynamic_andpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::andpsXmmXmm); }
	void dynamic_andpsXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::andpsXmmXmm); }
	void dynamic_orpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::orpsXmmXmm); }
	void dynamic_orpsXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::orpsXmmXmm); }
	void dynamic_xorpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::xorpsXmmXmm); }
	void dynamic_xorpsXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::xorpsXmmXmm); }

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
	void dynamic_movhlpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::movhlpsXmmXmm); }
	void dynamic_movlhpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::movlhpsXmmXmm); }
	void dynamic_movhpsXmmE64(DecodedOp* op) override;
	void dynamic_movhpsE64Xmm(DecodedOp* op) override;
	void dynamic_movlpsXmmE64(DecodedOp* op) override;
	void dynamic_movlpsE64Xmm(DecodedOp* op) override;
	void dynamic_movmskpsR32Xmm(DecodedOp* op) override;
	void dynamic_movssXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::movssXmmXmm); }
	void dynamic_movssXmmE32(DecodedOp* op) override;
	void dynamic_movssE32Xmm(DecodedOp* op) override;
	void dynamic_movntpsE128Xmm(DecodedOp* op) override { dynamic_movupsE128Xmm(op); }

	void dynamic_shufpsXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &JitSSE::shufpsXmmXmm); }
	void dynamic_shufpsXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &JitSSE::shufpsXmmXmm); }
	void dynamic_unpckhpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::unpckhpsXmmXmm); }
	void dynamic_unpckhpsXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::unpckhpsXmmXmm); }
	void dynamic_unpcklpsXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::unpcklpsXmmXmm); }
	void dynamic_unpcklpsXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::unpcklpsXmmXmm); }

	void dynamic_prefetchT0(DecodedOp* op) override {}
	void dynamic_prefetchT1(DecodedOp* op) override {}
	void dynamic_prefetchT2(DecodedOp* op) override {}
	void dynamic_prefetchNTA(DecodedOp* op) override {}

	void dynamic_cmppsXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &JitSSE::cmppsXmmXmm); }
	void dynamic_cmppsXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &JitSSE::cmppsXmmXmm); }

	void dynamic_cmpssXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &JitSSE::cmpssXmmXmm); }
	void dynamic_cmpssXmmE32(DecodedOp* op) override { opXmmE32Imm(op, &JitSSE::cmpssXmmXmm); }

	void dynamic_comissXmmXmm(DecodedOp* op) override;
	void dynamic_comissXmmE32(DecodedOp* op) override;
	void dynamic_ucomissXmmXmm(DecodedOp* op) override;
	void dynamic_ucomissXmmE32(DecodedOp* op) override;

	void dynamic_stmxcsr(DecodedOp* op) override;
	void dynamic_ldmxcsr(DecodedOp* op) override;
	void dynamic_sfence(DecodedOp* op) override;

	// SSE2
#ifdef BOXEDWINE_64
	virtual void cvtsi2sdXmmR64(SSERegPtr dst, RegPtr src) = 0;
#endif
	virtual void addpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void addsdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void subpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void subsdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void mulpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void mulsdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void divpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void divsdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void maxpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void maxsdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void minpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void minsdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void paddbXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void paddwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void padddXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void paddqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void paddsbXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void paddswXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void paddusbXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void padduswXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psubbXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psubwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psubdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psubqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psubsbXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psubswXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psubusbXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psubuswXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pmaddwdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pmulhwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pmullwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pmuludqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void sqrtpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void sqrtsdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void andnpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void andpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pandXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pandnXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void porXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pslldqXmm(SSERegPtr dst, U32 imm) = 0;
	virtual void psllqXmm(SSERegPtr dst, U32 imm) = 0;
	virtual void pslldXmm(SSERegPtr dst, U32 imm) = 0;
	virtual void psllwXmm(SSERegPtr dst, U32 imm) = 0;
	virtual void psradXmm(SSERegPtr dst, U32 imm) = 0;
	virtual void psrawXmm(SSERegPtr dst, U32 imm) = 0;
	virtual void psrldqXmm(SSERegPtr dst, U32 imm) = 0;
	virtual void psrlqXmm(SSERegPtr dst, U32 imm) = 0;
	virtual void psrldXmm(SSERegPtr dst, U32 imm) = 0;
	virtual void psrlwXmm(SSERegPtr dst, U32 imm) = 0;
	virtual void psllqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pslldXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psllwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psradXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psrawXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psrlqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psrldXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psrlwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pxorXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void orpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void xorpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cmppdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) = 0;
	virtual void cmpsdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) = 0;
	virtual void comisdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void ucomisdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pcmpgtbXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pcmpgtwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pcmpgtdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pcmpeqbXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pcmpeqwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pcmpeqdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cvtdq2pdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cvtdq2psXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cvtpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) = 0;
	virtual void cvtpi2pdXmmMmx(SSERegPtr dst, MMXRegPtr src) = 0;
	virtual void cvtpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cvtpd2psXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cvttpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) = 0;
	virtual void cvtps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cvtps2pdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cvtsd2siR32Xmm(RegPtr dst, SSERegPtr src) = 0;
	virtual void cvtsd2ssXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cvtsi2sdXmmR32(SSERegPtr dst, RegPtr src) = 0;
	virtual void cvtss2sdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cvttpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cvttps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void cvttsd2siR32Xmm(RegPtr dst, SSERegPtr src) = 0;
	virtual void movsdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void movupdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void movmskpd(RegPtr dst, SSERegPtr src) = 0;
	virtual void movd(RegPtr dst, SSERegPtr src) = 0;
	virtual void movd(SSERegPtr dst, RegPtr src) = 0;
	virtual void movdq2q(MMXRegPtr dst, SSERegPtr src) = 0;
	virtual void movq2dq(SSERegPtr dst, MMXRegPtr src) = 0;
	virtual void movq(SSERegPtr dst, SSERegPtr src) = 0;

	virtual void maskmovdqu(SSERegPtr dst, SSERegPtr src, RegPtr address) = 0;
	virtual void pshufdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) = 0;
	virtual void pshufhwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) = 0;
	virtual void pshuflwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) = 0;
	virtual void shufpdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) = 0;
	virtual void unpckhpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void unpcklpdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void punpckhbwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void punpckhwdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void punpckhdqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void punpckhqdqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void punpcklbwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void punpcklwdXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void punpckldqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void punpcklqdqXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void packssdwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void packsswbXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void packuswbXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pavgbXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pavgwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void psadbwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pmaxswXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pmaxubXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pminswXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pminubXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void pmulhuwXmmXmm(SSERegPtr dst, SSERegPtr src) = 0;
	virtual void lfence() = 0;
	virtual void mfence() = 0;
	virtual void clflush(RegPtr address) = 0;
	virtual void pause() = 0;
	virtual void pextrwR32Xmm(RegPtr dst, SSERegPtr src, U32 imm) = 0;
	virtual void pinsrwXmmR32(SSERegPtr dst, RegPtr src, U32 imm) = 0;
	virtual void pmovmskbR32Xmm(RegPtr dst, SSERegPtr src) = 0;

	void dynamic_addpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::addpdXmmXmm); }
	void dynamic_addpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::addpdXmmXmm); }
	void dynamic_addsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::addsdXmmXmm); }
	void dynamic_addsdXmmE64(DecodedOp* op) override { opXmmE64(op, &JitSSE::addsdXmmXmm); }
	void dynamic_subpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::subpdXmmXmm); }
	void dynamic_subpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::subpdXmmXmm); }
	void dynamic_subsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::subsdXmmXmm); }
	void dynamic_subsdXmmE64(DecodedOp* op) override { opXmmE64(op, &JitSSE::subsdXmmXmm); }
	void dynamic_mulpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::mulpdXmmXmm); }
	void dynamic_mulpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::mulpdXmmXmm); }
	void dynamic_mulsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::mulsdXmmXmm); }
	void dynamic_mulsdXmmE64(DecodedOp* op) override { opXmmE64(op, &JitSSE::mulsdXmmXmm); }
	void dynamic_divpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::divpdXmmXmm); }
	void dynamic_divpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::divpdXmmXmm); }
	void dynamic_divsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::divsdXmmXmm); }
	void dynamic_divsdXmmE64(DecodedOp* op) override { opXmmE64(op, &JitSSE::divsdXmmXmm); }
	void dynamic_maxpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::maxpdXmmXmm); }
	void dynamic_maxpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::maxpdXmmXmm); }
	void dynamic_maxsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::maxsdXmmXmm); }
	void dynamic_maxsdXmmE64(DecodedOp* op) override { opXmmE64(op, &JitSSE::maxsdXmmXmm); }
	void dynamic_minpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::minpdXmmXmm); }
	void dynamic_minpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::minpdXmmXmm); }
	void dynamic_minsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::minsdXmmXmm); }
	void dynamic_minsdXmmE64(DecodedOp* op) override { opXmmE64(op, &JitSSE::minsdXmmXmm); }
	void dynamic_sqrtpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::sqrtpdXmmXmm); }
	void dynamic_sqrtpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::sqrtpdXmmXmm); }
	void dynamic_sqrtsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::sqrtsdXmmXmm); }
	void dynamic_sqrtsdXmmE64(DecodedOp* op) override { opXmmE64(op, &JitSSE::sqrtsdXmmXmm); }
	void dynamic_paddbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::paddbXmmXmm); }
	void dynamic_paddbXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::paddbXmmXmm); }
	void dynamic_paddwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::paddwXmmXmm); }
	void dynamic_paddwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::paddwXmmXmm); }
	void dynamic_padddXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::padddXmmXmm); }
	void dynamic_padddXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::padddXmmXmm); }
	void dynamic_paddqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::paddqXmmXmm); }
	void dynamic_paddqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::paddqXmmXmm); }
	void dynamic_paddsbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::paddsbXmmXmm); }
	void dynamic_paddsbXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::paddsbXmmXmm); }
	void dynamic_paddswXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::paddswXmmXmm); }
	void dynamic_paddswXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::paddswXmmXmm); }
	void dynamic_paddusbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::paddusbXmmXmm); }
	void dynamic_paddusbXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::paddusbXmmXmm); }
	void dynamic_padduswXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::padduswXmmXmm); }
	void dynamic_padduswXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::padduswXmmXmm); }
	void dynamic_psubbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psubbXmmXmm); }
	void dynamic_psubbXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psubbXmmXmm); }
	void dynamic_psubwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psubwXmmXmm); }
	void dynamic_psubwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psubwXmmXmm); }
	void dynamic_psubdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psubdXmmXmm); }
	void dynamic_psubdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psubdXmmXmm); }
	void dynamic_psubqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psubqXmmXmm); }
	void dynamic_psubqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psubqXmmXmm); }
	void dynamic_psubsbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psubsbXmmXmm); }
	void dynamic_psubsbXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psubsbXmmXmm); }
	void dynamic_psubswXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psubswXmmXmm); }
	void dynamic_psubswXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psubswXmmXmm); }
	void dynamic_psubusbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psubusbXmmXmm); }
	void dynamic_psubusbXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psubusbXmmXmm); }
	void dynamic_psubuswXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psubuswXmmXmm); }
	void dynamic_psubuswXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psubuswXmmXmm); }
	void dynamic_pmaddwdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pmaddwdXmmXmm); }
	void dynamic_pmaddwdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pmaddwdXmmXmm); }
	void dynamic_pmulhwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pmulhwXmmXmm); }
	void dynamic_pmulhwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pmulhwXmmXmm); }
	void dynamic_pmullwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pmullwXmmXmm); }
	void dynamic_pmullwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pmullwXmmXmm); }
	void dynamic_pmuludqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pmuludqXmmXmm); }
	void dynamic_pmuludqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pmuludqXmmXmm); }
	void dynamic_andnpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::andnpdXmmXmm); }
	void dynamic_andnpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::andnpdXmmXmm); }
	void dynamic_andpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::andpdXmmXmm); }
	void dynamic_andpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::andpdXmmXmm); }
	void dynamic_pandXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pandXmmXmm); }
	void dynamic_pandXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pandXmmXmm); }
	void dynamic_pandnXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pandnXmmXmm); }
	void dynamic_pandnXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pandnXmmXmm); }
	void dynamic_porXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::porXmmXmm); }
	void dynamic_porXmmXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::porXmmXmm); }
	void dynamic_pslldqXmm(DecodedOp* op) override { opXmmImm(op, &JitSSE::pslldqXmm); }
	void dynamic_psllqXmm(DecodedOp* op) override { opXmmImm(op, &JitSSE::psllqXmm); }
	void dynamic_pslldXmm(DecodedOp* op) override { opXmmImm(op, &JitSSE::pslldXmm); }
	void dynamic_psllwXmm(DecodedOp* op) override { opXmmImm(op, &JitSSE::psllwXmm); }
	void dynamic_psradXmm(DecodedOp* op) override { opXmmImm(op, &JitSSE::psradXmm); }
	void dynamic_psrawXmm(DecodedOp* op) override { opXmmImm(op, &JitSSE::psrawXmm); }
	void dynamic_psrldqXmm(DecodedOp* op) override { opXmmImm(op, &JitSSE::psrldqXmm); }
	void dynamic_psrlqXmm(DecodedOp* op) override { opXmmImm(op, &JitSSE::psrlqXmm); }
	void dynamic_psrldXmm(DecodedOp* op) override { opXmmImm(op, &JitSSE::psrldXmm); }
	void dynamic_psrlwXmm(DecodedOp* op) override { opXmmImm(op, &JitSSE::psrlwXmm); }
	void dynamic_psllqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psllqXmmXmm); }
	void dynamic_psllqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psllqXmmXmm); }
	void dynamic_pslldXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pslldXmmXmm); }
	void dynamic_pslldXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pslldXmmXmm); }
	void dynamic_psllwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psllwXmmXmm); }
	void dynamic_psllwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psllwXmmXmm); }
	void dynamic_psradXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psradXmmXmm); }
	void dynamic_psradXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psradXmmXmm); }
	void dynamic_psrawXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psrawXmmXmm); }
	void dynamic_psrawXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psrawXmmXmm); }
	void dynamic_psrlqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psrlqXmmXmm); }
	void dynamic_psrlqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psrlqXmmXmm); }
	void dynamic_psrldXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psrldXmmXmm); }
	void dynamic_psrldXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psrldXmmXmm); }
	void dynamic_psrlwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psrlwXmmXmm); }
	void dynamic_psrlwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psrlwXmmXmm); }
	void dynamic_pxorXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pxorXmmXmm); }
	void dynamic_pxorXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pxorXmmXmm); }
	void dynamic_orpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::orpdXmmXmm); }
	void dynamic_orpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::orpdXmmXmm); }
	void dynamic_xorpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::xorpdXmmXmm); }
	void dynamic_xorpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::xorpdXmmXmm); }
	void dynamic_cmppdXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &JitSSE::cmppdXmmXmm); }
	void dynamic_cmppdXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &JitSSE::cmppdXmmXmm); }
	void dynamic_cmpsdXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &JitSSE::cmpsdXmmXmm); }
	void dynamic_cmpsdXmmE64(DecodedOp* op) override { opXmmE64Imm(op, &JitSSE::cmpsdXmmXmm); }
	void dynamic_comisdXmmXmm(DecodedOp* op) override;
	void dynamic_comisdXmmE64(DecodedOp* op) override;
	void dynamic_ucomisdXmmXmm(DecodedOp* op) override;
	void dynamic_ucomisdXmmE64(DecodedOp* op) override;
	void dynamic_pcmpgtbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pcmpgtbXmmXmm); }
	void dynamic_pcmpgtbXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pcmpgtbXmmXmm); }
	void dynamic_pcmpgtwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pcmpgtwXmmXmm); }
	void dynamic_pcmpgtwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pcmpgtwXmmXmm); }
	void dynamic_pcmpgtdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pcmpgtdXmmXmm); }
	void dynamic_pcmpgtdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pcmpgtdXmmXmm); }
	void dynamic_pcmpeqbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pcmpeqbXmmXmm); }
	void dynamic_pcmpeqbXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pcmpeqbXmmXmm); }
	void dynamic_pcmpeqwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pcmpeqwXmmXmm); }
	void dynamic_pcmpeqwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pcmpeqwXmmXmm); }
	void dynamic_pcmpeqdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pcmpeqdXmmXmm); }
	void dynamic_pcmpeqdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pcmpeqdXmmXmm); }

	void dynamic_cvtdq2pdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::cvtdq2pdXmmXmm, false); }
	void dynamic_cvtdq2pdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::cvtdq2pdXmmXmm, false); }
	void dynamic_cvtdq2psXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::cvtdq2psXmmXmm, false); }
	void dynamic_cvtdq2psXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::cvtdq2psXmmXmm, false); }
	void dynamic_cvtpd2dqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::cvtpd2dqXmmXmm, false); } // top bits in dest are zero'd
	void dynamic_cvtpd2dqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::cvtpd2dqXmmXmm, false); }
	void dynamic_cvtpd2psXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::cvtpd2psXmmXmm, false); } // top bits in dest are zero'd
	void dynamic_cvtpd2psXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::cvtpd2psXmmXmm, false); }
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
	void dynamic_cvtps2dqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::cvtps2dqXmmXmm, false); }
	void dynamic_cvtps2dqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::cvtps2dqXmmXmm, false); }
	void dynamic_cvtps2pdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::cvtps2pdXmmXmm, false); }
	void dynamic_cvtps2pdXmmE64(DecodedOp* op) override { opXmmE64(op, &JitSSE::cvtps2pdXmmXmm, false); }
	void dynamic_cvtsd2ssXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::cvtsd2ssXmmXmm); }
	void dynamic_cvtsd2ssXmmE64(DecodedOp* op) override { opXmmE64(op, &JitSSE::cvtsd2ssXmmXmm); }
	void dynamic_cvtss2sdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::cvtss2sdXmmXmm); }
	void dynamic_cvtss2sdXmmE32(DecodedOp* op) override { opXmmE32(op, &JitSSE::cvtss2sdXmmXmm); }
	void dynamic_cvttpd2dqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::cvttpd2dqXmmXmm); }
	void dynamic_cvttpd2dqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::cvttpd2dqXmmXmm); }
	void dynamic_cvttps2dqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::cvttps2dqXmmXmm, false); }
	void dynamic_cvttps2dqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::cvttps2dqXmmXmm, false); }

	void dynamic_movqXmmXmm(DecodedOp* op) override;
	void dynamic_movqE64Xmm(DecodedOp* op) override;
	void dynamic_movqXmmE64(DecodedOp* op) override;
	void dynamic_movsdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::movsdXmmXmm); } // top bits are preserved
	void dynamic_movsdXmmE64(DecodedOp* op) override; // // top bits are zero'd
	void dynamic_movsdE64Xmm(DecodedOp* op) override;
	void dynamic_movapdXmmXmm(DecodedOp* op) override { dynamic_movupdXmmXmm(op); }
	void dynamic_movapdXmmE128(DecodedOp* op) override { dynamic_movupdXmmE128(op); }
	void dynamic_movapdE128Xmm(DecodedOp* op) override { dynamic_movupdE128Xmm(op); }
	void dynamic_movupdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::movupdXmmXmm, false); }
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


	void dynamic_pshufdXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &JitSSE::pshufdXmmXmm); }
	void dynamic_pshufdXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &JitSSE::pshufdXmmXmm); }
	void dynamic_pshufhwXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &JitSSE::pshufhwXmmXmm); }
	void dynamic_pshufhwXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &JitSSE::pshufhwXmmXmm); }
	void dynamic_pshuflwXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &JitSSE::pshuflwXmmXmm); }
	void dynamic_pshuflwXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &JitSSE::pshuflwXmmXmm); }

	void dynamic_shufpdXmmXmm(DecodedOp* op) override { opXmmXmmImm(op, &JitSSE::shufpdXmmXmm); }
	void dynamic_shufpdXmmE128(DecodedOp* op) override { opXmmE128Imm(op, &JitSSE::shufpdXmmXmm); }
	void dynamic_unpckhpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::unpckhpdXmmXmm); }
	void dynamic_unpckhpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::unpckhpdXmmXmm); }
	void dynamic_unpcklpdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::unpcklpdXmmXmm); }

	void dynamic_unpcklpdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::unpcklpdXmmXmm); }
	void dynamic_punpckhbwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::punpckhbwXmmXmm); }
	void dynamic_punpckhbwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::punpckhbwXmmXmm); }
	void dynamic_punpckhwdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::punpckhwdXmmXmm); }
	void dynamic_punpckhwdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::punpckhwdXmmXmm); }
	void dynamic_punpckhdqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::punpckhdqXmmXmm); }
	void dynamic_punpckhdqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::punpckhdqXmmXmm); }
	void dynamic_punpckhqdqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::punpckhqdqXmmXmm); }
	void dynamic_punpckhqdqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::punpckhqdqXmmXmm); }
	void dynamic_punpcklbwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::punpcklbwXmmXmm); }
	void dynamic_punpcklbwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::punpcklbwXmmXmm); }
	void dynamic_punpcklwdXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::punpcklwdXmmXmm); }
	void dynamic_punpcklwdXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::punpcklwdXmmXmm); }
	void dynamic_punpckldqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::punpckldqXmmXmm); }
	void dynamic_punpckldqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::punpckldqXmmXmm); }
	void dynamic_punpcklqdqXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::punpcklqdqXmmXmm); }
	void dynamic_punpcklqdqXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::punpcklqdqXmmXmm); }
	void dynamic_packssdwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::packssdwXmmXmm); }
	void dynamic_packssdwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::packssdwXmmXmm); }
	void dynamic_packsswbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::packsswbXmmXmm); }
	void dynamic_packsswbXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::packsswbXmmXmm); }
	void dynamic_packuswbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::packuswbXmmXmm); }
	void dynamic_packuswbXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::packuswbXmmXmm); }
	void dynamic_pause(DecodedOp* op) override { pause(); }
	void dynamic_pavgbXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pavgbXmmXmm); }
	void dynamic_pavgbXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pavgbXmmXmm); }
	void dynamic_pavgwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pavgwXmmXmm); }
	void dynamic_pavgwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pavgwXmmXmm); }
	void dynamic_psadbwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::psadbwXmmXmm); }
	void dynamic_psadbwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::psadbwXmmXmm); }
	void dynamic_pmaxswXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pmaxswXmmXmm); }
	void dynamic_pmaxswXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pmaxswXmmXmm); }
	void dynamic_pmaxubXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pmaxubXmmXmm); }
	void dynamic_pmaxubXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pmaxubXmmXmm); }
	void dynamic_pminswXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pminswXmmXmm); }
	void dynamic_pminswXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pminswXmmXmm); }
	void dynamic_pminubXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pminubXmmXmm); }
	void dynamic_pminubXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pminubXmmXmm); }
	void dynamic_pmulhuwXmmXmm(DecodedOp* op) override { opXmmXmm(op, &JitSSE::pmulhuwXmmXmm); }
	void dynamic_pmulhuwXmmE128(DecodedOp* op) override { opXmmE128(op, &JitSSE::pmulhuwXmmXmm); }
	void dynamic_pextrwR32Xmm(DecodedOp* op) override;
	void dynamic_pextrwE16Xmm(DecodedOp* op) override;
	void dynamic_pinsrwXmmR32(DecodedOp* op) override;
	void dynamic_pinsrwXmmE16(DecodedOp* op) override;
	void dynamic_pmovmskbR32Xmm(DecodedOp* op) override;
	void dynamic_maskmovdquE128XmmXmm(DecodedOp* op) override;
	void dynamic_lfence(DecodedOp* op) override { lfence(); }
	void dynamic_mfence(DecodedOp* op) override { mfence(); }
	void dynamic_clflush(DecodedOp* op) override;
	void dynamic_FCOS(DecodedOp* op) override;
	void dynamic_FSIN(DecodedOp* op) override;

	virtual void IfSseLessThan(SSERegPtr src1, SSERegPtr src2) = 0;

	void createHelpers() override;
private:
	U8* createJitCosSub();
	U8* createJitCos();
};

#endif