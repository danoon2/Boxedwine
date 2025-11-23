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

enum DynMMXReg {
    DYN_MMX_REG_0 = 0,
    DYN_MMX_REG_1 = 1,
    DYN_MMX_REG_2 = 2,
    DYN_MMX_REG_3 = 3,
    DYN_MMX_REG_4 = 4,
    DYN_MMX_REG_5 = 5,
    DYN_MMX_REG_6 = 6,
    DYN_MMX_REG_7 = 7,
};

// Implementation of JIT that is host instruction independent
class JitMMX : public JitFPU {
public:
    using MmxMmxCallback = void(JitMMX::*)(DynMMXReg dst, DynMMXReg src);
    using MmxImmCallback = void(JitMMX::*)(DynMMXReg dst, U32 imm);

    JitMMX(CPU* cpu) : JitFPU(cpu) {}

    void startMMX();
    virtual void loadMMXFromReg(DynMMXReg mmx, RegPtr reg) = 0;
    virtual void storeCpuMMXReg(DynMMXReg reg, U32 index) = 0;
    virtual void storeMMXToReg(DynMMXReg mmx, RegPtr reg) = 0;
    virtual void loadCpuMMXReg(DynMMXReg reg, U32 index) = 0;
    virtual void loadMMXFromMem32(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
    virtual void loadMMXFromMem64(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
    virtual void storeMMXToMem32(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
    virtual void storeMMXToMem64(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) = 0;
    virtual DynMMXReg getTmpMMX(U8 inUse) = 0; // just in case some day I do mmx register caching
    virtual void xorMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void orMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void andMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void andnMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psllwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psrlwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psrawMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psllwMmx(DynMMXReg dst, U32 imm) = 0;
    virtual void psrlwMmx(DynMMXReg dst, U32 imm) = 0;
    virtual void psrawMmx(DynMMXReg dst, U32 imm) = 0;
    virtual void pslldMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psrldMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psradMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pslldMmx(DynMMXReg dst, U32 imm) = 0;
    virtual void psrldMmx(DynMMXReg dst, U32 imm) = 0;
    virtual void psradMmx(DynMMXReg dst, U32 imm) = 0;
    virtual void psllqMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psrlqMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psllqMmx(DynMMXReg dst, U32 imm) = 0;
    virtual void psrlqMmx(DynMMXReg dst, U32 imm) = 0;

    virtual void paddbMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void paddwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void padddMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void paddsbMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void paddswMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void paddusbMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void padduswMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;

    virtual void psubbMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psubwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psubdMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psubsbMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psubswMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psubusbMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psubuswMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;

    virtual void pmulhwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pmullwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pmaddwdMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;

    virtual void pcmpeqbMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pcmpeqwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pcmpeqdMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pcmpgtbMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pcmpgtwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pcmpgtdMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;

    virtual void packsswbMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void packssdwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void packuswbMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;

    virtual void punpckhbwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void punpckhwdMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void punpckhdqMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void punpcklbwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void punpcklwdMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void punpckldqMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;

    virtual void pavgbMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pavgwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psadbwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pextrwRegMmx(RegPtr dst, DynMMXReg src, U8 srcIndex) = 0;
    virtual void pinsrwMmxReg(DynMMXReg dest, RegPtr src, U8 dstIndex) = 0;
    virtual void pmaxswMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pmaxubMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pminswMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pminubMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pmovmskbMmxMmx(RegPtr dst, DynMMXReg src) = 0;
    virtual void pmulhuwMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pshufwMmxMmx(DynMMXReg dst, DynMMXReg src, U8 mask) = 0;
    virtual void maskmovq(DynMMXReg src, DynMMXReg mask, RegPtr destAddress) = 0;
    virtual void paddqMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void psubqMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;
    virtual void pmuludqMmxMmx(DynMMXReg dst, DynMMXReg src) = 0;

    void opMmxMmx(DecodedOp* op, MmxMmxCallback callback);
    void opMmxE64(DecodedOp* op, MmxMmxCallback callback, std::function<void()> fallback);
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
    void dynamic_pxorE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::xorMmxMmx, [op, this]() {JitCodeGen::dynamic_pxorE64(op); }); }
    void dynamic_porMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::orMmxMmx); }
    void dynamic_porE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::orMmxMmx, [op, this]() {JitCodeGen::dynamic_porE64(op); }); }
    void dynamic_pandMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::andMmxMmx); }
    void dynamic_pandE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::andMmxMmx, [op, this]() {JitCodeGen::dynamic_pandE64(op); }); }
    void dynamic_pandnMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::andnMmxMmx); }
    void dynamic_pandnE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::andnMmxMmx, [op, this]() {JitCodeGen::dynamic_pandnE64(op); }); }

    void dynamic_psllwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psllwMmxMmx); }
    void dynamic_psllwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psllwMmxMmx, [op, this]() {JitCodeGen::dynamic_psllwE64(op); }); }
    void dynamic_psrlwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psrlwMmxMmx); }
    void dynamic_psrlwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psrlwMmxMmx, [op, this]() {JitCodeGen::dynamic_psrlwE64(op); }); }
    void dynamic_psrawMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psrawMmxMmx); }
    void dynamic_psrawE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psrawMmxMmx, [op, this]() {JitCodeGen::dynamic_psrawE64(op); }); }
    void dynamic_psllw(DecodedOp* op) override { opMmx(op, &JitMMX::psllwMmx); }
    void dynamic_psrlw(DecodedOp* op) override { opMmx(op, &JitMMX::psrlwMmx); }
    void dynamic_psraw(DecodedOp* op) override { opMmx(op, &JitMMX::psrawMmx); }

    void dynamic_pslldMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pslldMmxMmx); }
    void dynamic_pslldE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pslldMmxMmx, [op, this]() {JitCodeGen::dynamic_pslldE64(op); }); }
    void dynamic_psrldMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psrldMmxMmx); }
    void dynamic_psrldE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psrldMmxMmx, [op, this]() {JitCodeGen::dynamic_psrldE64(op); }); }
    void dynamic_psradMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psradMmxMmx); }
    void dynamic_psradE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psradMmxMmx, [op, this]() {JitCodeGen::dynamic_psradE64(op); }); }
    void dynamic_pslld(DecodedOp* op) override { opMmx(op, &JitMMX::pslldMmx); }
    void dynamic_psrld(DecodedOp* op) override { opMmx(op, &JitMMX::psrldMmx); }
    void dynamic_psrad(DecodedOp* op) override { opMmx(op, &JitMMX::psradMmx); }

    void dynamic_psllqMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psllqMmxMmx); }
    void dynamic_psllqE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psllqMmxMmx, [op, this]() {JitCodeGen::dynamic_psllqE64(op); }); }
    void dynamic_psrlqMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psrlqMmxMmx); }
    void dynamic_psrlqE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psrlqMmxMmx, [op, this]() {JitCodeGen::dynamic_psrlqE64(op); }); }
    void dynamic_psllq(DecodedOp* op) override { opMmx(op, &JitMMX::psllqMmx); }
    void dynamic_psrlq(DecodedOp* op) override { opMmx(op, &JitMMX::psrlqMmx); }

    void dynamic_paddbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::paddbMmxMmx); }
    void dynamic_paddbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::paddbMmxMmx, [op, this]() {JitCodeGen::dynamic_paddbE64(op); }); }
    void dynamic_paddwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::paddwMmxMmx); }
    void dynamic_paddwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::paddwMmxMmx, [op, this]() {JitCodeGen::dynamic_paddwE64(op); }); }
    void dynamic_padddMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::padddMmxMmx); }
    void dynamic_padddE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::padddMmxMmx, [op, this]() {JitCodeGen::dynamic_padddE64(op); }); }

    void dynamic_paddsbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::paddsbMmxMmx); }
    void dynamic_paddsbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::paddsbMmxMmx, [op, this]() {JitCodeGen::dynamic_paddsbE64(op); }); }
    void dynamic_paddswMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::paddswMmxMmx); }
    void dynamic_paddswE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::paddswMmxMmx, [op, this]() {JitCodeGen::dynamic_paddswE64(op); }); }
    void dynamic_paddusbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::paddusbMmxMmx); }
    void dynamic_paddusbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::paddusbMmxMmx, [op, this]() {JitCodeGen::dynamic_paddusbE64(op); }); }
    void dynamic_padduswMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::padduswMmxMmx); }
    void dynamic_padduswE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::padduswMmxMmx, [op, this]() {JitCodeGen::dynamic_padduswE64(op); }); }

    void dynamic_psubbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubbMmxMmx); }
    void dynamic_psubbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubbMmxMmx, [op, this]() {JitCodeGen::dynamic_psubbE64(op); }); }
    void dynamic_psubwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubwMmxMmx); }
    void dynamic_psubwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubwMmxMmx, [op, this]() {JitCodeGen::dynamic_psubwE64(op); }); }
    void dynamic_psubdMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubdMmxMmx); }
    void dynamic_psubdE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubdMmxMmx, [op, this]() {JitCodeGen::dynamic_psubdE64(op); }); }

    void dynamic_psubsbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubsbMmxMmx); }
    void dynamic_psubsbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubsbMmxMmx, [op, this]() {JitCodeGen::dynamic_psubsbE64(op); }); }
    void dynamic_psubswMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubswMmxMmx); }
    void dynamic_psubswE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubswMmxMmx, [op, this]() {JitCodeGen::dynamic_psubswE64(op); }); }
    void dynamic_psubusbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubusbMmxMmx); }
    void dynamic_psubusbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubusbMmxMmx, [op, this]() {JitCodeGen::dynamic_psubusbE64(op); }); }
    void dynamic_psubuswMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubuswMmxMmx); }
    void dynamic_psubuswE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubuswMmxMmx, [op, this]() {JitCodeGen::dynamic_psubuswE64(op); }); }

    void dynamic_pmulhwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmulhwMmxMmx); }
    void dynamic_pmulhwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmulhwMmxMmx, [op, this]() {JitCodeGen::dynamic_pmulhwE64(op); }); }
    void dynamic_pmullwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmullwMmxMmx); }
    void dynamic_pmullwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmullwMmxMmx, [op, this]() {JitCodeGen::dynamic_pmullwE64(op); }); }
    void dynamic_pmaddwdMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmaddwdMmxMmx); }
    void dynamic_pmaddwdE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmaddwdMmxMmx, [op, this]() {JitCodeGen::dynamic_pmaddwdE64(op); }); }

    void dynamic_pcmpeqbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pcmpeqbMmxMmx); }
    void dynamic_pcmpeqbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pcmpeqbMmxMmx, [op, this]() {JitCodeGen::dynamic_pcmpeqbE64(op); }); }
    void dynamic_pcmpeqwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pcmpeqwMmxMmx); }
    void dynamic_pcmpeqwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pcmpeqwMmxMmx, [op, this]() {JitCodeGen::dynamic_pcmpeqwE64(op); }); }
    void dynamic_pcmpeqdMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pcmpeqdMmxMmx); }
    void dynamic_pcmpeqdE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pcmpeqdMmxMmx, [op, this]() {JitCodeGen::dynamic_pcmpeqdE64(op); }); }
    void dynamic_pcmpgtbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pcmpgtbMmxMmx); }
    void dynamic_pcmpgtbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pcmpgtbMmxMmx, [op, this]() {JitCodeGen::dynamic_pcmpgtbE64(op); }); }
    void dynamic_pcmpgtwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pcmpgtwMmxMmx); }
    void dynamic_pcmpgtwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pcmpgtwMmxMmx, [op, this]() {JitCodeGen::dynamic_pcmpgtwE64(op); }); }
    void dynamic_pcmpgtdMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pcmpgtdMmxMmx); }
    void dynamic_pcmpgtdE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pcmpgtdMmxMmx, [op, this]() {JitCodeGen::dynamic_pcmpgtdE64(op); }); }

    void dynamic_packsswbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::packsswbMmxMmx); }
    void dynamic_packsswbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::packsswbMmxMmx, [op, this]() {JitCodeGen::dynamic_packsswbE64(op); }); }
    void dynamic_packssdwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::packssdwMmxMmx); }
    void dynamic_packssdwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::packssdwMmxMmx, [op, this]() {JitCodeGen::dynamic_packssdwE64(op); }); }
    void dynamic_packuswbMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::packuswbMmxMmx); }
    void dynamic_packuswbE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::packuswbMmxMmx, [op, this]() {JitCodeGen::dynamic_packuswbE64(op); }); }

    void dynamic_punpckhbwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::punpckhbwMmxMmx); }
    void dynamic_punpckhbwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::punpckhbwMmxMmx, [op, this]() {JitCodeGen::dynamic_punpckhbwE64(op); }); }
    void dynamic_punpckhwdMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::punpckhwdMmxMmx); }
    void dynamic_punpckhwdE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::punpckhwdMmxMmx, [op, this]() {JitCodeGen::dynamic_punpckhwdE64(op); }); }
    void dynamic_punpckhdqMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::punpckhdqMmxMmx); }
    void dynamic_punpckhdqE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::punpckhdqMmxMmx, [op, this]() {JitCodeGen::dynamic_punpckhdqE64(op); }); }
    void dynamic_punpcklbwMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::punpcklbwMmxMmx); }
    void dynamic_punpcklbwE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::punpcklbwMmxMmx, [op, this]() {JitCodeGen::dynamic_punpcklbwE64(op); }); }
    void dynamic_punpcklwdMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::punpcklwdMmxMmx); }
    void dynamic_punpcklwdE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::punpcklwdMmxMmx, [op, this]() {JitCodeGen::dynamic_punpcklwdE64(op); }); }
    void dynamic_punpckldqMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::punpckldqMmxMmx); }
    void dynamic_punpckldqE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::punpckldqMmxMmx, [op, this]() {JitCodeGen::dynamic_punpckldqE64(op); }); }

    // SSE add ons
    void dynamic_pavgbMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pavgbMmxMmx); }
    void dynamic_pavgbMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pavgbMmxMmx, [op, this]() {JitCodeGen::dynamic_pavgbMmxE64(op); }); }
    void dynamic_pavgwMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pavgwMmxMmx); }
    void dynamic_pavgwMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pavgwMmxMmx, [op, this]() {JitCodeGen::dynamic_pavgwMmxE64(op); }); }
    void dynamic_psadbwMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psadbwMmxMmx); }
    void dynamic_psadbwMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psadbwMmxMmx, [op, this]() {JitCodeGen::dynamic_psadbwMmxE64(op); }); }
    void dynamic_pextrwR32Mmx(DecodedOp* op) override;
    void dynamic_pextrwE16Mmx(DecodedOp* op) override;
    void dynamic_pinsrwMmxR32(DecodedOp* op) override;
    void dynamic_pinsrwMmxE16(DecodedOp* op) override;
    void dynamic_pmaxswMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmaxswMmxMmx); }
    void dynamic_pmaxswMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmaxswMmxMmx, [op, this]() {JitCodeGen::dynamic_pmaxswMmxE64(op); }); }
    void dynamic_pmaxubMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmaxubMmxMmx); }
    void dynamic_pmaxubMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmaxubMmxMmx, [op, this]() {JitCodeGen::dynamic_pmaxubMmxE64(op); }); }
    void dynamic_pminswMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pminswMmxMmx); }
    void dynamic_pminswMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pminswMmxMmx, [op, this]() {JitCodeGen::dynamic_pminswMmxE64(op); }); }
    void dynamic_pminubMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pminubMmxMmx); }
    void dynamic_pminubMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pminubMmxMmx, [op, this]() {JitCodeGen::dynamic_pminubMmxE64(op); }); }
    void dynamic_pmovmskbR32Mmx(DecodedOp* op) override;
    void dynamic_pmulhuwMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmulhuwMmxMmx); }
    void dynamic_pmulhuwMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmulhuwMmxMmx, [op, this]() {JitCodeGen::dynamic_pmulhuwMmxE64(op); }); }
    void dynamic_pshufwMmxMmx(DecodedOp* op) override;
    void dynamic_pshufwMmxE64(DecodedOp* op) override;
    void dynamic_maskmovqEDIMmxMmx(DecodedOp* op) override;
    void dynamic_movntqE64Mmx(DecodedOp* op) override { dynamic_movE64Pq(op); }

    // SSE2 add ons
    void dynamic_paddqMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::paddqMmxMmx); }
    void dynamic_paddqMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::paddqMmxMmx, [op, this]() {JitCodeGen::dynamic_paddqMmxE64(op); }); }
    void dynamic_psubqMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::psubqMmxMmx); }
    void dynamic_psubqMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::psubqMmxMmx, [op, this]() {JitCodeGen::dynamic_psubqMmxE64(op); }); }
    void dynamic_pmuludqMmxMmx(DecodedOp* op) override { opMmxMmx(op, &JitMMX::pmuludqMmxMmx); }
    void dynamic_pmuludqMmxE64(DecodedOp* op) override { opMmxE64(op, &JitMMX::pmuludqMmxMmx, [op, this]() {JitCodeGen::dynamic_pmuludqMmxE64(op); }); }
};

#endif