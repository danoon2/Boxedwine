#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
#include "armv8btOps.h"

static U8 calculateIndexReg(Armv8btAsm* data, U32 index) {
	U8 result = data->vGetTmpReg();
	data->addValue32(result, data->getFpuTopReg(), index);
	data->andValue32(result, result, 7);
	return result;
}

static void hostWriteTag(Armv8btAsm* data, U32 value, U32 index) {
	U8 valueReg = data->getRegWithConst(value);
	U8 offsetReg = data->getTmpReg();
	data->addValue64(offsetReg, data->getFpuOffset(), (U32)(offsetof(FPU, tags)));

	if (index == 0) {
		data->writeMem32RegOffset(valueReg, offsetReg, data->getFpuTopReg(), 2);
	} else {
		U8 topReg = calculateIndexReg(data, index);
		data->writeMem32RegOffset(valueReg, offsetReg, topReg, 2);
		data->releaseTmpReg(topReg);
	}

	data->releaseTmpReg(valueReg);
	data->releaseTmpReg(offsetReg);
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
		reg = data->getTmpReg();
	}
	U8 addressReg = data->getAddressReg();
	data->vReadMemory32(addressReg, reg, true);
	data->vConvertFloatToDouble(reg, reg, false);
	data->releaseTmpReg(addressReg);
	return reg;
}

class FPUReg {
public:
	FPUReg(Armv8btAsm* data, U32 index, bool writeBack) : data(data), writeBack(writeBack), topReg(0) {
		this->reg = index + vMMX0;
		if (!data->isFpuRegCached[index]) {
			if (index == 0) {
				data->readMem64RegOffset(this->reg, data->getFpuOffset(), data->getFpuTopReg(), 3);
			} else {
				this->topReg = calculateIndexReg(data, index);
				data->vReadMem64RegOffset(this->reg, data->getFpuOffset(), this->topReg, 3);
				if (!writeBack) {
					data->releaseTmpReg(this->topReg);
				}
			}
			data->isFpuRegCached[index] = true;
		}
	}
	~FPUReg() {
		if (this->writeBack) {
			if (this->topReg == 0) {
				data->vWriteMem64RegOffset(this->reg, data->getFpuOffset(), data->getFpuTopReg(), 3);
			} else {
				data->vWriteMem64RegOffset(this->reg, data->getFpuOffset(), this->topReg, 3);
				data->releaseTmpReg(this->topReg);
			}
		}	
	}
	
	Armv8btAsm* data;
	bool writeBack;
	U8 reg;
	U8 topReg;
};


static void PREP_PUSH(Armv8btAsm* data) {
	U8 topReg = data->getFpuTopReg();
	data->subValue32(topReg, topReg, 1);
	data->andValue32(topReg, topReg, 7);
	data->writeMem32ValueOffset(topReg, data->getFpuOffset(), (U32)(offsetof(FPU, top)));
	hostWriteTag(data, TAG_Valid, 0);
	data->clearCachedFpuRegs();
}

static void FPU_POP(Armv8btAsm* data) {
	// this->tags[this->top] = TAG_Empty;
	// this->top = ((this->top + 1) & 7);
	hostWriteTag(data, TAG_Empty, 0);

	U8 topReg = data->getFpuTopReg();
	data->addValue32(topReg, topReg, 1);
	data->andValue32(topReg, topReg, 7);
	data->writeMem32ValueOffset(topReg, data->getFpuOffset(), (U32)(offsetof(FPU, top)));
	data->clearCachedFpuRegs();
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

	U8 tmpReg = data->getTmpReg();
	U8 tmpReg2 = data->getTmpReg();
	U8 offsetReg = data->getTmpReg();

	data->addValue64(offsetReg, data->getFpuOffset(), (U32)(offsetof(FPU, tags)));
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
	data->releaseTmpReg(offsetReg);
	if (tagIndex2 != -1) {
		data->orRegs32(tmpReg, tmpReg, tmpReg2);
	}
	data->readMem32ValueOffset(tmpReg2, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));	

	data->doIf(tmpReg, 0, DO_IF_EQUAL, [data, tmpReg2, v1, v2] {
			// if both tags are TAG_Valid
			data->fCmp64(v1, v2);
			data->doIf(0, 0, DO_IF_EQUAL, [data, tmpReg2] {
				data->andValue32(tmpReg2, tmpReg2, ~0x0700);
				data->orValue32(tmpReg2, tmpReg2, 0x4000);
				}, [data, tmpReg2] {
					data->doIf(0, 0, DO_IF_SIGNED_GREATER_THAN, [data, tmpReg2] {
						data->andValue32(tmpReg2, tmpReg2, ~0x4700);
						}, [data, tmpReg2] {
							data->doIf(0, 0, DO_IF_LESS_THAN, [data, tmpReg2] {
								data->andValue32(tmpReg2, tmpReg2, ~0x4600);
								data->orValue32(tmpReg2, tmpReg2, 0x0100);
								}, [data, tmpReg2] {
									data->andValue32(tmpReg2, tmpReg2, ~0x0200);
									data->orValue32(tmpReg2, tmpReg2, 0x4500);
								}, nullptr, false, false);
						}, nullptr, false, false);
				}, nullptr, false, false);
		}, [data, tmpReg2] {
			// if either tag is not TAG_Valid
			data->orValue32(tmpReg2, tmpReg2, 0x4500);
		});
	data->releaseTmpReg(tmpReg);
	data->writeMem32ValueOffset(tmpReg2, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
	data->releaseTmpReg(tmpReg2);
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
	data->releaseTmpReg(tmpRead);
}
void opFMUL_SINGLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, true);
	data->fMul(dst.reg, dst.reg, tmpRead, D_scaler);
	data->releaseTmpReg(tmpRead);
}
void opFCOM_SINGLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, false);
	doFCOM(data, dst.reg, 0, tmpRead, -1);
	data->releaseTmpReg(tmpRead);	
}
void opFCOM_SINGLE_REAL_Pop(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, false);
	doFCOM(data, dst.reg, 0, tmpRead, -1);
	data->releaseTmpReg(tmpRead);
	FPU_POP(data);
}
void opFSUB_SINGLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, true);
	data->fSub(dst.reg, dst.reg, tmpRead, D_scaler);
	data->releaseTmpReg(tmpRead);
}
void opFSUBR_SINGLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, true);
	data->fSub(dst.reg, tmpRead, dst.reg, D_scaler);
	data->releaseTmpReg(tmpRead);
}
void opFDIV_SINGLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, true);
	data->fDiv(dst.reg, dst.reg, tmpRead, D_scaler);
	data->releaseTmpReg(tmpRead);
}
void opFDIVR_SINGLE_REAL(Armv8btAsm* data) {
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, true);
	data->fDiv(dst.reg, tmpRead, dst.reg, D_scaler);
	data->releaseTmpReg(tmpRead);
}
void opFLD_STi(Armv8btAsm* data) {
	kpanic("1");
}
void opFXCH_STi(Armv8btAsm* data) {
	kpanic("1");
}
void opFNOP(Armv8btAsm* data) {
	kpanic("1");
}
void opFST_STi_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFCHS(Armv8btAsm* data) {
	kpanic("1");
}
void opFABS(Armv8btAsm* data) {
	kpanic("1");
}
void opFTST(Armv8btAsm* data) {
	kpanic("1");
}
void opFXAM(Armv8btAsm* data) {
	kpanic("1");
}
void opFLD1(Armv8btAsm* data) {
	kpanic("1");
}
void opFLDL2T(Armv8btAsm* data) {
	kpanic("1");
}
void opFLDL2E(Armv8btAsm* data) {
	kpanic("1");
}
void opFLDPI(Armv8btAsm* data) {
	kpanic("1");
}
void opFLDLG2(Armv8btAsm* data) {
	kpanic("1");
}
void opFLDLN2(Armv8btAsm* data) {
	kpanic("1");
}
void opFLDZ(Armv8btAsm* data) {
	kpanic("1");
}
void opF2XM1(Armv8btAsm* data) {
	kpanic("1");
}
void opFYL2X(Armv8btAsm* data) {
	kpanic("1");
}
void opFPTAN(Armv8btAsm* data) {
	kpanic("1");
}
void opFPATAN(Armv8btAsm* data) {
	kpanic("1");
}
void opFXTRACT(Armv8btAsm* data) {
	kpanic("1");
}
void opFPREM_nearest(Armv8btAsm* data) {
	kpanic("1");
}
void opFDECSTP(Armv8btAsm* data) {
	kpanic("1");
}
void opFINCSTP(Armv8btAsm* data) {
	kpanic("1");
}
void opFPREM(Armv8btAsm* data) {
	kpanic("1");
}
void opFYL2XP1(Armv8btAsm* data) {
	kpanic("1");
}
void opFSQRT(Armv8btAsm* data) {
	kpanic("1");
}
void opFSINCOS(Armv8btAsm* data) {
	kpanic("1");
}
void opFRNDINT(Armv8btAsm* data) {
	kpanic("1");
}
void opFSCALE(Armv8btAsm* data) {
	kpanic("1");
}
void opFSIN(Armv8btAsm* data) {
	kpanic("1");
}
void opFCOS(Armv8btAsm* data) {
	kpanic("1");
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
	kpanic("1");
}
void opFLDENV(Armv8btAsm* data) {
	kpanic("1");
}
void opFLDCW(Armv8btAsm* data) {
	kpanic("1");
}
void opFNSTENV(Armv8btAsm* data) {
	kpanic("1");
}
void opFNSTCW(Armv8btAsm* data) {
	kpanic("1");
}

void opFCMOV_ST0_STj_CF(Armv8btAsm* data) {
	kpanic("1");
}
void opFCMOV_ST0_STj_ZF(Armv8btAsm* data) {
	kpanic("1");
}
void opFCMOV_ST0_STj_CF_OR_ZF(Armv8btAsm* data) {
	kpanic("1");
}
void opFCMOV_ST0_STj_PF(Armv8btAsm* data) {
	kpanic("1");
}
void opFUCOMPP(Armv8btAsm* data) {
	kpanic("1");
}
void opFIADD_DWORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFIMUL_DWORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFICOM_DWORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFICOM_DWORD_INTEGER_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFISUB_DWORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFISUBR_DWORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFIDIV_DWORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFIDIVR_DWORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}

void opFCMOV_ST0_STj_NCF(Armv8btAsm* data) {
	kpanic("1");
}
void opFCMOV_ST0_STj_NZF(Armv8btAsm* data) {
	kpanic("1");
}
void opFCMOV_ST0_STj_NCF_AND_NZF(Armv8btAsm* data) {
	kpanic("1");
}
void opFCMOV_ST0_STj_NPF(Armv8btAsm* data) {
	kpanic("1");
}
void opFNCLEX(Armv8btAsm* data) {
	kpanic("1");
}
void opFNINIT(Armv8btAsm* data) {
	data->fpuTopRegSet = true;
	data->clearCachedFpuRegs();
	data->zeroReg(xFpuTop);
	data->writeMem32ValueOffset(xFpuTop, data->getFpuOffset(), (U32)(offsetof(FPU, top)));
	data->writeMem32ValueOffset(xFpuTop, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
}
void opFUCOMI_ST0_STj(Armv8btAsm* data) {
	kpanic("1");
}
void opFCOMI_ST0_STj(Armv8btAsm* data) {
	kpanic("1");
}
void opFILD_DWORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFISTTP32(Armv8btAsm* data) {
	kpanic("1");
}
void opFIST_DWORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFIST_DWORD_INTEGER_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFLD_EXTENDED_REAL(Armv8btAsm* data) {
	kpanic("1");
}
void opFSTP_EXTENDED_REAL(Armv8btAsm* data) {
	kpanic("1");
}

void opFADD_STi_ST0(Armv8btAsm* data) {
	kpanic("1");
}
void opFMUL_STi_ST0(Armv8btAsm* data) {
	kpanic("1");
}
void opFSUBR_STi_ST0(Armv8btAsm* data) {
	kpanic("1");
}
void opFSUB_STi_ST0(Armv8btAsm* data) {
	kpanic("1");
}
void opFDIVR_STi_ST0(Armv8btAsm* data) {
	kpanic("1");
}
void opFDIV_STi_ST0(Armv8btAsm* data) {
	kpanic("1");
}
void opFADD_DOUBLE_REAL(Armv8btAsm* data) {
	kpanic("1");
}
void opFMUL_DOUBLE_REAL(Armv8btAsm* data) {
	kpanic("1");
}
void opFCOM_DOUBLE_REAL(Armv8btAsm* data) {
	kpanic("1");
}
void opFCOM_DOUBLE_REAL_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFSUB_DOUBLE_REAL(Armv8btAsm* data) {
	kpanic("1");
}
void opFSUBR_DOUBLE_REAL(Armv8btAsm* data) {
	kpanic("1");
}
void opFDIV_DOUBLE_REAL(Armv8btAsm* data) {
	kpanic("1");
}
void opFDIVR_DOUBLE_REAL(Armv8btAsm* data) {
	kpanic("1");
}

void opFFREE_STi(Armv8btAsm* data) {
	kpanic("1");
}
void opFST_STi(Armv8btAsm* data) {
	kpanic("1");
}
void opFUCOM_STi(Armv8btAsm* data) {
	kpanic("1");
}
void opFUCOM_STi_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFLD_DOUBLE_REAL(Armv8btAsm* data) {
	kpanic("1");
}
void opFISTTP64(Armv8btAsm* data) {
	kpanic("1");
}
void opFST_DOUBLE_REAL(Armv8btAsm* data) {
	kpanic("1");
}
void opFST_DOUBLE_REAL_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFRSTOR(Armv8btAsm* data) {
	kpanic("1");
}
void opFNSAVE(Armv8btAsm* data) {
	kpanic("1");
}
void opFNSTSW(Armv8btAsm* data) {
	kpanic("1");
}

void opFADD_STi_ST0_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFMUL_STi_ST0_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFCOMPP(Armv8btAsm* data) {
	kpanic("1");
}
void opFSUBR_STi_ST0_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFSUB_STi_ST0_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFDIVR_STi_ST0_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFDIV_STi_ST0_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFIADD_WORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFIMUL_WORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFICOM_WORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFICOM_WORD_INTEGER_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFISUB_WORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFISUBR_WORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFIDIV_WORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFIDIVR_WORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}

void opFFREEP_STi(Armv8btAsm* data) {
	kpanic("1");
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
	kpanic("1");
}
void opFCOMI_ST0_STj_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFILD_WORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFISTTP16(Armv8btAsm* data) {
	kpanic("1");
}
void opFIST_WORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFIST_WORD_INTEGER_Pop(Armv8btAsm* data) {
	kpanic("1");
}
void opFBLD_PACKED_BCD(Armv8btAsm* data) {
	kpanic("1");
}
void opFILD_QWORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}
void opFBSTP_PACKED_BCD(Armv8btAsm* data) {
	kpanic("1");
}
void opFISTP_QWORD_INTEGER(Armv8btAsm* data) {
	kpanic("1");
}

#endif