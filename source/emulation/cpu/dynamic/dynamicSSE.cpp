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
#include "dynamicSSE.h"

void DynamicCodeGenSSE::opXmmXmm(DecodedOp* op, XmmXmmCallback callback) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    (this->*callback)((DynXMMReg)op->reg, (DynXMMReg)op->rm);
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::opXmmXmmImm(DecodedOp* op, XmmXmmImmCallback callback) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    (this->*callback)((DynXMMReg)op->reg, (DynXMMReg)op->rm, op->imm);
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::opXmmE128(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_128bit, DYN_ADDRESS, true, [op, callback, this](DynReg address, DynReg offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadXMMFromMem128(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}

void DynamicCodeGenSSE::opXmmE128Imm(DecodedOp* op, XmmXmmImmCallback callback, std::function<void()> fallback) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_128bit, DYN_ADDRESS, true, [op, callback, this](DynReg address, DynReg offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadXMMFromMem128(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX, op->imm);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}

void DynamicCodeGenSSE::opXmmE32(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, callback, this](DynReg address, DynReg offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadXMMFromMem32(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}

void DynamicCodeGenSSE::opXmmE32Imm(DecodedOp* op, XmmXmmImmCallback callback, std::function<void()> fallback) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, callback, this](DynReg address, DynReg offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadXMMFromMem32(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX, op->imm);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}

void DynamicCodeGenSSE::dynamic_cvtpi2psXmmMmx(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // high 64-bit remains untouched so we need to load it
    cvtpi2psXmmMmx((DynXMMReg)op->reg, (DynMMXReg)op->rm);
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvtpi2psXmmE64(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_64bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        DynMMXReg tmpMMX = getTmpMMX(op->reg);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // high 64-bit remains untouched so we need to load it
        loadMMXFromMem64(tmpMMX, address, offset, 0, 0);
        cvtpi2psXmmMmx((DynXMMReg)op->reg, tmpMMX);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvtpi2psXmmE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvtps2piMmxXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    cvtps2piMmxXmm((DynMMXReg)op->reg, (DynXMMReg)op->rm);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvtps2piMmxE64(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_64bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        DynXMMReg tmpXMM = getTmpXMM(op->reg);
        loadXMMFromMem64(tmpXMM, address, offset, 0, 0);
        cvtps2piMmxXmm((DynMMXReg)op->reg, tmpXMM);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvtps2piMmxE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvttps2piMmxXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    cvttps2piMmxXmm((DynMMXReg)op->reg, (DynXMMReg)op->rm);
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvttps2piMmxE64(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_64bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        DynXMMReg tmpXMM = getTmpXMM(op->reg);
        loadXMMFromMem64(tmpXMM, address, offset, 0, 0);
        cvttps2piMmxXmm((DynMMXReg)op->reg, tmpXMM);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvttps2piMmxE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvtsi2ssXmmR32(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // must load since top 96 bits are unchanged
    loadReg(op->rm, DYN_SRC, DYN_32bit);
    cvtsi2ssXmmR32((DynXMMReg)op->reg, DYN_SRC);
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvtsi2ssXmmE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        readMem(DYN_SRC, DYN_32bit, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // must load since top 96 bits are unchanged
        cvtsi2ssXmmR32((DynXMMReg)op->reg, DYN_SRC);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvtsi2ssXmmE32(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvtss2siR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm); // must load since top 96 bits are unchanged
    // loadReg(op->reg, DYN_SRC, DYN_32bit); // don't need to load since it will be clobbered
    cvtss2siR32Xmm(DYN_SRC, (DynXMMReg)op->rm);
    storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvtss2siR32E32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        DynXMMReg tmpXMM = getTmpXMM(0);
        loadXMMFromMem32(tmpXMM, address, offset, 0, 0);
        cvtss2siR32Xmm(DYN_SRC, tmpXMM);
        storeReg(op->reg, DYN_SRC, DYN_32bit, true);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvtss2siR32E32(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvttss2siR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm); // must load since top 96 bits are unchanged
    // loadReg(op->reg, DYN_SRC, DYN_32bit); // don't need to load since it will be clobbered
    cvttss2siR32Xmm(DYN_SRC, (DynXMMReg)op->rm);
    storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvttss2siR32E32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        DynXMMReg tmpXMM = getTmpXMM(0);
        loadXMMFromMem32(tmpXMM, address, offset, 0, 0);
        cvttss2siR32Xmm(DYN_SRC, tmpXMM);
        storeReg(op->reg, DYN_SRC, DYN_32bit, true);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvttss2siR32E32(op);
    });
}

void DynamicCodeGenSSE::dynamic_movupsXmmXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    storeCpuXMMReg(DynXMMReg(op->rm), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_movupsXmmE128(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_128bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        DynXMMReg tmpXMM = getTmpXMM(0);
        loadXMMFromMem128(tmpXMM, address, offset, 0, 0);
        storeCpuXMMReg(tmpXMM, op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movupsXmmE128(op);
    });
}

void DynamicCodeGenSSE::dynamic_movupsE128Xmm(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_128bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem128(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movupsE128Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_movhpsXmmE64(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_64bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadHighXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);        
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movhpsXmmE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_movlpsXmmE64(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_64bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadLowXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movlpsXmmE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_movhpsE64Xmm(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_128bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeHighXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movhpsE64Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_movlpsE64Xmm(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_128bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movlpsE64Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_movmskpsR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    movmskpsR32Xmm(DYN_SRC, DynXMMReg(op->rm));
    storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_movssXmmE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        // will fill top 96 with 0's
        loadXMMFromMem32(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movssXmmE32(op);
    });
}

void DynamicCodeGenSSE::dynamic_movssE32Xmm(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem32(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movlpsE64Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_stmxcsr(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        addRegReg(address, offset, DYN_32bit, false);
        stmxcsr(address);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_stmxcsr(op);
    });
}

void DynamicCodeGenSSE::dynamic_ldmxcsr(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        addRegReg(address, offset, DYN_32bit, false);
        ldmxcsr(address);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movssXmmE32(op);
    });
}

void DynamicCodeGenSSE::dynamic_sfence(DecodedOp* op) {
    sfence();
}

void DynamicCodeGenSSE::dynamic_comissXmmXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    comissXmmXmm(DynXMMReg(op->reg), DynXMMReg(op->rm));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlags(FLAGS_NONE);
}

void DynamicCodeGenSSE::dynamic_comissXmmE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem32(tmpReg, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        comissXmmXmm(DynXMMReg(op->reg), tmpReg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_comissXmmE32(op);
    });
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlags(FLAGS_NONE);
}

void DynamicCodeGenSSE::dynamic_ucomissXmmXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    ucomissXmmXmm(DynXMMReg(op->reg), DynXMMReg(op->rm));
    incrementEip(op->len);
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlags(FLAGS_NONE);
}

void DynamicCodeGenSSE::dynamic_ucomissXmmE32(DecodedOp* op) {    
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem32(tmpReg, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        ucomissXmmXmm(DynXMMReg(op->reg), tmpReg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_ucomissXmmE32(op);
    });
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlags(FLAGS_NONE);
}

#endif