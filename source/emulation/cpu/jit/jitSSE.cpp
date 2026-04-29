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

#include "boxedwine.h"

#ifdef BOXEDWINE_JIT
#include "jitSSE.h"

void JitSSE::opXmmXmm(DecodedOp* op, XmmXmmCallback callback, bool loadDest) {
    SSERegPtr reg;

    if (loadDest || isSseRegCached(op->reg)) {
        reg = loadCpuXMMReg(op->reg);
    } else {
        reg = getTmpSSE();
    }
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    (this->*callback)(reg, rm);
    storeCpuXMMReg(reg, op->reg);
}

void JitSSE::opXmmXmmImm(DecodedOp* op, XmmXmmImmCallback callback) {
    SSERegPtr reg = loadCpuXMMReg(op->reg);
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    (this->*callback)(reg, rm, op->imm);
    storeCpuXMMReg(reg, op->reg);
}

void JitSSE::opXmmImm(DecodedOp* op, XmmImmCallback callback) {
    SSERegPtr reg = loadCpuXMMReg(op->reg);
    (this->*callback)(reg, op->imm);
    storeCpuXMMReg(reg, op->reg);
}

void JitSSE::opXmmE128(DecodedOp* op, XmmXmmCallback callback, bool loadDest) {
    read(JitWidth::b128, calculateEaa(op), [op, callback, loadDest, this](MemPtr address) {
        SSERegPtr reg;
        if (loadDest || isSseRegCached(op->reg)) {
            reg = loadCpuXMMReg(op->reg);
        } else {
            reg = getTmpSSE();
        }
        (this->*callback)(reg, loadXMMFromMem128(SSE_TMP_INDEX, address));
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::opXmmE64Imm(DecodedOp* op, XmmXmmImmCallback callback) {
    read(JitWidth::b64, calculateEaa(op), [op, callback, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        SSERegPtr tmp = loadXMMFromMem64(SSE_TMP_INDEX, address);
        (this->*callback)(reg, tmp, op->imm);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::opXmmE64(DecodedOp* op, XmmXmmCallback callback, bool loadDest) {
    read(JitWidth::b64, calculateEaa(op), [op, callback, loadDest, this](MemPtr address) {
        SSERegPtr reg;
        if (loadDest || isSseRegCached(op->reg)) {
            reg = loadCpuXMMReg(op->reg);
        } else {
            reg = getTmpSSE();
        }
        SSERegPtr tmp = loadXMMFromMem64(SSE_TMP_INDEX, address);
        (this->*callback)(reg, tmp);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::opXmmE128Imm(DecodedOp* op, XmmXmmImmCallback callback) {
    read(JitWidth::b128, calculateEaa(op), [op, callback, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        SSERegPtr tmp = loadXMMFromMem128(SSE_TMP_INDEX, address);
        (this->*callback)(reg, tmp, op->imm);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::opXmmE32(DecodedOp* op, XmmXmmCallback callback) {
    read(JitWidth::b32, calculateEaa(op), [op, callback, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        SSERegPtr tmp = loadXMMFromMem32(SSE_TMP_INDEX, address);
        (this->*callback)(reg, tmp);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::opXmmE32Imm(DecodedOp* op, XmmXmmImmCallback callback) {
    read(JitWidth::b32, calculateEaa(op), [op, callback, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        SSERegPtr tmp = loadXMMFromMem32(SSE_TMP_INDEX, address);
        (this->*callback)(reg, tmp, op->imm);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_cvtpi2psXmmMmx(DecodedOp* op) {
    MMXRegPtr rm = loadCpuMMXReg(op->rm);
    SSERegPtr reg = loadCpuXMMReg(op->reg); // high 64-bit remains untouched so we need to load it
    cvtpi2psXmmMmx(reg, rm);
    storeCpuXMMReg(reg, op->reg);
}

void JitSSE::dynamic_cvtpi2psXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](MemPtr address) {        
        SSERegPtr reg = loadCpuXMMReg(op->reg); // high 64-bit remains untouched so we need to load it
        MMXRegPtr tmp = loadMMXFromMem64(SSE_TMP_INDEX, address);
        cvtpi2psXmmMmx(reg, tmp);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_cvtps2piMmxXmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    MMXRegPtr reg = getTmpMMX();
    cvtps2piMmxXmm(reg, rm);
    storeCpuMMXReg(reg, op->reg);
}

void JitSSE::dynamic_cvtps2piMmxE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem64(SSE_TMP_INDEX, address);
        MMXRegPtr reg = getTmpMMX();
        cvtps2piMmxXmm(reg, tmp);
        storeCpuMMXReg(reg, op->reg);
    });
}

void JitSSE::dynamic_cvttps2piMmxXmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    MMXRegPtr reg = getTmpMMX();
    cvttps2piMmxXmm(reg, rm);
    storeCpuMMXReg(reg, op->reg);
}

void JitSSE::dynamic_cvttps2piMmxE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem64(MMX_TMP_INDEX, address);
        MMXRegPtr reg = getTmpMMX();
        cvttps2piMmxXmm(reg, tmp);
        storeCpuMMXReg(reg, op->reg);
    });
}

void JitSSE::dynamic_cvtsi2ssXmmR32(DecodedOp* op) {
    SSERegPtr reg = loadCpuXMMReg(op->reg); // must load since top 96 bits are unchanged
    cvtsi2ssXmmR32(reg, getReadOnlyReg(op->rm));
    storeCpuXMMReg(reg, op->reg);
}

void JitSSE::dynamic_cvtsi2ssXmmE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](MemPtr address) {
        RegPtr reg = getTmpReg();
        readHost(JitWidth::b32, address, reg);
        SSERegPtr sse = loadCpuXMMReg(op->reg); // must load since top 96 bits are unchanged
        cvtsi2ssXmmR32(sse, reg);
        storeCpuXMMReg(sse, op->reg);
    });
}

void JitSSE::dynamic_cvtss2siR32Xmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    cvtss2siR32Xmm(getReg(op->reg, -1, false), rm);
}

void JitSSE::dynamic_cvtss2siR32E32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem32(MMX_TMP_INDEX, address);
        cvtss2siR32Xmm(getReg(op->reg, -1, false), tmp);
    });
}

void JitSSE::dynamic_cvttss2siR32Xmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    cvttss2siR32Xmm(getReg(op->reg, -1, false), rm);
}

void JitSSE::dynamic_cvttss2siR32E32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem32(MMX_TMP_INDEX, address);
        cvttss2siR32Xmm(getReg(op->reg, -1, false), tmp);
    });
}

void JitSSE::dynamic_movupsXmmXmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    storeCpuXMMReg(rm, op->reg);
}

void JitSSE::dynamic_movupsXmmE128(DecodedOp* op) {
    read(JitWidth::b128, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem128(op->reg, address);
        storeCpuXMMReg(tmp, op->reg);
    });
}

void JitSSE::dynamic_movupsE128Xmm(DecodedOp* op) {
    write(JitWidth::b128, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        storeXMMToMem128(reg, address);
    });
}

void JitSSE::dynamic_movhpsXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr reg = loadHighXMMFromMem64(op->reg, address);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_movlpsXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr reg = loadLowXMMFromMem64(op->reg, address);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_movhpsE64Xmm(DecodedOp* op) {
    write(JitWidth::b128, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        storeHighXMMToMem64(reg, address);
    });
}

void JitSSE::dynamic_movlpsE64Xmm(DecodedOp* op) {
    write(JitWidth::b128, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        storeXMMToMem64(reg, address);
    });
}

void JitSSE::dynamic_movmskpsR32Xmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    movmskpsR32Xmm(getReg(op->reg, -1, false), rm);
}

void JitSSE::dynamic_movssXmmE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](MemPtr address) {
        // will fill top 96 with 0's
        SSERegPtr reg = loadXMMFromMem32(op->reg, address);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_movssE32Xmm(DecodedOp* op) {
    write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        storeXMMToMem32(reg, address);
    });
}

void JitSSE::dynamic_stmxcsr(DecodedOp* op) {
    write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        stmxcsr(address);
    });
}

void JitSSE::dynamic_ldmxcsr(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](MemPtr address) {
        ldmxcsr(address);
    });
}

void JitSSE::dynamic_sfence(DecodedOp* op) {
    sfence();
}

void JitSSE::dynamic_comissXmmXmm(DecodedOp* op) {
    SSERegPtr reg = loadCpuXMMReg(op->reg);
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    comissXmmXmm(reg, rm);
}

void JitSSE::dynamic_comissXmmE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem32(SSE_TMP_INDEX, address);
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        comissXmmXmm(reg, tmp);
    });
}

void JitSSE::dynamic_ucomissXmmXmm(DecodedOp* op) {
    SSERegPtr reg = loadCpuXMMReg(op->reg);
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    ucomissXmmXmm(reg, rm);
}

void JitSSE::dynamic_ucomissXmmE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem32(SSE_TMP_INDEX, address);
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        ucomissXmmXmm(reg, tmp);
    });
}

void JitSSE::dynamic_comisdXmmXmm(DecodedOp* op) {
    SSERegPtr reg = loadCpuXMMReg(op->reg);
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    comisdXmmXmm(reg, rm);
}

void JitSSE::dynamic_comisdXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem64(SSE_TMP_INDEX, address);
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        comisdXmmXmm(reg, tmp);
    });
}

void JitSSE::dynamic_ucomisdXmmXmm(DecodedOp* op) {
    SSERegPtr reg = loadCpuXMMReg(op->reg);
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    ucomisdXmmXmm(reg, rm);
}

void JitSSE::dynamic_ucomisdXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem64(SSE_TMP_INDEX, address);
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        ucomisdXmmXmm(reg, tmp);
    });
}

void JitSSE::dynamic_cvtpd2piMmxXmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    MMXRegPtr reg = getTmpMMX();
    cvtpd2piMmxXmm(reg, rm);
    storeCpuMMXReg(reg, op->reg);
}

void JitSSE::dynamic_cvtpd2piMmxE128(DecodedOp* op) {
    read(JitWidth::b128, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem128(SSE_TMP_INDEX, address);
        MMXRegPtr reg = getTmpMMX();
        cvtpd2piMmxXmm(reg, tmp);
        storeCpuMMXReg(reg, op->reg);
    });
}

void JitSSE::dynamic_cvtpi2pdXmmMmx(DecodedOp* op) {
    MMXRegPtr rm = loadCpuMMXReg(op->rm);
    SSERegPtr reg;
    if (isSseRegCached(op->reg)) {
        reg = loadCpuXMMReg(op->reg);
    } else {
        reg = getTmpSSE();
    }
    cvtpi2pdXmmMmx(reg, rm);
    storeCpuXMMReg(reg, op->reg);
}

void JitSSE::dynamic_cvtpi2pdXmmE64(DecodedOp* op) {
    read(JitWidth::b128, calculateEaa(op), [op, this](MemPtr address) {
        MMXRegPtr tmp = loadMMXFromMem64(MMX_TMP_INDEX, address);
        SSERegPtr reg;
        if (isSseRegCached(op->reg)) {
            reg = loadCpuXMMReg(op->reg);
        } else {
            reg = getTmpSSE();
        }
        cvtpi2pdXmmMmx(reg, tmp);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_cvttpd2piMmxXmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    MMXRegPtr tmp = getTmpMMX();
    cvttpd2piMmxXmm(tmp, rm);
    storeCpuMMXReg(tmp, op->reg);
}

void JitSSE::dynamic_cvttpd2piMmE128(DecodedOp* op) {
    read(JitWidth::b128, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem128(SSE_TMP_INDEX, address);
        MMXRegPtr reg = getTmpMMX();
        cvttpd2piMmxXmm(reg, tmp);
        storeCpuMMXReg(reg, op->reg);
    });
}

void JitSSE::dynamic_cvtsd2siR32Xmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    cvtsd2siR32Xmm(getReg(op->reg, -1, false), rm);
}

void JitSSE::dynamic_cvtsd2siR32E64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem64(SSE_TMP_INDEX, address);
        cvtsd2siR32Xmm(getReg(op->reg, -1, false), tmp);
    });
}

void JitSSE::dynamic_cvtsi2sdXmmR32(DecodedOp* op) {
    SSERegPtr reg = loadCpuXMMReg(op->reg); // top bits are unchanged
    cvtsi2sdXmmR32(reg, getReadOnlyReg(op->rm));
    storeCpuXMMReg(reg, op->reg);
}

void JitSSE::dynamic_cvtsi2sdXmmE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](MemPtr address) {
        RegPtr tmp = getTmpReg();
        readHost(JitWidth::b32, address, tmp);
        SSERegPtr reg = loadCpuXMMReg(op->reg); // top bits are unchanged
        cvtsi2sdXmmR32(reg, tmp);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_cvttsd2siR32Xmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    cvttsd2siR32Xmm(getReg(op->reg, -1, false), rm);
}

void JitSSE::dynamic_cvttsd2siR32E64(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr tmp = loadXMMFromMem64(SSE_TMP_INDEX, address);
        cvttsd2siR32Xmm(getReg(op->reg, -1, false), tmp);
    });
}

void JitSSE::dynamic_movqXmmXmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    SSERegPtr reg;
    if (isSseRegCached(op->reg)) {
        reg = loadCpuXMMReg(op->reg);
    } else {
        reg = getTmpSSE();
    }
    movq(reg, rm);
    storeCpuXMMReg(reg, op->reg);
}

void JitSSE::dynamic_movqE64Xmm(DecodedOp* op) {
    write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        storeXMMToMem64(reg, address);
    });
}

void JitSSE::dynamic_movqXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr reg = loadXMMFromMem64(op->reg, address);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_movsdXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr reg = loadXMMFromMem64(op->reg, address);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_movsdE64Xmm(DecodedOp* op) {
    write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        storeXMMToMem64(reg, address);
    });
}

void JitSSE::dynamic_movupdXmmE128(DecodedOp* op) {
    read(JitWidth::b128, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr reg = loadXMMFromMem128(op->reg, address);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_movupdE128Xmm(DecodedOp* op) {
    write(JitWidth::b128, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        storeXMMToMem128(reg, address);
    });
}

void JitSSE::dynamic_movhpdXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](MemPtr address) {
        // low 64 bits are preserved
        SSERegPtr reg = loadHighXMMFromMem64(op->reg, address);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_movhpdE64Xmm(DecodedOp* op) {
    write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        storeHighXMMToMem64(reg, address);
    });
}

void JitSSE::dynamic_movlpdXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr reg = loadLowXMMFromMem64(op->reg, address);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_movlpdE64Xmm(DecodedOp* op) {
    write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        storeXMMToMem64(reg, address);
    });
}

void JitSSE::dynamic_movmskpdR32Xmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    movmskpd(getReg(op->reg, -1, false), rm);
}

void JitSSE::dynamic_movdXmmR32(DecodedOp* op) {
    SSERegPtr reg;
    if (isSseRegCached(op->reg)) {
        reg = loadCpuXMMReg(op->reg);
    } else {
        reg = getTmpSSE();
    }
    movd(reg, getReadOnlyReg(op->rm));
    storeCpuXMMReg(reg, op->reg);
}

void JitSSE::dynamic_movdXmmE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr reg = loadXMMFromMem32(op->reg, address);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_movdR32Xmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    movd(getReg(op->reg, -1, false), rm);
}

void JitSSE::dynamic_movdE32Xmm(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        storeXMMToMem32(reg, address);
    });
}

void JitSSE::dynamic_movdq2qMmxXmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    MMXRegPtr reg = getTmpMMX();
    movdq2q(reg, rm);
    storeCpuMMXReg(reg, op->reg);
}

void JitSSE::dynamic_movq2dqXmmMmx(DecodedOp* op) {
    MMXRegPtr rm = loadCpuMMXReg(op->rm);
    SSERegPtr reg;
    if (isSseRegCached(op->reg)) {
        reg = loadCpuXMMReg(op->reg);
    } else {
        reg = getTmpSSE();
    }
    movq2dq(reg, rm);
    storeCpuXMMReg(reg, op->reg);
}

void JitSSE::dynamic_maskmovdquE128XmmXmm(DecodedOp* op) {
    RegPtr address;
    if (op->base < 6 && cpu->thread->process->hasSetSeg[op->base]) {
        address = getTmpReg(7);
        addReg(JitWidth::b32, address, getReadOnlySegAddress(op->base));
    } else {
        address = getReadOnlyReg(7);
    }
    write(JitWidth::b128, address, nullptr, [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        SSERegPtr rm = loadCpuXMMReg(op->rm);
        maskmovdqu(reg, rm, address);
    });
}

void JitSSE::dynamic_pextrwR32Xmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    pextrwR32Xmm(getReg(op->reg, -1, false), rm, op->imm);
}

void JitSSE::dynamic_pextrwE16Xmm(DecodedOp* op) {
    write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        RegPtr tmp = getTmpReg();
        pextrwR32Xmm(tmp, reg, op->imm);
        writeHost(JitWidth::b16, address, tmp);
    });
}

void JitSSE::dynamic_pinsrwXmmR32(DecodedOp* op) {
    SSERegPtr reg = loadCpuXMMReg(op->reg);
    pinsrwXmmR32(reg, getReadOnlyReg(op->rm), op->imm);;
    storeCpuXMMReg(reg, op->reg);
}

void JitSSE::dynamic_pinsrwXmmE16(DecodedOp* op) {
    read(JitWidth::b16, calculateEaa(op), [op, this](MemPtr address) {
        SSERegPtr reg = loadCpuXMMReg(op->reg);
        RegPtr tmp = getTmpReg();
        readHost(JitWidth::b16, address, tmp);
        pinsrwXmmR32(reg, tmp, op->imm);
        storeCpuXMMReg(reg, op->reg);
    });
}

void JitSSE::dynamic_pmovmskbR32Xmm(DecodedOp* op) {
    SSERegPtr rm = loadCpuXMMReg(op->rm);
    pmovmskbR32Xmm(getReg(op->reg, -1, false), rm);
}

void JitSSE::dynamic_clflush(DecodedOp* op) {
    read(JitWidth::b8, calculateEaa(op), [op, this](MemPtr address) {
        clflush(address);
    });
}

// BSD 3-Clause License
// https://github.com/divideconcept/FastTrigo/blob/master/fasttrigo.cpp

/*

#include <math.h>
const float invtwopi=0.1591549f; // 0x3e22f981
const float twopi=6.283185f; // 0x40c90fda
const float threehalfpi=4.7123889f; // 0x4096cbe4
const float pi=3.141593f; // 0x40490fdc
const float halfpi=1.570796f; // 0x3fc90fd8
const float quarterpi=0.7853982f;

float cos_32s(float x)
{
    const float c1= 0.99940307f; // 0x3f7fd8e1
    const float c2=-0.49558072f; // 0xbefdbcc2
    const float c3= 0.03679168f; // 0x3d16b2df
    float x2;      // The input argument squared
    x2=x * x;
    return (c1 + x2*(c2 + c3 * x2));
}
float cos1(float angle){
    //clamp to the range 0..2pi
    angle=angle-floorf(angle*invtwopi)*twopi;
    angle=angle>0.f?angle:-angle;

    if(angle<halfpi) return cos_32s(angle);
    if(angle<pi) return -cos_32s(pi-angle);
    if(angle<threehalfpi) return -cos_32s(angle-pi);
    return cos_32s(twopi-angle);
}

*/

U8* JitSSE::createJitCosSub() {
    SSERegPtr xmm0 = getTmpSSE(); // counts on getTmpSSE returning regs in the same order
    SSERegPtr tmp = getTmpSSE();
    SSERegPtr tmp2 = getTmpSSE();

    // x2=x * x;
    mulssXmmXmm(xmm0, xmm0);

    // c3 * x2
    RegPtr r = getTmpReg();
    movValue(JitWidth::b32, r, 0x3d16b2df);
    movd(tmp, r);
    mulssXmmXmm(tmp, xmm0);

    // c2 + 
    movValue(JitWidth::b32, r, 0xbefdbcc2);
    movd(tmp2, r);
    addssXmmXmm(tmp, tmp2);

    // x2*(
    mulssXmmXmm(xmm0, tmp);

    // c1 + 
    movValue(JitWidth::b32, r, 0x3f7fd8e1);
    movd(tmp, r);
    addssXmmXmm(xmm0, tmp);

    nakedReturn();
    return createDynamicExecutableMemory();
}

U8* JitSSE::createJitCos() {
    SSERegPtr xmm0 = getTmpSSE(); // counts on getTmpSSE returning regs in the same order
    SSERegPtr xmm1 = getTmpSSE();
    SSERegPtr xmm2 = getTmpSSE();
    SSERegPtr xmm3 = getTmpSSE();

    // angle=angle-floorf(angle*invtwopi)*twopi;

    // angle*invtwopi
    RegPtr reg = getTmpReg();
    movValue(JitWidth::b32, reg, 0x3e22f981);
    movd(xmm2, reg);
    movq(xmm1, xmm0);
    mulssXmmXmm(xmm1, xmm2);

    // result contains angle*invtwopi
    
    // floorf
    // if pos then CVTTSS2SI else { neg CVTTSS2SI neg }
    // __m128 r = _mm_cvtepi32_ps(_mm_cvttps_epi32(f));
    // r = _mm_sub_ss(r, _mm_and_ps(_mm_cmplt_ss(f, r), kOne));
    cvttss2siR32Xmm(reg, xmm1);
    cvtsi2ssXmmR32(xmm2, reg);
    cmpssXmmXmm(xmm1, xmm2, 1); // 1 == less than

    // tmp is r
    // tmp2 is result of _mm_cmplt_ss
    movValue(JitWidth::b32, reg, 0x3f800000); // kOne
    movd(xmm3, reg);
    andpsXmmXmm(xmm1, xmm3);
    subssXmmXmm(xmm2, xmm1);

    // tmp contains floorf(angle*invtwopi)
    // *twopi
    movValue(JitWidth::b32, reg, 0x40c90fda); // twopi
    movd(xmm3, reg);
    mulssXmmXmm(xmm2, xmm3);

    // angle = angle - 
    subssXmmXmm(xmm0, xmm2);

    // angle=angle>0.f?angle:-angle;
    movValue(JitWidth::b32, reg, 0x80000000); // -0.0f
    movd(xmm2, reg);
    andnpsXmmXmm(xmm2, xmm0);

    RegPtr cos = getTmpReg();
    movValue(DYN_PTR, cos, (DYN_PTR_SIZE)cpu->thread->process->jitCosSub);

    // if(angle<halfpi) return cos_32s(angle);
    movValue(JitWidth::b32, reg, 0x3fc90fd8); // halfpi
    movd(xmm3, reg);
    IfSseLessThan(xmm2, xmm3); {
        movq(xmm0, xmm2);
        nakedCall(cos);
        nakedReturn();
    } EndIf();

    // if (angle < pi) return -cos_32s(pi - angle);
    movValue(JitWidth::b32, reg, 0x40490fdc); // pi
    movd(xmm3, reg);
    IfSseLessThan(xmm2, xmm3); {
        subpsXmmXmm(xmm3, xmm2);
        movq(xmm0, xmm3); // cos_32s expects it here
        nakedCall(cos);
        // neg result
        xorpsXmmXmm(xmm3, xmm3);
        subpsXmmXmm(xmm3, xmm0);
        movq(xmm0, xmm3);
        nakedReturn();
    } EndIf();

    // if (angle < threehalfpi) return -cos_32s(angle - pi);
    movValue(JitWidth::b32, reg, 0x4096cbe4); // threehalfpi
    movd(xmm1, reg); // tmp still contains PI so don't overwrite it
    IfSseLessThan(xmm2, xmm1); {
        subpsXmmXmm(xmm2, xmm3);
        movq(xmm0, xmm2);
        nakedCall(cos);
        // neg result
        xorpsXmmXmm(xmm3, xmm3);
        subpsXmmXmm(xmm3, xmm0);
        movq(xmm0, xmm3);
        nakedReturn();
    } EndIf();

    // return cos_32s(twopi - angle);
    movValue(JitWidth::b32, reg, 0x40c90fda); // twopi
    movd(xmm3, reg);
    subpsXmmXmm(xmm3, xmm2);
    movq(xmm0, xmm3); // cos_32s expects it here
    nakedCall(cos);
    nakedReturn();

    return createDynamicExecutableMemory();
}

void JitSSE::createHelpers() {
    JitMMX::createHelpers();
    JitSSE* jit = (JitSSE*)startNewJIT(cpu);
    cpu->thread->process->jitCosSub = (void*)jit->createJitCosSub();
    delete jit;
    jit = (JitSSE*)startNewJIT(cpu);
    cpu->thread->process->jitCos = (void*)jit->createJitCos();
    delete jit;
}

void JitSSE::dynamic_FCOS(DecodedOp* op) {
    if (1) {
        JitCodeGen::dynamic_FCOS(op);
    } else {
        RegPtr cos = getTmpReg();
        FPURegPtr result = getFPUTmp();
        RegPtr top = getTopReg();

        IfNotRegCached(top); {
            JitCodeGen::dynamic_FCOS(op);
        } StartElse(); {
            loadCpuFpuReg(result, top);
            SSERegPtr sse = std::make_shared<SSERegInternal>(result->hardwareReg(), 0xff);            
            cvtsd2ssXmmXmm(sse, sse);
            movValue(DYN_PTR, cos, (DYN_PTR_SIZE)cpu->thread->process->jitCos);
            top = nullptr;
            nakedCall(cos); // does not preserver regs
            top = getTopReg();
            cvtss2sdXmmXmm(sse, sse);
            syncXmmToCPU(top, result, 0);
        } EndIf();
    }
}

void JitSSE::dynamic_FSIN(DecodedOp* op) {
    if (1) {
        JitCodeGen::dynamic_FSIN(op);
    } else {
        RegPtr cos = getTmpReg();
        FPURegPtr result = getFPUTmp();
        RegPtr top = getTopReg();

        IfNotRegCached(top); {
            JitCodeGen::dynamic_FSIN(op);
        } StartElse(); {
            loadCpuFpuReg(result, top);
            SSERegPtr sse = std::make_shared<SSERegInternal>(result->hardwareReg(), 0xff);
            cvtsd2ssXmmXmm(sse, sse);
            {
                SSERegPtr halfPi = getTmpSSE();
                RegPtr reg = getTmpReg();
                movValue(JitWidth::b32, reg, 0x3fc90fd8); // halfpi
                movd(halfPi, reg);
                subpsXmmXmm(halfPi, sse);
                movq(sse, halfPi);
            }
            movValue(DYN_PTR, cos, (DYN_PTR_SIZE)cpu->thread->process->jitCos);
            top = nullptr;
            nakedCall(cos); // does not preserver regs
            top = getTopReg();
            cvtss2sdXmmXmm(sse, sse);
            syncXmmToCPU(top, result, 0);
        } EndIf();
    }
}

// Note that this is only used when there are no segments involved
void JitSSE::movsr(JitWidth valueWidth, U32 size, JitWidth regWidth) {
    if (currentOp->runCount == 0) {
        currentOp->flags2 |= OP_FLAG2_TRACED_STUB;
        emulateSingleOp(); // since this was never run, just stub it out so that we save jit code cache since its a lot of code
        return;
    }
#ifndef BOXEDWINE_64
    // 32-bit build, even with sse, can't handle this pass because it will run out of tmp registers.  8 registers on x86 just isn't enough.
    Jit::movsr(valueWidth, size, regWidth);
#else
    if (currentOp->STR_COUNT > 10 && currentOp->STR_TOTAL / currentOp->STR_COUNT < 16) {
        // for small copies just do it the simple way, no need to optimize with sse
        Jit::movsr(valueWidth, size, regWidth);
        return;
	}
    // will use 128-bit sse instructions to do the copying in 16 byte chunks
    //
    // might be interesting to try 256-bit chunks with YMM registers on AVX capable x64 CPUs, but Arm64 doesn't support 256-bit wide registers so it would require a separate code path for that.
    RegPtr esi = getStringRegEsi();
    RegPtr edi = getStringRegEdi();
    RegPtr ecx = getStringRegEcx();

    auto onFailure = [esi, edi, ecx, this]() {
        forceSyncBackIfNotCached(esi);
        forceSyncBackIfNotCached(edi);
        forceSyncBackIfNotCached(ecx);
        emulateSingleOp();
    };

    SSERegPtr sseReg = getTmpSSE();
    U32 bytesPerIter = 16;
    U32 mask;

    if (valueWidth == JitWidth::b32) {
        mask = 3;
    } else if (valueWidth == JitWidth::b16) {
        mask = 7;
    } else {
        mask = 15;
    }

    auto copyOneForward = [this, valueWidth, regWidth, size, esi, edi, ecx, onFailure]() {
        write(valueWidth, edi, read(valueWidth, esi, nullptr, onFailure), nullptr, onFailure);
        addValue(regWidth, esi, size);
        addValue(regWidth, edi, size);
        decReg(regWidth, ecx);
    };

    auto copyOneBackward = [this, valueWidth, regWidth, size, esi, edi, ecx, onFailure]() {
        write(valueWidth, edi, read(valueWidth, esi, nullptr, onFailure), nullptr, onFailure);
        subValue(regWidth, esi, size);
        subValue(regWidth, edi, size);
        decReg(regWidth, ecx);
    };

    bool doDF1 = currentOp->DF1 || (currentOp->DF1 == 0 && currentOp->DF0 == 0);
    bool doDF0 = currentOp->DF0 || (currentOp->DF1 == 0 && currentOp->DF0 == 0);
    
    if (doDF0 && doDF1) {
        IfDF();
    }
    if (doDF1) {        
        RegPtr delta = getTmpReg();
        mov(regWidth, delta, esi);
        subReg(regWidth, delta, edi);
        IfLessThan2(regWidth, delta, bytesPerIter); {
            If(regWidth, delta); {
                // Overlapping backward copies with EDI below ESI must observe each
                // element's previous write before reading the next lower source.
                U32 label = MarkJumpLocation();
                If(regWidth, ecx); {
                    copyOneBackward();
                    Goto(label);
                } EndIf();
            } EndIf();
        } EndIf();

        // Backward direction (DF=1)
        U32 label1 = MarkJumpLocation();
        IfTest(regWidth, ecx, mask); {
            copyOneBackward();
            Goto(label1);
        } EndIf();

        U32 label = MarkJumpLocation();
        If(regWidth, ecx); {
            RegPtr addr = getTmpReg();
            RegPtr addrOut = getTmpReg();

            subValueWithDest(regWidth, addr, esi, bytesPerIter - size);
            subValueWithDest(regWidth, addrOut, edi, bytesPerIter - size);

            read(JitWidth::b128, addr, [sseReg, this](MemPtr address) {
                loadXMMFromMem128(-1, address, sseReg);
            }, onFailure);

            write(JitWidth::b128, addrOut, nullptr, [sseReg, this](MemPtr address) {
                storeXMMToMem128(sseReg, address);
            }, onFailure);

            subValue(regWidth, esi, bytesPerIter);
            subValue(regWidth, edi, bytesPerIter);
            subValue(regWidth, ecx, bytesPerIter / size);
            Goto(label);
        } EndIf();
    }
    if (doDF1 && doDF0) {
        StartElse();
    }
    if (doDF0) {
        RegPtr delta = getTmpReg();
        mov(regWidth, delta, edi);
        subReg(regWidth, delta, esi);
        IfLessThan2(regWidth, delta, bytesPerIter); {
            If(regWidth, delta); {
                // Overlapping forward copies with EDI above ESI must read source
                // elements after earlier writes have happened.
                U32 label = MarkJumpLocation();
                If(regWidth, ecx); {
                    copyOneForward();
                    Goto(label);
                } EndIf();
            } EndIf();
        } EndIf();

        // Forward direction (DF=0)
        U32 label1 = MarkJumpLocation();
        IfTest(regWidth, ecx, mask); {
            copyOneForward();
            Goto(label1);
        } EndIf();

        U32 label = MarkJumpLocation();
        If(regWidth, ecx); {
            read(JitWidth::b128, esi, [sseReg, this](MemPtr address) {
                loadXMMFromMem128(-1, address, sseReg);
            }, onFailure);

            write(JitWidth::b128, edi, nullptr, [sseReg, this](MemPtr address) {
                storeXMMToMem128(sseReg, address);
            }, onFailure);

            addValue(regWidth, esi, bytesPerIter);
            addValue(regWidth, edi, bytesPerIter);
            subValue(regWidth, ecx, bytesPerIter / size);
            Goto(label);
        } EndIf();
    } 
    if (doDF1 && doDF0) {
        EndIf();
    }
#endif
}
#endif
