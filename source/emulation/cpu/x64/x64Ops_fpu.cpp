#include "boxedwine.h"

#if defined(BOXEDWINE_X64) && defined(BOXEDWINE_USE_SSE_FOR_FPU)

#include "x64Ops.h"
#include "x64Data.h"
#include "x64Asm.h"
#include "x64CPU.h"
#include "x64Ops_fpu.h"

#define XMM0 8
#define XMM1 9
#define TMP_XMM0 10

extern bool writesFlags[InstructionCount];

class PushPopFlags {
public:
	PushPopFlags(X64Asm* data) : data(data) {
		needFlags = data->currentOp ? (DecodedOp::getNeededFlags(data->currentBlock, data->currentOp, CF | PF | SF | ZF | AF | OF) != 0 || instructionInfo[data->currentOp->inst].flagsUsed != 0) : true;
		if (needFlags) {
			U8 flagsReg = data->getTmpReg();
			data->pushFlagsToReg(flagsReg, true, true);
			data->writeToMemFromReg(flagsReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_INSTRUCTION_FLAGS, 4, false);
			data->releaseTmpReg(flagsReg);
			data->flagsWrittenToInstructionStoredFlags = true; // so that checkMemory won't save them again if it gets called
			writesFlags[data->currentOp->inst] = true;
		}
	}

	~PushPopFlags() {
		if (needFlags) {
			U8 flagsReg = data->getTmpReg();
			data->writeToRegFromMem(flagsReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_INSTRUCTION_FLAGS, 4, false);
			data->flagsWrittenToInstructionStoredFlags = false;
			data->popFlagsFromReg(flagsReg, true, true);
			data->releaseTmpReg(flagsReg);
		}
	}

	bool needFlags;
	X64Asm* data;
};

static void loadConstantFromCPU(X64Asm* data, U8 xmm, U32 offset) {
	// movlpd
	data->write8(0x66);
	if (xmm < 8) {
		data->write8(0x41);
	} else {
		data->write8(0x45);
		xmm -= 8;
	}
	data->write8(0x0f);
	data->write8(0x12);
	data->write8(0x80 | (xmm << 3) | HOST_CPU);
	data->write32(offset);
}

static void movq_xmm_to_reg64(X64Asm* data, U8 xmm, U8 reg) {
	data->write8(0x66);
	if (xmm < 8) {
		data->write8(0x49);
	} else {
		data->write8(0x4d);
		xmm -= 8;
	}
	data->write8(0x0f);
	data->write8(0x7e);
	data->write8(0xc0 | (xmm << 3) | reg);
}

static void ucomisd(X64Asm* data, U8 xmm1, U8 xmm2) {
	data->write8(0x66);
	U8 rex = 0;
	if (xmm1 > 7) {
		rex = REX_MOD_REG;
		xmm1 -= 8;
	}
	if (xmm2 > 7) {
		rex += REX_MOD_RM;
		xmm2 -= 8;
	}
	if (rex) {
		data->write8(REX_BASE + rex);
	}
	data->write8(0x0f);
	data->write8(0x2e);
	data->write8(0xc0 | xmm2 | (xmm1 << 3));
}

static void addsd(X64Asm* data, U8 xmm1, U8 xmm2) {
	data->write8(0xf2);
	U8 rex = 0;
	if (xmm1 > 7) {
		rex = REX_MOD_REG;
		xmm1 -= 8;
	}
	if (xmm2 > 7) {
		rex += REX_MOD_RM;
		xmm2 -= 8;
	}
	if (rex) {
		data->write8(REX_BASE + rex);
	}
	data->write8(0x0f);
	data->write8(0x58);
	data->write8(0xc0 | xmm2 | (xmm1 << 3));
}

static void mulsd(X64Asm* data, U8 xmm1, U8 xmm2) {
	data->write8(0xf2);
	U8 rex = 0;
	if (xmm1 > 7) {
		rex = REX_MOD_REG;
		xmm1 -= 8;
	}
	if (xmm2 > 7) {
		rex += REX_MOD_RM;
		xmm2 -= 8;
	}
	if (rex) {
		data->write8(REX_BASE + rex);
	}
	data->write8(0x0f);
	data->write8(0x59);
	data->write8(0xc0 | xmm2 | (xmm1 << 3));
}

static void subsd(X64Asm* data, U8 xmm1, U8 xmm2) {
	data->write8(0xf2);
	U8 rex = 0;
	if (xmm1 > 7) {
		rex = REX_MOD_REG;
		xmm1 -= 8;
	}
	if (xmm2 > 7) {
		rex += REX_MOD_RM;
		xmm2 -= 8;
	}
	if (rex) {
		data->write8(REX_BASE + rex);
	}
	data->write8(0x0f);
	data->write8(0x5c);
	data->write8(0xc0 | xmm2 | (xmm1 << 3));
}

static void divsd(X64Asm* data, U8 xmm1, U8 xmm2) {
	data->write8(0xf2);
	U8 rex = 0;
	if (xmm1 > 7) {
		rex = REX_MOD_REG;
		xmm1 -= 8;
	}
	if (xmm2 > 7) {
		rex += REX_MOD_RM;
		xmm2 -= 8;
	}
	if (rex) {
		data->write8(REX_BASE + rex);
	}
	data->write8(0x0f);
	data->write8(0x5e);
	data->write8(0xc0 | xmm2 | (xmm1 << 3));
}

static void xorpd(X64Asm* data, U8 xmm1, U8 xmm2) {
	data->write8(0x66);
	U8 rex = 0;
	if (xmm1 > 7) {
		rex = REX_MOD_REG;
		xmm1 -= 8;
	}
	if (xmm2 > 7) {
		rex += REX_MOD_RM;
		xmm2 -= 8;
	}
	if (rex) {
		data->write8(REX_BASE + rex);
	}
	data->write8(0x0f);
	data->write8(0x57);
	data->write8(0xc0 | xmm2 | (xmm1 << 3));
}

static void andpd(X64Asm* data, U8 xmm1, U8 xmm2) {
	data->write8(0x66);
	U8 rex = 0;
	if (xmm1 > 7) {
		rex = REX_MOD_REG;
		xmm1 -= 8;
	}
	if (xmm2 > 7) {
		rex += REX_MOD_RM;
		xmm2 -= 8;
	}
	if (rex) {
		data->write8(REX_BASE + rex);
	}
	data->write8(0x0f);
	data->write8(0x54);
	data->write8(0xc0 | xmm2 | (xmm1 << 3));
}

static void sqrtsd(X64Asm* data, U8 xmm1, U8 xmm2) {
	data->write8(0xf2);
	U8 rex = 0;
	if (xmm1 > 7) {
		rex = REX_MOD_REG;
		xmm1 -= 8;
	}
	if (xmm2 > 7) {
		rex += REX_MOD_RM;
		xmm2 -= 8;
	}
	if (rex) {
		data->write8(REX_BASE + rex);
	}
	data->write8(0x0f);
	data->write8(0x51);
	data->write8(0xc0 | xmm2 | (xmm1 << 3));
}

static U8 calculateIndexReg(X64Asm* data, U8 topReg, U32 index) {
	U8 result = data->getTmpReg();
	data->addWithLea(result, true, topReg, true, -1, false, 0, index, 4);
	data->andReg(result, true, 7);
	return result;
}

// movsd qword ptr[HOST_CPU + topReg*8 + offsetof(CPU, fpu.regs[0].d)], xmm
static void syncXmmToCPU(X64Asm* data, U8 topReg, U8 xmm, U8 regIndex) {
	U8 indexReg = topReg;
	if (regIndex != 0) {
		indexReg = calculateIndexReg(data, topReg, regIndex);
	}

	data->write8(0xf2);
	if (xmm < 8) {
		data->write8(0x43);
	} else {
		data->write8(0x47);
		xmm -= 8;
	}
	data->write8(0x0f);
	data->write8(0x11);
	data->write8(0x84 | (xmm << 3));
	data->write8(0xc0 | HOST_CPU | (indexReg << 3));
	data->write32(offsetof(CPU, fpu.regs[0].d));

	if (regIndex != 0) {
		data->releaseTmpReg(indexReg);
	}
}

// movsd qword ptr[HOST_CPU + topReg*8 + offsetof(CPU, fpu.regs[0].d)], xmm
static void syncCPUToXmm(X64Asm* data, U8 topReg, U8 xmm, U8 regIndex) {
	U8 indexReg = topReg;
	if (regIndex != 0) {
		indexReg = calculateIndexReg(data, topReg, regIndex);
	}

	data->write8(0xf2);
	if (xmm < 8) {
		data->write8(0x43);
	} else {
		data->write8(0x47);
		xmm -= 8;
	}
	data->write8(0x0f);
	data->write8(0x10);
	data->write8(0x84 | (xmm << 3));
	data->write8(0xc0 | HOST_CPU | (indexReg << 3));
	data->write32(offsetof(CPU, fpu.regs[0].d));

	if (regIndex != 0) {
		data->releaseTmpReg(indexReg);
	}
}

static void signExtend_r16_to_r32(X64Asm* data, U8 reg) {
	data->write8(0x45);
	data->write8(0x0f);
	data->write8(0xbf);
	data->write8(0xc0 | reg | (reg << 3));
}

static void loadInt32AsDouble(X64Asm* data, U8 xmm, U32 rm) {
	U8 reg = data->getTmpReg();
	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, false, 4, memReg, false);
	
	// CVTSI2SD xmm1, dword ptr[r8]
	data->write8(0xf2);
	if (xmm > 7) {
		data->write8(0x47);
		xmm -= 8;
	} else {
		data->write8(0x43);
	}
	data->write8(0x0f);
	data->write8(0x2a);
	data->write8(0x04 | (xmm << 3));
	data->write8((memReg << 3) | reg);

	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}

static void loadInt16AsDouble(X64Asm* data, U8 xmm, U8 rm) {
	U8 reg = data->getTmpReg();
	data->calculateMemory(reg, true, rm);
	data->writeToRegFromMem(reg, true, reg, true, -1, false, 0, 0, 2, true);
	signExtend_r16_to_r32(data, reg);

	// CVTSI2SD xmm1, r8d
	data->write8(0xf2);
	if (xmm > 7) {
		data->write8(0x45);
		xmm -= 8;
	} else {
		data->write8(0x41);
	}
	data->write8(0x0f);
	data->write8(0x2a);
	data->write8(0xc0 | (xmm << 3) | reg);

	data->releaseTmpReg(reg);
}

static void loadInt64AsDouble(X64Asm* data, U8 xmm, U32 rm) {
	U8 reg = data->getTmpReg();
	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, false, 8, memReg, false);

	// CVTSI2SD xmm1, dword ptr[r8]
	data->write8(0xf2);
	if (xmm > 7) {
		data->write8(0x4f);
		xmm -= 8;
	} else {
		data->write8(0x4b);
	}
	data->write8(0x0f);
	data->write8(0x2a);
	data->write8(0x04 | (xmm << 3));
	data->write8((memReg << 3) | reg);

	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}

static void doubleToFloat(X64Asm* data, U8 src, U8 dst) {
	// CVTSD2SS dst, src
	U8 rex = 0;
	if (dst > 7) {
		rex = REX_MOD_REG;
		dst -= 8;
	}
	if (src > 7) {
		rex += REX_MOD_RM;
		src -= 8;
	}	

	data->write8(0xf2);
	if (rex) {
		data->write8(REX_BASE + rex);
	}
	data->write8(0x0f);
	data->write8(0x5a);
	data->write8(0xc0 | (dst << 3) | src);
}

static void floatToDouble(X64Asm* data, U8 src, U8 dst) {
	// CVTSS2SD dst, src
	U8 rex = 0;
	if (dst > 7) {
		rex = REX_MOD_REG;
		dst -= 8;
	}
	if (src > 7) {
		rex += REX_MOD_RM;
		src -= 8;
	}	

	data->write8(0xf3);
	if (rex) {
		data->write8(REX_BASE + rex);
	}
	data->write8(0x0f);
	data->write8(0x5a);
	data->write8(0xc0 | (dst << 3) | src);
}

static void storeFloat(X64Asm* data, U8 xmm, U8 addressReg, U8 hostMem) {
	// movss dword ptr[addressReg], xmm
	data->write8(0xf3);
	if (xmm < 8) {
		data->write8(0x43);
	} else {
		data->write8(0x47);
		xmm -= 8;
	}
	data->write8(0x0f);
	data->write8(0x11);
	data->write8(0x04 | (xmm << 3));
	data->write8((hostMem << 3) | addressReg);
}

static void storeInt32WithTruncation(X64Asm* data, U8 xmm, U8 addressReg, U8 memReg) {
	// cvttsd2si reg, xmm
	U8 tmpReg = data->getTmpReg();

	data->write8(0xf2);
	if (xmm < 8) {
		data->write8(0x44);
	} else {
		data->write8(0x45);
		xmm -= 8;
	}
	data->write8(0x0f);
	data->write8(0x2c);
	data->write8(0xc0 | (tmpReg << 3) | xmm);
	data->writeToMemFromReg(tmpReg, true, addressReg, true, memReg, true, 0, 0, 4, false);
	data->releaseTmpReg(tmpReg);
}

static void storeInt64WithTruncation(X64Asm* data, U8 xmm, U8 addressReg, U8 memReg) {
	// cvttsd2si reg, xmm
	U8 tmpReg = data->getTmpReg();

	data->write8(0xf2);
	if (xmm < 8) {
		data->write8(0x4c);
	} else {
		data->write8(0x4d);
		xmm -= 8;
	}
	data->write8(0x0f);
	data->write8(0x2c);
	data->write8(0xc0 | (tmpReg << 3) | xmm);
	data->writeToMemFromReg(tmpReg, true, addressReg, true, memReg, true, 0, 0, 8, false);
	data->releaseTmpReg(tmpReg);
}

static void storeInt16WithTruncation(X64Asm* data, U8 xmm, U8 addressReg, U8 memReg) {
	// cvttsd2si reg, xmm
	U8 tmpReg = data->getTmpReg();

	data->write8(0xf2);
	if (xmm < 8) {
		data->write8(0x44);
	} else {
		data->write8(0x45);
		xmm -= 8;
	}
	data->write8(0x0f);
	data->write8(0x2c);
	data->write8(0xc0 | (tmpReg << 3) | xmm);
	data->writeToMemFromReg(tmpReg, true, addressReg, true, memReg, true, 0, 0, 2, false);
	data->releaseTmpReg(tmpReg);
}

static void storeInt32(X64Asm* data, U8 xmm, U8 addressReg) {
	// :TODO: set rounding mode
	
	// cvtsd2si reg, xmm
	U8 tmpReg = data->getTmpReg();

	data->write8(0xf2);
	if (xmm < 8) {
		data->write8(0x44);
	} else {
		data->write8(0x45);
		xmm -= 8;
	}
	data->write8(0x0f);
	data->write8(0x2d);
	data->write8(0xc0 | (tmpReg << 3) | xmm);
	data->writeToMemFromReg(tmpReg, true, addressReg, true, -1, false, 0, 0, 4, false);
	data->releaseTmpReg(tmpReg);
}

static void storeInt64(X64Asm* data, U8 xmm, U8 addressReg) {
	// :TODO: set rounding mode

	// cvtsd2si reg, xmm
	U8 tmpReg = data->getTmpReg();

	data->write8(0xf2);
	if (xmm < 8) {
		data->write8(0x4c);
	} else {
		data->write8(0x4d);
		xmm -= 8;
	}
	data->write8(0x0f);
	data->write8(0x2d);
	data->write8(0xc0 | (tmpReg << 3) | xmm);
	data->writeToMemFromReg(tmpReg, true, addressReg, true, -1, false, 0, 0, 8, false);
	data->releaseTmpReg(tmpReg);
}

static void storeInt16(X64Asm* data, U8 xmm, U8 addressReg) {
	// :TODO: set rounding mode

	// cvtsd2si reg, xmm
	U8 tmpReg = data->getTmpReg();

	data->write8(0xf2);
	if (xmm < 8) {
		data->write8(0x44);
	} else {
		data->write8(0x45);
		xmm -= 8;
	}
	data->write8(0x0f);
	data->write8(0x2d);
	data->write8(0xc0 | (tmpReg << 3) | xmm);
	data->writeToMemFromReg(tmpReg, true, addressReg, true, -1, false, 0, 0, 2, false);
	data->releaseTmpReg(tmpReg);
}

static U8 getTopReg(X64Asm* data) {
	U8 topReg = data->getTmpReg();
	data->writeToRegFromMem(topReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FPU_TOP, 4, false);
	return topReg;
}

static void readFloat(X64Asm* data, U8 rm, U8 xmm) {
	U8 reg = data->getTmpReg();
	
	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, false, 4, memReg, false);

	// movss xmm1, dword ptr[reg + memReg]
	data->write8(0xf3);
	U32 x = xmm;
	if (x > 7) {
		data->write8(0x47);
		x -= 8;
	} else {
		data->write8(0x43);
	}
	data->write8(0x0f);
	data->write8(0x10);
	data->write8(0x04 | (x << 3));
	data->write8((memReg << 3) | reg);

	floatToDouble(data, xmm, xmm);

	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}

static void readDouble(X64Asm* data, U8 rm, U8 xmm) {
	U8 reg = data->getTmpReg();

	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, false, 8, memReg, false);

	// movsd xmm1, qword ptr[reg + memReg]
	data->write8(0xf2);
	if (xmm > 7) {
		data->write8(0x47);
		xmm -= 8;
	} else {
		data->write8(0x42);
	}
	data->write8(0x0f);
	data->write8(0x10);
	data->write8(0x04 | (xmm << 3));
	data->write8((memReg << 3) | reg);

	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}

static void writeDouble(X64Asm* data, U8 rm, U8 xmm) {
	U8 reg = data->getTmpReg();

	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, true, 8, memReg, false);

	// movsd qword ptr[reg+memReg], xmm1
	data->write8(0xf2);
	if (xmm > 7) {
		data->write8(0x47);
		xmm -= 8;
	} else {
		data->write8(0x43);
	}
	data->write8(0x0f);
	data->write8(0x11);
	data->write8(0x04 | (xmm << 3));
	data->write8((memReg << 3) | reg);

	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}

static void hostReadTag(X64Asm* data, U8 topReg, U8 regIndex, U8 resultReg) {
	if (regIndex == 0) {
		data->writeToRegFromMem(resultReg, true, HOST_CPU, true, topReg, true, 2, CPU_OFFSET_FPU_TAG, 4, false);
	} else {
		U8 indexReg = calculateIndexReg(data, topReg, regIndex);
		data->writeToRegFromMem(resultReg, true, HOST_CPU, true, indexReg, true, 2, CPU_OFFSET_FPU_TAG, 4, false);
		data->releaseTmpReg(indexReg);
	}
}
static void hostWriteTag(X64Asm* data, U8 topReg, U8 regIndex, U8 valueReg) {
	if (regIndex == 0) {
		data->writeToMemFromReg(valueReg, true, HOST_CPU, true, topReg, true, 2, CPU_OFFSET_FPU_TAG, 4, false);
	} else {
		U8 indexReg = calculateIndexReg(data, topReg, regIndex);
		data->writeToMemFromReg(valueReg, true, HOST_CPU, true, indexReg, true, 2, CPU_OFFSET_FPU_TAG, 4, false);
		data->releaseTmpReg(indexReg);
	}
}

class FPUReg {
public:
	FPUReg(X64Asm* data, U8 topReg, U32 regIndex) {
		this->reg = regIndex ? XMM1 : XMM0;
		syncCPUToXmm(data, topReg, this->reg, regIndex);
	}
	U8 reg;
};

static void PREP_PUSH(X64Asm* data, U8 topReg, bool writeTag) {
	data->addWithLea(topReg, true, topReg, true, -1, false, 0, 0xffffffff, 4);
	data->andReg(topReg, true, 7);
	data->writeToMemFromReg(topReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FPU_TOP, 4, false);
	if (writeTag) {
		data->writeToMemFromValue(TAG_Valid, HOST_CPU, true, topReg, true, 2, CPU_OFFSET_FPU_TAG, 4, false);
	}
}

static void FPU_POP(X64Asm* data, U8 topReg, U8 amount = 1) {
	// this->tags[this->top] = TAG_Empty;
	// this->top = ((this->top + 1) & 7);
	data->writeToMemFromValue(TAG_Empty, HOST_CPU, true, topReg, true, 2, CPU_OFFSET_FPU_TAG, 4, false);
	data->addWithLea(topReg, true, topReg, true, -1, false, 0, amount, 4);
	data->andReg(topReg, true, 7);
	data->writeToMemFromReg(topReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FPU_TOP, 4, false);
}

static void fcompare(X64Asm* data, U8 topReg, U8 v1, S32 tagIndex1, U8 v2, S32 tagIndex2, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) {
	U8 tagReg = data->getTmpReg();
	U8 tagReg2 = data->getTmpReg();

	if (tagIndex1 == 0) {
		data->writeToRegFromMem(tagReg, true, HOST_CPU, true, topReg, true, 2, CPU_OFFSET_FPU_TAG, 4, false);
	} else {
		U8 indexReg = calculateIndexReg(data, topReg, tagIndex1);
		data->writeToRegFromMem(tagReg, true, HOST_CPU, true, indexReg, true, 2, CPU_OFFSET_FPU_TAG, 4, false);
		data->releaseTmpReg(indexReg);
	}
	if (tagIndex2 == 0) {
		data->writeToRegFromMem(tagReg2, true, HOST_CPU, true, topReg, true, 2, CPU_OFFSET_FPU_TAG, 4, false);
	} else if (tagIndex2 != -1) {
		U8 indexReg = calculateIndexReg(data, topReg, tagIndex2);
		data->writeToRegFromMem(tagReg2, true, HOST_CPU, true, indexReg, true, 2, CPU_OFFSET_FPU_TAG, 4, false);
		data->releaseTmpReg(indexReg);
	}

	if (tagIndex2 != -1) {
		data->orRegReg(tagReg, true, tagReg2, true);
	}
	data->releaseTmpReg(tagReg2);

	data->andReg(tagReg, true, TAG_Empty);
	data->doIf(tagReg, true, 0, [data, pfnInvalid, pfnEqual, pfnGreaterThan, pfnLessThan, v1, v2] {
		// if both tags are TAG_Valid
		ucomisd(data, v1, v2);
		data->doIf(0, 0, 0, [data, pfnInvalid, pfnEqual] {			
			pfnInvalid();
		}, [data, pfnEqual, pfnGreaterThan, pfnLessThan, v1, v2] {
			data->doIf(0, 0, 0, [data, pfnEqual, v1, v2] {
				pfnEqual();
			}, [data, pfnGreaterThan, pfnLessThan, v1, v2] {
				data->doIf(0, 0, 0, [data, pfnLessThan] {
					pfnLessThan();
				}, [data, pfnGreaterThan] {
					// is not equal, is not less than and is not greater than, this means one of the is nan
					pfnGreaterThan();
				}, false, false, false, 0x73, 0x72); // jnb, jb
			}, false, false, false); // jnz, jz
		}, false, false, false, 0x7b, 0x7a); // jnp, jp
	}, [data, pfnInvalid] {
		// if either tag is not TAG_Valid
		pfnInvalid();
	}, false, true, true); // true = tagReg released
}

// #define FPU_SET_C0(fpu, C) (fpu)->sw &= ~0x0100; if (C != 0) (fpu)->sw |= 0x0100    
// #define FPU_SET_C1(fpu, C) (fpu)->sw &= ~0x0200; if (C != 0) (fpu)->sw |= 0x0200    
// #define FPU_SET_C2(fpu, C) (fpu)->sw &= ~0x0400; if (C != 0) (fpu)->sw |= 0x0400    
// #define FPU_SET_C3(fpu, C) (fpu)->sw &= ~0x4000; if (C != 0) (fpu)->sw |= 0x4000

static void doFCOM(X64Asm* data, U8 topReg, U8 v1, S32 tagIndex1, U8 v2, S32 tagIndex2) {
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
	data->writeToRegFromMem(swReg, true, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, fpu.sw)), 4, false);

	fcompare(data, topReg, v1, tagIndex1, v2, tagIndex2, [data, swReg] {
		// equal
		data->andReg(swReg, true, ~0x0700);
		data->orReg(swReg, true, 0x4000);
	}, [data, swReg] {
		// less than
		data->andReg(swReg, true, ~0x4600);
		data->orReg(swReg, true, 0x0100);
	}, [data, swReg] {
		// greater than
		data->andReg(swReg, true, ~0x4700);
	}, [data, swReg] {
		// invalid
		data->andReg(swReg, true, ~0x0200);
		data->orReg(swReg, true, 0x4500);
	});

	data->writeToMemFromReg(swReg, true, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, fpu.sw)), 4, false);
	data->releaseTmpReg(swReg);
}

static void doFCOMI(X64Asm* data, U8 topReg, U8 v1, S32 tagIndex1, U8 v2, S32 tagIndex2) {
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

	U8 flagsReg = data->getTmpReg();
	data->zeroReg(flagsReg, true, false);

	// shift 8 because popFlagsFromReg expects flags in AH
	fcompare(data, topReg, v1, tagIndex1, v2, tagIndex2, [data, flagsReg] {
		// equal
		data->orReg(flagsReg, true, ZF << 8);
	}, [data, flagsReg] {
		// less than
		data->orReg(flagsReg, true, CF << 8);
	}, [data, flagsReg] {
		// greater than
		// nothing
	}, [data, flagsReg] {
		// invalid
		data->orReg(flagsReg, true, (CF | PF | ZF) << 8);
	});

	data->popFlagsFromReg(flagsReg, true, true);
	data->releaseTmpReg(flagsReg);
}

void opFADD_ST0_STj(X64Asm* data, U8 reg) {
	// cpu->fpu.STV(0) += cpu->fpu.STV(reg)
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, reg);
	addsd(data, dst.reg, src.reg);
	syncXmmToCPU(data, topReg, dst.reg, 0);
	data->releaseTmpReg(topReg);
}

void opFMUL_ST0_STj(X64Asm* data, U8 reg) {
	// cpu->fpu.STV(0) *= cpu->fpu.STV(reg)
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, reg);
	mulsd(data, dst.reg, src.reg);
	syncXmmToCPU(data, topReg, dst.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFCOM_STi(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, reg);
	doFCOM(data, topReg, dst.reg, 0, src.reg, reg);
	data->releaseTmpReg(topReg);
}
void opFCOM_STi_Pop(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, reg);
	doFCOM(data, topReg, dst.reg, 0, src.reg, reg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFSUB_ST0_STj(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, reg);
	subsd(data, dst.reg, src.reg);
	syncXmmToCPU(data, topReg, dst.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFSUBR_ST0_STj(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	syncCPUToXmm(data, topReg, TMP_XMM0, reg);
	subsd(data, TMP_XMM0, dst.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFDIV_ST0_STj(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, reg);
	divsd(data, dst.reg, src.reg);
	syncXmmToCPU(data, topReg, dst.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFDIVR_ST0_STj(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	syncCPUToXmm(data, topReg, TMP_XMM0, reg);
	divsd(data, TMP_XMM0, dst.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFADD_SINGLE_REAL(X64Asm* data, U8 rm) {
	// cpu->fpu.FLD_F32_EA(cpu, address);
	// cpu->fpu.FADD_EA();
	
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);	
	readFloat(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	addsd(data, dst.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, dst.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFMUL_SINGLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readFloat(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	mulsd(data, dst.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, dst.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFCOM_SINGLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readFloat(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	doFCOM(data, topReg, dst.reg, 0, TMP_XMM0, -1);
	data->releaseTmpReg(topReg);
}
void opFCOM_SINGLE_REAL_Pop(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readFloat(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	doFCOM(data, topReg, dst.reg, 0, TMP_XMM0, -1);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFSUB_SINGLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readFloat(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	subsd(data, dst.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, dst.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFSUBR_SINGLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readFloat(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	subsd(data, TMP_XMM0, dst.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFDIV_SINGLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readFloat(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	divsd(data, dst.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, dst.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFDIVR_SINGLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readFloat(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	divsd(data, TMP_XMM0, dst.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFLD_STi(X64Asm* data, U8 reg) {
	// int reg_from = cpu->fpu.STV(reg);
	// cpu->fpu.PREP_PUSH();
	// cpu->fpu.FST(reg_from, cpu->fpu.STV(0));

	PushPopFlags flags(data); // flags get clobbered in when managing index from top
	U8 topReg = getTopReg(data);
	FPUReg fromTmp(data, topReg, reg);
	U8 tagReg = data->getTmpReg();
	hostReadTag(data, topReg, reg, tagReg); // read before push changes indexes

	PREP_PUSH(data, topReg, false); // will change topReg

	syncXmmToCPU(data, topReg, fromTmp.reg, 0);
	hostWriteTag(data, topReg, 0, tagReg);
	data->releaseTmpReg(tagReg);
	data->releaseTmpReg(topReg);
}
void opFXCH_STi(X64Asm* data, U8 reg) {
	// int tag = this->tags[other];
	// struct FPU_Reg reg = this->regs[other];
	// this->tags[other] = this->tags[st];
	// this->regs[other] = this->regs[st];
	// this->tags[st] = tag;
	// this->regs[st] = reg;
	PushPopFlags flags(data); // flags get clobbered in when managing index from top
	U8 topReg = getTopReg(data);
	FPUReg from(data, topReg, reg);
	FPUReg to(data, topReg, 0);

	// exchange xmm
	syncXmmToCPU(data, topReg, from.reg, 0);
	syncXmmToCPU(data, topReg, to.reg, reg);

	// exchange tags
	U8 tmpReg = data->getTmpReg();
	U8 tmpReg2 = data->getTmpReg();
	hostReadTag(data, topReg, 0, tmpReg);
	hostReadTag(data, topReg, reg, tmpReg2);
	hostWriteTag(data, topReg, 0, tmpReg2);
	hostWriteTag(data, topReg, reg, tmpReg);
	
	data->releaseTmpReg(tmpReg);
	data->releaseTmpReg(tmpReg2);
	data->releaseTmpReg(topReg);
}
void opFNOP(X64Asm* data) {
}

void opFST_STi_Pop(X64Asm* data, U8 reg) {
	// cpu->fpu.FST(cpu->fpu.STV(0), cpu->fpu.STV(reg));
	// cpu->fpu.FPOP();
	
	PushPopFlags flags(data); // flags get clobbered in when managing index from top
	U8 topReg = getTopReg(data);
	FPUReg src(data, topReg, 0);
	//FPUReg dst(data, topReg, reg, true, false);

	U8 tagReg = data->getTmpReg();
	hostReadTag(data, topReg, 0, tagReg);
	hostWriteTag(data, topReg, reg, tagReg);
	data->releaseTmpReg(tagReg);	

	syncXmmToCPU(data, topReg, src.reg, reg);

	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFCHS(X64Asm* data) {
	// this->regs[this->top].d = -1.0 * (this->regs[this->top].d);

	// flags not set, so don't need to save
	U8 topReg = getTopReg(data);
	FPUReg reg(data, topReg, 0);
	loadConstantFromCPU(data, TMP_XMM0, offsetof(CPU, fNeg));
	xorpd(data, reg.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, reg.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFABS(X64Asm* data) {
	// this->regs[this->top].d = fabs(this->regs[this->top].d);

	// flags not set, so don't need to save
	U8 topReg = getTopReg(data);
	FPUReg reg(data, topReg, 0);
	loadConstantFromCPU(data, TMP_XMM0, offsetof(CPU, fAbs));
	andpd(data, reg.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, reg.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFTST(X64Asm* data) {
	// this->regs[8].d = 0.0;
	// FCOM(this->top, 8);

	// flags not set, so don't need to save
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	loadConstantFromCPU(data, TMP_XMM0, offsetof(CPU, fZero));
	doFCOM(data, topReg, dst.reg, 0, TMP_XMM0, -1);
	data->releaseTmpReg(topReg);
}

void opFXAM(X64Asm* data) {
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

	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	U8 swReg = data->getTmpReg();
	FPUReg reg(data, topReg, 0);

	data->writeToRegFromMem(swReg, true, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, fpu.sw)), 4, false);
	data->andReg(swReg, true, ~0x4700);

	U8 bitsReg = data->getTmpReg();
	movq_xmm_to_reg64(data, reg.reg, bitsReg);
	
	data->cmpReg(bitsReg, true, 0, true);
	data->doIf(0, false, 0, [data, swReg] {
		data->orReg(swReg, true, 0x200);
		}, nullptr, false, false, false, 0x7d, 0x7c); // less than 0

	U8 tagReg = data->getTmpReg();
	hostReadTag(data, topReg, 0, tagReg);

	data->doIf(tagReg, true, TAG_Empty, [data, swReg] {
		data->orReg(swReg, true, 0x4100);
		}, [data, swReg, bitsReg] {
			// if tag is TAG_Valid or TAG_Zero		

			U8 tmpReg2 = data->getTmpReg();
			data->writeToRegFromReg(tmpReg2, true, bitsReg, true, 8);
			data->shiftRightReg(tmpReg2, true, 52, false, true);
			data->andReg(tmpReg2, true, 0x7ff);
			data->doIf(tmpReg2, true, 0x7ff, [data, swReg, bitsReg] {
				// isnan or isinf
				data->testReg(bitsReg, true, 0xfffffffffffff, true);
				data->doIf(0, false, 0, [data, swReg] {
					// inf
					data->orReg(swReg, true, 0x500);
				}, [data, swReg] {
					// nan
					data->orReg(swReg, true, 0x100);
				}, false, false);				
			}, [data, swReg, bitsReg] {
				data->testReg(bitsReg, true, 0x7fffffffffffffff, true);
				data->doIf(0, false, 0, [data, swReg] {
					// normal number
					data->orReg(swReg, true, 0x4000);
				}, [data, swReg] {
					// zero					
					data->orReg(swReg, true, 0x400);
				}, false, false);
			});
			data->releaseTmpReg(tmpReg2);
		}, false, true, true);
		data->releaseTmpReg(bitsReg);
		data->writeToMemFromReg(swReg, true, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, fpu.sw)), 4, false);
		data->releaseTmpReg(swReg);
}

void opFLD1(X64Asm* data) {
	PushPopFlags flags(data); // flags get clobbered in when managing index from top in PREP_PUSH
	U8 topReg = getTopReg(data);
	PREP_PUSH(data, topReg, true);	
	loadConstantFromCPU(data, TMP_XMM0, offsetof(CPU, fOne));
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFLDL2T(X64Asm* data) {
	PushPopFlags flags(data); // flags get clobbered in when managing index from top in PREP_PUSH
	U8 topReg = getTopReg(data);
	PREP_PUSH(data, topReg, true);
	loadConstantFromCPU(data, TMP_XMM0, offsetof(CPU, fL2T));
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFLDL2E(X64Asm* data) {
	PushPopFlags flags(data); // flags get clobbered in when managing index from top in PREP_PUSH
	U8 topReg = getTopReg(data);
	PREP_PUSH(data, topReg, true);
	loadConstantFromCPU(data, TMP_XMM0, offsetof(CPU, fL2E));
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFLDPI(X64Asm* data) {
	PushPopFlags flags(data); // flags get clobbered in when managing index from top in PREP_PUSH
	U8 topReg = getTopReg(data);
	PREP_PUSH(data, topReg, true);
	loadConstantFromCPU(data, TMP_XMM0, offsetof(CPU, fPi));
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFLDLG2(X64Asm* data) {
	PushPopFlags flags(data); // flags get clobbered in when managing index from top in PREP_PUSH
	U8 topReg = getTopReg(data);
	PREP_PUSH(data, topReg, true);
	loadConstantFromCPU(data, TMP_XMM0, offsetof(CPU, fLG2));
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFLDLN2(X64Asm* data) {
	PushPopFlags flags(data); // flags get clobbered in when managing index from top in PREP_PUSH
	U8 topReg = getTopReg(data);
	PREP_PUSH(data, topReg, true);
	loadConstantFromCPU(data, TMP_XMM0, offsetof(CPU, fLN2));
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFLDZ(X64Asm* data) {
	PushPopFlags flags(data); // flags get clobbered in when managing index from top in PREP_PUSH
	U8 topReg = getTopReg(data);
	PREP_PUSH(data, topReg, true);// firefight draws incorrectly if this uses TAG_Zero, probably a bug somewhere else in the fpu, like comp when looking at tags
	loadConstantFromCPU(data, TMP_XMM0, offsetof(CPU, fZero));
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFDECSTP(X64Asm* data) {
	// this->top = (this->top - 1) & 7;

	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	data->addWithLea(topReg, true, topReg, true, -1, false, 0, -1, 4);
	data->andReg(topReg, true, 7);
	data->writeToMemFromReg(topReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FPU_TOP, 4, false);
	data->releaseTmpReg(topReg);
}
void opFINCSTP(X64Asm* data) {
	// this->top = (this->top + 1) & 7;

	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	data->addWithLea(topReg, true, topReg, true, -1, false, 0, 1, 4);
	data->andReg(topReg, true, 7);
	data->writeToMemFromReg(topReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FPU_TOP, 4, false);
	data->releaseTmpReg(topReg);
}
// age of empires uses this for path finding
void opFSQRT(X64Asm* data) {
	// doesn't change flags
	U8 topReg = getTopReg(data);
	FPUReg reg(data, topReg, 0);
	sqrtsd(data, reg.reg, reg.reg);
	syncXmmToCPU(data, topReg, reg.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFLD_SINGLE_REAL(X64Asm* data, U8 rm) {
	// U32 value = readd(address); // might generate PF, so do before we adjust the stack
	// cpu->fpu.PREP_PUSH();
	// cpu->fpu.FLD_F32(value, cpu->fpu.STV(0));

	PushPopFlags flags(data); // checkMemory changes flags
	U8 topReg = getTopReg(data);	

	// first half of readFloat, not using actual readFloat since that would require a temp xmm
	U8 reg = data->getTmpReg();
	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, false, 4, memReg, false);

	PREP_PUSH(data, topReg, true);

	// movss xmm8, dword ptr[r8]
	U8 xmm = TMP_XMM0;
	data->write8(0xf3);
	if (xmm > 7) {
		data->write8(0x47);
		xmm -= 8;
	} else {
		data->write8(0x43);
	}
	data->write8(0x0f);
	data->write8(0x10);
	data->write8(0x04 | (xmm << 3));
	data->write8((memReg << 3) | reg);

	floatToDouble(data, TMP_XMM0, TMP_XMM0);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);

	data->releaseTmpReg(reg);
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(memReg);
}
void opFST_SINGLE_REAL(X64Asm* data, U8 rm) {	
	PushPopFlags flags(data); // checkMemory changes flags
	U8 reg = data->getTmpReg();
	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, true, 4, memReg, false);

	U8 topReg = getTopReg(data);
	FPUReg src(data, topReg, 0);
	doubleToFloat(data, src.reg, TMP_XMM0);
	storeFloat(data, TMP_XMM0, reg, memReg);
	data->releaseTmpReg(reg);
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(memReg);
}
void opFST_SINGLE_REAL_Pop(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // checkMemory changes flags
	U8 reg = data->getTmpReg();
	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, true, 4, memReg, false);

	U8 topReg = getTopReg(data);
	FPUReg src(data, topReg, 0);
	doubleToFloat(data, src.reg, TMP_XMM0);
	storeFloat(data, TMP_XMM0, reg, memReg);
	data->releaseTmpReg(reg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(memReg);
}
void opFNSTCW(X64Asm* data, U8 rm) {
	// writew(address, cpu->fpu.CW());

	PushPopFlags flags(data); // writeToEFromReg/checkMemory changes flags
	U8 tmpReg = data->getTmpReg();
	data->writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, fpu.cw)), 4, false);
	data->writeToEFromReg(rm, tmpReg, true, 2);
	data->releaseTmpReg(tmpReg);
}
static void doCMov(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	U8 tagReg = data->getTmpReg();
	FPUReg from(data, topReg, reg);	

	hostReadTag(data, topReg, reg, tagReg);
	syncXmmToCPU(data, topReg, from.reg, 0);
	hostWriteTag(data, topReg, 0, tagReg);

	data->releaseTmpReg(tagReg);
	data->releaseTmpReg(topReg);
}

void opFCMOV_ST0_STj_CF(X64Asm* data, U8 reg) {
	data->doIf(0, false, 0, [data, reg] {
		doCMov(data, reg);
	}, nullptr, false, false, false, 0x73, 0x72);
}
void opFCMOV_ST0_STj_ZF(X64Asm* data, U8 reg) {
	data->doIf(0, false, 0, [data, reg] {
		doCMov(data, reg);
	}, nullptr, false, false, false);
}
void opFCMOV_ST0_STj_CF_OR_ZF(X64Asm* data, U8 reg) {
	data->doIf(0, false, 0, [data, reg] {
		doCMov(data, reg);
	}, nullptr, false, false, false, 0x77, 0x76);
}
void opFCMOV_ST0_STj_PF(X64Asm* data, U8 reg) {
	data->doIf(0, false, 0, [data, reg] {
		doCMov(data, reg);
	}, nullptr, false, false, false, 0x7b, 0x7a);
}
void opFCMOV_ST0_STj_NCF(X64Asm* data, U8 reg) {
	data->doIf(0, false, 0, [data, reg] {
		doCMov(data, reg);
}, nullptr, false, false, false, 0x72, 0x73);
}
void opFCMOV_ST0_STj_NZF(X64Asm* data, U8 reg) {
	data->doIf(0, false, 0, [data, reg] {
		doCMov(data, reg);
}, nullptr, false, false, false, 0x74, 0x75);
}
void opFCMOV_ST0_STj_NCF_AND_NZF(X64Asm* data, U8 reg) {
	data->doIf(0, false, 0, [data, reg] {
		doCMov(data, reg);
}, nullptr, false, false, false, 0x76, 0x77);
}
void opFCMOV_ST0_STj_NPF(X64Asm* data, U8 reg) {
	data->doIf(0, false, 0, [data, reg] {
		doCMov(data, reg);
}, nullptr, false, false, false, 0x7a, 0x7b);
}
void opFUCOMPP(X64Asm* data) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg reg1(data, topReg, 0);
	FPUReg reg2(data, topReg, 1);
	// :TODO: FCOM and FUCOM currently do the same thing
	doFCOM(data, topReg, reg1.reg, 0, reg2.reg, 1);
	FPU_POP(data, topReg, 2);
	data->releaseTmpReg(topReg);
}
void opFIADD_DWORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt32AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt32AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	addsd(data, reg.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, reg.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFIMUL_DWORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt32AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt32AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	mulsd(data, reg.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, reg.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFICOM_DWORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt32AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt32AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	doFCOM(data, topReg, reg.reg, 0, TMP_XMM0, -1);
	data->releaseTmpReg(topReg);
}
void opFICOM_DWORD_INTEGER_Pop(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt32AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt32AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	doFCOM(data, topReg, reg.reg, 0, TMP_XMM0, -1);	
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFISUB_DWORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt32AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt32AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	subsd(data, reg.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, reg.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFISUBR_DWORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt32AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt32AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	subsd(data, TMP_XMM0, reg.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFIDIV_DWORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt32AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt32AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	divsd(data, reg.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, reg.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFIDIVR_DWORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt32AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt32AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	divsd(data, TMP_XMM0, reg.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFNCLEX(X64Asm* data) {
	// this->sw &= 0x7f00;
	PushPopFlags flags(data);
	U8 swReg = data->getTmpReg();
	data->writeToRegFromMem(swReg, true, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, fpu.sw)), 4, false);
	data->andReg(swReg, true, 0x7f00);
	data->writeToMemFromReg(swReg, true, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, fpu.sw)), 4, false);
	data->releaseTmpReg(swReg);
}
void opFNINIT(X64Asm* data) {
	/*
	SetCW(0x37F);
	this->sw = 0;
	this->top = FPU_GET_TOP(this);
	this->tags[0] = TAG_Empty;
	this->tags[1] = TAG_Empty;
	this->tags[2] = TAG_Empty;
	this->tags[3] = TAG_Empty;
	this->tags[4] = TAG_Empty;
	this->tags[5] = TAG_Empty;
	this->tags[6] = TAG_Empty;
	this->tags[7] = TAG_Empty;
	*/

	data->writeToMemFromValue(0x37f, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, fpu.cw)), 4, false);
	data->writeToMemFromValue(0, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, fpu.sw)), 4, false);
	data->writeToMemFromValue(0, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, fpu.top)), 4, false);
	for (int i = 0; i < 8; i++) {
		data->writeToMemFromValue(TAG_Empty, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FPU_TAG + i*sizeof(U32), 4, false);
	}
	data->writeToMemFromValue(0, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FPU_IS_MMX, 4, false);
}
void opFUCOMI_ST0_STj(X64Asm* data, U8 reg) {
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, reg);
	// :TODO: FCOM and FUCOM currently do the same thing
	// will clobber flags
	doFCOMI(data, topReg, dst.reg, 0, src.reg, reg);
	data->releaseTmpReg(topReg);
}
void opFCOMI_ST0_STj(X64Asm* data, U8 reg) {
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, reg);
	// will clobber flags
	doFCOMI(data, topReg, dst.reg, 0, src.reg, reg);
	data->releaseTmpReg(topReg);
}
void opFILD_DWORD_INTEGER(X64Asm* data, U8 rm) {
	// U32 value = readd(address); // might generate PF, so do before we adjust the stack
	// cpu->fpu.PREP_PUSH();
	// cpu->fpu.FLD_I32(value, cpu->fpu.STV(0));

	PushPopFlags flags(data); // loadInt32AsDouble/checkMemory changes flags
	U8 topReg = getTopReg(data);
	loadInt32AsDouble(data, TMP_XMM0, rm);
	PREP_PUSH(data, topReg, true);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}

void opFISTTP32(X64Asm* data, U8 rm) {
	// cpu->fpu.FSTT_I32(cpu, address);
	// cpu->fpu.FPOP();

	PushPopFlags flags(data); // checkMemory changes flags
	U8 topReg = getTopReg(data);
	U8 reg = data->getTmpReg();

	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, false, 4, memReg, false);

	FPUReg src(data, topReg, 0);
	storeInt32WithTruncation(data, src.reg, reg, memReg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}
void opFIST_DWORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // checkMemory changes flags
	U8 topReg = getTopReg(data);
	U8 reg = data->getTmpReg();

	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, false, 4, memReg, false);

	FPUReg src(data, topReg, 0);
	storeInt32WithTruncation(data, src.reg, reg, memReg); // until rounding is implemented, just truncate
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}
void opFIST_DWORD_INTEGER_Pop(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // checkMemory changes flags
	U8 topReg = getTopReg(data);
	U8 reg = data->getTmpReg();

	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, false, 4, memReg, false);

	FPUReg src(data, topReg, 0);
	storeInt32WithTruncation(data, src.reg, reg, memReg); // until rounding is implemented, just truncate
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}
void opFADD_STi_ST0(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, reg);
	FPUReg src(data, topReg, 0);
	addsd(data, dst.reg, src.reg);
	syncXmmToCPU(data, topReg, dst.reg, reg);
	data->releaseTmpReg(topReg);
}
void opFMUL_STi_ST0(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, reg);
	FPUReg src(data, topReg, 0);
	mulsd(data, dst.reg, src.reg);
	syncXmmToCPU(data, topReg, dst.reg, reg);
	data->releaseTmpReg(topReg);
}
void opFSUBR_STi_ST0(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, reg);
	syncCPUToXmm(data, topReg, TMP_XMM0, 0);
	subsd(data, TMP_XMM0, dst.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, reg);
	data->releaseTmpReg(topReg);
}
void opFSUB_STi_ST0(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, reg);
	FPUReg src(data, topReg, 0);
	subsd(data, dst.reg, src.reg);
	syncXmmToCPU(data, topReg, dst.reg, reg);
	data->releaseTmpReg(topReg);
}
void opFDIVR_STi_ST0(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, reg);
	syncCPUToXmm(data, topReg, TMP_XMM0, 0);
	divsd(data, TMP_XMM0, dst.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, reg);
	data->releaseTmpReg(topReg);
}
void opFDIV_STi_ST0(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, reg);
	FPUReg src(data, topReg, 0);
	divsd(data, dst.reg, src.reg);
	syncXmmToCPU(data, topReg, dst.reg, reg);
	data->releaseTmpReg(topReg);
}
void opFADD_DOUBLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readDouble(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	addsd(data, dst.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, dst.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFMUL_DOUBLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readDouble(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	mulsd(data, dst.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, dst.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFCOM_DOUBLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readDouble(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	doFCOM(data, topReg, dst.reg, 0, TMP_XMM0, -1);
	data->releaseTmpReg(topReg);
}
void opFCOM_DOUBLE_REAL_Pop(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readDouble(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	doFCOM(data, topReg, dst.reg, 0, TMP_XMM0, -1);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFSUB_DOUBLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readDouble(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	subsd(data, dst.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, dst.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFSUBR_DOUBLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readDouble(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	subsd(data, TMP_XMM0, dst.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFDIV_DOUBLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readDouble(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	divsd(data, dst.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, dst.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFDIVR_DOUBLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // flags get clobbered in checkMemory
	U8 topReg = getTopReg(data);
	readDouble(data, rm, TMP_XMM0);
	FPUReg dst(data, topReg, 0);
	divsd(data, TMP_XMM0, dst.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFFREE_STi(X64Asm* data, U8 reg) {
	// cpu->fpu.FFREE_STi(cpu->fpu.STV(reg));
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	U8 indexReg = calculateIndexReg(data, topReg, reg);
	data->writeToMemFromValue(TAG_Empty, HOST_CPU, true, topReg, true, 2, CPU_OFFSET_FPU_TAG, 4, false);
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(indexReg);
}
void opFST_STi(X64Asm* data, U8 reg) {
	// cpu->fpu.FST(cpu->fpu.STV(0), cpu->fpu.STV(reg));

	PushPopFlags flags(data); // flags get clobbered in when managing index from top
	U8 topReg = getTopReg(data);
	U8 tagReg = data->getTmpReg();
	FPUReg src(data, topReg, 0);	

	syncXmmToCPU(data, topReg, src.reg, reg);
	hostReadTag(data, topReg, 0, tagReg);
	hostWriteTag(data, topReg, reg, tagReg);

	data->releaseTmpReg(tagReg);
	data->releaseTmpReg(topReg);
}
void opFUCOM_STi(X64Asm* data, U8 reg) {
	PushPopFlags flags(data); // flags get clobbered in when managing index from top
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, reg);
	// :TODO: FCOM and FUCOM currently do the same thing
	doFCOM(data, topReg, dst.reg, 0, src.reg, reg);
	data->releaseTmpReg(topReg);
}
void opFUCOM_STi_Pop(X64Asm* data, U8 reg) {
	PushPopFlags flags(data); // flags get clobbered in when managing index from top
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, reg);
	// :TODO: FCOM and FUCOM currently do the same thing
	doFCOM(data, topReg, dst.reg, 0, src.reg, reg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFLD_DOUBLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // checkMemory changes flags
	U8 topReg = getTopReg(data);

	// first half of readDouble, not using actual readDouble since that would require a temp xmm
	U8 reg = data->getTmpReg();
	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, false, 8, memReg, false);

	PREP_PUSH(data, topReg, true);

	// movsd xmm1, qword ptr[r8]
	U8 xmm = TMP_XMM0;
	data->write8(0xf2);	
	if (xmm > 7) {
		data->write8(0x47);
		xmm -= 8;
	} else {
		data->write8(0x43);
	}
	data->write8(0x0f);
	data->write8(0x10);
	data->write8(0x04 | (xmm << 3));
	data->write8((memReg << 3) | reg);

	syncXmmToCPU(data, topReg, TMP_XMM0, 0);

	data->releaseTmpReg(reg);
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(memReg);
}
void opFISTTP64(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // checkMemory changes flags
	U8 topReg = getTopReg(data);

	U8 reg = data->getTmpReg();
	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, true, 8, memReg, false);

	FPUReg src(data, topReg, 0);
	storeInt64WithTruncation(data, src.reg, reg, memReg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}
void opFST_DOUBLE_REAL(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // writeDouble/checkMemory changes flags
	U8 topReg = getTopReg(data);
	FPUReg src(data, topReg, 0);

	writeDouble(data, rm, src.reg);
	data->releaseTmpReg(topReg);
}
void opFST_DOUBLE_REAL_Pop(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // writeDouble/checkMemory changes flags
	U8 topReg = getTopReg(data);
	FPUReg src(data, topReg, 0);

	writeDouble(data, rm, src.reg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFNSTSW(X64Asm* data, U8 rm) {
	// (fpu)->sw &= ~0x3800; (fpu)->sw |= (top & 7) << 11
	// writew(address, cpu->fpu.SW());

	PushPopFlags flags(data);
	U8 swReg = data->getTmpReg();
	U8 topReg = getTopReg(data);
	data->writeToRegFromMem(swReg, true, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, fpu.sw)), 4, false);
	data->andReg(swReg, true, ~0x3800);
	data->shiftLeftReg(topReg, true, 11);
	data->orRegReg(swReg, true, topReg, true);
	data->writeToRegFromE(swReg, true, rm, 2);
	data->releaseTmpReg(swReg);
	data->releaseTmpReg(topReg);
}
void opFADD_STi_ST0_Pop(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, reg);
	FPUReg src(data, topReg, 0);
	addsd(data, dst.reg, src.reg);
	syncXmmToCPU(data, topReg, dst.reg, reg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFMUL_STi_ST0_Pop(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, reg);
	FPUReg src(data, topReg, 0);
	mulsd(data, dst.reg, src.reg);
	syncXmmToCPU(data, topReg, dst.reg, reg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFCOMPP(X64Asm* data) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, 1);
	doFCOM(data, topReg, dst.reg, 0, src.reg, data->currentOp->reg);
	FPU_POP(data, topReg, 2);
	data->releaseTmpReg(topReg);
}
void opFSUBR_STi_ST0_Pop(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, reg);
	syncCPUToXmm(data, topReg, TMP_XMM0, 0);
	subsd(data, TMP_XMM0, dst.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, reg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFSUB_STi_ST0_Pop(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, reg);
	FPUReg src(data, topReg, 0);
	subsd(data, dst.reg, src.reg);
	syncXmmToCPU(data, topReg, dst.reg, reg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFDIVR_STi_ST0_Pop(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, reg);
	syncCPUToXmm(data, topReg, TMP_XMM0, 0);
	divsd(data, TMP_XMM0, dst.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, reg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFDIV_STi_ST0_Pop(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, reg);
	FPUReg src(data, topReg, 0);
	divsd(data, dst.reg, src.reg);
	syncXmmToCPU(data, topReg, dst.reg, reg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFIADD_WORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt16AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt16AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	addsd(data, reg.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, reg.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFIMUL_WORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt16AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt16AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	mulsd(data, reg.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, reg.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFICOM_WORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt16AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt16AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	doFCOM(data, topReg, reg.reg, 0, TMP_XMM0, -1);
	data->releaseTmpReg(topReg);
}
void opFICOM_WORD_INTEGER_Pop(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt16AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt16AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	doFCOM(data, topReg, reg.reg, 0, TMP_XMM0, -1);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFISUB_WORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt16AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt16AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	subsd(data, reg.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, reg.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFISUBR_WORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt16AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt16AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	subsd(data, TMP_XMM0, reg.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFIDIV_WORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt16AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt16AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	divsd(data, reg.reg, TMP_XMM0);
	syncXmmToCPU(data, topReg, reg.reg, 0);
	data->releaseTmpReg(topReg);
}
void opFIDIVR_WORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt16AsDouble calls checkMemory
	U8 topReg = getTopReg(data);
	loadInt16AsDouble(data, TMP_XMM0, rm);
	FPUReg reg(data, topReg, 0);
	divsd(data, TMP_XMM0, reg.reg);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFFREEP_STi(X64Asm* data, U8 reg) {
	PushPopFlags flags(data);
	U8 topReg = getTopReg(data);
	U8 indexReg = calculateIndexReg(data, topReg, reg);
	data->writeToMemFromValue(TAG_Empty, HOST_CPU, true, topReg, true, 2, CPU_OFFSET_FPU_TAG, 4, false);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(indexReg);
}
void opFNSTSW_AX(X64Asm* data) {
	PushPopFlags flags(data);
	U8 swReg = data->getTmpReg();
	U8 topReg = getTopReg(data);
	data->writeToRegFromMem(swReg, true, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, fpu.sw)), 4, false);
	data->andReg(swReg, true, ~0x3800);
	data->shiftLeftReg(topReg, true, 11);
	data->orRegReg(swReg, true, topReg, true);
	data->writeToRegFromReg(0, false, swReg, true, 2);
	data->releaseTmpReg(swReg);
	data->releaseTmpReg(topReg);
}
void opFUCOMI_ST0_STj_Pop(X64Asm* data, U8 reg) {
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, reg);
	// :TODO: FCOM and FUCOM currently do the same thing
	// will clobber flags
	doFCOMI(data, topReg, dst.reg, 0, src.reg, reg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFCOMI_ST0_STj_Pop(X64Asm* data, U8 reg) {
	U8 topReg = getTopReg(data);
	FPUReg dst(data, topReg, 0);
	FPUReg src(data, topReg, reg);
	// will clobber flags
	doFCOMI(data, topReg, dst.reg, 0, src.reg, reg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
}
void opFILD_WORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt16AsDouble/writeToRegFromMem/checkMemory changes flags
	U8 topReg = getTopReg(data);
	loadInt16AsDouble(data, TMP_XMM0, rm);
	PREP_PUSH(data, topReg, true);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);
}
void opFISTTP16(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // checkMemory changes flags
	U8 topReg = getTopReg(data);

	U8 reg = data->getTmpReg();
	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, true, 2, memReg, false);

	FPUReg src(data, topReg, 0);
	storeInt16WithTruncation(data, src.reg, reg, memReg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}
void opFIST_WORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // checkMemory changes flags
	U8 topReg = getTopReg(data);
	U8 reg = data->getTmpReg();

	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, true, 2, memReg, false);

	FPUReg src(data, topReg, 0);
	storeInt16WithTruncation(data, src.reg, reg, memReg); // until rounding is implemented, just truncate
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}
void opFIST_WORD_INTEGER_Pop(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // checkMemory changes flags
	U8 topReg = getTopReg(data);
	U8 reg = data->getTmpReg();

	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, true, 2, memReg, false);

	FPUReg src(data, topReg, 0);
	storeInt16WithTruncation(data, src.reg, reg, memReg); // until rounding is implemented, just truncate
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}
void opFILD_QWORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // loadInt32AsDouble/checkMemory changes flags
	U8 topReg = getTopReg(data);
	loadInt64AsDouble(data, TMP_XMM0, rm);
	PREP_PUSH(data, topReg, true);
	syncXmmToCPU(data, topReg, TMP_XMM0, 0);
	data->releaseTmpReg(topReg);

}
void opFISTP_QWORD_INTEGER(X64Asm* data, U8 rm) {
	PushPopFlags flags(data); // checkMemory changes flags
	U8 topReg = getTopReg(data);
	U8 reg = data->getTmpReg();

	data->calculateMemory(reg, true, rm);
	U8 memReg = data->getTmpReg();
	data->checkMemory(reg, true, true, 8, memReg, false);

	FPUReg src(data, topReg, 0);
	// storeInt64(data, src.reg, reg);
	//
	// for some reason storeInt64 causes exiting AoE to crash.  Perhaps SSE was put into a different rounding mode and truncation is just safer, or maybe there is a real bug in storeInt64
	storeInt64WithTruncation(data, src.reg, reg, memReg);
	FPU_POP(data, topReg);
	data->releaseTmpReg(topReg);
	data->releaseTmpReg(reg);
	data->releaseTmpReg(memReg);
}
#endif