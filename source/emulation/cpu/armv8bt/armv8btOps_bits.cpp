#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
#include "armv8btOps.h"

static void doBitTest16(Armv8btAsm* data, U8 maskReg, U8 valueReg, bool usesDone, std::function<void(U8 mask)> done) {
    // U16 mask = 1 << (cpu->reg[rm].u16 & 15);
    // cpu->fillFlagsNoCF();
    // cpu->setCF(cpu->reg[reg].u16 & mask);

    U8 tmpReg = data->getTmpReg();
    U8 tmpReg2 = data->getTmpReg();
    U32 flags = data->flagsNeeded();

    if ((flags & CF) || usesDone) {
        data->andValue32(tmpReg2, maskReg, 15);
    }
    if (flags & CF) {
        data->shiftRegRightWithReg32(tmpReg, valueReg, tmpReg2);
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, tmpReg, 0, 1, true);
    }
    data->releaseTmpReg(tmpReg);
    if (usesDone) {
        done(tmpReg2);
    }    
    data->releaseTmpReg(tmpReg2);
}

static void doBitTest32(Armv8btAsm* data, U8 maskReg, U8 valueReg, bool usesDone, std::function<void(U8 mask)> done) {
    // U32 mask = 1 << (cpu->reg[maskReg].u32 & 31);
    // cpu->fillFlagsNoCF();
    // cpu->setCF(cpu->reg[reg].u32 & mask);
    U8 tmpReg = data->getTmpReg();
    U8 tmpReg2 = data->getTmpReg();
    U32 flags = data->flagsNeeded();

    if ((flags & CF) || usesDone) {
        data->andValue32(tmpReg2, maskReg, 31);
    }
    if (flags & CF) {
        data->shiftRegRightWithReg32(tmpReg, valueReg, tmpReg2);
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, tmpReg, 0, 1, true);
    }
    data->releaseTmpReg(tmpReg);
    if (usesDone) {
        done(tmpReg2);
    }    
    data->releaseTmpReg(tmpReg2);
}

static U8 getBitAddress16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    // address += (((S16)cpu->reg[reg].u16) >> 4) * 2;
    data->signExtend(tmpReg, data->currentOp->reg, 16);
    data->shiftSignedRegRightWithValue32(tmpReg, tmpReg, 4);
    data->addRegs32(addressReg, addressReg, tmpReg, 1);
    data->releaseTmpReg(tmpReg);
    return addressReg;
}

static U8 getBitAddress32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    // address += (((S32)cpu->reg[reg].u32) >> 5) * 4;
    data->shiftSignedRegRightWithValue32(tmpReg, data->currentOp->reg, 5);
    data->addRegs32(addressReg, addressReg, tmpReg, 2);
    data->releaseTmpReg(tmpReg);
    return addressReg;
}

void doBitTestReg16(Armv8btAsm* data, std::function<void(U8 dstReg, U8 srcReg, U8 maskReg)> done) {
    doBitTest16(data, data->getNativeReg(data->currentOp->rm), data->getNativeReg(data->currentOp->reg), done!=nullptr, [data, done](U8 mask) {
        U8 tmpReg = data->getTmpReg();
        // :TODO: why isn't there an instruction that can set a single bit?
        data->loadConst(tmpReg, 1);
        data->shiftRegLeftWithReg32(tmpReg, tmpReg, mask);
        done(tmpReg, data->getNativeReg(data->currentOp->reg), tmpReg);
        data->movRegToReg(data->getNativeReg(data->currentOp->reg), tmpReg, 16, false);
        data->releaseTmpReg(tmpReg);
        });
}

void doBitTestMemory16(Armv8btAsm* data, std::function<void(U8 dstReg, U8 srcReg, U8 maskReg)> done) {
    U8 addressReg = getBitAddress16(data);
    U8 originalValueReg = data->getTmpReg();

    U8 memReg = data->getHostMem(addressReg, 16, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;

    data->readMemory(addressReg, originalValueReg, 16, false, data->currentOp->lock != 0);
    doBitTest16(data, data->getNativeReg(data->currentOp->reg), originalValueReg, done!=nullptr, [done, originalValueReg, addressReg, restartPos, data](U8 mask) {
        U8 tmpReg = data->getTmpReg();

        // :TODO: why isn't there an instruction that can set a single bit?
        data->loadConst(tmpReg, 1);
        data->shiftRegLeftWithReg32(tmpReg, tmpReg, mask);
        done(tmpReg, originalValueReg, tmpReg);

        data->writeMemory(addressReg, tmpReg, 16, false, data->currentOp->lock != 0, originalValueReg, restartPos);
        data->releaseTmpReg(tmpReg);
        });
    data->releaseTmpReg(originalValueReg);
    data->releaseTmpReg(addressReg);
}

void doBitTestReg32(Armv8btAsm* data, std::function<void(U8 dstReg, U8 srcReg, U8 maskReg)> done) {
    doBitTest32(data, data->getNativeReg(data->currentOp->rm), data->getNativeReg(data->currentOp->reg), done != nullptr, [data, done](U8 mask) {
        // cpu->reg[reg].u32 |= mask;
        U8 tmpReg = data->getTmpReg();
        // :TODO: why isn't there an instruction that can set a single bit?
        data->loadConst(tmpReg, 1);
        data->shiftRegLeftWithReg32(tmpReg, tmpReg, mask);
        done(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), tmpReg);
        data->releaseTmpReg(tmpReg);
        });
}

void doBitTestMemory32(Armv8btAsm* data, std::function<void(U8 dstReg, U8 srcReg, U8 maskReg)> done) {
    U8 addressReg = getBitAddress32(data);
    U8 originalValueReg = data->getTmpReg();

    U8 memReg = data->getHostMem(addressReg, 32, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;

    data->readMemory(addressReg, originalValueReg, 32, false, data->currentOp->lock != 0);
    doBitTest32(data, data->getNativeReg(data->currentOp->reg), originalValueReg, done != nullptr, [done, originalValueReg, addressReg, restartPos, data](U8 mask) {
        // writed(address, value | mask);
        U8 tmpReg = data->getTmpReg();

        // :TODO: why isn't there an instruction that can set a single bit?
        data->loadConst(tmpReg, 1);
        data->shiftRegLeftWithReg32(tmpReg, tmpReg, mask);
        done(tmpReg, originalValueReg, tmpReg);

        data->writeMemory(addressReg, tmpReg, 32, false, data->currentOp->lock != 0, originalValueReg, restartPos);
        data->releaseTmpReg(tmpReg);
        });
    data->releaseTmpReg(originalValueReg);
    data->releaseTmpReg(addressReg);
}
void opBtR16R16(Armv8btAsm* data) {
    doBitTestReg16(data, nullptr);
}

void opBtE16R16(Armv8btAsm* data) {
    doBitTestMemory16(data, nullptr);
}
void opBtR32R32(Armv8btAsm* data) {
    doBitTestReg32(data, nullptr);
}
void opBtE32R32(Armv8btAsm* data) {
    doBitTestMemory32(data, nullptr);
}

void opBtsR16R16(Armv8btAsm* data) {
    doBitTestReg16(data, [data](U8 dstReg, U8 srcReg, U8 maskReg) {
        data->orRegs32(dstReg, srcReg, maskReg);
        });
}
void opBtsE16R16(Armv8btAsm* data) {
    doBitTestMemory16(data, [data](U8 dstReg, U8 srcReg, U8 maskReg) {
        data->orRegs32(dstReg, srcReg, maskReg);
        });
}
void opBtsR32R32(Armv8btAsm* data) {    
    doBitTestReg32(data, [data](U8 dstReg, U8 srcReg, U8 maskReg) {
        data->orRegs32(dstReg, srcReg, maskReg);
        });
}
void opBtsE32R32(Armv8btAsm* data) {    
    doBitTestMemory32(data, [data](U8 dstReg, U8 srcReg, U8 maskReg) {
        data->orRegs32(dstReg, srcReg, maskReg);
        });
}

void opBtrR16R16(Armv8btAsm* data) {
    doBitTestReg16(data, [data](U8 dstReg, U8 srcReg, U8 maskReg) {
        data->andNotRegs32(dstReg, srcReg, maskReg);
        });
}
void opBtrE16R16(Armv8btAsm* data) {
    doBitTestMemory16(data, [data](U8 dstReg, U8 srcReg, U8 maskReg) {
        data->andNotRegs32(dstReg, srcReg, maskReg);
        });
}
void opBtrR32R32(Armv8btAsm* data) {
    doBitTestReg32(data, [data](U8 dstReg, U8 srcReg, U8 maskReg) {
        data->andNotRegs32(dstReg, srcReg, maskReg);
        });
}
void opBtrE32R32(Armv8btAsm* data) {
    doBitTestMemory32(data, [data](U8 dstReg, U8 srcReg, U8 maskReg) {
        data->andNotRegs32(dstReg, srcReg, maskReg);
        });
}

static void doBsf16(Armv8btAsm* data, U8 valueReg) {
    data->doIf(valueReg, 0, DO_IF_EQUAL, [data] {
        data->orValue32(xFLAGS, xFLAGS, ZF);
        }, [data, valueReg] {
            data->andValue32(xFLAGS, xFLAGS, ~ZF);
            data->reverseBits32(valueReg, valueReg);
            data->clz32(valueReg, valueReg);
            data->movRegToReg(data->getNativeReg(data->currentOp->reg), valueReg, 16, false);
        });
}

static void doBsf32(Armv8btAsm* data, U8 valueReg) {
    data->doIf(valueReg, 0, DO_IF_EQUAL, [data] {
        data->orValue32(xFLAGS, xFLAGS, ZF);
        }, [data, valueReg] {
            data->andValue32(xFLAGS, xFLAGS, ~ZF);
            U8 tmpReg = data->getTmpReg();
            data->reverseBits32(tmpReg, valueReg);
            data->clz32(data->getNativeReg(data->currentOp->reg), tmpReg);
            data->releaseTmpReg(tmpReg);
        });
}

void opBsfR16R16(Armv8btAsm* data) {
    // U16 value = cpu->reg[srcReg].u16;
    // cpu->fillFlagsNoZF();
    // if (value == 0) {
    //     cpu->addZF();
    // } else {
    //     U16 result = 0;
    //     while ((value & 0x01) == 0) { result++; value >>= 1; }
    //     cpu->removeZF();
    //     cpu->reg[dstReg].u16 = result;
    // }
    U8 tmpReg = data->getTmpReg();
    data->movRegToReg(tmpReg, data->getNativeReg(data->currentOp->rm), 16, true);
    doBsf16(data, tmpReg);  
    data->releaseTmpReg(tmpReg);
}
void opBsfR16E16(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    U8 addressReg = data->getAddressReg();
    data->readMemory(addressReg, tmpReg, 16, true);
    data->releaseTmpReg(addressReg);
    doBsf16(data, tmpReg);
    data->releaseTmpReg(tmpReg);
}
void opBsfR32R32(Armv8btAsm* data) {
    doBsf32(data, data->getNativeReg(data->currentOp->rm));
}
void opBsfR32E32(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    U8 addressReg = data->getAddressReg();
    data->readMemory(addressReg, tmpReg, 32, true);
    data->releaseTmpReg(addressReg);
    doBsf32(data, tmpReg);
    data->releaseTmpReg(tmpReg);
}

static void doBsr16(Armv8btAsm* data, U8 valueReg) {
    data->doIf(valueReg, 0, DO_IF_EQUAL, [data] {
        data->orValue32(xFLAGS, xFLAGS, ZF);
        }, [data, valueReg] {
            data->andValue32(xFLAGS, xFLAGS, ~ZF);
            data->clz32(valueReg, valueReg); // value reg is already in upper 16-bits
            data->subValue32(valueReg, valueReg, 15);
            data->subRegs32(valueReg, 31, valueReg); // 0 - valueReg
            data->movRegToReg(data->getNativeReg(data->currentOp->reg), valueReg, 16, false);
        });
}

static void doBsr32(Armv8btAsm* data, U8 valueReg) {
    data->doIf(valueReg, 0, DO_IF_EQUAL, [data] {
        data->orValue32(xFLAGS, xFLAGS, ZF);
        }, [data, valueReg] {
            data->andValue32(xFLAGS, xFLAGS, ~ZF);
            data->clz32(data->getNativeReg(data->currentOp->reg), valueReg);
            data->subValue32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), 31);
            data->subRegs32(data->getNativeReg(data->currentOp->reg), 31, data->getNativeReg(data->currentOp->reg)); // 0 - reg
        });
}

void opBsrR16R16(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    data->shiftRegLeftWithValue32(tmpReg, data->getNativeReg(data->currentOp->rm), 16);
    doBsr16(data, tmpReg);
    data->releaseTmpReg(tmpReg);
}
void opBsrR16E16(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    U8 addressReg = data->getAddressReg();
    data->readMemory(addressReg, tmpReg, 16, true);
    data->shiftRegLeftWithValue32(tmpReg, tmpReg, 16);
    data->releaseTmpReg(addressReg);
    doBsr16(data, tmpReg);
    data->releaseTmpReg(tmpReg);
}
void opBsrR32R32(Armv8btAsm* data) {
    doBsr32(data, data->getNativeReg(data->currentOp->rm));
}
void opBsrR32E32(Armv8btAsm* data) {
    U8 tmpReg = data->getTmpReg();
    U8 addressReg = data->getAddressReg();
    data->readMemory(addressReg, tmpReg, 32, true);
    data->releaseTmpReg(addressReg);
    doBsr32(data, tmpReg);
    data->releaseTmpReg(tmpReg);
}
void opBtcR16R16(Armv8btAsm* data) {
    doBitTestReg16(data, [data](U8 dstReg, U8 srcReg, U8 maskReg) {
        data->xorRegs32(dstReg, srcReg, maskReg);
        });
}
void opBtcE16R16(Armv8btAsm* data) {
    doBitTestMemory16(data, [data](U8 dstReg, U8 srcReg, U8 maskReg) {
        data->xorRegs32(dstReg, srcReg, maskReg);
        });
}
void opBtcR32R32(Armv8btAsm* data) {
    doBitTestReg32(data, [data](U8 dstReg, U8 srcReg, U8 maskReg) {
        data->xorRegs32(dstReg, srcReg, maskReg);
        });
}
void opBtcE32R32(Armv8btAsm* data) {
    doBitTestMemory32(data, [data](U8 dstReg, U8 srcReg, U8 maskReg) {
        data->xorRegs32(dstReg, srcReg, maskReg);
        });
}

void doBitTest(Armv8btAsm* data, U8 valueReg) {
    data->copyBitsFromSourceAtPositionToDest(xFLAGS, valueReg, data->currentOp->extra, 1);
}

void doBitMemoryTest(Armv8btAsm* data, U32 width, std::function<void(U8 dst, U8 src)> action) {
    U8 addressReg = data->getAddressReg();
    U8 originalValueReg = data->getTmpReg();
    U8 valueReg = data->getTmpReg();

    U8 memReg = data->getHostMem(addressReg, width, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;
    data->readMemory(addressReg, originalValueReg, width, false, data->currentOp->lock);
    action(valueReg, originalValueReg);
    data->writeMemory(addressReg, valueReg, width, false, data->currentOp->lock != 0, originalValueReg, restartPos);
    data->releaseTmpReg(addressReg);
    doBitTest(data, originalValueReg);
    data->releaseTmpReg(valueReg);
    data->releaseTmpReg(originalValueReg);
}

void opBtR16(Armv8btAsm* data) {
    // cpu->fillFlagsNoCF();
    // cpu->setCF(cpu->reg[reg].u16 & mask);
    doBitTest(data, data->getNativeReg(data->currentOp->reg));
}
void opBtE16(Armv8btAsm* data) {
    // cpu->fillFlagsNoCF();
    // value = readw(address);
    // cpu->setCF(value & mask);
    U8 addressReg = data->getAddressReg();
    U8 valueReg = data->getTmpReg();
    data->readMemory(addressReg, valueReg, 16, true);
    data->releaseTmpReg(addressReg);
    doBitTest(data, valueReg);
    data->releaseTmpReg(valueReg);
}
void opBtsR16(Armv8btAsm* data) {
    // cpu->fillFlagsNoCF();
    // cpu->setCF(cpu->reg[reg].u16 & mask);
    // cpu->reg[reg].u16 |= mask;
    doBitTest(data, data->getNativeReg(data->currentOp->reg));
    data->orValue32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), data->currentOp->imm);
}
void opBtsE16(Armv8btAsm* data) {
    // cpu->fillFlagsNoCF();
    // value = readw(address);
    // cpu->setCF(value & mask);
    // writew(address, value | mask);
    doBitMemoryTest(data, 16, [data](U8 dst, U8 src) {
        data->orValue32(dst, src, data->currentOp->imm);
        });
}
void opBtrR16(Armv8btAsm* data) {
    doBitTest(data, data->getNativeReg(data->currentOp->reg));
    data->andValue32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), ~data->currentOp->imm);
}
void opBtrE16(Armv8btAsm* data) {
    doBitMemoryTest(data, 16, [data](U8 dst, U8 src) {
        data->andValue32(dst, src, ~data->currentOp->imm);
        });
}
void opBtcR16(Armv8btAsm* data) {
    doBitTest(data, data->getNativeReg(data->currentOp->reg));
    data->xorValue32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), data->currentOp->imm);
}
void opBtcE16(Armv8btAsm* data) {
    doBitMemoryTest(data, 16, [data](U8 dst, U8 src) {
        data->xorValue32(dst, src, data->currentOp->imm);
        });
}

void opBtR32(Armv8btAsm* data) {
    doBitTest(data, data->getNativeReg(data->currentOp->reg));
}
void opBtE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 valueReg = data->getTmpReg();
    data->readMemory(addressReg, valueReg, 32, true);
    data->releaseTmpReg(addressReg);
    doBitTest(data, valueReg);
    data->releaseTmpReg(valueReg);
}
void opBtsR32(Armv8btAsm* data) {
    doBitTest(data, data->getNativeReg(data->currentOp->reg));
    data->orValue32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), data->currentOp->imm);
}
void opBtsE32(Armv8btAsm* data) {
    doBitMemoryTest(data, 32, [data](U8 dst, U8 src) {
        data->orValue32(dst, src, data->currentOp->imm);
        });
}
void opBtrR32(Armv8btAsm* data) {
    doBitTest(data, data->getNativeReg(data->currentOp->reg));
    data->andValue32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), ~data->currentOp->imm);
}
void opBtrE32(Armv8btAsm* data) {
    doBitMemoryTest(data, 32, [data](U8 dst, U8 src) {
        data->andValue32(dst, src, ~data->currentOp->imm);
        });
}
void opBtcR32(Armv8btAsm* data) {
    doBitTest(data, data->getNativeReg(data->currentOp->reg));
    data->xorValue32(data->getNativeReg(data->currentOp->reg), data->getNativeReg(data->currentOp->reg), data->currentOp->imm);
}
void opBtcE32(Armv8btAsm* data) {
    doBitMemoryTest(data, 32, [data](U8 dst, U8 src) {
        data->xorValue32(dst, src, data->currentOp->imm);
        });
}

#endif