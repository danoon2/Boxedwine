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
void opFCOM_STi(Armv8btAsm* data) {}
void opFCOM_STi_Pop(Armv8btAsm* data) {}
void opFSUB_ST0_STj(Armv8btAsm* data) {}
void opFSUBR_ST0_STj(Armv8btAsm* data) {}
void opFDIV_ST0_STj(Armv8btAsm* data) {}
void opFDIVR_ST0_STj(Armv8btAsm* data) {}
void opFADD_SINGLE_REAL(Armv8btAsm* data) {
	// cpu->fpu.FLD_F32_EA(cpu, address);
	// cpu->fpu.FADD_EA();
	U8 tmpRead = readFloat(data);
	FPUReg dst(data, 0, true);
	data->fAdd(dst.reg, dst.reg, tmpRead, D_scaler);
	data->releaseTmpReg(tmpRead);
}
void opFMUL_SINGLE_REAL(Armv8btAsm* data) {}
void opFCOM_SINGLE_REAL(Armv8btAsm* data) {}
void opFCOM_SINGLE_REAL_Pop(Armv8btAsm* data) {}
void opFSUB_SINGLE_REAL(Armv8btAsm* data) {}
void opFSUBR_SINGLE_REAL(Armv8btAsm* data) {}
void opFDIV_SINGLE_REAL(Armv8btAsm* data) {}
void opFDIVR_SINGLE_REAL(Armv8btAsm* data) {}

void opFLD_STi(Armv8btAsm* data) {}
void opFXCH_STi(Armv8btAsm* data) {}
void opFNOP(Armv8btAsm* data) {}
void opFST_STi_Pop(Armv8btAsm* data) {}
void opFCHS(Armv8btAsm* data) {}
void opFABS(Armv8btAsm* data) {}
void opFTST(Armv8btAsm* data) {}
void opFXAM(Armv8btAsm* data) {}
void opFLD1(Armv8btAsm* data) {}
void opFLDL2T(Armv8btAsm* data) {}
void opFLDL2E(Armv8btAsm* data) {}
void opFLDPI(Armv8btAsm* data) {}
void opFLDLG2(Armv8btAsm* data) {}
void opFLDLN2(Armv8btAsm* data) {}
void opFLDZ(Armv8btAsm* data) {}
void opF2XM1(Armv8btAsm* data) {}
void opFYL2X(Armv8btAsm* data) {}
void opFPTAN(Armv8btAsm* data) {}
void opFPATAN(Armv8btAsm* data) {}
void opFXTRACT(Armv8btAsm* data) {}
void opFPREM_nearest(Armv8btAsm* data) {}
void opFDECSTP(Armv8btAsm* data) {}
void opFINCSTP(Armv8btAsm* data) {}
void opFPREM(Armv8btAsm* data) {}
void opFYL2XP1(Armv8btAsm* data) {}
void opFSQRT(Armv8btAsm* data) {}
void opFSINCOS(Armv8btAsm* data) {}
void opFRNDINT(Armv8btAsm* data) {}
void opFSCALE(Armv8btAsm* data) {}
void opFSIN(Armv8btAsm* data) {}
void opFCOS(Armv8btAsm* data) {}
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
void opFST_SINGLE_REAL_Pop(Armv8btAsm* data) {}
void opFLDENV(Armv8btAsm* data) {}
void opFLDCW(Armv8btAsm* data) {}
void opFNSTENV(Armv8btAsm* data) {}
void opFNSTCW(Armv8btAsm* data) {}

void opFCMOV_ST0_STj_CF(Armv8btAsm* data) {}
void opFCMOV_ST0_STj_ZF(Armv8btAsm* data) {}
void opFCMOV_ST0_STj_CF_OR_ZF(Armv8btAsm* data) {}
void opFCMOV_ST0_STj_PF(Armv8btAsm* data) {}
void opFUCOMPP(Armv8btAsm* data) {}
void opFIADD_DWORD_INTEGER(Armv8btAsm* data) {}
void opFIMUL_DWORD_INTEGER(Armv8btAsm* data) {}
void opFICOM_DWORD_INTEGER(Armv8btAsm* data) {}
void opFICOM_DWORD_INTEGER_Pop(Armv8btAsm* data) {}
void opFISUB_DWORD_INTEGER(Armv8btAsm* data) {}
void opFISUBR_DWORD_INTEGER(Armv8btAsm* data) {}
void opFIDIV_DWORD_INTEGER(Armv8btAsm* data) {}
void opFIDIVR_DWORD_INTEGER(Armv8btAsm* data) {}

void opFCMOV_ST0_STj_NCF(Armv8btAsm* data) {}
void opFCMOV_ST0_STj_NZF(Armv8btAsm* data) {}
void opFCMOV_ST0_STj_NCF_AND_NZF(Armv8btAsm* data) {}
void opFCMOV_ST0_STj_NPF(Armv8btAsm* data) {}
void opFNCLEX(Armv8btAsm* data) {}
void opFNINIT(Armv8btAsm* data) {
	data->fpuTopRegSet = true;
	data->clearCachedFpuRegs();
	data->zeroReg(xFpuTop);
	data->writeMem32ValueOffset(xFpuTop, data->getFpuOffset(), (U32)(offsetof(FPU, top)));
	data->writeMem32ValueOffset(xFpuTop, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
}
void opFUCOMI_ST0_STj(Armv8btAsm* data) {}
void opFCOMI_ST0_STj(Armv8btAsm* data) {}
void opFILD_DWORD_INTEGER(Armv8btAsm* data) {}
void opFISTTP32(Armv8btAsm* data) {}
void opFIST_DWORD_INTEGER(Armv8btAsm* data) {}
void opFIST_DWORD_INTEGER_Pop(Armv8btAsm* data) {}
void opFLD_EXTENDED_REAL(Armv8btAsm* data) {}
void opFSTP_EXTENDED_REAL(Armv8btAsm* data) {}

void opFADD_STi_ST0(Armv8btAsm* data) {}
void opFMUL_STi_ST0(Armv8btAsm* data) {}
void opFSUBR_STi_ST0(Armv8btAsm* data) {}
void opFSUB_STi_ST0(Armv8btAsm* data) {}
void opFDIVR_STi_ST0(Armv8btAsm* data) {}
void opFDIV_STi_ST0(Armv8btAsm* data) {}
void opFADD_DOUBLE_REAL(Armv8btAsm* data) {}
void opFMUL_DOUBLE_REAL(Armv8btAsm* data) {}
void opFCOM_DOUBLE_REAL(Armv8btAsm* data) {}
void opFCOM_DOUBLE_REAL_Pop(Armv8btAsm* data) {}
void opFSUB_DOUBLE_REAL(Armv8btAsm* data) {}
void opFSUBR_DOUBLE_REAL(Armv8btAsm* data) {}
void opFDIV_DOUBLE_REAL(Armv8btAsm* data) {}
void opFDIVR_DOUBLE_REAL(Armv8btAsm* data) {}

void opFFREE_STi(Armv8btAsm* data) {}
void opFST_STi(Armv8btAsm* data) {}
void opFUCOM_STi(Armv8btAsm* data) {}
void opFUCOM_STi_Pop(Armv8btAsm* data) {}
void opFLD_DOUBLE_REAL(Armv8btAsm* data) {}
void opFISTTP64(Armv8btAsm* data) {}
void opFST_DOUBLE_REAL(Armv8btAsm* data) {}
void opFST_DOUBLE_REAL_Pop(Armv8btAsm* data) {}
void opFRSTOR(Armv8btAsm* data) {}
void opFNSAVE(Armv8btAsm* data) {}
void opFNSTSW(Armv8btAsm* data) {}

void opFADD_STi_ST0_Pop(Armv8btAsm* data) {}
void opFMUL_STi_ST0_Pop(Armv8btAsm* data) {}
void opFCOMPP(Armv8btAsm* data) {}
void opFSUBR_STi_ST0_Pop(Armv8btAsm* data) {}
void opFSUB_STi_ST0_Pop(Armv8btAsm* data) {}
void opFDIVR_STi_ST0_Pop(Armv8btAsm* data) {}
void opFDIV_STi_ST0_Pop(Armv8btAsm* data) {}
void opFIADD_WORD_INTEGER(Armv8btAsm* data) {}
void opFIMUL_WORD_INTEGER(Armv8btAsm* data) {}
void opFICOM_WORD_INTEGER(Armv8btAsm* data) {}
void opFICOM_WORD_INTEGER_Pop(Armv8btAsm* data) {}
void opFISUB_WORD_INTEGER(Armv8btAsm* data) {}
void opFISUBR_WORD_INTEGER(Armv8btAsm* data) {}
void opFIDIV_WORD_INTEGER(Armv8btAsm* data) {}
void opFIDIVR_WORD_INTEGER(Armv8btAsm* data) {}

void opFFREEP_STi(Armv8btAsm* data) {}
void opFNSTSW_AX(Armv8btAsm* data) {
	// AX = (fpu->sw &= ~0x3800) | (top & 7) << 11
	U8 tmpReg = data->getTmpReg();
	data->readMem32ValueOffset(tmpReg, data->getFpuOffset(), (U32)(offsetof(FPU, sw)));
	data->addValue32(tmpReg, tmpReg, ~0x3800);
	data->orRegs32(tmpReg, tmpReg, data->getFpuTopReg(), 11);
	data->movRegToReg(xEAX, tmpReg, 16, false);
	data->releaseTmpReg(tmpReg);
}
void opFUCOMI_ST0_STj_Pop(Armv8btAsm* data) {}
void opFCOMI_ST0_STj_Pop(Armv8btAsm* data) {}
void opFILD_WORD_INTEGER(Armv8btAsm* data) {}
void opFISTTP16(Armv8btAsm* data) {}
void opFIST_WORD_INTEGER(Armv8btAsm* data) {}
void opFIST_WORD_INTEGER_Pop(Armv8btAsm* data) {}
void opFBLD_PACKED_BCD(Armv8btAsm* data) {}
void opFILD_QWORD_INTEGER(Armv8btAsm* data) {}
void opFBSTP_PACKED_BCD(Armv8btAsm* data) {}
void opFISTP_QWORD_INTEGER(Armv8btAsm* data) {}

#endif