#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
#include "armv8btOps.h"
#include "../normal/instructions.h"
#include "armv8btOps_string.h"
#include "armv8btOps_shift.h"
#include "armv8btOps_sse_convert.h"
#include "armv8btOps_sse_minmax.h"
#include "armv8btOps_sse_shuffle.h"
#include "armv8btOps_bits.h"
#include "armv8btOps_mmx.h"
#include "armv8btOps_fpu.h"
#include "armv8btCPU.h"

#include "../common/common_other.h"

void setupFlagsForArith(Armv8btAsm* data, Arm8BtFlags* lazyFlags, U32& flags, bool& hardwareFlags, bool& usesSrc, bool& usesDst, bool& usesResult, bool& resultNeedsZeroExtends) {
    flags = data->flagsNeeded();
    
    if (!flags) {
        hardwareFlags = false;
        usesSrc = false;
        usesDst = false;
        usesResult = false;
    } else {
        // find out if xSrc, xDst and/or xResult will be by flag calculations so that we don't waste time populating them if they won't be used
        hardwareFlags = lazyFlags->usesHardwareFlags(flags);
        usesSrc = lazyFlags->usesSrc(flags);
        usesDst = lazyFlags->usesDst(flags);
        usesResult = lazyFlags->usesResult(flags);
    }
    // zf lazy flag calculation assumes result will be zero extended
    if ((flags & ZF) != 0) {
        resultNeedsZeroExtends = true;
    }
}

U8 setupRegForArith(Armv8btAsm* data, U8 untranslatedReg, bool usesFlagReg, U8 flagReg, U32 width) {
    U8 readReg;

    if (width == 8) {
        // 8 bit x86 registers are tricky because the 4 high bit registers (AH, CH, DH, BH) are the 2nd byte of EAX,ECX,EDX,EBX
        if (usesFlagReg) {
            readReg = flagReg;
            data->movReg8ToReg(untranslatedReg, flagReg);
        } else {
            readReg = data->getReadNativeReg8(untranslatedReg);
        }
    } else {
        if (usesFlagReg) {
            readReg = flagReg;
            data->movRegToReg(flagReg, data->getNativeReg(untranslatedReg), width, true);
        } else {
            readReg = data->getNativeReg(untranslatedReg);
        }
    }
    return readReg;
}

void writeResultArithReg(Armv8btAsm* data, bool usesResult, bool needsResult, bool resultNeedsZeroExtends, U32 width) {
    // resultNeedsZeroExtends: for example, AND/OR will never need to be zero extended
    if (usesResult && width < 32 && resultNeedsZeroExtends) {
        // for example, if width is 8-bit and we added 0xFF and 0xFF, we don't want the xResult to be 1FE, instead it should be 0xFE
        data->zeroExtend(xResult, xResult, width);
    }
    if (needsResult) {
        if (width == 8) {
            data->movRegToReg8(xResult, data->currentOp->reg);
        } else {
            data->movRegToReg(data->getNativeReg(data->currentOp->reg), xResult, width, false);
        }
    }
}

// arith functions assume all flags are set, no flag dependency with previous ops
void arithRE(Armv8btAsm* data, arithReg32 pfn, Arm8BtFlags* lazyFlags, U32 width, bool needsResult, bool resultNeedsZeroExtends) {
    U8 addressReg = data->getAddressReg();
    U32 flags;
    bool hardwareFlags, usesSrc, usesDst, usesResult;
    setupFlagsForArith(data, lazyFlags, flags, hardwareFlags, usesSrc, usesDst, usesResult, resultNeedsZeroExtends);

    data->readMemory(addressReg, xSrc, width, true);

    U8 readRegDst = setupRegForArith(data, data->currentOp->reg, usesDst, xDst, width);

    if (needsResult && !usesResult && !usesDst && width == 32) {
        pfn(data, readRegDst, readRegDst, xSrc, hardwareFlags);
    } else {
        pfn(data, xResult, readRegDst, xSrc, hardwareFlags);
        writeResultArithReg(data, usesResult, needsResult, resultNeedsZeroExtends, width);
    }
    if (width == 8) {
        data->releaseNativeReg8(readRegDst);
    }
    data->releaseTmpReg(addressReg);
    lazyFlags->setFlags(data, flags);
}

void arithER(Armv8btAsm* data, arithReg32 pfn, Arm8BtFlags* lazyFlags, U32 width, bool needsResult, bool resultNeedsZeroExtends) {
    U8 addressReg = data->getAddressReg();
    U32 flags;
    bool hardwareFlags, usesSrc, usesDst, usesResult;
    setupFlagsForArith(data, lazyFlags, flags, hardwareFlags, usesSrc, usesDst, usesResult, resultNeedsZeroExtends);
    
    U8 readRegSrc = setupRegForArith(data, data->currentOp->reg, usesSrc, xSrc, width);

    data->readWriteMemory(addressReg, xDst, xResult, width, [pfn, data, readRegSrc, hardwareFlags] {
        pfn(data, xResult, xDst, readRegSrc, hardwareFlags);
        }, data->currentOp->lock != 0, needsResult);

    // resultNeedsZeroExtends: for example, AND/OR will never need to be zero extended
    if (usesResult && width != 32 && resultNeedsZeroExtends) {
        data->zeroExtend(xResult, xResult, width);
    }    
    if (width == 8) {
        data->releaseNativeReg8(readRegSrc);
    }
    data->releaseTmpReg(addressReg);
    lazyFlags->setFlags(data, flags);
}

void arithRR(Armv8btAsm* data, arithReg32 pfn, Arm8BtFlags* lazyFlags, U32 width, bool needsResult, bool resultNeedsZeroExtends) {
    U32 flags;
    bool hardwareFlags, usesSrc, usesDst, usesResult;
    setupFlagsForArith(data, lazyFlags, flags, hardwareFlags, usesSrc, usesDst, usesResult, resultNeedsZeroExtends);

    U8 readRegDst = setupRegForArith(data, data->currentOp->reg, usesDst, xDst, width);
    U8 readRegSrc = setupRegForArith(data, data->currentOp->rm, usesSrc, xSrc, width);

    if (needsResult && width == 32 && !usesResult && !usesDst) {
        pfn(data, readRegDst, readRegDst, readRegSrc, hardwareFlags);
    } else {
        pfn(data, xResult, readRegDst, readRegSrc, hardwareFlags);
        writeResultArithReg(data, usesResult, needsResult, resultNeedsZeroExtends, width);
    }

    if (width == 8) {
        data->releaseNativeReg8(readRegSrc);
        data->releaseNativeReg8(readRegDst);
    }
    lazyFlags->setFlags(data, flags);
}

void arithRI(Armv8btAsm* data, arithReg32 pfnReg, arithValue32 pfnValue, Arm8BtFlags* lazyFlags, U32 width, bool needsResult, bool resultNeedsZeroExtends, bool needRegZeroExtended) {
    U32 flags;
    bool hardwareFlags, usesSrc, usesDst, usesResult;
    setupFlagsForArith(data, lazyFlags, flags, hardwareFlags, usesSrc, usesDst, usesResult, resultNeedsZeroExtends);
    U8 readRegDst = setupRegForArith(data, data->currentOp->reg, usesDst || needRegZeroExtended, xDst, width);

    if (needsResult && !usesResult && width == 32 && !usesSrc && !usesDst) {
        pfnValue(data, readRegDst, readRegDst, data->currentOp->imm, hardwareFlags);
    } else {
        if (usesSrc) {
            data->loadConst(xSrc, data->currentOp->imm);
            if (pfnReg) {
                pfnReg(data, xResult, readRegDst, xSrc, hardwareFlags);
            } else {
                pfnValue(data, xResult, readRegDst, data->currentOp->imm, hardwareFlags);
            }
        } else {
            pfnValue(data, xResult, readRegDst, data->currentOp->imm, hardwareFlags);
        }
        writeResultArithReg(data, usesResult, needsResult, resultNeedsZeroExtends, width);
    }
    if (width == 8) {
        data->releaseNativeReg8(readRegDst);
    }
    lazyFlags->setFlags(data, flags);
}

void arithIR(Armv8btAsm* data, arithReg32 pfnReg, arithValue32 pfnValue, Arm8BtFlags* lazyFlags, U32 width, bool needsResult, bool resultNeedsZeroExtends) {
    U32 flags;
    bool hardwareFlags, usesSrc, usesDst, usesResult;
    setupFlagsForArith(data, lazyFlags, flags, hardwareFlags, usesSrc, usesDst, usesResult, resultNeedsZeroExtends);
    U8 readRegSrc = setupRegForArith(data, data->currentOp->reg, usesSrc, xSrc, width);

    if (needsResult && !usesResult && width == 32 && !usesSrc) {
        pfnValue(data, readRegSrc, readRegSrc, data->currentOp->imm, hardwareFlags);
    } else {
        if (usesSrc) {
            data->loadConst(xDst, data->currentOp->imm);
            if (pfnReg) {
                pfnReg(data, xResult, readRegSrc, xDst, hardwareFlags);
            } else {
                pfnValue(data, xResult, readRegSrc, data->currentOp->imm, hardwareFlags);
            }
        } else {
            pfnValue(data, xResult, readRegSrc, data->currentOp->imm, hardwareFlags);
        }
        writeResultArithReg(data, usesResult, needsResult, resultNeedsZeroExtends, width);
    }
    if (width == 8) {
        data->releaseNativeReg8(readRegSrc);
    }
    lazyFlags->setFlags(data, flags);
}

void arithEI(Armv8btAsm* data, arithReg32 pfnReg, arithValue32 pfnValue, Arm8BtFlags* lazyFlags, U32 width, bool needsResult, bool resultNeedsZeroExtends) {
    U8 addressReg = data->getAddressReg();
    U32 flags;
    bool hardwareFlags, usesSrc, usesDst, usesResult;

    setupFlagsForArith(data, lazyFlags, flags, hardwareFlags, usesSrc, usesDst, usesResult, resultNeedsZeroExtends);
    if (usesSrc) {
        data->loadConst(xSrc, data->currentOp->imm);
    }

    data->readWriteMemory(addressReg, xDst, xResult, width, [usesSrc, pfnReg, data, flags, pfnValue] {
        if (usesSrc) {
            if (pfnReg) {
                pfnReg(data, xResult, xDst, xSrc, flags);
            }
            else {
                pfnValue(data, xResult, xDst, data->currentOp->imm, flags);
            }
        }
        else {
            pfnValue(data, xResult, xDst, data->currentOp->imm, flags);
        }
        }, data->currentOp->lock != 0, needsResult);

    // resultNeedsZeroExtends: for example, AND/OR will never need to be zero extended
    if (usesResult && width < 32 && resultNeedsZeroExtends) {
        // for example, if width is 8-bit and we added 0xFF and 0xFF, we don't want the xResult to be 1FE, instead it should be 0xFE
        data->zeroExtend(xResult, xResult, width);
    }    
    data->releaseTmpReg(addressReg);
    lazyFlags->setFlags(data, flags);
}

void arithIE(Armv8btAsm* data, arithReg32 pfnReg, arithValue32 pfnValue, Arm8BtFlags* lazyFlags, U32 width, bool needsResult, bool resultNeedsZeroExtends) {
    U8 addressReg = data->getAddressReg();
    U32 flags;
    bool hardwareFlags, usesSrc, usesDst, usesResult;

    setupFlagsForArith(data, lazyFlags, flags, hardwareFlags, usesSrc, usesDst, usesResult, resultNeedsZeroExtends);
    if (usesDst) {
        data->loadConst(xDst, data->currentOp->imm);
    }
    
    data->readWriteMemory(addressReg, xSrc, xResult, width, [usesDst, pfnReg, pfnValue, data, flags] {
        if (usesDst) {
            if (pfnReg) {
                pfnReg(data, xResult, xSrc, xDst, flags);
            }
            else {
                pfnValue(data, xResult, xSrc, data->currentOp->imm, flags);
            }
        }
        else {
            pfnValue(data, xResult, xSrc, data->currentOp->imm, flags);
        }
        }, data->currentOp->imm, needsResult);

    // resultNeedsZeroExtends: for example, AND/OR will never need to be zero extended
    if (usesResult && width < 32 && resultNeedsZeroExtends) {
        // for example, if width is 8-bit and we added 0xFF and 0xFF, we don't want the xResult to be 1FE, instead it should be 0xFE
        data->zeroExtend(xResult, xResult, width);
    }
    data->releaseTmpReg(addressReg);
    lazyFlags->setFlags(data, flags);
}

void addReg32(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, bool flags) {
    data->addRegs32(dst, src1, src2, 0, flags);
}
void addValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    data->addValue32(dst, src1, value, flags);
}

// or does not use hardware flags since it only sets PF/SF/ZF
void orReg32(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, bool flags) {
    data->orRegs32(dst, src1, src2);
}
void orValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    data->orValue32(dst, src1, value);
}

void adcReg32(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, bool flags) {
    U8 tmp = data->getTmpReg();
    data->copyBitsFromSourceAtPositionToDest(tmp, xFLAGS, 0, 1, false);
    data->addRegs32(tmp, tmp, src2);
    // ZF, SF and OF hardware flags can be used
    data->addRegs32(dst, src1, tmp, 0, flags);
    data->releaseTmpReg(tmp);
}
void adcValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    U8 tmp = data->getTmpReg();
    data->copyBitsFromSourceAtPositionToDest(tmp, xFLAGS, 0, 1, false);
    data->addValue32(tmp, tmp, value);
    // ZF, SF and OF hardware flags can be used
    data->addRegs32(dst, src1, tmp, 0, flags);
    data->releaseTmpReg(tmp);
}

void sbbReg32(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, bool flags) {
    U8 tmp = data->getTmpReg();
    data->copyBitsFromSourceAtPositionToDest(tmp, xFLAGS, 0, 1, false);
    data->addRegs32(tmp, tmp, src2);
    // ZF, SF and OF hardware flags can be used
    data->subRegs32(dst, src1, tmp, 0, flags);
    data->releaseTmpReg(tmp);
}
void sbbValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    U8 tmp = data->getTmpReg();
    data->copyBitsFromSourceAtPositionToDest(tmp, xFLAGS, 0, 1, false);
    data->addValue32(tmp, tmp, value);
    // ZF, SF and OF hardware flags can be used
    data->subRegs32(dst, src1, tmp, 0, flags);
    data->releaseTmpReg(tmp);
}

// or does not use hardware flags since it only sets PF/SF/ZF
void andReg32(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, bool flags) {
    data->andRegs32(dst, src1, src2);
}
void andValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    data->andValue32(dst, src1, value);
}

void subReg32(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, bool flags) {
    data->subRegs32(dst, src1, src2, 0, flags);
}
void subValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    data->subValue32(dst, src1, value, flags);
}

void xorReg32(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, bool flags) {
    // xor does not use hardware flags since it only sets PF/SF/ZF
    data->xorRegs32(dst, src1, src2);
}

void xorValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    // xor does not use hardware flags since it only sets PF/SF/ZF
    data->xorValue32(dst, src1, value);
}

void cmpReg32(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, bool flags) {
    data->subRegs32(dst, src1, src2, 0, flags);
}
void cmpValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    data->subValue32(dst, src1, value, flags);
}

void negReg32(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, bool flags) {
    data->subRegs32(dst, src2, src1, flags);
}
void negValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    data->subRegFromValue32(dst, src1, value, flags);
}

// or does not use hardware flags since it only sets PF/SF/ZF
void testReg32(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, bool flags) {
    data->andRegs32(dst, src1, src2);
}
void testValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    data->andValue32(dst, src1, value);
}

void opAddR8E8(Armv8btAsm* data) {
    arithRE(data, addReg32, ARM8BT_FLAGS_ADD8, 8, true, true);
}

void opAddE8R8(Armv8btAsm* data) {
    arithER(data, addReg32, ARM8BT_FLAGS_ADD8, 8, true, true);
}

void opAddR8R8(Armv8btAsm* data) {
    arithRR(data, addReg32, ARM8BT_FLAGS_ADD8, 8, true, true);
}

void opAddR8I8(Armv8btAsm* data) {
    arithRI(data, addReg32, addValue32, ARM8BT_FLAGS_ADD8, 8, true, true);
}

void opAddE8I8(Armv8btAsm* data) {
    arithEI(data, addReg32, addValue32, ARM8BT_FLAGS_ADD8, 8, true, true);
}

void opAddR16E16(Armv8btAsm* data) {
    arithRE(data, addReg32, ARM8BT_FLAGS_ADD16, 16, true, true);
}
void opAddE16R16(Armv8btAsm* data) {
    arithER(data, addReg32, ARM8BT_FLAGS_ADD16, 16, true, true);
}
void opAddR16R16(Armv8btAsm* data) {
    arithRR(data, addReg32, ARM8BT_FLAGS_ADD16, 16, true, true);
}
void opAddR16I16(Armv8btAsm* data) {
    arithRI(data, addReg32, addValue32, ARM8BT_FLAGS_ADD16, 16, true, true);
}
void opAddE16I16(Armv8btAsm* data) {
    arithEI(data, addReg32, addValue32, ARM8BT_FLAGS_ADD16, 16, true, true);
}
void opAddR32E32(Armv8btAsm* data) {
    arithRE(data, addReg32, ARM8BT_FLAGS_ADD32, 32, true, true);
}
void opAddE32R32(Armv8btAsm* data) {
    arithER(data, addReg32, ARM8BT_FLAGS_ADD32, 32, true, true);
}
void opAddR32R32(Armv8btAsm* data) {
    arithRR(data, addReg32, ARM8BT_FLAGS_ADD32, 32, true, true);
}
void opAddR32I32(Armv8btAsm* data) {
    arithRI(data, addReg32, addValue32, ARM8BT_FLAGS_ADD32, 32, true, true);
}
void opAddE32I32(Armv8btAsm* data) {
    arithEI(data, addReg32, addValue32, ARM8BT_FLAGS_ADD32, 32, true, true);
}
void opOrR8E8(Armv8btAsm* data) {
    arithRE(data, orReg32, ARM8BT_FLAGS_OR8, 8, true, false);
}
void opOrE8R8(Armv8btAsm* data) {
    arithER(data, orReg32, ARM8BT_FLAGS_OR8, 8, true, false);
}
void opOrR8R8(Armv8btAsm* data) {
    arithRR(data, orReg32, ARM8BT_FLAGS_OR8, 8, true, false);
}
void opOrR8I8(Armv8btAsm* data) {
    arithRI(data, orReg32, orValue32, ARM8BT_FLAGS_OR8, 8, true, false);
}
void opOrE8I8(Armv8btAsm* data) {
    arithEI(data, orReg32, orValue32, ARM8BT_FLAGS_OR8, 8, true, false);
}
void opOrR16E16(Armv8btAsm* data) {
    arithRE(data, orReg32, ARM8BT_FLAGS_OR16, 16, true, false);
}
void opOrE16R16(Armv8btAsm* data) {
    arithER(data, orReg32, ARM8BT_FLAGS_OR16, 16, true, false);
}
void opOrR16R16(Armv8btAsm* data) {
    arithRR(data, orReg32, ARM8BT_FLAGS_OR16, 16, true, false);
}
void opOrR16I16(Armv8btAsm* data) {
    arithRI(data, orReg32, orValue32, ARM8BT_FLAGS_OR16, 16, true, false);
}
void opOrE16I16(Armv8btAsm* data) {
    arithEI(data, orReg32, orValue32, ARM8BT_FLAGS_OR16, 16, true, false);
}
void opOrR32E32(Armv8btAsm* data) {
    arithRE(data, orReg32, ARM8BT_FLAGS_OR32, 32, true, false);
}
void opOrE32R32(Armv8btAsm* data) {
    arithER(data, orReg32, ARM8BT_FLAGS_OR32, 32, true, false);
}
void opOrR32R32(Armv8btAsm* data) {
    arithRR(data, orReg32, ARM8BT_FLAGS_OR32, 32, true, false);
}
void opOrR32I32(Armv8btAsm* data) {
    arithRI(data, orReg32, orValue32, ARM8BT_FLAGS_OR32, 32, true, false);
}
void opOrE32I32(Armv8btAsm* data) {
    arithEI(data, orReg32, orValue32, ARM8BT_FLAGS_OR32, 32, true, false);
}
void opAdcR8E8(Armv8btAsm* data) {
    arithRE(data, adcReg32, ARM8BT_FLAGS_ADC8, 8, true, true);
}
void opAdcE8R8(Armv8btAsm* data) {
    arithER(data, adcReg32, ARM8BT_FLAGS_ADC8, 8, true, true);
}
void opAdcR8R8(Armv8btAsm* data) {
    arithRR(data, adcReg32, ARM8BT_FLAGS_ADC8, 8, true, true);
}
void opAdcR8I8(Armv8btAsm* data) {
    arithRI(data, adcReg32, adcValue32, ARM8BT_FLAGS_ADC8, 8, true, true);
}
void opAdcE8I8(Armv8btAsm* data) {
    arithEI(data, adcReg32, adcValue32, ARM8BT_FLAGS_ADC8, 8, true, true);
}
void opAdcR16E16(Armv8btAsm* data) {
    arithRE(data, adcReg32, ARM8BT_FLAGS_ADC16, 16, true, true);
}
void opAdcE16R16(Armv8btAsm* data) {
    arithER(data, adcReg32, ARM8BT_FLAGS_ADC16, 16, true, true);
}
void opAdcR16R16(Armv8btAsm* data) {
    arithRR(data, adcReg32, ARM8BT_FLAGS_ADC16, 16, true, true);
}
void opAdcR16I16(Armv8btAsm* data) {
    arithRI(data, adcReg32, adcValue32, ARM8BT_FLAGS_ADC16, 16, true, true);
}
void opAdcE16I16(Armv8btAsm* data) {
    arithEI(data, adcReg32, adcValue32, ARM8BT_FLAGS_ADC16, 16, true, true);
}
void opAdcR32E32(Armv8btAsm* data) {
    arithRE(data, adcReg32, ARM8BT_FLAGS_ADC32, 32, true, true);
}
void opAdcE32R32(Armv8btAsm* data) {
    arithER(data, adcReg32, ARM8BT_FLAGS_ADC32, 32, true, true);
}
void opAdcR32R32(Armv8btAsm* data) {
    arithRR(data, adcReg32, ARM8BT_FLAGS_ADC32, 32, true, true);
}
void opAdcR32I32(Armv8btAsm* data) {
    arithRI(data, adcReg32, adcValue32, ARM8BT_FLAGS_ADC32, 32, true, true);
}
void opAdcE32I32(Armv8btAsm* data) {
    arithEI(data, adcReg32, adcValue32, ARM8BT_FLAGS_ADC32, 32, true, true);
}
void opSbbR8E8(Armv8btAsm* data) {
    arithRE(data, sbbReg32, ARM8BT_FLAGS_SBB8, 8, true, true);
}
void opSbbE8R8(Armv8btAsm* data) {
    arithER(data, sbbReg32, ARM8BT_FLAGS_SBB8, 8, true, true);
}
void opSbbR8R8(Armv8btAsm* data) {
    arithRR(data, sbbReg32, ARM8BT_FLAGS_SBB8, 8, true, true);
}
void opSbbR8I8(Armv8btAsm* data) {
    arithRI(data, sbbReg32, sbbValue32, ARM8BT_FLAGS_SBB8, 8, true, true);
}
void opSbbE8I8(Armv8btAsm* data) {
    arithEI(data, sbbReg32, sbbValue32, ARM8BT_FLAGS_SBB8, 8, true, true);
}
void opSbbR16E16(Armv8btAsm* data) {
    arithRE(data, sbbReg32, ARM8BT_FLAGS_SBB16, 16, true, true);
}
void opSbbE16R16(Armv8btAsm* data) {
    arithER(data, sbbReg32, ARM8BT_FLAGS_SBB16, 16, true, true);
}
void opSbbR16R16(Armv8btAsm* data) {
    arithRR(data, sbbReg32, ARM8BT_FLAGS_SBB16, 16, true, true);
}
void opSbbR16I16(Armv8btAsm* data) {
    arithRI(data, sbbReg32, sbbValue32, ARM8BT_FLAGS_SBB16, 16, true, true);
}
void opSbbE16I16(Armv8btAsm* data) {
    arithEI(data, sbbReg32, sbbValue32, ARM8BT_FLAGS_SBB16, 16, true, true);
}
void opSbbR32E32(Armv8btAsm* data) {
    arithRE(data, sbbReg32, ARM8BT_FLAGS_SBB32, 32, true, true);
}
void opSbbE32R32(Armv8btAsm* data) {
    arithER(data, sbbReg32, ARM8BT_FLAGS_SBB32, 32, true, true);
}
void opSbbR32R32(Armv8btAsm* data) {
    arithRR(data, sbbReg32, ARM8BT_FLAGS_SBB32, 32, true, true);
}
void opSbbR32I32(Armv8btAsm* data) {
    arithRI(data, sbbReg32, sbbValue32, ARM8BT_FLAGS_SBB32, 32, true, true);
}
void opSbbE32I32(Armv8btAsm* data) {
    arithEI(data, sbbReg32, sbbValue32, ARM8BT_FLAGS_SBB32, 32, true, true);
}
void opAndR8E8(Armv8btAsm* data) {
    arithRE(data, andReg32, ARM8BT_FLAGS_AND8, 8, true, false);
}
void opAndE8R8(Armv8btAsm* data) {
    arithER(data, andReg32, ARM8BT_FLAGS_AND8, 8, true, false);
}
void opAndR8R8(Armv8btAsm* data) {
    arithRR(data, andReg32, ARM8BT_FLAGS_AND8, 8, true, false);
}
void opAndR8I8(Armv8btAsm* data) {
    arithRI(data, andReg32, andValue32, ARM8BT_FLAGS_AND8, 8, true, false);
}
void opAndE8I8(Armv8btAsm* data) {
    arithEI(data, andReg32, andValue32, ARM8BT_FLAGS_AND8, 8, true, false);
}
void opAndR16E16(Armv8btAsm* data) {
    arithRE(data, andReg32, ARM8BT_FLAGS_AND16, 16, true, false);
}
void opAndE16R16(Armv8btAsm* data) {
    arithER(data, andReg32, ARM8BT_FLAGS_AND16, 16, true, false);
}
void opAndR16R16(Armv8btAsm* data) {
    arithRR(data, andReg32, ARM8BT_FLAGS_AND16, 16, true, false);
}
void opAndR16I16(Armv8btAsm* data) {
    arithRI(data, andReg32, andValue32, ARM8BT_FLAGS_AND16, 16, true, false);
}
void opAndE16I16(Armv8btAsm* data) {
    arithEI(data, andReg32, andValue32, ARM8BT_FLAGS_AND16, 16, true, false);
}
void opAndR32E32(Armv8btAsm* data) {
    arithRE(data, andReg32, ARM8BT_FLAGS_AND32, 32, true, false);
}
void opAndE32R32(Armv8btAsm* data) {
    arithER(data, andReg32, ARM8BT_FLAGS_AND32, 32, true, false);
}
void opAndR32R32(Armv8btAsm* data) {
    arithRR(data, andReg32, ARM8BT_FLAGS_AND32, 32, true, false);
}
void opAndR32I32(Armv8btAsm* data) {
    arithRI(data, andReg32, andValue32, ARM8BT_FLAGS_AND32, 32, true, false);
}
void opAndE32I32(Armv8btAsm* data) {
    arithEI(data, andReg32, andValue32, ARM8BT_FLAGS_AND32, 32, true, false);
}
void opSubR8E8(Armv8btAsm* data) {
    arithRE(data, subReg32, ARM8BT_FLAGS_SUB8, 8, true, true);
}
void opSubE8R8(Armv8btAsm* data) {
    arithER(data, subReg32, ARM8BT_FLAGS_SUB8, 8, true, true);
}
void opSubR8R8(Armv8btAsm* data) {
    arithRR(data, subReg32, ARM8BT_FLAGS_SUB8, 8, true, true);
}
void opSubR8I8(Armv8btAsm* data) {
    arithRI(data, subReg32, subValue32, ARM8BT_FLAGS_SUB8, 8, true, true);
}
void opSubE8I8(Armv8btAsm* data) {
    arithEI(data, subReg32, subValue32, ARM8BT_FLAGS_SUB8, 8, true, true);
}
void opSubR16E16(Armv8btAsm* data) {
    arithRE(data, subReg32, ARM8BT_FLAGS_SUB16, 16, true, true);
}
void opSubE16R16(Armv8btAsm* data) {
    arithER(data, subReg32, ARM8BT_FLAGS_SUB16, 16, true, true);
}
void opSubR16R16(Armv8btAsm* data) {
    arithRR(data, subReg32, ARM8BT_FLAGS_SUB16, 16, true, true);
}
void opSubR16I16(Armv8btAsm* data) {
    arithRI(data, subReg32, subValue32, ARM8BT_FLAGS_SUB16, 16, true, true);
}
void opSubE16I16(Armv8btAsm* data) {
    arithEI(data, subReg32, subValue32, ARM8BT_FLAGS_SUB16, 16, true, true);
}
void opSubR32E32(Armv8btAsm* data) {
    arithRE(data, subReg32, ARM8BT_FLAGS_SUB32, 32, true, true);
}
void opSubE32R32(Armv8btAsm* data) {
    arithER(data, subReg32, ARM8BT_FLAGS_SUB32, 32, true, true);
}
void opSubR32R32(Armv8btAsm* data) {
    arithRR(data, subReg32, ARM8BT_FLAGS_SUB32, 32, true, true);
}
void opSubR32I32(Armv8btAsm* data) {
    arithRI(data, subReg32, subValue32, ARM8BT_FLAGS_SUB32, 32, true, true);
}
void opSubE32I32(Armv8btAsm* data) {
    arithEI(data, subReg32, subValue32, ARM8BT_FLAGS_SUB32, 32, true, true);
}
void opXorR8E8(Armv8btAsm* data) {
    arithRE(data, xorReg32, ARM8BT_FLAGS_XOR8, 8, true, false);
}
void opXorE8R8(Armv8btAsm* data) {
    arithER(data, xorReg32, ARM8BT_FLAGS_XOR8, 8, true, false);
}
void opXorR8R8(Armv8btAsm* data) {
    arithRR(data, xorReg32, ARM8BT_FLAGS_XOR8, 8, true, false);
}
void opXorR8I8(Armv8btAsm* data) {
    arithRI(data, xorReg32, xorValue32, ARM8BT_FLAGS_XOR8, 8, true, false);
}
void opXorE8I8(Armv8btAsm* data) {
    arithEI(data, xorReg32, xorValue32, ARM8BT_FLAGS_XOR8, 8, true, false);
}
void opXorR16E16(Armv8btAsm* data) {
    arithRE(data, xorReg32, ARM8BT_FLAGS_XOR16, 16, true, false);
}
void opXorE16R16(Armv8btAsm* data) {
    arithER(data, xorReg32, ARM8BT_FLAGS_XOR16, 16, true, false);
}
void opXorR16R16(Armv8btAsm* data) {
    arithRR(data, xorReg32, ARM8BT_FLAGS_XOR16, 16, true, false);
}
void opXorR16I16(Armv8btAsm* data) {
    arithRI(data, xorReg32, xorValue32, ARM8BT_FLAGS_XOR16, 16, true, false);
}
void opXorE16I16(Armv8btAsm* data) {
    arithEI(data, xorReg32, xorValue32, ARM8BT_FLAGS_XOR16, 16, true, false);
}
void opXorR32E32(Armv8btAsm* data) {
    arithRE(data, xorReg32, ARM8BT_FLAGS_XOR32, 32, true, false);
}
void opXorE32R32(Armv8btAsm* data) {
    arithER(data, xorReg32, ARM8BT_FLAGS_XOR32, 32, true, false);
}
void opXorR32R32(Armv8btAsm* data) {
    arithRR(data, xorReg32, ARM8BT_FLAGS_XOR32, 32, true, false);
}
void opXorR32I32(Armv8btAsm* data) {
    arithRI(data, xorReg32, xorValue32, ARM8BT_FLAGS_XOR32, 32, true, false);
}
void opXorE32I32(Armv8btAsm* data) {
    arithEI(data, xorReg32, xorValue32, ARM8BT_FLAGS_XOR32, 32, true, false);
}
void opCmpR8E8(Armv8btAsm* data) {
    arithRE(data, cmpReg32, ARM8BT_FLAGS_SUB8, 8, false, true);
}
void opCmpE8R8(Armv8btAsm* data) {
    arithER(data, cmpReg32, ARM8BT_FLAGS_SUB8, 8, false, true);
}
void opCmpR8R8(Armv8btAsm* data) {
    arithRR(data, cmpReg32, ARM8BT_FLAGS_SUB8, 8, false, true);
}
void opCmpR8I8(Armv8btAsm* data) {
    arithRI(data, cmpReg32, cmpValue32, ARM8BT_FLAGS_SUB8, 8, false, true);
}
void opCmpE8I8(Armv8btAsm* data) {
    arithEI(data, cmpReg32, cmpValue32, ARM8BT_FLAGS_SUB8, 8, false, true);
}
void opCmpR16E16(Armv8btAsm* data) {
    arithRE(data, cmpReg32, ARM8BT_FLAGS_SUB16, 16, false, true);
}
void opCmpE16R16(Armv8btAsm* data) {
    arithER(data, cmpReg32, ARM8BT_FLAGS_SUB16, 16, false, true);
}
void opCmpR16R16(Armv8btAsm* data) {
    arithRR(data, cmpReg32, ARM8BT_FLAGS_SUB16, 16, false, true);
}
void opCmpR16I16(Armv8btAsm* data) {
    arithRI(data, cmpReg32, cmpValue32, ARM8BT_FLAGS_SUB16, 16, false, true);
}
void opCmpE16I16(Armv8btAsm* data) {
    arithEI(data, cmpReg32, cmpValue32, ARM8BT_FLAGS_SUB16, 16, false, true);
}
void opCmpR32E32(Armv8btAsm* data) {
    arithRE(data, cmpReg32, ARM8BT_FLAGS_SUB32, 32, false, true);
}
void opCmpE32R32(Armv8btAsm* data) {
    arithER(data, cmpReg32, ARM8BT_FLAGS_SUB32, 32, false, true);
}
void opCmpR32R32(Armv8btAsm* data) {
    arithRR(data, cmpReg32, ARM8BT_FLAGS_SUB32, 32, false, true);
}
void opCmpR32I32(Armv8btAsm* data) {
    arithRI(data, cmpReg32, cmpValue32, ARM8BT_FLAGS_SUB32, 32, false, true);
}
void opCmpE32I32(Armv8btAsm* data) {
    arithEI(data, cmpReg32, cmpValue32, ARM8BT_FLAGS_SUB32, 32, false, true);
}
void opTestE8R8(Armv8btAsm* data) {
    arithER(data, testReg32, ARM8BT_FLAGS_AND8, 8, false, false);
}
void opTestR8R8(Armv8btAsm* data) {
    arithRR(data, testReg32, ARM8BT_FLAGS_AND8, 8, false, false);
}
void opTestR8I8(Armv8btAsm* data) {
    arithRI(data, testReg32, testValue32, ARM8BT_FLAGS_AND8, 8, false, false);
}
void opTestE8I8(Armv8btAsm* data) {
    arithEI(data, testReg32, testValue32, ARM8BT_FLAGS_AND8, 8, false, false);
}
void opTestE16R16(Armv8btAsm* data) {
    arithER(data, testReg32, ARM8BT_FLAGS_AND16, 16, false, false);
}
void opTestR16R16(Armv8btAsm* data) {
    arithRR(data, testReg32, ARM8BT_FLAGS_AND16, 16, false, false);
}
void opTestR16I16(Armv8btAsm* data) {
    arithRI(data, testReg32, testValue32, ARM8BT_FLAGS_AND16, 16, false, false);
}
void opTestE16I16(Armv8btAsm* data) {
    arithEI(data, testReg32, testValue32, ARM8BT_FLAGS_AND16, 16, false, false);
}
void opTestE32R32(Armv8btAsm* data) {
    arithER(data, testReg32, ARM8BT_FLAGS_AND32, 32, false, false);
}
void opTestR32R32(Armv8btAsm* data) {
    arithRR(data, testReg32, ARM8BT_FLAGS_AND32, 32, false, false);
}
void opTestR32I32(Armv8btAsm* data) {
    arithRI(data, testReg32, testValue32, ARM8BT_FLAGS_AND32, 32, false, false);
}
void opTestE32I32(Armv8btAsm* data) {
    arithEI(data, testReg32, testValue32, ARM8BT_FLAGS_AND32, 32, false, false);
}

void opNotR8(Armv8btAsm* data) {
    // doesn't affect flags
    // *cpu->reg8[op->reg] = ~*cpu->reg8[op->reg];
    U8 tmpReg = data->getTmpReg();   
    U8 srcReg = data->getReadNativeReg8(data->currentOp->reg);

    data->notReg32(tmpReg, srcReg);
    data->movRegToReg8(tmpReg, data->currentOp->reg);
    data->releaseNativeReg8(srcReg);
    data->releaseTmpReg(tmpReg);
}
void opNotE8(Armv8btAsm* data) {
    // doesn't affect flags
    // writeb(eaa, ~readb(eaa));
    U8 addressReg = data->getAddressReg();
    U8 readReg = data->getTmpReg();
    U8 resultReg = data->getTmpReg();

    data->readWriteMemory(addressReg, readReg, resultReg, 8, [data, resultReg, readReg] {
        data->notReg32(resultReg, readReg);
        }, data->currentOp->lock != 0);

    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(readReg);
    data->releaseTmpReg(resultReg);
}
void opNotR16(Armv8btAsm* data) {
    // doesn't affect flags
    // cpu->reg[op->reg].u16 = ~cpu->reg[op->reg].u16;
    U8 tmpReg = data->getTmpReg();

    data->notReg32(tmpReg, data->getNativeReg(data->currentOp->reg));
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmpReg, 16, false);
    data->releaseTmpReg(tmpReg);
}
void opNotE16(Armv8btAsm* data) {
    // doesn't affect flags
    // writew(eaa, ~readw(eaa));
    U8 addressReg = data->getAddressReg();
    U8 readReg = data->getTmpReg();
    U8 resultReg = data->getTmpReg();

    data->readWriteMemory(addressReg, readReg, resultReg, 16, [data, resultReg, readReg] {
        data->notReg32(resultReg, readReg);
        }, data->currentOp->lock != 0);

    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(readReg);
    data->releaseTmpReg(resultReg);
}
void opNotR32(Armv8btAsm* data) {
    // doesn't affect flags
    // cpu->reg[op->reg].u32 = ~cpu->reg[op->reg].u32;

    data->notReg32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg));
}
void opNotE32(Armv8btAsm* data) {
    // doesn't affect flags
    // writed(eaa, ~readd(eaa));
    U8 addressReg = data->getAddressReg();
    U8 readReg = data->getTmpReg();
    U8 resultReg = data->getTmpReg();

    data->readWriteMemory(addressReg, readReg, resultReg, 32, [data, resultReg, readReg] {
        data->notReg32(resultReg, readReg);
        }, data->currentOp->lock != 0);

    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(readReg);
    data->releaseTmpReg(resultReg);
}

void opNegR8(Armv8btAsm* data) {
    data->currentOp->imm = 0;
    arithIR(data, negReg32, negValue32, ARM8BT_FLAGS_NEG8, 8, true, true);
}
void opNegE8(Armv8btAsm* data) {
    data->currentOp->imm = 0;
    arithIE(data, negReg32, negValue32, ARM8BT_FLAGS_NEG8, 8, true, true);
}
void opNegR16(Armv8btAsm* data) {
    data->currentOp->imm = 0;
    arithIR(data, negReg32, negValue32, ARM8BT_FLAGS_NEG16, 16, true, true);
}
void opNegE16(Armv8btAsm* data) {
    data->currentOp->imm = 0;
    arithIE(data, negReg32, negValue32, ARM8BT_FLAGS_NEG16, 16, true, true);
}
void opNegR32(Armv8btAsm* data) {
    data->currentOp->imm = 0;
    arithIR(data, negReg32, negValue32, ARM8BT_FLAGS_NEG32, 32, true, true);
}
void opNegE32(Armv8btAsm* data) {
    data->currentOp->imm = 0;
    arithIE(data, negReg32, negValue32, ARM8BT_FLAGS_NEG32, 32, true, true);
}

// cpu->fillFlagsNoCFOF();
// AX = (S16)((S8)AL) * (S8)(src);
// if ((S16)AX < -128 || (S16)AX>127) {
//     cpu->flags |= CF | OF;
// } else {
//     cpu->flags &= ~(CF | OF);
// }
void imul8(Armv8btAsm* data, U8 signExtendedSrc) {
    // other flags are undefined
    U8 dst = data->getTmpReg();

    data->movReg8ToReg(0, dst, true);
    data->signedMultiply32(dst, dst, signExtendedSrc);
    data->movRegToReg(xEAX, dst, 16, false);

    U32 flags = data->flagsNeeded();
    if (flags & (CF | OF)) {
        data->shiftSignedRegRightWithValue32(dst, dst, 8);
        // if ((S16)AX < -128 || (S16)AX>127) {
        data->cls32(dst, dst); // leading signed bits, will be 32 if -1 or 0
        // cls will be 31 if the lower 31 bits match the sign bit
        data->addValue32(dst, dst, 1); // bit 5 will be set (value 32), if cls resuslt was 31
        data->notReg32(dst, dst);
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, dst, 5, 1); // CF is 0x01 (bit 0)
        if (flags & OF) {
            data->copyBitsFromSourceToDestAtPosition(xFLAGS, xFLAGS, 11, 1); // OF is 0x800 (bit 11)
        }
    }

    data->releaseTmpReg(dst);
}

// cpu->fillFlagsNoCFOF();
// AX = AL * src;
// if (AH) {
//     cpu->flags |= CF | OF;
// } else {
//     cpu->flags &= ~(CF | OF);
// }
void mul8(Armv8btAsm* data, U8 src) {
    // other flags are undefined
    U8 dst = data->getTmpReg();    

    data->movReg8ToReg(0, dst);    
    data->unsignedMultiply32(dst, dst, src);
    data->movRegToReg(xEAX, dst, 16, false);

    U32 flags = data->flagsNeeded();
    if (flags & (CF|OF)) {
        data->shiftRegRightWithValue32(dst, dst, 8);
        data->clz32(dst, dst);
        data->notReg32(dst, dst);
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, dst, 5, 1); // CF is 0x01 (bit 0)
        if (flags & OF) {
            data->copyBitsFromSourceToDestAtPosition(xFLAGS, xFLAGS, 11, 1); // OF is 0x800 (bit 11)
        }
    }
    
    data->releaseTmpReg(dst);
}

// S32 result = (S32)((S16)AX) * (S16)src;
// cpu->fillFlagsNoCFOF();
// AX = (U16)result;
// DX = (U16)(result >> 16);
// if (result > 32767 || result < -32768) {
//     cpu->flags |= CF | OF;
// } else {
//     cpu->flags &= ~(CF | OF);
// }
void imul16(Armv8btAsm* data, U8 reg1, U8 signedSrc2, U8 dst1, U8 dst2, bool usesDst2) {
    U8 src1 = data->getTmpReg();

    // S32 result = (S32)((S16)AX) * (S16)src;
    data->signExtend(src1, reg1, 16);
    data->signedMultiply32(src1, src1, signedSrc2);
    // AX = (U16)result;
    data->movRegToReg(dst1, src1, 16, false);
    // DX = (U16)(result >> 16);
    if (usesDst2) {
        data->copyBitsFromSourceAtPositionToDest(dst2, src1, 16, 16, true);
    }
    U32 flags = data->flagsNeeded();
    if (flags & (CF | OF)) {
        data->shiftSignedRegRightWithValue32(src1, src1, 16);
        // if (result > 32767 || result < -32768) {
        data->cls32(src1, src1); // leading signed bits, will be 32 if -1 or 0
        // cls will be 31 if the lower 31 bits match the sign bit
        data->addValue32(src1, src1, 1); // bit 5 will be set (value 32), if cls resuslt was 31
        data->notReg32(src1, src1);
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, src1, 5, 1); // CF is 0x01 (bit 0)
        if (flags & OF) {
            data->copyBitsFromSourceToDestAtPosition(xFLAGS, xFLAGS, 11, 1); // OF is 0x800 (bit 11)
        }
    }
    data->releaseTmpReg(src1);
}

// U32 result = (U32)AX * src;
// cpu->fillFlagsNoCFOF();
// AX = (U16)result;
// DX = (U16)(result >> 16);
// if (DX) {
//     cpu->flags |= CF | OF;
// } else {
//     cpu->flags &= ~(CF | OF);
// }
void mul16(Armv8btAsm* data, U8 src) {
    // other flags are undefined
    U8 dst = data->getTmpReg();

    data->movRegToReg(dst, xEAX, 16, true);
    data->unsignedMultiply32(dst, dst, src);
    data->movRegToReg(xEAX, dst, 16, false);
    data->copyBitsFromSourceAtPositionToDest(xEDX, dst, 16, 16, true);

    U32 flags = data->flagsNeeded();
    if (flags & (CF | OF)) {
        data->shiftRegRightWithValue32(dst, dst, 16);
        data->clz32(dst, dst);
        data->notReg32(dst, dst);
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, dst, 5, 1); // CF is 0x01 (bit 0)
        if (flags & OF) {
            data->copyBitsFromSourceToDestAtPosition(xFLAGS, xFLAGS, 11, 1); // OF is 0x800 (bit 11)
        }
    }

    data->releaseTmpReg(dst);
}

// S64 result = (S64)((S32)EAX) * ((S32)(src));
// cpu->fillFlagsNoCFOF();
// EAX = (U32)result;
// EDX = (U32)(result >> 32);
// if (result > 0x7fffffffl || result < -0x7fffffffl) {
//     cpu->flags |= CF | OF;
// } else {
//     cpu->flags &= ~(CF | OF);
// }

void imul32(Armv8btAsm* data, U8 reg1, U8 reg2, U8 dst1, U8 dst2, bool usesDst2) {
    U8 resultReg = data->getTmpReg();

    data->signedMultiply32(resultReg, reg1, reg2); // resultReg will be 64-bit
    data->movRegToReg(dst1, resultReg, 32, false);
    if (usesDst2) {
        data->copyBitsFromSourceAtPositionToDest64(dst2, resultReg, 32, 32, true);
    }
    U32 flags = data->flagsNeeded();
    if (flags & (CF | OF)) {
        data->shiftSignedRegRightWithValue64(resultReg, resultReg, 32);
        // if (result > 0x7fffffffl || result < -0x7fffffffl) {
        data->cls32(resultReg, resultReg); // leading signed bits, will be 32 if -1 or 0
        // cls will be 31 if the lower 31 bits match the sign bit
        data->addValue32(resultReg, resultReg, 1); // bit 5 will be set (value 32), if cls resuslt was 31
        data->notReg32(resultReg, resultReg);
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, resultReg, 5, 1); // CF is 0x01 (bit 0)
        if (flags & OF) {
            data->copyBitsFromSourceToDestAtPosition(xFLAGS, xFLAGS, 11, 1); // OF is 0x800 (bit 11)
        }
    }
    data->releaseTmpReg(resultReg);
}

// U64 result = (U64)EAX * src;
// cpu->fillFlagsNoCFOF();
// EAX = (U32)result;
// EDX = (U32)(result >> 32);
// if (EDX) {
//     cpu->flags |= CF | OF;
// } else {
//     cpu->flags &= ~(CF | OF);
// }
void mul32(Armv8btAsm* data, U8 src) {
    // other flags are undefined
    U8 dst = data->getTmpReg();

    data->unsignedMultiply64(dst, xEAX, src);
    data->movRegToReg(xEAX, dst, 32, false);
    data->shiftRegRightWithValue64(xEDX, dst, 32);

    U32 flags = data->flagsNeeded();
    if (flags & (CF | OF)) {
        data->clz32(dst, xEDX);
        data->notReg32(dst, dst);
        if (flags & CF) {
            data->copyBitsFromSourceAtPositionToDest(xFLAGS, dst, 5, 1); // CF is 0x01 (bit 0)
        }
        if (flags & OF) {
            data->copyBitsFromSourceAtPositionToDest(dst, dst, 5, 1);
            data->copyBitsFromSourceToDestAtPosition(xFLAGS, dst, 11, 1); // OF is 0x800 (bit 11)
        }
    }

    data->releaseTmpReg(dst);
}

void opMulR8(Armv8btAsm* data) {
    U8 src = data->getTmpReg();
    data->movReg8ToReg(data->currentOp->reg, src);
    mul8(data, src);
    data->releaseTmpReg(src);
}
void opMulE8(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 src = data->getTmpReg();
    data->readMemory(addressReg, src, 8, true);
    mul8(data, src);
    data->releaseTmpReg(src);
    data->releaseTmpReg(addressReg);
}
void opMulR16(Armv8btAsm* data) {
    U8 src = data->getTmpReg();
    data->movRegToReg(src, data->getNativeReg(data->currentOp->reg), 16, true);
    mul16(data, src);
    data->releaseTmpReg(src);
}
void opMulE16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 src = data->getTmpReg();
    data->readMemory(addressReg, src, 16, true);
    mul16(data, src);
    data->releaseTmpReg(src);
    data->releaseTmpReg(addressReg);
}
void opMulR32(Armv8btAsm* data) {
    mul32(data, data->getNativeReg(data->currentOp->reg));
}
void opMulE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 src = data->getTmpReg();
    data->readMemory(addressReg, src, 32, true);
    mul32(data, src);
    data->releaseTmpReg(src);
    data->releaseTmpReg(addressReg);
}
void opIMulR8(Armv8btAsm* data) {
    U8 src = data->getTmpReg();
    data->movReg8ToReg(data->currentOp->reg, src, true);
    imul8(data, src);
    data->releaseTmpReg(src);
}
void opIMulE8(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 src = data->getTmpReg();
    data->readMemory(addressReg, src, 8, true, false, true);
    imul8(data, src);
    data->releaseTmpReg(src);
    data->releaseTmpReg(addressReg);
}
void opIMulR16(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    data->signExtend(tmpReg, data->getNativeReg(data->currentOp->reg), 16);
    imul16(data, xEAX, tmpReg, xEAX, xEDX, true);
    data->releaseTmpReg(tmpReg);
}
void opIMulE16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 src = data->getTmpReg();
    data->readMemory(addressReg, src, 16, true, false, true);
    imul16(data, xEAX, src, xEAX, xEDX, true);
    data->releaseTmpReg(src);
    data->releaseTmpReg(addressReg);
}
void opIMulR32(Armv8btAsm* data) {
    imul32(data, xEAX, data->getNativeReg(data->currentOp->reg), xEAX, xEDX, true);
}
void opIMulE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 src = data->getTmpReg();
    data->readMemory(addressReg, src, 32, true);
    imul32(data, xEAX, src, xEAX, xEDX, true);
    data->releaseTmpReg(src);
    data->releaseTmpReg(addressReg);
}

// if (src == 0) cpu->prepareException(EXCEPTION_DIVIDE, 0);
// quo = (S16)AX / src;
// quo8 = (S8)quo;
// rem = (S16)AX % src;
// if (quo != quo8) cpu->prepareException(EXCEPTION_DIVIDE, 1);
// AL = quo8;
// AH = rem;
void idiv8(Armv8btAsm* data, U8 src) {
    // flags are undefined for this op, so we don't need to worry about clobbering them with a compare
    data->doIf(src, 0, DO_IF_EQUAL, [data]() {
        data->syncRegsFromHost();

        // void common_prepareException(CPU* cpu, int code, int error)
        data->mov64(0, xCPU); // param 1 (CPU)
        data->loadConst(1, EXCEPTION_DIVIDE); // param 2 (code)
        data->loadConst(2, 0); // param 2 (error)

        data->callHost((void*)common_prepareException);
        data->syncRegsToHost();
        data->doJmp(false);
        }, nullptr);

    U8 quo = data->getTmpReg();
    U8 rem = data->getTmpReg();
    U8 tmpReg = data->getTmpReg();

    data->signExtend(tmpReg, xEAX, 16);
    // quo = (S16)AX / src;
    data->signedDivideReg32(quo, tmpReg, src);
    // rem = (S16)AX % src;
    data->multiplySubtract32(rem, quo, src, tmpReg);

    // quo8 = (S8)quo;
    // if (quo != quo8) cpu->prepareException(EXCEPTION_DIVIDE, 1);
    data->signExtend(tmpReg, quo, 8);
    data->doIf(tmpReg, quo, DO_IF_EQUAL, nullptr, [data]() {
        data->syncRegsFromHost();

        // void common_prepareException(CPU* cpu, int code, int error)
        data->mov64(0, xCPU); // param 1 (CPU)
        data->loadConst(1, EXCEPTION_DIVIDE); // param 2 (code)
        data->loadConst(2, 1); // param 2 (error)

        data->callHost((void*)common_prepareException);
        data->syncRegsToHost();
        data->doJmp(false);
        }, nullptr, true);
    // AL = (U8)quo;
    data->movRegToReg8(quo, 0);
    // AH = rem;
    data->movRegToReg8(rem, 4);
    data->releaseTmpReg(quo);
    data->releaseTmpReg(rem);
    data->releaseTmpReg(tmpReg);
}

// if (src == 0) cpu->prepareException(EXCEPTION_DIVIDE, 0);
// quo = AX / src;
// rem = AX % src;
// if (quo > 255) cpu->prepareException(EXCEPTION_DIVIDE, 1);
// AL = (U8)quo;
// AH = rem;
void div8(Armv8btAsm* data, U8 src) {
    // flags are undefined for this op, so we don't need to worry about clobbering them with a compare
    data->doIf(src, 0, DO_IF_EQUAL, [data]() {
        data->syncRegsFromHost();

        // void common_prepareException(CPU* cpu, int code, int error)
        data->mov64(0, xCPU); // param 1 (CPU)
        data->loadConst(1, EXCEPTION_DIVIDE); // param 2 (code)
        data->loadConst(2, 0); // param 2 (error)

        data->callHost((void*)common_prepareException);
        data->syncRegsToHost();
        data->doJmp(false);
        }, nullptr);

    U8 quo = data->getTmpReg();
    U8 rem = data->getTmpReg();
    U8 tmpReg = data->getTmpReg();

    data->movRegToReg(tmpReg, xEAX, 16, true);
    // quo = AX / src;
    data->unsignedDivideReg32(quo, tmpReg, src);
    // rem = AX % src;
    data->multiplySubtract32(rem, quo, src, tmpReg);

    // if (quo > 255) cpu->prepareException(EXCEPTION_DIVIDE, 1);
    data->doIf(quo, 255, DO_IF_LESS_THAN_OR_EQUAL, nullptr, [data]() {
        data->syncRegsFromHost();

        // void common_prepareException(CPU* cpu, int code, int error)
        data->mov64(0, xCPU); // param 1 (CPU)
        data->loadConst(1, EXCEPTION_DIVIDE); // param 2 (code)
        data->loadConst(2, 1); // param 2 (error)

        data->callHost((void*)common_prepareException);
        data->syncRegsToHost();
        data->doJmp(false);
        });
    // AL = (U8)quo;
    data->movRegToReg8(quo, 0);
    // AH = rem;
    data->movRegToReg8(rem, 4);
    data->releaseTmpReg(quo);
    data->releaseTmpReg(rem);
    data->releaseTmpReg(tmpReg);
}

// S32 num = (S32)(((U32)DX << 16) | AX);
// if (src == 0) cpu->prepareException(EXCEPTION_DIVIDE, 0);
// quo = num / src;
// rem = (S16)(num % src);
// quo16s = (S16)quo;
// if (quo != (S32)quo16s) cpu->prepareException(EXCEPTION_DIVIDE, 1);
// DX = rem;
// AX = quo16s;
void idiv16(Armv8btAsm* data, U8 src) {
    // flags are undefined for this op, so we don't need to worry about clobbering them with a compare
    data->doIf(src, 0, DO_IF_EQUAL, [data]() {
        data->syncRegsFromHost();

        // void common_prepareException(CPU* cpu, int code, int error)
        data->mov64(0, xCPU); // param 1 (CPU)
        data->loadConst(1, EXCEPTION_DIVIDE); // param 2 (code)
        data->loadConst(2, 0); // param 2 (error)

        data->callHost((void*)common_prepareException);
        data->syncRegsToHost();
        data->doJmp(false);
        }, nullptr);

    U8 quo = data->getTmpReg();
    U8 rem = data->getTmpReg();
    U8 tmpReg = data->getTmpReg();

    // S32 num = (S32)(((U32)DX << 16) | AX);
    data->shiftRegLeftWithValue32(tmpReg, xEDX, 16);
    data->copyBitsFromSourceAtPositionToDest(tmpReg, xEAX, 0, 16, true);
    // quo = num / src;
    U8 signedSrc = data->getTmpReg();
    data->signExtend(signedSrc, src, 16);
    data->signedDivideReg32(quo, tmpReg, signedSrc);
    data->releaseTmpReg(signedSrc);
    // rem = (S16)(num % src);
    data->multiplySubtract32(rem, quo, src, tmpReg);

    // if (quo != (S32)quo16) cpu->prepareException(EXCEPTION_DIVIDE, 1);
    data->signExtend(tmpReg, quo, 16);
    data->doIf(quo, tmpReg, DO_IF_EQUAL, nullptr, [data]() {
        data->syncRegsFromHost();

        // void common_prepareException(CPU* cpu, int code, int error)
        data->mov64(0, xCPU); // param 1 (CPU)
        data->loadConst(1, EXCEPTION_DIVIDE); // param 2 (code)
        data->loadConst(2, 1); // param 2 (error)

        data->callHost((void*)common_prepareException);
        data->syncRegsToHost();
        data->doJmp(false);
        }, nullptr, true);
    // AX = quo16;
    data->movRegToReg(xEAX, quo, 16, false);
    // DX = rem;
    data->movRegToReg(xEDX, rem, 16, false);
    data->releaseTmpReg(quo);
    data->releaseTmpReg(rem);
    data->releaseTmpReg(tmpReg);
}
// U32 num = ((U32)DX << 16) | AX;
// if (src == 0) cpu->prepareException(EXCEPTION_DIVIDE, 0);
// quo = num / src;
// rem = (U16)(num % src);
// quo16 = (U16)quo;
// if (quo != (U32)quo16) cpu->prepareException(EXCEPTION_DIVIDE, 1);
// DX = rem;
// AX = quo16;
void div16(Armv8btAsm* data, U8 src) {
    // flags are undefined for this op, so we don't need to worry about clobbering them with a compare
    data->doIf(src, 0, DO_IF_EQUAL, [data]() {
        data->syncRegsFromHost();

        // void common_prepareException(CPU* cpu, int code, int error)
        data->mov64(0, xCPU); // param 1 (CPU)
        data->loadConst(1, EXCEPTION_DIVIDE); // param 2 (code)
        data->loadConst(2, 0); // param 2 (error)

        data->callHost((void*)common_prepareException);
        data->syncRegsToHost();
        data->doJmp(false);
        }, nullptr);

    U8 quo = data->getTmpReg();
    U8 rem = data->getTmpReg();
    U8 tmpReg = data->getTmpReg();

    // U32 num = ((U32)DX << 16) | AX;
    data->shiftRegLeftWithValue32(tmpReg, xEDX, 16);
    data->copyBitsFromSourceAtPositionToDest(tmpReg, xEAX, 0, 16, true);
    // quo = num / src;
    data->unsignedDivideReg32(quo, tmpReg, src);
    // rem = (U16)(num % src);
    data->multiplySubtract32(rem, quo, src, tmpReg);

    // if (quo != (U32)quo16) cpu->prepareException(EXCEPTION_DIVIDE, 1);
    data->zeroExtend(tmpReg, quo, 16);
    data->doIf(quo, tmpReg, DO_IF_EQUAL, nullptr, [data]() {
        data->syncRegsFromHost();

        // void common_prepareException(CPU* cpu, int code, int error)
        data->mov64(0, xCPU); // param 1 (CPU)
        data->loadConst(1, EXCEPTION_DIVIDE); // param 2 (code)
        data->loadConst(2, 1); // param 2 (error)

        data->callHost((void*)common_prepareException);
        data->syncRegsToHost();
        data->doJmp(false);
        }, nullptr, true);
    // AX = quo16;
    data->movRegToReg(xEAX, quo, 16, false);
    // DX = rem;
    data->movRegToReg(xEDX, rem, 16, false);
    data->releaseTmpReg(quo);
    data->releaseTmpReg(rem);
    data->releaseTmpReg(tmpReg);
}
// S64 num = (S64)(((U64)EDX << 32) | EAX);
// if (src == 0) cpu->prepareException(EXCEPTION_DIVIDE, 0);
// quo = num / src;
// rem = (S32)(num % src);
// quo32s = (S32)quo;
// if (quo != (S64)quo32s) cpu->prepareException(EXCEPTION_DIVIDE, 1);
// EDX = rem;
// EAX = quo32s;
void idiv32(Armv8btAsm* data, U8 src) {
    // flags are undefined for this op, so we don't need to worry about clobbering them with a compare
    data->doIf(src, 0, DO_IF_EQUAL, [data]() {
        data->syncRegsFromHost();

        // void common_prepareException(CPU* cpu, int code, int error)
        data->mov64(0, xCPU); // param 1 (CPU)
        data->loadConst(1, EXCEPTION_DIVIDE); // param 2 (code)
        data->loadConst(2, 0); // param 2 (error)

        data->callHost((void*)common_prepareException);
        data->syncRegsToHost();
        data->doJmp(false);
        }, nullptr);

    U8 quo = data->getTmpReg();
    U8 rem = data->getTmpReg();
    U8 tmpReg = data->getTmpReg();

    // S64 num = (S64)(((U64)EDX << 32) | EAX);
    data->shiftRegLeftWithValue64(tmpReg, xEDX, 32);
    data->copyBitsFromSourceAtPositionToDest64(tmpReg, xEAX, 0, 32, true);
    // quo = num / src;
    U8 signedSrc = data->getTmpReg();
    data->signExtend64(signedSrc, src, 32);
    data->signedDivideReg64(quo, tmpReg, signedSrc);
    data->releaseTmpReg(signedSrc);
    // rem = (S32)(num % src);
    data->multiplySubtract64(rem, quo, src, tmpReg);

    // if (quo != (S64)quo32s) cpu->prepareException(EXCEPTION_DIVIDE, 1);
    data->signExtend64(tmpReg, quo, 32);
    data->doIf(quo, tmpReg, DO_IF_EQUAL, nullptr, [data]() {
        data->syncRegsFromHost();

        // void common_prepareException(CPU* cpu, int code, int error)
        data->mov64(0, xCPU); // param 1 (CPU)
        data->loadConst(1, EXCEPTION_DIVIDE); // param 2 (code)
        data->loadConst(2, 1); // param 2 (error)

        data->callHost((void*)common_prepareException);
        data->syncRegsToHost();
        data->doJmp(false);
        }, nullptr, true);
    // EAX = quo32;
    data->movRegToReg(xEAX, quo, 32, false);
    // EDX = rem;
    data->movRegToReg(xEDX, rem, 32, false);
    data->releaseTmpReg(quo);
    data->releaseTmpReg(rem);
    data->releaseTmpReg(tmpReg);
}

// U64 num = ((U64)EDX << 32) | EAX;
// if (src == 0) cpu->prepareException(EXCEPTION_DIVIDE, 0);
// quo = num / src;
// rem = (U32)(num % src);
// quo32 = (U32)quo;
// if (quo != (U64)quo32) cpu->prepareException(EXCEPTION_DIVIDE, 1);
// EDX = rem;
// EAX = quo32;
void div32(Armv8btAsm* data, U8 src) {
    // flags are undefined for this op, so we don't need to worry about clobbering them with a compare
    data->doIf(src, 0, DO_IF_EQUAL, [data]() {
        data->syncRegsFromHost();

        // void common_prepareException(CPU* cpu, int code, int error)
        data->mov64(0, xCPU); // param 1 (CPU)
        data->loadConst(1, EXCEPTION_DIVIDE); // param 2 (code)
        data->loadConst(2, 0); // param 2 (error)

        data->callHost((void*)common_prepareException);
        data->syncRegsToHost();
        data->doJmp(false);
        }, nullptr);

    U8 quo = data->getTmpReg();
    U8 rem = data->getTmpReg();
    U8 tmpReg = data->getTmpReg();

    // U64 num = ((U64)EDX << 32) | EAX;
    data->shiftRegLeftWithValue64(tmpReg, xEDX, 32);
    data->copyBitsFromSourceAtPositionToDest64(tmpReg, xEAX, 0, 32, true);
    // quo = num / src;
    data->unsignedDivideReg64(quo, tmpReg, src);
    // rem = (U32)(num % src);
    data->multiplySubtract64(rem, quo, src, tmpReg);

    // if (quo != (U64)quo32) cpu->prepareException(EXCEPTION_DIVIDE, 1);
    data->zeroExtend64(tmpReg, quo, 32);
    data->doIf(quo, tmpReg, DO_IF_EQUAL, nullptr, [data]() {
        data->syncRegsFromHost();

        // void common_prepareException(CPU* cpu, int code, int error)
        data->mov64(0, xCPU); // param 1 (CPU)
        data->loadConst(1, EXCEPTION_DIVIDE); // param 2 (code)
        data->loadConst(2, 1); // param 2 (error)

        data->callHost((void*)common_prepareException);
        data->syncRegsToHost();
        data->doJmp(false);
        }, nullptr, true);
    // EAX = quo32;
    data->movRegToReg(xEAX, quo, 32, false);
    // EDX = rem;
    data->movRegToReg(xEDX, rem, 32, false);
    data->releaseTmpReg(quo);
    data->releaseTmpReg(rem);
    data->releaseTmpReg(tmpReg);
}
void opDivR8(Armv8btAsm* data) {
    U8 src = data->getTmpReg();
    data->movReg8ToReg(data->currentOp->reg, src);
    div8(data, src);
    data->releaseTmpReg(src);
}
void opDivE8(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 src = data->getTmpReg();
    data->readMemory(addressReg, src, 8, true);
    data->releaseTmpReg(addressReg);
    div8(data, src);
    data->releaseTmpReg(src);    
}
void opDivR16(Armv8btAsm* data) {
    U8 src = data->getTmpReg();
    data->movRegToReg(src, data->getNativeReg(data->currentOp->reg), 16, true);
    div16(data, src);
    data->releaseTmpReg(src);
}
void opDivE16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 src = data->getTmpReg();
    data->readMemory(addressReg, src, 16, true);
    data->releaseTmpReg(addressReg);
    div16(data, src);
    data->releaseTmpReg(src);
}
void opDivR32(Armv8btAsm* data) {
    div32(data, data->getNativeReg(data->currentOp->reg));
}
void opDivE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 src = data->getTmpReg();
    data->readMemory(addressReg, src, 32, true);
    data->releaseTmpReg(addressReg);
    div32(data, src);
    data->releaseTmpReg(src);    
}
void opIDivR8(Armv8btAsm* data) {
    U8 src = data->getTmpReg();
    data->movReg8ToReg(data->currentOp->reg, src, true);
    idiv8(data, src);
    data->releaseTmpReg(src);
}
void opIDivE8(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 src = data->getTmpReg();
    data->readMemory(addressReg, src, 8, true, false, true);
    data->releaseTmpReg(addressReg);
    idiv8(data, src);
    data->releaseTmpReg(src);
}
void opIDivR16(Armv8btAsm* data) {
    U8 src = data->getTmpReg();
    data->movRegToReg(src, data->getNativeReg(data->currentOp->reg), 16, true);
    idiv16(data, src);
    data->releaseTmpReg(src);
}
void opIDivE16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 src = data->getTmpReg();
    data->readMemory(addressReg, src, 16, true);
    data->releaseTmpReg(addressReg);
    idiv16(data, src);
    data->releaseTmpReg(src);
}
void opIDivR32(Armv8btAsm* data) {
    idiv32(data, data->getNativeReg(data->currentOp->reg));
}
void opIDivE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 src = data->getTmpReg();
    data->readMemory(addressReg, src, 32, true);
    data->releaseTmpReg(addressReg);
    idiv32(data, src);
    data->releaseTmpReg(src);
}

void opXchgR8R8(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    data->movReg8ToReg(data->currentOp->reg, tmpReg);
    U8 srcReg = data->getReadNativeReg8(data->currentOp->rm);
    data->movRegToReg8(srcReg, data->currentOp->reg);
    data->movRegToReg8(tmpReg, data->currentOp->rm);
    data->releaseTmpReg(tmpReg);
    data->releaseNativeReg8(srcReg);
}
void opXchgE8R8(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();
    U8 srcReg = data->getReadNativeReg8(data->currentOp->reg);

    data->readWriteMemory(addressReg, tmpReg, srcReg, 8, [] {}, data->currentOp->lock != 0);

    data->movRegToReg8(tmpReg, data->currentOp->reg);
    data->releaseTmpReg(tmpReg);
    data->releaseNativeReg8(srcReg);
    data->releaseTmpReg(addressReg);
}
void opXchgR16R16(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    data->movRegToReg(tmpReg, data->getNativeReg(data->currentOp->reg), 32, false);
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), data->currentOp->rm, 16, false);
    data->movRegToReg(data->currentOp->rm, tmpReg, 16, false);
    data->releaseTmpReg(tmpReg);
}
void opXchgE16R16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    data->readWriteMemory(addressReg, tmpReg, data->getNativeReg(data->currentOp->reg), 16, [] {}, data->currentOp->lock != 0);

    data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmpReg, 16, false);
    data->releaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}
void opXchgR32R32(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    data->movRegToReg(tmpReg, data->getNativeReg(data->currentOp->reg), 32, false);
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), data->currentOp->rm, 32, false);
    data->movRegToReg(data->getNativeReg(data->currentOp->rm), tmpReg, 32, false);
    data->releaseTmpReg(tmpReg);
}
void opXchgE32R32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    data->readWriteMemory(addressReg, tmpReg, data->getNativeReg(data->currentOp->reg), 32, [] {}, data->currentOp->lock != 0);

    data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmpReg, 32, false);
    data->releaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}

// S32 res = (S16)arg1 * (S32)((S16)arg2);
// cpu->fillFlagsNoCFOF();
// if ((res >= -32767) && (res <= 32767)) {
//     cpu->removeFlag(CF | OF);
// } else {
//     cpu->addFlag(CF | OF);
// }
// cpu->reg[regResult].u16 = (U16)res;
void opDimulR16R16(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    data->signExtend(tmpReg, data->getNativeReg(data->currentOp->reg), 16);
    imul16(data, data->getNativeReg(data->currentOp->rm), tmpReg, data->getNativeReg(data->currentOp->reg), 0, false);
    data->releaseTmpReg(tmpReg);
}
void opDimulR16E16(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    U8 addressReg = data->getAddressReg();
    data->readMemory(addressReg, tmpReg, 16, true, false, true);
    data->releaseTmpReg(addressReg);
    imul16(data, data->getNativeReg(data->currentOp->reg), tmpReg, data->getNativeReg(data->currentOp->reg), 0, false);
    data->releaseTmpReg(tmpReg);
}
void opDimulR32R32(Armv8btAsm* data) {
    imul32(data, data->getNativeReg(data->currentOp->rm), data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), 0, false);
}
void opDimulR32E32(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    U8 addressReg = data->getAddressReg();
    data->readMemory(addressReg, tmpReg, 32, true);
    data->releaseTmpReg(addressReg);
    imul32(data, data->getNativeReg(data->currentOp->reg), tmpReg, data->getNativeReg(data->currentOp->reg), 0, false);
    data->releaseTmpReg(tmpReg);
}

void opCmpXchgR8R8(Armv8btAsm* data) {
    // cpu->dst.u8 = AL;
    // cpu->src.u8 = *cpu->reg8[dstReg];
    // cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    // cpu->lazyFlags = FLAGS_CMP8;
    // if (AL == cpu->src.u8) {
    //     *cpu->reg8[dstReg] = *cpu->reg8[srcReg];
    // } else {
    //     AL = cpu->src.u8;
    // }
    U32 flags = data->flagsNeeded();

    data->movReg8ToReg(0, xDst);
    data->movReg8ToReg(data->currentOp->reg, xSrc);

    data->subRegs32(xResult, xDst, xSrc, 0, true); // flags used by if below
    if (ARM8BT_FLAGS_CMP8->usesResult(flags)) {
        data->zeroExtend(xResult, xResult, 8);
    }
    data->doIf(0, 0, DO_IF_EQUAL, [data]() {
        data->movReg8ToReg8(data->currentOp->reg, data->currentOp->rm);
        }, [data] {
            data->movRegToReg8(xSrc, 0);
        }, nullptr, false, false);
    ARM8BT_FLAGS_CMP8->setFlags(data, flags);
}
void opCmpXchgE8R8(Armv8btAsm* data) {
    // cpu->dst.u8 = AL;
    // cpu->src.u8 = readb(address);
    // cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    // cpu->lazyFlags = FLAGS_CMP16;
    // if (AL == cpu->src.u8) {
    //     writeb(address, *cpu->reg8[srcReg]);
    // } else {
    //     AL = cpu->src.u8;
    // }
    U32 flags = data->flagsNeeded();
    U8 addressReg = data->getAddressReg();    
    U8 memReg = data->getHostMem(addressReg, 8, true);

    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;
    data->readMemory(addressReg, xSrc, 8, false, data->currentOp->lock != 0);
    data->movReg8ToReg(0, xDst);

    data->subRegs32(xResult, xDst, xSrc, 0, true); // flags used by if below
    if (ARM8BT_FLAGS_CMP8->usesResult(flags)) {
        data->zeroExtend(xResult, xResult, 8);
    }
    data->doIf(0, 0, DO_IF_EQUAL, [restartPos, addressReg, data]() {
        U8 tmpReg = data->getReadNativeReg8(data->currentOp->reg);
        data->writeMemory(addressReg, tmpReg, 8, false, data->currentOp->lock != 0, xSrc, restartPos, false);
        data->releaseNativeReg8(tmpReg);
        }, [data] {
            data->movRegToReg8(xSrc, 0);
        }, nullptr, false, false);
    if (data->currentOp->lock != 0) {
        data->fullMemoryBarrier(); // don't allow out of order read/write after this instruction until this completes
    }
    data->releaseTmpReg(addressReg);
    ARM8BT_FLAGS_CMP8->setFlags(data, flags);
}
void opCmpXchgR16R16(Armv8btAsm* data) {
    // cpu->dst.u16 = AX;
    // cpu->src.u16 = cpu->reg[dstReg].u16;
    // cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    // cpu->lazyFlags = FLAGS_CMP16;
    // if (AX == cpu->src.u16) {
    //     cpu->reg[dstReg].u16 = cpu->reg[srcReg].u16;
    // } else {
    //     AX = cpu->src.u16;
    // }
    U32 flags = data->flagsNeeded();

    data->movRegToReg(xDst, xEAX, 16, true);
    data->movRegToReg(xSrc, data->getNativeReg(data->currentOp->reg), 16, true);

    data->subRegs32(xResult, xDst, xSrc, 0, true); // flags used by if below
    if (ARM8BT_FLAGS_CMP16->usesResult(flags)) {
        data->zeroExtend(xResult, xResult, 16);
    }
    data->doIf(0, 0, DO_IF_EQUAL, [data]() {
        data->movRegToReg(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->rm), 16, false);
        }, [data] {
            data->movRegToReg(xEAX, data->getNativeReg(data->currentOp->reg), 16, false);
        }, nullptr, false, false);
    ARM8BT_FLAGS_CMP16->setFlags(data, flags);
}
void opCmpXchgE16R16(Armv8btAsm* data) {
    // cpu->dst.u16 = AX;
    // cpu->src.u16 = readw(address);
    // cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    // cpu->lazyFlags = FLAGS_CMP16;
    // if (AX == cpu->src.u16) {
    //     writew(address, cpu->reg[srcReg].u16);
    // } else {
    //     AX = cpu->src.u16;
    // }
    U32 flags = data->flagsNeeded();
    U8 addressReg = data->getAddressReg();

    U8 memReg = data->getHostMem(addressReg, 16, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;
    data->readMemory(addressReg, xSrc, 16, false, data->currentOp->lock != 0);
    data->movRegToReg(xDst, xEAX, 16, true);

    data->subRegs32(xResult, xDst, xSrc, 0, true); // flags used by if below
    if (ARM8BT_FLAGS_CMP16->usesResult(flags)) {
        data->zeroExtend(xResult, xResult, 16);
    }
    data->doIf(0, 0, DO_IF_EQUAL, [restartPos, addressReg, data]() {
        data->writeMemory(addressReg, data->getNativeReg(data->currentOp->reg), 16, false, data->currentOp->lock != 0, xSrc, restartPos, false);
        }, [data] {
            data->movRegToReg(xEAX, xSrc, 16, false);
        }, nullptr, false, false);
    if (data->currentOp->lock != 0) {
        data->fullMemoryBarrier(); // don't allow out of order read/write after this instruction until this completes
    }

    data->releaseTmpReg(addressReg);
    ARM8BT_FLAGS_CMP16->setFlags(data, flags);
}
void opCmpXchgR32R32(Armv8btAsm* data) {
    // cpu->dst.u32 = EAX;
    // cpu->src.u32 = cpu->reg[dstReg].u32;
    // cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    // cpu->lazyFlags = FLAGS_CMP32;
    // if (EAX == cpu->src.u32) {
    //     cpu->reg[dstReg].u32 = cpu->reg[srcReg].u32;
    // } else {
    //     EAX = cpu->src.u32;
    // }
    U32 flags = data->flagsNeeded();


    if (ARM8BT_FLAGS_CMP32->usesDst(flags)) {
        data->movRegToReg(xDst, xEAX, 32, false);
    }
    if (ARM8BT_FLAGS_CMP32->usesSrc(flags)) {
        data->movRegToReg(xSrc, data->getNativeReg(data->currentOp->reg), 32, false);
    }  
    data->subRegs32(xResult, xEAX, data->getNativeReg(data->currentOp->reg), 0, true); // flags used by if below
    data->doIf(0, 0, DO_IF_EQUAL, [data]() {
            data->movRegToReg(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->rm), 32, false);
        }, [data] {
            data->movRegToReg(xEAX, data->getNativeReg(data->currentOp->reg), 32, false);
        }, nullptr, false, false);
    ARM8BT_FLAGS_CMP32->setFlags(data, flags);
}

void opCmpXchgE32R32(Armv8btAsm* data) {
    // cpu->dst.u32 = EAX;
    // cpu->src.u32 = readd(address);
    // cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    // cpu->lazyFlags = FLAGS_CMP32;
    // if (EAX == cpu->src.u32) {
    //     writed(address, cpu->reg[srcReg].u32);
    // } else {
    //     EAX = cpu->src.u32;
    // }
    U32 flags = data->flagsNeeded();
    U8 addressReg = data->getAddressReg();

    U8 memReg = data->getHostMem(addressReg, 32, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;
    data->readMemory(addressReg, xSrc, 32, false, data->currentOp->lock != 0);

    if (ARM8BT_FLAGS_CMP32->usesDst(flags)) {
        data->movRegToReg(xDst, xEAX, 32, false);
    }        
    
    data->subRegs32(xResult, xEAX, xSrc, 0, true); // flags used by if below
    data->doIf(0, 0, DO_IF_EQUAL, [restartPos, addressReg, data]() {
        data->writeMemory(addressReg, data->getNativeReg(data->currentOp->reg), 32, false, data->currentOp->lock != 0, xSrc, restartPos, false);
        }, [data] {
            data->movRegToReg(xEAX, xSrc, 32, false);
        }, nullptr, false, false);
    if (data->currentOp->lock != 0) {
        data->fullMemoryBarrier(); // don't allow out of order read/write after this instruction until this completes
    }
    data->releaseTmpReg(addressReg);
    ARM8BT_FLAGS_CMP32->setFlags(data, flags);
}

void opIncR8(Armv8btAsm* data) {
    U32 flags = data->flagsNeeded();
    if (!flags) {
        U8 tmp = data->getTmpReg();
        U8 readRegDst = data->getReadNativeReg8(data->currentOp->reg);
        data->addValue32(tmp, readRegDst, 1);
        data->movRegToReg8(tmp, data->currentOp->reg);
        data->releaseNativeReg8(readRegDst);
        data->releaseTmpReg(tmp);
    } else {
        U8 readRegDst = data->getReadNativeReg8(data->currentOp->reg);
        data->addValue32(xResult, readRegDst, 1);
        data->zeroExtend(xResult, xResult, 8);
        data->movRegToReg8(xResult, data->currentOp->reg);
        data->releaseNativeReg8(readRegDst);
        ARM8BT_FLAGS_INC8->setFlags(data, flags);
    }
}
void opIncR16(Armv8btAsm* data) {
    U32 flags = data->flagsNeeded();
    if (!flags) {
        U8 tmp = data->getTmpReg();
        data->addValue32(tmp, data->getNativeReg(data->currentOp->reg), 1);
        data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmp, 16, false);
        data->releaseTmpReg(tmp);
    } else {
        data->addValue32(xResult, data->getNativeReg(data->currentOp->reg), 1);
        data->zeroExtend(xResult, xResult, 16);
        data->movRegToReg(data->getNativeReg(data->currentOp->reg), xResult, 16, false);
        ARM8BT_FLAGS_INC16->setFlags(data, flags);
    }
}
void opIncR32(Armv8btAsm* data) {
    U32 flags = data->flagsNeeded();
    if (!flags) {
        data->addValue32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), 1);
    } else {
        data->addValue32(xResult, data->getNativeReg(data->currentOp->reg), 1);
        data->movRegToReg(data->getNativeReg(data->currentOp->reg), xResult, 32, false);
        ARM8BT_FLAGS_INC32->setFlags(data, flags);
    }
}
void opIncE8(Armv8btAsm* data) {
    U32 flags = data->flagsNeeded();
    U8 addressReg = data->getAddressReg();

    data->readWriteMemory(addressReg, xDst, xResult, 8, [data, flags] {
        data->addValue32(xResult, xDst, 1);
        if (flags) {
            data->zeroExtend(xResult, xResult, 8);
            ARM8BT_FLAGS_INC8->setFlags(data, flags);
        }
        }, data->currentOp->lock != 0);
    data->releaseTmpReg(addressReg);
}
void opIncE16(Armv8btAsm* data) {
    U32 flags = data->flagsNeeded();
    U8 addressReg = data->getAddressReg();

    data->readWriteMemory(addressReg, xDst, xResult, 16, [data, flags] {
        data->addValue32(xResult, xDst, 1);
        if (flags) {
            data->zeroExtend(xResult, xResult, 16);
            ARM8BT_FLAGS_INC16->setFlags(data, flags);
        }
        }, data->currentOp->lock != 0);
    data->releaseTmpReg(addressReg);
}
void opIncE32(Armv8btAsm* data) {
    U32 flags = data->flagsNeeded();
    U8 addressReg = data->getAddressReg();

    data->readWriteMemory(addressReg, xDst, xResult, 32, [data] {
        data->addValue32(xResult, xDst, 1);
        }, data->currentOp->lock != 0);

    data->releaseTmpReg(addressReg);
    ARM8BT_FLAGS_INC32->setFlags(data, flags);
}

void opDecR8(Armv8btAsm* data) {
    U32 flags = data->flagsNeeded();
    if (!flags) {
        U8 tmp = data->getTmpReg();
        U8 readRegDst = data->getReadNativeReg8(data->currentOp->reg);
        data->subValue32(tmp, readRegDst, 1);
        data->movRegToReg8(tmp, data->currentOp->reg);
        data->releaseNativeReg8(readRegDst);
        data->releaseTmpReg(tmp);
    } else {
        U8 readRegDst = data->getReadNativeReg8(data->currentOp->reg);
        data->subValue32(xResult, readRegDst, 1);
        data->zeroExtend(xResult, xResult, 8);
        data->movRegToReg8(xResult, data->currentOp->reg);
        data->releaseNativeReg8(readRegDst);
        ARM8BT_FLAGS_DEC8->setFlags(data, flags);
    }
}

void opDecR16(Armv8btAsm* data) {
    U32 flags = data->flagsNeeded();
    if (!flags) {
        U8 tmp = data->getTmpReg();
        data->subValue32(tmp, data->getNativeReg(data->currentOp->reg), 1);
        data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmp, 16, false);
        data->releaseTmpReg(tmp);
    } else {
        data->subValue32(xResult, data->getNativeReg(data->currentOp->reg), 1);
        data->zeroExtend(xResult, xResult, 16);
        data->movRegToReg(data->getNativeReg(data->currentOp->reg), xResult, 16, false);
        ARM8BT_FLAGS_DEC16->setFlags(data, flags);
    }
}
void opDecR32(Armv8btAsm* data) {
    U32 flags = data->flagsNeeded();
    if (!flags) {
        data->subValue32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), 1);
    } else {
        data->subValue32(xResult, data->getNativeReg(data->currentOp->reg), 1);
        data->movRegToReg(data->getNativeReg(data->currentOp->reg), xResult, 32, false);
        ARM8BT_FLAGS_DEC32->setFlags(data, flags);
    }
}

void opDecE8(Armv8btAsm* data) {
    U32 flags = data->flagsNeeded();
    U8 addressReg = data->getAddressReg();

    data->readWriteMemory(addressReg, xDst, xResult, 8, [data, flags] {
        data->subValue32(xResult, xDst, 1);
        if (flags) {
            data->zeroExtend(xResult, xResult, 8);
            ARM8BT_FLAGS_DEC8->setFlags(data, flags);
        }
        }, data->currentOp->lock != 0);
    data->releaseTmpReg(addressReg);
}
void opDecE16(Armv8btAsm* data) {
    U32 flags = data->flagsNeeded();
    U8 addressReg = data->getAddressReg();

    data->readWriteMemory(addressReg, xDst, xResult, 16, [data, flags] {
        data->subValue32(xResult, xDst, 1);
        if (flags) {
            data->zeroExtend(xResult, xResult, 16);
            ARM8BT_FLAGS_DEC16->setFlags(data, flags);
        }
        }, data->currentOp->lock != 0);
    data->releaseTmpReg(addressReg);
}
void opDecE32(Armv8btAsm* data) {
    U32 flags = data->flagsNeeded();
    U8 addressReg = data->getAddressReg();

    data->readWriteMemory(addressReg, xDst, xResult, 32, [data] {
        data->subValue32(xResult, xDst, 1);
        }, data->currentOp->lock != 0);

    data->releaseTmpReg(addressReg);
    ARM8BT_FLAGS_DEC32->setFlags(data, flags);
}

void opPushSeg16(Armv8btAsm* data) {
    U8 tmp = data->getTmpReg();
    data->readMem32ValueOffset(tmp, xCPU, (S32)CPU_OFFSET_SEG_VALUE(data->currentOp->reg));
    data->pushNativeReg16(tmp);
    data->releaseTmpReg(tmp);
}

void opPopSeg16(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}

void opPushSeg32(Armv8btAsm* data) {
    U8 tmp = data->getTmpReg();
    data->readMem32ValueOffset(tmp, xCPU, (S32)CPU_OFFSET_SEG_VALUE(data->currentOp->reg));
    data->pushNativeReg32(tmp);
    data->releaseTmpReg(tmp);
}

void opPopSeg32(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}

void opPushR16(Armv8btAsm* data) {
    data->pushNativeReg16(data->getNativeReg(data->currentOp->reg));
}

void opPushR32(Armv8btAsm* data) {
    data->pushNativeReg32(data->getNativeReg(data->currentOp->reg));
}
void opPushE16(Armv8btAsm* data) {
    U8 tmpReg = data->getAddressReg();
    data->readMemory(tmpReg, tmpReg, 16, true);
    data->pushNativeReg16(tmpReg);
    data->releaseTmpReg(tmpReg);
}

void opPushE32(Armv8btAsm* data) {
    U8 tmpReg = data->getAddressReg();
    data->readMemory(tmpReg, tmpReg, 32, true);
    data->pushNativeReg32(tmpReg);
    data->releaseTmpReg(tmpReg);
}

void opPopR16(Armv8btAsm* data) {
    data->popNativeReg16(data->getNativeReg(data->currentOp->reg), false);
}
void opPopR32(Armv8btAsm* data) {
    data->popNativeReg32(data->getNativeReg(data->currentOp->reg));
}
void opPopE16(Armv8btAsm* data) {    
    U8 tmpReg = data->getTmpReg();    
    U8 addressReg = data->getAddressReg();
    data->peekNativeReg16(tmpReg, false);  
    data->writeMemory(addressReg, tmpReg, 16, true);
    // only adjust stack after the write succeeds (winfish depends on this)
    data->popStack16();
    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(tmpReg);
}
void opPopE32(Armv8btAsm* data) {    
    U8 tmpReg = data->getTmpReg();
    U8 addressReg = data->getAddressReg();
    data->peekNativeReg32(tmpReg);
    data->writeMemory(addressReg, tmpReg, 32, true);
    // only adjust stack after the write succeeds 
    data->popStack32();
    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(tmpReg);
}

void pushA16_reg(Armv8btAsm* data, U8 inESP, U8 outESP, U8 reg) {
    data->pushStack16(inESP, outESP);

    U8 addressReg = data->getTmpReg();
    data->andRegs32(addressReg, outESP, xStackMask);
    data->addRegs32(addressReg, addressReg, xSS);
    data->writeMemory(addressReg, reg, 16, true);
    data->releaseTmpReg(addressReg);
}

void opPushA16(Armv8btAsm* data) {
    U8 tmpSP = data->getTmpReg();   

    pushA16_reg(data, xESP, tmpSP, xEAX);
    pushA16_reg(data, tmpSP, tmpSP, xECX);
    pushA16_reg(data, tmpSP, tmpSP, xEDX);
    pushA16_reg(data, tmpSP, tmpSP, xEBX);
    pushA16_reg(data, tmpSP, tmpSP, xESP);
    pushA16_reg(data, tmpSP, tmpSP, xEBP);
    pushA16_reg(data, tmpSP, tmpSP, xESI);
    pushA16_reg(data, tmpSP, tmpSP, xEDI);

    data->movRegToReg(xESP, tmpSP, 16, false);
    data->releaseTmpReg(tmpSP);
}

void pushA32_reg(Armv8btAsm* data, U8 inESP, U8 outESP, U8 reg) {
    data->pushStack32(inESP, outESP);
    if (!KThread::currentThread()->process->hasSetSeg[SS]) {
        data->writeMemory(outESP, reg, 32, true);
    } else {
        U8 addressReg = data->getTmpReg();
        data->andRegs32(addressReg, outESP, xStackMask);
        data->addRegs32(addressReg, addressReg, xSS);
        data->writeMemory(addressReg, reg, 32, true);
        data->releaseTmpReg(addressReg);
    }
}

void opPushA32(Armv8btAsm* data) {
    U8 tmpSP = data->getTmpReg();
    
    pushA32_reg(data, xESP, tmpSP, xEAX);
    pushA32_reg(data, tmpSP, tmpSP, xECX);
    pushA32_reg(data, tmpSP, tmpSP, xEDX);
    pushA32_reg(data, tmpSP, tmpSP, xEBX);
    pushA32_reg(data, tmpSP, tmpSP, xESP);
    pushA32_reg(data, tmpSP, tmpSP, xEBP);
    pushA32_reg(data, tmpSP, tmpSP, xESI);
    pushA32_reg(data, tmpSP, tmpSP, xEDI);

    data->movRegToReg(xESP, tmpSP, 32, false);
    data->releaseTmpReg(tmpSP);
}

void popA16_reg(Armv8btAsm* data, U8 inESP, U8 outESP, U8 reg) {
    // this->seg[SS].address + (THIS_ESP & this->stackMask)
    U8 tmpReg = data->getTmpReg();
    data->andRegs32(tmpReg, inESP, xStackMask);
    data->addRegs32(tmpReg, tmpReg, xSS);
    data->readMemory(tmpReg, tmpReg, 16, true);
    data->movRegToReg(reg, tmpReg, 16, false);
    data->popStack16(inESP, outESP);
    data->releaseTmpReg(tmpReg);
}

void opPopA16(Armv8btAsm* data) {
    U8 tmpSP = data->getTmpReg();

    popA16_reg(data, xESP, tmpSP, xEDI);
    popA16_reg(data, tmpSP, tmpSP, xESI);
    popA16_reg(data, tmpSP, tmpSP, xEBP);    
    data->popStack16(tmpSP, tmpSP);
    popA16_reg(data, tmpSP, tmpSP, xEBX);
    popA16_reg(data, tmpSP, tmpSP, xEDX);
    popA16_reg(data, tmpSP, tmpSP, xECX);
    popA16_reg(data, tmpSP, tmpSP, xEAX);

    data->movRegToReg(xESP, tmpSP, 16, false);
    data->releaseTmpReg(tmpSP);
}

void popA32_reg(Armv8btAsm* data, U8 inESP, U8 outESP, U8 reg) {
    if (!KThread::currentThread()->process->hasSetSeg[SS]) {
        data->readMemory(inESP, reg, 32, true);
    } else {
        U8 tmpReg = data->getTmpReg();

        // this->seg[SS].address + (THIS_ESP & this->stackMask)
        data->andRegs32(tmpReg, inESP, xStackMask);
        data->addRegs32(tmpReg, tmpReg, xSS);

        data->readMemory(tmpReg, reg, 32, true);

        data->releaseTmpReg(tmpReg);
    }
    data->popStack32(inESP, outESP);
}

void opPopA32(Armv8btAsm* data) {
    U8 tmpSP = data->getTmpReg();

    // EDI = cpu->pop32();
    popA32_reg(data, xESP, tmpSP, xEDI);
    popA32_reg(data, tmpSP, tmpSP, xESI);
    popA32_reg(data, tmpSP, tmpSP, xEBP);
    data->popStack32(tmpSP, tmpSP);
    popA32_reg(data, tmpSP, tmpSP, xEBX);
    popA32_reg(data, tmpSP, tmpSP, xEDX);
    popA32_reg(data, tmpSP, tmpSP, xECX);
    popA32_reg(data, tmpSP, tmpSP, xEAX);

    data->movRegToReg(xESP, tmpSP, 32, false);
    data->releaseTmpReg(tmpSP);
}

void opPush16(Armv8btAsm* data) {
    U8 tmpReg = data->getRegWithConst(data->currentOp->imm);
    data->pushNativeReg16(tmpReg);
    data->releaseTmpReg(tmpReg);
}
void opPush32(Armv8btAsm* data) {
    U8 tmpReg = data->getRegWithConst(data->currentOp->imm);
    data->pushNativeReg32(tmpReg);
    data->releaseTmpReg(tmpReg);
}

void opPushF16(Armv8btAsm* data) {
    data->pushNativeReg16(xFLAGS);
}
void opPushF32(Armv8btAsm* data) {
    data->pushNativeReg32(xFLAGS);
}
void opPopF16(Armv8btAsm* data) {
    data->popNativeReg16(xFLAGS, true);
    data->orValue32(xFLAGS, xFLAGS, 2);
}

void opPopF32(Armv8btAsm* data) {
    data->popNativeReg32(xFLAGS);
    data->orValue32(xFLAGS, xFLAGS, 2);
}

void opBound16(Armv8btAsm* data) {
    // if (cpu->reg[reg].u16<readw(address) || cpu->reg[reg].u16>readw(address + 2)) 
    //    cpu->prepareException(EXCEPTION_BOUND, 0);

    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();
    U8 tmpReg2 = data->getTmpReg();

    data->readMemory(addressReg, tmpReg, 16, true, false, true);
    data->signExtend(tmpReg2, data->getNativeReg(data->currentOp->reg), 16);

    data->cmpRegs32(tmpReg, tmpReg2);
    // manually implemented do_if because this is a double if that shares code
    U32 pos = data->branchSignedGreaterThan();

    data->addValue32(addressReg, addressReg, 2);
    data->readMemory(addressReg, tmpReg, 16, true, false, true);
    data->releaseTmpReg(addressReg);

    data->cmpRegs32(tmpReg2, tmpReg);
    data->releaseTmpReg(tmpReg);
    data->releaseTmpReg(tmpReg2);
    U32 pos2 = data->branchSignedLessThanOrEqual();

    data->writeJumpAmount(pos, data->bufferPos);
    data->syncRegsFromHost();

    data->mov64(0, xCPU); // param 1 (CPU)
    data->loadConst(1, EXCEPTION_BOUND); // param 2 (code)
    data->loadConst(2, 0); // param 3 (error)

    data->callHost((void*)common_prepareException);
    data->syncRegsToHost();
    data->doJmp(true);    
    data->writeJumpAmount(pos2, data->bufferPos);    
}
void opBound32(Armv8btAsm* data) {
    // if (cpu->reg[reg].u32<readd(address) || cpu->reg[reg].u32>readd(address + 4)) {
    //    cpu->prepareException(EXCEPTION_BOUND, 0);
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    data->readMemory(addressReg, tmpReg, 32, true);

    data->cmpRegs32(tmpReg, data->getNativeReg(data->currentOp->reg));
    // manually implemented do_if because this is a double if that shares code
    U32 pos = data->branchSignedGreaterThan();

    data->addValue32(addressReg, addressReg, 4);
    data->readMemory(addressReg, tmpReg, 32, true);
    data->releaseTmpReg(addressReg);

    data->cmpRegs32(data->getNativeReg(data->currentOp->reg), tmpReg);
    data->releaseTmpReg(tmpReg);
    U32 pos2 = data->branchSignedLessThanOrEqual();

    data->writeJumpAmount(pos, data->bufferPos);
    data->syncRegsFromHost();

    data->mov64(0, xCPU); // param 1 (CPU)
    data->loadConst(1, EXCEPTION_BOUND); // param 2 (code)
    data->loadConst(2, 0); // param 3 (error)

    data->callHost((void*)common_prepareException);
    data->syncRegsToHost();
    data->doJmp(true);
    data->writeJumpAmount(pos2, data->bufferPos);
}

void opArplReg(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
}
void opArplMem(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
}
void opArplReg32(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
}
void opArplMem32(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
}

void opDaa(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}

void opDas(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opAaa(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opAas(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opAam(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opAad(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}

void opImulR16E16(Armv8btAsm* data) {
    U8 srcReg = data->getAddressReg();    
    U8 constReg = data->getRegWithConst((S16)((U16)data->currentOp->imm));
    data->readMemory(srcReg, srcReg, 16, true);
    imul16(data, srcReg, constReg, data->getNativeReg(data->currentOp->reg), 0, false);
    data->releaseTmpReg(constReg);
    data->releaseTmpReg(srcReg);
}

void opImulR16R16(Armv8btAsm* data) {
    U8 constReg = data->getRegWithConst((S16)((U16)data->currentOp->imm));
    imul16(data, data->getNativeReg(data->currentOp->rm), constReg, data->getNativeReg(data->currentOp->reg), 0, false);
    data->releaseTmpReg(constReg);
}
void opImulR32E32(Armv8btAsm* data) {
    U8 srcReg = data->getAddressReg();
    U8 constReg = data->getRegWithConst(data->currentOp->imm);
    data->readMemory(srcReg, srcReg, 32, true);
    imul32(data, srcReg, constReg, data->getNativeReg(data->currentOp->reg), 0, false);
    data->releaseTmpReg(constReg);
    data->releaseTmpReg(srcReg);
}
void opImulR32R32(Armv8btAsm* data) {
    U8 constReg = data->getRegWithConst(data->currentOp->imm);
    imul32(data, data->getNativeReg(data->currentOp->rm), constReg, data->getNativeReg(data->currentOp->reg), 0, false);
    data->releaseTmpReg(constReg);
}

void opInsb(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
}
void opInsw(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
}
void opInsd(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
}
void opOutsb(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
}
void opOutsw(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
}
void opOutsd(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
}

void doCondition(Armv8btAsm* data, Conditional conditional, const std::function<void()>& f) {
    // :TODO: check hardware flags if data->lazyFlags
    U32 flagsToTest;
    bool neg = false;
    bool singleFlag = true;
    bool multiOrFlag = false;
    U32 oneFlagPos = 0;
    bool checkZF = false;

    switch (conditional) {
    case condional_O: flagsToTest = OF; oneFlagPos = 11; break;
    case condional_NO: flagsToTest = OF; oneFlagPos = 11; neg = true; break;
    case condional_B: flagsToTest = CF; break;
    case condional_NB: flagsToTest = CF; neg = true; break;
    case condional_Z: flagsToTest = ZF; oneFlagPos = 6; break;
    case condional_NZ: flagsToTest = ZF; oneFlagPos = 6; neg = true; break;
    case condional_BE: flagsToTest = ZF | CF; singleFlag = false; multiOrFlag = true; break;
    case condional_NBE: flagsToTest = ZF | CF; neg = true; singleFlag = false; multiOrFlag = true; break;
    case condional_S: flagsToTest = SF; oneFlagPos = 7; break;
    case condional_NS: flagsToTest = SF; oneFlagPos = 7; neg = true; break;
    case condional_P: flagsToTest = PF; oneFlagPos = 2; break;
    case condional_NP: flagsToTest = PF; oneFlagPos = 2; neg = true; break;
    case condional_L: flagsToTest = SF | OF; singleFlag = false; break;
    case condional_NL: flagsToTest = SF | OF; neg = true; singleFlag = false; break;
    case condional_LE: flagsToTest = SF | OF | ZF; singleFlag = false; checkZF = true;  break;
    case condional_NLE: flagsToTest = SF | OF | ZF; neg = true; singleFlag = false; checkZF = true; break;
    }
            
    if (singleFlag) {
        if (neg) {
            data->doIfBitSet(xFLAGS, oneFlagPos, nullptr, f);
        } else {
            data->doIfBitSet(xFLAGS, oneFlagPos, f);
        }
    } else if (multiOrFlag) {     
        // BE and NBE
        U8 tmpReg = data->getTmpReg();
        data->andValue32(tmpReg, xFLAGS, flagsToTest, false);
        data->doIf(tmpReg, 0, neg ? DO_IF_EQUAL : DO_IF_NOT_EQUAL, f, nullptr);
        data->releaseTmpReg(tmpReg);
    } else {
        U8 tmpSF = data->getTmpReg();
        U8 tmpOF = data->getTmpReg();
        data->copyBitsFromSourceAtPositionToDest(tmpSF, xFLAGS, 7, 1, false);
        data->copyBitsFromSourceAtPositionToDest(tmpOF, xFLAGS, 11, 1, false);
        if (checkZF) {
            // LE and NLE
            // SF != OF || ZF
            U8 tmpZF = data->getTmpReg();
            data->copyBitsFromSourceAtPositionToDest(tmpZF, xFLAGS, 6, 1, false);
            data->xorRegs32(tmpSF, tmpSF, tmpOF);
            data->orRegs32(tmpSF, tmpSF, tmpZF);
            data->releaseTmpReg(tmpZF);
            data->releaseTmpReg(tmpOF);
            data->doIf(tmpSF, 0, neg ? DO_IF_EQUAL : DO_IF_NOT_EQUAL, f, nullptr);
            data->releaseTmpReg(tmpSF);
        } else {
            // L and NL
            // SF != OF
            data->doIf(tmpSF, tmpOF, neg ? DO_IF_EQUAL : DO_IF_NOT_EQUAL, f, nullptr, nullptr, true);
            data->releaseTmpReg(tmpSF);
            data->releaseTmpReg(tmpOF);
        }        
    }
}

static void doCMov(Armv8btAsm* data, Conditional conditional, bool mem, U32 width) {
    std::function f = [mem, width, data]() {
        if (!mem) {
            data->movRegToReg(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->rm), width, false);
        } else {
            U8 addressReg = data->getAddressReg();
            if (width == 32) {
                data->readMemory(addressReg, data->getNativeReg(data->currentOp->reg), width, true);
            } else {
                U8 tmpReg = data->getTmpReg();
                data->readMemory(addressReg, tmpReg, width, true);
                data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmpReg, width, false);
                data->releaseTmpReg(tmpReg);
            }
            data->releaseTmpReg(addressReg);
        }
    };
    doCondition(data, conditional, f);   
}

static void doJump(Armv8btAsm* data, Conditional conditional) {
    std::function f = [data]() {
        // :TODO: use a local jump if possible
        U32 eip = data->ip + data->currentOp->imm;
        if (!data->cpu->isBig()) {
            eip &= 0xFFFF;
        }
        data->loadConst(xBranchEip, eip);
        data->jmpRegToxBranchEip(false);
    };
    doCondition(data, conditional, f);
}

void opJumpO(Armv8btAsm* data) {
    doJump(data, condional_O);
}
void opJumpNO(Armv8btAsm* data) {
    doJump(data, condional_NO);
}
void opJumpB(Armv8btAsm* data) {
    doJump(data, condional_B);
}
void opJumpNB(Armv8btAsm* data) {
    doJump(data, condional_NB);
}
void opJumpZ(Armv8btAsm* data) {
    doJump(data, condional_Z);
}
void opJumpNZ(Armv8btAsm* data) {
    doJump(data, condional_NZ);
}
void opJumpBE(Armv8btAsm* data) {
    doJump(data, condional_BE);
}
void opJumpNBE(Armv8btAsm* data) {
    doJump(data, condional_NBE);
}
void opJumpS(Armv8btAsm* data) {
    doJump(data, condional_S);
}
void opJumpNS(Armv8btAsm* data) {
    doJump(data, condional_NS);
}
void opJumpP(Armv8btAsm* data) {
    doJump(data, condional_P);
}
void opJumpNP(Armv8btAsm* data) {
    doJump(data, condional_NP);
}
void opJumpL(Armv8btAsm* data) {
    doJump(data, condional_L);
}
void opJumpNL(Armv8btAsm* data) {
    doJump(data, condional_NL);
}
void opJumpLE(Armv8btAsm* data) {
    doJump(data, condional_LE);
}
void opJumpNLE(Armv8btAsm* data) {
    doJump(data, condional_NLE);
}

void opMovR8R8(Armv8btAsm* data) {
    data->movReg8ToReg8(data->currentOp->reg, data->currentOp->rm);
}
void opMovE8R8(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 readReg = data->getReadNativeReg8(data->currentOp->reg);
    data->writeMemory(addressReg, readReg, 8, true);
    data->releaseNativeReg8(readReg);
    data->releaseTmpReg(addressReg);
}
void opMovR8E8(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmp = data->getTmpReg();
    data->readMemory(addressReg, tmp, 8, true);
    data->movRegToReg8(tmp, data->currentOp->reg);
    data->releaseTmpReg(tmp);
    data->releaseTmpReg(addressReg);
}
void opMovR8I8(Armv8btAsm* data) {
    U8 tmp = data->getRegWithConst(data->currentOp->imm);
    data->movRegToReg8(tmp, data->currentOp->reg);
    data->releaseTmpReg(tmp);
}
void opMovE8I8(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmp = data->getRegWithConst(data->currentOp->imm);
    data->writeMemory(addressReg, tmp, 8, true);
    data->releaseTmpReg(tmp);
    data->releaseTmpReg(addressReg);
}
void opMovR16R16(Armv8btAsm* data) {
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->rm), 16, false);
}
void opMovE16R16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->writeMemory(addressReg, data->getNativeReg(data->currentOp->reg), 16, true);
    data->releaseTmpReg(addressReg);
}
void opMovR16E16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmp = data->getTmpReg();
    data->readMemory(addressReg, tmp, 16, true);
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmp, 16, false);
    data->releaseTmpReg(tmp);
    data->releaseTmpReg(addressReg);
}
void opMovR16I16(Armv8btAsm* data) {
    U8 tmp = data->getRegWithConst(data->currentOp->imm);
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmp, 16, false);
    data->releaseTmpReg(tmp);
}
void opMovE16I16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmp = data->getRegWithConst(data->currentOp->imm);
    data->writeMemory(addressReg, tmp, 16, true);
    data->releaseTmpReg(tmp);
    data->releaseTmpReg(addressReg);
}
void opMovR32R32(Armv8btAsm* data) {
    data->mov32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->rm));
}
void opMovE32R32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->writeMemory(addressReg, data->getNativeReg(data->currentOp->reg), 32, true);
    data->releaseTmpReg(addressReg);
}
void opMovR32E32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->readMemory(addressReg, data->getNativeReg(data->currentOp->reg), 32, true);
    data->releaseTmpReg(addressReg);
}
void opMovR32I32(Armv8btAsm* data) {
    data->loadConst(data->getNativeReg(data->currentOp->reg), data->currentOp->imm);
}
void opMovE32I32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmp = data->getRegWithConst(data->currentOp->imm);
    data->writeMemory(addressReg, tmp, 32, true);
    data->releaseTmpReg(tmp);
    data->releaseTmpReg(addressReg);
}

void opMovR16S16(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    data->readMem32ValueOffset(tmpReg, xCPU, (S32)CPU_OFFSET_SEG_VALUE(data->currentOp->rm));
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmpReg, 16, false);
    data->releaseTmpReg(tmpReg);
}

void opMovR32S16(Armv8btAsm* data) {
    // https://c9x.me/x86/html/file_module_x86_id_176.html
    // When the processor executes the instruction with a 32 - bit general - purpose register, it assumes that the 16 least - significant 
    // bits of the general - purpose register are the destination or source operand.If the register is a destination operand, the resulting 
    // value in the two high - order bytes of the register is implementation dependent.For the Pentium 4, Intel Xeon, and P6 family processors, 
    // the two high - order bytes are filled with zeros; for earlier 32 - bit IA - 32 processors, the two high order bytes are undefined.
    data->readMem32ValueOffset(data->getNativeReg(data->currentOp->reg), xCPU, (S32)CPU_OFFSET_SEG_VALUE(data->currentOp->rm));
}

void opMovE16S16(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    U8 addressReg = data->getAddressReg();
    data->readMem32ValueOffset(tmpReg, xCPU, (S32)CPU_OFFSET_SEG_VALUE(data->currentOp->reg));
    data->writeMemory(addressReg, tmpReg, 16, true);
    data->releaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}

void opMovS16R16(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}

void opMovS16E16(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}


void opMovAlOb(Armv8btAsm* data) {
    if (KThread::currentThread()->process->hasSetSeg[data->currentOp->base]) {
        U8 tmp = data->getTmpReg();
        data->addValue32(tmp, data->getSegReg(data->currentOp->base), data->currentOp->disp);
        data->readMemory(tmp, tmp, 8, true);
        data->movRegToReg8(tmp, 0);
        data->releaseTmpReg(tmp);
    } else {
        U8 tmp = data->getRegWithConst(data->currentOp->disp);
        data->readMemory(tmp, tmp, 8, true);
        data->movRegToReg8(tmp, 0);
        data->releaseTmpReg(tmp);
    }
}
void opMovAxOw(Armv8btAsm* data) {
    if (KThread::currentThread()->process->hasSetSeg[data->currentOp->base]) {
        U8 tmp = data->getTmpReg();
        data->addValue32(tmp, data->getSegReg(data->currentOp->base), data->currentOp->disp);
        data->readMemory(tmp, tmp, 16, true);
        data->movRegToReg(xEAX, tmp, 16, false);
        data->releaseTmpReg(tmp);
    } else {
        U8 tmp = data->getRegWithConst(data->currentOp->disp);
        data->readMemory(tmp, tmp, 16, true);
        data->movRegToReg(xEAX, tmp, 16, false);
        data->releaseTmpReg(tmp);
    }
}
void opMovEaxOd(Armv8btAsm* data) {
    if (KThread::currentThread()->process->hasSetSeg[data->currentOp->base]) {
        U8 tmp = data->getTmpReg();
        data->addValue32(tmp, data->getSegReg(data->currentOp->base), data->currentOp->disp);
        data->readMemory(tmp, xEAX, 32, true);
        data->releaseTmpReg(tmp);
    } else {
        U8 tmp = data->getRegWithConst(data->currentOp->disp);
        data->readMemory(tmp, xEAX, 32, true);
        data->releaseTmpReg(tmp);
    }
}
void opMovObAl(Armv8btAsm* data) {
    if (KThread::currentThread()->process->hasSetSeg[data->currentOp->base]) {
        U8 tmp = data->getTmpReg();
        data->addValue32(tmp, data->getSegReg(data->currentOp->base), data->currentOp->disp);
        data->writeMemory(tmp, xEAX, 8, true);
        data->releaseTmpReg(tmp);
    } else {
        U8 tmp = data->getRegWithConst(data->currentOp->disp);
        data->writeMemory(tmp, xEAX, 8, true);
        data->releaseTmpReg(tmp);
    }
}
void opMovOwAx(Armv8btAsm* data) {
    if (KThread::currentThread()->process->hasSetSeg[data->currentOp->base]) {
        U8 tmp = data->getTmpReg();
        data->addValue32(tmp, data->getSegReg(data->currentOp->base), data->currentOp->disp);
        data->writeMemory(tmp, xEAX, 16, true);
        data->releaseTmpReg(tmp);
    } else {
        U8 tmp = data->getRegWithConst(data->currentOp->disp);
        data->writeMemory(tmp, xEAX, 16, true);
        data->releaseTmpReg(tmp);
    }
}
void opMovOdEax(Armv8btAsm* data) {
    if (KThread::currentThread()->process->hasSetSeg[data->currentOp->base]) {
        U8 tmp = data->getTmpReg();
        data->addValue32(tmp, data->getSegReg(data->currentOp->base), data->currentOp->disp);
        data->writeMemory(tmp, xEAX, 32, true);
        data->releaseTmpReg(tmp);
    } else {
        U8 tmp = data->getRegWithConst(data->currentOp->disp);
        data->writeMemory(tmp, xEAX, 32, true);
        data->releaseTmpReg(tmp);
    }
}

void opMovGwXzR8(Armv8btAsm* data) {
    U8 src = data->getReadNativeReg8(data->currentOp->rm);
    U8 tmpReg = data->getTmpReg();
    data->zeroExtend(tmpReg, src, 8);
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmpReg, 16, false);
    data->releaseTmpReg(tmpReg);
    data->releaseNativeReg8(src);
}
void opMovGwXzE8(Armv8btAsm* data) {
    U8 tmpReg = data->getAddressReg();
    data->readMemory(tmpReg, tmpReg, 8, true);
    data->zeroExtend(tmpReg, tmpReg, 8);
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmpReg, 16, false);
    data->releaseTmpReg(tmpReg);
}
void opMovGwSxR8(Armv8btAsm* data) {
    U8 src = data->getReadNativeReg8(data->currentOp->rm);
    U8 tmpReg = data->getTmpReg();
    data->signExtend(tmpReg, src, 8);
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmpReg, 16, false);
    data->releaseTmpReg(tmpReg);
    data->releaseNativeReg8(src);
}
void opMovGwSxE8(Armv8btAsm* data) {
    U8 tmpReg = data->getAddressReg();
    data->readMemory(tmpReg, tmpReg, 8, true);
    data->signExtend(tmpReg, tmpReg, 8);
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmpReg, 16, false);
    data->releaseTmpReg(tmpReg);
}

void opMovGdXzR8(Armv8btAsm* data) {
    U8 src = data->getReadNativeReg8(data->currentOp->rm);
    data->zeroExtend(data->getNativeReg(data->currentOp->reg), src, 8);
    data->releaseNativeReg8(src);
}

void opMovGdXzE8(Armv8btAsm* data) {
    U8 tmpReg = data->getAddressReg();
    data->readMemory(tmpReg, tmpReg, 8, true);
    data->zeroExtend(data->getNativeReg(data->currentOp->reg), tmpReg, 8);
    data->releaseTmpReg(tmpReg);
}
void opMovGdSxR8(Armv8btAsm* data) {
    U8 src = data->getReadNativeReg8(data->currentOp->rm);
    data->signExtend(data->getNativeReg(data->currentOp->reg), src, 8);
    data->releaseNativeReg8(src);
}
void opMovGdSxE8(Armv8btAsm* data) {
    U8 tmpReg = data->getAddressReg();
    data->readMemory(tmpReg, tmpReg, 8, true);
    data->signExtend(data->getNativeReg(data->currentOp->reg), tmpReg, 8);
    data->releaseTmpReg(tmpReg);
}

void opMovGdXzR16(Armv8btAsm* data) {
    data->zeroExtend(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->rm), 16);
}
void opMovGdXzE16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->readMemory(addressReg, data->getNativeReg(data->currentOp->reg), 16, true);
    data->releaseTmpReg(addressReg);
}
void opMovGdSxR16(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    data->signExtend(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->rm), 16);
    data->releaseTmpReg(tmpReg);
}
void opMovGdSxE16(Armv8btAsm* data) {
    U8 tmpReg = data->getAddressReg();
    data->readMemory(tmpReg, tmpReg, 16, true);
    data->signExtend(data->getNativeReg(data->currentOp->reg), tmpReg, 16);
    data->releaseTmpReg(tmpReg);
}

void opMovRdCRx(Armv8btAsm* data) {
    data->loadConst(data->getNativeReg(data->currentOp->reg), 0);
}
void opMovCRxRd(Armv8btAsm* data) {
    // no op
}

void opLeaR16(Armv8btAsm* data) {
    data->calculateAddress16(data->getNativeReg(data->currentOp->reg));
}
void opLeaR32(Armv8btAsm* data) {
    data->calculateAddress32(data->getNativeReg(data->currentOp->reg));
}

void opNop(Armv8btAsm* data) {
}

// if (((S16)AX) < 0)
//     DX = 0xFFFF;
// else
//     DX = 0;
void opCwd(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    data->signExtend(tmpReg, xEAX, 16);
    data->shiftRegRightWithValue32(tmpReg, tmpReg, 16);
    data->movRegToReg(xEDX, tmpReg, 16, false);
    data->releaseTmpReg(tmpReg);
}
// if (((S32)EAX) < 0)
//     EDX = 0xFFFFFFFF;
// else
//     EDX = 0;
void opCwq(Armv8btAsm* data) {
    data->shiftSignedRegRightWithValue32(xEDX, xEAX, 31);
}
void opCallAp(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}

void opCallFar(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}

void opJmpAp(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opJmpFar(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opWait(Armv8btAsm* data) {
    // :TODO: nop?
}
void opSahf(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    data->andValue32(tmpReg, xEAX, 0xD500); // only SF|ZF|AF|PF|CF
    data->orValue32(tmpReg, tmpReg, 0x200); // always set
    data->copyBitsFromSourceAtPositionToDest(xFLAGS, tmpReg, 8, 8);
    data->releaseTmpReg(tmpReg);
}
void opLahf(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    data->andValue32(tmpReg, xFLAGS, SF | ZF | AF | PF | CF);
    data->orValue32(tmpReg, tmpReg, 2);
    data->movRegToReg8(tmpReg, 4); // 4 is AH
    data->releaseTmpReg(tmpReg);
}
void opSalc(Armv8btAsm* data) {
    // if (cpu->getCF()) AL = 0xFF; else AL = 0;
    U8 tmpReg = data->getTmpReg();
    data->copyBitsFromSourceToDestAtPosition(tmpReg, xFLAGS, 31, 1, false);
    data->shiftSignedRegRightWithValue32(tmpReg, tmpReg, 31);
    data->movRegToReg8(tmpReg, 0);
    data->releaseTmpReg(tmpReg);
}

static void doRetn16(Armv8btAsm* data, U32 bytes) {
    //kpanic("Need to test");
    // U16 eip = cpu->pop16();
    // SP = SP + op->imm;
    // cpu->eip.u32 = eip;

    data->popNativeReg16(xBranchEip, true);
    if (bytes) {
        U8 tmpReg2 = data->getTmpReg();
        data->movRegToReg(tmpReg2, xESP, 16, true);
        data->addValue32(tmpReg2, tmpReg2, bytes);
        data->movRegToReg(xESP, tmpReg2, 16, false);
        data->releaseTmpReg(tmpReg2);
    }
    data->jmpRegToxBranchEip(true);
    data->done = true;
}
static void doRetn32(Armv8btAsm* data, U32 bytes) {
    // kpanic("Need to test");
    // U32 eip = cpu->pop32();
    // ESP = ESP + op->imm;
    // cpu->eip.u32 = eip;

    data->popNativeReg32(xBranchEip);
    if (bytes) {
        data->addValue32(xESP, xESP, bytes);
    }
    data->jmpRegToxBranchEip(false);
    data->done = true;
}
static void doRetf(Armv8btAsm* data, U32 big) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opRetn16Iw(Armv8btAsm* data) {
    doRetn16(data, data->currentOp->imm);
}
void opRetn32Iw(Armv8btAsm* data) {
    doRetn32(data, data->currentOp->imm);
}
void opRetn16(Armv8btAsm* data) {
    doRetn16(data, 0);
}
void opRetn32(Armv8btAsm* data) {
    doRetn32(data, 0);
}
void opRetf16(Armv8btAsm* data) {
    doRetf(data, 0);
}
void opRetf32(Armv8btAsm* data) {
    doRetf(data, 1);
}
void opInvalid(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
    data->done = true;
}
void opInt3(Armv8btAsm* data) {
    data->signalIllegalInstruction(5); // 5=ILL_PRVOPC
    data->done = true;
}
void opInt80(Armv8btAsm* data) {
    // kpanic("Need to test");

    // don't call common_runSingleOp\emulateSingleOp so that we don't  have to end the block
    data->syncRegsFromHost();

    // void ksyscall(cpu, op->len)
    data->mov64(0, xCPU); // param 1 (CPU)
    data->loadConst(1, data->currentOp->len); // param 2 (eipCount)

    data->callHost((void*)ksyscall);
    data->syncRegsToHost();

    // this catches if the thread should be terminated
    U8 tmpReg = data->getTmpReg();
    data->readMem32ValueOffset(tmpReg, xCPU, CPU_OFFSET_EXIT_TO_START_LOOP);
    data->doIf(tmpReg, 1, DO_IF_EQUAL, [data]() {
        U8 tmpReg2 = data->getTmpReg();
        data->readMem64ValueOffset(tmpReg2, xCPU, CPU_OFFSET_RETURN_ADDRESS);
        data->branchNativeRegister(tmpReg2);
        data->releaseTmpReg(tmpReg2);
        }, nullptr);
    data->releaseTmpReg(tmpReg);
    data->doJmp(false);
}

void opInt98(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opInt99(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opIntIb(Armv8btAsm* data) {
#ifdef __TEST
    if (data->currentOp->imm == 0x97) {
        data->addReturnFromTest();
        data->done = true;
    } else
#endif
    {
        // Wine will emulate things like Int 21 (DOS)
        data->signalIllegalInstruction(5);
    }
}
void opInt9A(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opIntO(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
    data->done = true;
}
static void doIret(Armv8btAsm* data, U32 big, U32 eip) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opIret(Armv8btAsm* data) {
    doIret(data, 0, data->ip);
}
void opIret32(Armv8btAsm* data) {
    doIret(data, 1, data->ip);
}
void opXlat(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    if (data->currentOp->ea16) {
        // AL = readb(cpu->seg[op->base].address + (U16)(BX + AL));
        U8 tmpReg2 = data->getTmpReg();
        data->movRegToReg(tmpReg2, xEBX, 16, true);
        data->movReg8ToReg(0, tmpReg);
        data->addRegs32(tmpReg, tmpReg, tmpReg2);
        data->releaseTmpReg(tmpReg2);
        if (data->cpu->thread->process->hasSetSeg[data->currentOp->base]) {
            data->addRegs32(tmpReg, tmpReg, data->getSegReg(data->currentOp->base));
        }
        data->readMemory(tmpReg, tmpReg, 8, true);
        data->movRegToReg8(tmpReg, 0);
    } else {
        // AL = readb(cpu->seg[op->base].address + EBX + AL);
        data->movReg8ToReg(0, tmpReg);
        data->addRegs32(tmpReg, tmpReg, xEBX);
        if (data->cpu->thread->process->hasSetSeg[data->currentOp->base]) {
            data->addRegs32(tmpReg, tmpReg, data->getSegReg(data->currentOp->base));
        }
        data->readMemory(tmpReg, tmpReg, 8, true);
        data->movRegToReg8(tmpReg, 0);
    }
    data->releaseTmpReg(tmpReg);
}
void opICEBP(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
}

void opHlt(Armv8btAsm* data) {
    // requires ring 0 access
    data->signalIllegalInstruction(5); // 5=ILL_PRVOPC
    data->done = true;
}
void opCmc(Armv8btAsm* data) {
    // Complement Carry Flag
    // cpu->setCF(!cpu->getCF());
    U8 tmpReg = data->getTmpReg();
    data->notReg32(tmpReg, xFLAGS);
    data->copyBitsFromSourceAtPositionToDest(xFLAGS, tmpReg, 0, 1, true);
    data->releaseTmpReg(tmpReg);
}
void opClc(Armv8btAsm* data) {
    // cpu->fillFlags();
    // cpu->removeCF();
    data->andValue32(xFLAGS, xFLAGS, ~CF);
}
void opStc(Armv8btAsm* data) {
    // cpu->fillFlags();
    // cpu->addCF();
    data->orValue32(xFLAGS, xFLAGS, CF);
}
void opCli(Armv8btAsm* data) {
    // cpu->removeFlag(IF);
    data->andValue32(xFLAGS, xFLAGS, ~IF);
}
void opSti(Armv8btAsm* data) {
    // cpu->addFlag(IF);
    data->orValue32(xFLAGS, xFLAGS, IF);
}
void opCld(Armv8btAsm* data) {
    // cpu->removeFlag(DF);
    data->andValue32(xFLAGS, xFLAGS, ~DF);
}
void opStd(Armv8btAsm* data) {
    // cpu->addFlag(DF);
    data->orValue32(xFLAGS, xFLAGS, DF);
}

static void getRdtsc(CPU* cpu) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    uint64_t result = (uint64_t)(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
}

void opRdtsc(Armv8btAsm* data) {
    // U64 t = cpu->instructionCount + cpu->blockInstructionCount + op->imm;
    // EAX = (U32)t;
    // EDX = (U32)(t >> 32);

    // MRS x0, CNTVCT_EL0
    U8 tmpReg = data->getTmpReg();
    data->getVirtualCounter(tmpReg);
    data->movRegToReg(xEAX, tmpReg, 32, false);
    data->shiftRegRightWithValue64(xEDX, tmpReg, 32);
    data->releaseTmpReg(tmpReg);

    /*
    data->syncRegsFromHost();
    data->mov64(0, xCPU); // param 1 (CPU)
    data->callHost((void*)getRdtsc);
    data->syncRegsToHost();
    */
}
void opCPUID(Armv8btAsm* data) {   
    // kpanic("Need to test");
    // switch (EAX) {
    // case 0:	/* Vendor ID String and maximum level? */
    //     EAX = 2;  /* Maximum level */
    //     EBX = 'G' | ('e' << 8) | ('n' << 16) | ('u' << 24);
    //     EDX = 'i' | ('n' << 8) | ('e' << 16) | ('I' << 24);
    //     ECX = 'n' | ('t' << 8) | ('e' << 16) | ('l' << 24);
    //     break;
    // case 1:	/* get processor type/family/model/stepping and feature flags */
    //     EBX = 0;			/* Not Supported */
    //     ECX = 0;			/* No features */
    //     EDX = 0x00000011;	/* FPU+TimeStamp/RDTSC */
    //     if (KSystem::pentiumLevel == 2) {
    //         EAX = 0x633;		/* intel pentium 2 */
    //     } else if (KSystem::pentiumLevel == 3) {
    //         EAX = 0x686;		/* Intel Pentium III 733 MHz */
    //         EDX |= (1 << 24);    // FXSAVE, FXRESTOR
    //         EDX |= (1 << 25);    // SSE
    //     } else if (KSystem::pentiumLevel == 4) {
    //         EAX = 0xF07;		/* Intel Pentium 4 1.4 GHz */
    //         EDX |= (1 << 24);    // FXSAVE, FXRESTOR
    //         EDX |= (1 << 25);    // SSE
    //         EDX |= (1 << 26);    // SSE2
    //     }
    //     EDX |= (1 << 5);     /* MSR */
    //     EDX |= (1 << 15);    /* support CMOV instructions */
    //     EDX |= (1 << 13);    /* PTE Global Flag */
    //     EDX |= (1 << 8);     /* CMPXCHG8B instruction */
    //     EDX |= (1 << 23);    // MMX
    //     break;
    // case 2: // TLB and cache
    //     EAX = 0x3020101;
    //     EBX = 0;
    //     ECX = 0;
    //     EDX = 0x0C040843;
    //     break;
    // case 0x80000000:
    //     EAX = 0;
    //     break;
    // default:
    //     kwarn("Unhandled CPUID Function %X", EAX);
    //     EAX = 0;
    //     EBX = 0;
    //     ECX = 0;
    //     EDX = 0;
    //     break;
    // }

    data->doIf(xEAX, 0, DO_IF_EQUAL, [data] {
        data->loadConst(xEAX, 2);
        data->loadConst(xEBX, 'G' | ('e' << 8) | ('n' << 16) | ('u' << 24));
        data->loadConst(xEDX, 'i' | ('n' << 8) | ('e' << 16) | ('I' << 24));
        data->loadConst(xECX, 'n' | ('t' << 8) | ('e' << 16) | ('l' << 24));
        }, [data] {
            data->doIf(xEAX, 1, DO_IF_EQUAL, [data] {
                data->loadConst(xEBX, 0);
                data->loadConst(xECX, 0);

                U32 edx = 0x00000011;	/* FPU+TimeStamp/RDTSC */
                U32 eax = 0x633;
                if (KSystem::pentiumLevel == 2) {
                    eax = 0x633;		/* intel pentium 2 */
                } else if (KSystem::pentiumLevel == 3) {
                    eax = 0x686;		/* Intel Pentium III 733 MHz */
                    edx |= (1 << 24);    // FXSAVE, FXRESTOR
                    edx |= (1 << 25);    // SSE
                } else if (KSystem::pentiumLevel == 4) {
                    eax = 0xF07;		/* Intel Pentium 4 1.4 GHz */
                    edx |= (1 << 24);    // FXSAVE, FXRESTOR
                    edx |= (1 << 25);    // SSE
                    edx |= (1 << 26);    // SSE2
                }
                edx |= (1 << 5);     /* MSR */
                edx |= (1 << 15);    /* support CMOV instructions */
                edx |= (1 << 13);    /* PTE Global Flag */
                edx |= (1 << 8);     /* CMPXCHG8B instruction */
                edx |= (1 << 23);    // MMX
                data->loadConst(xEDX, edx);
                data->loadConst(xEAX, eax);
                }, [data] {
                    data->doIf(xEAX, 2, DO_IF_EQUAL, [data] {
                        data->loadConst(xEAX, 0x3020101);
                        data->loadConst(xEDX, 0x0C040843);
                        data->loadConst(xEBX, 0);
                        data->loadConst(xECX, 0);
                        }, [data] {
                            data->loadConst(xEAX, 0);
                        });
                });
        });
}

static void doEnter(Armv8btAsm* data, bool big, U32 bytes, U32 level) {    
    if (level != 0) {
        data->emulateSingleOp(data->currentOp);
        data->done = true;
    } else {
        if (big) {
            data->pushNativeReg32(xEBP);
            data->movRegToReg(xEBP, xESP, 32, false);
            if (bytes) {
                data->pushStack32(xESP, xESP, bytes);
            }
        } else {
            data->pushNativeReg16(xEBP);
            data->movRegToReg(xEBP, xESP, 16, false);
            if (bytes) {
                data->pushStack16(xESP, xESP, bytes);
            }
        }
    }
}

void opEnter16(Armv8btAsm* data) {
    //kpanic("Need to test");
    doEnter(data, false, data->currentOp->imm, data->currentOp->disp);
}
void opEnter32(Armv8btAsm* data) {
    //kpanic("Need to test");
    doEnter(data, true, data->currentOp->imm, data->currentOp->disp);
}
void opLeave16(Armv8btAsm* data) {
    // kpanic("Need to test");
    //SP = BP;
    //BP = cpu->pop16();
    data->movRegToReg(xESP, xEBP, 16, false);
    data->popNativeReg16(xEBP, false);
}
void opLeave32(Armv8btAsm* data) {
    // kpanic("Need to test");
    // ESP = EBP;
    // EBP = cpu->pop32();
    data->movRegToReg(xESP, xEBP, 32, false);
    data->popNativeReg32(xEBP);
}

void doLoadSegment(Armv8btAsm* data, bool big) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}

void opLoadSegment16(Armv8btAsm* data) {
    //kpanic("Need to test");
    doLoadSegment(data, false);
}
void opLoadSegment32(Armv8btAsm* data) {
    //kpanic("Need to test");
    doLoadSegment(data, true);
}

void opLoopNZ(Armv8btAsm* data) {
    // if (op->ea16) {
    //     CX--;
    //     if (CX != 0 && !cpu->getZF()) {
    //         cpu->eip.u32 += op->imm;
    //         NEXT_BRANCH1();
    //     } else {
    //         NEXT_BRANCH2();
    //     }
    // } else {
    //     ECX--;
    //     if (ECX != 0 && !cpu->getZF()) {
    //         cpu->eip.u32 += op->imm;
    //         NEXT_BRANCH1();
    //     } else {
    //         NEXT_BRANCH2();
    //     }
    // }
    U8 tmpReg = data->getTmpReg();
    if (data->currentOp->ea16) {
        data->movRegToReg(tmpReg, xECX, 16, true);
        data->subValue32(tmpReg, tmpReg, 1);
        data->movRegToReg(xECX, tmpReg, 16, false);

        // 1) set tmpReg to 0x20 if it was 0 else it will be 0
        data->clz32(tmpReg, tmpReg);
    } else {
        data->subValue32(xECX, xECX, 1);
        // 1) set tmpReg to 0x20 if it was 0 else it will be 0
        data->clz32(tmpReg, xECX);
    }
    
    // 2) cont. ... set tmpReg to 0x20 if it was 0 else it will be 0
    data->andValue32(tmpReg, tmpReg, 0x20); 

    // or into tmpReg, ZF
    data->copyBitsFromSourceAtPositionToDest(tmpReg, xFLAGS, 6, 1); // ZF is at position 6 (0x40)

    // if E(CX) is 0 || ZF
    data->compareZeroAndBranch(tmpReg, true, data->ip + data->currentOp->imm);
    data->releaseTmpReg(tmpReg);
}
void opLoopZ(Armv8btAsm* data) {
    // if (op->ea16) {
    //     CX--;
    //     if (CX != 0 && cpu->getZF()) {
    //         cpu->eip.u32 += op->imm;
    //         NEXT_BRANCH1();
    //     } else {
    //         NEXT_BRANCH2();
    //     }
    // } else {
    //     ECX--;
    //     if (ECX != 0 && cpu->getZF()) {
    //         cpu->eip.u32 += op->imm;
    //         NEXT_BRANCH1();
    //     } else {
    //         NEXT_BRANCH2();
    //     }
    // }
    U8 tmpReg = data->getTmpReg();
    if (data->currentOp->ea16) {
        data->movRegToReg(tmpReg, xECX, 16, true);
        data->subValue32(tmpReg, tmpReg, 1);
        data->movRegToReg(xECX, tmpReg, 16, false);

        // 1) set tmpReg to 0x20 if it was 0 else it will be 0
        data->clz32(tmpReg, tmpReg);
    } else {
        data->subValue32(xECX, xECX, 1);
        // 1) set tmpReg to 0x20 if it was 0 else it will be 0
        data->clz32(tmpReg, xECX);
    }

    // 2) cont. ... set tmpReg to 0x20 if it was 0 else it will be 0
    data->andValue32(tmpReg, tmpReg, 0x20);

    // or into tmpReg, !ZF
    U8 tmpReg2 = data->getTmpReg();
    data->notReg32(tmpReg2, xFLAGS);
    data->copyBitsFromSourceAtPositionToDest(tmpReg, tmpReg2, 6, 1); // ZF is at position 6 (0x40)
    data->releaseTmpReg(tmpReg2);

    // if E(CX) is 0 || !ZF
    data->compareZeroAndBranch(tmpReg, true, data->ip + data->currentOp->imm);
    data->releaseTmpReg(tmpReg);
}
void opLoop(Armv8btAsm* data) {
    // if (op->ea16) {
    //     CX--;
    //     if (CX != 0) {
    //         cpu->eip.u32 += op->imm;
    //         NEXT_BRANCH1();
    //     } else {
    //         NEXT_BRANCH2();
    //     }
    // } else {
    //     ECX--;
    //     if (ECX != 0) {
    //         cpu->eip.u32 += op->imm;
    //         NEXT_BRANCH1();
    //     } else {
    //         NEXT_BRANCH2();
    //     }
    // }
    if (data->currentOp->ea16) {
        U8 tmpReg = data->getTmpReg();
        data->movRegToReg(tmpReg, xECX, 16, true);
        data->subValue32(tmpReg, tmpReg, 1);
        data->movRegToReg(xECX, tmpReg, 16, false);
        data->compareZeroAndBranch(tmpReg, false, data->ip + data->currentOp->imm);
        data->releaseTmpReg(tmpReg);
    } else {
        data->subValue32(xECX, xECX, 1);
        data->compareZeroAndBranch(xECX, false, data->ip + data->currentOp->imm);
    }
}
void opJcxz(Armv8btAsm* data) {
    // if ((op->ea16 ? CX : ECX) == 0) {
    //     cpu->eip.u32 += op->imm;
    //     NEXT_BRANCH1();
    // } else {
    //     NEXT_BRANCH2();
    // }
    if (data->currentOp->ea16) {
        U8 tmpReg = data->getTmpReg();
        data->movRegToReg(tmpReg, xECX, 16, true);
        data->compareZeroAndBranch(tmpReg, true, data->ip + data->currentOp->imm);
        data->releaseTmpReg(tmpReg);
    } else {
        data->compareZeroAndBranch(xECX, true, data->ip + data->currentOp->imm);
    }
}

void opInAlIb(Armv8btAsm* data) {
    //kpanic("Need to test");
    U8 tmpReg = data->getRegWithConst(0xFF);
    data->movRegToReg8(tmpReg, 0);
    data->releaseTmpReg(tmpReg);
}
void opInAxIb(Armv8btAsm* data) {
    //kpanic("Need to test");
    U8 tmpReg = data->getRegWithConst(0xFFFF);
    data->movRegToReg(xEAX, tmpReg, 16, false);
    data->releaseTmpReg(tmpReg);
}
void opInEaxIb(Armv8btAsm* data) {
   // kpanic("Need to test");
    data->orValue32(xEAX, xEAX, 0xFFFFFFFF);
}
void opOutIbAl(Armv8btAsm* data) {
    //kpanic("Need to test");
}
void opOutIbAx(Armv8btAsm* data) {
    //kpanic("Need to test");
}
void opOutIbEax(Armv8btAsm* data) {
    //kpanic("Need to test");
}
void opInAlDx(Armv8btAsm* data) {
    //kpanic("Need to test");
    U8 tmpReg = data->getRegWithConst(0xFF);
    data->movRegToReg8(tmpReg, 0);
    data->releaseTmpReg(tmpReg);
}
void opInAxDx(Armv8btAsm* data) {
    //kpanic("Need to test");
    U8 tmpReg = data->getRegWithConst(0xFFFF);
    data->movRegToReg(xEAX, tmpReg, 16, false);
    data->releaseTmpReg(tmpReg);
}
void opInEaxDx(Armv8btAsm* data) {
    //kpanic("Need to test");
    data->orValue32(xEAX, xEAX, 0xFFFFFFFF);
}
void opOutDxAl(Armv8btAsm* data) {
}
void opOutDxAx(Armv8btAsm* data) {
}
void opOutDxEax(Armv8btAsm* data) {
}

void opCallJw(Armv8btAsm* data) {
    //kpanic("Need to test");
    // cpu->push16(cpu->eip.u32 + op->len);
    // cpu->eip.u32 += (S16)op->imm;
    U8 tmpReg = data->getRegWithConst(data->ip);
    data->pushNativeReg16(tmpReg);
    data->loadConst(xBranchEip, (data->ip + (S16)data->currentOp->imm) & 0xFFFF);
    data->jmpRegToxBranchEip(false);
    data->releaseTmpReg(tmpReg);
    data->done = true;
}
void opCallJd(Armv8btAsm* data) {
    // kpanic("Need to test");
    // cpu->push32(cpu->eip.u32 + op->len);
    // cpu->eip.u32 += (S32)op->imm;
    U8 tmpReg = data->getRegWithConst(data->ip);
    data->pushNativeReg32(tmpReg);
    data->loadConst(xBranchEip, data->ip + (S32)(data->currentOp->imm));
    data->jmpRegToxBranchEip(false);
    data->releaseTmpReg(tmpReg);
    data->done = true;
}
void opJmpJw(Armv8btAsm* data) {
    //kpanic("Need to test");
    // cpu->eip.u32 += (S16)op->imm;
    data->jumpTo(data->ip + (S16)(data->currentOp->imm));
    data->done = true;
}
void opJmpJd(Armv8btAsm* data) {
    // kpanic("Need to test");
    // cpu->eip.u32 += (S32)op->imm;
    data->jumpTo(data->ip + (S32)(data->currentOp->imm));
    data->done = true;
}
void opJmpJb(Armv8btAsm* data) {
    // kpanic("Need to test");
    // cpu->eip.u32 += (S8)op->imm;
    data->jumpTo(data->ip + (S8)(data->currentOp->imm));
    data->done = true;
}
void opCallR16(Armv8btAsm* data) {
    // kpanic("Need to test");
    // U16 dest = cpu->reg[op->reg].u16;
    // cpu->push16(cpu->eip.u32 + op->len);
    // cpu->eip.u32 = dest;
    U8 tmpReg2 = data->getRegWithConst(data->ip);    
    data->movRegToReg(xBranchEip, data->getNativeReg(data->currentOp->reg), 16, true);
    data->pushNativeReg16(tmpReg2);
    data->releaseTmpReg(tmpReg2);
    data->jmpRegToxBranchEip(false);
    data->done = true;
}
void opCallR32(Armv8btAsm* data) {
    // kpanic("Need to test");
    // U32 dest = cpu->reg[op->reg].u32;
    // cpu->push32(cpu->eip.u32 + op->len);
    // cpu->eip.u32 = dest;
    U8 tmpReg2 = data->getRegWithConst(data->ip);
    data->movRegToReg(xBranchEip, data->getNativeReg(data->currentOp->reg), 32, false);
    data->pushNativeReg32(tmpReg2);
    data->releaseTmpReg(tmpReg2);
    data->jmpRegToxBranchEip(false);
    data->done = true;
}
void opCallE16(Armv8btAsm* data) {
    // kpanic("Need to test");
    // U32 neweip = readw(eaa(cpu, op));
    // cpu->push16(cpu->eip.u32 + op->len);
    // cpu->eip.u32 = neweip;
    U8 addressReg = data->getAddressReg();   
    data->readMemory(addressReg, xBranchEip, 16, true);
    data->releaseTmpReg(addressReg);

    U8 tmpReg = data->getRegWithConst(data->ip);
    data->pushNativeReg16(tmpReg);
    data->releaseTmpReg(tmpReg);

    data->jmpRegToxBranchEip(false);
    data->done = true;
}
void opCallE32(Armv8btAsm* data) {
    // kpanic("Need to test");
    // U32 neweip = readd(eaa(cpu, op));
    // cpu->push32(cpu->eip.u32 + op->len);
    // cpu->eip.u32 = neweip;
    U8 addressReg = data->getAddressReg();
    data->readMemory(addressReg, xBranchEip, 32, true);
    data->releaseTmpReg(addressReg);

    U8 tmpReg = data->getRegWithConst(data->ip);
    data->pushNativeReg32(tmpReg);
    data->releaseTmpReg(tmpReg);

    data->jmpRegToxBranchEip(false);
    data->done = true;
}
void opCallFarE16(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opCallFarE32(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opJmpR16(Armv8btAsm* data) {
    // kpanic("Need to test");
    // cpu->eip.u32 = cpu->reg[op->reg].u16;
    data->movRegToReg(xBranchEip, data->getNativeReg(data->currentOp->reg), 16, true);
    data->jmpRegToxBranchEip(false);
    data->done = true;
}
void opJmpR32(Armv8btAsm* data) {
    // kpanic("Need to test");
    // cpu->eip.u32 = cpu->reg[op->reg].u32;
    data->mov32(xBranchEip, data->getNativeReg(data->currentOp->reg));
    data->jmpRegToxBranchEip(false);
    data->done = true;
}
void opJmpE16(Armv8btAsm* data) {
    //kpanic("Need to test");
    // U32 neweip = readw(eaa(cpu, op));
    // cpu->eip.u32 = neweip;
    U8 addressReg = data->getAddressReg();
    data->readMemory(addressReg, xBranchEip, 16, true);
    data->releaseTmpReg(addressReg);
    data->jmpRegToxBranchEip(false);
    data->done = true;
}
void opJmpE32(Armv8btAsm* data) {
    // kpanic("Need to test");
    // U32 neweip = readd(eaa(cpu, op));
    // cpu->eip.u32 = neweip;
    U8 addressReg = data->getAddressReg();
    data->readMemory(addressReg, xBranchEip, 32, true);
    data->releaseTmpReg(addressReg);
    data->jmpRegToxBranchEip(false);
    data->done = true;
}
void opJmpFarE16(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opJmpFarE32(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}

void opLarR16R16(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opLarR16E16(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opLslR16R16(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opLslR16E16(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opLslR32R32(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}
void opLslR32E32(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
    data->done = true;
}

void opCmovO_R16R16(Armv8btAsm* data) {
    // if (cpu->getOF()) cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    doCMov(data, condional_O, false, 16);
}
void opCmovO_R16E16(Armv8btAsm* data) {
    // if (cpu->getOF()) cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    doCMov(data, condional_O, true, 16);
}
void opCmovNO_R16R16(Armv8btAsm* data) {
    // if (!cpu->getOF()) cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    doCMov(data, condional_NO, false, 16);
}
void opCmovNO_R16E16(Armv8btAsm* data) {
    // if (!cpu->getOF()) cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    doCMov(data, condional_NO, true, 16);
}
void opCmovB_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_B, false, 16);
}
void opCmovB_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_B, true, 16);
}
void opCmovNB_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_NB, false, 16);
}
void opCmovNB_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_NB, true, 16);
}
void opCmovZ_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_Z, false, 16);
}
void opCmovZ_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_Z, true, 16);
}
void opCmovNZ_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_NZ, false, 16);
}
void opCmovNZ_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_NZ, true, 16);
}
void opCmovBE_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_BE, false, 16);
}
void opCmovBE_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_BE, true, 16);
}
void opCmovNBE_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_NBE, false, 16);
}
void opCmovNBE_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_NBE, true, 16);
}
void opCmovS_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_S, false, 16);
}
void opCmovS_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_S, true, 16);
}
void opCmovNS_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_NS, false, 16);
}
void opCmovNS_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_NS, true, 16);
}
void opCmovP_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_P, false, 16);
}
void opCmovP_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_P, true, 16);
}
void opCmovNP_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_NP, false, 16);
}
void opCmovNP_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_NP, true, 16);
}
void opCmovL_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_L, false, 16);
}
void opCmovL_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_L, true, 16);
}
void opCmovNL_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_NL, false, 16);
}
void opCmovNL_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_NL, true, 16);
}
void opCmovLE_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_LE, false, 16);
}
void opCmovLE_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_LE, true, 16);
}
void opCmovNLE_R16R16(Armv8btAsm* data) {
    doCMov(data, condional_NLE, false, 16);
}
void opCmovNLE_R16E16(Armv8btAsm* data) {
    doCMov(data, condional_NLE, true, 16);
}

void opCmovO_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_O, false, 32);
}
void opCmovO_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_O, true, 32);
}
void opCmovNO_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_NO, false, 32);
}
void opCmovNO_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_NO, true, 32);
}
void opCmovB_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_B, false, 32);
}
void opCmovB_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_B, true, 32);
}
void opCmovNB_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_NB, false, 32);
}
void opCmovNB_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_NB, true, 32);
}
void opCmovZ_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_Z, false, 32);
}
void opCmovZ_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_Z, true, 32);
}
void opCmovNZ_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_NZ, false, 32);
}
void opCmovNZ_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_NZ, true, 32);
}
void opCmovBE_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_BE, false, 32);
}
void opCmovBE_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_BE, true, 32);
}
void opCmovNBE_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_NBE, false, 32);
}
void opCmovNBE_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_NBE, true, 32);
}
void opCmovS_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_S, false, 32);
}
void opCmovS_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_S, true, 32);
}
void opCmovNS_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_NS, false, 32);
}
void opCmovNS_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_NS, true, 32);
}
void opCmovP_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_P, false, 32);
}
void opCmovP_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_P, true, 32);
}
void opCmovNP_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_NP, false, 32);
}
void opCmovNP_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_NP, true, 32);
}
void opCmovL_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_L, false, 32);
}
void opCmovL_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_L, true, 32);
}
void opCmovNL_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_NL, false, 32);
}
void opCmovNL_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_NL, true, 32);
}
void opCmovLE_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_LE, false, 32);
}
void opCmovLE_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_LE, true, 32);
}
void opCmovNLE_R32R32(Armv8btAsm* data) {
    doCMov(data, condional_NLE, false, 32);
}
void opCmovNLE_R32E32(Armv8btAsm* data) {
    doCMov(data, condional_NLE, true, 32);
}

static void doSetCondition(Armv8btAsm* data, Conditional conditional, bool mem) {
    // :TODO: check hardware flags if data->lazyFlags
    U32 flagsToTest;
    bool neg = false;
    bool singleFlag = true;
    bool multiOrFlag = false;
    U32 oneFlagPos = 0;
    bool checkZF = false;

    switch (conditional) {
    case condional_O: flagsToTest = OF; oneFlagPos = 11; break;
    case condional_NO: flagsToTest = OF; oneFlagPos = 11; neg = true; break;
    case condional_B: flagsToTest = CF; break;
    case condional_NB: flagsToTest = CF; neg = true; break;
    case condional_Z: flagsToTest = ZF; oneFlagPos = 6; break;
    case condional_NZ: flagsToTest = ZF; oneFlagPos = 6; neg = true; break;
    case condional_BE: flagsToTest = ZF | CF; singleFlag = false; multiOrFlag = true; break;
    case condional_NBE: flagsToTest = ZF | CF; neg = true; singleFlag = false; multiOrFlag = true; break;
    case condional_S: flagsToTest = SF; oneFlagPos = 7; break;
    case condional_NS: flagsToTest = SF; oneFlagPos = 7; neg = true; break;
    case condional_P: flagsToTest = PF; oneFlagPos = 2; break;
    case condional_NP: flagsToTest = PF; oneFlagPos = 2; neg = true; break;
    case condional_L: flagsToTest = SF | OF; singleFlag = false; break;
    case condional_NL: flagsToTest = SF | OF; neg = true; singleFlag = false; break;
    case condional_LE: flagsToTest = SF | OF | ZF; singleFlag = false; checkZF = true;  break;
    case condional_NLE: flagsToTest = SF | OF | ZF; neg = true; singleFlag = false; checkZF = true; break;
    }
    if (singleFlag || multiOrFlag) {
        data->testValue32(xFLAGS, flagsToTest);  
    } else {
        U8 tmpSF = data->getTmpReg();
        U8 tmpOF = data->getTmpReg();
        data->copyBitsFromSourceAtPositionToDest(tmpSF, xFLAGS, 7, 1, false);
        data->copyBitsFromSourceAtPositionToDest(tmpOF, xFLAGS, 11, 1, false);
        if (checkZF) {
            // LE and NLE
            // SF != OF || ZF
            U8 tmpZF = data->getTmpReg();
            data->copyBitsFromSourceAtPositionToDest(tmpZF, xFLAGS, 6, 1, false);
            data->xorRegs32(tmpSF, tmpSF, tmpOF);
            data->orRegs32(tmpSF, tmpSF, tmpZF);
            data->cmpValue32(tmpSF, 0);
            // if tmpSF == 1 then ZF == 0
            // if tmpSF == 0 then ZF == 1
            data->releaseTmpReg(tmpZF);
        } else {
            // L and NL
            // SF != OF
            data->cmpRegs32(tmpSF, tmpOF);
            // if tmpSF != tmpOF then ZF == 0
        }
        data->releaseTmpReg(tmpSF);
        data->releaseTmpReg(tmpOF);
    }
    U8 tmpReg = data->getTmpReg();
    if (neg) {
        // ZF = 1
        data->csetEq(tmpReg);
    } else {
        // ZF = 0
        data->csetNe(tmpReg);
    }
    if (mem) {
        U8 addressReg = data->getAddressReg();
        data->writeMemory(addressReg, tmpReg, 8, true);
        data->releaseTmpReg(addressReg);
    } else {        
        data->movRegToReg8(tmpReg, data->currentOp->reg);        
    }
    data->releaseTmpReg(tmpReg);
}

void opSetO_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_O, false);
}
void opSetO_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_O, true);
}
void opSetNO_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_NO, false);
}
void opSetNO_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_NO, true);
}
void opSetB_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_B, false);
}
void opSetB_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_B, true);
}
void opSetNB_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_NB, false);
}
void opSetNB_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_NB, true);
}
void opSetZ_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_Z, false);
}
void opSetZ_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_Z, true);
}
void opSetNZ_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_NZ, false);
}
void opSetNZ_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_NZ, true);
}
void opSetBE_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_BE, false);
}
void opSetBE_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_BE, true);
}
void opSetNBE_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_NBE, false);
}
void opSetNBE_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_NBE, true);
}
void opSetS_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_S, false);
}
void opSetS_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_S, true);
}
void opSetNS_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_NS, false);
}
void opSetNS_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_NS, true);
}
void opSetP_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_P, false);
}
void opSetP_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_P, true);
}
void opSetNP_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_NP, false);
}
void opSetNP_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_NP, true);
}
void opSetL_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_L, false);
}
void opSetL_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_L, true);
}
void opSetNL_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_NL, false);
}
void opSetNL_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_NL, true);
}
void opSetLE_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_LE, false);
}
void opSetLE_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_LE, true);
}
void opSetNLE_R8(Armv8btAsm* data) {
    doSetCondition(data, condional_NLE, false);
}
void opSetNLE_E8(Armv8btAsm* data) {
    doSetCondition(data, condional_NLE, true);
}

void opSLDTReg(Armv8btAsm* data) {
    klog("ARMv8: SDLT not implemented");
}
void opSLDTE16(Armv8btAsm* data) {
    klog("ARMv8: SDLT not implemented");
}
void opSTRReg(Armv8btAsm* data) {
    klog("ARMv8: STR not implemented");
}
void opSTRE16(Armv8btAsm* data) {
    klog("ARMv8: STR not implemented");
}
void opLLDTR16(Armv8btAsm* data) {
    klog("ARMv8: LLDT not implemented");
}
void opLLDTE16(Armv8btAsm* data) {
    klog("ARMv8: LLDT not implemented");
}
void opLTRR16(Armv8btAsm* data) {
    klog("ARMv8: LTR not implemented");
}
void opLTRE16(Armv8btAsm* data) {
    klog("ARMv8: LTR not implemented");
}
void opVERRR16(Armv8btAsm* data) {
    klog("ARMv8: VERR not implemented");
}
void opVERRE16(Armv8btAsm* data) {
    klog("ARMv8: VERR not implemented");
}
void opVERWR16(Armv8btAsm* data) {
    klog("ARMv8: VERW not implemented");
}
void opVERWE16(Armv8btAsm* data) {
    klog("ARMv8: VERW not implemented");
}
void opSGDT(Armv8btAsm* data) {
    klog("ARMv8: SGDT not implemented");
}
void opSIDT(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getRegWithConst(1023);
    data->writeMemory(addressReg, tmpReg, 16, true);
    data->addValue32(addressReg, addressReg, 2);
    data->loadConst(tmpReg, 0);
    data->writeMemory(addressReg, tmpReg, 32, true);
    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(tmpReg);
}
void opLGDT(Armv8btAsm* data) {
    klog("ARMv8: LGDT not implemented");
}
void opLIDT(Armv8btAsm* data) {
    klog("ARMv8: LIDT not implemented");
}
void opSMSWRreg(Armv8btAsm* data) {
    klog("ARMv8: SMSWR not implemented");
}
void opSMSW(Armv8btAsm* data) {
    klog("ARMv8: SMSW not implemented");
}
void opLMSWRreg(Armv8btAsm* data) {
    klog("ARMv8: LMSWR not implemented");
}
void opLMSW(Armv8btAsm* data) {
    klog("ARMv8: LMSW not implemented");
}
void opINVLPG(Armv8btAsm* data) {
    klog("ARMv8: INVLPG not implemented");
}
// data->movReg8ToReg(untranslatedReg, flagReg);
// readReg = data->getReadNativeReg8(untranslatedReg);
void opXaddR8R8(Armv8btAsm* data) {
    // cpu->src.u8 = *cpu->reg8[op->reg];
    // cpu->dst.u8 = *cpu->reg8[op->rm];
    // cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    // cpu->lazyFlags = FLAGS_ADD8;
    // *cpu->reg8[op->reg] = cpu->dst.u8;
    // *cpu->reg8[op->rm] = cpu->result.u8;
    U32 flags = data->flagsNeeded();

    data->movReg8ToReg(data->currentOp->rm, xDst);
    data->movReg8ToReg(data->currentOp->reg, xSrc);
    data->addRegs32(xResult, xDst, xSrc);
    if (flags) {
        data->zeroExtend(xResult, xResult, 8);
    }
    data->movRegToReg8(xDst, data->currentOp->reg);
    data->movRegToReg8(xResult, data->currentOp->rm);
    ARM8BT_FLAGS_ADD8->setFlags(data, flags);
}
void opXaddR8E8(Armv8btAsm* data) {
    // cpu->src.u8 = *cpu->reg8[op->reg];
    // cpu->dst.u8 = readb(address);
    // cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    // cpu->lazyFlags = FLAGS_ADD8;
    // *cpu->reg8[op->reg] = cpu->dst.u8;
    // writeb(address, cpu->result.u8);
    U32 flags = data->flagsNeeded();
    
    data->movReg8ToReg(data->currentOp->reg, xSrc);

    U8 addressReg = data->getAddressReg();
    U8 memReg = data->getHostMem(addressReg, 8, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;
    data->readMemory(addressReg, xDst, 8, false, data->currentOp->lock != 0);
    data->addRegs32(xResult, xSrc, xDst);
    if (flags) {
        data->zeroExtend(xResult, xResult, 8);
    }
    data->writeMemory(addressReg, xResult, 8, false, data->currentOp->lock != 0, xDst, restartPos);
    data->movRegToReg8(xDst, data->currentOp->reg);
    data->releaseTmpReg(addressReg);
    ARM8BT_FLAGS_ADD8->setFlags(data, flags);
}

void opXaddR16R16(Armv8btAsm* data) {
    // cpu->src.u16 = cpu->reg[op->reg].u16;
    // cpu->dst.u16 = cpu->reg[op->rm].u16;
    // cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    // cpu->lazyFlags = FLAGS_ADD16;
    // cpu->reg[op->reg].u16 = cpu->dst.u16;
    // cpu->reg[op->rm].u16 = cpu->result.u16;
    U32 flags = data->flagsNeeded();
    bool needsHardwareFlags = false;

    if (ARM8BT_FLAGS_ADD16->usesDst(flags)) {
        data->movRegToReg(xDst, data->getNativeReg(data->currentOp->rm), 16, true);
    }
    if (ARM8BT_FLAGS_ADD16->usesSrc(flags)) {
        data->movRegToReg(xSrc, data->getNativeReg(data->currentOp->reg), 16, true);
    }
    needsHardwareFlags = ARM8BT_FLAGS_ADD16->usesHardwareFlags(flags);
    data->addRegs32(xResult, data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->rm), 0, needsHardwareFlags);
    if (flags) {
        data->zeroExtend(xResult, xResult, 16);
    }
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->rm), 16, false);
    data->movRegToReg(data->getNativeReg(data->currentOp->rm), xResult, 16, false);
    ARM8BT_FLAGS_ADD16->setFlags(data, flags);
}
void opXaddR16E16(Armv8btAsm* data) {
    // cpu->src.u16 = cpu->reg[op->reg].u16;
    // cpu->dst.u16 = readw(address);
    // cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    // cpu->lazyFlags = FLAGS_ADD16;
    // cpu->reg[op->reg].u16 = cpu->dst.u16;
    // writew(address, cpu->result.u16);
    U32 flags = data->flagsNeeded();
    bool needsHardwareFlags = false;

    if (ARM8BT_FLAGS_ADD16->usesSrc(flags)) {
        data->movRegToReg(xSrc, data->getNativeReg(data->currentOp->reg), 16, true);
    }
    needsHardwareFlags = ARM8BT_FLAGS_ADD16->usesHardwareFlags(flags);
    U8 addressReg = data->getAddressReg();
    U8 memReg = data->getHostMem(addressReg, 16, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;
    data->readMemory(addressReg, xDst, 16, false, data->currentOp->lock != 0);
    data->addRegs32(xResult, data->getNativeReg(data->currentOp->reg), xDst, 0, needsHardwareFlags);
    if (flags) {
        data->zeroExtend(xResult, xResult, 16);
    }
    data->writeMemory(addressReg, xResult, 16, false, data->currentOp->lock != 0, xDst, restartPos);
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), xDst, 16, false);
    data->releaseTmpReg(addressReg);
    ARM8BT_FLAGS_ADD16->setFlags(data, flags);
}

void opXaddR32R32(Armv8btAsm* data) {
    // cpu->src.u32 = cpu->reg[op->r1].u32;
    // cpu->dst.u32 = cpu->reg[op->r2].u32;
    // cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    // cpu->lazyFlags = FLAGS_ADD32;
    // cpu->reg[op->r1].u32 = cpu->dst.u32;
    // cpu->reg[op->r2].u32 = cpu->result.u32;
    U32 flags = data->flagsNeeded();
    bool needsHardwareFlags = false;

    if (ARM8BT_FLAGS_ADD32->usesDst(flags)) {
        data->movRegToReg(xDst, data->getNativeReg(data->currentOp->rm), 32, false);
    }
    if (ARM8BT_FLAGS_ADD32->usesSrc(flags)) {
        data->movRegToReg(xSrc, data->getNativeReg(data->currentOp->reg), 32, false);
    }
    needsHardwareFlags = ARM8BT_FLAGS_ADD32->usesHardwareFlags(flags);
    data->addRegs32(xResult, data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->rm), 0, needsHardwareFlags);
    data->movRegToReg(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->rm), 32, false);
    data->movRegToReg(data->getNativeReg(data->currentOp->rm), data->getNativeReg(xResult), 32, false);
    ARM8BT_FLAGS_ADD32->setFlags(data, flags);
}
void opXaddR32E32(Armv8btAsm* data) {
    // cpu->src.u32 = cpu->reg[op->r1].u32;
    // cpu->dst.u32 = readd(cpu->thread, eaa);
    // cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    // cpu->lazyFlags = FLAGS_ADD32;
    // cpu->reg[op->r1].u32 = cpu->dst.u32;
    // writed(cpu->thread, eaa, cpu->result.u32);
    U32 flags = data->flagsNeeded();
    bool needsHardwareFlags = false;

    if (ARM8BT_FLAGS_ADD32->usesSrc(flags)) {
        data->movRegToReg(xSrc, data->getNativeReg(data->currentOp->reg), 32, false);
    }
    needsHardwareFlags = ARM8BT_FLAGS_ADD32->usesHardwareFlags(flags);

    U8 addressReg = data->getAddressReg();
    data->readWriteMemory(addressReg, xDst, xResult, 32, [data, needsHardwareFlags] {
        data->addRegs32(xResult, data->getNativeReg(data->currentOp->reg), xDst, 0, needsHardwareFlags);
        }, data->currentOp->lock != 0);

    data->movRegToReg(data->getNativeReg(data->currentOp->reg), xDst, 32, false);
    data->releaseTmpReg(addressReg);
    ARM8BT_FLAGS_ADD32->setFlags(data, flags);
}

// 29 23 03 11          add	w9, w25, #0xc8          // calculate address
// 0a 80 02 8b          add	x10, x0, x2, lsl #32    // copy EAX/ECX into value1

// 2f 69 74 f8          ldr	x15, [x9, x20]          // read 64-bit value2
// 5f 01 0f eb          cmp	x10, x15                // compare value1 and value2
// a0 00 00 54          b.eq	0x502d32524  

// if value1 != value2
// 08 79 19 12          and w8, w8, #0xffffffbf     // clear ZF
// e0 03 0f 2a          mov	w0, w15                 // set EAX to value2
// e2 fd 60 d3          lsr	x2, x15, #32            // set EDX to value2
// 04 00 00 14          b	0x502d32530

// else
// 6b 80 01 8b          add	x11, x3, x1, lsl #32    // combine EDX/EBX so that it will be written in one instruction
// 2b 69 34 f8          str	x11, [x9, x20]          // write EDX/EBX to memory
// 08 01 1a 32          orr	w8, w8, #0x40           // add ZF

void opCmpXchg8b(Armv8btAsm* data) {
    // U64 value1 = ((U64)EDX) << 32 | EAX;
    // U64 value2 = readq(address);
    // cpu->fillFlags();
    // if (value1 == value2) {
    //     cpu->addZF();
    //     writed(address, EBX);
    //     writed(address + 4, ECX);
    // } else {
    //     cpu->removeZF();
    //     EDX = (U32)(value2 >> 32);
    //     EAX = (U32)value2;
    // }
    U32 flags = DecodedOp::getNeededFlags(data->currentBlock, data->currentOp, CF | SF | PF | AF | OF | ZF);
    
    U8 addressReg = data->getAddressReg();    
    U8 tmpReg = data->getTmpReg();
    data->addRegs64(tmpReg, xEAX, xEDX, 32);

    U8 memReg = data->getHostMem(addressReg, 64, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;
    data->readMemory(addressReg, xSrc, 64, false, data->currentOp->lock != 0);
    data->cmpRegs64(tmpReg, xSrc);
    data->doIf(0, 0, DO_IF_EQUAL, [restartPos, addressReg, data, flags]() {
        U8 tmpReg2 = data->getTmpReg();
        data->addRegs64(tmpReg2, xEBX, xECX, 32);        
        data->writeMemory(addressReg, tmpReg2, 64, false, data->currentOp->lock != 0, xSrc, restartPos, false);
        if (flags & ZF) {
            data->orValue32(xFLAGS, xFLAGS, ZF);
        }
        data->releaseTmpReg(tmpReg2);
        }, [data, flags] {
            if (flags & ZF) {
                data->andValue32(xFLAGS, xFLAGS, ~ZF);
            }
            data->movRegToReg(xEAX, xSrc, 32, false);
            data->copyBitsFromSourceAtPositionToDest64(xEDX, xSrc, 32, 32, false);
        }, nullptr, false, false);

    if (data->currentOp->lock != 0) {
        data->fullMemoryBarrier(); // don't allow out of order read/write after this instruction until this completes
    }
    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(tmpReg);
}

void opBswap32(Armv8btAsm* data) {
    data->reverseBytes32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg));
}

void opEmms(Armv8btAsm* data) {}
void opFxsave(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
}

void opFxrstor(Armv8btAsm* data) {
    data->emulateSingleOp(data->currentOp);
}

void opLdmxcsr(Armv8btAsm* data) {
    // :TODO: need to implement SSE control/status register
}
void opStmxcsr(Armv8btAsm* data) {
    // :TODO: need to implement SSE control/status register
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getRegWithConst(0x1F80);
    data->writeMemory(addressReg, tmpReg, 32, true);
    data->releaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}
void opXsave(Armv8btAsm* data) {} // Core 2+
void opXrstor(Armv8btAsm* data) {} // Core 2+
void opLfence(Armv8btAsm* data) {
    data->fullMemoryBarrier(); // :TODO: is a full barrier necessary (dmb ishld)
}
void opMfence(Armv8btAsm* data) {
    data->fullMemoryBarrier();
}
void opSfence(Armv8btAsm* data) {
    data->fullMemoryBarrier(); // :TODO: is a full barrier necessary
}
void opClflush(Armv8btAsm* data) {
    klog("clflush called.  Not sure what to do");
}

// SSE1
void opAddpsXmm(Armv8btAsm* data) {
    data->fAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opAddpsE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->fAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opAddssXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->fAdd(vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opAddssE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, vTmpReg, true);
    data->fAdd(vTmpReg, data->getNativeSseReg(data->currentOp->reg), vTmpReg, S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opSubpsXmm(Armv8btAsm* data) {
    data->fSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opSubpsE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->fSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opSubssXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->fSub(vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opSubssE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, vTmpReg, true);
    data->fSub(vTmpReg, data->getNativeSseReg(data->currentOp->reg), vTmpReg, S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opMulpsXmm(Armv8btAsm* data) {
    data->fMul(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opMulpsE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->fMul(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opMulssXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->fMul(vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opMulssE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, vTmpReg, true);
    data->fMul(vTmpReg, data->getNativeSseReg(data->currentOp->reg), vTmpReg, S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opDivpsXmm(Armv8btAsm* data) {
    data->fDiv(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opDivpsE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->fDiv(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opDivssXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->fDiv(vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opDivssE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, vTmpReg, true);
    data->fDiv(vTmpReg, data->getNativeSseReg(data->currentOp->reg), vTmpReg, S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opRcppsXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].ps.f32[0] = 1.0f / cpu->xmm[rm].ps.f32[0]
    // cpu->xmm[reg].ps.f32[1] = 1.0f / cpu->xmm[rm].ps.f32[1]
    // cpu->xmm[reg].ps.f32[2] = 1.0f / cpu->xmm[rm].ps.f32[2]
    // cpu->xmm[reg].ps.f32[3] = 1.0f / cpu->xmm[rm].ps.f32[3]
    data->fReciprocal(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opRcppsE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->fReciprocal(data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opRcpssXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].ps.f32[0] = 1.0f / cpu->xmm[rm].ps.f32[0]
    U8 vTmpReg = data->vGetTmpReg();
    data->fReciprocal(vTmpReg, data->getNativeSseReg(data->currentOp->rm), S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opRcpssE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, vTmpReg, true);
    data->fReciprocal(vTmpReg, vTmpReg, S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opSqrtpsXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.f32[0] = sqrt(cpu->xmm[rm].pd.f32[0] )
    // cpu->xmm[reg].pd.f32[1] = sqrt(cpu->xmm[rm].pd.f32[1] )
    // cpu->xmm[reg].pd.f32[2] = sqrt(cpu->xmm[rm].pd.f32[2] )
    // cpu->xmm[reg].pd.f32[3] = sqrt(cpu->xmm[rm].pd.f32[3] )
    data->fSqrt(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opSqrtpsE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->fSqrt(data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opSqrtssXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].ps.f32[0] = sqrt(cpu->xmm[rm].ps.f32[0] )
    U8 vTmpReg = data->vGetTmpReg();
    data->fSqrt(vTmpReg, data->getNativeSseReg(data->currentOp->rm), S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opSqrtssE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, vTmpReg, true);
    data->fSqrt(vTmpReg, vTmpReg, S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opRsqrtpsXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].ps.f32[0] = 1.0f / sqrt(cpu->xmm[rm].ps.f32[0] )
    // cpu->xmm[reg].ps.f32[1] = 1.0f / sqrt(cpu->xmm[rm].ps.f32[1] )
    // cpu->xmm[reg].ps.f32[2] = 1.0f / sqrt(cpu->xmm[rm].ps.f32[2] )
    // cpu->xmm[reg].ps.f32[3] = 1.0f / sqrt(cpu->xmm[rm].ps.f32[3] )
    data->fRsqrt(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opRsqrtpsE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->fRsqrt(data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opRsqrtssXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].ps.f32[0] = 1.0f / sqrt(cpu->xmm[rm].ps.f32[0] )
    U8 vTmpReg = data->vGetTmpReg();
    data->fRsqrt(vTmpReg, data->getNativeSseReg(data->currentOp->rm), S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opRsqrtssE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, vTmpReg, true);
    data->fRsqrt(vTmpReg, vTmpReg, S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opPavgbXmmXmm(Armv8btAsm* data) {
    data->vUnsignedRoundedAverage(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPavgbXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vUnsignedRoundedAverage(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPavgwXmmXmm(Armv8btAsm* data) {
    data->vUnsignedRoundedAverage(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPavgwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vUnsignedRoundedAverage(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsadbwXmmXmm(Armv8btAsm* data) {
    data->vUnsignedAbsoluteDifference(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->rm), B16);
    data->vUnsignedAddPairsLong(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), B16);
    data->vUnsignedAddPairsLong(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), H8);
    data->vUnsignedAddPairsLong(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), S4);
}

void opPsadbwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vUnsignedAbsoluteDifference(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), vTmpReg, B16);
    data->vUnsignedAddPairsLong(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), B16);
    data->vUnsignedAddPairsLong(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), H8);
    data->vUnsignedAddPairsLong(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opPextrwR32Xmm(Armv8btAsm* data) {
    data->vMovToGeneralReg32ZeroExtend(data->getNativeReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), data->currentOp->imm & 7, H8);
}
void opPextrwE16Xmm(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory16(addressReg, data->getNativeSseReg(data->currentOp->reg), data->currentOp->imm & 7, true);
    data->releaseTmpReg(addressReg);
}

void opPinsrwXmmR32(Armv8btAsm* data) {
    data->vMovFromGeneralReg16(data->getNativeSseReg(data->currentOp->reg), data->currentOp->imm & 7, data->getNativeReg(data->currentOp->rm));
}

void opPinsrwXmmE16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vReadMemory16(addressReg, data->getNativeSseReg(data->currentOp->reg), data->currentOp->imm & 7, true);
    data->releaseTmpReg(addressReg);
}

void opPmaxswXmmXmm(Armv8btAsm* data) {
    data->vSignedMax(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPmaxswXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSignedMax(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPmaxubXmmXmm(Armv8btAsm* data) {
    data->vUnsignedMax(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPmaxubXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vUnsignedMax(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPminswXmmXmm(Armv8btAsm* data) {
    data->vSignedMin(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
    data->vSignedMin(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPminswXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSignedMin(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPminubXmmXmm(Armv8btAsm* data) {
    data->vUnsignedMin(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPminubXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vUnsignedMin(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opPmovmskbR32Xmm(Armv8btAsm* data) {
    // for all 16 bytes set a bit in a mask if it is signed
    U8 vTmpReg = data->vGetTmpReg();

    U8 bitMaskReg = data->getSSEConstant(SSE_BYTE8_BIT_MASK);
    // turn all the bits to 1 if signed
    data->vSignedShiftRightValue(vTmpReg, data->getNativeSseReg(data->currentOp->rm), 7, B16);
    // mask out the bit that should be set, so index 0 will set bit 0, index 1 will set bit 1, etc
    data->vAnd(vTmpReg, vTmpReg, bitMaskReg, B16);

    U8 vTmpReg2 = data->vGetTmpReg();
    data->vDup(vTmpReg2, vTmpReg, 1, D2);

    data->vAddAcrossVectorToScaler(vTmpReg, vTmpReg, B8);
    data->vAddAcrossVectorToScaler(vTmpReg2, vTmpReg2, B8);

    U8 tmpReg = data->getTmpReg();
    U8 tmpReg2 = data->getTmpReg();

    data->vMovToGeneralReg32ZeroExtend(tmpReg, vTmpReg, 0, B16);
    data->vMovToGeneralReg32ZeroExtend(tmpReg2, vTmpReg2, 0, B16);
    data->addRegs32(data->getNativeReg(data->currentOp->reg), tmpReg, tmpReg2, 8);

    data->releaseTmpReg(tmpReg);
    data->releaseTmpReg(tmpReg2);
    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(vTmpReg2);
    data->vReleaseTmpReg(bitMaskReg);
}
void opPmulhuwXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    // high result of H0*H0, H1*H1, H2*H2 and H3*H3 will be in H1, H3, H5, H7
    data->vUnsignedMulLongLower(vTmpReg1, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H4);
    // high result of H4*H4, H5*H5, H6*H6 and H7*H7 will be in H1, H3, H5, H7
    data->vUnsignedMulLongUpper(vTmpReg2, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
    data->vUnzipOdds(data->getNativeSseReg(data->currentOp->reg), vTmpReg1, vTmpReg2, H8);
    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vTmpReg2);
}
void opPmulhuwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg2, true);
    data->vUnsignedMulLongLower(vTmpReg1, data->getNativeSseReg(data->currentOp->reg), vTmpReg2, H4);
    data->vUnsignedMulLongUpper(vTmpReg2, data->getNativeSseReg(data->currentOp->reg), vTmpReg2, H8);
    data->vUnzipOdds(data->getNativeSseReg(data->currentOp->reg), vTmpReg1, vTmpReg2, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vTmpReg2);
}

void opAndnpsXmmXmm(Armv8btAsm* data) {
    data->vAndNot(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), data->getNativeSseReg(data->currentOp->reg), B16);
}
void opAndnpsXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vAndNot(data->getNativeSseReg(data->currentOp->reg), vTmpReg, data->getNativeSseReg(data->currentOp->reg), B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opAndpsXmmXmm(Armv8btAsm* data) {
    data->vAnd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opAndpsXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vAnd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opOrpsXmmXmm(Armv8btAsm* data) {
    data->vOr(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opOrpsXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vOr(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opXorpsXmmXmm(Armv8btAsm* data) {
    data->vXor(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opXorpsXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vXor(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opMovapsXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[r1].ps = cpu->xmm[r2].ps;
    data->vMov128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm));
}
void opMovapsXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovapsE128Xmm(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovhlpsXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[r1].ps.u64[0] = cpu->xmm[r2].ps.u64[1];
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, data->getNativeSseReg(data->currentOp->rm), 1);
}
void opMovlhpsXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[r1].ps.u64[1] = cpu->xmm[r2].ps.u64[0];
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 1, data->getNativeSseReg(data->currentOp->rm), 0);
}
void opMovhpsXmmE64(Armv8btAsm* data) {
    // cpu->xmm[reg].ps.u64[1] = readq(address);
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, data->getNativeSseReg(data->currentOp->reg), 1, true);
    data->releaseTmpReg(addressReg);
}
void opMovhpsE64Xmm(Armv8btAsm* data) {
    // writeq(address, cpu->xmm[reg].ps.u64[1]);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory64(addressReg, data->getNativeSseReg(data->currentOp->reg), 1, true);
    data->releaseTmpReg(addressReg);
}
void opMovlpsXmmE64(Armv8btAsm* data) {
    // cpu->xmm[reg].ps.u64[0] = readq(address);
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, data->currentOp->reg, 0, true);
    data->releaseTmpReg(addressReg);
}
void opMovlpsE64Xmm(Armv8btAsm* data) {
    // writeq(address, cpu->xmm[reg].ps.u64[0]);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory64(addressReg, data->getNativeSseReg(data->currentOp->reg), 0, true);
    data->releaseTmpReg(addressReg);
}

void opMovmskpsR32Xmm(Armv8btAsm* data) {
    // cpu->reg[reg].u32 = (cpu->xmm[rm].pd.u32[0] >> 31) | ((cpu->xmm[rm].pd.u32[1] >> 31) << 1) | ((cpu->xmm[rm].pd.u32[2] >> 31) << 2) | ((cpu->xmm[rm].pd.u32[3] >> 31) << 3)
    U8 vTmpReg = data->vGetTmpReg();

    U8 bitMaskReg = data->getSSEConstant(SSE_INT32_BIT_MASK);
    data->vSignedShiftRightValue(vTmpReg, data->getNativeSseReg(data->currentOp->rm), 31, S4);
    data->vAnd(vTmpReg, vTmpReg, bitMaskReg, B16);
    data->vAddAcrossVectorToScaler(vTmpReg, vTmpReg, S4);
    data->vMovToGeneralReg32ZeroExtend(data->getNativeReg(data->currentOp->reg), vTmpReg, 0, S4);

    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(bitMaskReg);
}
void opMovssXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[r1].ps.u32[0] = cpu->xmm[r2].ps.u32[0];
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, data->getNativeSseReg(data->currentOp->rm), 0);
}
void opMovssXmmE32(Armv8btAsm* data) {
    // cpu->xmm[reg].ps.u32[0] = readd(address);
    // cpu->xmm[reg].ps.u32[1] = 0;
    // cpu->xmm[reg].ps.u32[2] = 0;
    // cpu->xmm[reg].ps.u32[3] = 0;
    U8 addressReg = data->getAddressReg();
    data->vReadMemory32(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovssE32Xmm(Armv8btAsm* data) {
    // writed(address, cpu->xmm[reg].ps.u32[0]);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory32(addressReg, data->getNativeSseReg(data->currentOp->reg), 0, true);
    data->releaseTmpReg(addressReg);
}
void opMovupsXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[r1].ps = cpu->xmm[r2].ps;
    data->vMov128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm));
}
void opMovupsXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovupsE128Xmm(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovntpsE128Xmm(Armv8btAsm* data) {
    // writeq(address, cpu->xmm[reg].ps.u64[0]);
    // writeq(address + 8, cpu->xmm[reg].ps.u64[1]);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opUnpckhpsXmmXmm(Armv8btAsm* data) {
    data->vZipFromHigh128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opUnpckhpsXmmE128(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    data->vZipFromHigh128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), tmpReg, S4);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}
void opUnpcklpsXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].ps.f32[0] = cpu->xmm[reg].ps.f32[0];
    // cpu->xmm[reg].ps.f32[1] = cpu->xmm[rm].ps.f32[0];
    // cpu->xmm[reg].ps.f32[2] = cpu->xmm[reg].ps.f32[1];
    // cpu->xmm[reg].ps.f32[3] = cpu->xmm[rm].ps.f32[1];
    data->vZipFromLow128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opUnpcklpsXmmE128(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, tmpReg, true);
    data->vZipFromLow128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), tmpReg, S4);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}
void opPrefetchT0(Armv8btAsm* data) {
    // nop
}
void opPrefetchT1(Armv8btAsm* data) {
    // nop
}
void opPrefetchT2(Armv8btAsm* data) {
    // nop
}
void opPrefetchNTA(Armv8btAsm* data) {
    // nop
}

void fcmp(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, VectorWidth width) {
    int which = data->currentOp->imm & 7;
    switch (which) {
    case 0: // eq
        data->fCmpEqual(dst, src1, src2, width);
        break;
    case 1: // lt
        data->fCmpLessThan(dst, src1, src2, width);
        break;
    case 2: // le
        data->fCmpLessThanOrEqual(dst, src1, src2, width);
        break;
    case 3: // unord
    {
        U8 vTmpReg = data->vGetTmpReg();
        U8 vTmpReg2 = data->vGetTmpReg();
        data->fCmpEqual(vTmpReg, src1, src1, width);
        data->fCmpEqual(vTmpReg2, src2, src2, width);
        data->vAnd(dst, vTmpReg, vTmpReg2, B16);
        data->vNot(dst, dst, B16);
        data->vReleaseTmpReg(vTmpReg);
        data->vReleaseTmpReg(vTmpReg2);
        break;
    }
    case 4: // neq
        data->fCmpEqual(dst, src1, src2, width);
        data->vNot(dst, dst, B16);
        break;
    case 5: // nlt
        data->fCmpLessThan(dst, src1, src2, width);
        data->vNot(dst, dst, B16);
        break;
    case 6: // nle
        data->fCmpLessThanOrEqual(dst, src1, src2, width);
        data->vNot(dst, dst, B16);
        break;
    case 7: // ord
    {
        U8 vTmpReg = data->vGetTmpReg();
        U8 vTmpReg2 = data->vGetTmpReg();
        data->fCmpEqual(vTmpReg, src1, src1, width);
        data->fCmpEqual(vTmpReg2, src2, src2, width);
        data->vAnd(dst, vTmpReg, vTmpReg2, B16);
        data->vReleaseTmpReg(vTmpReg);
        data->vReleaseTmpReg(vTmpReg2);
        break;
    }

    }
}

void opCmppsXmmXmm(Armv8btAsm* data) {
    fcmp(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opCmppsXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    fcmp(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opCmpssXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    fcmp(data, vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
}
void opCmpssXmmE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, vTmpReg, true);
    fcmp(data, vTmpReg, data->getNativeSseReg(data->currentOp->reg), vTmpReg, S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void comisXmmXmm(Armv8btAsm* data, U8 reg1, U8 reg2, bool is64bit) {
    // cpu->flags &= ~(AF | OF | SF | CF | PF | ZF);
    // if (isnan(a.f64[0]) || isnan(b.f64[0])) {
    //     cpu->flags |= CF | ZF | PF;
    // } else if (a.f64[0] == b.f64[0]) {
    //     cpu->flags |= ZF;
    // } else if (a.f64[0] < b.f64[0]) {
    //     cpu->flags |= CF;
    // }
    U8 tmpReg = data->getTmpReg();

    // cpu->flags &= ~(AF | OF | SF | CF | PF | ZF);
    data->movn(tmpReg, AF | OF | SF | CF | PF | ZF);
    data->andRegs32(xFLAGS, xFLAGS, tmpReg);

    if (is64bit) {
        data->fCmp64(reg1, reg2);
    } else {
        data->fCmp32(reg1, reg2);
    }

    // if (isnan(a.f64[0]) || isnan(b.f64[0])) { cpu->flags |= CF | ZF | PF; }
    data->orValue32(tmpReg, xFLAGS, CF | PF | ZF); // 2 instructions, 1 for load, 1 for or
    data->cselVs(xFLAGS, tmpReg, xFLAGS);

    // else if (a.f64[0] == b.f64[0]) { cpu->flags |= ZF; }
    data->csetEq(tmpReg);
    data->orRegs32(xFLAGS, xFLAGS, tmpReg, 6); // ZF

    // else if (a.f64[0] < b.f64[0]) {cpu->flags |= CF; }
    data->csetLt(tmpReg);
    data->orRegs32(xFLAGS, xFLAGS, tmpReg); // CF

    data->releaseTmpReg(tmpReg);
}

void opComissXmmXmm(Armv8btAsm* data) {
    comisXmmXmm(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), false);
}

void opComissXmmE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    comisXmmXmm(data, data->getNativeSseReg(data->currentOp->reg), vTmpReg, false);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opUcomissXmmXmm(Armv8btAsm* data) {
    opComissXmmXmm(data);
}
void opUcomissXmmE32(Armv8btAsm* data) {
    opComissXmmE32(data);
}

// SSE2
void opAddpdXmmXmm(Armv8btAsm* data) {
    data->fAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
}
void opAddpdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->fAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opAddsdXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->fAdd(vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opAddsdXmmE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->fAdd(vTmpReg, data->getNativeSseReg(data->currentOp->reg), vTmpReg, D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opSubpdXmmXmm(Armv8btAsm* data) {
    data->fSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
}
void opSubpdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->fSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opSubsdXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->fSub(vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opSubsdXmmE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->fSub(vTmpReg, data->getNativeSseReg(data->currentOp->reg), vTmpReg, D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opMulpdXmmXmm(Armv8btAsm* data) {
    data->fMul(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
}
void opMulpdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->fMul(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opMulsdXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->fMul(vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opMulsdXmmE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->fMul(vTmpReg, data->getNativeSseReg(data->currentOp->reg), vTmpReg, D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opDivpdXmmXmm(Armv8btAsm* data) {
    data->fDiv(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
}

void opDivpdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->fDiv(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opDivsdXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->fDiv(vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opDivsdXmmE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->fDiv(vTmpReg, data->getNativeSseReg(data->currentOp->reg), vTmpReg, D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPaddbXmmXmm(Armv8btAsm* data) {
    data->vAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPaddbXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPaddwXmmXmm(Armv8btAsm* data) {
    data->vAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPaddwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPadddXmmXmm(Armv8btAsm* data) {
    data->vAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opPadddXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPaddqXmmXmm(Armv8btAsm* data) {
    data->vAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
}
void opPaddqXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPaddsbXmmXmm(Armv8btAsm* data) {
    data->vSignedSaturatingAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPaddsbXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSignedSaturatingAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPaddswXmmXmm(Armv8btAsm* data) {
    data->vSignedSaturatingAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPaddswXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSignedSaturatingAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPaddusbXmmXmm(Armv8btAsm* data) {
    data->vUnsignedSaturatingAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPaddusbXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vUnsignedSaturatingAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPadduswXmmXmm(Armv8btAsm* data) {
    data->vUnsignedSaturatingAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPadduswXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vUnsignedSaturatingAdd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubbXmmXmm(Armv8btAsm* data) {
    data->vSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPsubbXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubwXmmXmm(Armv8btAsm* data) {
    data->vSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPsubwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubdXmmXmm(Armv8btAsm* data) {
    data->vSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opPsubdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubqXmmXmm(Armv8btAsm* data) {
    data->vSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
}
void opPsubqXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubsbXmmXmm(Armv8btAsm* data) {
    data->vSignedSaturatingSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPsubsbXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSignedSaturatingSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubswXmmXmm(Armv8btAsm* data) {
    data->vSignedSaturatingSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPsubswXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSignedSaturatingSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubusbXmmXmm(Armv8btAsm* data) {
    data->vUnsignedSaturatingSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPsubusbXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vUnsignedSaturatingSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubuswXmmXmm(Armv8btAsm* data) {
    data->vUnsignedSaturatingSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPsubuswXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vUnsignedSaturatingSub(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPmaddwdXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    // multiply bottom 4x 16-bit numbers and put them into 4x 32-bit number in vTmpReg1
    data->vSignedMulLongLower(vTmpReg1, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H4);
    // multiply top 4x 16-bit numbers and put them into 4x 32-bit number in vTmpReg2
    data->vSignedMulLongUpper(vTmpReg2, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
    data->vAddPairs(data->getNativeSseReg(data->currentOp->reg), vTmpReg1, vTmpReg2, S4);
    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vTmpReg2);
}
void opPmaddwdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg2, true);
    data->vSignedMulLongLower(vTmpReg1, data->getNativeSseReg(data->currentOp->reg), vTmpReg2, H4);
    data->vSignedMulLongUpper(vTmpReg2, data->getNativeSseReg(data->currentOp->reg), vTmpReg2, H8);
    data->vAddPairs(data->getNativeSseReg(data->currentOp->reg), vTmpReg1, vTmpReg2, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vTmpReg2);
}
void opPmulhwXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    // high result of H0*H0, H1*H1, H2*H2 and H3*H3 will be in H1, H3, H5, H7
    data->vSignedMulLongLower(vTmpReg1, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H4);
    // high result of H4*H4, H5*H5, H6*H6 and H7*H7 will be in H1, H3, H5, H7
    data->vSignedMulLongUpper(vTmpReg2, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
    data->vUnzipOdds(data->getNativeSseReg(data->currentOp->reg), vTmpReg1, vTmpReg2, H8);
    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vTmpReg2);
}
void opPmulhwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg2, true);
    data->vSignedMulLongLower(vTmpReg1, data->getNativeSseReg(data->currentOp->reg), vTmpReg2, H4);
    data->vSignedMulLongUpper(vTmpReg2, data->getNativeSseReg(data->currentOp->reg), vTmpReg2, H8);
    data->vUnzipOdds(data->getNativeSseReg(data->currentOp->reg), vTmpReg1, vTmpReg2, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vTmpReg2);
}
void opPmullwXmmXmm(Armv8btAsm* data) {
    data->vMul(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPmullwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vMul(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPmuludqXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[r1].pi.u64[0] = cpu->xmm[r1].pi.u32[0] * cpu->xmm[r2].pi.u32[0]
    // cpu->xmm[r1].pi.u64[1] = cpu->xmm[r1].pi.u32[2] * cpu->xmm[r2].pi.u32[2]
    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vConvertInt64ToLowerInt32(vTmpReg1, data->getNativeSseReg(data->currentOp->reg));
    data->vConvertInt64ToLowerInt32(vTmpReg2, data->getNativeSseReg(data->currentOp->rm));
    data->vUnsignedMulLongLower(data->getNativeSseReg(data->currentOp->reg), vTmpReg1, vTmpReg2, S2);
    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vTmpReg2);
}
void opPmuludqXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg1, true);
    data->vConvertInt64ToLowerInt32(vTmpReg1, vTmpReg1);
    data->vConvertInt64ToLowerInt32(vTmpReg2, data->getNativeSseReg(data->currentOp->reg));
    data->vUnsignedMulLongLower(data->getNativeSseReg(data->currentOp->reg), vTmpReg1, vTmpReg2, S2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vTmpReg2);
}
void opSqrtpdXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.f64[0] = sqrt(cpu->xmm[rm].pd.f64[0] )
    // cpu->xmm[reg].pd.f64[1] = sqrt(cpu->xmm[rm].pd.f64[1] )
    data->fSqrt(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
}
void opSqrtpdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->fSqrt(data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opSqrtsdXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.f64[0] = sqrt(cpu->xmm[rm].pd.f64[0] )
    U8 vTmpReg = data->vGetTmpReg();
    data->fSqrt(vTmpReg, data->getNativeSseReg(data->currentOp->rm), D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opSqrtsdXmmE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->fSqrt(vTmpReg, vTmpReg, D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opAndnpdXmmXmm(Armv8btAsm* data) {
    data->vAndNot(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), data->getNativeSseReg(data->currentOp->reg), B16);
}
void opAndnpdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vAndNot(data->getNativeSseReg(data->currentOp->reg), vTmpReg, data->getNativeSseReg(data->currentOp->reg), B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opAndpdXmmXmm(Armv8btAsm* data) {
    data->vAnd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opAndpdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vAnd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPandXmmXmm(Armv8btAsm* data) {
    data->vAnd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPandXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vAnd(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPandnXmmXmm(Armv8btAsm* data) {
    data->vAndNot(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), data->getNativeSseReg(data->currentOp->reg), B16);
}
void opPandnXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vAndNot(data->getNativeSseReg(data->currentOp->reg), vTmpReg, data->getNativeSseReg(data->currentOp->reg), B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPorXmmXmm(Armv8btAsm* data) {
    data->vOr(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), data->getNativeSseReg(data->currentOp->reg), B16);
}
void opPorXmmXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vOr(data->getNativeSseReg(data->currentOp->reg), vTmpReg, data->getNativeSseReg(data->currentOp->reg), B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPslldqXmm(Armv8btAsm* data) {
    // _mm_srli_si128
    if (data->currentOp->imm > 15) {
        data->vLoadConst(data->getNativeSseReg(data->currentOp->reg), 0, B16);
    } else {
        U8 vTmpReg = data->vGetTmpReg();
        data->vLoadConst(vTmpReg, 0, B16);
        data->vExtractVectorFromPair(data->getNativeSseReg(data->currentOp->reg), vTmpReg, data->getNativeSseReg(data->currentOp->reg), 16-data->currentOp->imm);
        data->vReleaseTmpReg(vTmpReg);
    }
}
void opPsllqXmm(Armv8btAsm* data) {
    data->vShiftLeftValue(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->currentOp->imm, D2);
}
void opPsllqXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeSseReg(data->currentOp->rm), 0, D2);
    data->vShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsllqXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, D2);
    data->vShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPslldXmm(Armv8btAsm* data) {
    data->vShiftLeftValue(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->currentOp->imm, S4);
}
void opPslldXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeSseReg(data->currentOp->rm), 0, S4);
    data->vShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->vReleaseTmpReg(vTmpReg);
}
void opPslldXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, S4);
    data->vShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsllwXmm(Armv8btAsm* data) {
    data->vShiftLeftValue(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->currentOp->imm, H8);
}
void opPsllwXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeSseReg(data->currentOp->rm), 0, H8);
    data->vShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsllwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, H8);
    data->vShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsradXmm(Armv8btAsm* data) {
    data->vSignedShiftRightValue(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->currentOp->imm, S4);
}
void opPsradXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeSseReg(data->currentOp->rm), 0, S4);
    data->vNeg(vTmpReg, vTmpReg, S4);
    data->vSignedShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsradXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, S4);
    data->vNeg(vTmpReg, vTmpReg, S4);
    data->vSignedShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrawXmm(Armv8btAsm* data) {
    data->vSignedShiftRightValue(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->currentOp->imm, H8);
}
void opPsrawXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeSseReg(data->currentOp->rm), 0, H8);
    data->vNeg(vTmpReg, vTmpReg, H8);
    data->vSignedShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrawXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, H8);
    data->vNeg(vTmpReg, vTmpReg, H8);
    data->vSignedShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrldqXmm(Armv8btAsm* data) {
    // _mm_srli_si128
    if (data->currentOp->imm > 15) {
        data->vLoadConst(data->getNativeSseReg(data->currentOp->reg), 0, B16);
    } else {
        U8 vTmpReg = data->vGetTmpReg();
        data->vLoadConst(vTmpReg, 0, B16);
        data->vExtractVectorFromPair(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, data->currentOp->imm);
        data->vReleaseTmpReg(vTmpReg);
    }
}
void opPsrlqXmm(Armv8btAsm* data) {
    data->vShiftRightValue(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->currentOp->imm, D2);
}
void opPsrlqXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeSseReg(data->currentOp->rm), 0, D2);
    data->vNeg(vTmpReg, vTmpReg, D2);
    data->vShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrlqXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, D2);
    data->vNeg(vTmpReg, vTmpReg, D2);
    data->vShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrldXmm(Armv8btAsm* data) {
    data->vShiftRightValue(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->currentOp->imm, S4);
}
void opPsrldXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeSseReg(data->currentOp->rm), 0, S4);
    data->vNeg(vTmpReg, vTmpReg, S4);
    data->vShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrldXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, S4);
    data->vNeg(vTmpReg, vTmpReg, S4);
    data->vShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrlwXmm(Armv8btAsm* data) {
    data->vShiftRightValue(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->currentOp->imm, H8);
}
void opPsrlwXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeSseReg(data->currentOp->rm), 0, H8);
    data->vNeg(vTmpReg, vTmpReg, H8);
    data->vShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrlwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, H8);
    data->vNeg(vTmpReg, vTmpReg, H8);
    data->vShiftWithReg(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPxorXmmXmm(Armv8btAsm* data) {
    data->vXor(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPxorXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vXor(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opOrpdXmmXmm(Armv8btAsm* data) {
    data->vOr(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opOrpdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vOr(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opXorpdXmmXmm(Armv8btAsm* data) {
    data->vXor(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opXorpdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vXor(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opCmppdXmmXmm(Armv8btAsm* data) {
    fcmp(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
}
void opCmppdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    fcmp(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opCmpsdXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    fcmp(data, vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
}
void opCmpsdXmmE64(Armv8btAsm* data) {    
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    fcmp(data, vTmpReg, data->getNativeSseReg(data->currentOp->reg), vTmpReg, D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opComisdXmmXmm(Armv8btAsm* data) {
    comisXmmXmm(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), true);
}

void opComisdXmmE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    comisXmmXmm(data, data->getNativeSseReg(data->currentOp->reg), vTmpReg, true);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opUcomisdXmmXmm(Armv8btAsm* data) {
    opComisdXmmXmm(data);
}
void opUcomisdXmmE64(Armv8btAsm* data) {
    opComisdXmmE64(data);
}
void opPcmpgtbXmmXmm(Armv8btAsm* data) {
    data->vCmpGreaterThan(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPcmpgtbXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vCmpGreaterThan(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPcmpgtwXmmXmm(Armv8btAsm* data) {
    data->vCmpGreaterThan(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPcmpgtwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vCmpGreaterThan(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPcmpgtdXmmXmm(Armv8btAsm* data) {
    data->vCmpGreaterThan(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opPcmpgtdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vCmpGreaterThan(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPcmpeqbXmmXmm(Armv8btAsm* data) {
    data->vCmpEqual(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPcmpeqbXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vCmpEqual(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPcmpeqwXmmXmm(Armv8btAsm* data) {
    data->vCmpEqual(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPcmpeqwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vCmpEqual(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPcmpeqdXmmXmm(Armv8btAsm* data) {
    data->vCmpEqual(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opPcmpeqdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vCmpEqual(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opMovqXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].pi.u64[0] = cpu->xmm[rm].pi.u64[0];
    // cpu->xmm[reg].pi.u64[1] = 0;
    data->vZeroExtend64To128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm));
}
void opMovqE64Xmm(Armv8btAsm* data) {
    // writeq(address, cpu->xmm[reg].pi.u64[0]);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory64(addressReg, data->getNativeSseReg(data->currentOp->reg), 0, true);
    data->releaseTmpReg(addressReg);
}
void opMovqXmmE64(Armv8btAsm* data) {
    // cpu->xmm[reg].pi.u64[0] = readq(address);
    // cpu->xmm[reg].pi.u64[1] = 0;
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovsdXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.u64[0] = cpu->xmm[rm].pd.u64[0]
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, data->getNativeSseReg(data->currentOp->rm), 0);
}
void opMovsdXmmE64(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.u64[0] = readq(address);
    // cpu->xmm[reg].pd.u64[1] = 0; // yes, memory to reg will 0 out the top, but xmm to xmm does not, unlike movq
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovsdE64Xmm(Armv8btAsm* data) {
    // writeq(address, cpu->xmm[reg].pd.u64[0]);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory64(addressReg, data->getNativeSseReg(data->currentOp->reg), 0, true);
    data->releaseTmpReg(addressReg);
}
void opMovapdXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[r1].pd = cpu->xmm[r2].pd;
    data->vMov128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm));
}
void opMovapdXmmE128(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.u64[0] = readq(address);
    // cpu->xmm[reg].pd.u64[1] = readq(address + 8);
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovapdE128Xmm(Armv8btAsm* data) {
    // writeq(address, cpu->xmm[reg].pd.u64[0]);
    // writeq(address + 8, cpu->xmm[reg].pd.u64[1]);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovupdXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[r1].pd = cpu->xmm[r2].pd;
    data->vMov128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm));
}
void opMovupdXmmE128(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.u64[0] = readq(address);
    // cpu->xmm[reg].pd.u64[1] = readq(address + 8);
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovupdE128Xmm(Armv8btAsm* data) {
    // writeq(address, cpu->xmm[reg].pd.u64[0]);
    // writeq(address + 8, cpu->xmm[reg].pd.u64[1]);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovhpdXmmE64(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.u64[1] = readq(address);
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, data->getNativeSseReg(data->currentOp->reg), 1, true);
    data->releaseTmpReg(addressReg);
}
void opMovhpdE64Xmm(Armv8btAsm* data) {
    // writeq(address, cpu->xmm[reg].pd.u64[1]);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory64(addressReg, data->getNativeSseReg(data->currentOp->reg), 1, true);
    data->releaseTmpReg(addressReg);
}
void opMovlpdXmmE64(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.u64[0] = readq(address);
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, data->currentOp->reg, 0, true);
    data->releaseTmpReg(addressReg);
}
void opMovlpdE64Xmm(Armv8btAsm* data) {
    // writeq(address, cpu->xmm[reg].pd.u64[0]);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory64(addressReg, data->getNativeSseReg(data->currentOp->reg), 0, true);
    data->releaseTmpReg(addressReg);
}

void opMovmskpdR32Xmm(Armv8btAsm* data) {
    // cpu->reg[reg].u32 = (cpu->xmm[rm].pd.u64[0] >> 63) | ((cpu->xmm[rm].pd.u64[1] >> 63) << 1)
    U8 vTmpReg = data->vGetTmpReg();
    U8 tmpReg = data->getTmpReg();         

    data->vShiftRightValue(vTmpReg, data->getNativeSseReg(data->currentOp->rm), 63, D2);
    data->vMovToGeneralReg32ZeroExtend(data->getNativeReg(data->currentOp->reg), vTmpReg, 0, S4);
    data->vMovToGeneralReg32ZeroExtend(tmpReg, vTmpReg, 2, S4);
    data->shiftRegLeftWithValue32(tmpReg, tmpReg, 1);
    data->orRegs32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), tmpReg);
    
    data->releaseTmpReg(tmpReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opMovdXmmR32(Armv8btAsm* data) {
    // couldn't see a way to do this in 1 instruction, "mov s0, w0" is not valid
    data->vMovFromGeneralReg32(data->getNativeSseReg(data->currentOp->reg), 0, data->getNativeReg(data->currentOp->rm));
    data->vMov32ToScaler(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), 0);
}
void opMovdXmmE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vReadMemory32(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovdR32Xmm(Armv8btAsm* data) {
    data->vMovToGeneralReg32ZeroExtend(data->getNativeReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), 0, S4);
}
void opMovdE32Xmm(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory32(addressReg, data->getNativeSseReg(data->currentOp->reg), 0, true);
    data->releaseTmpReg(addressReg);
}
void opMovdqaXmmXmm(Armv8btAsm* data) {
    data->vMov128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm));
}
void opMovdqaXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovdqaE128Xmm(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovdquXmmXmm(Armv8btAsm* data) {
    data->vMov128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm));
}
void opMovdquXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovdquE128Xmm(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovntpdE128Xmm(Armv8btAsm* data) {
    // writeq(address, cpu->xmm[reg].pd.u64[0]);
    // writeq(address + 8, cpu->xmm[reg].pd.u64[1]);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovntdqE128Xmm(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory128(addressReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovntiE32R32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->writeMemory(addressReg, data->getNativeReg(data->currentOp->reg), 32, true);
    data->releaseTmpReg(addressReg);
}
void opMaskmovdquE128XmmXmm(Armv8btAsm* data) {
    // maskmovdqu xmm1, xmm2
    // this will mov xmm1[i] to DS:EDI[i] if (xmm2[i] & 80)
    U8 addressReg = data->getTmpReg();
    bool needToReleaseAddressReg = true;

    if (data->currentOp->ea16) {
        data->movRegToReg(addressReg, xEDI, 16, true);
        if (data->cpu->thread->process->hasSetSeg[DS]) {
            data->addRegs32(addressReg, addressReg, xDS);
        }
    } else {
        if (data->cpu->thread->process->hasSetSeg[DS]) {
            data->addRegs32(addressReg, xEDI, xDS);
        } else {
            data->releaseTmpReg(addressReg);
            addressReg = xEDI;
            needToReleaseAddressReg = false;
        }
    }
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpRegMask = data->vGetTmpReg();

    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSignedShiftRightValue(vTmpRegMask, data->getNativeSseReg(data->currentOp->rm), 7, B16);
    data->vSelectBit(vTmpRegMask, data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->vWriteMemory128(addressReg, vTmpRegMask, true);

    if (needToReleaseAddressReg) {
        data->releaseTmpReg(addressReg);
    }
    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(vTmpRegMask);
}

void opUnpckhpdXmmXmm(Armv8btAsm* data) {
    data->vZipFromHigh128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
}
void opUnpckhpdXmmE128(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    data->vZipFromHigh128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), tmpReg, D2);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}
void opUnpcklpdXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.u64[0] = cpu->xmm[reg].pd.u64[0];
    // cpu->xmm[reg].pd.u64[1] = cpu->xmm[rm].pd.u64[0];
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 1, data->getNativeSseReg(data->currentOp->rm), 0);
}
void opUnpcklpdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, data->getNativeSseReg(data->currentOp->reg), 1, true);
    data->releaseTmpReg(addressReg);
}
void opPunpckhbwXmmXmm(Armv8btAsm* data) {
    data->vZipFromHigh128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPunpckhbwXmmE128(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    data->vZipFromHigh128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), tmpReg, B16);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}
void opPunpckhwdXmmXmm(Armv8btAsm* data) {
    data->vZipFromHigh128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPunpckhwdXmmE128(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    data->vZipFromHigh128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), tmpReg, H8);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}
void opPunpckhdqXmmXmm(Armv8btAsm* data) {
    data->vZipFromHigh128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opPunpckhdqXmmE128(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    data->vZipFromHigh128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), tmpReg, S4);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}
void opPunpckhqdqXmmXmm(Armv8btAsm* data) {
    data->vZipFromHigh128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
}
void opPunpckhqdqXmmE128(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    data->vZipFromHigh128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), tmpReg, D2);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}
void opPunpcklbwXmmXmm(Armv8btAsm* data) {
    data->vZipFromLow128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), B16);
}
void opPunpcklbwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vZipFromLow128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPunpcklwdXmmXmm(Armv8btAsm* data) {
    data->vZipFromLow128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
}
void opPunpcklwdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vZipFromLow128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPunpckldqXmmXmm(Armv8btAsm* data) {
    data->vZipFromLow128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
}
void opPunpckldqXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vZipFromLow128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPunpcklqdqXmmXmm(Armv8btAsm* data) {
    data->vZipFromLow128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
}
void opPunpcklqdqXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vZipFromLow128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPackssdwXmmXmm(Armv8btAsm* data) {
    if (data->currentOp->reg == data->currentOp->rm) {
        U8 vTmpReg = data->vGetTmpReg();
        data->vSignedSaturateToSignedNarrowToLowerAndClear(vTmpReg, data->getNativeSseReg(data->currentOp->reg), S4);
        data->vSignedSaturateToSignedNarrowToUpperAndKeep(vTmpReg, data->getNativeSseReg(data->currentOp->rm), S4);
        data->vMov128(data->getNativeSseReg(data->currentOp->reg), vTmpReg);
        data->vReleaseTmpReg(vTmpReg);
    } else {
        data->vSignedSaturateToSignedNarrowToLowerAndClear(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), S4);
        data->vSignedSaturateToSignedNarrowToUpperAndKeep(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
    }
}
void opPackssdwXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSignedSaturateToSignedNarrowToLowerAndClear(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), S4);
    data->vSignedSaturateToSignedNarrowToUpperAndKeep(data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPacksswbXmmXmm(Armv8btAsm* data) {
    if (data->currentOp->reg == data->currentOp->rm) {
        U8 vTmpReg = data->vGetTmpReg();
        data->vSignedSaturateToSignedNarrowToLowerAndClear(vTmpReg, data->getNativeSseReg(data->currentOp->reg), H8);
        data->vSignedSaturateToSignedNarrowToUpperAndKeep(vTmpReg, data->getNativeSseReg(data->currentOp->rm), H8);
        data->vMov128(data->getNativeSseReg(data->currentOp->reg), vTmpReg);
        data->vReleaseTmpReg(vTmpReg);
    } else {
        data->vSignedSaturateToSignedNarrowToLowerAndClear(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), H8);
        data->vSignedSaturateToSignedNarrowToUpperAndKeep(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
    }
}
void opPacksswbXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSignedSaturateToSignedNarrowToLowerAndClear(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), H8);
    data->vSignedSaturateToSignedNarrowToUpperAndKeep(data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPackuswbXmmXmm(Armv8btAsm* data) {
    if (data->currentOp->reg == data->currentOp->rm) {
        U8 vTmpReg = data->vGetTmpReg();
        data->vSignedSaturateToUnsignedNarrowToLowerAndClear(vTmpReg, data->getNativeSseReg(data->currentOp->reg), H8);
        data->vSignedSaturateToUnsignedNarrowToUpperAndKeep(vTmpReg, data->getNativeSseReg(data->currentOp->rm), H8);
        data->vMov128(data->getNativeSseReg(data->currentOp->reg), vTmpReg);
        data->vReleaseTmpReg(vTmpReg);
    } else {
        data->vSignedSaturateToUnsignedNarrowToLowerAndClear(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), H8);
        data->vSignedSaturateToUnsignedNarrowToUpperAndKeep(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), H8);
    }
}
void opPackuswbXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    data->vSignedSaturateToUnsignedNarrowToLowerAndClear(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), H8);
    data->vSignedSaturateToUnsignedNarrowToUpperAndKeep(data->getNativeSseReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opPause(Armv8btAsm* data) {
    data->yield();
}

void opNone(Armv8btAsm* data) {
    data->invalidOp(data->currentOp->originalOp);
}

void opCallback(Armv8btAsm* data) {
    //kpanic("Need to test");
    // typedef void (OPCALL *OpCallback)(CPU* cpu, currentOp* op);
    data->syncRegsFromHost();

    data->mov64(0, xCPU); // param 1 (CPU)
    data->loadConst(1, 0); // param 2 (op)

    data->callHost((void*)data->currentOp->pfn);
    data->syncRegsToHost();
    data->doJmp(true);
    data->done = true;
}

void opDone(Armv8btAsm* data) {
    kpanic("armv8btOps.opDone should not have been called");
}

void opCustom1(Armv8btAsm* data) {
    // used by the normal core for the firstOp JIT tracking
    kpanic("armv8btOps.opCustom1 should not have been called");
}

Armv8btOp armv8btEncoder[InstructionCount] = {
    opAddR8E8,
    opAddE8R8,
    opAddR8R8,
    opAddR8I8,
    opAddE8I8,
    opAddR16E16,
    opAddE16R16,
    opAddR16R16,
    opAddR16I16,
    opAddE16I16,
    opAddR32E32,
    opAddE32R32,
    opAddR32R32,
    opAddR32I32,
    opAddE32I32,

    opOrR8E8,
    opOrE8R8,
    opOrR8R8,
    opOrR8I8,
    opOrE8I8,
    opOrR16E16,
    opOrE16R16,
    opOrR16R16,
    opOrR16I16,
    opOrE16I16,
    opOrR32E32,
    opOrE32R32,
    opOrR32R32,
    opOrR32I32,
    opOrE32I32,

    opAdcR8E8,
    opAdcE8R8,
    opAdcR8R8,
    opAdcR8I8,
    opAdcE8I8,
    opAdcR16E16,
    opAdcE16R16,
    opAdcR16R16,
    opAdcR16I16,
    opAdcE16I16,
    opAdcR32E32,
    opAdcE32R32,
    opAdcR32R32,
    opAdcR32I32,
    opAdcE32I32,

    opSbbR8E8,
    opSbbE8R8,
    opSbbR8R8,
    opSbbR8I8,
    opSbbE8I8,
    opSbbR16E16,
    opSbbE16R16,
    opSbbR16R16,
    opSbbR16I16,
    opSbbE16I16,
    opSbbR32E32,
    opSbbE32R32,
    opSbbR32R32,
    opSbbR32I32,
    opSbbE32I32,

    opAndR8E8,
    opAndE8R8,
    opAndR8R8,
    opAndR8I8,
    opAndE8I8,
    opAndR16E16,
    opAndE16R16,
    opAndR16R16,
    opAndR16I16,
    opAndE16I16,
    opAndR32E32,
    opAndE32R32,
    opAndR32R32,
    opAndR32I32,
    opAndE32I32,

    opSubR8E8,
    opSubE8R8,
    opSubR8R8,
    opSubR8I8,
    opSubE8I8,
    opSubR16E16,
    opSubE16R16,
    opSubR16R16,
    opSubR16I16,
    opSubE16I16,
    opSubR32E32,
    opSubE32R32,
    opSubR32R32,
    opSubR32I32,
    opSubE32I32,

    opXorR8E8,
    opXorE8R8,
    opXorR8R8,
    opXorR8I8,
    opXorE8I8,
    opXorR16E16,
    opXorE16R16,
    opXorR16R16,
    opXorR16I16,
    opXorE16I16,
    opXorR32E32,
    opXorE32R32,
    opXorR32R32,
    opXorR32I32,
    opXorE32I32,

    opCmpR8E8,
    opCmpE8R8,
    opCmpR8R8,
    opCmpR8I8,
    opCmpE8I8,
    opCmpR16E16,
    opCmpE16R16,
    opCmpR16R16,
    opCmpR16I16,
    opCmpE16I16,
    opCmpR32E32,
    opCmpE32R32,
    opCmpR32R32,
    opCmpR32I32,
    opCmpE32I32,

    opTestE8R8,
    opTestR8R8,
    opTestR8I8,
    opTestE8I8,
    opTestE16R16,
    opTestR16R16,
    opTestR16I16,
    opTestE16I16,
    opTestE32R32,
    opTestR32R32,
    opTestR32I32,
    opTestE32I32,

    opNotR8,
    opNotE8,
    opNotR16,
    opNotE16,
    opNotR32,
    opNotE32,

    opNegR8,
    opNegE8,
    opNegR16,
    opNegE16,
    opNegR32,
    opNegE32,

    opMulR8,
    opMulE8,
    opMulR16,
    opMulE16,
    opMulR32,
    opMulE32,

    opIMulR8,
    opIMulE8,
    opIMulR16,
    opIMulE16,
    opIMulR32,
    opIMulE32,

    opDivR8,
    opDivE8,
    opDivR16,
    opDivE16,
    opDivR32,
    opDivE32,

    opIDivR8,
    opIDivE8,
    opIDivR16,
    opIDivE16,
    opIDivR32,
    opIDivE32,

    opXchgR8R8,
    opXchgE8R8,
    opXchgR16R16,
    opXchgE16R16,
    opXchgR32R32,
    opXchgE32R32,

    opBtR16R16,
    opBtE16R16,
    opBtR32R32,
    opBtE32R32,

    opBtsR16R16,
    opBtsE16R16,
    opBtsR32R32,
    opBtsE32R32,

    opBtrR16R16,
    opBtrE16R16,
    opBtrR32R32,
    opBtrE32R32,

    opBsfR16R16,
    opBsfR16E16,
    opBsfR32R32,
    opBsfR32E32,

    opBsrR16R16,
    opBsrR16E16,
    opBsrR32R32,
    opBsrR32E32,

    opBtcR16R16,
    opBtcE16R16,
    opBtcR32R32,
    opBtcE32R32,

    opBtR16,
    opBtE16,
    opBtsR16,
    opBtsE16,
    opBtrR16,
    opBtrE16,
    opBtcR16,
    opBtcE16,

    opBtR32,
    opBtE32,
    opBtsR32,
    opBtsE32,
    opBtrR32,
    opBtrE32,
    opBtcR32,
    opBtcE32,

    opDshlR16R16,
    opDshlE16R16,
    opDshlClR16R16,
    opDshlClE16R16,
    opDshrR16R16,
    opDshrE16R16,
    opDshrClR16R16,
    opDshrClE16R16,

    opDshlR32R32,
    opDshlE32R32,
    opDshlClR32R32,
    opDshlClE32R32,
    opDshrR32R32,
    opDshrE32R32,
    opDshrClR32R32,
    opDshrClE32R32,

    opDimulR16R16,
    opDimulR16E16,
    opDimulR32R32,
    opDimulR32E32,

    opCmpXchgR8R8,
    opCmpXchgE8R8,
    opCmpXchgR16R16,
    opCmpXchgE16R16,
    opCmpXchgR32R32,
    opCmpXchgE32R32,

    opIncR8,
    opIncR16,
    opIncR32,
    opIncE8,
    opIncE16,
    opIncE32,

    opDecR8,
    opDecR16,
    opDecR32,
    opDecE8,
    opDecE16,
    opDecE32,

    opPushSeg16,
    opPopSeg16,
    opPushSeg32,
    opPopSeg32,

    opPushR16,
    opPushR32,
    opPushE16,
    opPushE32,

    opPopR16,
    opPopR32,
    opPopE16,
    opPopE32,

    opPushA16,
    opPushA32,
    opPopA16,
    opPopA32,

    opPush16,
    opPush32,

    opPushF16,
    opPushF32,
    opPopF16,
    opPopF32,

    opBound16,
    opBound32,

    opArplReg,
    opArplMem,
    opArplReg32,
    opArplMem32,

    opDaa,
    opDas,
    opAaa,
    opAas,
    opAam,
    opAad,

    opImulR16E16,
    opImulR16R16,
    opImulR32E32,
    opImulR32R32,

    opInsb,
    opInsw,
    opInsd,

    opOutsb,
    opOutsw,
    opOutsd,

    opJumpO,
    opJumpNO,
    opJumpB,
    opJumpNB,
    opJumpZ,
    opJumpNZ,
    opJumpBE,
    opJumpNBE,
    opJumpS,
    opJumpNS,
    opJumpP,
    opJumpNP,
    opJumpL,
    opJumpNL,
    opJumpLE,
    opJumpNLE,

    opMovR8R8,
    opMovE8R8,
    opMovR8E8,
    opMovR8I8,
    opMovE8I8,
    opMovR16R16,
    opMovE16R16,
    opMovR16E16,
    opMovR16I16,
    opMovE16I16,
    opMovR32R32,
    opMovE32R32,
    opMovR32E32,
    opMovR32I32,
    opMovE32I32,

    opMovR16S16,
    opMovR32S16,
    opMovE16S16,
    opMovS16R16,
    opMovS16E16,

    opMovAlOb,
    opMovAxOw,
    opMovEaxOd,
    opMovObAl,
    opMovOwAx,
    opMovOdEax,

    opMovGwXzR8,
    opMovGwXzE8,
    opMovGwSxR8,
    opMovGwSxE8,

    opMovGdXzR8,
    opMovGdXzE8,
    opMovGdSxR8,
    opMovGdSxE8,

    opMovGdXzR16,
    opMovGdXzE16,
    opMovGdSxR16,
    opMovGdSxE16,

    opMovRdCRx,
    opMovCRxRd,

    opLeaR16,
    opLeaR32,

    opNop,
    opCwd,
    opCwq,
    opCallAp,
    opCallFar,
    opJmpAp,
    opJmpFar,
    opWait,
    opSahf,
    opLahf,
    opSalc,
    opRetn16Iw,
    opRetn32Iw,
    opRetn16,
    opRetn32,
    opRetf16,
    opRetf32,
    opInvalid,
    opInt3,
    opInt80,
    opInt98,
    opInt99,
    opInt9A,
    opIntIb,
    opIntO,
    opIret,
    opIret32,
    opXlat,
    opICEBP,
    opHlt,
    opCmc,
    opClc,
    opStc,
    opCli,
    opSti,
    opCld,
    opStd,
    opRdtsc,
    opCPUID,

    opEnter16,
    opEnter32,
    opLeave16,
    opLeave32,

    opLoadSegment16,
    opLoadSegment32,

    opMovsb,
    opMovsw,
    opMovsd,
    opCmpsb,
    opCmpsw,
    opCmpsd,
    opStosb,
    opStosw,
    opStosd,
    opLodsb,
    opLodsw,
    opLodsd,
    opScasb,
    opScasw,
    opScasd,

    opRolR8I8,
    opRolE8I8,
    opRorR8I8,
    opRorE8I8,
    opRclR8I8,
    opRclE8I8,
    opRcrR8I8,
    opRcrE8I8,
    opShlR8I8,
    opShlE8I8,
    opShrR8I8,
    opShrE8I8,
    opSarR8I8,
    opSarE8I8,

    opRolR16I8,
    opRolE16I8,
    opRorR16I8,
    opRorE16I8,
    opRclR16I8,
    opRclE16I8,
    opRcrR16I8,
    opRcrE16I8,
    opShlR16I8,
    opShlE16I8,
    opShrR16I8,
    opShrE16I8,
    opSarR16I8,
    opSarE16I8,

    opRolR32I8,
    opRolE32I8,
    opRorR32I8,
    opRorE32I8,
    opRclR32I8,
    opRclE32I8,
    opRcrR32I8,
    opRcrE32I8,
    opShlR32I8,
    opShlE32I8,
    opShrR32I8,
    opShrE32I8,
    opSarR32I8,
    opSarE32I8,

    opRolR8Cl,
    opRolE8Cl,
    opRorR8Cl,
    opRorE8Cl,
    opRclR8Cl,
    opRclE8Cl,
    opRcrR8Cl,
    opRcrE8Cl,
    opShlR8Cl,
    opShlE8Cl,
    opShrR8Cl,
    opShrE8Cl,
    opSarR8Cl,
    opSarE8Cl,

    opRolR16Cl,
    opRolE16Cl,
    opRorR16Cl,
    opRorE16Cl,
    opRclR16Cl,
    opRclE16Cl,
    opRcrR16Cl,
    opRcrE16Cl,
    opShlR16Cl,
    opShlE16Cl,
    opShrR16Cl,
    opShrE16Cl,
    opSarR16Cl,
    opSarE16Cl,

    opRolR32Cl,
    opRolE32Cl,
    opRorR32Cl,
    opRorE32Cl,
    opRclR32Cl,
    opRclE32Cl,
    opRcrR32Cl,
    opRcrE32Cl,
    opShlR32Cl,
    opShlE32Cl,
    opShrR32Cl,
    opShrE32Cl,
    opSarR32Cl,
    opSarE32Cl,

    opFADD_ST0_STj,
    opFMUL_ST0_STj,
    opFCOM_STi,
    opFCOM_STi_Pop,
    opFSUB_ST0_STj,
    opFSUBR_ST0_STj,
    opFDIV_ST0_STj,
    opFDIVR_ST0_STj,
    opFADD_SINGLE_REAL,
    opFMUL_SINGLE_REAL,
    opFCOM_SINGLE_REAL,
    opFCOM_SINGLE_REAL_Pop,
    opFSUB_SINGLE_REAL,
    opFSUBR_SINGLE_REAL,
    opFDIV_SINGLE_REAL,
    opFDIVR_SINGLE_REAL,

    opFLD_STi,
    opFXCH_STi,
    opFNOP,
    opFST_STi_Pop,
    opFCHS,
    opFABS,
    opFTST,
    opFXAM,
    opFLD1,
    opFLDL2T,
    opFLDL2E,
    opFLDPI,
    opFLDLG2,
    opFLDLN2,
    opFLDZ,
    opF2XM1,
    opFYL2X,
    opFPTAN,
    opFPATAN,
    opFXTRACT,
    opFPREM_nearest,
    opFDECSTP,
    opFINCSTP,
    opFPREM,
    opFYL2XP1,
    opFSQRT,
    opFSINCOS,
    opFRNDINT,
    opFSCALE,
    opFSIN,
    opFCOS,
    opFLD_SINGLE_REAL,
    opFST_SINGLE_REAL,
    opFST_SINGLE_REAL_Pop,
    opFLDENV,
    opFLDCW,
    opFNSTENV,
    opFNSTCW,

    opFCMOV_ST0_STj_CF,
    opFCMOV_ST0_STj_ZF,
    opFCMOV_ST0_STj_CF_OR_ZF,
    opFCMOV_ST0_STj_PF,
    opFUCOMPP,
    opFIADD_DWORD_INTEGER,
    opFIMUL_DWORD_INTEGER,
    opFICOM_DWORD_INTEGER,
    opFICOM_DWORD_INTEGER_Pop,
    opFISUB_DWORD_INTEGER,
    opFISUBR_DWORD_INTEGER,
    opFIDIV_DWORD_INTEGER,
    opFIDIVR_DWORD_INTEGER,

    opFCMOV_ST0_STj_NCF,
    opFCMOV_ST0_STj_NZF,
    opFCMOV_ST0_STj_NCF_AND_NZF,
    opFCMOV_ST0_STj_NPF,
    opFNCLEX,
    opFNINIT,
    opFUCOMI_ST0_STj,
    opFCOMI_ST0_STj,
    opFILD_DWORD_INTEGER,
    opFISTTP32,
    opFIST_DWORD_INTEGER,
    opFIST_DWORD_INTEGER_Pop,
    opFLD_EXTENDED_REAL,
    opFSTP_EXTENDED_REAL,

    opFADD_STi_ST0,
    opFMUL_STi_ST0,
    opFSUBR_STi_ST0,
    opFSUB_STi_ST0,
    opFDIVR_STi_ST0,
    opFDIV_STi_ST0,
    opFADD_DOUBLE_REAL,
    opFMUL_DOUBLE_REAL,
    opFCOM_DOUBLE_REAL,
    opFCOM_DOUBLE_REAL_Pop,
    opFSUB_DOUBLE_REAL,
    opFSUBR_DOUBLE_REAL,
    opFDIV_DOUBLE_REAL,
    opFDIVR_DOUBLE_REAL,

    opFFREE_STi,
    opFST_STi,
    opFUCOM_STi,
    opFUCOM_STi_Pop,
    opFLD_DOUBLE_REAL,
    opFISTTP64,
    opFST_DOUBLE_REAL,
    opFST_DOUBLE_REAL_Pop,
    opFRSTOR,
    opFNSAVE,
    opFNSTSW,

    opFADD_STi_ST0_Pop,
    opFMUL_STi_ST0_Pop,
    opFCOMPP,
    opFSUBR_STi_ST0_Pop,
    opFSUB_STi_ST0_Pop,
    opFDIVR_STi_ST0_Pop,
    opFDIV_STi_ST0_Pop,
    opFIADD_WORD_INTEGER,
    opFIMUL_WORD_INTEGER,
    opFICOM_WORD_INTEGER,
    opFICOM_WORD_INTEGER_Pop,
    opFISUB_WORD_INTEGER,
    opFISUBR_WORD_INTEGER,
    opFIDIV_WORD_INTEGER,
    opFIDIVR_WORD_INTEGER,

    opFFREEP_STi,
    opFNSTSW_AX,
    opFUCOMI_ST0_STj_Pop,
    opFCOMI_ST0_STj_Pop,
    opFILD_WORD_INTEGER,
    opFISTTP16,
    opFIST_WORD_INTEGER,
    opFIST_WORD_INTEGER_Pop,
    opFBLD_PACKED_BCD,
    opFILD_QWORD_INTEGER,
    opFBSTP_PACKED_BCD,
    opFISTP_QWORD_INTEGER,

    opLoopNZ,
    opLoopZ,
    opLoop,
    opJcxz,

    opInAlIb,
    opInAxIb,
    opInEaxIb,
    opOutIbAl,
    opOutIbAx,
    opOutIbEax,
    opInAlDx,
    opInAxDx,
    opInEaxDx,
    opOutDxAl,
    opOutDxAx,
    opOutDxEax,

    opCallJw,
    opCallJd,
    opJmpJw,
    opJmpJd,
    opJmpJb,
    opCallR16,
    opCallR32,
    opCallE16,
    opCallE32,
    opCallFarE16,
    opCallFarE32,
    opJmpR16,
    opJmpR32,
    opJmpE16,
    opJmpE32,
    opJmpFarE16,
    opJmpFarE32,

    opLarR16R16,
    opLarR16E16,
    opLslR16R16,
    opLslR16E16,
    opLslR32R32,
    opLslR32E32,

    opCmovO_R16R16,
    opCmovO_R16E16,
    opCmovNO_R16R16,
    opCmovNO_R16E16,
    opCmovB_R16R16,
    opCmovB_R16E16,
    opCmovNB_R16R16,
    opCmovNB_R16E16,
    opCmovZ_R16R16,
    opCmovZ_R16E16,
    opCmovNZ_R16R16,
    opCmovNZ_R16E16,
    opCmovBE_R16R16,
    opCmovBE_R16E16,
    opCmovNBE_R16R16,
    opCmovNBE_R16E16,
    opCmovS_R16R16,
    opCmovS_R16E16,
    opCmovNS_R16R16,
    opCmovNS_R16E16,
    opCmovP_R16R16,
    opCmovP_R16E16,
    opCmovNP_R16R16,
    opCmovNP_R16E16,
    opCmovL_R16R16,
    opCmovL_R16E16,
    opCmovNL_R16R16,
    opCmovNL_R16E16,
    opCmovLE_R16R16,
    opCmovLE_R16E16,
    opCmovNLE_R16R16,
    opCmovNLE_R16E16,

    opCmovO_R32R32,
    opCmovO_R32E32,
    opCmovNO_R32R32,
    opCmovNO_R32E32,
    opCmovB_R32R32,
    opCmovB_R32E32,
    opCmovNB_R32R32,
    opCmovNB_R32E32,
    opCmovZ_R32R32,
    opCmovZ_R32E32,
    opCmovNZ_R32R32,
    opCmovNZ_R32E32,
    opCmovBE_R32R32,
    opCmovBE_R32E32,
    opCmovNBE_R32R32,
    opCmovNBE_R32E32,
    opCmovS_R32R32,
    opCmovS_R32E32,
    opCmovNS_R32R32,
    opCmovNS_R32E32,
    opCmovP_R32R32,
    opCmovP_R32E32,
    opCmovNP_R32R32,
    opCmovNP_R32E32,
    opCmovL_R32R32,
    opCmovL_R32E32,
    opCmovNL_R32R32,
    opCmovNL_R32E32,
    opCmovLE_R32R32,
    opCmovLE_R32E32,
    opCmovNLE_R32R32,
    opCmovNLE_R32E32,

    opSetO_R8,
    opSetO_E8,
    opSetNO_R8,
    opSetNO_E8,
    opSetB_R8,
    opSetB_E8,
    opSetNB_R8,
    opSetNB_E8,
    opSetZ_R8,
    opSetZ_E8,
    opSetNZ_R8,
    opSetNZ_E8,
    opSetBE_R8,
    opSetBE_E8,
    opSetNBE_R8,
    opSetNBE_E8,
    opSetS_R8,
    opSetS_E8,
    opSetNS_R8,
    opSetNS_E8,
    opSetP_R8,
    opSetP_E8,
    opSetNP_R8,
    opSetNP_E8,
    opSetL_R8,
    opSetL_E8,
    opSetNL_R8,
    opSetNL_E8,
    opSetLE_R8,
    opSetLE_E8,
    opSetNLE_R8,
    opSetNLE_E8,

    opSLDTReg,
    opSLDTE16,
    opSTRReg,
    opSTRE16,
    opLLDTR16,
    opLLDTE16,
    opLTRR16,
    opLTRE16,
    opVERRR16,
    opVERRE16,
    opVERWR16,
    opVERWE16,

    opSGDT,
    opSIDT,
    opLGDT,
    opLIDT,
    opSMSWRreg,
    opSMSW,
    opLMSWRreg,
    opLMSW,
    opINVLPG,

    opXaddR8R8,
    opXaddR8E8,
    opXaddR16R16,
    opXaddR16E16,
    opXaddR32R32,
    opXaddR32E32,
    opCmpXchg8b,
    opBswap32,

    opPunpcklbwMmx,
    opPunpcklbwE64,
    opPunpcklwdMmx,
    opPunpcklwdE64,
    opPunpckldqMmx,
    opPunpckldqE64,
    opPacksswbMmx,
    opPacksswbE64,
    opPcmpgtbMmx,
    opPcmpgtbE64,
    opPcmpgtwMmx,
    opPcmpgtwE64,
    opPcmpgtdMmx,
    opPcmpgtdE64,
    opPackuswbMmx,
    opPackuswbE64,
    opPunpckhbwMmx,
    opPunpckhbwE64,
    opPunpckhwdMmx,
    opPunpckhwdE64,
    opPunpckhdqMmx,
    opPunpckhdqE64,
    opPackssdwMmx,
    opPackssdwE64,
    opMovPqR32,
    opMovPqE32,
    opMovPqMmx,
    opMovPqE64,
    opPsrlw,
    opPsraw,
    opPsllw,
    opPsrld,
    opPsrad,
    opPslld,
    opPsrlq,
    opPsllq,
    opPcmpeqbMmx,
    opPcmpeqbE64,
    opPcmpeqwMmx,
    opPcmpeqwE64,
    opPcmpeqdMmx,
    opPcmpeqdE64,
    opEmms,
    opMovR32Pq,
    opMovE32Pq,
    opMovMmxPq,
    opMovE64Pq,
    opPsrlwMmx,
    opPsrlwE64,
    opPsrldMmx,
    opPsrldE64,
    opPsrlqMmx,
    opPsrlqE64,
    opPmullwMmx,
    opPmullwE64,
    opPsubusbMmx,
    opPsubusbE64,
    opPsubuswMmx,
    opPsubuswE64,
    opPandMmx,
    opPandE64,
    opPaddusbMmx,
    opPaddusbE64,
    opPadduswMmx,
    opPadduswE64,
    opPandnMmx,
    opPandnE64,
    opPsrawMmx,
    opPsrawE64,
    opPsradMmx,
    opPsradE64,
    opPmulhwMmx,
    opPmulhwE64,
    opPsubsbMmx,
    opPsubsbE64,
    opPsubswMmx,
    opPsubswE64,
    opPorMmx,
    opPorE64,
    opPaddsbMmx,
    opPaddsbE64,
    opPaddswMmx,
    opPaddswE64,
    opPxorMmx,
    opPxorE64,
    opPsllwMmx,
    opPsllwE64,
    opPslldMmx,
    opPslldE64,
    opPsllqMmx,
    opPsllqE64,
    opPmaddwdMmx,
    opPmaddwdE64,
    opPsubbMmx,
    opPsubbE64,
    opPsubwMmx,
    opPsubwE64,
    opPsubdMmx,
    opPsubdE64,
    opPaddbMmx,
    opPaddbE64,
    opPaddwMmx,
    opPaddwE64,
    opPadddMmx,
    opPadddE64,

    opFxsave, // P2+
    opFxrstor, // P2+
    opLdmxcsr, // P3+ SSE1
    opStmxcsr, // P3+ SSE1
    opXsave, // Core 2+
    opLfence, // P4+ SSE2
    opXrstor, // Core 2+
    opMfence, // P4+ SSE2
    opSfence, // P3+ SSE1
    opClflush, // P4+ SSE2

    // SSE1
    opAddpsXmm,
    opAddpsE128,
    opAddssXmm,
    opAddssE32,
    opSubpsXmm,
    opSubpsE128,
    opSubssXmm,
    opSubssE32,
    opMulpsXmm,
    opMulpsE128,
    opMulssXmm,
    opMulssE32,
    opDivpsXmm,
    opDivpsE128,
    opDivssXmm,
    opDivssE32,
    opRcppsXmm,
    opRcppsE128,
    opRcpssXmm,
    opRcpssE32,
    opSqrtpsXmm,
    opSqrtpsE128,
    opSqrtssXmm,
    opSqrtssE32,
    opRsqrtpsXmm,
    opRsqrtpsE128,
    opRsqrtssXmm,
    opRsqrtssE32,
    opMaxpsXmm,
    opMaxpsE128,
    opMaxssXmm,
    opMaxssE32,
    opMinpsXmm,
    opMinpsE128,
    opMinssXmm,
    opMinssE32,
    opPavgbMmxMmx,
    opPavgbMmxE64,
    opPavgbXmmXmm,
    opPavgbXmmE128,
    opPavgwMmxMmx,
    opPavgwMmxE64,
    opPavgwXmmXmm,
    opPavgwXmmE128,
    opPsadbwMmxMmx,
    opPsadbwMmxE64,
    opPsadbwXmmXmm,
    opPsadbwXmmE128,
    opPextrwR32Mmx,
    opPextrwE16Mmx,
    opPextrwR32Xmm,
    opPextrwE16Xmm,
    opPinsrwMmxR32,
    opPinsrwMmxE16,
    opPinsrwXmmR32,
    opPinsrwXmmE16,
    opPmaxswMmxMmx,
    opPmaxswMmxE64,
    opPmaxswXmmXmm,
    opPmaxswXmmE128,
    opPmaxubMmxMmx,
    opPmaxubMmxE64,
    opPmaxubXmmXmm,
    opPmaxubXmmE128,
    opPminswMmxMmx,
    opPminswMmxE64,
    opPminswXmmXmm,
    opPminswXmmE128,
    opPminubMmxMmx,
    opPminubMmxE64,
    opPminubXmmXmm,
    opPminubXmmE128,
    opPmovmskbR32Mmx,
    opPmovmskbR32Xmm,
    opPmulhuwMmxMmx,
    opPmulhuwMmxE64,
    opPmulhuwXmmXmm,
    opPmulhuwXmmE128,
    opPshufwMmxMmx,
    opPshufwMmxE64,
    opAndnpsXmmXmm,
    opAndnpsXmmE128,
    opAndpsXmmXmm,
    opAndpsXmmE128,
    opOrpsXmmXmm,
    opOrpsXmmE128,
    opXorpsXmmXmm,
    opXorpsXmmE128,
    opCvtpi2psXmmMmx,
    opCvtpi2psXmmE64,
    opCvtps2piMmxXmm,
    opCvtps2piMmxE64,
    opCvtsi2ssXmmR32,
    opCvtsi2ssXmmE32,
    opCvtss2siR32Xmm,
    opCvtss2siR32E32,
    opCvttps2piMmxXmm,
    opCvttps2piMmxE64,
    opCvttss2siR32Xmm,
    opCvttss2siR32E32,
    opMovapsXmmXmm,
    opMovapsXmmE128,
    opMovapsE128Xmm,
    opMovhlpsXmmXmm,
    opMovlhpsXmmXmm,
    opMovhpsXmmE64,
    opMovhpsE64Xmm,
    opMovlpsXmmE64,
    opMovlpsE64Xmm,
    opMovmskpsR32Xmm,
    opMovssXmmXmm,
    opMovssXmmE32,
    opMovssE32Xmm,
    opMovupsXmmXmm,
    opMovupsXmmE128,
    opMovupsE128Xmm,
    opMaskmovqEDIMmxMmx,
    opMovntpsE128Xmm,
    opMovntqE64Mmx,
    opShufpsXmmXmm,
    opShufpsXmmE128,
    opUnpckhpsXmmXmm,
    opUnpckhpsXmmE128,
    opUnpcklpsXmmXmm,
    opUnpcklpsXmmE128,
    opPrefetchT0,
    opPrefetchT1,
    opPrefetchT2,
    opPrefetchNTA,
    opCmppsXmmXmm,
    opCmppsXmmE128,
    opCmpssXmmXmm,
    opCmpssXmmE32,
    opComissXmmXmm,
    opComissXmmE32,
    opUcomissXmmXmm,
    opUcomissXmmE32,

    // SSE2
    opAddpdXmmXmm,
    opAddpdXmmE128,
    opAddsdXmmXmm,
    opAddsdXmmE64,
    opSubpdXmmXmm,
    opSubpdXmmE128,
    opSubsdXmmXmm,
    opSubsdXmmE64,
    opMulpdXmmXmm,
    opMulpdXmmE128,
    opMulsdXmmXmm,
    opMulsdXmmE64,
    opDivpdXmmXmm,
    opDivpdXmmE128,
    opDivsdXmmXmm,
    opDivsdXmmE64,
    opMaxpdXmmXmm,
    opMaxpdXmmE128,
    opMaxsdXmmXmm,
    opMaxsdXmmE64,
    opMinpdXmmXmm,
    opMinpdXmmE128,
    opMinsdXmmXmm,
    opMinsdXmmE64,
    opPaddbXmmXmm,
    opPaddbXmmE128,
    opPaddwXmmXmm,
    opPaddwXmmE128,
    opPadddXmmXmm,
    opPadddXmmE128,
    opPaddqMmxMmx,
    opPaddqMmxE64,
    opPaddqXmmXmm,
    opPaddqXmmE128,
    opPaddsbXmmXmm,
    opPaddsbXmmE128,
    opPaddswXmmXmm,
    opPaddswXmmE128,
    opPaddusbXmmXmm,
    opPaddusbXmmE128,
    opPadduswXmmXmm,
    opPadduswXmmE128,
    opPsubbXmmXmm,
    opPsubbXmmE128,
    opPsubwXmmXmm,
    opPsubwXmmE128,
    opPsubdXmmXmm,
    opPsubdXmmE128,
    opPsubqMmxMmx,
    opPsubqMmxE64,
    opPsubqXmmXmm,
    opPsubqXmmE128,
    opPsubsbXmmXmm,
    opPsubsbXmmE128,
    opPsubswXmmXmm,
    opPsubswXmmE128,
    opPsubusbXmmXmm,
    opPsubusbXmmE128,
    opPsubuswXmmXmm,
    opPsubuswXmmE128,
    opPmaddwdXmmXmm,
    opPmaddwdXmmE128,
    opPmulhwXmmXmm,
    opPmulhwXmmE128,
    opPmullwXmmXmm,
    opPmullwXmmE128,
    opPmuludqMmxMmx,
    opPmuludqMmxE64,
    opPmuludqXmmXmm,
    opPmuludqXmmE128,
    opSqrtpdXmmXmm,
    opSqrtpdXmmE128,
    opSqrtsdXmmXmm,
    opSqrtsdXmmE64,
    opAndnpdXmmXmm,
    opAndnpdXmmE128,
    opAndpdXmmXmm,
    opAndpdXmmE128,
    opPandXmmXmm,
    opPandXmmE128,
    opPandnXmmXmm,
    opPandnXmmE128,
    opPorXmmXmm,
    opPorXmmXmmE128,
    opPslldqXmm,
    opPsllqXmm,
    opPsllqXmmXmm,
    opPsllqXmmE128,
    opPslldXmm,
    opPslldXmmXmm,
    opPslldXmmE128,
    opPsllwXmm,
    opPsllwXmmXmm,
    opPsllwXmmE128,
    opPsradXmm,
    opPsradXmmXmm,
    opPsradXmmE128,
    opPsrawXmm,
    opPsrawXmmXmm,
    opPsrawXmmE128,
    opPsrldqXmm,
    opPsrlqXmm,
    opPsrlqXmmXmm,
    opPsrlqXmmE128,
    opPsrldXmm,
    opPsrldXmmXmm,
    opPsrldXmmE128,
    opPsrlwXmm,
    opPsrlwXmmXmm,
    opPsrlwXmmE128,
    opPxorXmmXmm,
    opPxorXmmE128,
    opOrpdXmmXmm,
    opOrpdXmmE128,
    opXorpdXmmXmm,
    opXorpdXmmE128,
    opCmppdXmmXmm,
    opCmppdXmmE128,
    opCmpsdXmmXmm,
    opCmpsdXmmE64,
    opComisdXmmXmm,
    opComisdXmmE64,
    opUcomisdXmmXmm,
    opUcomisdXmmE64,
    opPcmpgtbXmmXmm,
    opPcmpgtbXmmE128,
    opPcmpgtwXmmXmm,
    opPcmpgtwXmmE128,
    opPcmpgtdXmmXmm,
    opPcmpgtdXmmE128,
    opPcmpeqbXmmXmm,
    opPcmpeqbXmmE128,
    opPcmpeqwXmmXmm,
    opPcmpeqwXmmE128,
    opPcmpeqdXmmXmm,
    opPcmpeqdXmmE128,
    opCvtdq2pdXmmXmm,
    opCvtdq2pdXmmE128,
    opCvtdq2psXmmXmm,
    opCvtdq2psXmmE128,
    opCvtpd2piMmxXmm,
    opCvtpd2piMmxE128,
    opCvtpd2dqXmmXmm,
    opCvtpd2dqXmmE128,
    opCvtpd2psXmmXmm,
    opCvtpd2psXmmE128,
    opCvtpi2pdXmmMmx,
    opCvtpi2pdXmmE64,
    opCvtps2dqXmmXmm,
    opCvtps2dqXmmE128,
    opCvtps2pdXmmXmm,
    opCvtps2pdXmmE64,
    opCvtsd2siR32Xmm,
    opCvtsd2siR32E64,
    opCvtsd2ssXmmXmm,
    opCvtsd2ssXmmE64,
    opCvtsi2sdXmmR32,
    opCvtsi2sdXmmE32,
    opCvtss2sdXmmXmm,
    opCvtss2sdXmmE32,
    opCvttpd2piMmxXmm,
    opCvttpd2piMmE128,
    opCvttpd2dqXmmXmm,
    opCvttpd2dqXmmE128,
    opCvttps2dqXmmXmm,
    opCvttps2dqXmmE128,
    opCvttsd2siR32Xmm,
    opCvttsd2siR32E64,
    opMovqXmmXmm,
    opMovqE64Xmm,
    opMovqXmmE64,
    opMovsdXmmXmm,
    opMovsdXmmE64,
    opMovsdE64Xmm,
    opMovapdXmmXmm,
    opMovapdXmmE128,
    opMovapdE128Xmm,
    opMovupdXmmXmm,
    opMovupdXmmE128,
    opMovupdE128Xmm,
    opMovhpdXmmE64,
    opMovhpdE64Xmm,
    opMovlpdXmmE64,
    opMovlpdE64Xmm,
    opMovmskpdR32Xmm,
    opMovdXmmR32,
    opMovdXmmE32,
    opMovdR32Xmm,
    opMovdE32Xmm,
    opMovdqaXmmXmm,
    opMovdqaXmmE128,
    opMovdqaE128Xmm,
    opMovdquXmmXmm,
    opMovdquXmmE128,
    opMovdquE128Xmm,
    opMovdq2qMmxXmm,
    opMovq2dqXmmMmx,
    opMovntpdE128Xmm,
    opMovntdqE128Xmm,
    opMovntiE32R32,
    opMaskmovdquE128XmmXmm,
    opPshufdXmmXmm,
    opPshufdXmmE128,
    opPshufhwXmmXmm,
    opPshufhwXmmE128,
    opPshuflwXmmXmm,
    opPshuflwXmmE128,
    opUnpckhpdXmmXmm,
    opUnpckhpdXmmE128,
    opUnpcklpdXmmXmm,
    opUnpcklpdXmmE128,
    opPunpckhbwXmmXmm,
    opPunpckhbwXmmE128,
    opPunpckhwdXmmXmm,
    opPunpckhwdXmmE128,
    opPunpckhdqXmmXmm,
    opPunpckhdqXmmE128,
    opPunpckhqdqXmmXmm,
    opPunpckhqdqXmmE128,
    opPunpcklbwXmmXmm,
    opPunpcklbwXmmE128,
    opPunpcklwdXmmXmm,
    opPunpcklwdXmmE128,
    opPunpckldqXmmXmm,
    opPunpckldqXmmE128,
    opPunpcklqdqXmmXmm,
    opPunpcklqdqXmmE128,
    opPackssdwXmmXmm,
    opPackssdwXmmE128,
    opPacksswbXmmXmm,
    opPacksswbXmmE128,
    opPackuswbXmmXmm,
    opPackuswbXmmE128,
    opShufpdXmmXmm,
    opShufpdXmmE128,
    opPause,

    opNone,
    opCallback,
    opDone,
    opCustom1
};

#endif
