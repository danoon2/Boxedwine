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

#ifndef __JIT_MMX_H__
#define __JIT_MMX_H__

#include "jitFPU.h"

class MMXRegInternal {
public:
    MMXRegInternal(U8 hardwareReg, U8 emulatedReg) : reg(hardwareReg), emulatedReg(emulatedReg) {}

    U8 hardwareReg() { return reg; }

    U8 emulatedReg;
private:
    U8 reg;
};

using MMXRegPtr = std::shared_ptr<MMXRegInternal>;

#define MMX_TMP_INDEX 0xff

// Implementation of JIT that is host instruction independent
class JitMMX : public JitFPU {
public:
    using MmxMmxCallback = void(JitMMX::*)(MMXRegPtr dst, MMXRegPtr src);
    using MmxImmCallback = void(JitMMX::*)(MMXRegPtr dst, U32 imm);

    JitMMX(CPU* cpu) : JitFPU(cpu) {}

    void startMMX();
    virtual MMXRegPtr getTmpMMX() = 0;
    virtual MMXRegPtr loadMMXFromReg(RegPtr reg) = 0;
    virtual void storeCpuMMXReg(MMXRegPtr reg, U32 index) = 0;
    virtual void storeMMXToReg(MMXRegPtr mmx, RegPtr reg) = 0;
    virtual MMXRegPtr loadCpuMMXReg(U8 index) = 0;
    virtual MMXRegPtr loadMMXFromMem32(U8 index, RegPtr rm, RegPtr sib) = 0;
    virtual MMXRegPtr loadMMXFromMem64(U8 index, RegPtr rm, RegPtr sib) = 0;
    virtual void storeMMXToMem32(MMXRegPtr reg, RegPtr rm, RegPtr sib) = 0;
    virtual void storeMMXToMem64(MMXRegPtr reg, RegPtr rm, RegPtr sib) = 0;    
    virtual void xorMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void orMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void andMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void andnMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psllwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psrlwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psrawMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psllwMmx(MMXRegPtr dst, U32 imm) = 0;
    virtual void psrlwMmx(MMXRegPtr dst, U32 imm) = 0;
    virtual void psrawMmx(MMXRegPtr dst, U32 imm) = 0;
    virtual void pslldMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psrldMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psradMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pslldMmx(MMXRegPtr dst, U32 imm) = 0;
    virtual void psrldMmx(MMXRegPtr dst, U32 imm) = 0;
    virtual void psradMmx(MMXRegPtr dst, U32 imm) = 0;
    virtual void psllqMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psrlqMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psllqMmx(MMXRegPtr dst, U32 imm) = 0;
    virtual void psrlqMmx(MMXRegPtr dst, U32 imm) = 0;

    virtual void paddbMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void paddwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void padddMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void paddsbMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void paddswMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void paddusbMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void padduswMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;

    virtual void psubbMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psubwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psubdMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psubsbMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psubswMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psubusbMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psubuswMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;

    virtual void pmulhwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pmullwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pmaddwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;

    virtual void pcmpeqbMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pcmpeqwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pcmpeqdMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pcmpgtbMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pcmpgtwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pcmpgtdMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;

    virtual void packsswbMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void packssdwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void packuswbMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;

    virtual void punpckhbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void punpckhwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void punpckhdqMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void punpcklbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void punpcklwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void punpckldqMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;

    virtual void pavgbMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pavgwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psadbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pextrwRegMmx(RegPtr dst, MMXRegPtr src, U8 srcIndex) = 0;
    virtual void pinsrwMmxReg(MMXRegPtr dest, RegPtr src, U8 dstIndex) = 0;
    virtual void pmaxswMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pmaxubMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pminswMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pminubMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pmovmskbMmxMmx(RegPtr dst, MMXRegPtr src) = 0;
    virtual void pmulhuwMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pshufwMmxMmx(MMXRegPtr dst, MMXRegPtr src, U8 mask) = 0;
    virtual void maskmovq(MMXRegPtr src, MMXRegPtr mask, RegPtr destAddress) = 0;
    virtual void paddqMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void psubqMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;
    virtual void pmuludqMmxMmx(MMXRegPtr dst, MMXRegPtr src) = 0;

    void opMmxMmx(DecodedOp* op, MmxMmxCallback callback);
    void opMmxE64(DecodedOp* op, MmxMmxCallback callback);
    void opMmx(DecodedOp* op, MmxImmCallback callback);

    void dynamic_emms(DecodedOp* op) override;
    void dynamic_movPqR32(DecodedOp* op) override;
    void dynamic_movPqE32(DecodedOp* op) override;
    void dynamic_movR32Pq(DecodedOp* op) override;
    void dynamic_movE32Pq(DecodedOp* op) override;
    void dynamic_movPqMmx(DecodedOp* op) override;
    void dynamic_movPqE64(DecodedOp* op) override;
    void dynamic_movE64Pq(DecodedOp* op) override;
    void dynamic_movMmxPq(DecodedOp* op) override;

    void dynamic_pxorMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::xorMmxMmx); }
    void dynamic_pxorE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::xorMmxMmx); }
    void dynamic_porMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::orMmxMmx); }
    void dynamic_porE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::orMmxMmx); }
    void dynamic_pandMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::andMmxMmx); }
    void dynamic_pandE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::andMmxMmx); }
    void dynamic_pandnMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::andnMmxMmx); }
    void dynamic_pandnE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::andnMmxMmx); }

    void dynamic_psllwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psllwMmxMmx); }
    void dynamic_psllwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psllwMmxMmx); }
    void dynamic_psrlwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psrlwMmxMmx); }
    void dynamic_psrlwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psrlwMmxMmx); }
    void dynamic_psrawMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psrawMmxMmx); }
    void dynamic_psrawE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psrawMmxMmx); }
    void dynamic_psllw(DecodedOp* op) override { opMmx(op, &JitMMX::psllwMmx); }
    void dynamic_psrlw(DecodedOp* op) override { opMmx(op, &JitMMX::psrlwMmx); }
    void dynamic_psraw(DecodedOp* op) override { opMmx(op, &JitMMX::psrawMmx); }

    void dynamic_pslldMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pslldMmxMmx); }
    void dynamic_pslldE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pslldMmxMmx); }
    void dynamic_psrldMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psrldMmxMmx); }
    void dynamic_psrldE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psrldMmxMmx); }
    void dynamic_psradMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psradMmxMmx); }
    void dynamic_psradE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psradMmxMmx); }
    void dynamic_pslld(DecodedOp* op) override { opMmx(op, &JitMMX::pslldMmx); }
    void dynamic_psrld(DecodedOp* op) override { opMmx(op, &JitMMX::psrldMmx); }
    void dynamic_psrad(DecodedOp* op) override { opMmx(op, &JitMMX::psradMmx); }

    void dynamic_psllqMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psllqMmxMmx); }
    void dynamic_psllqE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psllqMmxMmx); }
    void dynamic_psrlqMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psrlqMmxMmx); }
    void dynamic_psrlqE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psrlqMmxMmx); }
    void dynamic_psllq(DecodedOp* op) override { opMmx(op, &JitMMX::psllqMmx); }
    void dynamic_psrlq(DecodedOp* op) override { opMmx(op, &JitMMX::psrlqMmx); }

    void dynamic_paddbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::paddbMmxMmx); }
    void dynamic_paddbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::paddbMmxMmx); }
    void dynamic_paddwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::paddwMmxMmx); }
    void dynamic_paddwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::paddwMmxMmx); }
    void dynamic_padddMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::padddMmxMmx); }
    void dynamic_padddE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::padddMmxMmx); }

    void dynamic_paddsbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::paddsbMmxMmx); }
    void dynamic_paddsbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::paddsbMmxMmx); }
    void dynamic_paddswMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::paddswMmxMmx); }
    void dynamic_paddswE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::paddswMmxMmx); }
    void dynamic_paddusbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::paddusbMmxMmx); }
    void dynamic_paddusbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::paddusbMmxMmx); }
    void dynamic_padduswMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::padduswMmxMmx); }
    void dynamic_padduswE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::padduswMmxMmx); }

    void dynamic_psubbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubbMmxMmx); }
    void dynamic_psubbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubbMmxMmx); }
    void dynamic_psubwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubwMmxMmx); }
    void dynamic_psubwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubwMmxMmx); }
    void dynamic_psubdMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubdMmxMmx); }
    void dynamic_psubdE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubdMmxMmx); }

    void dynamic_psubsbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubsbMmxMmx); }
    void dynamic_psubsbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubsbMmxMmx); }
    void dynamic_psubswMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubswMmxMmx); }
    void dynamic_psubswE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubswMmxMmx); }
    void dynamic_psubusbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubusbMmxMmx); }
    void dynamic_psubusbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubusbMmxMmx); }
    void dynamic_psubuswMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubuswMmxMmx); }
    void dynamic_psubuswE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubuswMmxMmx); }

    void dynamic_pmulhwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmulhwMmxMmx); }
    void dynamic_pmulhwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmulhwMmxMmx); }
    void dynamic_pmullwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmullwMmxMmx); }
    void dynamic_pmullwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmullwMmxMmx); }
    void dynamic_pmaddwdMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmaddwdMmxMmx); }
    void dynamic_pmaddwdE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmaddwdMmxMmx); }

    void dynamic_pcmpeqbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pcmpeqbMmxMmx); }
    void dynamic_pcmpeqbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pcmpeqbMmxMmx); }
    void dynamic_pcmpeqwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pcmpeqwMmxMmx); }
    void dynamic_pcmpeqwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pcmpeqwMmxMmx); }
    void dynamic_pcmpeqdMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pcmpeqdMmxMmx); }
    void dynamic_pcmpeqdE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pcmpeqdMmxMmx); }
    void dynamic_pcmpgtbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pcmpgtbMmxMmx); }
    void dynamic_pcmpgtbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pcmpgtbMmxMmx); }
    void dynamic_pcmpgtwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pcmpgtwMmxMmx); }
    void dynamic_pcmpgtwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pcmpgtwMmxMmx); }
    void dynamic_pcmpgtdMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pcmpgtdMmxMmx); }
    void dynamic_pcmpgtdE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pcmpgtdMmxMmx); }

    void dynamic_packsswbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::packsswbMmxMmx); }
    void dynamic_packsswbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::packsswbMmxMmx); }
    void dynamic_packssdwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::packssdwMmxMmx); }
    void dynamic_packssdwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::packssdwMmxMmx); }
    void dynamic_packuswbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::packuswbMmxMmx); }
    void dynamic_packuswbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::packuswbMmxMmx); }

    void dynamic_punpckhbwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::punpckhbwMmxMmx); }
    void dynamic_punpckhbwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::punpckhbwMmxMmx); }
    void dynamic_punpckhwdMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::punpckhwdMmxMmx); }
    void dynamic_punpckhwdE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::punpckhwdMmxMmx); }
    void dynamic_punpckhdqMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::punpckhdqMmxMmx); }
    void dynamic_punpckhdqE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::punpckhdqMmxMmx); }
    void dynamic_punpcklbwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::punpcklbwMmxMmx); }
    void dynamic_punpcklbwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::punpcklbwMmxMmx); }
    void dynamic_punpcklwdMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::punpcklwdMmxMmx); }
    void dynamic_punpcklwdE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::punpcklwdMmxMmx); }
    void dynamic_punpckldqMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::punpckldqMmxMmx); }
    void dynamic_punpckldqE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::punpckldqMmxMmx); }

    // SSE add ons
    void dynamic_pavgbMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pavgbMmxMmx); }
    void dynamic_pavgbMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pavgbMmxMmx); }
    void dynamic_pavgwMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pavgwMmxMmx); }
    void dynamic_pavgwMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pavgwMmxMmx); }
    void dynamic_psadbwMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psadbwMmxMmx); }
    void dynamic_psadbwMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psadbwMmxMmx); }
    void dynamic_pextrwR32Mmx(DecodedOp* op) override;
    void dynamic_pextrwE16Mmx(DecodedOp* op) override;
    void dynamic_pinsrwMmxR32(DecodedOp* op) override;
    void dynamic_pinsrwMmxE16(DecodedOp* op) override;
    void dynamic_pmaxswMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmaxswMmxMmx); }
    void dynamic_pmaxswMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmaxswMmxMmx); }
    void dynamic_pmaxubMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmaxubMmxMmx); }
    void dynamic_pmaxubMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmaxubMmxMmx); }
    void dynamic_pminswMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pminswMmxMmx); }
    void dynamic_pminswMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pminswMmxMmx); }
    void dynamic_pminubMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pminubMmxMmx); }
    void dynamic_pminubMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pminubMmxMmx); }
    void dynamic_pmovmskbR32Mmx(DecodedOp* op) override;
    void dynamic_pmulhuwMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmulhuwMmxMmx); }
    void dynamic_pmulhuwMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmulhuwMmxMmx); }
    void dynamic_pshufwMmxMmx(DecodedOp* op) override;
    void dynamic_pshufwMmxE64(DecodedOp* op) override;
    void dynamic_maskmovqEDIMmxMmx(DecodedOp* op) override;
    void dynamic_movntqE64Mmx(DecodedOp* op) override { dynamic_movE64Pq(op); }

    // SSE2 add ons
    void dynamic_paddqMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::paddqMmxMmx); }
    void dynamic_paddqMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::paddqMmxMmx); }
    void dynamic_psubqMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubqMmxMmx); }
    void dynamic_psubqMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubqMmxMmx); }
    void dynamic_pmuludqMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmuludqMmxMmx); }
    void dynamic_pmuludqMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmuludqMmxMmx); }
};

#endif