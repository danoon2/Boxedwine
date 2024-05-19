#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
#include "armv8btOps.h"

static void dshl16(Armv8btAsm* data, U8 result, U8 dst, U8 src) {
    //cpu->src.u32 = imm;
    //cpu->dst.u32 = cpu->reg[reg].u16;
    //cpu->dst2.u32 = cpu->reg[rm].u16;
    //tmp = (cpu->dst.u32 << 16) | cpu->dst2.u32;
    //result = tmp << cpu->src.u8;
    //if (imm > 16) {
    //    result |= ((U32)(cpu->reg[rm].u16) << (imm - 16));
    //}
    //cpu->result.u16 = (U16)(result >> 16);
    //cpu->reg[reg].u16 = cpu->result.u16;
    //cpu->lazyFlags = FLAGS_DSHL16;

    U32 flags = data->flagsNeeded();
    if (ARM8BT_FLAGS_DSHL16->usesDst(flags)) {
        data->movRegToReg(xDst, dst, 16, true);
    }
    if (data->currentOp->imm == 16) {
        data->movRegToReg(result, src, 16, false);
    } else if (data->currentOp->imm < 16) {
        data->copyBitsFromSourceToDestAtPosition(result, dst, data->currentOp->imm, 16 - data->currentOp->imm, true);
        data->copyBitsFromSourceAtPositionToDest(result, src, 16 - data->currentOp->imm, data->currentOp->imm, true);
    } else {
        // If a count is greater than the operand size, the result is undefined.
        U8 amount = data->currentOp->imm & 0xF;
        data->copyBitsFromSourceToDestAtPosition(result, src, amount, 16 - amount, true);
        data->copyBitsFromSourceAtPositionToDest(result, src, 16 - amount, amount, true);
    }
    if (ARM8BT_FLAGS_DSHL16->usesResult(flags) && result != xResult) {
        data->movRegToReg(xResult, result, 16, true);
    }
    ARM8BT_FLAGS_DSHL16->setFlags(data, flags);
}

static void dshr16(Armv8btAsm* data, U8 result, U8 dst, U8 src) {
    // cpu->src.u32 = imm;
    // cpu->dst.u32 = (cpu->reg[reg].u16) | ((U32)(cpu->reg[rm].u16) << 16);
    // result = cpu->dst.u32 >> cpu->src.u8;
    // if (imm > 16) {
    //     result |= ((U32)(cpu->reg[rm].u16) << (32 - imm));
    // }
    // cpu->result.u16 = (U16)result;
    // cpu->reg[reg].u16 = cpu->result.u16;
    // cpu->lazyFlags = FLAGS_DSHR16;

    U32 flags = data->flagsNeeded();

    if (ARM8BT_FLAGS_DSHR16->usesDst(flags)) {
        data->movRegToReg(xDst, dst, 16, true);
    }
    if (data->currentOp->imm == 16) {
        data->movRegToReg(result, src, 16, false);
    } else if (data->currentOp->imm < 16) {
        data->copyBitsFromSourceAtPositionToDest(result, dst, data->currentOp->imm, 16 - data->currentOp->imm, true);
        data->copyBitsFromSourceToDestAtPosition(result, src, 16-data->currentOp->imm, data->currentOp->imm, true);        
    } else {
        // If a count is greater than the operand size, the result is undefined.
        U8 amount = data->currentOp->imm & 0xF;
        data->copyBitsFromSourceToDestAtPosition(result, src, 16 - amount, amount, true);
        data->copyBitsFromSourceAtPositionToDest(result, src, amount, 16 - amount, true);
    }
    if (ARM8BT_FLAGS_DSHR16->usesResult(flags) && result != xResult) {
        data->movRegToReg(xResult, result, 16, true);
    }
    ARM8BT_FLAGS_DSHR16->setFlags(data, flags);
}

static void dshl32(Armv8btAsm* data, U8 result, U8 dst, U8 src) {
    //cpu->src.u32 = imm;
    //cpu->dst.u32 = cpu->reg[reg].u32;
    //cpu->result.u32 = (cpu->reg[reg].u32 << imm) | (cpu->reg[rm].u32 >> (32 - imm));
    //cpu->reg[reg].u32 = cpu->result.u32;
    //cpu->lazyFlags = FLAGS_DSHL32;

    U32 flags = data->flagsNeeded();
    if (ARM8BT_FLAGS_DSHL32->usesDst(flags)) {
        data->movRegToReg(xDst, dst, 32, false);
    }
    // imm should already be masked to 0-31
    data->copyBitsFromSourceToDestAtPosition(result, dst, data->currentOp->imm, 32 - data->currentOp->imm, true);
    data->copyBitsFromSourceAtPositionToDest(result, src, 32 - data->currentOp->imm, data->currentOp->imm, true);
    if (flags && ARM8BT_FLAGS_DSHL32->usesResult(flags) && result != xResult) {
        data->movRegToReg(xResult, dst, 32, false);
    }
    ARM8BT_FLAGS_DSHL32->setFlags(data, flags);
}

static void dshr32(Armv8btAsm* data, U8 result, U8 dst, U8 src) {
    // cpu->src.u32 = imm;
    // cpu->dst.u32 = cpu->reg[reg].u32;
    // cpu->result.u32 = (cpu->reg[reg].u32 >> imm) | (cpu->reg[rm].u32 << (32 - imm));
    // cpu->reg[reg].u32 = cpu->result.u32;
    // cpu->lazyFlags = FLAGS_DSHR32;

    U32 flags = data->flagsNeeded();
    if (ARM8BT_FLAGS_DSHR32->usesDst(flags)) {
        data->movRegToReg(xDst, dst, 32, false);
    }
    // imm should already be masked to 0-31
    data->copyBitsFromSourceAtPositionToDest(result, dst, data->currentOp->imm, 32 - data->currentOp->imm, true);
    data->copyBitsFromSourceToDestAtPosition(result, src, 32 - data->currentOp->imm, data->currentOp->imm, true);
    if (ARM8BT_FLAGS_DSHR32->usesResult(flags) && result != xResult) {
        data->movRegToReg(xResult, dst, 32, false);
    }
    ARM8BT_FLAGS_DSHR32->setFlags(data, flags);
}

static void dshl16Cl(Armv8btAsm* data, U8 result, U8 dst, U8 src, std::function<void(void)> writeResult) {
    // if (CL & 0x1f) {
    //     cpu->src.u32 = CL & 0x1f;
    //     cpu->dst.u32 = cpu->reg[reg].u16;
    //     cpu->dst2.u32 = cpu->reg[rm].u16;
    //     tmp = (cpu->dst.u32 << 16) | cpu->dst2.u32;
    //     result = tmp << cpu->src.u8;
    //     if (cpu->src.u32 > 16) {
    //         result |= ((U32)(cpu->reg[rm].u16) << (cpu->src.u32 - 16));
    //     }
    //     cpu->result.u16 = (U16)(result >> 16);
    //     cpu->reg[reg].u16 = cpu->result.u16;
    //     cpu->lazyFlags = FLAGS_DSHL16;
    // }    

    U8 tmpReg = data->getTmpReg();
    data->andValue32(tmpReg, xECX, 0x1f, true);    

    data->doIf(0, 0, DO_IF_NOT_EQUAL, [writeResult, tmpReg, data, result, dst, src]() {
        U8 tmpReg2 = data->getTmpReg();
        U32 flags = data->flagsNeeded();

        if (ARM8BT_FLAGS_DSHL16_CL->usesDst(flags)) {
            data->movRegToReg(xDst, dst, 16, true);
        }
        if (ARM8BT_FLAGS_DSHL16_CL->usesSrc(flags)) {
            data->movRegToReg(xSrc, tmpReg, 32, false);
        }

        data->shiftRegLeftWithValue32(tmpReg2, dst, 16);
        data->movRegToReg(tmpReg2, src, 16, false);
        data->shiftRegLeftWithReg32(tmpReg2, tmpReg2, tmpReg);
        data->shiftRegRightWithValue32(tmpReg2, tmpReg2, 16);
        data->movRegToReg(result, tmpReg2, 16, false);
        data->releaseTmpReg(tmpReg2);

        if (ARM8BT_FLAGS_DSHL16_CL->usesResult(flags) && result != xResult) {
            data->movRegToReg(xResult, result, 16, true);
        }
        ARM8BT_FLAGS_DSHL16_CL->setFlags(data, flags);
        if (writeResult) {
            writeResult();
        }
        }, nullptr, nullptr, false, false);
    data->releaseTmpReg(tmpReg);
}

static void dshr16Cl(Armv8btAsm* data, U8 result, U8 dst, U8 src, std::function<void(void)> writeResult) {
    // if (CL & 0x1f) {
    //     U32 result;
    //     cpu->src.u32 = CL & 0x1f;
    //     cpu->dst.u32 = (cpu->reg[reg].u16) | ((U32)(cpu->reg[rm].u16) << 16);
    //     result = cpu->dst.u32 >> cpu->src.u8;
    //     if (cpu->src.u32 > 16) {
    //         result |= ((U32)(cpu->reg[rm].u16) << (32 - cpu->src.u32));
    //     }
    //     cpu->result.u16 = (U16)result;
    //     cpu->reg[reg].u16 = cpu->result.u16;
    //     cpu->lazyFlags = FLAGS_DSHR16;
    // }

    U8 tmpReg = data->getTmpReg();
    data->andValue32(tmpReg, xECX, 0x1f, true);

    data->doIf(0, 0, DO_IF_NOT_EQUAL, [writeResult, tmpReg, data, result, dst, src]() {
        U8 tmpReg2 = data->getTmpReg();
        U32 flags = data->flagsNeeded();

        if (ARM8BT_FLAGS_DSHR16_CL->usesSrc(flags)) {
            data->movRegToReg(xSrc, tmpReg, 32, false);
        }

        // cpu->dst.u32 = (cpu->reg[reg].u16) | ((U32)(cpu->reg[rm].u16) << 16);
        data->shiftRegLeftWithValue32(xDst, src, 16);
        data->movRegToReg(xDst, dst, 16, false);

        // result = cpu->dst.u32 >> cpu->src.u8;
        data->shiftRegRightWithReg32(tmpReg2, xDst, tmpReg);
        data->movRegToReg(result, tmpReg2, 16, false);
        data->releaseTmpReg(tmpReg2);

        if (ARM8BT_FLAGS_DSHR16_CL->usesResult(flags) && result != xResult) {
            data->movRegToReg(xResult, result, 16, true);
        }
        ARM8BT_FLAGS_DSHR16_CL->setFlags(data, flags);
        if (writeResult) {
            writeResult();
        }
        }, nullptr, nullptr, false, false);
    data->releaseTmpReg(tmpReg);
}

static void dshl32Cl(Armv8btAsm* data, U8 result, U8 dst, U8 src, std::function<void(void)> writeResult) {
    // if (CL & 0x1f) {
    //     cpu->src.u32 = CL & 0x1f;
    //     cpu->dst.u32 = cpu->reg[reg].u32;
    //     cpu->result.u32 = (cpu->dst.u32 << cpu->src.u32);
    //     cpu->result.u32 |= (cpu->reg[rm].u32 >> (32 - cpu->src.u32));
    //     cpu->reg[reg].u32 = cpu->result.u32;
    //     cpu->lazyFlags = FLAGS_DSHL32;
    // }

    U8 tmpReg = data->getTmpReg();
    data->andValue32(tmpReg, xECX, 0x1f, true);    

    data->doIf(0, 0, DO_IF_NOT_EQUAL, [writeResult, tmpReg, data, result, dst, src]() {
        U32 flags = data->flagsNeeded();

        if (ARM8BT_FLAGS_DSHL32_CL->usesDst(flags)) {
            data->movRegToReg(xDst, dst, 32, false);
        }
        if (ARM8BT_FLAGS_DSHL32_CL->usesSrc(flags)) {
            data->movRegToReg(xSrc, tmpReg, 32, false);
        }

        // cpu->result.u32 = (cpu->dst.u32 << cpu->src.u32);
        // cpu->result.u32 |= (cpu->reg[rm].u32 >> (32 - cpu->src.u32));
        U8 tmpReg2 = data->getTmpReg();
        data->shiftRegLeftWithReg32(tmpReg2, dst, tmpReg);
        data->subRegFromValue32(tmpReg, tmpReg, 32);
        data->shiftRegRightWithReg32(tmpReg, src, tmpReg);
        data->orRegs32(result, tmpReg, tmpReg2);
        data->releaseTmpReg(tmpReg2);

        if (ARM8BT_FLAGS_DSHL32_CL->usesResult(flags) && result != xResult) {
            data->movRegToReg(xResult, dst, 32, true);
        }
        ARM8BT_FLAGS_DSHL32_CL->setFlags(data, flags);
        if (writeResult) {
            writeResult();
        }
        }, nullptr, nullptr, false, false);
    data->releaseTmpReg(tmpReg);
}

static void dshr32Cl(Armv8btAsm* data, U8 result, U8 dst, U8 src, std::function<void(void)> writeResult) {
    // if (CL & 0x1f) {
    //     cpu->src.u32 = CL & 0x1f;
    //     cpu->dst.u32 = cpu->reg[reg].u32;
    //     cpu->result.u32 = (cpu->dst.u32 >> cpu->src.u32);
    //     cpu->result.u32 |= (cpu->reg[rm].u32 << (32 - cpu->src.u32));
    //     cpu->reg[reg].u32 = cpu->result.u32;
    //     cpu->lazyFlags = FLAGS_DSHR32;
    // }

    U8 tmpReg = data->getTmpReg();
    data->andValue32(tmpReg, xECX, 0x1f, true);

    data->doIf(0, 0, DO_IF_NOT_EQUAL, [writeResult, tmpReg, data, result, dst, src]() {
        U32 flags = data->flagsNeeded();

        if (ARM8BT_FLAGS_DSHR32_CL->usesDst(flags)) {
            data->movRegToReg(xDst, dst, 32, false);
        }
        if (ARM8BT_FLAGS_DSHR32_CL->usesSrc(flags)) {
            data->movRegToReg(xSrc, tmpReg, 32, false);
        }

        // cpu->result.u32 = (cpu->dst.u32 >> cpu->src.u32);
        // cpu->result.u32 |= (cpu->reg[rm].u32 << (32 - cpu->src.u32));
        U8 tmpReg2 = data->getTmpReg();
        data->shiftRegRightWithReg32(tmpReg2, dst, tmpReg);
        data->subRegFromValue32(tmpReg, tmpReg, 32);
        data->shiftRegLeftWithReg32(tmpReg, src, tmpReg);
        data->orRegs32(result, tmpReg, tmpReg2);
        data->releaseTmpReg(tmpReg2);

        if (ARM8BT_FLAGS_DSHR32_CL->usesResult(flags) && result != xResult) {
            data->movRegToReg(xResult, dst, 32, true);
        }
        ARM8BT_FLAGS_DSHR32_CL->setFlags(data, flags);
        if (writeResult) {
            writeResult();
        }
        }, nullptr, nullptr, false, false);
    data->releaseTmpReg(tmpReg);
}

void opDshlR16R16(Armv8btAsm* data) {
    dshl16(data, data->currentOp->reg, data->currentOp->reg, data->currentOp->rm);
}

void opDshlE16R16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    U8 memReg = data->getHostMem(addressReg, 16, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;

    data->readMemory(addressReg, tmpReg, 16, false, data->currentOp->lock != 0);
    dshl16(data, xResult, tmpReg, data->currentOp->reg);
    data->writeMemory(addressReg, xResult, 16, false, data->currentOp->lock != 0, tmpReg, restartPos);
    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(tmpReg);
}

void opDshlClR16R16(Armv8btAsm* data) {
    dshl16Cl(data, data->currentOp->reg, data->currentOp->reg, data->currentOp->rm, nullptr);
}
void opDshlClE16R16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    U8 memReg = data->getHostMem(addressReg, 16, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;

    data->readMemory(addressReg, tmpReg, 16, false, data->currentOp->lock != 0);
    dshl16Cl(data, xResult, tmpReg, data->currentOp->reg, [data, addressReg, tmpReg, restartPos]() {
        data->writeMemory(addressReg, xResult, 16, false, data->currentOp->lock != 0, tmpReg, restartPos);
        });
    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(tmpReg);
}
void opDshrR16R16(Armv8btAsm* data) {
    dshr16(data, data->currentOp->reg, data->currentOp->reg, data->currentOp->rm);
}
void opDshrE16R16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    U8 memReg = data->getHostMem(addressReg, 16, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;

    data->readMemory(addressReg, tmpReg, 16, false, data->currentOp->lock != 0);
    dshr16(data, xResult, tmpReg, data->currentOp->reg);
    data->writeMemory(addressReg, xResult, 16, false, data->currentOp->lock != 0, tmpReg, restartPos);
    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(tmpReg);
}
void opDshrClR16R16(Armv8btAsm* data) {
    dshr16Cl(data, data->currentOp->reg, data->currentOp->reg, data->currentOp->rm, nullptr);
}
void opDshrClE16R16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    U8 memReg = data->getHostMem(addressReg, 16, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;

    data->readMemory(addressReg, tmpReg, 16, false, data->currentOp->lock != 0);
    dshr16Cl(data, xResult, tmpReg, data->currentOp->reg, [data, addressReg, tmpReg, restartPos]() {
        data->writeMemory(addressReg, xResult, 16, false, data->currentOp->lock != 0, tmpReg, restartPos);
        });
    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(tmpReg);
}

void opDshlR32R32(Armv8btAsm* data) {
    dshl32(data, data->currentOp->reg, data->currentOp->reg, data->currentOp->rm);
}
void opDshlE32R32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    U8 memReg = data->getHostMem(addressReg, 32, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;

    data->readMemory(addressReg, tmpReg, 32, false, data->currentOp->lock != 0);
    dshl32(data, xResult, tmpReg, data->currentOp->reg);
    data->writeMemory(addressReg, xResult, 32, false, data->currentOp->lock != 0, tmpReg, restartPos);
    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(tmpReg);
}

void opDshlClR32R32(Armv8btAsm* data) {
    dshl32Cl(data, data->currentOp->reg, data->currentOp->reg, data->currentOp->rm, nullptr);
}
void opDshlClE32R32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    U8 memReg = data->getHostMem(addressReg, 32, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;

    data->readMemory(addressReg, tmpReg, 32, false, data->currentOp->lock != 0);
    dshl32Cl(data, xResult, tmpReg, data->currentOp->reg, [data, addressReg, tmpReg, restartPos]() {
        data->writeMemory(addressReg, xResult, 32, false, data->currentOp->lock != 0, tmpReg, restartPos);
        });
    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(tmpReg);
}

void opDshrR32R32(Armv8btAsm* data) {
    dshr32(data, data->currentOp->reg, data->currentOp->reg, data->currentOp->rm);
}

void opDshrE32R32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    U8 memReg = data->getHostMem(addressReg, 32, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;

    data->readMemory(addressReg, tmpReg, 32, false, data->currentOp->lock != 0);
    dshr32(data, xResult, tmpReg, data->currentOp->reg);
    data->writeMemory(addressReg, xResult, 32, false, data->currentOp->lock != 0, tmpReg, restartPos);
    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(tmpReg);
}

void opDshrClR32R32(Armv8btAsm* data) {
    dshr32Cl(data, data->currentOp->reg, data->currentOp->reg, data->currentOp->rm, nullptr);
}
void opDshrClE32R32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->getTmpReg();

    U8 memReg = data->getHostMem(addressReg, 32, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U32 restartPos = data->bufferPos;

    data->readMemory(addressReg, tmpReg, 32, false, data->currentOp->lock != 0);
    dshr32Cl(data, xResult, tmpReg, data->currentOp->reg, [data, addressReg, tmpReg, restartPos]() {
        data->writeMemory(addressReg, xResult, 32, false, data->currentOp->lock != 0, tmpReg, restartPos);
        });
    data->releaseTmpReg(addressReg);
    data->releaseTmpReg(tmpReg);
}

void shlValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    data->shiftRegLeftWithValue32(dst, src1, value);
}

void shlRegCl32(Armv8btAsm* data, U8 dst, U8 src1, U8 cl) {
    // this is already masked with 0x1f
    data->shiftRegLeftWithReg32(dst, src1, cl);
}

void shrValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    data->shiftRegRightWithValue32(dst, src1, value);
}

void shrRegCl32(Armv8btAsm* data, U8 dst, U8 src1, U8 cl) {
    // this is already masked with 0x1f
    data->shiftRegRightWithReg32(dst, src1, cl);
}

void sarValue8(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    U8 tmpReg = data->getTmpReg();
    data->signExtend(tmpReg, src1, 8);
    data->shiftSignedRegRightWithValue32(dst, tmpReg, value);
    data->releaseTmpReg(tmpReg);
}

void sarRegCl8(Armv8btAsm* data, U8 dst, U8 src1, U8 cl) {
    // this is already masked with 0x1f
    U8 tmpReg = data->getTmpReg();
    data->signExtend(tmpReg, src1, 8);
    data->shiftSignedRegRightWithReg32(dst, tmpReg, cl);
    data->releaseTmpReg(tmpReg);
}

void sarValue16(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    U8 tmpReg = data->getTmpReg();
    data->signExtend(tmpReg, src1, 16);
    data->shiftSignedRegRightWithValue32(dst, tmpReg, value);
    data->releaseTmpReg(tmpReg);
}

void sarRegCl16(Armv8btAsm* data, U8 dst, U8 src1, U8 cl) {
    // this is already masked with 0x1f
    U8 tmpReg = data->getTmpReg();
    data->signExtend(tmpReg, src1, 16);
    data->shiftSignedRegRightWithReg32(dst, tmpReg, cl);
    data->releaseTmpReg(tmpReg);
}

void sarRegCl32(Armv8btAsm* data, U8 dst, U8 src1, U8 cl) {
    // this is already masked with 0x1f
    data->shiftSignedRegRightWithReg32(dst, src1, cl);
}

void sarValue32(Armv8btAsm* data, U8 dst, U8 src1, U32 value, bool flags) {
    data->shiftSignedRegRightWithValue32(dst, src1, value);
}

typedef bool(*shiftOp)(Armv8btAsm* data, U8 result, U8 reg8, U32 width);

// result and reg are guaranteed to be different regs
bool doRol(Armv8btAsm* data, U8 result, U8 reg, U32 width) {
    U8 tmpReg = data->getTmpReg();

    U32 flags = data->flagsNeeded();

    if (!(data->currentOp->imm & (width - 1))) {
        if (data->currentOp->imm) {
            // cpu->setCF(reg8 & 1);
            if (flags & CF) {
                data->copyBitsFromSourceAtPositionToDest(xFLAGS, reg, 0, 1);
            }
            // cpu->setOF((reg8 & 1) ^ (reg8 >> 7));
            if (flags & OF) {
                data->copyBitsFromSourceAtPositionToDest(tmpReg, reg, width - 1, 1, false);
                data->xorRegs32(tmpReg, tmpReg, reg);
                data->copyBitsFromSourceToDestAtPosition(xFLAGS, tmpReg, 11, 1); // OF is 0x800 (bit 11)
            }
        }
        data->releaseTmpReg(tmpReg);
        return false;
    }
    U32 imm = data->currentOp->imm & (width - 1);

    // cpu->fillFlagsNoCFOF()

    if (width == 32) {
        data->rotateRightWithValue32(result, reg, 32 - imm);
    } else {
        // result = (var1 << var2) | (var1 >> (8 - var2));
        data->shiftRegLeftWithValue32(result, reg, imm);
        data->shiftRegRightWithValue32(tmpReg, reg, width - imm);
        data->orRegs32(result, result, tmpReg);
    }

    // cpu->setCF(result & 1);
    if (flags & CF) {
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, result, 0, 1);
    }
    // cpu->setOF((result & 1) ^ (result >> 7));
    if (flags & OF) {
        data->copyBitsFromSourceAtPositionToDest(tmpReg, result, width - 1, 1, false);
        data->xorRegs32(tmpReg, tmpReg, result);
        data->copyBitsFromSourceToDestAtPosition(xFLAGS, tmpReg, 11, 1); // OF is 0x800 (bit 11)
    }

    data->releaseTmpReg(tmpReg);
    return true;
}

// result and reg are guaranteed to be different regs
bool doRolCl(Armv8btAsm* data, U8 result, U8 reg, U32 width) {
    U8 tmpReg = data->getTmpReg();
    U32 flags = data->flagsNeeded();
    // cpu->fillFlagsNoCFOF()
    bool needsFlags = (flags & (CF | OF)) != 0;

    data->andValue32(tmpReg, xECX, 0x1f);
    data->doIf(tmpReg, 0, DO_IF_EQUAL, [data, result, reg, width]() {
        if (result != reg) {
            data->movRegToReg(result, reg, 32, false);
        }
        }, [data, result, reg, tmpReg, needsFlags, flags, width]() {
            if (width == 32) {
                data->subRegFromValue32(tmpReg, tmpReg, 32);
                data->rotateRightWithReg32(result, reg, tmpReg);
            } else {
                // result = (var1 << var2) | (var1 >> (8 - var2));
                data->andValue32(tmpReg, xECX, width - 1);
                data->shiftRegLeftWithReg32(result, reg, tmpReg);
                data->subRegFromValue32(tmpReg, tmpReg, width);
                data->shiftRegRightWithReg32(tmpReg, reg, tmpReg);
                data->orRegs32(result, result, tmpReg);
            }

            // cpu->setCF(result & 1);
            if (flags & CF) {
                data->copyBitsFromSourceAtPositionToDest(xFLAGS, result, 0, 1);
            }
            // cpu->setOF((result & 1) ^ (result >> 31));
            if (flags & OF) {
                data->copyBitsFromSourceAtPositionToDest(tmpReg, result, width - 1, 1, false);
                data->xorRegs32(tmpReg, tmpReg, result);
                data->copyBitsFromSourceToDestAtPosition(xFLAGS, tmpReg, 11, 1); // OF is 0x800 (bit 11)
            }
        });

    data->releaseTmpReg(tmpReg);
    return true;
}

// result and reg are guaranteed to be different regs
bool doRor(Armv8btAsm* data, U8 result, U8 reg, U32 width) {
    U8 tmpReg = data->getTmpReg();

    U32 flags = data->flagsNeeded();
    if (!(data->currentOp->imm & (width - 1))) {
        if (data->currentOp->imm) {
            // cpu->setCF(reg & 0x80);
            if (flags & CF) {
                data->copyBitsFromSourceAtPositionToDest(xFLAGS, reg, width - 1, 1);
            }
            // cpu->setOF((reg8 ^ (reg << 1)) & 0x80);
            if (flags & OF) {
                data->shiftRegLeftWithValue32(tmpReg, reg, 1);
                data->xorRegs32(tmpReg, tmpReg, reg);
                data->copyBitsFromSourceAtPositionToDest(tmpReg, tmpReg, width - 1, 1, false);
                data->copyBitsFromSourceToDestAtPosition(xFLAGS, tmpReg, 11, 1); // OF is 0x800 (bit 11)
            }
        }
        data->releaseTmpReg(tmpReg);
        return false;
    }
    U32 imm = data->currentOp->imm & (width - 1);

    // cpu->fillFlagsNoCFOF()
    if (width == 32) {
        data->rotateRightWithValue32(result, reg, imm);
    } else {
        // result = (var1 >> var2) | (var1 << (8 - var2));
        data->shiftRegRightWithValue32(result, reg, imm);
        data->shiftRegLeftWithValue32(tmpReg, reg, width - imm);
        data->orRegs32(result, result, tmpReg);
    }

    // cpu->setCF(result & 0x80);
    if (flags & CF) {
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, result, width - 1, 1);
    }
    // cpu->setOF((result ^ (result << 1)) & 0x80);
    if (flags & OF) {
        data->shiftRegLeftWithValue32(tmpReg, result, 1);
        data->xorRegs32(tmpReg, tmpReg, result);
        data->copyBitsFromSourceAtPositionToDest(tmpReg, tmpReg, width - 1, 1, false);
        data->copyBitsFromSourceToDestAtPosition(xFLAGS, tmpReg, 11, 1); // OF is 0x800 (bit 11)
    }

    data->releaseTmpReg(tmpReg);
    return true;
}

// result and reg are guaranteed to be different regs
bool doRorCl(Armv8btAsm* data, U8 result, U8 reg, U32 width) {
    U8 tmpReg = data->getTmpReg();
    U32 flags = data->flagsNeeded();
    // cpu->fillFlagsNoCFOF()
    bool needsFlags = (flags & (CF | OF)) != 0;

    data->andValue32(tmpReg, xECX, 0x1f);
    data->doIf(tmpReg, 0, DO_IF_EQUAL, [data, result, reg, width]() {
        if (result != reg) {
            data->movRegToReg(result, reg, 32, false);
        }
        }, [data, result, reg, tmpReg, needsFlags, flags, width]() {
            if (width == 32) {
                data->rotateRightWithReg32(result, reg, tmpReg);
            } else {
                // result = (var1 >> var2) | (var1 << (8 - var2));
                data->andValue32(tmpReg, xECX, width - 1);
                data->shiftRegRightWithReg32(result, reg, tmpReg);
                data->subRegFromValue32(tmpReg, tmpReg, width);
                data->shiftRegLeftWithReg32(tmpReg, reg, tmpReg);
                data->orRegs32(result, result, tmpReg);
            }

            // cpu->setCF(result & 0x80);
            if (flags & CF) {
                data->copyBitsFromSourceAtPositionToDest(xFLAGS, result, width - 1, 1);
            }
            // cpu->setOF((result ^ (result << 1)) & 0x80);
            if (flags & OF) {
                data->shiftRegLeftWithValue32(tmpReg, result, 1);
                data->xorRegs32(tmpReg, tmpReg, result);
                data->copyBitsFromSourceAtPositionToDest(tmpReg, tmpReg, width - 1, 1, false);
                data->copyBitsFromSourceToDestAtPosition(xFLAGS, tmpReg, 11, 1); // OF is 0x800 (bit 11)
            }
        });

    data->releaseTmpReg(tmpReg);
    return true;
}

// result and reg are guaranteed to be different regs
bool doRcl(Armv8btAsm* data, U8 result, U8 reg, U32 width) {
    if (data->currentOp->imm == 0) {
        return false;
    }
    U8 tmpReg = data->getTmpReg();

    // cpu->fillFlagsNoCFOF()
    U32 flags = data->flagsNeeded();

    // result = (var1 << var2) | ((cpu->flags & CF) << (var2 - 1)) | (var1 >> (9 - var2));
    data->shiftRegLeftWithValue32(result, reg, data->currentOp->imm);
    if ((width + 1) - data->currentOp->imm < 32) {
        data->shiftRegRightWithValue32(tmpReg, reg, (width + 1) - data->currentOp->imm);
        data->orRegs32(result, result, tmpReg);
    }
    data->copyBitsFromSourceToDestAtPosition(result, xFLAGS, data->currentOp->imm - 1, 1, true);

    // cpu->setCF(((var1 >> (8 - var2)) & 1));
    if (flags & (CF | OF)) { // OF depends on this
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, reg, (width - data->currentOp->imm), 1);
    }
    // cpu->setOF((cpu->flags & CF) ^ (result >> 7));
    if (flags & OF) {
        data->copyBitsFromSourceAtPositionToDest(tmpReg, result, (width - 1), 1, false);
        data->xorRegs32(tmpReg, tmpReg, xFLAGS);
        data->copyBitsFromSourceToDestAtPosition(xFLAGS, tmpReg, 11, 1); // OF is 0x800 (bit 11)
    }
    data->releaseTmpReg(tmpReg);
    return true;
}

// result and reg are guaranteed to be different regs
bool doRclCl(Armv8btAsm* data, U8 result, U8 reg, U32 width) {
    U8 tmpReg = data->getTmpReg();

    data->andValue32(tmpReg, xECX, 0x1f);
    data->doIf(tmpReg, 0, DO_IF_EQUAL, [data, result, reg, width]() {
        if (result != reg) {
            data->movRegToReg(result, reg, 32, false);
        }
        }, [data, result, reg, tmpReg, width]() {
            U32 flags = data->flagsNeeded();

            // var2 = var2 % 17;
            data->tmpRegInUse[result - xTmp1] = false;
            if (width != 32) {
                data->modValue32(tmpReg, tmpReg, width + 1);
            }
            data->tmpRegInUse[result - xTmp1] = true;

            // result = (var1 << var2) | ((cpu->flags & CF) << (var2 - 1)) | (var1 >> (17 - var2));

            // (var1 << var2)
            data->shiftRegLeftWithReg32(result, reg, tmpReg);

            // (var1 >> (17 - var2))
            U8 tmpReg2 = data->getTmpReg();
            data->subRegFromValue32(tmpReg2, tmpReg, width + 1);

            if (width == 32) {
                data->shiftRegRightWithReg64(tmpReg2, reg, tmpReg2);
            } else {
                data->shiftRegRightWithReg32(tmpReg2, reg, tmpReg2);
            }
            data->orRegs32(result, result, tmpReg2);

            // ((cpu->flags & CF) << (var2 - 1))           
            data->subValue32(xSrc, tmpReg, 1); // we can use xSrc, because there should be no active lazy flags at this point and we need to save tmpReg with the mod value for below if we set cF
            data->andValue32(tmpReg2, xFLAGS, CF);
            data->shiftRegLeftWithReg32(tmpReg2, tmpReg2, xSrc);
            data->orRegs32(result, result, tmpReg2);
            data->releaseTmpReg(tmpReg2);

            // cpu->setCF(((var1 >> (8 - var2)) & 1));
            if (flags & (CF | OF)) { // OF depends on this
                data->subRegFromValue32(tmpReg, tmpReg, width);
                data->shiftRegRightWithReg32(tmpReg, reg, tmpReg);
                data->copyBitsFromSourceAtPositionToDest(xFLAGS, tmpReg, 0, 1);
            }
            // cpu->setOF((cpu->flags & CF) ^ (result >> 7));
            if (flags & OF) {
                data->copyBitsFromSourceAtPositionToDest(tmpReg, result, (width - 1), 1, false);
                data->xorRegs32(tmpReg, tmpReg, xFLAGS);
                data->copyBitsFromSourceToDestAtPosition(xFLAGS, tmpReg, 11, 1); // OF is 0x800 (bit 11)
            }
        });

    data->releaseTmpReg(tmpReg);
    return true;
}

// result and reg are guaranteed to be different regs
bool doRcr(Armv8btAsm* data, U8 result, U8 reg, U32 width) {
    if (data->currentOp->imm == 0) {
        return false;
    }
    U8 tmpReg = data->getTmpReg();

    // cpu->fillFlagsNoCFOF()
    U32 flags = data->flagsNeeded();

    // result = (var1 >> var2) | ((cpu->flags & CF) << (8 - var2)) | (var1 << (9 - var2));
    data->shiftRegRightWithValue32(result, reg, data->currentOp->imm);
    if ((width + 1) - data->currentOp->imm < 32) {
        data->shiftRegLeftWithValue32(tmpReg, reg, (width + 1) - data->currentOp->imm);
        data->orRegs32(result, result, tmpReg);
    }
    data->copyBitsFromSourceToDestAtPosition(result, xFLAGS, (width - data->currentOp->imm), 1, true);

    // cpu->setCF((var1 >> (var2 - 1)) & 1);
    if (flags & CF) {
        data->copyBitsFromSourceAtPositionToDest(xFLAGS, reg, (data->currentOp->imm - 1), 1);
    }
    // cpu->setOF((result ^ (result << 1)) & 0x80);
    if (flags & OF) {
        data->shiftRegLeftWithValue32(tmpReg, result, 1);
        data->xorRegs32(tmpReg, tmpReg, result);
        data->copyBitsFromSourceAtPositionToDest(tmpReg, tmpReg, width - 1, 1, false);
        data->copyBitsFromSourceToDestAtPosition(xFLAGS, tmpReg, 11, 1); // OF is 0x800 (bit 11)
    }
    data->releaseTmpReg(tmpReg);
    return true;
}

// result and reg are guaranteed to be different regs
bool doRcrCl(Armv8btAsm* data, U8 result, U8 reg, U32 width) {
    U8 tmpReg = data->getTmpReg();

    data->andValue32(tmpReg, xECX, 0x1f);
    data->doIf(tmpReg, 0, DO_IF_EQUAL, [data, result, reg, width]() {
        if (result != reg) {
            data->movRegToReg(result, reg, 32, false);
        }
        }, [data, result, reg, tmpReg, width]() {
            U32 flags = data->flagsNeeded();

            // var2 = var2 % 17;
            data->tmpRegInUse[result - xTmp1] = false;
            if (width != 32) {
                data->modValue32(tmpReg, tmpReg, width + 1);
            }
            data->tmpRegInUse[result - xTmp1] = true;

            // result = (var1 >> var2) | ((cpu->flags & CF) << (8 - var2)) | (var1 << (9 - var2));

            // (var1 << var2)
            data->shiftRegRightWithReg32(result, reg, tmpReg);

            // (var1 << (9 - var2))
            U8 tmpReg2 = data->getTmpReg();
            data->subRegFromValue32(tmpReg2, tmpReg, width + 1);

            if (width == 32) {
                data->shiftRegLeftWithReg64(tmpReg2, reg, tmpReg2);
            } else {
                data->shiftRegLeftWithReg32(tmpReg2, reg, tmpReg2);
            }
            data->orRegs32(result, result, tmpReg2);

            // ((cpu->flags & CF) << (8 - var2)) 
            data->releaseTmpReg(tmpReg2); // subRegFromValue32 might need this tmp reg
            data->subRegFromValue32(xSrc, tmpReg, width); // we can use xSrc, because there should be no active lazy flags at this point and we need to save tmpReg with the mod value for below if we set cF
            tmpReg2 = data->getTmpReg();
            data->andValue32(tmpReg2, xFLAGS, CF);
            data->shiftRegLeftWithReg32(tmpReg2, tmpReg2, xSrc);
            data->orRegs32(result, result, tmpReg2);
            data->releaseTmpReg(tmpReg2);

            // cpu->setCF((var1 >> (var2 - 1)) & 1);
            if (flags & CF) {
                data->subValue32(tmpReg, tmpReg, 1);
                data->shiftRegRightWithReg32(tmpReg, reg, tmpReg);
                data->copyBitsFromSourceAtPositionToDest(xFLAGS, tmpReg, 0, 1);
            }
            // cpu->setOF((result ^ (result << 1)) & 0x80);
            if (flags & OF) {
                data->shiftRegLeftWithValue32(tmpReg, result, 1);
                data->xorRegs32(tmpReg, tmpReg, result);
                data->copyBitsFromSourceAtPositionToDest(tmpReg, tmpReg, width - 1, 1, false);
                data->copyBitsFromSourceToDestAtPosition(xFLAGS, tmpReg, 11, 1); // OF is 0x800 (bit 11)
            }
        });

    data->releaseTmpReg(tmpReg);
    return true;
}

void doShiftReg8(Armv8btAsm* data, shiftOp pfn) {
    U8 reg8 = data->getTmpReg();
    U8 result = data->getTmpReg();

    data->movReg8ToReg(data->currentOp->reg, reg8);
    if (pfn(data, result, reg8, 8)) {
        data->movRegToReg8(result, data->currentOp->reg);
    }

    data->releaseTmpReg(reg8);
    data->releaseTmpReg(result);
}

void doShiftReg16(Armv8btAsm* data, shiftOp pfn) {
    U8 reg = data->getTmpReg();
    U8 result = data->getTmpReg();

    data->movRegToReg(reg, data->getNativeReg(data->currentOp->reg), 16, true);
    if (pfn(data, result, reg, 16)) {
        data->movRegToReg(data->getNativeReg(data->currentOp->reg), result, 16, false);
    }

    data->releaseTmpReg(reg);
    data->releaseTmpReg(result);
}

void doShiftReg32(Armv8btAsm* data, shiftOp pfn) {
    U8 result = data->getTmpReg();

    // pfn is allowed to assume result and src will be different regs
    if (pfn(data, result, data->getNativeReg(data->currentOp->reg), 32)) {
        data->movRegToReg(data->getNativeReg(data->currentOp->reg), result, 32, false);
    }

    data->releaseTmpReg(result);
}

void doShiftMemory(Armv8btAsm* data, shiftOp pfn, U32 width) {
    U8 addressReg = data->getAddressReg();
    U32 restartPos = data->bufferPos;
    U8 reg = data->getTmpReg();

    U8 memReg = data->getHostMem(addressReg, width, true);
    data->addRegs64(addressReg, addressReg, memReg);
    data->releaseHostMem(memReg);

    U8 result = data->getTmpReg();

    data->readMemory(addressReg, reg, width, false, data->currentOp->lock != 0);
    if (pfn(data, result, reg, width)) {
        data->writeMemory(addressReg, result, width, false, data->currentOp->lock != 0, reg, restartPos);
    }

    data->releaseTmpReg(reg);
    data->releaseTmpReg(result);
    data->releaseTmpReg(addressReg);
}

typedef void(*shiftRegCl32)(Armv8btAsm* data, U8 dst, U8 src1, U8 cl);

void arithShiftRegCl(Armv8btAsm* data, shiftRegCl32 pfn, Arm8BtFlags* lazyFlags, U32 width, bool regNeedsZeroExtend = false) {
    U8 tmpReg = data->getTmpReg();
    data->andValue32(tmpReg, xECX, 0x1f);
    data->doIf(tmpReg, 0, DO_IF_NOT_EQUAL, [data, tmpReg, width, pfn, lazyFlags, regNeedsZeroExtend] {
            U32 flags;
            bool hardwareFlags, usesSrc, usesDst, usesResult;
            bool resultNeedsZeroExtends = true;
            setupFlagsForArith(data, lazyFlags, flags, hardwareFlags, usesSrc, usesDst, usesResult, resultNeedsZeroExtends);

            U8 readRegDst = setupRegForArith(data, data->currentOp->reg, usesDst || regNeedsZeroExtend, xDst, width);
            if (usesSrc) {
                data->movRegToReg(xSrc, tmpReg, 32, false);
            }
            if (width == 32 && !usesResult) {
                pfn(data, data->getNativeReg(data->currentOp->reg), readRegDst, tmpReg);
            } else {
                pfn(data, xResult, readRegDst, tmpReg);
                writeResultArithReg(data, usesResult, true, resultNeedsZeroExtends, width);
            }

            if (width == 8) {
                data->releaseNativeReg8(readRegDst);
            }
            lazyFlags->setFlags(data, flags);
        });
    data->releaseTmpReg(tmpReg);
}

void arithShiftMemoryCl(Armv8btAsm* data, shiftRegCl32 pfn, Arm8BtFlags* lazyFlags, U32 width) {
    U8 tmpReg = data->getTmpReg();
    data->andValue32(tmpReg, xECX, 0x1f);
    data->doIf(tmpReg, 0, DO_IF_EQUAL, nullptr, [data, tmpReg, width, pfn, lazyFlags] {
        U8 addressReg = data->getAddressReg();
        U32 flags;
        bool hardwareFlags, usesSrc, usesDst, usesResult;
        bool resultNeedsZeroExtends = true;
        setupFlagsForArith(data, lazyFlags, flags, hardwareFlags, usesSrc, usesDst, usesResult, resultNeedsZeroExtends);

        if (usesSrc) {
            data->movRegToReg(xSrc, tmpReg, 32, false);
        }

        U8 memReg = data->getHostMem(addressReg, width, true);
        data->addRegs64(addressReg, addressReg, memReg);
        data->releaseHostMem(memReg);

        // keep the locked read/write loop as small as possible
        U32 restartPos = data->bufferPos;
        data->readMemory(addressReg, xDst, width, false, data->currentOp->lock != 0);
        pfn(data, xResult, xDst, tmpReg);
        data->writeMemory(addressReg, xResult, width, false, data->currentOp->lock != 0, xDst, restartPos);
        data->releaseTmpReg(addressReg);

        lazyFlags->setFlags(data, flags);
        });
    data->releaseTmpReg(tmpReg);
}

void opRolR8I8(Armv8btAsm* data) {
    doShiftReg8(data, doRol);
}
void opRolE8I8(Armv8btAsm* data) {
    doShiftMemory(data, doRol, 8);
}
void opRorR8I8(Armv8btAsm* data) {
    doShiftReg8(data, doRor);
}
void opRorE8I8(Armv8btAsm* data) {
    doShiftMemory(data, doRor, 8);
}
void opRclR8I8(Armv8btAsm* data) {
    doShiftReg8(data, doRcl);
}
void opRclE8I8(Armv8btAsm* data) {
    doShiftMemory(data, doRcl, 8);
}
void opRcrR8I8(Armv8btAsm* data) {
    doShiftReg8(data, doRcr);
}
void opRcrE8I8(Armv8btAsm* data) {
    doShiftMemory(data, doRcr, 8);
}
void opShlR8I8(Armv8btAsm* data) {
    arithRI(data, NULL, shlValue32, ARM8BT_FLAGS_SHL8, 8, true, true);
}
void opShlE8I8(Armv8btAsm* data) {
    arithEI(data, NULL, shlValue32, ARM8BT_FLAGS_SHL8, 8, true, true);
}
void opShrR8I8(Armv8btAsm* data) {
    arithRI(data, NULL, shrValue32, ARM8BT_FLAGS_SHR8, 8, true, true, true);
}
void opShrE8I8(Armv8btAsm* data) {
    arithEI(data, NULL, shrValue32, ARM8BT_FLAGS_SHR8, 8, true, true);
}
void opSarR8I8(Armv8btAsm* data) {
    arithRI(data, NULL, sarValue8, ARM8BT_FLAGS_SAR8, 8, true, true);
}
void opSarE8I8(Armv8btAsm* data) {
    arithEI(data, NULL, sarValue8, ARM8BT_FLAGS_SAR8, 8, true, true);
}

void opRolR16I8(Armv8btAsm* data) {
    doShiftReg16(data, doRol);
}
void opRolE16I8(Armv8btAsm* data) {
    doShiftMemory(data, doRol, 16);
}
void opRorR16I8(Armv8btAsm* data) {
    doShiftReg16(data, doRor);
}
void opRorE16I8(Armv8btAsm* data) {
    doShiftMemory(data, doRor, 16);
}
void opRclR16I8(Armv8btAsm* data) {
    doShiftReg16(data, doRcl);
}
void opRclE16I8(Armv8btAsm* data) {
    doShiftMemory(data, doRcl, 16);
}
void opRcrR16I8(Armv8btAsm* data) {
    doShiftReg16(data, doRcr);
}
void opRcrE16I8(Armv8btAsm* data) {
    doShiftMemory(data, doRcr, 16);
}
void opShlR16I8(Armv8btAsm* data) {
    arithRI(data, NULL, shlValue32, ARM8BT_FLAGS_SHL16, 16, true, true);
}
void opShlE16I8(Armv8btAsm* data) {
    arithEI(data, NULL, shlValue32, ARM8BT_FLAGS_SHL16, 16, true, true);
}
void opShrR16I8(Armv8btAsm* data) {
    arithRI(data, NULL, shrValue32, ARM8BT_FLAGS_SHR16, 16, true, true, true);
}
void opShrE16I8(Armv8btAsm* data) {
    arithEI(data, NULL, shrValue32, ARM8BT_FLAGS_SHR16, 16, true, true);
}
void opSarR16I8(Armv8btAsm* data) {
    arithRI(data, NULL, sarValue16, ARM8BT_FLAGS_SAR16, 16, true, true);
}
void opSarE16I8(Armv8btAsm* data) {
    arithEI(data, NULL, sarValue16, ARM8BT_FLAGS_SAR16, 16, true, true);
}
void opRolR32I8(Armv8btAsm* data) {
    doShiftReg32(data, doRol);
}
void opRolE32I8(Armv8btAsm* data) {
    doShiftMemory(data, doRol, 32);
}
void opRorR32I8(Armv8btAsm* data) {
    doShiftReg32(data, doRor);
}
void opRorE32I8(Armv8btAsm* data) {
    doShiftMemory(data, doRor, 32);
}
void opRclR32I8(Armv8btAsm* data) {
    doShiftReg32(data, doRcl);
}
void opRclE32I8(Armv8btAsm* data) {
    doShiftMemory(data, doRcl, 32);
}
void opRcrR32I8(Armv8btAsm* data) {
    doShiftReg32(data, doRcr);
}
void opRcrE32I8(Armv8btAsm* data) {
    doShiftMemory(data, doRcr, 32);
}
void opShlR32I8(Armv8btAsm* data) {
    arithRI(data, NULL, shlValue32, ARM8BT_FLAGS_SHL32, 32, true, true);
}
void opShlE32I8(Armv8btAsm* data) {
    arithEI(data, NULL, shlValue32, ARM8BT_FLAGS_SHL32, 32, true, true);
}
void opShrR32I8(Armv8btAsm* data) {
    arithRI(data, NULL, shrValue32, ARM8BT_FLAGS_SHR32, 32, true, true);
}
void opShrE32I8(Armv8btAsm* data) {
    arithEI(data, NULL, shrValue32, ARM8BT_FLAGS_SHR32, 32, true, true);
}
void opSarR32I8(Armv8btAsm* data) {
    arithRI(data, NULL, sarValue32, ARM8BT_FLAGS_SAR32, 32, true, true);
}
void opSarE32I8(Armv8btAsm* data) {
    arithEI(data, NULL, sarValue32, ARM8BT_FLAGS_SAR32, 32, true, true);
}
void opRolR8Cl(Armv8btAsm* data) {
    doShiftReg8(data, doRolCl);
}
void opRolE8Cl(Armv8btAsm* data) {
    doShiftMemory(data, doRolCl, 8);
}
void opRorR8Cl(Armv8btAsm* data) {
    doShiftReg8(data, doRorCl);
}
void opRorE8Cl(Armv8btAsm* data) {
    doShiftMemory(data, doRorCl, 8);
}
void opRclR8Cl(Armv8btAsm* data) {
    doShiftReg8(data, doRclCl);
}
void opRclE8Cl(Armv8btAsm* data) {
    doShiftMemory(data, doRclCl, 8);
}
void opRcrR8Cl(Armv8btAsm* data) {
    doShiftReg8(data, doRcrCl);
}
void opRcrE8Cl(Armv8btAsm* data) {
    doShiftMemory(data, doRcrCl, 8);
}
void opShlR8Cl(Armv8btAsm* data) {
    arithShiftRegCl(data, shlRegCl32, ARM8BT_FLAGS_SHL8, 8);
}
void opShlE8Cl(Armv8btAsm* data) {
    arithShiftMemoryCl(data, shlRegCl32, ARM8BT_FLAGS_SHL8, 8);
}
void opShrR8Cl(Armv8btAsm* data) {
    arithShiftRegCl(data, shrRegCl32, ARM8BT_FLAGS_SHR8, 8, true);
}
void opShrE8Cl(Armv8btAsm* data) {
    arithShiftMemoryCl(data, shrRegCl32, ARM8BT_FLAGS_SHR8, 8);
}
void opSarR8Cl(Armv8btAsm* data) {
    arithShiftRegCl(data, sarRegCl8, ARM8BT_FLAGS_SAR8, 8);
}
void opSarE8Cl(Armv8btAsm* data) {
    arithShiftMemoryCl(data, sarRegCl8, ARM8BT_FLAGS_SAR8, 8);
}

void opRolR16Cl(Armv8btAsm* data) {
    doShiftReg16(data, doRolCl);
}
void opRolE16Cl(Armv8btAsm* data) {
    doShiftMemory(data, doRolCl, 16);
}
void opRorR16Cl(Armv8btAsm* data) {
    doShiftReg16(data, doRorCl);
}
void opRorE16Cl(Armv8btAsm* data) {
    doShiftMemory(data, doRorCl, 16);
}
void opRclR16Cl(Armv8btAsm* data) {
    doShiftReg16(data, doRclCl);
}
void opRclE16Cl(Armv8btAsm* data) {
    doShiftMemory(data, doRclCl, 16);
}
void opRcrR16Cl(Armv8btAsm* data) {
    doShiftReg16(data, doRcrCl);
}
void opRcrE16Cl(Armv8btAsm* data) {
    doShiftMemory(data, doRcrCl, 16);
}
void opShlR16Cl(Armv8btAsm* data) {
    arithShiftRegCl(data, shlRegCl32, ARM8BT_FLAGS_SHL16, 16);
}
void opShlE16Cl(Armv8btAsm* data) {
    arithShiftMemoryCl(data, shlRegCl32, ARM8BT_FLAGS_SHL16, 16);
}
void opShrR16Cl(Armv8btAsm* data) {
    arithShiftRegCl(data, shrRegCl32, ARM8BT_FLAGS_SHR16, 16, true);
}
void opShrE16Cl(Armv8btAsm* data) {
    arithShiftMemoryCl(data, shrRegCl32, ARM8BT_FLAGS_SHR16, 16);
}
void opSarR16Cl(Armv8btAsm* data) {
    arithShiftRegCl(data, sarRegCl16, ARM8BT_FLAGS_SAR16, 16);
}
void opSarE16Cl(Armv8btAsm* data) {
    arithShiftMemoryCl(data, sarRegCl16, ARM8BT_FLAGS_SAR16, 16);
}

void opRolR32Cl(Armv8btAsm* data) {
    doShiftReg32(data, doRolCl);
}
void opRolE32Cl(Armv8btAsm* data) {
    doShiftMemory(data, doRolCl, 32);
}
void opRorR32Cl(Armv8btAsm* data) {
    doShiftReg32(data, doRorCl);
}
void opRorE32Cl(Armv8btAsm* data) {
    doShiftMemory(data, doRorCl, 32);
}
void opRclR32Cl(Armv8btAsm* data) {
    doShiftReg32(data, doRclCl);
}
void opRclE32Cl(Armv8btAsm* data) {
    doShiftMemory(data, doRclCl, 32);
}
void opRcrR32Cl(Armv8btAsm* data) {
    doShiftReg32(data, doRcrCl);
}
void opRcrE32Cl(Armv8btAsm* data) {
    doShiftMemory(data, doRcrCl, 32);
}
void opShlR32Cl(Armv8btAsm* data) {
    arithShiftRegCl(data, shlRegCl32, ARM8BT_FLAGS_SHL32, 32);
}
void opShlE32Cl(Armv8btAsm* data) {
    arithShiftMemoryCl(data, shlRegCl32, ARM8BT_FLAGS_SHL32, 32);
}
void opShrR32Cl(Armv8btAsm* data) {
    arithShiftRegCl(data, shrRegCl32, ARM8BT_FLAGS_SHR32, 32);
}
void opShrE32Cl(Armv8btAsm* data) {
    arithShiftMemoryCl(data, shrRegCl32, ARM8BT_FLAGS_SHR32, 32);
}
void opSarR32Cl(Armv8btAsm* data) {
    arithShiftRegCl(data, sarRegCl32, ARM8BT_FLAGS_SAR32, 32);
}
void opSarE32Cl(Armv8btAsm* data) {
    arithShiftMemoryCl(data, sarRegCl32, ARM8BT_FLAGS_SAR32, 32);
}

#endif
