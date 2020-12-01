#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
#include "armv8btOps.h"

static void doBitTest16(Armv8btAsm* data, U8 maskReg, U8 valueReg, std::function<void(U8 mask)> done) {
    // U16 mask = 1 << (cpu->reg[rm].u16 & 15);
    // cpu->fillFlagsNoCF();
    // cpu->setCF(cpu->reg[reg].u16 & mask);
    if (data->lazyFlags) {
        U32 flags = DecodedOp::getNeededFlags(data->currentBlock, data->decodedOp, OF | SF | ZF | PF | AF);
        if (flags) {
            data->fillFlags(flags);
        }
        data->lazyFlags = NULL;
    }
    U8 tmpReg = data->getTmpReg();
    U8 tmpReg2 = data->getTmpReg();
    data->andValue32(tmpReg2, maskReg, 15);
    if (data->flagsNeeded() & CF) {
        data->shiftRegRightWithReg32(tmpReg, valueReg, tmpReg2);
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, tmpReg, 0, 1, true);
    }
    data->releaseTmpReg(tmpReg);
    if (done) {
        done(tmpReg2);
    }    
    data->releaseTmpReg(tmpReg2);
}

static void doBitTest32(Armv8btAsm* data, U8 maskReg, U8 valueReg, std::function<void(U8 mask)> done) {
    // U32 mask = 1 << (cpu->reg[maskReg].u32 & 31);
    // cpu->fillFlagsNoCF();
    // cpu->setCF(cpu->reg[reg].u32 & mask);
    if (data->lazyFlags) {
        U32 flags = DecodedOp::getNeededFlags(data->currentBlock, data->decodedOp, OF | SF | ZF | PF | AF);
        if (flags) {
            data->fillFlags(flags);
        }
        data->lazyFlags = NULL;
    }
    U8 tmpReg = data->getTmpReg();
    U8 tmpReg2 = data->getTmpReg();
    data->andValue32(tmpReg2, maskReg, 31);
    if (data->flagsNeeded() & CF) {
        data->shiftRegRightWithReg32(tmpReg, valueReg, tmpReg2);
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, tmpReg, 0, 1, true);
    }
    data->releaseTmpReg(tmpReg);
    if (done) {
        done(tmpReg2);
    }    
    data->releaseTmpReg(tmpReg2);
}

static U8 getBitAddress16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    // address += (((S16)cpu->reg[reg].u16) >> 4) * 2;
    data->signExtend(tmpReg, data->decodedOp->reg, 16);
    data->shiftSignedRegRightWithValue32(tmpReg, tmpReg, 4);
    data->shiftRegLeftWithValue32(tmpReg, tmpReg, 1);
    data->addRegs32(addressReg, addressReg, tmpReg);
    data->releaseTmpReg(tmpReg);
    return addressReg;
}

static U8 getBitAddress32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    // address += (((S32)cpu->reg[reg].u32) >> 5) * 4;
    data->shiftSignedRegRightWithValue32(tmpReg, data->decodedOp->reg, 5);
    data->shiftRegLeftWithValue32(tmpReg, tmpReg, 2);
    data->addRegs32(addressReg, addressReg, tmpReg);
    data->releaseTmpReg(tmpReg);
    return addressReg;
}

void opBtR16R16(Armv8btAsm* data) {
    doBitTest16(data, data->getNativeReg(data->decodedOp->rm), data->getNativeReg(data->decodedOp->reg), nullptr);
}

void opBtE16R16(Armv8btAsm* data) {
    U8 addressReg = getBitAddress16(data);
    U8 tmpReg = data->getTmpReg();

    data->readMemory(addressReg, tmpReg, 16, true);
    data->releaseTmpReg(addressReg);
    doBitTest16(data, data->getNativeReg(data->decodedOp->reg), tmpReg, nullptr);
    data->releaseTmpReg(tmpReg);    
}
void opBtR32R32(Armv8btAsm* data) {
    doBitTest32(data, data->getNativeReg(data->decodedOp->rm), data->getNativeReg(data->decodedOp->reg), nullptr);
}
void opBtE32R32(Armv8btAsm* data) {
    U8 addressReg = getBitAddress32(data);
    U8 tmpReg = data->getTmpReg();

    data->readMemory(addressReg, tmpReg, 32, true);
    data->releaseTmpReg(addressReg);
    doBitTest32(data, data->getNativeReg(data->decodedOp->reg), tmpReg, nullptr);
    data->releaseTmpReg(tmpReg);    
}

void opBtsR16R16(Armv8btAsm* data) {
    doBitTest16(data, data->getNativeReg(data->decodedOp->rm), data->getNativeReg(data->decodedOp->reg), [data](U8 mask) {
        U8 tmpReg = data->getTmpReg();
        // :TODO: why isn't there an instruction that can set a single bit?
        data->loadConst(tmpReg, 1);
        data->shiftRegLeftWithReg32(tmpReg, tmpReg, mask);
        data->orRegs32(tmpReg, tmpReg, data->getNativeReg(data->decodedOp->reg));

        data->movRegToReg(data->getNativeReg(data->decodedOp->reg), tmpReg, 16, false);
        data->releaseTmpReg(tmpReg);
        });
}
void opBtsE16R16(Armv8btAsm* data) {
    U8 addressReg = getBitAddress16(data);
    U8 originalValueReg = data->getTmpReg();
    U32 restartPos = data->bufferPos;

    data->readMemory(addressReg, originalValueReg, 16, true, data->decodedOp->lock != 0);
    doBitTest16(data, data->getNativeReg(data->decodedOp->reg), originalValueReg, [originalValueReg, addressReg, restartPos, data](U8 mask) {
        U8 tmpReg = data->getTmpReg();

        // :TODO: why isn't there an instruction that can set a single bit?
        data->loadConst(tmpReg, 1);
        data->shiftRegLeftWithReg32(tmpReg, tmpReg, mask);
        data->orRegs32(tmpReg, tmpReg, originalValueReg);

        data->writeMemory(addressReg, tmpReg, 16, true, data->decodedOp->lock != 0, originalValueReg, restartPos);
        data->releaseTmpReg(tmpReg);
        });
    data->releaseTmpReg(originalValueReg);
    data->releaseTmpReg(addressReg);
}
void opBtsR32R32(Armv8btAsm* data) {    
    doBitTest32(data, data->getNativeReg(data->decodedOp->rm), data->getNativeReg(data->decodedOp->reg), [data](U8 mask) {
        // cpu->reg[reg].u32 |= mask;
        U8 tmpReg = data->getTmpReg();
        // :TODO: why isn't there an instruction that can set a single bit?
        data->loadConst(tmpReg, 1);
        data->shiftRegLeftWithReg32(tmpReg, tmpReg, mask);
        data->orRegs32(tmpReg, tmpReg, data->getNativeReg(data->decodedOp->reg));

        data->movRegToReg(data->getNativeReg(data->decodedOp->reg), tmpReg, 32, false);
        data->releaseTmpReg(tmpReg);
        });
}
void opBtsE32R32(Armv8btAsm* data) {    
    U8 addressReg = getBitAddress16(data);
    U8 originalValueReg = data->getTmpReg();
    U32 restartPos = data->bufferPos;

    data->readMemory(addressReg, originalValueReg, 32, true, data->decodedOp->lock != 0);
    doBitTest16(data, data->getNativeReg(data->decodedOp->reg), originalValueReg, [originalValueReg, addressReg, restartPos, data](U8 mask) {
        // writed(address, value | mask);
        U8 tmpReg = data->getTmpReg();

        // :TODO: why isn't there an instruction that can set a single bit?
        data->loadConst(tmpReg, 1);
        data->shiftRegLeftWithReg32(tmpReg, tmpReg, mask);
        data->orRegs32(tmpReg, tmpReg, originalValueReg);

        data->writeMemory(addressReg, tmpReg, 32, true, data->decodedOp->lock != 0, originalValueReg, restartPos);
        data->releaseTmpReg(tmpReg);
        });
    data->releaseTmpReg(originalValueReg);
    data->releaseTmpReg(addressReg);
}

void opBtrR16R16(Armv8btAsm* data) {}
void opBtrE16R16(Armv8btAsm* data) {}
void opBtrR32R32(Armv8btAsm* data) {}
void opBtrE32R32(Armv8btAsm* data) {}

void opBsfR16R16(Armv8btAsm* data) {}
void opBsfR16E16(Armv8btAsm* data) {}
void opBsfR32R32(Armv8btAsm* data) {}
void opBsfR32E32(Armv8btAsm* data) {}

void opBsrR16R16(Armv8btAsm* data) {}
void opBsrR16E16(Armv8btAsm* data) {}
void opBsrR32R32(Armv8btAsm* data) {}
void opBsrR32E32(Armv8btAsm* data) {}

void opBtcR16R16(Armv8btAsm* data) {}
void opBtcE16R16(Armv8btAsm* data) {}
void opBtcR32R32(Armv8btAsm* data) {}
void opBtcE32R32(Armv8btAsm* data) {}

void opBtR16(Armv8btAsm* data) {}
void opBtE16(Armv8btAsm* data) {}
void opBtsR16(Armv8btAsm* data) {}
void opBtsE16(Armv8btAsm* data) {}
void opBtrR16(Armv8btAsm* data) {}
void opBtrE16(Armv8btAsm* data) {}
void opBtcR16(Armv8btAsm* data) {}
void opBtcE16(Armv8btAsm* data) {}

void opBtR32(Armv8btAsm* data) {}
void opBtE32(Armv8btAsm* data) {}
void opBtsR32(Armv8btAsm* data) {}
void opBtsE32(Armv8btAsm* data) {}
void opBtrR32(Armv8btAsm* data) {}
void opBtrE32(Armv8btAsm* data) {}
void opBtcR32(Armv8btAsm* data) {}
void opBtcE32(Armv8btAsm* data) {}

#endif