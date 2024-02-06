#ifndef __ARMV8BT_ASM_H__
#define __ARMV8BT_ASM_H__

#ifdef BOXEDWINE_ARMV8BT

typedef enum {
    B8,
    B16,
    H_scaler,
    H4,
    H8,
    S_scaler,
    S2,
    S4,
    D_scaler,
    D1,
    D2
} VectorWidth;

#define xEAX 0
#define xECX 1
#define xEDX 2
#define xEBX 3
#define xESP 4
#define xEBP 5
#define xESI 6
#define xEDI 7
#define xFLAGS 8

// x9 to x15 caller saved
#define xTmp1 9
// must be at know location for exception handler
#define xBranch 9
#define xTmp2 10
#define xBranchLargeAddressOffset 10
#define xTmp3 11
#define xTmp4 12
#define xTmp5 13
#define xTmp6 14

#define xNumberOfTmpRegs 6

#define xOffset xSrc
#define xPage xDst

#define xSrc 15
#define xDst 16
#define xResult 17
#define xBranchEip 17

// don't use x18

// x19 to x29 callee saved
// These should be one that won't need to be reloaded after a function call
#define xCPU 19
#define xMemRead 20
#define xMemWrite 21

// addresses, not values
#define xES 22
#define xCS 23
#define xSS 24
#define xDS 25
#define xFS 26
#define xGS 27
#define xStackMask 28
#define xFpuTop 30
#define xFpuOffset 29

// x29 is the frame register
// x30 is the link register (used to return from subroutines)

// 18 is used as TEB?
// Apple doc: The register x18 is reserved for the platform. Conforming software should not make use of it.

#define xXMM0 0
#define xXMM1 1
#define xXMM2 2
#define xXMM3 3
#define xXMM4 4
#define xXMM5 5
#define xXMM6 6
#define xXMM7 7

// v8 - v15 are non volatile for the bottom 64-bits which is fine for MMX
// on real x86 hardware, MMX registers are also used by the FPU, so MMX and the FPU can't exist at the same time
// because of this, the FPU will use these registers too as cache
#define vMMX0 8
#define vMMX1 9
#define vMMX2 10
#define vMMX3 11
#define vMMX4 12
#define vMMX5 13
#define vMMX6 14
#define vMMX7 15

// v16 - v31 are volitile
#define vTmp1 16
#define vTmp2 17
#define vTmp3 18
#define vTmp4 19
#define vTmp5 20
#define vTmp6 21
#define vNumberOfTmpRegs 6

#define NUMBER_OF_SSE_CONSTANTS 6

#define SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE 0
#define SSE_MIN_INT32_MINUS_ONE_AS_DOUBLE 1
#define SSE_MAX_INT32_PLUS_ONE_AS_FLOAT 2
#define SSE_MIN_INT32_MINUS_ONE_AS_FLOAT 3
#define SSE_INT32_BIT_MASK 4
#define SSE_BYTE8_BIT_MASK 5

#include "armv8btData.h"

#define CPU_OFFSET_EAX (U32)(offsetof(CPU, reg[0].u32))
#define CPU_OFFSET_ECX (U32)(offsetof(CPU, reg[1].u32))
#define CPU_OFFSET_EDX (U32)(offsetof(CPU, reg[2].u32))
#define CPU_OFFSET_EBX (U32)(offsetof(CPU, reg[3].u32))
#define CPU_OFFSET_ESP (U32)(offsetof(CPU, reg[4].u32))
#define CPU_OFFSET_EBP (U32)(offsetof(CPU, reg[5].u32))
#define CPU_OFFSET_ESI (U32)(offsetof(CPU, reg[6].u32))
#define CPU_OFFSET_EDI (U32)(offsetof(CPU, reg[7].u32))
#define CPU_OFFSET_FLAGS (U32)(offsetof(CPU, flags))

#define CPU_OFFSET_ES_ADDRESS (U32)(offsetof(CPU, seg[ES].address))
#define CPU_OFFSET_CS_ADDRESS (U32)(offsetof(CPU, seg[CS].address))
#define CPU_OFFSET_SS_ADDRESS (U32)(offsetof(CPU, seg[SS].address))
#define CPU_OFFSET_DS_ADDRESS (U32)(offsetof(CPU, seg[DS].address))
#define CPU_OFFSET_FS_ADDRESS (U32)(offsetof(CPU, seg[FS].address))
#define CPU_OFFSET_GS_ADDRESS (U32)(offsetof(CPU, seg[GS].address))

#define CPU_OFFSET_ES (U32)(offsetof(CPU, seg[ES].value))
#define CPU_OFFSET_CS (U32)(offsetof(CPU, seg[CS].value))
#define CPU_OFFSET_SS (U32)(offsetof(CPU, seg[SS].value))
#define CPU_OFFSET_DS (U32)(offsetof(CPU, seg[DS].value))
#define CPU_OFFSET_FS (U32)(offsetof(CPU, seg[FS].value))
#define CPU_OFFSET_GS (U32)(offsetof(CPU, seg[GS].value))

#define CPU_OFFSET_SEG_VALUE(i) ((U32)(offsetof(CPU, seg)) + sizeof(Seg) * i + (U32)(offsetof(Seg, value)))

#define CPU_OFFSET_MEM (U32)(offsetof(Armv8btCPU, memOffset))
#define CPU_OFFSET_NEG_MEM (U32)(offsetof(Armv8btCPU, negMemOffset))
#define CPU_OFFSET_OP_PAGES (U32)(offsetof(Armv8btCPU, eipToHostInstructionPages))
#define CPU_OFFSET_EIP_HOST_MAPPING (U32)(offsetof(Armv8btCPU, eipToHostInstructionAddressSpaceMapping))

#define CPU_OFFSET_EIP (U32)(offsetof(Armv8btCPU, eip.u32))
#define CPU_OFFSET_EIP_FROM (U32)(offsetof(Armv8btCPU, fromEip))
#define CPU_OFFSET_EXIT_TO_START_LOOP (U32)(offsetof(Armv8btCPU, exitToStartThreadLoop))
#define CPU_OFFSET_RETURN_ADDRESS (U32)(offsetof(Armv8btCPU, returnToLoopAddress))
#define CPU_OFFSET_MEMOFFSET (U32)(offsetof(CPU, memOffsets))
#define CPU_OFFSET_SYNC_TO_HOST_ADDRESS (U32)(offsetof(Armv8btCPU, syncToHostAddress))
#define CPU_OFFSET_SYNC_FROM_HOST_ADDRESS (U32)(offsetof(Armv8btCPU, syncFromHostAddress))
#define CPU_OFFSET_DO_SINGLE_OP_ADDRESS (U32)(offsetof(Armv8btCPU, doSingleOpAddress))

typedef void (*PFN_FPU_REG)(CPU* cpu, U32 reg);
typedef void (*PFN_FPU_ADDRESS)(CPU* cpu, U32 address);
typedef void (*PFN_FPU)(CPU* cpu);

#define CLEAR_BUFFER_SIZE 256

enum DoIfOperator {
    DO_IF_EQUAL,
    DO_IF_NOT_EQUAL,
    DO_IF_GREATER_THAN,
    DO_IF_SIGNED_GREATER_THAN,
    DO_IF_LESS_THAN_OR_EQUAL,
    DO_IF_LESS_THAN,
    DO_IF_SIGNED_LESS_THAN_OR_EQUAL
};

class Armv8btAsm : public Armv8btData {
public:  
    Armv8btAsm(Armv8btCPU* cpu);    
    virtual void translateInstruction() override;

    U32 flagsNeeded();
    U8 getTmpReg();
    void releaseTmpReg(U8 reg);
    U8 getNativeReg(U8 reg);
    U8 getNativeFpuReg(U8 reg);
    U8 getNativeMmxReg(U8 reg);
    U8 getNativeSseReg(U8 reg);
    U8 getReadNativeReg8(U8 reg);
    void releaseNativeReg8(U8 reg);
    U8 getSegReg(U8 seg);
    U8 getFpuTopReg();
    U8 getFpuOffset();
    U8 getFpuTagOffset();
    void releaseFpuTagOffset(U8 offsetReg);

    void invalidOp(U32 op);
    void logOp(U32 eip);
    void signalIllegalInstruction(int code);

	void saveNativeState();
	void restoreNativeState();
    void addReturn();
    void createCodeForDoSingleOp();
    void emulateSingleOp(DecodedOp* op);
    void createCodeForJmpAndTranslateIfNecessary();
    void callRetranslateChunk();
#ifdef BOXEDWINE_POSIX
    void createCodeForRunSignal();
#endif
    void createCodeForSyncToHost();
    void createCodeForSyncFromHost();

    void writeToRegFromValue(U8 reg, U64 value);    
    void setNativeFlags(U32 flags, U32 mask);
    void write64Buffer(U8* buffer, U64 value);
    void write32Buffer(U8* buffer, U32 value); 
    void write16Buffer(U8* buffer, U16 value); 
    
    // arm instructions
    void pushPair(U8 r1, U8 r2);
    void popPair(U8 r1, U8 r2);    
    void movk(U8 reg, U16 value, U8 shift = 0); // (move with keep) shift can be 0, 16, 32 or 48    
    void movz(U8 reg, U16 value, U8 shift = 0); // (move with zero) shift can be 0, 16, 32 or 48
    void movn(U8 reg, U16 value, U8 shift = 0); // (move with not) shift can be 0, 16, 32 or 48
    void mov32(U8 dst, U8 src);
    void mov64(U8 dst, U8 src);
    void zeroReg(U8 reg);
    void ubfm32(U8 dst, U8 src, U8 immr, U8 imms);
    void ubfm64(U8 dst, U8 src, U8 immr, U8 imms);
    void bfm32(U8 dst, U8 src, U8 immr, U8 imms);
    void bfm64(U8 dst, U8 src, U8 immr, U8 imms);
    void clz32(U8 dst, U8 src); // count leading zeros
    void clz64(U8 dst, U8 src); // count leading zeros
    void cls32(U8 dst, U8 src); // count leading sign bits
    void cls64(U8 dst, U8 src); // count leading sign bits
    void csetEq(U8 reg); // ZF = 1
    void csetNe(U8 reg); // ZF = 0
    void csetCc(U8 reg); // CF = 1
    void csetVs(U8 reg); // OF = 1
    void csetLt(U8 reg);
    void yield();

    void cselVs(U8 dst, U8 ifTrueReg, U8 ifFalseReg);

    // high level
    U8 getAddressReg();
    void calculateAddress16(U8 dst);
    void calculateAddress32(U8 dst);
    void doIf(U8 reg, U32 value, DoIfOperator op, std::function<void(void)> ifBlock, std::function<void(void)> elseBlock = nullptr, std::function<void(void)> afterCmpBeforeBranchBlock = nullptr, bool valueIsReg = false, bool generateCmp = true);
    void doIfBitSet(U8 reg, U32 bitPos, std::function<void(void)> ifBlock, std::function<void(void)> elseBlock=nullptr);
    void compareZeroAndBranch(U8 reg, bool isZero, U32 eip);
    void writeJumpAmount(U32 pos, U32 toLocation);
    void doJmp(bool mightNeedCS); // jump to current cpu->eip
    void jmpRegToxBranchEip(bool mightNeedCS);
    virtual void jumpTo(U32 eip) override; // a jump that could be within the same chunk, this will be filled out when the entire chunk is encoded
    void addTodoLinkJump(U32 eip);
    U8 getRegWithConst(U64 value);
    void branchNativeRegister(U8 reg);
    U32 branchEQ();
    U32 branchNE();
    U32 branchUnsignedGreaterThan();
    U32 branchUnsignedGreaterThanOrEqual();
    U32 branchUnsignedLessThan();
    U32 branchUnsignedLessThanOrEqual();
    U32 branchSignedGreaterThan();    
    U32 branchSignedLessThanOrEqual();
    U32 branch();
    void syncRegsToHost();
    void syncRegsFromHost(bool eipInBranchReg = false);
    void callSyncRegsFromHost(); // doesn't set eip
    void callHost(void* pfn);
    void ret();
    void blr(U8 reg);

    // stack
    void pushNativeReg16(U8 reg);    
    void popNativeReg16(U8 reg, bool zeroExtend);
    void popNativeReg32(U8 reg);
    void pushNativeReg32(U8 reg);

    void peekNativeReg16(U8 reg, bool zeroExtend);
    void peekNativeReg32(U8 reg);
    void popStack16();
    void popStack16(U8 reg, U8 resultReg);
    void popStack32();
    void popStack32(U8 reg, U8 resultReg);

    void pushStack16(U8 reg, U8 resultReg, U32 amount=2);
    void pushStack32(U8 reg, U8 resultReg, U32 amount=4);

    // mov reg to reg
    void movReg8ToReg(U8 untranslatedSrc, U8 dst, bool signExtend=false);
    void movRegToReg8(U8 src, U8 untranslatedDst);
    void movReg8ToReg8(U8 untranslatedDst, U8 untranslatedSrc);
    void movRegToReg(U8 dst, U8 src, U32 width, bool zeroExtend);    
    void zeroExtend(U8 dst, U8 src, U32 width);
    void zeroExtend64(U8 dst, U8 src, U32 width);
    void signExtend(U8 dst, U8 src, U32 width);
    void signExtend64(U8 dst, U8 src, U32 width);
    void notReg32(U8 dst, U8 src);

    U8 getHostMem(U8 regEmulatedAddress, U32 width, bool write, bool skipAlignmentCheck = false, S8 tmpReg = -1);
    void releaseHostMem(U8 reg);

    // mov to/from memory
    void readWriteMemory(U8 addressReg, U8 readDst, U8 writeSrc, U32 width, std::function<void(void)> pfn, bool lock = false, bool doWrite = true);
    void readMemory(U8 addressReg, U8 dst, U32 width, bool addMemOffsetToAddress, bool lock = false, bool signExtend = false);
    void writeMemory(U8 addressReg, U8 src, U32 width, bool addMemOffsetToAddress, bool lock = false, U8 regWithOriginalValue = 0, U32 restartPos = 0, bool generateMemoryBarrierForLock = true);
    void fullMemoryBarrier();

    void readMem8ValueOffset(U8 dst, U8 base, S32 offset, bool signExtend = false);
    void readMem16ValueOffset(U8 dst, U8 base, S32 offset, bool signExtend = false);
    void readMem32ValueOffset(U8 dst, U8 base, S32 offset);
    void readMem64ValueOffset(U8 dst, U8 base, S32 offset);

    void readMem8RegOffset(U8 dst, U8 base, U8 offsetReg, bool signExtend = false);
    void readMem16RegOffset(U8 dst, U8 base, U8 offsetReg, bool signExtend = false);
    void readMem32RegOffset(U8 dst, U8 base, U8 offsetReg, U32 lsl = 0); // lsl can be 0 or 2
    void readMem64RegOffset(U8 dst, U8 base, U8 offsetReg, U32 lsl = 0); // lsl can be 0 or 3

    void readMem8Lock(U8 dst, U8 base);
    void readMem16Lock(U8 dst, U8 base);
    void readMem32Lock(U8 dst, U8 base);
    void readMem64Lock(U8 dst, U8 base);

    void writeMem8ValueOffset(U8 dst, U8 base, S32 offset);
    void writeMem16ValueOffset(U8 dst, U8 base, S32 offset);
    void writeMem32ValueOffset(U8 dst, U8 base, S32 offset);
    void writeMem64ValueOffset(U8 dst, U8 base, S32 offset);

    void writeMem8RegOffset(U8 dst, U8 base, U8 offsetReg);
    void writeMem16RegOffset(U8 dst, U8 base, U8 offsetReg);
    void writeMem32RegOffset(U8 dst, U8 base, U8 offsetReg, U32 lsl = 0); // lsl can be 0 or 3
    void writeMem64RegOffset(U8 dst, U8 base, U8 offsetReg, U32 lsl = 0); // lsl can be 0 or 3

    void reverseBytes32(U8 dst, U8 src);
    void reverseBits32(U8 dst, U8 src);

    // 
    void loadConst(U8 reg, U64 value);    

    // arith
    void addRegs32(U8 dst, U8 src1, U8 src2, U8 src2ShiftLeft = 0, bool flags = false);
    void addRegs64(U8 dst, U8 src1, U8 src2, U8 src2ShiftLeft = 0, bool flags = false);
    void addValue32(U8 dst, U8 src, U32 value, bool flags = false);
    void addValue64(U8 dst, U8 src, U32 value);

    void subRegs32(U8 dst, U8 src1, U8 src2, U8 src2ShiftLeft = 0, bool flags = false);
    void subRegs64(U8 dst, U8 src1, U8 src2, bool flags = false);
    void subValue32(U8 dst, U8 src, U32 value, bool flags = false);
    void subValue64(U8 dst, U8 src, U32 value, bool flags = false);
    void subRegFromValue32(U8 dst, U8 src, U32 value, bool flags = false);

    void andRegs32(U8 dst, U8 src1, U8 src2, bool flags = false);
    void andRegs64(U8 dst, U8 src1, U8 src2);
    void testRegs32(U8 src1, U8 src2);
    void testRegs64(U8 src1, U8 src2);
    void andValue32(U8 dst, U8 src, U32 value, bool flags = false);
    void testValue32(U8 src, U32 value);
    void andValue64(U8 dst, U8 src, U64 value);
    void testValue64(U8 src, U64 value);

    void andNotRegs32(U8 dst, U8 src1, U8 src2);    

    void orRegs32(U8 dst, U8 src1, U8 src2, U32 shiftLeft = 0);
    void orRegs64(U8 dst, U8 src1, U8 src2, U32 shiftLeft = 0);
    void orValue32(U8 dst, U8 src, U32 value);

    void xorRegs32(U8 dst, U8 src1, U8 src2);
    void xorValue32(U8 dst, U8 src, U32 value);

    void cmpRegs32(U8 src1, U8 src2);
    void cmpRegs64(U8 src1, U8 src2);
    void cmpValue32(U8 src, U32 value);
    void cmpValue64(U8 src, U32 value);

    void modValue32(U8 dst, U8 src, U32 value);
    void unsignedDivideReg32(U8 dst, U8 top, U8 bottom);
    void unsignedDivideReg64(U8 dst, U8 top, U8 bottom);
    void signedDivideReg32(U8 dst, U8 top, U8 bottom);
    void signedDivideReg64(U8 dst, U8 top, U8 bottom);
    void multiplySubtract32(U8 dst, U8 src1, U8 src2, U8 sub);
    void multiplySubtract64(U8 dst, U8 src1, U8 src2, U8 sub);

    void shiftRegLeftWithValue32(U8 dst, U8 src, U8 value);
    void shiftRegLeftWithValue64(U8 dst, U8 src, U8 value);
    void shiftRegLeftWithReg32(U8 dst, U8 src, U8 amountReg);
    void shiftRegLeftWithReg64(U8 dst, U8 src, U8 amountReg);
    void shiftRegRightWithValue32(U8 dst, U8 src, U8 value);
    void shiftRegRightWithValue64(U8 dst, U8 src, U8 value);
    void shiftSignedRegRightWithValue32(U8 dst, U8 src, U8 value);
    void shiftSignedRegRightWithValue64(U8 dst, U8 src, U8 value);
    void shiftRegRightWithReg32(U8 dst, U8 src, U8 amountReg);
    void shiftRegRightWithReg64(U8 dst, U8 src, U8 amountReg);
    void shiftSignedRegRightWithReg32(U8 dst, U8 src, U8 amountReg);             
    void rotateRightWithReg32(U8 dst, U8 src, U8 amountReg);
    void rotateRightWithValue32(U8 dst, U8 src, U8 value);

    void signedMultiply32(U8 dst, U8 src1, U8 src2); // src1 and src2 are signed 32-bit, dst is signed 64-bit
    void unsignedMultiply32(U8 dst, U8 src1, U8 src2);
    void unsignedMultiply64(U8 dst, U8 src1, U8 src2);

    void copyBitsFromSourceAtPositionToDest(U8 dst, U8 src, U8 srcLsb, U8 width, bool preserveOtherBits = true);
    void copyBitsFromSourceAtPositionToDest64(U8 dst, U8 src, U8 srcLsb, U8 width, bool preserveOtherBits = true);
    void copyBitsFromSourceToDestAtPosition(U8 dst, U8 src, U8 dstLsb, U8 width, bool preserveOtherBits = true);
    void copyBitsFromSourceToDestAtPosition64(U8 dst, U8 src, U8 dstLsb, U8 width, bool preserveOtherBits = true);
    void clearBits(U8 dst, U8 lsb, U8 width);   

    void getDF(U8 dst, U32 width);
    void getVirtualCounter(U8 dst);

    U8 getSSEConstant(U32 c);

    void vReadMem128RegOffset(U8 dst, U8 base, U8 offsetReg); 
    void vReadMemory128(U8 addressReg, U8 dst, bool addMemOffsetToAddress);
    void vReadMem128ValueOffset(U8 dst, U8 base, S32 offset);
    void vWriteMem128RegOffset(U8 dst, U8 base, U8 offsetReg);
    void vWriteMemory128(U8 addressReg, U8 dst, bool addMemOffsetToAddress);
    void vWriteMem128ValueOffset(U8 dst, U8 base, S32 offset);

    void vReadMemory64(U8 addressReg, U8 dst, U32 index, bool addMemOffsetToAddress);
    void vWriteMemory64(U8 addressReg, U8 dst, U32 index, bool addMemOffsetToAddress);

    // zero extend reads
    void vReadMem32RegOffset(U8 dst, U8 base, U8 offsetReg);
    void vReadMemory32(U8 addressReg, U8 dst, bool addMemOffsetToAddress);
    void vReadMem32ValueOffset(U8 dst, U8 base, S32 offset);
    void vReadMem64RegOffset(U8 dst, U8 base, U8 offsetReg, U32 lsl = 0); // lsl can be 0 or 3
    void vReadMemory64(U8 addressReg, U8 dst, bool addMemOffsetToAddress);
    void vReadMem64ValueOffset(U8 dst, U8 base, S32 offset);

    void vWriteMem64RegOffset(U8 dst, U8 base, U8 offsetReg, U32 lsl = 0); // lsl can be 0 or 3

    void vReadMemory32(U8 addressReg, U8 dst, U32 index, bool addMemOffsetToAddress);
    void vWriteMemory32(U8 addressReg, U8 dst, U32 index, bool addMemOffsetToAddress);

    void vReadMemory16(U8 addressReg, U8 dst, U32 index, bool addMemOffsetToAddress);
    void vWriteMemory16(U8 addressReg, U8 dst, U32 index, bool addMemOffsetToAddress);

    void vReadMemMultiple128(U8 dst, U8 base, U32 numberOfRegs, bool incrementBase);
    void vWriteMemMultiple128(U8 dst, U8 base, U32 numberOfRegs, bool incrementBase);
    void vReadMemMultiple64(U8 dst, U8 base, U32 numberOfRegs, bool incrementBase);
    void vWriteMemMultiple64(U8 dst, U8 base, U32 numberOfRegs, bool incrementBase);

    void vMov128(U8 dst, U8 src);
    void vMov64(U8 dst, U32 dstIndex, U8 src, U32 srcIndex);
    void vMov64ToScaler(U8 dst, U8 src, U32 srcIndex);
    void vMov32(U8 dst, U32 dstIndex, U8 src, U32 srcIndex);
    void vMov32ToScaler(U8 dst, U8 src, U32 srcIndex);
    void vMov16(U8 dst, U32 dstIndex, U8 src, U32 srcIndex);
    void vMovFromGeneralReg16(U8 dst, U32 dstIndex, U8 src);
    void vMovFromGeneralReg32(U8 dst, U32 dstIndex, U8 src);
    void vMovToGeneralReg32ZeroExtend(U8 dst, U8 src, U32 srcIndex, VectorWidth width);
    void vMovFromGeneralReg64(U8 dst, U32 dstIndex, U8 src);
    void vMovToGeneralReg64(U8 dst, U8 src, U32 srcIndex);
    void vExtractVectorFromPair(U8 dst, U8 src1, U8 src2, U32 startIndexOfSrc1);
    void vZeroExtend64To128(U8 dst, U8 src);
    void vLoadConst(U8 dst, U64 value, VectorWidth width);

    void vZipFromLow128(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vZipFromHigh128(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vUnzipOdds(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vUnzipEvens(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vTbx(U8 dst, U8 src, U8 srcCount, U8 srcIndex, VectorWidth width);

    void vConvertInt32ToFloat(U8 dst, U8 src, bool isVector);
    void vConvertInt64ToDouble(U8 dst, U8 src, bool isVector);
    void vConvertInt64ToLowerInt32(U8 dst, U8 src);

    void vSignedSaturateToSignedNarrowToLowerAndClear(U8 dst, U8 src, VectorWidth width); // upper is cleared
    void vSignedSaturateToSignedNarrowToUpperAndKeep(U8 dst, U8 src, VectorWidth width); // lower is not cleared
    void vUnsignedSaturateToUnsignedNarrowToLowerAndClear(U8 dst, U8 src, VectorWidth width); // upper is cleared
    void vUnsignedSaturateToUnsignedNarrowToUpperAndKeep(U8 dst, U8 src, VectorWidth width); // lower is not cleared
    void vSignedSaturateToUnsignedNarrowToLowerAndClear(U8 dst, U8 src, VectorWidth width); // upper is cleared
    void vSignedSaturateToUnsignedNarrowToUpperAndKeep(U8 dst, U8 src, VectorWidth width); // lower is not cleared

    void vSignExtend64To128(U8 dst, U8 src, VectorWidth width);

    void vConvertDoubleToInt64RoundToCurrentMode(U8 dst, U8 src, bool isVector);
    void vConvertDoubleToInt64RoundToZero(U8 dst, U8 src, bool isVector);
    void vConvertDoubleToInt64RoundToNearest(U8 dst, U8 src, bool isVector);
    void vConvertDoubleToGeneralReg32RoundToNearest(U8 dst, U8 src);
    void vConvertDoubleToGeneralReg32RoundToZero(U8 dst, U8 src);
    void vConvertDoubleToGeneralReg32RoundToCurrentMode(U8 dst, U8 src);    
    void vConvertDoubleToFloatRoundToCurrentModeAndKeep(U8 dst, U8 src); // does not affect upper bits
    void vConvertDoubleToFloatRoundToZeroAndKeep(U8 dst, U8 src);

    void vConvertFloatToGeneralReg32RoundToCurrentMode(U8 dst, U8 src);
    void vConvertFloatToGeneralReg32RoundToZero(U8 dst, U8 src);
    void vConvertFloatToGeneralReg32RoundToNearest(U8 dst, U8 src);
    void vConvertFloatToInt32RoundToCurrentMode(U8 dst, U8 src, bool isVector);
    void vConvertFloatToInt32RoundToZero(U8 dst, U8 src, bool isVector);
    void vConvertFloatToInt32RoundToNearest(U8 dst, U8 src, bool isVector);
    void vConvertFloatToDouble(U8 dst, U8 src, bool isVector);

    void vDup(U8 dst, U8 src, U8 srcIndex, VectorWidth width);
    void vNeg(U8 dst, U8 src, VectorWidth width);

    void vUnsignedAbsoluteDifference(U8 dst, U8 src1, U8 src2, VectorWidth width);    
    void vMul(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vUnsignedMulLongLower(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vUnsignedMulLongUpper(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vSignedMulLongLower(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vSignedMulLongUpper(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vAddAcrossVectorToScaler(U8 dst, U8 src, VectorWidth width);
    void vUnsignedAddPairsLong(U8 dst, U8 src, VectorWidth width);
    void vUnsignedAddAcrossVectLong(U8 dst, U8 src, VectorWidth width);
    void vAddPairs(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vAdd(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vUnsignedSaturatingAdd(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vSignedSaturatingAdd(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vSub(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vUnsignedSaturatingSub(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vSignedSaturatingSub(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vOr(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vXor(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vAnd(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vAndNot(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vNot(U8 dst, U8 src1, VectorWidth width);
    void vSignedShiftRightValue(U8 dst, U8 src, U8 amount, VectorWidth width);    
    void vShiftLeftValue(U8 dst, U8 src, U8 amount, VectorWidth width);
    void vShiftRightValue(U8 dst, U8 src, U8 amount, VectorWidth width);
    void vShiftWithReg(U8 dst, U8 src, U8 srcAmounts, VectorWidth width); //if srcAmount element is positive then it will shift left, else right
    void vSignedShiftWithReg(U8 dst, U8 src, U8 srcAmounts, VectorWidth width); //if srcAmount element is positive then it will shift left, else right
    void vShiftRightValueAndNarrow(U8 dst, U8 src, U8 amount, VectorWidth width);
    void vSelectBit(U8 dst, U8 src1, U8 src2, VectorWidth width); // if dst bit is set, then src1 bit will be copied to dst, else src2 bit will be copied
    void vCmpGreaterThan(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vCmpEqual(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vUnsignedMin(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vSignedMin(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vUnsignedMax(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vSignedMax(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void vUnsignedRoundedAverage(U8 dst, U8 src1, U8 src2, VectorWidth width);

    void vReleaseTmpReg(U8 reg);
    U8 vGetTmpReg();

    void fMovFromGeneralRegister64(U8 dst, U8 src);
    void fCmp64(U8 src1, U8 src2); // non signaling
    void fCmp32(U8 src1, U8 src2); // non signaling
    void fCmpGreaterThanOrEqual(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void fCmpGreaterThan(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void fCmpLessThanOrEqual(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void fCmpLessThan(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void fCmpEqual(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void fSqrt(U8 dst, U8 src, VectorWidth width);
    void fRsqrt(U8 dst, U8 src, VectorWidth width);
    void fReciprocal(U8 dst, U8 src, VectorWidth width);
    void fAdd(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void fSub(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void fMul(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void fDiv(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void fMin(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void fMax(U8 dst, U8 src1, U8 src2, VectorWidth width);
    void fAbs(U8 dst, U8 src, VectorWidth width);
    void fNeg(U8 dst, U8 src, VectorWidth width);

    bool tmpRegInUse[xNumberOfTmpRegs];
    bool vTmpRegInUse[vNumberOfTmpRegs];
    
#ifdef __TEST
    void addReturnFromTest();
#endif

    virtual void reset() override;
private:
    void vMemMultiple(U8 dst, U8 base, U32 numberOfRegs, U8 thirdByte, bool is1128);
    void vIns(U8 rd, U8 rn, U8 imm4, U8 imm5);

    bool isEipInChunk(U32 eip);
};

#endif
#endif
