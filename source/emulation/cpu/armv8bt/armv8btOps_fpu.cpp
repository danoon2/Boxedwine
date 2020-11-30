#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
#include "armv8btOps.h"
#include "../common/common_fpu.h"

static U8 loadInt32AsDouble(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	U8 tmpReg = data->vGetTmpReg();
	data->vReadMemory32(addressReg, tmpReg, true);
	data->vSignExtend64To128(tmpReg, tmpReg, S4);
	data->vConvertInt64ToDouble(tmpReg, tmpReg, false);
	data->releaseTmpReg(addressReg);
	return tmpReg;
}

static U8 loadInt16AsDouble(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	U8 tmpReg = data->vGetTmpReg();
	data->vReadMemory16(addressReg, tmpReg, 0, true);
	data->vSignExtend64To128(tmpReg, tmpReg, H8);
	data->vSignExtend64To128(tmpReg, tmpReg, S4);
	data->vConvertInt64ToDouble(tmpReg, tmpReg, false);
	data->releaseTmpReg(addressReg);
	return tmpReg;
}

static U8 loadInt64AsDouble(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	U8 tmpReg = data->vGetTmpReg();
	data->vReadMemory64(addressReg, tmpReg, 0, true);
	data->vConvertInt64ToDouble(tmpReg, tmpReg, false);
	data->releaseTmpReg(addressReg);
	return tmpReg;
}

static U8 calculateIndexReg(Armv8btAsm* data, U32 index) {
	U8 result = data->vGetTmpReg();
	data->addValue32(result, data->getFpuTopReg(), index);
	data->andValue32(result, result, 7);
	return result;
}

static void hostReadTag(Armv8btAsm* data, U32 index, U8 resultReg) {
	U8 offsetReg = data->getFpuTagOffset();

	if (index == 0) {
		data->readMem32RegOffset(resultReg, offsetReg, data->getFpuTopReg(), 2);
	} else {
		U8 topReg = calculateIndexReg(data, index);
		data->readMem32RegOffset(resultReg, offsetReg, topReg, 2);
		data->releaseTmpReg(topReg);
	}
	data->releaseFpuTagOffset(offsetReg);
}

static void hostWriteTag(Armv8btAsm* data, U8 valueReg, bool releaseValueReg, U32 index) {
	U8 offsetReg = data->getFpuTagOffset();

	if (index == 0) {
		data->writeMem32RegOffset(valueReg, offsetReg, data->getFpuTopReg(), 2);
	} else {
		U8 topReg = calculateIndexReg(data, index);
		data->writeMem32RegOffset(valueReg, offsetReg, topReg, 2);
		data->releaseTmpReg(topReg);
	}
	if (releaseValueReg) {
		data->releaseTmpReg(valueReg);
	}
	data->releaseFpuTagOffset(offsetReg);
}

static void hostSaveDouble(Armv8btAsm* data, U32 index) {
	if (index == 0) {
		data->vWriteMem64RegOffset(vMMX0+index, data->getFpuOffset(), data->getFpuTopReg(), 3);
	} else {
		U8 topReg = calculateIndexReg(data, index);
		data->vWriteMem64RegOffset(vMMX0 + index, data->getFpuOffset(), topReg, 3);
		data->releaseTmpReg(topReg);
	}
	data->isFpuRegCached[index] = true;
}

// add  w10, w25, #0x8
// ldr  s9, [x10, x20]
// fcvt d9, s9
static U8 readFloat(Armv8btAsm* data, U8 reg = 0) {
	if (!reg) {
		reg = data->vGetTmpReg();
	}
	U8 addressReg = data->getAddressReg();
	data->vReadMemory32(addressReg, reg, true);
	data->vConvertFloatToDouble(reg, reg, false);
	data->releaseTmpReg(addressReg);
	return reg;
}

static U8 readDouble(Armv8btAsm* data, U8 reg = 0) {
	if (!reg) {
		reg = data->vGetTmpReg();
	}
	U8 addressReg = data->getAddressReg();
	data->vReadMemory64(addressReg, reg, true);
	data->releaseTmpReg(addressReg);
	return reg;
}

class FPUReg {
public:
	FPUReg(Armv8btAsm* data, U32 index, bool writeBack, bool read = true) : data(data), writeBack(writeBack), topReg(0) {
		this->reg = index + vMMX0;
		if (!data->isFpuRegCached[index]) {
			if (index == 0) {
				if (read) {
					data->vReadMem64RegOffset(this->reg, data->getFpuOffset(), data->getFpuTopReg(), 3);
				}
			} else {
				this->topReg = calculateIndexReg(data, index);
				if (read) {
					data->vReadMem64RegOffset(this->reg, data->getFpuOffset(), this->topReg, 3);
				}
				if (!writeBack) {
					data->releaseTmpReg(this->topReg);
				}
			}
			if (read) {
				data->isFpuRegCached[index] = true;
			}
		} else if (writeBack && index>0) {
			this->topReg = calculateIndexReg(data, index);
		} 
	}
	~FPUReg() {
		doWrite();
	}
	void doWrite() {
		if (this->writeBack) {
			if (this->topReg == 0) {
				data->vWriteMem64RegOffset(this->reg, data->getFpuOffset(), data->getFpuTopReg(), 3);
			} else {
				data->vWriteMem64RegOffset(this->reg, data->getFpuOffset(), this->topReg, 3);
				data->releaseTmpReg(this->topReg);
			}
			this->writeBack = false;
		}
	}
	void hostReadTag(U8 resultReg) {
		U8 offsetReg = data->getFpuTagOffset();
		data->readMem32RegOffset(resultReg, offsetReg, this->topReg, 2);
		data->releaseFpuTagOffset(offsetReg);
	}
	void hostWriteTag(U8 valueReg) {
		U8 offsetReg = data->getFpuTagOffset();
		data->writeMem32RegOffset(valueReg, offsetReg, this->topReg, 2);
		data->releaseFpuTagOffset(offsetReg);
	}
	Armv8btAsm* data;
	bool writeBack;
	U8 reg;
	U8 topReg;
};

static void PREP_PUSH(Armv8btAsm* data, bool writeTag = true) {
	U8 topReg = data->getFpuTopReg();
	data->subValue32(topReg, topReg, 1);
	data->andValue32(topReg, topReg, 7);
	data->writeMem32ValueOffset(topReg, data->getFpuOffset(), (U32)(offsetof(FPU, top)));
	if (writeTag) {
		hostWriteTag(data, data->getRegWithConst(TAG_Valid), true, 0);
	}
	data->clearCachedFpuRegs();
}

static void FPU_POP(Armv8btAsm* data) {
	// this->tags[this->top] = TAG_Empty;
	// this->top = ((this->top + 1) & 7);
	hostWriteTag(data, data->getRegWithConst(TAG_Empty), true, 0);

	U8 topReg = data->getFpuTopReg();
	data->addValue32(topReg, topReg, 1);
	data->andValue32(topReg, topReg, 7);
	data->writeMem32ValueOffset(topReg, data->getFpuOffset(), (U32)(offsetof(FPU, top)));
	data->clearCachedFpuRegs();
}

static void fcompare(Armv8btAsm* data, U8 v1, S32 tagIndex1, U8 v2, S32 tagIndex2, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) {
	U8 tmpReg = data->getTmpReg();
	U8 tmpReg2 = data->getTmpReg();
	U8 offsetReg = data->getFpuTagOffset();

	if (tagIndex1 == 0) {
		data->readMem32RegOffset(tmpReg, offsetReg, data->getFpuTopReg(), 2);
	} else {
		U8 topReg = calculateIndexReg(data, tagIndex1);
		data->readMem32RegOffset(tmpReg, offsetReg, topReg, 2);
		data->releaseTmpReg(topReg);
	}
	if (tagIndex2 == 0) {
		data->readMem32RegOffset(tmpReg2, offsetReg, data->getFpuTopReg(), 2);
	} else if (tagIndex2 != -1) {
		U8 topReg = calculateIndexReg(data, tagIndex2);
		data->readMem32RegOffset(tmpReg2, offsetReg, topReg, 2);
		data->releaseTmpReg(topReg);
	}
	data->releaseFpuTagOffset(offsetReg);
	if (tagIndex2 != -1) {
		data->orRegs32(tmpReg, tmpReg, tmpReg2);
	}
	data->readMem32ValueOffset(tmpReg2, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));

	data->doIf(tmpReg, 0, DO_IF_EQUAL, [data, pfnInvalid, pfnEqual, pfnGreaterThan, pfnLessThan, v1, v2] {
		// if both tags are TAG_Valid
		data->fCmp64(v1, v2);
		data->doIf(0, 0, DO_IF_EQUAL, [data, pfnInvalid, pfnEqual] {
			pfnEqual();
			}, [data, pfnInvalid, pfnGreaterThan, pfnLessThan] {
				data->doIf(0, 0, DO_IF_SIGNED_GREATER_THAN, [data, pfnGreaterThan] {
					pfnGreaterThan();
					}, [data, pfnInvalid, pfnLessThan] {
						data->doIf(0, 0, DO_IF_LESS_THAN, [data, pfnLessThan] {
							pfnLessThan();
							}, [data, pfnInvalid] {
								// is not equal, is not less than and is not greater than, this means one of the is nan
								pfnInvalid();
							}, nullptr, false, false);
					}, nullptr, false, false);
			}, nullptr, false, false);
		}, [data, pfnInvalid] {
			// if either tag is not TAG_Valid
			pfnInvalid();
		});
	data->releaseTmpReg(tmpReg);
	data->writeMem32ValueOffset(tmpReg2, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
	data->releaseTmpReg(tmpReg2);
}

// #define FPU_SET_C0(fpu, C) (fpu)->sw &= ~0x0100; if (C != 0) (fpu)->sw |= 0x0100    
// #define FPU_SET_C1(fpu, C) (fpu)->sw &= ~0x0200; if (C != 0) (fpu)->sw |= 0x0200    
// #define FPU_SET_C2(fpu, C) (fpu)->sw &= ~0x0400; if (C != 0) (fpu)->sw |= 0x0400    
// #define FPU_SET_C3(fpu, C) (fpu)->sw &= ~0x4000; if (C != 0) (fpu)->sw |= 0x4000

static void doFCOM(Armv8btAsm* data, U8 v1, S32 tagIndex1, U8 v2, S32 tagIndex2) {
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

	U8 swReg = data->getTmpReg();
	data->readMem32ValueOffset(swReg, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));

	fcompare(data, v1, tagIndex1, v2, tagIndex2, [data, swReg] {
			// equal
			data->andValue32(swReg, swReg, ~0x0700);
			data->orValue32(swReg, swReg, 0x4000);
		}, [data, swReg] {
			// less than
			data->andValue32(swReg, swReg, ~0x4600);
			data->orValue32(swReg, swReg, 0x0100);
		}, [data, swReg] {
			// greater than
			data->andValue32(swReg, swReg, ~0x4700);
		}, [data, swReg] {
			// invalid
			data->andValue32(swReg, swReg, ~0x0200);
			data->orValue32(swReg, swReg, 0x4500);
		});
	
	data->writeMem32ValueOffset(swReg, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
	data->releaseTmpReg(swReg);
}

static void doFCOMI(Armv8btAsm* data, U8 v1, S32 tagIndex1, U8 v2, S32 tagIndex2) {
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

	data->lazyFlags = NULL;
	data->andValue32(xFLAGS, xFLAGS, ~FMASK_TEST);

	fcompare(data, v1, tagIndex1, v2, tagIndex2, [data] {
		// equal
		data->orValue32(xFLAGS, xFLAGS, ZF);
		}, [data] {
			// less than
			data->orValue32(xFLAGS, xFLAGS, CF);
		}, [data] {
			// greater than
			// nothing
		}, [data] {
			// invalid
			data->orValue32(xFLAGS, xFLAGS, CF | PF | ZF);
		});
}
void opFADD_ST0_STj(Armv8btAsm* data) {
	// cpu->fpu.STV(0) += cpu->fpu.STV(reg)
	FPUReg dst(data, 0, true);
	FPUReg src(data, data->decodedOp->reg, false);
	data->fAdd(dst.reg, dst.reg, src.reg, D_scaler);
}

void opFMUL_ST0_STj(Armv8btAsm* data) {
	// cpu->fpu.STV(0) *= cpu->fpu.STV(reg)
	FPUReg dst(data, 0, true);
	FPUReg src(data, data->decodedOp->reg, false);
	data->fMul(dst.reg, dst.reg, src.reg, D_scaler);
}
void opFCOM_STi(Armv8btAsm* data) {
	FPUReg dst(data, 0, false);
	FPUReg src(data, data->decodedOp->reg, false);
	doFCOM(data, dst.reg, 0, src.reg, data->decodedOp->reg);
}
void opFCOM_STi_Pop(Armv8btAsm* data) {
	FPUReg dst(data, 0, false);
	FPUReg src(data, data->decodedOp->reg, false);
	doFCOM(data, dst.reg, 0, src.reg, data->decodedOp->reg);
	FPU_POP(data);
}
void opFSUB_ST0_STj(Armv8btAsm* data) {
	FPUReg dst(data, 0, true);
	FPUReg src(data, data->decodedOp->reg, false);
	data->fSub(dst.reg, dst.reg, src.reg, D_scaler);
}
void opFSUBR_ST0_STj(Armv8btAsm* data) {
	FPUReg dst(data, 0, true);
	FPUReg src(data, data->decodedOp->reg, false);
	data->fSub(dst.reg, src.reg, dst.reg, D_scaler);
}
void opFDIV_ST0_STj(Armv8btAsm* data) {
	FPUReg dst(data, 0, true);
	FPUReg src(data, data->decodedOp->reg, false);
	data->fDiv(dst.reg, dst.reg, src.reg, D_scaler);
}
void opFDIVR_ST0_STj(Armv8btAsm* data) {
	FPUReg dst(data, 0, true);
	FPUReg src(data, data->decodedOp->reg, false);
	data->fDiv(dst.reg, src.reg, dst.reg, D_scaler);
}
void opFADD_SINGLE_REAL(Armv8btAsm* data) {
	// cpu->fpu.FLD_F32_EA(cpu, address);
	// cpu->fpu.FADD_EA();
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, true);
	data->fAdd(dst.reg, dst.reg, tmpRead, D_scaler);
	data->vReleaseTmpReg(tmpRead);
}
void opFMUL_SINGLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, true);
	data->fMul(dst.reg, dst.reg, tmpRead, D_scaler);
	data->vReleaseTmpReg(tmpRead);
}
void opFCOM_SINGLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, false);
	doFCOM(data, dst.reg, 0, tmpRead, -1);
	data->vReleaseTmpReg(tmpRead);
}
void opFCOM_SINGLE_REAL_Pop(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, false);
	doFCOM(data, dst.reg, 0, tmpRead, -1);
	data->vReleaseTmpReg(tmpRead);
	FPU_POP(data);
}
void opFSUB_SINGLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, true);
	data->fSub(dst.reg, dst.reg, tmpRead, D_scaler);
	data->vReleaseTmpReg(tmpRead);
}
void opFSUBR_SINGLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, true);
	data->fSub(dst.reg, tmpRead, dst.reg, D_scaler);
	data->vReleaseTmpReg(tmpRead);
}
void opFDIV_SINGLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, true);
	data->fDiv(dst.reg, dst.reg, tmpRead, D_scaler);
	data->vReleaseTmpReg(tmpRead);
}
void opFDIVR_SINGLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, true);
	data->fDiv(dst.reg, tmpRead, dst.reg, D_scaler);
	data->vReleaseTmpReg(tmpRead);
}
void opFLD_STi(Armv8btAsm* data) {
	// int reg_from = cpu->fpu.STV(reg);
	// cpu->fpu.PREP_PUSH();
	// cpu->fpu.FST(reg_from, cpu->fpu.STV(0));
	FPUReg from(data, data->decodedOp->reg, false);
	U8 tagReg = data->getTmpReg();
	from.hostReadTag(tagReg); // read before push changes indexes
	PREP_PUSH(data, false);
	FPUReg to(data, 0, true, false);
	data->vMov64(to.reg, 0, from.reg, 0);
	to.hostWriteTag(tagReg);
	data->releaseTmpReg(tagReg);
	
}
void opFXCH_STi(Armv8btAsm* data) {
	// int tag = this->tags[other];
	// struct FPU_Reg reg = this->regs[other];
	// this->tags[other] = this->tags[st];
	// this->regs[other] = this->regs[st];
	// this->tags[st] = tag;
	// this->regs[st] = reg;
	FPUReg from(data, data->decodedOp->reg, true);
	FPUReg to(data, 0, true);
	U8 vTmpReg = data->vGetTmpReg();
	data->vMov64(vTmpReg, 0, from.reg, 0);
	data->vMov64(from.reg, 0, to.reg, 0);
	data->vMov64(to.reg, 0, vTmpReg, 0);
	data->vReleaseTmpReg(vTmpReg);

	U8 tmpReg = data->getTmpReg();
	U8 tmpReg2 = data->getTmpReg();
	from.hostReadTag(tmpReg);
	to.hostReadTag(tmpReg2);
	from.hostWriteTag(tmpReg2);
	to.hostWriteTag(tmpReg);
	data->releaseTmpReg(tmpReg);
	data->releaseTmpReg(tmpReg2);
}
void opFNOP(Armv8btAsm* data) {	
}
void opFST_STi_Pop(Armv8btAsm* data) {
	// cpu->fpu.FST(cpu->fpu.STV(0), cpu->fpu.STV(reg));
	// cpu->fpu.FPOP();
	FPUReg to(data, data->decodedOp->reg, true, false);
	FPUReg from(data, 0, false);
	U8 tagReg = data->getTmpReg();
	from.hostReadTag(tagReg);	
	data->vMov64(to.reg, 0, from.reg, 0);
	to.hostWriteTag(tagReg);
	to.doWrite();
	data->releaseTmpReg(tagReg);
	FPU_POP(data);
}
void opFCHS(Armv8btAsm* data) {
	// this->regs[this->top].d = -1.0 * (this->regs[this->top].d);
	FPUReg reg(data, 0, true);
	data->fNeg(reg.reg, reg.reg, D_scaler);
}
void opFABS(Armv8btAsm* data) {
	// this->regs[this->top].d = fabs(this->regs[this->top].d);
	FPUReg reg(data, 0, true);
	data->fAbs(reg.reg, reg.reg, D_scaler);
}
void opFTST(Armv8btAsm* data) {
	// this->regs[8].d = 0.0;
	// FCOM(this->top, 8);
	FPUReg dst(data, 0, false);
	U8 vTmpReg = data->vGetTmpReg();
	data->vMovFromGeneralReg64(vTmpReg, 0, 31);
	doFCOM(data, dst.reg, 0, vTmpReg, -1);
	data->vReleaseTmpReg(vTmpReg);
}
void opFXAM(Armv8btAsm* data) {
	// S64 bits = this->regs[this->top].l;
	// if ((bits & 0x8000000000000000l) != 0)    //sign
	// {
	// 	   FPU_SET_C1(this, 1);
	// } else {
	// 	   FPU_SET_C1(this, 0);
	// }
	// if (this->tags[this->top] == TAG_Empty) {
	// 	   FPU_SET_C3(this, 1);
	// 	   FPU_SET_C2(this, 0);
	// 	   FPU_SET_C0(this, 1);
	// 	   return;
	// }
	// if (isnan(this->regs[this->top].d)) {
	// 	   FPU_SET_C3(this, 0);
	// 	   FPU_SET_C2(this, 0);
	// 	   FPU_SET_C0(this, 1);
	// } else if (isinf(this->regs[this->top].d)) {
	//     FPU_SET_C3(this, 0);
	// 	   FPU_SET_C2(this, 1);
	// 	   FPU_SET_C0(this, 1);
	// } else if (this->regs[this->top].d == 0.0) {       //zero or normalized number.
	//    FPU_SET_C3(this, 1);
	//    FPU_SET_C2(this, 0);
	//    FPU_SET_C0(this, 0);
	// } else {
	//    FPU_SET_C3(this, 0);
	//    FPU_SET_C2(this, 1);
	//    FPU_SET_C0(this, 0);
	// }	
	U8 offsetReg = data->getFpuTagOffset();
	U8 swReg = data->getTmpReg();
	FPUReg reg(data, 0, false);

	data->readMem32ValueOffset(swReg, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
	data->andValue32(swReg, swReg, ~0x4700);

	// :TODO: check sign bit
	U8 bitsReg = data->getTmpReg();
	data->vMovToGeneralReg64(bitsReg, reg.reg, 0);
	data->testValue64(bitsReg, 0x8000000000000000);
	data->doIf(0, 0, DO_IF_NOT_EQUAL, [data, swReg] {
		data->orValue32(swReg, swReg, 0x200);
		}, nullptr, nullptr, false, false);

	U8 tagReg = data->getTmpReg();
	data->readMem32RegOffset(tagReg, offsetReg, data->getFpuTopReg(), 2);
	data->releaseFpuTagOffset(offsetReg);

	data->doIf(tagReg, 0, DO_IF_EQUAL, [data, swReg, bitsReg] {
		// if tag is TAG_Valid		
		
		U8 tmpReg2 = data->getTmpReg();
		data->andValue64(tmpReg2, bitsReg, 0x7ff0000000000000);
		data->shiftRegRightWithValue64(tmpReg2, tmpReg2, 52);
		data->doIf(tmpReg2, 0x7ff, DO_IF_NOT_EQUAL, [data, swReg, bitsReg] {
			data->testValue64(bitsReg, 0x7fffffffffffffff);
			data->doIf(0, 0, DO_IF_EQUAL, [data, swReg] {
				// normal number
				data->orValue32(swReg, swReg, 0x4000);
				}, [data, swReg] {
					// zero					
					data->orValue32(swReg, swReg, 0x400);
				}, nullptr, false, false);
			}, [data, swReg, bitsReg] {
				// isnan or isinf
				data->testValue64(bitsReg, 0xfffffffffffff);
				data->doIf(0, 0, DO_IF_EQUAL, [data, swReg] {
					// inf
					data->orValue32(swReg, swReg, 0x500);
					}, [data, swReg] {
						// nan
						data->orValue32(swReg, swReg, 0x100);
					}, nullptr, false, false);
			});
		data->releaseTmpReg(tmpReg2);
		}, [data, swReg] {
			// if tag is not TAG_Valid
			data->orValue32(swReg, swReg, 0x4100);
		});
	data->releaseTmpReg(bitsReg);
	data->releaseTmpReg(tagReg);
	data->writeMem32ValueOffset(swReg, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
	data->releaseTmpReg(swReg);
}
void opFLD1(Armv8btAsm* data) {
	PREP_PUSH(data);
	FPUReg reg(data, 0, true, false);
	U8 tmpReg = data->getTmpReg();
	data->loadConst(tmpReg, 0x3FF0000000000000);
	data->vMovFromGeneralReg64(reg.reg, 0, tmpReg);
	data->releaseTmpReg(tmpReg);
}
void opFLDL2T(Armv8btAsm* data) {
	PREP_PUSH(data);
	FPUReg reg(data, 0, true, false);
	U8 tmpReg = data->getTmpReg();
	data->loadConst(tmpReg, 0x400A934F0979A371);
	data->vMovFromGeneralReg64(reg.reg, 0, tmpReg);
	data->releaseTmpReg(tmpReg);
}
void opFLDL2E(Armv8btAsm* data) {
	PREP_PUSH(data);
	FPUReg reg(data, 0, true, false);
	U8 tmpReg = data->getTmpReg();
	data->loadConst(tmpReg, 0x3FF71547652B82FE);
	data->vMovFromGeneralReg64(reg.reg, 0, tmpReg);
	data->releaseTmpReg(tmpReg);
}
void opFLDPI(Armv8btAsm* data) {
	PREP_PUSH(data);
	FPUReg reg(data, 0, true, false);
	U8 tmpReg = data->getTmpReg();
	data->loadConst(tmpReg, 0x400921FB54442D18);
	data->vMovFromGeneralReg64(reg.reg, 0, tmpReg);
	data->releaseTmpReg(tmpReg);
}
void opFLDLG2(Armv8btAsm* data) {
	PREP_PUSH(data);
	FPUReg reg(data, 0, true, false);
	U8 tmpReg = data->getTmpReg();
	data->loadConst(tmpReg, 0x3FD34413509F79FF);
	data->vMovFromGeneralReg64(reg.reg, 0, tmpReg);
	data->releaseTmpReg(tmpReg);
}
void opFLDLN2(Armv8btAsm* data) {
	PREP_PUSH(data);
	FPUReg reg(data, 0, true, false);
	U8 tmpReg = data->getTmpReg();
	data->loadConst(tmpReg, 0x3FE62E42FEFA39EF);
	data->vMovFromGeneralReg64(reg.reg, 0, tmpReg);
	data->releaseTmpReg(tmpReg);
}
void opFLDZ(Armv8btAsm* data) {
	PREP_PUSH(data);
	FPUReg reg(data, 0, true, false);
	data->vMovFromGeneralReg64(reg.reg, 0, 31);
}

void opF2XM1(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_F2XM1);
	data->syncRegsToHost();
}
void opFYL2X(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_FYL2X);
	data->syncRegsToHost();
}
void opFPTAN(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_FPTAN);
	data->syncRegsToHost();
}
void opFPATAN(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_FPATAN);
	data->syncRegsToHost();
}
void opFXTRACT(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_FXTRACT);
	data->syncRegsToHost();
}
void opFPREM_nearest(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_FPREM_nearest);
	data->syncRegsToHost();
}
void opFDECSTP(Armv8btAsm* data) {
	// this->top = (this->top - 1) & 7;
	U8 topReg = data->getFpuTopReg();
	data->subValue32(topReg, topReg, 1);
	data->andValue32(topReg, topReg, 7);
	data->writeMem32ValueOffset(topReg, data->getFpuOffset(), (U32)(offsetof(FPU, top)));
	data->clearCachedFpuRegs();
}
void opFINCSTP(Armv8btAsm* data) {
	// this->top = (this->top + 1) & 7;
	U8 topReg = data->getFpuTopReg();
	data->addValue32(topReg, topReg, 1);
	data->andValue32(topReg, topReg, 7);
	data->writeMem32ValueOffset(topReg, data->getFpuOffset(), (U32)(offsetof(FPU, top)));
	data->clearCachedFpuRegs();
}
void opFPREM(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_FPREM);
	data->syncRegsToHost();
}
void opFYL2XP1(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_FYL2XP1);
	data->syncRegsToHost();
}
void opFSQRT(Armv8btAsm* data) {
	// this->regs[this->top].d = sqrt(this->regs[this->top].d);
	FPUReg reg(data, 0, true, false);
	data->fSqrt(reg.reg, reg.reg, D_scaler);
}
void opFSINCOS(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_FSINCOS);
	data->syncRegsToHost();
}
void opFRNDINT(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_FRNDINT);
	data->syncRegsToHost();
}
void opFSCALE(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_FSCALE);
	data->syncRegsToHost();
}
void opFSIN(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_FSIN);
	data->syncRegsToHost();
}
void opFCOS(Armv8btAsm* data) {
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->callHost((void*)common_FCOS);
	data->syncRegsToHost();
}
void opFLD_SINGLE_REAL(Armv8btAsm* data) {
	// U32 value = readd(address); // might generate PF, so do before we adjust the stack
	// cpu->fpu.PREP_PUSH();
	// cpu->fpu.FLD_F32(value, cpu->fpu.STV(0));
	readFloat(data, vMMX0);
	PREP_PUSH(data);
	hostSaveDouble(data, 0);
}
void opFST_SINGLE_REAL(Armv8btAsm* data) {
	FPUReg src(data, 0, false);
	U8 vTmpReg = data->vGetTmpReg();
	data->vConvertDoubleToFloatRoundToCurrentModeAndKeep(vTmpReg, src.reg);
	U8 addressReg = data->getAddressReg();
	data->vWriteMemory32(addressReg, vTmpReg, 0, true);
	data->releaseTmpReg(addressReg);
	data->vReleaseTmpReg(vTmpReg);
}
void opFST_SINGLE_REAL_Pop(Armv8btAsm* data) {
	FPUReg src(data, 0, false);
	U8 vTmpReg = data->vGetTmpReg();
	data->vConvertDoubleToFloatRoundToCurrentModeAndKeep(vTmpReg, src.reg);
	U8 addressReg = data->getAddressReg();
	data->vWriteMemory32(addressReg, vTmpReg, 0, true);
	data->releaseTmpReg(addressReg);
	data->vReleaseTmpReg(vTmpReg);
	FPU_POP(data);
}
void opFLDENV(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->mov64(1, addressReg);
	data->releaseTmpReg(addressReg);
	data->callHost((void*)common_FLDENV);
	data->syncRegsToHost();
}
void opFLDCW(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->mov64(1, addressReg);
	data->releaseTmpReg(addressReg);
	data->callHost((void*)common_FLDCW);
	data->syncRegsToHost();
}
void opFNSTENV(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->mov64(1, addressReg);
	data->releaseTmpReg(addressReg);
	data->callHost((void*)common_FNSTENV);
	data->syncRegsToHost();
}
void opFNSTCW(Armv8btAsm* data) {
	// writew(address, cpu->fpu.CW());
	U8 tmpReg = data->getTmpReg();
	U8 addressReg = data->getAddressReg();
	data->readMem32ValueOffset(tmpReg, data->getFpuOffset(), (U32)(offsetof(FPU, cw)));
	data->writeMemory(addressReg, tmpReg, 16, true);
	data->releaseTmpReg(addressReg);
	data->releaseTmpReg(tmpReg);
}

static void doCMov(Armv8btAsm* data) {
	FPUReg from(data, data->decodedOp->reg, false);
	U8 tagReg = data->getTmpReg();
	from.hostReadTag(tagReg); // read before push changes indexes
	FPUReg to(data, 0, true, false);
	data->vMov64(to.reg, 0, from.reg, 0);
	to.hostWriteTag(tagReg);
	data->releaseTmpReg(tagReg);
}

void opFCMOV_ST0_STj_CF(Armv8btAsm* data) {
	doCondition(data, condional_B, [data] {
		doCMov(data);
		});
}
void opFCMOV_ST0_STj_ZF(Armv8btAsm* data) {
	doCondition(data, condional_Z, [data] {
		doCMov(data);
		});
}
void opFCMOV_ST0_STj_CF_OR_ZF(Armv8btAsm* data) {
	doCondition(data, condional_BE, [data] {
		doCMov(data);
		});
}
void opFCMOV_ST0_STj_PF(Armv8btAsm* data) {
	doCondition(data, condional_P, [data] {
		doCMov(data);
		});
}
void opFUCOMPP(Armv8btAsm* data) {
	FPUReg reg1(data, 0, false);
	FPUReg reg2(data, 1, false);
	// :TODO: FCOM and FUCOM currently do the same thing
	doFCOM(data, reg1.reg, 0, reg2.reg, 1);
	FPU_POP(data);
	FPU_POP(data);
}
void opFIADD_DWORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt32AsDouble(data);
	FPUReg reg(data, 0, true);
	data->fAdd(reg.reg, reg.reg, vTmpReg, D_scaler);
	data->vReleaseTmpReg(vTmpReg);
}
void opFIMUL_DWORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt32AsDouble(data);
	FPUReg reg(data, 0, true);
	data->fMul(reg.reg, reg.reg, vTmpReg, D_scaler);
	data->vReleaseTmpReg(vTmpReg);
}
void opFICOM_DWORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt32AsDouble(data);
	FPUReg reg(data, 0, false);
	doFCOM(data, reg.reg, 0, vTmpReg, -1);
	data->vReleaseTmpReg(vTmpReg);
}
void opFICOM_DWORD_INTEGER_Pop(Armv8btAsm* data) {
	U8 vTmpReg = loadInt32AsDouble(data);
	FPUReg reg(data, 0, false);
	doFCOM(data, reg.reg, 0, vTmpReg, -1);
	data->vReleaseTmpReg(vTmpReg);
	FPU_POP(data);
}
void opFISUB_DWORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt32AsDouble(data);
	FPUReg reg(data, 0, true);
	data->fSub(reg.reg, reg.reg, vTmpReg, D_scaler);
	data->vReleaseTmpReg(vTmpReg);
}
void opFISUBR_DWORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt32AsDouble(data);
	FPUReg reg(data, 0, true);
	data->fSub(reg.reg, vTmpReg, reg.reg, D_scaler);
	data->vReleaseTmpReg(vTmpReg);
}
void opFIDIV_DWORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt32AsDouble(data);
	FPUReg reg(data, 0, true);
	data->fDiv(reg.reg, reg.reg, vTmpReg, D_scaler);
	data->vReleaseTmpReg(vTmpReg);
}
void opFIDIVR_DWORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt32AsDouble(data);
	FPUReg reg(data, 0, true);
	data->fDiv(reg.reg, vTmpReg, reg.reg, D_scaler);
	data->vReleaseTmpReg(vTmpReg);
}

void opFCMOV_ST0_STj_NCF(Armv8btAsm* data) {
	doCondition(data, condional_NB, [data] {
		doCMov(data);
		});
}
void opFCMOV_ST0_STj_NZF(Armv8btAsm* data) {
	doCondition(data, condional_NZ, [data] {
		doCMov(data);
		});
}
void opFCMOV_ST0_STj_NCF_AND_NZF(Armv8btAsm* data) {
	doCondition(data, condional_NBE, [data] {
		doCMov(data);
		});
}
void opFCMOV_ST0_STj_NPF(Armv8btAsm* data) {
	doCondition(data, condional_NP, [data] {
		doCMov(data);
		});
}
void opFNCLEX(Armv8btAsm* data) {
	// this->sw &= 0x7f00;
	U8 tmpReg = data->getTmpReg();
	data->readMem32ValueOffset(tmpReg, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
	data->andValue32(tmpReg, tmpReg, 0x7f00);
	data->writeMem32ValueOffset(tmpReg, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
	data->releaseTmpReg(tmpReg);
}
void opFNINIT(Armv8btAsm* data) {
	data->fpuTopRegSet = true;
	data->clearCachedFpuRegs();
	data->zeroReg(xFpuTop);
	data->writeMem32ValueOffset(xFpuTop, data->getFpuOffset(), (U32)(offsetof(FPU, top)));
	data->writeMem32ValueOffset(xFpuTop, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
}
void opFUCOMI_ST0_STj(Armv8btAsm* data) {
	FPUReg dst(data, 0, false);
	FPUReg src(data, data->decodedOp->reg, false);
	// :TODO: FCOM and FUCOM currently do the same thing
	doFCOMI(data, dst.reg, 0, src.reg, data->decodedOp->reg);
}
void opFCOMI_ST0_STj(Armv8btAsm* data) {
	FPUReg dst(data, 0, false);
	FPUReg src(data, data->decodedOp->reg, false);
	doFCOMI(data, dst.reg, 0, src.reg, data->decodedOp->reg);
}
void opFILD_DWORD_INTEGER(Armv8btAsm* data) {
	// U32 value = readd(address); // might generate PF, so do before we adjust the stack
	// cpu->fpu.PREP_PUSH();
	// cpu->fpu.FLD_I32(value, cpu->fpu.STV(0));
	U8 vTmpReg = loadInt32AsDouble(data);
	PREP_PUSH(data);
	data->vWriteMem64RegOffset(vTmpReg, data->getFpuOffset(), data->getFpuTopReg(), 3);
	data->vReleaseTmpReg(vTmpReg);
}
void opFISTTP32(Armv8btAsm* data) {
	// cpu->fpu.FSTT_I32(cpu, address);
	// cpu->fpu.FPOP();
	U8 addressReg = data->getAddressReg();
	FPUReg reg(data, 0, false);
	U8 tmpReg = data->getTmpReg();
	data->vConvertDoubleToGeneralReg32RoundToZero(tmpReg, reg.reg);
	data->writeMemory(addressReg, tmpReg, 32, true);
	data->releaseTmpReg(tmpReg);
	data->releaseTmpReg(addressReg);
	FPU_POP(data);
}
void opFIST_DWORD_INTEGER(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	FPUReg reg(data, 0, false);
	U8 tmpReg = data->getTmpReg();
	// round based on control word
	data->vConvertDoubleToGeneralReg32RoundToNearest(tmpReg, reg.reg);
	data->writeMemory(addressReg, tmpReg, 32, true);
	data->releaseTmpReg(tmpReg);
	data->releaseTmpReg(addressReg);
}
void opFIST_DWORD_INTEGER_Pop(Armv8btAsm* data) {
	opFIST_DWORD_INTEGER(data);
	FPU_POP(data);
}
void opFLD_EXTENDED_REAL(Armv8btAsm* data) {
	// void common_FLD_EXTENDED_REAL(CPU * cpu, U32 address)
	U8 addressReg = data->getAddressReg();
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->mov64(1, addressReg);
	data->releaseTmpReg(addressReg);
	data->callHost((void*)common_FLD_EXTENDED_REAL);
	data->syncRegsToHost();
}
void opFSTP_EXTENDED_REAL(Armv8btAsm* data) {
	// void common_FSTP_EXTENDED_REAL(CPU* cpu, U32 address) 
	U8 addressReg = data->getAddressReg();
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->mov64(1, addressReg);
	data->releaseTmpReg(addressReg);
	data->callHost((void*)common_FSTP_EXTENDED_REAL);
	data->syncRegsToHost();
}

void opFADD_STi_ST0(Armv8btAsm* data) {
	FPUReg dst(data, data->decodedOp->reg, true);
	FPUReg src(data, 0, false);
	data->fAdd(dst.reg, dst.reg, src.reg, D_scaler);
}
void opFMUL_STi_ST0(Armv8btAsm* data) {
	FPUReg dst(data, data->decodedOp->reg, true);
	FPUReg src(data, 0, false);
	data->fMul(dst.reg, dst.reg, src.reg, D_scaler);
}
void opFSUBR_STi_ST0(Armv8btAsm* data) {
	FPUReg dst(data, data->decodedOp->reg, true);
	FPUReg src(data, 0, false);
	data->fSub(dst.reg, src.reg, dst.reg, D_scaler);
}
void opFSUB_STi_ST0(Armv8btAsm* data) {
	FPUReg dst(data, data->decodedOp->reg, true);
	FPUReg src(data, 0, false);
	data->fSub(dst.reg, dst.reg, src.reg, D_scaler);
}
void opFDIVR_STi_ST0(Armv8btAsm* data) {
	FPUReg dst(data, data->decodedOp->reg, true);
	FPUReg src(data, 0, false);
	data->fDiv(dst.reg, src.reg, dst.reg, D_scaler);
}
void opFDIV_STi_ST0(Armv8btAsm* data) {
	FPUReg dst(data, data->decodedOp->reg, true);
	FPUReg src(data, 0, false);
	data->fDiv(dst.reg, dst.reg, src.reg, D_scaler);
}
void opFADD_DOUBLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readDouble(data);
	FPUReg dst(data, 0, true);
	data->fAdd(dst.reg, dst.reg, tmpRead, D_scaler);
	data->vReleaseTmpReg(tmpRead);
}
void opFMUL_DOUBLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readDouble(data);
	FPUReg dst(data, 0, true);
	data->fMul(dst.reg, dst.reg, tmpRead, D_scaler);
	data->vReleaseTmpReg(tmpRead);
}
void opFCOM_DOUBLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readDouble(data);
	FPUReg dst(data, 0, false);
	doFCOM(data, dst.reg, 0, tmpRead, -1);
	data->vReleaseTmpReg(tmpRead);
}
void opFCOM_DOUBLE_REAL_Pop(Armv8btAsm* data) {
	U8 tmpRead = readDouble(data);
	FPUReg dst(data, 0, false);
	doFCOM(data, dst.reg, 0, tmpRead, -1);
	data->vReleaseTmpReg(tmpRead);
	FPU_POP(data);
}
void opFSUB_DOUBLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readDouble(data);
	FPUReg dst(data, 0, true);
	data->fSub(dst.reg, dst.reg, tmpRead, D_scaler);
	data->vReleaseTmpReg(tmpRead);
}
void opFSUBR_DOUBLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readDouble(data);
	FPUReg dst(data, 0, true);
	data->fSub(dst.reg, tmpRead, dst.reg, D_scaler);
	data->vReleaseTmpReg(tmpRead);
}
void opFDIV_DOUBLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readDouble(data);
	FPUReg dst(data, 0, true);
	data->fDiv(dst.reg, dst.reg, tmpRead, D_scaler);
	data->vReleaseTmpReg(tmpRead);
}
void opFDIVR_DOUBLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readDouble(data);
	FPUReg dst(data, 0, true);
	data->fDiv(dst.reg, tmpRead, dst.reg, D_scaler);
	data->vReleaseTmpReg(tmpRead);
}

void opFFREE_STi(Armv8btAsm* data) {
	// cpu->fpu.FFREE_STi(cpu->fpu.STV(reg));
	hostWriteTag(data, data->getRegWithConst(TAG_Empty), true, data->decodedOp->reg);
}
void opFST_STi(Armv8btAsm* data) {
	// cpu->fpu.FST(cpu->fpu.STV(0), cpu->fpu.STV(reg));
	FPUReg src(data, 0, false);
	FPUReg dst(data, data->decodedOp->reg, true, false);

	U8 tagReg = data->getTmpReg();
	src.hostReadTag(tagReg);
	data->vMov64(dst.reg, 0, src.reg, 0);	
	dst.hostWriteTag(tagReg);
}
void opFUCOM_STi(Armv8btAsm* data) {
	FPUReg dst(data, 0, false);
	FPUReg src(data, data->decodedOp->reg, false);
	// :TODO: FCOM and FUCOM currently do the same thing
	doFCOM(data, dst.reg, 0, src.reg, data->decodedOp->reg);
}
void opFUCOM_STi_Pop(Armv8btAsm* data) {
	FPUReg dst(data, 0, false);
	FPUReg src(data, data->decodedOp->reg, false);
	// :TODO: FCOM and FUCOM currently do the same thing
	doFCOM(data, dst.reg, 0, src.reg, data->decodedOp->reg);
	FPU_POP(data);
}
void opFLD_DOUBLE_REAL(Armv8btAsm* data) {
	// U64 value = readq(address); // might generate PF, so do before we adjust the stack
	// cpu->fpu.PREP_PUSH();
	// cpu->fpu.FLD_F64(value, cpu->fpu.STV(0));
	readDouble(data, vMMX0);
	PREP_PUSH(data);
	hostSaveDouble(data, 0);
}
void opFISTTP64(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	FPUReg reg(data, 0, false);
	U8 tmpReg = data->getTmpReg();
	data->vConvertDoubleToInt64RoundToZero(tmpReg, reg.reg, false);
	data->writeMemory(addressReg, tmpReg, 64, true);
	data->releaseTmpReg(tmpReg);
	data->releaseTmpReg(addressReg);
	FPU_POP(data);
}
void opFST_DOUBLE_REAL(Armv8btAsm* data) {
	FPUReg src(data, 0, false);
	U8 addressReg = data->getAddressReg();
	data->vWriteMemory64(addressReg, src.reg, 0, true);
	data->releaseTmpReg(addressReg);
}
void opFST_DOUBLE_REAL_Pop(Armv8btAsm* data) {
	FPUReg src(data, 0, false);
	U8 addressReg = data->getAddressReg();
	data->vWriteMemory64(addressReg, src.reg, 0, true);
	data->releaseTmpReg(addressReg);
	FPU_POP(data);
}
void opFRSTOR(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->mov64(1, addressReg);
	data->releaseTmpReg(addressReg);
	data->callHost((void*)common_FRSTOR);
	data->syncRegsToHost();
}
void opFNSAVE(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->mov64(1, addressReg);
	data->releaseTmpReg(addressReg);
	data->callHost((void*)common_FNSAVE);
	data->syncRegsToHost();
}
void opFNSTSW(Armv8btAsm* data) {
	// (fpu)->sw &= ~0x3800; (fpu)->sw |= (top & 7) << 11
	// writew(address, cpu->fpu.SW());
	U8 tmpReg = data->getTmpReg();
	data->readMem32ValueOffset(tmpReg, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
	data->andValue32(tmpReg, tmpReg, ~0x3800);
	data->orRegs32(tmpReg, tmpReg, data->getFpuTopReg(), 11);
	U8 addressReg = data->getAddressReg();
	data->writeMemory(addressReg, tmpReg, 16, true);
	data->releaseTmpReg(addressReg);
	data->releaseTmpReg(tmpReg);
}

void opFADD_STi_ST0_Pop(Armv8btAsm* data) {
	FPUReg dst(data, data->decodedOp->reg, true);
	FPUReg src(data, 0, false);
	data->fAdd(dst.reg, dst.reg, src.reg, D_scaler);
	FPU_POP(data);
}
void opFMUL_STi_ST0_Pop(Armv8btAsm* data) {
	FPUReg dst(data, data->decodedOp->reg, true);
	FPUReg src(data, 0, false);
	data->fMul(dst.reg, dst.reg, src.reg, D_scaler);
	FPU_POP(data);
}
void opFCOMPP(Armv8btAsm* data) {
	FPUReg reg1(data, 0, false);
	FPUReg reg2(data, 1, false);
	doFCOM(data, reg1.reg, 0, reg2.reg, 1);
	FPU_POP(data);
	FPU_POP(data);
}
void opFSUBR_STi_ST0_Pop(Armv8btAsm* data) {
	FPUReg dst(data, data->decodedOp->reg, true);
	FPUReg src(data, 0, false);
	data->fSub(dst.reg, src.reg, dst.reg, D_scaler);
	FPU_POP(data);
}
void opFSUB_STi_ST0_Pop(Armv8btAsm* data) {
	FPUReg dst(data, data->decodedOp->reg, true);
	FPUReg src(data, 0, false);
	data->fSub(dst.reg, dst.reg, src.reg, D_scaler);
	FPU_POP(data);
}
void opFDIVR_STi_ST0_Pop(Armv8btAsm* data) {
	FPUReg dst(data, data->decodedOp->reg, true);
	FPUReg src(data, 0, false);
	data->fDiv(dst.reg, src.reg, dst.reg, D_scaler);
	FPU_POP(data);
}
void opFDIV_STi_ST0_Pop(Armv8btAsm* data) {
	FPUReg dst(data, data->decodedOp->reg, true);
	FPUReg src(data, 0, false);
	data->fDiv(dst.reg, dst.reg, src.reg, D_scaler);
	FPU_POP(data);
}
void opFIADD_WORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt16AsDouble(data);
	FPUReg reg(data, 0, true);
	data->fAdd(reg.reg, reg.reg, vTmpReg, D_scaler);
	data->vReleaseTmpReg(vTmpReg);
}
void opFIMUL_WORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt16AsDouble(data);
	FPUReg reg(data, 0, true);
	data->fMul(reg.reg, reg.reg, vTmpReg, D_scaler);
	data->vReleaseTmpReg(vTmpReg);
}
void opFICOM_WORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt16AsDouble(data);
	FPUReg reg(data, 0, false);
	doFCOM(data, reg.reg, 0, vTmpReg, -1);
	data->vReleaseTmpReg(vTmpReg);
}
void opFICOM_WORD_INTEGER_Pop(Armv8btAsm* data) {
	U8 vTmpReg = loadInt32AsDouble(data);
	FPUReg reg(data, 0, false);
	doFCOM(data, reg.reg, 0, vTmpReg, -1);
	data->vReleaseTmpReg(vTmpReg);
	FPU_POP(data);
}
void opFISUB_WORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt16AsDouble(data);
	FPUReg reg(data, 0, true);
	data->fSub(reg.reg, reg.reg, vTmpReg, D_scaler);
	data->vReleaseTmpReg(vTmpReg);
}
void opFISUBR_WORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt16AsDouble(data);
	FPUReg reg(data, 0, true);
	data->fSub(reg.reg, vTmpReg, reg.reg, D_scaler);
	data->vReleaseTmpReg(vTmpReg);
}
void opFIDIV_WORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt16AsDouble(data);
	FPUReg reg(data, 0, true);
	data->fDiv(reg.reg, reg.reg, vTmpReg, D_scaler);
	data->vReleaseTmpReg(vTmpReg);
}
void opFIDIVR_WORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt16AsDouble(data);
	FPUReg reg(data, 0, true);
	data->fDiv(reg.reg, vTmpReg, reg.reg, D_scaler);
	data->vReleaseTmpReg(vTmpReg);
}
void opFFREEP_STi(Armv8btAsm* data) {
	hostWriteTag(data, data->getRegWithConst(TAG_Empty), true, data->decodedOp->reg);
	FPU_POP(data);
}
void opFNSTSW_AX(Armv8btAsm* data) {
	// AX = (fpu->sw &= ~0x3800) | (top & 7) << 11
	U8 tmpReg = data->getTmpReg();
	data->readMem32ValueOffset(tmpReg, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
	data->andValue32(tmpReg, tmpReg, ~0x3800);
	data->orRegs32(tmpReg, tmpReg, data->getFpuTopReg(), 11);
	data->movRegToReg(xEAX, tmpReg, 16, false);
	data->releaseTmpReg(tmpReg);
}
void opFUCOMI_ST0_STj_Pop(Armv8btAsm* data) {
	FPUReg dst(data, 0, false);
	FPUReg src(data, data->decodedOp->reg, false);
	// :TODO: FCOM and FUCOM currently do the same thing
	doFCOMI(data, dst.reg, 0, src.reg, data->decodedOp->reg);
	FPU_POP(data);
}
void opFCOMI_ST0_STj_Pop(Armv8btAsm* data) {
	FPUReg dst(data, 0, false);
	FPUReg src(data, data->decodedOp->reg, false);
	doFCOMI(data, dst.reg, 0, src.reg, data->decodedOp->reg);
	FPU_POP(data);
}
void opFILD_WORD_INTEGER(Armv8btAsm* data) {
	// S16 value = (S16)readw(address); // might generate PF, so do before we adjust the stack
	// cpu->fpu.PREP_PUSH();
	// cpu->fpu.FLD_I16(value, cpu->fpu.STV(0));
	U8 vTmpReg = loadInt16AsDouble(data);
	PREP_PUSH(data);
	data->vWriteMem64RegOffset(vTmpReg, data->getFpuOffset(), data->getFpuTopReg(), 3);
	data->vReleaseTmpReg(vTmpReg);
}
void opFISTTP16(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	FPUReg reg(data, 0, false);
	U8 tmpReg = data->getTmpReg();
	data->vConvertDoubleToGeneralReg32RoundToZero(tmpReg, reg.reg);
	data->writeMemory(addressReg, tmpReg, 16, true);
	data->releaseTmpReg(tmpReg);
	data->releaseTmpReg(addressReg);
	FPU_POP(data);
}
void opFIST_WORD_INTEGER(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	FPUReg reg(data, 0, false);
	U8 tmpReg = data->getTmpReg();
	// round based on control word
	data->vConvertDoubleToGeneralReg32RoundToNearest(tmpReg, reg.reg);
	data->writeMemory(addressReg, tmpReg, 16, true);
	data->releaseTmpReg(tmpReg);
	data->releaseTmpReg(addressReg);
}
void opFIST_WORD_INTEGER_Pop(Armv8btAsm* data) {
	opFIST_WORD_INTEGER(data);
	FPU_POP(data);
}
void opFBLD_PACKED_BCD(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->mov64(1, addressReg);
	data->releaseTmpReg(addressReg);
	data->callHost((void*)common_FBLD_PACKED_BCD);
	data->syncRegsToHost();
}
void opFILD_QWORD_INTEGER(Armv8btAsm* data) {
	U8 vTmpReg = loadInt64AsDouble(data);
	PREP_PUSH(data);
	data->vWriteMem64RegOffset(vTmpReg, data->getFpuOffset(), data->getFpuTopReg(), 3);
	data->vReleaseTmpReg(vTmpReg);
}
void opFBSTP_PACKED_BCD(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	data->syncRegsFromHost();
	data->mov64(0, xCPU); // param 1 (CPU)
	data->mov64(1, addressReg);
	data->releaseTmpReg(addressReg);
	data->callHost((void*)common_FBSTP_PACKED_BCD);
	data->syncRegsToHost();
}
void opFISTP_QWORD_INTEGER(Armv8btAsm* data) {
	U8 addressReg = data->getAddressReg();
	FPUReg reg(data, 0, false);
	// round based on control word
	data->vConvertDoubleToInt64RoundToCurrentMode(reg.reg, reg.reg, false);
	data->vWriteMemory64(addressReg, reg.reg, 0, true);
	data->releaseTmpReg(addressReg);
}

#endif