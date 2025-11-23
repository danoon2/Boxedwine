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

#ifndef __DYNAMICMMX_H__
#define __DYNAMICMMX_H__

#include "dynamicFPU.h"

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
class DynamicCodeGenMMX : public DynamicCodeGenFPU {
public:
    using MmxMmxCallback = void(DynamicCodeGenMMX::*)(DynMMXReg dst, DynMMXReg src);
    using MmxImmCallback = void(DynamicCodeGenMMX::*)(DynMMXReg dst, U32 imm);

    DynamicCodeGenMMX(CPU* cpu) : DynamicCodeGenFPU(cpu) {}

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

    void dynamic_pxorMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::xorMmxMmx); }
    void dynamic_pxorE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::xorMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pxorE64(op); }); }
    void dynamic_porMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::orMmxMmx); }
    void dynamic_porE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::orMmxMmx, [op, this]() {DynamicCodeGen::dynamic_porE64(op); }); }
    void dynamic_pandMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::andMmxMmx); }
    void dynamic_pandE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::andMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pandE64(op); }); }
    void dynamic_pandnMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::andnMmxMmx); }
    void dynamic_pandnE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::andnMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pandnE64(op); }); }

    void dynamic_psllwMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psllwMmxMmx); }
    void dynamic_psllwE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psllwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psllwE64(op); }); }
    void dynamic_psrlwMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psrlwMmxMmx); }
    void dynamic_psrlwE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psrlwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psrlwE64(op); }); }
    void dynamic_psrawMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psrawMmxMmx); }
    void dynamic_psrawE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psrawMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psrawE64(op); }); }
    void dynamic_psllw(DecodedOp* op) override { opMmx(op, &DynamicCodeGenMMX::psllwMmx); }
    void dynamic_psrlw(DecodedOp* op) override { opMmx(op, &DynamicCodeGenMMX::psrlwMmx); }
    void dynamic_psraw(DecodedOp* op) override { opMmx(op, &DynamicCodeGenMMX::psrawMmx); }

    void dynamic_pslldMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pslldMmxMmx); }
    void dynamic_pslldE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pslldMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pslldE64(op); }); }
    void dynamic_psrldMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psrldMmxMmx); }
    void dynamic_psrldE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psrldMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psrldE64(op); }); }
    void dynamic_psradMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psradMmxMmx); }
    void dynamic_psradE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psradMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psradE64(op); }); }
    void dynamic_pslld(DecodedOp* op) override { opMmx(op, &DynamicCodeGenMMX::pslldMmx); }
    void dynamic_psrld(DecodedOp* op) override { opMmx(op, &DynamicCodeGenMMX::psrldMmx); }
    void dynamic_psrad(DecodedOp* op) override { opMmx(op, &DynamicCodeGenMMX::psradMmx); }

    void dynamic_psllqMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psllqMmxMmx); }
    void dynamic_psllqE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psllqMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psllqE64(op); }); }
    void dynamic_psrlqMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psrlqMmxMmx); }
    void dynamic_psrlqE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psrlqMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psrlqE64(op); }); }
    void dynamic_psllq(DecodedOp* op) override { opMmx(op, &DynamicCodeGenMMX::psllqMmx); }
    void dynamic_psrlq(DecodedOp* op) override { opMmx(op, &DynamicCodeGenMMX::psrlqMmx); }

    void dynamic_paddbMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::paddbMmxMmx); }
    void dynamic_paddbE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::paddbMmxMmx, [op, this]() {DynamicCodeGen::dynamic_paddbE64(op); }); }
    void dynamic_paddwMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::paddwMmxMmx); }
    void dynamic_paddwE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::paddwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_paddwE64(op); }); }
    void dynamic_padddMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::padddMmxMmx); }
    void dynamic_padddE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::padddMmxMmx, [op, this]() {DynamicCodeGen::dynamic_padddE64(op); }); }

    void dynamic_paddsbMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::paddsbMmxMmx); }
    void dynamic_paddsbE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::paddsbMmxMmx, [op, this]() {DynamicCodeGen::dynamic_paddsbE64(op); }); }
    void dynamic_paddswMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::paddswMmxMmx); }
    void dynamic_paddswE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::paddswMmxMmx, [op, this]() {DynamicCodeGen::dynamic_paddswE64(op); }); }
    void dynamic_paddusbMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::paddusbMmxMmx); }
    void dynamic_paddusbE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::paddusbMmxMmx, [op, this]() {DynamicCodeGen::dynamic_paddusbE64(op); }); }
    void dynamic_padduswMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::padduswMmxMmx); }
    void dynamic_padduswE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::padduswMmxMmx, [op, this]() {DynamicCodeGen::dynamic_padduswE64(op); }); }

    void dynamic_psubbMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psubbMmxMmx); }
    void dynamic_psubbE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psubbMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psubbE64(op); }); }
    void dynamic_psubwMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psubwMmxMmx); }
    void dynamic_psubwE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psubwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psubwE64(op); }); }
    void dynamic_psubdMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psubdMmxMmx); }
    void dynamic_psubdE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psubdMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psubdE64(op); }); }

    void dynamic_psubsbMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psubsbMmxMmx); }
    void dynamic_psubsbE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psubsbMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psubsbE64(op); }); }
    void dynamic_psubswMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psubswMmxMmx); }
    void dynamic_psubswE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psubswMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psubswE64(op); }); }
    void dynamic_psubusbMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psubusbMmxMmx); }
    void dynamic_psubusbE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psubusbMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psubusbE64(op); }); }
    void dynamic_psubuswMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psubuswMmxMmx); }
    void dynamic_psubuswE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psubuswMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psubuswE64(op); }); }

    void dynamic_pmulhwMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pmulhwMmxMmx); }
    void dynamic_pmulhwE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pmulhwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pmulhwE64(op); }); }
    void dynamic_pmullwMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pmullwMmxMmx); }
    void dynamic_pmullwE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pmullwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pmullwE64(op); }); }
    void dynamic_pmaddwdMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pmaddwdMmxMmx); }
    void dynamic_pmaddwdE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pmaddwdMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pmaddwdE64(op); }); }

    void dynamic_pcmpeqbMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pcmpeqbMmxMmx); }
    void dynamic_pcmpeqbE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pcmpeqbMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pcmpeqbE64(op); }); }
    void dynamic_pcmpeqwMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pcmpeqwMmxMmx); }
    void dynamic_pcmpeqwE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pcmpeqwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pcmpeqwE64(op); }); }
    void dynamic_pcmpeqdMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pcmpeqdMmxMmx); }
    void dynamic_pcmpeqdE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pcmpeqdMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pcmpeqdE64(op); }); }
    void dynamic_pcmpgtbMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pcmpgtbMmxMmx); }
    void dynamic_pcmpgtbE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pcmpgtbMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pcmpgtbE64(op); }); }
    void dynamic_pcmpgtwMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pcmpgtwMmxMmx); }
    void dynamic_pcmpgtwE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pcmpgtwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pcmpgtwE64(op); }); }
    void dynamic_pcmpgtdMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pcmpgtdMmxMmx); }
    void dynamic_pcmpgtdE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pcmpgtdMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pcmpgtdE64(op); }); }

    void dynamic_packsswbMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::packsswbMmxMmx); }
    void dynamic_packsswbE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::packsswbMmxMmx, [op, this]() {DynamicCodeGen::dynamic_packsswbE64(op); }); }
    void dynamic_packssdwMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::packssdwMmxMmx); }
    void dynamic_packssdwE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::packssdwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_packssdwE64(op); }); }
    void dynamic_packuswbMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::packuswbMmxMmx); }
    void dynamic_packuswbE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::packuswbMmxMmx, [op, this]() {DynamicCodeGen::dynamic_packuswbE64(op); }); }

    void dynamic_punpckhbwMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::punpckhbwMmxMmx); }
    void dynamic_punpckhbwE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::punpckhbwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_punpckhbwE64(op); }); }
    void dynamic_punpckhwdMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::punpckhwdMmxMmx); }
    void dynamic_punpckhwdE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::punpckhwdMmxMmx, [op, this]() {DynamicCodeGen::dynamic_punpckhwdE64(op); }); }
    void dynamic_punpckhdqMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::punpckhdqMmxMmx); }
    void dynamic_punpckhdqE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::punpckhdqMmxMmx, [op, this]() {DynamicCodeGen::dynamic_punpckhdqE64(op); }); }
    void dynamic_punpcklbwMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::punpcklbwMmxMmx); }
    void dynamic_punpcklbwE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::punpcklbwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_punpcklbwE64(op); }); }
    void dynamic_punpcklwdMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::punpcklwdMmxMmx); }
    void dynamic_punpcklwdE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::punpcklwdMmxMmx, [op, this]() {DynamicCodeGen::dynamic_punpcklwdE64(op); }); }
    void dynamic_punpckldqMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::punpckldqMmxMmx); }
    void dynamic_punpckldqE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::punpckldqMmxMmx, [op, this]() {DynamicCodeGen::dynamic_punpckldqE64(op); }); }

    // SSE add ons
    void dynamic_pavgbMmxMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pavgbMmxMmx); }
    void dynamic_pavgbMmxE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pavgbMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pavgbMmxE64(op); }); }
    void dynamic_pavgwMmxMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pavgwMmxMmx); }
    void dynamic_pavgwMmxE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pavgwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pavgwMmxE64(op); }); }
    void dynamic_psadbwMmxMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psadbwMmxMmx); }
    void dynamic_psadbwMmxE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psadbwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psadbwMmxE64(op); }); }
    void dynamic_pextrwR32Mmx(DecodedOp* op) override;
    void dynamic_pextrwE16Mmx(DecodedOp* op) override;
    void dynamic_pinsrwMmxR32(DecodedOp* op) override;
    void dynamic_pinsrwMmxE16(DecodedOp* op) override;
    void dynamic_pmaxswMmxMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pmaxswMmxMmx); }
    void dynamic_pmaxswMmxE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pmaxswMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pmaxswMmxE64(op); }); }
    void dynamic_pmaxubMmxMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pmaxubMmxMmx); }
    void dynamic_pmaxubMmxE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pmaxubMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pmaxubMmxE64(op); }); }
    void dynamic_pminswMmxMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pminswMmxMmx); }
    void dynamic_pminswMmxE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pminswMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pminswMmxE64(op); }); }
    void dynamic_pminubMmxMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pminubMmxMmx); }
    void dynamic_pminubMmxE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pminubMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pminubMmxE64(op); }); }
    void dynamic_pmovmskbR32Mmx(DecodedOp* op) override;
    void dynamic_pmulhuwMmxMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pmulhuwMmxMmx); }
    void dynamic_pmulhuwMmxE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pmulhuwMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pmulhuwMmxE64(op); }); }
    void dynamic_pshufwMmxMmx(DecodedOp* op) override;
    void dynamic_pshufwMmxE64(DecodedOp* op) override;
    void dynamic_maskmovqEDIMmxMmx(DecodedOp* op) override;
    void dynamic_movntqE64Mmx(DecodedOp* op) override { dynamic_movE64Pq(op); }

    // SSE2 add ons
    void dynamic_paddqMmxMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::paddqMmxMmx); }
    void dynamic_paddqMmxE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::paddqMmxMmx, [op, this]() {DynamicCodeGen::dynamic_paddqMmxE64(op); }); }
    void dynamic_psubqMmxMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::psubqMmxMmx); }
    void dynamic_psubqMmxE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::psubqMmxMmx, [op, this]() {DynamicCodeGen::dynamic_psubqMmxE64(op); }); }
    void dynamic_pmuludqMmxMmx(DecodedOp* op) override { opMmxMmx(op, &DynamicCodeGenMMX::pmuludqMmxMmx); }
    void dynamic_pmuludqMmxE64(DecodedOp* op) override { opMmxE64(op, &DynamicCodeGenMMX::pmuludqMmxMmx, [op, this]() {DynamicCodeGen::dynamic_pmuludqMmxE64(op); }); }
};

#endif