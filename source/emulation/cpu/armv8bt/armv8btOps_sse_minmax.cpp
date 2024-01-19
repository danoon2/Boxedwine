#include "boxedwine.h"
#include "armv8btAsm.h"

#ifdef BOXEDWINE_ARMV8BT
static void fmin(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, VectorWidth width) {
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();

    if (dst == src2) {
        kpanic("fmin: dst!=src");
    }
    // get the min values
    data->fMin(vTmpReg, src1, src2, width);

    // see if they are equal (like -0.0f == 0.0f) or 
    // if they are neither greather than or less than (because NaN was used)
    // in both those cases, SSE says to use the one on the right
    data->fCmpGreaterThan(vTmpReg2, src2, src1, width);
    data->fCmpGreaterThan(dst, src1, src2, width);
    data->vOr(dst, dst, vTmpReg2, (width == S_scaler || width == D_scaler)?B8:B16);

    // if the above comparisons are true, then 1's will be in reg, other wise 0's
    // when doing this call, the min values will be used as calculated for the 1's
    // for the 0's, the right side will be used (nan's and equal values)
    data->vSelectBit(dst, vTmpReg, src2, (width == S_scaler || width == D_scaler) ? B8 : B16);
    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(vTmpReg2);
}

static void fmax(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, VectorWidth width) {
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();

    if (dst == src2) {
        kpanic("fmax: dst!=src");
    }
    // get the min values
    data->fMax(vTmpReg, src1, src2, width);

    // see if they are equal (like -0.0f == 0.0f) or 
    // if they are neither greather than or less than (because NaN was used)
    // in both those cases, SSE says to use the one on the right
    data->fCmpGreaterThan(vTmpReg2, src1, src2, width);
    data->fCmpGreaterThan(dst, src2, src1, width);
    data->vOr(dst, dst, vTmpReg2, (width == S_scaler || width == D_scaler) ? B8 : B16);

    // if the above comparisons are true, then 1's will be in reg, other wise 0's
    // when doing this call, the min values will be used as calculated for the 1's
    // for the 0's, the right side will be used (nan's and equal values)
    data->vSelectBit(dst, vTmpReg, src2, (width == S_scaler || width == D_scaler) ? B8 : B16);
    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(vTmpReg2);
}

void opMaxpsXmm(Armv8btAsm* data) {
    if (data->currentOp->reg == data->currentOp->rm) {
        U8 vTmpReg = data->vGetTmpReg();
        fmax(data, vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
        data->vMov128(data->getNativeSseReg(data->currentOp->reg), vTmpReg);
        data->vReleaseTmpReg(vTmpReg);
    } else {
        fmax(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
    }
}
void opMaxpsE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    fmax(data, vTmpReg2, data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->vMov128(data->getNativeSseReg(data->currentOp->reg), vTmpReg2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(vTmpReg2);
}
void opMaxssXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    fmax(data, vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opMaxssE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory32(addressReg, vTmpReg, true);
    fmax(data, vTmpReg2, data->getNativeSseReg(data->currentOp->reg), vTmpReg, S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg2, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opMinpsXmm(Armv8btAsm* data) {
    if (data->currentOp->reg == data->currentOp->rm) {
        U8 vTmpReg = data->vGetTmpReg();
        fmin(data, vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
        data->vMov128(data->getNativeSseReg(data->currentOp->reg), vTmpReg);
        data->vReleaseTmpReg(vTmpReg);
    } else {
        fmin(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
    }
}
void opMinpsE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    fmin(data, vTmpReg2, data->getNativeSseReg(data->currentOp->reg), vTmpReg, S4);
    data->vMov128(data->getNativeSseReg(data->currentOp->reg), vTmpReg2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(vTmpReg2);
}

void opMinssXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    fmin(data, vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opMinssE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory32(addressReg, vTmpReg, true);
    fmin(data, vTmpReg2, data->getNativeSseReg(data->currentOp->reg), vTmpReg, S_scaler);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg2, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opMaxpdXmmXmm(Armv8btAsm* data) {
    if (data->currentOp->reg == data->currentOp->rm) {
        U8 vTmpReg = data->vGetTmpReg();
        fmax(data, vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
        data->vMov128(data->getNativeSseReg(data->currentOp->reg), vTmpReg);
        data->vReleaseTmpReg(vTmpReg);
    } else {
        fmax(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
    }
}
void opMaxpdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    fmax(data, vTmpReg2, data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->vMov128(data->getNativeSseReg(data->currentOp->reg), vTmpReg2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(vTmpReg2);
}
void opMaxsdXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    fmax(data, vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opMaxsdXmmE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    fmax(data, vTmpReg2, data->getNativeSseReg(data->currentOp->reg), vTmpReg, D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg2, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opMinpdXmmXmm(Armv8btAsm* data) {
    if (data->currentOp->reg == data->currentOp->rm) {
        U8 vTmpReg = data->vGetTmpReg();
        fmin(data, vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
        data->vMov128(data->getNativeSseReg(data->currentOp->reg), vTmpReg);
        data->vReleaseTmpReg(vTmpReg);
    } else {
        fmin(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D2);
    }
}
void opMinpdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    fmin(data, vTmpReg2, data->getNativeSseReg(data->currentOp->reg), vTmpReg, D2);
    data->vMov128(data->getNativeSseReg(data->currentOp->reg), vTmpReg2);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
    data->vReleaseTmpReg(vTmpReg2);
}
void opMinsdXmmXmm(Armv8btAsm* data) {
    U8 vTmpReg = data->vGetTmpReg();
    fmin(data, vTmpReg, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg, 0);
    data->vReleaseTmpReg(vTmpReg);
}
void opMinsdXmmE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    fmin(data, vTmpReg2, data->getNativeSseReg(data->currentOp->reg), vTmpReg, D_scaler);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, vTmpReg2, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

#endif