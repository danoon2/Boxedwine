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

void DynamicCodeGenSSE::opXmmXmm(DecodedOp* op, XmmXmmCallback callback, bool loadDest) {
    if (loadDest) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    }
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

void DynamicCodeGenSSE::opXmmImm(DecodedOp* op, XmmImmCallback callback) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    (this->*callback)((DynXMMReg)op->reg, op->imm);
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::opXmmE128(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback, bool loadDest) {
    read(DYN_128bit, calculateEaa(op), [op, callback, loadDest, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        if (loadDest) {
            loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        }
        loadXMMFromMem128(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}

void DynamicCodeGenSSE::opXmmE64Imm(DecodedOp* op, XmmXmmImmCallback callback, std::function<void()> fallback) {
    read(DYN_64bit, calculateEaa(op), [op, callback, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadXMMFromMem64(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX, op->imm);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}

void DynamicCodeGenSSE::opXmmE64(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback, bool loadDest) {
    read(DYN_64bit, calculateEaa(op), [op, callback, loadDest, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpMMX = getTmpXMM(op->reg);
        if (loadDest) {
            loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        }
        loadXMMFromMem64(tmpMMX, address, offset, 0, 0);
        (this->*callback)((DynXMMReg)op->reg, tmpMMX);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}

void DynamicCodeGenSSE::opXmmE128Imm(DecodedOp* op, XmmXmmImmCallback callback, std::function<void()> fallback) {
    read(DYN_128bit, calculateEaa(op), [op, callback, this](RegPtr address, RegPtr offset) {
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
    read(DYN_32bit, calculateEaa(op), [op, callback, this](RegPtr address, RegPtr offset) {
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
    read(DYN_32bit, calculateEaa(op), [op, callback, this](RegPtr address, RegPtr offset) {
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
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
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
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
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
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
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
    cvtsi2ssXmmR32((DynXMMReg)op->reg, getReadOnlyReg(op->rm));
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvtsi2ssXmmE32(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg();
        read(DYN_32bit, reg, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // must load since top 96 bits are unchanged
        cvtsi2ssXmmR32((DynXMMReg)op->reg, reg);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvtsi2ssXmmE32(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvtss2siR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm); // must load since top 96 bits are unchanged
    cvtss2siR32Xmm(getReg(op->reg, -1, false), (DynXMMReg)op->rm);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvtss2siR32E32(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpXMM = getTmpXMM(0);
        loadXMMFromMem32(tmpXMM, address, offset, 0, 0);
        cvtss2siR32Xmm(getReg(op->reg, -1, false), tmpXMM);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvtss2siR32E32(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvttss2siR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm); // must load since top 96 bits are unchanged
    cvttss2siR32Xmm(getReg(op->reg, -1, false), (DynXMMReg)op->rm);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvttss2siR32E32(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpXMM = getTmpXMM(0);
        loadXMMFromMem32(tmpXMM, address, offset, 0, 0);
        cvttss2siR32Xmm(getReg(op->reg, -1, false), tmpXMM);
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
    read(DYN_128bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpXMM = getTmpXMM(0);
        loadXMMFromMem128(tmpXMM, address, offset, 0, 0);
        storeCpuXMMReg(tmpXMM, op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movupsXmmE128(op);
    });
}

void DynamicCodeGenSSE::dynamic_movupsE128Xmm(DecodedOp* op) {
    write(DYN_128bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem128(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movupsE128Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_movhpsXmmE64(DecodedOp* op) {
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadHighXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);        
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movhpsXmmE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_movlpsXmmE64(DecodedOp* op) {
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadLowXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movlpsXmmE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_movhpsE64Xmm(DecodedOp* op) {
    write(DYN_128bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeHighXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movhpsE64Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_movlpsE64Xmm(DecodedOp* op) {
    write(DYN_128bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movlpsE64Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_movmskpsR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    movmskpsR32Xmm(getReg(op->reg, -1, false), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_movssXmmE32(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        // will fill top 96 with 0's
        loadXMMFromMem32(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movssXmmE32(op);
    });
}

void DynamicCodeGenSSE::dynamic_movssE32Xmm(DecodedOp* op) {
    write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem32(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movlpsE64Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_stmxcsr(DecodedOp* op) {
    write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        addReg(DYN_32bit, address, offset, false);
        stmxcsr(address);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_stmxcsr(op);
    });
}

void DynamicCodeGenSSE::dynamic_ldmxcsr(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        addReg(DYN_32bit, address, offset, false);
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
}

void DynamicCodeGenSSE::dynamic_comissXmmE32(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem32(tmpReg, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        comissXmmXmm(DynXMMReg(op->reg), tmpReg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_comissXmmE32(op);
    });
}

void DynamicCodeGenSSE::dynamic_ucomissXmmXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    ucomissXmmXmm(DynXMMReg(op->reg), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_ucomissXmmE32(DecodedOp* op) {    
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem32(tmpReg, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        ucomissXmmXmm(DynXMMReg(op->reg), tmpReg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_ucomissXmmE32(op);
    });
}

void DynamicCodeGenSSE::dynamic_comisdXmmXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    comisdXmmXmm(DynXMMReg(op->reg), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_comisdXmmE64(DecodedOp* op) {
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem64(tmpReg, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        comisdXmmXmm(DynXMMReg(op->reg), tmpReg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_comisdXmmE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_ucomisdXmmXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    ucomisdXmmXmm(DynXMMReg(op->reg), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_ucomisdXmmE64(DecodedOp* op) {
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem64(tmpReg, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        ucomisdXmmXmm(DynXMMReg(op->reg), tmpReg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_ucomisdXmmE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvtpd2piMmxXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    cvtpd2piMmxXmm(DynMMXReg(op->reg), DynXMMReg(op->rm));
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvtpd2piMmxE128(DecodedOp* op) {
    read(DYN_128bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem128(tmpReg, address, offset, 0, 0);
        cvtpd2piMmxXmm(DynMMXReg(op->reg), tmpReg);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvtpd2piMmxE128(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvtpi2pdXmmMmx(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    cvtpi2pdXmmMmx(DynXMMReg(op->reg), DynMMXReg(op->rm));
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvtpi2pdXmmE64(DecodedOp* op) {
    read(DYN_128bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynMMXReg tmpReg = getTmpMMX(op->reg);
        loadMMXFromMem64(tmpReg, address, offset, 0, 0);
        cvtpi2pdXmmMmx(DynXMMReg(op->reg), tmpReg);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvtpi2pdXmmE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvttpd2piMmxXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    cvttpd2piMmxXmm(DynMMXReg(op->reg), DynXMMReg(op->rm));
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvttpd2piMmE128(DecodedOp* op) {
    read(DYN_128bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem128(tmpReg, address, offset, 0, 0);
        cvttpd2piMmxXmm(DynMMXReg(op->reg), tmpReg);
        storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvttpd2piMmE128(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvtsd2siR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    cvtsd2siR32Xmm(getReg(op->reg, -1, false), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvtsd2siR32E64(DecodedOp* op) {
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem64(tmpReg, address, offset, 0, 0);
        cvtsd2siR32Xmm(getReg(op->reg, -1, false), tmpReg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvtsd2siR32E64(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvtsi2sdXmmR32(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // top bits are unchanged
    cvtsi2sdXmmR32(DynXMMReg(op->reg), getReadOnlyReg(op->rm));
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvtsi2sdXmmE32(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        RegPtr tmp = getTmpReg();
        read(DYN_32bit, tmp, address, offset, 0, 0);
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // top bits are unchanged
        cvtsi2sdXmmR32(DynXMMReg(op->reg), tmp);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvtsi2sdXmmE32(op);
    });
}

void DynamicCodeGenSSE::dynamic_cvttsd2siR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    cvttsd2siR32Xmm(getReg(op->reg, -1, false), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_cvttsd2siR32E64(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        DynXMMReg tmpReg = getTmpXMM(op->reg);
        loadXMMFromMem64(tmpReg, address, offset, 0, 0);
        cvttsd2siR32Xmm(getReg(op->reg, -1, false), tmpReg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cvttsd2siR32E64(op);
    });
}

void DynamicCodeGenSSE::dynamic_movqXmmXmm(DecodedOp* op) {
    loadCpuXMMReg64ZeroExtend(DynXMMReg(op->rm), op->rm);
    storeCpuXMMReg(DynXMMReg(op->rm), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_movqE64Xmm(DecodedOp* op) {
    write(DYN_64bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movqE64Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_movqXmmE64(DecodedOp* op) {
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movqXmmE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_movsdXmmE64(DecodedOp* op) {
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movsdXmmE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_movsdE64Xmm(DecodedOp* op) {
    write(DYN_64bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movsdE64Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_movupdXmmE128(DecodedOp* op) {
    read(DYN_128bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadXMMFromMem128(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movupdXmmE128(op);
    });
}

void DynamicCodeGenSSE::dynamic_movupdE128Xmm(DecodedOp* op) {
    write(DYN_128bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem128(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movupdE128Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_movhpdXmmE64(DecodedOp* op) {
    read(DYN_64bit,calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // so that low 64 bits are preserved
        loadHighXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movhpdXmmE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_movhpdE64Xmm(DecodedOp* op) {
    write(DYN_64bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeHighXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movhpdE64Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_movlpdXmmE64(DecodedOp* op) {
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg); // so that top 64 bits are preserved
        loadLowXMMFromMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movlpdXmmE64(op);
    });
}

void DynamicCodeGenSSE::dynamic_movlpdE64Xmm(DecodedOp* op) {
    write(DYN_64bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem64(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movlpdE64Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_movmskpdR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    movmskpd(getReg(op->reg, -1, false), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_movdXmmR32(DecodedOp* op) {
    movd(DynXMMReg(op->reg), getReadOnlyReg(op->rm));
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_movdXmmE32(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadXMMFromMem32(DynXMMReg(op->reg), address, offset, 0, 0);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movdXmmE32(op);
    });
}

void DynamicCodeGenSSE::dynamic_movdR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    movd(getReg(op->reg, -1, false), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_movdE32Xmm(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        storeXMMToMem32(DynXMMReg(op->reg), address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_movdE32Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_movdq2qMmxXmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    movdq2q(DynMMXReg(op->reg), DynXMMReg(op->rm));
    storeCpuMMXReg(DynMMXReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_movq2dqXmmMmx(DecodedOp* op) {
    loadCpuMMXReg(DynMMXReg(op->rm), op->rm);
    movq2dq(DynXMMReg(op->reg), DynMMXReg(op->rm));
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_maskmovdquE128XmmXmm(DecodedOp* op) {
    RegPtr address;
    if (op->base < 6 && cpu->thread->process->hasSetSeg[op->base]) {
        address = getTmpReg(7);
        addReg(DYN_32bit, address, getReadOnlySegAddress(op->base), false);
    } else {
        address = getReadOnlyReg(7);
    }
    write(DYN_64bit, address, nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
        addReg(DYN_32bit, address, offset, false);
        maskmovdqu(DynXMMReg(op->reg), DynXMMReg(op->rm), address);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_maskmovqEDIMmxMmx(op);
    });
}

void DynamicCodeGenSSE::dynamic_pextrwR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm); // must load since top 96 bits are unchanged
    pextrwR32Xmm(getReg(op->reg, -1, false), (DynXMMReg)op->rm, op->imm);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_pextrwE16Xmm(DecodedOp* op) {
    write(DYN_64bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        RegPtr tmp = getTmpReg();
        pextrwR32Xmm(tmp, (DynXMMReg)op->reg, op->imm);
        write(DYN_16bit, address, offset, 0, 0, tmp);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_pextrwE16Xmm(op);
    });
}

void DynamicCodeGenSSE::dynamic_pinsrwXmmR32(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->reg), op->reg);    
    pinsrwXmmR32((DynXMMReg)op->reg, getReadOnlyReg(op->rm), op->imm);;
    storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_pinsrwXmmE16(DecodedOp* op) {
    read(DYN_16bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadCpuXMMReg(DynXMMReg(op->reg), op->reg);
        RegPtr tmp = getTmpReg();
        read(DYN_16bit, tmp, address, offset, 0, 0);
        pinsrwXmmR32((DynXMMReg)op->reg, tmp, op->imm);
        storeCpuXMMReg(DynXMMReg(op->reg), op->reg);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_pinsrwXmmE16(op);
    });
}

void DynamicCodeGenSSE::dynamic_pmovmskbR32Xmm(DecodedOp* op) {
    loadCpuXMMReg(DynXMMReg(op->rm), op->rm);
    pmovmskbR32Xmm(getReg(op->reg, -1, false), DynXMMReg(op->rm));
    incrementEip(op->len);
}

void DynamicCodeGenSSE::dynamic_clflush(DecodedOp* op) {
    read(DYN_8bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        clflush(address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_clflush(op);
    });
}
#endif