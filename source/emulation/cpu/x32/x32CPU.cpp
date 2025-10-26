/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#ifdef BOXEDWINE_DYNAMIC32

#include "x32CPU.h"
#include "../dynamic/dynamicSSE.h"
#include "x86Asm.h"
#include <array>

// cdecl calling convention states EAX, ECX, and EDX are caller saved

// -: 31.1, 31.0, 31.2 (esi saved)
// -: 31.1, 31.2, 31.2 (esi not saved)
// with the 2 scores above, it looks like pushing/popping esi everytime going in and out of the JIT code does not have much overhead
// 
// Quake scores with different regs cached, seems like cachiing eax has the most impact
// 0: 32.4, 32.4, 32.6
// 1: 31.2, 31.1, 31.3
// 2: 31.5, 31.4, 31.4
// 3: 31.2, 31.0, 31.1
// 4: 31.4, 31.2, 31.3
// 5: 31.2, 31.4, 31.2
// 6: 31.5, 31.6, 31.6
// 7: 30.9, 31.0, 31.1
// 0+6: 32.2, 32.3, 32.1
static U8 regCache[] = { 5, 0, 0, 0, 0, 0, 6, 0 };

class X86DynamicCodeGen : public DynamicCodeGenSSE {
public:    
    X86DynamicCodeGen(CPU* cpu) : DynamicCodeGenSSE(cpu) {}

    // V2
    void preOp(DecodedOp* op) override;
    RegPtr getReg(U8 reg) override;
    RegPtr getReg8(U8 reg) override;
    RegPtr getReadOnlyReg(U8 reg, bool delayed = false) override;
    RegPtr getReadOnlyReg8(U8 reg, bool delayed = false) override;
    RegPtr getTmpReg() override;
    RegPtr getTmpReg(U8 reg, bool delayed = false) override;
    RegPtr getTmpReg8(U8 reg, bool delayed = false) override;
    RegPtr getTmpRegForCallResult() override;
    RegPtr getSegAddress(U8 reg) override;

    U8 findTmpReg();
    void updateFlagsIfNecessary();
    void updateHardwareFlags(U32 flags);

    void read(DynWidth width, RegPtr dest, RegPtr reg, U8 lsl, U32 disp) override;
    void read(DynWidth width, RegPtr dest, RegPtr reg, RegPtr sib, U8 lsl, U32 disp) override;
    void write(DynWidth width, RegPtr reg, U32 disp, RegPtr src) override;
    void write(DynWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, RegPtr src) override;

    void IfLessThan2(DynWidth regWidth, RegPtr reg, U32 value) override;
    void IfNot(DynWidth regWidth, RegPtr reg) override;
    void If(DynWidth regWidth, RegPtr reg) override;
    void IfEqual(DynWidth regWidth, RegPtr reg, U32 value) override;
    void IfBitSet2(DynWidth regWidth, RegPtr reg, U32 value, bool bigJump = false) override;

    void callHostFunction(void* address, const std::vector<DynParam>& params) override;
    void callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) override;
    void pushParam(const DynParam& param);

    void mov(DynWidth regWidth, RegPtr dest, RegPtr src) override;
    void movValue(DynWidth regWidth, RegPtr dst, U32 imm) override;

    void addReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void addValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) override;
    void adcReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void adcValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) override;
    void orReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void orValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) override;
    void subReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void subValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) override;
    void sbbReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void sbbValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) override;
    void andReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void andValue(DynWidth regWidth, RegPtr reg, U32 immm, bool checkFlags) override;
    void xorReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void xorValue(DynWidth regWidth, RegPtr reg, U32 immm, bool checkFlags) override;
    void cmpReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void cmpValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) override;
    void testReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void testValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) override;
    void shrReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void shrValue(DynWidth regWidth, RegPtr reg, U32 immm, bool checkFlags) override;
    void shlReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void shlValue(DynWidth regWidth, RegPtr reg, U32 immm, bool checkFlags) override;
    void sarReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void sarValue(DynWidth regWidth, RegPtr reg, U32 immm, bool checkFlags) override;
    void negReg2(DynWidth regWidth, RegPtr reg, bool checkFlags) override;
    void notReg2(DynWidth regWidth, RegPtr reg, bool checkFlags) override;

    std::array<bool, 8> regUsed2;
    DecodedOp* currentOp = nullptr;

    void incrementEip(U32 inc) override;
    void setConditional(DynConditional condition) override;
    void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) override;
    void addRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void orRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void subRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void andRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void xorRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void shrRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void sarRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void shlRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void rolRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void rorRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void imulRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) override;
    void imulRegReg64(DynReg high64, DynReg dst, DynReg src, bool doneWithSrcReg) override;
    void mulRegReg64(DynReg high64, DynReg dst, DynReg src, bool doneWithSrcReg) override;
    void divRegRegWithRemainder(DynReg dest, DynReg src, DynReg remainder, DynWidth width) override;
    void idivRegRegWithRemainder(DynReg dest, DynReg src, DynReg remainder, DynWidth width) override;

    void addRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void orRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void subRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void andRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void xorRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void shrRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void sarRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void shlRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void rolRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void rorRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;
    void imulRegImm(DynReg reg, DynWidth regWidth, U32 imm) override;

    void negReg(DynReg reg, DynWidth regWidth) override;
    void notReg(DynReg reg, DynWidth regWidth) override;
    void callHostFunction(void* address, bool hasReturn = false, U32 argCount = 0, U32 arg1 = 0, DynCallParamType arg1Type = DYN_PARAM_CONST_32, bool doneWithArg1 = true, U32 arg2 = 0, DynCallParamType arg2Type = DYN_PARAM_CONST_32, bool doneWithArg2 = true, U32 arg3 = 0, DynCallParamType arg3Type = DYN_PARAM_CONST_32, bool doneWithArg3 = true, U32 arg4 = 0, DynCallParamType arg4Type = DYN_PARAM_CONST_32, bool doneWithArg4 = true, U32 arg5 = 0, DynCallParamType arg5Type = DYN_PARAM_CONST_32, bool doneWithArg5 = true) override;
    void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) override;
    void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) override;
    void movToReg(DynReg reg, DynWidth width, U32 imm) override;
    void calculateEaa(DecodedOp* op, DynReg reg) override;
    void pushValue(U32 arg, DynCallParamType argType) override;
    void byteSwapReg32(DynReg reg) override;
    void zeroExtendReg16To32(DynReg dest, DynReg src) override;
    void JumpIf(DynReg reg, bool doneWithReg, U32 address) override;
    void JumpIfNot(DynReg reg, bool doneWithReg, U32 address) override;
    void JumpInBlock(U32 address) override;
    void IfNot(DynReg reg, bool doneWithReg) override;
    void If(DynReg reg, bool doneWithReg, bool bigJump = false) override;
    void IfPtrEqual(DynReg reg, DYN_PTR_SIZE value, bool doneWithReg) override;
    void StartElse(bool bigJump = false) override;
    void EndIf(bool bigJump = false) override;
    void blockExit() override;

    void loadReg(U8 reg, DynReg tmpReg, DynWidth width) override;
    void storeReg(U8 reg, DynReg srcReg, DynWidth width, bool doneWithSrcReg) override;
    void storeReg(U8 reg, DynWidth dstWidth, U32 imm) override;

    // FPU
    void storeCpuFpuReg(DynFpuReg reg, DynReg index, DynFpuWidth width = DYN_FPU_64_BIT) override;
    void loadCpuFpuReg(DynFpuReg reg, DynReg index, DynFpuWidth width = DYN_FPU_64_BIT) override;
    void loadCpuFpuRegConst(DynFpuReg reg, U32 offset) override;
    void loadFpuRegFromInt(DynFpuReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void storeFpuReg(DynFpuReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp, DynFpuWidth width = DYN_FPU_64_BIT) override;
    void fpuRegToInt32(DynReg regDst, DynFpuReg fpuRegSrc, bool truncate) override;
    void fpuRegToInt64(DynFpuReg regDst, DynFpuReg fpuRegSrc, bool truncate) override;
    void fpuRegInt64To64(DynFpuReg regDst, DynFpuReg fpuRegSrc) override;
    void loadFpuReg(DynFpuReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp, DynFpuWidth width = DYN_FPU_64_BIT) override;
    void fpuRegExtend32To64(DynFpuReg dst, DynFpuReg src) override;
    void fpuReg64To32(DynFpuReg dst, DynFpuReg src) override;
    void regToFpuReg(DynFpuReg dst, DynReg src) override;
    void updateFPURounding(DynReg tmp1, DynReg tmp2) override;
    void restoreFPURounding() override;

    void fpuAdd(DynFpuReg dst, DynFpuReg src) override;
    void fpuMul(DynFpuReg dst, DynFpuReg src) override;
    void fpuSub(DynFpuReg dst, DynFpuReg src) override;
    void fpuDiv(DynFpuReg dst, DynFpuReg src) override;
    void fpuXor(DynFpuReg dst, DynFpuReg src) override;
    void fpuAnd(DynFpuReg dst, DynFpuReg src) override;
    void fpuSqrt(DynFpuReg dst, DynFpuReg src) override;
    void fcompare(DynFpuReg fpuReg1, DynFpuReg fpuReg2, DynReg ordTags, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) override;

    // MMX
    DynMMXReg getTmpMMX(U8 inUse) override { return inUse == 0 ? (DynMMXReg)1 : (DynMMXReg)0; }
    void loadMMXFromReg(DynMMXReg mmx, DynReg reg) override;
    void storeCpuMMXReg(DynMMXReg reg, U32 index) override;
    void storeMMXToReg(DynMMXReg mmx, DynReg reg) override;
    void loadCpuMMXReg(DynMMXReg reg, U32 index) override;
    void loadMMXFromMem32(DynMMXReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void loadMMXFromMem64(DynMMXReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void storeMMXToMem32(DynMMXReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void storeMMXToMem64(DynMMXReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void xorMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void orMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void andMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void andnMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psllwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psrlwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psrawMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psllwMmx(DynMMXReg dst, U32 imm) override;
    void psrlwMmx(DynMMXReg dst, U32 imm) override;
    void psrawMmx(DynMMXReg dst, U32 imm) override;
    void pslldMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psrldMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psradMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pslldMmx(DynMMXReg dst, U32 imm) override;
    void psrldMmx(DynMMXReg dst, U32 imm) override;
    void psradMmx(DynMMXReg dst, U32 imm) override;
    void psllqMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psrlqMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psllqMmx(DynMMXReg dst, U32 imm) override;
    void psrlqMmx(DynMMXReg dst, U32 imm) override;    
    void paddbMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void paddwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void padddMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void paddsbMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void paddswMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void paddusbMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void padduswMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psubbMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psubwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psubdMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psubsbMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psubswMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psubusbMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psubuswMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pmulhwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pmullwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pmaddwdMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pcmpeqbMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pcmpeqwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pcmpeqdMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pcmpgtbMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pcmpgtwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pcmpgtdMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void packsswbMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void packssdwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void packuswbMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void punpckhbwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void punpckhwdMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void punpckhdqMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void punpcklbwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void punpcklwdMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void punpckldqMmxMmx(DynMMXReg dst, DynMMXReg src) override;

    void pavgbMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pavgwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psadbwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pextrwRegMmx(DynReg dst, DynMMXReg src, U8 srcIndex) override;
    void pinsrwMmxReg(DynMMXReg dest, DynReg src, U8 dstIndex) override;
    void pmaxswMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pmaxubMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pminswMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pminubMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pmovmskbMmxMmx(DynReg dst, DynMMXReg src) override;
    void pmulhuwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pshufwMmxMmx(DynMMXReg dst, DynMMXReg src, U8 mask) override;
    void maskmovq(DynMMXReg src, DynMMXReg mask, DynReg destAddress) override;

    void paddqMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psubqMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pmuludqMmxMmx(DynMMXReg dst, DynMMXReg src) override;

    // SSE
    DynXMMReg getTmpXMM(U8 inUse) override { return inUse == 0 ? (DynXMMReg)1 : (DynXMMReg)0; }
    void storeCpuXMMReg(DynXMMReg reg, U32 index) override;
    void loadCpuXMMReg(DynXMMReg reg, U32 index) override;
    void loadCpuXMMReg64ZeroExtend(DynXMMReg reg, U32 index) override;
    void loadXMMFromMem128(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void loadXMMFromMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void loadLowXMMFromMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void loadHighXMMFromMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void loadXMMFromMem32(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void storeXMMToMem128(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void storeXMMToMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void storeXMMToMem32(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void storeHighXMMToMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) override;

    void addpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void addssXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void subpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void subssXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void mulpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void mulssXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void divpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void divssXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void rcppsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void rcpssXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void sqrtpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void sqrtssXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void rsqrtpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void rsqrtssXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void maxpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void maxssXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void minpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void minssXmmXmm(DynXMMReg dst, DynXMMReg src) override;    
    void andnpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void andpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void orpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void xorpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvtpi2psXmmMmx(DynXMMReg dst, DynMMXReg src) override;
    void cvtps2piMmxXmm(DynMMXReg dst, DynXMMReg src) override;
    void cvtsi2ssXmmR32(DynXMMReg dst, DynReg src) override;
    void cvtss2siR32Xmm(DynReg dst, DynXMMReg src) override;
    void cvttps2piMmxXmm(DynMMXReg dst, DynXMMReg src) override;
    void cvttss2siR32Xmm(DynReg dst, DynXMMReg src) override;
    void movhlpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void movlhpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void movmskpsR32Xmm(DynReg dst, DynXMMReg src) override;
    void movssXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void shufpsXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) override;
    void unpckhpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void unpcklpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cmppsXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) override;
    void cmpssXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) override;
    void comissXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void ucomissXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void sfence() override;
    void stmxcsr(DynReg address) override;
    void ldmxcsr(DynReg address) override;

    // SSE2
    void addpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void addsdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void subpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void subsdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void mulpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void mulsdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void divpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void divsdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void maxpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void maxsdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void minpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void minsdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void paddbXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void paddwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void padddXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void paddqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void paddsbXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void paddswXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void paddusbXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void padduswXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psubbXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psubwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psubdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psubqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psubsbXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psubswXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psubusbXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psubuswXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pmaddwdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pmulhwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pmullwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pmuludqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void sqrtpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void sqrtsdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void andnpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void andpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pandXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pandnXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void porXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pslldqXmm(DynXMMReg dst, U32 imm) override;
    void psllqXmm(DynXMMReg dst, U32 imm) override;
    void pslldXmm(DynXMMReg dst, U32 imm) override;
    void psllwXmm(DynXMMReg dst, U32 imm) override;
    void psradXmm(DynXMMReg dst, U32 imm) override;
    void psrawXmm(DynXMMReg dst, U32 imm) override;
    void psrldqXmm(DynXMMReg dst, U32 imm) override;
    void psrlqXmm(DynXMMReg dst, U32 imm) override;
    void psrldXmm(DynXMMReg dst, U32 imm) override;
    void psrlwXmm(DynXMMReg dst, U32 imm) override;
    void psllqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pslldXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psllwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psradXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psrawXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psrlqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psrldXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psrlwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pxorXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void orpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void xorpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cmppdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) override;
    void cmpsdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) override;
    void comisdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void ucomisdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pcmpgtbXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pcmpgtwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pcmpgtdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pcmpeqbXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pcmpeqwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pcmpeqdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvtdq2pdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvtdq2psXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvtpd2piMmxXmm(DynMMXReg dst, DynXMMReg src) override;
    void cvtpi2pdXmmMmx(DynXMMReg dst, DynMMXReg src) override;
    void cvtpd2dqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvtpd2psXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvttpd2piMmxXmm(DynMMXReg dst, DynXMMReg src) override;
    void cvtps2dqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvtps2pdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvtsd2siR32Xmm(DynReg dst, DynXMMReg src) override;
    void cvtsd2ssXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvtsi2sdXmmR32(DynXMMReg dst, DynReg src) override;
    void cvtss2sdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvttpd2dqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvttps2dqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvttsd2siR32Xmm(DynReg dst, DynXMMReg src) override;
    void movsdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void movupdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void movmskpd(DynReg dst, DynXMMReg src) override;
    void movd(DynReg dst, DynXMMReg src) override;
    void movd(DynXMMReg dst, DynReg src) override;
    void movdq2q(DynMMXReg dst, DynXMMReg src) override;
    void movq2dq(DynXMMReg dst, DynMMXReg src) override;

    void maskmovdqu(DynXMMReg dst, DynXMMReg src, DynReg address) override;
    void pshufdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) override;
    void pshufhwXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) override;
    void pshuflwXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) override;
    void shufpdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) override;
    void unpckhpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void unpcklpdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void punpckhbwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void punpckhwdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void punpckhdqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void punpckhqdqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void punpcklbwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void punpcklwdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void punpckldqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void punpcklqdqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void packssdwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void packsswbXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void packuswbXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pavgbXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pavgwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void psadbwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pmaxswXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pmaxubXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pminswXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pminubXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void pmulhuwXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void lfence() override;
    void mfence() override;
    void clflush(DynReg rm, DynReg sib, U8 lsl, U32 disp) override;
    void pause() override;
    void pextrwR32Xmm(DynReg dst, DynXMMReg src, U32 imm) override;
    void pinsrwXmmR32(DynXMMReg dst, DynReg src, U32 imm) override;
    void pmovmskbR32Xmm(DynReg dst, DynXMMReg src) override;

    // optional override, hopefully faster than the common_ methods
    void dynamic_rdtsc(DecodedOp* op) override;
    void dynamic_checkFlags(DecodedOp* op, DynReg tmpReg, DynReg tmpReg2);
    void dynamic_arithE32R32_lock(DecodedOp* op, std::function<void(DynReg dest, DynReg address, DynReg offset)> callback, std::function<void()> fallback, bool writeReg = false);
    void dynamic_arithE16R16_lock(DecodedOp* op, std::function<void(DynReg dest, DynReg address, DynReg offset)> callback, std::function<void()> fallback, bool writeReg = false);
    void dynamic_arithE8R8_lock(DecodedOp* op, std::function<void(DynReg dest, DynReg address, DynReg offset)> callback, std::function<void()> fallback, bool writeReg = false);
    void dynamic_arithE32_lock(DecodedOp* op, std::function<void(DynReg address, DynReg offset)> callback, std::function<void()> fallback);
    void dynamic_arithE16_lock(DecodedOp* op, std::function<void(DynReg address, DynReg offset)> callback, std::function<void()> fallback);
    void dynamic_arithE8_lock(DecodedOp* op, std::function<void(DynReg address, DynReg offset)> callback, std::function<void()> fallback);

    void dynamic_cmpxchg8b_lock(DecodedOp* op) override;
    void dynamic_cmpxchge32r32_lock(DecodedOp* op) override;
    void dynamic_cmpxchge16r16_lock(DecodedOp* op) override;
    void dynamic_cmpxchge8r8_lock(DecodedOp* op) override;
    void dynamic_xchge32r32_lock(DecodedOp* op) override;
    void dynamic_xchge16r16_lock(DecodedOp* op) override;
    void dynamic_xchge8r8_lock(DecodedOp* op) override;
    void dynamic_xaddr32e32_lock(DecodedOp* op) override;
    void dynamic_xaddr16e16_lock(DecodedOp* op) override;
    void dynamic_xaddr8e8_lock(DecodedOp* op) override;
    void dynamic_adde32r32_lock(DecodedOp* op) override;
    void dynamic_adde16r16_lock(DecodedOp* op) override;
    void dynamic_adde8r8_lock(DecodedOp* op) override;
    void dynamic_add32_mem_lock(DecodedOp* op) override;
    void dynamic_add16_mem_lock(DecodedOp* op) override;
    void dynamic_add8_mem_lock(DecodedOp* op) override;
    void dynamic_sube32r32_lock(DecodedOp* op) override;
    void dynamic_sube16r16_lock(DecodedOp* op) override;
    void dynamic_sube8r8_lock(DecodedOp* op) override;
    void dynamic_sub32_mem_lock(DecodedOp* op) override;
    void dynamic_sub16_mem_lock(DecodedOp* op) override;
    void dynamic_sub8_mem_lock(DecodedOp* op) override;
    void dynamic_inc32_mem32_lock(DecodedOp* op) override;
    void dynamic_inc16_mem16_lock(DecodedOp* op) override;
    void dynamic_inc8_mem8_lock(DecodedOp* op) override;
    void dynamic_dec32_mem32_lock(DecodedOp* op) override;
    void dynamic_dec16_mem16_lock(DecodedOp* op) override;
    void dynamic_dec8_mem8_lock(DecodedOp* op) override;
    void dynamic_note32_lock(DecodedOp* op) override;
    void dynamic_note16_lock(DecodedOp* op) override;
    void dynamic_note8_lock(DecodedOp* op) override;
    void dynamic_nege32_lock(DecodedOp* op) override;
    void dynamic_nege16_lock(DecodedOp* op) override;
    void dynamic_nege8_lock(DecodedOp* op) override;
    void dynamic_btse32_lock(DecodedOp* op) override;
    void dynamic_btse16_lock(DecodedOp* op) override;
    void dynamic_btse32r32_lock(DecodedOp* op) override;
    void dynamic_btse16r16_lock(DecodedOp* op) override;
    void dynamic_btre32_lock(DecodedOp* op) override;
    void dynamic_btre16_lock(DecodedOp* op) override;
    void dynamic_btre32r32_lock(DecodedOp* op) override;
    void dynamic_btre16r16_lock(DecodedOp* op) override;
    void dynamic_btce32_lock(DecodedOp* op) override;
    void dynamic_btce16_lock(DecodedOp* op) override;
    void dynamic_btce32r32_lock(DecodedOp* op) override;
    void dynamic_btce16r16_lock(DecodedOp* op) override;    

protected:
    friend void startNewJIT(CPU* cpu, U32 address, DecodedOp* op);

    void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) override;
    void movToCpuFromReg(DynReg sib, U8 lsl, U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) override;
    void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width) override;
    void movToRegFromCpu(DynReg reg, DynReg sib, U8 lsl, U32 srcOffset, DynWidth width) override;
    void movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm) override;
    void movToCpu(DynReg sib, U8 lsl, U32 dstOffset, DynWidth dstWidth, U32 imm) override;
    void IfLessThan(DynReg reg, U32 value, bool doneWithReg) override;
    void IfBitSet(DynReg reg, U32 value, bool doneWithReg, bool bigJump = false) override;
    void IfNotBitSet(DynReg reg, U32 value, bool doneWithReg, bool bigJump = false);

    U32 getBufferSize() override;
    U32 getIfJumpSize() override;    
    U8* getBuffer() override;

    void IfEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg);
    void IfNotEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg);    
    void setCC(X86Asm::Reg32 reg, DynConditionEvaluate condition);
    void evaluateToReg(X86Asm::Reg32 reg, X86Asm::Reg32 left, X86Asm::Reg32 right, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg);
    void evaluateToReg(X86Asm::Reg32 reg, X86Asm::Reg32 left, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg);    

    void jmp(DynReg reg) override;
    void readMem(DynReg reg, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) override;    
    void readMem(DynReg reg, DynWidth width, DynReg address, U8 lsl, U32 disp) override;
    void writeMem(DynReg reg, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) override;
    void writeMem(DynReg reg, DynWidth width, DynReg address, U32 disp) override;
    void writeMem(U32 value, DynWidth width, DynReg address, U32 disp) override;
    void writeMem(U32 value, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) override;
    void and32(DynReg dst, U32 imm) override;
    void shr32(DynReg reg, U32 imm) override;

    void preCommitJIT() override;
    void patch(U8* begin) override;
    U8* createStartJITCode() override;

    void loadCache();
    void writeCache();
    void loadHardwareFlags();
    void calculateEffectiveEaa32(DecodedOp* op, DynReg reg, DynReg tmpReg);
    void calculateEffectiveEaa16(DecodedOp* op, DynReg reg, DynReg tmpReg);

    X86Asm x86;
};

void X86DynamicCodeGen::preOp(DecodedOp* op) {
    regUsed2.fill(false);
    currentOp = op;
}

U8 X86DynamicCodeGen::findTmpReg() {
    U8 tmpReg = 0xff;
    for (int i = 3; i >= 0; i--) {
        if (!regUsed2[i]) {
            regUsed2[i] = true;
            tmpReg = i;
            break;
        }
    }
    if (tmpReg == 0xff) {
        kpanic("X86DynamicCodeGen::getTmpReg ran out of tmp regs");
    }
    return tmpReg;
}

RegPtr X86DynamicCodeGen::getTmpReg() {
    return std::shared_ptr<DynReg2>(new DynReg2(findTmpReg(), 0xff), [this](DynReg2* p) {
        if (p->isLoaded()) {
            regUsed2[p->hardwareReg()] = false;
        }
        delete p;
        });
}

RegPtr X86DynamicCodeGen::getTmpReg(U8 reg, bool delayed) {    
    if (delayed) {
        auto getTmp = [reg, this]() {
            U8 hardwareReg = findTmpReg();
            if (regCache[reg]) {
                x86.mov(X86Asm::Reg32(hardwareReg), X86Asm::Reg32(regCache[reg]));
            } else {
                x86.readMem(X86Asm::Reg32(hardwareReg), x86.edi, CPU::offsetofReg32(reg));
            }
            return hardwareReg;
            };

        return std::shared_ptr<DynReg2>(new DynReg2(0xff, reg, getTmp), [this](DynReg2* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
            });
    } else {
        RegPtr result = std::shared_ptr<DynReg2>(new DynReg2(findTmpReg(), reg), [this](DynReg2* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
            });
        if (regCache[reg]) {
            x86.mov(X86Asm::Reg32(result->hardwareReg()), X86Asm::Reg32(regCache[reg]));
        } else {
            x86.readMem(X86Asm::Reg32(result->hardwareReg()), x86.edi, CPU::offsetofReg32(reg));
        }
        return result;
    }
}

RegPtr X86DynamicCodeGen::getTmpRegForCallResult() {
    U8 reg;

    if (regUsed2[0]) {
        reg = findTmpReg();
    } else {
        reg = 0;
        regUsed2[0] = true;
    }
    return std::shared_ptr<DynReg2>(new DynReg2(reg, 0xff), [this](DynReg2* p) {
        if (p->isLoaded()) {
            regUsed2[p->hardwareReg()] = false;
        }
        delete p;
    });
}

RegPtr X86DynamicCodeGen::getTmpReg8(U8 reg, bool delayed) {
    bool isHigh = reg > 3;
    RegPtr result;

    if (isHigh) {
        reg -= 4;
    }
    if (delayed) {
        auto getTmp = [reg, this]() {
            U8 hardwareReg = findTmpReg();
            if (regCache[reg]) {
                x86.mov(X86Asm::Reg32(hardwareReg), X86Asm::Reg32(regCache[reg]));
            } else {
                x86.readMem(X86Asm::Reg32(hardwareReg), x86.edi, CPU::offsetofReg32(reg));
            }
            return hardwareReg;
        };

        result = std::shared_ptr<DynReg2>(new DynReg2(0xff, reg, isHigh, getTmp), [this](DynReg2* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
        });        
    } else {
        result = std::shared_ptr<DynReg2>(new DynReg2(findTmpReg(), reg, isHigh), [this](DynReg2* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
            });
        if (regCache[reg]) {
            x86.mov(X86Asm::Reg32(result->hardwareReg()), X86Asm::Reg32(regCache[reg]));
        } else {
            x86.readMem(X86Asm::Reg32(result->hardwareReg()), x86.edi, CPU::offsetofReg32(reg));
        }
    }
    return result;
}

RegPtr X86DynamicCodeGen::getSegAddress(U8 reg) {
    RegPtr result = getTmpReg();
    x86.readMem(X86Asm::Reg32(result->hardwareReg()), x86.edi, CPU::offsetofSegAddress(reg));
    return result;
}

RegPtr X86DynamicCodeGen::getReg(U8 reg) {
    if (regCache[reg]) {
        return std::shared_ptr<DynReg2>(new DynReg2(regCache[reg], reg), [this](DynReg2* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
        });
    } else {        
        RegPtr result = std::shared_ptr<DynReg2>(new DynReg2(findTmpReg(), reg), [this](DynReg2* p) {
            x86.writeMem(x86.edi, CPU::offsetofReg32(p->emulatedReg), X86Asm::Reg32(p->hardwareReg()));
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
            });
        x86.readMem(X86Asm::Reg32(result->hardwareReg()), x86.edi, CPU::offsetofReg32(reg));
        return result;
    }
}

RegPtr X86DynamicCodeGen::getReg8(U8 reg) {
    bool isHigh = reg > 3;
    if (isHigh) {
        reg -= 4;
    }
    // for 8-bit, only the first 4 registers in x86 can use them
    RegPtr result = std::shared_ptr<DynReg2>(new DynReg2(findTmpReg(), reg, isHigh), [this](DynReg2* p) {
        if (p->isLoaded()) {
            if (regCache[p->emulatedReg]) {
                x86.mov(X86Asm::Reg32(regCache[p->emulatedReg]), X86Asm::Reg32(p->hardwareReg()));
            } else {
                x86.writeMem(x86.edi, CPU::offsetofReg32(p->emulatedReg), X86Asm::Reg32(p->hardwareReg()));
            }
            regUsed2[p->hardwareReg()] = false;
        }
        delete p;
        });
    if (regCache[reg]) {
        x86.mov(X86Asm::Reg32(result->hardwareReg()), X86Asm::Reg32(regCache[reg]));
    } else {
        x86.readMem(X86Asm::Reg32(result->hardwareReg()), x86.edi, CPU::offsetofReg32(reg));
    }
    return result;
}

RegPtr X86DynamicCodeGen::getReadOnlyReg(U8 reg, bool delayed) {
    if (regCache[reg]) {
        return std::shared_ptr<DynReg2>(new DynReg2(regCache[reg], reg), [this](DynReg2* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
            });
    } else {
        return getTmpReg(reg, delayed);
    }
}

RegPtr X86DynamicCodeGen::getReadOnlyReg8(U8 reg, bool delayed) {
    return getTmpReg8(reg, delayed);
}

static void dynamic_fillFlags(CPU * cpu) {
    cpu->fillFlags();
    cpu->dynamicFlags = ((cpu->flags & (CF | PF | AF | ZF | SF)) << 8) | ((cpu->flags & OF) >> 11);
}

void X86DynamicCodeGen::updateHardwareFlags(U32 flags) {
    call(::dynamic_fillFlags);
    RegPtr reg = getTmpRegForCallResult();
    x86.readMem(X86Asm::Reg32(reg->hardwareReg()), x86.edi, offsetof(CPU, dynamicFlags));
    if (reg->hardwareReg() == 0) {
        if (flags & OF) {
            x86.add(x86.al, 127); // (will restore OF)
        }
        x86.sahf();
    } else {
        x86.xchg(X86Asm::Reg32(reg->hardwareReg()), x86.eax);
        if (flags & OF) {
            x86.add(x86.al, 127); // (will restore OF)
        }
        x86.sahf();
        x86.xchg(X86Asm::Reg32(reg->hardwareReg()), x86.eax);
    }
}

void X86DynamicCodeGen::updateFlagsIfNecessary() {
    U32 neededFlags = currentOp->needsToSetFlags(cpu);
    if (neededFlags) {
        bool savedEAX = false;

        if (regUsed2[0]) {
            x86.push(x86.eax);
            savedEAX = true;
        }
        if (neededFlags & OF) {
            x86.lahf();
            x86.seto(X86Asm::Reg8(0));
            x86.shl(X86Asm::Reg8(0), 3);
            x86.xchg(X86Asm::Reg8(4), X86Asm::Reg8(0));
        } else {
            x86.lahf();
            x86.shr(X86Asm::Reg16(0), 8);
        }
        // mask so we don't clobber DF
        x86.andMem32(x86.edi, offsetof(CPU, flags), ~FMASK_TEST);
        x86.orMemReg(x86.ax, x86.edi, offsetof(CPU, flags));
        this->storeLazyFlags(FLAGS_NONE);
        this->currentLazyFlags = FLAGS_NONE;
        if (savedEAX) {
            x86.pop(x86.eax);
        }
    }
}

U8 get8bitReg(RegPtr reg) {
    if (reg->isHigh) {
        return reg->hardwareReg() + 4;
    }
    return reg->hardwareReg();
}

void X86DynamicCodeGen::IfLessThan2(DynWidth regWidth, RegPtr reg, U32 value) {
    if (regWidth == DYN_32bit) {
        x86.IfLessThan(X86Asm::Reg32(reg->hardwareReg()), value);
    } else if (regWidth == DYN_16bit) {
        x86.IfLessThan(X86Asm::Reg16(reg->hardwareReg()), (U16)value);
    } else if (regWidth == DYN_8bit) {
        x86.IfLessThan(X86Asm::Reg8(get8bitReg(reg)), (U8)value);
    } else {
        kpanic_fmt("X86DynamicCodeGen::IfNot unexpected width: %d", (U32)regWidth);
    }
}

void X86DynamicCodeGen::IfNot(DynWidth regWidth, RegPtr reg) {
    if (regWidth == DYN_32bit) {
        x86.IfZero(X86Asm::Reg32(reg->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.IfZero(X86Asm::Reg16(reg->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.IfZero(X86Asm::Reg8(get8bitReg(reg)));
    } else {
        kpanic_fmt("X86DynamicCodeGen::IfNot unexpected width: %d", (U32)regWidth);
    }
}

void X86DynamicCodeGen::If(DynWidth regWidth, RegPtr reg) {
    if (regWidth == DYN_32bit) {
        x86.IfNotZero(X86Asm::Reg32(reg->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.IfNotZero(X86Asm::Reg16(reg->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.IfNotZero(X86Asm::Reg8(get8bitReg(reg)));
    } else {
        kpanic_fmt("X86DynamicCodeGen::If unexpected width: %d", (U32)regWidth);
    }
}

void X86DynamicCodeGen::IfEqual(DynWidth regWidth, RegPtr reg, U32 value) {
    if (regWidth == DYN_32bit) {
        x86.IfEqual(X86Asm::Reg32(reg->hardwareReg()), value);
    } else if (regWidth == DYN_16bit) {
        x86.IfEqual(X86Asm::Reg16(reg->hardwareReg()), value);
    } else if (regWidth == DYN_8bit) {
        x86.IfEqual(X86Asm::Reg8(get8bitReg(reg)), value);
    } else {
        kpanic_fmt("X86DynamicCodeGen::IfEqual unexpected width: %d", (U32)regWidth);
    }
}

void X86DynamicCodeGen::IfBitSet2(DynWidth regWidth, RegPtr reg, U32 value, bool bigJump) {
    if (regWidth == DYN_8bit) {
        x86.IfBitSet(X86Asm::Reg32(get8bitReg(reg)), value, bigJump);
    } else {
        x86.IfBitSet(X86Asm::Reg32(reg->hardwareReg()), value, bigJump);
    }
}

void X86DynamicCodeGen::mov(DynWidth regWidth, RegPtr dest, RegPtr src) {
    if (regWidth == DYN_32bit) {
        x86.mov(X86Asm::Reg32(dest->hardwareReg()), X86Asm::Reg32(src->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.mov(X86Asm::Reg16(dest->hardwareReg()), X86Asm::Reg16(src->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.mov(X86Asm::Reg8(get8bitReg(dest)), X86Asm::Reg8(get8bitReg(src)));
    } else {
        kpanic_fmt("X86DynamicCodeGen::mov unexpected width: %d", (U32)regWidth);
    }
}

void X86DynamicCodeGen::movValue(DynWidth regWidth, RegPtr dst, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.mov(X86Asm::Reg32(dst->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.mov(X86Asm::Reg16(dst->hardwareReg()), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.mov(X86Asm::Reg8(get8bitReg(dst)), (U8)imm);
    } else {
        kpanic_fmt("X86DynamicCodeGen::mov unexpected width: %d", (U32)regWidth);
    }
}

void X86DynamicCodeGen::pushParam(const DynParam& param) {
    switch (param.type) {
    case DYN_PARAM_REG_8:
        x86.movzx(X86Asm::Reg32(param.reg->hardwareReg()), X86Asm::Reg8(get8bitReg(param.reg)));
        x86.push(X86Asm::Reg32(param.reg->hardwareReg()));
        break;
    case DYN_PARAM_REG_16:
        x86.movzx(X86Asm::Reg32(param.reg->hardwareReg()), X86Asm::Reg16(param.reg->hardwareReg()));
        x86.push(X86Asm::Reg32(param.reg->hardwareReg()));
        break;
    case DYN_PARAM_REG_32:
        x86.push(X86Asm::Reg32(param.reg->hardwareReg()));
        break;
    case DYN_PARAM_CPU:
        x86.push(x86.edi);
        break;
    case DYN_PARAM_CONST_8:
        x86.push((U32)(param.value & 0xFF));
        break;
    case DYN_PARAM_CONST_16:
        x86.push((U32)(param.value & 0xFFFF));
        break;
    case DYN_PARAM_CONST_32:
        x86.push(param.value);
        break;
    case DYN_PARAM_CONST_PTR:
        x86.push((U32)param.value);
        break;
    default:
        kpanic_fmt("x32CPU: unknown argType: %d", param.type);
        break;
    }
}

void X86DynamicCodeGen::callHostFunction(void* address, const std::vector<DynParam>& params) {
    writeCache();    

    if (regUsed2[x86.eax.reg]) {
        x86.push(x86.eax);
    }
    if (regUsed2[x86.ecx.reg]) {
        x86.push(x86.ecx);
    }
    if (regUsed2[x86.edx.reg]) {
        x86.push(x86.edx);
    }
    for (int i = params.size() - 1; i >= 0; i--) {
        pushParam(params[i]);
    }

    x86.call(address);

    if (params.size()) {
        x86.add(x86.esp, 4 * params.size());
    }
    if (regUsed2[x86.edx.reg]) {
        x86.pop(x86.edx);
    }
    if (regUsed2[x86.ecx.reg]) {
        x86.pop(x86.ecx);
    }
    if (regUsed2[x86.eax.reg]) {
        x86.pop(x86.eax);
    }

    loadCache();
}

void X86DynamicCodeGen::callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) {
    writeCache();

    if (regUsed2[0] && result->hardwareReg() != 0) {
        kpanic("X86DynamicCodeGen::callHostFunctionWithResult");
    }
    bool pushedEAX = false;
    bool pushedECX = false;
    bool pushedEDX = false;

    if (regUsed2[x86.eax.reg] && (result->isLoaded() && result->hardwareReg() != 0)) {
        x86.push(x86.eax);
        pushedEAX = true;
    }
    if (regUsed2[x86.ecx.reg] && (result->isLoaded() && result->hardwareReg() != 1)) {
        x86.push(x86.ecx);
        pushedECX = true;
    }
    if (regUsed2[x86.edx.reg] && (result->isLoaded() && result->hardwareReg() != 2)) {
        x86.push(x86.edx);
        pushedEDX = true;
    }
    for (int i = params.size() - 1; i >= 0; i--) {
        pushParam(params[i]);
    }

    x86.call(address);

    if (params.size()) {
        x86.add(x86.esp, 4 * params.size());
    }
    if (pushedEDX) {
        x86.pop(x86.edx);
    }
    if (pushedECX) {
        x86.pop(x86.ecx);
    }
    if (result->hardwareReg() != 0) {
        x86.mov(X86Asm::Reg32(result->hardwareReg()), x86.eax);
    }
    if (pushedEAX) {
        x86.pop(x86.eax);
    }
    loadCache();
}

void X86DynamicCodeGen::read(DynWidth width, RegPtr dest, RegPtr reg, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(dest->hardwareReg()), X86Asm::Reg32(reg->hardwareReg()), lsl, disp);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(dest->hardwareReg()), X86Asm::Reg32(reg->hardwareReg()), lsl, disp);
    } else if (width == DYN_8bit) {
        x86.readMem(X86Asm::Reg8(get8bitReg(dest)), X86Asm::Reg32(reg->hardwareReg()), lsl, disp);
    } else {
        kpanic_fmt("X86DynamicCodeGen::readMem unexpected width: %d", (U32)width);
    }
}

void X86DynamicCodeGen::read(DynWidth width, RegPtr dest, RegPtr reg, RegPtr sib, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(dest->hardwareReg()), X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(dest->hardwareReg()), X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
    } else if (width == DYN_8bit) {
        x86.readMem(X86Asm::Reg8(get8bitReg(dest)), X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
    } else {
        kpanic_fmt("X86DynamicCodeGen::readMem unexpected width: %d", (U32)width);
    }
}

void X86DynamicCodeGen::write(DynWidth width, RegPtr reg, U32 disp, RegPtr src) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(reg->hardwareReg()), disp, X86Asm::Reg32(src->hardwareReg()));
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(reg->hardwareReg()), disp, X86Asm::Reg16(src->hardwareReg()));
    } else if (width == DYN_8bit) {
        x86.writeMem(X86Asm::Reg32(reg->hardwareReg()), disp, X86Asm::Reg8(get8bitReg(src)));
    } else {
        kpanic_fmt("X86DynamicCodeGen::write unexpected width: %d", (U32)width);
    }
}

void X86DynamicCodeGen::write(DynWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, RegPtr src) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, X86Asm::Reg32(src->hardwareReg()));
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, X86Asm::Reg16(src->hardwareReg()));
    } else if (width == DYN_8bit) {
        x86.writeMem(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, X86Asm::Reg8(get8bitReg(src)));
    } else {
        kpanic_fmt("X86DynamicCodeGen::writeMem unexpected width: %d", (U32)width);
    }
}

void X86DynamicCodeGen::addReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.add(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.add(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.add(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::addReg");
    }
    if (checkFlags) {
        rm = nullptr; // if there is only one reference then letting go of it could help updateFlagsIfNecessary
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::addValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.add(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.add(X86Asm::Reg16(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_8bit) {
        x86.add(X86Asm::Reg8(get8bitReg(reg)), imm);
    } else {
        kpanic("X86DynamicCodeGen::addValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::adcReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    updateHardwareFlags(CF);
    if (regWidth == DYN_32bit) {
        x86.adc(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.adc(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.adc(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::addReg");
    }
    if (checkFlags) {
        rm = nullptr; // if there is only one reference then letting go of it could help updateFlagsIfNecessary
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::adcValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    updateHardwareFlags(CF);
    if (regWidth == DYN_32bit) {
        x86.adc(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.adc(X86Asm::Reg16(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_8bit) {
        x86.adc(X86Asm::Reg8(get8bitReg(reg)), imm);
    } else {
        kpanic("X86DynamicCodeGen::addValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::orReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.or_(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.or_(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.or_(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::orReg");
    }
    if (checkFlags) {
        rm = nullptr; // if there is only one reference then letting go of it could help updateFlagsIfNecessary
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::orValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.or_(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.or_(X86Asm::Reg16(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_8bit) {
        x86.or_(X86Asm::Reg8(get8bitReg(reg)), imm);
    } else {
        kpanic("X86DynamicCodeGen::orValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::subReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.sub(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.sub(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.sub(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::subReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::subValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.sub(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.sub(X86Asm::Reg16(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_8bit) {
        x86.sub(X86Asm::Reg8(get8bitReg(reg)), imm);
    } else {
        kpanic("X86DynamicCodeGen::subValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::sbbReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    updateHardwareFlags(CF);
    if (regWidth == DYN_32bit) {
        x86.sbb(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.sbb(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.sbb(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::subReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::sbbValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    updateHardwareFlags(CF);
    if (regWidth == DYN_32bit) {
        x86.sbb(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.sbb(X86Asm::Reg16(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_8bit) {
        x86.sbb(X86Asm::Reg8(get8bitReg(reg)), imm);
    } else {
        kpanic("X86DynamicCodeGen::subValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::andReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.and_(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.and_(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.and_(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::andReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::andValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.and_(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.and_(X86Asm::Reg16(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_8bit) {
        x86.and_(X86Asm::Reg8(get8bitReg(reg)), imm);
    } else {
        kpanic("X86DynamicCodeGen::andValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::xorReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.xor_(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.xor_(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.xor_(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::xorReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::xorValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.xor_(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.xor_(X86Asm::Reg16(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_8bit) {
        x86.xor_(X86Asm::Reg8(get8bitReg(reg)), imm);
    } else {
        kpanic("X86DynamicCodeGen::xorValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::cmpReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.cmp(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.cmp(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.cmp(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::cmpReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::cmpValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.cmp(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.cmp(X86Asm::Reg16(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_8bit) {
        x86.cmp(X86Asm::Reg8(get8bitReg(reg)), imm);
    } else {
        kpanic("X86DynamicCodeGen::cmpValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::testReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.test(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.test(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.test(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::testReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::testValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.test(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.test(X86Asm::Reg16(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_8bit) {
        x86.test(X86Asm::Reg8(get8bitReg(reg)), imm);
    } else {
        kpanic("X86DynamicCodeGen::testValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::shrReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.shr(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.shr(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.shr(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::shrReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::shrValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.shr(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.shr(X86Asm::Reg16(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_8bit) {
        x86.shr(X86Asm::Reg8(get8bitReg(reg)), imm);
    } else {
        kpanic("X86DynamicCodeGen::shrValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::shlReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.shl(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.shl(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.shl(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::shlReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::shlValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.shl(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.shl(X86Asm::Reg16(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_8bit) {
        x86.shl(X86Asm::Reg8(get8bitReg(reg)), imm);
    } else {
        kpanic("X86DynamicCodeGen::shlValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::sarReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.sar(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.sar(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.sar(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::sarReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::sarValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.sar(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.sar(X86Asm::Reg16(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_8bit) {
        x86.sar(X86Asm::Reg8(get8bitReg(reg)), imm);
    } else {
        kpanic("X86DynamicCodeGen::sarValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::storeCpuXMMReg(DynXMMReg reg, U32 index) {
    x86.movaps(x86.edi, index * 16 + offsetof(CPU, xmm), X86Asm::XMM(reg));
}

void X86DynamicCodeGen::storeXMMToMem128(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movups(X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp, X86Asm::XMM(reg));
}

void X86DynamicCodeGen::storeXMMToMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movlps(X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp, X86Asm::XMM(reg));
}

void X86DynamicCodeGen::storeXMMToMem32(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movss(X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp, X86Asm::XMM(reg));
}

void X86DynamicCodeGen::storeHighXMMToMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movhps(X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp, X86Asm::XMM(reg));
}

void X86DynamicCodeGen::loadCpuXMMReg(DynXMMReg reg, U32 index) {
    x86.movaps(X86Asm::XMM(reg), x86.edi, index * 16 + offsetof(CPU, xmm));
}

void X86DynamicCodeGen::loadCpuXMMReg64ZeroExtend(DynXMMReg reg, U32 index) {
    x86.movq(X86Asm::XMM(reg), x86.edi, index * 16 + offsetof(CPU, xmm));
}

void X86DynamicCodeGen::loadXMMFromMem128(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movups(X86Asm::XMM(reg), X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp);
}

void X86DynamicCodeGen::loadXMMFromMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movq(X86Asm::XMM(reg), X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp);
}

void X86DynamicCodeGen::loadLowXMMFromMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movlps(X86Asm::XMM(reg), X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp);
}

void X86DynamicCodeGen::loadHighXMMFromMem64(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movhps(X86Asm::XMM(reg), X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp);
}

void X86DynamicCodeGen::loadXMMFromMem32(DynXMMReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movss(X86Asm::XMM(reg), X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp);
}

void X86DynamicCodeGen::addpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.addps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::addssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.addss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::subpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.subps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::subssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.subss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::mulpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.mulps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::mulssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.mulss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::divpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.divps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::divssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.divss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::rcppsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.rcpps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::rcpssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.rcpss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::sqrtpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.sqrtps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::sqrtssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.sqrtss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::rsqrtpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.rsqrtps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::rsqrtssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.rsqrtss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::maxpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.maxps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::maxssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.maxss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::minpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.minps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::minssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.minss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pavgbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pavgb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pavgwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pavgw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psadbwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psadbw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pextrwRegMmx(DynReg dst, DynMMXReg src, U8 srcIndex) {
    x86.pextrw(X86Asm::Reg32(dst), X86Asm::XMM(src), srcIndex);
}

void X86DynamicCodeGen::pinsrwMmxReg(DynMMXReg dst, DynReg src, U8 dstIndex) {
    x86.pinsrw(X86Asm::XMM(dst), X86Asm::Reg32(src), dstIndex);
}

void X86DynamicCodeGen::pmaxswMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmaxsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmaxubMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmaxub(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pminswMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pminsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pminubMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pminub(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmovmskbMmxMmx(DynReg dst, DynMMXReg src) {
    x86.pmovmskb(X86Asm::Reg32(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmulhuwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmulhuw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pshufwMmxMmx(DynMMXReg dst, DynMMXReg src, U8 order) {
    x86.pshuflw(X86Asm::XMM(dst), X86Asm::XMM(src), order);
}

void X86DynamicCodeGen::maskmovq(DynMMXReg src, DynMMXReg mask, DynReg destAddress) {
    x86.push(x86.edi);
    x86.mov(x86.edi, X86Asm::Reg32(destAddress));
    // this works because the top 64-bits of the mask should be 0's since its used for MMX
    x86.maskmovdqu(X86Asm::XMM(src), X86Asm::XMM(mask));
    x86.pop(x86.edi);
}

void X86DynamicCodeGen::paddqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmuludqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmuludq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::andnpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.andnps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::andpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.andps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::orpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.orps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::xorpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.xorps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtpi2psXmmMmx(DynXMMReg dst, DynMMXReg src) {
    // cvtpi2ps need to keep top 64-bits of the xmm dst
    DynXMMReg tmp = getTmpXMM(dst);
    x86.cvtdq2ps(X86Asm::XMM(tmp), X86Asm::XMM(src));
    x86.movsd(X86Asm::XMM(dst), X86Asm::XMM(tmp));
}

void X86DynamicCodeGen::cvtps2piMmxXmm(DynMMXReg dst, DynXMMReg src) {
    x86.cvtps2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtsi2ssXmmR32(DynXMMReg dst, DynReg src) {
    x86.cvtsi2ss(X86Asm::XMM(dst), X86Asm::Reg32(src));
}

void X86DynamicCodeGen::cvtss2siR32Xmm(DynReg dst, DynXMMReg src) {
    x86.cvtss2si(X86Asm::Reg32(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvttps2piMmxXmm(DynMMXReg dst, DynXMMReg src) {
    x86.cvttps2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvttss2siR32Xmm(DynReg dst, DynXMMReg src) {
    x86.cvttss2si(X86Asm::Reg32(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movhlpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.movhlps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movlhpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.movss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::shufpsXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.shufps(X86Asm::XMM(dst), X86Asm::XMM(src), (U8)imm);
}

void X86DynamicCodeGen::cmppsXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.cmpps(X86Asm::XMM(dst), X86Asm::XMM(src), (U8)imm);
}

void X86DynamicCodeGen::cmpssXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.cmpss(X86Asm::XMM(dst), X86Asm::XMM(src), (U8)imm);
}

void X86DynamicCodeGen::loadHardwareFlags() {
    x86.pushFlags();
    x86.pop(DYN_SRC);
    x86.and_(X86Asm::Reg32(DYN_SRC), FMASK_TEST);

    //this->andCPUFlagsImm(~FMASK_TEST, tmpReg2);
    movToRegFromCpu(DYN_DEST, offsetof(CPU, flags), DYN_32bit);
    andRegImm(DYN_DEST, DYN_32bit, ~FMASK_TEST);

    //this->orCPUFlagsReg(tmpReg, tmpReg2, true);
    orRegReg(DYN_SRC, DYN_DEST, DYN_32bit, true);
    movToCpuFromReg(offsetof(CPU, flags), DYN_SRC, DYN_32bit, true);
}

void X86DynamicCodeGen::comissXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.comiss(X86Asm::XMM(dst), X86Asm::XMM(src));
    loadHardwareFlags();
}

void X86DynamicCodeGen::ucomissXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.ucomiss(X86Asm::XMM(dst), X86Asm::XMM(src));
    loadHardwareFlags();
}

void X86DynamicCodeGen::stmxcsr(DynReg address) {
    x86.stmxcsr(X86Asm::Reg32(address), 0);
}

void X86DynamicCodeGen::ldmxcsr(DynReg address) {
    x86.ldmxcsr(X86Asm::Reg32(address), 0);
}

void X86DynamicCodeGen::sfence() {
    x86.sfence();
}

void X86DynamicCodeGen::unpckhpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.unpckhps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::unpcklpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.unpcklps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movmskpsR32Xmm(DynReg dst, DynXMMReg src) {
    x86.movmskps(X86Asm::Reg32(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::loadMMXFromReg(DynMMXReg dst, DynReg src) {
    x86.movd(X86Asm::XMM(dst), X86Asm::Reg32(src));
}

void X86DynamicCodeGen::storeCpuMMXReg(DynMMXReg reg, U32 index) {
    x86.movq(x86.edi, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif), X86Asm::XMM(reg));
}

void X86DynamicCodeGen::storeMMXToReg(DynMMXReg src, DynReg dst) {
    x86.movd(X86Asm::Reg32(dst), X86Asm::XMM(src));
    regUsed[dst] = true;
}

void X86DynamicCodeGen::loadCpuMMXReg(DynMMXReg reg, U32 index) {
    x86.movq(X86Asm::XMM(reg), x86.edi, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif));
}

void X86DynamicCodeGen::loadMMXFromMem32(DynMMXReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movd(X86Asm::XMM(reg), X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp);
}

void X86DynamicCodeGen::loadMMXFromMem64(DynMMXReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movq(X86Asm::XMM(reg), X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp);
}

void X86DynamicCodeGen::storeMMXToMem32(DynMMXReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movd(X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp, X86Asm::XMM(reg));
}

void X86DynamicCodeGen::storeMMXToMem64(DynMMXReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.movq(X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp, X86Asm::XMM(reg));
}

void X86DynamicCodeGen::xorMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pxor(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::orMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.por(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::andMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pand(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::andnMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pandn(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psllwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psllw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psrlwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psrlw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psrawMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psraw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psllwMmx(DynMMXReg dst, U32 imm) {
    x86.psllw(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psrlwMmx(DynMMXReg dst, U32 imm) {
    x86.psrlw(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psrawMmx(DynMMXReg dst, U32 imm) {
    x86.psraw(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::pslldMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pslld(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psrldMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psrld(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psradMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psrad(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pslldMmx(DynMMXReg dst, U32 imm) {
    x86.pslld(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psrldMmx(DynMMXReg dst, U32 imm) {
    x86.psrld(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psradMmx(DynMMXReg dst, U32 imm) {
    x86.psrad(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psllqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psllq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psrlqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psrlq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psllqMmx(DynMMXReg dst, U32 imm) {
    x86.psllq(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psrlqMmx(DynMMXReg dst, U32 imm) {
    x86.psrlq(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::paddbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::paddwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::padddMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::paddsbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddsb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::paddswMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::paddusbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddusb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::padduswMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddusw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubdMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubsbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubsb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubswMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubusbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubusb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubuswMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubusw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmulhwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmulhw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmullwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmullw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmaddwdMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmaddwd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pcmpeqbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pcmpeqb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pcmpeqwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pcmpeqw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pcmpeqdMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pcmpeqd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pcmpgtbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pcmpgtb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pcmpgtwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pcmpgtw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pcmpgtdMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pcmpgtd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::packsswbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    // xmm is twice as big as mm, so we need to adjust a little before the pack
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(src));
    x86.packsswb(X86Asm::XMM(dst), X86Asm::XMM(dst));
}

void X86DynamicCodeGen::packssdwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    // xmm is twice as big as mm, so we need to adjust a little before the pack
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(src));
    x86.packssdw(X86Asm::XMM(dst), X86Asm::XMM(dst));
}

void X86DynamicCodeGen::packuswbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    // xmm is twice as big as mm, so we need to adjust a little before the pack
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(src));
    x86.packuswb(X86Asm::XMM(dst), X86Asm::XMM(dst));
}

void X86DynamicCodeGen::punpckhbwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    // :TODO: maybe move bytes 4-7 to 8-11 instead of 0-7 to 8-15 so that we don't have to do the movhlps to mov them back down?
    x86.movlhps(X86Asm::XMM(src), X86Asm::XMM(src));
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(dst));
    x86.punpckhbw(X86Asm::XMM(dst), X86Asm::XMM(src));
    x86.movhlps(X86Asm::XMM(dst), X86Asm::XMM(dst));
}

void X86DynamicCodeGen::punpckhwdMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.movlhps(X86Asm::XMM(src), X86Asm::XMM(src));
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(dst));
    x86.punpckhwd(X86Asm::XMM(dst), X86Asm::XMM(src));
    x86.movhlps(X86Asm::XMM(dst), X86Asm::XMM(dst));
}

void X86DynamicCodeGen::punpckhdqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.movlhps(X86Asm::XMM(src), X86Asm::XMM(src));
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(dst));
    x86.punpckhdq(X86Asm::XMM(dst), X86Asm::XMM(src));
    x86.movhlps(X86Asm::XMM(dst), X86Asm::XMM(dst));
}

void X86DynamicCodeGen::punpcklbwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.punpcklbw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::punpcklwdMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.punpcklwd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::punpckldqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.punpckldq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::addpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.addpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::addsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.addsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::subpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.subpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::subsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.subsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::mulpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.mulpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::mulsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.mulsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::divpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.divpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::divsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.divsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::maxpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.maxpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::maxsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.maxsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::minpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.minpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::minsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.minsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::paddbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::paddwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::padddXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::paddqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::paddsbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddsb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::paddswXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::paddusbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddusb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::padduswXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddusw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubsbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubsb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubswXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubusbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubusb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psubuswXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubusw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmaddwdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmaddwd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmulhwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmulhw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmullwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmullw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmuludqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmuludq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::sqrtpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.sqrtpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::sqrtsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.sqrtsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::andnpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.andnpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::andpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.andpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pandXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pand(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pandnXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pandn(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::porXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.por(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pslldqXmm(DynXMMReg dst, U32 imm) {
    x86.pslldq(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psllqXmm(DynXMMReg dst, U32 imm) {
    x86.psllq(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::pslldXmm(DynXMMReg dst, U32 imm) {
    x86.pslld(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psllwXmm(DynXMMReg dst, U32 imm) {
    x86.psllw(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psradXmm(DynXMMReg dst, U32 imm) {
    x86.psrad(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psrawXmm(DynXMMReg dst, U32 imm) {
    x86.psraw(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psrldqXmm(DynXMMReg dst, U32 imm) {
    x86.psrldq(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psrlqXmm(DynXMMReg dst, U32 imm) {
    x86.psrlq(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psrldXmm(DynXMMReg dst, U32 imm) {
    x86.psrld(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psrlwXmm(DynXMMReg dst, U32 imm) {
    x86.psrlw(X86Asm::XMM(dst), imm);
}

void X86DynamicCodeGen::psllqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psllq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pslldXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pslld(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psllwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psllw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psradXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psrad(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psrawXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psraw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psrlqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psrlq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psrldXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psrld(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psrlwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psrlw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pxorXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pxor(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::orpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.orpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::xorpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.xorpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cmppdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.cmppd(X86Asm::XMM(dst), X86Asm::XMM(src), (U8)imm);
}

void X86DynamicCodeGen::cmpsdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.cmpsd(X86Asm::XMM(dst), X86Asm::XMM(src), (U8)imm);
}

void X86DynamicCodeGen::comisdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.comisd(X86Asm::XMM(dst), X86Asm::XMM(src));
    loadHardwareFlags();
}

void X86DynamicCodeGen::ucomisdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.ucomisd(X86Asm::XMM(dst), X86Asm::XMM(src));
    loadHardwareFlags();
}

void X86DynamicCodeGen::pcmpgtbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pcmpgtb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pcmpgtwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pcmpgtw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pcmpgtdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pcmpgtd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pcmpeqbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pcmpeqb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pcmpeqwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pcmpeqw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pcmpeqdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pcmpeqd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtdq2pdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtdq2pd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtdq2psXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtdq2ps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtpd2piMmxXmm(DynMMXReg dst, DynXMMReg src) {
    x86.cvtpd2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtpi2pdXmmMmx(DynXMMReg dst, DynMMXReg src) {
    x86.cvtdq2pd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtpd2dqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtpd2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtpd2psXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtpd2ps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvttpd2piMmxXmm(DynMMXReg dst, DynXMMReg src) {
    x86.cvttpd2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtps2dqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtps2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtps2pdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtps2pd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtsd2siR32Xmm(DynReg dst, DynXMMReg src) {
    x86.cvtsd2si(X86Asm::Reg32(dst), X86Asm::XMM(src));
    regUsed[dst] = true;
}

void X86DynamicCodeGen::cvtsd2ssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtsd2ss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtsi2sdXmmR32(DynXMMReg dst, DynReg src) {
    x86.cvtsi2sd(X86Asm::XMM(dst), X86Asm::Reg32(src));
}

void X86DynamicCodeGen::cvtss2sdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtss2sd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvttpd2dqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvttpd2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvttps2dqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvttps2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvttsd2siR32Xmm(DynReg dst, DynXMMReg src) {
    x86.cvttsd2si(X86Asm::Reg32(dst), X86Asm::XMM(src));
    regUsed[dst] = true;
}

void X86DynamicCodeGen::movsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.movsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movupdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.movdqu(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movmskpd(DynReg dst, DynXMMReg src) {
    x86.movmskpd(X86Asm::Reg32(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movd(DynReg dst, DynXMMReg src) {
    x86.movd(X86Asm::Reg32(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movd(DynXMMReg dst, DynReg src) {
    x86.movd(X86Asm::XMM(dst), X86Asm::Reg32(src));
}

void X86DynamicCodeGen::movdq2q(DynMMXReg dst, DynXMMReg src) {
    x86.movq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movq2dq(DynXMMReg dst, DynMMXReg src) {
    x86.movq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::maskmovdqu(DynXMMReg src, DynXMMReg mask, DynReg address) {
    x86.push(x86.edi);
    x86.mov(x86.edi, X86Asm::Reg32(address));
    x86.maskmovdqu(X86Asm::XMM(src), X86Asm::XMM(mask));
    x86.pop(x86.edi);
}

void X86DynamicCodeGen::pshufdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.pshufd(X86Asm::XMM(dst), X86Asm::XMM(src), imm);
}

void X86DynamicCodeGen::pshufhwXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.pshufhw(X86Asm::XMM(dst), X86Asm::XMM(src), imm);
}

void X86DynamicCodeGen::pshuflwXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.pshuflw(X86Asm::XMM(dst), X86Asm::XMM(src), imm);
}

void X86DynamicCodeGen::shufpdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.shufpd(X86Asm::XMM(dst), X86Asm::XMM(src), imm);
}

void X86DynamicCodeGen::unpckhpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.unpckhpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::unpcklpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.unpcklpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::punpckhbwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpckhbw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::punpckhwdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpckhwd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::punpckhdqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpckhdq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::punpckhqdqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpckhqdq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::punpcklbwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpcklbw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::punpcklwdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpcklwd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::punpckldqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpckldq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::punpcklqdqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpcklqdq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::packssdwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.packssdw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::packsswbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.packsswb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::packuswbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.packuswb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pavgbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pavgb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pavgwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pavgw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::psadbwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psadbw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmaxswXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmaxsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmaxubXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmaxub(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pminswXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pminsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pminubXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pminub(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmulhuwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmulhuw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::lfence() {
    x86.lfence();
}

void X86DynamicCodeGen::mfence() {
    x86.mfence();
}

void X86DynamicCodeGen::clflush(DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.clflush(X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp);
}

void X86DynamicCodeGen::pause() {
    x86.pause();
}

void X86DynamicCodeGen::pextrwR32Xmm(DynReg dst, DynXMMReg src, U32 imm) {
    x86.pextrw(X86Asm::Reg32(dst), X86Asm::XMM(src), (U8)imm);
}

void X86DynamicCodeGen::pinsrwXmmR32(DynXMMReg dst, DynReg src, U32 imm) {
    x86.pinsrw(X86Asm::XMM(dst), X86Asm::Reg32(src), (U8)imm);
}

void X86DynamicCodeGen::pmovmskbR32Xmm(DynReg dst, DynXMMReg src) {
    x86.pmovmskb(X86Asm::Reg32(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::updateFPURounding(DynReg tmp1, DynReg tmp2) {
    x86.stmxcsr(x86.edi, offsetof(CPU, sseControlStateTmp));

    const DynReg SSE_REG = tmp1;
    const DynReg FPU_REG = tmp2;

    movToRegFromCpu(SSE_REG, offsetof(CPU, sseControlStateTmp), DYN_32bit);
    movToRegFromCpu(FPU_REG, offsetof(CPU, fpu.round), DYN_32bit);

    andRegImm(SSE_REG, DYN_32bit, ~0x6000); // clear rounding
    shlRegImm(FPU_REG, DYN_32bit, 13);
    orRegReg(SSE_REG, FPU_REG, DYN_32bit, true); // set rounding in SSE

    // there is no way to set sse rounding from a register
    movToCpuFromReg(offsetof(CPU, sseControlStateTmp2), SSE_REG, DYN_32bit, true);

    x86.ldmxcsr(x86.edi, offsetof(CPU, sseControlStateTmp2));
}

void X86DynamicCodeGen::restoreFPURounding() {
    x86.ldmxcsr(x86.edi, offsetof(CPU, sseControlStateTmp));
};

void X86DynamicCodeGen::storeCpuFpuReg(DynFpuReg reg, DynReg index, DynFpuWidth width) {
    storeFpuReg(reg, (DynReg)7, index, 3, offsetof(CPU, fpu.regCache[0].d));
}

void X86DynamicCodeGen::loadCpuFpuReg(DynFpuReg reg, DynReg index, DynFpuWidth width) {
    loadFpuReg(reg, (DynReg)7, index, 3, offsetof(CPU, fpu.regCache[0].d));
}

void X86DynamicCodeGen::loadCpuFpuRegConst(DynFpuReg reg, U32 offset) {
    x86.movsd(X86Asm::XMM(reg), x86.edi, offset);
}

void X86DynamicCodeGen::fpuRegToInt32(DynReg regDst, DynFpuReg fpuRegSrc, bool truncate) {
    if (truncate) {
        x86.cvttsd2si(regDst, X86Asm::XMM(fpuRegSrc));
    } else {
        x86.cvtsd2si(regDst, X86Asm::XMM(fpuRegSrc));
    }
}

void X86DynamicCodeGen::fpuRegToInt64(DynFpuReg regDst, DynFpuReg fpuRegSrc, bool truncate) {
    if (truncate) {
        x86.cvttpd2dq(X86Asm::XMM(regDst), X86Asm::XMM(fpuRegSrc));
    } else {
        x86.cvtpd2dq(X86Asm::XMM(regDst), X86Asm::XMM(fpuRegSrc));
    }
}

void X86DynamicCodeGen::fpuRegInt64To64(DynFpuReg regDst, DynFpuReg fpuRegSrc) {
    x86.cvtdq2pd(X86Asm::XMM(regDst), X86Asm::XMM(fpuRegSrc));
}

void X86DynamicCodeGen::storeFpuReg(DynFpuReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        x86.movsd(X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp, X86Asm::XMM(reg));
    } else {
        x86.movss(X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp, X86Asm::XMM(reg));
    }
}

void X86DynamicCodeGen::loadFpuReg(DynFpuReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        x86.movsd(X86Asm::XMM(reg), X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp);
    } else {
        x86.movss(X86Asm::XMM(reg), X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp);
    }
}

void X86DynamicCodeGen::fpuRegExtend32To64(DynFpuReg dst, DynFpuReg src) {
    x86.cvtss2sd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::fpuReg64To32(DynFpuReg dst, DynFpuReg src) {
    x86.cvtsd2ss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::loadFpuRegFromInt(DynFpuReg reg, DynReg rm, DynReg sib, U8 lsl, U32 disp) {
    x86.cvtsi2sd(X86Asm::XMM(reg), X86Asm::Reg32(rm), X86Asm::Reg32(sib), lsl, disp);
}

void X86DynamicCodeGen::regToFpuReg(DynFpuReg dst, DynReg src) {
    x86.cvtsi2sd(X86Asm::XMM(dst), X86Asm::Reg32(src));
}

void X86DynamicCodeGen::fpuAdd(DynFpuReg dst, DynFpuReg src) {
    x86.addsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::fpuMul(DynFpuReg dst, DynFpuReg src) {
    x86.mulsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::fpuSub(DynFpuReg dst, DynFpuReg src) {
    x86.subsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::fpuDiv(DynFpuReg dst, DynFpuReg src) {
    x86.divsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::fpuXor(DynFpuReg dst, DynFpuReg src) {
    x86.xorpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::fpuAnd(DynFpuReg dst, DynFpuReg src) {
    x86.andpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::fpuSqrt(DynFpuReg dst, DynFpuReg src) {
    x86.sqrtsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::fcompare(DynFpuReg fpuReg1, DynFpuReg fpuReg2, DynReg ordTags, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) {
    subRegImm(ordTags, DYN_8bit, TAG_Empty);
    IfNot(ordTags, true);
        pfnInvalid();
    StartElse();
        x86.ucomisd(X86Asm::XMM(fpuReg2), X86Asm::XMM(fpuReg1));
        x86.IfPF();
            pfnInvalid();
        StartElse();
            x86.IfZF();
                pfnEqual();
            StartElse();
                x86.IfCF();
                    pfnLessThan();
                StartElse();
                    pfnGreaterThan();
                EndIf();
            EndIf();
        EndIf();
    EndIf();
}

void X86DynamicCodeGen::jmp(DynReg reg) {
    x86.jmp(X86Asm::Reg32(reg));
}

void X86DynamicCodeGen::readMem(DynReg reg, DynWidth width, DynReg address, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(reg), X86Asm::Reg32(address), lsl, disp);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(reg), X86Asm::Reg32(address), lsl, disp);
    } else if (width == DYN_8bit) {
        x86.readMem(X86Asm::Reg8(reg), X86Asm::Reg32(address), lsl, disp);
    } else {
        kpanic_fmt("X86DynamicCodeGen::readMem unexpected width: %d", (U32)width);
    }
}

void X86DynamicCodeGen::readMem(DynReg reg, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(reg), X86Asm::Reg32(address), offset, lsl, disp);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(reg), X86Asm::Reg32(address), offset, lsl, disp);
    } else if (width == DYN_8bit) {
        x86.readMem(X86Asm::Reg8(reg), X86Asm::Reg32(address), offset, lsl, disp);
    } else {
        kpanic_fmt("X86DynamicCodeGen::readMem unexpected width: %d", (U32)width);
    }
}

void X86DynamicCodeGen::writeMem(DynReg reg, DynWidth width, DynReg address, U32 disp) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, X86Asm::Reg32(reg));
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, X86Asm::Reg16(reg));
    } else if (width == DYN_8bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, X86Asm::Reg8(reg));
    } else {
        kpanic_fmt("X86DynamicCodeGen::writeMem unexpected width: %d", (U32)width);
    }
}

void X86DynamicCodeGen::writeMem(U32 value, DynWidth width, DynReg address, U32 disp) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, value);
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, (U16)value);
    } else if (width == DYN_8bit) {
        x86.writeMem(X86Asm::Reg32(address), disp, (U8)value);
    } else {
        kpanic_fmt("X86DynamicCodeGen::writeMem unexpected width: %d", (U32)width);
    }
}

void X86DynamicCodeGen::writeMem(DynReg reg, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, X86Asm::Reg32(reg));
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, X86Asm::Reg16(reg));
    } else if (width == DYN_8bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, X86Asm::Reg8(reg));
    } else {
        kpanic_fmt("X86DynamicCodeGen::writeMem unexpected width: %d", (U32)width);
    }
}

void X86DynamicCodeGen::writeMem(U32 value, DynWidth width, DynReg address, DynReg offset, U8 lsl, U32 disp) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, value);
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, (U16)value);
    } else if (width == DYN_8bit) {
        x86.writeMem(X86Asm::Reg32(address), offset, lsl, disp, (U8)value);
    } else {
        kpanic_fmt("X86DynamicCodeGen::writeMem unexpected width: %d", (U32)width);
    }
}

void X86DynamicCodeGen::and32(DynReg dst, U32 imm) {
    x86.and_(X86Asm::Reg32(dst), imm);
}

void X86DynamicCodeGen::shr32(DynReg dst, U32 imm) {
    x86.shr(X86Asm::Reg32(dst), imm);
}

U32 X86DynamicCodeGen::getBufferSize() {
    return (U32)x86.buffer.size();
}

U8* X86DynamicCodeGen::getBuffer() {
    return x86.buffer.data();
}


U32 X86DynamicCodeGen::getIfJumpSize() {
    return (U32)x86.ifJump.size();
}

void writeBlockExitForJIT(U8* buffer) {
    X86Asm x86;

    // writeCache
    for (int i = 0; i < 8; i++) {
        if (regCache[i]) {
            x86.writeMem(x86.edi, CPU::offsetofReg32(i), X86Asm::Reg32(regCache[i]));
        }
    }

    x86.pop(x86.ebp);
    x86.pop(x86.esi);
    x86.pop(x86.edi);
    x86.pop(x86.ebx);

    x86.ret();
    memcpy(buffer, x86.buffer.data(), x86.buffer.size());
}

void X86DynamicCodeGen::blockExit() {

    writeCache();

    x86.pop(x86.ebp);
    x86.pop(x86.esi);
    x86.pop(x86.edi);
    x86.pop(x86.ebx);

    x86.ret();
}

void X86DynamicCodeGen::incrementEip(U32 inc) {
    x86.addMem32(x86.edi, offsetof(CPU, eip.u32), inc);
}

void setConditionInReg(DynamicData* data, DynConditional condition, DynReg reg);

void X86DynamicCodeGen::setConditional(DynConditional condition) {
    bool setnz = true;
    // changing conditions to ones that are optimized
    if (condition == Z) {
        condition = NZ;
        setnz = false;
    } else if (condition == NS) {
        condition = S;
        setnz = false;
    } else if (condition == NB) {
        condition = B;
        setnz = false;
    } else if (condition == NO) {
        condition = O;
        setnz = false;
    } else if (condition == NBE) {
        condition = BE;
        setnz = false;
    } else if (condition == NLE) {
        condition = LE;
        setnz = false;
    } else if (condition == NL) {
        condition = L;
        setnz = false;
    }
    setConditionInReg(condition, DYN_CALL_RESULT);
    x86.test(x86.eax, x86.eax);
    if (setnz) {
        x86.setnz(X86Asm::Reg8(x86.eax.reg));
    } else {
        x86.setz(X86Asm::Reg8(x86.eax.reg));
    }
}

void X86DynamicCodeGen::setCC(X86Asm::Reg32 reg, DynConditionEvaluate condition) {

    switch (condition) {
    case DYN_EQUALS:
        x86.setz(X86Asm::Reg8(reg.reg));
        break;
    case DYN_NOT_EQUALS:
        x86.setnz(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_UNSIGNED:
        x86.setb(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_EQUAL_UNSIGNED:
        x86.setbe(X86Asm::Reg8(reg.reg));
        break;
    case DYN_GREATER_THAN_EQUAL_UNSIGNED:
        x86.setnb(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_SIGNED:
        x86.setl(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_EQUAL_SIGNED:
        x86.setle(X86Asm::Reg8(reg.reg));
        break;
    default:
        kpanic_fmt("x32CPU::evaluateToRegFromRegs unknown condition %d", condition);
    }
}

void X86DynamicCodeGen::evaluateToReg(X86Asm::Reg32 reg, X86Asm::Reg32 left, X86Asm::Reg32 right, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    if (regWidth == DYN_32bit) {
        x86.cmp(left, right);
    } else if (regWidth == DYN_16bit) {
        x86.cmp(X86Asm::Reg16(left.reg), X86Asm::Reg16(right.reg));
    } else if (regWidth == DYN_8bit) {
        x86.cmp(X86Asm::Reg8(left.reg), X86Asm::Reg8(right.reg));
    } else {
        kpanic_fmt("x32CPU::evaluateToReg reg width %d", regWidth);
    }

    regUsed[reg.reg] = true;
    setCC(reg, condition);
    x86.movzx(reg, X86Asm::Reg8(reg.reg));

    if (doneWithLeftReg) {
        regUsed[left.reg] = false;
    }
    if (doneWithRightReg) {
        regUsed[right.reg] = false;
    }
}

void X86DynamicCodeGen::evaluateToReg(X86Asm::Reg32 reg, X86Asm::Reg32 left, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg) {
    if (regWidth == DYN_32bit) {
        x86.cmp(left, rightConst);
    } else if (regWidth == DYN_16bit) {
        x86.cmp(X86Asm::Reg16(left.reg), (U16)rightConst);
    } else if (regWidth == DYN_8bit) {
        x86.cmp(X86Asm::Reg8(left.reg), (U8)rightConst);
    } else {
        kpanic_fmt("x32CPU::evaluateToReg reg width %d", regWidth);
    }

    regUsed[reg.reg] = true;
    setCC(reg, condition);
    x86.movzx(reg, X86Asm::Reg8(reg.reg));

    if (doneWithLeftReg) {
        regUsed[left.reg] = false;
    }
}

void X86DynamicCodeGen::evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    if (reg >= 4) {
        kpanic_fmt("x32CPU::evaluateToRegFromRegs doesn't support reg %d", reg);
    }
    if (isRightConst) {
        evaluateToReg(X86Asm::Reg32(reg), X86Asm::Reg32(left), rightConst, regWidth, condition, doneWithLeftReg);
    } else {
        evaluateToReg(X86Asm::Reg32(reg), X86Asm::Reg32(left), X86Asm::Reg32(right), regWidth, condition, doneWithLeftReg, doneWithRightReg);
    }
}

void X86DynamicCodeGen::callHostFunction(void* address, bool hasReturn, U32 argCount, U32 arg1, DynCallParamType arg1Type, bool doneWithArg1, U32 arg2, DynCallParamType arg2Type, bool doneWithArg2, U32 arg3, DynCallParamType arg3Type, bool doneWithArg3, U32 arg4, DynCallParamType arg4Type, bool doneWithArg4, U32 arg5, DynCallParamType arg5Type, bool doneWithArg5) {
    bool regDone[4] = { false, false, false, false };

    if (argCount >= 5) {
        if (isParamTypeReg(arg5Type) && doneWithArg5) {
            if (arg5 < 4) {
                regDone[arg5] = true;
            }
        }
    }
    if (argCount >= 4) {
        if (isParamTypeReg(arg4Type) && doneWithArg4) {
            if (arg4 < 4) {
                regDone[arg4] = true;
            }
        }
    }
    if (argCount >= 3) {
        if (isParamTypeReg(arg3Type) && doneWithArg3) {
            if (arg3 < 4) {
                regDone[arg3] = true;
            }
        }
    }
    if (argCount >= 2) {
        if (isParamTypeReg(arg2Type) && doneWithArg2) {
            if (arg2 < 4) {
                regDone[arg2] = true;
            }
        }
    }
    if (argCount >= 1) {
        if (isParamTypeReg(arg1Type) && doneWithArg1) {
            if (arg1 < 4) {
                regDone[arg1] = true;
            }
        }
    }

    writeCache();

    if (hasReturn) {
        regUsed[x86.eax.reg] = true;
    } else if (regUsed[x86.eax.reg] && !hasReturn && !regDone[x86.eax.reg]) {
        x86.push(x86.eax);
    }
    if (regUsed[x86.ecx.reg] && !regDone[x86.ecx.reg]) {
        x86.push(x86.ecx);
    }
    if (regUsed[x86.edx.reg] && !regDone[x86.edx.reg]) {
        x86.push(x86.edx);
    }
    if (argCount >= 5) {
        pushValue(arg5, arg5Type);
    }
    if (argCount >= 4) {
        pushValue(arg4, arg4Type);
    }
    if (argCount >= 3) {
        pushValue(arg3, arg3Type);
    }
    if (argCount >= 2) {
        pushValue(arg2, arg2Type);
    }
    if (argCount >= 1) {
        pushValue(arg1, arg1Type);
    }

    x86.call(address);

    if (argCount) {
        x86.add(x86.esp, 4 * argCount);
    }
    if (regUsed[x86.edx.reg] && !regDone[x86.edx.reg]) {
        x86.pop(x86.edx);
    }
    if (regUsed[x86.ecx.reg] && !regDone[x86.ecx.reg]) {
        x86.pop(x86.ecx);
    }
    if (!hasReturn && regUsed[x86.eax.reg] && !regDone[x86.eax.reg]) {
        x86.pop(x86.eax);
    }
    loadCache();
    for (int i = 0; i < 4; i++) {
        if (regDone[i]) {
            regUsed[i] = false;
        }
    }
}

void X86DynamicCodeGen::addRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.add(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.add(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.add(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("X86DynamicCodeGen::addRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::orRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.or_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.or_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.or_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("X86DynamicCodeGen::orRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::subRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.sub(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.sub(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.sub(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("X86DynamicCodeGen::subRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::andRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.and_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.and_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.and_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("X86DynamicCodeGen::andRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::xorRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.xor_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.xor_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("X86DynamicCodeGen::xorRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::shrRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.shr(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.shr(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.shr(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("X86DynamicCodeGen::shrRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::sarRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.sar(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.sar(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.sar(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("X86DynamicCodeGen::sarRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::shlRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.shl(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.shl(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.shl(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("X86DynamicCodeGen::shlRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::rolRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.rol(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.rol(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.rol(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("X86DynamicCodeGen::rolRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::rorRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.ror(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.ror(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else if (regWidth == DYN_8bit) {
        x86.ror(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
    } else {
        kpanic("X86DynamicCodeGen::rorRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::imulRegReg(DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (regWidth == DYN_32bit) {
        x86.imul(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
    } else if (regWidth == DYN_16bit) {
        x86.imul(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
    } else {
        kpanic("X86DynamicCodeGen::imulRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

void X86DynamicCodeGen::imulRegReg64(DynReg high64, DynReg dst, DynReg src, bool doneWithSrcReg) {
    if (high64 != 2) {
        kpanic("X86DynamicCodeGen::imulRegReg64 expects high64 to be EDX");
    }
    if (dst != 0) {
        kpanic("X86DynamicCodeGen::imulRegReg64 expects dst to be EAX");
    }
    x86.imul(X86Asm::Reg32(src));
    if (doneWithSrcReg) {
        regUsed[src] = false;
    }
}

void X86DynamicCodeGen::mulRegReg64(DynReg high64, DynReg dst, DynReg src, bool doneWithSrcReg) {
    if (high64 != 2) {
        kpanic("X86DynamicCodeGen::imulRegReg64 expects high64 to be EDX");
    }
    if (dst != 0) {
        kpanic("X86DynamicCodeGen::imulRegReg64 expects dst to be EAX");
    }
    x86.mul(X86Asm::Reg32(src));
    if (doneWithSrcReg) {
        regUsed[src] = false;
    }
}

void X86DynamicCodeGen::divRegRegWithRemainder(DynReg dest, DynReg src, DynReg remainder, DynWidth width) {
    if (dest != 0) {
        kpanic("X86DynamicCodeGen::divRegRegWithRemainder dest to be EAX");
    }
    if (remainder != 2) {
        kpanic("X86DynamicCodeGen::divRegRegWithRemainder remainder to be EDX");
    }
    if (width == DYN_16bit) {
        x86.xor_(x86.dx, x86.dx); // 16-bit div on x86 makes a 32-bit word from DX and AX, then divides that by the source
        x86.div(X86Asm::Reg16(src));
    } else if (width == DYN_32bit) {
        x86.xor_(x86.edx, x86.edx); // 32-bit div on x86 makes a 64-bit word from EDX and EAX, then divides that by the source
        x86.div(X86Asm::Reg32(src));
    } else {
        kpanic("X86DynamicCodeGen::divRegRegWithRemainder");
    }
}

void X86DynamicCodeGen::idivRegRegWithRemainder(DynReg dest, DynReg src, DynReg remainder, DynWidth width) {
    if (dest != 0) {
        kpanic("X86DynamicCodeGen::idivRegRegWithRemainder dest to be EAX");
    }
    if (remainder != 2) {
        kpanic("X86DynamicCodeGen::idivRegRegWithRemainder remainder to be EDX");
    }
    if (width == DYN_16bit) {
        // sign extend ax into dx, sinc idiv uses dx:ax
        x86.mov(x86.dx, x86.ax);
        x86.sar(x86.dx, 15);
        x86.idiv(X86Asm::Reg16(src));
    } else if (width == DYN_32bit) {
        // sign extend eax into edx, sinc idiv uses edx:eax
        x86.mov(x86.edx, x86.eax);
        x86.sar(x86.edx, 31);
        x86.idiv(X86Asm::Reg32(src));
    } else {
        kpanic("X86DynamicCodeGen::idivRegRegWithRemainder");
    }
}

void X86DynamicCodeGen::negReg2(DynWidth regWidth, RegPtr reg, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.neg(X86Asm::Reg32(reg->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.neg(X86Asm::Reg16(reg->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.neg(X86Asm::Reg8(get8bitReg(reg)));
    } else {
        kpanic("X86DynamicCodeGen::negReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::notReg2(DynWidth regWidth, RegPtr reg, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.not_(X86Asm::Reg32(reg->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.not_(X86Asm::Reg16(reg->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.not_(X86Asm::Reg8(get8bitReg(reg)));
    } else {
        kpanic("X86DynamicCodeGen::notReg");
    }
    // not doesn't set flags
}

void X86DynamicCodeGen::negReg(DynReg reg, DynWidth regWidth) {
    if (regWidth == DYN_32bit) {
        x86.neg(X86Asm::Reg32(reg));
    } else if (regWidth == DYN_16bit) {
        x86.neg(X86Asm::Reg16(reg));
    } else if (regWidth == DYN_8bit) {
        x86.neg(X86Asm::Reg8(reg));
    } else {
        kpanic("X86DynamicCodeGen::negReg");
    }
}

void X86DynamicCodeGen::notReg(DynReg reg, DynWidth regWidth) {
    if (regWidth == DYN_32bit) {
        x86.not_(X86Asm::Reg32(reg));
    } else if (regWidth == DYN_16bit) {
        x86.not_(X86Asm::Reg16(reg));
    } else if (regWidth == DYN_8bit) {
        x86.not_(X86Asm::Reg8(reg));
    } else {
        kpanic("X86DynamicCodeGen::notReg");
    }
}

void X86DynamicCodeGen::addRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.add(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.add(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.add(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::addRegImm");
    }
}

void X86DynamicCodeGen::orRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.or_(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.or_(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.or_(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::orRegImm");
    }
}

void X86DynamicCodeGen::subRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.sub(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.sub(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.sub(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::subRegImm");
    }
}

void X86DynamicCodeGen::andRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.and_(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.and_(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.and_(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::andRegImm");
    }
}

void X86DynamicCodeGen::xorRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.xor_(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.xor_(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.xor_(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::xorRegImm");
    }
}

void X86DynamicCodeGen::shrRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.shr(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.shr(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.shr(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::shrRegImm");
    }
}

void X86DynamicCodeGen::sarRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.sar(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.sar(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.sar(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::sarRegImm");
    }
}

void X86DynamicCodeGen::shlRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.shl(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.shl(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.shl(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::shlRegImm");
    }
}

void X86DynamicCodeGen::rolRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.rol(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.rol(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.rol(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::rolRegImm");
    }
}

void X86DynamicCodeGen::rorRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.ror(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.ror(X86Asm::Reg16(reg), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.ror(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::rorRegImm");
    }
}

void X86DynamicCodeGen::imulRegImm(DynReg reg, DynWidth regWidth, U32 imm) {
    if (regWidth == DYN_32bit) {
        x86.imul(X86Asm::Reg32(reg), imm);
    } else if (regWidth == DYN_16bit) {
        x86.imul(X86Asm::Reg16(reg), (U16)imm);
    } else {
        kpanic("X86DynamicCodeGen::imulRegImm");
    }
}

// :TODO: move to base
void X86DynamicCodeGen::calculateEaa(DecodedOp* op, DynReg reg) {
    regUsed[reg] = true;

    if (op->ea16) {
        // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)
        X86Asm::Reg16 reg16(reg);

        x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(reg));

        if (op->data.disp) {
            x86.add(reg16, (U16)op->data.disp);
        }
        if (op->rm != 8) {
            // intentional 16-bit add
            if (regCache[op->rm]) {
                x86.add(reg16, X86Asm::Reg16(regCache[op->rm]));
            } else {
                x86.addMemReg(reg16, x86.edi, CPU::offsetofReg16(op->rm));
            }
        }

        if (op->sibIndex != 8) {
            // intentional 16-bit add
            if (regCache[op->sibIndex]) {
                x86.add(reg16, X86Asm::Reg16(regCache[op->sibIndex]));
            } else {
                x86.addMemReg(reg16, x86.edi, CPU::offsetofReg16(op->sibIndex));
            }
        }

        // seg[6] is always 0
        if (op->base < 6) {
            // intentional 32-bit add
            x86.addMemReg(X86Asm::Reg32(reg), x86.edi, CPU::offsetofSegAddress(op->base));
        }
    } else {
        if (op->sibIndex != 8) {
            loadReg(op->sibIndex, reg, DYN_32bit);
            if (op->sibScale) {
                x86.shl(X86Asm::Reg32(reg), op->sibScale);
            }
            if (op->rm != 8) {
                if (regCache[op->rm]) {
                    x86.add(X86Asm::Reg32(reg), X86Asm::Reg32(regCache[op->rm]));
                } else {
                    x86.addMemReg(X86Asm::Reg32(reg), x86.edi, CPU::offsetofReg32(op->rm));
                }
            }
            if (op->data.disp) {
                x86.add(X86Asm::Reg32(reg), op->data.disp);
            }
        } else if (op->rm != 8) {
            loadReg(op->rm, reg, DYN_32bit);
            if (op->data.disp) {
                x86.add(X86Asm::Reg32(reg), op->data.disp);
            }
        } else if (op->data.disp) {
            x86.mov(X86Asm::Reg32(reg), op->data.disp);
        } else {
            x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
        }

        // seg[6] is always 0
        if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
            x86.addMemReg(X86Asm::Reg32(reg), x86.edi, CPU::offsetofSegAddress(op->base));
        }
    }
}

void X86DynamicCodeGen::byteSwapReg32(DynReg reg) {
    x86.bswap(reg);
}

void X86DynamicCodeGen::movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    regUsed[dst] = true;
    if (dstWidth <= srcWidth) {
        movToRegFromReg(dst, dstWidth, src, srcWidth, doneWithSrcReg);
    } else {
        if (dstWidth == DYN_32bit) {
            if (srcWidth == DYN_16bit) {
                x86.movsx(X86Asm::Reg32(dst), X86Asm::Reg16(src));
            } else if (srcWidth == DYN_8bit) {
                x86.movsx(X86Asm::Reg32(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth == DYN_16bit) {
            if (srcWidth == DYN_8bit) {
                x86.movsx(X86Asm::Reg16(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
            }
        } else {
            kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
        }
        if (doneWithSrcReg) {
            regUsed[src] = false;
        }
    }
}

void X86DynamicCodeGen::movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    regUsed[dst] = true;
    if (dstWidth <= srcWidth) {
        if (dst == src) // downsizing doesn't need anything
            return;
        if (dstWidth == DYN_32bit) {
            x86.mov(X86Asm::Reg32(dst), X86Asm::Reg32(src));
        } else if (dstWidth == DYN_16bit) {
            x86.mov(X86Asm::Reg16(dst), X86Asm::Reg16(src));
        } else if (dstWidth == DYN_8bit) {
            x86.mov(X86Asm::Reg8(dst), X86Asm::Reg8(src));
        } else {
            kpanic_fmt("unknown dstWidth in x32CPU::movToRegFromReg %d", dstWidth);
        }
    } else {
        if (dstWidth == DYN_32bit) {
            if (srcWidth == DYN_16bit) {
                x86.movzx(X86Asm::Reg32(dst), X86Asm::Reg16(src));
            } else if (srcWidth == DYN_8bit) {
                x86.movzx(X86Asm::Reg32(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth == DYN_16bit) {
            if (srcWidth == DYN_8bit) {
                x86.movzx(X86Asm::Reg16(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }
        } else {
            kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
        }
    }
    if (doneWithSrcReg) {
        regUsed[src] = false;
    }
}

void X86DynamicCodeGen::movToReg(DynReg reg, DynWidth width, U32 imm) {
    regUsed[reg] = true;
    if (width == DYN_32bit) {
        x86.mov(X86Asm::Reg32(reg), imm);
    } else if (width == DYN_16bit) {
        x86.mov(X86Asm::Reg16(reg), (U16)imm);
    } else if (width == DYN_8bit) {
        x86.mov(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic_fmt("unknown width in x32CPU::movToReg %d", width);
    }
}

void X86DynamicCodeGen::pushValue(U32 arg, DynCallParamType argType) {
    switch (argType) {
    case DYN_PARAM_REG_8:
        x86.movzx(X86Asm::Reg32(arg), X86Asm::Reg8(arg));
        x86.push(X86Asm::Reg32(arg));
        break;
    case DYN_PARAM_REG_16:
        x86.movzx(X86Asm::Reg32(arg), X86Asm::Reg16(arg));
        x86.push(X86Asm::Reg32(arg));
        break;
    case DYN_PARAM_REG_32:
        x86.push(X86Asm::Reg32(arg));
        break;
    case DYN_PARAM_CPU:
        x86.push(x86.edi);
        break;
    case DYN_PARAM_CONST_8:
        x86.push((U32)(arg & 0xFF));
        break;
    case DYN_PARAM_CONST_16:
        x86.push((U32)(arg & 0xFFFF));
        break;
    case DYN_PARAM_CONST_32:
        x86.push(arg);
        break;
    case DYN_PARAM_CONST_PTR:
        x86.push((U32)arg);
        break;
    case DYN_PARAM_CPU_REG_8:
    {
        loadReg(arg, DYN_CALL_RESULT, DYN_8bit);
        x86.movzx(x86.eax, X86Asm::Reg8(DYN_CALL_RESULT));
        x86.push(x86.eax);
        regUsed[DYN_CALL_RESULT] = false;
        break;
    }
    case DYN_PARAM_CPU_REG_16:
    {
        loadReg(arg, DYN_CALL_RESULT, DYN_16bit);
        x86.movzx(x86.eax, X86Asm::Reg16(DYN_CALL_RESULT));
        x86.push(x86.eax);
        regUsed[DYN_CALL_RESULT] = false;
        break;
    }
    case DYN_PARAM_CPU_REG_32:
    {
        loadReg(arg, DYN_CALL_RESULT, DYN_32bit);
        x86.push(x86.eax);
        regUsed[DYN_CALL_RESULT] = false;
        break;
    }
    default:
        kpanic_fmt("x32CPU: unknown argType: %d", argType);
        break;
    }
}

void X86DynamicCodeGen::zeroExtendReg16To32(DynReg dest, DynReg src) {
    x86.movzx(X86Asm::Reg32(dest), X86Asm::Reg16(src));
    regUsed[dest] = true;
}

void X86DynamicCodeGen::movToRegFromCpu(DynReg reg, DynReg sib, U8 lsl, U32 srcOffset, DynWidth width) {
    this->regUsed[reg] = true;
    // mov reg, [edi+srcOffset]    
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(reg), x86.edi, X86Asm::Reg32(sib), lsl, srcOffset);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(reg), x86.edi, X86Asm::Reg32(sib), lsl, srcOffset);
    } else if (width == DYN_8bit) {
        x86.readMem(X86Asm::Reg8(reg), x86.edi, X86Asm::Reg32(sib), lsl, srcOffset);
    } else {
        kpanic_fmt("unknown dstWidth in X86DynamicCodeGen::movToRegFromCpu %d", width);
    }
}

void X86DynamicCodeGen::movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width) {
    this->regUsed[reg] = true;
    // mov reg, [edi+srcOffset]    
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(reg), x86.edi, srcOffset);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(reg), x86.edi, srcOffset);
    } else if (width == DYN_8bit) {
        x86.readMem(X86Asm::Reg8(reg), x86.edi, srcOffset);
    } else {
        kpanic_fmt("unknown dstWidth in X86DynamicCodeGen::movToRegFromCpu %d", width);
    }
}

void X86DynamicCodeGen::movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) {
    if (width == DYN_32bit) {
        x86.writeMem(x86.edi, dstOffset, X86Asm::Reg32(reg));
    } else if (width == DYN_16bit) {
        x86.writeMem(x86.edi, dstOffset, X86Asm::Reg16(reg));
    } else if (width == DYN_8bit) {
        x86.writeMem(x86.edi, dstOffset, X86Asm::Reg8(reg));
    } else {
        kpanic_fmt("unknown dstWidth in X86DynamicCodeGen::movToCpuFromReg %d", width);
    }
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::movToCpuFromReg(DynReg sib, U8 lsl, U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) {
    if (width == DYN_32bit) {
        x86.writeMem(x86.edi, sib, lsl, dstOffset, X86Asm::Reg32(reg));
    } else if (width == DYN_16bit) {
        x86.writeMem(x86.edi, sib, lsl, dstOffset, X86Asm::Reg16(reg));
    } else if (width == DYN_8bit) {
        x86.writeMem(x86.edi, sib, lsl, dstOffset, X86Asm::Reg8(reg));
    } else {
        kpanic_fmt("unknown dstWidth in X86DynamicCodeGen::movToCpuFromReg %d", width);
    }
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm) {
    if (dstWidth == DYN_32bit) {
        x86.writeMem(x86.edi, dstOffset, imm);
    } else if (dstWidth == DYN_16bit) {
        x86.writeMem(x86.edi, dstOffset, (U16)imm);
    } else if (dstWidth == DYN_8bit) {
        x86.writeMem(x86.edi, dstOffset, (U8)imm);
    } else {
        kpanic_fmt("unknown dstWidth in X86DynamicCodeGen::movToCpu %d", dstWidth);
    }
}

void X86DynamicCodeGen::movToCpu(DynReg sib, U8 lsl, U32 dstOffset, DynWidth dstWidth, U32 imm) {
    if (dstWidth == DYN_32bit) {
        x86.writeMem(x86.edi, sib, lsl, dstOffset, imm);
    } else if (dstWidth == DYN_16bit) {
        x86.writeMem(x86.edi, sib, lsl, dstOffset, (U16)imm);
    } else if (dstWidth == DYN_8bit) {
        x86.writeMem(x86.edi, sib, lsl, dstOffset, (U8)imm);
    } else {
        kpanic_fmt("unknown dstWidth in X86DynamicCodeGen::movToCpu %d", dstWidth);
    }
}

void X86DynamicCodeGen::JumpIf(DynReg reg, bool doneWithReg, U32 address) {
    x86.test(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
    x86.jnz(address);
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::JumpIfNot(DynReg reg, bool doneWithReg, U32 address) {
    x86.test(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
    x86.jz(address);
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::JumpInBlock(U32 address) {
    x86.jmp(address);
}

void X86DynamicCodeGen::IfPtrEqual(DynReg reg, DYN_PTR_SIZE value, bool doneWithReg) {
    IfEqual(X86Asm::Reg32(reg), value, doneWithReg);
}

void X86DynamicCodeGen::IfLessThan(DynReg reg, U32 value, bool doneWithReg) {
    x86.IfLessThan(X86Asm::Reg32(reg), value);

    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::IfEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    x86.IfEqual(reg, value);
    if (doneWithReg) {
        regUsed[reg.reg] = false;
    }
}

void X86DynamicCodeGen::IfNotEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    x86.IfNotEqual(reg, value);
    if (doneWithReg) {
        regUsed[reg.reg] = false;
    }
}


void X86DynamicCodeGen::IfBitSet(DynReg reg, U32 value, bool doneWithReg, bool bigJump) {
    x86.IfBitSet(X86Asm::Reg32(reg), value, bigJump);
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::IfNotBitSet(DynReg reg, U32 value, bool doneWithReg, bool bigJump) {
    x86.IfNotBitSet(X86Asm::Reg32(reg), value, bigJump);
    if (doneWithReg) {
        regUsed[reg] = false;
    }

}
void X86DynamicCodeGen::If(DynReg reg, bool doneWithReg, bool bigJump) {
    x86.IfNotZero(X86Asm::Reg32(reg), bigJump);
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::IfNot(DynReg reg, bool doneWithReg) {
    x86.IfZero(X86Asm::Reg32(reg));
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void X86DynamicCodeGen::StartElse(bool bigJump) {
    x86.Else(bigJump);
}

void X86DynamicCodeGen::EndIf(bool bigJump) {
    x86.EndIf(bigJump);
}

void X86DynamicCodeGen::dynamic_rdtsc(DecodedOp* op) {
    x86.rdtsc();
    storeReg(0, DynReg(0), DYN_32bit, false);
    storeReg(2, DynReg(2), DYN_32bit, false);
    incrementEip(op->len);
}

void X86DynamicCodeGen::dynamic_checkFlags(DecodedOp* op, DynReg tmpReg, DynReg tmpReg2) {
    if (op->needsToSetFlags(cpu)) {
        this->x86.pushFlags();
        this->x86.pop(X86Asm::Reg32(tmpReg));
        regUsed[tmpReg] = true;
        this->x86.and_(X86Asm::Reg32(tmpReg), FMASK_TEST);

        //this->andCPUFlagsImm(~FMASK_TEST, tmpReg2);
        movToRegFromCpu(tmpReg2, offsetof(CPU, flags), DYN_32bit);
        andRegImm(tmpReg2, DYN_32bit, ~FMASK_TEST);

        //this->orCPUFlagsReg(tmpReg, tmpReg2, true);
        orRegReg(tmpReg, tmpReg2, DYN_32bit, true);
        movToCpuFromReg(offsetof(CPU, flags), tmpReg, DYN_32bit, true);

        this->storeLazyFlags(FLAGS_NONE);
        this->currentLazyFlags = FLAGS_NONE;
    }
}

void X86DynamicCodeGen::dynamic_cmpxchg8b_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_cmpxchg8b_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            // :TODO: what if it crossed a page bound, movToMem only checked the first 4 bytes, not all 8
            // need to load eax edx
            writeCache();
            this->x86.mov(x86.ebp, X86Asm::Reg32(addressReg));
            this->x86.mov(x86.esi, X86Asm::Reg32(offsetReg));
            DynamicCodeGen::loadReg(0, (DynReg)0, DYN_32bit);
            DynamicCodeGen::loadReg(1, (DynReg)1, DYN_32bit);
            DynamicCodeGen::loadReg(2, (DynReg)2, DYN_32bit);
            DynamicCodeGen::loadReg(3, (DynReg)3, DYN_32bit);
            this->x86.cmpxchg8b(x86.esi, x86.ebp, 0, 0);
            loadCache();
            storeReg(0, (DynReg)0, DYN_32bit, true);
            storeReg(1, (DynReg)1, DYN_32bit, true);
            storeReg(2, (DynReg)2, DYN_32bit, true);
            storeReg(3, (DynReg)3, DYN_32bit, true);            
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_cmpxchg8b_lock(op);
        });

        currentLazyFlags = nullptr;
    }
}
void X86DynamicCodeGen::dynamic_cmpxchge32r32_lock(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
        loadReg(0, DYN_CALL_RESULT, DYN_32bit);
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        this->x86.cmpxchg(X86Asm::Reg32(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
        this->regUsed[DYN_DEST] = false;
        this->regUsed[DYN_SRC] = false;
        dynamic_checkFlags(op, DYN_DEST, DYN_SRC);
        storeReg(0, DYN_CALL_RESULT, DYN_32bit, true);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cmpxchge32r32_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_cmpxchge16r16_lock(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
        loadReg(0, DYN_CALL_RESULT, DYN_16bit);
        loadReg(op->reg, DYN_DEST, DYN_16bit);
        this->x86.cmpxchg(X86Asm::Reg16(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
        this->regUsed[DYN_DEST] = false;
        this->regUsed[DYN_SRC] = false;
        dynamic_checkFlags(op, DYN_DEST, DYN_SRC);
        storeReg(0, DYN_CALL_RESULT, DYN_16bit, true);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cmpxchge32r32_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_cmpxchge8r8_lock(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_8bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
        loadReg(0, DYN_CALL_RESULT, DYN_8bit);
        loadReg(op->reg, DYN_DEST, DYN_8bit);
        this->x86.cmpxchg(X86Asm::Reg8(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
        this->regUsed[DYN_DEST] = false;
        this->regUsed[DYN_SRC] = false;
        dynamic_checkFlags(op, DYN_DEST, DYN_SRC);
        storeReg(0, DYN_CALL_RESULT, DYN_8bit, true);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cmpxchge32r32_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_xchge32r32_lock(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        this->x86.xchg(X86Asm::Reg32(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_xchge32r32_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_xchge16r16_lock(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
        loadReg(op->reg, DYN_DEST, DYN_16bit);
        this->x86.xchg(X86Asm::Reg16(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_xchge16r16_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_xchge8r8_lock(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
        loadReg(op->reg, DYN_DEST, DYN_8bit);
        this->x86.xchg(X86Asm::Reg8(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
        storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_xchge8r8_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_arithE32R32_lock(DecodedOp* op, std::function<void(DynReg dest, DynReg address, DynReg offset)> callback, std::function<void()> fallback, bool writeReg) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, callback, writeReg, this](DynReg addressReg, DynReg offsetReg) {
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        callback(DYN_DEST, addressReg, offsetReg);
        this->regUsed[DYN_SRC] = false;
        this->regUsed[DYN_ADDRESS] = false;
        dynamic_checkFlags(op, DYN_ADDRESS, DYN_SRC);
        if (writeReg) {
            storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        }
        incrementEip(op->len);
    }, [op, fallback, this]() {
        fallback();
    });
}

void X86DynamicCodeGen::dynamic_arithE16R16_lock(DecodedOp* op, std::function<void(DynReg dest, DynReg address, DynReg offset)> callback, std::function<void()> fallback, bool writeReg) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, callback, writeReg, this](DynReg addressReg, DynReg offsetReg) {
        loadReg(op->reg, DYN_DEST, DYN_16bit);
        callback(DYN_DEST, addressReg, offsetReg);
        this->regUsed[DYN_SRC] = false;
        this->regUsed[DYN_ADDRESS] = false;
        dynamic_checkFlags(op, DYN_ADDRESS, DYN_SRC);
        if (writeReg) {
            storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        }
        incrementEip(op->len);
    }, [op, fallback, this]() {
        fallback();
    });
}
void X86DynamicCodeGen::dynamic_arithE8R8_lock(DecodedOp* op, std::function<void(DynReg dest, DynReg address, DynReg offset)> callback, std::function<void()> fallback, bool writeReg) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_8bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, callback, writeReg, this](DynReg addressReg, DynReg offsetReg) {
        loadReg(op->reg, DYN_DEST, DYN_8bit);
        callback(DYN_DEST, addressReg, offsetReg);
        this->regUsed[DYN_SRC] = false;
        this->regUsed[DYN_ADDRESS] = false;
        dynamic_checkFlags(op, DYN_ADDRESS, DYN_SRC);
        if (writeReg) {
            storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        }
        incrementEip(op->len);
    }, [op, fallback, this]() {
        fallback();
    });
}

void X86DynamicCodeGen::dynamic_arithE32_lock(DecodedOp* op, std::function<void(DynReg address, DynReg offset)> callback, std::function<void()> fallback) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, callback, this](DynReg addressReg, DynReg offsetReg) {
        callback(addressReg, offsetReg);
        this->regUsed[DYN_SRC] = false;
        this->regUsed[DYN_ADDRESS] = false;
        dynamic_checkFlags(op, DYN_ADDRESS, DYN_SRC);
        incrementEip(op->len);
    }, [op, fallback, this]() {
        fallback();
    });
}

void X86DynamicCodeGen::dynamic_arithE16_lock(DecodedOp* op, std::function<void(DynReg address, DynReg offset)> callback, std::function<void()> fallback) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, callback, this](DynReg addressReg, DynReg offsetReg) {
        callback(addressReg, offsetReg);
        this->regUsed[DYN_SRC] = false;
        this->regUsed[DYN_ADDRESS] = false;
        dynamic_checkFlags(op, DYN_ADDRESS, DYN_SRC);
        incrementEip(op->len);
    }, [op, fallback, this]() {
        fallback();
    });
}
void X86DynamicCodeGen::dynamic_arithE8_lock(DecodedOp* op, std::function<void(DynReg address, DynReg offset)> callback, std::function<void()> fallback) {
    calculateEaa(op, DYN_ADDRESS);
    movToMem(DYN_ADDRESS, DYN_8bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, callback, this](DynReg addressReg, DynReg offsetReg) {
        callback(addressReg, offsetReg);
        this->regUsed[DYN_SRC] = false;
        this->regUsed[DYN_ADDRESS] = false;
        dynamic_checkFlags(op, DYN_ADDRESS, DYN_SRC);
        incrementEip(op->len);
    }, [op, fallback, this]() {
        fallback();
    });
}

void X86DynamicCodeGen::dynamic_xaddr32e32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](DynReg dest, DynReg addressReg, DynReg offsetReg) {
        this->x86.xadd(X86Asm::Reg32(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_xaddr32e32_lock(op);
    }, true);
}

void X86DynamicCodeGen::dynamic_xaddr16e16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](DynReg dest, DynReg addressReg, DynReg offsetReg) {
        this->x86.xadd(X86Asm::Reg16(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_xaddr16e16_lock(op);
    }, true);
}
void X86DynamicCodeGen::dynamic_xaddr8e8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](DynReg dest, DynReg addressReg, DynReg offsetReg) {
        this->x86.xadd(X86Asm::Reg8(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_xaddr8e8_lock(op);
    }, true);
}

void X86DynamicCodeGen::dynamic_adde32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](DynReg dest, DynReg addressReg, DynReg offsetReg) {
        this->x86.lock();
        this->x86.addMemReg(X86Asm::Reg32(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_adde32r32_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_adde16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](DynReg dest, DynReg addressReg, DynReg offsetReg) {
        this->x86.lock();
        this->x86.addMemReg(X86Asm::Reg16(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_adde16r16_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_adde8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](DynReg dest, DynReg addressReg, DynReg offsetReg) {
        this->x86.lock();
        this->x86.addMemReg(X86Asm::Reg8(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_adde8r8_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_add32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](DynReg addressReg, DynReg offsetReg) {
        this->x86.lock();
        this->x86.addMem32(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, op->imm);
    }, [op, this]() {
        DynamicCodeGen::dynamic_add32_mem_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_add16_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](DynReg addressReg, DynReg offsetReg) {
        this->x86.lock();
        this->x86.addMem16(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, op->imm);
    }, [op, this]() {
        DynamicCodeGen::dynamic_add16_mem_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_add8_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](DynReg addressReg, DynReg offsetReg) {
        this->x86.lock();
        this->x86.addMem8(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, op->imm);
    }, [op, this]() {
        DynamicCodeGen::dynamic_add8_mem_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_sube32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](DynReg dest, DynReg addressReg, DynReg offsetReg) {
        this->x86.lock();
        this->x86.subMemReg(X86Asm::Reg32(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_sube32r32_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_sube16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](DynReg dest, DynReg addressReg, DynReg offsetReg) {
        this->x86.lock();
        this->x86.subMemReg(X86Asm::Reg16(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_sube16r16_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_sube8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](DynReg dest, DynReg addressReg, DynReg offsetReg) {
        this->x86.lock();
        this->x86.subMemReg(X86Asm::Reg8(DYN_DEST), X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_sube8r8_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_sub32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](DynReg addressReg, DynReg offsetReg) {
        this->x86.lock();
        this->x86.subMem32(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, op->imm);
    }, [op, this]() {
        DynamicCodeGen::dynamic_sub32_mem_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_sub16_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](DynReg addressReg, DynReg offsetReg) {
        this->x86.lock();
        this->x86.subMem16(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, op->imm);
    }, [op, this]() {
        DynamicCodeGen::dynamic_sub16_mem_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_sub8_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](DynReg addressReg, DynReg offsetReg) {
        this->x86.lock();
        this->x86.subMem8(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, op->imm);
    }, [op, this]() {
        DynamicCodeGen::dynamic_sub8_mem_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_inc32_mem32_lock(DecodedOp* op) {    
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_inc32_mem32_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.add(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg));
            this->x86.lock();
            this->x86.addMem32(X86Asm::Reg32(addressReg), 0, 1);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_inc32_mem32_lock(op);
        });        
        
        currentLazyFlags = nullptr;
    }    
}
void X86DynamicCodeGen::dynamic_inc16_mem16_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_inc16_mem16_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.add(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg));
            this->x86.lock();
            this->x86.addMem16(X86Asm::Reg32(addressReg), 0, 1);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_inc16_mem16_lock(op);
        });

        currentLazyFlags = nullptr;
    }
}
void X86DynamicCodeGen::dynamic_inc8_mem8_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_inc8_mem8_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.add(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg));
            this->x86.lock();
            this->x86.addMem8(X86Asm::Reg32(addressReg), 0, 1);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_inc8_mem8_lock(op);
        });

        currentLazyFlags = nullptr;
    }
}
void X86DynamicCodeGen::dynamic_dec32_mem32_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_dec32_mem32_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.add(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg));
            this->x86.lock();
            this->x86.addMem32(X86Asm::Reg32(addressReg), 0, (U32)(-1));
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_dec32_mem32_lock(op);
        });
        currentLazyFlags = nullptr;
    }
}
void X86DynamicCodeGen::dynamic_dec16_mem16_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_dec16_mem16_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.add(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg));
            this->x86.lock();
            this->x86.addMem16(X86Asm::Reg32(addressReg), 0, (U16)(-1));
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_dec16_mem16_lock(op);
        });
        currentLazyFlags = nullptr;
    }
}
void X86DynamicCodeGen::dynamic_dec8_mem8_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_dec8_mem8_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.add(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg));
            this->x86.lock();
            this->x86.addMem8(X86Asm::Reg32(addressReg), 0, (U8)(-1));
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_dec8_mem8_lock(op);
        });
        currentLazyFlags = nullptr;
    }
}

void X86DynamicCodeGen::dynamic_note32_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_note32_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            this->x86.notMem32(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_note32_lock(op);
        });
        currentLazyFlags = nullptr;
    }
}

void X86DynamicCodeGen::dynamic_note16_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_note16_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            this->x86.notMem16(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_note16_lock(op);
        });
        currentLazyFlags = nullptr;
    }
}

void X86DynamicCodeGen::dynamic_note8_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_note8_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_8bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            this->x86.notMem8(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_note8_lock(op);
        });
        currentLazyFlags = nullptr;
    }
}

void X86DynamicCodeGen::dynamic_nege32_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_nege32_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            this->x86.negMem32(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_nege32_lock(op);
        });
        currentLazyFlags = nullptr;
    }
}

void X86DynamicCodeGen::dynamic_nege16_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_nege16_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            this->x86.negMem16(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_nege16_lock(op);
        });
        currentLazyFlags = nullptr;
    }
}

void X86DynamicCodeGen::dynamic_nege8_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu)) {
        DynamicCodeGen::dynamic_nege8_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_8bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            this->x86.negMem8(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_nege8_lock(op);
        });
        currentLazyFlags = nullptr;
    }
}

void X86DynamicCodeGen::dynamic_btse32_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & CF) {
        DynamicCodeGen::dynamic_btse32_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            // imm is the mask, not the bitshift
            U8 imm = std::countr_zero(op->imm);
            this->x86.btsMem32(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, (U8)imm);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_btse32_lock(op);
        });
    }
}

void X86DynamicCodeGen::dynamic_btse16_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & CF) {
        DynamicCodeGen::dynamic_btse16_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            // imm is the mask, not the bitshift
            U8 imm = std::countr_zero(op->imm);
            this->x86.btsMem16(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, (U8)imm);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_btse16_lock(op);
        });
    }
}

void X86DynamicCodeGen::calculateEffectiveEaa32(DecodedOp* op, DynReg reg, DynReg tmpReg) {
    calculateEaa(op, DYN_ADDRESS);
    movToRegFromReg(tmpReg, DYN_32bit, reg, DYN_32bit, false);
    sarRegImm(tmpReg, DYN_32bit, 5);
    shlRegImm(tmpReg, DYN_32bit, 2);
    addRegReg(DYN_ADDRESS, tmpReg, DYN_32bit, true);
}
void X86DynamicCodeGen::calculateEffectiveEaa16(DecodedOp* op, DynReg reg, DynReg tmpReg) {
    calculateEaa(op, DYN_ADDRESS);
    movToRegFromReg(tmpReg, DYN_32bit, reg, DYN_32bit, false);
    sarRegImm(tmpReg, DYN_16bit, 4);
    shlRegImm(tmpReg, DYN_16bit, 1);
    zeroExtendReg16To32(tmpReg, tmpReg);
    addRegReg(DYN_ADDRESS, tmpReg, DYN_32bit, true);
}

void X86DynamicCodeGen::dynamic_btse32r32_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & CF) {
        DynamicCodeGen::dynamic_btse32r32_lock(op);
    } else {
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        calculateEffectiveEaa32(op, DYN_DEST, DYN_SRC);
        andRegImm(DYN_DEST, DYN_32bit, 0x1f);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {            
            this->x86.lock();
            this->x86.btsMem32(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, X86Asm::Reg32(DYN_DEST));
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_btse32r32_lock(op);
        });
    }
    incrementEip(op->len);    
}

void X86DynamicCodeGen::dynamic_btse16r16_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & CF) {
        DynamicCodeGen::dynamic_btse16r16_lock(op);
    } else {
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        calculateEffectiveEaa16(op, DYN_DEST, DYN_SRC);
        andRegImm(DYN_DEST, DYN_32bit, 0xf);
        movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            this->x86.btsMem16(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, X86Asm::Reg16(DYN_DEST));
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_btse16r16_lock(op);
        });
    }
    incrementEip(op->len);
}

void X86DynamicCodeGen::dynamic_btre32_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & CF) {
        DynamicCodeGen::dynamic_btre32_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            // imm is the mask, not the bitshift
            U8 imm = std::countr_zero(op->imm);
            this->x86.btrMem32(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, (U8)imm);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_btre32_lock(op);
        });
    }
}
void X86DynamicCodeGen::dynamic_btre16_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & CF) {
        DynamicCodeGen::dynamic_btre16_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            // imm is the mask, not the bitshift
            U8 imm = std::countr_zero(op->imm);
            this->x86.btrMem16(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, (U8)imm);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_btre16_lock(op);
        });
    }
}
void X86DynamicCodeGen::dynamic_btre32r32_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & CF) {
        DynamicCodeGen::dynamic_btre32r32_lock(op);
    } else {
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        calculateEffectiveEaa32(op, DYN_DEST, DYN_SRC);
        andRegImm(DYN_DEST, DYN_32bit, 0x1f);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            this->x86.btrMem32(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, X86Asm::Reg32(DYN_DEST));
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_btre32r32_lock(op);
        });
    }
    incrementEip(op->len);
}
void X86DynamicCodeGen::dynamic_btre16r16_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & CF) {
        DynamicCodeGen::dynamic_btre16r16_lock(op);
    } else {
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        calculateEffectiveEaa16(op, DYN_DEST, DYN_SRC);
        andRegImm(DYN_DEST, DYN_32bit, 0xf);
        movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            this->x86.btrMem16(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, X86Asm::Reg16(DYN_DEST));
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_btre16r16_lock(op);
        });
    }
    incrementEip(op->len);
}

void X86DynamicCodeGen::dynamic_btce32_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & CF) {
        DynamicCodeGen::dynamic_btce32_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            // imm is the mask, not the bitshift
            U8 imm = std::countr_zero(op->imm);
            this->x86.btcMem32(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, (U8)imm);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_btce32_lock(op);
        });
    }
}
void X86DynamicCodeGen::dynamic_btce16_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & CF) {
        DynamicCodeGen::dynamic_btce16_lock(op);
    } else {
        calculateEaa(op, DYN_ADDRESS);
        movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            // imm is the mask, not the bitshift
            U8 imm = std::countr_zero(op->imm);
            this->x86.btcMem16(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, (U8)imm);
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_btce16_lock(op);
        });
    }
}
void X86DynamicCodeGen::dynamic_btce32r32_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & CF) {
        DynamicCodeGen::dynamic_btce32r32_lock(op);
    } else {
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        calculateEffectiveEaa32(op, DYN_DEST, DYN_SRC);
        andRegImm(DYN_DEST, DYN_32bit, 0x1f);
        movToMem(DYN_ADDRESS, DYN_32bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            this->x86.btcMem32(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, X86Asm::Reg32(DYN_DEST));
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_btce32r32_lock(op);
        });
    }
    incrementEip(op->len);
}
void X86DynamicCodeGen::dynamic_btce16r16_lock(DecodedOp* op) {
    if (op->needsToSetFlags(cpu) & CF) {
        DynamicCodeGen::dynamic_btce16r16_lock(op);
    } else {
        loadReg(op->reg, DYN_DEST, DYN_32bit);
        calculateEffectiveEaa16(op, DYN_DEST, DYN_SRC);
        andRegImm(DYN_DEST, DYN_32bit, 0xf);
        movToMem(DYN_ADDRESS, DYN_16bit, 0, DynCallParamType(0), false, true, DYN_SRC, [op, this](DynReg addressReg, DynReg offsetReg) {
            this->x86.lock();
            this->x86.btcMem16(X86Asm::Reg32(addressReg), X86Asm::Reg32(offsetReg), 0, 0, X86Asm::Reg16(DYN_DEST));
            incrementEip(op->len);
        }, [op, this]() {
            DynamicCodeGen::dynamic_btce16r16_lock(op);
        });
    }
    incrementEip(op->len);
}

U8* X86DynamicCodeGen::createStartJITCode() {
    x86.push(x86.ebx);
    x86.push(x86.edi);
    x86.push(x86.esi);
    x86.push(x86.ebp);
    // on win32 ecx contains cpu
    x86.mov(x86.edi, x86.ecx);

    loadCache();

    // :TODO: what about other x86 platforms that use a different calling convention
    // 
    // jmp ((DecodedOp*)edx)->pfn
    x86.readMem(x86.eax, x86.edx, offsetof(DecodedOp, pfnJitCode));
    x86.jmp(x86.eax);
    return createDynamicExecutableMemory();
}

void X86DynamicCodeGen::preCommitJIT() {
    for (DynamicJump& jmp : x86.jumps) {
        U32 bufferIndex = 0;

        if (!eipToBufferPos.get(jmp.eip, bufferIndex)) {
            return;
        }
        *(U32*)&x86.buffer.data()[jmp.bufferPos] = bufferIndex - jmp.bufferPos - 4;
    }
}

void X86DynamicCodeGen::patch(U8* begin) {
    for (U32 i = 0; i < x86.patch.size(); i++) {
        U32 pos = x86.patch[i];
        U32* value = (U32*)(&begin[pos]);
        *value = *value - (U32)(begin + pos + 4);
    }
}

// :TODO: it would be nice to return the mapped register rather than always copying to tmpReg, but currently the dynamic code assumes in many places that it can clobber the register, like in callHostFunction when it zero extends it
void X86DynamicCodeGen::loadReg(U8 reg, DynReg tmpReg, DynWidth width) {
    bool isHigh8bit = (width == DYN_8bit) && (reg >= 4);

    if (!isHigh8bit && regCache[reg]) {
        // tmpReg can only be 0-3 (see enum DynReg), on x86 those reg can do 8-bit math, the other can't, so for 8-bit, we need to move it into the tmpReg
        if (width == DYN_8bit) {
            x86.mov(X86Asm::Reg32(tmpReg), X86Asm::Reg32(regCache[reg]));
            regUsed[tmpReg] = true;
        } else {
            movToRegFromReg(tmpReg, width, (DynReg)regCache[reg], width, false);
            regUsed[tmpReg] = true;
        }
    } else if (isHigh8bit && regCache[reg - 4]) {
        x86.mov(X86Asm::Reg32(tmpReg), X86Asm::Reg32(regCache[reg - 4]));
        x86.mov(X86Asm::Reg8(tmpReg), X86Asm::Reg8(tmpReg + 4)); // copy AH to AL since this is where the caller expects it
        regUsed[tmpReg] = true;
    } else {
        DynamicCodeGen::loadReg(reg, tmpReg, width);
    }
}

void X86DynamicCodeGen::storeReg(U8 reg, DynReg srcReg, DynWidth width, bool doneWithSrcReg) {
    if (doneWithSrcReg) {
        regUsed[srcReg] = false;
    }
    bool isHigh8bit = (width == DYN_8bit) && (reg >= 4);

    if (!isHigh8bit && regCache[reg]) {
        if (width == DYN_8bit) {
            X86Asm::Reg32 tmpReg(0);

            if (srcReg == 0) {
                tmpReg = X86Asm::Reg32(1);
            }
            if (regUsed[tmpReg.reg]) {
                x86.push(tmpReg);
            }
            x86.mov(tmpReg, X86Asm::Reg32(regCache[reg]));
            x86.mov(X86Asm::Reg8(tmpReg.reg), X86Asm::Reg8(srcReg));
            x86.mov(X86Asm::Reg32(regCache[reg]), tmpReg);
            if (regUsed[tmpReg.reg]) {
                x86.pop(tmpReg);
            }
        } else if (width == DYN_16bit) {
            x86.mov(X86Asm::Reg16(regCache[reg]), X86Asm::Reg16(srcReg));
        } else {
            x86.mov(X86Asm::Reg32(regCache[reg]), X86Asm::Reg32(srcReg));
        }
    } else if (isHigh8bit && regCache[reg - 4]) {
        X86Asm::Reg32 tmpReg(0);

        if (srcReg == 0) {
            tmpReg = X86Asm::Reg32(1);
        }
        if (regUsed[tmpReg.reg]) {
            x86.push(tmpReg);
        }
        x86.mov(tmpReg, X86Asm::Reg32(regCache[reg - 4]));
        x86.mov(X86Asm::Reg8(tmpReg.reg + 4), X86Asm::Reg8(srcReg)); // +4 for high byte (like AH)
        x86.mov(X86Asm::Reg32(regCache[reg - 4]), tmpReg);
        if (regUsed[tmpReg.reg]) {
            x86.pop(tmpReg);
        }
    } 
    else {
        DynamicCodeGen::storeReg(reg, srcReg, width, doneWithSrcReg);
    }
}

void X86DynamicCodeGen::storeReg(U8 reg, DynWidth dstWidth, U32 imm) {
    bool isHigh8bit = (dstWidth == DYN_8bit) && (reg >= 4);

    if (!isHigh8bit && regCache[reg]) {
        if (dstWidth == DYN_8bit) {
            if (regUsed[0]) {
                x86.push(x86.eax);
            }
            x86.mov(x86.eax, X86Asm::Reg32(regCache[reg]));
            x86.mov(x86.al, (U8)imm);
            x86.mov(X86Asm::Reg32(regCache[reg]), x86.eax);
            if (regUsed[0]) {
                x86.pop(x86.eax);
            }
        } else if (dstWidth == DYN_16bit) {
            x86.mov(X86Asm::Reg16(regCache[reg]), (U16)imm);
        } else {
            x86.mov(X86Asm::Reg32(regCache[reg]), imm);
        }
    } else if (isHigh8bit && regCache[reg - 4]) {
        if (regUsed[0]) {
            x86.push(x86.eax);
        }
        x86.mov(x86.eax, X86Asm::Reg32(regCache[reg - 4]));
        x86.mov(x86.ah, (U8)imm);
        x86.mov(X86Asm::Reg32(regCache[reg - 4]), x86.eax);
        if (regUsed[0]) {
            x86.pop(x86.eax);
        }
    } 
    else {
        DynamicCodeGen::storeReg(reg, dstWidth, imm);
    }
}

void X86DynamicCodeGen::loadCache() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i]) {
            DynamicCodeGen::loadReg(i, (DynReg)regCache[i], DYN_32bit);
        }
    }
}

void X86DynamicCodeGen::writeCache() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i]) {
            DynamicCodeGen::storeReg(i, (DynReg)regCache[i], DYN_32bit, false);
        }
    }    
}

DynamicCodeGen* startNewJIT(CPU* cpu) {
    return new X86DynamicCodeGen(cpu);
}

void startNewJIT(CPU* cpu, U32 address, DecodedOp* op) {
    X86DynamicCodeGen data(cpu);
    data.doJIT(address, op);
}

#endif