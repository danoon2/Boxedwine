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
    RegPtr getReg(U8 reg, S8 hint = -1, bool load = true) override;
    RegPtr getReg8(U8 reg) override;
    RegPtr getReadOnlyReg(U8 reg, bool delayed = false, S8 hint = -1) override;
    RegPtr getReadOnlyReg8(U8 reg, bool delayed = false, S8 hint = -1) override;
    RegPtr getTmpReg() override;
    RegPtr getTmpRegWithHint(S8 hint) override;
    RegPtr getTmpReg(U8 reg, bool delayed = false, S8 hint = -1) override;
    RegPtr getTmpReg8(U8 reg, bool delayed = false, S8 hint = -1) override;
    RegPtr getTmpRegForCallResult() override;
    RegPtr getReadOnlySegAddress(U8 reg) override;
    RegPtr getTmpSegAddress(U8 reg) override;
    RegPtr getReadOnlySegValue(U8 reg) override;
    RegPtr getTmpEip() override;
    RegPtr getEip(bool load = true) override;
    RegPtr getReadOnlyFlags() override;
    void setFlags(RegPtr flags, U32 mask) override;
    void pushReg(RegPtr reg) override;
    void popReg(RegPtr reg) override;

    U8 findTmpReg(S8 hint = -1);
    bool isTmpRegAvailable() override;
    void updateFlagsIfNecessary();
    void updateHardwareFlags(U32 flags = FMASK_TEST);

    void read(DynWidth width, RegPtr dest, RegPtr reg, U8 lsl, U32 disp) override;
    void read(DynWidth width, RegPtr dest, RegPtr reg, RegPtr sib, U8 lsl, U32 disp) override;
    void write(DynWidth width, RegPtr reg, U32 disp, RegPtr src) override;
    void write(DynWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, RegPtr src) override;
    void write(DynWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, U32 value) override;
    RegPtr readCPU(DynWidth width, U32 offset) override;
    RegPtr readCPU(DynWidth width, RegPtr sib, U8 lsl, U32 offset) override;
    void writeCPU(DynWidth width, U32 offset, RegPtr src) override;
    void writeCPU(DynWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr src) override;
    void writeCPUValue(DynWidth width, RegPtr sib, U8 lsl, U32 offset, U32 src) override;
    void writeCPUValue(DynWidth width, U32 offset, U32 src) override;

    void IfSmallStack(bool bigJump = false) override;
    void IfLessThan2(DynWidth regWidth, RegPtr reg, U32 value, bool bigJump = false) override;
    void IfNot(DynWidth regWidth, RegPtr reg) override;
    void IfNotCPU(DynWidth regWidth, RegPtr sib, U8 lsl, U32 offset, bool bigJump = false) override;
    void If(DynWidth regWidth, RegPtr reg, bool bigJump = false) override;
    void IfEqual(DynWidth regWidth, RegPtr reg, U32 value) override;
    void IfBitSet2(DynWidth regWidth, RegPtr reg, U32 value, bool bigJump = false) override;
    void IfFlagSet(U32 flags, bool bigJump = false) override;
    void IfNotFlagSet(U32 flags);
    void IfCondition(DynConditional condition) override;
    void JumpIfCondition(DynConditional condition, U32 address) override;
    void setReg(DynConditional condition, RegPtr reg) override;
    void jmp(RegPtr reg) override;    
    U32 MarkJumpLocation() override;
    void Goto(U32 location) override;

    void callHostFunction(void* address, const std::vector<DynParam>& params) override;
    void callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) override;
    void pushParam(const DynParam& param);

    void mov(DynWidth regWidth, RegPtr dest, RegPtr src) override;
    void movValue(DynWidth regWidth, RegPtr dst, U32 imm) override;
    void mov8(RegPtr dest, bool isDestHigh, RegPtr src, bool isSrcHigh) override;
    void movzx(DynWidth dstWidth, RegPtr dest, DynWidth srcWidth, RegPtr src) override;
    void movsx(DynWidth dstWidth, RegPtr dest, DynWidth srcWidth, RegPtr src) override;

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
    void shrdReg(DynWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl, bool checkFlags) override;
    void shrdValue(DynWidth regWidth, RegPtr reg, RegPtr rm, U32 imm, bool checkFlags) override;
    void shlReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void shlValue(DynWidth regWidth, RegPtr reg, U32 immm, bool checkFlags) override;
    void shldReg(DynWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl, bool checkFlags) override;
    void shldValue(DynWidth regWidth, RegPtr reg, RegPtr rm, U32 imm, bool checkFlags) override;
    void sarReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void sarValue(DynWidth regWidth, RegPtr reg, U32 immm, bool checkFlags) override;
    void rolReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void rolValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) override;
    void rorReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void rorValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) override;
    void rclReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void rclValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) override;
    void rcrReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void rcrValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) override;
    void negReg2(DynWidth regWidth, RegPtr reg, bool checkFlags) override;
    void notReg2(DynWidth regWidth, RegPtr reg, bool checkFlags) override;
    void mulReg(DynWidth regWidth, RegPtr reg, bool checkFlags) override;
    void imulReg(DynWidth regWidth, RegPtr reg, bool checkFlags) override;
    void imulRRI(DynWidth regWidth, RegPtr dst, RegPtr src, U32 src2, bool checkFlags) override;
    void imulRR(DynWidth regWidth, RegPtr dst, RegPtr src, bool checkFlags) override;    
    void xaddReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void byteSwapReg32(RegPtr reg) override;
    void btReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void btValue(DynWidth regWidth, RegPtr reg, U32 mask, bool checkFlags) override;
    void btsReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void btsValue(DynWidth regWidth, RegPtr reg, U32 mask, bool checkFlags) override;
    void btrReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void btrValue(DynWidth regWidth, RegPtr reg, U32 mask, bool checkFlags) override;
    void btcReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void btcValue(DynWidth regWidth, RegPtr reg, U32 mask, bool checkFlags) override;
    void bsfReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;
    void bsrReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) override;

    std::array<bool, 8> regUsed2{ 0 };
    DecodedOp* currentOp = nullptr;

    void incrementEip(U32 inc) override;

    void divRegRegWithRemainder2(DynWidth regWidth, RegPtr dest, RegPtr src, RegPtr remainder) override;
    void idivRegRegWithRemainder2(DynWidth regWidth, RegPtr dest, RegPtr src, RegPtr remainder) override;
    void incReg(DynWidth regWidth, RegPtr dest, bool checkFlags) override;
    void decReg(DynWidth regWidth, RegPtr dest, bool checkFlags) override;
    void xchgReg(DynWidth regWidth, RegPtr dest, RegPtr src) override;

    void JumpInBlock(U32 address) override;
    void StartElse(bool bigJump = false) override;
    void EndIf(bool bigJump = false) override;
    void blockExit() override;

    // FPU
    void storeCpuFpuReg(DynFpuReg reg, RegPtr index, DynFpuWidth width = DYN_FPU_64_BIT) override;
    void loadCpuFpuReg(DynFpuReg reg, RegPtr index, DynFpuWidth width = DYN_FPU_64_BIT) override;
    void loadCpuFpuRegConst(DynFpuReg reg, U32 offset) override;
    void loadFpuRegFromInt(DynFpuReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeFpuReg(DynFpuReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp, DynFpuWidth width = DYN_FPU_64_BIT) override;
    RegPtr fpuRegToInt32(DynFpuReg fpuRegSrc, bool truncate) override;
    void fpuRegToInt64(DynFpuReg regDst, DynFpuReg fpuRegSrc, bool truncate) override;
    void fpuRegInt64To64(DynFpuReg regDst, DynFpuReg fpuRegSrc) override;
    void loadFpuReg(DynFpuReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp, DynFpuWidth width = DYN_FPU_64_BIT) override;
    void fpuRegExtend32To64(DynFpuReg dst, DynFpuReg src) override;
    void fpuReg64To32(DynFpuReg dst, DynFpuReg src) override;
    void regToFpuReg(DynFpuReg dst, RegPtr src) override;
    void updateFPURounding() override;
    void restoreFPURounding() override;

    void fpuAdd(DynFpuReg dst, DynFpuReg src) override;
    void fpuMul(DynFpuReg dst, DynFpuReg src) override;
    void fpuSub(DynFpuReg dst, DynFpuReg src) override;
    void fpuDiv(DynFpuReg dst, DynFpuReg src) override;
    void fpuXor(DynFpuReg dst, DynFpuReg src) override;
    void fpuAnd(DynFpuReg dst, DynFpuReg src) override;
    void fpuSqrt(DynFpuReg dst, DynFpuReg src) override;
    void fcompare(DynFpuReg fpuReg1, DynFpuReg fpuReg2, RegPtr ordTags, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) override;

    // MMX
    DynMMXReg getTmpMMX(U8 inUse) override { return inUse == 0 ? (DynMMXReg)1 : (DynMMXReg)0; }
    void loadMMXFromReg(DynMMXReg mmx, RegPtr reg) override;
    void storeCpuMMXReg(DynMMXReg reg, U32 index) override;
    void storeMMXToReg(DynMMXReg mmx, RegPtr reg) override;
    void loadCpuMMXReg(DynMMXReg reg, U32 index) override;
    void loadMMXFromMem32(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void loadMMXFromMem64(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeMMXToMem32(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeMMXToMem64(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
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
    void pextrwRegMmx(RegPtr dst, DynMMXReg src, U8 srcIndex) override;
    void pinsrwMmxReg(DynMMXReg dest, RegPtr src, U8 dstIndex) override;
    void pmaxswMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pmaxubMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pminswMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pminubMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pmovmskbMmxMmx(RegPtr dst, DynMMXReg src) override;
    void pmulhuwMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pshufwMmxMmx(DynMMXReg dst, DynMMXReg src, U8 mask) override;
    void maskmovq(DynMMXReg src, DynMMXReg mask, RegPtr destAddress) override;

    void paddqMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void psubqMmxMmx(DynMMXReg dst, DynMMXReg src) override;
    void pmuludqMmxMmx(DynMMXReg dst, DynMMXReg src) override;

    // SSE
    DynXMMReg getTmpXMM(U8 inUse) override { return inUse == 0 ? (DynXMMReg)1 : (DynXMMReg)0; }
    void storeCpuXMMReg(DynXMMReg reg, U32 index) override;
    void loadCpuXMMReg(DynXMMReg reg, U32 index) override;
    void loadCpuXMMReg64ZeroExtend(DynXMMReg reg, U32 index) override;
    void loadXMMFromMem128(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void loadXMMFromMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void loadLowXMMFromMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void loadHighXMMFromMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void loadXMMFromMem32(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeXMMToMem128(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeXMMToMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeXMMToMem32(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeHighXMMToMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;

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
    void cvtsi2ssXmmR32(DynXMMReg dst, RegPtr src) override;
    void cvtss2siR32Xmm(RegPtr dst, DynXMMReg src) override;
    void cvttps2piMmxXmm(DynMMXReg dst, DynXMMReg src) override;
    void cvttss2siR32Xmm(RegPtr dst, DynXMMReg src) override;
    void movhlpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void movlhpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void movmskpsR32Xmm(RegPtr dst, DynXMMReg src) override;
    void movssXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void shufpsXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) override;
    void unpckhpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void unpcklpsXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cmppsXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) override;
    void cmpssXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) override;
    void comissXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void ucomissXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void sfence() override;
    void stmxcsr(RegPtr address) override;
    void ldmxcsr(RegPtr address) override;

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
    void cvtsd2siR32Xmm(RegPtr dst, DynXMMReg src) override;
    void cvtsd2ssXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvtsi2sdXmmR32(DynXMMReg dst, RegPtr src) override;
    void cvtss2sdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvttpd2dqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvttps2dqXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void cvttsd2siR32Xmm(RegPtr dst, DynXMMReg src) override;
    void movsdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void movupdXmmXmm(DynXMMReg dst, DynXMMReg src) override;
    void movmskpd(RegPtr dst, DynXMMReg src) override;
    void movd(RegPtr dst, DynXMMReg src) override;
    void movd(DynXMMReg dst, RegPtr src) override;
    void movdq2q(DynMMXReg dst, DynXMMReg src) override;
    void movq2dq(DynXMMReg dst, DynMMXReg src) override;

    void maskmovdqu(DynXMMReg dst, DynXMMReg src, RegPtr address) override;
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
    void clflush(RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void pause() override;
    void pextrwR32Xmm(RegPtr dst, DynXMMReg src, U32 imm) override;
    void pinsrwXmmR32(DynXMMReg dst, RegPtr src, U32 imm) override;
    void pmovmskbR32Xmm(RegPtr dst, DynXMMReg src) override;

    // optional override, hopefully faster than the common_ methods
    void dynamic_rdtsc(DecodedOp* op) override;
    void dynamic_arithE32R32_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, std::function<void()> fallback, bool writeReg = false);
    void dynamic_arithE16R16_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, std::function<void()> fallback, bool writeReg = false);
    void dynamic_arithE8R8_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, std::function<void()> fallback, bool writeReg = false);
    void dynamic_arithE32_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback, std::function<void()> fallback);
    void dynamic_arithE16_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback, std::function<void()> fallback);
    void dynamic_arithE8_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback, std::function<void()> fallback);

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

    U32 getBufferSize() override;
    U32 getIfJumpSize() override;    
    U8* getBuffer() override; 

    void preCommitJIT() override;
    void patch(U8* begin) override;
    U8* createStartJITCode() override;

    void loadCache();
    void writeCache();
    void loadHardwareFlags();

    X86Asm x86;
};

void X86DynamicCodeGen::preOp(DecodedOp* op) {
    regUsed2.fill(false);
    currentOp = op;
}

bool X86DynamicCodeGen::isTmpRegAvailable() {
    for (int i = 0; i < 4; i++) {
        if (!regUsed2[i]) {
            return true;
        }
    }
    return false;
}

U8 X86DynamicCodeGen::findTmpReg(S8 hint) {
    if (hint >= 0 && !regUsed2[hint]) {
        regUsed2[hint] = true;
        return (U8)hint;
    }
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
    return getTmpRegWithHint(-1);
}

RegPtr X86DynamicCodeGen::getTmpRegWithHint(S8 hint) {
    return std::shared_ptr<DynReg>(new DynReg(findTmpReg(hint), 0xff), [this](DynReg* p) {
        if (p->isLoaded()) {
            regUsed2[p->hardwareReg()] = false;
        }
        delete p;
        });
}

RegPtr X86DynamicCodeGen::getTmpReg(U8 reg, bool delayed, S8 hint) {    
    if (delayed) {
        auto getTmp = [reg, hint, this]() {
            U8 hardwareReg = findTmpReg(hint);
            if (regCache[reg]) {
                x86.mov(X86Asm::Reg32(hardwareReg), X86Asm::Reg32(regCache[reg]));
            } else {
                x86.readMem(X86Asm::Reg32(hardwareReg), x86.edi, CPU::offsetofReg32(reg));
            }
            return hardwareReg;
            };

        return std::shared_ptr<DynReg>(new DynReg(0xff, reg, getTmp), [this](DynReg* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
            });
    } else {
        RegPtr result = std::shared_ptr<DynReg>(new DynReg(findTmpReg(hint), reg), [this](DynReg* p) {
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
    return std::shared_ptr<DynReg>(new DynReg(reg, 0xff), [this](DynReg* p) {
        if (p->isLoaded()) {
            regUsed2[p->hardwareReg()] = false;
        }
        delete p;
    });
}

RegPtr X86DynamicCodeGen::getTmpReg8(U8 reg, bool delayed, S8 hint) {
    bool isHigh = reg > 3;
    RegPtr result;

    if (isHigh) {
        reg -= 4;
    }
    if (delayed) {
        auto getTmp = [reg, hint, this]() {
            U8 hardwareReg = findTmpReg(hint);
            if (regCache[reg]) {
                x86.mov(X86Asm::Reg32(hardwareReg), X86Asm::Reg32(regCache[reg]));
            } else {
                x86.readMem(X86Asm::Reg32(hardwareReg), x86.edi, CPU::offsetofReg32(reg));
            }
            return hardwareReg;
        };

        result = std::shared_ptr<DynReg>(new DynReg(0xff, reg, isHigh, getTmp), [this](DynReg* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
        });        
    } else {
        result = std::shared_ptr<DynReg>(new DynReg(findTmpReg(hint), reg, isHigh), [this](DynReg* p) {
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

RegPtr X86DynamicCodeGen::getTmpEip() {
    RegPtr result = getTmpReg();
    x86.readMem(X86Asm::Reg32(result->hardwareReg()), x86.edi, offsetof(CPU, eip.u32));
    return result;
}

RegPtr X86DynamicCodeGen::getEip(bool load) {
    RegPtr result = std::shared_ptr<DynReg>(new DynReg(findTmpReg(), 0xff), [this](DynReg* p) {
        x86.writeMem(x86.edi, offsetof(CPU, eip.u32), X86Asm::Reg32(p->hardwareReg()));
        regUsed2[p->hardwareReg()] = false;
        delete p;
        });
    if (load) {
        x86.readMem(X86Asm::Reg32(result->hardwareReg()), x86.edi, offsetof(CPU, eip.u32));
    }
    return result;
}

RegPtr X86DynamicCodeGen::getTmpSegAddress(U8 reg) {
    return getReadOnlySegAddress(reg);
}

RegPtr X86DynamicCodeGen::getReadOnlySegAddress(U8 reg) {
    RegPtr result = getTmpReg();
    x86.readMem(X86Asm::Reg32(result->hardwareReg()), x86.edi, CPU::offsetofSegAddress(reg));
    return result;
}

RegPtr X86DynamicCodeGen::getReadOnlySegValue(U8 reg) {
    RegPtr result = getTmpReg();
    x86.readMem(X86Asm::Reg32(result->hardwareReg()), x86.edi, CPU::offsetofSegValue(reg));
    return result;
}

RegPtr X86DynamicCodeGen::getReg(U8 reg, S8 hint, bool load) {
    if (regCache[reg]) {
        if (hint >= 0 && !regUsed2[hint]) {
            regUsed2[hint] = true;
            RegPtr result = std::shared_ptr<DynReg>(new DynReg(hint, reg), [reg, this](DynReg* p) {
                x86.mov(X86Asm::Reg32(regCache[reg]), X86Asm::Reg32(p->hardwareReg()));
                regUsed2[p->hardwareReg()] = false;
                delete p;
                });
            if (load) {
                x86.mov(X86Asm::Reg32(result->hardwareReg()), X86Asm::Reg32(regCache[reg]));
            }
            return result;
        }
        return std::shared_ptr<DynReg>(new DynReg(regCache[reg], reg), [this](DynReg* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
        });
    } else {        
        RegPtr result = std::shared_ptr<DynReg>(new DynReg(findTmpReg(hint), reg), [this](DynReg* p) {
            x86.writeMem(x86.edi, CPU::offsetofReg32(p->emulatedReg), X86Asm::Reg32(p->hardwareReg()));
            regUsed2[p->hardwareReg()] = false;
            delete p;
            });
        if (load) {
            x86.readMem(X86Asm::Reg32(result->hardwareReg()), x86.edi, CPU::offsetofReg32(reg));
        }
        return result;
    }
}

RegPtr X86DynamicCodeGen::getReg8(U8 reg) {
    bool isHigh = reg > 3;
    if (isHigh) {
        reg -= 4;
    }
    // for 8-bit, only the first 4 registers in x86 can use them
    RegPtr result = std::shared_ptr<DynReg>(new DynReg(findTmpReg(), reg, isHigh), [this](DynReg* p) {
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

RegPtr X86DynamicCodeGen::getReadOnlyReg(U8 reg, bool delayed, S8 hint) {
    if (regCache[reg]) {
        return std::shared_ptr<DynReg>(new DynReg(regCache[reg], reg), [this](DynReg* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
            });
    } else {
        return getTmpReg(reg, delayed, hint);
    }
}

RegPtr X86DynamicCodeGen::getReadOnlyReg8(U8 reg, bool delayed, S8 hint) {
    return getTmpReg8(reg, delayed, hint);
}

static void dynamic_fillFlags(CPU * cpu) {
    cpu->fillFlags();
}

void X86DynamicCodeGen::setFlags(RegPtr flags, U32 mask) {
    if (mask & DF) {
        call_RI(common_setFlags, DYN_32bit, flags, mask);
    } else {
        RegPtr reg = getTmpReg();
        x86.readMem(X86Asm::Reg32(reg->hardwareReg()), x86.edi, offsetof(CPU, flags));
        x86.and_(X86Asm::Reg32(reg->hardwareReg()), ~mask);
        x86.and_(X86Asm::Reg32(flags->hardwareReg()), mask);
        x86.or_(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(flags->hardwareReg()));
        x86.writeMem(x86.edi, offsetof(CPU, flags), X86Asm::Reg32(reg->hardwareReg()));
    }
}

RegPtr X86DynamicCodeGen::getReadOnlyFlags() {
    RegPtr reg = getTmpReg();
    x86.readMem(X86Asm::Reg32(reg->hardwareReg()), x86.edi, offsetof(CPU, flags));
    orValue(DYN_32bit, reg, 2, false);
    andValue(DYN_32bit, reg, 0xFCFFFF, false);
    return reg;
}

void X86DynamicCodeGen::updateHardwareFlags(U32 flags) { 
    if (flags == CF) {
        x86.btMem32(x86.edi, offsetof(CPU, flags), 0);
        return;
    }
    if (flags == ZF) {
        x86.btMem32(x86.edi, offsetof(CPU, flags), 6);
        x86.IfCF();
            x86.cmp(x86.eax, x86.eax); // will set ZF
        x86.Else();
            x86.cmp(x86.edi, 0); // edi should not be 0, it contains CPU, so this will clear ZF
        x86.EndIf();
        return;
    }
    bool eaxPushed = false;

    if (!isTmpRegAvailable()) {
        x86.push(x86.eax);
        eaxPushed = true;
        regUsed2[0] = false;
    }
    RegPtr reg = getTmpRegForCallResult();
    x86.readMem(X86Asm::Reg32(reg->hardwareReg()), x86.edi, offsetof(CPU, flags));
    x86.xchg(X86Asm::Reg8(reg->hardwareReg()), X86Asm::Reg8(reg->hardwareReg() + 4));
    if (flags & OF) {
        x86.shr(X86Asm::Reg8(reg->hardwareReg()), 3);
    }
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
    reg = nullptr;
    if (eaxPushed) {
        x86.pop(x86.eax);
        regUsed2[0] = true;
    }
}

void X86DynamicCodeGen::updateFlagsIfNecessary() {
    U32 neededFlags = currentOp->needsToSetFlags(cpu);
    if (neededFlags) {
        if (neededFlags == CF) {
            x86.IfCF();
                x86.btsMem32(x86.edi, offsetof(CPU, flags), 0);
            x86.Else();
                x86.btrMem32(x86.edi, offsetof(CPU, flags), 0);
            x86.EndIf();
            return;
        }
        if (neededFlags == ZF) {
            x86.IfZF();
                x86.btsMem32(x86.edi, offsetof(CPU, flags), 6);
            x86.Else();
                x86.btrMem32(x86.edi, offsetof(CPU, flags), 6);
            x86.EndIf();
            return;
        }
        if (neededFlags == SF) {
            x86.IfSF();
                x86.btsMem32(x86.edi, offsetof(CPU, flags), 7);
            x86.Else();
                x86.btrMem32(x86.edi, offsetof(CPU, flags), 7);
            x86.EndIf();
            return;
        }
        if (neededFlags == OF) {
            x86.IfOF();
                x86.btsMem32(x86.edi, offsetof(CPU, flags), 11);
            x86.Else();
                x86.btrMem32(x86.edi, offsetof(CPU, flags), 11);
            x86.EndIf();
            return;
        }
        if (neededFlags == PF) {
            x86.IfPF();
                x86.btsMem32(x86.edi, offsetof(CPU, flags), 2);
            x86.Else();
                x86.btrMem32(x86.edi, offsetof(CPU, flags), 2);
            x86.EndIf();
            return;
        }
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

void X86DynamicCodeGen::IfSmallStack(bool bigJump) {
    RegPtr reg = getTmpReg();
    x86.readMem(X86Asm::Reg32(reg->hardwareReg()), x86.edi, offsetof(CPU, stackNotMask));
    If(DYN_32bit, reg, bigJump);
}

void X86DynamicCodeGen::IfLessThan2(DynWidth regWidth, RegPtr reg, U32 value, bool bigJump) {
    if (regWidth == DYN_32bit) {
        x86.IfLessThan(X86Asm::Reg32(reg->hardwareReg()), value, bigJump);
    } else if (regWidth == DYN_16bit) {
        x86.IfLessThan(X86Asm::Reg16(reg->hardwareReg()), (U16)value, bigJump);
    } else if (regWidth == DYN_8bit) {
        x86.IfLessThan(X86Asm::Reg8(get8bitReg(reg)), (U8)value, bigJump);
    } else {
        kpanic_fmt("X86DynamicCodeGen::IfNot unexpected width: %d", (U32)regWidth);
    }
}

void X86DynamicCodeGen::IfNotCPU(DynWidth regWidth, RegPtr sib, U8 lsl, U32 offset, bool bigJump) {
    if (regWidth == DYN_32bit) {
        x86.cmpMem32(x86.edi, X86Asm::Reg32(sib->hardwareReg()), lsl, offset, 0);
    } else if (regWidth == DYN_16bit) {
        x86.cmpMem16(x86.edi, X86Asm::Reg32(sib->hardwareReg()), lsl, offset, 0);
    } else if (regWidth == DYN_8bit) {
        x86.cmpMem8(x86.edi, X86Asm::Reg32(sib->hardwareReg()), lsl, offset, 0);
    } else {
        kpanic_fmt("X86DynamicCodeGen::IfNotCPU unexpected width: %d", (U32)regWidth);
    }
    x86.IfZF(bigJump);
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

void X86DynamicCodeGen::If(DynWidth regWidth, RegPtr reg, bool bigJump) {
    if (regWidth == DYN_32bit) {
        x86.IfNotZero(X86Asm::Reg32(reg->hardwareReg()), bigJump);
    } else if (regWidth == DYN_16bit) {
        x86.IfNotZero(X86Asm::Reg16(reg->hardwareReg()), bigJump);
    } else if (regWidth == DYN_8bit) {
        x86.IfNotZero(X86Asm::Reg8(get8bitReg(reg)), bigJump);
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

void X86DynamicCodeGen::mov8(RegPtr dest, bool isDestHigh, RegPtr src, bool isSrcHigh) {
    x86.mov(X86Asm::Reg8(dest->hardwareReg() + (isDestHigh ? 4 : 0)), X86Asm::Reg8(src->hardwareReg() + (isSrcHigh ? 4 : 0)));
}

void X86DynamicCodeGen::movzx(DynWidth dstWidth, RegPtr dest, DynWidth srcWidth, RegPtr src) {    
    if (dstWidth == DYN_32bit) {
        if (srcWidth == DYN_16bit) {
            x86.movzx(X86Asm::Reg32(dest->hardwareReg()), X86Asm::Reg16(src->hardwareReg()));
        } else if (srcWidth == DYN_8bit) {
            x86.movzx(X86Asm::Reg32(dest->hardwareReg()), X86Asm::Reg8(get8bitReg(src)));
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in X86DynamicCodeGen::movzx %d <= %d", dstWidth, srcWidth);
        }
    } else if (dstWidth == DYN_16bit) {
        if (srcWidth == DYN_8bit) {
            x86.movzx(X86Asm::Reg16(dest->hardwareReg()), X86Asm::Reg8(get8bitReg(src)));
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in X86DynamicCodeGen::movzx %d <= %d", dstWidth, srcWidth);
        }
    } else {
        kpanic_fmt("unknown width in X86DynamicCodeGen::movzx %d <= %d", dstWidth, srcWidth);
    }
}

void X86DynamicCodeGen::movsx(DynWidth dstWidth, RegPtr dest, DynWidth srcWidth, RegPtr src) {
    if (dstWidth == DYN_32bit) {
        if (srcWidth == DYN_16bit) {
            x86.movsx(X86Asm::Reg32(dest->hardwareReg()), X86Asm::Reg16(src->hardwareReg()));
        } else if (srcWidth == DYN_8bit) {
            x86.movsx(X86Asm::Reg32(dest->hardwareReg()), X86Asm::Reg8(get8bitReg(src)));
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in X86DynamicCodeGen::movsx %d <= %d", dstWidth, srcWidth);
        }
    } else if (dstWidth == DYN_16bit) {
        if (srcWidth == DYN_8bit) {
            x86.movsx(X86Asm::Reg16(dest->hardwareReg()), X86Asm::Reg8(get8bitReg(src)));
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in X86DynamicCodeGen::movsx %d <= %d", dstWidth, srcWidth);
        }
    } else {
        kpanic_fmt("unknown width in X86DynamicCodeGen::movsx %d <= %d", dstWidth, srcWidth);
    }
}

void X86DynamicCodeGen::IfFlagSet(U32 flag, bool bigJump) {
    RegPtr reg = getTmpReg();
    x86.readMem(X86Asm::Reg32(reg->hardwareReg()), x86.edi, offsetof(CPU, flags));
    x86.IfBitSet(X86Asm::Reg32(reg->hardwareReg()), flag, bigJump);    
}

void X86DynamicCodeGen::IfNotFlagSet(U32 flag) {
    RegPtr reg = getTmpReg();
    x86.readMem(X86Asm::Reg32(reg->hardwareReg()), x86.edi, offsetof(CPU, flags));
    x86.IfNotBitSet(X86Asm::Reg32(reg->hardwareReg()), flag);

}

void X86DynamicCodeGen::setReg(DynConditional condition, RegPtr reg) {
    U8 hardwareReg = reg->hardwareReg();
    if (reg->isHigh) {
        hardwareReg += 4;
    }
    X86Asm::Reg8 r8(hardwareReg);

    switch (condition) {
    case O:
        x86.testMem32(x86.edi, offsetof(CPU, flags), OF);
        x86.setnz(r8);
        break;
    case NO:
        x86.testMem32(x86.edi, offsetof(CPU, flags), OF);
        x86.setz(r8);
        break;
    case B:
        x86.testMem32(x86.edi, offsetof(CPU, flags), CF);
        x86.setnz(r8);
        break;
    case NB:
        x86.testMem32(x86.edi, offsetof(CPU, flags), CF);
        x86.setz(r8);
        break;
    case Z:
        x86.testMem32(x86.edi, offsetof(CPU, flags), ZF);
        x86.setnz(r8);
        break;
    case NZ:
        x86.testMem32(x86.edi, offsetof(CPU, flags), ZF);
        x86.setz(r8);
        break;
    case BE:
        x86.testMem32(x86.edi, offsetof(CPU, flags), ZF | CF);
        x86.setnz(r8);
        break;
    case NBE:
        x86.testMem32(x86.edi, offsetof(CPU, flags), ZF | CF);
        x86.setz(r8);
        break;
    case S:
        x86.testMem32(x86.edi, offsetof(CPU, flags), SF);
        x86.setnz(r8);
        break;
    case NS:
        x86.testMem32(x86.edi, offsetof(CPU, flags), SF);
        x86.setz(r8);
        break;
    case P:
        x86.testMem32(x86.edi, offsetof(CPU, flags), PF);
        x86.setnz(r8);
        break;
    case NP:
        x86.testMem32(x86.edi, offsetof(CPU, flags), PF);
        x86.setz(r8);
        break;
    case L:
        updateHardwareFlags(SF | OF); // SF != OF
        x86.setl(r8);
        break;
    case NL:
        updateHardwareFlags(SF | OF);
        x86.setnl(r8);
        break;
    case LE:
        updateHardwareFlags(SF | OF | ZF);
        x86.setle(r8);
        break;
    case NLE:
        updateHardwareFlags(SF | OF | ZF);
        x86.setnle(r8);
        break;
    }
}

void X86DynamicCodeGen::JumpIfCondition(DynConditional condition, U32 address) {
    switch (condition) {
    case O:
        x86.testMem32(x86.edi, offsetof(CPU, flags), OF);
        x86.jnz(address);
        break;
    case NO:
        x86.testMem32(x86.edi, offsetof(CPU, flags), OF);
        x86.jz(address);
        break;
    case B:
        x86.testMem32(x86.edi, offsetof(CPU, flags), CF);
        x86.jnz(address);
        break;
    case NB:
        x86.testMem32(x86.edi, offsetof(CPU, flags), CF);
        x86.jz(address);
        break;
    case Z:
        x86.testMem32(x86.edi, offsetof(CPU, flags), ZF);
        x86.jnz(address);
        break;
    case NZ:
        x86.testMem32(x86.edi, offsetof(CPU, flags), ZF);
        x86.jz(address);
        break;
    case BE:
        x86.testMem32(x86.edi, offsetof(CPU, flags), ZF | CF);
        x86.jnz(address);
        break;
    case NBE:
        x86.testMem32(x86.edi, offsetof(CPU, flags), ZF | CF);
        x86.jz(address);
        break;
    case S:
        x86.testMem32(x86.edi, offsetof(CPU, flags), SF);
        x86.jnz(address);
        break;
    case NS:
        x86.testMem32(x86.edi, offsetof(CPU, flags), SF);
        x86.jz(address);
        break;
    case P:
        x86.testMem32(x86.edi, offsetof(CPU, flags), PF);
        x86.jnz(address);
        break;
    case NP:
        x86.testMem32(x86.edi, offsetof(CPU, flags), PF);
        x86.jz(address);
        break;
    case L:
        updateHardwareFlags(SF | OF);
        x86.jl(address);
        break;
    case NL:
        updateHardwareFlags(SF | OF);
        x86.jnl(address);
        break;
    case LE:
        updateHardwareFlags(SF | OF | ZF);
        x86.jle(address);
        break;
    case NLE:
        updateHardwareFlags(SF | OF | ZF);
        x86.jnle(address);
        break;
    }
}

void X86DynamicCodeGen::IfCondition(DynConditional condition) {
    RegPtr tmp = getTmpReg();

    switch (condition) {
    case O:
        IfFlagSet(OF);
        break;
    case NO:
        IfNotFlagSet(OF);
        break;
    case B:
        IfFlagSet(CF);
        break;
    case NB:
        IfNotFlagSet(CF);
        break;
    case Z:
        IfFlagSet(ZF);
        break;
    case NZ:
        IfNotFlagSet(ZF);
        break;
    case BE:
        IfFlagSet(ZF | CF);
        break;
    case NBE:
        IfNotFlagSet(ZF | CF);
        break;
    case S:
        IfFlagSet(SF);
        break;
    case NS:
        IfNotFlagSet(SF);
        break;
    case P:
        IfFlagSet(PF);
        break;
    case NP:
        IfNotFlagSet(PF);
        break;
    case L:
        updateHardwareFlags(SF | OF);
        x86.setl(X86Asm::Reg8(tmp->hardwareReg()));
        If(DYN_8bit, tmp);
        break;
    case NL:
        updateHardwareFlags(SF | OF);
        x86.setnl(X86Asm::Reg8(tmp->hardwareReg()));
        If(DYN_8bit, tmp);
        break;
    case LE:
        updateHardwareFlags(SF | OF | ZF);
        x86.setle(X86Asm::Reg8(tmp->hardwareReg()));
        If(DYN_8bit, tmp);
        break;
    case NLE:
        updateHardwareFlags(SF | OF | ZF);
        x86.setnle(X86Asm::Reg8(tmp->hardwareReg()));
        If(DYN_8bit, tmp);
        break;
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

void X86DynamicCodeGen::pushReg(RegPtr reg) {
    x86.push(X86Asm::Reg32(reg->hardwareReg()));
}

void X86DynamicCodeGen::popReg(RegPtr reg) {
    x86.pop(X86Asm::Reg32(reg->hardwareReg()));
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

    x86.readMem(x86.ecx, x86.edi, offsetof(CPU, lazyFlags));
    x86.sub(x86.ecx, (U32)FLAGS_NONE);
    x86.IfNotZF();
        x86.push(x86.edi);
        x86.call(::dynamic_fillFlags);
        x86.add(x86.esp, 4);
    x86.EndIf();
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

void X86DynamicCodeGen::write(DynWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, U32 value) {
    if (width == DYN_32bit) {
        x86.writeMem(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, value);
    } else if (width == DYN_16bit) {
        x86.writeMem(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, (U16)value);
    } else if (width == DYN_8bit) {
        x86.writeMem(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, (U8)value);
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
        kpanic_fmt("X86DynamicCodeGen::write unexpected width: %d", (U32)width);
    }
}

void X86DynamicCodeGen::xaddReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.xadd(X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(reg->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.xadd(X86Asm::Reg16(rm->hardwareReg()), X86Asm::Reg16(reg->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.xadd(X86Asm::Reg8(get8bitReg(rm)), X86Asm::Reg8(get8bitReg(reg)));
    } else {
        kpanic("X86DynamicCodeGen::xaddReg");
    }
    if (checkFlags) {
        rm = nullptr; // if there is only one reference then letting go of it could help updateFlagsIfNecessary
        updateFlagsIfNecessary();
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

void X86DynamicCodeGen::shldReg(DynWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl, bool checkFlags) {    
    if (reg->hardwareReg() == 1 || rm->hardwareReg() == 1) {
        kpanic("X86DynamicCodeGen::shldReg cl");
    }
    bool ecxPushed = false;
    if (regUsed2[1]) {
        x86.push(x86.ecx);
        regUsed2[1] = false;
        ecxPushed = true;
    }
    if (checkFlags) {
        // if cl is 0, it should not change flags
        If(DYN_8bit, cl);
    }    
    if (regWidth == DYN_32bit) {
        x86.shld(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(cl->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.shld(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()), X86Asm::Reg16(cl->hardwareReg()));
    } else {
        kpanic("X86DynamicCodeGen::shldReg");
    }    
    if (checkFlags) {
        rm = nullptr;
        updateFlagsIfNecessary();
        EndIf();        
    }
    if (ecxPushed) {
        x86.pop(x86.ecx);
        regUsed2[1] = true;
    }
}

void X86DynamicCodeGen::shldValue(DynWidth regWidth, RegPtr reg, RegPtr rm, U32 imm, bool checkFlags) {
    // don't need to check if imm is 0, that was handled in the decoder, if it was 0, the decoder will replace shld with nop

    if (regWidth == DYN_32bit) {
        x86.shld(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.shld(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()), (U16)imm);
    } else {
        kpanic("X86DynamicCodeGen::shldReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::shrdReg(DynWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl, bool checkFlags) {
    if (reg->hardwareReg() == 1 || rm->hardwareReg() == 1) {
        kpanic("X86DynamicCodeGen::shrdReg cl");
    }
    bool ecxPushed = false;
    if (regUsed2[1]) {
        x86.push(x86.ecx);
        regUsed2[1] = false;
        ecxPushed = true;
    }
    if (checkFlags) {
        // if cl is 0, it should not change flags
        If(DYN_8bit, cl);
    }
    if (regWidth == DYN_32bit) {
        x86.shrd(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(cl->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.shrd(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()), X86Asm::Reg16(cl->hardwareReg()));
    } else {
        kpanic("X86DynamicCodeGen::shrdReg");
    }
    if (checkFlags) {
        rm = nullptr;
        updateFlagsIfNecessary();
        EndIf();
    }
    if (ecxPushed) {
        x86.pop(x86.ecx);
        regUsed2[1] = true;
    }
}

void X86DynamicCodeGen::shrdValue(DynWidth regWidth, RegPtr reg, RegPtr rm, U32 imm, bool checkFlags) {
    // don't need to check if imm is 0, that was handled in the decoder, if it was 0, the decoder will replace shrd with nop

    if (regWidth == DYN_32bit) {
        x86.shrd(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.shrd(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()), (U16)imm);
    } else {
        kpanic("X86DynamicCodeGen::shrdReg");
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

void X86DynamicCodeGen::rolReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.rol(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.rol(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.rol(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::rolReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::rolValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.rol(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.rol(X86Asm::Reg16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.rol(X86Asm::Reg8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::rolValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::rorReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.ror(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.ror(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.ror(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::rorReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::rorValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.ror(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.ror(X86Asm::Reg16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.ror(X86Asm::Reg8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::rorValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::rclReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    updateHardwareFlags(CF);
    if (regWidth == DYN_32bit) {
        x86.rcl(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.rcl(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.rcl(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::rclReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::rclValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    updateHardwareFlags(CF);
    if (regWidth == DYN_32bit) {
        x86.rcl(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.rcl(X86Asm::Reg16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.rcl(X86Asm::Reg8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::rclValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::rcrReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    updateHardwareFlags(CF);
    if (regWidth == DYN_32bit) {
        x86.rcr(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.rcr(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.rcr(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg8(get8bitReg(rm)));
    } else {
        kpanic("X86DynamicCodeGen::rcrReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::rcrValue(DynWidth regWidth, RegPtr reg, U32 imm, bool checkFlags) {
    updateHardwareFlags(CF);
    if (regWidth == DYN_32bit) {
        x86.rcr(X86Asm::Reg32(reg->hardwareReg()), imm);
    } else if (regWidth == DYN_16bit) {
        x86.rcr(X86Asm::Reg16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == DYN_8bit) {
        x86.rcr(X86Asm::Reg8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("X86DynamicCodeGen::rcrValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::btReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (checkFlags && currentOp->getNeededFlagsAfter(ZF)) {
        updateHardwareFlags(ZF); // The CF flag contains the value of the selected bit. The ZF flag is unaffected. The OF, SF, AF, and PF flags are undefined.  
    }
    if (regWidth == DYN_32bit) {
        x86.bt(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.bt(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else {
        kpanic("X86DynamicCodeGen::btReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::btValue(DynWidth regWidth, RegPtr reg, U32 mask, bool checkFlags) {
    // convert mask to bit position
    mask = std::countr_zero(mask);

    if (checkFlags && currentOp->getNeededFlagsAfter(ZF)) {
        updateHardwareFlags(ZF); // The CF flag contains the value of the selected bit. The ZF flag is unaffected. The OF, SF, AF, and PF flags are undefined.
    }
    if (regWidth == DYN_32bit) {
        x86.bt(X86Asm::Reg32(reg->hardwareReg()), (U8)mask);
    } else if (regWidth == DYN_16bit) {
        x86.bt(X86Asm::Reg16(reg->hardwareReg()), (U8)mask);
    } else {
        kpanic("X86DynamicCodeGen::btValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::btsReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (checkFlags && currentOp->getNeededFlagsAfter(ZF)) {
        updateHardwareFlags(ZF); // The CF flag contains the value of the selected bit before it is set. The ZF flag is unaffected. The OF, SF, AF, and PF flags are undefined.
    }
    if (regWidth == DYN_32bit) {
        x86.bts(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.bts(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else {
        kpanic("X86DynamicCodeGen::btsReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::btsValue(DynWidth regWidth, RegPtr reg, U32 mask, bool checkFlags) {
    // convert mask to bit position
    mask = std::countr_zero(mask);

    if (checkFlags && currentOp->getNeededFlagsAfter(ZF)) {
        updateHardwareFlags(ZF); // The CF flag contains the value of the selected bit before it is set. The ZF flag is unaffected. The OF, SF, AF, and PF flags are undefined.
    }
    if (regWidth == DYN_32bit) {
        x86.bts(X86Asm::Reg32(reg->hardwareReg()), (U8)mask);
    } else if (regWidth == DYN_16bit) {
        x86.bts(X86Asm::Reg16(reg->hardwareReg()), (U8)mask);
    } else {
        kpanic("X86DynamicCodeGen::btsValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::btrReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (checkFlags && currentOp->getNeededFlagsAfter(ZF)) {
        updateHardwareFlags(ZF); // The CF flag contains the value of the selected bit before it is cleared. The ZF flag is unaffected. The OF, SF, AF, and PF flags are undefined.
    }
    if (regWidth == DYN_32bit) {
        x86.btr(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.btr(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else {
        kpanic("X86DynamicCodeGen::btrReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::btrValue(DynWidth regWidth, RegPtr reg, U32 mask, bool checkFlags) {
    // convert mask to bit position
    mask = std::countr_zero(mask);

    if (checkFlags && currentOp->getNeededFlagsAfter(ZF)) {
        updateHardwareFlags(ZF); // The CF flag contains the value of the selected bit before it is cleared.The ZF flag is unaffected.The OF, SF, AF, and PF flags are undefined.
    }
    if (regWidth == DYN_32bit) {
        x86.btr(X86Asm::Reg32(reg->hardwareReg()), (U8)mask);
    } else if (regWidth == DYN_16bit) {
        x86.btr(X86Asm::Reg16(reg->hardwareReg()), (U8)mask);
    } else {
        kpanic("X86DynamicCodeGen::btrValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::btcReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (checkFlags && currentOp->getNeededFlagsAfter(ZF)) {
        updateHardwareFlags(ZF); // The CF flag contains the value of the selected bit before it is complemented. The ZF flag is unaffected. The OF, SF, AF, and PF flags are undefined.
    }
    if (regWidth == DYN_32bit) {
        x86.btc(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.btc(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else {
        kpanic("X86DynamicCodeGen::btcReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::btcValue(DynWidth regWidth, RegPtr reg, U32 mask, bool checkFlags) {
    // convert mask to bit position
    mask = std::countr_zero(mask);

    if (checkFlags && currentOp->getNeededFlagsAfter(ZF)) {
        updateHardwareFlags(ZF); // The CF flag contains the value of the selected bit before it is complemented. The ZF flag is unaffected. The OF, SF, AF, and PF flags are undefined.
    }
    if (regWidth == DYN_32bit) {
        x86.btc(X86Asm::Reg32(reg->hardwareReg()), (U8)mask);
    } else if (regWidth == DYN_16bit) {
        x86.btc(X86Asm::Reg16(reg->hardwareReg()), (U8)mask);
    } else {
        kpanic("X86DynamicCodeGen::btcValue");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::bsfReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.bsf(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.bsf(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else {
        kpanic("X86DynamicCodeGen::bsfReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::bsrReg(DynWidth regWidth, RegPtr reg, RegPtr rm, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.bsr(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(rm->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.bsr(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg16(rm->hardwareReg()));
    } else {
        kpanic("X86DynamicCodeGen::bsrReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::storeCpuXMMReg(DynXMMReg reg, U32 index) {
    x86.movaps(x86.edi, index * 16 + offsetof(CPU, xmm), X86Asm::XMM(reg));
}

void X86DynamicCodeGen::storeXMMToMem128(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movups(X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, X86Asm::XMM(reg));
}

void X86DynamicCodeGen::storeXMMToMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movlps(X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, X86Asm::XMM(reg));
}

void X86DynamicCodeGen::storeXMMToMem32(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movss(X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, X86Asm::XMM(reg));
}

void X86DynamicCodeGen::storeHighXMMToMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movhps(X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, X86Asm::XMM(reg));
}

void X86DynamicCodeGen::loadCpuXMMReg(DynXMMReg reg, U32 index) {
    x86.movaps(X86Asm::XMM(reg), x86.edi, index * 16 + offsetof(CPU, xmm));
}

void X86DynamicCodeGen::loadCpuXMMReg64ZeroExtend(DynXMMReg reg, U32 index) {
    x86.movq(X86Asm::XMM(reg), x86.edi, index * 16 + offsetof(CPU, xmm));
}

void X86DynamicCodeGen::loadXMMFromMem128(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movups(X86Asm::XMM(reg), X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
}

void X86DynamicCodeGen::loadXMMFromMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movq(X86Asm::XMM(reg), X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
}

void X86DynamicCodeGen::loadLowXMMFromMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movlps(X86Asm::XMM(reg), X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
}

void X86DynamicCodeGen::loadHighXMMFromMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movhps(X86Asm::XMM(reg), X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
}

void X86DynamicCodeGen::loadXMMFromMem32(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movss(X86Asm::XMM(reg), X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
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

void X86DynamicCodeGen::pextrwRegMmx(RegPtr dst, DynMMXReg src, U8 srcIndex) {
    x86.pextrw(X86Asm::Reg32(dst->hardwareReg()), X86Asm::XMM(src), srcIndex);
}

void X86DynamicCodeGen::pinsrwMmxReg(DynMMXReg dst, RegPtr src, U8 dstIndex) {
    x86.pinsrw(X86Asm::XMM(dst), X86Asm::Reg32(src->hardwareReg()), dstIndex);
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

void X86DynamicCodeGen::pmovmskbMmxMmx(RegPtr dst, DynMMXReg src) {
    x86.pmovmskb(X86Asm::Reg32(dst->hardwareReg()), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pmulhuwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmulhuw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::pshufwMmxMmx(DynMMXReg dst, DynMMXReg src, U8 order) {
    x86.pshuflw(X86Asm::XMM(dst), X86Asm::XMM(src), order);
}

void X86DynamicCodeGen::maskmovq(DynMMXReg src, DynMMXReg mask, RegPtr destAddress) {
    x86.push(x86.edi);
    x86.mov(x86.edi, X86Asm::Reg32(destAddress->hardwareReg()));
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

void X86DynamicCodeGen::cvtsi2ssXmmR32(DynXMMReg dst, RegPtr src) {
    x86.cvtsi2ss(X86Asm::XMM(dst), X86Asm::Reg32(src->hardwareReg()));
}

void X86DynamicCodeGen::cvtss2siR32Xmm(RegPtr dst, DynXMMReg src) {
    x86.cvtss2si(X86Asm::Reg32(dst->hardwareReg()), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvttps2piMmxXmm(DynMMXReg dst, DynXMMReg src) {
    x86.cvttps2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvttss2siR32Xmm(RegPtr dst, DynXMMReg src) {
    x86.cvttss2si(X86Asm::Reg32(dst->hardwareReg()), X86Asm::XMM(src));
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
    RegPtr flags = getTmpReg();

    x86.pushFlags();
    x86.pop(X86Asm::Reg32(flags->hardwareReg()));
    x86.and_(X86Asm::Reg32(flags->hardwareReg()), FMASK_TEST);

    //this->andCPUFlagsImm(~FMASK_TEST, tmpReg2);
    RegPtr reg = readCPU(DYN_32bit, offsetof(CPU, flags));
    andValue(DYN_32bit, reg, ~FMASK_TEST, false);
    orReg(DYN_32bit, reg, flags, false);
    writeCPU(DYN_32bit, offsetof(CPU, flags), reg);
}

void X86DynamicCodeGen::comissXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.comiss(X86Asm::XMM(dst), X86Asm::XMM(src));
    loadHardwareFlags();
}

void X86DynamicCodeGen::ucomissXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.ucomiss(X86Asm::XMM(dst), X86Asm::XMM(src));
    loadHardwareFlags();
}

void X86DynamicCodeGen::stmxcsr(RegPtr address) {
    x86.stmxcsr(X86Asm::Reg32(address->hardwareReg()), 0);
}

void X86DynamicCodeGen::ldmxcsr(RegPtr address) {
    x86.ldmxcsr(X86Asm::Reg32(address->hardwareReg()), 0);
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

void X86DynamicCodeGen::movmskpsR32Xmm(RegPtr dst, DynXMMReg src) {
    x86.movmskps(X86Asm::Reg32(dst->hardwareReg()), X86Asm::XMM(src));
}

void X86DynamicCodeGen::loadMMXFromReg(DynMMXReg dst, RegPtr src) {
    x86.movd(X86Asm::XMM(dst), X86Asm::Reg32(src->hardwareReg()));
}

void X86DynamicCodeGen::storeCpuMMXReg(DynMMXReg reg, U32 index) {
    x86.movq(x86.edi, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif), X86Asm::XMM(reg));
}

void X86DynamicCodeGen::storeMMXToReg(DynMMXReg src, RegPtr dst) {
    x86.movd(X86Asm::Reg32(dst->hardwareReg()), X86Asm::XMM(src));
}

void X86DynamicCodeGen::loadCpuMMXReg(DynMMXReg reg, U32 index) {
    x86.movq(X86Asm::XMM(reg), x86.edi, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif));
}

void X86DynamicCodeGen::loadMMXFromMem32(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movd(X86Asm::XMM(reg), X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
}

void X86DynamicCodeGen::loadMMXFromMem64(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movq(X86Asm::XMM(reg), X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
}

void X86DynamicCodeGen::storeMMXToMem32(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movd(X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, X86Asm::XMM(reg));
}

void X86DynamicCodeGen::storeMMXToMem64(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movq(X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, X86Asm::XMM(reg));
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

void X86DynamicCodeGen::cvtsd2siR32Xmm(RegPtr dst, DynXMMReg src) {
    x86.cvtsd2si(X86Asm::Reg32(dst->hardwareReg()), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtsd2ssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtsd2ss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::cvtsi2sdXmmR32(DynXMMReg dst, RegPtr src) {
    x86.cvtsi2sd(X86Asm::XMM(dst), X86Asm::Reg32(src->hardwareReg()));
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

void X86DynamicCodeGen::cvttsd2siR32Xmm(RegPtr dst, DynXMMReg src) {
    x86.cvttsd2si(X86Asm::Reg32(dst->hardwareReg()), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.movsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movupdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.movdqu(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movmskpd(RegPtr dst, DynXMMReg src) {
    x86.movmskpd(X86Asm::Reg32(dst->hardwareReg()), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movd(RegPtr dst, DynXMMReg src) {
    x86.movd(X86Asm::Reg32(dst->hardwareReg()), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movd(DynXMMReg dst, RegPtr src) {
    x86.movd(X86Asm::XMM(dst), X86Asm::Reg32(src->hardwareReg()));
}

void X86DynamicCodeGen::movdq2q(DynMMXReg dst, DynXMMReg src) {
    x86.movq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::movq2dq(DynXMMReg dst, DynMMXReg src) {
    x86.movq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::maskmovdqu(DynXMMReg src, DynXMMReg mask, RegPtr address) {
    x86.push(x86.edi);
    x86.mov(x86.edi, X86Asm::Reg32(address->hardwareReg()));
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

void X86DynamicCodeGen::clflush(RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.clflush(X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
}

void X86DynamicCodeGen::pause() {
    x86.pause();
}

void X86DynamicCodeGen::pextrwR32Xmm(RegPtr dst, DynXMMReg src, U32 imm) {
    x86.pextrw(X86Asm::Reg32(dst->hardwareReg()), X86Asm::XMM(src), (U8)imm);
}

void X86DynamicCodeGen::pinsrwXmmR32(DynXMMReg dst, RegPtr src, U32 imm) {
    x86.pinsrw(X86Asm::XMM(dst), X86Asm::Reg32(src->hardwareReg()), (U8)imm);
}

void X86DynamicCodeGen::pmovmskbR32Xmm(RegPtr dst, DynXMMReg src) {
    x86.pmovmskb(X86Asm::Reg32(dst->hardwareReg()), X86Asm::XMM(src));
}

void X86DynamicCodeGen::updateFPURounding() {
    x86.stmxcsr(x86.edi, offsetof(CPU, sseControlStateTmp));

    RegPtr sse = readCPU(DYN_32bit, offsetof(CPU, sseControlStateTmp));
    RegPtr fpu = readCPU(DYN_32bit, offsetof(CPU, fpu.round));

    andValue(DYN_32bit, sse, ~0x6000, false); // clear rounding
    shlValue(DYN_32bit, fpu, 13, false);
    orReg(DYN_32bit, sse, fpu, false); // set rounding in SSE

    // there is no way to set sse rounding from a register
    writeCPU(DYN_32bit, offsetof(CPU, sseControlStateTmp2), sse);

    x86.ldmxcsr(x86.edi, offsetof(CPU, sseControlStateTmp2));
}

void X86DynamicCodeGen::restoreFPURounding() {
    x86.ldmxcsr(x86.edi, offsetof(CPU, sseControlStateTmp));
};

void X86DynamicCodeGen::storeCpuFpuReg(DynFpuReg reg, RegPtr index, DynFpuWidth width) {
    x86.movsd(x86.edi, X86Asm::Reg32(index->hardwareReg()), 3, offsetof(CPU, fpu.regCache[0].d), X86Asm::XMM(reg));
}

void X86DynamicCodeGen::loadCpuFpuReg(DynFpuReg reg, RegPtr index, DynFpuWidth width) {
    x86.movsd(X86Asm::XMM(reg), x86.edi, X86Asm::Reg32(index->hardwareReg()), 3, offsetof(CPU, fpu.regCache[0].d));
}

void X86DynamicCodeGen::loadCpuFpuRegConst(DynFpuReg reg, U32 offset) {
    x86.movsd(X86Asm::XMM(reg), x86.edi, offset);
}

RegPtr X86DynamicCodeGen::fpuRegToInt32(DynFpuReg fpuRegSrc, bool truncate) {
    RegPtr result = getTmpReg();
    if (truncate) {
        x86.cvttsd2si(result->hardwareReg(), X86Asm::XMM(fpuRegSrc));
    } else {
        x86.cvtsd2si(result->hardwareReg(), X86Asm::XMM(fpuRegSrc));
    }
    return result;
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

void X86DynamicCodeGen::storeFpuReg(DynFpuReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        x86.movsd(X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, X86Asm::XMM(reg));
    } else {
        x86.movss(X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp, X86Asm::XMM(reg));
    }
}

void X86DynamicCodeGen::loadFpuReg(DynFpuReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        x86.movsd(X86Asm::XMM(reg), X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
    } else {
        x86.movss(X86Asm::XMM(reg), X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
    }
}

void X86DynamicCodeGen::fpuRegExtend32To64(DynFpuReg dst, DynFpuReg src) {
    x86.cvtss2sd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::fpuReg64To32(DynFpuReg dst, DynFpuReg src) {
    x86.cvtsd2ss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void X86DynamicCodeGen::loadFpuRegFromInt(DynFpuReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.cvtsi2sd(X86Asm::XMM(reg), X86Asm::Reg32(rm->hardwareReg()), X86Asm::Reg32(sib->hardwareReg()), lsl, disp);
}

void X86DynamicCodeGen::regToFpuReg(DynFpuReg dst, RegPtr src) {
    x86.cvtsi2sd(X86Asm::XMM(dst), X86Asm::Reg32(src->hardwareReg()));
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

void X86DynamicCodeGen::fcompare(DynFpuReg fpuReg1, DynFpuReg fpuReg2, RegPtr ordTags, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) {
    subValue(DYN_8bit, ordTags, TAG_Empty, false);
    IfNot(DYN_8bit, ordTags);
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

void X86DynamicCodeGen::jmp(RegPtr reg) {
    x86.jmp(X86Asm::Reg32(reg->hardwareReg()));
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

void X86DynamicCodeGen::divRegRegWithRemainder2(DynWidth regWidth, RegPtr dest, RegPtr src, RegPtr remainder) {
    if (dest->hardwareReg() != 0) {
        kpanic("X86DynamicCodeGen::divRegRegWithRemainder dest to be EAX");
    }
    if (remainder->hardwareReg() != 2) {
        kpanic("X86DynamicCodeGen::divRegRegWithRemainder remainder to be EDX");
    }
    if (regWidth == DYN_16bit) {
        x86.xor_(x86.dx, x86.dx); // 16-bit div on x86 makes a 32-bit word from DX and AX, then divides that by the source
        x86.div(X86Asm::Reg16(src->hardwareReg()));
    } else if (regWidth == DYN_32bit) {
        x86.xor_(x86.edx, x86.edx); // 32-bit div on x86 makes a 64-bit word from EDX and EAX, then divides that by the source
        x86.div(X86Asm::Reg32(src->hardwareReg()));
    } else {
        kpanic("X86DynamicCodeGen::divRegRegWithRemainder");
    }
}

void X86DynamicCodeGen::idivRegRegWithRemainder2(DynWidth regWidth, RegPtr dest, RegPtr src, RegPtr remainder) {
    if (dest->hardwareReg() != 0) {
        kpanic("X86DynamicCodeGen::idivRegRegWithRemainder dest to be EAX");
    }
    if (remainder->hardwareReg() != 2) {
        kpanic("X86DynamicCodeGen::idivRegRegWithRemainder remainder to be EDX");
    }
    if (regWidth == DYN_16bit) {
        // sign extend ax into dx, sinc idiv uses dx:ax
        x86.mov(x86.dx, x86.ax);
        x86.sar(x86.dx, 15);
        x86.idiv(X86Asm::Reg16(src->hardwareReg()));
    } else if (regWidth == DYN_32bit) {
        // sign extend eax into edx, sinc idiv uses edx:eax
        x86.mov(x86.edx, x86.eax);
        x86.sar(x86.edx, 31);
        x86.idiv(X86Asm::Reg32(src->hardwareReg()));
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

void X86DynamicCodeGen::mulReg(DynWidth regWidth, RegPtr reg, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        // EDX:EAX = (U64)EAX * src;
        if (regUsed2[0] || regUsed2[2]) {
            kpanic("X86DynamicCodeGen::mulReg 32");
        }
        RegPtr eax = getReg(0);
        regUsed2[2] = true;
        regUsed2[0] = true;
        x86.mov(x86.eax, X86Asm::Reg32(eax->hardwareReg()));
        x86.mulEax(X86Asm::Reg32(reg->hardwareReg()));
        x86.mov(X86Asm::Reg32(eax->hardwareReg()), x86.eax);
        x86.mov(X86Asm::Reg32(getReg(2)->hardwareReg()), x86.edx);
        regUsed2[2] = false;
        regUsed2[0] = false;
    } else if (regWidth == DYN_16bit) {
        // DX:AX = AX * src;
        if (regUsed2[0] || regUsed2[2]) {
            kpanic("X86DynamicCodeGen::mulReg 16");
        }
        RegPtr eax = getReg(0);
        regUsed2[2] = true;
        regUsed2[0] = true;
        x86.mov(x86.eax, X86Asm::Reg32(eax->hardwareReg()));
        x86.mulAx(X86Asm::Reg16(reg->hardwareReg()));
        x86.mov(X86Asm::Reg16(eax->hardwareReg()), x86.ax);
        x86.mov(X86Asm::Reg16(getReg(2)->hardwareReg()), x86.dx);
        regUsed2[2] = false;
        regUsed2[0] = false;
    } else if (regWidth == DYN_8bit) {
        // AX = AL * src;
        if (regUsed2[0]) {
            kpanic("X86DynamicCodeGen::mulReg 8");
        }
        RegPtr eax = getReg(0);
        x86.mov(x86.ax, X86Asm::Reg16(eax->hardwareReg()));
        x86.mulAl(X86Asm::Reg8(get8bitReg(reg)));
        x86.mov(X86Asm::Reg16(eax->hardwareReg()), x86.ax);
    } else {
        kpanic("X86DynamicCodeGen::mulReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::imulRR(DynWidth regWidth, RegPtr dst, RegPtr src, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.imul(X86Asm::Reg32(dst->hardwareReg()), X86Asm::Reg32(src->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.imul(X86Asm::Reg16(dst->hardwareReg()), X86Asm::Reg16(src->hardwareReg()));
    } else {
        kpanic("X86DynamicCodeGen::imulRR");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::incReg(DynWidth regWidth, RegPtr reg, bool checkFlags) {
    if (checkFlags && currentOp->getNeededFlagsAfter(CF)) {
        // inc doesn't update CF so we need to preserve it
        updateHardwareFlags(CF);
    }
    if (regWidth == DYN_32bit) {
        x86.inc(X86Asm::Reg32(reg->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.inc(X86Asm::Reg16(reg->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.inc(X86Asm::Reg8(get8bitReg(reg)));
    } else {
        kpanic("X86DynamicCodeGen::incReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::decReg(DynWidth regWidth, RegPtr reg, bool checkFlags) {
    if (checkFlags && currentOp->getNeededFlagsAfter(CF)) {
        // dec doesn't update CF so we need to preserve it
        updateHardwareFlags(CF);
    }
    if (regWidth == DYN_32bit) {
        x86.dec(X86Asm::Reg32(reg->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.dec(X86Asm::Reg16(reg->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.dec(X86Asm::Reg8(get8bitReg(reg)));
    } else {
        kpanic("X86DynamicCodeGen::decReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::xchgReg(DynWidth regWidth, RegPtr dest, RegPtr src) {
    if (regWidth == DYN_32bit) {
        x86.xchg(X86Asm::Reg32(dest->hardwareReg()), X86Asm::Reg32(src->hardwareReg()));
    } else if (regWidth == DYN_16bit) {
        x86.xchg(X86Asm::Reg16(dest->hardwareReg()), X86Asm::Reg16(src->hardwareReg()));
    } else if (regWidth == DYN_8bit) {
        x86.xchg(X86Asm::Reg8(get8bitReg(dest)), X86Asm::Reg8(get8bitReg(src)));
    } else {
        kpanic("X86DynamicCodeGen::xchgReg");
    }
}

void X86DynamicCodeGen::imulRRI(DynWidth regWidth, RegPtr dst, RegPtr src, U32 src2, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        x86.imul(X86Asm::Reg32(dst->hardwareReg()), X86Asm::Reg32(src->hardwareReg()), src2);
    } else if (regWidth == DYN_16bit) {
        x86.imul(X86Asm::Reg16(dst->hardwareReg()), X86Asm::Reg16(src->hardwareReg()), src2);
    } else {
        kpanic("X86DynamicCodeGen::imulRRI");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::imulReg(DynWidth regWidth, RegPtr reg, bool checkFlags) {
    if (regWidth == DYN_32bit) {
        // EDX:EAX = (S64)((S32)EAX) * ((S32)(src));
        if (regUsed2[0] || regUsed2[2]) {
            kpanic("X86DynamicCodeGen::imulReg 32");
        }
        RegPtr eax = getReg(0);
        regUsed2[2] = true;
        regUsed2[0] = true;
        x86.mov(x86.eax, X86Asm::Reg32(eax->hardwareReg()));
        x86.imulEax(X86Asm::Reg32(reg->hardwareReg()));
        x86.mov(X86Asm::Reg32(eax->hardwareReg()), x86.eax);
        x86.mov(X86Asm::Reg32(getReg(2)->hardwareReg()), x86.edx);
        regUsed2[2] = false;
        regUsed2[0] = false;
    } else if (regWidth == DYN_16bit) {
        // DX:AX = (S32)((S16)AX) * (S16)src;
        if (regUsed2[0] || regUsed2[2]) {
            kpanic("X86DynamicCodeGen::imulReg 16");
        }
        RegPtr eax = getReg(0);
        regUsed2[2] = true;
        regUsed2[0] = true;
        x86.mov(x86.eax, X86Asm::Reg32(eax->hardwareReg()));
        x86.imulAx(X86Asm::Reg16(reg->hardwareReg()));
        x86.mov(X86Asm::Reg16(eax->hardwareReg()), x86.ax);
        x86.mov(X86Asm::Reg16(getReg(2)->hardwareReg()), x86.dx);
        regUsed2[2] = false;
        regUsed2[0] = false;
    } else if (regWidth == DYN_8bit) {
        // AX = (S16)((S8)AL) * (S8)(src);
        if (regUsed2[0]) {
            kpanic("X86DynamicCodeGen::imulReg 8");
        }
        RegPtr eax = getReg(0);
        x86.mov(x86.ax, X86Asm::Reg16(eax->hardwareReg()));
        x86.imulAl(X86Asm::Reg8(get8bitReg(reg)));
        x86.mov(X86Asm::Reg16(eax->hardwareReg()), x86.ax);
    } else {
        kpanic("X86DynamicCodeGen::imulReg");
    }
    if (checkFlags) {
        updateFlagsIfNecessary();
    }
}

void X86DynamicCodeGen::byteSwapReg32(RegPtr reg) {
    x86.bswap(X86Asm::Reg32(reg->hardwareReg()));
}

RegPtr X86DynamicCodeGen::readCPU(DynWidth width, U32 offset) {
    RegPtr reg = getTmpReg();

    // mov reg, [edi+srcOffset]    
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(reg->hardwareReg()), x86.edi, offset);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(reg->hardwareReg()), x86.edi, offset);
    } else if (width == DYN_8bit) {
        x86.readMem(X86Asm::Reg8(get8bitReg(reg)), x86.edi, offset);
    } else {
        kpanic_fmt("unknown dstWidth in X86DynamicCodeGen::readCPU %d", width);
    }
    return reg;
}

RegPtr X86DynamicCodeGen::readCPU(DynWidth width, RegPtr sib, U8 lsl, U32 offset) {
    RegPtr reg = getTmpReg();
 
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(reg->hardwareReg()), x86.edi, X86Asm::Reg32(sib->hardwareReg()), lsl, offset);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(reg->hardwareReg()), x86.edi, X86Asm::Reg32(sib->hardwareReg()), lsl, offset);
    } else if (width == DYN_8bit) {
        x86.readMem(X86Asm::Reg8(get8bitReg(reg)), x86.edi, X86Asm::Reg32(sib->hardwareReg()), lsl, offset);
    } else {
        kpanic_fmt("unknown dstWidth in X86DynamicCodeGen::readCPU %d", width);
    }
    return reg;
}

void X86DynamicCodeGen::writeCPUValue(DynWidth width, RegPtr sib, U8 lsl, U32 offset, U32 src) {
    if (width == DYN_32bit) {
        x86.writeMem(x86.edi, X86Asm::Reg32(sib->hardwareReg()), lsl, offset, src);
    } else if (width == DYN_16bit) {
        x86.writeMem(x86.edi, X86Asm::Reg32(sib->hardwareReg()), lsl, offset, (U16)src);
    } else if (width == DYN_8bit) {
        x86.writeMem(x86.edi, X86Asm::Reg32(sib->hardwareReg()), lsl, offset, (U8)src);
    } else {
        kpanic_fmt("unknown dstWidth in X86DynamicCodeGen::writeCPUValue %d", width);
    }
}

void X86DynamicCodeGen::writeCPUValue(DynWidth width, U32 offset, U32 src) {
    if (width == DYN_32bit) {
        x86.writeMem(x86.edi, offset, src);
    } else if (width == DYN_16bit) {
        x86.writeMem(x86.edi, offset, (U16)src);
    } else if (width == DYN_8bit) {
        x86.writeMem(x86.edi, offset, (U8)src);
    } else {
        kpanic_fmt("unknown dstWidth in X86DynamicCodeGen::writeCPUValue %d", width);
    }
}

void X86DynamicCodeGen::writeCPU(DynWidth width, U32 offset, RegPtr src) {
    if (width == DYN_32bit) {
        x86.writeMem(x86.edi, offset, X86Asm::Reg32(src->hardwareReg()));
    } else if (width == DYN_16bit) {
        x86.writeMem(x86.edi, offset, X86Asm::Reg16(src->hardwareReg()));
    } else if (width == DYN_8bit) {
        x86.writeMem(x86.edi, offset, X86Asm::Reg8(src->hardwareReg()));
    } else {
        kpanic_fmt("unknown dstWidth in X86DynamicCodeGen::writeCPU %d", width);
    }
}

void X86DynamicCodeGen::writeCPU(DynWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr src) {
    if (width == DYN_32bit) {
        x86.writeMem(x86.edi, X86Asm::Reg32(sib->hardwareReg()), lsl, offset, X86Asm::Reg32(src->hardwareReg()));
    } else if (width == DYN_16bit) {
        x86.writeMem(x86.edi, X86Asm::Reg32(sib->hardwareReg()), lsl, offset, X86Asm::Reg16(src->hardwareReg()));
    } else if (width == DYN_8bit) {
        x86.writeMem(x86.edi, X86Asm::Reg32(sib->hardwareReg()), lsl, offset, X86Asm::Reg8(src->hardwareReg()));
    } else {
        kpanic_fmt("unknown dstWidth in X86DynamicCodeGen::writeCPU %d", width);
    }
}

void X86DynamicCodeGen::JumpInBlock(U32 address) {
    x86.jmp(address);
}

void X86DynamicCodeGen::StartElse(bool bigJump) {
    x86.Else(bigJump);
}

void X86DynamicCodeGen::EndIf(bool bigJump) {
    x86.EndIf(bigJump);
}

U32 X86DynamicCodeGen::MarkJumpLocation() {
    return (U32)x86.buffer.size();
}

void X86DynamicCodeGen::Goto(U32 location) {
    U32 amount = location - x86.buffer.size() - 1;

    if (amount > 127) {
        amount = location - x86.buffer.size() - 5;
        x86.goto32(amount);
    } else {
        x86.goto8(amount);
    }
}

void X86DynamicCodeGen::dynamic_rdtsc(DecodedOp* op) {
    x86.rdtsc();
    x86.writeMem(x86.edi, CPU::offsetofReg32(0), x86.eax);
    x86.writeMem(x86.edi, CPU::offsetofReg32(2), x86.edx);
    incrementEip(op->len);
}

void X86DynamicCodeGen::dynamic_cmpxchg8b_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_64bit, calculateEaa(op), nullptr, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(PF | SF | AF | CF | OF)) { // The ZF flag is set if the destination operand and EDX:EAX are equal; otherwise it is cleared. The CF, PF, AF, SF, and OF flags are unaffected.
            updateHardwareFlags();
        }
        writeCache();
        this->x86.mov(x86.ebp, X86Asm::Reg32(addressReg->hardwareReg()));
        this->x86.mov(x86.esi, X86Asm::Reg32(offsetReg->hardwareReg()));
        for (int i = 0; i < 4; i++) {
            x86.readMem(X86Asm::Reg32(i), x86.edi, CPU::offsetofReg32(i));
        }
        this->x86.cmpxchg8b(x86.esi, x86.ebp, 0, 0);
        for (int i = 0; i < 4; i++) {
            x86.writeMem(x86.edi, CPU::offsetofReg32(i), X86Asm::Reg32(i));
        }
        loadCache();
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cmpxchg8b_lock(op);
    }, true);
}
void X86DynamicCodeGen::dynamic_cmpxchge32r32_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr eax = getReg(0, 0);
        if (eax->hardwareReg() != 0) {
            kpanic("X86DynamicCodeGen::dynamic_cmpxchge32r32_lock");
        }
        RegPtr reg;

        if (op->reg == 0) {
            reg = eax;
        } else {
            reg = getReadOnlyReg(op->reg);
        }
        this->x86.cmpxchg(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0);
        reg = nullptr;
        eax = nullptr;
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cmpxchge32r32_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_cmpxchge16r16_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_16bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr eax = getReg(0, 0);
        if (eax->hardwareReg() != 0) {
            kpanic("X86DynamicCodeGen::dynamic_cmpxchge32r32_lock");
        }
        RegPtr reg;

        if (op->reg == 0) {
            reg = eax;
        } else {
            reg = getReadOnlyReg(op->reg);
        }
        this->x86.cmpxchg(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0);
        reg = nullptr;
        eax = nullptr;
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cmpxchge16r16_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_cmpxchge8r8_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_8bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr eax = getReg(0, 0);
        if (eax->hardwareReg() != 0) {
            kpanic("X86DynamicCodeGen::dynamic_cmpxchge32r32_lock");
        }
        RegPtr reg;

        if (op->reg == 0) {
            reg = eax;
        } else {
            reg = getReadOnlyReg8(op->reg);
        }
        this->x86.cmpxchg(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0);
        reg = nullptr;
        eax = nullptr;
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_cmpxchge8r8_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_xchge32r32_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg(op->reg);
        this->x86.xchg(X86Asm::Reg32(reg->hardwareReg()), X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0);
        reg = nullptr;
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_xchge32r32_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_xchge16r16_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_16bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg(op->reg);
        this->x86.xchg(X86Asm::Reg16(reg->hardwareReg()), X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0);
        reg = nullptr;
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_xchge16r16_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_xchge8r8_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_8bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg8(op->reg);
        this->x86.xchg(X86Asm::Reg8(get8bitReg(reg)), X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0);
        reg = nullptr;
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_xchge8r8_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_arithE32R32_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, std::function<void()> fallback, bool writeReg) {
    DynamicCodeGen::write(DYN_32bit, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg(op->reg);
        callback(reg, address, offset);
        reg = nullptr;
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}

void X86DynamicCodeGen::dynamic_arithE16R16_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, std::function<void()> fallback, bool writeReg) {
    DynamicCodeGen::write(DYN_16bit, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg(op->reg);
        callback(reg, address, offset);
        reg = nullptr;
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}
void X86DynamicCodeGen::dynamic_arithE8R8_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, std::function<void()> fallback, bool writeReg) {
    DynamicCodeGen::write(DYN_8bit, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg8(op->reg);
        callback(reg, address, offset);
        reg = nullptr;
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}

void X86DynamicCodeGen::dynamic_arithE32_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback, std::function<void()> fallback) {
    DynamicCodeGen::write(DYN_32bit, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        callback(address, offset);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}

void X86DynamicCodeGen::dynamic_arithE16_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback, std::function<void()> fallback) {
    DynamicCodeGen::write(DYN_16bit, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        callback(address, offset);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}
void X86DynamicCodeGen::dynamic_arithE8_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback, std::function<void()> fallback) {
    DynamicCodeGen::write(DYN_8bit, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        callback(address, offset);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [fallback]() {
        fallback();
    });
}

void X86DynamicCodeGen::dynamic_xaddr32e32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.xadd(X86Asm::Reg32(dest->hardwareReg()), X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_xaddr32e32_lock(op);
    }, true);
}

void X86DynamicCodeGen::dynamic_xaddr16e16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.xadd(X86Asm::Reg16(dest->hardwareReg()), X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_xaddr16e16_lock(op);
    }, true);
}
void X86DynamicCodeGen::dynamic_xaddr8e8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.xadd(X86Asm::Reg8(get8bitReg(dest)), X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_xaddr8e8_lock(op);
    }, true);
}

void X86DynamicCodeGen::dynamic_adde32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.addMemReg(X86Asm::Reg32(dest->hardwareReg()), X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_adde32r32_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_adde16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.addMemReg(X86Asm::Reg16(dest->hardwareReg()), X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_adde16r16_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_adde8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.addMemReg(X86Asm::Reg8(get8bitReg(dest)), X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_adde8r8_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_add32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.addMem32(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0, op->imm);
    }, [op, this]() {
        DynamicCodeGen::dynamic_add32_mem_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_add16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.addMem16(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0, op->imm);
    }, [op, this]() {
        DynamicCodeGen::dynamic_add16_mem_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_add8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.addMem8(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0, op->imm);
    }, [op, this]() {
        DynamicCodeGen::dynamic_add8_mem_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_sube32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.subMemReg(X86Asm::Reg32(dest->hardwareReg()), X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_sube32r32_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_sube16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.subMemReg(X86Asm::Reg16(dest->hardwareReg()), X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_sube16r16_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_sube8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.subMemReg(X86Asm::Reg8(get8bitReg(dest)), X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_sube8r8_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_sub32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.subMem32(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0, op->imm);
    }, [op, this]() {
        DynamicCodeGen::dynamic_sub32_mem_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_sub16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.subMem16(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0, op->imm);
    }, [op, this]() {
        DynamicCodeGen::dynamic_sub16_mem_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_sub8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.subMem8(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0, op->imm);
    }, [op, this]() {
        DynamicCodeGen::dynamic_sub8_mem_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_inc32_mem32_lock(DecodedOp* op) {    
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.incMem32(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_inc32_mem32_lock(op);
    }); 
}
void X86DynamicCodeGen::dynamic_inc16_mem16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.incMem16(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_inc16_mem16_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_inc8_mem8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.incMem8(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_inc8_mem8_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_dec32_mem32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.decMem32(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_dec32_mem32_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_dec16_mem16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.decMem16(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_dec16_mem16_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_dec8_mem8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.decMem8(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_dec8_mem8_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_note32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.notMem32(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_note32_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_note16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.notMem16(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_note16_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_note8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.notMem8(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_note8_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_nege32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.negMem32(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_nege32_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_nege16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.negMem16(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_nege16_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_nege8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.negMem8(X86Asm::Reg32(addressReg->hardwareReg()), X86Asm::Reg32(offsetReg->hardwareReg()), 0, 0);
    }, [op, this]() {
        DynamicCodeGen::dynamic_nege8_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_btse32_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.btsMem32(X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0, (U8)imm);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_btse32_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_btse16_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_16bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.btsMem16(X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0, (U8)imm);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_btse16_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_btse32r32_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_32bit, calculateEffectiveEaa32(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(DYN_32bit, reg, 0x1f, false);
        this->x86.lock();
        this->x86.btsMem32(X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0, X86Asm::Reg32(reg->hardwareReg()));
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_btse32r32_lock(op);
    });  
}

void X86DynamicCodeGen::dynamic_btse16r16_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_16bit, calculateEffectiveEaa16(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(DYN_16bit, reg, 0xf, false);
        this->x86.lock();
        this->x86.btsMem16(X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0, X86Asm::Reg16(reg->hardwareReg()));
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_btse16r16_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_btre32_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.btrMem32(X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0, (U8)imm);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_btre32_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_btre16_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_16bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.btrMem16(X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0, (U8)imm);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_btre16_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_btre32r32_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_32bit, calculateEffectiveEaa32(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(DYN_32bit, reg, 0x1f, false);
        this->x86.lock();
        this->x86.btrMem32(X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0, X86Asm::Reg32(reg->hardwareReg()));
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_btre32r32_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_btre16r16_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_16bit, calculateEffectiveEaa16(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(DYN_16bit, reg, 0xf, false);
        this->x86.lock();
        this->x86.btrMem16(X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0, X86Asm::Reg16(reg->hardwareReg()));
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_btre16r16_lock(op);
    });
}

void X86DynamicCodeGen::dynamic_btce32_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_32bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.btcMem32(X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0, (U8)imm);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_btce32_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_btce16_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_16bit, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.btcMem16(X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0, (U8)imm);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_btce16_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_btce32r32_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_32bit, calculateEffectiveEaa32(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(DYN_32bit, reg, 0x1f, false);
        this->x86.lock();
        this->x86.btcMem32(X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0, X86Asm::Reg32(reg->hardwareReg()));
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_btce32r32_lock(op);
    });
}
void X86DynamicCodeGen::dynamic_btce16r16_lock(DecodedOp* op) {
    DynamicCodeGen::write(DYN_16bit, calculateEffectiveEaa16(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(DYN_16bit, reg, 0xf, false);
        this->x86.lock();
        this->x86.btcMem16(X86Asm::Reg32(address->hardwareReg()), X86Asm::Reg32(offset->hardwareReg()), 0, 0, X86Asm::Reg16(reg->hardwareReg()));
        updateFlagsIfNecessary();
        incrementEip(op->len);
    }, [op, this]() {
        DynamicCodeGen::dynamic_btce16r16_lock(op);
    });
}

U8* X86DynamicCodeGen::createStartJITCode() {
    x86.push(x86.ebx);
    x86.push(x86.edi);
    x86.push(x86.esi);
    x86.push(x86.ebp);
    // on win32 ecx contains cpu
    x86.mov(x86.edi, x86.ecx);

    x86.push(x86.edx); // passed in op
    x86.push(x86.edi);
    x86.call(::dynamic_fillFlags);
    x86.pop(x86.edi);
    x86.pop(x86.edx);

    loadCache();

    // :TODO: what about other x86 platforms that use a different calling convention
    // 
    // jmp ((DecodedOp*)edx)->pfn
    x86.readMem(x86.eax, x86.edx, offsetof(DecodedOp, pfnJitCode));
    x86.jmp(x86.eax);
    U8* result = createDynamicExecutableMemory();
    patch(result);
    return result;
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

void X86DynamicCodeGen::loadCache() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i]) {
            x86.readMem(X86Asm::Reg32(regCache[i]), x86.edi, CPU::offsetofReg32(i));
        }
    }
}

void X86DynamicCodeGen::writeCache() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i]) {
            x86.writeMem(x86.edi, CPU::offsetofReg32(i), X86Asm::Reg32(regCache[i]));
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