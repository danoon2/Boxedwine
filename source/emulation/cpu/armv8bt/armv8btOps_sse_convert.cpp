#include "boxedwine.h"
#include "armv8btAsm.h"

#ifdef BOXEDWINE_ARMV8BT
// When converting Double/Float to int32, if the value doesn't fit, SSE convert the value to 0x80000000, but ARM converts it to 0x7FFFFFFF,
// so we have to spend a lot of effort to correct that
static void cvtps2piXmm(Armv8btAsm* data, U8 to, U8 from, bool truncate) {
    // cpu->reg_mmx[reg].sd.d0 = cpu->xmm[rm].pd.f32[0];
    // cpu->reg_mmx[reg].sd.d1 = cpu->xmm[rm].pd.f32[1];

    //if (cpu->xmm[rm].pd.f32[0] >= 2147483648.0 || cpu->xmm[rm].pd.f32[0] <= -2147483649.0) {
    //    cpu->reg_mmx[reg].sd.d0 = 0x80000000;
    //} else {
    //    cpu->reg_mmx[reg].sd.d0 = (int32_t)cpu->xmm[rm].pd.f32[0];
    //}    

    // :TODO: I'm not sure if it is necessary to set the top 2 floats to 0 before doing the conversions
    //U8 vTmpReg = data->vGetTmpReg();
    //data->vLoadConst(vTmpReg, 0, 4);
    //data->vMov64(vTmpReg, 0, from, 0);
    //from = vTmpReg;

    U8 vPlusOne = data->getSSEConstant(SSE_MAX_INT32_PLUS_ONE_AS_FLOAT);
    U8 vMinusOne = data->getSSEConstant(SSE_MIN_INT32_MINUS_ONE_AS_FLOAT);

    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();

    // will set lane to all 1's if value is too large or too small to convert
    data->fCmpGreaterThanOrEqual(vTmpReg1, from, vPlusOne, S4);
    data->fCmpGreaterThanOrEqual(vTmpReg2, vMinusOne, from, S4);
    data->vOr(vTmpReg1, vTmpReg1, vTmpReg2, B16);

    // do the convert from float to int32
    if (truncate) {
        data->vConvertFloatToInt32RoundToZero(vTmpReg2, from, true);
    } else {
        data->vConvertFloatToInt32RoundToCurrentMode(vTmpReg2, from, true);
    }

    // clear out int32's if was too large or too small
    data->vAndNot(vTmpReg2, vTmpReg2, vTmpReg1, B16);

    // if the value was too large or too smaller, then vTmpReg1 contains all 1's.  
    // Right shift 31, left shift by 31 to create 0x80000000
    data->vShiftRightValue(vTmpReg1, vTmpReg1, 31, S4);
    data->vShiftLeftValue(vTmpReg1, vTmpReg1, 31, S4);

    // this will be 0x80000000 or 0x00000000 depending on if the value was too large or too small
    data->vOr(to, vTmpReg2, vTmpReg1, B16);

    data->vReleaseTmpReg(vTmpReg2);
    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vPlusOne);
    data->vReleaseTmpReg(vMinusOne);
}

static void cvtsd2siR32Xmm(Armv8btAsm* data, U8 to, U8 from, bool truncate) {
    // cpu->reg[r1].u32 = cpu->xmm[rm].pd.f64[0];

    //if (cpu->xmm[rm].pd.f64[0] >= 2147483648.0 || cpu->xmm[rm].pd.f64[0] <= -2147483649.0) {
    //    cpu->reg[r1].u32 = 0x80000000;
    //} else {
    //    cpu->reg[r1].u32 = (int32_t)cpu->xmm[rm].pd.f64[0];
    //}    

    U8 vPlusOne = data->getSSEConstant(SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE);
    U8 vMinusOne = data->getSSEConstant(SSE_MIN_INT32_MINUS_ONE_AS_DOUBLE);

    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    U8 tmpReg = data->getTmpReg();
    U8 tmpReg2 = data->getTmpReg();

    // will set lane to all 1's if value is too large or too small to convert
    data->fCmpGreaterThanOrEqual(vTmpReg1, from, vPlusOne, D_scaler);
    data->fCmpGreaterThanOrEqual(vTmpReg2, vMinusOne, from, D_scaler);
    data->vOr(vTmpReg1, vTmpReg1, vTmpReg2, B8);

    data->vMovToGeneralReg32ZeroExtend(tmpReg, vTmpReg1, 0, S4); // vTmpReg1 is all 0's or all 1's, so no conversions are necessary

    // do the convert from double to int64
    if (truncate) {
        data->vConvertDoubleToGeneralReg32RoundToZero(tmpReg2, from);
    } else {
        data->vConvertDoubleToGeneralReg32RoundToCurrentMode(tmpReg2, from);
    }

    // clear out int32 if was too large or too small
    data->andNotRegs32(tmpReg2, tmpReg2, tmpReg);

    // if the value was too large or too smaller, then tmpReg contains all 1's.  
    data->andValue32(tmpReg, tmpReg, 0x80000000); // will be 0x80000000 if all 1's, else 0

    // will be good value or'd with 0, or bad value (0), or'd with 0x80000000
    data->orRegs32(to, tmpReg2, tmpReg);
    data->releaseTmpReg(tmpReg);
    data->releaseTmpReg(tmpReg2);
    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vTmpReg2);
    data->vReleaseTmpReg(vPlusOne);
    data->vReleaseTmpReg(vMinusOne);
}

static void cvtpd2piXmm(Armv8btAsm* data, U8 to, U8 from, bool truncate) {
    // cpu->reg_mmx[reg].sd.d0 = cpu->xmm[rm].pd.f64[0];
    // cpu->reg_mmx[reg].sd.d1 = cpu->xmm[rm].pd.f64[1];

    //if (cpu->xmm[rm].pd.f64[0] >= 2147483648.0 || cpu->xmm[rm].pd.f64[0] <= -2147483649.0) {
    //    cpu->reg_mmx[reg].sd.d0 = 0x80000000;
    //} else {
    //    cpu->reg_mmx[reg].sd.d0 = (int32_t)cpu->xmm[rm].pd.f64[0];
    //}    

    U8 vPlusOne = data->getSSEConstant(SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE);
    U8 vMinusOne = data->getSSEConstant(SSE_MIN_INT32_MINUS_ONE_AS_DOUBLE);

    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();

    // will set lane to all 1's if value is too large or too small to convert
    data->fCmpGreaterThanOrEqual(vTmpReg1, from, vPlusOne, D2);
    data->fCmpGreaterThanOrEqual(vTmpReg2, vMinusOne, from, D2);
    data->vOr(vTmpReg1, vTmpReg1, vTmpReg2, B16);

    // do the convert from double to int64
    if (truncate) {
        data->vConvertDoubleToInt64RoundToZero(vTmpReg2, from, true);
    } else {
        data->vConvertDoubleToInt64RoundToCurrentMode(vTmpReg2, from, true);
    }

    // clear out int64's if was too large or too small
    data->vAndNot(vTmpReg2, vTmpReg2, vTmpReg1, B16);

    // do the convert from int64 to int32
    data->vConvertInt64ToLowerInt32(vTmpReg2, vTmpReg2);

    // if the value was too large or too smaller, then vTmpReg1 contains all 1's.  
    // Right shift 63, convert 64-bit to 32-bit, left shift by 31 to create 0x80000000
    data->vShiftRightValue(vTmpReg1, vTmpReg1, 63, D2);
    data->vConvertInt64ToLowerInt32(vTmpReg1, vTmpReg1);
    data->vShiftLeftValue(vTmpReg1, vTmpReg1, 31, S4);

    // this will be 0x80000000 or 0x00000000 depending on if the value was too large or too small
    data->vOr(to, vTmpReg2, vTmpReg1, B16);

    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vTmpReg2);
    data->vReleaseTmpReg(vPlusOne);
    data->vReleaseTmpReg(vMinusOne);
}

static void cvtss2siR32Xmm(Armv8btAsm* data, U8 to, U8 from, bool truncate) {
    // cpu->reg[r1].u32 = cpu->xmm[rm].ps.f32[0];

    //if (cpu->xmm[rm].ps.f32[0] >= 2147483648.0 || cpu->xmm[rm].ps.f32[0] <= -2147483649.0) {
    //    cpu->reg[r1].u32 = 0x80000000;
    //} else {
    //    cpu->reg[r1].u32 = (int32_t)cpu->xmm[rm].ps.f32[0];
    //}    

    U8 vPlusOne = data->getSSEConstant(SSE_MAX_INT32_PLUS_ONE_AS_FLOAT);
    U8 vMinusOne = data->getSSEConstant(SSE_MIN_INT32_MINUS_ONE_AS_FLOAT);

    U8 vTmpReg1 = data->vGetTmpReg();
    U8 vTmpReg2 = data->vGetTmpReg();
    U8 tmpReg = data->getTmpReg();
    U8 tmpReg2 = data->getTmpReg();

    // will set lane to all 1's if value is too large or too small to convert
    data->fCmpGreaterThanOrEqual(vTmpReg1, from, vPlusOne, S_scaler);
    data->fCmpGreaterThanOrEqual(vTmpReg2, vMinusOne, from, S_scaler);
    data->vOr(vTmpReg1, vTmpReg1, vTmpReg2, B8);

    data->vMovToGeneralReg32ZeroExtend(tmpReg, vTmpReg1, 0, S4); // vTmpReg1 is all 0's or all 1's, so no conversions are necessary

    // do the convert from float to int32
    if (truncate) {
        data->vConvertFloatToGeneralReg32RoundToZero(tmpReg2, from);
    } else {
        data->vConvertFloatToGeneralReg32RoundToCurrentMode(tmpReg2, from);
    }

    // clear out int32 if was too large or too small
    data->andNotRegs32(tmpReg2, tmpReg2, tmpReg);

    // if the value was too large or too smaller, then tmpReg contains all 1's.  
    data->andValue32(tmpReg, tmpReg, 0x80000000); // will be 0x80000000 if all 1's, else 0

    // will be good value or'd with 0, or bad value (0), or'd with 0x80000000
    data->orRegs32(to, tmpReg2, tmpReg);
    data->releaseTmpReg(tmpReg);
    data->releaseTmpReg(tmpReg2);
    data->vReleaseTmpReg(vTmpReg1);
    data->vReleaseTmpReg(vTmpReg2);
    data->vReleaseTmpReg(vPlusOne);
    data->vReleaseTmpReg(vMinusOne);
}

void opCvtss2siR32Xmm(Armv8btAsm* data) {
    cvtss2siR32Xmm(data, data->getNativeReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), false);
}
void opCvtss2siR32E32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, vTmpReg, true);
    cvtss2siR32Xmm(data, data->getNativeReg(data->currentOp->reg), vTmpReg, false);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opCvttps2piMmxXmm(Armv8btAsm* data) {
    cvtps2piXmm(data, data->getNativeMmxReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), true);
}
void opCvttps2piMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    cvtps2piXmm(data, data->getNativeMmxReg(data->currentOp->reg), vTmpReg, true);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opCvttss2siR32Xmm(Armv8btAsm* data) {
    cvtss2siR32Xmm(data, data->getNativeReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), true);
}
void opCvttss2siR32E32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, vTmpReg, true);
    cvtss2siR32Xmm(data, data->getNativeReg(data->currentOp->reg), vTmpReg, true);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opCvtps2piMmxXmm(Armv8btAsm* data) {
    cvtps2piXmm(data, data->getNativeMmxReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), false);
}
void opCvtps2piMmxE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    cvtps2piXmm(data, data->getNativeMmxReg(data->currentOp->reg), vTmpReg, false);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opCvtsi2ssXmmR32(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.f32[0] = cpu->reg_mmx[reg].sd.d0;
    U8 tmpReg = data->vGetTmpReg();
    data->vMovFromGeneralReg32(tmpReg, 0, data->getNativeReg(data->currentOp->rm));
    data->vConvertInt32ToFloat(tmpReg, tmpReg, false);
    data->vMov32(data->getNativeReg(data->currentOp->reg), 0, tmpReg, 0);
    data->vReleaseTmpReg(tmpReg);
}
void opCvtsi2ssXmmE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, tmpReg, true);
    data->vConvertInt32ToFloat(tmpReg, tmpReg, false);
    data->vMov32(data->getNativeReg(data->currentOp->reg), 0, tmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(tmpReg);
}

void opCvtpd2piMmxXmm(Armv8btAsm* data) {
    cvtpd2piXmm(data, data->getNativeMmxReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), false);
}
void opCvtpd2piMmxE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    cvtpd2piXmm(data, data->getNativeMmxReg(data->currentOp->reg), vTmpReg, false);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opCvtpi2pdXmmMmx(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.f64[0] = cpu->reg_mmx[reg].sd.d0;
    // cpu->xmm[reg].pd.f64[1] = cpu->reg_mmx[reg].sd.d1;    
    data->vSignExtend64To128(data->getNativeSseReg(data->currentOp->reg), data->getNativeMmxReg(data->currentOp->rm), S4);
    data->vConvertInt64ToDouble(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), true);
}
void opCvtpi2pdXmmE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, tmpReg, true);
    data->vSignExtend64To128(data->getNativeSseReg(data->currentOp->reg), tmpReg, S4);
    data->vConvertInt64ToDouble(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), true);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(tmpReg);
}
void opCvtsd2siR32Xmm(Armv8btAsm* data) {
    cvtsd2siR32Xmm(data, data->getNativeReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), false);
}
void opCvtsd2siR32E64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    cvtsd2siR32Xmm(data, data->getNativeReg(data->currentOp->reg), vTmpReg, false);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opCvtsi2sdXmmR32(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.f64[0] = cpu->reg[rm].u32;
    U8 tmpReg = data->vGetTmpReg();
    data->vMovFromGeneralReg32(tmpReg, 0, data->getNativeReg(data->currentOp->rm));
    data->vSignExtend64To128(tmpReg, tmpReg, S4);
    data->vConvertInt64ToDouble(tmpReg, tmpReg, false);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, tmpReg, 0);
    data->vReleaseTmpReg(tmpReg);
}
void opCvtsi2sdXmmE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, tmpReg, true);
    data->vSignExtend64To128(tmpReg, tmpReg, S4);
    data->vConvertInt64ToDouble(tmpReg, tmpReg, false);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, tmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(tmpReg);
}
void opCvttpd2piMmxXmm(Armv8btAsm* data) {
    cvtpd2piXmm(data, data->getNativeMmxReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), true);
}
void opCvttpd2piMmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    cvtpd2piXmm(data, data->getNativeMmxReg(data->currentOp->reg), vTmpReg, true);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opCvttsd2siR32Xmm(Armv8btAsm* data) {
    cvtsd2siR32Xmm(data, data->getNativeReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), true);
}
void opCvttsd2siR32E64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, vTmpReg, true);
    cvtsd2siR32Xmm(data, data->getNativeReg(data->currentOp->reg), vTmpReg, true);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opCvtpi2psXmmMmx(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.f32[0] = cpu->reg_mmx[reg].sd.d0;
    // cpu->xmm[reg].pd.f32[1] = cpu->reg_mmx[reg].sd.d1; 
    U8 tmpReg = data->vGetTmpReg();
    data->vConvertInt32ToFloat(tmpReg, data->getNativeSseReg(data->currentOp->reg), true);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, tmpReg, 0); // need to maintain the high 64-bits
    data->vReleaseTmpReg(tmpReg);
}
void opCvtpi2psXmmE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, tmpReg, true);
    data->vConvertInt32ToFloat(tmpReg, tmpReg, true);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, tmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(tmpReg);
}

void opCvtpd2psXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].ps.f32[0] = (float)cpu->xmm[rm].pd.f64[0];
    // cpu->xmm[reg].ps.f32[1] = (float)cpu->xmm[rm].pd.f64[1];
    // cpu->xmm[reg].pi.u32[2] = 0
    // cpu->xmm[reg].pi.u32[3] = 0
    U8 tmpReg = data->vGetTmpReg();
    data->vLoadConst(tmpReg, 0, B16);
    data->vConvertDoubleToFloatRoundToCurrentModeAndKeep(tmpReg, data->getNativeSseReg(data->currentOp->rm));
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, tmpReg, 0); // need to maintain the high 64-bits
    data->vReleaseTmpReg(tmpReg);
}
void opCvtpd2psXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    data->vConvertDoubleToFloatRoundToCurrentModeAndKeep(tmpReg, tmpReg);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, tmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(tmpReg);
}

void opCvtps2pdXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.f64[0] = (double)cpu->xmm[rm].ps.f32[0];
    // cpu->xmm[reg].pd.f64[1] = (double)cpu->xmm[rm].ps.f32[1];
    data->vConvertFloatToDouble(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), true);
}

void opCvtps2pdXmmE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, tmpReg, true); // will 0 out the top
    data->vConvertFloatToDouble(data->getNativeSseReg(data->currentOp->reg), tmpReg, true);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(tmpReg);
}

void opCvtsd2ssXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].ps.f32[0] = (double)cpu->xmm[rm].pd.f64[0];
    U8 tmpReg = data->vGetTmpReg();
    data->vConvertDoubleToFloatRoundToCurrentModeAndKeep(tmpReg, data->getNativeSseReg(data->currentOp->rm));
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, tmpReg, 0);
    data->vReleaseTmpReg(tmpReg);
}
void opCvtsd2ssXmmE64(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->vGetTmpReg();
    data->vReadMemory64(addressReg, tmpReg, true); // will 0 out the top
    data->vConvertDoubleToFloatRoundToCurrentModeAndKeep(tmpReg, tmpReg);
    data->vMov32(data->getNativeSseReg(data->currentOp->reg), 0, tmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(tmpReg);
}

void opCvtdq2pdXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.f64[0] = (double)cpu->xmm[rm].pi.i32[0];
    // cpu->xmm[reg].pd.f64[1] = (double)cpu->xmm[rm].pi.i32[1];
    data->vSignExtend64To128(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), S4);
    data->vConvertInt64ToDouble(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->reg), true);
}
void opCvtdq2pdXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    data->vSignExtend64To128(tmpReg, tmpReg, S4);
    data->vConvertInt64ToDouble(data->getNativeSseReg(data->currentOp->reg), tmpReg, true);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(tmpReg);
}

void opCvtdq2psXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].ps.f32[0] = (float)cpu->xmm[rm].pi.i32[0];
    // cpu->xmm[reg].ps.f32[1] = (float)cpu->xmm[rm].pi.i32[1];
    // cpu->xmm[reg].ps.f32[2] = (float)cpu->xmm[rm].pi.i32[2];
    // cpu->xmm[reg].ps.f32[3] = (float)cpu->xmm[rm].pi.i32[3];
    data->vConvertInt32ToFloat(data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), true);
}
void opCvtdq2psXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, tmpReg, true);
    data->vConvertInt32ToFloat(data->getNativeSseReg(data->currentOp->reg), tmpReg, true);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(tmpReg);
}
void opCvtpd2dqXmmXmm(Armv8btAsm* data) {
    cvtpd2piXmm(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), false);
}
void opCvtpd2dqXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    cvtpd2piXmm(data, data->getNativeSseReg(data->currentOp->reg), vTmpReg, false);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opCvtps2dqXmmXmm(Armv8btAsm* data) {
    cvtps2piXmm(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), false);
}
void opCvtps2dqXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    cvtps2piXmm(data, data->getNativeSseReg(data->currentOp->reg), vTmpReg, false);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}

void opCvtss2sdXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].pd.f64[0] = (float)cpu->xmm[rm].ps.f32[0];
    U8 tmpReg = data->vGetTmpReg();
    data->vConvertFloatToDouble(tmpReg, data->getNativeSseReg(data->currentOp->rm), false);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, tmpReg, 0);
    data->vReleaseTmpReg(tmpReg);
}
void opCvtss2sdXmmE32(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 tmpReg = data->vGetTmpReg();
    data->vReadMemory32(addressReg, tmpReg, true);
    data->vConvertFloatToDouble(tmpReg, tmpReg, false);
    data->vMov64(data->getNativeSseReg(data->currentOp->reg), 0, tmpReg, 0);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(tmpReg);
}
void opCvttpd2dqXmmXmm(Armv8btAsm* data) {
    // cpu->xmm[reg].pi.s32[0] = (S32)cpu->xmm[rm].pd.f64[0];
    // cpu->xmm[reg].pi.s32[1] = (S32)cpu->xmm[rm].pd.f64[1];
    // cpu->xmm[reg].pi.u32[2] = 0
    // cpu->xmm[reg].pi.u32[3] = 0
    cvtpd2piXmm(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), true);
}
void opCvttpd2dqXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    cvtpd2piXmm(data, data->getNativeSseReg(data->currentOp->reg), vTmpReg, true);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
void opCvttps2dqXmmXmm(Armv8btAsm* data) {
    cvtps2piXmm(data, data->getNativeSseReg(data->currentOp->reg), data->getNativeSseReg(data->currentOp->rm), true);
}
void opCvttps2dqXmmE128(Armv8btAsm* data) {
    U8 addressReg = data->getAddressReg();
    U8 vTmpReg = data->vGetTmpReg();
    data->vReadMemory128(addressReg, vTmpReg, true);
    cvtps2piXmm(data, data->getNativeSseReg(data->currentOp->reg), vTmpReg, true);
    data->releaseTmpReg(addressReg);
    data->vReleaseTmpReg(vTmpReg);
}
#endif