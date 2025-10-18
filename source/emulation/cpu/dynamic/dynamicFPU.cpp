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
#include "dynamicFPU.h"

void DynamicCodeGenFPU::calculateIndexReg(DynReg result, DynReg topReg, U32 index) {
    movToRegFromReg(result, DYN_32bit, topReg, DYN_32bit, false);
    addRegImm(result, DYN_32bit, index);
    andRegImm(result, DYN_32bit, 7);
    regUsed[result] = true;
}

void DynamicCodeGenFPU::readFPUTag(DynReg indexReg, DynReg result) {
    movToRegFromCpu(result, indexReg, 2, offsetof(CPU, fpu.tags[0]), DYN_32bit);
    regUsed[result] = true;
}

void DynamicCodeGenFPU::writeFPUTag(DynReg indexReg, DynReg valueReg) {
    movToCpuFromReg(indexReg, 2, offsetof(CPU, fpu.tags[0]), valueReg, DYN_32bit, false);
}

void DynamicCodeGenFPU::dynamic_FPU_PREP_PUSH(DynReg topReg, bool writeTag) {
    subRegImm(topReg, DYN_32bit, (U32)1);
    andRegImm(topReg, DYN_32bit, (U32)7);
    movToCpuFromReg(offsetof(CPU, fpu.top), topReg, DYN_32bit, false);

    if (writeTag) {
        movToCpu(topReg, 2, offsetof(CPU, fpu.tags[0]), DYN_32bit, (U32)TAG_Valid);
    }
}

void DynamicCodeGenFPU::getIsCachedReg(DynReg result, DynReg indexReg) {
    xorRegReg(result, result, DYN_32bit, false);
    static_assert(sizeof(cpu->fpu.isRegCached) == 9, "false");
    movToRegFromCpu(result, indexReg, 0, offsetof(CPU, fpu.isRegCached), DYN_8bit);
    regUsed[result] = true;
}

void DynamicCodeGenFPU::setRegIsCached(U8 regIsCached, DynReg indexReg) {
    movToCpu(indexReg, 0, offsetof(CPU, fpu.isRegCached), DYN_8bit, regIsCached);
}

// movsd qword ptr[HOST_CPU + topReg*8 + offsetof(CPU, fpu.regs[0].d)], xmm
void DynamicCodeGenFPU::syncXmmToCPU(DynReg topReg, DynFpuReg fpuReg, U8 regIndex, DynReg tmpReg) {
    DynReg indexReg = topReg;
    if (regIndex != 0) {
        indexReg = tmpReg;
        calculateIndexReg(tmpReg, topReg, regIndex);
    }
    syncXmmToCPUWithIndexReg(indexReg, fpuReg, tmpReg);
}

void DynamicCodeGenFPU::syncXmmToCPUWithIndexReg(DynReg indexReg, DynFpuReg xmm, DynReg tmpReg) {
    static_assert(sizeof(cpu->fpu.regCache) == 72, "false");
    storeCpuFpuReg(xmm, indexReg);
    setRegIsCached(1, indexReg);
    regUsed[tmpReg] = false;
}

static void dynamic_cache_float(CPU* cpu, U32 index) {
    cpu->fpu.getF64(cpu->fpu.STV(index));
    cpu->fpu.getF64(cpu->fpu.STV(0));
}

// movsd xmm, qword ptr[HOST_CPU + topReg*8 + offsetof(CPU, fpu.regs[0].d)]
void DynamicCodeGenFPU::syncCPUToXmm(DynReg topReg, DynFpuReg fpuReg, U8 regIndex, DynReg calculatedIndexReg, DynReg tmpReg2, bool doneWithIndexReg) {
    DynReg indexReg = topReg;
    if (regIndex != 0) {
        indexReg = calculatedIndexReg;
        calculateIndexReg(calculatedIndexReg, topReg, regIndex);
    } else if (!doneWithIndexReg) {
        movToRegFromReg(calculatedIndexReg, DYN_32bit, topReg, DYN_32bit, false);
    }
    getIsCachedReg(tmpReg2, indexReg);
    IfNot(tmpReg2, true);
    callHostFunction(dynamic_cache_float, false, 2, 0, DYN_PARAM_CPU, false, regIndex, DYN_PARAM_CONST_32, false);
    EndIf();
    loadCpuFpuReg(fpuReg, indexReg);
    if (doneWithIndexReg) {
        regUsed[calculatedIndexReg] = false;
    }
    regUsed[tmpReg2] = false;
}

void DynamicCodeGenFPU::getTopReg(DynReg reg) {
    movToRegFromCpu(reg, offsetof(CPU, fpu.top), DYN_32bit);
}

#define XMM_TMP DYN_FPU_REG_2

class FPUReg {
public:
    FPUReg(DynamicCodeGenFPU* data, DynReg topReg, U32 regIndex, DynReg calculatedIndexReg, DynReg tmpReg2, bool doneWithIndexReg) {
        this->reg = regIndex ? DYN_FPU_REG_1 : DYN_FPU_REG_0;
        data->syncCPUToXmm(topReg, this->reg, regIndex, calculatedIndexReg, tmpReg2, doneWithIndexReg);
    }
    DynFpuReg reg;
};

void DynamicCodeGenFPU::dynamic_SINGLE_REAL(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback, bool reverse) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [reverse, op, callback, this](DynReg address, DynReg offset) {
        loadFpuReg(XMM_TMP, address, offset, 0, 0, DYN_FPU_32_BIT);
        fpuRegExtend32To64(XMM_TMP, XMM_TMP);
        const DynReg TOP_REG = DYN_ADDRESS;
        this->getTopReg(TOP_REG);
        FPUReg dst(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);

        if (reverse) {
            (this->*callback)(XMM_TMP, dst.reg);
        } else {
            (this->*callback)(dst.reg, XMM_TMP);
        }
        syncXmmToCPU(TOP_REG, reverse ? XMM_TMP : dst.reg, 0, DYN_SRC);
        incrementEip(op->len);
    }, [op, fallback, this]() {
        fallback();
    });
}

void DynamicCodeGenFPU::dynamic_FCOM_SINGLE_REAL(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        loadFpuReg(XMM_TMP, address, offset, 0, 0, DYN_FPU_32_BIT);
        fpuRegExtend32To64(XMM_TMP, XMM_TMP);
        const DynReg TOP_REG = DYN_ADDRESS;
        this->getTopReg(TOP_REG);
        FPUReg dst(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);
        
        const DynReg TOP_TAG = DYN_DEST;
        readFPUTag(TOP_REG, TOP_TAG);

        doFCOM(XMM_TMP, dst.reg, TOP_TAG, DYN_SRC);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FCOM_SINGLE_REAL(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FCOM_SINGLE_REAL_Pop(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        loadFpuReg(XMM_TMP, address, offset, 0, 0, DYN_FPU_32_BIT);
        fpuRegExtend32To64(XMM_TMP, XMM_TMP);
        const DynReg TOP_REG = DYN_ADDRESS;
        this->getTopReg(TOP_REG);
        FPUReg dst(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);

        const DynReg TOP_TAG = DYN_DEST;
        readFPUTag(TOP_REG, TOP_TAG);

        doFCOM(XMM_TMP, dst.reg, TOP_TAG, DYN_SRC);
        dynamic_FPU_POP(TOP_REG);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FCOM_SINGLE_REAL(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FCOMPP(DecodedOp* op) {
    const DynReg TOP_REG = DYN_ADDRESS;
    DynReg INDEX_REG = DYN_SRC;

    getTopReg(TOP_REG);
    FPUReg reg1(this, TOP_REG, 1, INDEX_REG, DYN_DEST, false);
    FPUReg reg2(this, TOP_REG, 0, DYN_CALL_RESULT, DYN_DEST, true);

    const DynReg TOP_TAG = DYN_DEST;
    const DynReg INDEX_TAG = DYN_CALL_RESULT;

    readFPUTag(INDEX_REG, INDEX_TAG);
    readFPUTag(TOP_REG, TOP_TAG);

    orRegReg(TOP_TAG, INDEX_TAG, DYN_32bit, true);
    doFCOM(reg1.reg, reg2.reg, TOP_TAG, INDEX_TAG);
    dynamic_FPU_POP(TOP_REG, 2);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_STi_ST0(DecodedOp* op, XmmXmmCallback callback, bool reverse, bool pop) {
    const DynReg TOP_REG = DYN_ADDRESS;
    DynReg INDEX_REG = DYN_SRC;

    getTopReg(TOP_REG);
    FPUReg dst(this, TOP_REG, op->reg, INDEX_REG, DYN_DEST, false);
    FPUReg src(this, TOP_REG, 0, DYN_CALL_RESULT, DYN_DEST, true);
    if (reverse) {
        (this->*callback)(src.reg, dst.reg);
        syncXmmToCPUWithIndexReg(INDEX_REG, src.reg, DYN_DEST);
    } else {
        (this->*callback)(dst.reg, src.reg);
        syncXmmToCPUWithIndexReg(INDEX_REG, dst.reg, DYN_DEST);
    }
    if (pop) {
        dynamic_FPU_POP(TOP_REG);
    }
    regUsed[DYN_ADDRESS] = false;
    incrementEip(op->len);
}

void DynamicCodeGenFPU::doFCOM_STi(DecodedOp* op, bool pop) {
    const DynReg TOP_REG = DYN_ADDRESS;
    DynReg INDEX_REG = DYN_SRC;
    
    getTopReg(TOP_REG);
    FPUReg dst(this, TOP_REG, op->reg, INDEX_REG, DYN_DEST, false);
    FPUReg src(this, TOP_REG, 0, DYN_CALL_RESULT, DYN_DEST, true);

    const DynReg TOP_TAG = DYN_DEST;
    const DynReg INDEX_TAG = DYN_CALL_RESULT;

    readFPUTag(INDEX_REG, INDEX_TAG);
    readFPUTag(TOP_REG, TOP_TAG);

    orRegReg(TOP_TAG, INDEX_TAG, DYN_32bit, true);
    doFCOM(dst.reg, src.reg, TOP_TAG, INDEX_TAG);
    if (pop) {
        dynamic_FPU_POP(TOP_REG);
    }
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCOM_STi(DecodedOp* op) {
    doFCOM_STi(op, false);
}

void DynamicCodeGenFPU::dynamic_FCOM_STi_Pop(DecodedOp* op) {
    doFCOM_STi(op, true);
}

void DynamicCodeGenFPU::dynamic_FUCOM_STi(DecodedOp* op) {
    doFCOM_STi(op, false);
}

void DynamicCodeGenFPU::dynamic_FUCOM_STi_Pop(DecodedOp* op) {
    doFCOM_STi(op, true);
}

void DynamicCodeGenFPU::dynamic_FPU_POP(DynReg topReg, U8 amount) {
    // this->tags[this->top] = TAG_Empty;
    // this->top = ((this->top + 1) & 7);
    movToCpu(topReg, 2, offsetof(CPU, fpu.tags[0]), DYN_32bit, (U32)TAG_Empty);
    addRegImm(topReg, DYN_32bit, amount);
    andRegImm(topReg, DYN_32bit, 7);
    movToCpuFromReg(offsetof(CPU, fpu.top), topReg, DYN_32bit, false);
}

void DynamicCodeGenFPU::dynamic_ST0_STj(DecodedOp* op, XmmXmmCallback callback, bool reverse) {
    getTopReg(DYN_ADDRESS);
    FPUReg src(this, DYN_ADDRESS, op->reg, DYN_SRC, DYN_DEST, true);
    FPUReg dst(this, DYN_ADDRESS, 0, DYN_SRC, DYN_DEST, true);
    if (reverse) {
        (this->*callback)(src.reg, dst.reg);
        syncXmmToCPU(DYN_ADDRESS, src.reg, 0, DYN_SRC);
    } else {
        (this->*callback)(dst.reg, src.reg);
        syncXmmToCPU(DYN_ADDRESS, dst.reg, 0, DYN_SRC);
    }
    regUsed[DYN_ADDRESS] = false;
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_DOUBLE_REAL(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback, bool reverse) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_64bit, DYN_ADDRESS, true, [reverse, op, callback, this](DynReg address, DynReg offset) {
        loadFpuReg(XMM_TMP, address, offset, 0, 0);
        this->getTopReg(DYN_ADDRESS);
        FPUReg dst(this, DYN_ADDRESS, 0, DYN_SRC, DYN_DEST, true);

        if (reverse) {
            (this->*callback)(XMM_TMP, dst.reg);
        } else {
            (this->*callback)(dst.reg, XMM_TMP);
        }
        syncXmmToCPU(DYN_ADDRESS, reverse ? XMM_TMP : dst.reg, 0, DYN_SRC);
        incrementEip(op->len);
    }, [op, fallback, this]() {
        fallback();
    });
}

void DynamicCodeGenFPU::dynamic_FCOM_DOUBLE_REAL(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_64bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        loadFpuReg(XMM_TMP, address, offset, 0, 0);
        const DynReg TOP_REG = DYN_ADDRESS;
        this->getTopReg(TOP_REG);
        FPUReg dst(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);

        const DynReg TOP_TAG = DYN_DEST;
        readFPUTag(TOP_REG, TOP_TAG);

        doFCOM(XMM_TMP, dst.reg, TOP_TAG, DYN_SRC);
        incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_FCOM_SINGLE_REAL(op);
            }, true);
}

void DynamicCodeGenFPU::dynamic_FCOM_DOUBLE_REAL_Pop(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_64bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        loadFpuReg(XMM_TMP, address, offset, 0, 0);
        const DynReg TOP_REG = DYN_ADDRESS;
        this->getTopReg(TOP_REG);
        FPUReg dst(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);

        const DynReg TOP_TAG = DYN_DEST;
        readFPUTag(TOP_REG, TOP_TAG);

        doFCOM(XMM_TMP, dst.reg, TOP_TAG, DYN_SRC);
        dynamic_FPU_POP(TOP_REG);
        incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_FCOM_SINGLE_REAL(op);
            }, true);
}

void DynamicCodeGenFPU::dynamic_DWORD_INTEGER(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback, bool reverse) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [reverse, op, callback, this](DynReg address, DynReg offset) {
        loadFpuRegFromInt(XMM_TMP, address, offset, 0, 0);
        this->getTopReg(DYN_ADDRESS);
        FPUReg dst(this, DYN_ADDRESS, 0, DYN_SRC, DYN_DEST, true);

        if (reverse) {
            (this->*callback)(XMM_TMP, dst.reg);
        } else {
            (this->*callback)(dst.reg, XMM_TMP);
        }
        syncXmmToCPU(DYN_ADDRESS, reverse ? XMM_TMP : dst.reg, 0, DYN_SRC);
        incrementEip(op->len);
    }, [op, fallback, this]() {
        fallback();
    });
}

void DynamicCodeGenFPU::dynamic_FICOM_DWORD_INTEGER(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        loadFpuRegFromInt(XMM_TMP, address, offset, 0, 0);

        const DynReg TOP_REG = DYN_ADDRESS;
        this->getTopReg(TOP_REG);
        FPUReg dst(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);

        const DynReg TOP_TAG = DYN_DEST;
        readFPUTag(TOP_REG, TOP_TAG);

        doFCOM(XMM_TMP, dst.reg, TOP_TAG, DYN_SRC);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FCOM_SINGLE_REAL(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FICOM_DWORD_INTEGER_Pop(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        loadFpuRegFromInt(XMM_TMP, address, offset, 0, 0);

        const DynReg TOP_REG = DYN_ADDRESS;
        this->getTopReg(TOP_REG);
        FPUReg dst(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);

        const DynReg TOP_TAG = DYN_DEST;
        readFPUTag(TOP_REG, TOP_TAG);

        doFCOM(XMM_TMP, dst.reg, TOP_TAG, DYN_SRC);
        dynamic_FPU_POP(TOP_REG);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FCOM_SINGLE_REAL(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_WORD_INTEGER(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback, bool reverse) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true, [reverse, op, callback, this](DynReg address, DynReg offset) {
        readMem(DYN_DEST, DYN_16bit, address, offset, 0, 0);
        movToRegFromRegSignExtend(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit, false);
        regToFpuReg(XMM_TMP, DYN_DEST);
        this->getTopReg(DYN_ADDRESS);
        FPUReg dst(this, DYN_ADDRESS, 0, DYN_SRC, DYN_DEST, true);

        if (reverse) {
            (this->*callback)(XMM_TMP, dst.reg);
        } else {
            (this->*callback)(dst.reg, XMM_TMP);
        }
        syncXmmToCPU(DYN_ADDRESS, reverse ? XMM_TMP : dst.reg, 0, DYN_SRC);
        incrementEip(op->len);
    }, [op, fallback, this]() {
        fallback();
    });
}

void DynamicCodeGenFPU::dynamic_FICOM_WORD_INTEGER(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        readMem(DYN_DEST, DYN_16bit, address, offset, 0, 0);
        movToRegFromRegSignExtend(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit, false);
        regToFpuReg(XMM_TMP, DYN_DEST);

        const DynReg TOP_REG = DYN_ADDRESS;
        this->getTopReg(TOP_REG);
        FPUReg dst(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);

        const DynReg TOP_TAG = DYN_DEST;
        readFPUTag(TOP_REG, TOP_TAG);

        doFCOM(XMM_TMP, dst.reg, TOP_TAG, DYN_SRC);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FCOM_SINGLE_REAL(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FICOM_WORD_INTEGER_Pop(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        readMem(DYN_DEST, DYN_16bit, address, offset, 0, 0);
        movToRegFromRegSignExtend(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit, false);
        regToFpuReg(XMM_TMP, DYN_DEST);

        const DynReg TOP_REG = DYN_ADDRESS;
        this->getTopReg(TOP_REG);
        FPUReg dst(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);

        const DynReg TOP_TAG = DYN_DEST;
        readFPUTag(TOP_REG, TOP_TAG);

        doFCOM(XMM_TMP, dst.reg, TOP_TAG, DYN_SRC);
        dynamic_FPU_POP(TOP_REG);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FCOM_SINGLE_REAL(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FCHS(DecodedOp* op) {
    getTopReg(DYN_ADDRESS);
    FPUReg dst(this, DYN_ADDRESS, 0, DYN_SRC, DYN_DEST, true);
    fpuXor(XMM_TMP, XMM_TMP);
    fpuSub(XMM_TMP, dst.reg);
    syncXmmToCPU(DYN_ADDRESS, XMM_TMP, 0, DYN_SRC);
    regUsed[DYN_ADDRESS] = false;
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FABS(DecodedOp* op) {
    getTopReg(DYN_ADDRESS);
    FPUReg dst(this, DYN_ADDRESS, 0, DYN_SRC, DYN_DEST, true);
    loadCpuFpuRegConst(XMM_TMP, offsetof(CPU, fAbs));
    fpuAnd(dst.reg, XMM_TMP);
    syncXmmToCPU(DYN_ADDRESS, dst.reg, 0, DYN_SRC);
    regUsed[DYN_ADDRESS] = false;
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FTST(DecodedOp* op) {
    // this->regs[8].d = 0.0;
    // FCOM(this->top, 8);

    const DynReg TOP_REG = DYN_ADDRESS;
    getTopReg(TOP_REG);
    FPUReg dst(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);
    fpuXor(XMM_TMP, XMM_TMP);
    
    const DynReg TOP_TAG = DYN_DEST;
    readFPUTag(TOP_REG, TOP_TAG);

    doFCOM(XMM_TMP, dst.reg, TOP_TAG, DYN_SRC);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLD_STi(DecodedOp* op) {
    // int reg_from = cpu->fpu.STV(reg);
    // cpu->fpu.PREP_PUSH();
    // cpu->fpu.FST(reg_from, cpu->fpu.STV(0));

    getTopReg(DYN_ADDRESS);
    FPUReg fromTmp(this, DYN_ADDRESS, op->reg, DYN_SRC, DYN_DEST, true);
    if (op->reg) {
        calculateIndexReg(DYN_CALL_RESULT, DYN_ADDRESS, op->reg);
        readFPUTag(DYN_CALL_RESULT, DYN_CALL_RESULT);
    } else {
        readFPUTag(DYN_ADDRESS, DYN_CALL_RESULT);
    }
    dynamic_FPU_PREP_PUSH(DYN_ADDRESS, false); // will change topReg

    syncXmmToCPU(DYN_ADDRESS, fromTmp.reg, 0, DYN_SRC);
    writeFPUTag(DYN_ADDRESS, DYN_CALL_RESULT);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FXCH_STi(DecodedOp* op) {
    // int tag = this->tags[other];
    // struct FPU_Reg reg = this->regs[other];
    // this->tags[other] = this->tags[st];
    // this->regs[other] = this->regs[st];
    // this->tags[st] = tag;
    // this->regs[st] = reg;
    const DynReg TOP_REG = DYN_ADDRESS;
    const DynReg INDEX_REG = DYN_CALL_RESULT;

    getTopReg(TOP_REG);
    FPUReg from(this, TOP_REG, op->reg, DYN_SRC, DYN_DEST, true);
    FPUReg to(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);

    // exchange xmm
    syncXmmToCPU(TOP_REG, from.reg, 0, DYN_SRC);
    syncXmmToCPU(TOP_REG, to.reg, op->reg, DYN_SRC);

    calculateIndexReg(INDEX_REG, TOP_REG, op->reg);

    // exchange tags
    readFPUTag(TOP_REG, DYN_SRC);
    readFPUTag(INDEX_REG, DYN_DEST);
    writeFPUTag(TOP_REG, DYN_DEST);
    writeFPUTag(INDEX_REG, DYN_SRC);

    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FNOP(DecodedOp* op) {
    incrementEip(op->len);
}

void DynamicCodeGenFPU::doFST_STi(DecodedOp* op, bool pop) {
    // cpu->fpu.FST(cpu->fpu.STV(0), cpu->fpu.STV(reg));
    // cpu->fpu.FPOP();    

    const DynReg TOP_REG = DYN_ADDRESS;
    const DynReg INDEX_REG = DYN_CALL_RESULT;

    getTopReg(TOP_REG);
    calculateIndexReg(INDEX_REG, TOP_REG, op->reg);

    FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);

    // copy tag
    readFPUTag(TOP_REG, DYN_SRC);
    writeFPUTag(INDEX_REG, DYN_SRC);

    syncXmmToCPUWithIndexReg(INDEX_REG, src.reg, DYN_SRC);

    if (pop) {
        dynamic_FPU_POP(TOP_REG);
    }
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FST_STi(DecodedOp* op) {
    doFST_STi(op, false);
}

void DynamicCodeGenFPU::dynamic_FST_STi_Pop(DecodedOp* op) {
    doFST_STi(op, true);
}

void DynamicCodeGenFPU::dynamic_FLD1(DecodedOp* op) {
    const DynReg TOP_REG = DYN_ADDRESS;

    getTopReg(TOP_REG);
    dynamic_FPU_PREP_PUSH(TOP_REG, true);
    movToReg(DYN_SRC, DYN_32bit, 1);
    regToFpuReg(XMM_TMP, DYN_SRC);
    syncXmmToCPU(TOP_REG, XMM_TMP, 0, DYN_DEST);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDL2T(DecodedOp* op) {
    const DynReg TOP_REG = DYN_ADDRESS;

    getTopReg(TOP_REG);
    dynamic_FPU_PREP_PUSH(TOP_REG, true);
    loadCpuFpuRegConst(XMM_TMP, offsetof(CPU, fL2T));
    syncXmmToCPU(TOP_REG, XMM_TMP, 0, DYN_SRC);
    regUsed[DYN_ADDRESS] = false;
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDL2E(DecodedOp* op) {
    const DynReg TOP_REG = DYN_ADDRESS;

    getTopReg(TOP_REG);
    dynamic_FPU_PREP_PUSH(TOP_REG, true);
    loadCpuFpuRegConst(XMM_TMP, offsetof(CPU, fL2E));
    syncXmmToCPU(TOP_REG, XMM_TMP, 0, DYN_SRC);
    regUsed[DYN_ADDRESS] = false;
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDPI(DecodedOp* op) {
    const DynReg TOP_REG = DYN_ADDRESS;

    getTopReg(TOP_REG);
    dynamic_FPU_PREP_PUSH(TOP_REG, true);
    loadCpuFpuRegConst(XMM_TMP, offsetof(CPU, fPi));
    syncXmmToCPU(TOP_REG, XMM_TMP, 0, DYN_SRC);
    regUsed[DYN_ADDRESS] = false;
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDLG2(DecodedOp* op) {
    const DynReg TOP_REG = DYN_ADDRESS;

    getTopReg(TOP_REG);
    dynamic_FPU_PREP_PUSH(TOP_REG, true);
    loadCpuFpuRegConst(XMM_TMP, offsetof(CPU, fLG2));
    syncXmmToCPU(TOP_REG, XMM_TMP, 0, DYN_SRC);
    regUsed[DYN_ADDRESS] = false;
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDLN2(DecodedOp* op) {
    const DynReg TOP_REG = DYN_ADDRESS;

    getTopReg(TOP_REG);
    dynamic_FPU_PREP_PUSH(TOP_REG, true);
    loadCpuFpuRegConst(XMM_TMP, offsetof(CPU, fLN2));
    syncXmmToCPU(TOP_REG, XMM_TMP, 0, DYN_SRC);
    regUsed[DYN_ADDRESS] = false;
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDZ(DecodedOp* op) {
    const DynReg TOP_REG = DYN_ADDRESS;

    getTopReg(TOP_REG);
    dynamic_FPU_PREP_PUSH(TOP_REG, true);
    fpuXor(XMM_TMP, XMM_TMP);
    syncXmmToCPU(TOP_REG, XMM_TMP, 0, DYN_DEST);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLD_SINGLE_REAL(DecodedOp* op) {
    // U32 value = readd(address); // might generate PF, so do before we adjust the stack
    // cpu->fpu.PREP_PUSH();
    // cpu->fpu.FLD_F32(value, cpu->fpu.STV(0));

    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_ADDRESS;

        loadFpuReg(XMM_TMP, address, offset, 0, 0, DYN_FPU_32_BIT);
        fpuRegExtend32To64(XMM_TMP, XMM_TMP);                
        getTopReg(TOP_REG);
        dynamic_FPU_PREP_PUSH(TOP_REG, true);
        syncXmmToCPU(TOP_REG, XMM_TMP, 0, DYN_DEST);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FLD_SINGLE_REAL(op);
    });
}

void DynamicCodeGenFPU::dynamic_FST_SINGLE_REAL(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST; // can't use DYN_ADDRESS or DYN_SRC, they will be used to pass in address and offset

        getTopReg(TOP_REG);
        FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_CALL_RESULT, true); //not great API, but I know that DYN_SRC won't be used here because it's only needed if the passed in regIndex isn't 0
        fpuReg64To32(src.reg, src.reg);
        storeFpuReg(src.reg, address, offset, 0, 0, DYN_FPU_32_BIT);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FST_SINGLE_REAL(op);
    });
}

void DynamicCodeGenFPU::dynamic_FST_SINGLE_REAL_Pop(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST; // can't use DYN_ADDRESS or DYN_SRC, they will be used to pass in address and offset

        getTopReg(TOP_REG);
        FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_CALL_RESULT, true); //not great API, but I know that DYN_SRC won't be used here because it's only needed if the passed in regIndex isn't 0
        fpuReg64To32(src.reg, src.reg);
        storeFpuReg(src.reg, address, offset, 0, 0, DYN_FPU_32_BIT);
        dynamic_FPU_POP(TOP_REG);
        incrementEip(op->len);
    }, [op, this]() {
        // 3dfx logo triggers this path
        DynamicCodeGen::dynamic_FST_SINGLE_REAL_Pop(op);
    });
}

void DynamicCodeGenFPU::dynamic_FST_DOUBLE_REAL(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_64bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST; // can't use DYN_ADDRESS or DYN_SRC, they will be used to pass in address and offset

        getTopReg(TOP_REG);
        FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_CALL_RESULT, true); //not great API, but I know that DYN_SRC won't be used here because it's only needed if the passed in regIndex isn't 0
        storeFpuReg(src.reg, address, offset, 0, 0, DYN_FPU_64_BIT);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FST_DOUBLE_REAL(op);
    });
}

void DynamicCodeGenFPU::dynamic_FST_DOUBLE_REAL_Pop(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_64bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST; // can't use DYN_ADDRESS or DYN_SRC, they will be used to pass in address and offset

        getTopReg(TOP_REG);
        FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_CALL_RESULT, true); //not great API, but I know that DYN_SRC won't be used here because it's only needed if the passed in regIndex isn't 0
        storeFpuReg(src.reg, address, offset, 0, 0, DYN_FPU_64_BIT);
        dynamic_FPU_POP(TOP_REG);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FST_DOUBLE_REAL_Pop(op);
    });
}

void DynamicCodeGenFPU::dynamic_FNSTCW(DecodedOp* op) {
    // cpu->memory->writew(address, cpu->fpu.CW());
    calculateEaa(op, DYN_ADDRESS);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, fpu.cw), DYN_16bit);
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDCW(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    movToCpuFromReg(offsetof(CPU, fpu.cw), DYN_CALL_RESULT, DYN_32bit, false);
    shrRegImm(DYN_CALL_RESULT, DYN_32bit, 10);
    andRegImm(DYN_CALL_RESULT, DYN_32bit, 3);
    movToCpuFromReg(offsetof(CPU, fpu.round), DYN_CALL_RESULT, DYN_32bit, true);
    incrementEip(op->len);
}

// motorhead uses this
void DynamicCodeGenFPU::dynamic_FRNDINT(DecodedOp* op) {
    // double value = this->regCache[this->top].d;
    // this->regCache[this->top].d = (double)(S64)FROUND(value);
    const DynReg TOP_REG = DYN_ADDRESS;

    getTopReg(TOP_REG);
    FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);
    updateFPURounding(DYN_CALL_RESULT, DYN_DEST);
    getTopReg(TOP_REG);
    fpuRegToInt64(src.reg, src.reg, false);
    fpuRegInt64To64(src.reg, src.reg);
    restoreFPURounding();
    syncXmmToCPU(TOP_REG, src.reg, 0, DYN_DEST);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FDECSTP(DecodedOp* op) {
    // this->top = (this->top - 1) & 7;
    const DynReg TOP_REG = DYN_ADDRESS;

    getTopReg(TOP_REG);
    subRegImm(TOP_REG, DYN_32bit, (U32)1);
    andRegImm(TOP_REG, DYN_32bit, (U32)7);
    movToCpuFromReg(offsetof(CPU, fpu.top), TOP_REG, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicCodeGenFPU::dynamic_FINCSTP(DecodedOp* op) {
    // this->top = (this->top + 1) & 7;
    const DynReg TOP_REG = DYN_ADDRESS;

    getTopReg(TOP_REG);
    addRegImm(TOP_REG, DYN_32bit, (U32)1);
    andRegImm(TOP_REG, DYN_32bit, (U32)7);
    movToCpuFromReg(offsetof(CPU, fpu.top), TOP_REG, DYN_32bit, true);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_doCMov(U8 regIndex) {
    const DynReg TOP_REG = DYN_ADDRESS;
    const DynReg INDEX_REG = DYN_SRC;

    getTopReg(TOP_REG);
    FPUReg src(this, TOP_REG, regIndex, INDEX_REG, DYN_DEST, false);

    syncXmmToCPUWithIndexReg(TOP_REG, src.reg, DYN_DEST);
    readFPUTag(INDEX_REG, DYN_SRC);
    writeFPUTag(TOP_REG, DYN_SRC);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_CF(DecodedOp* op) {
    setConditionInReg(B, DYN_CALL_RESULT);
    If(DYN_CALL_RESULT, true);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_ZF(DecodedOp* op) {
    setConditionInReg(Z, DYN_CALL_RESULT);
    If(DYN_CALL_RESULT, true);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_CF_OR_ZF(DecodedOp* op) {
    setConditionInReg(BE, DYN_CALL_RESULT);
    If(DYN_CALL_RESULT, true);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_PF(DecodedOp* op) {
    setConditionInReg(P, DYN_CALL_RESULT);
    If(DYN_CALL_RESULT, true);
    dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_NCF(DecodedOp* op) {
    setConditionInReg(NB, DYN_CALL_RESULT);
    If(DYN_CALL_RESULT, true);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_NZF(DecodedOp* op) {
    setConditionInReg(NZ, DYN_CALL_RESULT);
    If(DYN_CALL_RESULT, true);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_NCF_AND_NZF(DecodedOp* op) {
    setConditionInReg(NBE, DYN_CALL_RESULT);
    If(DYN_CALL_RESULT, true);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_NPF(DecodedOp* op) {
    setConditionInReg(NP, DYN_CALL_RESULT);
    If(DYN_CALL_RESULT, true);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

// age of empires uses this for path finding
void DynamicCodeGenFPU::dynamic_FSQRT(DecodedOp* op) {
    const DynReg TOP_REG = DYN_ADDRESS;

    getTopReg(TOP_REG);
    FPUReg reg(this, TOP_REG, 0, DYN_SRC, DYN_DEST, true);
    fpuSqrt(reg.reg, reg.reg);
    syncXmmToCPU(TOP_REG, reg.reg, 0, DYN_SRC);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FILD_DWORD_INTEGER(DecodedOp* op) {
    // U32 value = readd(address); // might generate PF, so do before we adjust the stack
    // cpu->fpu.PREP_PUSH();
    // cpu->fpu.FLD_I32(value, cpu->fpu.STV(0));

    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST;

        getTopReg(TOP_REG);
        loadFpuRegFromInt(XMM_TMP, address, offset, 0, 0);
        dynamic_FPU_PREP_PUSH(TOP_REG, true); // will change topReg
        syncXmmToCPUWithIndexReg(TOP_REG, XMM_TMP, DYN_SRC);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FILD_DWORD_INTEGER(op);
    });
}

// #define FPU_SET_C0(fpu, C) (fpu)->sw &= ~0x0100; if (C != 0) (fpu)->sw |= 0x0100    
// #define FPU_SET_C1(fpu, C) (fpu)->sw &= ~0x0200; if (C != 0) (fpu)->sw |= 0x0200    
// #define FPU_SET_C2(fpu, C) (fpu)->sw &= ~0x0400; if (C != 0) (fpu)->sw |= 0x0400    
// #define FPU_SET_C3(fpu, C) (fpu)->sw &= ~0x4000; if (C != 0) (fpu)->sw |= 0x4000

void DynamicCodeGenFPU::doFCOM(DynFpuReg fpuReg1, DynFpuReg fpuReg2, DynReg ordTags, DynReg tmpReg) {
    // if (((this->tags[st] != TAG_Valid) && (this->tags[st] != TAG_Zero)) ||
    // 	((this->tags[other] != TAG_Valid) && (this->tags[other] != TAG_Zero)) || isnan(this->regs[st].d) || isnan(this->regs[other].d)) {
    // 	FPU_SET_C3(this, 1);
    // 	FPU_SET_C2(this, 1);
    // 	FPU_SET_C0(this, 1);
    // 	return;
    // }
    // if (this->regs[st].d == this->regs[other].d) {
    // 	FPU_SET_C3(this, 1);
    // 	FPU_SET_C2(this, 0);
    // 	FPU_SET_C0(this, 0);
    // 	return;
    // }
    // if (this->regs[st].d < this->regs[other].d) {
    // 	FPU_SET_C3(this, 0);
    // 	FPU_SET_C2(this, 0);
    // 	FPU_SET_C0(this, 1);
    // 	return;
    // }
    // st > other
    // FPU_SET_C3(this, 0);
    // FPU_SET_C2(this, 0);
    // FPU_SET_C0(this, 0);	

    const DynReg SW_REG = tmpReg;
    movToRegFromCpu(SW_REG, offsetof(CPU, fpu.sw), DYN_32bit);

    fcompare(fpuReg1, fpuReg2, ordTags, [SW_REG, this] {
        // equal
        andRegImm(SW_REG, DYN_32bit, ~0x0700);
        orRegImm(SW_REG, DYN_32bit, 0x4000);
    }, [SW_REG, this] {
        // less than
        andRegImm(SW_REG, DYN_32bit, ~0x4600);
        orRegImm(SW_REG, DYN_32bit, 0x0100);
    }, [SW_REG, this] {
        // greater than
        andRegImm(SW_REG, DYN_32bit, ~0x4700);
    }, [SW_REG, this] {
        // invalid
        andRegImm(SW_REG, DYN_32bit, ~0x0200);
        orRegImm(SW_REG, DYN_32bit, 0x4500);
    });
    movToCpuFromReg(offsetof(CPU, fpu.sw), SW_REG, DYN_32bit, true);
}

void DynamicCodeGenFPU::doFCOMI(DynFpuReg fpuReg1, DynFpuReg fpuReg2, DynReg ordTags, DynReg flagsReg) {
    // if (((this->tags[st] != TAG_Valid) && (this->tags[st] != TAG_Zero)) ||
    //     ((this->tags[other] != TAG_Valid) && (this->tags[other] != TAG_Zero)) || isnan(this->regs[st].d) || isnan(this->regs[other].d)) {
    //     setFlags(cpu, ZF | PF | CF);
    //     return;
    // }
    // if (this->regs[st].d == this->regs[other].d) {
    //     setFlags(cpu, ZF);
    //     return;
    // }
    // if (this->regs[st].d < this->regs[other].d) {
    //     setFlags(cpu, CF);
    //     return;
    // }
    // st > other
    // setFlags(cpu, 0);

    xorRegReg(flagsReg, flagsReg, DYN_32bit, false);

    // shift 8 because popFlagsFromReg expects flags in AH
    fcompare(fpuReg1, fpuReg2, ordTags, [flagsReg, this] {
        // equal
        orRegImm(flagsReg, DYN_32bit, ZF);
    }, [flagsReg, this] {
        // less than
        orRegImm(flagsReg, DYN_32bit, CF);
    }, [flagsReg, this] {
        // greater than
        // nothing
    }, [flagsReg, this] {
        // invalid
        orRegImm(flagsReg, DYN_32bit, CF | PF | ZF);
    });
    movToCpuFromReg(offsetof(CPU, flags), flagsReg, DYN_32bit, true);
    storeLazyFlags(FLAGS_NONE);
}

void DynamicCodeGenFPU::dynamic_FUCOMPP(DecodedOp* op) {
    dynamic_FCOMPP(op);
}

void DynamicCodeGenFPU::dynamic_FNCLEX(DecodedOp* op) {
    // this->sw &= 0x7f00;
    movToRegFromCpu(DYN_SRC, offsetof(CPU, fpu.sw), DYN_32bit);
    andRegImm(DYN_SRC, DYN_32bit, 0x7f00);
    movToCpuFromReg(offsetof(CPU, fpu.sw), DYN_SRC, DYN_32bit, true);
}

void DynamicCodeGenFPU::dynamic_FNSTSW(DecodedOp* op) {
    // fpu.sw &= ~0x3800; 
    // fpu.sw |= (fpu.top & 7) << 11
    // writew(address, fpu.SW());

    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_16bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST; // can't use DYN_ADDRESS or DYN_SRC, they will be used to pass in address and offset
        const DynReg SW_REG = DYN_CALL_RESULT;

        getTopReg(TOP_REG);
        andRegImm(TOP_REG, DYN_32bit, 7);
        movToRegFromCpu(SW_REG, offsetof(CPU, fpu.sw), DYN_32bit);
        andRegImm(SW_REG, DYN_32bit, ~0x3800);
        shlRegImm(TOP_REG, DYN_32bit, 11);
        orRegReg(SW_REG, TOP_REG, DYN_32bit, true);
        writeMem(SW_REG, DYN_16bit, address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FNSTSW(op);
    });
}

void DynamicCodeGenFPU::dynamic_FNSTSW_AX(DecodedOp* op) {
    const DynReg TOP_REG = DYN_DEST;
    const DynReg SW_REG = DYN_CALL_RESULT;

    getTopReg(TOP_REG);
    andRegImm(TOP_REG, DYN_32bit, 7);
    movToRegFromCpu(SW_REG, offsetof(CPU, fpu.sw), DYN_32bit);
    andRegImm(SW_REG, DYN_32bit, ~0x3800);
    shlRegImm(TOP_REG, DYN_32bit, 11);
    orRegReg(SW_REG, TOP_REG, DYN_32bit, true);
    storeReg(0, SW_REG, DYN_16bit, true);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FNINIT(DecodedOp* op) {
    /* 
    SetCW(0x37F);
    this->sw = 0;
    this->top = FPU_GET_TOP(this);
    this->tags[0] = TAG_Empty;
    this->tags[1] = TAG_Empty;
    this->tags[2] = TAG_Empty;
    this->tags[3] = TAG_Empty;
    this->tags[4] = TAG_Empty;
    this->tags[5] = TAG_Empty;
    this->tags[6] = TAG_Empty;
    this->tags[7] = TAG_Empty;
    this->isMMXInUse = false;
    memset(isRegCached, 0, sizeof(isRegCached));
    */

    movToCpu(offsetof(CPU, fpu.cw), DYN_32bit, 0x37f);
    movToCpu(offsetof(CPU, fpu.sw), DYN_32bit, 0);
    movToCpu(offsetof(CPU, fpu.top), DYN_32bit, 0);
    movToCpu(offsetof(CPU, fpu.round), DYN_32bit, 0);
    movToCpu(offsetof(CPU, fpu.isMMXInUse), DYN_8bit, 0);
    movToCpu(offsetof(CPU, fpu.isRegCached), DYN_32bit, 0);
    movToCpu(offsetof(CPU, fpu.isRegCached)+4, DYN_32bit, 0);

    for (int i = 0; i < 8; i++) {
        movToCpu(offsetof(CPU, fpu.tags[0]) + i * sizeof(U32), DYN_32bit, TAG_Empty);
    }
}

void DynamicCodeGenFPU::doFCOMI_ST0_STj(DecodedOp* op, bool pop) {
    const DynReg TOP_REG = DYN_ADDRESS;
    DynReg INDEX_REG = DYN_SRC;

    getTopReg(TOP_REG);
    FPUReg dst(this, TOP_REG, op->reg, INDEX_REG, DYN_DEST, false);
    FPUReg src(this, TOP_REG, 0, DYN_CALL_RESULT, DYN_DEST, true);

    const DynReg TOP_TAG = DYN_DEST;
    const DynReg INDEX_TAG = DYN_CALL_RESULT;

    readFPUTag(INDEX_REG, INDEX_TAG);
    readFPUTag(TOP_REG, TOP_TAG);

    orRegReg(TOP_TAG, INDEX_TAG, DYN_32bit, true);
    doFCOMI(dst.reg, src.reg, TOP_TAG, INDEX_TAG);
    if (pop) {
        dynamic_FPU_POP(TOP_REG);
    }
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FUCOMI_ST0_STj(DecodedOp* op) {
    doFCOMI_ST0_STj(op, false);
}

void DynamicCodeGenFPU::dynamic_FCOMI_ST0_STj(DecodedOp* op) {
    dynamic_FUCOMI_ST0_STj(op);
}

void DynamicCodeGenFPU::dynamic_FISTTP32(DecodedOp* op) {
    // cpu->fpu.FSTT_I32(cpu, address);
    // cpu->fpu.FPOP();
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST; // can't use DYN_ADDRESS or DYN_SRC, they will be used to pass in address and offset

        getTopReg(TOP_REG);
        FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_CALL_RESULT, true); //not great API, but I know that DYN_SRC won't be used here because it's only needed if the passed in regIndex isn't 0
        fpuRegToInt32(DYN_CALL_RESULT, src.reg, true);
        writeMem(DYN_CALL_RESULT, DYN_32bit, address, offset, 0, 0);
        dynamic_FPU_POP(TOP_REG);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FISTTP32(op);
    });
}

void DynamicCodeGenFPU::dynamic_FISTTP64(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_64bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST; // can't use DYN_ADDRESS or DYN_SRC, they will be used to pass in address and offset

        getTopReg(TOP_REG);

        // some apps seem to do a memcpy like thing with data pushed in and out
        // we don't want the loaded 64-bit int to change because of rounding if its writen directly back out
        // so if its not already cached (64-bit format vs 80-bit), then do the slow way to keep the 64-bit precision
        // 
        // see FPU::FLD_I64
        getIsCachedReg(DYN_CALL_RESULT, TOP_REG);
        If(DYN_CALL_RESULT, true);
            FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_CALL_RESULT, true); //not great API, but I know that DYN_SRC won't be used here because it's only needed if the passed in regIndex isn't 0
            fpuRegToInt64(src.reg, src.reg, true);
            storeFpuReg(src.reg, address, offset, 0, 0, DYN_FPU_64_BIT);
            dynamic_FPU_POP(TOP_REG);
            incrementEip(op->len);
        StartElse();
            DynamicCodeGen::dynamic_FISTTP64(op);
        EndIf();
    }, [op, this]() {
        DynamicCodeGen::dynamic_FISTTP64(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FIST_DWORD_INTEGER(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST; // can't use DYN_ADDRESS or DYN_SRC, they will be used to pass in address and offset

        updateFPURounding(DYN_CALL_RESULT, DYN_DEST);
        getTopReg(TOP_REG);
        FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_CALL_RESULT, true); //not great API, but I know that DYN_SRC won't be used here because it's only needed if the passed in regIndex isn't 0
        fpuRegToInt32(DYN_CALL_RESULT, src.reg, false);
        restoreFPURounding();
        writeMem(DYN_CALL_RESULT, DYN_32bit, address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FIST_DWORD_INTEGER(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FIST_DWORD_INTEGER_Pop(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST; // can't use DYN_ADDRESS or DYN_SRC, they will be used to pass in address and offset

        updateFPURounding(DYN_CALL_RESULT, DYN_DEST);
        getTopReg(TOP_REG);
        FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_CALL_RESULT, true); //not great API, but I know that DYN_SRC won't be used here because it's only needed if the passed in regIndex isn't 0
        fpuRegToInt32(DYN_CALL_RESULT, src.reg, false);
        restoreFPURounding();
        writeMem(DYN_CALL_RESULT, DYN_32bit, address, offset, 0, 0);
        dynamic_FPU_POP(TOP_REG);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FIST_DWORD_INTEGER_Pop(op);
    }, true);
}

void DynamicCodeGenFPU::doFFREE_STi(DecodedOp* op, bool pop) {
    // cpu->fpu.FFREE_STi(cpu->fpu.STV(reg)); this->tags[st] = TAG_Empty;
    const DynReg TOP_REG = DYN_SRC;
    const DynReg INDEX_REG = DYN_DEST;
    getTopReg(TOP_REG);
    calculateIndexReg(INDEX_REG, TOP_REG, op->reg);
    movToCpu(INDEX_REG, 2, offsetof(CPU, fpu.tags[0]), DYN_32bit, TAG_Empty);
    if (pop) {
        dynamic_FPU_POP(TOP_REG);
    }
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FFREE_STi(DecodedOp* op) {
    doFFREE_STi(op, false);
}

void DynamicCodeGenFPU::dynamic_FLD_DOUBLE_REAL(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_64bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_SRC;

        loadFpuReg(XMM_TMP, address, offset, 0, 0);        
        getTopReg(TOP_REG);
        dynamic_FPU_PREP_PUSH(TOP_REG, true);
        syncXmmToCPU(TOP_REG, XMM_TMP, 0, DYN_DEST);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FLD_DOUBLE_REAL(op);
    });
}

void DynamicCodeGenFPU::dynamic_FFREEP_STi(DecodedOp* op) {
    doFFREE_STi(op, true);
}

void DynamicCodeGenFPU::dynamic_FUCOMI_ST0_STj_Pop(DecodedOp* op) {
    doFCOMI_ST0_STj(op, true);
}

void DynamicCodeGenFPU::dynamic_FCOMI_ST0_STj_Pop(DecodedOp* op) {
    doFCOMI_ST0_STj(op, true);
}

// bang bang uses this
void DynamicCodeGenFPU::dynamic_FILD_WORD_INTEGER(DecodedOp* op) {
    // S16 value = (S16)cpu->memory->readw(address); // might generate PF, so do before we adjust the stack
    // cpu->fpu.PREP_PUSH();
    // cpu->fpu.FLD_I16(value, cpu->fpu.STV(0));
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST;

        getTopReg(TOP_REG);
        readMem(DYN_SRC, DYN_16bit, address, offset, 0, 0);        
        movToRegFromRegSignExtend(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false);
        regToFpuReg(XMM_TMP, DYN_SRC);
        dynamic_FPU_PREP_PUSH(TOP_REG, true); // will change topReg
        syncXmmToCPUWithIndexReg(TOP_REG, XMM_TMP, DYN_SRC);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FILD_DWORD_INTEGER(op);
    });
}

// SSE3 instruction
void DynamicCodeGenFPU::dynamic_FISTTP16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST; // can't use DYN_ADDRESS or DYN_SRC, they will be used to pass in address and offset

        getTopReg(TOP_REG);
        FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_CALL_RESULT, true); //not great API, but I know that DYN_SRC won't be used here because it's only needed if the passed in regIndex isn't 0
        fpuRegToInt32(DYN_CALL_RESULT, src.reg, true);
        writeMem(DYN_CALL_RESULT, DYN_32bit, address, offset, 0, 0);
        dynamic_FPU_POP(TOP_REG);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FISTTP16(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FIST_WORD_INTEGER(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST; // can't use DYN_ADDRESS or DYN_SRC, they will be used to pass in address and offset

        updateFPURounding(DYN_CALL_RESULT, DYN_DEST);
        getTopReg(TOP_REG);
        FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_CALL_RESULT, true); //not great API, but I know that DYN_SRC won't be used here because it's only needed if the passed in regIndex isn't 0
        fpuRegToInt32(DYN_CALL_RESULT, src.reg, false);
        restoreFPURounding();
        writeMem(DYN_CALL_RESULT, DYN_16bit, address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FIST_WORD_INTEGER(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FIST_WORD_INTEGER_Pop(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, (DynCallParamType)0, false, true, DYN_SRC, [op, this](DynReg address, DynReg offset) {
        const DynReg TOP_REG = DYN_DEST; // can't use DYN_ADDRESS or DYN_SRC, they will be used to pass in address and offset

        updateFPURounding(DYN_CALL_RESULT, DYN_DEST);
        getTopReg(TOP_REG);
        FPUReg src(this, TOP_REG, 0, DYN_SRC, DYN_CALL_RESULT, true); //not great API, but I know that DYN_SRC won't be used here because it's only needed if the passed in regIndex isn't 0
        fpuRegToInt32(DYN_CALL_RESULT, src.reg, false);
        restoreFPURounding();
        writeMem(DYN_CALL_RESULT, DYN_16bit, address, offset, 0, 0);
        dynamic_FPU_POP(TOP_REG);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FIST_WORD_INTEGER(op);
    }, true);
}

#endif