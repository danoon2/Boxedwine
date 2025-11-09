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

RegPtr DynamicCodeGenFPU::calculateIndexReg(RegPtr topReg, U32 index) {
    RegPtr result = getTmpReg();
    mov(DYN_32bit, result, topReg);
    addValue(DYN_32bit, result, index, false);
    andValue(DYN_32bit, result, 7, false);
    return result;
}

RegPtr DynamicCodeGenFPU::readFPUTag(RegPtr indexReg) {
    return readCPU(DYN_8bit, indexReg, 0, offsetof(CPU, fpu.tags[0]));
}

void DynamicCodeGenFPU::writeFPUTag(RegPtr indexReg, RegPtr valueReg) {
    writeCPU(DYN_8bit, indexReg, 0, offsetof(CPU, fpu.tags[0]), valueReg);
}

void DynamicCodeGenFPU::dynamic_FPU_PREP_PUSH(RegPtr topReg, bool writeTag) {
    subValue(DYN_32bit, topReg, 1, false);
    andValue(DYN_32bit, topReg, 7, false);
    writeCPU(DYN_32bit, offsetof(CPU, fpu.top), topReg);

    if (writeTag) {
        writeCPUValue(DYN_8bit, topReg, 0, offsetof(CPU, fpu.tags[0]), TAG_Valid);
    }
}

void DynamicCodeGenFPU::IfNotRegCached(RegPtr indexReg, bool bigJump) {
    static_assert(sizeof(cpu->fpu.isRegCached) == 9, "false");
    IfNotCPU(DYN_8bit, indexReg, 0, offsetof(CPU, fpu.isRegCached), bigJump);
}

void DynamicCodeGenFPU::setRegIsCached(RegPtr indexReg, bool regIsCached) {
    writeCPUValue(DYN_8bit, indexReg, 0, offsetof(CPU, fpu.isRegCached), regIsCached ? 1 : 0);
}

// movsd qword ptr[HOST_CPU + topReg*8 + offsetof(CPU, fpu.regs[0].d)], xmm
void DynamicCodeGenFPU::syncXmmToCPU(RegPtr topReg, DynFpuReg fpuReg, U8 regIndex) {
    if (regIndex == 0) {
        syncXmmToCPUWithIndexReg(topReg, fpuReg);
    } else {
        syncXmmToCPUWithIndexReg(calculateIndexReg(topReg, regIndex), fpuReg);
    }
}

void DynamicCodeGenFPU::syncXmmToCPUWithIndexReg(RegPtr indexReg, DynFpuReg xmm) {
    static_assert(sizeof(cpu->fpu.regCache) == 72, "false");
    storeCpuFpuReg(xmm, indexReg);
    setRegIsCached(indexReg, true);
}

static void dynamic_cache_float(CPU* cpu, U32 index) {
    cpu->fpu.getF64(cpu->fpu.STV(index));
    cpu->fpu.getF64(cpu->fpu.STV(0));
}

// movsd xmm, qword ptr[HOST_CPU + topReg*8 + offsetof(CPU, fpu.regs[0].d)]
RegPtr DynamicCodeGenFPU::syncCPUToXmm(RegPtr topReg, DynFpuReg fpuReg, U8 regIndex) {
    RegPtr indexReg = topReg;
    if (regIndex != 0) {
        indexReg = calculateIndexReg(topReg, regIndex);
    }
    IfNotRegCached(indexReg);
        call_I(dynamic_cache_float, (U32)regIndex);
    EndIf();
    loadCpuFpuReg(fpuReg, indexReg);
    return indexReg;
}

RegPtr DynamicCodeGenFPU::getTopReg() {
    return readCPU(DYN_32bit, offsetof(CPU, fpu.top));
}

#define XMM_TMP DYN_FPU_REG_2

class FPUReg {
public:
    FPUReg(DynamicCodeGenFPU* data, RegPtr topReg, U32 regIndex, RegPtr& calculatedIndexReg) {
        this->reg = regIndex ? DYN_FPU_REG_1 : DYN_FPU_REG_0;
        calculatedIndexReg = data->syncCPUToXmm(topReg, this->reg, regIndex);
    }
    FPUReg(DynamicCodeGenFPU* data, RegPtr topReg, U32 regIndex) {
        this->reg = regIndex ? DYN_FPU_REG_1 : DYN_FPU_REG_0;
        data->syncCPUToXmm(topReg, this->reg, regIndex);
    }
    DynFpuReg reg;
};

void DynamicCodeGenFPU::dynamic_SINGLE_REAL(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback, bool reverse) {
    read(DYN_32bit, calculateEaa(op), [reverse, op, callback, this](RegPtr address, RegPtr offset) {
        loadFpuReg(XMM_TMP, address, offset, 0, 0, DYN_FPU_32_BIT);
        fpuRegExtend32To64(XMM_TMP, XMM_TMP);
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        if (reverse) {
            (this->*callback)(XMM_TMP, dst.reg);
        } else {
            (this->*callback)(dst.reg, XMM_TMP);
        }
        syncXmmToCPU(top, reverse ? XMM_TMP : dst.reg, 0);
        incrementEip(op->len);
    }, [fallback] {
        fallback();
    }, true);
}

void DynamicCodeGenFPU::dynamic_FCOM_SINGLE_REAL(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadFpuReg(XMM_TMP, address, offset, 0, 0, DYN_FPU_32_BIT);
        fpuRegExtend32To64(XMM_TMP, XMM_TMP);
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);        

        doFCOM(XMM_TMP, dst.reg, readFPUTag(std::move(top)));
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FCOM_SINGLE_REAL(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FCOM_SINGLE_REAL_Pop(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadFpuReg(XMM_TMP, std::move(address), std::move(offset), 0, 0, DYN_FPU_32_BIT);
        fpuRegExtend32To64(XMM_TMP, XMM_TMP);
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);
        
        doFCOM(XMM_TMP, dst.reg, readFPUTag(top));
        dynamic_FPU_POP(top);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FCOM_SINGLE_REAL_Pop(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FCOMPP(DecodedOp* op) {
    RegPtr top = getTopReg();
    RegPtr index;
    FPUReg reg1(this, top, 1, index);
    FPUReg reg2(this, top, 0);
    RegPtr tag = readFPUTag(top);

    orReg(DYN_8bit, tag, readFPUTag(index), false);
    doFCOM(reg1.reg, reg2.reg, tag);
    tag = nullptr;
    index = nullptr;
    dynamic_FPU_POP(top, 2);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_STi_ST0(DecodedOp* op, XmmXmmCallback callback, bool reverse, bool pop) {
    RegPtr top = getTopReg();
    RegPtr index;
    FPUReg dst(this, top, op->reg, index);
    FPUReg src(this, top, 0);

    if (reverse) {
        (this->*callback)(src.reg, dst.reg);
        syncXmmToCPUWithIndexReg(index, src.reg);
    } else {
        (this->*callback)(dst.reg, src.reg);
        syncXmmToCPUWithIndexReg(index, dst.reg);
    }
    if (pop) {
        dynamic_FPU_POP(top);
    }
    incrementEip(op->len);
}

void DynamicCodeGenFPU::doFCOM_STi(DecodedOp* op, bool pop) {
    RegPtr top = getTopReg();
    RegPtr index;
    FPUReg dst(this, top, op->reg, index);
    FPUReg src(this, top, 0);
    RegPtr tag = readFPUTag(top);

    orReg(DYN_8bit, tag, readFPUTag(std::move(index)), false);
    doFCOM(dst.reg, src.reg, tag);
    if (pop) {
        dynamic_FPU_POP(top);
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

void DynamicCodeGenFPU::dynamic_FPU_POP(RegPtr topReg, U8 amount) {
    // this->tags[this->top] = TAG_Empty;
    // this->top = ((this->top + 1) & 7);
    writeCPUValue(DYN_8bit, topReg, 0, offsetof(CPU, fpu.tags[0]), TAG_Empty);
    addValue(DYN_32bit, topReg, amount, false);
    andValue(DYN_32bit, topReg, 7, false);
    writeCPU(DYN_32bit, offsetof(CPU, fpu.top), topReg);
}

void DynamicCodeGenFPU::dynamic_ST0_STj(DecodedOp* op, XmmXmmCallback callback, bool reverse) {
    RegPtr top = getTopReg();
    FPUReg src(this, top, op->reg);
    FPUReg dst(this, top, 0);
    if (reverse) {
        (this->*callback)(src.reg, dst.reg);
        syncXmmToCPU(top, src.reg, 0);
    } else {
        (this->*callback)(dst.reg, src.reg);
        syncXmmToCPU(top, dst.reg, 0);
    }
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_DOUBLE_REAL(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback, bool reverse) {
    read(DYN_64bit, calculateEaa(op), [reverse, op, callback, this](RegPtr address, RegPtr offset) {
        loadFpuReg(XMM_TMP, address, offset, 0, 0);
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        if (reverse) {
            (this->*callback)(XMM_TMP, dst.reg);
        } else {
            (this->*callback)(dst.reg, XMM_TMP);
        }
        syncXmmToCPU(top, reverse ? XMM_TMP : dst.reg, 0);
        incrementEip(op->len);
    }, [fallback] {
        fallback();
    }, true);
}

void DynamicCodeGenFPU::dynamic_FCOM_DOUBLE_REAL(DecodedOp* op) {
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadFpuReg(XMM_TMP, std::move(address), std::move(offset), 0, 0);
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(XMM_TMP, dst.reg, readFPUTag(top));
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FCOM_DOUBLE_REAL(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FCOM_DOUBLE_REAL_Pop(DecodedOp* op) {
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadFpuReg(XMM_TMP, std::move(address), std::move(offset), 0, 0);
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(XMM_TMP, dst.reg, readFPUTag(top));
        dynamic_FPU_POP(top);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FCOM_DOUBLE_REAL_Pop(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_DWORD_INTEGER(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback, bool reverse) {
    read(DYN_32bit, calculateEaa(op), [reverse, op, callback, this](RegPtr address, RegPtr offset) {
        loadFpuRegFromInt(XMM_TMP, address, offset, 0, 0);
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        if (reverse) {
            (this->*callback)(XMM_TMP, dst.reg);
        } else {
            (this->*callback)(dst.reg, XMM_TMP);
        }
        syncXmmToCPU(top, reverse ? XMM_TMP : dst.reg, 0);
        incrementEip(op->len);
    }, [fallback] {
        fallback();
    }, true);
}

void DynamicCodeGenFPU::dynamic_FICOM_DWORD_INTEGER(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadFpuRegFromInt(XMM_TMP, std::move(address), std::move(offset), 0, 0);
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(XMM_TMP, dst.reg, readFPUTag(top));
        incrementEip(op->len);
    }, [op, this] {
        DynamicCodeGen::dynamic_FICOM_DWORD_INTEGER(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FICOM_DWORD_INTEGER_Pop(DecodedOp* op) {
    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadFpuRegFromInt(XMM_TMP, std::move(address), std::move(offset), 0, 0);
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(XMM_TMP, dst.reg, readFPUTag(top));
        dynamic_FPU_POP(top);
        incrementEip(op->len);
    }, [op, this] {
        DynamicCodeGen::dynamic_FICOM_DWORD_INTEGER_Pop(op);
    }, true);
}

void DynamicCodeGenFPU::loadFpuRegFromShort(DynFpuReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    RegPtr result = getTmpReg();
    read(DYN_16bit, result, rm, sib, 0, 0);
    movsx(DYN_32bit, result, DYN_16bit, result);
    regToFpuReg(reg, std::move(result));
}

void DynamicCodeGenFPU::dynamic_WORD_INTEGER(DecodedOp* op, XmmXmmCallback callback, std::function<void()> fallback, bool reverse) {
    read(DYN_16bit, calculateEaa(op), [reverse, op, callback, this](RegPtr address, RegPtr offset) {
        loadFpuRegFromShort(XMM_TMP, address, offset, 0, 0);
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        if (reverse) {
            (this->*callback)(XMM_TMP, dst.reg);
        } else {
            (this->*callback)(dst.reg, XMM_TMP);
        }
        syncXmmToCPU(top, reverse ? XMM_TMP : dst.reg, 0);
        incrementEip(op->len);
    }, [fallback] {
        fallback();
    }, true);
}

void DynamicCodeGenFPU::dynamic_FICOM_WORD_INTEGER(DecodedOp* op) {
    read(DYN_16bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadFpuRegFromShort(XMM_TMP, address, offset, 0, 0);
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(XMM_TMP, dst.reg, readFPUTag(top));
        incrementEip(op->len);
    }, [op, this] {
        DynamicCodeGen::dynamic_FICOM_WORD_INTEGER(op);
    });
}

void DynamicCodeGenFPU::dynamic_FICOM_WORD_INTEGER_Pop(DecodedOp* op) {
    read(DYN_16bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadFpuRegFromShort(XMM_TMP, address, offset, 0, 0);
        RegPtr top = getTopReg();
        FPUReg dst(this, top, 0);

        doFCOM(XMM_TMP, dst.reg, readFPUTag(top));
        dynamic_FPU_POP(top);
        incrementEip(op->len);
    }, [op, this] {
        DynamicCodeGen::dynamic_FICOM_WORD_INTEGER_Pop(op);
    });
}

void DynamicCodeGenFPU::dynamic_FCHS(DecodedOp* op) {
    RegPtr top = getTopReg();
    FPUReg dst(this, top, 0);
    fpuXor(XMM_TMP, XMM_TMP);
    fpuSub(XMM_TMP, dst.reg);
    syncXmmToCPU(top, XMM_TMP, 0);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FABS(DecodedOp* op) {
    RegPtr top = getTopReg();
    FPUReg dst(this, top, 0);
    loadCpuFpuRegConst(XMM_TMP, offsetof(CPU, fAbs));
    fpuAnd(dst.reg, XMM_TMP);
    syncXmmToCPU(top, dst.reg, 0);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FTST(DecodedOp* op) {
    // this->regs[8].d = 0.0;
    // FCOM(this->top, 8);

    RegPtr top = getTopReg();
    FPUReg dst(this, top, 0);

    fpuXor(XMM_TMP, XMM_TMP);
    doFCOM(XMM_TMP, dst.reg, readFPUTag(top));
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLD_STi(DecodedOp* op) {
    // int reg_from = cpu->fpu.STV(reg);
    // cpu->fpu.PREP_PUSH();
    // cpu->fpu.FST(reg_from, cpu->fpu.STV(0));

    RegPtr top = getTopReg();
    FPUReg fromTmp(this, top, op->reg);
    RegPtr tag;
    if (op->reg) {
        tag = readFPUTag(calculateIndexReg(top, op->reg));
    } else {
        tag = readFPUTag(top);
    }
    dynamic_FPU_PREP_PUSH(top, false); // will change topReg

    syncXmmToCPU(top, fromTmp.reg, 0);
    writeFPUTag(top, tag);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FXCH_STi(DecodedOp* op) {
    // int tag = this->tags[other];
    // struct FPU_Reg reg = this->regs[other];
    // this->tags[other] = this->tags[st];
    // this->regs[other] = this->regs[st];
    // this->tags[st] = tag;
    // this->regs[st] = reg;

    RegPtr top = getTopReg();
    FPUReg from(this, top, op->reg);
    FPUReg to(this, top, 0);

    // exchange xmm
    syncXmmToCPU(top, from.reg, 0);
    syncXmmToCPU(top, to.reg, op->reg);

    RegPtr index = calculateIndexReg(top, op->reg);

    // exchange tags
    RegPtr topIndex = readFPUTag(top);
    writeFPUTag(top, readFPUTag(index));
    writeFPUTag(index, topIndex);

    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FNOP(DecodedOp* op) {
    incrementEip(op->len);
}

void DynamicCodeGenFPU::doFST_STi(DecodedOp* op, bool pop) {
    // cpu->fpu.FST(cpu->fpu.STV(0), cpu->fpu.STV(reg));
    // cpu->fpu.FPOP();    
    RegPtr top = getTopReg();
    RegPtr index = calculateIndexReg(top, op->reg);

    FPUReg src(this, top, 0);

    // copy tag
    writeFPUTag(index, readFPUTag(top));

    syncXmmToCPUWithIndexReg(index, src.reg);

    if (pop) {
        dynamic_FPU_POP(top);
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
    RegPtr top = getTopReg();    
    RegPtr reg = getTmpReg();

    dynamic_FPU_PREP_PUSH(top, true);
    movValue(DYN_32bit, reg, 1);
    regToFpuReg(XMM_TMP, reg);
    syncXmmToCPU(top, XMM_TMP, 0);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::fpuLoadConst(U32 offset) {
    RegPtr top = getTopReg();
    dynamic_FPU_PREP_PUSH(top, true);
    loadCpuFpuRegConst(XMM_TMP, offset);
    syncXmmToCPU(top, XMM_TMP, 0);    
}

void DynamicCodeGenFPU::dynamic_FLDL2T(DecodedOp* op) {
    fpuLoadConst(offsetof(CPU, fL2T));
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDL2E(DecodedOp* op) {
    fpuLoadConst(offsetof(CPU, fL2E));
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDPI(DecodedOp* op) {
    fpuLoadConst(offsetof(CPU, fPi));
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDLG2(DecodedOp* op) {
    fpuLoadConst(offsetof(CPU, fLG2));
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDLN2(DecodedOp* op) {
    fpuLoadConst(offsetof(CPU, fLN2));
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDZ(DecodedOp* op) {
    RegPtr top = getTopReg();

    dynamic_FPU_PREP_PUSH(top, true);
    fpuXor(XMM_TMP, XMM_TMP);
    syncXmmToCPU(top, XMM_TMP, 0);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLD_SINGLE_REAL(DecodedOp* op) {
    // U32 value = readd(address); // might generate PF, so do before we adjust the stack
    // cpu->fpu.PREP_PUSH();
    // cpu->fpu.FLD_F32(value, cpu->fpu.STV(0));

    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadFpuReg(XMM_TMP, address, offset, 0, 0, DYN_FPU_32_BIT);
        fpuRegExtend32To64(XMM_TMP, XMM_TMP);                
        RegPtr top = getTopReg();
        dynamic_FPU_PREP_PUSH(top, true);
        syncXmmToCPU(top, XMM_TMP, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FLD_SINGLE_REAL(op);
    });
}

void DynamicCodeGenFPU::dynamic_FST_SINGLE_REAL(DecodedOp* op) {
    write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);
        fpuReg64To32(src.reg, src.reg);
        storeFpuReg(src.reg, address, offset, 0, 0, DYN_FPU_32_BIT);        
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FST_SINGLE_REAL(op);
    });
}

void DynamicCodeGenFPU::dynamic_FST_SINGLE_REAL_Pop(DecodedOp* op) {
    write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);
        fpuReg64To32(src.reg, src.reg);
        storeFpuReg(src.reg, address, offset, 0, 0, DYN_FPU_32_BIT);
        dynamic_FPU_POP(top);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FST_SINGLE_REAL_Pop(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FST_DOUBLE_REAL(DecodedOp* op) {
    write(DYN_64bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);
        storeFpuReg(src.reg, address, offset, 0, 0);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FST_DOUBLE_REAL(op);
    });
}

void DynamicCodeGenFPU::dynamic_FST_DOUBLE_REAL_Pop(DecodedOp* op) {
    write(DYN_64bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);
        storeFpuReg(src.reg, address, offset, 0, 0);
        dynamic_FPU_POP(top);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FST_DOUBLE_REAL_Pop(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FNSTCW(DecodedOp* op) {
    // cpu->memory->writew(address, cpu->fpu.CW());
    write(DYN_16bit, calculateEaa(op), readCPU(DYN_16bit, offsetof(CPU, fpu.cw)));
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FLDCW(DecodedOp* op) {
    RegPtr cw = read(DYN_16bit, calculateEaa(op));
    movzx(DYN_32bit, cw, DYN_16bit, cw);
    writeCPU(DYN_32bit, offsetof(CPU, fpu.cw), cw);

    shrValue(DYN_32bit, cw, 10, false);
    andValue(DYN_32bit, cw, 3, false);
    writeCPU(DYN_32bit, offsetof(CPU, fpu.round), cw);
    incrementEip(op->len);
}

// motorhead uses this
void DynamicCodeGenFPU::dynamic_FRNDINT(DecodedOp* op) {
    // double value = this->regCache[this->top].d;
    // this->regCache[this->top].d = (double)(S64)FROUND(value);
    RegPtr top = getTopReg();
    FPUReg src(this, top, 0);
    updateFPURounding();
    fpuRegToInt64(src.reg, src.reg, false);
    fpuRegInt64To64(src.reg, src.reg);
    restoreFPURounding();
    syncXmmToCPU(top, src.reg, 0);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FDECSTP(DecodedOp* op) {
    // this->top = (this->top - 1) & 7;
    RegPtr top = getTopReg();
    subValue(DYN_32bit, top, 1, false);
    andValue(DYN_32bit, top, 7, false);
    writeCPU(DYN_32bit, offsetof(CPU, fpu.top), top);
    incrementEip(op->len);
}
void DynamicCodeGenFPU::dynamic_FINCSTP(DecodedOp* op) {
    // this->top = (this->top + 1) & 7;
    RegPtr top = getTopReg();
    addValue(DYN_32bit, top, 1, false);
    andValue(DYN_32bit, top, 7, false);
    writeCPU(DYN_32bit, offsetof(CPU, fpu.top), top);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_doCMov(U8 regIndex) {
    RegPtr top = getTopReg();
    RegPtr index;
    FPUReg src(this, top, regIndex, index);

    syncXmmToCPUWithIndexReg(top, src.reg);
    writeFPUTag(top, readFPUTag(index));
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_CF(DecodedOp* op) {
    IfCondition(B);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_ZF(DecodedOp* op) {
    IfCondition(Z);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_CF_OR_ZF(DecodedOp* op) {
    IfCondition(BE);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_PF(DecodedOp* op) {
    IfCondition(P);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_NCF(DecodedOp* op) {
    IfCondition(NB);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_NZF(DecodedOp* op) {
    IfCondition(NZ);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_NCF_AND_NZF(DecodedOp* op) {
    IfCondition(NBE);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FCMOV_ST0_STj_NPF(DecodedOp* op) {
    IfCondition(NP);
        dynamic_doCMov(op->reg);
    EndIf();
    incrementEip(op->len);
}

// age of empires uses this for path finding
void DynamicCodeGenFPU::dynamic_FSQRT(DecodedOp* op) {
    RegPtr top = getTopReg();
    FPUReg reg(this, top, 0);
    fpuSqrt(reg.reg, reg.reg);
    syncXmmToCPU(top, reg.reg, 0);
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FILD_DWORD_INTEGER(DecodedOp* op) {
    // U32 value = readd(address); // might generate PF, so do before we adjust the stack
    // cpu->fpu.PREP_PUSH();
    // cpu->fpu.FLD_I32(value, cpu->fpu.STV(0));

    read(DYN_32bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        RegPtr top = getTopReg();

        loadFpuRegFromInt(XMM_TMP, address, offset, 0, 0);
        dynamic_FPU_PREP_PUSH(top, true); // will change topReg
        syncXmmToCPUWithIndexReg(top, XMM_TMP);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FILD_DWORD_INTEGER(op);
    });
}

// #define FPU_SET_C0(fpu, C) (fpu)->sw &= ~0x0100; if (C != 0) (fpu)->sw |= 0x0100    
// #define FPU_SET_C1(fpu, C) (fpu)->sw &= ~0x0200; if (C != 0) (fpu)->sw |= 0x0200    
// #define FPU_SET_C2(fpu, C) (fpu)->sw &= ~0x0400; if (C != 0) (fpu)->sw |= 0x0400    
// #define FPU_SET_C3(fpu, C) (fpu)->sw &= ~0x4000; if (C != 0) (fpu)->sw |= 0x4000

void DynamicCodeGenFPU::doFCOM(DynFpuReg fpuReg1, DynFpuReg fpuReg2, RegPtr ordTags) {
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

    RegPtr sw = readCPU(DYN_32bit, offsetof(CPU, fpu.sw));
    fcompare(fpuReg1, fpuReg2, ordTags, [sw, this] {
        // equal
        andValue(DYN_32bit, sw, ~0x0700, false);
        orValue(DYN_32bit, sw, 0x4000, false);
    }, [sw, this] {
        // less than
        andValue(DYN_32bit, sw, ~0x4600, false);
        orValue(DYN_32bit, sw, 0x0100, false);
    }, [sw, this] {
        // greater than
        andValue(DYN_32bit, sw, ~0x4700, false);
    }, [sw, this] {
        // invalid
        andValue(DYN_32bit, sw, ~0x0200, false);
        orValue(DYN_32bit, sw, 0x4500, false);
    });
    writeCPU(DYN_32bit, offsetof(CPU, fpu.sw), sw);
}

void DynamicCodeGenFPU::doFCOMI(DynFpuReg fpuReg1, DynFpuReg fpuReg2, RegPtr ordTags) {
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

    RegPtr flagsReg = getTmpReg();
    xorReg(DYN_32bit, flagsReg, flagsReg, false);

    // shift 8 because popFlagsFromReg expects flags in AH
    fcompare(fpuReg1, fpuReg2, ordTags, [flagsReg, this] {
        // equal
        orValue(DYN_32bit, flagsReg, ZF, false);
    }, [flagsReg, this] {
        // less than
        orValue(DYN_32bit, flagsReg, CF, false);
    }, [flagsReg, this] {
        // greater than
        // nothing
    }, [flagsReg, this] {
        // invalid
        orValue(DYN_32bit, flagsReg, CF | PF | ZF, false);
    });
    storeLazyFlags(FLAGS_NONE);
    currentLazyFlags = FLAGS_NONE;
    setFlags(flagsReg, FMASK_TEST);
}

void DynamicCodeGenFPU::dynamic_FUCOMPP(DecodedOp* op) {
    dynamic_FCOMPP(op);
}

void DynamicCodeGenFPU::dynamic_FNCLEX(DecodedOp* op) {
    // this->sw &= 0x7f00;
    RegPtr sw = readCPU(DYN_32bit, offsetof(CPU, fpu.sw));
    
    andValue(DYN_32bit, sw, 0x7f00, false);
    writeCPU(DYN_32bit, offsetof(CPU, fpu.sw), sw);
}

void DynamicCodeGenFPU::dynamic_FNSTSW(DecodedOp* op) {
    // fpu.sw &= ~0x3800; 
    // fpu.sw |= (fpu.top & 7) << 11
    // writew(address, fpu.SW());

    write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr top = getTopReg();
        RegPtr sw = readCPU(DYN_32bit, offsetof(CPU, fpu.sw));

        andValue(DYN_32bit, sw, ~0x3800, false);
        shlValue(DYN_32bit, top, 11, false);
        orReg(DYN_32bit, sw, top, false);
        write(DYN_16bit, address, offset, 0, 0, sw);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FNSTSW(op);
    });
}

void DynamicCodeGenFPU::dynamic_FNSTSW_AX(DecodedOp* op) {
    RegPtr top = getTopReg();
    RegPtr sw = readCPU(DYN_32bit, offsetof(CPU, fpu.sw));

    andValue(DYN_32bit, sw, ~0x3800, false);
    shlValue(DYN_32bit, top, 11, false);
    orReg(DYN_32bit, sw, top, false);
    mov(DYN_16bit, getReg(0), sw);
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
    writeCPUValue(DYN_32bit, offsetof(CPU, fpu.cw), 0x37f);
    writeCPUValue(DYN_32bit, offsetof(CPU, fpu.sw), 0);
    writeCPUValue(DYN_32bit, offsetof(CPU, fpu.top), 0);
    writeCPUValue(DYN_32bit, offsetof(CPU, fpu.round), 0);
    writeCPUValue(DYN_8bit, offsetof(CPU, fpu.isMMXInUse), 0);
    writeCPUValue(DYN_32bit, offsetof(CPU, fpu.isRegCached), 0);
    writeCPUValue(DYN_32bit, offsetof(CPU, fpu.isRegCached) + 4, 0);

    writeCPUValue(DYN_32bit, offsetof(CPU, fpu.tags[0]), TAG_Empty | (TAG_Empty << 8) | (TAG_Empty << 16) | (TAG_Empty << 24));
    writeCPUValue(DYN_32bit, offsetof(CPU, fpu.tags[0])+4, TAG_Empty | (TAG_Empty << 8) | (TAG_Empty << 16) | (TAG_Empty << 24));
}

void DynamicCodeGenFPU::doFCOMI_ST0_STj(DecodedOp* op, bool pop) {
    RegPtr top = getTopReg();
    RegPtr index;
    FPUReg dst(this, top, op->reg, index);
    FPUReg src(this, top, 0);
    RegPtr tag = readFPUTag(top);

    orReg(DYN_32bit, tag, readFPUTag(std::move(index)), false);
    doFCOMI(dst.reg, src.reg, tag);
    if (pop) {
        dynamic_FPU_POP(top);
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
    write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);

        write(DYN_32bit, address, offset, 0, 0, fpuRegToInt32(src.reg, true));
        dynamic_FPU_POP(top);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FISTTP32(op);
    });
}

void DynamicCodeGenFPU::dynamic_FISTTP64(DecodedOp* op) {
    RegPtr top = getTopReg();

    // some apps seem to do a memcpy like thing with data pushed in and out
    // we don't want the loaded 64-bit int to change because of rounding if its writen directly back out
    // so if its not already cached (64-bit format vs 80-bit), then do the slow way to keep the 64-bit precision
    // 
    // see FPU::FLD_I64
    IfNotRegCached(top, true);
        DynamicCodeGen::dynamic_FISTTP64(op);
    StartElse(true);
        write(DYN_64bit, calculateEaa(op), nullptr, [top, op, this](RegPtr address, RegPtr offset) {
        
            FPUReg src(this, top, 0);
            fpuRegToInt64(src.reg, src.reg, true);
            storeFpuReg(src.reg, address, offset, 0, 0, DYN_FPU_64_BIT);
            dynamic_FPU_POP(top);
            incrementEip(op->len);                
        }, [op, this]() {
            DynamicCodeGen::dynamic_FISTTP64(op);
        }, true);
    EndIf(true);
}

void DynamicCodeGenFPU::dynamic_FIST_DWORD_INTEGER(DecodedOp* op) {
    write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {        
        updateFPURounding(); // set rounding first since it needs 2 tmp regs

        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);
        
        write(DYN_32bit, address, offset, 0, 0, fpuRegToInt32(src.reg, false));
        restoreFPURounding();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FIST_DWORD_INTEGER(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FIST_DWORD_INTEGER_Pop(DecodedOp* op) {
    write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        updateFPURounding(); // set rounding first since it needs 2 tmp regs

        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);
        
        write(DYN_32bit, address, offset, 0, 0, fpuRegToInt32(src.reg, false));
        restoreFPURounding();
        dynamic_FPU_POP(top);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FIST_DWORD_INTEGER_Pop(op);
    }, true);
}

void DynamicCodeGenFPU::doFFREE_STi(DecodedOp* op, bool pop) {
    // cpu->fpu.FFREE_STi(cpu->fpu.STV(reg)); this->tags[st] = TAG_Empty;
    RegPtr top = getTopReg();
    RegPtr index = calculateIndexReg(top, op->reg);
    writeCPUValue(DYN_8bit, index, 0, offsetof(CPU, fpu.tags[0]), TAG_Empty);
    if (pop) {
        dynamic_FPU_POP(top);
    }
    incrementEip(op->len);
}

void DynamicCodeGenFPU::dynamic_FFREE_STi(DecodedOp* op) {
    doFFREE_STi(op, false);
}

void DynamicCodeGenFPU::dynamic_FLD_DOUBLE_REAL(DecodedOp* op) {
    read(DYN_64bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {
        loadFpuReg(XMM_TMP, address, offset, 0, 0);
        RegPtr top = getTopReg();
        dynamic_FPU_PREP_PUSH(top, true);
        syncXmmToCPU(top, XMM_TMP, 0);
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
    read(DYN_16bit, calculateEaa(op), [op, this](RegPtr address, RegPtr offset) {        
        loadFpuRegFromShort(XMM_TMP, address, offset, 0, 0);
        RegPtr top = getTopReg();
        dynamic_FPU_PREP_PUSH(top, true); // will change topReg
        syncXmmToCPUWithIndexReg(top, XMM_TMP);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FILD_WORD_INTEGER(op);
    });
}

// SSE3 instruction
void DynamicCodeGenFPU::dynamic_FISTTP16(DecodedOp* op) {
    write(DYN_16bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);

        write(DYN_16bit, address, offset, 0, 0, fpuRegToInt32(src.reg, true));
        dynamic_FPU_POP(top);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FISTTP16(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FIST_WORD_INTEGER(DecodedOp* op) {
    write(DYN_16bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        updateFPURounding();

        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);

        write(DYN_16bit, address, offset, 0, 0, fpuRegToInt32(src.reg, false));
        restoreFPURounding();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FIST_WORD_INTEGER(op);
    }, true);
}

void DynamicCodeGenFPU::dynamic_FIST_WORD_INTEGER_Pop(DecodedOp* op) {
    write(DYN_16bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        updateFPURounding();

        RegPtr top = getTopReg();
        FPUReg src(this, top, 0);

        write(DYN_16bit, address, offset, 0, 0, fpuRegToInt32(src.reg, false));
        restoreFPURounding();
        dynamic_FPU_POP(top);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_FIST_WORD_INTEGER_Pop(op);
    }, true);
}

#endif