#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
#include "armv8btOps.h"
#include "armv8btCPU.h"

void opPunpcklbwMmx(Armv8btAsm* data) {
    // dest->ub.b7 = src->ub.b3;
    // dest->ub.b6 = dest->ub.b3;
    // dest->ub.b5 = src->ub.b2;
    // dest->ub.b4 = dest->ub.b2;
    // dest->ub.b3 = src->ub.b1;
    // dest->ub.b2 = dest->ub.b1;
    // dest->ub.b1 = src->ub.b0;
    // dest->ub.b0 = dest->ub.b0;
    data->vZipFromLow128(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPunpcklbwE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vZipFromLow128(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPunpcklwdMmx(Armv8btAsm* data) {
    data->vZipFromLow128(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
}
void opPunpcklwdE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vZipFromLow128(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPunpckldqMmx(Armv8btAsm* data) {
    data->vZipFromLow128(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), S2);
}
void opPunpckldqE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vZipFromLow128(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, S2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPacksswbMmx(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();

    if (data->currentOp->reg == data->currentOp->rm) {
        data->vSignedSaturateToSignedNarrowToLowerAndClear(vTmpReg, data->getNativeMmxReg(data->currentOp->reg), H8);
        data->vSignedSaturateToSignedNarrowToLowerAndClear(vTmpReg2, data->getNativeMmxReg(data->currentOp->rm), H8);
        data->vMov32(data->getNativeMmxReg(data->currentOp->reg), 0, vTmpReg, 0);
        data->vMov32(data->getNativeMmxReg(data->currentOp->reg), 1, vTmpReg2, 0);
    } else {
        data->vSignedSaturateToSignedNarrowToLowerAndClear(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), H8);
        data->vSignedSaturateToSignedNarrowToLowerAndClear(vTmpReg2, data->getNativeMmxReg(data->currentOp->rm), H8);
        data->vMov32(data->getNativeMmxReg(data->currentOp->reg), 1, vTmpReg2, 0);
    }
    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(vTmpReg2);
}
void opPacksswbE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSignedSaturateToSignedNarrowToLowerAndClear(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), H8);
    data->vSignedSaturateToSignedNarrowToLowerAndClear(vTmpReg, vTmpReg, H8);
    data->vMov32(data->getNativeMmxReg(data->currentOp->reg), 1, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPcmpgtbMmx(Armv8btAsm* data) {
    data->vCmpGreaterThan(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPcmpgtbE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vCmpGreaterThan(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPcmpgtwMmx(Armv8btAsm* data) {
    data->vCmpGreaterThan(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
}
void opPcmpgtwE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vCmpGreaterThan(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPcmpgtdMmx(Armv8btAsm* data) {
    data->vCmpGreaterThan(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), S2);
}
void opPcmpgtdE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vCmpGreaterThan(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, S2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPackuswbMmx(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();

    if (data->currentOp->reg == data->currentOp->rm) {
        data->vSignedSaturateToUnsignedNarrowToLowerAndClear(vTmpReg, data->getNativeMmxReg(data->currentOp->reg), H8);
        data->vSignedSaturateToUnsignedNarrowToLowerAndClear(vTmpReg2, data->getNativeMmxReg(data->currentOp->rm), H8);
        data->vMov32(data->getNativeMmxReg(data->currentOp->reg), 0, vTmpReg, 0);
        data->vMov32(data->getNativeMmxReg(data->currentOp->reg), 1, vTmpReg2, 0);
    } else {
        data->vSignedSaturateToUnsignedNarrowToLowerAndClear(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), H8);
        data->vSignedSaturateToUnsignedNarrowToLowerAndClear(vTmpReg2, data->getNativeMmxReg(data->currentOp->rm), H8);
        data->vMov32(data->getNativeMmxReg(data->currentOp->reg), 1, vTmpReg2, 0);
    }
    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(vTmpReg2);
}
void opPackuswbE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSignedSaturateToUnsignedNarrowToLowerAndClear(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), H8);
    data->vSignedSaturateToUnsignedNarrowToLowerAndClear(vTmpReg, vTmpReg, H8);
    data->vMov32(data->getNativeMmxReg(data->currentOp->reg), 1, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPunpckhbwMmx(Armv8btAsm* data) {
    // dest->ub.b0 = dest->ub.b4;
    // dest->ub.b1 = src->ub.b4;
    // dest->ub.b2 = dest->ub.b5;
    // dest->ub.b3 = src->ub.b5;
    // dest->ub.b4 = dest->ub.b6;
    // dest->ub.b5 = src->ub.b6;
    // dest->ub.b6 = dest->ub.b7;
    // dest->ub.b7 = src->ub.b7;

    data->vZipFromLow128(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B16);
    data->vMov64ToScaler(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), 1);
}
void opPunpckhbwE64(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, tmpReg, true);
    data->vZipFromLow128(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), tmpReg, B16);
    data->vMov64ToScaler(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), 1);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}
void opPunpckhwdMmx(Armv8btAsm* data) {
    data->vZipFromLow128(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H8);
    data->vMov64ToScaler(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), 1);
}
void opPunpckhwdE64(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, tmpReg, true);
    data->vZipFromLow128(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), tmpReg, H8);
    data->vMov64ToScaler(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), 1);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}
void opPunpckhdqMmx(Armv8btAsm* data) {
    data->vZipFromLow128(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), S4);
    data->vMov64ToScaler(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), 1);
}
void opPunpckhdqE64(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, tmpReg, true);
    data->vZipFromLow128(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), tmpReg, S4);
    data->vMov64ToScaler(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), 1);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}
void opPackssdwMmx(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();

    if (data->currentOp->reg == data->currentOp->rm) {
        data->vSignedSaturateToSignedNarrowToLowerAndClear(vTmpReg, data->getNativeMmxReg(data->currentOp->reg), S4);
        data->vSignedSaturateToSignedNarrowToLowerAndClear(vTmpReg2, data->getNativeMmxReg(data->currentOp->rm), S4);
        data->vMov32(data->getNativeMmxReg(data->currentOp->reg), 0, vTmpReg, 0);
        data->vMov32(data->getNativeMmxReg(data->currentOp->reg), 1, vTmpReg2, 0);
    } else {
        data->vSignedSaturateToSignedNarrowToLowerAndClear(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), S4);
        data->vSignedSaturateToSignedNarrowToLowerAndClear(vTmpReg2, data->getNativeMmxReg(data->currentOp->rm), S4);
        data->vMov32(data->getNativeMmxReg(data->currentOp->reg), 1, vTmpReg2, 0);
    }
    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(vTmpReg2);
}
void opPackssdwE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSignedSaturateToSignedNarrowToLowerAndClear(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), S4);
    data->vSignedSaturateToSignedNarrowToLowerAndClear(vTmpReg, vTmpReg, S4);
    data->vMov32(data->getNativeMmxReg(data->currentOp->reg), 1, vTmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opMovPqR32(Armv8btAsm* data) {
    // MMX_reg* rmrq = &cpu->reg_mmx[r1];
    // rmrq->ud.d0 = cpu->reg[r2].u32;
    // rmrq->ud.d1 = 0;
    data->vLoadConst(data->getNativeMmxReg(data->currentOp->reg), 0, B16);
    data->vMovFromGeneralReg32(data->getNativeMmxReg(data->currentOp->reg), 0, data->getNativeReg(data->currentOp->rm));
}
void opMovPqE32(Armv8btAsm* data) {
    // MMX_reg* rmrq = &cpu->reg_mmx[reg];
    // rmrq->ud.d0 = readd(address);
    // rmrq->ud.d1 = 0;
    U8 addressReg = data->getAddressReg();
    data->vReadMemory32(addressReg, data->getNativeMmxReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opMovPqMmx(Armv8btAsm* data) {
    // cpu->reg_mmx[r1].q = cpu->reg_mmx[r2].q;
    data->vMov64(data->getNativeMmxReg(data->currentOp->reg), 0, data->getNativeMmxReg(data->currentOp->rm), 0);
}
void opMovPqE64(Armv8btAsm* data) {
    // cpu->reg_mmx[reg].q = readq(address);
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, data->getNativeMmxReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
}
void opPsrlw(Armv8btAsm* data) {
    // dest->uw.w0 >>= imm;
    // dest->uw.w1 >>= imm;
    // dest->uw.w2 >>= imm;
    // dest->uw.w3 >>= imm;
    data->vShiftRightValue(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->currentOp->imm, H8);
}
void opPsraw(Armv8btAsm* data) {
    data->vSignedShiftRightValue(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->currentOp->imm, H8);
}
void opPsllw(Armv8btAsm* data) {
    data->vShiftLeftValue(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->currentOp->imm, H8);
}
void opPsrld(Armv8btAsm* data) {
    data->vShiftRightValue(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->currentOp->imm, S4);
}
void opPsrad(Armv8btAsm* data) {
    data->vSignedShiftRightValue(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->currentOp->imm, S4);
}
void opPslld(Armv8btAsm* data) {
    data->vShiftLeftValue(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->currentOp->imm, S4);
}
void opPsrlq(Armv8btAsm* data) {
    data->vShiftRightValue(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->currentOp->imm, D2);
}
void opPsllq(Armv8btAsm* data) {
    data->vShiftLeftValue(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->currentOp->imm, D2);
}
void opPcmpeqbMmx(Armv8btAsm* data) {
    data->vCmpEqual(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B16);
}
void opPcmpeqbE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vCmpEqual(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B16);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPcmpeqwMmx(Armv8btAsm* data) {
    data->vCmpEqual(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H8);
}
void opPcmpeqwE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vCmpEqual(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPcmpeqdMmx(Armv8btAsm* data) {
    data->vCmpEqual(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), S4);
}
void opPcmpeqdE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vCmpEqual(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opMovR32Pq(Armv8btAsm* data) {
    // cpu->reg[r1].u32 = cpu->reg_mmx[r2].ud.d0;
    data->vMovToGeneralReg32ZeroExtend(data->getNativeReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), 0, S4);
}
void opMovE32Pq(Armv8btAsm* data) {
    // writed(address, cpu->reg_mmx[reg].ud.d0);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory32(addressReg, data->getNativeMmxReg(data->currentOp->reg), 0, true);
    data->releaseTmpReg(addressReg);
}
void opMovMmxPq(Armv8btAsm* data) {
    // cpu->reg_mmx[r1].q = cpu->reg_mmx[r2].q;
    data->vMov64(data->getNativeMmxReg(data->currentOp->reg), 0, data->getNativeMmxReg(data->currentOp->rm), 0);
}
void opMovE64Pq(Armv8btAsm* data) {
    // writeq(address, cpu->reg_mmx[reg].q);
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory64(addressReg, data->getNativeMmxReg(data->currentOp->reg), 0, true);
    data->releaseTmpReg(addressReg);
}
void opPsrlwMmx(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeMmxReg(data->currentOp->rm), 0, H4);
    data->vNeg(vTmpReg, vTmpReg, H4);
    data->vShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrlwE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, H4);
    data->vNeg(vTmpReg, vTmpReg, H4);
    data->vShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrldMmx(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeMmxReg(data->currentOp->rm), 0, S2);
    data->vNeg(vTmpReg, vTmpReg, S2);
    data->vShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, S2);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrldE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, S2);
    data->vNeg(vTmpReg, vTmpReg, S2);
    data->vShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, S2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrlqMmx(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vNeg(vTmpReg, data->getNativeMmxReg(data->currentOp->rm), D2);
    data->vShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, D2);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrlqE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vNeg(vTmpReg, vTmpReg, D_scaler);
    data->vShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, D_scaler);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPmullwMmx(Armv8btAsm* data) {
    data->vMul(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
}
void opPmullwE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vMul(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opPsubusbMmx(Armv8btAsm* data) {
    data->vUnsignedSaturatingSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPsubusbE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vUnsignedSaturatingSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubuswMmx(Armv8btAsm* data) {
    data->vUnsignedSaturatingSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
}
void opPsubuswE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vUnsignedSaturatingSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opPandMmx(Armv8btAsm* data) {
    data->vAnd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPandE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vAnd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPaddusbMmx(Armv8btAsm* data) {
    data->vUnsignedSaturatingAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPaddusbE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vUnsignedSaturatingAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPadduswMmx(Armv8btAsm* data) {
    data->vUnsignedSaturatingAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
}
void opPadduswE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vUnsignedSaturatingAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPandnMmx(Armv8btAsm* data) {
    data->vAndNot(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), data->getNativeMmxReg(data->currentOp->reg), B8);
}
void opPandnE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vAndNot(data->getNativeMmxReg(data->currentOp->reg), vTmpReg, data->getNativeMmxReg(data->currentOp->reg), B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opPsrawMmx(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeMmxReg(data->currentOp->rm), 0, H4);
    data->vNeg(vTmpReg, vTmpReg, H4);
    data->vSignedShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsrawE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, H4);
    data->vNeg(vTmpReg, vTmpReg, H4);
    data->vSignedShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opPsradMmx(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeMmxReg(data->currentOp->rm), 0, S2);
    data->vNeg(vTmpReg, vTmpReg, S2);
    data->vSignedShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, S2);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsradE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, S2);
    data->vNeg(vTmpReg, vTmpReg, S2);
    data->vSignedShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, S2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPmulhwMmx(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vSignedMulLongLower(vTmpReg, data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
    data->vShiftRightValueAndNarrow(data->getNativeMmxReg(data->currentOp->reg), vTmpReg, 16, S4);
    data->vReleaseTmpReg(vTmpReg);
}
void opPmulhwE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSignedMulLongLower(vTmpReg, data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->vShiftRightValueAndNarrow(data->getNativeMmxReg(data->currentOp->reg), vTmpReg, 16, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opPsubsbMmx(Armv8btAsm* data) {
    data->vSignedSaturatingSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPsubsbE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSignedSaturatingSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubswMmx(Armv8btAsm* data) {
    data->vSignedSaturatingSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
}
void opPsubswE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSignedSaturatingSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPorMmx(Armv8btAsm* data) {
    data->vOr(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPorE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vOr(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPaddsbMmx(Armv8btAsm* data) {
    data->vSignedSaturatingAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPaddsbE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSignedSaturatingAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPaddswMmx(Armv8btAsm* data) {
    data->vSignedSaturatingAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
}
void opPaddswE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSignedSaturatingAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPxorMmx(Armv8btAsm* data) {
    data->vXor(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPxorE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vXor(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opPsllwMmx(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeMmxReg(data->currentOp->rm), 0, H4);
    data->vShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsllwE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, H4);
    data->vShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opPslldMmx(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vDup(vTmpReg, data->getNativeMmxReg(data->currentOp->rm), 0, S2);
    data->vShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, S2);
    data->vReleaseTmpReg(vTmpReg);
}
void opPslldE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vDup(vTmpReg, vTmpReg, 0, S2);
    data->vShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, S2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsllqMmx(Armv8btAsm* data) {
    data->vShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), D_scaler);
}
void opPsllqE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vShiftWithReg(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, D_scaler);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPmaddwdMmx(Armv8btAsm* data) {
    U8 vTmpReg1 = data->vGetTmpReg();
    // multiply bottom 4x 16-bit numbers and put them into 4x 32-bit number in vTmpReg1
    data->vSignedMulLongLower(vTmpReg1, data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
    data->vAddPairs(data->getNativeMmxReg(data->currentOp->reg), vTmpReg1, vTmpReg1, S4);
    // :TODO: is there a reason to clear out the upper 64-bits?
    data->vReleaseTmpReg(vTmpReg1);
}
void opPmaddwdE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg2, true);
    data->vSignedMulLongLower(vTmpReg1, data->getNativeMmxReg(data->currentOp->reg), vTmpReg2, H4);
    data->vAddPairs(data->getNativeMmxReg(data->currentOp->reg), vTmpReg1, vTmpReg2, S4);
    // :TODO: is there a reason to clear out the upper 64-bits?
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vTmpReg2);
}

void opPsubbMmx(Armv8btAsm* data) {
    data->vSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPsubbE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubwMmx(Armv8btAsm* data) {
    data->vSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
}
void opPsubwE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubdMmx(Armv8btAsm* data) {
    data->vSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), S2);
}
void opPsubdE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, S2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPaddbMmx(Armv8btAsm* data) {
    data->vAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPaddbE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPaddwMmx(Armv8btAsm* data) {
    data->vAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
}
void opPaddwE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPadddMmx(Armv8btAsm* data) {
    data->vAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), S2);
}
void opPadddE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, S2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPavgbMmxMmx(Armv8btAsm* data) {
    data->vUnsignedRoundedAverage(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPavgbMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vUnsignedRoundedAverage(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPavgwMmxMmx(Armv8btAsm* data) {
    data->vUnsignedRoundedAverage(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
}
void opPavgwMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vUnsignedRoundedAverage(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsadbwMmxMmx(Armv8btAsm* data) {
    data->vUnsignedAbsoluteDifference(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
    data->vUnsignedAddAcrossVectLong(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), B8);
}
void opPsadbwMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vUnsignedAbsoluteDifference(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->vUnsignedAddAcrossVectLong(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPextrwR32Mmx(Armv8btAsm* data) {
    data->vMovToGeneralReg32ZeroExtend(data->getNativeReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), data->currentOp->imm & 3, H8);
}
void opPextrwE16Mmx(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory16(addressReg, data->getNativeMmxReg(data->currentOp->reg), data->currentOp->imm & 3, true);
    data->releaseTmpReg(addressReg);
}
void opPinsrwMmxR32(Armv8btAsm* data) {
    // cpu->reg_mmx[r1].uw[data->imm] = cpu->reg[r2].u16
    data->vMovFromGeneralReg16(data->getNativeMmxReg(data->currentOp->reg), data->currentOp->imm & 3, data->getNativeReg(data->currentOp->rm));
}
void opPinsrwMmxE16(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vReadMemory16(addressReg, data->getNativeMmxReg(data->currentOp->reg), data->currentOp->imm & 3, true);
    data->releaseTmpReg(addressReg);
}
void opPmaxswMmxMmx(Armv8btAsm* data) {
    data->vSignedMax(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
}
void opPmaxswMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSignedMax(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPmaxubMmxMmx(Armv8btAsm* data) {
    data->vUnsignedMax(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPmaxubMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vUnsignedMax(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPminswMmxMmx(Armv8btAsm* data) {
    data->vSignedMin(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
}
void opPminswMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSignedMin(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPminubMmxMmx(Armv8btAsm* data) {
    data->vUnsignedMin(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), B8);
}
void opPminubMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vUnsignedMin(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPmovmskbR32Mmx(Armv8btAsm* data) {
    // for all 8 bytes set a bit in a mask if it is signed
    U8 vTmpReg = data->vGetTmpReg();

    U8 bitMaskReg = data->getSSEConstant(SSE_BYTE8_BIT_MASK);
    // turn all the bits to 1 if signed
    data->vSignedShiftRightValue(vTmpReg, data->getNativeMmxReg(data->currentOp->rm), 7, B8);
    // mask out the bit that should be set, so index 0 will set bit 0, index 1 will set bit 1, etc
    data->vAnd(vTmpReg, vTmpReg, bitMaskReg, B8);
    // add bits 0-7 for indexes 0-7 to end up with the mask
    data->vAddAcrossVectorToScaler(vTmpReg, vTmpReg, B8);
    data->vMovToGeneralReg32ZeroExtend(data->getNativeReg(data->currentOp->reg), vTmpReg, 0, B16);

    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(bitMaskReg);
}
void opPmulhuwMmxMmx(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    data->vUnsignedMulLongLower(vTmpReg, data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), H4);
    data->vShiftRightValueAndNarrow(data->getNativeMmxReg(data->currentOp->reg), vTmpReg, 16, S4);
    data->vReleaseTmpReg(vTmpReg);
}
void opPmulhuwMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vUnsignedMulLongLower(vTmpReg, data->getNativeMmxReg(data->currentOp->reg), vTmpReg, H4);
    data->vShiftRightValueAndNarrow(data->getNativeMmxReg(data->currentOp->reg), vTmpReg, 16, S4);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opMaskmovqEDIMmxMmx(Armv8btAsm* data) {
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

    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSignedShiftRightValue(vTmpRegMask, data->getNativeMmxReg(data->currentOp->rm), 7, B8);
    data->vSelectBit(vTmpRegMask, data->getNativeMmxReg(data->currentOp->reg), vTmpReg, B8);
    data->vWriteMemory64(addressReg, vTmpRegMask, 0, true);

    if (needToReleaseAddressReg) {
        data->releaseTmpReg(addressReg);
    }
    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(vTmpRegMask);
}
void opMovntqE64Mmx(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    data->vWriteMemory64(addressReg, data->getNativeMmxReg(data->currentOp->reg), 0, true);
    data->releaseTmpReg(addressReg);
}
void opPaddqMmxMmx(Armv8btAsm* data) {
    data->vAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), D2);
}
void opPaddqMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vAdd(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, D2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPsubqMmxMmx(Armv8btAsm* data) {
    data->vSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), D_scaler);
}
void opPsubqMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vSub(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, D_scaler);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opPmuludqMmxMmx(Armv8btAsm* data) {
    // cpu->reg_mmx[r1].q = (U64)cpu->reg_mmx[r1].ud.d0 * (U64)cpu->reg_mmx[r2].ud.d0;
    data->vUnsignedMulLongLower(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), S2);
}
void opPmuludqMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    data->vUnsignedMulLongLower(data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->reg), vTmpReg, S2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opMovdq2qMmxXmm(Armv8btAsm* data) {
    data->vMov64ToScaler(data->getNativeMmxReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), 0);
}
void opMovq2dqXmmMmx(Armv8btAsm* data) {
    data->vMov64ToScaler(data->getNativeSseReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), 0);
}
#endif