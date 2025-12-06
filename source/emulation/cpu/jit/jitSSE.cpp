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

#ifdef BOXEDWINE_DYNAMIC
#include "jitSSE.h"

void JitSSE::opXmmXmm(DecodedOp* op, XmmXmmCallback callback, bool loadDest) {
    if (loadDest) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    }
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    (this->*callback)((DynXMMReg)op->reg, (DynXMMReg)op->rm);
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::opXmmXmmImm(DecodedOp* op, XmmXmmImmCallback callback) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    (this->*callback)((DynXMMReg)op->reg, (DynXMMReg)op->rm, op->imm);
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::opXmmImm(DecodedOp* op, XmmImmCallback callback) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    (this->*callback)((DynXMMReg)op->reg, op->imm);
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::opXmmE128(DecodedOp* op, XmmXmmCallback callback, bool loadDest) {
    read(JitWidth::b128, calculateEaa(op), [op, callback, loadDest, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        if (loadDest) {
            loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        }
        loadXMMFromMem128(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::opXmmE64Imm(DecodedOp* op, XmmXmmImmCallback callback) {
    read(JitWidth::b64, calculateEaa(op), [op, callback, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadXMMFromMem64(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX, op->imm);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::opXmmE64(DecodedOp* op, XmmXmmCallback callback, bool loadDest) {
    read(JitWidth::b64, calculateEaa(op), [op, callback, loadDest, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        if (loadDest) {
            loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        }
        loadXMMFromMem64(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::opXmmE128Imm(DecodedOp* op, XmmXmmImmCallback callback) {
    read(JitWidth::b128, calculateEaa(op), [op, callback, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadXMMFromMem128(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX, op->imm);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::opXmmE32(DecodedOp* op, XmmXmmCallback callback) {
    read(JitWidth::b32, calculateEaa(op), [op, callback, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadXMMFromMem32(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::opXmmE32Imm(DecodedOp* op, XmmXmmImmCallback callback) {
    read(JitWidth::b32, calculateEaa(op), [op, callback, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadXMMFromMem32(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX, op->imm);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_cvtpi2psXmmMmx(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // high 64-bit remains untouched so we need to load it
    cvtpi2psXmmMmx((DynXMMReg)op->reg, (DynMMXReg)op->rm);
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_cvtpi2psXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynMMXReg tmpMMX = getTmpMMX(op->reg);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // high 64-bit remains untouched so we need to load it
        loadMMXFromMem64(tmpMMX, address, offset, 0, 0);
        cvtpi2psXmmMmx((DynXMMReg)op->reg, tmpMMX);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_cvtps2piMmxXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    cvtps2piMmxXmm((DynMMXReg)op->reg, (DynXMMReg)op->rm);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_cvtps2piMmxE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpXMM = getTmpXMM(op->reg);
        loadXMMFromMem64(tmpXMM, address, offset, 0, 0);
        cvtps2piMmxXmm((DynMMXReg)op->reg, tmpXMM);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_cvttps2piMmxXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    cvttps2piMmxXmm((DynMMXReg)op->reg, (DynXMMReg)op->rm);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_cvttps2piMmxE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpXMM = getTmpXMM(op->reg);
        loadXMMFromMem64(tmpXMM, address, offset, 0, 0);
        cvttps2piMmxXmm((DynMMXReg)op->reg, tmpXMM);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_cvtsi2ssXmmR32(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // must load since top 96 bits are unchanged
    cvtsi2ssXmmR32((DynXMMReg)op->reg, getReadOnlyReg(op->rm));
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_cvtsi2ssXmmE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg();
        read(JitWidth::b32, reg, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // must load since top 96 bits are unchanged
        cvtsi2ssXmmR32((DynXMMReg)op->reg, reg);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_cvtss2siR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm); // must load since top 96 bits are unchanged
    cvtss2siR32Xmm(getReg(op->reg, -1, false), (DynXMMReg)op->rm);
    incrementEip(op->len);
}

void JitSSE::dynamic_cvtss2siR32E32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpXMM = getTmpXMM(0);
        loadXMMFromMem32(tmpXMM, address, offset, 0, 0);
        cvtss2siR32Xmm(getReg(op->reg, -1, false), tmpXMM);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_cvttss2siR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm); // must load since top 96 bits are unchanged
    cvttss2siR32Xmm(getReg(op->reg, -1, false), (DynXMMReg)op->rm);
    incrementEip(op->len);
}

void JitSSE::dynamic_cvttss2siR32E32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpXMM = getTmpXMM(0);
        loadXMMFromMem32(tmpXMM, address, offset, 0, 0);
        cvttss2siR32Xmm(getReg(op->reg, -1, false), tmpXMM);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movupsXmmXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    storeCpuXMMReg(DynXMMReg(op->rm), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_movupsXmmE128(DecodedOp* op) {
    read(JitWidth::b128, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpXMM = getTmpXMM(0);
        loadXMMFromMem128(tmpXMM, address, offset, 0, 0);
        storeCpuXMMReg(tmpXMM, op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movupsE128Xmm(DecodedOp* op) {
    write(JitWidth::b128, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem128(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movhpsXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadHighXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movlpsXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadLowXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movhpsE64Xmm(DecodedOp* op) {
    write(JitWidth::b128, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeHighXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movlpsE64Xmm(DecodedOp* op) {
    write(JitWidth::b128, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movmskpsR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    movmskpsR32Xmm(getReg(op->reg, -1, false), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void JitSSE::dynamic_movssXmmE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        // will fill top 96 with 0's
        loadXMMFromMem32(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movssE32Xmm(DecodedOp* op) {
    write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem32(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_stmxcsr(DecodedOp* op) {
    write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        addReg(JitWidth::b32, address, offset);
        stmxcsr(address);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_ldmxcsr(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        addReg(JitWidth::b32, address, offset);
        ldmxcsr(address);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_sfence(DecodedOp* op) {
    sfence();
    incrementEip(op->len);
}

void JitSSE::dynamic_comissXmmXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    comissXmmXmm(DynXMMReg(op->reg), DynXMMReg(op->rm));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlags(FLAGS_NONE);
}

void JitSSE::dynamic_comissXmmE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem32(tmpReg, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        comissXmmXmm(DynXMMReg(op->reg), tmpReg);
        incrementEip(op->len);
    });
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlags(FLAGS_NONE);
}

void JitSSE::dynamic_ucomissXmmXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    ucomissXmmXmm(DynXMMReg(op->reg), DynXMMReg(op->rm));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlags(FLAGS_NONE);
}

void JitSSE::dynamic_ucomissXmmE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem32(tmpReg, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        ucomissXmmXmm(DynXMMReg(op->reg), tmpReg);
        incrementEip(op->len);
    });
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlags(FLAGS_NONE);
}

void JitSSE::dynamic_comisdXmmXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    comisdXmmXmm(DynXMMReg(op->reg), DynXMMReg(op->rm));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlags(FLAGS_NONE);
}

void JitSSE::dynamic_comisdXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem64(tmpReg, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        comisdXmmXmm(DynXMMReg(op->reg), tmpReg);
        incrementEip(op->len);
    });
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlags(FLAGS_NONE);
}

void JitSSE::dynamic_ucomisdXmmXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    ucomisdXmmXmm(DynXMMReg(op->reg), DynXMMReg(op->rm));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlags(FLAGS_NONE);
}

void JitSSE::dynamic_ucomisdXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem64(tmpReg, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        ucomisdXmmXmm(DynXMMReg(op->reg), tmpReg);
        incrementEip(op->len);
    });
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlags(FLAGS_NONE);
}

void JitSSE::dynamic_cvtpd2piMmxXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    cvtpd2piMmxXmm(DynMMXReg(op->reg), DynXMMReg(op->rm));
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_cvtpd2piMmxE128(DecodedOp* op) {
    read(JitWidth::b128, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem128(tmpReg, address, offset, 0, 0);
        cvtpd2piMmxXmm(DynMMXReg(op->reg), tmpReg);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_cvtpi2pdXmmMmx(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    cvtpi2pdXmmMmx(DynXMMReg(op->reg), DynMMXReg(op->rm));
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_cvtpi2pdXmmE64(DecodedOp* op) {
    read(JitWidth::b128, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynMMXReg tmpReg = getTmpMMX(op->reg);
        loadMMXFromMem64(tmpReg, address, offset, 0, 0);
        cvtpi2pdXmmMmx(DynXMMReg(op->reg), tmpReg);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_cvttpd2piMmxXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    cvttpd2piMmxXmm(DynMMXReg(op->reg), DynXMMReg(op->rm));
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_cvttpd2piMmE128(DecodedOp* op) {
    read(JitWidth::b128, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem128(tmpReg, address, offset, 0, 0);
        cvttpd2piMmxXmm(DynMMXReg(op->reg), tmpReg);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_cvtsd2siR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    cvtsd2siR32Xmm(getReg(op->reg, -1, false), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void JitSSE::dynamic_cvtsd2siR32E64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem64(tmpReg, address, offset, 0, 0);
        cvtsd2siR32Xmm(getReg(op->reg, -1, false), tmpReg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_cvtsi2sdXmmR32(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // top bits are unchanged
    cvtsi2sdXmmR32(DynXMMReg(op->reg), getReadOnlyReg(op->rm));
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_cvtsi2sdXmmE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        RegPtr tmp = getTmpReg();
        read(JitWidth::b32, tmp, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // top bits are unchanged
        cvtsi2sdXmmR32(DynXMMReg(op->reg), tmp);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_cvttsd2siR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    cvttsd2siR32Xmm(getReg(op->reg, -1, false), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void JitSSE::dynamic_cvttsd2siR32E64(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem64(tmpReg, address, offset, 0, 0);
        cvttsd2siR32Xmm(getReg(op->reg, -1, false), tmpReg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movqXmmXmm(DecodedOp* op) {
    loadCpuXMMReg64ZeroExtend(DynXMMReg(op->rm), op->rm);
    storeCpuXMMReg(DynXMMReg(op->rm), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_movqE64Xmm(DecodedOp* op) {
    write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movqXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movsdXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movsdE64Xmm(DecodedOp* op) {
    write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movupdXmmE128(DecodedOp* op) {
    read(JitWidth::b128, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadXMMFromMem128(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movupdE128Xmm(DecodedOp* op) {
    write(JitWidth::b128, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem128(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movhpdXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // so that low 64 bits are preserved
        loadHighXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movhpdE64Xmm(DecodedOp* op) {
    write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeHighXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movlpdXmmE64(DecodedOp* op) {
    read(JitWidth::b64, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // so that top 64 bits are preserved
        loadLowXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movlpdE64Xmm(DecodedOp* op) {
    write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movmskpdR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    movmskpd(getReg(op->reg, -1, false), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void JitSSE::dynamic_movdXmmR32(DecodedOp* op) {
    movd(DynXMMReg(op->reg), getReadOnlyReg(op->rm));
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_movdXmmE32(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadXMMFromMem32(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movdR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    movd(getReg(op->reg, -1, false), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void JitSSE::dynamic_movdE32Xmm(DecodedOp* op) {
    read(JitWidth::b32, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem32(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_movdq2qMmxXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    movdq2q(DynMMXReg(op->reg), DynXMMReg(op->rm));
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_movq2dqXmmMmx(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    movq2dq(DynXMMReg(op->reg), DynMMXReg(op->rm));
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_maskmovdquE128XmmXmm(DecodedOp* op) {
    RegPtr address;
    if (op->base < 6 && cpu->thread->process->hasSetSeg[op->base]) {
        address = getTmpReg(7);
        addReg(JitWidth::b32, address, getReadOnlySegAddress(op->base));
    } else {
        address = getReadOnlyReg(7);
    }
    write(JitWidth::b64, address, nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
        addReg(JitWidth::b32, address, offset);
        maskmovdqu(DynXMMReg(op->reg), DynXMMReg(op->rm), address);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_pextrwR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm); // must load since top 96 bits are unchanged
    pextrwR32Xmm(getReg(op->reg, -1, false), (DynXMMReg)op->rm, op->imm);
    incrementEip(op->len);
}

void JitSSE::dynamic_pextrwE16Xmm(DecodedOp* op) {
    write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        RegPtr tmp = getTmpReg();
        pextrwR32Xmm(tmp, (DynXMMReg)op->reg, op->imm);
        write(JitWidth::b16, address, offset, 0, 0, tmp);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_pinsrwXmmR32(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    pinsrwXmmR32((DynXMMReg)op->reg, getReadOnlyReg(op->rm), op->imm);;
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void JitSSE::dynamic_pinsrwXmmE16(DecodedOp* op) {
    read(JitWidth::b16, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        RegPtr tmp = getTmpReg();
        read(JitWidth::b16, tmp, address, offset, 0, 0);
        pinsrwXmmR32((DynXMMReg)op->reg, tmp, op->imm);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    });
}

void JitSSE::dynamic_pmovmskbR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    pmovmskbR32Xmm(getReg(op->reg, -1, false), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void JitSSE::dynamic_clflush(DecodedOp* op) {
    read(JitWidth::b8, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        clflush(address, offset, 0, 0);
        incrementEip(op->len);
    });
}
#endif