#include "boxedwine.h"
#include "armv8btAsm.h"

#ifdef BOXEDWINE_ARMV8BT

static void pshuflw_dst_noequal_src(Armv8btAsm* data, U8 dst, U8 src, U8 mask) {
    data->vMov16(dst, 0, src, mask & 3);
    data->vMov16(dst, 1, src, (mask >> 2) & 3);
    data->vMov16(dst, 2, src, (mask >> 4) & 3);
    data->vMov16(dst, 3, src, (mask >> 6) & 3);
}

static void pshufhw_dst_noequal_src(Armv8btAsm* data, U8 dst, U8 src, U8 mask) {
    data->vMov16(dst, 4, src, 4+(mask & 3));
    data->vMov16(dst, 5, src, 4+((mask >> 2) & 3));
    data->vMov16(dst, 6, src, 4+((mask >> 4) & 3));
    data->vMov16(dst, 7, src, 4+((mask >> 6) & 3));
}

static void pshufd_dst_noequal_src(Armv8btAsm* data, U8 dst, U8 src, U8 mask) {
    data->vMov32(dst, 0, src, mask & 3);
    data->vMov32(dst, 1, src, (mask >> 2) & 3);
    data->vMov32(dst, 2, src, (mask >> 4) & 3);
    data->vMov32(dst, 3, src, (mask >> 6) & 3);
}

static void interleave_shufq(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, U8 mask) {
    data->vMov64(dst, 0, src1, mask & 1);
    data->vMov64(dst, 1, src2, (mask >> 1) & 1);
}

static void interleave_shufd(Armv8btAsm* data, U8 dst, U8 src1, U8 src2, U8 mask) {
    U8 index1 = mask & 3;
    U8 index2 = (mask >> 2) & 3;
    U8 index3 = (mask >> 4) & 3;
    U8 index4 = (mask >> 6) & 3;

    if (index2 == 0 && index1 != 0 && dst == src1) {
        if (index1 == 1) {
            // swap
            U8 tmp = data->vGetTmpReg();
            data->vMov32(tmp, 0, src1, index1);
            data->vMov32(dst, 1, src1, index2);
            data->vMov32(dst, 0, tmp, 0);
            data->releaseTmpReg(tmp);
        } else {
            data->vMov32(dst, 1, src1, index2);
            data->vMov32(dst, 0, src1, index1);
        }
    } else {
        data->vMov32(dst, 0, src1, index1);
        data->vMov32(dst, 1, src1, index2);
    }

    data->vMov32(dst, 2, src2, index3);
    data->vMov32(dst, 3, src2, index4);
}

void opPshufwMmxMmx(Armv8btAsm* data) {
    if (data->currentOp->reg == data->currentOp->rm) {
        U8 vTmpReg = data->vGetTmpReg();
        pshuflw_dst_noequal_src(data, vTmpReg, data->getNativeMmxReg(data->currentOp->rm), data->currentOp->imm);
        data->vMov64ToScaler(data->getNativeMmxReg(data->currentOp->reg), vTmpReg, 0);
        data->vReleaseTmpReg(vTmpReg);
    } else {
        pshuflw_dst_noequal_src(data, data->getNativeMmxReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), data->currentOp->imm);
    }
}
void opPshufwMmxE64(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory64(addressReg, tmpReg, true);
    pshuflw_dst_noequal_src(data, data->getNativeMmxReg(data->currentOp->reg), tmpReg, data->currentOp->imm);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}

void opPshufdXmmXmm(Armv8btAsm* data) {
    if (data->currentOp->reg == data->currentOp->rm) {
        U8 vTmpReg = data->vGetTmpReg();
        pshufd_dst_noequal_src(data, vTmpReg, data->getNativeSseReg(data->currentOp->rm), data->currentOp->imm);
        data->vMov128(data->getNativeSseReg(data->currentOp->reg), vTmpReg);
        data->vReleaseTmpReg(vTmpReg);
    } else {
        pshufd_dst_noequal_src(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), data->currentOp->imm);
    }
}

void opPshufdXmmE128(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    pshufd_dst_noequal_src(data, data->getNativeSseReg(data->currentOp->reg), tmpReg, data->currentOp->imm);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}

void opPshufhwXmmXmm(Armv8btAsm* data) {
    if (data->currentOp->reg == data->currentOp->rm) {
        U8 vTmpReg = data->vGetTmpReg();
        pshufhw_dst_noequal_src(data, vTmpReg, data->getNativeSseReg(data->currentOp->rm), data->currentOp->imm);
        data->vMov64ToScaler(data->getNativeSseReg(data->currentOp->reg), vTmpReg, 0);
        data->vReleaseTmpReg(vTmpReg);
    } else {
        pshufhw_dst_noequal_src(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), data->currentOp->imm);
        data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, data->getNativeSseReg(data->currentOp->rm), 0);
    }
}

void opPshufhwXmmE128(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    pshufhw_dst_noequal_src(data, data->getNativeSseReg(data->currentOp->reg), tmpReg, data->currentOp->imm);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, tmpReg, 0);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}

void opPshuflwXmmXmm(Armv8btAsm* data) {
    if (data->currentOp->reg == data->currentOp->rm) {
        U8 vTmpReg = data->vGetTmpReg();
        pshuflw_dst_noequal_src(data, vTmpReg, data->getNativeSseReg(data->currentOp->rm), data->currentOp->imm);
        data->vMov64ToScaler(data->getNativeSseReg(data->currentOp->reg), vTmpReg, 0);
        data->vReleaseTmpReg(vTmpReg);
    } else {
        pshuflw_dst_noequal_src(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), data->currentOp->imm);
        data->vMov64(data->getNativeSseReg(data->currentOp->reg), 1, data->getNativeSseReg(data->currentOp->rm), 1);
    }
}

void opPshuflwXmmE128(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    pshuflw_dst_noequal_src(data, data->getNativeSseReg(data->currentOp->reg), tmpReg, data->currentOp->imm);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 1, tmpReg, 1);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}

void opShufpdXmmXmm(Armv8btAsm* data) {
    // r.f64[0] = ((imm8 & 1) == 0) ? a.f64[0] : a.f64[1];
    // r.f64[1] = ((imm8 & 2) == 0) ? b.f64[0] : b.f64[1];
    interleave_shufq(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), data->currentOp->imm);
}
void opShufpdXmmE128(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    interleave_shufq(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), tmpReg, data->currentOp->imm);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}

void opShufpsXmmXmm(Armv8btAsm* data) {
    interleave_shufd(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), data->currentOp->imm);
}
void opShufpsXmmE128(Armv8btAsm* data) {
    U8 tmpReg = data->vGetTmpReg();
    U8 addressReg = data->getAddressReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    interleave_shufd(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), tmpReg, data->currentOp->imm);
    data->vReleaseTmpReg(tmpReg);
    data->releaseTmpReg(addressReg);
}

#endif